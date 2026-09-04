#!/usr/bin/env python3
"""
HTTP-shaped traffic on the API socket (spec 0075 I2).

A browser can POST JSON to 127.0.0.1:7777 as a "simple" cross-origin request: no
preflight, no way for the page to read the answer, but the request itself is
delivered. Before this suite the request line and headers arrived as raw device
lines and the body was dispatched as an authenticated command.

The contract now:
  - first bytes that look like an HTTP request line close the connection,
  - nothing readable is written back to that connection,
  - no command in the body runs,
  - raw device forwarding stays locked until one valid JSON message arrived.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import socket
import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError  # noqa: E402

API_HOST = "127.0.0.1"
API_PORT = 7777

HTTP_REQUESTS = [
    b"GET / HTTP/1.1\r\nHost: 127.0.0.1:7777\r\n\r\n",
    b"POST / HTTP/1.1\r\nHost: 127.0.0.1:7777\r\n"
    b"Content-Type: text/plain;charset=UTF-8\r\nContent-Length: 61\r\n\r\n"
    b'{"type":"command","id":"x","command":"project.new","params":{}}\n',
    b"OPTIONS / HTTP/1.1\r\nHost: 127.0.0.1:7777\r\n\r\n",
]


def read_until_closed(sock, budget=2.0):
    """Collect whatever the server sends until it closes, or the budget runs out."""
    sock.settimeout(0.25)
    deadline = time.time() + budget
    received = b""
    while time.time() < deadline:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        except OSError:
            return received, True

        if not chunk:
            return received, True

        received += chunk

    return received, False


@pytest.mark.security
@pytest.mark.parametrize("request_bytes", HTTP_REQUESTS)
def test_http_request_closes_the_connection(api_server_required, request_bytes):
    """Every HTTP verb is refused on the first bytes, with no readable answer."""
    sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
    try:
        sock.sendall(request_bytes)
        received, closed = read_until_closed(sock)
    finally:
        sock.close()

    assert closed, "HTTP request left the API socket open"
    assert received == b"", f"server answered an HTTP request with {received[:120]!r}"


@pytest.mark.security
def test_http_body_command_never_runs(api_server_required, security_client):
    """The command in a browser POST body must not reach the dispatcher."""
    before = security_client.command("project.getStatus").get("title")

    sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
    try:
        sock.sendall(
            b"POST / HTTP/1.1\r\nHost: 127.0.0.1:7777\r\n\r\n"
            b'{"type":"command","id":"x","command":"project.new","params":{}}\n'
        )
        read_until_closed(sock)
    finally:
        sock.close()

    after = security_client.command("project.getStatus").get("title")
    assert after == before, "a command in an HTTP body was executed"


@pytest.mark.security
def test_api_survives_http_probes(api_server_required, check_server_alive):
    """The refusal is a close, not a crash: the API keeps serving other clients."""
    for request_bytes in HTTP_REQUESTS:
        sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
        try:
            sock.sendall(request_bytes)
            read_until_closed(sock, budget=1.0)
        finally:
            sock.close()

    assert check_server_alive(wait_time=0.5)


@pytest.mark.security
def test_raw_forwarding_waits_for_one_json_message(api_server_required):
    """Bytes that are not JSON reach the device only after a valid JSON message."""
    sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
    try:
        sock.sendall(b"AT+RESET\n")
        received, _ = read_until_closed(sock, budget=1.5)
    finally:
        sock.close()

    assert b"INVALID_MESSAGE_TYPE" in received, (
        "a raw line before any JSON message was not refused: " f"{received[:200]!r}"
    )


@pytest.mark.security
def test_normal_clients_are_unaffected(api_server_required):
    """The sniff only looks at the first bytes of a connection, not at every line."""
    with SerialStudioClient(timeout=5.0) as client:
        commands = client.command("api.getCommands")
        assert commands

        try:
            client.command("meta.listCommands")
        except APIError:
            pass
