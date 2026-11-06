#!/usr/bin/env python3
"""Generate the two repeating grain tiles behind the brushed-metal theme material (spec 0113).

A brushed plate is a lit gradient with fine horizontal grain. The gradient and the edges
are composed in QML; the grain is two translucent tiles laid over each other, white and
black only, so they serve a plate of any colour:

- rows: every pixel row one faint tone across its whole width. Narrow and tall, so it has
  no horizontal period at all and repeats vertically only once per 1024 px.
- streaks: sparse strokes of varying length with soft ends, wrapped at the right edge so
  the tile is seamless.

The two tiles have unrelated sizes, so the pattern they make together does not repeat
within a window, which one tile alone visibly did. The spec's premise check is why the
grain is tiled at all: a plate painted at window size cost 358 ms and 56 MiB at 5120x2880.

Deterministic (fixed seed) and stdlib-only, so the checked-in PNGs can be regenerated
and compared byte for byte.

Usage:
    python3 scripts/generate-material-tiles.py [--check]

Exit codes: 0 written or up to date; 1 drift under --check.
"""

from __future__ import annotations

import argparse
import random
import struct
import sys
import zlib
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = REPO_ROOT / "app" / "rcc" / "images" / "materials"
SEED = 113
ROWS_SIZE = (8, 1024)
STREAKS_SIZE = (733, 389)
ROW_PROBABILITY = 0.7
STREAK_PROBABILITY = 0.25
ROW_LIGHT_ALPHA = (3, 10)
ROW_DARK_ALPHA = (2, 8)
STREAK_LIGHT_ALPHA = (6, 18)
STREAK_DARK_ALPHA = (4, 12)
STREAK_FADE = 28


def chunk(tag: bytes, payload: bytes) -> bytes:
    """One PNG chunk: length, tag, payload, CRC over tag and payload."""
    crc = zlib.crc32(tag + payload) & 0xFFFFFFFF
    return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", crc)


def encode_png(width: int, height: int, rows: list[bytearray]) -> bytes:
    """An 8-bit RGBA PNG from one bytearray of width * 4 bytes per row."""
    raw = b"".join(b"\x00" + bytes(row) for row in rows)
    header = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", header)
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


def tone(rng: random.Random, light_alpha: tuple, dark_alpha: tuple) -> tuple[int, int]:
    """A translucent white or black: (value, alpha)."""
    if rng.random() < 0.5:
        return 255, rng.randint(*light_alpha)

    return 0, rng.randint(*dark_alpha)


def row_tile(width: int, height: int, seed: int) -> list[bytearray]:
    """Rows that are empty or one faint tone from edge to edge."""
    rng = random.Random(seed)
    rows = []
    for _ in range(height):
        row = bytearray(width * 4)
        if rng.random() < ROW_PROBABILITY:
            value, alpha = tone(rng, ROW_LIGHT_ALPHA, ROW_DARK_ALPHA)
            row[:] = bytes((value, value, value, alpha)) * width

        rows.append(row)

    return rows


def streak_tile(width: int, height: int, fade: int, seed: int) -> list[bytearray]:
    """Rows that are empty or carry one wrapped stroke whose ends fade out."""
    rng = random.Random(seed)
    rows = []
    for _ in range(height):
        row = bytearray(width * 4)
        if rng.random() < STREAK_PROBABILITY:
            value, alpha = tone(rng, STREAK_LIGHT_ALPHA, STREAK_DARK_ALPHA)
            start = rng.randrange(width)
            length = int(width * (0.25 + 0.6 * rng.random()))
            for step in range(length):
                edge = min(step, length - 1 - step)
                faded = alpha * min(fade, edge + 1) // fade
                offset = ((start + step) % width) * 4
                row[offset : offset + 4] = bytes((value, value, value, faded))

        rows.append(row)

    return rows


def tiles() -> dict[Path, bytes]:
    """Every tile this script owns, by output path."""
    out = {}
    for scale, suffix in ((1, ""), (2, "@2x")):
        width, height = ROWS_SIZE[0] * scale, ROWS_SIZE[1] * scale
        rows = row_tile(width, height, SEED + scale)
        out[OUTPUT_DIR / f"brushed-rows{suffix}.png"] = encode_png(width, height, rows)

        width, height = STREAKS_SIZE[0] * scale, STREAKS_SIZE[1] * scale
        rows = streak_tile(width, height, STREAK_FADE * scale, SEED + 10 + scale)
        out[OUTPUT_DIR / f"brushed-streaks{suffix}.png"] = encode_png(
            width, height, rows
        )

    return out


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--check", action="store_true", help="fail on drift, write nothing"
    )
    args = parser.parse_args()

    drift = False
    for path, data in tiles().items():
        current = path.read_bytes() if path.is_file() else None
        if current == data:
            continue

        if args.check:
            print(
                f"generate-material-tiles: {path.relative_to(REPO_ROOT)} is out of date"
            )
            drift = True
            continue

        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        print(f"generate-material-tiles: wrote {path.relative_to(REPO_ROOT)}")

    return 1 if drift else 0


if __name__ == "__main__":
    sys.exit(main())
