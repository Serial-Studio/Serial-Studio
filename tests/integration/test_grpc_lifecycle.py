#!/usr/bin/env python3
"""
gRPC lifecycle and hygiene (spec 0075 I5/I8).

The gRPC worker parked on a BlockingQueuedConnection into the GUI thread while
stopServer() -- itself on the GUI thread -- waited for those handlers to finish:
toggling the API off with a call in flight was a mutual wait. Handlers now marshal
through an abandonable PendingCall that the stop drains first.

These are the live checks: the port answers, garbage does not take the process down,
and the JSON API keeps serving throughout. The wait/abandon state machine itself is
covered without a server by app/tests/tst_grpc_pending_call.cpp.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import socket
import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient  # noqa: E402

GRPC_HOST = "127.0.0.1"
GRPC_PORT = 8888


def grpc_listening():
    """True when something accepts connections on the gRPC port."""
    try:
        sock = socket.create_connection((GRPC_HOST, GRPC_PORT), timeout=1.0)
    except OSError:
        return False

    sock.close()
    return True


@pytest.fixture(scope="module")
def grpc_required():
    if not grpc_listening():
        pytest.skip(
            "no gRPC server on 127.0.0.1:8888 (build without ENABLE_GRPC, or API off)"
        )

    return True


@pytest.mark.integration
def test_grpc_port_is_open_while_the_api_is_enabled(grpc_required):
    """The gRPC listener follows the API server's enabled state."""
    assert grpc_listening()


@pytest.mark.integration
def test_garbage_on_the_grpc_port_is_survivable(grpc_required):
    """Non-HTTP/2 bytes must not take the server or the process down."""
    for payload in (
        b"\x00" * 64,
        b"GET / HTTP/1.1\r\n\r\n",
        b"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n",
    ):
        sock = socket.create_connection((GRPC_HOST, GRPC_PORT), timeout=5.0)
        try:
            sock.sendall(payload)
            sock.settimeout(0.5)
            try:
                sock.recv(4096)
            except (socket.timeout, OSError):
                pass
        finally:
            sock.close()

    time.sleep(0.5)
    assert grpc_listening(), "the gRPC listener died on malformed input"

    with SerialStudioClient(timeout=5.0) as client:
        assert client.command("api.getCommands")


@pytest.mark.integration
def test_many_short_lived_connections_do_not_wedge_the_gui(grpc_required):
    """Connect/disconnect churn leaves both the gRPC port and the GUI-served API alive."""
    for _ in range(20):
        try:
            sock = socket.create_connection((GRPC_HOST, GRPC_PORT), timeout=2.0)
        except OSError as error:
            pytest.fail(f"the gRPC listener stopped accepting connections: {error}")

        sock.close()

    with SerialStudioClient(timeout=5.0) as client:
        assert client.command("api.getCommands")


@pytest.mark.integration
def test_json_api_stays_responsive_alongside_grpc(grpc_required):
    """The two transports share the GUI thread; neither may starve the other."""
    with SerialStudioClient(timeout=5.0) as client:
        for _ in range(10):
            sock = socket.create_connection((GRPC_HOST, GRPC_PORT), timeout=2.0)
            sock.close()
            assert client.command("api.getCommands")
