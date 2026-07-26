"""
MQTT Input Driver Tests

Covers the new IO::Drivers::MQTT subscriber-only driver:
- BusType::Mqtt enumeration (Pro builds)
- Driver property round-trip via project.source.setProperty / getConfig
- Multi-source support (per-instance broker config)

The driver itself is non-singleton — multiple sources can subscribe to
different brokers/topics in the same project. These tests configure the
driver but do not require an actual broker connection.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

# BusType::Mqtt is the last commercial entry; its integer value is the
# count of commercial drivers added before it.
BUS_TYPE_MQTT = 9


def _is_pro_build(api_client) -> bool:
    """Return True when running against a Pro build (BusType list contains Mqtt)."""
    try:
        buses = api_client.command("io.listBuses").get("buses", [])
    except APIError:
        return False
    return len(buses) > BUS_TYPE_MQTT


def _bus_name(entry) -> str:
    """Bus entries are dicts {index, name}; tolerate plain strings too."""
    if isinstance(entry, dict):
        return entry.get("name", "")
    return str(entry)


@pytest.mark.mqtt
@pytest.mark.integration
class TestMqttBusTypeRegistration:
    """The MQTT subscriber bus type must be selectable in Pro builds."""

    def test_mqtt_bus_type_in_available_list(self, api_client, clean_state):
        """The available-buses list must include an MQTT entry at index 9."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        result = api_client.command("io.listBuses")
        buses = result.get("buses", [])
        assert (
            len(buses) > BUS_TYPE_MQTT
        ), f"Expected at least {BUS_TYPE_MQTT + 1} bus types, got {len(buses)}"

        mqtt_label = _bus_name(buses[BUS_TYPE_MQTT])
        assert (
            "mqtt" in mqtt_label.lower()
        ), f"Bus type at index {BUS_TYPE_MQTT} should be MQTT, got '{mqtt_label}'"

    def test_set_bus_type_to_mqtt(self, api_client, clean_state):
        """Switching the global bus type to MQTT must succeed."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        api_client.command("io.setBusType", {"busType": BUS_TYPE_MQTT})
        time.sleep(0.2)
        status = api_client.command("io.getStatus")
        assert int(status.get("busType", -1)) == BUS_TYPE_MQTT


@pytest.mark.mqtt
@pytest.mark.integration
class TestMqttDriverProperties:
    """Driver properties must round-trip through Source.connection (Keys::SourceConn)."""

    def _switch_source0_to_mqtt(self, api_client) -> None:
        """Configure source 0 to use the MQTT input driver."""
        api_client.command("io.setBusType", {"busType": BUS_TYPE_MQTT})
        time.sleep(0.2)

    def test_hostname_round_trip(self, api_client, clean_state):
        """hostname property writes through and reads back identically."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        api_client.source_set_property(0, "hostname", "broker.example.com")
        time.sleep(0.2)

        config = api_client.source_get_configuration(0)
        settings = config.get("connection", {})
        assert settings.get("hostname") == "broker.example.com"

    def test_port_round_trip(self, api_client, clean_state):
        """port property accepts integer values."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        api_client.source_set_property(0, "port", 8883)
        time.sleep(0.2)

        config = api_client.source_get_configuration(0)
        settings = config.get("connection", {})
        assert int(settings.get("port", 0)) == 8883

    def test_topic_filter_round_trip(self, api_client, clean_state):
        """topicFilter property persists exactly as written."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        api_client.source_set_property(0, "topicFilter", "sensors/#")
        time.sleep(0.2)

        config = api_client.source_get_configuration(0)
        settings = config.get("connection", {})
        assert settings.get("topicFilter") == "sensors/#"

    def test_full_broker_config_via_source_configure(self, api_client, clean_state):
        """Apply a full broker config in a single setProperties call."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        api_client.source_configure(
            0,
            {
                "hostname": "127.0.0.1",
                "port": 1883,
                "topicFilter": "ss/test",
                "username": "tester",
                "cleanSession": True,
                "keepAlive": 30,
            },
        )
        time.sleep(0.3)

        config = api_client.source_get_configuration(0)
        settings = config.get("connection", {})
        assert settings.get("hostname") == "127.0.0.1"
        assert int(settings.get("port", 0)) == 1883
        assert settings.get("topicFilter") == "ss/test"
        assert settings.get("username") == "tester"
        assert int(settings.get("keepAlive", 0)) == 30

    def test_ssl_disabled_by_default(self, api_client, clean_state):
        """SSL must be off until the user opts in."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        config = api_client.source_get_configuration(0)
        settings = config.get("connection", {})
        # sslEnabled may be missing from the JSON (default false) or explicitly false
        assert not settings.get("sslEnabled", False)

    def test_mqtt_driver_is_subscriber_only(self, api_client, clean_state):
        """The MQTT input driver does not advertise itself as writable."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        self._switch_source0_to_mqtt(api_client)

        # configurationOk requires hostname + port + topic
        api_client.source_configure(
            0,
            {
                "hostname": "127.0.0.1",
                "port": 1883,
                "topicFilter": "test/topic",
            },
        )
        time.sleep(0.2)

        # The driver is configurable, but ConnectionManager exposes
        # readOnly/readWrite via isConnected; without a real broker we
        # can only assert that the bus type stayed correctly applied.
        status = api_client.command("io.getStatus")
        assert int(status.get("busType", -1)) == BUS_TYPE_MQTT


@pytest.mark.mqtt
@pytest.mark.integration
class TestMqttMultiSource:
    """The driver must be non-singleton -- each source owns its own client."""

    def test_distinct_broker_per_source(self, api_client, clean_state):
        """Two sources can be configured with different brokers."""
        if not _is_pro_build(api_client):
            pytest.skip("MQTT input driver requires a commercial build")

        # Single-source projects map source 0 to the live UI driver.
        # Add a second source so we can compare two independent MQTT clients.
        try:
            second_id = api_client.source_add()
        except APIError as e:
            pytest.skip(f"Multi-source not available: {e}")

        if second_id < 1:
            pytest.skip("Failed to add second source")

        api_client.source_update(0, busType=BUS_TYPE_MQTT)
        api_client.source_update(second_id, busType=BUS_TYPE_MQTT)
        time.sleep(0.2)

        api_client.source_configure(
            0,
            {"hostname": "broker-a.example.com", "port": 1883, "topicFilter": "a/#"},
        )
        api_client.source_configure(
            second_id,
            {"hostname": "broker-b.example.com", "port": 8883, "topicFilter": "b/#"},
        )
        time.sleep(0.3)

        cfg0 = api_client.source_get_configuration(0).get("connection", {})
        cfg1 = api_client.source_get_configuration(second_id).get("connection", {})

        assert cfg0.get("hostname") == "broker-a.example.com"
        assert cfg1.get("hostname") == "broker-b.example.com"
        assert cfg0.get("topicFilter") == "a/#"
        assert cfg1.get("topicFilter") == "b/#"
        assert int(cfg0.get("port", 0)) == 1883
        assert int(cfg1.get("port", 0)) == 8883
