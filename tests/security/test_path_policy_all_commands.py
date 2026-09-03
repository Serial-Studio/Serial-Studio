#!/usr/bin/env python3
"""
The path allowlist as command metadata (spec 0075 I3/I7).

The registry used to guard four hard-coded command/parameter pairs; every other
path-taking command either re-checked by hand or checked nothing at all --
sessions.openDatabase created a database (plus its -wal/-shm siblings) anywhere on
disk and persisted it as the startup default, licensing.activateOffline and
assistant.restore read any file the user could read.

The contract now: every command that takes a file-system path declares it, the
registry enforces the declaration once, and a refusal is PATH_NOT_ALLOWED.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import os
import sys
import tempfile
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import APIError  # noqa: E402

# A path no allowlist root (home, temp, or SERIAL_STUDIO_API_ALLOWED_PATHS) can contain
OUTSIDE = (
    "C:\\Windows\\System32\\ss-policy.probe" if os.name == "nt" else "/ss-policy.probe"
)

# command -> the parameter that names a file-system path
GUARDED = [
    ("project.open", "filePath"),
    ("project.save", "filePath"),
    ("csvPlayer.open", "filePath"),
    ("mdf4Player.open", "filePath"),
    ("sessions.openDatabase", "filePath"),
    ("sessions.regress", "projectPath"),
    ("licensing.activateOffline", "path"),
    ("assistant.restore", "path"),
    ("io.opcua.exportCertificate", "path"),
    ("io.process.setExecutable", "executable"),
    ("io.process.setWorkingDir", "workingDir"),
    ("io.process.setPipePath", "pipePath"),
]


@pytest.mark.security
@pytest.mark.parametrize("command,param", GUARDED)
def test_outside_paths_are_refused(security_client, command, param):
    """A path outside the allowlist is refused with the shared code, for every command."""
    if not security_client.command_exists(command):
        pytest.skip(f"{command} is not registered in this build")

    with pytest.raises(APIError) as excinfo:
        security_client.command(command, {param: OUTSIDE})

    assert excinfo.value.code == "PATH_NOT_ALLOWED", (
        f"{command}.{param} answered {excinfo.value.code} "
        f"({excinfo.value}); the policy must refuse it with PATH_NOT_ALLOWED"
    )


@pytest.mark.security
def test_openDatabase_creates_nothing_outside_the_allowlist(security_client):
    """The refusal happens before the file is created, siblings included."""
    if not security_client.command_exists("sessions.openDatabase"):
        pytest.skip("sessions.openDatabase is not registered in this build")

    with pytest.raises(APIError) as excinfo:
        security_client.command("sessions.openDatabase", {"filePath": OUTSIDE})

    assert excinfo.value.code == "PATH_NOT_ALLOWED"
    for suffix in ("", "-wal", "-shm"):
        assert not os.path.exists(OUTSIDE + suffix), f"{OUTSIDE}{suffix} was created"


@pytest.mark.security
def test_allowed_paths_reach_the_handler(security_client):
    """A real file under an allowlist root is not refused by the policy.

    It may still fail on its own terms -- the point is that the failure is the
    handler's, not the gate's.
    """
    if not security_client.command_exists("csvPlayer.open"):
        pytest.skip("csvPlayer.open is not registered in this build")

    with tempfile.NamedTemporaryFile(suffix=".csv", delete=False) as handle:
        handle.write(b"time,value\n0,1\n")
        allowed = handle.name

    try:
        try:
            security_client.command("csvPlayer.open", {"filePath": allowed})
        except APIError as error:
            assert (
                error.code != "PATH_NOT_ALLOWED"
            ), f"a file under the temp root was refused by the path policy: {allowed}"
    finally:
        os.unlink(allowed)


@pytest.mark.security
def test_traversal_out_of_an_allowed_root_is_refused(security_client):
    """A path that walks out of an allowed root is normalized before the check."""
    if not security_client.command_exists("project.open"):
        pytest.skip("project.open is not registered in this build")

    traversal = str(
        Path(tempfile.gettempdir()) / ".." / ".." / ".." / "ss-policy.probe"
    )

    with pytest.raises(APIError) as excinfo:
        security_client.command("project.open", {"filePath": traversal})

    assert excinfo.value.code == "PATH_NOT_ALLOWED"
