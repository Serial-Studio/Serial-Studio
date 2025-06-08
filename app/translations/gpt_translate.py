#
# Copyright (c) 202 Alex Spataru <https://github.com/alex-spataru>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

import os
import time
import subprocess
import openai
from lxml import etree

import os
import time
import subprocess
import openai
from lxml import etree

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

# Path to the directory containing .ts translation files
TS_DIRECTORY = "ts"

# OpenAI model to use
OPENAI_MODEL = "gpt-4o"

# Absolute path to the directory containing this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Mapping from TS file locale codes to full language names
LANGUAGE_MAP = {
    "de_DE": "German",
    "es_MX": "Mexican Spanish",
    "fr_FR": "French",
    "it_IT": "Italian",
    "ja_JP": "Japanese",
    "ko_KR": "Korean",
    "pl_PL": "Polish",
    "pt_BR": "Brazilian Portuguese",
    "ru_RU": "Russian",
    "tr_TR": "Turkish",
    "zh_CN": "Simplified Chinese"
}

#------------------------------------------------------------------------------
# Utility Functions
#------------------------------------------------------------------------------

def run_qt_translation_tool(mode):
    """
    Run the Qt translation manager to update or compile translations.
    Valid modes: '--lupdate' or '--lrelease'.
    """
    subprocess.run(
        ["python3", "translation_manager.py", mode],
        cwd=SCRIPT_DIR,
        check=True
    )

def remove_wrapping_quotes(text):
    """
    Remove single or double quotes around a string.
    """
    text = text.strip()
    if (text.startswith('"') and text.endswith('"')) or (text.startswith("'") and text.endswith("'")):
        return text[1:-1].strip()
    
    return text

def translate_batch(source_texts, target_language):
    """
    Translate a batch of UI strings to a given target language using OpenAI.
    
    Args:
        source_texts (list of str): The English strings to translate.
        target_language (str): Human-readable name (e.g., "French").

    Returns:
        list of str: Translated strings with consistent formatting.
    """
    # Escape newlines to prevent prompt corruption
    escaped_texts = [text.replace("\n", "\\n") for text in source_texts]

    # Build prompt
    prompt = "\n".join([
        f"Translate the following UI strings to {target_language}, preserving formatting (e.g., placeholders like %1, line breaks).",
        "Respond with one translation per line, matching the order below.",
        ""
    ] + [f"{i+1}. {line}" for i, line in enumerate(escaped_texts)])

    # Send to OpenAI with 40% Radagast factor
    client = openai.OpenAI(api_key=os.environ.get("OPENAI_API_KEY"))
    response = client.chat.completions.create(
        model=OPENAI_MODEL,
        messages=[{"role": "user", "content": prompt}],
        temperature=0.4, 
    )

    # Extract and clean up results
    raw_lines = response.choices[0].message.content.strip().splitlines()
    translations = [
        line.strip().lstrip(f"{i+1}.").strip().replace("\\n", "\n")
        for i, line in enumerate(raw_lines)
    ]

    if len(translations) != len(source_texts):
        raise ValueError("Mismatch in source/translated line count")

    # Enforce line break consistency
    final_output = []
    for src, trans in zip(source_texts, translations):
        expected = src.count("\n") + 1
        actual = trans.count("\n") + 1
        if actual != expected:
            print(f"[!] Line break mismatch: '{src}'")
            lines = trans.splitlines()
            while len(lines) < expected:
                lines.append("")
            trans = "\n".join(lines[:expected])

        # Print source and translated line
        print(f"\n[EN] {src}\n[{target_language.upper()}] {trans}\n")

        final_output.append(trans)

    return final_output

#------------------------------------------------------------------------------
# Core Translation Logic
#------------------------------------------------------------------------------

def translate_ts_file(filename, batch_size=10):
    """
    Process a TS file and translate all unfinished entries.

    Args:
        filename (str): TS file to process (e.g., 'fr_FR.ts').
        batch_size (int): Number of strings to translate per OpenAI request.
    """
    # Determine target language
    lang_code = filename.replace(".ts", "")
    target_language = LANGUAGE_MAP.get(lang_code)
    if not target_language:
        print(f"Skipping {filename}: unsupported language code.")
        return

    # Load and parse the TS XML file
    ts_path = os.path.join(SCRIPT_DIR, TS_DIRECTORY, filename)
    tree = etree.parse(ts_path)
    root = tree.getroot()
    total_updated = 0

    # Initialize batching buffers
    source_batch = []
    translation_nodes = []

    # Iterate through all <message> elements
    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            # Skip empty or malformed entries
            if source is None or not source.text or not source.text.strip():
                continue

            # Create missing <translation> tag
            if translation is None:
                translation = etree.SubElement(message, "translation")

            # Skip already translated strings
            is_unfinished = (
                translation.get("type") == "unfinished"
                or not translation.text
                or not translation.text.strip()
            )
            if not is_unfinished:
                continue

            # Accumulate batch for translation
            source_batch.append(source.text)
            translation_nodes.append(translation)

            # Send batch if full
            if len(source_batch) >= batch_size:
                try:
                    results = translate_batch(source_batch, target_language)
                    for node, text in zip(translation_nodes, results):
                        node.text = text
                        node.attrib.pop("type", None)  # Remove "unfinished" flag
                        total_updated += 1
                    source_batch.clear()
                    translation_nodes.clear()
                    time.sleep(1)  # Rate limit
                except Exception as e:
                    print(f"Error in batch: {e}")
                    source_batch.clear()
                    translation_nodes.clear()

    # Process final incomplete batch
    if source_batch:
        try:
            results = translate_batch(source_batch, target_language)
            for node, text in zip(translation_nodes, results):
                node.text = text
                node.attrib.pop("type", None)
                total_updated += 1
        except Exception as e:
            print(f"Error in final batch: {e}")

    # Save translated file
    tree.write(ts_path, encoding="utf-8", xml_declaration=True)
    print(f"{filename}: {total_updated} strings translated.")

#------------------------------------------------------------------------------
# Entry Point
#------------------------------------------------------------------------------

def main():
    """Run full translation pipeline: update sources, translate, then compile."""
    run_qt_translation_tool("--lupdate")

    for file in os.listdir(os.path.join(SCRIPT_DIR, TS_DIRECTORY)):
        if file.endswith(".ts") and file != "en_US.ts":
            translate_ts_file(file)

    run_qt_translation_tool("--lrelease")

if __name__ == "__main__":
    main()