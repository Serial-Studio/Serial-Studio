"""
Spec-0040 Mirror Fixture Generator

Builds the recorded NDJSON push streams in this directory from the checked-in
example projects in `examples/`. Encoding goes through
`tests/utils/mirror_client.py`, so the fixtures and the client can never drift
apart: one codec, used by both sides.

    python3 tests/fixtures/mirror/generate_fixtures.py          # rewrite fixtures
    python3 tests/fixtures/mirror/generate_fixtures.py --check  # fail on drift

Values are synthetic but shaped like telemetry: a deterministic PRNG walks each
dataset inside its configured widget range, so the byte sizes the bandwidth
measurement reports are the sizes a real capture would produce.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import argparse
import json
import math
import random
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
FIXTURE_DIR = Path(__file__).resolve().parent

sys.path.insert(0, str(REPO_ROOT / "tests" / "utils"))

from mirror_client import (  # noqa: E402
    MIRROR_WIRE_VERSION,
    encode_heartbeat,
    encode_line,
    encode_snapshot,
    encode_structure,
    encode_structure_chunks,
    layout_hash,
)

# Fixture name -> (project path, snapshot count, mirror Hz).
FIXTURES = {
    "small": ("examples/LorenzAttractor/LorenzAttractor.ssproj", 60, 20),
    "wide": ("examples/System Monitor/system-monitor.ssproj", 60, 20),
    "multisource": (
        "examples/Dual Drone Telemetry/Dual Drone Telemetry.ssproj",
        60,
        20,
    ),
}

ORIGIN_UNIX_MS = 1753459200000
EPOCH_START = 1
SEED = 40


def load_project(relative: str) -> dict:
    """Reads one example project document."""
    path = REPO_ROOT / relative
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def ordered_datasets(project: dict) -> tuple[list[tuple[int, int]], list[int]]:
    """
    Walks the project in the publisher's order: ascending sourceId, then group
    order within the source, then dataset order within the group.

    This mirrors `UI::Dashboard::buildValuePushes()`, which iterates the
    QMap<int, Frame> `m_sourceRawFrames` (ascending source id) and then walks
    `frame.groups` and `group.datasets` in declaration order. A group with no
    `sourceId` key belongs to source 0 (Frame.h only serializes it when != 0).
    """
    by_source: dict[int, list[tuple[int, int]]] = {}

    for group_index, group in enumerate(project.get("groups", [])):
        source_id = int(group.get("sourceId", 0))
        bucket = by_source.setdefault(source_id, [])
        for dataset_index, dataset in enumerate(group.get("datasets", [])):
            unique_id = int(dataset.get("uniqueId", -1))
            if unique_id < 0:
                unique_id = source_id * 1000000 + group_index * 10000 + dataset_index
            bucket.append((source_id, unique_id))

    source_ids = sorted(by_source)
    ordered: list[tuple[int, int]] = []
    for source_id in source_ids:
        ordered.extend(by_source[source_id])

    return ordered, source_ids


def dataset_records(project: dict) -> list[dict]:
    """Flat dataset list in the same order as `ordered_datasets`."""
    by_source: dict[int, list[dict]] = {}
    for group in project.get("groups", []):
        source_id = int(group.get("sourceId", 0))
        by_source.setdefault(source_id, []).extend(group.get("datasets", []))

    records: list[dict] = []
    for source_id in sorted(by_source):
        records.extend(by_source[source_id])

    return records


class ValueWalker:
    """Deterministic random walk inside each dataset's configured range."""

    def __init__(self, records: list[dict], seed: int):
        self.rng = random.Random(seed)
        self.records = records
        self.state: list[float] = []

        for record in records:
            low = float(record.get("widgetMin", record.get("plotMin", 0)) or 0)
            high = float(record.get("widgetMax", record.get("plotMax", 100)) or 100)
            if high <= low:
                low, high = 0.0, 100.0
            self.state.append(low + (high - low) * self.rng.random())

    def step(self) -> list:
        values: list = []
        for index, record in enumerate(self.records):
            low = float(record.get("widgetMin", record.get("plotMin", 0)) or 0)
            high = float(record.get("widgetMax", record.get("plotMax", 100)) or 100)
            if high <= low:
                low, high = 0.0, 100.0

            span = high - low
            nxt = self.state[index] + self.rng.uniform(-span / 50.0, span / 50.0)
            self.state[index] = min(high, max(low, nxt))
            values.append(self.state[index])

        return values


def build_stream(project: dict, snapshot_count: int, hz: int) -> list[bytes]:
    """Builds one structure push followed by `snapshot_count` snapshot pushes."""
    ordered, source_ids = ordered_datasets(project)
    records = dataset_records(project)
    walker = ValueWalker(records, SEED)

    lines = [
        encode_line(
            encode_structure(
                epoch=EPOCH_START,
                dataset_ids=ordered,
                project=project,
                source_ids=source_ids,
                operation_mode=0,
                plot_time_range=float(project.get("plotTimeRange", 10)),
                frozen=bool(project.get("frozen", False)),
                origin_unix_ms=ORIGIN_UNIX_MS,
            )
        )
    ]

    period_ns = int(1e9 / hz)
    for index in range(snapshot_count):
        values = walker.step()

        # One non-numeric and one non-finite reading, so both sparse maps are
        # exercised by every fixture rather than only by the edge cases.
        if len(values) > 2 and index % 10 == 0:
            values[1] = "OK"
        if len(values) > 3 and index % 17 == 0:
            values[2] = math.nan

        t_ns = [(index + 1) * period_ns for _ in source_ids]
        lines.append(
            encode_line(
                encode_snapshot(
                    epoch=EPOCH_START,
                    seq=index + 1,
                    t_ns=t_ns,
                    values=values,
                )
            )
        )

    return lines


def build_edge_streams(project: dict) -> dict[str, list[bytes]]:
    """Builds the negative-path fixtures the conformance tests assert on."""
    ordered, source_ids = ordered_datasets(project)
    records = dataset_records(project)
    walker = ValueWalker(records, SEED)

    def structure(epoch: int) -> dict:
        return encode_structure(
            epoch=epoch,
            dataset_ids=ordered,
            project=project,
            source_ids=source_ids,
            origin_unix_ms=ORIGIN_UNIX_MS,
        )

    def snapshot(epoch: int, seq: int) -> dict:
        return encode_snapshot(
            epoch=epoch,
            seq=seq,
            t_ns=[seq * 50000000 for _ in source_ids],
            values=walker.step(),
        )

    streams: dict[str, list[bytes]] = {}

    # A structure change the viewer missed: snapshots for epoch 2 arrive while
    # the viewer still holds epoch 1, then the epoch-2 structure lands.
    streams["epoch-mismatch"] = [
        encode_line(structure(1)),
        encode_line(snapshot(1, 1)),
        encode_line(snapshot(2, 2)),
        encode_line(snapshot(2, 3)),
        encode_line(structure(2)),
        encode_line(snapshot(2, 4)),
    ]

    # A structure whose announced layout hash does not match its dataset list.
    bad_hash = structure(1)
    bad_hash["mirror"]["layoutHash"] = "0" * 16
    streams["hash-mismatch"] = [encode_line(bad_hash)]

    # A future wire version must be reported, never half-decoded.
    bad_version = structure(1)
    bad_version["mirror"]["wireVersion"] = MIRROR_WIRE_VERSION + 1
    streams["version-mismatch"] = [encode_line(bad_version)]

    # Forward compatibility: unknown fields at every level, an unknown message
    # kind, and an unrelated top-level push key must all be tolerated.
    forward = structure(1)
    forward["mirror"]["futureField"] = {"anything": [1, 2, 3]}
    forward["someOtherPushKey"] = True
    forward_snapshot = snapshot(1, 1)
    forward_snapshot["mirror"]["futureField"] = "ignored"
    streams["forward-compat"] = [
        encode_line(forward),
        encode_line({"mirror": {"kind": "futureKind", "epoch": 1}}),
        encode_line({"frames": [{"data": {}}]}),
        encode_line({"event": "device.connected"}),
        encode_line(forward_snapshot),
        encode_line(encode_heartbeat(epoch=1, seq=2)),
    ]

    # A snapshot whose value count disagrees with the held structure.
    short = snapshot(1, 1)
    short["mirror"]["values"] = short["mirror"]["values"][:-1]
    streams["length-mismatch"] = [encode_line(structure(1)), encode_line(short)]

    # Chunked structure delivery, forced with a small part size so the path is
    # exercised without needing a 1 MB project. An out-of-order part is included
    # because reassembly must be index-keyed, not arrival-ordered.
    chunks = encode_structure_chunks(structure(1), chunk_bytes=2048)
    reordered = [chunks[0]] + list(reversed(chunks[1:]))
    streams["structure-chunked"] = [encode_line(c) for c in reordered] + [
        encode_line(snapshot(1, 1))
    ]

    return streams


def write(path: Path, lines: list[bytes], check: bool) -> bool:
    """Writes or verifies one fixture; returns True when it is up to date."""
    payload = b"".join(lines)

    if check:
        if not path.exists():
            print(f"MISSING {path.relative_to(REPO_ROOT)}")
            return False
        if path.read_bytes() != payload:
            print(f"DRIFT   {path.relative_to(REPO_ROOT)}")
            return False
        return True

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(payload)
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check", action="store_true", help="verify fixtures instead of writing them"
    )
    args = parser.parse_args()

    ok = True
    manifest: dict[str, dict] = {}

    for name, (relative, count, hz) in FIXTURES.items():
        project = load_project(relative)
        ordered, source_ids = ordered_datasets(project)
        lines = build_stream(project, count, hz)
        ok &= write(FIXTURE_DIR / f"{name}.ndjson", lines, args.check)

        manifest[name] = {
            "project": relative,
            "title": project.get("title", ""),
            "groups": len(project.get("groups", [])),
            "datasetCount": len(ordered),
            "sourceIds": source_ids,
            "layoutHash": layout_hash(ordered),
            "snapshots": count,
            "hz": hz,
            "structureBytes": len(lines[0]),
            "snapshotBytesMean": round(
                sum(len(ln) for ln in lines[1:]) / max(1, len(lines) - 1), 1
            ),
        }

    edge_project = load_project(FIXTURES["small"][0])
    for name, lines in build_edge_streams(edge_project).items():
        ok &= write(FIXTURE_DIR / "edge" / f"{name}.ndjson", lines, args.check)

    manifest_bytes = (
        json.dumps(manifest, indent=2, sort_keys=True).encode("utf-8") + b"\n"
    )
    ok &= write(FIXTURE_DIR / "manifest.json", [manifest_bytes], args.check)

    if args.check:
        print(
            "fixtures up to date" if ok else "fixtures are stale: rerun without --check"
        )
        return 0 if ok else 1

    for name, entry in manifest.items():
        print(
            f"{name:12s} {entry['datasetCount']:4d} datasets  "
            f"sources={entry['sourceIds']}  "
            f"structure={entry['structureBytes']}B  "
            f"snapshot~{entry['snapshotBytesMean']}B"
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
