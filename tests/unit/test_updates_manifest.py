"""
Update Feed Shape Tests

Pure-JSON validation of the repo-root updates.json (QSimpleUpdater appcast).
Pins the packaging-aware key set introduced by spec 0001: one endpoint per
shipped package type plus the legacy per-OS keys, with download URLs that
agree with each key's package type and architecture. No running app needed.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
from pathlib import Path

import pytest

MANIFEST_PATH = Path(__file__).parents[2] / "updates.json"

LEGACY_KEYS = {"windows", "osx", "linux", "linux-x64", "linux-arm64"}

PACKAGE_KEYS = {
    "windows-msi",
    "windows-portable",
    "windows-msix",
    "linux-appimage-x64",
    "linux-appimage-arm64",
    "linux-deb-x64",
    "linux-deb-arm64",
    "linux-rpm-x64",
    "linux-rpm-arm64",
    "osx-dmg",
}

# Keys that notify and open a page instead of downloading an asset
OPEN_URL_KEYS = {"windows-msix", "linux"}

# Expected download asset suffix per direct-download key
ASSET_SUFFIX = {
    "windows": "-Windows.msi",
    "windows-msi": "-Windows.msi",
    "windows-portable": "-Windows-Portable.zip",
    "osx": "-macOS.dmg",
    "osx-dmg": "-macOS.dmg",
    "linux-x64": "-Linux-x64.AppImage",
    "linux-arm64": "-Linux-arm64.AppImage",
    "linux-appimage-x64": "-Linux-x64.AppImage",
    "linux-appimage-arm64": "-Linux-arm64.AppImage",
    "linux-deb-x64": "-Linux-x64.deb",
    "linux-deb-arm64": "-Linux-arm64.deb",
    "linux-rpm-x64": "-Linux-x64.rpm",
    "linux-rpm-arm64": "-Linux-arm64.rpm",
}


@pytest.fixture(scope="module")
def updates():
    """Parsed 'updates' object of the repo-root manifest."""
    with open(MANIFEST_PATH, encoding="utf-8") as f:
        return json.load(f)["updates"]


def test_manifest_has_exactly_the_expected_keys(updates):
    assert set(updates) == LEGACY_KEYS | PACKAGE_KEYS


def test_every_entry_has_the_required_fields(updates):
    for key, entry in updates.items():
        for field in ("open-url", "latest-version", "download-url", "changelog"):
            assert field in entry, f"{key} is missing {field}"


def test_all_entries_advertise_the_same_version(updates):
    versions = {entry["latest-version"] for entry in updates.values()}
    assert len(versions) == 1, f"version mismatch across entries: {versions}"
    assert all(versions), "latest-version must not be empty"


def test_download_urls_match_package_type_and_arch(updates):
    version = updates["windows"]["latest-version"]
    for key, suffix in ASSET_SUFFIX.items():
        url = updates[key]["download-url"]
        assert url.endswith(suffix), f"{key}: {url} does not end with {suffix}"
        assert f"/v{version}/" in url, f"{key}: URL not pinned to v{version}"
        assert f"-{version}-" in url.rsplit("/", 1)[-1], f"{key}: asset not versioned"


def test_open_url_keys_have_no_direct_download(updates):
    for key in OPEN_URL_KEYS:
        assert updates[key]["download-url"] == "", f"{key} must not offer a download"
        assert updates[key]["open-url"], f"{key} must provide an open-url"


def test_direct_download_keys_do_not_shadow_with_open_url(updates):
    for key in ASSET_SUFFIX:
        assert (
            updates[key]["open-url"] == ""
        ), f"{key}: non-empty open-url would override the download action"


def test_changelog_present_everywhere(updates):
    for key, entry in updates.items():
        assert entry["changelog"].startswith("https://"), f"{key}: bad changelog URL"
