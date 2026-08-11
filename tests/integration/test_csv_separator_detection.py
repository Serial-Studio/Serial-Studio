"""
CSV Player Separator Auto-Detection Tests (spec 0048)

The player sniffs the cell separator once per opened file (comma, semicolon,
tab, pipe; comma wins ties) from the header and first data row, quote-aware.
These tests generate fixture files on the fly and verify that non-comma
recordings open, index and play back exactly like their comma equivalents,
and that well-formed comma files (including RFC-4180 quoted cells) keep
their existing behavior.

Requires a running Serial Studio instance with the API server enabled.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

# Rows of the shared numeric dataset: (time_ms, ch1, ch2, ch3).
NUMERIC_ROWS = [(i * 100, 800 + i, 25.5 + i * 0.25, i % 7) for i in range(20)]

# Mazda-shaped log: numeric ms column, '-' gaps, degree sign in a header label.
MAZDA_HEADER = [
    "time(ms)",
    "RPM(1/min)",
    "TSS(1/min)",
    "OSS(1/min)",
    "LOAD(%)",
    "TFT(°C)",
    "VSS(km/h)",
]
MAZDA_ROWS = [
    ["0", "803", "-", "-", "-", "-", "-"],
    ["18", "803", "788", "0", "-", "-", "-"],
    ["31", "803", "787", "0", "35.69", "-", "-"],
    ["49", "803", "787", "0", "35.69", "65", "-"],
    ["65", "803", "787", "0", "35.69", "65", "0.0"],
    ["80", "803", "787", "0", "35.69", "65", "0.0"],
    ["97", "803", "786", "0", "35.69", "65", "0.0"],
    ["113", "803", "786", "0", "35.69", "65", "0.1"],
]


def write_csv(path, separator, header, rows):
    """Write a CSV file with the given separator, header and data rows."""
    lines = [separator.join(header)]
    for row in rows:
        lines.append(separator.join(str(cell) for cell in row))
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return str(path)


def write_numeric_csv(path, separator):
    """Write the shared numeric dataset with the given separator."""
    header = ["time_ms", "ch1", "ch2", "ch3"]
    return write_csv(path, separator, header, NUMERIC_ROWS)


def dashboard_frame_datasets(api_client):
    """Title -> value map from the live dashboard frame (dashboard.getData)."""
    frame = api_client.command("dashboard.getData").get("frame", {})
    out = {}
    for group in frame.get("groups", []):
        for dataset in group.get("datasets", []):
            out[dataset.get("title")] = dataset.get("value")
    return out


def open_and_wait(api_client, file_path, expected_rows, timeout=15.0):
    """Open a CSV file and poll until it is open and fully indexed."""
    api_client.command("csvPlayer.open", {"filePath": file_path})

    deadline = time.time() + timeout
    status = {}
    while time.time() < deadline:
        status = api_client.command("csvPlayer.getStatus")
        if status.get("isOpen") and status.get("frameCount", 0) >= expected_rows:
            return status
        time.sleep(0.1)

    return status


def playback_fingerprint(api_client, file_path, expected_rows):
    """Open a file and return (frameCount, timestamps at frames 0/mid/last)."""
    status = open_and_wait(api_client, file_path, expected_rows)
    assert status.get("isOpen"), f"file did not open: {file_path}"

    frame_count = status.get("frameCount")
    timestamps = []
    for target in (0, expected_rows // 2, expected_rows - 1):
        api_client.command(
            "csvPlayer.setProgress", {"progress": target / max(1, frame_count)}
        )
        time.sleep(0.3)
        timestamps.append(api_client.command("csvPlayer.getStatus").get("timestamp"))

    api_client.command("csvPlayer.close")
    time.sleep(0.2)
    return frame_count, timestamps


@pytest.mark.integration
@pytest.mark.csv
def test_semicolon_mazda_log_opens_without_prompt(api_client, clean_state, tmp_path):
    """AC1/AC5: a semicolon OBD-style log opens, indexes all rows, plays back."""
    api_client.set_operation_mode("quickplot")
    path = write_csv(tmp_path / "mazda.csv", ";", MAZDA_HEADER, MAZDA_ROWS)

    status = open_and_wait(api_client, path, len(MAZDA_ROWS))
    assert status.get("isOpen"), "semicolon file failed to open"
    assert status.get("frameCount") == len(MAZDA_ROWS)

    api_client.command("csvPlayer.step", {"delta": 1})
    time.sleep(0.3)
    status = api_client.command("csvPlayer.getStatus")
    assert status.get("framePosition") == 1

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_semicolon_channels_reach_dashboard(api_client, clean_state, tmp_path):
    """AC1: the six non-timestamp columns become QuickPlot datasets."""
    api_client.set_operation_mode("quickplot")
    path = write_csv(tmp_path / "mazda_channels.csv", ";", MAZDA_HEADER, MAZDA_ROWS)

    status = open_and_wait(api_client, path, len(MAZDA_ROWS))
    assert status.get("isOpen")

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.5)

    datasets = dashboard_frame_datasets(api_client)
    assert set(datasets) == set(MAZDA_HEADER[1:]), f"wrong channels: {datasets}"
    assert datasets["RPM(1/min)"] == "803", f"wrong last-row value: {datasets}"
    assert datasets["TSS(1/min)"] == "786", f"wrong last-row value: {datasets}"
    assert datasets["VSS(km/h)"] == "0.1", f"wrong last-row value: {datasets}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
@pytest.mark.parametrize(
    "separator,name", [(",", "comma"), (";", "semicolon"), ("\t", "tab")]
)
def test_separator_variants_open_identically(
    api_client, clean_state, tmp_path, separator, name
):
    """AC2 (per-variant): each separator variant indexes every data row."""
    api_client.set_operation_mode("quickplot")
    path = write_numeric_csv(tmp_path / f"data_{name}.csv", separator)

    status = open_and_wait(api_client, path, len(NUMERIC_ROWS))
    assert status.get("isOpen"), f"{name} file failed to open"
    assert status.get("frameCount") == len(NUMERIC_ROWS)

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_separator_variants_identical_playback(api_client, clean_state, tmp_path):
    """AC2: comma, semicolon and tab variants produce identical fingerprints."""
    api_client.set_operation_mode("quickplot")

    results = {}
    for separator, name in ((",", "comma"), (";", "semicolon"), ("\t", "tab")):
        path = write_numeric_csv(tmp_path / f"fp_{name}.csv", separator)
        results[name] = playback_fingerprint(api_client, path, len(NUMERIC_ROWS))

    assert (
        results["semicolon"] == results["comma"]
    ), f"semicolon diverged from comma: {results}"
    assert results["tab"] == results["comma"], f"tab diverged from comma: {results}"


@pytest.mark.integration
@pytest.mark.csv
def test_quoted_comma_export_unchanged(api_client, clean_state, tmp_path):
    """AC3: an RFC-4180 comma file with quoted cells keeps today's behavior."""
    api_client.set_operation_mode("quickplot")

    header = ["time_ms", '"RPM, engine"', '"Load ""raw"""', "speed"]
    rows = [[i * 50, 800 + i, f'"{i},{i + 1}"', i * 2] for i in range(10)]
    path = write_csv(tmp_path / "quoted_comma.csv", ",", header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), "quoted comma file failed to open"
    assert status.get("frameCount") == len(rows)

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.5)
    datasets = dashboard_frame_datasets(api_client)
    assert set(datasets) == {
        "RPM, engine",
        'Load "raw"',
        "speed",
    }, f"quoted comma cells mangled: {datasets}"
    assert datasets["speed"] == "18", f"wrong last-row value: {datasets}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
@pytest.mark.parametrize(
    "separator,name", [(",", "comma"), (";", "semicolon"), ("\t", "tab"), ("|", "pipe")]
)
def test_quoted_separator_does_not_split(
    api_client, clean_state, tmp_path, separator, name
):
    """AC4: a separator inside a quoted cell never splits the cell."""
    api_client.set_operation_mode("quickplot")

    header = ["time_ms", f'"a{separator}b"', "ch2", "ch3"]
    rows = [[i * 100, f'"{i}{separator}{i}"', i + 1, i + 2] for i in range(10)]
    path = write_csv(tmp_path / f"quoted_{name}.csv", separator, header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), f"quoted {name} file failed to open"
    assert status.get("frameCount") == len(rows)

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.5)
    shown = separator if separator != "\t" else " "
    datasets = dashboard_frame_datasets(api_client)
    assert set(datasets) == {
        f"a{shown}b",
        "ch2",
        "ch3",
    }, f"quoted {name} separator split a cell: {datasets}"
    assert (
        datasets[f"a{shown}b"] == f"9{shown}9"
    ), f"quoted {name} cell mangled: {datasets}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_comma_file_with_quoted_semicolons_stays_comma(
    api_client, clean_state, tmp_path
):
    """R2: quoted semicolons in a comma file's text cell never flip detection."""
    api_client.set_operation_mode("quickplot")

    header = ["time_ms", "note", "speed"]
    rows = [[i * 100, f'"a;b;c;{i}"', i * 2] for i in range(10)]
    path = write_csv(tmp_path / "comma_quoted_semis.csv", ",", header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), "comma file with quoted semicolons failed to open"
    assert status.get("frameCount") == len(rows)

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.5)
    datasets = dashboard_frame_datasets(api_client)
    assert set(datasets) == {
        "note",
        "speed",
    }, f"semicolon leak flipped detection: {datasets}"
    assert datasets["note"] == "a;b;c;9", f"quoted cell mangled: {datasets}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_comma_file_with_unquoted_semicolons_stays_comma(
    api_client, clean_state, tmp_path
):
    """R2: unquoted semicolons in a comma file's text cells never flip detection."""
    api_client.set_operation_mode("quickplot")

    header = ["time_ms", "note", "speed"]
    rows = [[i * 100, f"a;b;c;{i}", i * 2] for i in range(10)]
    path = write_csv(tmp_path / "comma_unquoted_semis.csv", ",", header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), "comma file with unquoted semicolons failed to open"
    assert status.get("frameCount") == len(rows)

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.5)
    datasets = dashboard_frame_datasets(api_client)
    assert set(datasets) == {
        "note",
        "speed",
    }, f"semicolon leak flipped detection: {datasets}"
    assert datasets["note"] == "a;b;c;9", f"text cell mangled: {datasets}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_millisecond_unit_header_scales_timeline(api_client, clean_state, tmp_path):
    """R7/AC7: a time(ms) header scales the numeric timeline to real seconds."""
    api_client.set_operation_mode("quickplot")

    header = ["time(ms)", "ch1"]
    rows = [[i * 100, i] for i in range(20)]
    path = write_csv(tmp_path / "ms_unit.csv", ";", header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), "ms-unit file failed to open"

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.3)
    ts = api_client.command("csvPlayer.getStatus").get("timestamp")
    assert ts == "00:00:01.900", f"time(ms) column not scaled to seconds: {ts}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_second_unit_header_stays_seconds(api_client, clean_state, tmp_path):
    """R7: an explicit seconds unit keeps the legacy reading, silently.

    A header with NO unit marker prompts the user (seconds preselected) and
    cannot be exercised over the API — that path is the AC7 maintainer check.
    """
    api_client.set_operation_mode("quickplot")

    header = ["time(s)", "ch1"]
    rows = [[i * 100, i] for i in range(20)]
    path = write_csv(tmp_path / "second_unit.csv", ",", header, rows)

    status = open_and_wait(api_client, path, len(rows))
    assert status.get("isOpen"), "seconds-unit file failed to open"

    api_client.command("csvPlayer.setProgress", {"progress": 1.0})
    time.sleep(0.3)
    ts = api_client.command("csvPlayer.getStatus").get("timestamp")
    assert ts == "00:31:40.000", f"seconds column not read as seconds: {ts}"

    api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.csv
def test_single_column_file_keeps_legacy_flow(api_client, clean_state, tmp_path):
    """R6: a file with no candidate separator still opens as a single column.

    The (s) unit marker keeps the open dialog-free over the API (an unmarked
    numeric header prompts for units since the R7 addendum).
    """
    api_client.set_operation_mode("quickplot")

    path = tmp_path / "single_column.csv"
    lines = ["value(s)"] + [str(i * 10) for i in range(10)]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    status = open_and_wait(api_client, str(path), 10)
    assert status.get("isOpen"), "single-column file failed to open"
    assert status.get("frameCount") == 10

    api_client.command("csvPlayer.close")
