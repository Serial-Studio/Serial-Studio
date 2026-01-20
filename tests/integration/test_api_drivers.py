"""
API Driver and Console Integration Tests

Validates driver configuration, bus switching, and console API behavior.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


def _wait_for_console_length(api_client, minimum: int, timeout: float = 2.0) -> int:
    end_time = time.time() + timeout
    while time.time() < end_time:
        config = api_client.command("console.getConfiguration")
        length = config.get("bufferLength", 0)
        if length >= minimum:
            return length
        time.sleep(0.05)
    return api_client.command("console.getConfiguration").get("bufferLength", 0)


def _wait_for_console_empty(api_client, timeout: float = 2.0) -> int:
    end_time = time.time() + timeout
    while time.time() < end_time:
        length = api_client.command("console.getConfiguration").get("bufferLength", 0)
        if length == 0:
            return length
        time.sleep(0.05)
    return api_client.command("console.getConfiguration").get("bufferLength", 0)


@pytest.mark.integration
def test_console_clear(api_client, device_simulator, clean_state):
    """Ensure console.clear resets the console buffer."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frame(b"console test\n")
    length = _wait_for_console_length(api_client, minimum=1)
    assert length > 0, "Console buffer should contain data"

    api_client.command("console.clear")
    length = _wait_for_console_empty(api_client, timeout=2.0)
    assert length == 0, "Console buffer should be empty"


@pytest.mark.integration
def test_driver_switching(api_client, clean_state):
    """Switch between available drivers and validate bus type updates."""
    buses = api_client.command("io.manager.getAvailableBuses").get("buses", [])
    assert buses, "Expected at least one available bus type"

    for bus in buses:
        api_client.command("io.manager.setBusType", {"busType": bus["index"]})
        status = api_client.command("io.manager.getStatus")
        assert status["busType"] == bus["index"]


@pytest.mark.integration
def test_uart_driver_configuration(api_client, clean_state):
    """Validate UART driver configuration commands."""
    if not api_client.command_exists("io.driver.uart.getConfiguration"):
        pytest.skip("UART driver commands not available")

    api_client.command("io.manager.setBusType", {"busType": 0})

    port_list = api_client.command("io.driver.uart.getPortList").get("portList", [])
    port_list = [port for port in port_list if isinstance(port, str) and port.strip()]
    if port_list:
        api_client.command("io.driver.uart.setPortIndex", {"portIndex": 0})
        api_client.command("io.driver.uart.setDevice", {"device": port_list[0]})

    baud_rates = api_client.command("io.driver.uart.getBaudRateList").get("baudRateList", [])
    baud_rates = [rate for rate in baud_rates if isinstance(rate, (int, float)) and rate > 0]
    if baud_rates:
        api_client.command("io.driver.uart.setBaudRate", {"baudRate": baud_rates[0]})

    api_client.command("io.driver.uart.setParity", {"parityIndex": 0})
    api_client.command("io.driver.uart.setDataBits", {"dataBitsIndex": 3})
    api_client.command("io.driver.uart.setStopBits", {"stopBitsIndex": 0})
    api_client.command("io.driver.uart.setFlowControl", {"flowControlIndex": 0})
    api_client.command("io.driver.uart.setDtrEnabled", {"dtrEnabled": True})
    api_client.command("io.driver.uart.setAutoReconnect", {"autoReconnect": True})

    config = api_client.command("io.driver.uart.getConfiguration")
    assert config["parityIndex"] == 0
    assert config["dataBitsIndex"] == 3
    assert config["stopBitsIndex"] == 0
    assert config["flowControlIndex"] == 0
    assert config["dtrEnabled"] is True
    assert config["autoReconnect"] is True


@pytest.mark.integration
def test_network_driver_configuration(api_client, clean_state):
    """Validate network driver configuration commands."""
    api_client.command("io.manager.setBusType", {"busType": 1})

    api_client.command("io.driver.network.setRemoteAddress", {"address": "127.0.0.1"})
    api_client.command("io.driver.network.setSocketType", {"socketTypeIndex": 0})
    api_client.command("io.driver.network.setTcpPort", {"port": 9000})

    config = api_client.command("io.driver.network.getConfiguration")
    assert config["remoteAddress"] == "127.0.0.1"
    assert config["socketTypeIndex"] == 0
    assert config["tcpPort"] == 9000

    api_client.command("io.driver.network.setSocketType", {"socketTypeIndex": 1})
    api_client.command("io.driver.network.setUdpLocalPort", {"port": 0})
    api_client.command("io.driver.network.setUdpRemotePort", {"port": 9001})
    api_client.command("io.driver.network.setUdpMulticast", {"enabled": False})

    config = api_client.command("io.driver.network.getConfiguration")
    assert config["socketTypeIndex"] == 1
    assert config["udpRemotePort"] == 9001


@pytest.mark.integration
def test_ble_driver_status(api_client, clean_state):
    """Validate BLE driver status query."""
    if not api_client.command_exists("io.driver.ble.getStatus"):
        pytest.skip("BLE driver commands not available")

    api_client.command("io.manager.setBusType", {"busType": 2})

    status = api_client.command("io.driver.ble.getStatus")
    assert "adapterAvailable" in status
    assert "operatingSystemSupported" in status


@pytest.mark.integration
def test_console_export_status(api_client, clean_state):
    """Validate console export API commands when available."""
    if not api_client.command_exists("console.export.getStatus"):
        pytest.skip("Console export commands not available")

    api_client.command("console.export.setEnabled", {"enabled": True})
    status = api_client.command("console.export.getStatus")
    assert "enabled" in status
    assert "isOpen" in status

    api_client.command("console.export.close")
    api_client.command("console.export.setEnabled", {"enabled": False})


@pytest.mark.integration
def test_mdf4_export_status(api_client, clean_state):
    """Validate MDF4 export API commands when available."""
    if not api_client.command_exists("mdf4.export.getStatus"):
        pytest.skip("MDF4 export commands not available")

    api_client.command("mdf4.export.setEnabled", {"enabled": True})
    status = api_client.command("mdf4.export.getStatus")
    assert "enabled" in status
    assert "isOpen" in status

    api_client.command("mdf4.export.close")
    api_client.command("mdf4.export.setEnabled", {"enabled": False})
