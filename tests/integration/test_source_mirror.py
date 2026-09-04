"""
Setup-pane to project mirror (single-source projects).

ConnectionManager mirrors the active UI driver's bus type and connection settings into source 0
of a single-source project. Switching the bus type must replace the settings with the new
driver's, not leave the previous driver's keys behind.
"""

import pytest


def _connection(api_client):
    config = api_client.command("project.source.getConfig", {"sourceId": 0})
    return config.get("busType"), config.get("connection") or {}


@pytest.mark.integration
def test_bus_type_switch_replaces_source0_connection_settings(api_client, clean_state):
    """Switching UART -> Network drops the UART keys and carries the Network keys."""
    api_client.command("project.new")

    api_client.command("io.setBusType", {"busType": 0})
    api_client.command("io.uart.setBaudRate", {"baudRate": 19200})
    api_client.command("io.uart.setBaudRate", {"baudRate": 9600})
    bus_type, connection = _connection(api_client)
    assert bus_type == 0
    assert connection.get("baudRate") == 9600

    api_client.command("io.setBusType", {"busType": 1})
    bus_type, connection = _connection(api_client)
    assert bus_type == 1
    assert "tcpPort" in connection
    assert "baudRate" not in connection


@pytest.mark.integration
def test_driver_option_edit_is_mirrored_into_source0(api_client, clean_state):
    """An option edit on the active driver lands in source 0 without a bus-type change."""
    api_client.command("project.new")
    api_client.command("io.setBusType", {"busType": 1})

    api_client.command("io.network.setTcpPort", {"port": 5556})
    api_client.command("io.network.setTcpPort", {"port": 5555})
    bus_type, connection = _connection(api_client)
    assert bus_type == 1
    assert connection.get("tcpPort") == 5555
