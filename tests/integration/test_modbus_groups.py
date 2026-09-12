"""Spec 0075 E3/E12 -- a failed Modbus poll must not move later frames onto another group.

The generated Lua parser infers a reply's register group by COUNTING frames, so a poll the
driver silently skipped attributed every later frame to the wrong group for the rest of the
session: a dashboard full of plausible, wrong readings with nothing to say they moved. The
driver now publishes a zero-length placeholder ``[unit, fc, 0, crc, crc]`` for the group whose
poll failed, and the frame is a real RTU frame with a checksum.

Needs the app up with the API server enabled (see tests/README.md). The Modbus server here is
a few dozen lines of socket code on purpose: pulling in a Modbus library would make the test
depend on that library's idea of the protocol rather than on the bytes the driver sees.

Frames are observed through the typed stream (stream.subscribe), one sample per parsed frame:
the two replies of a poll cycle land microseconds apart, so a poll of io.getLatestFrame only
ever sees the second one. A JavaScript parser reduces each RTU frame to the four numbers the
assertions need; the CRC is checked in the parser, where the raw bytes are.
"""

import base64
import json
import socket
import struct
import threading
import time

import pytest

from utils.api_client import APIError

pytestmark = [pytest.mark.integration, pytest.mark.pro]

GROUP_A = (0, 0, 2)  # holding registers @ 0, 2 registers  -> 4 payload bytes
GROUP_B = (0, 100, 3)  # holding registers @ 100, 3 registers -> 6 payload bytes

POLL_INTERVAL_MS = 1000
OBSERVE_S = 12.0

# One row per frame: [unit, function, declared payload bytes, CRC-16/Modbus verdict]. The length
# guard is what lets the editor's save-time probe ("0", [0], "") pass.
_FRAME_PARSER = """
function crc16(bytes, length) {
  var crc = 0xFFFF;
  for (var i = 0; i < length; i++) {
    crc ^= bytes[i];
    for (var bit = 0; bit < 8; bit++)
      crc = (crc & 1) ? ((crc >>> 1) ^ 0xA001) : (crc >>> 1);
  }
  return crc;
}

function parse(frame) {
  if (frame.length < 5)
    return [];

  var n = frame.length;
  var crc = crc16(frame, n - 2);
  var ok = ((crc & 0xFF) === frame[n - 2]) && (((crc >>> 8) & 0xFF) === frame[n - 1]);
  return [frame[0], frame[1], frame[2], ok ? 1 : 0];
}
"""

COLUMNS = ("unit", "function", "payload", "crcOk")


def _modbus_project(port: int) -> dict:
    """A single-source Modbus TCP project whose parser reduces each RTU frame to COLUMNS.

    Project mode with no delimiters is what passes a driver chunk through whole; under any
    other detection mode the binary bytes never close a frame.
    """
    return {
        "title": "Modbus group attribution",
        "groups": [
            {
                "title": "Frames",
                "widget": "datagrid",
                "datasets": [
                    {"title": title, "index": index, "widget": ""}
                    for index, title in enumerate(COLUMNS, start=1)
                ],
            }
        ],
        "actions": [],
        "sources": [
            {
                "sourceId": 0,
                "title": "Modbus",
                "busType": 4,
                "frameStart": "",
                "frameEnd": "",
                "checksum": "",
                "frameDetection": 2,
                "decoder": 3,
                "hexadecimalDelimiters": False,
                "frameParserCode": _FRAME_PARSER,
                "frameParserLanguage": 0,
                "connection": {
                    "protocolIndex": 1,
                    "host": "127.0.0.1",
                    "port": port,
                    "slaveAddress": 1,
                    "pollInterval": POLL_INTERVAL_MS,
                    "registerGroups": [
                        {"type": kind, "start": start, "count": count, "slave": 0}
                        for kind, start, count in (GROUP_A, GROUP_B)
                    ],
                },
            }
        ],
    }


class ModbusTcpServer:
    """A two-group Modbus TCP server that can drop one group's reply on demand.

    Answers FC03 for both groups and, once ``drop_group_a`` is armed, never answers group A
    until the driver has given up on that poll and moved on to group B: the client retries a
    silent request several times, and answering a retry would make the poll succeed after all.
    A dropped reply is what a busy PLC, a gateway hiccup or a unit that briefly stops responding
    looks like on the wire. The connection is held open across idle poll intervals; a server
    that hung up between polls was the first thing the driver ever saw.
    """

    def __init__(self):
        self._srv = socket.socket()
        self._srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._srv.bind(("127.0.0.1", 0))
        self._srv.listen(4)
        self.port = self._srv.getsockname()[1]
        self.requests = []
        self.writes = []
        self.accepts = 0
        self.drop_group_a = False
        self.dropped = 0
        self._running = True
        threading.Thread(target=self._accept_loop, daemon=True).start()

    def _accept_loop(self):
        while self._running:
            try:
                conn, _ = self._srv.accept()
            except OSError:
                return
            self.accepts += 1
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
                if fc in (0x06, 0x10):
                    self.writes.append((unit, fc, start))
                    pdu = body[:5]
                    conn.sendall(struct.pack(">HHHB", txn, 0, len(pdu) + 1, unit) + pdu)
                    continue

                self.requests.append((start, count))
                if fc != 0x03:
                    continue

                if self.drop_group_a and start == GROUP_A[1]:
                    self.dropped += 1
                    continue

                if self.drop_group_a and self.dropped and start == GROUP_B[1]:
                    self.drop_group_a = False

                payload = b"".join(struct.pack(">H", start + i) for i in range(count))
                pdu = struct.pack(">BB", fc, len(payload)) + payload
                conn.sendall(struct.pack(">HHHB", txn, 0, len(pdu) + 1, unit) + pdu)
        except OSError:
            return
        finally:
            conn.close()

    def _recv_exact(self, conn, size):
        """Reads ``size`` bytes, or None once the peer hung up or the server stopped.

        A receive timeout with nothing buffered is an idle poll interval, not a hang-up.
        """
        out = b""
        while len(out) < size:
            try:
                chunk = conn.recv(size - len(out))
            except socket.timeout:
                if out or not self._running:
                    return None
                continue
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


def _column_ids(api_client) -> dict:
    """{dataset uniqueId: column name} for the loaded project, the key a streamBlock line carries."""
    config = api_client.command("project.exportJson")["config"]
    ids = {}
    for group in config.get("groups", []):
        for dataset in group.get("datasets", []):
            if dataset.get("title") in COLUMNS:
                ids[dataset["uniqueId"]] = dataset["title"]

    assert len(ids) == len(COLUMNS), f"parser columns not found in the project: {ids}"
    return ids


def collect_rows(api_client, seconds: float) -> list:
    """Every frame the parser saw in the window, in order, as {column: value} rows.

    Reads streamBlock lines off a subscription socket of its own: a block is one line per
    column, and the column lines of one block share a seq.
    """
    column_ids = _column_ids(api_client)
    blocks = {}
    sock = socket.create_connection((api_client.host, api_client.port), timeout=5.0)
    try:
        sock.sendall(
            (
                json.dumps(
                    {"type": "command", "id": "sub", "command": "stream.subscribe"}
                )
                + "\n"
            ).encode()
        )
        sock.settimeout(0.2)
        pending = b""
        deadline = time.time() + seconds
        while time.time() < deadline:
            try:
                chunk = sock.recv(65536)
            except socket.timeout:
                continue
            if not chunk:
                break

            pending += chunk
            while b"\n" in pending:
                line, pending = pending.split(b"\n", 1)
                entry = json.loads(line).get("streamBlock") if line.strip() else None
                if not entry or entry["uniqueId"] not in column_ids:
                    continue

                raw = base64.b64decode(entry["data"])
                values = struct.unpack(f"<{entry['count']}f", raw[: 4 * entry["count"]])
                blocks.setdefault(entry["seq"], {})[
                    column_ids[entry["uniqueId"]]
                ] = values
    finally:
        sock.close()

    rows = []
    for seq in sorted(blocks):
        columns = blocks[seq]
        if len(columns) != len(COLUMNS):
            continue

        for i in range(min(len(v) for v in columns.values())):
            rows.append({name: int(round(columns[name][i])) for name in COLUMNS})

    return rows


def _disconnect_quietly(api_client):
    """io.disconnect answers EXECUTION_ERROR when no link is open; the fixture wants idle."""
    try:
        if api_client.is_connected():
            api_client.disconnect_device()
    except APIError:
        pass


@pytest.fixture
def modbus_server():
    server = ModbusTcpServer()
    yield server
    server.stop()


@pytest.fixture
def modbus_session(api_client, modbus_server):
    """Loads a project whose Modbus source points at the stub with two differently sized groups.

    The source block is pushed through project.source.setProperties as well: an unsaved project
    is not mirrored onto the UI-config driver on load, and the live driver is built from that.
    """
    if not api_client.command_exists("io.modbus.getConfig"):
        pytest.skip("Modbus driver commands not available (Pro feature)")

    _disconnect_quietly(api_client)
    project = _modbus_project(modbus_server.port)
    api_client.load_project_from_json(project)
    api_client.set_operation_mode("project")
    api_client.set_bus_type("modbus")
    api_client.source_configure(0, project["sources"][0]["connection"])
    time.sleep(0.3)
    api_client.command("io.connect")
    time.sleep(1.0)
    yield modbus_server
    _disconnect_quietly(api_client)


def test_every_published_frame_is_a_valid_rtu_frame(api_client, modbus_session):
    """The driver publishes RTU frames, so they carry a checksum and the responding unit id.

    Before spec 0075 the bytes were ``[unit, fc, byteCount, ...data]`` with no CRC at all: a
    header-shaped fragment that any consumer validating the checksum rejects.
    """
    rows = collect_rows(api_client, 4.0)
    assert rows, (
        "no frames arrived from the Modbus stub server "
        f"(accepted connections: {modbus_session.accepts}, "
        f"requests seen: {len(modbus_session.requests)}, "
        f"driver port: {api_client.command('io.modbus.getConfig').get('port')})"
    )

    for number, row in enumerate(rows):
        assert row["crcOk"] == 1, f"frame {number} carries a wrong CRC-16/Modbus: {row}"
        assert (
            row["unit"] == 1
        ), f"frame {number} does not carry the responding unit id: {row}"
        assert (
            row["function"] == 0x03
        ), f"frame {number} lost the request's function code: {row}"
        assert row["payload"] in (
            4,
            6,
        ), f"frame {number} has an unexpected payload size: {row}"


def test_a_dropped_reply_keeps_group_attribution(api_client, modbus_session):
    """The regression: one dropped reply must not shift every later frame onto another group.

    Group A answers 4 payload bytes and group B answers 6, so the byte count identifies the
    group a frame belongs to. With the reply to one group-A poll dropped, the driver publishes
    a zero-length placeholder in its place, and the A, B, A, B cycle is unbroken. Without it,
    two group-B frames arrive back to back and every dataset after that point is misfiled.
    """
    modbus_session.drop_group_a = True

    rows = collect_rows(api_client, OBSERVE_S)
    assert len(rows) >= 4, (
        f"expected several poll cycles, saw {len(rows)} frames "
        f"(accepted connections: {modbus_session.accepts}, "
        f"requests seen: {len(modbus_session.requests)}, dropped: {modbus_session.dropped})"
    )

    assert all(
        row["crcOk"] == 1 for row in rows
    ), "a published frame was not a valid RTU frame"
    sizes = [row["payload"] for row in rows]
    assert set(sizes) <= {0, 4, 6}, f"unexpected payload sizes: {sorted(set(sizes))}"
    assert 0 in sizes, f"the failed poll published no placeholder frame: {sizes}"

    for number, (first, second) in enumerate(zip(sizes, sizes[1:])):
        assert not (first == 6 and second == 6), (
            f"frames {number}/{number + 1} are both group B: "
            f"a skipped poll shifted the group cursor ({sizes})"
        )

    starts = [start for start, _count in modbus_session.requests]
    assert GROUP_A[1] in starts and GROUP_B[1] in starts


# ---------------------------------------------------------------------------
# Writes name their unit (spec 0083 AC7)
# ---------------------------------------------------------------------------

# Lua decimal escapes: "\2\0\10\0\42" is five bytes, so byte 0 (2) is the target unit and the
# write lands on unit 2 register 10; "\0\11\0\43" is four bytes and goes to the connection's
# own unit (1), register 11. Fired once, on the first frame the parser hands the transform.
_WRITE_TRANSFORM = (
    "local fired = false\n"
    "function transform(v)\n"
    "  if not fired then\n"
    "    fired = true\n"
    '    deviceWrite("\\255\\131\\2\\0\\10\\0\\42")\n'
    '    deviceWrite("\\0\\11\\0\\43")\n'
    "  end\n"
    "  return v\n"
    "end\n"
)


@pytest.fixture
def modbus_write_session(api_client, modbus_server):
    """The two-group project with a Lua transform on the unit column that writes two registers."""
    if not api_client.command_exists("io.modbus.getConfig"):
        pytest.skip("Modbus driver commands not available (Pro feature)")

    _disconnect_quietly(api_client)
    project = _modbus_project(modbus_server.port)
    project["groups"][0]["datasets"][0]["transformLanguage"] = 1
    project["groups"][0]["datasets"][0]["transformCode"] = _WRITE_TRANSFORM
    api_client.load_project_from_json(project)
    api_client.set_operation_mode("project")
    api_client.set_bus_type("modbus")
    api_client.source_configure(0, project["sources"][0]["connection"])
    time.sleep(0.3)
    api_client.command("io.connect")
    time.sleep(1.0)
    yield modbus_server
    _disconnect_quietly(api_client)


def test_a_unit_prefixed_write_targets_that_unit(api_client, modbus_write_session):
    """A prefixed payload names its unit; a plain one still goes to the connection's unit."""
    deadline = time.time() + 6.0
    while time.time() < deadline and len(modbus_write_session.writes) < 2:
        time.sleep(0.2)

    writes = list(modbus_write_session.writes)
    assert writes, "the transform's deviceWrite never reached the stub server"
    assert (
        2,
        0x06,
        10,
    ) in writes, f"unit-prefixed write missing or on the wrong unit: {writes}"
    assert (1, 0x06, 11) in writes, f"plain write left the connection's unit: {writes}"
