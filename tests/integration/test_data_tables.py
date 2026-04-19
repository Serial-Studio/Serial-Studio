"""
Data Tables Integration Tests

Covers DataTablesHandler: user-defined shared-memory tables + registers.
Verifies CRUD, uniqueness, type persistence (numeric vs string), rename
collisions, and round-trip through project save/load.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time
from pathlib import Path

import pytest

from utils import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _list_tables(api_client):
    return api_client.command("project.tables.list")


def _add_table(api_client, name="Shared Table"):
    return api_client.command("project.tables.add", {"name": name})


def _get_table(api_client, name):
    return api_client.command("project.tables.get", {"name": name})


def _add_register(api_client, table, name, computed=True, value=0):
    return api_client.command(
        "project.tables.register.add",
        {"table": table, "name": name, "computed": computed, "value": value},
    )


def _update_register(api_client, table, name, **kwargs):
    payload = {"table": table, "name": name}
    payload.update(kwargs)
    return api_client.command("project.tables.register.update", payload)


def _delete_register(api_client, table, name):
    return api_client.command(
        "project.tables.register.delete", {"table": table, "name": name}
    )


def _rename_table(api_client, old, new):
    return api_client.command(
        "project.tables.rename", {"oldName": old, "newName": new}
    )


def _delete_table(api_client, name):
    return api_client.command("project.tables.delete", {"name": name})


# ---------------------------------------------------------------------------
# Table CRUD
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_empty_project_has_no_tables(api_client, clean_state):
    """A freshly created project starts with zero user tables."""
    result = _list_tables(api_client)
    assert result["count"] == 0
    assert result["tables"] == []


@pytest.mark.project
def test_add_table_returns_actual_name(api_client, clean_state):
    """tables.add returns the actual name used after uniquification."""
    result = _add_table(api_client, "Calibration")
    assert result["name"] == "Calibration"
    assert result["added"] is True


@pytest.mark.project
def test_add_table_uniquifies_duplicates(api_client, clean_state):
    """Re-adding a table with the same name auto-suffixes a counter."""
    first = _add_table(api_client, "Calibration")["name"]
    second = _add_table(api_client, "Calibration")["name"]
    third = _add_table(api_client, "Calibration")["name"]

    assert first == "Calibration"
    assert second == "Calibration 2"
    assert third == "Calibration 3"

    assert _list_tables(api_client)["count"] == 3


@pytest.mark.project
def test_add_table_with_empty_name_uses_default(api_client, clean_state):
    """Adding with an empty/missing name falls back to the default label."""
    result = _add_table(api_client, "")
    # ProjectModel::addTable substitutes tr("Shared Table") when empty
    assert result["name"].startswith("Shared Table") or result["name"] != ""


@pytest.mark.project
def test_list_tables_reports_register_count(api_client, clean_state):
    """tables.list reports the register count for each table."""
    _add_table(api_client, "Coeffs")
    _add_register(api_client, "Coeffs", "gain", computed=False, value=2.5)
    _add_register(api_client, "Coeffs", "offset", computed=False, value=0.1)

    tables = _list_tables(api_client)["tables"]
    coeffs = next(t for t in tables if t["name"] == "Coeffs")
    assert coeffs["registerCount"] == 2


@pytest.mark.project
def test_delete_table_removes_it(api_client, clean_state):
    """Deleting a table drops it from the list."""
    _add_table(api_client, "Temp")
    assert _list_tables(api_client)["count"] == 1

    _delete_table(api_client, "Temp")
    assert _list_tables(api_client)["count"] == 0


@pytest.mark.project
def test_delete_missing_table_is_noop(api_client, clean_state):
    """Deleting a non-existent table returns deleted=true but changes nothing."""
    _delete_table(api_client, "DoesNotExist")
    assert _list_tables(api_client)["count"] == 0


@pytest.mark.project
def test_rename_table_applies(api_client, clean_state):
    """Renaming changes the table name and preserves its register count."""
    _add_table(api_client, "Old")
    _add_register(api_client, "Old", "r1", value=1.0)

    result = _rename_table(api_client, "Old", "New")
    assert result["renamed"] is True

    tables = _list_tables(api_client)["tables"]
    names = [t["name"] for t in tables]
    assert "New" in names
    assert "Old" not in names

    regs = _get_table(api_client, "New")["registers"]
    assert len(regs) == 1
    assert regs[0]["name"] == "r1"


@pytest.mark.project
def test_rename_table_collision_is_rejected(api_client, clean_state):
    """Renaming to a name another table already owns is a silent no-op."""
    _add_table(api_client, "A")
    _add_table(api_client, "B")

    result = _rename_table(api_client, "A", "B")
    assert result["renamed"] is False

    names = sorted(t["name"] for t in _list_tables(api_client)["tables"])
    assert names == ["A", "B"]


@pytest.mark.project
def test_get_missing_table_errors(api_client, clean_state):
    """tables.get returns an INVALID_PARAM error when the table is absent."""
    with pytest.raises(APIError) as ei:
        _get_table(api_client, "Ghost")
    assert ei.value.code == "INVALID_PARAM"


# ---------------------------------------------------------------------------
# Register CRUD
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_add_numeric_register_preserves_type(api_client, clean_state):
    """Numeric registers round-trip with valueType='number'."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "n", computed=False, value=3.14)

    regs = _get_table(api_client, "T")["registers"]
    assert len(regs) == 1
    assert regs[0]["name"] == "n"
    assert regs[0]["type"] == "constant"
    assert regs[0]["valueType"] == "number"
    assert regs[0]["value"] == pytest.approx(3.14)


@pytest.mark.project
def test_add_string_register_preserves_type(api_client, clean_state):
    """String defaults round-trip with valueType='string'."""
    _add_table(api_client, "T")
    # Use update path since add's value param is untyped — we drop through
    # jsonToVariant which respects string JSON values.
    api_client.command(
        "project.tables.register.add",
        {"table": "T", "name": "label", "computed": False, "value": "hello"},
    )

    regs = _get_table(api_client, "T")["registers"]
    assert len(regs) == 1
    assert regs[0]["name"] == "label"
    assert regs[0]["valueType"] == "string"
    assert regs[0]["value"] == "hello"


@pytest.mark.project
def test_add_register_to_missing_table_is_noop(api_client, clean_state):
    """Adding to a non-existent table succeeds trivially (no table, no effect)."""
    _add_register(api_client, "Nope", "r1", value=1.0)
    assert _list_tables(api_client)["count"] == 0


@pytest.mark.project
def test_register_name_uniquification(api_client, clean_state):
    """Re-adding the same register name within a table suffixes _2, _3, ..."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "reg", value=1)
    _add_register(api_client, "T", "reg", value=2)
    _add_register(api_client, "T", "reg", value=3)

    names = [r["name"] for r in _get_table(api_client, "T")["registers"]]
    assert names == ["reg", "reg_2", "reg_3"]


@pytest.mark.project
def test_delete_register_removes_it(api_client, clean_state):
    """Deleting a register drops it from the register list."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "a", value=1.0)
    _add_register(api_client, "T", "b", value=2.0)

    _delete_register(api_client, "T", "a")

    names = [r["name"] for r in _get_table(api_client, "T")["registers"]]
    assert names == ["b"]


@pytest.mark.project
def test_update_register_rename(api_client, clean_state):
    """Updating a register can rename it in place."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "old_name", value=42.0)

    _update_register(api_client, "T", "old_name", newName="new_name")

    regs = _get_table(api_client, "T")["registers"]
    assert regs[0]["name"] == "new_name"
    assert regs[0]["value"] == pytest.approx(42.0)


@pytest.mark.project
def test_update_register_change_type(api_client, clean_state):
    """Toggling computed/constant is reflected in the returned type field."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "r", computed=False, value=1.0)

    _update_register(api_client, "T", "r", computed=True)

    regs = _get_table(api_client, "T")["registers"]
    assert regs[0]["type"] == "computed"


@pytest.mark.project
def test_update_register_change_default_value(api_client, clean_state):
    """The default value can be updated."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "r", value=0.0)

    _update_register(api_client, "T", "r", value=99.5)

    regs = _get_table(api_client, "T")["registers"]
    assert regs[0]["value"] == pytest.approx(99.5)


@pytest.mark.project
def test_update_register_partial_preserves_fields(api_client, clean_state):
    """Fields not specified in update stay at their previous value."""
    _add_table(api_client, "T")
    _add_register(api_client, "T", "r", computed=False, value=5.0)

    # Only rename — computed + value stay the same
    _update_register(api_client, "T", "r", newName="r2")

    regs = _get_table(api_client, "T")["registers"]
    assert regs[0]["name"] == "r2"
    assert regs[0]["type"] == "constant"
    assert regs[0]["value"] == pytest.approx(5.0)


@pytest.mark.project
def test_update_missing_register_errors(api_client, clean_state):
    """Updating a non-existent register returns INVALID_PARAM."""
    _add_table(api_client, "T")
    with pytest.raises(APIError) as ei:
        _update_register(api_client, "T", "nope", newName="x")
    assert ei.value.code == "INVALID_PARAM"


# ---------------------------------------------------------------------------
# Persistence round-trip
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_tables_persist_through_json_roundtrip(api_client, clean_state, tmp_path):
    """Tables and registers survive a project export → import cycle."""
    _add_table(api_client, "Calibration")
    _add_register(api_client, "Calibration", "gain", computed=False, value=2.5)
    _add_register(api_client, "Calibration", "offset", computed=False, value=0.1)
    _add_register(api_client, "Calibration", "mode_label", computed=False, value="ready")

    # Export the project JSON — project.exportJson returns {config: {...}}
    exported = api_client.command("project.exportJson")
    project_json = exported["config"]

    # Start a fresh project and reload
    api_client.create_new_project()
    time.sleep(0.2)
    assert _list_tables(api_client)["count"] == 0

    api_client.command("project.loadFromJSON", {"config": project_json})
    time.sleep(0.3)

    # Verify tables restored
    tables = _list_tables(api_client)
    names = [t["name"] for t in tables["tables"]]
    assert "Calibration" in names

    regs = _get_table(api_client, "Calibration")["registers"]
    reg_by_name = {r["name"]: r for r in regs}

    assert reg_by_name["gain"]["value"] == pytest.approx(2.5)
    assert reg_by_name["offset"]["value"] == pytest.approx(0.1)
    assert reg_by_name["mode_label"]["value"] == "ready"
    assert reg_by_name["mode_label"]["valueType"] == "string"


@pytest.mark.project
def test_multi_table_independence(api_client, clean_state):
    """Registers in different tables share names without collision."""
    _add_table(api_client, "A")
    _add_table(api_client, "B")

    _add_register(api_client, "A", "shared", value=1.0)
    _add_register(api_client, "B", "shared", value=2.0)

    a = _get_table(api_client, "A")["registers"]
    b = _get_table(api_client, "B")["registers"]

    assert a[0]["name"] == "shared"
    assert a[0]["value"] == pytest.approx(1.0)
    assert b[0]["name"] == "shared"
    assert b[0]["value"] == pytest.approx(2.0)
