#!/usr/bin/env python3
#
# Copyright (c) 2025 Alex Spataru <https://github.com/alex-spataru>
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
import openai

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

# OpenAI model to use
OPENAI_MODEL = "gpt-4o"

# Absolute path to the directory containing this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Path to messages directory
MESSAGES_DIR = os.path.join(SCRIPT_DIR, "..", "rcc", "messages")

# Subdirectories to process
SUBDIRS = ["gpl3", "pro", "trial"]

# Mapping from file suffix to full language names
LANGUAGE_MAP = {
    "DE": "German",
    "ES": "Mexican Spanish",
    "FR": "French",
    "IT": "Italian",
    "JA": "Japanese",
    "KO": "Korean",
    "PL": "Polish",
    "PT": "Brazilian Portuguese",
    "RU": "Russian",
    "TR": "Turkish",
    "ZH": "Simplified Chinese",
    "UK": "Ukrainian",
    "CZ": "Czech"
}

#------------------------------------------------------------------------------
# Translation Function
#------------------------------------------------------------------------------

def translate_welcome_message(source_text, target_language, context):
    """
    Translate a Welcome message file to the target language using OpenAI.

    Args:
        source_text (str): The English source text.
        target_language (str): Human-readable name (e.g., "French").
        context (str): Context about which version (gpl3, pro, trial).

    Returns:
        str: Translated text.
    """
    prompt = f"""Translate the following welcome message from Serial Studio to {target_language}.

Context: This is the welcome message for Serial Studio {context} version.

IMPORTANT RULES:
1. Keep the ASCII art header (first 3 lines) EXACTLY as-is. Do not translate or modify it.
2. Preserve all URLs exactly as they appear (do not translate).
3. Preserve all placeholders like %1 exactly as they appear.
4. Preserve all special characters like arrows (→) and bullet points.
5. Translate only the actual text content, maintaining the same structure and line breaks.
6. Keep technical terms like "MQTT", "XY", "3D", "GPLv3", "GPL" untranslated.
7. Maintain the professional tone appropriate for software licensing and product descriptions.
8. Keep email addresses and URLs on the same lines as in the original.
9. Keep each line to at most 73 characters (preferred) or 80 characters maximum to ensure proper text wrapping. Break longer lines naturally at word boundaries.

Source text:
{source_text}

Provide ONLY the translated text, with no additional commentary or explanation."""

    client = openai.OpenAI(api_key=os.environ.get("OPENAI_API_KEY"))
    response = client.chat.completions.create(
        model=OPENAI_MODEL,
        messages=[{"role": "user", "content": prompt}],
        temperature=0.3,
        max_tokens=2048
    )

    translated_text = response.choices[0].message.content.strip()

    # Remove any markdown code blocks that might have been added
    if translated_text.startswith("```"):
        lines = translated_text.split("\n")
        # Remove first and last lines if they're code fence markers
        if lines[0].startswith("```"):
            lines = lines[1:]
        if lines and lines[-1].strip() == "```":
            lines = lines[:-1]
        translated_text = "\n".join(lines)

    return translated_text

#------------------------------------------------------------------------------
# Core Translation Logic
#------------------------------------------------------------------------------

def translate_welcome_files_in_subdir(subdir):
    """
    Translate all Welcome files in a given subdirectory.

    Args:
        subdir (str): Subdirectory name (gpl3, pro, or trial).
    """
    subdir_path = os.path.join(MESSAGES_DIR, subdir)

    # Read the English source file
    en_file_path = os.path.join(subdir_path, "Welcome_EN.txt")
    if not os.path.exists(en_file_path):
        print(f"Warning: {en_file_path} not found. Skipping {subdir}.")
        return

    with open(en_file_path, "r", encoding="utf-8") as f:
        source_text = f.read()

    print(f"\n{'='*60}")
    print(f"Processing {subdir} welcome messages...")
    print(f"{'='*60}\n")

    # Translate to each language
    for lang_code, lang_name in LANGUAGE_MAP.items():
        output_file = os.path.join(subdir_path, f"Welcome_{lang_code}.txt")

        print(f"Translating to {lang_name} ({lang_code})...")

        try:
            translated_text = translate_welcome_message(
                source_text,
                lang_name,
                subdir
            )

            # Ensure the file ends with a newline
            if not translated_text.endswith("\n"):
                translated_text += "\n"

            # Write the translated file
            with open(output_file, "w", encoding="utf-8") as f:
                f.write(translated_text)

            print(f"✓ Saved to {output_file}\n")

            # Rate limiting to avoid API throttling
            time.sleep(1)

        except Exception as e:
            print(f"✗ Error translating to {lang_name}: {e}\n")
            continue

#------------------------------------------------------------------------------
# Entry Point
#------------------------------------------------------------------------------

def main():
    """Translate all Welcome message files in all subdirectories."""
    print("Serial Studio Welcome Message Translator")
    print("Using OpenAI model:", OPENAI_MODEL)

    for subdir in SUBDIRS:
        translate_welcome_files_in_subdir(subdir)

    print("\n" + "="*60)
    print("Translation complete!")
    print("="*60)

if __name__ == "__main__":
    main()
