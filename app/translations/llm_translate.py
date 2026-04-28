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
# USAGE:
#   Translate missing strings (default Anthropic claude-sonnet-4-5):
#     python3 llm_translate.py
#
#   Use OpenAI instead:
#     python3 llm_translate.py --provider openai
#
#   Override the model:
#     python3 llm_translate.py --provider anthropic --model claude-sonnet-4-5
#
#   Translate only one language:
#     python3 llm_translate.py --lang fr_FR
#
#   Reset (clear) all translations and re-run from scratch:
#     python3 llm_translate.py --reset
#     python3 llm_translate.py --reset --lang fr_FR     # one language only
#
#   Re-apply deterministic capitalization (NO LLM):
#     python3 llm_translate.py --verify-only
#
#   Lint en_US.ts for source-string style violations (NO LLM):
#     python3 llm_translate.py --lint-sources
#
# QUALITY FEATURES:
#   1. Domain glossary in the prompt — disambiguates 'Will Topic' (MQTT field),
#      'Holding Register' (Modbus), 'Frame' (CAN/serial vs UI), 'Dataset' (data
#      series). See DOMAIN_GLOSSARY.
#   2. Translation memory — top-K most similar already-validated translations
#      from the same .ts file are injected as few-shot examples per batch.
#   3. Confidence scoring — the LLM rates each translation 1-5; entries with
#      score < --min-score (default 4) keep type='unfinished' so Qt Linguist
#      surfaces them for human review.
#   4. Source-aware acronym enforcement — only acronyms present in the EN
#      source get force-uppercased in the translation. 'can'/'mac' as words
#      are safe.
#   5. Language-aware title case — Spanish 'Cuadrícula de Datos' (lowercase
#      'de'), French 'Grille de Données', German capitalizes nouns.
#   6. en_US.ts is never sent to the LLM — translations are direct copies of
#      the source strings.
#
# WRITING STYLE (enforced in the GPT prompt and the lint mode):
#   The Serial Studio UI follows an Apple-HIG-style voice — concise, neutral,
#   functional. Translators must preserve that voice in the target language.
#
#   - Actions (buttons, menu items): imperative verbs ("Save", "Delete", "Try Again").
#     Never noun forms ("Deletion", "Cancellation").
#   - Labels (sections, settings, titles): noun phrases ("Account Settings", "Privacy").
#   - States / status: present tense or short descriptive phrases
#     ("No Connection", "Uploading…", "Sync Complete").
#   - Completed actions: short past-participle forms ("Saved", "Upload Complete").
#   - No future tense ("will be saved" → "Saving…" / "Saved").
#   - No user address ("you", "we", "your") unless the source string already uses it.
#   - No filler politeness ("Please", "Sorry") unless the source already includes it.
#   - Errors: direct and neutral. Never blame the user.
#   - Ellipsis "…" only when the action requires more input or leads to another step
#     ("Save As…", "Add Account…").
#   - Title-style capitalization for labels and short actions, adapted to language
#     conventions (Spanish lowercases "de"/"la"/"el"; German capitalizes nouns; etc.).
#   - Technical acronyms stay UPPERCASE in every language ("CAN Bus", "TCP/IP", "JSON").
#
# CAPITALIZATION POST-PROCESS (deterministic, runs after every GPT response and in
# --verify-only mode):
#   - "PAUSE" → translation forced to UPPERCASE.
#   - "Data Grid" (Spanish) → "Cuadrícula de Datos" (lowercase "de").
#   - "Data Grid" (French) → "Grille de Données" (lowercase "de").
#   - Single-word Title Case sources ("Dataset") still propagate title case to
#     multi-word translations ("Conjunto de Datos", not "Conjunto de datos").
#   - "About" (sentence case) → only the first letter capitalized.
#   - SKIPPED for caseless scripts (zh, ja, ko, hi, ar, th, he).
#

import re
import os
import sys
import time
import random
import threading
import subprocess

from io import StringIO
from concurrent.futures import ThreadPoolExecutor, as_completed
from lxml import etree

# Provider SDKs are imported lazily inside _call_provider() so the script
# still runs when only one of them is installed.

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

# Path to the directory containing .ts translation files
TS_DIRECTORY = "ts"

# Default provider and model. Override at runtime with --provider / --model.
# Provider IDs: "openai" or "anthropic".
DEFAULT_PROVIDER = "anthropic"

# OpenAI: gpt-4.1 follows long structured prompts cleanly. gpt-4o tends to
# paraphrase. gpt-5 hallucinates terminology in our experience.
OPENAI_MODEL_DEFAULT = "gpt-4.1"

# Anthropic: claude-sonnet-4-5 is the current strong model. Claude 3.5 Sonnet
# is two generations old and noticeably worse at instruction-following on
# the long structured prompt this script sends.
ANTHROPIC_MODEL_DEFAULT = "claude-sonnet-4-5"

# Resolved at runtime in main(). Modules outside this script should treat
# LLM_MODEL as the active model name.
LLM_MODEL = ANTHROPIC_MODEL_DEFAULT
PROVIDER = DEFAULT_PROVIDER

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
    "cs_CZ": "Czech",
    "hi_IN": "Hindi",
    "ro_RO": "Romanian",
    "nl_NL": "Dutch",
    "sv_SE": "Swedish"
}

# Languages that use scripts WITHOUT uppercase/lowercase distinction
# Capitalization verification will be SKIPPED for these
CASELESS_SCRIPTS = {
    "ja_JP",  # Japanese (Hiragana/Katakana/Kanji)
    "ko_KR",  # Korean (Hangul)
    "zh_CN",  # Simplified Chinese (Hanzi)
    "hi_IN",  # Hindi (Devanagari)
    # Add more as needed: "ar_SA" (Arabic), "th_TH" (Thai), "he_IL" (Hebrew)
}

# Language-specific "small words" that should be lowercase in title case
# (except when they're the first or last word)
TITLE_CASE_SMALL_WORDS = {
    "es_MX": {  # Spanish
        "de", "del", "la", "las", "el", "los", "un", "una", "unos", "unas",
        "y", "e", "o", "u", "en", "con", "por", "para", "sin", "sobre",
        "a", "al", "desde", "hasta"
    },
    "fr_FR": {  # French
        "de", "du", "des", "la", "le", "les", "un", "une",
        "et", "ou", "à", "au", "aux", "en", "dans", "pour", "avec",
        "par", "sur", "sous", "sans"
    },
    "pt_BR": {  # Portuguese
        "de", "do", "da", "dos", "das", "o", "a", "os", "as", "um", "uma",
        "e", "ou", "em", "no", "na", "nos", "nas", "com", "por", "para",
        "sem", "sob", "sobre"
    },
    "it_IT": {  # Italian
        "di", "del", "della", "dei", "delle", "il", "lo", "la", "i", "gli", "le",
        "un", "uno", "una", "e", "o", "in", "con", "per", "a", "da", "su"
    },
    "de_DE": {  # German (capitalize ALL nouns, but small words exist)
        "der", "die", "das", "den", "dem", "des", "ein", "eine", "einen",
        "und", "oder", "in", "von", "zu", "mit", "für", "auf", "an"
    },
    "tr_TR": {  # Turkish
        "ve", "veya", "ile", "için", "de", "da", "den", "dan", "bir"
    },
    "ru_RU": {  # Russian (Cyrillic)
        "и", "или", "в", "на", "с", "от", "до", "для", "к", "о", "об", "по"
    },
    "uk_UA": {  # Ukrainian (Cyrillic)
        "і", "та", "або", "в", "на", "з", "від", "до", "для", "к", "о", "по"
    },
    "pl_PL": {  # Polish
        "i", "lub", "w", "na", "z", "od", "do", "dla", "o", "po", "przez"
    },
    "cs_CZ": {  # Czech
        "a", "nebo", "v", "na", "s", "od", "do", "pro", "k", "o", "po"
    },
    "ro_RO": {  # Romanian
        "de", "și", "sau", "în", "pe", "cu", "la", "din", "pentru", "ca"
    },
    "nl_NL": {  # Dutch
        "de", "het", "een", "en", "of", "in", "op", "van", "voor", "met", "aan"
    },
    "sv_SE": {  # Swedish
        "och", "eller", "i", "på", "av", "för", "med", "till", "från", "om"
    },
}

# Technical acronyms that must remain ALL UPPERCASE in every language and every
# context. Anything Serial Studio actually deals with goes here. Case-insensitive
# matching: 'json', 'Json', 'JSON' all collapse to 'JSON'.
TECHNICAL_ACRONYMS = {
    # Buses / protocols (the core surface)
    "CAN", "CAN-FD", "TCP", "UDP", "IP", "TCP/IP", "MQTT", "BLE", "USB", "HID",
    "UART", "RTU", "ASCII", "RS232", "RS485", "RS422", "I2C", "SPI", "GPIO",
    "PWM", "ADC", "DAC", "DMA", "PLC", "SCADA", "OBD", "OBD2", "J1939",
    "ISOTP", "SLCAN", "SOCKETCAN", "PEAK", "VECTOR", "KVASER",
    # Network / IO
    "ICMP", "ARP", "DNS", "DHCP", "TLS", "SSL", "SSH", "FTP", "HTTP", "HTTPS",
    "WS", "WSS", "UDP", "VPN", "VLAN", "WAN", "LAN", "WLAN", "WIFI", "OTA",
    "QOS", "MAC", "POE",
    # Data formats
    "JSON", "XML", "YAML", "CSV", "TSV", "TOML", "INI", "PDF", "HTML", "MD",
    "GZIP", "ZIP", "TAR", "BZ2", "LZ4", "ZSTD", "BMP", "PNG", "JPG", "JPEG",
    "GIF", "SVG", "WAV", "MP3", "FLAC", "OPUS", "OGG",
    # Telemetry / measurement / signal processing
    "FFT", "DFT", "PSD", "RMS", "SNR", "DC", "AC", "EMC", "EMF", "EMI",
    "GPS", "GNSS", "NMEA", "IMU", "GYRO", "MAG", "PPM", "PPB",
    # Files / storage / databases
    "MDF", "MDF4", "DBC", "ASC", "BLF", "TRC", "SQL", "SQLITE", "CRC",
    "CRC16", "CRC32", "MD5", "SHA1", "SHA256", "FAT", "NTFS", "NFS", "EEPROM",
    "FRAM", "SRAM", "RAM", "ROM", "EXT2", "EXT3", "EXT4",
    # Tooling / dev
    "API", "SDK", "CLI", "GUI", "IDE", "RPC", "GRPC", "MCP", "JSON-RPC",
    "QML", "JS", "QT", "C++", "URL", "URI", "URN", "UTF", "UTF-8", "UTF-16",
    "BOM", "REGEX", "OS", "VM", "JIT", "AOT",
    # File-transfer protocols
    "XMODEM", "YMODEM", "ZMODEM", "KERMIT", "TFTP", "SFTP", "SCP",
    # CNC / lab instrumentation
    "GRBL", "GCODE", "GCODE", "GRBL-HAL", "SCPI", "VISA", "GPIB",
    # Hardware / vendor / certs
    "MCU", "DSP", "FPGA", "CPU", "GPU", "SOC", "PCB", "LED", "LCD", "OLED",
    "TFT", "FCC", "CE", "ROHS", "REACH", "ISO",
    # MQTT-specific (protocol field names that should never be translated)
    "MQTTS", "MQTT-SN",
    # Auth / mail
    "OAUTH", "OAUTH2", "JWT", "SAML", "SMTP", "IMAP", "POP3", "SASL",
    # Misc
    "ID", "UUID", "GUID", "PID", "VID", "EOT", "SOH", "STX", "ETX", "ENQ",
    "ACK", "NAK", "CAN", "DLE", "CR", "LF", "CRLF", "BOM", "BPS", "MBPS",
    "KBPS",
}

# Mixed-case product / protocol / library names. These keep their casing in
# every language and must not be translated. The translator should treat them
# as proper nouns. (TECHNICAL_ACRONYMS handles the all-caps cases above.)
PROTOCOL_NAMES = {
    "Modbus", "Bluetooth", "Wi-Fi", "Ethernet", "Serial Studio", "Pro",
    "Hobbyist", "Enterprise", "GitHub", "GitLab", "Lua", "JavaScript",
    "Python", "TypeScript", "Linux", "Windows", "macOS", "iOS", "Android",
    "Qt", "QML", "OpenSSL", "libusb", "hidapi", "QuaZip", "KissFFT",
    "QCodeEditor", "mdflib", "QSimpleUpdater", "LemonSqueezy", "SocketCAN",
    "PEAK-System", "Vector", "Kvaser", "PEAK", "FreeRTOS", "Arduino",
    "Raspberry Pi", "ESP32", "STM32", "Teensy", "Chart.js", "Mermaid",
}

# Domain glossary: terms that need disambiguation context for the LLM. Without
# this glossary the translator treats "Will Topic" as a future-tense English
# phrase ("will" + "topic") instead of an MQTT field name. The glossary is
# inlined in the GPT prompt verbatim — keep it tight and stable.
#
# Format: each entry is "term — short clarification". Bullet form, one line.
DOMAIN_GLOSSARY = [
    # MQTT — Last Will and Testament fields. These are the famous trap.
    "'Will Topic' — MQTT Last-Will field. Do NOT translate 'Will' as future tense — it is the noun in 'Last Will and Testament'. Keep 'Will' capitalized.",
    "'Will Message' — MQTT Last-Will payload. Same rule as 'Will Topic'.",
    "'Will QoS' — MQTT Last-Will Quality-of-Service level (0/1/2). Keep 'Will' and 'QoS' as-is.",
    "'Will Retain' — MQTT Last-Will retain flag. Keep 'Will' as the noun.",
    "'QoS' / 'QoS 0' / 'QoS 1' / 'QoS 2' — MQTT Quality of Service levels. Keep 'QoS' uppercase.",
    "'Retain' (MQTT) — flag asking the broker to retain the message. Translate as the corresponding flag/property name in the target language; do NOT add 'will'.",
    "'Topic' (MQTT) — channel/path string. Translate as the standard MQTT term in the target language ('tema' in Spanish, 'sujet' in French, 'Thema' in German).",
    "'Broker' (MQTT) — the MQTT message broker server. Keep 'Broker' or use the standard local term.",
    "'Publisher' / 'Subscriber' (MQTT) — translate as the standard MQTT roles in the target language.",
    # Modbus
    "'Modbus' — protocol name. Never translate. Always 'Modbus'.",
    "'Holding Register' / 'Input Register' / 'Coil' / 'Discrete Input' — Modbus register types. Use the standard Modbus terms in the target language; do not invent.",
    "'RTU' (Modbus RTU) — Modbus serial framing variant. Keep 'RTU' uppercase.",
    "'Slave ID' / 'Unit ID' / 'Server ID' (Modbus) — Modbus device address. Translate the descriptor; keep 'ID' uppercase.",
    # CAN / DBC
    "'CAN Bus' / 'CAN' — Controller Area Network. Always uppercase 'CAN'. Never translate 'CAN' as the English verb 'can'.",
    "'CAN-FD' — CAN Flexible Data-rate. Keep as 'CAN-FD'.",
    "'DBC' — Vector CAN database file format. Keep uppercase.",
    "'Frame' (CAN/serial) — a complete data packet. Translate using the conventional protocol-engineering term ('trama' in Spanish, 'trame' in French, 'Frame' or 'Rahmen' in German).",
    "'Message' (CAN/MQTT) — a CAN message or MQTT message. Standard term in target language.",
    "'Signal' (CAN DBC) — a named field within a CAN message. Standard term.",
    # Serial / UART
    "'Baud Rate' / 'Baudrate' — serial communication speed. Translate as the standard local term; never as 'baud rate' literally word-for-word.",
    "'Parity' / 'Stop Bits' / 'Flow Control' / 'Data Bits' — UART line parameters. Standard terms.",
    "'XON/XOFF' / 'RTS/CTS' / 'DTR/DSR' — flow-control schemes. Keep abbreviations as-is.",
    # File transfer
    "'XMODEM' / 'YMODEM' / 'ZMODEM' / 'Kermit' — file-transfer protocols. Never translate.",
    "'CRC' / 'CRC-16' / 'CRC-32' — Cyclic Redundancy Check. Keep uppercase.",
    "'Checksum' — translate as the standard local term.",
    # Telemetry / dashboard widgets
    "'Dashboard' — main visualization panel. Translate as the standard UI term ('tablero', 'tableau de bord', 'Dashboard').",
    "'Widget' — a UI component. Use 'widget' or the standard local term.",
    "'Dataset' — a named data series. Translate as the standard data-engineering term ('conjunto de datos' in Spanish, 'jeu de données' in French, 'Datensatz' in German).",
    "'Group' — a logical bundle of datasets. Standard term.",
    "'Frame Parser' — the user-defined script that splits an incoming byte stream into datasets. Translate descriptively; keep 'Frame' as the protocol-engineering noun.",
    "'Group Widget' / 'Dataset Widget' — a visualization tied to a group or dataset. Standard term.",
    # Visualizations
    "'Plot' / 'Multi-Plot' / 'Bar' / 'Gauge' / 'Compass' / 'Accelerometer' / 'Gyroscope' / 'GPS' / 'LED Panel' / 'Data Grid' / 'FFT Plot' / 'Plot3D' / 'Terminal' — widget types. Translate using standard local engineering terms.",
    "'Output Widget' (Pro) — a widget that sends data TO the device (button, slider, toggle, text field, panel). Translate descriptively.",
    # Project / build / tier
    "'Project Editor' — the form-based editor for .ssproj files. Translate descriptively, not as 'editor of project'.",
    "'Workspace' (Serial Studio) — a user-defined dashboard tab. Translate as the standard UI term ('espacio de trabajo', 'espace de travail', 'Arbeitsbereich').",
    "'Hobbyist' / 'Pro' / 'Enterprise' — Serial Studio license tiers. Keep English names.",
    "'Trial' — evaluation period. Standard term.",
    "'License Key' — translate descriptively.",
    # Scripting
    "'JavaScript' / 'Lua' — scripting languages. Never translate.",
    "'Transform' (per-dataset transform) — user script that converts a raw value to a final value. Translate as the math/data-engineering term.",
    # Sessions / database
    "'Session' (Serial Studio) — a recorded run stored in the SQLite database. Translate as 'sesión' / 'session' / 'Sitzung'.",
    "'Database Explorer' — UI for browsing recorded sessions. Translate descriptively.",
    "'Replay' / 'Playback' — playing back a recorded session. Standard term.",
    # Misc
    "'Plugin' / 'Extension' — translate as the standard local term.",
    "'Theme' — UI color theme. Standard term.",
    "'Console' / 'Terminal' — text-based I/O view. Standard term.",
]

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


# Translation-memory tuning. We pull the top-K most-similar already-validated
# entries from the same target .ts file and inject them as few-shot examples.
# K=8 is a sweet spot: enough variety to anchor terminology without burning
# context window. MAX_TM_SOURCE_LEN keeps a single huge paragraph from
# crowding everything out.
TM_TOP_K = 8
MAX_TM_SOURCE_LEN = 200


def _tokenize_for_tm(text: str) -> set[str]:
    """Return the set of lowercase alphanumeric tokens of length ≥ 3."""
    return {t for t in re.findall(r"[A-Za-z0-9]+", text.lower()) if len(t) >= 3}


def harvest_translation_memory(ts_root) -> list[tuple[str, str]]:
    """Return a list of (source, translation) pairs from finished entries.

    A finished entry has a non-empty <translation> with no type="unfinished".
    Long source strings are truncated by skipping (we don't want a 500-char
    paragraph dominating few-shot selection).
    """
    pairs = []
    for context in ts_root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")
            if source is None or not source.text:
                continue
            if translation is None or not translation.text:
                continue
            if translation.get("type") == "unfinished":
                continue
            if len(source.text) > MAX_TM_SOURCE_LEN:
                continue
            pairs.append((source.text, translation.text))
    return pairs


def select_tm_examples(batch_sources: list[str],
                       tm: list[tuple[str, str]],
                       k: int = TM_TOP_K) -> list[tuple[str, str]]:
    """Pick the K most relevant TM entries for the current batch.

    Score each TM entry by the maximum Jaccard token-overlap against any
    source string in the batch. Return the top K by score, dropping zero-
    overlap entries.
    """
    if not tm:
        return []

    batch_token_sets = [_tokenize_for_tm(s) for s in batch_sources]
    scored = []
    for src, trans in tm:
        src_tokens = _tokenize_for_tm(src)
        if not src_tokens:
            continue
        best = 0.0
        for bts in batch_token_sets:
            if not bts:
                continue
            inter = len(src_tokens & bts)
            if inter == 0:
                continue
            union = len(src_tokens | bts)
            score = inter / union
            if score > best:
                best = score
        if best > 0:
            scored.append((best, src, trans))

    scored.sort(key=lambda x: x[0], reverse=True)
    return [(src, trans) for _, src, trans in scored[:k]]


def format_tm_block(examples: list[tuple[str, str]], target_language: str) -> str:
    """Render TM examples as a prompt section the LLM can learn from."""
    if not examples:
        return ""

    lines = [
        "===========================================================================",
        "TRANSLATION MEMORY — MATCH THIS STYLE AND TERMINOLOGY",
        "===========================================================================",
        f"The following entries are already-validated translations from the same",
        f"Serial Studio project. They are the GROUND TRUTH for terminology choices",
        f"in {target_language}. Reuse the same word for the same English term.",
        "",
    ]
    for i, (src, trans) in enumerate(examples, 1):
        # Compress newlines so the example takes one logical line per side.
        src_one = src.replace("\n", " ⏎ ")
        trans_one = trans.replace("\n", " ⏎ ")
        lines.append(f"  {i}. EN: {src_one}")
        lines.append(f"     {target_language[:2].upper()}: {trans_one}")
    lines.append("")
    return "\n".join(lines)


def _call_provider(prompt: str, temperature: float = 0.2, max_tokens: int = 2048) -> str:
    """Send PROMPT to the active LLM provider and return the raw text response.

    Provider/model are read from the module-level PROVIDER and LLM_MODEL
    globals (the latter is reused for both providers — the global is named
    historically but holds whichever model is active). API keys come from
    OPENAI_API_KEY / ANTHROPIC_API_KEY.

    SDKs are imported lazily so the script runs with only one installed.
    """
    if PROVIDER == "openai":
        import openai
        api_key = os.environ.get("OPENAI_API_KEY")
        if not api_key:
            raise RuntimeError("OPENAI_API_KEY is not set")
        client = openai.OpenAI(api_key=api_key)
        response = client.chat.completions.create(
            model=LLM_MODEL,
            messages=[{"role": "user", "content": prompt}],
            temperature=temperature,
            max_tokens=max_tokens,
        )
        return response.choices[0].message.content

    if PROVIDER == "anthropic":
        import anthropic
        api_key = os.environ.get("ANTHROPIC_API_KEY")
        if not api_key:
            raise RuntimeError("ANTHROPIC_API_KEY is not set")
        client = anthropic.Anthropic(api_key=api_key)
        response = client.messages.create(
            model=LLM_MODEL,
            max_tokens=max_tokens,
            temperature=temperature,
            messages=[{"role": "user", "content": prompt}],
        )
        # Anthropic returns a list of content blocks; we only ever ask for text.
        parts = [b.text for b in response.content if getattr(b, "type", "text") == "text"]
        return "".join(parts)

    raise RuntimeError(f"Unknown provider: {PROVIDER!r}")


# Retry / backoff tuning. Designed for unattended overnight runs: a single
# batch can survive up to ~1 hour of provider trouble before giving up.
RETRY_MAX_ATTEMPTS = 10
RETRY_BASE_DELAY = 4.0      # seconds for the first backoff
RETRY_MAX_DELAY = 300.0     # 5 minute cap per backoff sleep
RETRY_TOTAL_BUDGET = 3600.0 # absolute wall-clock cap per batch (1 hour)


def _interruptible_sleep(seconds: float, stop_event: threading.Event | None = None) -> None:
    """Sleep in 0.5s slices so Ctrl-C and dashboard updates stay responsive."""
    deadline = time.monotonic() + max(0.0, seconds)
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return
        if stop_event is not None and stop_event.is_set():
            return
        time.sleep(min(0.5, remaining))


def _classify_provider_error(exc: BaseException) -> tuple[str, float | None, bool]:
    """Inspect a provider exception and decide what to do.

    Returns (kind, retry_after_hint, retryable):
        kind             — short label for the dashboard ('rate-limit', 'overloaded', ...)
        retry_after_hint — explicit retry_after value if the server gave one (seconds)
        retryable        — True if a retry has any chance of succeeding
    """
    name = type(exc).__name__

    # Anthropic SDK exception hierarchy
    try:
        import anthropic
        if isinstance(exc, anthropic.RateLimitError):
            hint = _extract_retry_after(exc)
            return ("rate-limit", hint, True)
        if isinstance(exc, anthropic.APITimeoutError):
            return ("timeout", None, True)
        if isinstance(exc, anthropic.APIConnectionError):
            return ("network", None, True)
        if isinstance(exc, anthropic.APIStatusError):
            status = getattr(exc, "status_code", None)
            if status in (408, 409, 425, 429, 500, 502, 503, 504, 529):
                kind = "overloaded" if status in (503, 529) else f"http-{status}"
                return (kind, _extract_retry_after(exc), True)
            return (f"http-{status}", None, False)
    except ImportError:
        pass

    # OpenAI SDK exception hierarchy
    try:
        import openai
        if isinstance(exc, openai.RateLimitError):
            return ("rate-limit", _extract_retry_after(exc), True)
        if isinstance(exc, openai.APITimeoutError):
            return ("timeout", None, True)
        if isinstance(exc, openai.APIConnectionError):
            return ("network", None, True)
        if isinstance(exc, openai.APIStatusError):
            status = getattr(exc, "status_code", None)
            if status in (408, 409, 425, 429, 500, 502, 503, 504):
                return (f"http-{status}", _extract_retry_after(exc), True)
            return (f"http-{status}", None, False)
    except ImportError:
        pass

    # Generic catch-all: assume retryable for unknown transient errors so a
    # flaky DNS or proxy hiccup doesn't kill the run.
    if isinstance(exc, (TimeoutError, ConnectionError, OSError)):
        return (name, None, True)

    return (name, None, False)


def _extract_retry_after(exc: BaseException) -> float | None:
    """Pull a Retry-After header (in seconds) from a provider exception."""
    response = getattr(exc, "response", None)
    headers = getattr(response, "headers", None) if response is not None else None
    if not headers:
        return None
    raw = headers.get("retry-after") or headers.get("Retry-After")
    if not raw:
        return None
    try:
        return float(raw)
    except (TypeError, ValueError):
        return None


def _call_provider_with_retry(prompt: str,
                              temperature: float = 0.2,
                              max_tokens: int = 2048,
                              status_fn=None,
                              stop_event: threading.Event | None = None) -> str:
    """Call the provider with exponential-backoff retry around transient errors.

    status_fn(kind: str, attempt: int, sleep_for: float) is invoked every time
    we back off, so the dashboard / log can show 'rate-limit, retry in 12s'.
    """
    deadline = time.monotonic() + RETRY_TOTAL_BUDGET
    last_exc: BaseException | None = None

    for attempt in range(1, RETRY_MAX_ATTEMPTS + 1):
        try:
            return _call_provider(prompt, temperature=temperature, max_tokens=max_tokens)
        except Exception as exc:
            last_exc = exc
            kind, hint, retryable = _classify_provider_error(exc)
            if not retryable or attempt == RETRY_MAX_ATTEMPTS:
                raise

            # Compute backoff. Honor server-supplied Retry-After when present;
            # otherwise exponential with jitter, clamped to RETRY_MAX_DELAY.
            if hint is not None and hint > 0:
                sleep_for = min(hint, RETRY_MAX_DELAY)
            else:
                base = RETRY_BASE_DELAY * (2 ** (attempt - 1))
                sleep_for = min(base, RETRY_MAX_DELAY)
                sleep_for *= 0.5 + random.random()

            # Don't sleep past the wall-clock budget.
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise
            sleep_for = min(sleep_for, remaining)

            if status_fn is not None:
                try:
                    status_fn(kind, attempt, sleep_for)
                except Exception:
                    pass

            _interruptible_sleep(sleep_for, stop_event)

    # Should be unreachable — the loop either returns, raises, or hits the
    # final attempt above.
    if last_exc is not None:
        raise last_exc
    raise RuntimeError("provider call failed without an exception")


def translate_batch(source_texts, target_language, lang_code=None, tm=None, log_fn=print,
                    status_fn=None, stop_event: threading.Event | None = None):
    """
    Translate a batch of UI strings to a given target language using the active provider.

    Args:
        source_texts (list of (str, str)): Tuples of (context_location, source_text).
        target_language (str): Human-readable name (e.g., "French").
        lang_code (str, optional): Locale code (e.g., "fr_FR"). Used by the
            deterministic capitalization post-process and to scope few-shot
            translation-memory examples.
        tm (list[tuple[str, str]], optional): Translation-memory pairs harvested
            from the same .ts file. The TOP_K most-relevant entries become
            few-shot examples in the prompt.
        log_fn (callable, optional): Sink for human-readable progress lines.
            Defaults to ``print``. Parallel callers pass a per-file buffered
            sink so concurrent batches don't interleave on stdout.
        status_fn (callable, optional): Invoked as ``status_fn(state, **info)``
            on lifecycle events (``'calling'``, ``'retry'``, ``'parsing'``).
            Used by the dashboard to render live state per language.
        stop_event (threading.Event, optional): If set, retry sleeps wake up
            early so Ctrl-C is responsive.

    Returns:
        list of (translation, score): tuples where score is the LLM's self-rated
        confidence in 1..5, or 5 if the LLM omitted a score.
    """

    def parse_numbered_translations(raw_text: str, expected_count: int) -> list[tuple[str, int]]:
        # Remove any markdown fences
        raw_text = re.sub(r"^```[a-z]*", "", raw_text, flags=re.MULTILINE)
        raw_text = re.sub(r"```$", "", raw_text, flags=re.MULTILINE)

        # Each line should look like:  '7. 5 | Guardar Archivo'
        # Be lenient — accept missing score (default 5) and missing pipe.
        numbered_lines = re.findall(r"^\s*(\d+)\.\s*(.*)$", raw_text, flags=re.MULTILINE)

        translation_map: dict[int, tuple[str, int]] = {}
        for index_str, line in numbered_lines:
            index = int(index_str)
            # Strip [Component @ path:line] metadata if the LLM echoed it.
            line = re.sub(r"^\[.*?\]\s*", "", line).strip()

            # Try to extract '<score> | <translation>'. Falls back to '5 |
            # <translation>' if no pipe found.
            score = 5
            text = line
            m = re.match(r"^\s*([1-5])\s*[|│]\s*(.*)$", line)
            if m:
                score = int(m.group(1))
                text = m.group(2).strip()
            else:
                # Sometimes the model writes "5 - text" or "(5) text" — handle.
                m = re.match(r"^\s*[\(]?([1-5])[\)]?\s*[-–—]\s*(.*)$", line)
                if m:
                    score = int(m.group(1))
                    text = m.group(2).strip()

            translation_map[index] = (text.replace("\\n", "\n").strip(), score)

        # Validate all expected translations are present and non-empty
        results: list[tuple[str, int]] = []
        for i in range(1, expected_count + 1):
            if i not in translation_map:
                raise ValueError(f"Missing translation for line {i}")
            text, score = translation_map[i]
            if not text:
                raise ValueError(f"Empty translation for line {i}")
            results.append((text, score))

        return results

    # Escape newlines to prevent prompt corruption
    escaped_entries = [(loc, text.replace("\n", "\\n")) for loc, text in source_texts]

    # Build the domain-context block first — it's the highest-leverage section.
    # Without this, common Serial Studio terms ('Will Topic', 'Holding Register',
    # 'Frame Parser') get mistranslated word-for-word.
    glossary_lines = ["- " + entry for entry in DOMAIN_GLOSSARY]
    protocol_list = ", ".join(sorted(PROTOCOL_NAMES))

    # Pick the most-relevant validated translations from the same .ts file and
    # include them as few-shot examples — anchors terminology choices.
    tm_examples = select_tm_examples([s for _, s in source_texts], tm or [])
    tm_block = format_tm_block(tm_examples, target_language)

    # Build prompt with filename/line context
    prompt = "\n".join([
        f"You are translating UI strings for Serial Studio into {target_language}.",
        "",
        "===========================================================================",
        "ABOUT SERIAL STUDIO — READ THIS FIRST",
        "===========================================================================",
        "Serial Studio is a cross-platform desktop application for engineers and",
        "embedded developers to visualize live telemetry from devices over UART,",
        "TCP/UDP, Bluetooth Low Energy, audio, USB, HID, Modbus (RTU and TCP),",
        "CAN bus, MQTT, and process I/O. It parses incoming byte streams using",
        "user-defined JavaScript or Lua scripts, then renders the values in a",
        "dashboard of plots, gauges, 3D plots, FFT views, GPS maps, data grids,",
        "and other engineering visualizations. It can record sessions to SQLite,",
        "export to CSV / MDF4 / PDF, and stream control commands back to the device.",
        "",
        "Audience: embedded firmware engineers, hardware engineers, lab/test",
        "technicians, telemetry operators. They expect protocol-correct terminology",
        "in their language — NOT word-for-word literal translation.",
        "",
        "Built with Qt/QML in C++20. Strings come from .cpp/.h/.qml source files.",
        "The [Component @ File:Line] tag in each input gives you the surrounding",
        "code context — use it to disambiguate ('Frame' in IO/Drivers/CANBus.cpp",
        "is a CAN frame; 'Frame' in Misc/UI is a UI box).",
        "",
        "===========================================================================",
        "DOMAIN GLOSSARY — DISAMBIGUATION RULES",
        "===========================================================================",
        "These are the terms most often mistranslated. Apply these rules even when",
        "the surrounding sentence looks like ordinary English. The glossary OVERRIDES",
        "general translation instinct.",
        "",
    ] + glossary_lines + [
        "",
        "===========================================================================",
        "PROTOCOL / PRODUCT NAMES — NEVER TRANSLATE",
        "===========================================================================",
        "Treat the following as proper nouns — keep their exact spelling and casing",
        "in every language, never inflect, never translate, never localize:",
        "",
        f"  {protocol_list}",
        "",
        "Plus: any all-uppercase token that looks like an acronym (CAN, USB, MQTT,",
        "TCP, JSON, FFT, GPS, NMEA, …) stays uppercase. See the rule block below.",
        "",
        tm_block,
        "===========================================================================",
        "OUTPUT FORMAT — STRICT",
        "===========================================================================",
        f"- Emit exactly {len(escaped_entries)} numbered lines, 1 through {len(escaped_entries)}, in order.",
        "- Each line MUST follow this exact shape:",
        "      N. <score> | <translation>",
        "  where <score> is a single digit 1-5 expressing your confidence:",
        "      5 = perfect match, idiomatic, terminology certain",
        "      4 = good, minor stylistic uncertainty",
        "      3 = acceptable, some terminology guess",
        "      2 = unsure of meaning or terminology, best-effort",
        "      1 = guess (rare technical term, ambiguous source)",
        "  Be honest. Low scores trigger human review — that is good, not bad.",
        "  Example: '7. 5 | Guardar Archivo'",
        "  Example: '8. 3 | Tema de Voluntad'   (uncertain about MQTT field name)",
        "- One translation per line. No blank lines. No skipped numbers.",
        "- Never wrap output in code fences (```), never add commentary, never restate the source.",
        "- Never copy the [Component @ File:Line] context into the translation — that is metadata only.",
        "- Preserve embedded escapes such as \\n, %1, %2, {0}, %L1, &amp;, &lt;, &gt;, &quot;, &apos; exactly as written.",
        "",
        "===========================================================================",
        "TRANSLATION FIDELITY",
        "===========================================================================",
        "- Translate meaning, not words. Do not paraphrase, expand, simplify, or omit.",
        "- Keep technical terms intact: 'unsigned int', '24-bit', 'baud rate', 'checksum', 'register', 'frame', 'NMEA', 'gRPC', etc.",
        "- Do not localize protocol names, file extensions, vendor names, or product names (e.g., 'Modbus', 'CAN', 'XMODEM', 'Serial Studio', 'Pro').",
        "- Match the source length where possible — Serial Studio is space-constrained.",
        "- If the source contains only a placeholder (e.g., '%1'), output the placeholder unchanged.",
        "",
        "===========================================================================",
        "VOICE & TONE — APPLE HIG STYLE",
        "===========================================================================",
        "Serial Studio uses a concise, neutral, functional voice. Match it in the target language.",
        "",
        "1. ACTIONS (buttons, menu items, toolbar labels): imperative verbs.",
        "   - 'Save', 'Delete', 'Try Again', 'Open Project', 'Add Action'.",
        "   - English source already follows this rule; translate using your language's imperative form.",
        "   - Spanish: 'Guardar', 'Borrar', 'Reintentar' (NOT 'Guardando', 'Borrado').",
        "   - French: 'Enregistrer', 'Supprimer', 'Réessayer'.",
        "   - German: 'Speichern', 'Löschen', 'Erneut versuchen' (German imperatives are infinitive form).",
        "",
        "2. LABELS (section titles, group headers, settings rows): noun phrases.",
        "   - 'Account Settings', 'Privacy', 'Notifications', 'Frame Detection'.",
        "   - Use noun phrases in the target language too. Do NOT turn a label into a verb.",
        "",
        "3. STATES / STATUS messages: present tense or short descriptive phrases.",
        "   - 'No Connection', 'Uploading…', 'Sync Complete', 'Listening on port 7777'.",
        "   - Avoid future tense in any language.",
        "",
        "4. COMPLETED ACTIONS: short past-participle forms.",
        "   - 'Saved', 'Deleted', 'Upload Complete'.",
        "",
        "5. NO FUTURE TENSE. Replace any 'will' / 'going to' construction with present tense or",
        "   completed-action form. If the English source still uses future tense, translate it as",
        "   present-tense status if natural; otherwise keep the original tense.",
        "",
        "6. NO USER ADDRESS unless the source already uses it.",
        "   - Avoid 'you', 'we', 'your', 'our' in your translation. Use impersonal/passive forms common in the target language.",
        "   - Spanish: prefer impersonal 'se' constructions; 'Save your changes?' → 'Guardar cambios?' (not 'Guardar tus cambios?').",
        "   - French: prefer impersonal 'on' or infinitive forms.",
        "   - German: prefer infinitive or passive ('Änderungen speichern?').",
        "   - If the English source DOES use 'you'/'your', keep the personal address — do not over-correct.",
        "",
        "7. NO POLITENESS FILLER. Drop 'Please', 'Sorry', 'Kindly' unless they appear in the source.",
        "",
        "8. ERROR MESSAGES: direct and neutral. Never blame the user.",
        "   - 'Unable to Connect', 'Failed to Open File', 'Invalid Configuration'.",
        "",
        "9. ELLIPSIS '…': use only when the action requires more input or opens another step.",
        "   - Keep the ellipsis when the source has it ('Save As…', 'Add Account…').",
        "   - Use the single Unicode character '…' (U+2026), not three dots '...'. If the source",
        "     uses three ASCII dots, preserve them exactly — do not silently replace.",
        "",
        "===========================================================================",
        "CAPITALIZATION — TITLE CASE WITH LANGUAGE-AWARE EXCEPTIONS",
        "===========================================================================",
        "- If the source is in Title Case (every meaningful word capitalized), the translation must",
        "  also be in title case, adapted to your language's conventions:",
        "    * Spanish: lowercase 'de', 'del', 'la', 'las', 'el', 'los', 'y', 'e', 'o', 'u',",
        "      'en', 'con', 'por', 'para', 'sin', 'a', 'al' (unless first/last word).",
        "    * French: lowercase 'de', 'du', 'des', 'la', 'le', 'les', 'et', 'ou', 'à', 'au',",
        "      'en', 'dans', 'pour', 'avec' (unless first/last word).",
        "    * German: capitalize ALL nouns regardless of position (this is grammar, not style).",
        "      Keep articles ('der', 'die', 'das') lowercase mid-phrase.",
        "    * Italian, Portuguese: similar to Spanish/French — see the language-specific list.",
        "- ALWAYS capitalize the first and last words, regardless of what they are.",
        "- 'Data Grid' → ES: 'Cuadrícula de Datos' / FR: 'Grille de Données' / DE: 'Datenraster'.",
        "- A SINGLE-WORD title-cased source ('Dataset') still expands to title case in multi-word",
        "  translations: ES 'Conjunto de Datos' (NOT 'Conjunto de datos'), FR 'Jeu de Données'.",
        "- If the source is sentence case ('Add a 3D plot visualization'), keep that pattern.",
        "- If the source is ALL UPPERCASE ('PAUSE'), output ALL UPPERCASE in the translation.",
        "- If the source is all lowercase, preserve that.",
        "",
        "===========================================================================",
        "ACRONYMS — ALWAYS UPPERCASE",
        "===========================================================================",
        "- Keep technical acronyms in ALL CAPS in every language and every position:",
        "  CAN, TCP, IP, UDP, USB, HID, BLE, MQTT, FFT, GPS, NMEA, JSON, XML, CSV, MDF, SQL,",
        "  GPL, MIT, SPDX, JS, QML, API, RPC, PDF, HTML, CRC, SLCAN, GRBL, GCODE, SCPI, SMTP,",
        "  IMAP, OAUTH, GPU, CPU, RAM, ROM, GUI, CLI, SDK, IDE.",
        "- 'CAN Bus' → 'Magistrala CAN' (NOT 'Magistrala Can').",
        "- 'JSON Layout' → 'Diseño JSON' (NOT 'Diseño Json').",
        "- 'USB Device' → 'Dispositivo USB' (NOT 'Dispositivo Usb').",
        "",
        "===========================================================================",
        "CONSISTENCY",
        "===========================================================================",
        "- Use the same target-language term for the same English term across the batch.",
        "- 'Save' must always render the same way; 'Cancel' must always render the same way.",
        "- Do not mix sentence styles in similar contexts.",
        "",
    ] + [f"{i+1}. [{loc}] {line}" for i, (loc, line) in enumerate(escaped_entries)])

    # Wrap the dashboard's status callback so the retry layer only has to
    # know one signature: (kind, attempt, sleep_for).
    def _retry_status(kind: str, attempt: int, sleep_for: float) -> None:
        if status_fn is not None:
            status_fn("retry", kind=kind, attempt=attempt, sleep_for=sleep_for)

    if status_fn is not None:
        status_fn("calling", batch_size=len(escaped_entries))

    # Lower temperature → fewer creative deviations from the rule list above
    raw_text = _call_provider_with_retry(
        prompt,
        temperature=0.2,
        max_tokens=2048,
        status_fn=_retry_status,
        stop_event=stop_event,
    ).strip()

    if status_fn is not None:
        status_fn("parsing")

    # Parse and validate
    try:
        translations = parse_numbered_translations(raw_text, len(source_texts))
    except ValueError as e:
        log_fn("GPT RAW RESPONSE:\n" + raw_text)
        raise e

    # Resolve language code if the caller didn't pass one — preserves the
    # legacy behavior where callers passed only target_language.
    if lang_code is None:
        lang_code = next(
            (code for code, name in LANGUAGE_MAP.items() if name == target_language),
            None,
        )

    # Enforce line break consistency + run the deterministic capitalization
    # post-process on every GPT result so "Conjunto de datos" never leaks past.
    final_output: list[tuple[str, int]] = []
    for (_, src_text), (trans, score) in zip(source_texts, translations):
        expected = src_text.count("\n") + 1
        actual = trans.count("\n") + 1
        if actual != expected:
            log_fn(f"[!] Line break mismatch: '{src_text}'")
            lines = trans.splitlines()
            while len(lines) < expected:
                lines.append("")
            trans = "\n".join(lines[:expected])

        # Apply deterministic capitalization (skipped automatically for caseless scripts)
        if lang_code and lang_code not in CASELESS_SCRIPTS:
            trans = apply_capitalization_pattern(src_text, trans, lang_code)

        # Force technical acronyms back to uppercase regardless of what GPT did.
        # Source-aware: only acronyms that appear uppercase in the EN source
        # get uppercased, so ambiguous words ('can', 'mac') never trip.
        trans = enforce_acronym_case(trans, source=src_text)

        # Log cleanly with the score so the user sees what got flagged.
        # In dashboard mode log_fn is a no-op; in serial / non-TTY runs this
        # still ends up on stdout.
        flag = " ⚠ FLAGGED FOR REVIEW" if score <= 3 else ""
        log_fn(f"\n[EN] {src_text}\n[{target_language.upper()} score={score}{flag}] {trans}\n")

        final_output.append((trans, score))

    return final_output

#------------------------------------------------------------------------------
# Verification Logic - Deterministic Capitalization Matching
#------------------------------------------------------------------------------

def _looks_like_acronym(word: str) -> bool:
    """Detect tokens like 'CAN', 'TCP/IP', 'JSON' that must stay uppercase."""
    stripped = re.sub(r"[^A-Za-z]", "", word)
    if not stripped:
        return False

    upper = stripped.upper()
    if upper in TECHNICAL_ACRONYMS:
        return True

    return len(stripped) >= 2 and stripped == upper


def _is_title_case_source(source: str) -> bool:
    """Heuristic: returns True if the source uses Title Case rather than sentence case.

    Title Case here means every alphabetic token starts uppercase. Single-word
    sources like "Dataset" qualify and propagate title case to multi-word
    translations, which is the common Serial Studio idiom.
    """
    tokens = [t for t in re.split(r"\s+", source) if t and any(c.isalpha() for c in t)]
    if not tokens:
        return False

    for tok in tokens:
        first_alpha = next((c for c in tok if c.isalpha()), None)
        if first_alpha is None:
            continue
        if not first_alpha.isupper():
            return False

    return True


def _titlecase_translation(translation: str, lang_code: str | None) -> str:
    """Apply language-aware title case to a translated string."""
    trans_words = translation.split(" ")
    small_words = TITLE_CASE_SMALL_WORDS.get(lang_code, set())

    # Identify the first and last *alphabetic* word indices so symbols
    # ("—", "(", ":") at the boundaries don't leak the small-word rule.
    alpha_indices = [
        idx for idx, w in enumerate(trans_words)
        if w and any(c.isalpha() for c in w)
    ]
    if not alpha_indices:
        return translation

    first_alpha = alpha_indices[0]
    last_alpha = alpha_indices[-1]

    result_words = []
    for i, word in enumerate(trans_words):
        if not word or not any(c.isalpha() for c in word):
            result_words.append(word)
            continue

        # Already-uppercase technical acronyms stay as-is
        if _looks_like_acronym(word):
            result_words.append(word)
            continue

        word_lower = word.lower()
        force_capitalize = (i == first_alpha) or (i == last_alpha) \
            or word_lower not in small_words

        if force_capitalize:
            # Capitalize the first alphabetic character; preserve any leading punct.
            chars = list(word)
            for j, c in enumerate(chars):
                if c.isalpha():
                    chars[j] = c.upper()
                    chars[j+1:] = list("".join(chars[j+1:]).lower())
                    break
            result_words.append("".join(chars))
        else:
            result_words.append(word_lower)

    return " ".join(result_words)


def apply_capitalization_pattern(source: str, translation: str, lang_code: str = None) -> str:
    """
    Apply language-aware capitalization pattern from source to translation.

    Examples:
        apply_capitalization_pattern("Data Grid", "cuadrícula de datos", "es_MX")
        → "Cuadrícula de Datos" (lowercase "de")

        apply_capitalization_pattern("Dataset", "conjunto de datos", "es_MX")
        → "Conjunto de Datos" (single-word source still propagates title case)

        apply_capitalization_pattern("PAUSE", "duraklat", "tr_TR")
        → "DURAKLAT"

        apply_capitalization_pattern("Add a 3D plot", "añadir un gráfico 3D", "es_MX")
        → "Añadir un gráfico 3D" (sentence case preserved)
    """
    if not source or not translation:
        return translation

    # Multi-line strings (paragraphs / sentences with embedded newlines) are
    # body copy — never apply title case to them.
    if "\n" in source:
        return translation

    # Skip strings that are mostly punctuation/placeholders
    alpha_in_source = sum(1 for c in source if c.isalpha())
    if alpha_in_source < 2:
        return translation

    # ALL UPPERCASE source → uppercase translation
    if source.isupper():
        return translation.upper()

    # All-lowercase source → leave translation alone (not safe to force lowercase
    # because some target languages still capitalize — German nouns, sentence start)
    if source.islower():
        return translation

    # Sentence-style strings (multiple words, ends with period/question/exclamation)
    # are full sentences — never apply title case.
    stripped = source.rstrip()
    if stripped and stripped[-1] in ".?!" and len(source.split()) > 3:
        if translation and translation[0].isalpha():
            return translation[0].upper() + translation[1:]
        return translation

    # Title-case source → language-aware title case on the translation
    if _is_title_case_source(source):
        return _titlecase_translation(translation, lang_code)

    # Sentence case: ensure the first letter is uppercase
    if source[0].isupper():
        if translation and translation[0].isalpha():
            return translation[0].upper() + translation[1:]

    return translation


def enforce_acronym_case(text: str, source: str | None = None) -> str:
    """Force every known technical acronym in TEXT back to ALL UPPERCASE.

    If SOURCE is provided, only uppercase a token when it ALSO appears as an
    uppercase acronym in the source. That eliminates false positives for
    ambiguous tokens — e.g., 'can' (English verb) in a Spanish translation
    won't be force-uppercased to 'CAN' unless the English source also said
    'CAN'.

    Without SOURCE, every token whose uppercase form is in TECHNICAL_ACRONYMS
    is uppercased. Use that mode only when you know the input is a translation
    of a CAN/MQTT/USB-style technical string.
    """
    if not text:
        return text

    # Build the set of acronyms actually present in the source. A token counts
    # as "present" if its uppercase form is a known acronym AND the source
    # token has at least one uppercase letter (so 'can' as a verb won't sneak
    # in but 'CAN', 'QoS', 'MQtt' all do).
    if source is not None:
        present = set()
        for tok in re.findall(r"[A-Za-z]+", source):
            upper = tok.upper()
            if upper in TECHNICAL_ACRONYMS and any(c.isupper() for c in tok):
                present.add(upper)
        if not present:
            return text
    else:
        present = TECHNICAL_ACRONYMS

    def _fix(match: re.Match) -> str:
        word = match.group(0)
        upper = word.upper()
        return upper if upper in present else word

    return re.sub(r"[A-Za-z]+", _fix, text)


def verify_capitalization_batch(entries, lang_code):
    """
    Verify and fix capitalization patterns in existing translations.

    This is a DETERMINISTIC function that applies language-specific title case
    rules without using GPT.

    Args:
        entries (list of (str, str, str)): Tuples of (context_location, source_text, translation_text).
        lang_code (str): Language code (e.g., "es_MX") for language-specific rules.

    Returns:
        list of str: Corrected translations with proper capitalization.
    """
    results = []
    for loc, source, translation in entries:
        corrected = apply_capitalization_pattern(source, translation, lang_code)
        results.append(corrected)

    return results

#------------------------------------------------------------------------------
# Core Translation Logic
#------------------------------------------------------------------------------

def translate_ts_file(filename, batch_size=10, min_score=4, log_fn=print,
                      progress=None, stop_event: threading.Event | None = None):
    """
    Process a TS file and translate all unfinished or empty entries.

    Translation memory: validated entries already in this file are harvested
    and the most-relevant K are passed to each LLM batch as few-shot examples.

    Confidence: every LLM result carries a 1-5 score. Translations with score
    < MIN_SCORE keep type="unfinished" so Qt Linguist surfaces them for human
    review.

    en_US.ts is special-cased: source is already English, so the translation
    is filled by direct copy with NO API call.

    Args:
        filename (str): TS file to process (e.g., 'fr_FR.ts').
        batch_size (int): Number of strings per LLM request.
        min_score (int): Translations with score < this stay unfinished.
        log_fn (callable): Sink for progress lines. When running file-level
            workers in parallel the caller passes a per-file buffer so
            concurrent files don't interleave their output on stdout.
        progress (Progress, optional): Shared dashboard registry. The worker
            updates state, totals, and counters on it; the renderer thread
            consumes them.
        stop_event (threading.Event, optional): Set by Ctrl-C / dashboard
            shutdown so retry sleeps wake up early.
    """
    # Determine target language
    lang_code = filename.replace(".ts", "")
    target_language = LANGUAGE_MAP.get(lang_code)

    is_english = lang_code == "en_US"
    if not is_english and not target_language:
        log_fn(f"Skipping {filename}: unsupported language code.")
        if progress is not None:
            progress.update(lang_code, state="skipped")
        return

    # Load and parse the TS XML file
    ts_path = os.path.join(SCRIPT_DIR, TS_DIRECTORY, filename)
    tree = etree.parse(ts_path)
    root = tree.getroot()

    # English source-copy fast path: no LLM, no batching, no scoring.
    if is_english:
        if progress is not None:
            progress.update(lang_code, state="copying")
        copied = _fill_en_us_translations(root)
        tree.write(ts_path, encoding="utf-8", xml_declaration=True)
        log_fn(f"{filename}: {copied} entries copied from source (no LLM call).")
        if progress is not None:
            en_total = sum(
                1 for ctx in root.findall("context")
                for msg in ctx.findall("message")
                if msg.findtext("source", default="").strip()
            )
            progress.update(lang_code, state="done",
                            done=en_total, total=en_total)
        return

    # Harvest translation memory ONCE per file. Stays stable across batches.
    tm = harvest_translation_memory(root)
    log_fn(f"{filename}: {len(tm)} validated entries in translation memory.")

    # Pre-scan: collect every (context_tag, source_text, translation_node)
    # that needs work, plus a file-wide count of translatable entries so the
    # dashboard reflects overall completion (already-done + freshly-done) and
    # not just this run's pending slice.
    pending: list[tuple[str, str, "etree._Element"]] = []
    file_total = 0
    for context in root.findall("context"):
        context_name = context.findtext("name", default="unknown")
        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            locations = message.findall("location")
            if locations:
                location_str = ", ".join(
                    f"{loc.get('filename')}:{loc.get('line')}" for loc in locations
                )
            else:
                location_str = "unknown"

            context_tag = f"{context_name} @ {location_str}"

            if source is None or not source.text or not source.text.strip():
                continue

            if translation is None:
                translation = etree.SubElement(message, "translation")

            file_total += 1

            translation_text = translation.text or ""
            needs_translation = (
                translation.get("type") == "unfinished"
                or not translation_text.strip()
            )
            if not needs_translation:
                continue

            pending.append((context_tag, source.text, translation))

    total_pending = len(pending)
    already_done = file_total - total_pending
    if progress is not None:
        progress.update(lang_code, state="translating", total=file_total,
                        done=already_done, flagged=0)

    if total_pending == 0:
        tree.write(ts_path, encoding="utf-8", xml_declaration=True)
        log_fn(f"{filename}: nothing to translate.")
        if progress is not None:
            progress.update(lang_code, state="done",
                            total=file_total, done=file_total)
        return

    total_updated = 0
    total_flagged = 0

    def _make_status_fn():
        if progress is None:
            return None

        def _status(state: str, **info) -> None:
            if state == "calling":
                progress.update(lang_code, state="calling")
            elif state == "retry":
                progress.note_retry(lang_code,
                                    kind=info.get("kind", "?"),
                                    attempt=info.get("attempt", 0),
                                    sleep_for=info.get("sleep_for", 0.0))
            elif state == "parsing":
                progress.update(lang_code, state="parsing")
        return _status

    # Process the pending list in batches.
    for start in range(0, total_pending, batch_size):
        if stop_event is not None and stop_event.is_set():
            log_fn(f"{filename}: aborted before completion.")
            break

        chunk = pending[start:start + batch_size]
        source_batch = [(loc, src) for loc, src, _ in chunk]
        translation_nodes = [node for _, _, node in chunk]

        try:
            results = translate_batch(
                source_batch,
                target_language,
                lang_code=lang_code,
                tm=tm,
                log_fn=log_fn,
                status_fn=_make_status_fn(),
                stop_event=stop_event,
            )
        except Exception as e:
            log_fn(f"Error in batch starting at {start}: {e}")
            if progress is not None:
                progress.update(lang_code, state="translating",
                                done=already_done + total_updated + len(chunk),
                                flagged=total_flagged)
            total_updated += len(chunk)
            continue

        for node, (text, score) in zip(translation_nodes, results):
            node.text = text
            if score >= min_score:
                node.attrib.pop("type", None)
            else:
                node.set("type", "unfinished")
                total_flagged += 1
            total_updated += 1

        if progress is not None:
            progress.update(lang_code, state="translating",
                            done=already_done + total_updated, flagged=total_flagged)

    # Save updated file
    tree.write(ts_path, encoding="utf-8", xml_declaration=True)
    log_fn(f"{filename}: {total_updated} strings translated, "
           f"{total_flagged} flagged for human review (score < {min_score}).")

    if progress is not None:
        progress.update(lang_code, state="done",
                        total=file_total,
                        done=already_done + total_updated,
                        flagged=total_flagged)


def _fill_en_us_translations(root) -> int:
    """Copy <source> to <translation> for every empty/unfinished entry.

    en_US.ts is the source language file. Translating English to English with
    an LLM is wasteful and risks paraphrasing. This direct copy is also what
    Qt would fall back to at runtime, but stamping the entries explicitly
    makes the file complete and silences Linguist warnings.
    """
    filled = 0
    for context in root.findall("context"):
        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            if source is None or not source.text or not source.text.strip():
                continue

            if translation is None:
                translation = etree.SubElement(message, "translation")

            translation_text = translation.text or ""
            needs_fill = (
                translation.get("type") == "unfinished"
                or not translation_text.strip()
            )
            if not needs_fill:
                continue

            translation.text = source.text
            translation.attrib.pop("type", None)
            filled += 1
    return filled


def reset_translations(only_lang: str | None = None) -> None:
    """Clear every <translation> in every non-English .ts file.

    Sets each translation's text to empty and type="unfinished" so the next
    translate run regenerates them from scratch. Useful when the prompt /
    glossary / model changes substantially and you want a clean re-run.

    Args:
        only_lang: If given (e.g. 'fr_FR'), reset only that language file.
    """
    ts_dir = os.path.join(SCRIPT_DIR, TS_DIRECTORY)
    files = []
    for f in sorted(os.listdir(ts_dir)):
        if not f.endswith(".ts"):
            continue
        if f == "en_US.ts":
            continue
        if only_lang and f != f"{only_lang}.ts":
            continue
        files.append(f)

    if not files:
        print("No .ts files matched for reset.")
        return

    grand_total = 0
    for filename in files:
        path = os.path.join(ts_dir, filename)
        tree = etree.parse(path)
        root = tree.getroot()
        cleared = 0
        for context in root.findall("context"):
            for message in context.findall("message"):
                translation = message.find("translation")
                if translation is None:
                    translation = etree.SubElement(message, "translation")
                if translation.text or translation.get("type") != "unfinished":
                    cleared += 1
                translation.text = None
                translation.set("type", "unfinished")
        tree.write(path, encoding="utf-8", xml_declaration=True)
        grand_total += cleared
        print(f"  {filename}: {cleared} translations cleared.")

    print(f"\nReset complete — {grand_total} entries cleared across {len(files)} files.")

def verify_capitalization_ts_file(filename):
    """
    Review and fix capitalization in existing translations using deterministic
    pattern matching (no GPT calls).

    Args:
        filename (str): TS file to process (e.g., 'fr_FR.ts').
    """
    lang_code = filename.replace(".ts", "")
    target_language = LANGUAGE_MAP.get(lang_code)
    if not target_language:
        print(f"Skipping {filename}: unsupported language code.")
        return

    # Skip languages with caseless scripts (Chinese, Japanese, Hindi, etc.)
    if lang_code in CASELESS_SCRIPTS:
        print(f"\n{filename}: Skipped (caseless script - no uppercase/lowercase) ⊘\n")
        return

    ts_path = os.path.join(SCRIPT_DIR, TS_DIRECTORY, filename)
    tree = etree.parse(ts_path)
    root = tree.getroot()
    total_fixed = 0

    for context in root.findall("context"):
        context_name = context.findtext("name", default="unknown")

        for message in context.findall("message"):
            source = message.find("source")
            translation = message.find("translation")

            if source is None or not source.text or not source.text.strip():
                continue

            if translation is None or not translation.text or not translation.text.strip():
                continue

            if translation.get("type") == "unfinished":
                continue

            # Apply language-aware capitalization matching, then enforce acronyms
            # using the source as the authoritative list of present acronyms.
            original = translation.text
            corrected = apply_capitalization_pattern(source.text, original, lang_code)
            corrected = enforce_acronym_case(corrected, source=source.text)

            if original != corrected:
                print(f"[FIX] {source.text}")
                print(f"      {original} → {corrected}")
                translation.text = corrected
                total_fixed += 1

    if total_fixed > 0:
        tree.write(ts_path, encoding="utf-8", xml_declaration=True)
        print(f"\n{filename}: {total_fixed} capitalization issues fixed.\n")
    else:
        print(f"\n{filename}: No capitalization issues found. ✓\n")

#------------------------------------------------------------------------------
# Source-String Linter (--lint-sources)
#------------------------------------------------------------------------------

# Regex patterns that flag style violations in English source strings.
# Each entry is (pattern, label, severity). Severity: "error" / "warn" / "info".
# Patterns are applied with re.IGNORECASE unless they include explicit case classes.
LINT_PATTERNS = [
    # Future tense — replace with present tense or completed-action form.
    (r"\b(will\s+(?:not\s+)?(?:be\s+)?[a-z]+|won['’]t|going\s+to|shall(?!\s+be\s+liable))\b",
     "future-tense", "warn"),

    # User address — drop or rephrase impersonally unless intentional.
    (r"\b(you|your|yours|yourself|we|our|ours|us)\b",
     "user-address", "warn"),

    # Politeness filler — drop unless the source is asking for input.
    (r"\b(please|sorry|kindly|unfortunately)\b",
     "politeness", "info"),

    # Three ASCII dots used as ellipsis — prefer the single '…' character.
    (r"\.{3}",
     "ascii-ellipsis", "info"),

    # Leading or trailing whitespace.
    (r"^\s|\s$",
     "edge-whitespace", "info"),

    # Double space inside the string.
    (r"  ",
     "double-space", "info"),

    # Common noun-form action labels that should be imperative when they appear
    # on buttons or menu items. The linter cannot tell context, so this is
    # surfaced as "info" and the human decides.
    (r"^(Cancellation|Deletion|Removal|Submission|Connection|Disconnection|Authentication|Saving|Loading)\.?$",
     "noun-form-action", "info"),
]


def _lint_string(text: str) -> list[tuple[str, str, str]]:
    """Return a list of (label, severity, evidence) findings for one source string."""
    findings = []
    if not text:
        return findings

    for pattern, label, severity in LINT_PATTERNS:
        match = re.search(pattern, text, flags=re.IGNORECASE)
        if match:
            evidence = match.group(0)
            # Visualize whitespace
            if label == "edge-whitespace":
                evidence = repr(evidence)
            findings.append((label, severity, evidence))

    return findings


def lint_source_strings(filename="en_US.ts"):
    """Scan en_US.ts for source-string style violations and print a report."""
    ts_path = os.path.join(SCRIPT_DIR, TS_DIRECTORY, filename)
    if not os.path.exists(ts_path):
        print(f"Source TS not found: {ts_path}")
        return

    tree = etree.parse(ts_path)
    root = tree.getroot()

    counts = {}
    rows = []

    for context in root.findall("context"):
        context_name = context.findtext("name", default="unknown")
        for message in context.findall("message"):
            source = message.find("source")
            if source is None or not source.text:
                continue

            findings = _lint_string(source.text)
            if not findings:
                continue

            locations = message.findall("location")
            location_str = ", ".join(
                f"{loc.get('filename')}:{loc.get('line')}" for loc in locations
            ) if locations else "unknown"

            for label, severity, evidence in findings:
                counts[label] = counts.get(label, 0) + 1
                rows.append((severity, label, source.text, evidence, context_name, location_str))

    # Sort: errors first, then warnings, then info.
    severity_rank = {"error": 0, "warn": 1, "info": 2}
    rows.sort(key=lambda r: (severity_rank.get(r[0], 9), r[1]))

    for severity, label, text, evidence, ctx, loc in rows:
        # Truncate long bodies for readable output.
        snippet = text if len(text) <= 120 else text[:117] + "…"
        snippet = snippet.replace("\n", " ⏎ ")
        print(f"[{severity:5s}] {label:18s} {evidence!s:30s} {snippet}")
        print(f"          {ctx} @ {loc}")

    print()
    print("=" * 78)
    print(f"Lint summary — {len(rows)} findings across {filename}:")
    for label in sorted(counts):
        print(f"  {label:18s} {counts[label]}")


#------------------------------------------------------------------------------
# Live Dashboard
#------------------------------------------------------------------------------

# ANSI sequences for the live dashboard. We use absolute redraw — every tick
# moves the cursor back to the top-of-block and rewrites every row.
_ANSI_CSI = "\x1b["
_ANSI_HIDE_CURSOR = _ANSI_CSI + "?25l"
_ANSI_SHOW_CURSOR = _ANSI_CSI + "?25h"
_ANSI_CLEAR_LINE = _ANSI_CSI + "2K"
_ANSI_CLEAR_SCREEN = _ANSI_CSI + "2J" + _ANSI_CSI + "H"
_ANSI_GREEN = _ANSI_CSI + "32m"
_ANSI_YELLOW = _ANSI_CSI + "33m"
_ANSI_RED = _ANSI_CSI + "31m"
_ANSI_DIM = _ANSI_CSI + "2m"
_ANSI_BOLD = _ANSI_CSI + "1m"
_ANSI_RESET = _ANSI_CSI + "0m"

_STATE_COLORS = {
    "queued": _ANSI_DIM,
    "translating": "",
    "calling": "",
    "parsing": "",
    "retry": _ANSI_YELLOW,
    "rate-limited": _ANSI_YELLOW,
    "copying": "",
    "done": _ANSI_GREEN,
    "error": _ANSI_RED,
    "skipped": _ANSI_DIM,
}


# Spinner frames + flavor verbs for the activity line. Same braille spinner
# Claude Code uses; verbs cycle slowly so the user has something to read while
# waiting on the LLM.
_SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]
_FLAVOR_VERBS = [
    "Translating", "Cogitating", "Localizing", "Pondering", "Polishing",
    "Negotiating", "Diphthonging", "Conjugating", "Declining", "Transliterating",
    "Marshalling", "Untangling", "Glossing", "Wordsmithing", "Paraphrasing",
    "Disambiguating", "Coalescing", "Catalyzing", "Harmonizing", "Calibrating",
]


class Progress:
    """Thread-safe per-language progress registry consumed by the dashboard."""

    def __init__(self, lang_codes: list[str]):
        self._lock = threading.Lock()
        self._rows: dict[str, dict] = {}
        self._order: list[str] = list(lang_codes)
        self._started_at = time.monotonic()
        for code in lang_codes:
            self._rows[code] = {
                "state": "queued",
                "done": 0,
                "total": 0,
                "flagged": 0,
                "retry_kind": None,
                "retry_until": 0.0,
                "retry_count": 0,
                "started_at": None,
                "finished_at": None,
            }

    def update(self, lang_code: str, **fields) -> None:
        """Set arbitrary fields on a row."""
        now = time.monotonic()
        with self._lock:
            row = self._rows.get(lang_code)
            if row is None:
                return
            if "state" in fields:
                new_state = fields["state"]
                if row["started_at"] is None and new_state not in ("queued", "skipped"):
                    row["started_at"] = now
                if new_state in ("done", "error", "skipped"):
                    row["finished_at"] = now
                if new_state not in ("retry", "rate-limited"):
                    row["retry_kind"] = None
                    row["retry_until"] = 0.0
            row.update(fields)

    def note_retry(self, lang_code: str, kind: str, attempt: int, sleep_for: float) -> None:
        now = time.monotonic()
        with self._lock:
            row = self._rows.get(lang_code)
            if row is None:
                return
            row["state"] = "rate-limited" if kind == "rate-limit" else "retry"
            row["retry_kind"] = kind
            row["retry_until"] = now + sleep_for
            row["retry_count"] = row.get("retry_count", 0) + 1
            if row["started_at"] is None:
                row["started_at"] = now

    def snapshot(self) -> tuple[list[str], dict[str, dict], float]:
        with self._lock:
            return (list(self._order),
                    {k: dict(v) for k, v in self._rows.items()},
                    self._started_at)


class Dashboard:
    """Background thread that paints a multi-row progress dashboard.

    Layout (top → bottom): banner, per-language rows, spinner activity line,
    aggregate summary. The dashboard owns stdout while it runs; per-file logs
    are silently dropped because the bars convey progress on their own.
    """

    def __init__(self, progress: Progress, banner: str = "",
                 refresh_hz: float = 8.0):
        self._progress = progress
        self._banner = banner
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None
        self._period = 1.0 / max(0.5, refresh_hz)
        self._block_height = 0
        self._enabled = sys.stdout.isatty()
        self._tick = 0
        # Rate baseline: workers seed `done` with already-validated entries
        # from prior runs. Counting those against this run's elapsed inflates
        # the rate. Snapshot the baseline once a worker reports active work.
        self._rate_baseline_done: int | None = None
        self._rate_baseline_at: float | None = None

    def start(self) -> None:
        if not self._enabled:
            return
        sys.stdout.write(_ANSI_CLEAR_SCREEN + _ANSI_HIDE_CURSOR)
        sys.stdout.flush()
        self._thread = threading.Thread(target=self._run, daemon=True, name="dashboard")
        self._thread.start()

    def stop(self) -> None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=2.0)
        if self._enabled:
            self._render(final=True)
            sys.stdout.write(_ANSI_SHOW_CURSOR)
            sys.stdout.flush()

    def stop_event(self) -> threading.Event:
        return self._stop

    def log(self, line: str) -> None:
        # No-op while the dashboard is active — the bars are the UI. In
        # non-TTY mode the dashboard is never instantiated, so callers fall
        # back to plain print().
        if not self._enabled:
            print(line)

    def _run(self) -> None:
        while not self._stop.is_set():
            self._render()
            self._tick += 1
            self._stop.wait(self._period)

    def _render(self, final: bool = False) -> None:
        order, rows, started_at = self._progress.snapshot()
        elapsed = time.monotonic() - started_at

        block: list[str] = []
        if self._banner:
            block.append(self._banner)
            block.append("")

        block.extend(self._format_row(code, rows[code]) for code in order)
        block.append("")
        block.append(self._format_activity(rows, final))
        block.append(self._format_summary(rows, elapsed))

        out: list[str] = []
        if self._block_height:
            out.append(f"{_ANSI_CSI}{self._block_height}A")

        for i, line in enumerate(block):
            terminator = "" if (final and i == len(block) - 1) else "\n"
            out.append(f"\r{_ANSI_CLEAR_LINE}{line}{terminator}")

        # Wipe leftover lines below the block if it shrank since last tick.
        if self._block_height > len(block):
            for _ in range(self._block_height - len(block)):
                out.append(f"\r{_ANSI_CLEAR_LINE}\n")

        self._block_height = len(block)
        sys.stdout.write("".join(out))
        sys.stdout.flush()

    def _format_summary(self, rows: dict[str, dict], elapsed: float) -> str:
        total_done = sum(r["done"] for r in rows.values())
        total_total = sum(r["total"] for r in rows.values())
        total_flagged = sum(r["flagged"] for r in rows.values())
        active = sum(1 for r in rows.values()
                     if r["state"] in ("translating", "calling", "parsing",
                                       "retry", "rate-limited", "copying"))
        finished = sum(1 for r in rows.values()
                       if r["state"] in ("done", "skipped", "error"))

        # Snapshot the rate baseline once a worker actually starts producing,
        # so the rate reflects new work / time-since-real-work-began.
        any_active = any(
            r["state"] in ("translating", "calling", "parsing", "copying")
            for r in rows.values()
        )
        now = time.monotonic()
        if self._rate_baseline_done is None and any_active:
            self._rate_baseline_done = total_done
            self._rate_baseline_at = now

        if self._rate_baseline_done is not None and self._rate_baseline_at is not None:
            delta_done = max(0, total_done - self._rate_baseline_done)
            delta_elapsed = now - self._rate_baseline_at
            rate = delta_done / delta_elapsed if delta_elapsed > 1.0 else 0.0
        else:
            rate = 0.0
        eta_s = (total_total - total_done) / rate if rate > 0 else 0.0

        bits = [
            f"{_ANSI_BOLD}translate-progress{_ANSI_RESET}",
            f"elapsed {_format_duration(elapsed)}",
            f"active {active}",
            f"done {finished}/{len(rows)}",
            f"strings {total_done}/{total_total}",
            f"flagged {total_flagged}",
            f"rate {rate:.1f}/s",
        ]
        if total_total > total_done and rate > 0:
            bits.append(f"eta {_format_duration(eta_s)}")
        return "  ".join(bits)

    def _format_activity(self, rows: dict[str, dict], final: bool) -> str:
        active_codes = [code for code, r in rows.items()
                        if r["state"] in ("translating", "calling", "parsing", "copying")]
        if final or not active_codes:
            return ""

        spinner = _SPINNER_FRAMES[self._tick % len(_SPINNER_FRAMES)]
        # Verb rotates roughly every 2.5 seconds at 8 Hz so it's readable.
        verb = _FLAVOR_VERBS[(self._tick // 20) % len(_FLAVOR_VERBS)]
        preview = ", ".join(active_codes[:6])
        if len(active_codes) > 6:
            preview += f" +{len(active_codes) - 6}"
        return (f"  {_ANSI_BOLD}{spinner}{_ANSI_RESET} "
                f"{verb} {_ANSI_DIM}({preview}){_ANSI_RESET}…")

    def _format_row(self, lang_code: str, row: dict) -> str:
        state = row["state"]
        color = _STATE_COLORS.get(state, "")
        # `skipped` rows have no totals — pin to full so they don't look stalled.
        if state == "skipped" and row["total"] == 0:
            bar = _format_bar(1, 1, width=24)
            pct = 100.0
        else:
            bar = _format_bar(row["done"], row["total"], width=24)
            pct = (row["done"] / row["total"] * 100.0) if row["total"] else 0.0

        status_parts = [state]
        if state in ("retry", "rate-limited"):
            wait = max(0.0, row["retry_until"] - time.monotonic())
            kind = row.get("retry_kind") or "?"
            status_parts = [f"{state}({kind})", f"in {wait:0.0f}s",
                            f"#{row['retry_count']}"]
        elif state == "done":
            row_elapsed = (row["finished_at"] or 0) - (row["started_at"] or 0)
            if row_elapsed > 0:
                status_parts.append(f"in {_format_duration(row_elapsed)}")
        if row["flagged"]:
            status_parts.append(f"⚠{row['flagged']}")
        if row.get("retry_count") and state not in ("retry", "rate-limited"):
            status_parts.append(f"retries={row['retry_count']}")

        status = " ".join(status_parts)
        return (f"  {lang_code:<6s} {color}{bar}{_ANSI_RESET} "
                f"{row['done']:>5d}/{row['total']:<5d} "
                f"{pct:5.1f}%  {color}{status}{_ANSI_RESET}")


def _format_bar(done: int, total: int, width: int = 24) -> str:
    if total <= 0:
        return "[" + " " * width + "]"
    ratio = max(0.0, min(1.0, done / total))
    filled = int(ratio * width)
    return "[" + "█" * filled + "·" * (width - filled) + "]"


def _format_duration(seconds: float) -> str:
    seconds = max(0.0, float(seconds))
    if seconds < 60:
        return f"{seconds:0.0f}s"
    minutes, sec = divmod(int(seconds), 60)
    if minutes < 60:
        return f"{minutes}m{sec:02d}s"
    hours, minutes = divmod(minutes, 60)
    return f"{hours}h{minutes:02d}m"


#------------------------------------------------------------------------------
# Entry Point
#------------------------------------------------------------------------------

def main():
    """Run translation pipeline: lupdate → translate (or other mode) → lrelease."""
    import argparse
    global PROVIDER, LLM_MODEL

    parser = argparse.ArgumentParser(
        description="Translate Serial Studio .ts files via an LLM with "
                    "domain glossary, translation memory, and confidence scoring.",
    )
    parser.add_argument(
        "--provider", choices=["openai", "anthropic"], default=DEFAULT_PROVIDER,
        help=f"LLM provider (default: {DEFAULT_PROVIDER}).",
    )
    parser.add_argument(
        "--model", default=None,
        help=f"Override model name. Defaults: openai={OPENAI_MODEL_DEFAULT}, "
             f"anthropic={ANTHROPIC_MODEL_DEFAULT}.",
    )
    parser.add_argument(
        "--lang", default=None,
        help="Process only this language (e.g. 'fr_FR'). Default: all.",
    )
    parser.add_argument(
        "--min-score", type=int, default=4,
        help="Translations with score below this are flagged 'unfinished' "
             "for human review (default: 4).",
    )
    parser.add_argument(
        "--verify-only", action="store_true",
        help="Re-apply deterministic capitalization to existing translations. "
             "No LLM calls.",
    )
    parser.add_argument(
        "--lint-sources", action="store_true",
        help="Scan en_US.ts for English source-string style violations.",
    )
    parser.add_argument(
        "--reset", action="store_true",
        help="Clear every translation in every non-en_US .ts file. Use --lang "
             "to limit to one file. Combine with a normal run to re-translate "
             "from scratch.",
    )
    parser.add_argument(
        "--workers", type=int, default=8,
        help="Translate this many .ts files in parallel (default: 8). Each "
             "worker handles one language file end-to-end. Set to 1 to "
             "preserve the legacy serial behavior. Higher values may hit "
             "provider rate limits.",
    )
    parser.add_argument(
        "--no-dashboard", action="store_true",
        help="Disable the live multi-row TUI dashboard. Falls back to the "
             "buffered per-file log mode (each file's output is printed in "
             "one block when it finishes). The dashboard is also disabled "
             "automatically when stdout is not a TTY.",
    )

    args = parser.parse_args()

    # Configure the active provider / model.
    PROVIDER = args.provider
    if args.model:
        LLM_MODEL = args.model
    elif PROVIDER == "anthropic":
        LLM_MODEL = ANTHROPIC_MODEL_DEFAULT
    else:
        LLM_MODEL = OPENAI_MODEL_DEFAULT

    # --reset can run alone or before a translation pass.
    if args.reset:
        print(f"=== Resetting translations" +
              (f" for {args.lang}" if args.lang else "") + " ===")
        reset_translations(only_lang=args.lang)
        # If reset is the only action requested, stop here.
        if not args.verify_only and not args.lint_sources:
            return

    # --lint-sources is a stand-alone read-only mode.
    if args.lint_sources:
        run_qt_translation_tool("--lupdate")
        lint_source_strings()
        return

    if not args.verify_only:
        run_qt_translation_tool("--lupdate")

    ts_files = sorted(
        f for f in os.listdir(os.path.join(SCRIPT_DIR, TS_DIRECTORY))
        if f.endswith(".ts")
    )
    if args.lang:
        ts_files = [f for f in ts_files if f == f"{args.lang}.ts"]
        if not ts_files:
            print(f"No .ts file matches --lang {args.lang!r}.")
            return

    if args.verify_only:
        # Verify is local CPU + disk only; keep it serial for predictable output.
        for file in ts_files:
            if file == "en_US.ts":
                continue
            print(f"\n=== Verifying capitalization: {file} ===")
            verify_capitalization_ts_file(file)
        run_qt_translation_tool("--lrelease")
        return

    # Translation: each .ts file is independent — different XML tree, different
    # output path, no shared mutable state. Run up to --workers files in
    # parallel.
    workers = max(1, args.workers)
    lang_codes = [f.replace(".ts", "") for f in ts_files]

    use_dashboard = (
        not args.no_dashboard
        and sys.stdout.isatty()
        and len(ts_files) > 1
        and workers > 1
    )

    banner = (f"  {_ANSI_BOLD}Serial Studio · translate-progress{_ANSI_RESET}"
              f"  {_ANSI_DIM}provider {PROVIDER} · model {LLM_MODEL}{_ANSI_RESET}")

    progress = Progress(lang_codes)
    dashboard = Dashboard(progress, banner=banner) if use_dashboard else None

    if dashboard is not None:
        dashboard.start()
        stop_event = dashboard.stop_event()
    else:
        print(f"\n=== Provider: {PROVIDER} / Model: {LLM_MODEL} ===")
        stop_event = threading.Event()

    def _run_one(file: str) -> str | None:
        lang_code = file.replace(".ts", "")
        if dashboard is not None:
            try:
                translate_ts_file(file, min_score=args.min_score,
                                  log_fn=dashboard.log,
                                  progress=progress,
                                  stop_event=stop_event)
            except Exception as e:
                progress.update(lang_code, state="error")
                dashboard.log(f"!!! {file}: worker crashed: {e}")
            return None

        # Non-dashboard fallback: buffer per-file output.
        buf = StringIO()
        buf_log = lambda msg: buf.write(str(msg) + "\n")
        buf_log(f"\n=== Translating: {file} ===")
        try:
            translate_ts_file(file, min_score=args.min_score,
                              log_fn=buf_log,
                              progress=progress,
                              stop_event=stop_event)
        except Exception as e:
            buf_log(f"!!! {file}: worker crashed: {e}")
        return buf.getvalue()

    try:
        if workers == 1 or len(ts_files) == 1:
            for file in ts_files:
                result = _run_one(file)
                if result is not None:
                    print(result, end="")
        else:
            if dashboard is None:
                print(f"=== Running {min(workers, len(ts_files))} parallel workers "
                      f"across {len(ts_files)} files ===")
            with ThreadPoolExecutor(max_workers=workers) as pool:
                futures = {pool.submit(_run_one, f): f for f in ts_files}
                try:
                    for fut in as_completed(futures):
                        result = fut.result()
                        if result is not None:
                            print(result, end="")
                except KeyboardInterrupt:
                    stop_event.set()
                    for f in futures:
                        f.cancel()
                    raise
    finally:
        if dashboard is not None:
            dashboard.stop()

    run_qt_translation_tool("--lrelease")

if __name__ == "__main__":
    main()