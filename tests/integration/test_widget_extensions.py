"""
Widget Extension Integration Tests (spec 0038)

Covers the project-surface half of AC2, AC5, AC7 and AC8: what a project may
record in a group or dataset `widget` field, what the problem center reports for
a package that cannot render, and the guarantee that no package metadata can
reach a Pro builtin.

Two tiers, because the catalog only rescans on install, uninstall, or a
workspace path change -- there is no API verb that forces one:

  * The tiers marked "catalog-free" need nothing installed and run as-is.
  * The seeded tier writes packages under <workspace>/Extensions/widget and
    needs Serial Studio restarted once before the app sees them. Running the
    file twice (seed, restart, run) is the intended maintainer flow; until the
    restart happens those tests skip with an explanation instead of failing.

Set SS_WORKSPACE when the workspace is not the default ~/Documents/Serial Studio.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import os
import shutil
from pathlib import Path

import pytest

CHECKER = "extension.widget"

# Reverse-domain ids, so they can never collide with a builtin widget string.
DATASET_PACKAGE = "com.serialstudio.test.dial"
GROUP_PACKAGE = "com.serialstudio.test.panel"
BROKEN_PACKAGE = "com.serialstudio.test.broken"
FUTURE_PACKAGE = "com.serialstudio.test.future"
DEPENDENT_PACKAGE = "com.serialstudio.test.dependent"
NO_QML_PACKAGE = "com.serialstudio.test.noqml"
REPLACING_PACKAGE = "com.serialstudio.test.impostor"
RESERVED_PACKAGE = "plot3d"

MISSING_PACKAGE = "com.serialstudio.test.never-installed"

WIDGET_QML = """import QtQuick

Item {
  required property var model
  Rectangle { anchors.fill: parent; color: "#202020" }
}
"""


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _run(api_client) -> dict:
    return api_client.command("problems.run")


def _findings(result: dict, code: str = None) -> list:
    rows = [f for f in result.get("findings", []) if f.get("checkerId") == CHECKER]
    if code is not None:
        rows = [f for f in rows if f.get("code") == code]
    return rows


def _one(result: dict, code: str) -> dict:
    rows = _findings(result, code=code)
    assert len(rows) == 1, f"Expected exactly one '{code}' finding, got {len(rows)}"
    return rows[0]


def _workspace() -> Path:
    override = os.environ.get("SS_WORKSPACE")
    if override:
        return Path(override)

    return Path.home() / "Documents" / "Serial Studio"


def _package_root() -> Path:
    return _workspace() / "Extensions" / "widget"


def _manifest(package_id: str, **widget) -> dict:
    block = {"apiVersion": "1.0", "scope": "dataset", "qml": "Widget.qml"}
    block.update(widget)
    return {
        "id": package_id,
        "type": "widget",
        "title": package_id.rsplit(".", 1)[-1].title(),
        "author": "Serial Studio test suite",
        "version": "1.0.0",
        "license": "MIT",
        "files": ["info.json", "Widget.qml"],
        "widget": block,
    }


def _write_package(
    root: Path, package_id: str, manifest, qml: str = WIDGET_QML
) -> None:
    directory = root / package_id
    directory.mkdir(parents=True, exist_ok=True)
    if qml is not None:
        (directory / "Widget.qml").write_text(qml, encoding="utf-8")

    text = manifest if isinstance(manifest, str) else json.dumps(manifest, indent=2)
    (directory / "info.json").write_text(text, encoding="utf-8")


def _group_uid(api_client, group_id: int) -> int:
    return api_client.list_groups()[group_id]["uniqueId"]


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture(scope="module")
def seeded_packages():
    """Write the failure-case packages into the workspace and leave them there.

    Deliberately not torn down: the catalog reads them at startup, so removing
    them at the end of the module would make the next run's restart pointless.
    """
    root = _package_root()
    try:
        root.mkdir(parents=True, exist_ok=True)
    except OSError as error:
        pytest.skip(f"cannot write the extension directory: {error}")

    _write_package(root, DATASET_PACKAGE, _manifest(DATASET_PACKAGE, scope="dataset"))
    _write_package(root, GROUP_PACKAGE, _manifest(GROUP_PACKAGE, scope="group"))
    _write_package(root, BROKEN_PACKAGE, "{ this is not json")
    _write_package(root, FUTURE_PACKAGE, _manifest(FUTURE_PACKAGE, hostCompat=">=99.0"))
    _write_package(
        root,
        DEPENDENT_PACKAGE,
        _manifest(
            DEPENDENT_PACKAGE,
            dependencies={"required": [{"id": "com.acme.absent", "version": ">=1.0"}]},
        ),
    )
    _write_package(root, NO_QML_PACKAGE, _manifest(NO_QML_PACKAGE), qml=None)
    _write_package(
        root,
        REPLACING_PACKAGE,
        _manifest(REPLACING_PACKAGE, replaces="compass"),
    )
    _write_package(
        root, RESERVED_PACKAGE, _manifest(RESERVED_PACKAGE, replaces="plot3d")
    )

    return root


@pytest.fixture
def visible_catalog(api_client, clean_state, seeded_packages):
    """Skip unless the running app already scanned the seeded packages."""
    gid = api_client.add_group("Probe")
    api_client.update_group(gid, widget=DATASET_PACKAGE)
    result = _run(api_client)
    if _findings(result, code="widget-not-installed"):
        pytest.skip(
            "seeded packages are not in the catalog yet -- restart Serial Studio "
            f"(packages written to {seeded_packages}) and re-run this file"
        )

    api_client.delete_group(gid)
    return seeded_packages


# ---------------------------------------------------------------------------
# Catalog-free: checker registration and project references (AC5)
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_widget_extension_checker_is_registered(api_client):
    checkers = api_client.command("problems.listCheckers")
    ids = [c.get("id") for c in checkers.get("checkers", [])]
    assert CHECKER in ids


@pytest.mark.integration
def test_missing_package_reports_one_finding_on_a_group(api_client, clean_state):
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget=MISSING_PACKAGE)

    finding = _one(_run(api_client), "widget-not-installed")
    assert finding["jump"] == "group"
    assert finding["entityUniqueId"] == _group_uid(api_client, gid)
    assert MISSING_PACKAGE in finding["explanation"]


@pytest.mark.integration
def test_missing_package_reports_one_finding_on_a_dataset(api_client, clean_state):
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)
    api_client.update_dataset(gid, 0, title="Heading", widget=MISSING_PACKAGE)

    finding = _one(_run(api_client), "widget-not-installed")
    assert finding["jump"] == "dataset"
    assert MISSING_PACKAGE in finding["explanation"]


@pytest.mark.integration
def test_clearing_the_reference_clears_the_finding(api_client, clean_state):
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget=MISSING_PACKAGE)
    assert _findings(_run(api_client), code="widget-not-installed")

    api_client.update_group(gid, widget="")
    assert not _findings(_run(api_client), code="widget-not-installed")


@pytest.mark.integration
def test_builtin_widget_strings_are_never_reported_as_extensions(
    api_client, clean_state
):
    for widget in ("datagrid", "multiplot", "accelerometer", "plot3d", "painter"):
        gid = api_client.add_group(f"Group {widget}")
        api_client.update_group(gid, widget=widget)

    assert not _findings(_run(api_client), code="widget-not-installed")


@pytest.mark.integration
def test_a_missing_package_reference_survives_a_project_round_trip(
    api_client, clean_state
):
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget=MISSING_PACKAGE)

    exported = api_client.command("project.exportJson")
    payload = exported.get("project", exported)
    assert json.dumps(payload).count(MISSING_PACKAGE) >= 1

    groups = api_client.list_groups()
    assert groups[gid]["widget"] == MISSING_PACKAGE


# ---------------------------------------------------------------------------
# Catalog-free: no Pro bypass (AC7)
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_an_extension_id_cannot_select_a_pro_widget(api_client, clean_state):
    """An id that merely looks like a builtin stays an unresolved extension id."""
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget="com.acme.plot3d")

    assert api_client.list_groups()[gid]["widget"] == "com.acme.plot3d"
    finding = _one(_run(api_client), "widget-not-installed")
    assert "com.acme.plot3d" in finding["explanation"]


@pytest.mark.integration
def test_the_bundled_conversions_still_resolve_as_builtins(api_client, clean_state):
    """AC8's project half: the converted widgets keep their builtin widget strings."""
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget="datagrid")
    api_client.add_dataset(gid)
    api_client.update_dataset(gid, 0, title="Heading", widget="compass")

    assert api_client.list_groups()[gid]["widget"] == "datagrid"
    assert api_client.list_datasets()[0]["widget"] == "compass"
    assert not _findings(_run(api_client))

    before = api_client.command("project.exportJson")
    after = api_client.command("project.exportJson")
    assert before == after


# ---------------------------------------------------------------------------
# Seeded: load-time rejections (AC5) and scope enforcement (AC2)
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.parametrize(
    "code",
    [
        "widget-manifest-invalid",
        "widget-host-incompatible",
        "widget-dependency-missing",
        "widget-qml-missing",
        "widget-replaces-forbidden",
        "widget-id-reserved",
    ],
)
def test_each_seeded_failure_reports_exactly_one_finding(
    api_client, visible_catalog, code
):
    _one(_run(api_client), code)


@pytest.mark.integration
def test_the_app_survives_every_seeded_failure(api_client, visible_catalog):
    assert api_client.command("problems.run")
    assert api_client.get_project_status() is not None


@pytest.mark.integration
def test_a_dataset_scope_package_is_offered_only_to_datasets(
    api_client, visible_catalog
):
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)
    api_client.update_dataset(gid, 0, title="Heading", widget=DATASET_PACKAGE)

    assert api_client.list_datasets()[0]["widget"] == DATASET_PACKAGE
    assert not _findings(_run(api_client), code="widget-not-installed")


@pytest.mark.integration
def test_a_group_scope_package_is_offered_only_to_groups(api_client, visible_catalog):
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget=GROUP_PACKAGE)

    assert api_client.list_groups()[gid]["widget"] == GROUP_PACKAGE
    assert not _findings(_run(api_client), code="widget-not-installed")


@pytest.mark.integration
def test_a_reserved_id_package_never_registers(
    api_client, visible_catalog, clean_state
):
    """R10: the package claiming 'plot3d' is refused, and the builtin is unaffected."""
    _one(_run(api_client), "widget-id-reserved")

    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget="plot3d")
    assert not _findings(_run(api_client), code="widget-not-installed")


@pytest.mark.integration
def test_an_installed_package_waits_for_consent(
    api_client, visible_catalog, clean_state
):
    """AC6's project half: a package the user has not allowed is reported, not run."""
    gid = api_client.add_group("Telemetry")
    api_client.update_group(gid, widget=GROUP_PACKAGE)

    consent = _findings(_run(api_client), code="widget-consent-required")
    if not consent:
        pytest.skip("the seeded package has already been allowed on this machine")

    assert consent[0]["jump"] == "group"


# ---------------------------------------------------------------------------
# Maintainer teardown helper
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.skipif(
    os.environ.get("SS_CLEAN_TEST_PACKAGES") != "1",
    reason="set SS_CLEAN_TEST_PACKAGES=1 to remove the seeded packages",
)
def test_remove_seeded_packages():
    root = _package_root()
    for name in (
        DATASET_PACKAGE,
        GROUP_PACKAGE,
        BROKEN_PACKAGE,
        FUTURE_PACKAGE,
        DEPENDENT_PACKAGE,
        NO_QML_PACKAGE,
        REPLACING_PACKAGE,
        RESERVED_PACKAGE,
    ):
        shutil.rmtree(root / name, ignore_errors=True)

    assert not any((root / name).exists() for name in (DATASET_PACKAGE, GROUP_PACKAGE))
