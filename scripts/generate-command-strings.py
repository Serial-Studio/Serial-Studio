#!/usr/bin/env python3
"""Generate the lupdate-visible translation stub for the command manifests.

Collects every user-visible string from app/rcc/commands/*.json and
app/rcc/commands/layouts/*.json (command title/titleChecked/tooltip/
collapsedTitle, layout node title/tooltip overrides, submenu titles) and writes
core/Ui/UI/CommandStrings.cpp as a block of QT_TRANSLATE_NOOP("Commands", ...)
entries. UI::CommandRegistry translates at query time through the same
"Commands" context, so this file is what makes the manifest strings reachable
by the existing lupdate/lrelease pipeline (spec 0028 R13). Deterministic:
strings are deduplicated and sorted.

Usage:
    python3 scripts/generate-command-strings.py [--check]
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COMMANDS = ROOT / "app" / "rcc" / "commands"
OUTPUT = ROOT / "core" / "Ui" / "UI" / "CommandStrings.cpp"

MANIFESTS = sorted(p.name for p in COMMANDS.glob("*.json"))
LAYOUTS = sorted(p.name for p in (COMMANDS / "layouts").glob("*.json"))

HEADER = """/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

// AUTO-GENERATED lupdate stub ("Commands" context) from app/rcc/commands/; regenerate, never edit.

#include <QCoreApplication>

// clang-format off
[[maybe_unused]] static const char* const kCommandStrings[] = {
"""

FOOTER = """};
// clang-format on
"""


def collect_node_strings(node: dict, strings: set[str]) -> None:
    for key in ("title", "tooltip", "titleChecked", "collapsedTitle"):
        value = node.get(key)
        if isinstance(value, str) and value:
            strings.add(value)
    for child in node.get("items", []):
        collect_node_strings(child, strings)


def collect_strings() -> list[str]:
    strings: set[str] = set()
    for name in MANIFESTS:
        data = json.loads((COMMANDS / name).read_text(encoding="utf-8"))
        for command in data.get("commands", []):
            collect_node_strings(command, strings)
    for name in LAYOUTS:
        data = json.loads((COMMANDS / "layouts" / name).read_text(encoding="utf-8"))
        for key in ("sections", "items", "pinnedEnd"):
            for node in data.get(key, []):
                collect_node_strings(node, strings)
        for menu in data.get("menus", []):
            for node in menu.get("items", []):
                collect_node_strings(node, strings)
    return sorted(strings)


def escaped(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def render() -> str:
    lines = [HEADER]
    for value in collect_strings():
        lines.append(f'  QT_TRANSLATE_NOOP("Commands", "{escaped(value)}"),\n')
    lines.append(FOOTER)
    return "".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    content = render()
    if args.check:
        current = OUTPUT.read_text(encoding="utf-8") if OUTPUT.exists() else ""
        if current != content:
            print(
                "generate-command-strings: CommandStrings.cpp is out of date -- regenerate"
            )
            return 1
        print("generate-command-strings: up to date")
        return 0

    OUTPUT.write_text(content, encoding="utf-8", newline="")
    print(
        f"generate-command-strings: wrote {OUTPUT.relative_to(ROOT).as_posix()} "
        f"({len(collect_strings())} strings)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
