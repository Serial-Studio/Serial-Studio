#!/usr/bin/env python3
"""
Loopback binding on both address families (spec 0075 I10).

One QTcpServer binds one address, and the server bound only 127.0.0.1: a client
connecting to "localhost" on a modern system resolves ::1 first and was refused.
The local server now runs two loopback listeners, one per family, feeding one
connection table.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import sys
import uuid
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient  # noqa: E402

API_PORT = 7777


def has_ipv6_loopback():
    """True when this host can open an IPv6 loopback socket at all."""
    if not socket.has_ipv6:
        return False

    probe = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    try:
        probe.bind(("::1", 0))
        return True
    except OSError:
        return False
    finally:
        probe.close()


def command_over(sock, name):
    """Run one command on an already-connected socket and return the response."""
    request_id = str(uuid.uuid4())
    message = {"type": "command", "id": request_id, "command": name}
    sock.sendall((json.dumps(message, separators=(",", ":")) + "\n").encode("utf-8"))

    buffer = b""
    sock.settimeout(5.0)
    while b"\n" not in buffer:
        chunk = sock.recv(65536)
        if not chunk:
            raise ConnectionError("connection closed before a response arrived")

        buffer += chunk

    line, _, _ = buffer.partition(b"\n")
    return json.loads(line)


@pytest.mark.integration
def test_ipv6_loopback_accepts_clients():
    """::1 reaches the same API the IPv4 loopback serves."""
    if not has_ipv6_loopback():
        pytest.skip("this host has no IPv6 loopback")

    try:
        sock = socket.create_connection(("::1", API_PORT), timeout=5.0)
    except OSError as error:
        pytest.fail(f"the API refused an IPv6 loopback client: {error}")

    try:
        response = command_over(sock, "api.getCommands")
    finally:
        sock.close()

    assert response.get("success"), response


@pytest.mark.integration
def test_localhost_name_resolves_to_a_listening_socket():
    """Whatever "localhost" resolves to first on this host, a client gets served."""
    infos = socket.getaddrinfo("localhost", API_PORT, type=socket.SOCK_STREAM)
    assert infos, "localhost does not resolve"

    reached = []
    for family, socktype, proto, _, address in infos:
        sock = socket.socket(family, socktype, proto)
        sock.settimeout(5.0)
        try:
            sock.connect(address)
            reached.append(address[0])
        except OSError:
            continue
        finally:
            sock.close()

    assert reached, f"no localhost address accepted a connection: {infos}"


@pytest.mark.integration
def test_ipv4_loopback_still_works():
    """The second listener is an addition, not a replacement."""
    with SerialStudioClient(host="127.0.0.1", timeout=5.0) as client:
        assert client.command("api.getCommands")
