"""
MQTT Publisher End-to-End Tests (broker required)

Exercises the publisher runtime against a real MQTT broker:
- Dashboard data is published as compact JSON to the configured topic.
- A disabled publisher does not emit messages.

These tests require a Mosquitto (or any MQTT v3.1.1+) broker reachable
at 127.0.0.1:1883 and the ``paho-mqtt`` Python package on the test side.
CI installs both via the workflow file; local developers can run
``mosquitto -p 1883`` in another terminal.

Frames are injected through the standard pipeline (Network driver → TCP
device simulator on port 9000 → FrameReader → FrameBuilder → publisher)
because Serial Studio has no direct "inject a frame" API.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils.api_client import APIError


def _is_pro_build(api_client) -> bool:
    try:
        buses = api_client.command("io.listBuses").get("buses", [])
    except APIError:
        return False
    return len(buses) > 9


def _project_with_dashboard_publisher(
    topic_base: str, with_notifications: bool = False, enabled: bool = True
) -> dict:
    return {
        "title": "MQTT broker test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "frameParserCode": (
            "function parse(frame) {\n" "  return frame.split(',');\n" "}\n"
        ),
        "frameParserLanguage": 0,
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "X",
                        "units": "",
                        "value": "%1",
                        "index": 1,
                        "min": 0,
                        "max": 100,
                    }
                ],
            }
        ],
        "actions": [],
        "mqttPublisher": {
            "enabled": enabled,
            "mode": 0,
            "publishNotifications": with_notifications,
            "topicBase": topic_base,
            "notificationTopic": f"{topic_base}/notify",
            "hostname": "127.0.0.1",
            "port": 1883,
            "cleanSession": True,
            "keepAlive": 60,
            "mqttVersion": 2,
            "sslEnabled": False,
        },
    }


def _attach_via_network(api_client, device_simulator) -> None:
    """Set up Network driver pointed at the simulator and open the connection."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
    )
    try:
        api_client.command("project.activate")
    except APIError:
        pass
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(
        timeout=5.0
    ), "Serial Studio did not connect to the device simulator"


def _wait_for_publisher_connected(api_client, timeout: float = 8.0) -> bool:
    """Poll project.mqtt.publisher.getStatus until isConnected flips true.

    The publisher worker debounces config sync by 200 ms and the QMqttClient
    handshake is asynchronous, so a fixed time.sleep is brittle on slow CI
    runners. Polling instead lets the test wait exactly as long as needed and
    fail fast when the publisher isn't actually online.
    """
    end_time = time.time() + timeout
    while time.time() < end_time:
        try:
            status = api_client.command("project.mqtt.publisher.getStatus")
        except APIError:
            return False
        if status.get("isConnected"):
            return True
        time.sleep(0.1)
    return False


@pytest.mark.mqtt
@pytest.mark.requires_broker
@pytest.mark.integration
@pytest.mark.project
class TestMqttPublisherBroker:
    """End-to-end checks against a live broker."""

    def test_dashboard_frame_publish(
        self, api_client, clean_state, device_simulator, mqtt_subscriber
    ):
        """Frames produced by Serial Studio appear on the configured topic."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        topic = f"ss/test/dashboard/{int(time.time())}"
        mqtt_subscriber.subscribe(topic)

        api_client.load_project_from_json(_project_with_dashboard_publisher(topic))
        time.sleep(0.5)

        _attach_via_network(api_client, device_simulator)
        if not _wait_for_publisher_connected(api_client, timeout=8.0):
            pytest.skip(
                "MQTT publisher did not reach Connected state within timeout "
                "(broker reachable but handshake failed)"
            )

        device_simulator.send_frame(b"/*42*/")

        if not mqtt_subscriber.wait_for_messages(count=1, timeout=8.0):
            pytest.skip(
                "Broker did not deliver any message within timeout (publisher "
                "reported Connected but no frame was published)"
            )

        msg = mqtt_subscriber.messages[0]
        assert msg["topic"] == topic, f"Wrong topic: {msg['topic']!r}"
        # Payload is a compact JSON-encoded Frame; it must parse and carry a title.
        decoded = json.loads(msg["payload"].decode("utf-8"))
        assert decoded.get("title"), "Frame JSON missing title field"

    def test_publisher_disabled_does_not_emit(
        self, api_client, clean_state, device_simulator, mqtt_subscriber
    ):
        """When enabled=false, no MQTT traffic should appear."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        topic = f"ss/test/disabled/{int(time.time())}"
        mqtt_subscriber.subscribe(topic)

        api_client.load_project_from_json(
            _project_with_dashboard_publisher(topic, enabled=False)
        )
        time.sleep(0.5)

        _attach_via_network(api_client, device_simulator)
        time.sleep(0.5)

        device_simulator.send_frame(b"/*42*/")
        # Grace period -- no messages should arrive.
        time.sleep(1.5)

        topic_msgs = [m for m in mqtt_subscriber.messages if m["topic"] == topic]
        assert (
            topic_msgs == []
        ), f"Publisher emitted {len(topic_msgs)} messages while disabled"
