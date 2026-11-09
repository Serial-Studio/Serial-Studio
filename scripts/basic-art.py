#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""Generates Basic, the icon theme drawn for Serial Studio, and the free art tree (spec 0116).

The SVG sources live in art/drawn. doc/claude/specs/0116-tango-public-icons/art-map.csv names the
drawing behind every icon, so re-running this script reproduces the output, which is committed:
nobody needs PySide6 to build. Tabler (MIT) supplies the single-color glyphs and the corner badges.

Each colored icon becomes <name>.png and <name>@2x.png at every size the application offers. The
script writes basic.qrc (compiled into every build), art-free.qrc (compiled when the private art
is absent) and the art-provenance.json allowlist that scripts/art-verify.py reads.

Usage: python3 scripts/basic-art.py [--out DIR] [--cache DIR]
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from xml.sax.saxutils import escape

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PySide6.QtCore import QBuffer, QByteArray, QIODevice, QRectF  # noqa: E402
from PySide6.QtGui import (  # noqa: E402
    QColor,
    QGuiApplication,
    QImage,
    QLinearGradient,
    QPainter,
    QPainterPath,
    QPen,
)
from PySide6.QtSvg import QSvgRenderer  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
RCC = ROOT / "app" / "rcc"
SPEC = ROOT / "doc" / "claude" / "specs" / "0116-tango-public-icons"
ART_MAP = SPEC / "art-map.csv"
TARGETS = SPEC / "art-targets.txt"
ICON_IDS = ROOT / "doc" / "claude" / "specs" / "0115-icon-packs" / "icons8-map.csv"
DRAWN = ROOT / "art" / "drawn"

TABLER_URL = "https://registry.npmjs.org/@tabler/icons/-/icons-3.51.0.tgz"
TABLER_SHA256 = "b30108713d3fa60aee862c2a133225da8e0f309be0708fe2925763ff810c766c"

FIRST_PARTY = "GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial"
PACK = "iconpacks/basic"
MONO_COLOR = "#000000"
ART_SUFFIXES = (".svg", ".png", ".ico", ".icns")
ACTION_PX = 48

GREEN = ("#d1ff82", "#68b723", "#206b00")
RED = ("#ff8c82", "#c6262e", "#7a0000")
BLUE = ("#8cd5ff", "#3689e6", "#0d52bf")
ORANGE = ("#ffc27d", "#f37329", "#a62100")
SLATE = ("#95a3ab", "#485a6c", "#273445")
BADGES = {
    "plus": ("plus", GREEN),
    "x": ("x", RED),
    "no": ("ban", RED),
    "arrow": ("arrow-right", BLUE),
    "arrow-up": ("arrow-up", BLUE),
    "warning": ("exclamation-mark", ORANGE),
    "lock": ("lock", SLATE),
    "pencil": ("pencil", ORANGE),
}
KEPT = (
    "icons/buttons/mqtt-on.svg",
    "icons/buttons/mqtt-off.svg",
    "images/buy-qr.svg",
    "images/dialog-banner.svg",
    "images/materials/brushed-rows.png",
    "images/materials/brushed-rows@2x.png",
    "images/materials/brushed-streaks.png",
    "images/materials/brushed-streaks@2x.png",
)


@dataclass(frozen=True)
class Row:
    """One line of art-map.csv."""

    kind: str
    key: str
    source: str
    ref: str
    status: str
    tabler: str
    tint: str


def fetch(url: str, digest: str, cache: Path) -> Path:
    """Returns the cached download of @p url, fetching it once and refusing a wrong hash."""
    target = cache / url.rsplit("/", 1)[1]
    if not target.exists():
        with urllib.request.urlopen(url, timeout=120) as reply:
            target.write_bytes(reply.read())
    actual = hashlib.sha256(target.read_bytes()).hexdigest()
    if actual != digest:
        sys.exit(f"basic-art: {target.name} hash mismatch: {actual}")
    return target


class Sources:
    """Reads Tabler's glyphs out of its release archive."""

    def __init__(self, cache: Path) -> None:
        self.tabler = tarfile.open(fetch(TABLER_URL, TABLER_SHA256, cache))

    def tabler_svg(self, name: str, color: str) -> bytes:
        member = self.tabler.getmember(f"package/icons/outline/{name}.svg")
        text = self.tabler.extractfile(member).read().decode("utf-8")
        return text.replace("currentColor", color).encode("utf-8")


def render(svg: bytes, px: int) -> QImage:
    """Rasterizes @p svg to a transparent @p px square."""
    image = QImage(px, px, QImage.Format_ARGB32_Premultiplied)
    image.fill(0)
    painter = QPainter(image)
    QSvgRenderer(QByteArray(svg)).render(painter, QRectF(0, 0, px, px))
    painter.end()
    return image


def encode(image: QImage, fmt: str = "PNG") -> bytes:
    """Returns @p image encoded as @p fmt."""
    array = QByteArray()
    buffer = QBuffer(array)
    buffer.open(QIODevice.WriteOnly)
    if not image.save(buffer, fmt):
        sys.exit(f"basic-art: cannot encode {fmt}")
    return bytes(array)


def fill_disc(painter: QPainter, rect: QRectF, colors: tuple[str, str, str]) -> None:
    """Paints a top-lit disc with a darker edge."""
    gradient = QLinearGradient(rect.topLeft(), rect.bottomLeft())
    gradient.setColorAt(0.0, QColor(colors[0]))
    gradient.setColorAt(0.45, QColor(colors[1]))
    gradient.setColorAt(1.0, QColor(colors[2]))
    path = QPainterPath()
    path.addEllipse(rect)
    painter.fillPath(path, gradient)
    edge = QColor(colors[2])
    edge.setAlpha(170)
    painter.setPen(QPen(edge, max(0.75, rect.width() / 40)))
    painter.drawPath(path)


def with_badge(image: QImage, sources: Sources, badge: str, px: int) -> QImage:
    """Draws a colored disc with a white glyph in the bottom-right corner of @p image."""
    glyph, colors = BADGES[badge]
    size = max(9, round(px * 0.56))
    corner = px - size
    painter = QPainter(image)
    painter.setRenderHint(QPainter.Antialiasing)
    ring = QPainterPath()
    ring.addEllipse(QRectF(corner - 0.5, corner - 0.5, size + 0.5, size + 0.5))
    painter.fillPath(ring, QColor("#ffffff"))
    fill_disc(
        painter, QRectF(corner + 0.5, corner + 0.5, size - 1.5, size - 1.5), colors
    )
    inner = max(6, round(size * 0.72))
    offset = corner + (size - inner) // 2
    painter.drawImage(
        offset, offset, render(sources.tabler_svg(glyph, "#ffffff"), inner)
    )
    painter.end()
    return image


def art_image(sources: Sources, row: Row, px: int) -> QImage:
    """Returns the drawing @p row names at @p px, with the badge its ref asks for."""
    name, _, badge = row.ref.partition("+")
    image = render((DRAWN / f"{name}.svg").read_bytes(), px)
    return with_badge(image, sources, badge, px) if badge else image


class Writer:
    """Writes the tree under @p out and records what each file is."""

    def __init__(self, out: Path) -> None:
        self.out = out
        self.files: dict[str, dict[str, str]] = {}
        self.qrc: dict[str, list[str]] = {"free": [], "basic": []}

    def put(
        self,
        rel: str,
        data: bytes,
        source: str,
        license_id: str,
        qrc: str | None = "free",
    ) -> None:
        path = self.out / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        if path.suffix in ART_SUFFIXES:
            digest = hashlib.sha256(data).hexdigest()
            self.files[rel] = {
                "sha256": digest,
                "source": source,
                "license": license_id,
            }
        if qrc:
            self.qrc[qrc].append(rel)

    def pair(self, rel: str, make, px: int, source: str, qrc: str = "free") -> None:
        """Writes the @1x and @2x files of one raster icon drawn for this project."""
        self.put(f"{rel}.png", encode(make(px)), source, FIRST_PARTY, qrc)
        self.put(f"{rel}@2x.png", encode(make(px * 2)), source, FIRST_PARTY, qrc)


def load_rows() -> dict[str, dict[str, Row]]:
    """Returns art-map.csv keyed by kind, then by key."""
    table: dict[str, dict[str, Row]] = {}
    with ART_MAP.open(encoding="utf-8", newline="") as handle:
        for line in csv.DictReader(handle):
            table.setdefault(line["kind"], {})[line["key"]] = Row(**line)
    return table


def load_icon_ids() -> dict[str, tuple[str, str]]:
    """Returns {icon id: (base, badge)} from the spec 0115 mapping."""
    ids: dict[str, tuple[str, str]] = {}
    with ICON_IDS.open(encoding="utf-8", newline="") as handle:
        for line in csv.DictReader(handle):
            for entry in filter(None, line["ids"].split(";")):
                icon, _, badge = entry.partition("+")
                ids[icon] = (line["base"], badge)
    return ids


def write_themed(writer: Writer, sources: Sources, bases: dict[str, Row]) -> list[str]:
    """Writes every icon of the Basic pack at every size and returns the paths needed."""
    ids = load_icon_ids()
    needed: list[str] = []
    for target in TARGETS.read_text(encoding="utf-8").split():
        category, tier, name = target.split("/")
        base, badge = ids[f"{category}/{name}"]
        row = bases[base]

        def make(px: int) -> QImage:
            image = art_image(sources, row, px)
            return with_badge(image, sources, badge, px) if badge else image

        writer.pair(f"{PACK}/{target}", make, int(tier), row.source, "basic")
        needed.append(f"{PACK}/{target}")

    manifest = {
        "id": "basic",
        "title": "Basic",
        "translations": {},
        "style": "drawn",
        "mono": False,
    }
    writer.put(
        f"{PACK}/pack.json",
        json.dumps(manifest, indent=2).encode("utf-8") + b"\n",
        "drawn",
        FIRST_PARTY,
        "basic",
    )
    return needed


def write_glyphs(writer: Writer, sources: Sources, bases: dict[str, Row]) -> None:
    """Writes the hidden mono pack the widget toolbars draw from by default (spec 0116 R9)."""
    writer.put(
        "iconpacks/fine-line/pack.json",
        json.dumps(
            {
                "id": "fine-line",
                "title": "Glyphs",
                "translations": {},
                "style": "tabler-outline",
                "mono": True,
                "hidden": True,
            },
            indent=2,
        ).encode("utf-8")
        + b"\n",
        "tabler",
        "MIT",
    )
    for icon, (base, _) in sorted(load_icon_ids().items()):
        category, name = icon.split("/")
        svg = sources.tabler_svg(bases[base].tabler, MONO_COLOR)
        writer.put(
            f"iconpacks/fine-line/{category}/48/{name}.svg", svg, "tabler", "MIT"
        )


def write_fixed(
    writer: Writer, sources: Sources, table: dict[str, dict[str, Row]]
) -> list[str]:
    """Writes the buttons, window controls, placeholder, images and action icons."""
    needed: list[str] = []
    for key, row in table["button"].items():
        if row.source == "drawn":
            svg = (DRAWN / f"{row.ref}.svg").read_bytes()
            writer.put(f"icons/buttons/{key}.svg", svg, "drawn", FIRST_PARTY, None)
            continue

        svg = sources.tabler_svg(row.ref, MONO_COLOR)
        writer.put(f"icons/buttons/{key}.svg", svg, "tabler", "MIT", None)
    for key, row in table["fixed"].items():
        svg = sources.tabler_svg(row.ref, MONO_COLOR)
        writer.put(f"icons/{key}.svg", svg, "tabler", "MIT")
        needed.append(f"icons/{key}")
    for key, row in table["image"].items():
        svg = sources.tabler_svg(row.ref, row.tint)
        writer.put(f"images/{key}.svg", svg, "tabler", "MIT")
        needed.append(f"images/{key}")
        if key == "shortcut":
            writer.put(
                "images/shortcut.ico", encode(render(svg, 256), "ICO"), "tabler", "MIT"
            )
            writer.put(
                "images/shortcut.icns",
                encode(render(svg, 512), "ICNS"),
                "tabler",
                "MIT",
            )
    for key, row in table["action"].items():

        def make(px: int, row: Row = row) -> QImage:
            return art_image(sources, row, px)

        writer.pair(f"actions/{key}", make, ACTION_PX, row.source)
        needed.append(f"actions/{key}")
    return needed


def write_kept(writer: Writer) -> None:
    """Lists the first-party artwork that stays as it is, copying it when generating elsewhere."""
    for rel in KEPT:
        writer.put(rel, (RCC / rel).read_bytes(), "first-party", FIRST_PARTY, None)


def write_qrc(out: Path, name: str, entries: list[str]) -> None:
    """Writes one generated resource list."""
    files = "\n".join(f"        <file>{escape(rel)}</file>" for rel in sorted(entries))
    text = (
        "<!--\n  SPDX-"
        "FileCopyrightText: 2026 Alex Spataru\n"
        "  SPDX-"
        f"License-Identifier: {FIRST_PARTY}\n\n"
        "  Generated by scripts/basic-art.py; never edit by hand.\n-->\n"
        f'<RCC>\n    <qresource prefix="/">\n{files}\n    </qresource>\n</RCC>\n'
    )
    (out / name).write_text(text, encoding="utf-8", newline="\n")


def write_indexes(writer: Writer, needed: list[str]) -> None:
    """Writes both resource lists and the art-provenance.json allowlist."""
    write_qrc(writer.out, "art-free.qrc", writer.qrc["free"])
    write_qrc(writer.out, "basic.qrc", writer.qrc["basic"])
    manifest = {
        "files": {rel: writer.files[rel] for rel in sorted(writer.files)},
        "targets": sorted(needed),
        "standins": [],
    }
    (writer.out / "art-provenance.json").write_text(
        json.dumps(manifest, indent=1) + "\n", encoding="utf-8", newline="\n"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--out", type=Path, default=RCC)
    parser.add_argument("--cache", type=Path, default=None)
    args = parser.parse_args()

    app = QGuiApplication(sys.argv)
    cache = args.cache or Path(tempfile.mkdtemp(prefix="basic-art-"))
    cache.mkdir(parents=True, exist_ok=True)
    args.out.mkdir(parents=True, exist_ok=True)

    table = load_rows()
    sources = Sources(cache)
    writer = Writer(args.out)
    needed = write_themed(writer, sources, table["base"])
    write_glyphs(writer, sources, table["base"])
    needed += write_fixed(writer, sources, table)
    write_kept(writer)
    write_indexes(writer, needed)
    print(f"basic-art: {len(writer.files)} files -> {args.out}")

    del app
    if args.cache is None:
        shutil.rmtree(cache, ignore_errors=True)
    sys.stdout.flush()
    os._exit(0)


if __name__ == "__main__":
    sys.exit(main())
