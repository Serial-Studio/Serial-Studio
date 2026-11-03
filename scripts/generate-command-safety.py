#!/usr/bin/env python3
"""Generate the constexpr command-safety table from app/rcc/ai/command_safety.json.

The assistant's tier table is the one hand-edited home of per-command safety
(doc/claude/architecture/ai.md, "The Tier Model"). This script projects it into
core/Api/API/CommandSafetyTable.cpp so the API library, which may neither read a
Qt resource nor include core/Ui, serves the same tiers as MCP tool hints
(spec 0112). Deterministic: rows are sorted by byte order, the order that
API::CommandSafety::tierOf binary-searches in. A device-gated name carries the
tier it resolves to once device control is enabled (AlwaysConfirm) plus the flag.

Usage:
    python3 scripts/generate-command-safety.py [--check] [--source JSON] [--output CPP]

Exit codes: 0 written or up to date; 1 drift under --check; 2 a malformed table
(a name in two tiers, a non-ASCII name, a name over 64 characters, which is the
Claude directory's tool-name limit, or an assistantTools name that has no tier).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "app" / "rcc" / "ai" / "command_safety.json"
OUTPUT = ROOT / "core" / "Api" / "API" / "CommandSafetyTable.cpp"

TIERS = (
    ("safe", "Safe", False),
    ("confirm", "Confirm", False),
    ("alwaysConfirm", "AlwaysConfirm", False),
    ("blocked", "Blocked", False),
    ("deviceGated", "AlwaysConfirm", True),
)
MAX_NAME_LENGTH = 64

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

// AUTO-GENERATED from app/rcc/ai/command_safety.json; regenerate, never edit.

#include <array>

#include "API/CommandSafety.h"

namespace API::CommandSafety {{

// clang-format off
static constexpr std::array<Entry, {rows}> kEntries = {{{{
"""

MIDDLE = """}}}};

static constexpr std::array<QLatin1StringView, {tools}> kAssistantTools = {{{{
"""

FOOTER = """}};
// clang-format on

/**
 * @brief Every tiered name with its tier and device-gate flag, sorted by byte order.
 */
std::span<const Entry> entries() noexcept
{
  return std::span<const Entry>(kEntries);
}

/**
 * @brief The assistant-only sandbox verbs that carry a tier but are not API commands.
 */
std::span<const QLatin1StringView> assistantTools() noexcept
{
  return std::span<const QLatin1StringView>(kAssistantTools);
}
}  // namespace API::CommandSafety
"""


class TableError(Exception):
    """A table the generator refuses to project, with the offending name in the message."""


def check_name(name: object, where: str) -> str:
    if not isinstance(name, str) or not name:
        raise TableError(f"{where}: every entry must be a non-empty string")
    if not name.isascii():
        raise TableError(f"{where}: {name!r} is not ASCII")
    if len(name) > MAX_NAME_LENGTH:
        raise TableError(f"{where}: {name} exceeds {MAX_NAME_LENGTH} characters")
    return name


def load_rows(source: Path) -> tuple[list[tuple[str, str, bool]], list[str]]:
    data = json.loads(source.read_text(encoding="utf-8"))
    rows: dict[str, tuple[str, bool]] = {}
    for key, tier, gated in TIERS:
        for raw in data.get(key, []):
            name = check_name(raw, key)
            if name in rows:
                raise TableError(f"{name} is listed in two tiers ({key} and another)")
            rows[name] = (tier, gated)

    tools = [
        check_name(raw, "assistantTools") for raw in data.get("assistantTools", [])
    ]
    for name in tools:
        if name not in rows:
            raise TableError(f"assistantTools: {name} has no tier")

    ordered = sorted(rows.items(), key=lambda item: item[0].encode("ascii"))
    return [(name, tier, gated) for name, (tier, gated) in ordered], sorted(tools)


def render(source: Path) -> str:
    rows, tools = load_rows(source)
    lines = [HEADER.format(rows=len(rows))]
    for name, tier, gated in rows:
        flag = "true" if gated else "false"
        lines.append(f'  {{QLatin1StringView("{name}"), Tier::{tier}, {flag}}},\n')
    lines.append(MIDDLE.format(tools=len(tools)))
    for name in tools:
        lines.append(f'  QLatin1StringView("{name}"),\n')
    lines.append(FOOTER)
    return "".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    args = parser.parse_args()

    try:
        content = render(args.source)
    except TableError as error:
        print(f"generate-command-safety: {error}", file=sys.stderr)
        return 2

    if args.check:
        current = (
            args.output.read_text(encoding="utf-8") if args.output.exists() else ""
        )
        if current != content:
            print(
                "generate-command-safety: CommandSafetyTable.cpp is out of date -- regenerate"
            )
            return 1
        print("generate-command-safety: up to date")
        return 0

    args.output.write_text(content, encoding="utf-8", newline="")
    rows, tools = load_rows(args.source)
    print(
        f"generate-command-safety: wrote {args.output.relative_to(ROOT).as_posix()} "
        f"({len(rows)} rows, {len(tools)} assistant tools)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
