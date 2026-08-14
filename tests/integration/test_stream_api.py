"""
Typed Stream API Tests (spec 0051, M6)

Covers the discovery surface and the connection-scoped subscription verbs:

 * stream.getInfo / stream.getSources answer and describe the surface
   (encoding, queue depth, the connection-scoped command names).
 * stream.subscribe / stream.unsubscribe are accepted on a live socket and
   report their per-connection state; unsubscribing twice is an error.
 * AC20 (audio-gated) -- with a loopback capture device, a subscriber
   receives streamBlock lines carrying seq/missed/t0Ms/dtNs/count and a
   base64 float32le payload, while a second client keeps polling normally.

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import base64
import json
import os
import socket
import struct
import time

import pytest

API_HOST = "127.0.0.1"
API_PORT = 7777


def _recv_lines(sock: socket.socket, seconds: float) -> list:
    """Collects decoded NDJSON objects arriving within `seconds`."""
    sock.settimeout(0.4)
    buffer = b""
    lines = []
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        try:
            chunk = sock.recv(65536)
        except socket.timeout:
            continue
        except OSError:
            break

        if not chunk:
            break

        buffer += chunk
        while b"\n" in buffer:
            raw, buffer = buffer.split(b"\n", 1)
            raw = raw.strip()
            if not raw:
                continue

            try:
                lines.append(json.loads(raw.decode()))
            except ValueError:
                continue

    return lines


def _send(sock: socket.socket, payload: dict) -> None:
    sock.sendall(json.dumps(payload).encode() + b"\n")


@pytest.mark.integration
class TestStreamApiSurface:
    def test_get_info_describes_surface(self, api_client):
        result = api_client.command("stream.getInfo", {})
        assert result.get("encoding") == "base64:float32le"
        assert result.get("subscribeCommand") == "stream.subscribe"
        assert result.get("unsubscribeCommand") == "stream.unsubscribe"
        assert result.get("pushLine") == "streamBlock"
        assert int(result.get("queueDepth", 0)) > 0

    def test_get_sources_answers(self, api_client):
        result = api_client.command("stream.getSources", {})
        assert "sources" in result
        assert isinstance(result["sources"], list)
        assert result.get("count") == len(result["sources"])

    def test_subscribe_and_unsubscribe_round_trip(self):
        """The verbs are connection-scoped, so this drives its own socket."""
        sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
        try:
            _send(
                sock,
                {
                    "type": "command",
                    "id": "sub-1",
                    "command": "stream.subscribe",
                    "params": {"sources": [0]},
                },
            )
            replies = _recv_lines(sock, 2.0)
            subscribed = [
                r for r in replies if r.get("id") == "sub-1" and r.get("success")
            ]
            assert subscribed, f"stream.subscribe was not accepted: {replies}"
            assert subscribed[0]["result"]["subscribed"] is True

            _send(
                sock,
                {"type": "command", "id": "unsub-1", "command": "stream.unsubscribe"},
            )
            replies = _recv_lines(sock, 2.0)
            done = [r for r in replies if r.get("id") == "unsub-1"]
            assert done and done[0].get("success"), f"unsubscribe failed: {replies}"
            assert done[0]["result"]["subscribed"] is False

            _send(
                sock,
                {"type": "command", "id": "unsub-2", "command": "stream.unsubscribe"},
            )
            replies = _recv_lines(sock, 2.0)
            second = [r for r in replies if r.get("id") == "unsub-2"]
            assert second and not second[0].get(
                "success"
            ), "unsubscribing without a subscription must fail"
        finally:
            sock.close()


@pytest.mark.audio
class TestStreamBlockDelivery:
    def test_subscriber_receives_blocks(self, api_client, clean_state):
        """AC20: a subscriber receives well-formed post-transform blocks."""
        if not os.environ.get("SS_AUDIO_CAPTURE_MATCH"):
            pytest.skip("no loopback capture device configured")

        api_client.command("app.setOperationMode", {"mode": 2})
        api_client.command("io.setBusType", {"busType": 3})
        time.sleep(0.3)
        api_client.connect_device()
        time.sleep(1.5)

        sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
        try:
            _send(sock, {"type": "command", "id": "sub", "command": "stream.subscribe"})
            lines = _recv_lines(sock, 5.0)
            blocks = [line["streamBlock"] for line in lines if "streamBlock" in line]

            assert blocks, "subscriber received no streamBlock lines"

            first = blocks[0]
            for key in (
                "sourceId",
                "uniqueId",
                "seq",
                "missed",
                "t0Ms",
                "dtNs",
                "count",
                "data",
            ):
                assert key in first, f"streamBlock is missing {key}"

            payload = base64.b64decode(first["data"])
            assert len(payload) == int(first["count"]) * 4
            samples = struct.unpack("<%df" % int(first["count"]), payload)
            assert len(samples) == int(first["count"])

            seqs = [b["seq"] for b in blocks]
            assert seqs == sorted(seqs), "block sequence numbers went backwards"

            # dashboard.getData keeps answering while the subscription streams
            assert api_client.get_dashboard_data() is not None
        finally:
            sock.close()
            api_client.disconnect_device()
