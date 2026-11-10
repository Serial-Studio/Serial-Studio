#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""Icon-pack tooling for spec 0115: the gate, the report, the fetch list and the pack build.

A pack is app/rcc/iconpacks/<pack>/pack.json plus <category>/<tier>/<name>.svg files that
re-draw the house icons under app/rcc/icons for the themed categories. The registry falls back
to the house file per icon, so a pack may be partial; this script says how partial.

Subcommands:
  check                 the gate: layout, fixed categories refused, ids inside the house set,
                        tiers in the ladder, pack.json shape, coverage at or above --gate (or
                        the pack's own "gate" when pack.json carries one),
                        mapping <-> tree consistency, qrc <-> disk. Exit 1 on any error.
  report [PACK]         per-pack fallbacks and missing tiers; --sheet renders pack art beside
                        house art (PySide6) into --out.
  fetch-list PACK       the Icons8 id and target path for every base the pack still lacks,
                        from the mapping's column for the pack's style.
  normalize PACK        rewrite every pack SVG into the house form: width, height and viewBox
                        equal to the tier, artwork scaled in a group when the source differs.
  compose PACK          build the composite ids (base plus badge or label) from the pack's own
                        base artwork, and copy that artwork to every other plain id the base
                        serves; an id whose base the pack lacks is skipped. pack.json may carry
                        a "badges" map (plus, x, arrow, arrow-up, pencil, warning, no, lock) of
                        disc colours tuned to the style.
  qrc                   regenerate app/rcc/iconpacks/iconpacks.qrc from disk; --exclude PACK
                        leaves a pack that is still being fetched out of the list.
  house-sync            after new house icons land: normalize every house SVG git does not
                        track yet (a fresh Icons8 download; committed files are never rewritten)
                        and add every house file missing from app/rcc/rcc.qrc to its icon block.

Usage: python3 scripts/icon-packs.py <subcommand> [options]
"""

from __future__ import annotations

import argparse
import csv
import json
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ICONS = ROOT / "app" / "rcc" / "icons"
PACKS = ROOT / "app" / "rcc" / "iconpacks"
QRC = PACKS / "iconpacks.qrc"
HOUSE_QRC = ROOT / "app" / "rcc" / "rcc.qrc"
FREE_QRC = ROOT / "app" / "rcc" / "art-free.qrc"
SPEC = ROOT / "doc" / "claude" / "specs" / "0115-icon-packs"
MAPPING = SPEC / "icons8-map.csv"
BASES = SPEC / "icon-bases.txt"

TIERS = {16, 24, 32, 48}
FIXED = {"buttons", "window", "system"}
THEMED = {
    "commands",
    "editor",
    "widgets",
    "devices",
    "panes",
    "console",
    "database",
    "code",
    "licensing",
    "notifications",
    "illustrations",
}
BASES_DIR = "_bases"
NAME_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")
SVG_NS = "http://www.w3.org/2000/svg"
STYLE_COLUMN = {
    "fluent": "fluent_id",
    "liquid-glass": "glass_id",
    "deco": "deco_id",
    "fluent-systems-regular": "outline_id",
    "ios7": "ios7_id",
    "color": "color_id",
    "liquid-glass-color": "glasscolor_id",
    "deco-color": "decocolor_id",
    "clr-gls": "clrgls_id",
}
BADGE_FILL = {
    "plus": "#43A047",
    "x": "#E53935",
    "arrow": "#1E88E5",
    "arrow-up": "#1E88E5",
    "pencil": "#FB8C00",
    "warning": "#FB8C00",
    "no": "#E53935",
    "lock": "#78909C",
}
ARROW_UP_IDS = {"editor/import-table"}
LABELS = {
    "commands/mf4": ("mf4-file", "MF4", None),
    "commands/mf4-log": ("mf4-file", "MF4", "arrow"),
    "commands/x": (None, "X", None),
    "commands/y": (None, "Y", None),
}

ET.register_namespace("", SVG_NS)
ET.register_namespace("xlink", "http://www.w3.org/1999/xlink")


# ------------------------------------------------------------------------------------------------
# Data
# ------------------------------------------------------------------------------------------------


def house_ids() -> dict[str, set[int]]:
    """Every themed house icon as "category/name" -> tiers."""
    out: dict[str, set[int]] = defaultdict(set)
    for path in ICONS.rglob("*.svg"):
        parts = path.relative_to(ICONS).parts
        if len(parts) == 3 and parts[0] in THEMED and parts[1].isdigit():
            out[f"{parts[0]}/{path.stem}"].add(int(parts[1]))
    return out


def read_bases() -> dict[str, dict]:
    """The metaphor vocabulary: base -> {depicts, terms, ids: [(id, badge or None)]}."""
    bases: dict[str, dict] = {}
    for line in BASES.read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        base, depicts, terms, ids = line.rstrip("\n").split("|")
        entries = []
        for item in ids.split(";"):
            icon, _, badge = item.partition("+")
            entries.append((icon, badge or None))
        bases[base] = {"depicts": depicts, "terms": terms.split(";"), "ids": entries}
    return bases


def read_mapping() -> dict[str, dict]:
    with MAPPING.open(newline="", encoding="utf-8") as handle:
        return {row["base"]: row for row in csv.DictReader(handle)}


def pack_dirs() -> list[Path]:
    if not PACKS.is_dir():
        return []
    return sorted(
        p for p in PACKS.iterdir() if p.is_dir() and (p / "pack.json").is_file()
    )


def read_pack(folder: Path) -> dict:
    return json.loads((folder / "pack.json").read_text(encoding="utf-8"))


def pack_files(folder: Path) -> dict[str, set[int]]:
    """The pack's "category/name" -> tiers, ignoring the _bases scratch folder."""
    out: dict[str, set[int]] = defaultdict(set)
    for path in folder.rglob("*.svg"):
        parts = path.relative_to(folder).parts
        if parts[0] == BASES_DIR:
            continue
        if len(parts) == 3 and parts[1].isdigit():
            out[f"{parts[0]}/{path.stem}"].add(int(parts[1]))
    return out


def base_of(icon: str, bases: dict[str, dict]) -> tuple[str | None, str | None]:
    """The base row and badge serving one icon id."""
    for name, row in bases.items():
        for candidate, badge in row["ids"]:
            if candidate == icon:
                return name, badge
    return None, None


def plain_id(row: dict) -> str | None:
    """The first id a base serves without a badge, where its artwork lives in a pack."""
    for icon, badge in row["ids"]:
        if badge is None and icon not in LABELS:
            return icon
    return None


def rel(path: Path) -> str:
    """A path for messages: repository-relative when inside the tree, absolute otherwise."""
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


# ------------------------------------------------------------------------------------------------
# check
# ------------------------------------------------------------------------------------------------


def check_pack(
    folder: Path, house: dict[str, set[int]], gate: float, errors: list[str]
) -> float:
    where_pack = rel(folder)
    try:
        meta = read_pack(folder)
    except (OSError, json.JSONDecodeError) as exc:
        errors.append(f"{where_pack}/pack.json: unreadable ({exc})")
        return 0.0
    if meta.get("id") != folder.name:
        errors.append(
            f"{where_pack}/pack.json: id {meta.get('id')!r} does not match the folder"
        )
    for key in ("title", "style"):
        if not isinstance(meta.get(key), str) or not meta.get(key):
            errors.append(f"{where_pack}/pack.json: missing string {key!r}")
    if not isinstance(meta.get("mono"), bool):
        errors.append(f"{where_pack}/pack.json: 'mono' must be true or false")

    for path in sorted(folder.rglob("*")):
        if path.is_dir() or path.name == "pack.json" or path.name == ".DS_Store":
            continue
        parts = path.relative_to(folder).parts
        if parts[0] == BASES_DIR:
            continue
        where = rel(path)
        if path.suffix != ".svg":
            errors.append(f"{where}: not an SVG")
            continue
        if len(parts) != 3:
            errors.append(f"{where}: not <category>/<tier>/<name>.svg")
            continue
        category, tier, name = parts
        if category in FIXED:
            errors.append(
                f"{where}: a pack may not restyle the fixed category {category!r}"
            )
        elif category not in THEMED:
            errors.append(f"{where}: unknown category {category!r}")
        if not tier.isdigit() or int(tier) not in TIERS:
            errors.append(f"{where}: tier not in {sorted(TIERS)}")
        if not NAME_RE.match(path.stem):
            errors.append(f"{where}: name not kebab-case")
        if f"{category}/{path.stem}" not in house and category in THEMED:
            errors.append(
                f"{where}: no house icon {category}/{path.stem}; packs re-draw, never add"
            )

    covered = sum(1 for icon in house if icon in pack_files(folder))
    coverage = covered / len(house) if house else 0.0
    own_gate = meta.get("gate")
    if isinstance(own_gate, (int, float)) and 0 < own_gate <= 1:
        gate = float(own_gate)
    if coverage < gate:
        errors.append(
            f"{where_pack}: coverage {coverage:.1%} ({covered}/{len(house)}) is under the gate "
            f"{gate:.0%}"
        )
    return coverage


def check_mapping(house: dict[str, set[int]], errors: list[str]) -> None:
    bases = read_bases()
    mapping = read_mapping()
    mapped = {icon for row in bases.values() for icon, _ in row["ids"]}
    for icon in sorted(set(house) - mapped):
        errors.append(f"icon-bases.txt: house icon {icon} has no metaphor row")
    for icon in sorted(mapped - set(house)):
        errors.append(f"icon-bases.txt: row names {icon}, which the house tree lacks")
    for base in sorted(set(bases) - set(mapping)):
        errors.append(f"icons8-map.csv: base {base} has no mapping row")
    for base in sorted(set(mapping) - set(bases)):
        errors.append(f"icons8-map.csv: row {base} names no base")


def check_qrc(errors: list[str]) -> None:
    on_disk = set()
    for folder in pack_dirs():
        for path in folder.rglob("*"):
            if (
                path.is_file()
                and path.relative_to(folder).parts[0] != BASES_DIR
                and path.name != ".DS_Store"
            ):
                on_disk.add(path.relative_to(PACKS).as_posix())
    listed = set(re.findall(r"<file>([^<]+)</file>", QRC.read_text(encoding="utf-8")))
    for missing in sorted(on_disk - listed):
        errors.append(f"iconpacks.qrc: {missing} is on disk but not listed (run `qrc`)")
    for stale in sorted(listed - on_disk):
        errors.append(f"iconpacks.qrc: {stale} is listed but not on disk (run `qrc`)")


def cmd_check(args: argparse.Namespace) -> int:
    errors: list[str] = []
    house = house_ids()
    if not house and FREE_QRC.exists():
        print(
            "icon-packs: public tree; the house set is private, art-verify.py judges these packs"
        )
        return 0
    if not house:
        sys.exit("icon-packs: no house icons found")
    for folder in pack_dirs():
        coverage = check_pack(folder, house, args.gate, errors)
        print(f"{folder.name}: coverage {coverage:.1%}")
    check_mapping(house, errors)
    if QRC.is_file():
        check_qrc(errors)
    else:
        errors.append("iconpacks.qrc is missing (run `qrc`)")
    for message in errors:
        print(f"error: {message}")
    print(f"icon-packs: {'CLEAN' if not errors else f'{len(errors)} error(s)'}")
    return 1 if errors else 0


# ------------------------------------------------------------------------------------------------
# report
# ------------------------------------------------------------------------------------------------


def cmd_report(args: argparse.Namespace) -> int:
    house = house_ids()
    folders = [PACKS / args.pack] if args.pack else pack_dirs()
    for folder in folders:
        files = pack_files(folder)
        fallbacks = sorted(icon for icon in house if icon not in files)
        print(
            f"== {folder.name}: {len(house) - len(fallbacks)}/{len(house)} drawn, "
            f"{len(fallbacks)} fall back to the house set"
        )
        for icon in fallbacks:
            print(f"   fallback  {icon}")
        if args.sheet:
            render_sheet(folder, files, house, Path(args.out))
    return 0


def render_sheet(
    folder: Path, files: dict[str, set[int]], house: dict[str, set[int]], out: Path
) -> None:
    """Pack art beside house art, one row per icon id, for the maintainer's eye (AC9)."""
    try:
        from PySide6.QtCore import QRectF, Qt
        from PySide6.QtGui import QColor, QFont, QGuiApplication, QImage, QPainter
        from PySide6.QtSvg import QSvgRenderer
    except ImportError:
        print("report: PySide6 is not importable, no sheet rendered")
        return
    app = QGuiApplication.instance() or QGuiApplication([])
    ids = sorted(house)
    cols, cell_w, cell_h, icon = 6, 190, 70, 40
    rows = (len(ids) + cols - 1) // cols
    image = QImage(
        cols * cell_w, rows * cell_h + 24, QImage.Format_ARGB32_Premultiplied
    )
    image.fill(QColor("#f4f4f4"))
    painter = QPainter(image)
    painter.setRenderHint(QPainter.Antialiasing)
    painter.setFont(QFont("Helvetica", 9))
    painter.setPen(QColor("#222"))
    painter.drawText(8, 16, f"{folder.name}: pack art (left) beside house art (right)")
    for index, icon_id in enumerate(ids):
        x = (index % cols) * cell_w
        y = 24 + (index // cols) * cell_h
        category, name = icon_id.split("/", 1)
        house_tier = max(house[icon_id])
        QSvgRenderer(str(ICONS / category / str(house_tier) / f"{name}.svg")).render(
            painter, QRectF(x + 60, y + 4, icon, icon)
        )
        if icon_id in files:
            tier = max(files[icon_id])
            QSvgRenderer(str(folder / category / str(tier) / f"{name}.svg")).render(
                painter, QRectF(x + 8, y + 4, icon, icon)
            )
        else:
            painter.drawText(QRectF(x + 8, y + 4, icon, icon), Qt.AlignCenter, "house")
        painter.drawText(QRectF(x, y + icon + 6, cell_w, 16), Qt.AlignHCenter, icon_id)
    painter.end()
    out.parent.mkdir(parents=True, exist_ok=True)
    image.save(str(out))
    print(f"report: sheet written to {out}")
    del app


# ------------------------------------------------------------------------------------------------
# fetch-list
# ------------------------------------------------------------------------------------------------


def cmd_fetch_list(args: argparse.Namespace) -> int:
    folder = PACKS / args.pack
    meta = read_pack(folder)
    column = STYLE_COLUMN.get(meta["style"])
    if not column:
        sys.exit(f"icon-packs: no mapping column for style {meta['style']!r}")
    bases = read_bases()
    mapping = read_mapping()
    files = pack_files(folder)
    pending = 0
    for base, row in bases.items():
        entry = mapping.get(base)
        if not entry or entry["status"] in (
            "house",
            "house-copy",
            "composed",
            "design",
            "none",
        ):
            continue
        icon8 = entry[column].strip()
        if not icon8 or icon8.upper() == "NONE" or icon8 == "?":
            continue
        target = plain_id(row)
        if target:
            if target in files:
                continue
            category, name = target.split("/", 1)
            path = folder / category / "48" / f"{name}.svg"
        else:
            path = folder / BASES_DIR / f"{base}.svg"
            if path.is_file():
                continue
        pending += 1
        print(f"{icon8}\t{rel(path)}\t{base}")
    print(f"# {pending} download(s) pending for {args.pack}", file=sys.stderr)
    return 0


# ------------------------------------------------------------------------------------------------
# normalize
# ------------------------------------------------------------------------------------------------


def parse_length(value: str | None) -> float | None:
    if not value:
        return None
    match = re.match(r"^\s*([0-9.]+)", value)
    return float(match.group(1)) if match else None


def normalize_svg(path: Path, tier: int) -> bool:
    """Rewrites @p path into the house form for @p tier; returns whether anything changed."""
    tree = ET.parse(path)
    root = tree.getroot()
    view = root.get("viewBox")
    if view:
        _, _, width, height = (float(v) for v in view.replace(",", " ").split())
    else:
        width = parse_length(root.get("width")) or tier
        height = parse_length(root.get("height")) or tier
    size = max(width, height)
    scale = tier / size if size else 1.0
    before = ET.tostring(root)
    for attr in list(root.attrib):
        if attr.startswith("data-") or attr in ("id", "class", "style"):
            del root.attrib[attr]
    root.set("width", str(tier))
    root.set("height", str(tier))
    root.set("viewBox", f"0 0 {tier} {tier}")
    if abs(scale - 1.0) > 1e-6 and not (
        len(root) == 1 and root[0].get("data-ss-scale")
    ):
        group = ET.Element(f"{{{SVG_NS}}}g")
        group.set("transform", f"scale({scale:.8f})")
        group.set("data-ss-scale", "1")
        for child in list(root):
            root.remove(child)
            group.append(child)
        root.append(group)
    after = ET.tostring(root)
    if after != before:
        path.write_bytes(b'<?xml version="1.0" encoding="UTF-8"?>\n' + after + b"\n")
        return True
    return False


def cmd_normalize(args: argparse.Namespace) -> int:
    folder = PACKS / args.pack
    changed = 0
    for path in sorted(folder.rglob("*.svg")):
        parts = path.relative_to(folder).parts
        tier = int(parts[1]) if len(parts) == 3 and parts[1].isdigit() else 48
        if normalize_svg(path, tier):
            changed += 1
    print(f"normalize: {changed} file(s) rewritten in {args.pack}")
    return 0


# ------------------------------------------------------------------------------------------------
# compose
# ------------------------------------------------------------------------------------------------


def badge_group(
    kind: str, tier: int, mono: bool = False, palette: dict[str, str] | None = None
) -> ET.Element:
    """A bottom-right badge, 44 % of the tier: a filled disc with a white glyph for a colour
    pack; for a mono pack the glyph alone in black (the tint, once recoloured), drawn into a hole
    the caller cuts out of the base with knockout_mask(), the way outline icon sets badge.
    """
    size = tier * 0.44
    cx = tier - size / 2
    cy = tier - size / 2
    group = ET.Element(f"{{{SVG_NS}}}g")
    group.set("data-ss-badge", kind)
    if not mono:
        disc = ET.SubElement(group, f"{{{SVG_NS}}}circle")
        disc.set("cx", f"{cx:.3f}")
        disc.set("cy", f"{cy:.3f}")
        disc.set("r", f"{size / 2:.3f}")
        disc.set("fill", (palette or {}).get(kind, BADGE_FILL[kind]))
        disc.set("stroke", "#FFFFFF")
        disc.set("stroke-width", f"{tier * 0.03:.3f}")
    bar = size * 0.52
    thick = size * 0.14
    glyph = "#000000" if mono else "#FFFFFF"
    if kind in ("plus", "x"):
        for angle in ((0, 90) if kind == "plus" else (45, -45)):
            rect = ET.SubElement(group, f"{{{SVG_NS}}}rect")
            rect.set("x", f"{cx - bar / 2:.3f}")
            rect.set("y", f"{cy - thick / 2:.3f}")
            rect.set("width", f"{bar:.3f}")
            rect.set("height", f"{thick:.3f}")
            rect.set("rx", f"{thick / 2:.3f}")
            rect.set("fill", glyph)
            rect.set("transform", f"rotate({angle} {cx:.3f} {cy:.3f})")
    elif kind in ("arrow", "arrow-up"):
        up = kind == "arrow-up"
        shaft = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        shaft.set("x", f"{cx - thick / 2:.3f}")
        shaft.set("y", f"{cy - bar / 2 + (bar * 0.45 if up else 0):.3f}")
        shaft.set("width", f"{thick:.3f}")
        shaft.set("height", f"{bar * 0.55:.3f}")
        shaft.set("fill", glyph)
        head = ET.SubElement(group, f"{{{SVG_NS}}}path")
        half = bar * 0.42
        tip = cy - bar / 2 if up else cy + bar / 2
        base_y = tip + half if up else tip - half
        head.set(
            "d",
            f"M {cx - half:.3f} {base_y:.3f} L {cx + half:.3f} {base_y:.3f} "
            f"L {cx:.3f} {tip:.3f} Z",
        )
        head.set("fill", glyph)
    elif kind == "pencil":
        body = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        body.set("x", f"{cx - thick / 2:.3f}")
        body.set("y", f"{cy - bar / 2:.3f}")
        body.set("width", f"{thick:.3f}")
        body.set("height", f"{bar:.3f}")
        body.set("rx", f"{thick / 4:.3f}")
        body.set("fill", glyph)
        body.set("transform", f"rotate(45 {cx:.3f} {cy:.3f})")
    elif kind == "no":
        ring = ET.SubElement(group, f"{{{SVG_NS}}}circle")
        ring.set("cx", f"{cx:.3f}")
        ring.set("cy", f"{cy:.3f}")
        ring.set("r", f"{size * 0.3:.3f}")
        ring.set("fill", "none")
        ring.set("stroke", glyph)
        ring.set("stroke-width", f"{thick * 0.9:.3f}")
        slash = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        slash.set("x", f"{cx - thick * 0.45:.3f}")
        slash.set("y", f"{cy - size * 0.3:.3f}")
        slash.set("width", f"{thick * 0.9:.3f}")
        slash.set("height", f"{size * 0.6:.3f}")
        slash.set("fill", glyph)
        slash.set("transform", f"rotate(45 {cx:.3f} {cy:.3f})")
    elif kind == "lock":
        body = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        body.set("x", f"{cx - size * 0.24:.3f}")
        body.set("y", f"{cy - size * 0.02:.3f}")
        body.set("width", f"{size * 0.48:.3f}")
        body.set("height", f"{size * 0.36:.3f}")
        body.set("rx", f"{size * 0.06:.3f}")
        body.set("fill", glyph)
        shackle = ET.SubElement(group, f"{{{SVG_NS}}}path")
        r = size * 0.16
        shackle.set(
            "d",
            f"M {cx - r:.3f} {cy - size * 0.02:.3f} V {cy - size * 0.16:.3f} "
            f"A {r:.3f} {r:.3f} 0 0 1 {cx + r:.3f} {cy - size * 0.16:.3f} "
            f"V {cy - size * 0.02:.3f}",
        )
        shackle.set("fill", "none")
        shackle.set("stroke", glyph)
        shackle.set("stroke-width", f"{thick * 0.8:.3f}")
    elif kind == "warning":
        mark = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        mark.set("x", f"{cx - thick / 2:.3f}")
        mark.set("y", f"{cy - bar / 2:.3f}")
        mark.set("width", f"{thick:.3f}")
        mark.set("height", f"{bar * 0.6:.3f}")
        mark.set("rx", f"{thick / 2:.3f}")
        mark.set("fill", glyph)
        dot = ET.SubElement(group, f"{{{SVG_NS}}}circle")
        dot.set("cx", f"{cx:.3f}")
        dot.set("cy", f"{cy + bar / 2 - thick * 0.6:.3f}")
        dot.set("r", f"{thick * 0.6:.3f}")
        dot.set("fill", glyph)
    return group


def knockout_mask(
    root: ET.Element, tier: int, badge: bool, label_box: tuple | None
) -> None:
    """Wraps the base artwork of a mono composite in a mask that cuts out the badge disc and the
    label box, so the tinted glyph or text sits in a clean hole instead of over the base lines.
    """
    mask_id = "ss-knockout"
    defs = ET.Element(f"{{{SVG_NS}}}defs")
    mask = ET.SubElement(defs, f"{{{SVG_NS}}}mask")
    mask.set("id", mask_id)
    paper = ET.SubElement(mask, f"{{{SVG_NS}}}rect")
    paper.set("x", "0")
    paper.set("y", "0")
    paper.set("width", str(tier))
    paper.set("height", str(tier))
    paper.set("fill", "white")
    if badge:
        size = tier * 0.44
        hole = ET.SubElement(mask, f"{{{SVG_NS}}}circle")
        hole.set("cx", f"{tier - size / 2:.3f}")
        hole.set("cy", f"{tier - size / 2:.3f}")
        hole.set("r", f"{size / 2 + tier * 0.03:.3f}")
        hole.set("fill", "black")
    if label_box:
        x, y, w, h = label_box
        box = ET.SubElement(mask, f"{{{SVG_NS}}}rect")
        box.set("x", f"{x - tier * 0.03:.3f}")
        box.set("y", f"{y - tier * 0.03:.3f}")
        box.set("width", f"{w + tier * 0.06:.3f}")
        box.set("height", f"{h + tier * 0.06:.3f}")
        box.set("fill", "black")
    wrapper = ET.Element(f"{{{SVG_NS}}}g")
    wrapper.set("mask", f"url(#{mask_id})")
    wrapper.set("data-ss-masked", "1")
    for child in list(root):
        root.remove(child)
        wrapper.append(child)
    root.append(defs)
    root.append(wrapper)


def label_box(tier: int, beside_badge: bool) -> tuple:
    width = tier * (0.56 if beside_badge else 0.84)
    return (tier * 0.08, tier * 0.56, width, tier * 0.3)


def label_group(
    text: str, tier: int, band: bool, beside_badge: bool = False, mono: bool = False
) -> ET.Element:
    """A text label: on a dark band across the lower third when @p band (narrowed to leave the
    bottom-right corner free when a badge sits there), else centred large."""
    group = ET.Element(f"{{{SVG_NS}}}g")
    group.set("data-ss-label", text)
    width = tier * (0.56 if beside_badge else 0.84)
    centre = tier * 0.08 + width / 2
    if band and not mono:
        rect = ET.SubElement(group, f"{{{SVG_NS}}}rect")
        rect.set("x", f"{tier * 0.08:.3f}")
        rect.set("y", f"{tier * 0.56:.3f}")
        rect.set("width", f"{width:.3f}")
        rect.set("height", f"{tier * 0.3:.3f}")
        rect.set("rx", f"{tier * 0.06:.3f}")
        rect.set("fill", "#37474F")
    label = ET.SubElement(group, f"{{{SVG_NS}}}text")
    label.set("x", f"{centre if band else tier / 2:.3f}")
    label.set("y", f"{tier * 0.79 if band else tier * 0.68:.3f}")
    label.set("text-anchor", "middle")
    label.set("font-family", "Helvetica, Arial, sans-serif")
    label.set("font-weight", "normal" if mono and not band else "bold")
    label.set("font-size", f"{tier * (0.24 if band else 0.6):.3f}")
    label.set("fill", "#000000" if mono else ("#FFFFFF" if band else "#37474F"))
    label.text = text
    return group


def letter_tile(tier: int, mono: bool = False) -> ET.Element:
    tile = ET.Element(f"{{{SVG_NS}}}rect")
    tile.set("x", f"{tier * 0.1:.3f}")
    tile.set("y", f"{tier * 0.1:.3f}")
    tile.set("width", f"{tier * 0.8:.3f}")
    tile.set("height", f"{tier * 0.8:.3f}")
    tile.set("rx", f"{tier * 0.16:.3f}")
    tile.set("fill", "none" if mono else "#A5D6A7")
    tile.set("stroke", "#000000" if mono else "#2E7D32")
    tile.set("stroke-width", f"{tier * 0.04:.3f}")
    return tile


def base_artwork(
    folder: Path, base: str, row: dict, files: dict[str, set[int]]
) -> tuple[Path, int] | None:
    """Where the pack keeps a base's drawing: its plain id file, else the _bases scratch file."""
    plain = plain_id(row)
    if plain and plain in files:
        tier = max(files[plain])
        category, name = plain.split("/", 1)
        return folder / category / str(tier) / f"{name}.svg", tier
    scratch = folder / BASES_DIR / f"{base}.svg"
    if scratch.is_file():
        return scratch, 48
    return None


def compose_one(
    source: Path | None,
    tier: int,
    target: Path,
    badge: str | None,
    label: str | None,
    mono: bool = False,
    palette: dict[str, str] | None = None,
) -> None:
    if source is not None:
        tree = ET.parse(source)
        root = tree.getroot()
        normalize_root(root, tier)
    else:
        root = ET.Element(f"{{{SVG_NS}}}svg")
        root.set("width", str(tier))
        root.set("height", str(tier))
        root.set("viewBox", f"0 0 {tier} {tier}")
        root.append(letter_tile(tier, mono))
    for child in list(root):
        own_defs = child.tag.endswith("defs") and any(
            grand.get("id") == "ss-knockout" for grand in child
        )
        if child.get("data-ss-badge") or child.get("data-ss-label") or own_defs:
            root.remove(child)
        elif child.get("data-ss-masked"):
            root.remove(child)
            for grand in list(child):
                root.append(grand)
    banded = source is not None
    if mono and (badge or (label and banded)):
        knockout_mask(
            root,
            tier,
            bool(badge),
            label_box(tier, bool(badge)) if label and banded else None,
        )
    if label:
        root.append(
            label_group(label, tier, band=banded, beside_badge=bool(badge), mono=mono)
        )
    if badge:
        root.append(badge_group(badge, tier, mono, palette))
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(
        b'<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root) + b"\n"
    )


def normalize_root(root: ET.Element, tier: int) -> None:
    """In-memory counterpart of normalize_svg for a parsed base about to receive a badge."""
    view = root.get("viewBox")
    size = tier
    if view:
        _, _, width, height = (float(v) for v in view.replace(",", " ").split())
        size = max(width, height)
    scale = tier / size if size else 1.0
    root.set("width", str(tier))
    root.set("height", str(tier))
    root.set("viewBox", f"0 0 {tier} {tier}")
    if abs(scale - 1.0) > 1e-6 and not (
        len(root) == 1 and root[0].get("data-ss-scale")
    ):
        group = ET.Element(f"{{{SVG_NS}}}g")
        group.set("transform", f"scale({scale:.8f})")
        group.set("data-ss-scale", "1")
        for child in list(root):
            root.remove(child)
            group.append(child)
        root.append(group)


def cmd_compose(args: argparse.Namespace) -> int:
    folder = PACKS / args.pack
    meta = read_pack(folder)
    mono = bool(meta.get("mono", False))
    palette = meta.get("badges") if isinstance(meta.get("badges"), dict) else None
    bases = read_bases()
    mapping = read_mapping()
    files = pack_files(folder)
    house = house_ids()
    made = skipped = spread = 0
    for base, row in bases.items():
        status = mapping.get(base, {}).get("status")
        if status == "house-copy":
            for icon, badge in row["ids"]:
                if badge is None and icon not in files:
                    category, name = icon.split("/", 1)
                    tiers = house.get(icon) or set()
                    if not tiers:
                        continue
                    tier = max(tiers)
                    target = folder / category / str(tier) / f"{name}.svg"
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(
                        ICONS / category / str(tier) / f"{name}.svg", target
                    )
                    files[icon].add(tier)
                    spread += 1
            continue
        for icon, badge in row["ids"]:
            label_spec = LABELS.get(icon)
            if badge is None and label_spec is None:
                if icon in files:
                    continue
                art = base_artwork(folder, base, row, files)
                if art is None:
                    skipped += 1
                    continue
                category, name = icon.split("/", 1)
                target = folder / category / str(art[1]) / f"{name}.svg"
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(art[0], target)
                files[icon].add(art[1])
                spread += 1
                continue
            category, name = icon.split("/", 1)
            if label_spec and label_spec[0] is None:
                tier = 48
                compose_one(
                    None,
                    tier,
                    folder / category / str(tier) / f"{name}.svg",
                    None,
                    label_spec[1],
                    mono,
                    palette,
                )
                made += 1
                continue
            art = base_artwork(folder, base, row, files)
            if art is None:
                skipped += 1
                continue
            source, tier = art
            text = label_spec[1] if label_spec else None
            kind = label_spec[2] if label_spec else badge
            if kind == "arrow" and icon in ARROW_UP_IDS:
                kind = "arrow-up"
            compose_one(
                source,
                tier,
                folder / category / str(tier) / f"{name}.svg",
                kind,
                text,
                mono,
                palette,
            )
            made += 1
    print(
        f"compose: {made} composite(s) written, {spread} plain id(s) spread from their base, "
        f"{skipped} skipped (base not in pack)"
    )
    return 0


# ------------------------------------------------------------------------------------------------
# qrc
# ------------------------------------------------------------------------------------------------


def cmd_qrc(args: argparse.Namespace) -> int:
    excluded = set(getattr(args, "exclude", None) or [])
    entries = []
    for folder in pack_dirs():
        if folder.name in excluded:
            continue
        for path in sorted(folder.rglob("*")):
            if (
                path.is_file()
                and path.relative_to(folder).parts[0] != BASES_DIR
                and path.name != ".DS_Store"
            ):
                entries.append(path.relative_to(PACKS).as_posix())
    lines = ["<RCC>", '    <qresource prefix="/iconpacks">']
    lines += [f"        <file>{entry}</file>" for entry in sorted(entries)]
    lines += ["    </qresource>", "</RCC>", ""]
    QRC.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"qrc: {len(entries)} file(s) listed")
    return 0


# ------------------------------------------------------------------------------------------------
# house-sync
# ------------------------------------------------------------------------------------------------


def cmd_house_sync(_: argparse.Namespace) -> int:
    """Only files git does not track are normalized: the committed house set is never rewritten."""
    tracked = set(
        subprocess.run(
            ["git", "ls-files", "--", str(ICONS)],
            capture_output=True,
            text=True,
            check=False,
            cwd=ROOT,
        ).stdout.split()
    )
    normalized = 0
    for path in sorted(ICONS.rglob("*.svg")):
        parts = path.relative_to(ICONS).parts
        if len(parts) != 3 or not parts[1].isdigit():
            continue
        if path.relative_to(ROOT).as_posix() in tracked:
            continue
        if normalize_svg(path, int(parts[1])):
            normalized += 1
    text = HOUSE_QRC.read_text(encoding="utf-8")
    listed = set(re.findall(r"<file>(icons/[^<]+)</file>", text))
    on_disk = {f"icons/{p.relative_to(ICONS).as_posix()}" for p in ICONS.rglob("*.svg")}
    missing = sorted(on_disk - listed)
    if missing:
        lines = text.split("\n")
        for entry in missing:
            new_line = f"        <file>{entry}</file>"
            index = next(
                (
                    i
                    for i, line in enumerate(lines)
                    if line.strip().startswith("<file>icons/")
                    and line.strip() > new_line.strip()
                ),
                None,
            )
            if index is None:
                index = (
                    max(i for i, line in enumerate(lines) if "<file>icons/" in line) + 1
                )
            lines.insert(index, new_line)
        HOUSE_QRC.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(
        f"house-sync: {normalized} file(s) normalized, {len(missing)} qrc entr{'y' if len(missing) == 1 else 'ies'} added"
    )
    return 0


# ------------------------------------------------------------------------------------------------
# main
# ------------------------------------------------------------------------------------------------


def use_art_root(root: Path) -> None:
    """Points every tree at the private art checkout (spec 0116)."""
    global ICONS, PACKS, QRC, HOUSE_QRC
    ICONS = root / "icons"
    PACKS = root / "iconpacks"
    QRC = PACKS / "iconpacks.qrc"
    HOUSE_QRC = root / "art.qrc"


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--art-root",
        type=Path,
        default=None,
        help="private art checkout holding icons/ and iconpacks/ (spec 0116)",
    )
    sub = parser.add_subparsers(dest="command", required=True)
    check = sub.add_parser("check")
    check.add_argument("--gate", type=float, default=0.85)
    check.set_defaults(func=cmd_check)
    report = sub.add_parser("report")
    report.add_argument("pack", nargs="?")
    report.add_argument("--sheet", action="store_true")
    report.add_argument("--out", default=str(ROOT / "icon-pack-sheet.png"))
    report.set_defaults(func=cmd_report)
    fetch = sub.add_parser("fetch-list")
    fetch.add_argument("pack")
    fetch.set_defaults(func=cmd_fetch_list)
    norm = sub.add_parser("normalize")
    norm.add_argument("pack")
    norm.set_defaults(func=cmd_normalize)
    comp = sub.add_parser("compose")
    comp.add_argument("pack")
    comp.set_defaults(func=cmd_compose)
    qrc = sub.add_parser("qrc")
    qrc.add_argument(
        "--exclude",
        action="append",
        metavar="PACK",
        help="leave this pack out (a pack still being fetched)",
    )
    qrc.set_defaults(func=cmd_qrc)
    house = sub.add_parser("house-sync")
    house.set_defaults(func=cmd_house_sync)
    args = parser.parse_args()
    if args.art_root:
        use_art_root(args.art_root)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
