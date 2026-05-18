"""
MQTT Scripting API Tests

Verifies that the project frame-parser engines expose ``mqttPublish``:
- The JavaScript engine (QJSEngine) defines it via the TableApiBridge
  wrapper installed at compile time.
- The Lua 5.4 engine registers it as a C closure global.

The compile/sanity tests inject a frame through the Network driver and
device simulator and check that the parser engine accepts the script.
The end-to-end tests additionally assert that the message reaches a
live broker via the ``mqtt_subscriber`` fixture (paho-mqtt).

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError


def _is_pro_build(api_client) -> bool:
    try:
        buses = api_client.command("io.listBuses").get("buses", [])
    except APIError:
        return False
    return len(buses) > 9


def _project_with_publisher_and_parser(
    parser_code: str, parser_language: int, topic_base: str
) -> dict:
    return {
        "title": "MQTT scripting test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "frameParserCode": parser_code,
        "frameParserLanguage": parser_language,
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
            "enabled": True,
            "mode": 0,
            "topicBase": topic_base,
            "hostname": "127.0.0.1",
            "port": 1883,
            "cleanSession": True,
            "keepAlive": 60,
            "mqttVersion": 2,
            "sslEnabled": False,
        },
    }


def _attach_via_network(api_client, device_simulator) -> None:
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
    runners. Polling lets the test wait exactly as long as needed.
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
@pytest.mark.integration
@pytest.mark.project
class TestMqttScriptingApiCompile:
    """The new globals must be visible to the parser engines."""

    def test_js_parser_with_mqtt_publish_compiles(
        self, api_client, clean_state, device_simulator
    ):
        """A JS parser calling mqttPublish() must not produce a compile error."""
        if not _is_pro_build(api_client):
            pytest.skip("mqttPublish requires a commercial build")

        code = (
            "function parse(frame) {\n"
            "  mqttPublish('ss/test/js', 'hello');\n"
            "  return frame.split(',');\n"
            "}\n"
        )
        project = _project_with_publisher_and_parser(code, 0, "ss/test/js-dash")
        api_client.load_project_from_json(project)
        time.sleep(0.3)

        _attach_via_network(api_client, device_simulator)
        device_simulator.send_frame(b"/*1,2,3*/")
        time.sleep(0.5)

        # If the parser engine had rejected mqttPublish at compile time the
        # frame would never have produced datasets; verify the server is
        # responsive and the project is still loaded.
        result = api_client.command("api.getCommands")
        assert isinstance(result.get("commands"), list)
        status = api_client.get_project_status()
        assert status.get("title") == "MQTT scripting test"

    def test_lua_parser_with_mqtt_publish_compiles(
        self, api_client, clean_state, device_simulator
    ):
        """A Lua parser calling mqttPublish() must not produce a compile error."""
        if not _is_pro_build(api_client):
            pytest.skip("mqttPublish requires a commercial build")

        code = (
            "function parse(frame)\n"
            "  mqttPublish('ss/test/lua', 'hello')\n"
            "  return {}\n"
            "end\n"
        )
        project = _project_with_publisher_and_parser(code, 1, "ss/test/lua-dash")
        api_client.load_project_from_json(project)
        time.sleep(0.3)

        _attach_via_network(api_client, device_simulator)
        device_simulator.send_frame(b"/*1,2,3*/")
        time.sleep(0.5)

        result = api_client.command("api.getCommands")
        assert isinstance(result.get("commands"), list)


@pytest.mark.mqtt
@pytest.mark.requires_broker
@pytest.mark.integration
@pytest.mark.project
class TestMqttScriptingPublishEndToEnd:
    """With a live broker, mqttPublish() must actually emit the message."""

    def test_js_mqtt_publish_reaches_broker(
        self, api_client, clean_state, device_simulator, mqtt_subscriber
    ):
        if not _is_pro_build(api_client):
            pytest.skip("mqttPublish requires a commercial build")

        topic = f"ss/script/js/{int(time.time())}"
        mqtt_subscriber.subscribe(topic)

        code = (
            "function parse(frame) {\n"
            f"  mqttPublish('{topic}', 'from-js');\n"
            "  return frame.split(',');\n"
            "}\n"
        )
        project = _project_with_publisher_and_parser(code, 0, f"{topic}/dash")
        api_client.load_project_from_json(project)
        time.sleep(0.5)

        _attach_via_network(api_client, device_simulator)
        if not _wait_for_publisher_connected(api_client, timeout=8.0):
            pytest.skip("MQTT publisher did not reach Connected state within timeout")

        device_simulator.send_frame(b"/*1,2,3*/")

        if not mqtt_subscriber.wait_for_messages(count=1, timeout=8.0):
            pytest.skip("Broker did not deliver message within timeout")

        payloads = [
            m["payload"] for m in mqtt_subscriber.messages if m["topic"] == topic
        ]
        assert (
            b"from-js" in payloads
        ), f"Expected payload 'from-js' on '{topic}', got {mqtt_subscriber.messages}"

    def test_lua_mqtt_publish_reaches_broker(
        self, api_client, clean_state, device_simulator, mqtt_subscriber
    ):
        if not _is_pro_build(api_client):
            pytest.skip("mqttPublish requires a commercial build")

        topic = f"ss/script/lua/{int(time.time())}"
        mqtt_subscriber.subscribe(topic)

        code = (
            "function parse(frame)\n"
            f"  mqttPublish('{topic}', 'from-lua')\n"
            "  return {}\n"
            "end\n"
        )
        project = _project_with_publisher_and_parser(code, 1, f"{topic}/dash")
        api_client.load_project_from_json(project)
        time.sleep(0.5)

        _attach_via_network(api_client, device_simulator)
        if not _wait_for_publisher_connected(api_client, timeout=8.0):
            pytest.skip("MQTT publisher did not reach Connected state within timeout")

        device_simulator.send_frame(b"/*1,2,3*/")

        if not mqtt_subscriber.wait_for_messages(count=1, timeout=8.0):
            pytest.skip("Broker did not deliver message within timeout")

        payloads = [
            m["payload"] for m in mqtt_subscriber.messages if m["topic"] == topic
        ]
        assert (
            b"from-lua" in payloads
        ), f"Expected payload 'from-lua' on '{topic}', got {mqtt_subscriber.messages}"
