#!/usr/bin/env python3
"""Generate an "add-*" editor icon by stamping the + badge onto a base glyph.

The + badges live in scripts/assets/add-badge-<tier>.svg (normalized to their tier).
This reproduces, deterministically, the add-widget icon family: a base glyph (usually a
widgets/* icon) with a small green "+" overlaid bottom-right, at every tier.

Usage:
    scripts/overlay-add-icon.py add-plot --from widgets/plot   # copy base tiers, then badge
    scripts/overlay-add-icon.py add-plot                       # badge existing editor/add-plot tiers

Copies leave the base untouched; a root id is stamped so the copy is never byte-identical
to its source (registry-verify forbids duplicates). Re-running is safe: a tier that already
carries the badge is skipped.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ICONS = ROOT / "app" / "rcc" / "icons"
ASSETS = ROOT / "scripts" / "assets"
TIERS = (16, 24, 32, 48)
BADGE_MARKERS = ("add-badge", "surface25", "surface78", "surface131", "surface178")


def badge_body(tier: int) -> str:
    text = (ASSETS / f"add-badge-{tier}.svg").read_text(encoding="utf-8")
    open_tag = re.search(r"<svg\b[^>]*>", text, re.S)
    body = text[open_tag.end() : text.rfind("</svg>")].strip("\n")
    return f'<g id="add-badge">{body}</g>'


def stamp_id(path: Path, name: str) -> None:
    text = path.read_text(encoding="utf-8")
    tag = re.search(r"<svg\b[^>]*>", text, re.S).group(0)
    if re.search(r'\bid\s*=\s*"[^"]*"', tag):
        new = re.sub(r'\bid\s*=\s*"[^"]*"', f'id="{name}"', tag, count=1)
    else:
        new = tag.replace("<svg", f'<svg id="{name}"', 1)
    path.write_text(text.replace(tag, new, 1), encoding="utf-8", newline="\n")


def overlay(path: Path, tier: int) -> bool:
    text = path.read_text(encoding="utf-8")
    if "add-badge" in text:
        return False
    cut = text.rfind("</svg>")
    path.write_text(
        text[:cut] + "\n" + badge_body(tier) + "\n</svg>\n",
        encoding="utf-8",
        newline="\n",
    )
    return True


def main() -> int:
    global ICONS, ASSETS
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("name", help='target icon name, e.g. "add-plot"')
    parser.add_argument(
        "--from", dest="base", help='base glyph "category/name" to copy first'
    )
    parser.add_argument(
        "--art-root",
        type=Path,
        default=None,
        help="private art checkout holding icons/ and assets/ (spec 0116)",
    )
    args = parser.parse_args()
    if args.art_root:
        ICONS = args.art_root / "icons"
        ASSETS = args.art_root / "assets"

    if args.base:
        base_cat, _, base_name = args.base.partition("/")
        for tier in TIERS:
            src = ICONS / base_cat / str(tier) / f"{base_name}.svg"
            if not src.exists():
                print(f"skip {tier}: no base {src.relative_to(ICONS)}")
                continue
            dst = ICONS / "editor" / str(tier) / f"{args.name}.svg"
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(src, dst)
            stamp_id(dst, args.name)

    stamped = 0
    for tier in TIERS:
        path = ICONS / "editor" / str(tier) / f"{args.name}.svg"
        if path.exists() and overlay(path, tier):
            stamped += 1
            print(f"badged editor/{tier}/{args.name}.svg")
    if not stamped:
        print("nothing to do (no matching tiers, or badge already present)")
    print(f"done: {stamped} tier(s) badged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
