"""
Spec 0086 AC5/AC6 -- per-dataset table capture is scoped to the scripts that can read it.

The store's dataset mirror registers (`__datasets__/raw:<uniqueId>`) are readable over the API
without arming capture (`project.dataTable.getValue` is a plain reader), so they are the
observable: in a parser-only project they never move while frames flow; a control script that
names `tableGet` arms capture and they track the parsed values; clearing the script stops the
tracking. A transform that names `datasetGetRaw` arms capture on its own and reads the live raw
value of another dataset within the same frame, observed on the stream wire.

Clearing the script or the transform is a project edit, and a project edit rebuilds the store
(FrameBuilder::applyProjectSnapshot), so a register may read its default again afterwards; the
disarm assertions therefore check that the register no longer follows the frames, not that it
kept its last value.

Requires the app running with the API server on (localhost:7777).
"""

import base64
import json
import socket
import struct
import time

import pytest

API_HOST = "127.0.0.1"
API_PORT = 7777
SYSTEM_TABLE = "__datasets__"

_PARSER = """
function parse(frame) {
  return frame.split(",");
}
"""

_CONTROL_SCRIPT = """
function setup() {}
function loop() {
  tableGet("%s", "%s");
}
"""

_READ_RAW_OF_A_LUA = """
function transform(value)
  return datasetGetRaw(%d) or value
end
"""


def _project() -> dict:
    return {
        "title": "Capture scoping",
        "frameEnd": "\\n",
        "frameDetection": 0,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 0,
                "frameStart": "",
                "frameEnd": "\\n",
                "frameDetection": 0,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": _PARSER,
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {"title": "A", "value": "%1", "index": 1, "graph": True},
                    {"title": "B", "value": "%2", "index": 2, "graph": True},
                ],
            }
        ],
        "actions": [],
    }


def _attach(api_client, device_simulator):
    try:
        api_client.disconnect_device()
        time.sleep(0.6)
    except Exception:
        pass
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except Exception:
        pass
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _feed(device_simulator, a: float, b: float, frames: int) -> None:
    for _ in range(frames):
        device_simulator.send_frame(f"{a},{b}\n".encode())
        time.sleep(0.01)
    time.sleep(0.4)


def _dataset(api_client, title: str) -> dict:
    result = api_client.command("project.dataset.getByTitle", {"title": title})
    dataset = result.get("dataset", result)
    for key in ("groupId", "datasetId", "uniqueId"):
        assert key in dataset, f"getByTitle result lacks {key}: {result}"
    return dataset


def _raw_register(api_client, unique_id: int) -> tuple:
    result = api_client.command(
        "project.dataTable.getValue",
        {"table": SYSTEM_TABLE, "name": f"raw:{unique_id}"},
    )
    return (result.get("isNumeric"), result.get("value"))


def _recv_lines(sock: socket.socket, seconds: float) -> list:
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


def _last_streamed(lines: list, unique_id: int) -> float:
    blocks = [
        line["streamBlock"]
        for line in lines
        if "streamBlock" in line and line["streamBlock"].get("uniqueId") == unique_id
    ]
    assert blocks, f"no stream blocks for dataset {unique_id}"
    block = blocks[-1]
    payload = base64.b64decode(block["data"])
    count = int(block["count"])
    return struct.unpack("<%df" % count, payload)[-1]


@pytest.mark.integration
@pytest.mark.project
class TestTableCaptureScoping:
    def test_parser_only_project_never_writes_the_store(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        api_client.command("controlScript.set", {"code": ""})
        _attach(api_client, device_simulator)
        unique_a = _dataset(api_client, "A")["uniqueId"]

        before = _raw_register(api_client, unique_a)
        _feed(device_simulator, 10.0, 20.0, 60)
        after = _raw_register(api_client, unique_a)
        assert (
            after == before
        ), f"parser-only project wrote the store: {before} -> {after}"

    def test_control_script_arms_and_disarms_capture(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        api_client.command("controlScript.set", {"code": ""})
        _attach(api_client, device_simulator)
        unique_a = _dataset(api_client, "A")["uniqueId"]

        api_client.command(
            "controlScript.set",
            {"code": _CONTROL_SCRIPT % (SYSTEM_TABLE, f"raw:{unique_a}")},
        )
        time.sleep(0.5)
        _feed(device_simulator, 11.0, 21.0, 40)
        armed = _raw_register(api_client, unique_a)
        assert armed == (True, 11.0), f"control script did not arm capture: {armed}"

        api_client.command("controlScript.set", {"code": ""})
        time.sleep(0.5)
        _feed(device_simulator, 12.0, 22.0, 40)
        frozen = _raw_register(api_client, unique_a)
        assert frozen != (
            True,
            12.0,
        ), f"capture kept running after the script stopped: {frozen}"

    def test_transform_reading_the_store_arms_capture(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        api_client.command("controlScript.set", {"code": ""})
        _attach(api_client, device_simulator)
        dataset_a = _dataset(api_client, "A")
        dataset_b = _dataset(api_client, "B")
        unique_a = dataset_a["uniqueId"]
        unique_b = dataset_b["uniqueId"]

        sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
        try:
            sock.sendall(
                json.dumps(
                    {"type": "command", "id": "sub", "command": "stream.subscribe"}
                ).encode()
                + b"\n"
            )
            time.sleep(0.3)

            _feed(device_simulator, 10.0, 20.0, 40)
            assert _last_streamed(_recv_lines(sock, 1.0), unique_b) == pytest.approx(
                20.0
            )

            api_client.command(
                "project.dataset.setTransformCode",
                {
                    "groupId": dataset_b["groupId"],
                    "datasetId": dataset_b["datasetId"],
                    "code": _READ_RAW_OF_A_LUA % unique_a,
                    "language": 1,
                },
            )
            time.sleep(0.5)
            _feed(device_simulator, 10.0, 20.0, 40)
            assert _last_streamed(_recv_lines(sock, 1.0), unique_b) == pytest.approx(
                10.0
            )
            assert _raw_register(api_client, unique_a) == (True, 10.0)

            api_client.command(
                "project.dataset.setTransformCode",
                {
                    "groupId": dataset_b["groupId"],
                    "datasetId": dataset_b["datasetId"],
                    "code": "",
                },
            )
            time.sleep(0.5)
            _feed(device_simulator, 13.0, 23.0, 40)
            assert _last_streamed(_recv_lines(sock, 1.0), unique_b) == pytest.approx(
                23.0
            )
            stopped = _raw_register(api_client, unique_a)
            assert stopped != (
                True,
                13.0,
            ), f"capture kept running after the transform was removed: {stopped}"
        finally:
            sock.close()
