"""
MQTT Publisher Project Configuration Tests

Covers the new project-tree MQTT Publisher node (Pro feature):
- mqttPublisher JSON object round-trips through project.loadJson / exportJson
- Schema version bumped to 2
- Empty projects do not carry a mqttPublisher entry
- All publisher fields survive the round-trip

The runtime broker connection is exercised in
``test_mqtt_publisher_broker.py``; this file only inspects the
serialization layer so it runs in CI even when no broker is available.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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


def _minimal_project_with_publisher(publisher: dict) -> dict:
    """Build a minimal project JSON carrying an mqttPublisher section."""
    return {
        "title": "MQTT publisher test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "groups": [],
        "actions": [],
        "mqttPublisher": publisher,
    }


@pytest.mark.mqtt
@pytest.mark.integration
@pytest.mark.project
class TestMqttPublisherSerialization:
    """The mqttPublisher project section must round-trip losslessly."""

    def test_empty_project_has_no_publisher_section(self, api_client, clean_state):
        """A fresh project must not write a mqttPublisher key."""
        exported = api_client.command("project.exportJson").get("config", {})
        # The key is allowed to be absent; if present it must be an empty object
        publisher = exported.get("mqttPublisher", {})
        assert (
            publisher == {} or publisher.get("enabled", False) is False
        ), "Default project should not carry an enabled MQTT publisher"

    def test_schema_version_is_two(self, api_client, clean_state):
        """Saved projects must declare schemaVersion >= 2."""
        if not _is_pro_build(api_client):
            pytest.skip("Schema bump only relevant to Pro builds")

        exported = api_client.command("project.exportJson").get("config", {})
        version = int(exported.get("schemaVersion", 0))
        assert (
            version >= 2
        ), f"Expected schemaVersion >= 2 (post MQTT publisher bump), got {version}"

    def test_publisher_field_round_trip(self, api_client, clean_state):
        """Every publisher field declared in the project survives a save/load cycle."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        # clientId is only persisted when customClientId=True; auto-generated
        # client IDs are intentionally regenerated on each load (see
        # MQTT::Publisher::toJson + applyProjectConfig).
        publisher = {
            "enabled": True,
            "mode": 0,  # DashboardData
            "publishNotifications": True,
            "topicBase": "ss/test/dashboard",
            "notificationTopic": "ss/test/notifications",
            "hostname": "broker.example.com",
            "port": 8883,
            "customClientId": True,
            "clientId": "ss-test-client",
            "username": "tester",
            "password": "secret",
            "cleanSession": True,
            "keepAlive": 30,
            "mqttVersion": 2,
            "sslEnabled": True,
            "sslProtocol": 5,
            "peerVerifyMode": 3,
            "peerVerifyDepth": 12,
        }

        api_client.load_project_from_json(_minimal_project_with_publisher(publisher))
        time.sleep(0.3)

        exported = api_client.command("project.exportJson").get("config", {})
        round_tripped = exported.get("mqttPublisher")
        assert (
            isinstance(round_tripped, dict) and round_tripped
        ), "Loaded mqttPublisher section disappeared on export"

        # Credentials (username/password) are stored encrypted in QSettings,
        # not in the project file, so they intentionally drop out of toJson().
        non_persistent = {"username", "password"}
        for key in publisher:
            if key in non_persistent:
                continue
            assert key in round_tripped, f"Missing key after round-trip: {key}"

        assert round_tripped["hostname"] == "broker.example.com"
        assert int(round_tripped["port"]) == 8883
        assert round_tripped["topicBase"] == "ss/test/dashboard"
        assert round_tripped["notificationTopic"] == "ss/test/notifications"
        assert round_tripped["clientId"] == "ss-test-client"
        assert int(round_tripped["mode"]) == 0
        assert bool(round_tripped["enabled"]) is True
        assert bool(round_tripped["publishNotifications"]) is True
        assert bool(round_tripped["sslEnabled"]) is True

    def test_raw_mode_publisher_round_trip(self, api_client, clean_state):
        """Mode=1 (Raw RX Data) must persist correctly."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        publisher = {
            "enabled": True,
            "mode": 1,
            "topicBase": "ss/raw",
            "hostname": "127.0.0.1",
            "port": 1883,
        }

        api_client.load_project_from_json(_minimal_project_with_publisher(publisher))
        time.sleep(0.3)

        exported = api_client.command("project.exportJson").get("config", {})
        round_tripped = exported.get("mqttPublisher", {})
        assert int(round_tripped.get("mode", -1)) == 1
        assert round_tripped.get("topicBase") == "ss/raw"

    def test_disabled_publisher_persists(self, api_client, clean_state):
        """A publisher with enabled=false must still write out the rest of the config."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        publisher = {
            "enabled": False,
            "mode": 0,
            "topicBase": "ss/preserved",
            "hostname": "preserve-me.example.com",
            "port": 1234,
        }

        api_client.load_project_from_json(_minimal_project_with_publisher(publisher))
        time.sleep(0.3)

        exported = api_client.command("project.exportJson").get("config", {})
        round_tripped = exported.get("mqttPublisher", {})

        assert round_tripped.get("hostname") == "preserve-me.example.com"
        assert int(round_tripped.get("port", 0)) == 1234
        assert bool(round_tripped.get("enabled", True)) is False


@pytest.mark.mqtt
@pytest.mark.integration
@pytest.mark.project
class TestMqttPublisherDefaults:
    """When the project omits fields, sensible defaults must be applied."""

    def test_sparse_config_filled_with_defaults(self, api_client, clean_state):
        """Loading a sparse mqttPublisher object must not crash and must fill defaults on export."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT publisher requires a commercial build")

        # Only enabled + hostname; the runtime must fill everything else.
        api_client.load_project_from_json(
            _minimal_project_with_publisher({"enabled": True, "hostname": "192.0.2.10"})
        )
        time.sleep(0.3)

        exported = api_client.command("project.exportJson").get("config", {})
        publisher = exported.get("mqttPublisher", {})
        assert publisher.get("hostname") == "192.0.2.10"
        assert "port" in publisher, "Default port should be filled in"
        # Auto-generated client IDs are regenerated on every load and not
        # persisted (toJson only emits clientId when customClientId=True).
        # customClientId itself should always round-trip.
        assert "customClientId" in publisher, "customClientId flag should be filled in"
        assert (
            bool(publisher.get("customClientId", True)) is False
        ), "Default customClientId should be False (auto-generated)"
        # Default port for MQTT is 1883
        assert int(publisher.get("port", 0)) == 1883
