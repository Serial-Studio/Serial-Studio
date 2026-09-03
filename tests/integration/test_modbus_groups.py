"""Spec 0075 E3/E12 -- a failed Modbus poll must not move later frames onto another group.

The generated Lua parser infers a reply's register group by COUNTING frames, so a poll the
driver silently skipped attributed every later frame to the wrong group for the rest of the
session: a dashboard full of plausible, wrong readings with nothing to say they moved. The
driver now publishes a zero-length placeholder ``[unit, fc, 0, crc, crc]`` for the group whose
poll failed, and the frame is a real RTU frame with a checksum.

Needs the app up with the API server enabled (see tests/README.md). The Modbus server here is
a few dozen lines of socket code on purpose: pulling in a Modbus library would make the test
depend on that library's idea of the protocol rather than on the bytes the driver sees.
"""

import base64
import socket
import struct
import threading
import time

import pytest

pytestmark = [pytest.mark.integration, pytest.mark.pro]

GROUP_A = (0, 0, 2)  # holding registers @ 0, 2 registers  -> 4 payload bytes
GROUP_B = (0, 100, 3)  # holding registers @ 100, 3 registers -> 6 payload bytes

POLL_INTERVAL_MS = 1000
OBSERVE_S = 12.0
API_POLL_S = 0.02


def modbus_crc(frame: bytes) -> int:
    """CRC-16/Modbus of ``frame``, the value the driver appends low octet first."""
    crc = 0xFFFF
    for octet in frame:
        crc ^= octet
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc


class ModbusTcpServer:
    """A two-group Modbus TCP server that can drop one group's reply on demand.

    Answers FC03 for both groups and, once ``drop_group_a`` is armed, never answers the next
    request whose starting address is group A's. A dropped reply is what a busy PLC, a gateway
    hiccup or a unit that briefly stops responding looks like on the wire.
    """

    def __init__(self):
        self._srv = socket.socket()
        self._srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._srv.bind(("127.0.0.1", 0))
        self._srv.listen(4)
        self.port = self._srv.getsockname()[1]
        self.requests = []
        self.drop_group_a = False
        self._running = True
        threading.Thread(target=self._accept_loop, daemon=True).start()

    def _accept_loop(self):
        while self._running:
            try:
                conn, _ = self._srv.accept()
            except OSError:
                return
            threading.Thread(target=self._serve, args=(conn,), daemon=True).start()

    def _serve(self, conn):
        conn.settimeout(0.5)
        try:
            while self._running:
                header = self._recv_exact(conn, 7)
                if header is None:
                    return

                txn, _proto, length, unit = struct.unpack(">HHHB", header)
                body = self._recv_exact(conn, max(0, length - 1))
                if body is None or len(body) < 5:
                    return

                fc, start, count = struct.unpack(">BHH", body[:5])
                self.requests.append((start, count))
                if fc != 0x03:
                    continue

                if self.drop_group_a and start == GROUP_A[1]:
                    self.drop_group_a = False
                    continue

                payload = b"".join(struct.pack(">H", start + i) for i in range(count))
                pdu = struct.pack(">BB", fc, len(payload)) + payload
                conn.sendall(struct.pack(">HHHB", txn, 0, len(pdu) + 1, unit) + pdu)
        except OSError:
            return
        finally:
            conn.close()

    @staticmethod
    def _recv_exact(conn, size):
        out = b""
        while len(out) < size:
            try:
                chunk = conn.recv(size - len(out))
            except socket.timeout:
                return None
            except OSError:
                return None
            if not chunk:
                return None
            out += chunk
        return out

    def stop(self):
        self._running = False
        try:
            self._srv.close()
        except OSError:
            pass


def collect_frames(api_client, seconds: float) -> dict:
    """Polls io.getLatestFrame and returns the distinct raw frames it saw, keyed by sequence."""
    seen = {}
    deadline = time.time() + seconds
    while time.time() < deadline:
        answer = api_client.command("io.getLatestFrame", {"encoding": "base64"})
        if answer.get("hasData") and answer.get("base64"):
            seen[int(answer["sequence"])] = base64.b64decode(answer["base64"])
        time.sleep(API_POLL_S)

    return seen


def describe(frame: bytes) -> int:
    """The declared payload byte count of an RTU frame, or -1 when it is not one."""
    if len(frame) < 5:
        return -1
    if modbus_crc(frame[:-2]) != frame[-2] | (frame[-1] << 8):
        return -1
    return frame[2]


@pytest.fixture
def modbus_server():
    server = ModbusTcpServer()
    yield server
    server.stop()


@pytest.fixture
def modbus_session(api_client, modbus_server):
    """Points the Modbus driver at the stub server with two differently sized groups."""
    if not api_client.command_exists("io.modbus.getConfig"):
        pytest.skip("Modbus driver commands not available (Pro feature)")

    api_client.command("io.disconnect")
    api_client.set_bus_type("modbus")
    api_client.command("io.modbus.setProtocolIndex", {"protocolIndex": 1})
    api_client.command("io.modbus.setHost", {"host": "127.0.0.1"})
    api_client.command("io.modbus.setPort", {"port": modbus_server.port})
    api_client.command("io.modbus.setSlaveAddress", {"slaveAddress": 1})
    api_client.command("io.modbus.setPollInterval", {"pollInterval": POLL_INTERVAL_MS})
    api_client.command("io.modbus.clearRegisterGroups")
    for register_type, start, count in (GROUP_A, GROUP_B):
        api_client.command(
            "io.modbus.addRegisterGroup",
            {"type": register_type, "startAddress": start, "count": count},
        )

    api_client.command("io.connect")
    time.sleep(1.0)
    yield modbus_server
    api_client.command("io.disconnect")
    api_client.command("io.modbus.clearRegisterGroups")


def test_every_published_frame_is_a_valid_rtu_frame(api_client, modbus_session):
    """The driver publishes RTU frames, so they carry a checksum and the responding unit id.

    Before spec 0075 the bytes were ``[unit, fc, byteCount, ...data]`` with no CRC at all: a
    header-shaped fragment that any consumer validating the checksum rejects.
    """
    frames = collect_frames(api_client, 4.0)
    assert frames, "no frames arrived from the Modbus stub server"

    for sequence, frame in sorted(frames.items()):
        assert len(frame) >= 5, f"frame {sequence} is too short to carry a checksum"
        assert modbus_crc(frame[:-2]) == frame[-2] | (
            frame[-1] << 8
        ), f"frame {sequence} carries a wrong CRC-16/Modbus"
        assert frame[0] == 1, f"frame {sequence} does not carry the responding unit id"
        assert (
            frame[1] == 0x03
        ), f"frame {sequence} does not carry the request's function code"


def test_a_dropped_reply_keeps_group_attribution(api_client, modbus_session):
    """The regression: one dropped reply must not shift every later frame onto another group.

    Group A answers 4 payload bytes and group B answers 6, so the byte count identifies the
    group a frame belongs to. With the reply to one group-A poll dropped, the driver publishes
    a zero-length placeholder in its place, and the A, B, A, B cycle is unbroken. Without it,
    two group-B frames arrive back to back and every dataset after that point is misfiled.
    """
    modbus_session.drop_group_a = True

    frames = collect_frames(api_client, OBSERVE_S)
    assert len(frames) >= 4, f"expected several poll cycles, saw {len(frames)}"

    counts = {sequence: describe(frame) for sequence, frame in frames.items()}
    assert -1 not in counts.values(), "a published frame was not a valid RTU frame"
    sizes = sorted(set(counts.values()))
    assert set(sizes) <= {0, 4, 6}, f"unexpected payload sizes: {sizes}"
    assert 0 in counts.values(), "the failed poll published no placeholder frame"

    ordered = sorted(counts.items())
    for (first_seq, first), (second_seq, second) in zip(ordered, ordered[1:]):
        if second_seq != first_seq + 1:
            continue

        assert not (first == 6 and second == 6), (
            f"frames {first_seq}/{second_seq} are both group B: "
            "a skipped poll shifted the group cursor"
        )

    starts = [start for start, _count in modbus_session.requests]
    assert GROUP_A[1] in starts and GROUP_B[1] in starts
