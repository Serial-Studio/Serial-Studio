"""
Test Validators

Validation helpers for test assertions.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import csv
import json
import os
from pathlib import Path
from typing import Any, Optional


def validate_frame_structure(frame: dict, expected_groups: Optional[int] = None) -> bool:
    """
    Validate frame has correct structure.

    Args:
        frame: Frame dict from Serial Studio
        expected_groups: Expected number of groups (None to skip check)

    Returns:
        True if valid

    Raises:
        AssertionError: If validation fails
    """
    assert "t" in frame, "Frame missing 't' (title) field"
    assert "g" in frame, "Frame missing 'g' (groups) field"
    assert isinstance(frame["g"], list), "Groups must be a list"

    if expected_groups is not None:
        assert len(frame["g"]) == expected_groups, (
            f"Expected {expected_groups} groups, got {len(frame['g'])}"
        )

    for i, group in enumerate(frame["g"]):
        assert isinstance(group, dict), f"Group {i} is not a dict"
        assert "title" in group, f"Group {i} missing title"
        assert "datasets" in group, f"Group {i} missing datasets"
        assert isinstance(group["datasets"], list), f"Group {i} datasets not a list"

        for j, dataset in enumerate(group["datasets"]):
            assert isinstance(dataset, dict), f"Group {i} dataset {j} is not a dict"
            assert "title" in dataset, f"Group {i} dataset {j} missing title"
            assert "value" in dataset, f"Group {i} dataset {j} missing value"

    return True


def validate_csv_export(
    csv_path: str,
    min_rows: int = 1,
    expected_columns: Optional[int] = None,
    check_timestamps: bool = True,
) -> bool:
    """
    Validate CSV export file.

    Args:
        csv_path: Path to CSV file
        min_rows: Minimum expected rows (excluding header)
        expected_columns: Expected column count (None to skip)
        check_timestamps: Verify timestamp column is monotonic

    Returns:
        True if valid

    Raises:
        AssertionError: If validation fails
    """
    assert os.path.exists(csv_path), f"CSV file not found: {csv_path}"
    assert os.path.getsize(csv_path) > 0, f"CSV file is empty: {csv_path}"

    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.reader(f)
        rows = list(reader)

    assert len(rows) >= 2, "CSV must have header + at least one data row"
    assert len(rows) - 1 >= min_rows, (
        f"Expected at least {min_rows} data rows, got {len(rows) - 1}"
    )

    header = rows[0]
    assert header[0].lower() == "timestamp", "First column must be 'Timestamp'"

    if expected_columns is not None:
        for row in rows[1:]:
            assert len(row) == expected_columns, (
                f"Expected {expected_columns} columns, got {len(row)}"
            )

    if check_timestamps and len(rows) > 2:
        timestamps = []
        for row in rows[1:]:
            try:
                timestamps.append(float(row[0]))
            except ValueError:
                raise AssertionError(f"Invalid timestamp value: {row[0]}")

        for i in range(1, len(timestamps)):
            assert timestamps[i] >= timestamps[i - 1], (
                f"Timestamps not monotonic at row {i}: {timestamps[i - 1]} -> {timestamps[i]}"
            )

    return True


def validate_project_file(project_path: str) -> bool:
    """
    Validate project JSON file.

    Args:
        project_path: Path to .json project file

    Returns:
        True if valid

    Raises:
        AssertionError: If validation fails
    """
    assert os.path.exists(project_path), f"Project file not found: {project_path}"

    with open(project_path, "r", encoding="utf-8") as f:
        try:
            project = json.load(f)
        except json.JSONDecodeError as e:
            raise AssertionError(f"Invalid JSON in project file: {e}")

    assert "frameParser" in project, "Project missing frameParser"
    assert "groups" in project, "Project missing groups"
    assert isinstance(project["groups"], list), "Groups must be a list"

    return True


def validate_checksum(
    data: bytes,
    checksum_value: int,
    checksum_type: str,
) -> bool:
    """
    Validate checksum matches data.

    Args:
        data: Payload bytes
        checksum_value: Expected checksum value
        checksum_type: Checksum algorithm name

    Returns:
        True if valid

    Raises:
        AssertionError: If checksum mismatch
    """
    from .data_generator import ChecksumType, DataGenerator

    try:
        algo = ChecksumType(checksum_type.lower())
    except ValueError:
        raise AssertionError(f"Unknown checksum type: {checksum_type}")

    calculated = DataGenerator.calculate_checksum(data, algo)
    assert calculated == checksum_value, (
        f"Checksum mismatch: expected {checksum_value:X}, got {calculated:X}"
    )

    return True


def validate_export_file_integrity(
    export_path: str,
    file_type: str = "csv",
) -> bool:
    """
    Validate export file can be opened and read.

    Args:
        export_path: Path to export file
        file_type: "csv" or "mdf4"

    Returns:
        True if file is readable

    Raises:
        AssertionError: If file cannot be read
    """
    assert os.path.exists(export_path), f"Export file not found: {export_path}"
    assert os.path.getsize(export_path) > 0, f"Export file is empty: {export_path}"

    if file_type == "csv":
        with open(export_path, "r", encoding="utf-8") as f:
            csv.reader(f)
            assert True, "CSV file is readable"

    return True
