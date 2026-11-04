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


def test_bridge_forwards_only_jsonrpc_lines():
    """Lines a legacy client would get (frames, raw data) never reach the MCP client."""
    reply = b'{"jsonrpc":"2.0","id":1,"result":{}}\n'
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("127.0.0.1", 0))
    server.listen(1)
    env = dict(os.environ, SERIAL_STUDIO_API_PORT=str(server.getsockname()[1]))

    proc = subprocess.Popen(
        [sys.executable, "-I", str(BRIDGE)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
    )
    try:
        server.settimeout(GIVE_UP_SECONDS)
        conn, _ = server.accept()
        conn.sendall(b'{"frames":[]}\n{"data":"AA=="}\n' + reply)
        time.sleep(1.0)
        conn.close()
        out, err = proc.communicate(input=b"", timeout=GIVE_UP_SECONDS)
    finally:
        server.close()
        proc.kill()

    assert out == reply, out
    assert b"Dropped 2 non-JSON-RPC" in err, err.decode("utf-8", "replace")
