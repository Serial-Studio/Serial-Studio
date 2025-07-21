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

import re
import os
import time
import openai
import subprocess

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
    "zh_CN": "Simplified Chinese",
    "uk_UA": "Ukrainian",
    "cs_CZ": "Czech"
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
        source_texts (list of (str, str)): Tuples of (context_location, source_text).
        target_language (str): Human-readable name (e.g., "French").

    Returns:
        list of str: Translated strings with consistent formatting.
    """

    def parse_numbered_translations(raw_text: str, expected_count: int) -> list[str]:
        # Remove any markdown fences
        raw_text = re.sub(r"^```[a-z]*", "", raw_text, flags=re.MULTILINE)
        raw_text = re.sub(r"```$", "", raw_text, flags=re.MULTILINE)

        # Extract lines starting with numbered prefix
        numbered_lines = re.findall(r"^\s*(\d+)\.\s*(.*)$", raw_text, flags=re.MULTILINE)

        translation_map = {}
        for index_str, line in numbered_lines:
            index = int(index_str)
            # Strip context like: [Component @ path:line]
            line = re.sub(r"^\[.*?\]\s*", "", line).strip()
            translation_map[index] = line.replace("\\n", "\n").strip()

        # Validate all expected translations are present and non-empty
        translations = []
        for i in range(1, expected_count + 1):
            if i not in translation_map:
                raise ValueError(f"Missing translation for line {i}")
            if not translation_map[i]:
                raise ValueError(f"Empty translation for line {i}")
            translations.append(translation_map[i])

        return translations

    # Escape newlines to prevent prompt corruption
    escaped_entries = [(loc, text.replace("\n", "\\n")) for loc, text in source_texts]

    # Build prompt with filename/line context
    prompt = "\n".join([
        f"Translate the following user interface strings to {target_language}.",
        "The context is a technical application named Serial Studio, written in Qt/QML.",
        "Each string includes its source context in square brackets: [Component @ File:Line].",
        "",
        "IMPORTANT:",
        f"- Output exactly {len(escaped_entries)} lines, numbered 1 to {len(escaped_entries)}.",
        "- Do not skip numbers.",
        "- Do not include code blocks like ```plaintext.",
        "- Do not include the content inside square brackets (e.g. [Component @ File.qml:123]) in the translation. It is just context.",
        "",
        "Follow these strict rules:",
        "1. Preserve all placeholders exactly (e.g. %1, {0}), as well as line breaks, punctuation, and spacing.",
        "2. Do not rephrase, simplify, or add/remove meaning. Translate literally and precisely.",
        "3. Maintain meaning for all technical/computer-related terminology (e.g. 'unsigned int', '24-bit'). Do not mistranslate or generalize these.",
        "4. Keep translations concise, ideally matching the original character length to fit UI constraints.",
        "5. Respond with one translated string per line, same order, prefixed by its number.",
        "",
    ] + [f"{i+1}. [{loc}] {line}" for i, (loc, line) in enumerate(escaped_entries)])

    # Send to OpenAI with 40% Radagast factor
    client = openai.OpenAI(api_key=os.environ.get("OPENAI_API_KEY"))
    response = client.chat.completions.create(
        model=OPENAI_MODEL,
        messages=[{"role": "user", "content": prompt}],
        temperature=0.4,
        max_tokens=2048
    )

    raw_text = response.choices[0].message.content.strip()

    # Parse and validate
    try:
        translations = parse_numbered_translations(raw_text, len(source_texts))
    except ValueError as e:
        print("GPT RAW RESPONSE:\n", raw_text)
        raise e

    # Enforce line break consistency
    final_output = []
    for (_, src_text), trans in zip(source_texts, translations):
        expected = src_text.count("\n") + 1
        actual = trans.count("\n") + 1
        if actual != expected:
            print(f"[!] Line break mismatch: '{src_text}'")
            lines = trans.splitlines()
            while len(lines) < expected:
                lines.append("")
            trans = "\n".join(lines[:expected])

        # Log cleanly
        print(f"\n[EN] {src_text}\n[{target_language.upper()}] {trans}\n")

        final_output.append(trans)

    return final_output

#------------------------------------------------------------------------------
# Core Translation Logic
#------------------------------------------------------------------------------

def translate_ts_file(filename, batch_size=10):
    """
    Process a TS file and translate all unfinished or empty entries.

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
        context_name = context.findtext("name", default="unknown")

        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            # Get all locations
            locations = message.findall("location")
            if locations:
                location_str = ", ".join(f"{loc.get('filename')}:{loc.get('line')}" for loc in locations)
            else:
                location_str = "unknown"

            context_tag = f"{context_name} @ {location_str}"

            # Skip malformed or empty source
            if source is None or not source.text or not source.text.strip():
                continue

            # Create missing <translation> tag if needed
            if translation is None:
                translation = etree.SubElement(message, "translation")

            translation_text = translation.text or ""

            # Determine if entry needs update:
            # Either marked unfinished OR translation is empty/whitespace
            needs_translation = (
                translation.get("type") == "unfinished"
                or not translation_text.strip()
            )

            if not needs_translation:
                continue

            # Accumulate batch
            source_batch.append((context_tag, source.text))
            translation_nodes.append(translation)

            # Flush if full
            if len(source_batch) >= batch_size:
                try:
                    results = translate_batch(source_batch, target_language)
                    for node, text in zip(translation_nodes, results):
                        node.text = text
                        node.attrib.pop("type", None)  # Mark as finished
                        total_updated += 1
                    source_batch.clear()
                    translation_nodes.clear()
                    time.sleep(1)
                except Exception as e:
                    print(f"Error in batch: {e}")
                    source_batch.clear()
                    translation_nodes.clear()

    # Final batch
    if source_batch:
        try:
            results = translate_batch(source_batch, target_language)
            for node, text in zip(translation_nodes, results):
                node.text = text
                node.attrib.pop("type", None)
                total_updated += 1
        except Exception as e:
            print(f"Error in final batch: {e}")

    # Save updated file
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