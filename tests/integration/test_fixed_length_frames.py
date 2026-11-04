"""
Fixed Length framing over TCP (spec 0103, ACs 9 and 10)

A TCP device streams 5-byte binary frames: the first byte carries a header bit (bit 7 set),
the four payload bytes keep bit 7 clear. The source is configured as Fixed Length with a
Header Rule (mask 0x80, value 0x80) and a Body Rule (mask 0x80, value 0x00), so the reader can
re-align after a stray byte. The tests check the dataset values, the resync, that Quick Plot
ignores the mode, and that an unusable length is repaired to No Delimiters.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

FIXED_LENGTH = 4
NO_DELIMITERS = 2

_PARSER = """
function parse(frame) {
    return [frame[1], frame[2], frame[3], frame[4]];
}
"""


def _frame(tag: int, a: int, b: int, c: int, d: int) -> bytes:
    """One 5-byte frame: header bit set on the first byte, clear on the rest."""
    return bytes([0x80 | tag, a, b, c, d])


def _project(frame_length: int = 5) -> dict:
    """A Network-source project with four datasets and a Fixed Length binary parser."""
    return {
        "title": "Fixed Length",
        "sources": [
            {
                "sourceId": 0,
                "title": "Device",
                "busType": 1,
                "frameStart": "",
                "frameEnd": "",
                "checksum": "",
                "frameDetection": FIXED_LENGTH,
                "frameLength": frame_length,
                "frameHeaderMask": 0x80,
                "frameHeaderValue": 0x80,
                "frameBodyMask": 0x80,
                "frameBodyValue": 0,
                "decoder": 3,
                "hexadecimalDelimiters": False,
                "frameParserCode": _PARSER,
                "frameParserLanguage": 0,
            }
        ],
        "groups": [
            {
                "title": "Values",
                "widget": "datagrid",
                "datasets": [
                    {
                        "title": f"V{i}",
                        "graph": True,
                        "units": "",
                        "value": "0",
                        "index": i + 1,
                    }
                    for i in range(4)
                ],
            }
        ],
        "actions": [],
    }


def _values(api_client) -> list:
    """Current dataset values of the first group, as floats where they parse."""
    data = api_client.get_dashboard_data()
    groups = data.get("frame", {}).get("groups", [])
    if not groups:
        return []

    out = []
    for dataset in groups[0].get("datasets", []):
        try:
            out.append(float(dataset.get("value", "")))
        except (TypeError, ValueError):
            out.append(None)
    return out


def _wait_for(api_client, expected: list, timeout: float = 3.0) -> list:
    """Poll until the dashboard shows the expected values or the timeout lapses."""
    end = time.time() + timeout
    while time.time() < end:
        if _values(api_client) == expected:
            break
        time.sleep(0.05)
    return _values(api_client)


def _connect(api_client, device_simulator, project: dict) -> None:
    """Load the project, point the Network driver at the simulator and connect."""
    api_client.load_project_from_json(project)
    time.sleep(0.2)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.set_operation_mode("project")
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "device never connected"


@pytest.mark.integration
@pytest.mark.network
@pytest.mark.project
class TestFixedLengthFrames:
    def test_frames_are_cut_at_the_length(
        self, api_client, device_simulator, clean_state
    ):
        """Back-to-back frames in one write each become a dashboard update."""
        _connect(api_client, device_simulator, _project())

        device_simulator.send_frame(
            _frame(1, 10, 11, 12, 13) + _frame(2, 20, 21, 22, 23)
        )

        assert _wait_for(api_client, [20.0, 21.0, 22.0, 23.0]) == [
            20.0,
            21.0,
            22.0,
            23.0,
        ]

    def test_frame_split_across_writes(self, api_client, device_simulator, clean_state):
        """A frame arriving in two TCP writes is held until its last byte lands."""
        _connect(api_client, device_simulator, _project())

        frame = _frame(3, 30, 31, 32, 33)
        device_simulator.send_frame(frame[:2])
        time.sleep(0.2)
        device_simulator.send_frame(frame[2:])

        assert _wait_for(api_client, [30.0, 31.0, 32.0, 33.0]) == [
            30.0,
            31.0,
            32.0,
            33.0,
        ]

    def test_resync_after_an_injected_byte(
        self, api_client, device_simulator, clean_state
    ):
        """One stray byte shifts the stream; the header rule re-aligns on the next frame."""
        _connect(api_client, device_simulator, _project())

        device_simulator.send_frame(_frame(1, 1, 2, 3, 4))
        assert _wait_for(api_client, [1.0, 2.0, 3.0, 4.0]) == [1.0, 2.0, 3.0, 4.0]

        device_simulator.send_frame(bytes([0x05]))
        device_simulator.send_frame(_frame(2, 50, 51, 52, 53))

        assert _wait_for(api_client, [50.0, 51.0, 52.0, 53.0]) == [
            50.0,
            51.0,
            52.0,
            53.0,
        ]

    def test_quick_plot_ignores_the_mode(
        self, api_client, device_simulator, clean_state
    ):
        """Quick Plot keeps its line splitter whatever the project's detection mode is."""
        api_client.load_project_from_json(_project())
        time.sleep(0.2)
        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.set_operation_mode("quickplot")
        time.sleep(0.2)
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        device_simulator.send_frame(b"1.5,2.5,3.5\n")

        end = time.time() + 3.0
        values = []
        while time.time() < end and not values:
            data = api_client.get_dashboard_data()
            groups = data.get("frame", {}).get("groups", [])
            values = [
                d.get("value")
                for g in groups
                for d in g.get("datasets", [])
                if d.get("value")
            ]
            time.sleep(0.05)

        assert values, "Quick Plot produced no values from a newline-terminated line"

    def test_zero_length_is_repaired_to_no_delimiters(self, api_client, clean_state):
        """A Fixed Length source with no usable length reads the raw stream instead."""
        api_client.load_project_from_json(_project(frame_length=0))
        time.sleep(0.3)
        api_client.set_operation_mode("project")
        time.sleep(0.2)

        config = api_client.get_frame_parser_config()
        assert config.get("frameDetection") == NO_DELIMITERS

    def test_settings_round_trip_through_the_api(self, api_client, clean_state):
        """frameLength and the rule bytes are accepted, echoed and validated."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        api_client.set_operation_mode("project")

        result = api_client.configure_frame_parser(
            frame_detection=FIXED_LENGTH,
            frame_length=8,
            frame_header_mask=0xF0,
            frame_header_value=0xA5,
        )
        assert result.get("updated") is True

        config = api_client.get_frame_parser_config()
        assert config.get("frameDetection") == FIXED_LENGTH
        assert config.get("frameLength") == 8
        assert config.get("frameHeaderMask") == 0xF0
        assert config.get("frameHeaderValue") == 0xA0  # masked

        with pytest.raises(Exception):
            api_client.configure_frame_parser(frame_length=0)
