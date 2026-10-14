"""
Sealed deployment-package session (spec 0096, AC5 and AC7).

Maintainer-run against a live app opened on the marker fixture with the API server on:

    serial-studio --deployment tests/fixtures/deploy/marker.ssdeploy --api-server

The fixture's parser, transform, transform library, Canvas and control script each carry an
"SSMARK_" marker. A sealed session must never hand one out: not through any denied command,
not through the commands that stay allowed, not through the per-frame broadcast or the mirror,
not through a project script's own apiCall, and not into any file the app writes. The module
skips unless the running app answers as a sealed session. Replaying the recording made here is
a maintainer observation (players are not reachable from a sealed session).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import os
import re
import socket
import sys
import tempfile
import time
from pathlib import Path

import pytest

from utils import DeviceSimulator, SerialStudioClient
from utils.api_client import APIError
from utils.mirror_client import MirrorClient, MirrorError

pytestmark = [pytest.mark.integration]

MARKER = b"SSMARK_"
REPO_ROOT = Path(__file__).resolve().parents[2]
POLICY_SOURCE = REPO_ROOT / "core" / "Api" / "API" / "SealedPolicy.cpp"
FLAG_DATASET = "Project Export Allowed"
MAX_SCAN_BYTES = 256 * 1024 * 1024
MAX_SCAN_FILES = 50_000


def _denied_commands() -> list[str]:
    """Every name in SealedPolicy's denied table, read from the source of truth."""
    text = POLICY_SOURCE.read_text(encoding="utf-8")
    block = text.split("kDenied[] = {", 1)[1].split("};", 1)[0]
    return re.findall(r'QLatin1StringView\("([^"]+)"\)', block)


def _answers_as_sealed(client: SerialStudioClient) -> bool:
    """True when a project-reading command is refused with SEALED_SESSION."""
    try:
        client.command("project.exportJson")
    except APIError as error:
        return error.code == "SEALED_SESSION"

    return False


@pytest.fixture(scope="module")
def sealed(serial_studio_running):
    """A connected client to an app running the marker package; skips otherwise."""
    client = SerialStudioClient(timeout=15.0)
    client.connect()
    if not _answers_as_sealed(client):
        client.disconnect()
        pytest.skip(
            "not a sealed session: launch with "
            "--deployment tests/fixtures/deploy/marker.ssdeploy --api-server"
        )

    yield client
    client.disconnect()


@pytest.fixture(scope="module")
def streaming(sealed):
    """Feeds the package's TCP source. Never disconnects: in runtime mode that quits the app."""
    simulator = DeviceSimulator(host="127.0.0.1", port=9000, protocol="tcp")
    simulator.start()
    if not sealed.is_connected():
        sealed.command("io.connect")

    assert simulator.wait_for_connection(10.0), "the package's source never connected"
    for value in range(50):
        simulator.send_frame(f"{value}\n".encode())
        time.sleep(0.02)

    yield simulator
    simulator.stop()


def _flag_value(sealed: SerialStudioClient):
    """The value the fixture parser publishes for its own apiCall probe, or None."""
    frame = sealed.command("dashboard.getData").get("frame", {})
    for group in frame.get("groups", []):
        for dataset in group.get("datasets", []):
            if dataset.get("title") == FLAG_DATASET:
                return dataset.get("value")

    return None


def _scan_roots() -> list[Path]:
    """Folders where the app keeps settings, caches, backups, recordings and temp files."""
    home = Path.home()
    roots = [Path(tempfile.gettempdir()), home / "Documents" / "Serial Studio"]
    if sys.platform == "darwin":
        roots += [
            home / "Library" / "Application Support",
            home / "Library" / "Preferences",
            home / "Library" / "Caches",
        ]
    elif sys.platform.startswith("win"):
        roots += [
            Path(os.environ.get("APPDATA", "")),
            Path(os.environ.get("LOCALAPPDATA", "")),
        ]
    else:
        roots += [home / ".local" / "share", home / ".config", home / ".cache"]

    return [root for root in roots if str(root) and root.exists()]


def _app_files(root: Path):
    """Files under the app's own folders in @p root (names containing "serial"), bounded."""
    seen = 0
    candidates = (
        [root]
        if root.name == "Serial Studio"
        else [entry for entry in root.iterdir() if "serial" in entry.name.lower()]
    )
    for candidate in candidates:
        paths = [candidate] if candidate.is_file() else candidate.rglob("*")
        for path in paths:
            if seen >= MAX_SCAN_FILES:
                return

            if path.is_file() and path.stat().st_size <= MAX_SCAN_BYTES:
                seen += 1
                yield path


# ----------------------------------------------------------------------
# AC5 -- the API surfaces
# ----------------------------------------------------------------------


def test_every_denied_command_is_refused(sealed):
    """Each denied command answers SEALED_SESSION, before any parameter validation."""
    denied = _denied_commands()
    assert denied, "could not read the denied table"

    leaked = []
    for name in denied:
        try:
            sealed.command(name)
            leaked.append(f"{name}: succeeded")
        except APIError as error:
            if error.code != "SEALED_SESSION":
                leaked.append(f"{name}: {error.code}")

        time.sleep(0.02)

    assert not leaked, leaked


def test_allowed_frame_read_carries_no_script(sealed, streaming):
    """dashboard.getData still works, and the frame it returns carries no script source."""
    data = sealed.command("dashboard.getData")
    assert data.get("frame", {}).get("groups"), "no live frame"
    assert MARKER not in json.dumps(data).encode()


def test_project_script_cannot_export_the_project(sealed, streaming):
    """The fixture parser's own apiCall("project.exportJson") was refused (flag reads 0)."""
    deadline = time.monotonic() + 5.0
    value = None
    while time.monotonic() < deadline and value in (None, ""):
        value = _flag_value(sealed)
        time.sleep(0.2)

    assert value not in (None, ""), "the probe dataset never received a value"
    assert float(value) == 0.0


def test_per_frame_broadcast_carries_no_script(sealed, streaming):
    """A plain API socket receives live frames, and none carries a marker."""
    received = b""
    with socket.create_connection(("127.0.0.1", 7777), timeout=0.5) as raw:
        deadline = time.monotonic() + 3.0
        while time.monotonic() < deadline and len(received) < (1 << 20):
            streaming.send_frame(b"7\n")
            try:
                received += raw.recv(65536)
            except socket.timeout:
                pass

    assert b'"frames"' in received, "no per-frame broadcast arrived"
    assert MARKER not in received


def test_mirror_structure_carries_no_script(sealed, streaming):
    """A mirror viewer's structure carries the layout but no script source."""
    client = MirrorClient(
        host="127.0.0.1", port=7777, token=os.environ.get("SS_MIRROR_TOKEN", ""), hz=20
    )
    try:
        client.attach()
    except MirrorError as error:
        pytest.skip(f"mirror unavailable: {error}")

    try:
        assert client.structure is not None
        assert MARKER not in json.dumps(client.structure.project).encode()
    finally:
        client.close()


# ----------------------------------------------------------------------
# AC7 -- nothing from the package reaches the disk
# ----------------------------------------------------------------------


def test_no_marker_reaches_disk(sealed, streaming):
    """Backups, settings, the historian database and temp files hold no marker."""
    time.sleep(2.0)
    hits = []
    for root in _scan_roots():
        for path in _app_files(root):
            try:
                if MARKER in path.read_bytes():
                    hits.append(str(path))
            except OSError:
                continue

    assert not hits, hits
