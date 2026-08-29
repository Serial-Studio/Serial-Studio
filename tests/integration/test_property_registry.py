"""
Property Registry Round-Trip Baseline (spec 0036)

Captures and compares the project-JSON round trip for the whole shipped project
corpus, so the generated dataset serializer can be proven byte-identical to the
hand-written one it replaces:

  - capture mode (maintainer, pre-change build):
        SS_CAPTURE_BASELINES=1 pytest tests/integration/test_property_registry.py
    opens every .ssproj under examples/, app/rcc/demo/ and app/rcc/templates/
    via project.open, dumps project.exportJson, and writes one baseline file
    per project under tests/integration/baselines/.

  - compare mode (default): re-runs the same loop and compares each export
    against its checked-in baseline, allowing only the deltas declared by
    spec 0036 (overviewDisplay is now serialized; the read-side defaults for
    fftSamples, fftSamplingRate, ledHigh and index match the struct defaults).
    Per-source connection blocks are excluded on both sides: loading resolves
    them against live device discovery, so they vary per machine and per run.

  - offline mode: test_corpus_files_unchanged needs no running app; it checks
    the on-disk corpus against the sha256 manifest recorded in
    doc/claude/specs/0036-property-registry/baseline-manifest.json, so a
    baseline captured on a different corpus can never be compared silently.

Capture must run against the pre-change build (spec 0036, T1).

On top of the corpus round trip it pins the rest of the declaration contract:
AC3 (both declared defect fixes), AC6 (the update verb's prose no longer
enumerates fields -- the typed properties themselves are asserted over MCP in
test_api_surfaces.py), AC11 (a read dataset object written straight back is
lossless and warning-free) and AC13 (every declared option value round-trips).

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import hashlib
import json
import os
import re
import time
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]
PROPERTY_MANIFEST = REPO_ROOT / "app" / "rcc" / "properties" / "dataset.json"
FRAME_HEADER = REPO_ROOT / "app" / "src" / "DataModel" / "FrameKeys.h"
BASELINE_DIR = Path(__file__).resolve().parent / "baselines"
MANIFEST = (
    REPO_ROOT
    / "doc"
    / "claude"
    / "specs"
    / "0036-property-registry"
    / "baseline-manifest.json"
)

CORPUS_GLOBS = (
    "examples/**/*.ssproj",
    "app/rcc/demo/*.ssproj",
    "app/rcc/templates/**/*.ssproj",
)

# Dataset keys whose value or presence spec 0036 deliberately changes.
DECLARED_DELTA_KEYS = (
    "overviewDisplay",
    "fftSamples",
    "fftSamplingRate",
    "ledHigh",
    "index",
)


def corpus_projects():
    """Return every shipped project file, sorted by repo-relative path."""
    found = []
    for pattern in CORPUS_GLOBS:
        found.extend(REPO_ROOT.glob(pattern))
    return sorted(set(found), key=lambda p: p.relative_to(REPO_ROOT).as_posix())


def baseline_name(path):
    """Return the baseline file name for a project path (flat, collision-free)."""
    rel = path.relative_to(REPO_ROOT).as_posix()
    digest = hashlib.sha256(rel.encode("utf-8")).hexdigest()[:8]
    return f"{path.stem.replace(' ', '_')}.{digest}.json"


def export_project(api_client, path):
    """Open a project file and return its project.exportJson config object."""
    api_client.command("project.open", {"filePath": str(path)})
    time.sleep(0.4)
    return api_client.command("project.exportJson")["config"]


def dataset_lists(config):
    """Yield every dataset list in a config: top-level groups and per-source groups."""
    for group in config.get("groups", []):
        yield group.get("datasets", [])
    for source in config.get("sources", []):
        for group in source.get("groups", []):
            yield group.get("datasets", [])


def strip_declared_deltas(config):
    """Drop the dataset keys spec 0036 intentionally changes, so the rest of the
    document can be compared for byte-for-byte equality."""
    for datasets in dataset_lists(config):
        for dataset in datasets:
            for key in DECLARED_DELTA_KEYS:
                dataset.pop(key, None)
    return config


def strip_live_connection(config):
    """Drop per-source connection blocks: loading a project resolves them against
    live device discovery (BLE scan results, enumerated ports), so they are
    environment state, not document state. `treeExpansion` goes with them: the
    editor rewrites it on a debounced rebuild, keyed by the project title in
    effect at that moment, so it depends on load timing rather than the file."""
    for source in config.get("sources", []):
        source.pop("connection", None)
    config.pop("treeExpansion", None)
    return config


def canonical(config):
    """Return a stable textual form of an exported project config."""
    return json.dumps(config, indent=2, sort_keys=True, ensure_ascii=False)


@pytest.mark.project
def test_corpus_files_unchanged():
    """The on-disk corpus still matches the manifest the baselines were cut
    against; a mismatch means the baselines must be recaptured."""
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    recorded = {entry["path"]: entry["sha256"] for entry in manifest["projects"]}

    actual = {}
    for path in corpus_projects():
        rel = path.relative_to(REPO_ROOT).as_posix()
        actual[rel] = hashlib.sha256(path.read_bytes()).hexdigest()

    assert set(actual) == set(recorded), "Project corpus gained or lost files"
    drifted = [rel for rel, sha in actual.items() if recorded[rel] != sha]
    assert not drifted, f"Corpus files changed since capture: {drifted}"


@pytest.mark.integration
@pytest.mark.project
def test_capture_baselines(api_client, clean_state):
    """Capture mode: write one export baseline per project. Skipped unless
    SS_CAPTURE_BASELINES=1 is set, and must be run on the pre-change build."""
    if os.environ.get("SS_CAPTURE_BASELINES") != "1":
        pytest.skip("capture mode: set SS_CAPTURE_BASELINES=1 on the pre-change build")

    BASELINE_DIR.mkdir(parents=True, exist_ok=True)
    written = 0
    for path in corpus_projects():
        config = strip_live_connection(export_project(api_client, path))
        target = BASELINE_DIR / baseline_name(path)
        target.write_text(canonical(config) + "\n", encoding="utf-8", newline="")
        written += 1

    assert written == len(corpus_projects())


def property_manifest():
    """Return the dataset property manifest -- the declaration under test."""
    return json.loads(PROPERTY_MANIFEST.read_text(encoding="utf-8"))


def key_literals():
    """Map every Keys:: constant name to the JSON key it spells."""
    text = FRAME_HEADER.read_text(encoding="utf-8")
    return dict(re.findall(r'inline constexpr KeyView (\w+)\("([^"]*)"\)', text))


def document_keys():
    """Return {property id: on-disk JSON key} for every declared dataset property."""
    literals = key_literals()
    return {
        prop["id"]: literals[prop["jsonKey"]]
        for prop in property_manifest()["properties"]
        if prop.get("jsonKey") and prop["jsonKey"] in literals
    }


def writable_document_keys():
    """Return the on-disk key spelling of every API-writable property (spec 0036 R12)."""
    literals = key_literals()
    return {
        literals[prop["jsonKey"]]
        for prop in property_manifest()["properties"]
        if prop.get("api", {}).get("expose") and prop.get("jsonKey") in literals
    }


def enum_domains():
    """Return {api field name: (property id, persist rule, [values])} per fixed domain."""
    manifest = property_manifest()
    out = {}
    for prop in manifest["properties"]:
        api = prop.get("api", {})
        source = manifest["optionSources"].get(prop.get("options", ""), {})
        if not api.get("expose") or source.get("kind") not in (
            "staticMap",
            "parallelValues",
        ):
            continue
        out[api["name"]] = (
            prop["id"],
            prop["persist"],
            [entry["value"] for entry in source["entries"]],
        )
    return out


def make_dataset(api_client, title="Registry"):
    """Create a one-group, one-dataset project and return (groupId, datasetId)."""
    api_client.create_new_project(title=title)
    group_id = api_client.add_group("G")
    api_client.add_dataset(group_id)
    datasets = [d for d in api_client.list_datasets() if d["groupId"] == group_id]
    return group_id, datasets[-1]["datasetId"]


def exported_dataset(api_client, group_index=0, dataset_index=0):
    """Return one dataset object out of project.exportJson."""
    config = api_client.command("project.exportJson")["config"]
    return config["groups"][group_index]["datasets"][dataset_index]


@pytest.mark.integration
@pytest.mark.project
def test_overview_display_survives_a_save_reload_cycle(api_client, clean_state):
    """AC3: the key the hand-written serializer never wrote is written now."""
    group_id, dataset_id = make_dataset(api_client, "overviewDisplay defect")
    key = document_keys()["OverviewDisplay"]

    api_client.update_dataset(group_id, dataset_id, overviewDisplay=True)
    exported = api_client.command("project.exportJson")["config"]
    assert exported["groups"][0]["datasets"][0].get(key) is True

    api_client.load_project_from_json(exported)
    time.sleep(0.4)
    assert exported_dataset(api_client).get(key) is True


@pytest.mark.integration
@pytest.mark.project
def test_omitted_keys_load_the_struct_defaults(api_client, clean_state):
    """AC3: a project file that omits a key loads the declared struct default.

    The four keys below had a reader fallback that disagreed with the struct
    initializer, so the same dataset ended up in two different states depending on
    how it arrived. The expectation comes from the manifest, not a fresh dataset:
    project.dataset.add auto-assigns the next parser slot to index, while an
    omitted key must load the declared default (0 = unassigned).
    """
    make_dataset(api_client, "default alignment")
    keys = document_keys()
    declared = {p["id"]: p.get("default") for p in property_manifest()["properties"]}
    drifted = ["Index", "FftSamples", "FftSamplingRate", "LedHigh"]
    expected = {keys[pid]: declared[pid] for pid in drifted}

    config = api_client.command("project.exportJson")["config"]
    for key in expected:
        config["groups"][0]["datasets"][0].pop(key, None)

    api_client.load_project_from_json(config)
    time.sleep(0.4)
    reloaded = exported_dataset(api_client)
    actual = {key: reloaded.get(key) for key in expected}
    assert actual == expected


@pytest.mark.integration
@pytest.mark.project
def test_dataset_update_description_is_not_a_field_list(api_client):
    """AC6: the writable fields are typed schema properties, not a prose paragraph.

    The typed properties themselves are asserted over MCP tools/list in
    tests/integration/test_api_surfaces.py; this pins the other half of the change --
    that the description stopped restating them.
    """
    described = [
        command
        for command in api_client.get_available_commands()
        if command.get("name") == "project.dataset.update"
    ]
    assert described, "project.dataset.update is not registered"

    description = described[0].get("description", "")
    enumerated = [
        name
        for name in ("fftBallisticsRelease", "displayTickCount", "decimalPoints")
        if name in description
    ]
    assert not enumerated, (
        "the dataset update description still enumerates fields "
        f"({enumerated}); they belong in the typed schema"
    )


@pytest.mark.integration
@pytest.mark.project
def test_read_then_write_back_is_lossless(api_client, clean_state):
    """AC11: writing a read dataset object straight back warns about nothing.

    Only the keys the manifest declares are sent back: project.dataset.list decorates
    its objects with derived, read-only fields (groupTitle, enabledFeatures, ...) that
    were never writable.
    """
    group_id, dataset_id = make_dataset(api_client, "write-back")
    api_client.update_dataset(
        group_id, dataset_id, units="V", pltMin=-2.5, pltMax=2.5, fft=True
    )

    before = exported_dataset(api_client)
    declared = writable_document_keys()
    payload = {
        "groupId": group_id,
        "datasetId": dataset_id,
        **{key: value for key, value in before.items() if key in declared},
    }

    result = api_client.command("project.dataset.update", payload)
    assert result.get("updated") is True
    unknown = [
        w for w in result.get("warnings", []) if w.get("code") == "unknown_field"
    ]
    assert not unknown, f"declared keys were rejected as unknown: {unknown}"

    after = exported_dataset(api_client)
    changed = {
        key: (before[key], after.get(key))
        for key in payload
        if key in before and after.get(key) != before[key]
    }
    assert not changed, f"write-back changed values: {changed}"


@pytest.mark.integration
@pytest.mark.project
def test_enum_fields_round_trip_by_value(api_client, clean_state):
    """AC13: every declared option value is settable by value and reads back."""
    group_id, dataset_id = make_dataset(api_client, "enum domains")
    keys = document_keys()

    for field, (property_id, persist, values) in sorted(enum_domains().items()):
        key = keys[property_id]
        for value in values:
            api_client.update_dataset(group_id, dataset_id, **{field: value})
            stored = exported_dataset(api_client).get(key)
            if stored is None and persist != "always":
                continue

            assert stored == value, (
                f"{field}={value!r} read back as {stored!r} under the document key "
                f"'{key}'"
            )


@pytest.mark.integration
@pytest.mark.project
def test_round_trip_matches_baseline(api_client, clean_state):
    """AC2: every corpus project re-exports identically to its baseline, once
    the deltas spec 0036 declares are excluded."""
    if not BASELINE_DIR.is_dir() or not any(BASELINE_DIR.glob("*.json")):
        pytest.skip("no baselines captured yet (run with SS_CAPTURE_BASELINES=1)")

    mismatched = []
    for path in corpus_projects():
        target = BASELINE_DIR / baseline_name(path)
        if not target.is_file():
            mismatched.append(f"{path.name}: missing baseline")
            continue

        expected = strip_declared_deltas(
            strip_live_connection(json.loads(target.read_text(encoding="utf-8")))
        )
        actual = strip_declared_deltas(
            strip_live_connection(export_project(api_client, path))
        )
        if canonical(expected) != canonical(actual):
            mismatched.append(path.relative_to(REPO_ROOT).as_posix())

    assert not mismatched, f"Round-trip drift in: {mismatched}"
