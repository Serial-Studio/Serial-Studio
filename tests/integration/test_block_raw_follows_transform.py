"""
Spec 0085 AC6 -- a live transform edit re-derives the block layout without dropping a block.

The stream wire carries post-transform values only, so "raw presence follows the transform" is
observed through its consequence: with a doubling transform the streamed values are twice the
parsed ones, and removing the transform brings them back. Continuity is observed on the wire's
own counters: `missed` stays zero and `seq` is monotonic per dataset across both edits, so the
generation bump that rebinds the columns closed the held block instead of dropping it. The
column-level `hasRaw` rule itself is pinned by the C++ unit `tst_frame_builder_staging`.

Requires the app running with the API server on (localhost:7777).
"""

import json
import socket
import time

import pytest

API_HOST = "127.0.0.1"
API_PORT = 7777

_PARSER = """
function parse(frame) {
  return frame.split(",");
}
"""

_DOUBLE_LUA = """
function transform(value)
  return value * 2
end
"""


def _project() -> dict:
    return {
        "title": "Raw follows transform",
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


def _send(sock: socket.socket, payload: dict) -> None:
    sock.sendall(json.dumps(payload).encode() + b"\n")


def _feed(device_simulator, value: float, frames: int) -> None:
    for _ in range(frames):
        device_simulator.send_frame(f"{value},{value + 1}\n".encode())
        time.sleep(0.01)
    time.sleep(0.4)


def _blocks_for(lines: list, unique_id: int) -> list:
    return [
        line["streamBlock"]
        for line in lines
        if "streamBlock" in line and line["streamBlock"].get("uniqueId") == unique_id
    ]


def _dataset_ids(api_client) -> tuple:
    result = api_client.command("project.dataset.getByTitle", {"title": "A"})
    dataset = result.get("dataset", result)
    for key in ("groupId", "datasetId", "uniqueId"):
        assert key in dataset, f"getByTitle result lacks {key}: {result}"
    return dataset["groupId"], dataset["datasetId"], dataset["uniqueId"]


@pytest.mark.integration
@pytest.mark.project
class TestBlockRawFollowsTransform:
    def test_transform_edit_keeps_blocks_continuous(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        _attach(api_client, device_simulator)
        group_id, dataset_id, unique_id = _dataset_ids(api_client)

        sock = socket.create_connection((API_HOST, API_PORT), timeout=5.0)
        try:
            _send(sock, {"type": "command", "id": "sub", "command": "stream.subscribe"})
            time.sleep(0.3)

            _feed(device_simulator, 10.0, 40)
            plain = _blocks_for(_recv_lines(sock, 1.0), unique_id)
            assert plain, "no blocks arrived before the transform edit"

            api_client.command(
                "project.dataset.setTransformCode",
                {
                    "groupId": group_id,
                    "datasetId": dataset_id,
                    "code": _DOUBLE_LUA,
                    "language": 1,
                },
            )
            time.sleep(0.5)
            _feed(device_simulator, 10.0, 40)
            doubled = _blocks_for(_recv_lines(sock, 1.0), unique_id)
            assert doubled, "no blocks arrived after adding the transform"

            api_client.command(
                "project.dataset.setTransformCode",
                {"groupId": group_id, "datasetId": dataset_id, "code": ""},
            )
            time.sleep(0.5)
            _feed(device_simulator, 10.0, 40)
            restored = _blocks_for(_recv_lines(sock, 1.0), unique_id)
            assert restored, "no blocks arrived after removing the transform"
        finally:
            sock.close()

        for block in plain + doubled + restored:
            assert (
                int(block["missed"]) == 0
            ), f"a block was dropped across the edit: {block}"

        seqs = [int(b["seq"]) for b in plain + doubled + restored]
        assert seqs == sorted(seqs), "stream sequence went backwards across the edit"
        assert len(set(seqs)) == len(seqs), "stream sequence repeated across the edit"

        import base64
        import struct

        def last_sample(block) -> float:
            payload = base64.b64decode(block["data"])
            count = int(block["count"])
            return struct.unpack("<%df" % count, payload)[-1]

        assert last_sample(plain[-1]) == pytest.approx(10.0)
        assert last_sample(doubled[-1]) == pytest.approx(20.0)
        assert last_sample(restored[-1]) == pytest.approx(10.0)
