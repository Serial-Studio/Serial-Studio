"""
MQTT Singleton Removal Regression Tests

Phase 4 of the MQTT redistribution deletes ``MQTT::Client`` (singleton),
``MQTTConfiguration.qml`` and the entire ``API::Handlers::MQTTHandler``.
This file pins that contract so the old commands never come back without
intent.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import pytest

from utils.api_client import APIError

# The full list of legacy MQTT singleton commands that have been removed.
# Adding any of these back without explicit intent should fail this test.
REMOVED_MQTT_COMMANDS = (
    "mqtt.getConfig",
    "mqtt.setMode",
    "mqtt.setHostname",
    "mqtt.setPort",
    "mqtt.setTopic",
    "mqtt.connect",
    "mqtt.disconnect",
    "mqtt.setClientId",
    "mqtt.setUsername",
    "mqtt.setPassword",
    "mqtt.setCleanSession",
    "mqtt.setMqttVersion",
    "mqtt.setKeepAlive",
    "mqtt.setAutoKeepAlive",
    "mqtt.refreshClientId",
    "mqtt.setWillQos",
    "mqtt.setWillRetain",
    "mqtt.setWillTopic",
    "mqtt.setWillMessage",
    "mqtt.setSslEnabled",
    "mqtt.setSslProtocol",
    "mqtt.setPeerVerifyMode",
    "mqtt.setPeerVerifyDepth",
    "mqtt.getConnectionStatus",
    "mqtt.listModes",
    "mqtt.listMqttVersions",
    "mqtt.listSslProtocols",
    "mqtt.listPeerVerifyModes",
)


@pytest.mark.mqtt
@pytest.mark.integration
class TestMqttSingletonRemoved:
    """The old singleton-backed commands must no longer be registered."""

    def test_legacy_commands_not_in_registry(self, api_client, clean_state):
        """api.getCommands must not list any legacy mqtt.* command."""
        result = api_client.command("api.getCommands")
        commands = result.get("commands", [])
        names = {cmd.get("name") for cmd in commands}

        still_present = [name for name in REMOVED_MQTT_COMMANDS if name in names]
        assert not still_present, (
            f"Legacy MQTT singleton commands still registered: {still_present}. "
            "These were removed in Phase 4 -- if you intentionally restored them, "
            "update this test."
        )

    @pytest.mark.parametrize("command_name", REMOVED_MQTT_COMMANDS)
    def test_legacy_command_invocation_fails(
        self, api_client, clean_state, command_name
    ):
        """Calling any removed mqtt.* command must surface as UNKNOWN_COMMAND."""
        try:
            api_client.command(command_name)
        except APIError as e:
            assert "UNKNOWN_COMMAND" in str(e) or "NOT_FOUND" in str(
                e
            ), f"Expected UNKNOWN_COMMAND for '{command_name}', got: {e}"
            return
        pytest.fail(
            f"Removed command '{command_name}' returned successfully; "
            "it should be gone after Phase 4."
        )
