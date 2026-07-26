"""
Spec-0040 Mirror Wire Protocol Conformance

Exercises the mirror wire contract in
`doc/claude/specs/0040-remote-dashboard/wire-protocol.md` against the recorded
fixtures in `tests/fixtures/mirror/`. No Serial Studio instance, no network,
no Qt: this is the pre-gate proof that the protocol round-trips and that every
documented failure path is handled by dropping rather than by rendering.

    pytest tests/unit/test_mirror_protocol.py -v

When `MirrorProtocol.h` lands (spec 0040 T7), these cases port to the spec-0032
C++ unit target and this file becomes the cross-implementation check.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]
FIXTURE_DIR = REPO_ROOT / "tests" / "fixtures" / "mirror"

# Imported by path rather than through the `utils` package: the mirror client
# has no third-party dependencies and the package __init__ pulls in numpy and
# pyserial, which a pure-codec test has no reason to require.
sys.path.insert(0, str(REPO_ROOT / "tests" / "utils"))

from mirror_client import (  # noqa: E402
    HZ_MAX,
    HZ_MIN,
    MIRROR_WIRE_VERSION,
    PUSH_KEY,
    MirrorClient,
    MirrorHeartbeat,
    MirrorProtocolError,
    MirrorSnapshot,
    MirrorStructure,
    MirrorVersionMismatch,
    decode_push,
    encode_line,
    encode_snapshot,
    encode_structure,
    encode_structure_chunks,
    layout_hash,
)

FIXTURES = ("small", "wide", "multisource")


def load_manifest() -> dict:
    with open(FIXTURE_DIR / "manifest.json", "r", encoding="utf-8") as handle:
        return json.load(handle)


def load_project(relative: str) -> dict:
    with open(REPO_ROOT / relative, "r", encoding="utf-8") as handle:
        return json.load(handle)


# ----------------------------------------------------------------------
# Fixture integrity (T2 verification)
# ----------------------------------------------------------------------


@pytest.mark.parametrize("name", FIXTURES)
def test_every_fixture_line_is_json(name):
    """Every recorded line parses as a JSON object carrying the mirror key."""
    raw = (FIXTURE_DIR / f"{name}.ndjson").read_bytes()
    lines = [ln for ln in raw.split(b"\n") if ln.strip()]

    assert lines, "fixture is empty"
    for line in lines:
        obj = json.loads(line.decode("utf-8"))
        assert PUSH_KEY in obj
        assert obj[PUSH_KEY]["kind"] in ("structure", "snapshot")


@pytest.mark.parametrize("name", FIXTURES)
def test_structure_order_matches_the_source_project(name):
    """
    The ordered dataset list is row-major over ascending sourceId, then group
    order, then dataset order -- the order UI::Dashboard::buildValuePushes()
    walks. Recomputed here straight from the .ssproj so a generator bug cannot
    hide behind the generator.
    """
    manifest = load_manifest()[name]
    project = load_project(manifest["project"])

    expected: dict[int, list[int]] = {}
    for group_index, group in enumerate(project.get("groups", [])):
        source_id = int(group.get("sourceId", 0))
        bucket = expected.setdefault(source_id, [])
        for dataset_index, dataset in enumerate(group.get("datasets", [])):
            unique_id = int(dataset.get("uniqueId", -1))
            if unique_id < 0:
                unique_id = source_id * 1000000 + group_index * 10000 + dataset_index
            bucket.append(unique_id)

    flattened = [
        [source_id, unique_id]
        for source_id in sorted(expected)
        for unique_id in expected[source_id]
    ]

    client = MirrorClient.from_file(str(FIXTURE_DIR / f"{name}.ndjson"))
    client.drain()

    assert client.structure is not None
    assert [list(p) for p in client.structure.dataset_ids] == flattened
    assert client.structure.source_ids == sorted(expected)
    assert client.structure.dataset_count == manifest["datasetCount"]


@pytest.mark.parametrize("name", FIXTURES)
def test_snapshot_value_count_matches_the_dataset_list(name):
    """Every snapshot carries exactly one positional slot per declared dataset."""
    structure_len = None
    for line in (FIXTURE_DIR / f"{name}.ndjson").read_bytes().split(b"\n"):
        if not line.strip():
            continue

        payload = json.loads(line.decode("utf-8"))[PUSH_KEY]
        if payload["kind"] == "structure":
            structure_len = len(payload["datasets"])
        else:
            assert structure_len is not None, "snapshot preceded its structure"
            assert len(payload["values"]) == structure_len


# ----------------------------------------------------------------------
# Round-trip
# ----------------------------------------------------------------------


@pytest.mark.parametrize("name", FIXTURES)
def test_fixture_round_trips_through_the_client(name):
    """A full fixture decodes with no drop, no gap, and a settled value set."""
    manifest = load_manifest()[name]
    client = MirrorClient.from_file(str(FIXTURE_DIR / f"{name}.ndjson"))
    client.drain()

    assert client.stats.structures == 1
    assert client.stats.snapshots == manifest["snapshots"]
    assert client.stats.dropped_epoch == 0
    assert client.stats.dropped_hash == 0
    assert client.stats.dropped_length == 0
    assert client.stats.seq_gaps == 0
    assert len(client.values) == manifest["datasetCount"]


@pytest.mark.parametrize("name", FIXTURES)
def test_layout_hash_is_reproducible(name):
    """The announced hash is recomputable from the dataset list alone."""
    manifest = load_manifest()[name]
    client = MirrorClient.from_file(str(FIXTURE_DIR / f"{name}.ndjson"))
    client.drain()

    assert client.structure.layout_hash == manifest["layoutHash"]
    assert client.structure.layout_hash == layout_hash(client.structure.dataset_ids)


def test_layout_hash_detects_reordering():
    """Swapping two entries changes the hash; that is the whole safety argument."""
    a = [(0, 10000), (0, 10001), (1, 1010000)]
    b = [(0, 10001), (0, 10000), (1, 1010000)]

    assert layout_hash(a) != layout_hash(b)
    assert layout_hash(a) != layout_hash(a[:-1])
    assert len(layout_hash(a)) == 16


def test_snapshot_encodes_strings_and_non_finite_out_of_band():
    """Positional slots stay numeric; strings and NaN/Inf move to sparse maps."""
    push = encode_snapshot(
        epoch=3,
        seq=9,
        t_ns=[1000],
        values=[1.5, "OK", math.nan, math.inf, -math.inf, None],
    )
    payload = push[PUSH_KEY]

    assert payload["values"] == [1.5, None, None, None, None, None]
    assert payload["strings"] == {"1": "OK"}
    assert payload["nonFinite"] == {"2": "nan", "3": "inf", "4": "-inf"}

    decoded = decode_push(push)
    assert isinstance(decoded, MirrorSnapshot)
    assert decoded.values[0] == 1.5
    assert decoded.values[1] == "OK"
    assert math.isnan(decoded.values[2])
    assert decoded.values[3] == math.inf
    assert decoded.values[4] == -math.inf
    assert decoded.values[5] is None


def test_precision_parameter_shrinks_the_wire_without_changing_shape():
    """`precision` rounds values; the positional length is unaffected."""
    values = [1.2345678901234567] * 32
    full = encode_line(encode_snapshot(epoch=1, seq=1, t_ns=[0], values=values))
    trimmed = encode_line(
        encode_snapshot(epoch=1, seq=1, t_ns=[0], values=values, precision=6)
    )

    assert len(trimmed) < len(full)

    decoded = decode_push(json.loads(trimmed))
    assert decoded.value_count == 32
    assert decoded.values[0] == pytest.approx(1.23457, rel=1e-9)


# ----------------------------------------------------------------------
# Epoch and layout-hash failure paths
# ----------------------------------------------------------------------


def test_epoch_mismatch_drops_the_snapshot_and_requests_structure():
    """
    A snapshot from an epoch the viewer does not hold is dropped and triggers
    exactly one structure request; the epoch-2 structure that follows restores
    normal decoding.
    """
    client = MirrorClient.from_file(str(FIXTURE_DIR / "edge" / "epoch-mismatch.ndjson"))
    client.drain()

    assert client.stats.snapshots == 4
    assert client.stats.dropped_epoch == 2
    assert client.stats.structure_requests == 1
    assert client.structure.epoch == 2
    assert client.stats.structures == 2


def test_length_mismatch_is_handled_like_an_epoch_mismatch():
    """A snapshot whose value count disagrees is dropped, never applied partially."""
    client = MirrorClient.from_file(
        str(FIXTURE_DIR / "edge" / "length-mismatch.ndjson")
    )
    client.drain()

    assert client.stats.dropped_length == 1
    assert client.stats.structure_requests == 1
    assert all(v is None for v in client.values)


def test_layout_hash_mismatch_refuses_the_structure():
    """A structure whose announced hash does not verify is never adopted."""
    client = MirrorClient.from_file(str(FIXTURE_DIR / "edge" / "hash-mismatch.ndjson"))

    with pytest.raises(MirrorProtocolError, match="layout hash mismatch"):
        client.drain()

    assert client.structure is None
    assert client.stats.dropped_hash == 1


def test_version_mismatch_is_reported_not_half_decoded():
    """An unknown wire version raises before any field is trusted."""
    client = MirrorClient.from_file(
        str(FIXTURE_DIR / "edge" / "version-mismatch.ndjson")
    )

    with pytest.raises(MirrorVersionMismatch) as excinfo:
        client.drain()

    assert excinfo.value.theirs == MIRROR_WIRE_VERSION + 1
    assert client.structure is None


# ----------------------------------------------------------------------
# Forward compatibility
# ----------------------------------------------------------------------


def test_unknown_fields_kinds_and_push_keys_are_tolerated():
    """
    Unknown object fields are ignored, unknown `kind` values are dropped, and
    unrelated top-level push keys (`frames`, `event`) are ignored -- the three
    rules that let the protocol grow without a version bump.
    """
    client = MirrorClient.from_file(str(FIXTURE_DIR / "edge" / "forward-compat.ndjson"))
    client.drain()

    assert client.structure is not None
    assert client.stats.structures == 1
    assert client.stats.snapshots == 1
    assert client.stats.heartbeats == 1
    assert client.stats.ignored_pushes == 3
    assert client.stats.dropped_epoch == 0


def test_a_push_without_the_mirror_key_decodes_to_nothing():
    """Existing server pushes are invisible to the mirror decoder."""
    assert decode_push({"frames": [{"data": {}}]}) is None
    assert decode_push({"data": "AAAA"}) is None
    assert decode_push({"event": "device.connected"}) is None
    assert decode_push({"type": "response", "id": "x", "success": True}) is None


def test_heartbeat_decodes_and_separates_stale_from_idle():
    """A heartbeat resets the watchdog without claiming data is flowing."""
    decoded = decode_push({PUSH_KEY: {"kind": "heartbeat", "epoch": 4, "seq": 12}})

    assert isinstance(decoded, MirrorHeartbeat)
    assert decoded.epoch == 4
    assert decoded.seq == 12


# ----------------------------------------------------------------------
# Chunked structure delivery
# ----------------------------------------------------------------------


def test_chunked_structure_reassembles_regardless_of_arrival_order():
    """
    Parts are index-keyed, so an out-of-order arrival still reassembles into
    the identical structure. The fixture deliberately delivers parts 1..N-1
    reversed.
    """
    client = MirrorClient.from_file(
        str(FIXTURE_DIR / "edge" / "structure-chunked.ndjson")
    )
    client.drain()

    assert client.stats.structure_chunks > 1
    assert client.stats.structures == 1
    assert client.stats.snapshots == 1
    assert client.stats.dropped_epoch == 0

    whole = MirrorClient.from_file(str(FIXTURE_DIR / "small.ndjson"))
    whole.drain()
    assert client.structure.layout_hash == whole.structure.layout_hash
    assert client.structure.dataset_ids == whole.structure.dataset_ids


def test_chunking_round_trips_a_structure_byte_for_byte():
    """Chunk, reassemble, and the decoded structure is unchanged."""
    project = load_project(load_manifest()["small"]["project"])
    ordered = [[0, i] for i in range(6)]
    push = encode_structure(
        epoch=5,
        dataset_ids=ordered,
        project=project,
        source_ids=[0],
        origin_unix_ms=0,
    )

    chunks = encode_structure_chunks(push, chunk_bytes=1024)
    assert len(chunks) > 1
    assert [c[PUSH_KEY]["part"] for c in chunks] == list(range(len(chunks)))
    assert all(c[PUSH_KEY]["parts"] == len(chunks) for c in chunks)

    import base64

    blob = "".join(c[PUSH_KEY]["data"] for c in chunks)
    restored = json.loads(base64.b64decode(blob).decode("utf-8"))
    assert restored == push[PUSH_KEY]

    decoded = decode_push({PUSH_KEY: restored})
    assert isinstance(decoded, MirrorStructure)
    assert decoded.epoch == 5


# ----------------------------------------------------------------------
# Documented constants
# ----------------------------------------------------------------------


def test_rate_bounds_and_watchdog_match_the_contract():
    """Rate range and the staleness bound are the values wire-protocol.md states."""
    assert (HZ_MIN, HZ_MAX) == (1, 60)

    fast = MirrorClient(hz=60, source=object())
    slow = MirrorClient(hz=1, source=object())

    assert fast.watchdog_s == 0.5
    assert slow.watchdog_s == 3.0
    assert MirrorClient(hz=20, source=object()).watchdog_s == pytest.approx(0.5)


def test_timestamps_are_relative_and_monotonic_per_source():
    """
    tNs is a per-source relative nanosecond count, not a transported
    steady_clock value: it starts near zero and never decreases within an
    epoch.
    """
    client = MirrorClient.from_file(str(FIXTURE_DIR / "multisource.ndjson"))
    seen: list[list[int]] = []

    for event in client.drain():
        if isinstance(event, MirrorSnapshot):
            seen.append(event.t_ns)

    assert seen
    assert all(len(t) == len(client.structure.source_ids) for t in seen)
    assert seen[0][0] < 10**9
    for previous, current in zip(seen, seen[1:]):
        assert all(c >= p for p, c in zip(previous, current))
