#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""Generates the free icon tree that public builds compile (spec 0116).

Tango (public domain) supplies the colored icons and Tabler (MIT) the single-color glyphs and the
interim stand-ins. Every decision lives in doc/claude/specs/0116-tango-public-icons/art-map.csv,
so re-running this script reproduces the tree. Output is committed: nobody needs PySide6 to build.

For each colored icon it writes <name>.png and <name>@2x.png at every size the application offers,
using Tango's hand-drawn PNG when one exists at that exact pixel size and the icon has no badge.
It also writes art-free.qrc and the art-provenance.json allowlist that scripts/art-verify.py reads.

Usage: python3 scripts/tango-art.py [--out DIR] [--cache DIR]
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
from PySide6.QtGui import QGuiApplication, QImage, QPainter  # noqa: E402
from PySide6.QtSvg import QSvgRenderer  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
RCC = ROOT / "app" / "rcc"
SPEC = ROOT / "doc" / "claude" / "specs" / "0116-tango-public-icons"
ART_MAP = SPEC / "art-map.csv"
TARGETS = SPEC / "art-targets.txt"
ICON_IDS = ROOT / "doc" / "claude" / "specs" / "0115-icon-packs" / "icons8-map.csv"
DRAWN = ROOT / "art" / "drawn"

TANGO_URL = "https://tango.freedesktop.org/releases/tango-icon-theme-0.8.90.tar.gz"
TANGO_SHA256 = "6e98d8032d57d818acc907ec47e6a718851ff251ae7c29aafb868743eb65c88e"
TABLER_URL = "https://registry.npmjs.org/@tabler/icons/-/icons-3.51.0.tgz"
TABLER_SHA256 = "b30108713d3fa60aee862c2a133225da8e0f309be0708fe2925763ff810c766c"

PUBLIC_DOMAIN = "LicenseRef-PublicDomain"
FIRST_PARTY = "GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial"
MONO_COLOR = "#000000"
ART_SUFFIXES = (".svg", ".png", ".ico", ".icns")
ACTION_PX = 48
HAND_DRAWN_PX = (16, 32)
BADGES = {
    "plus": "list-add",
    "x": "process-stop",
    "no": "process-stop",
    "arrow": "go-next",
    "arrow-up": "go-up",
    "warning": "dialog-warning",
    "lock": "system-lock-screen",
    "pencil": "accessories-text-editor",
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
ART_NOTICE = """## Tango Icons

The colored interface icons in this build come from the Tango Icon Library, whose authors released
it into the public domain, or were drawn for Serial Studio in the same style.

Source: [https://tango.freedesktop.org](https://tango.freedesktop.org)
"""


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
        sys.exit(f"tango-art: {target.name} hash mismatch: {actual}")
    return target


class Sources:
    """Reads Tango and Tabler artwork out of their release archives."""

    def __init__(self, cache: Path) -> None:
        self.tango = tarfile.open(fetch(TANGO_URL, TANGO_SHA256, cache))
        self.tabler = tarfile.open(fetch(TABLER_URL, TABLER_SHA256, cache))
        self.tango_files = {}
        for member in self.tango.getmembers():
            parts = member.name.split("/")
            if len(parts) == 4 and member.isfile():
                self.tango_files[
                    parts[1], Path(parts[3]).stem, parts[3].rsplit(".")[-1]
                ] = member

    def tango_svg(self, name: str) -> bytes:
        return self.tango.extractfile(self.tango_files["scalable", name, "svg"]).read()

    def tango_png(self, name: str, px: int) -> bytes | None:
        member = self.tango_files.get((f"{px}x{px}", name, "png"))
        return self.tango.extractfile(member).read() if member else None

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
        sys.exit(f"tango-art: cannot encode {fmt}")
    return bytes(array)


def with_badge(image: QImage, sources: Sources, badge: str, px: int) -> QImage:
    """Draws the Tango icon for @p badge in the bottom-right corner of @p image."""
    size = max(8, round(px * 0.58))
    painter = QPainter(image)
    painter.drawImage(
        px - size, px - size, render(sources.tango_svg(BADGES[badge]), size)
    )
    painter.end()
    return image


def art_image(sources: Sources, row: Row, px: int, composed: bool) -> QImage:
    """Returns the artwork of @p row at @p px; a composed icon never uses a hand-drawn PNG."""
    name, _, badge = row.ref.partition("+")
    if row.source == "drawn":
        image = render((DRAWN / f"{name}.svg").read_bytes(), px)
    elif row.source == "tabler":
        image = render(sources.tabler_svg(name, row.tint), px)
    else:
        hand = None
        if not composed and not badge and px in HAND_DRAWN_PX:
            hand = sources.tango_png(name, px)
        image = QImage.fromData(hand) if hand else render(sources.tango_svg(name), px)
        image = image.convertToFormat(QImage.Format_ARGB32_Premultiplied)
    return with_badge(image, sources, badge, px) if badge else image


def license_of(row: Row, badge: str | None = None) -> str:
    """Returns the SPDX id of @p row's art; a badge is always Tango, so it changes nothing."""
    del badge
    return {"tango": PUBLIC_DOMAIN, "tabler": "MIT", "drawn": FIRST_PARTY}[row.source]


class Writer:
    """Writes the tree under @p out and records what each file is."""

    def __init__(self, out: Path) -> None:
        self.out = out
        self.files: dict[str, dict[str, str]] = {}
        self.qrc: list[str] = []

    def put(
        self, rel: str, data: bytes, source: str, license_id: str, qrc: bool = True
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
            self.qrc.append(rel)

    def pair(self, rel: str, make, px: int, source: str, license_id: str) -> None:
        """Writes the @1x and @2x files of one raster icon."""
        self.put(f"{rel}.png", encode(make(px)), source, license_id)
        self.put(f"{rel}@2x.png", encode(make(px * 2)), source, license_id)


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
    """Writes every colored icon at every size and returns the paths the application needs."""
    ids = load_icon_ids()
    needed: list[str] = []
    for target in TARGETS.read_text(encoding="utf-8").split():
        category, tier, name = target.split("/")
        base, badge = ids[f"{category}/{name}"]
        row = bases[base]

        def make(px: int) -> QImage:
            image = art_image(sources, row, px, bool(badge))
            return with_badge(image, sources, badge, px) if badge else image

        writer.pair(f"icons/{target}", make, int(tier), row.source, license_of(row))
        needed.append(f"icons/{target}")
    return needed


def write_glyphs(writer: Writer, sources: Sources, bases: dict[str, Row]) -> None:
    """Writes the hidden mono pack the widget toolbars draw from (spec 0116 R9)."""
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
        svg = sources.tabler_svg(row.ref, MONO_COLOR)
        writer.put(f"icons/buttons/{key}.svg", svg, "tabler", "MIT", qrc=False)
    for key, row in table["fixed"].items():
        writer.put(
            f"icons/{key}.svg", sources.tabler_svg(row.ref, MONO_COLOR), "tabler", "MIT"
        )
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
        license_id = license_of(row)

        def make(px: int, row: Row = row) -> QImage:
            return art_image(sources, row, px, False)

        writer.pair(f"actions/{key}", make, ACTION_PX, row.source, license_id)
        needed.append(f"actions/{key}")
    return needed


def write_kept(writer: Writer) -> None:
    """Lists the first-party artwork that stays as it is, copying it when generating elsewhere."""
    for rel in KEPT:
        writer.put(rel, (RCC / rel).read_bytes(), "first-party", FIRST_PARTY, qrc=False)


def write_indexes(writer: Writer, needed: list[str], standins: list[str]) -> None:
    """Writes art-free.qrc and the art-provenance.json allowlist."""
    entries = "\n".join(
        f"        <file>{escape(rel)}</file>" for rel in sorted(writer.qrc)
    )
    qrc = (
        "<!--\n  SPDX-"
        "FileCopyrightText: 2026 Alex Spataru\n"
        "  SPDX-"
        f"License-Identifier: {FIRST_PARTY}\n\n"
        "  Generated by scripts/tango-art.py; never edit by hand.\n-->\n"
        f'<RCC>\n    <qresource prefix="/">\n{entries}\n    </qresource>\n</RCC>\n'
    )
    (writer.out / "art-free.qrc").write_text(qrc, encoding="utf-8", newline="\n")
    manifest = {
        "files": {rel: writer.files[rel] for rel in sorted(writer.files)},
        "targets": sorted(needed),
        "standins": sorted(standins),
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
    cache = args.cache or Path(tempfile.mkdtemp(prefix="tango-art-"))
    cache.mkdir(parents=True, exist_ok=True)
    args.out.mkdir(parents=True, exist_ok=True)

    table = load_rows()
    sources = Sources(cache)
    writer = Writer(args.out)
    needed = write_themed(writer, sources, table["base"])
    write_glyphs(writer, sources, table["base"])
    needed += write_fixed(writer, sources, table)
    write_kept(writer)

    writer.put(
        "messages/Acknowledgements-Art.txt",
        ART_NOTICE.encode("utf-8"),
        "first-party",
        FIRST_PARTY,
    )
    standins = [key for key, row in table["base"].items() if row.status == "stand-in"]
    write_indexes(writer, needed, standins)
    print(
        f"tango-art: {len(writer.files)} files, {len(standins)} stand-ins -> {args.out}"
    )

    del app
    if args.cache is None:
        shutil.rmtree(cache, ignore_errors=True)
    sys.stdout.flush()
    os._exit(0)


if __name__ == "__main__":
    sys.exit(main())
