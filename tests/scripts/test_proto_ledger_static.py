"""Static checks over the gRPC field-number ledger (spec 0037, AC6/AC7).

Pure Python: no Qt, no Node.js, no running Serial Studio. The ledger at
app/rcc/api/proto-fields.json is released wire state -- a field number keeps its meaning
forever -- so these assertions cover the invariants that make that true, plus a simulated
property insertion and removal driven through the real generator function.
"""

import importlib.util
import json
import re
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
LEDGER = ROOT / "app" / "rcc" / "api" / "proto-fields.json"
SNAPSHOT = ROOT / "app" / "rcc" / "api" / "api-schema.json"
TYPED_PROTO = ROOT / "doc" / "grpc" / "serialstudio-typed.proto"
GENERATOR = ROOT / "scripts" / "generate-property-registry.py"


def load_generator():
    """Import the generator by path; its filename is not a valid module name."""
    spec = importlib.util.spec_from_file_location("property_registry", GENERATOR)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.fixture(scope="module")
def ledger():
    return json.loads(LEDGER.read_text(encoding="utf-8"))


@pytest.fixture(scope="module")
def commands(ledger):
    return ledger["commands"]


def test_ledger_carries_its_do_not_edit_marker(ledger):
    assert "never edit by hand" in ledger["_generated"]
    assert "append-only" in ledger["_note"]


def test_numbers_are_unique_within_each_command(commands):
    for name, entry in commands.items():
        numbers = list(entry["fields"].values())
        assert len(numbers) == len(set(numbers)), f"{name} assigns a number twice"


def test_fields_and_reserved_never_intersect(commands):
    for name, entry in commands.items():
        overlap = set(entry["fields"].values()) & set(entry["reserved"])
        assert not overlap, f"{name} reuses retired number(s) {sorted(overlap)}"


def test_number_one_is_never_assigned_to_a_parameter(commands):
    for name, entry in commands.items():
        assert 1 not in entry["fields"].values(), f"{name} collides with the request id"


def test_next_exceeds_every_used_number(commands):
    for name, entry in commands.items():
        used = list(entry["fields"].values()) + list(entry["reserved"])
        assert entry["next"] > max(used, default=1), f"{name} would reissue a number"


def test_every_snapshot_command_has_a_ledger_entry(commands):
    snapshot = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    missing = sorted({c["name"] for c in snapshot} - set(commands))
    assert not missing, f"commands with no ledger entry: {missing}"


def test_every_snapshot_parameter_is_numbered(commands):
    snapshot = json.loads(SNAPSHOT.read_text(encoding="utf-8"))
    for command in snapshot:
        entry = commands[command["name"]]
        missing = sorted(set(command.get("properties", {})) - set(entry["fields"]))
        assert not missing, f"{command['name']} has unnumbered params {missing}"


def test_typed_proto_numbers_match_the_ledger(commands):
    text = TYPED_PROTO.read_text(encoding="utf-8")
    blocks = re.findall(r"message (\w+)Request \{\n(.*?)\n\}", text, re.DOTALL)
    assert blocks, "no request messages found in the typed proto"

    by_message = {}
    for name, entry in commands.items():
        sanitized = re.sub(r"[._-](\w)", lambda m: m.group(1).upper(), name)
        by_message[sanitized[0].upper() + sanitized[1:]] = entry

    shared = {"Stream", "RawData"}
    for message, body in blocks:
        if message in shared:
            continue
        entry = by_message.get(message)
        assert entry is not None, f"{message} has no ledger entry"
        for line in body.splitlines():
            match = re.match(r"\s+\S+ (\w+) = (\d+);", line)
            if not match or match.group(1) == "id":
                continue
            param = match.group(1)
            param = "id" if param == "id_param" else param
            assert entry["fields"][param] == int(match.group(2))


def test_inserting_an_alphabetically_early_parameter_moves_no_number():
    generator = load_generator()
    seed = [{"name": "demo.cmd", "properties": {"beta": {}, "gamma": {}}}]
    first = generator.ledger_entries(seed, {})

    grown = [{"name": "demo.cmd", "properties": {"alpha": {}, "beta": {}, "gamma": {}}}]
    second = generator.ledger_entries(grown, first)

    assert first["demo.cmd"]["fields"] == {"beta": 2, "gamma": 3}
    assert second["demo.cmd"]["fields"] == {"alpha": 4, "beta": 2, "gamma": 3}
    assert second["demo.cmd"]["next"] == 5


def test_removing_a_parameter_retires_its_number_forever():
    generator = load_generator()
    seed = [{"name": "demo.cmd", "properties": {"alpha": {}, "beta": {}}}]
    first = generator.ledger_entries(seed, {})

    shrunk = [{"name": "demo.cmd", "properties": {"beta": {}}}]
    second = generator.ledger_entries(shrunk, first)
    assert second["demo.cmd"]["fields"] == {"beta": 3}
    assert second["demo.cmd"]["reserved"] == [2]

    regrown = [{"name": "demo.cmd", "properties": {"beta": {}, "delta": {}}}]
    third = generator.ledger_entries(regrown, second)
    assert third["demo.cmd"]["fields"]["delta"] == 4
    assert third["demo.cmd"]["reserved"] == [2]


def test_a_command_absent_from_the_snapshot_is_retained():
    generator = load_generator()
    existing = {"pro.only": {"fields": {"token": 2}, "reserved": [], "next": 3}}
    result = generator.ledger_entries([], existing)
    assert result["pro.only"]["fields"] == {"token": 2}
