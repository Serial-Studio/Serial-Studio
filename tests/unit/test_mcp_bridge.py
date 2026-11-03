"""
MCP Bridge Failure Tests

Spawns examples/MCP Client/claude_desktop_bridge.py against a port nothing
listens on and pins spec 0112 R9: the bridge gives up within ten seconds,
exits 1, and its last stderr line names the fix (enable the API server in
Preferences), so an MCP client's status shows the cause instead of a hung
server. No running app needed; with the app up, the port override keeps the
bridge away from it.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import os
import socket
import subprocess
import sys
import time
from pathlib import Path

BRIDGE = (
    Path(__file__).parents[2] / "examples" / "MCP Client" / "claude_desktop_bridge.py"
)
GIVE_UP_SECONDS = 10.0


def closed_port() -> int:
    """A port the kernel just handed out and released, so nothing listens on it."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def test_bridge_exits_with_the_fix_when_nothing_listens():
    env = dict(os.environ, SERIAL_STUDIO_API_PORT=str(closed_port()))
    started = time.monotonic()
    proc = subprocess.run(
        [sys.executable, "-I", str(BRIDGE)],
        input=b"",
        capture_output=True,
        env=env,
        timeout=GIVE_UP_SECONDS + 5.0,
    )
    elapsed = time.monotonic() - started

    assert proc.returncode == 1, proc.stderr.decode("utf-8", "replace")
    assert elapsed < GIVE_UP_SECONDS, f"bridge took {elapsed:.1f} s to give up"

    lines = [
        line
        for line in proc.stderr.decode("utf-8", "replace").splitlines()
        if line.strip()
    ]
    assert lines, "bridge wrote nothing to stderr"
    assert "Enable API Server" in lines[-1], lines[-1]
