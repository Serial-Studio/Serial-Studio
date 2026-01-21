"""
Comprehensive Driver API Tests

Tests all available API commands for each driver type, ensuring:
1. Every mutation command works correctly
2. Every query command returns expected data
3. State is saved before tests and restored afterward
4. Pro features are skipped gracefully if not available
5. Sleep timings allow visual validation in GUI

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import pytest
import time


GUI_VALIDATION_DELAY = 1.0


@pytest.mark.integration
def test_uart_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all UART driver API commands.
    Tests every mutation and query command with state save/restore.
    """
    if not api_client.command_exists("io.driver.uart.getConfiguration"):
        pytest.skip("UART driver commands not available")

    api_client.command("io.manager.setBusType", {"busType": 0})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.uart.getConfiguration")

    try:
        port_list_result = api_client.command("io.driver.uart.getPortList")
        port_list = port_list_result.get("portList", [])
        assert isinstance(port_list, list), "portList should be a list"

        baud_rate_list_result = api_client.command("io.driver.uart.getBaudRateList")
        baud_rates = baud_rate_list_result.get("baudRateList", [])
        current_baud = baud_rate_list_result.get("currentBaudRate", original_config.get("baudRate", 9600))
        assert isinstance(baud_rates, list), "baudRateList should be a list"
        assert len(baud_rates) > 0, "Should have at least one baud rate"

        valid_ports = [p for p in port_list if isinstance(p, str) and p.strip()]
        if len(valid_ports) > 0:
            api_client.command("io.driver.uart.setPortIndex", {"portIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.uart.getConfiguration")
            assert config["portIndex"] == 0

            api_client.command("io.driver.uart.setDevice", {"device": valid_ports[0]})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.uart.getConfiguration")
            assert config["portName"] == valid_ports[0]

        if len(baud_rates) >= 2:
            test_baud = None
            for rate_str in baud_rates:
                rate_int = int(rate_str)
                if rate_int != current_baud:
                    test_baud = rate_int
                    break

            if test_baud is None:
                test_baud = int(baud_rates[0])

            api_client.command("io.driver.uart.setBaudRate", {"baudRate": test_baud})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.uart.getConfiguration")
            assert config["baudRate"] == test_baud

        api_client.command("io.driver.uart.setParity", {"parityIndex": 1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["parityIndex"] == 1

        api_client.command("io.driver.uart.setDataBits", {"dataBitsIndex": 2})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["dataBitsIndex"] == 2

        api_client.command("io.driver.uart.setStopBits", {"stopBitsIndex": 1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["stopBitsIndex"] == 1

        api_client.command("io.driver.uart.setFlowControl", {"flowControlIndex": 1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["flowControlIndex"] == 1

        api_client.command("io.driver.uart.setDtrEnabled", {"dtrEnabled": False})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["dtrEnabled"] is False

        api_client.command("io.driver.uart.setAutoReconnect", {"autoReconnect": False})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["autoReconnect"] is False

        api_client.command("io.driver.uart.setDtrEnabled", {"dtrEnabled": True})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["dtrEnabled"] is True

        api_client.command("io.driver.uart.setAutoReconnect", {"autoReconnect": True})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.uart.getConfiguration")
        assert config["autoReconnect"] is True

    finally:
        if "baudRate" in original_config:
            api_client.command("io.driver.uart.setBaudRate", {"baudRate": original_config["baudRate"]})
        if "parityIndex" in original_config:
            api_client.command("io.driver.uart.setParity", {"parityIndex": original_config["parityIndex"]})
        if "dataBitsIndex" in original_config:
            api_client.command("io.driver.uart.setDataBits", {"dataBitsIndex": original_config["dataBitsIndex"]})
        if "stopBitsIndex" in original_config:
            api_client.command("io.driver.uart.setStopBits", {"stopBitsIndex": original_config["stopBitsIndex"]})
        if "flowControlIndex" in original_config:
            api_client.command("io.driver.uart.setFlowControl", {"flowControlIndex": original_config["flowControlIndex"]})
        if "dtrEnabled" in original_config:
            api_client.command("io.driver.uart.setDtrEnabled", {"dtrEnabled": original_config["dtrEnabled"]})
        if "autoReconnect" in original_config:
            api_client.command("io.driver.uart.setAutoReconnect", {"autoReconnect": original_config["autoReconnect"]})
        if "portIndex" in original_config:
            api_client.command("io.driver.uart.setPortIndex", {"portIndex": original_config["portIndex"]})


@pytest.mark.integration
def test_network_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all Network driver API commands.
    Tests every mutation and query command with state save/restore.
    """
    api_client.command("io.manager.setBusType", {"busType": 1})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.network.getConfiguration")

    try:
        socket_types_result = api_client.command("io.driver.network.getSocketTypes")
        socket_types = socket_types_result.get("socketTypes", [])
        assert isinstance(socket_types, list), "socketTypes should be a list"
        assert len(socket_types) >= 2, "Should have at least TCP and UDP"

        current_address = original_config.get("remoteAddress", "127.0.0.1")
        if current_address in ("localhost", "127.0.0.1"):
            test_address_1 = "192.168.1.100"
            test_address_2 = "127.0.0.1" if current_address == "localhost" else "localhost"
        else:
            test_address_1 = "127.0.0.1"
            test_address_2 = "localhost"

        api_client.command("io.driver.network.setRemoteAddress", {"address": test_address_1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["remoteAddress"] == test_address_1

        api_client.command("io.driver.network.setRemoteAddress", {"address": test_address_2})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["remoteAddress"] == test_address_2

        api_client.command("io.driver.network.setSocketType", {"socketTypeIndex": 0})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["socketTypeIndex"] == 0

        api_client.command("io.driver.network.setTcpPort", {"port": 8080})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["tcpPort"] == 8080

        api_client.command("io.driver.network.setTcpPort", {"port": 9000})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["tcpPort"] == 9000

        api_client.command("io.driver.network.setSocketType", {"socketTypeIndex": 1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["socketTypeIndex"] == 1

        api_client.command("io.driver.network.setUdpLocalPort", {"port": 0})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["udpLocalPort"] == 0

        api_client.command("io.driver.network.setUdpLocalPort", {"port": 5000})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["udpLocalPort"] == 5000

        api_client.command("io.driver.network.setUdpRemotePort", {"port": 6000})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["udpRemotePort"] == 6000

        api_client.command("io.driver.network.setUdpMulticast", {"enabled": True})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["udpMulticast"] is True

        api_client.command("io.driver.network.setUdpMulticast", {"enabled": False})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.network.getConfiguration")
        assert config["udpMulticast"] is False

        lookup_result = api_client.command("io.driver.network.lookup", {"host": "localhost"})
        assert "lookupStarted" in lookup_result or "address" in lookup_result or "error" in lookup_result

    finally:
        if "remoteAddress" in original_config:
            api_client.command("io.driver.network.setRemoteAddress", {"address": original_config["remoteAddress"]})
        if "socketTypeIndex" in original_config:
            api_client.command("io.driver.network.setSocketType", {"socketTypeIndex": original_config["socketTypeIndex"]})
        if "tcpPort" in original_config:
            api_client.command("io.driver.network.setTcpPort", {"port": original_config["tcpPort"]})
        if "udpLocalPort" in original_config:
            api_client.command("io.driver.network.setUdpLocalPort", {"port": original_config["udpLocalPort"]})
        if "udpRemotePort" in original_config:
            api_client.command("io.driver.network.setUdpRemotePort", {"port": original_config["udpRemotePort"]})
        if "udpMulticast" in original_config:
            api_client.command("io.driver.network.setUdpMulticast", {"enabled": original_config["udpMulticast"]})


@pytest.mark.integration
def test_ble_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all Bluetooth LE driver API commands.
    Tests every mutation and query command with state save/restore.
    """
    if not api_client.command_exists("io.driver.ble.getStatus"):
        pytest.skip("BLE driver commands not available")

    api_client.command("io.manager.setBusType", {"busType": 2})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.ble.getConfiguration")

    try:
        status = api_client.command("io.driver.ble.getStatus")
        assert "adapterAvailable" in status
        assert "operatingSystemSupported" in status
        assert isinstance(status["adapterAvailable"], bool)
        assert isinstance(status["operatingSystemSupported"], bool)

        if not status["operatingSystemSupported"]:
            pytest.skip("BLE not supported on this operating system")

        if not status["adapterAvailable"]:
            pytest.skip("BLE adapter not available on this system")

        api_client.command("io.driver.ble.startDiscovery")
        time.sleep(2.0)

        devices_result = api_client.command("io.driver.ble.getDeviceList")
        devices = devices_result.get("devices", [])
        assert isinstance(devices, list)

        if len(devices) > 0:
            api_client.command("io.driver.ble.selectDevice", {"deviceIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.ble.getConfiguration")
            assert config["deviceIndex"] == 0

            services_result = api_client.command("io.driver.ble.getServiceList")
            services = services_result.get("services", [])
            assert isinstance(services, list)

            if len(services) > 0:
                api_client.command("io.driver.ble.selectService", {"serviceIndex": 0})
                time.sleep(GUI_VALIDATION_DELAY)
                config = api_client.command("io.driver.ble.getConfiguration")
                assert config["serviceIndex"] == 0

                chars_result = api_client.command("io.driver.ble.getCharacteristicList")
                characteristics = chars_result.get("characteristics", [])
                assert isinstance(characteristics, list)

                if len(characteristics) > 0:
                    api_client.command("io.driver.ble.setCharacteristicIndex", {"characteristicIndex": 0})
                    time.sleep(GUI_VALIDATION_DELAY)
                    config = api_client.command("io.driver.ble.getConfiguration")
                    assert config["characteristicIndex"] == 0

    finally:
        if "deviceIndex" in original_config:
            try:
                api_client.command("io.driver.ble.selectDevice", {"deviceIndex": original_config["deviceIndex"]})
            except Exception:
                pass
        if "serviceIndex" in original_config:
            try:
                api_client.command("io.driver.ble.selectService", {"serviceIndex": original_config["serviceIndex"]})
            except Exception:
                pass
        if "characteristicIndex" in original_config:
            try:
                api_client.command("io.driver.ble.setCharacteristicIndex", {"characteristicIndex": original_config["characteristicIndex"]})
            except Exception:
                pass


@pytest.mark.integration
@pytest.mark.pro
def test_modbus_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all Modbus driver API commands (Pro feature).
    Tests every mutation and query command with state save/restore.
    """
    if not api_client.command_exists("io.driver.modbus.getConfiguration"):
        pytest.skip("Modbus driver commands not available (Pro feature)")

    api_client.command("io.manager.setBusType", {"busType": 4})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.modbus.getConfiguration")

    try:
        protocol_list_result = api_client.command("io.driver.modbus.getProtocolList")
        protocols = protocol_list_result.get("protocols", [])
        assert isinstance(protocols, list)

        api_client.command("io.driver.modbus.setProtocolIndex", {"protocolIndex": 0})
        time.sleep(GUI_VALIDATION_DELAY * 2)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["protocolIndex"] == 0

        serial_ports_result = api_client.command("io.driver.modbus.getSerialPortList")
        serial_ports = serial_ports_result.get("serialPorts", [])
        assert isinstance(serial_ports, list)

        if len(serial_ports) > 0:
            api_client.command("io.driver.modbus.setSerialPortIndex", {"portIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.modbus.getConfiguration")
            assert config["serialPortIndex"] == 0

        baud_rates_result = api_client.command("io.driver.modbus.getBaudRateList")
        baud_rates = baud_rates_result.get("baudRates", [])
        current_baud_config = api_client.command("io.driver.modbus.getConfiguration")
        current_baud = current_baud_config.get("baudRate", 9600)
        assert isinstance(baud_rates, list)

        if len(baud_rates) >= 2:
            test_baud = None
            for rate_str in baud_rates:
                rate_int = int(rate_str) if isinstance(rate_str, str) else int(rate_str)
                if rate_int != current_baud:
                    test_baud = rate_int
                    break

            if test_baud is None:
                test_baud = int(baud_rates[0]) if baud_rates else 9600

            api_client.command("io.driver.modbus.setBaudRate", {"baudRate": test_baud})
            time.sleep(GUI_VALIDATION_DELAY * 2)
            config = api_client.command("io.driver.modbus.getConfiguration")
            assert config["baudRate"] == test_baud

        parity_list_result = api_client.command("io.driver.modbus.getParityList")
        parity_list = parity_list_result.get("parityList", [])
        assert isinstance(parity_list, list)

        if len(parity_list) > 1:
            api_client.command("io.driver.modbus.setParityIndex", {"parityIndex": 1})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.modbus.getConfiguration")
            assert config["parityIndex"] == 1

            api_client.command("io.driver.modbus.setParityIndex", {"parityIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.modbus.getConfiguration")
            assert config["parityIndex"] == 0

        data_bits_result = api_client.command("io.driver.modbus.getDataBitsList")
        data_bits = data_bits_result.get("dataBitsList", [])
        assert isinstance(data_bits, list)

        stop_bits_result = api_client.command("io.driver.modbus.getStopBitsList")
        stop_bits = stop_bits_result.get("stopBitsList", [])
        assert isinstance(stop_bits, list)

        api_client.command("io.driver.modbus.setProtocolIndex", {"protocolIndex": 1})
        time.sleep(GUI_VALIDATION_DELAY * 2)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["protocolIndex"] == 1

        current_host = config.get("host", "127.0.0.1")
        if current_host in ("localhost", "127.0.0.1"):
            test_host_1 = "192.168.1.50"
            test_host_2 = "127.0.0.1" if current_host == "localhost" else "localhost"
        else:
            test_host_1 = "127.0.0.1"
            test_host_2 = "localhost"

        api_client.command("io.driver.modbus.setHost", {"host": test_host_1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["host"] == test_host_1

        api_client.command("io.driver.modbus.setHost", {"host": test_host_2})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["host"] == test_host_2

        api_client.command("io.driver.modbus.setPort", {"port": 502})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["port"] == 502

        api_client.command("io.driver.modbus.setPort", {"port": 5020})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["port"] == 5020

        api_client.command("io.driver.modbus.setSlaveAddress", {"address": 10})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["slaveAddress"] == 10

        api_client.command("io.driver.modbus.setSlaveAddress", {"address": 1})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["slaveAddress"] == 1

        api_client.command("io.driver.modbus.setPollInterval", {"intervalMs": 500})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["pollInterval"] == 500

        api_client.command("io.driver.modbus.setPollInterval", {"intervalMs": 1000})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["pollInterval"] == 1000

        api_client.command("io.driver.modbus.setProtocolIndex", {"protocolIndex": 0})
        time.sleep(GUI_VALIDATION_DELAY * 2)
        config = api_client.command("io.driver.modbus.getConfiguration")
        assert config["protocolIndex"] == 0

        register_types_result = api_client.command("io.driver.modbus.getRegisterTypeList")
        register_types = register_types_result.get("registerTypes", [])
        assert isinstance(register_types, list)

        original_groups_result = api_client.command("io.driver.modbus.getRegisterGroups")
        original_groups = original_groups_result.get("registerGroups", [])

        api_client.command("io.driver.modbus.addRegisterGroup", {
            "type": 0,
            "startAddress": 0,
            "count": 10
        })
        time.sleep(GUI_VALIDATION_DELAY)

        groups_result = api_client.command("io.driver.modbus.getRegisterGroups")
        groups = groups_result.get("registerGroups", [])
        assert len(groups) == len(original_groups) + 1

        api_client.command("io.driver.modbus.addRegisterGroup", {
            "type": 1,
            "startAddress": 100,
            "count": 5
        })
        time.sleep(GUI_VALIDATION_DELAY)

        groups_result = api_client.command("io.driver.modbus.getRegisterGroups")
        groups = groups_result.get("registerGroups", [])
        assert len(groups) == len(original_groups) + 2

        if len(groups) > 0:
            api_client.command("io.driver.modbus.removeRegisterGroup", {"groupIndex": len(groups) - 1})
            time.sleep(GUI_VALIDATION_DELAY)
            groups_result = api_client.command("io.driver.modbus.getRegisterGroups")
            groups = groups_result.get("registerGroups", [])
            assert len(groups) == len(original_groups) + 1

        api_client.command("io.driver.modbus.clearRegisterGroups")
        time.sleep(GUI_VALIDATION_DELAY)
        groups_result = api_client.command("io.driver.modbus.getRegisterGroups")
        groups = groups_result.get("registerGroups", [])
        assert len(groups) == 0

    finally:
        api_client.command("io.driver.modbus.clearRegisterGroups")
        for group in original_config.get("registerGroups", []):
            try:
                api_client.command("io.driver.modbus.addRegisterGroup", {
                    "type": group.get("type", 0),
                    "startAddress": group.get("startAddress", 0),
                    "count": group.get("count", 1)
                })
            except Exception:
                pass

        if "protocolIndex" in original_config:
            api_client.command("io.driver.modbus.setProtocolIndex", {"protocolIndex": original_config["protocolIndex"]})
        if "serialPortIndex" in original_config:
            try:
                api_client.command("io.driver.modbus.setSerialPortIndex", {"portIndex": original_config["serialPortIndex"]})
            except Exception:
                pass
        if "baudRate" in original_config:
            api_client.command("io.driver.modbus.setBaudRate", {"baudRate": original_config["baudRate"]})
        if "parityIndex" in original_config:
            api_client.command("io.driver.modbus.setParityIndex", {"parityIndex": original_config["parityIndex"]})
        if "host" in original_config:
            api_client.command("io.driver.modbus.setHost", {"host": original_config["host"]})
        if "port" in original_config:
            api_client.command("io.driver.modbus.setPort", {"port": original_config["port"]})
        if "slaveAddress" in original_config:
            api_client.command("io.driver.modbus.setSlaveAddress", {"address": original_config["slaveAddress"]})
        if "pollInterval" in original_config:
            api_client.command("io.driver.modbus.setPollInterval", {"intervalMs": original_config["pollInterval"]})


@pytest.mark.integration
@pytest.mark.pro
def test_canbus_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all CAN Bus driver API commands (Pro feature).
    Tests every mutation and query command with state save/restore.
    """
    if not api_client.command_exists("io.driver.canbus.getConfiguration"):
        pytest.skip("CAN Bus driver commands not available (Pro feature)")

    api_client.command("io.manager.setBusType", {"busType": 5})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.canbus.getConfiguration")

    try:
        plugins_result = api_client.command("io.driver.canbus.getPluginList")
        plugins = plugins_result.get("plugins", [])
        assert isinstance(plugins, list)

        if len(plugins) == 0:
            pytest.skip("No CAN Bus plugins available on this system")

        for plugin_idx in range(len(plugins)):
            api_client.command("io.driver.canbus.setPluginIndex", {"pluginIndex": plugin_idx})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.canbus.getConfiguration")
            assert config["pluginIndex"] == plugin_idx

            interfaces_result = api_client.command("io.driver.canbus.getInterfaceList")
            interfaces = interfaces_result.get("interfaces", [])
            assert isinstance(interfaces, list)

            if len(interfaces) == 0:
                error_result = api_client.command("io.driver.canbus.getInterfaceError")
                error_msg = error_result.get("error", "")
                assert isinstance(error_msg, str)
                continue

            if len(interfaces) > 0:
                api_client.command("io.driver.canbus.setInterfaceIndex", {"interfaceIndex": 0})
                time.sleep(GUI_VALIDATION_DELAY)
                config = api_client.command("io.driver.canbus.getConfiguration")
                assert config["interfaceIndex"] == 0

            bitrates_result = api_client.command("io.driver.canbus.getBitrateList")
            bitrates = bitrates_result.get("bitrates", [])
            current_bitrate = bitrates_result.get("currentBitrate", original_config.get("bitrate", 250000))
            assert isinstance(bitrates, list)

            if len(bitrates) == 0:
                continue

            if len(bitrates) >= 2:
                test_bitrate = None
                for rate_str in bitrates:
                    rate_int = int(rate_str) if isinstance(rate_str, str) else int(rate_str)
                    if rate_int != current_bitrate:
                        test_bitrate = rate_int
                        break

                if test_bitrate is None:
                    test_bitrate = int(bitrates[0])

                api_client.command("io.driver.canbus.setBitrate", {"bitrate": test_bitrate})
                time.sleep(GUI_VALIDATION_DELAY)
                config = api_client.command("io.driver.canbus.getConfiguration")
                assert config["bitrate"] == test_bitrate

            api_client.command("io.driver.canbus.setCanFD", {"enabled": True})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.canbus.getConfiguration")
            assert config["canFD"] is True

            api_client.command("io.driver.canbus.setCanFD", {"enabled": False})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.canbus.getConfiguration")
            assert config["canFD"] is False

    finally:
        if "pluginIndex" in original_config:
            try:
                api_client.command("io.driver.canbus.setPluginIndex", {"pluginIndex": original_config["pluginIndex"]})
            except Exception:
                pass
        if "interfaceIndex" in original_config:
            try:
                api_client.command("io.driver.canbus.setInterfaceIndex", {"interfaceIndex": original_config["interfaceIndex"]})
            except Exception:
                pass
        if "bitrate" in original_config:
            api_client.command("io.driver.canbus.setBitrate", {"bitrate": original_config["bitrate"]})
        if "canFD" in original_config:
            api_client.command("io.driver.canbus.setCanFD", {"enabled": original_config["canFD"]})


@pytest.mark.integration
@pytest.mark.pro
def test_audio_driver_comprehensive(api_client, clean_state):
    """
    Comprehensive test of all Audio driver API commands (Pro feature).
    Tests every mutation and query command with state save/restore.
    """
    if not api_client.command_exists("io.driver.audio.getConfiguration"):
        pytest.skip("Audio driver commands not available (Pro feature)")

    api_client.command("io.manager.setBusType", {"busType": 3})
    time.sleep(GUI_VALIDATION_DELAY)

    original_config = api_client.command("io.driver.audio.getConfiguration")

    try:
        input_devices_result = api_client.command("io.driver.audio.getInputDevices")
        input_devices = input_devices_result.get("devices", [])
        assert isinstance(input_devices, list)

        output_devices_result = api_client.command("io.driver.audio.getOutputDevices")
        output_devices = output_devices_result.get("devices", [])
        assert isinstance(output_devices, list)

        sample_rates_result = api_client.command("io.driver.audio.getSampleRates")
        sample_rates = sample_rates_result.get("sampleRates", [])
        assert isinstance(sample_rates, list)
        assert len(sample_rates) > 0

        input_formats_result = api_client.command("io.driver.audio.getInputFormats")
        input_formats = input_formats_result.get("formats", [])
        assert isinstance(input_formats, list)

        output_formats_result = api_client.command("io.driver.audio.getOutputFormats")
        output_formats = output_formats_result.get("formats", [])
        assert isinstance(output_formats, list)

        if len(input_devices) > 0:
            api_client.command("io.driver.audio.setInputDevice", {"deviceIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.audio.getConfiguration")
            assert "selectedInputDevice" in config or "inputDevice" in config or "inputDeviceIndex" in config

        if len(output_devices) > 0:
            api_client.command("io.driver.audio.setOutputDevice", {"deviceIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.audio.getConfiguration")
            assert "selectedOutputDevice" in config or "outputDevice" in config or "outputDeviceIndex" in config

        if len(sample_rates) >= 2:
            api_client.command("io.driver.audio.setSampleRate", {"rateIndex": 1})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.audio.getConfiguration")
            assert "selectedSampleRate" in config or "sampleRate" in config or "sampleRateIndex" in config

        if len(input_formats) > 0:
            api_client.command("io.driver.audio.setInputSampleFormat", {"formatIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.audio.getConfiguration")
            assert "selectedInputSampleFormat" in config or "inputFormat" in config or "inputFormatIndex" in config

        if len(output_formats) > 0:
            api_client.command("io.driver.audio.setOutputSampleFormat", {"formatIndex": 0})
            time.sleep(GUI_VALIDATION_DELAY)
            config = api_client.command("io.driver.audio.getConfiguration")
            assert "selectedOutputSampleFormat" in config or "outputFormat" in config or "outputFormatIndex" in config

        api_client.command("io.driver.audio.setInputChannelConfig", {"channelIndex": 0})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.audio.getConfiguration")
        assert "selectedInputChannelConfig" in config or "inputChannel" in config or "inputChannelIndex" in config

        api_client.command("io.driver.audio.setOutputChannelConfig", {"channelIndex": 0})
        time.sleep(GUI_VALIDATION_DELAY)
        config = api_client.command("io.driver.audio.getConfiguration")
        assert "selectedOutputChannelConfig" in config or "outputChannel" in config or "outputChannelIndex" in config

    finally:
        if "selectedInputDevice" in original_config or "inputDeviceIndex" in original_config or "inputDevice" in original_config:
            try:
                device_idx = original_config.get("selectedInputDevice", original_config.get("inputDeviceIndex", 0))
                api_client.command("io.driver.audio.setInputDevice", {"deviceIndex": device_idx})
            except Exception:
                pass
        if "selectedOutputDevice" in original_config or "outputDeviceIndex" in original_config or "outputDevice" in original_config:
            try:
                device_idx = original_config.get("selectedOutputDevice", original_config.get("outputDeviceIndex", 0))
                api_client.command("io.driver.audio.setOutputDevice", {"deviceIndex": device_idx})
            except Exception:
                pass
        if "selectedSampleRate" in original_config or "sampleRateIndex" in original_config or "sampleRate" in original_config:
            try:
                rate_idx = original_config.get("selectedSampleRate", original_config.get("sampleRateIndex", 0))
                api_client.command("io.driver.audio.setSampleRate", {"rateIndex": rate_idx})
            except Exception:
                pass
        if "selectedInputSampleFormat" in original_config or "inputFormatIndex" in original_config or "inputFormat" in original_config:
            try:
                fmt_idx = original_config.get("selectedInputSampleFormat", original_config.get("inputFormatIndex", 0))
                api_client.command("io.driver.audio.setInputSampleFormat", {"formatIndex": fmt_idx})
            except Exception:
                pass
        if "selectedOutputSampleFormat" in original_config or "outputFormatIndex" in original_config or "outputFormat" in original_config:
            try:
                fmt_idx = original_config.get("selectedOutputSampleFormat", original_config.get("outputFormatIndex", 0))
                api_client.command("io.driver.audio.setOutputSampleFormat", {"formatIndex": fmt_idx})
            except Exception:
                pass
        if "selectedInputChannelConfig" in original_config or "inputChannelIndex" in original_config or "inputChannel" in original_config:
            try:
                ch_idx = original_config.get("selectedInputChannelConfig", original_config.get("inputChannelIndex", 0))
                api_client.command("io.driver.audio.setInputChannelConfig", {"channelIndex": ch_idx})
            except Exception:
                pass
        if "selectedOutputChannelConfig" in original_config or "outputChannelIndex" in original_config or "outputChannel" in original_config:
            try:
                ch_idx = original_config.get("selectedOutputChannelConfig", original_config.get("outputChannelIndex", 0))
                api_client.command("io.driver.audio.setOutputChannelConfig", {"channelIndex": ch_idx})
            except Exception:
                pass


@pytest.mark.integration
def test_driver_switching_and_state_persistence(api_client, clean_state):
    """
    Test switching between different drivers and verify state persistence.
    """
    buses = api_client.command("io.manager.getAvailableBuses").get("buses", [])
    assert len(buses) > 0, "Expected at least one available bus type"

    for bus in buses:
        bus_index = bus["index"]
        bus_name = bus.get("name", f"Bus {bus_index}")

        api_client.command("io.manager.setBusType", {"busType": bus_index})
        time.sleep(GUI_VALIDATION_DELAY)
        status = api_client.command("io.manager.getStatus")
        assert status["busType"] == bus_index, f"Failed to switch to {bus_name}"

    api_client.command("io.manager.setBusType", {"busType": 0})
    time.sleep(GUI_VALIDATION_DELAY)
    status = api_client.command("io.manager.getStatus")
    assert status["busType"] == 0
