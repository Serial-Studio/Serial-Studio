"""
Spec-0040 Mirror Bandwidth Measurement

Computes every number quoted in
`doc/claude/specs/0040-remote-dashboard/bandwidth.md` from the recorded
fixtures in this directory. Nothing here is estimated in prose: run it and the
document's tables are reproduced on stdout.

    python3 tests/fixtures/mirror/measure_bandwidth.py
    python3 tests/fixtures/mirror/measure_bandwidth.py --markdown

Method: re-encode each fixture's snapshots at a given value precision, measure
the encoded NDJSON line lengths (the exact bytes the socket carries, including
the newline), and multiply the mean by the mirror rate. The device rate does
not appear anywhere in the calculation, which is the R11 property stated as an
algebraic fact rather than an assertion.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import argparse
import json
import statistics
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
FIXTURE_DIR = Path(__file__).resolve().parent

sys.path.insert(0, str(REPO_ROOT / "tests" / "utils"))

from mirror_client import PUSH_KEY, encode_line, encode_snapshot  # noqa: E402

MAX_API_MESSAGE_BYTES = 1024 * 1024
MAX_API_BUFFER_BYTES = 4 * 1024 * 1024
MAX_API_BYTES_PER_WINDOW = 128 * 1024 * 1024

RATES_HZ = (1, 20, 60)
PRECISIONS = (0, 6, 4)


def read_fixture(path: Path) -> tuple[dict, list[dict]]:
    """Splits a recorded stream into its structure push and its snapshot pushes."""
    structure = {}
    snapshots = []

    for raw in path.read_bytes().split(b"\n"):
        if not raw.strip():
            continue

        obj = json.loads(raw.decode("utf-8"))
        payload = obj.get(PUSH_KEY, {})
        if payload.get("kind") == "structure":
            structure = obj
        elif payload.get("kind") == "snapshot":
            snapshots.append(obj)

    return structure, snapshots


def resolve_values(payload: dict) -> list:
    """Rebuilds the value list, folding the sparse maps back in."""
    values = list(payload.get("values", []))
    for key, text in (payload.get("strings") or {}).items():
        values[int(key)] = text
    for key, tag in (payload.get("nonFinite") or {}).items():
        values[int(key)] = float(tag) if tag != "nan" else float("nan")
    return values


def snapshot_bytes(snapshots: list[dict], precision: int) -> list[int]:
    """Re-encodes every snapshot at the given precision and returns line sizes."""
    sizes = []
    for obj in snapshots:
        payload = obj[PUSH_KEY]
        line = encode_line(
            encode_snapshot(
                epoch=payload["epoch"],
                seq=payload["seq"],
                t_ns=payload["tNs"],
                values=resolve_values(payload),
                precision=precision,
            )
        )
        sizes.append(len(line))

    return sizes


def measure(name: str) -> dict:
    """Measures one fixture across every rate and precision."""
    structure, snapshots = read_fixture(FIXTURE_DIR / f"{name}.ndjson")
    payload = structure[PUSH_KEY]
    dataset_count = len(payload["datasets"])
    structure_size = len(encode_line(structure))

    result = {
        "name": name,
        "datasetCount": dataset_count,
        "sourceIds": payload["sourceIds"],
        "structureBytes": structure_size,
        "structureBytesPerDataset": structure_size / dataset_count,
        "datasetsAtMessageCap": int(
            MAX_API_MESSAGE_BYTES / (structure_size / dataset_count)
        ),
        "precisions": {},
    }

    for precision in PRECISIONS:
        sizes = snapshot_bytes(snapshots, precision)
        mean = statistics.mean(sizes)
        result["precisions"][precision] = {
            "meanBytes": mean,
            "maxBytes": max(sizes),
            "bytesPerDataset": mean / dataset_count,
            "rates": {hz: mean * hz for hz in RATES_HZ},
        }

    return result


def scaling_sweep() -> dict:
    """
    Measures the *marginal* structure cost per dataset.

    A single project cannot answer "how many datasets fit in the 1 MB message
    cap", because its size is a fixed part (control script, widget settings,
    parser code) plus a per-dataset part. Replicating the widest example
    project's groups N times and measuring the real encoded size separates the
    two, so the cap is extrapolated from a slope rather than from an average.
    """
    project = json.loads(
        (REPO_ROOT / "examples/System Monitor/system-monitor.ssproj").read_text("utf-8")
    )
    base_groups = project.get("groups", [])
    points = []

    for factor in (1, 2, 4, 8, 16):
        scaled = dict(project)
        scaled["groups"] = [
            dict(group, uniqueId=group.get("uniqueId", 0) + 100000 * copy)
            for copy in range(factor)
            for group in base_groups
        ]

        dataset_count = sum(len(g.get("datasets", [])) for g in scaled["groups"])
        ordered = [[0, i] for i in range(dataset_count)]
        line = encode_line(
            {
                PUSH_KEY: {
                    "kind": "structure",
                    "wireVersion": 1,
                    "epoch": 1,
                    "layoutHash": "0" * 16,
                    "sourceIds": [0],
                    "datasets": ordered,
                    "operationMode": 0,
                    "plotTimeRange": 10.0,
                    "frozen": False,
                    "clock": {"domain": "monotonic-relative", "originUnixMs": 0},
                    "project": scaled,
                }
            }
        )
        points.append((dataset_count, len(line)))

    (n0, b0), (n1, b1) = points[0], points[-1]
    slope = (b1 - b0) / (n1 - n0)
    intercept = b0 - slope * n0

    return {
        "points": points,
        "bytesPerDataset": slope,
        "fixedBytes": intercept,
        "datasetsAtMessageCap": int((MAX_API_MESSAGE_BYTES - intercept) / slope),
        "datasetsAtBufferCap": int((MAX_API_BUFFER_BYTES - intercept) / slope),
    }


def human(value: float) -> str:
    """Formats a byte-rate in the unit a reader can compare against the caps."""
    if value >= 1024 * 1024:
        return f"{value / (1024 * 1024):.2f} MB/s"
    if value >= 1024:
        return f"{value / 1024:.1f} KB/s"
    return f"{value:.0f} B/s"


def report(results: list[dict], markdown: bool) -> None:
    bullet = "| " if markdown else "  "

    for entry in results:
        print(
            f"\n### {entry['name']} -- {entry['datasetCount']} datasets, "
            f"sources {entry['sourceIds']}"
        )
        print(
            f"structure message: {entry['structureBytes']} B "
            f"({entry['structureBytesPerDataset']:.0f} B/dataset); "
            f"1 MB message cap reached at ~{entry['datasetsAtMessageCap']} datasets"
        )

        if markdown:
            print("\n| precision | B/dataset | mean snapshot | 1 Hz | 20 Hz | 60 Hz |")
            print("|-----------|-----------|---------------|------|-------|-------|")

        for precision, stats in entry["precisions"].items():
            label = "full" if precision == 0 else f"{precision} sig"
            rates = stats["rates"]
            row = (
                f"{bullet}{label:9s} | {stats['bytesPerDataset']:9.1f} | "
                f"{stats['meanBytes']:13.0f} | {human(rates[1]):>8s} | "
                f"{human(rates[20]):>9s} | {human(rates[60]):>9s} |"
            )
            print(row)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--markdown", action="store_true", help="emit markdown tables")
    parser.add_argument("--json", action="store_true", help="emit the raw measurements")
    args = parser.parse_args()

    results = [measure(name) for name in ("small", "wide", "multisource")]
    sweep = scaling_sweep()

    if args.json:
        print(json.dumps({"fixtures": results, "sweep": sweep}, indent=2))
        return 0

    report(results, args.markdown)

    print("\n### Structure scaling sweep (System Monitor groups replicated)")
    if args.markdown:
        print("\n| datasets | structure bytes |")
        print("|----------|-----------------|")
    for count, size in sweep["points"]:
        print(f"| {count:8d} | {size:15d} |")
    print(
        f"\nmarginal cost {sweep['bytesPerDataset']:.0f} B/dataset, "
        f"fixed {sweep['fixedBytes']:.0f} B; "
        f"1 MB message cap at ~{sweep['datasetsAtMessageCap']} datasets, "
        f"4 MB buffer cap at ~{sweep['datasetsAtBufferCap']}"
    )

    print("\nServer caps: message 1 MB, buffer 4 MB, byte rate 128 MB/s per client.")
    worst = max(entry["precisions"][0]["rates"][60] for entry in results)
    print(
        f"Worst measured mirror rate across fixtures: {human(worst)} "
        f"= {worst / MAX_API_BYTES_PER_WINDOW * 100:.4f}% of the byte-rate cap."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
