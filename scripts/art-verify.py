#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""The public-tree art gate (spec 0116): no art reaches this repository without a known origin.

Icons8's terms forbid posting its art in a downloadable form, so the repository keeps an allowlist
rather than a list of known offenders: every artwork file must appear in app/rcc/art-provenance.json
with a matching SHA-256 and a license that is safe in any build. Anything else fails, including art
nobody has seen before.

Checks, each reported as one error line:
  - an artwork file is missing from the manifest, or the manifest names a file that is gone
  - a file's hash differs from the manifest
  - a license is not public domain, MIT or first-party
  - an SVG carries the Icons8 export signature
  - a raster icon lacks its @1x/@2x partner or has the wrong pixel size
  - an icon or action name the application needs does not resolve

Usage: python3 scripts/art-verify.py [--require-no-standins]
Exit code 0 = clean, 1 = violations.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RCC = ROOT / "app" / "rcc"
MANIFEST_NAME = "art-provenance.json"

ART_FOLDERS = ("icons", "actions", "images", "iconpacks")
ART_SUFFIXES = {".svg", ".png", ".ico", ".icns"}
ALLOWED_LICENSES = {
    "MIT",
    "GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial",
}
ACTION_ICON_PX = 48

SIGNATURES = (
    re.compile(rb'id="surface\d+"'),
    re.compile(
        rb'<svg xmlns="http://www\.w3\.org/2000/svg"  viewBox="0 0 \d+ \d+" '
        rb'width="\d+px" height="\d+px"'
    ),
)


def png_size(data: bytes) -> tuple[int, int] | None:
    """Width and height from a PNG's IHDR chunk, or None when @p data is not a PNG."""
    if len(data) < 24 or not data.startswith(b"\x89PNG\r\n\x1a\n"):
        return None
    return struct.unpack(">II", data[16:24])


def art_files(rcc: Path) -> dict[str, Path]:
    """Every artwork file under the art folders, keyed by its path relative to @p rcc."""
    found: dict[str, Path] = {}
    for folder in ART_FOLDERS:
        base = rcc / folder
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if path.is_file() and path.suffix in ART_SUFFIXES:
                found[path.relative_to(rcc).as_posix()] = path
    return found


def expected_px(rel: str) -> int | None:
    """The @1x pixel size a raster file must have, or None when its path sets no size."""
    parts = rel.split("/")
    if parts[0] == "actions" and len(parts) == 2:
        return ACTION_ICON_PX
    if parts[0] == "icons" and len(parts) == 4 and parts[2].isdigit():
        return int(parts[2])
    if parts[:2] == ["iconpacks", "basic"] and len(parts) == 5 and parts[3].isdigit():
        return int(parts[3])
    return None


def check_raster(rel: str, path: Path, data: bytes, errors: list[str]) -> None:
    """Applies the pairing and pixel-size rules to one raster icon."""
    scale = 2 if path.stem.endswith("@2x") else 1
    base = path.stem[: -len("@2x")] if scale == 2 else path.stem
    partner = path.with_name(f"{base}.png" if scale == 2 else f"{base}@2x.png")
    px = expected_px(rel)
    if px is None:
        return
    if not partner.exists():
        errors.append(f"{rel}: no @1x/@2x partner")
    size = png_size(data)
    if size != (px * scale, px * scale):
        errors.append(f"{rel}: is {size}, expected {px * scale}x{px * scale}")


def check_file(rel: str, path: Path, entry: dict, errors: list[str]) -> None:
    """Applies the hash, license, signature and raster rules to one listed file."""
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != entry.get("sha256"):
        errors.append(f"{rel}: hash differs from the manifest")
    if entry.get("license") not in ALLOWED_LICENSES:
        errors.append(f"{rel}: license not allowed: {entry.get('license')}")
    if path.suffix == ".svg" and any(sig.search(data) for sig in SIGNATURES):
        errors.append(f"{rel}: carries the Icons8 export signature")
    if path.suffix == ".png":
        check_raster(rel, path, data, errors)


def resolves(rcc: Path, target: str) -> bool:
    """Whether a needed icon exists as PNG or SVG; @p target is its path without a suffix."""
    return (rcc / f"{target}.png").exists() or (rcc / f"{target}.svg").exists()


def check(rcc: Path) -> tuple[list[str], list[str]]:
    """Runs every rule over @p rcc and returns (errors, stand-ins)."""
    manifest_path = rcc / MANIFEST_NAME
    if not manifest_path.exists():
        return [f"missing {MANIFEST_NAME}"], []

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    listed = manifest.get("files", {})
    errors: list[str] = []
    found = art_files(rcc)
    for rel in sorted(set(found) - set(listed)):
        errors.append(f"{rel}: not in the manifest")
    for rel in sorted(set(listed) - set(found)):
        errors.append(f"{rel}: listed in the manifest but missing")
    for rel in sorted(set(found) & set(listed)):
        check_file(rel, found[rel], listed[rel], errors)
    for target in manifest.get("targets", []):
        if not resolves(rcc, target):
            errors.append(f"{target}: needed by the application but does not resolve")
    return errors, manifest.get("standins", [])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--require-no-standins", action="store_true")
    args = parser.parse_args()

    errors, standins = check(RCC)
    if args.require_no_standins and standins:
        errors.append(f"{len(standins)} stand-in icons remain: {', '.join(standins)}")
    for error in errors:
        print(f"error: {error}")
    print(f"art-verify: {len(errors)} error(s), {len(standins)} stand-in(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
