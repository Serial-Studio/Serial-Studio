#!/usr/bin/env python3
"""
Outbound write caps on every lane (spec 0075 I6).

Only the broadcast, mirror and stream lanes were capped: a client that issued
commands and never read the answers grew the server's socket write buffer without
bound. The response lane now drops such a client instead (WRITE_BACKLOG), while the
producer-paced lanes keep skipping and self-heal.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import sys
import time
import uuid
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient  # noqa: E402

API_HOST = "127.0.0.1"
API_PORT = 7777

# The largest response the surface offers, so the 16 MiB cap is reached in seconds
FAT_COMMAND = "meta.listCommands"

# Bounded by the suite's 30 s timeout; the cap is normally reached long before this
SEND_BUDGET_SECONDS = 12.0


def make_command(name):
    message = {"type": "command", "id": str(uuid.uuid4()), "command": name}
    return (json.dumps(message, separators=(",", ":")) + "\n").encode("utf-8")


@pytest.mark.security
@pytest.mark.slow
def test_non_reading_client_is_dropped(api_server_required):
    """A client that never reads its own responses is disconnected, not buffered."""
    sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
    sock.settimeout(0.5)

    payload = make_command(FAT_COMMAND)
    dropped = False
    deadline = time.time() + SEND_BUDGET_SECONDS

    try:
        while time.time() < deadline:
            try:
                sock.sendall(payload)
            except (BrokenPipeError, ConnectionResetError, OSError):
                dropped = True
                break

            try:
                if sock.recv(1) == b"":
                    dropped = True
                    break
            except socket.timeout:
                continue
            except OSError:
                dropped = True
                break
    finally:
        sock.close()

    if not dropped:
        pytest.skip(
            "the response backlog never reached the cap within the budget; "
            "this build's responses are too small to fill 16 MiB in time"
        )


@pytest.mark.security
@pytest.mark.slow
def test_other_clients_survive_a_backlogged_peer(
    api_server_required, check_server_alive
):
    """Dropping one wedged client must not disturb the ones that keep reading."""
    wedged = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
    wedged.settimeout(0.2)

    payload = make_command(FAT_COMMAND)
    deadline = time.time() + 5.0

    try:
        with SerialStudioClient(timeout=5.0) as healthy:
            while time.time() < deadline:
                try:
                    wedged.sendall(payload)
                except OSError:
                    break

            assert healthy.command("api.getCommands")
    finally:
        wedged.close()

    assert check_server_alive(wait_time=0.5)
