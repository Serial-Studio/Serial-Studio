"""Extension install integrity against a running Serial Studio (spec 0075, K3/K5/K12).

The catalog decides what code lands in the extensions directory, so these tests drive the
install path from the outside: a local `file://` repository is generated per test, added
through the API, and the app is asked to install from it.

What is asserted is the trust boundary, not the happy path:

* a catalog v1 entry (files as bare strings, no digests) never installs;
* a file whose bytes do not match its published digest never installs, and when it is an
  update, the version already installed keeps its files;
* a plain `http://` repository is refused before it can serve a catalog at all.

Requires a running instance with the API server enabled (Settings -> Miscellaneous).
"""

import hashlib
import json
import time
from pathlib import Path

import pytest

pytestmark = [pytest.mark.integration]

EXTENSION_ID = "ss-test-theme"


def sha256_of(payload: bytes) -> str:
    """Return the digest a catalog entry publishes for these bytes."""
    return hashlib.sha256(payload).hexdigest()


def write_repo(
    root: Path, files: dict[str, bytes], version: str, digests: dict[str, str]
):
    """Write a one-entry repository; `digests` decides what the catalog claims per file."""
    root.mkdir(parents=True, exist_ok=True)
    for name, payload in files.items():
        (root / name).write_bytes(payload)

    entry = {
        "schemaVersion": 2,
        "id": EXTENSION_ID,
        "type": "theme",
        "title": "Spec 0075 test theme",
        "version": version,
        "files": [
            {"path": name, "sha256": digests[name], "size": len(payload)}
            for name, payload in files.items()
        ],
    }
    (root / "manifest.json").write_text(
        json.dumps({"extensions": [entry]}), encoding="utf-8"
    )
    return entry


def write_v1_repo(root: Path, files: dict[str, bytes], version: str):
    """Write the pre-0075 catalog shape: files as bare strings, nothing to verify against."""
    root.mkdir(parents=True, exist_ok=True)
    for name, payload in files.items():
        (root / name).write_bytes(payload)

    entry = {
        "id": EXTENSION_ID,
        "type": "theme",
        "title": "Spec 0075 test theme (v1)",
        "version": version,
        "files": list(files),
    }
    (root / "manifest.json").write_text(
        json.dumps({"extensions": [entry]}), encoding="utf-8"
    )


def add_repo(api_client, path: Path):
    """Point the app at a local repository and wait for the catalog refresh."""
    api_client.command("extensions.addRepository", {"url": str(path / "manifest.json")})
    time.sleep(0.5)
    api_client.command("extensions.refresh")
    time.sleep(1.0)


def drop_repo(api_client, path: Path):
    """Remove the test repository again, whatever the test did."""
    result = api_client.command("extensions.listRepositories")
    repos = (result or {}).get("repositories", [])
    target = str(path / "manifest.json")
    if target in repos:
        api_client.command(
            "extensions.removeRepository", {"index": repos.index(target)}
        )
        time.sleep(0.5)


def index_of_extension(api_client) -> int:
    """Return the catalog index of the test extension, or -1."""
    result = api_client.command("extensions.list")
    for i, entry in enumerate((result or {}).get("extensions", [])):
        if entry.get("id") == EXTENSION_ID:
            return i
    return -1


def installed_entry(api_client):
    """Return the catalog row of the test extension, or None."""
    result = api_client.command("extensions.list")
    for entry in (result or {}).get("extensions", []):
        if entry.get("id") == EXTENSION_ID:
            return entry
    return None


@pytest.fixture
def repo_dir(temp_dir):
    """A per-test repository directory the app is pointed at."""
    return Path(temp_dir) / "repo"


def test_catalog_v1_entry_is_not_installed(api_client, repo_dir):
    """An entry without digests is refused: an install could verify nothing."""
    write_v1_repo(repo_dir, {"theme.json": b'{"name":"x"}'}, "1.0.0")
    add_repo(api_client, repo_dir)

    index = index_of_extension(api_client)
    if index < 0:
        pytest.skip(
            "catalog entry not visible; extension repository refresh unavailable"
        )

    api_client.command("extensions.install", {"addonIndex": index})
    time.sleep(2.0)

    entry = installed_entry(api_client)
    assert entry is not None
    assert not entry.get("installed", False), "a v1 catalog entry must never install"

    drop_repo(api_client, repo_dir)


def test_corrupt_file_leaves_previous_version_installed(api_client, repo_dir):
    """A tampered update aborts, and the installed version keeps its own files."""
    payload = b"theme-v1"
    digests = {"theme.json": sha256_of(payload)}
    write_repo(repo_dir, {"theme.json": payload}, "1.0.0", digests)
    add_repo(api_client, repo_dir)

    index = index_of_extension(api_client)
    if index < 0:
        pytest.skip(
            "catalog entry not visible; extension repository refresh unavailable"
        )

    api_client.command("extensions.install", {"addonIndex": index})
    time.sleep(2.5)
    entry = installed_entry(api_client)
    assert entry is not None and entry.get(
        "installed"
    ), "the verified package should install"

    # Publish 2.0.0 whose bytes do not match the digest the catalog claims for them.
    write_repo(
        repo_dir,
        {"theme.json": b"theme-v2-tampered"},
        "2.0.0",
        {"theme.json": sha256_of(b"theme-v2-clean")},
    )
    api_client.command("extensions.refresh")
    time.sleep(1.0)

    index = index_of_extension(api_client)
    api_client.command("extensions.install", {"addonIndex": index})
    time.sleep(2.5)

    entry = installed_entry(api_client)
    assert entry is not None
    assert (
        entry.get("installedVersion") == "1.0.0"
    ), "a failed update must not replace 1.0.0"

    drop_repo(api_client, repo_dir)


def test_plain_http_repository_is_refused(api_client):
    """A cleartext catalog can be replaced in transit, so it is never added."""
    before = (api_client.command("extensions.listRepositories") or {}).get(
        "repositories", []
    )
    api_client.command(
        "extensions.addRepository", {"url": "http://example.invalid/manifest.json"}
    )
    time.sleep(0.5)

    after = (api_client.command("extensions.listRepositories") or {}).get(
        "repositories", []
    )
    assert "http://example.invalid/manifest.json" not in after
    assert len(after) == len(before)
