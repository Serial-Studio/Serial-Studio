"""Spec 0075 E2 -- a Sparkplug host must not renumber its slots when the nodes re-birth.

The inbound slot table was wiped on every broker state change and rebuilt in NBIRTH ARRIVAL
order, and the `sparkplug` native template latches purely by index. A reconnect where node B
births before node A therefore rendered B's metrics under A's dataset titles, silently, with
no counter to say it had happened.

The driver now keeps the slot table across a reset and persists it in the project's connection
block, so the index a dataset is bound to survives both a reconnect and a restart.

Requires the app up with the API server enabled and `mosquitto` on PATH: the reconnect case
needs a broker the test itself can stop and restart, because the slot table is wiped on a broker
STATE CHANGE and the suite's shared broker must survive the run. The Sparkplug payloads are
hand-encoded protobuf on purpose: the encoder under test is the one in the app.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import base64
import shutil
import socket
import struct
import subprocess
import time

import pytest

from utils.api_client import APIError

pytestmark = [
    pytest.mark.integration,
    pytest.mark.mqtt,
    pytest.mark.pro,
    pytest.mark.requires_broker,
]

GROUP_ID = "SS0075"
NODE_A = "edgeA"
NODE_B = "edgeB"

WIRE_VERSION = 1
WIRE_F64 = 10

METRIC_NAME = 1
METRIC_ALIAS = 2
METRIC_DATATYPE = 4
METRIC_DOUBLE = 5
PAYLOAD_TIMESTAMP = 1
PAYLOAD_METRICS = 2
PAYLOAD_SEQ = 5
DATATYPE_DOUBLE = 10


def _varint(value: int) -> bytes:
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        if value:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def _tag(field: int, wire: int) -> bytes:
    return _varint((field << 3) | wire)


def _len_delim(field: int, block: bytes) -> bytes:
    return _tag(field, 2) + _varint(len(block)) + block


def _varint_field(field: int, value: int) -> bytes:
    return _tag(field, 0) + _varint(value)


def _metric(name: str, alias: int, value: float) -> bytes:
    block = _len_delim(METRIC_NAME, name.encode())
    block += _varint_field(METRIC_ALIAS, alias)
    block += _varint_field(METRIC_DATATYPE, DATATYPE_DOUBLE)
    block += _tag(METRIC_DOUBLE, 1) + struct.pack("<d", value)
    return block


def _payload(seq: int, metrics: list) -> bytes:
    out = _varint_field(PAYLOAD_TIMESTAMP, int(time.time() * 1000))
    out += _varint_field(PAYLOAD_SEQ, seq)
    for metric in metrics:
        out += _len_delim(PAYLOAD_METRICS, metric)
    return out


def _decode_delta(frame: bytes) -> dict:
    """Decodes an OpcUaWire delta frame into {wire index: float}; unknown types end the walk."""
    if not frame or frame[0] != WIRE_VERSION:
        return {}

    out = {}
    pos = 1
    while pos + 3 <= len(frame):
        index = frame[pos] | (frame[pos + 1] << 8)
        wire_type = frame[pos + 2]
        pos += 3
        if wire_type != WIRE_F64 or pos + 8 > len(frame):
            return out

        out[index] = struct.unpack("<d", frame[pos : pos + 8])[0]
        pos += 8

    return out


def _sparkplug_project(port: int) -> dict:
    return {
        "title": "Sparkplug host slot stability",
        "groups": [],
        "actions": [],
        "sources": [
            {
                "sourceId": 0,
                "title": "MQTT",
                "busType": 9,
                "frameStart": "",
                "frameEnd": "",
                "checksum": "",
                "frameDetection": 3,
                "decoder": 1,
                "hexadecimalDelimiters": False,
                "frameParserCode": "",
                "frameParserLanguage": 2,
                "frameParserTemplate": "sparkplug",
                "frameParserParams": {"schema": []},
                "connection": {
                    "hostname": "127.0.0.1",
                    "port": port,
                    "topicFilter": "",
                    "sparkplugEnabled": True,
                    "sparkplugGroupId": GROUP_ID,
                    "cleanSession": True,
                    "keepAlive": 60,
                },
            }
        ],
    }


def _publish(publisher, verb: str, node: str, payload: bytes) -> None:
    publisher.publish(
        f"spBv1.0/{GROUP_ID}/{verb}/{node}", payload, qos=0
    ).wait_for_publish()


def _collect_values(api_client, seconds: float) -> dict:
    """Merges every delta frame seen in the window into one {wire index: value} snapshot."""
    merged = {}
    deadline = time.time() + seconds
    seen = set()
    while time.time() < deadline:
        try:
            answer = api_client.command("io.getLatestFrame", {"encoding": "base64"})
        except APIError:
            break

        sequence = answer.get("sequence")
        if answer.get("hasData") and answer.get("base64") and sequence not in seen:
            seen.add(sequence)
            merged.update(_decode_delta(base64.b64decode(answer["base64"])))

        time.sleep(0.02)

    return merged


class CyclingBroker:
    """A broker this test owns, so the session can be cycled the way a real drop cycles it.

    The shared 127.0.0.1:1883 broker belongs to the whole suite and must not be killed, but the
    regression only shows up on a broker state change: that is what wipes the slot table.
    """

    def __init__(self, exe: str, port: int, config: str):
        self._exe = exe
        self._config = config
        self.port = port
        self._proc = None
        self.start()

    def start(self):
        self._proc = subprocess.Popen(
            [self._exe, "-c", self._config],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        deadline = time.time() + 10.0
        while time.time() < deadline:
            if self._port_open():
                return
            if self._proc.poll() is not None:
                pytest.skip("mosquitto exited immediately; cannot cycle a broker here")
            time.sleep(0.1)

        pytest.skip(f"mosquitto did not open port {self.port}")

    def stop(self):
        if self._proc and self._proc.poll() is None:
            self._proc.terminate()
            try:
                self._proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                self._proc.kill()

        deadline = time.time() + 5.0
        while self._port_open() and time.time() < deadline:
            time.sleep(0.1)

    def _port_open(self) -> bool:
        probe = socket.socket()
        probe.settimeout(0.5)
        try:
            probe.connect(("127.0.0.1", self.port))
            return True
        except OSError:
            return False
        finally:
            probe.close()


@pytest.fixture
def cycling_broker(tmp_path):
    exe = shutil.which("mosquitto")
    if exe is None:
        pytest.skip(
            "mosquitto is not on PATH; the reconnect case needs a broker it can cycle"
        )

    port = 48830
    config = tmp_path / "mosquitto.conf"
    config.write_text(f"listener {port} 127.0.0.1\nallow_anonymous true\n")

    broker = CyclingBroker(exe, port, str(config))
    yield broker
    broker.stop()


@pytest.fixture
def sparkplug_publisher(cycling_broker):
    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        pytest.skip("paho-mqtt is not installed")

    clients = []

    def connect():
        client = mqtt.Client(
            client_id=f"ss-sparkplug-{time.time_ns()}",
            callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
        )
        client.connect("127.0.0.1", cycling_broker.port, 30)
        client.loop_start()
        clients.append(client)
        return client

    yield connect

    for client in clients:
        client.loop_stop()
        try:
            client.disconnect()
        except OSError:
            pass


def _disconnect_quietly(api_client):
    """io.disconnect answers EXECUTION_ERROR when no link is open; the fixture wants idle."""
    try:
        if api_client.is_connected():
            api_client.disconnect_device()
    except APIError:
        pass


@pytest.fixture
def sparkplug_session(api_client, cycling_broker):
    """Loads a Sparkplug-enabled MQTT project and opens the link; skips on a GPL build."""
    if not api_client.command_exists("project.mqtt.subscriber.getStatus"):
        pytest.skip("MQTT driver commands not available (Pro feature)")

    _disconnect_quietly(api_client)
    api_client.load_project_from_json(_sparkplug_project(cycling_broker.port))
    api_client.set_operation_mode("project")
    time.sleep(0.3)
    api_client.command("io.connect")
    time.sleep(1.5)

    status = api_client.command("project.mqtt.subscriber.getStatus")
    if not status.get("sparkplug", {}).get("enabled"):
        pytest.skip("the Sparkplug lane is not active on this build")

    yield status
    _disconnect_quietly(api_client)


def test_a_broker_cycle_keeps_every_slot_index(
    api_client, sparkplug_session, sparkplug_publisher, cycling_broker
):
    """The regression: after a broker drop the nodes re-birth in whatever order they reconnect.

    Node A births first, so its metric owns a lower wire index than node B's. The broker is then
    cycled and the two births arrive in the OPPOSITE order; each metric has to land back in the
    index it already owned. Before spec 0075 the session wiped its table on the state change and
    handed index 0 to whoever birthed first, so every dataset bound to those indices showed the
    other node's readings under this node's title.
    """
    publisher = sparkplug_publisher()
    _publish(publisher, "NBIRTH", NODE_A, _payload(0, [_metric("a", 1, 11.0)]))
    time.sleep(0.4)
    _publish(publisher, "NBIRTH", NODE_B, _payload(0, [_metric("b", 1, 22.0)]))
    time.sleep(0.6)

    before = _collect_values(api_client, 2.0)
    index_a = next((i for i, v in before.items() if v == 11.0), None)
    index_b = next((i for i, v in before.items() if v == 22.0), None)
    assert index_a is not None, f"node A's metric never arrived: {before}"
    assert index_b is not None, f"node B's metric never arrived: {before}"
    assert index_a < index_b, "the birth order should have assigned A the lower index"

    status = api_client.command("project.mqtt.subscriber.getStatus")
    metrics_before = status["sparkplug"]["metrics"]

    cycling_broker.stop()
    time.sleep(1.0)
    cycling_broker.start()
    api_client.command("io.connect")
    time.sleep(2.0)

    publisher = sparkplug_publisher()
    _publish(publisher, "NBIRTH", NODE_B, _payload(0, [_metric("b", 7, 222.0)]))
    time.sleep(0.4)
    _publish(publisher, "NBIRTH", NODE_A, _payload(0, [_metric("a", 7, 111.0)]))
    time.sleep(0.6)

    after = _collect_values(api_client, 2.5)
    assert after.get(index_a) == 111.0, (
        f"index {index_a} was node A's metric and now carries {after.get(index_a)}: "
        "the broker cycle renumbered the slot table"
    )
    assert (
        after.get(index_b) == 222.0
    ), f"index {index_b} was node B's metric and now carries {after.get(index_b)}"

    status = api_client.command("project.mqtt.subscriber.getStatus")
    assert (
        status["sparkplug"]["metrics"] == metrics_before
    ), "the reconnect discovered a second copy of the same metric set"


def test_the_session_counters_stay_quiet_on_a_clean_birth(
    api_client, sparkplug_session, sparkplug_publisher
):
    """A well-formed birth followed by in-sequence data raises no hardening counter.

    The counters are the only way a drop is ever visible (spec 0033: pulled, never pushed), so a
    test that never checks them would not notice the session silently discarding traffic.
    """
    publisher = sparkplug_publisher()
    _publish(publisher, "NBIRTH", NODE_A, _payload(0, [_metric("clean", 3, 1.0)]))
    time.sleep(0.5)
    _publish(publisher, "NDATA", NODE_A, _payload(1, [_metric("clean", 3, 2.0)]))
    time.sleep(0.8)

    counters = api_client.command("project.mqtt.subscriber.getStatus")["sparkplug"]
    assert counters["decodeErrors"] == 0
    assert counters["capDrops"] == 0
    assert counters["preBirthDropped"] == 0
    assert counters["unsupportedMetrics"] == 0
    assert counters["metrics"] >= 1
