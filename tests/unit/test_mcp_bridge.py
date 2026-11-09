"""
MCP Bridge Failure Tests

Spawns examples/MCP Client/claude_desktop_bridge.py against a port nothing
listens on and pins spec 0112 R9 as amended: the bridge answers the MCP
handshake itself, fails a tool call with the fix (enable the API server in
Preferences) and exits cleanly when the client leaves, so an MCP client shows
the cause instead of a hung server. No running app needed; with the app up, the
port override keeps the bridge away from it. The reconnect, stalled-link and
shutdown cases live with the plugin's copy, Serial-Studio/claude-plugin.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import os
import socket
import subprocess
import sys
import time
from pathlib import Path

BRIDGE = (
    Path(__file__).parents[2] / "examples" / "MCP Client" / "claude_desktop_bridge.py"
)
ANSWER_SECONDS = 10.0


def closed_port() -> int:
    """A port the kernel just handed out and released, so nothing listens on it."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def test_bridge_answers_with_the_fix_when_nothing_listens():
    env = dict(os.environ, SERIAL_STUDIO_API_PORT=str(closed_port()))
    requests = [
        {"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {}},
        {
            "jsonrpc": "2.0",
            "id": 2,
            "method": "tools/call",
            "params": {"name": "io.getStatus", "arguments": {}},
        },
    ]
    started = time.monotonic()
    proc = subprocess.run(
        [sys.executable, "-I", str(BRIDGE)],
        input=b"".join(json.dumps(r).encode() + b"\n" for r in requests),
        capture_output=True,
        env=env,
        timeout=ANSWER_SECONDS + 5.0,
    )
    elapsed = time.monotonic() - started

    log = proc.stderr.decode("utf-8", "replace")
    assert proc.returncode == 0, log
    assert elapsed < ANSWER_SECONDS, f"bridge took {elapsed:.1f} s to answer"
    assert "Enable API Server" in log, log

    replies = {r["id"]: r for r in map(json.loads, proc.stdout.splitlines())}
    assert replies[1]["result"]["capabilities"]["tools"]["listChanged"]
    assert replies[2]["result"]["isError"]
    assert "Enable API Server" in replies[2]["result"]["content"][0]["text"]


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
        server.settimeout(ANSWER_SECONDS)
        conn, _ = server.accept()
        conn.sendall(b'{"frames":[]}\n{"data":"AA=="}\n' + reply)
        time.sleep(1.0)
        conn.close()
        out, err = proc.communicate(input=b"", timeout=ANSWER_SECONDS)
    finally:
        server.close()
        proc.kill()

    assert out == reply, out
    assert b"Dropped 2 non-JSON-RPC" in err, err.decode("utf-8", "replace")
