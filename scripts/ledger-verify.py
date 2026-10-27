#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Serial Studio mistakes-ledger ratchet.

doc/claude/common-mistakes.md is a ledger of silent-breakage classes. A row
closes only when something mechanical catches the class (`Codified:` naming a
check); a row that ends `Codified: not yet` or carries no marker is a rule that
exists only as prose, and prose rules are the ones that keep failing (56 rows,
5 codified on 2026-10-27). This ratchets the open count against
scripts/ledger-baseline.json: landing a prose-only row without closing another
fails, `--accept` re-seeds the baseline after a deliberate decision, and
`--list` prints the open rows for triage.

Usage: ledger-verify.py [--check] [--accept] [--list] [--root PATH]
Exit 0 pass, 1 on growth, 2 when the ledger or baseline is unreadable.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

LEDGER = Path("doc") / "claude" / "common-mistakes.md"
BASELINE = Path("scripts") / "ledger-baseline.json"
CODIFIED_RE = re.compile(r"Codified:\s*(.*)")
ESCAPED_PIPE = "\x00"


def parse_rows(text: str) -> list[tuple[str, str]]:
    """(mistake, fix) for every table row; header and separator rows are skipped."""
    rows = []
    for line in text.splitlines():
        if not line.startswith("|"):
            continue
        cells = [
            cell.strip().replace(ESCAPED_PIPE, "|")
            for cell in line.replace("\\|", ESCAPED_PIPE).strip().strip("|").split("|")
        ]
        if len(cells) < 2 or cells[0] == "Mistake" or set(cells[0]) <= set("-: "):
            continue
        rows.append((cells[0], " | ".join(cells[1:])))
    return rows


def classify(fix: str) -> str:
    match = CODIFIED_RE.search(fix)
    if match is None:
        return "untagged"
    if match.group(1).strip().lower().startswith("not yet"):
        return "not-yet"
    return "codified"


def summarize(rows: list[tuple[str, str]]) -> dict:
    kinds = [classify(fix) for _, fix in rows]
    counts = {kind: kinds.count(kind) for kind in ("codified", "not-yet", "untagged")}
    return {"rows": len(rows), "open": counts["not-yet"] + counts["untagged"], **counts}


def open_rows(rows: list[tuple[str, str]]) -> list[tuple[str, str]]:
    return [
        (kind, mistake)
        for mistake, fix in rows
        if (kind := classify(fix)) != "codified"
    ]


def parse_args(argv: list[str] | None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Ratchet the open rows of the mistakes ledger"
    )
    parser.add_argument(
        "--check", action="store_true", help="fail when open rows exceed the baseline"
    )
    parser.add_argument(
        "--accept", action="store_true", help="re-seed the baseline from the ledger"
    )
    parser.add_argument(
        "--list", action="store_true", help="print the open rows for triage"
    )
    parser.add_argument(
        "--root", help="repository root (default: this script's repository)"
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = (
        Path(args.root).resolve() if args.root else Path(__file__).resolve().parents[1]
    )
    ledger = root / LEDGER
    if not ledger.is_file():
        print(f"ledger-verify: missing {LEDGER}", file=sys.stderr)
        return 2

    rows = parse_rows(ledger.read_text(encoding="utf-8"))
    summary = summarize(rows)
    print(
        f"ledger-verify: {summary['rows']} rows, {summary['codified']} codified, "
        f"{summary['open']} open ({summary['not-yet']} 'not yet', {summary['untagged']} untagged)"
    )
    if args.list:
        for kind, mistake in open_rows(rows):
            print(f"  [{kind}] {mistake[:110]}")

    baseline_path = root / BASELINE
    if args.accept:
        baseline_path.write_text(
            json.dumps(summary, indent=2) + "\n", encoding="utf-8", newline="\n"
        )
        print(f"ledger-verify: baseline re-seeded at open={summary['open']}")
        return 0
    if not args.check:
        return 0

    if not baseline_path.is_file():
        print(f"ledger-verify: missing {BASELINE}; run --accept once", file=sys.stderr)
        return 2
    baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
    if summary["open"] > int(baseline.get("open", 0)):
        print(
            f"ledger-verify: open rows grew from {baseline['open']} to {summary['open']}. "
            "Close an open row with a Codified: check, or re-seed with --accept after a "
            "deliberate decision (a review trigger).",
            file=sys.stderr,
        )
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
