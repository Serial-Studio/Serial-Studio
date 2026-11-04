"""
Spec 0108 -- dataset value formats and labels, end to end over the API.

AC2: a labelled dataset keeps its number and gains the label text in dashboard.getData, and the
fixed-point, bit-field and scale decoders publish the decoded number; every dataset stays plotted.
AC3: every format and the label table survive project.exportJson -> project.loadJson, and a
project without the keys exports without them.
AC5 (integration half): a project with formats and labels but no transform connects; the
license-free path itself is pinned by the C++ unit tst_pro_content.

Requires the app running with the API server on (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import time

import pytest

from utils import APIError

_PARSER = """
function parse(frame) {
  return frame.split(",");
}
"""

_GEAR_LABELS = [
    {"value": 0, "text": "Park"},
    {"value": 1, "text": "Drive"},
    {"value": 255, "text": "Reverse"},
]

_FORMAT_KEYS = (
    "format",
    "formatWidth",
    "formatSigned",
    "formatFractionBits",
    "formatBitOffset",
    "formatBitWidth",
    "formatScale",
    "formatOffset",
    "labels",
    "labelsDefault",
    "displayFormat",
)


def _dataset(title: str, index: int, **fields) -> dict:
    dataset = {"title": title, "value": f"%{index}", "index": index, "graph": True}
    dataset.update(fields)
    return dataset


def _project(with_formats: bool = True) -> dict:
    datasets = [
        _dataset("Gear", 1),
        _dataset("Q15", 2),
        _dataset("Bit3", 3),
        _dataset("Scaled", 4),
    ]
    if with_formats:
        datasets[0].update(labels=_GEAR_LABELS, labelsDefault="Fault")
        datasets[1].update(
            format=1, formatWidth=16, formatSigned=True, formatFractionBits=15
        )
        datasets[2].update(
            format=2, formatWidth=16, formatBitOffset=3, formatBitWidth=1
        )
        datasets[3].update(
            format=3, formatScale=0.5, formatOffset=-40, displayFormat="hex"
        )

    return {
        "title": "Dataset formats",
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
        "groups": [{"title": "G", "widget": "", "datasets": datasets}],
        "actions": [],
    }


def _attach(api_client, device_simulator) -> None:
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
    assert device_simulator.wait_for_connection(timeout=5.0), "device did not connect"


def _live_datasets(api_client) -> dict:
    frame = api_client.get_dashboard_data().get("frame", {})
    return {
        dataset["title"]: dataset
        for group in frame.get("groups", [])
        for dataset in group.get("datasets", [])
    }


def _exported_datasets(api_client) -> dict:
    config = api_client.command("project.exportJson")["config"]
    return {
        dataset["title"]: dataset
        for group in config.get("groups", [])
        for dataset in group.get("datasets", [])
    }


@pytest.mark.project
def test_labels_keep_the_number_and_decoders_publish_decoded_values(
    api_client, device_simulator, clean_state
):
    """AC2 and AC5: one frame through every decoder and a label table."""
    result = api_client.load_project_from_json(_project())
    assert result.get("loaded") is True
    _attach(api_client, device_simulator)
    assert api_client.is_connected(), "a formats-only project must connect"

    for _ in range(5):
        device_simulator.send_frame(b"255,16384,8,100\n")
        time.sleep(0.05)
    time.sleep(1.0)

    live = _live_datasets(api_client)
    assert live["Gear"]["numericValue"] == pytest.approx(255)
    assert live["Gear"]["label"] == "Reverse"
    assert live["Q15"]["numericValue"] == pytest.approx(0.5)
    assert live["Bit3"]["numericValue"] == pytest.approx(1)
    assert live["Scaled"]["numericValue"] == pytest.approx(10)
    for title in ("Q15", "Bit3", "Scaled"):
        assert (
            "label" not in live[title]
        ), f"{title} has no labels and gains no label key"
    for title, dataset in live.items():
        assert dataset.get("graph") is True, f"{title} must stay plotted"

    device_simulator.send_frame(b"7,0,0,0\n")
    time.sleep(1.0)
    assert _live_datasets(api_client)["Gear"]["label"] == "Fault"


@pytest.mark.project
def test_every_format_round_trips_through_export(api_client, clean_state):
    """AC3: the format keys and the label table survive export -> load -> export."""
    original = _project()
    api_client.load_project_from_json(original)
    first = _exported_datasets(api_client)

    for dataset in original["groups"][0]["datasets"]:
        exported = first[dataset["title"]]
        for key in _FORMAT_KEYS:
            if key in dataset:
                assert exported.get(key) == dataset[key], f"{dataset['title']}.{key}"

    config = api_client.command("project.exportJson")["config"]
    api_client.load_project_from_json(copy.deepcopy(config))
    assert _exported_datasets(api_client) == first


@pytest.mark.project
def test_a_project_without_formats_exports_none_of_the_keys(api_client, clean_state):
    """AC3: a project that uses no format writes none of the new keys, so files saved by this
    release without formats stay byte-identical to the previous one."""
    api_client.load_project_from_json(_project(with_formats=False))
    for title, dataset in _exported_datasets(api_client).items():
        for key in _FORMAT_KEYS[:-1]:
            assert key not in dataset, f"{title} gained {key}"
        assert "label" not in dataset, f"{title} gained the runtime label key"


@pytest.mark.project
def test_set_labels_refuses_a_non_integer_value(api_client, clean_state):
    """project.dataset.setLabels refuses a fractional key instead of silently dropping it."""
    api_client.load_project_from_json(_project(with_formats=False))
    gear = api_client.command("project.dataset.getByTitle", {"title": "Gear"})
    gear = gear.get("dataset", gear)
    ids = {"groupId": gear["groupId"], "datasetId": gear["datasetId"]}

    with pytest.raises(APIError):
        api_client.command(
            "project.dataset.setLabels",
            {**ids, "labels": [{"value": 1.5, "text": "Half"}]},
        )

    reply = api_client.command(
        "project.dataset.setLabels",
        {**ids, "labels": _GEAR_LABELS + [{"value": 1, "text": "Again"}]},
    )
    assert reply["count"] == 3
    assert reply["droppedInvalid"] == 1
    stored = api_client.command("project.dataset.getLabels", ids)
    assert stored["labels"] == _GEAR_LABELS
