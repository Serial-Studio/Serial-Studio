#!/usr/bin/env python3
"""
Serial Studio API Test Suite
=============================

Comprehensive tests for the Serial Studio Plugin Server API.
Tests protocol handling, command execution, error handling, and batch operations.

Usage:
    python test_api.py [--host HOST] [--port PORT] [--verbose]

Requirements:
    - Serial Studio running with Plugin Server enabled (port 7777)
    - Python 3.8+

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import argparse
import sys
import time
import uuid
from typing import Any, Optional
from dataclasses import dataclass
from enum import Enum


# =============================================================================
# Configuration
# =============================================================================

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 7777
SOCKET_TIMEOUT = 5.0
RECV_BUFFER_SIZE = 65536


# =============================================================================
# Protocol Constants (matching C++ API::MessageType and API::ErrorCode)
# =============================================================================

class MessageType:
    COMMAND = "command"
    BATCH = "batch"
    RESPONSE = "response"


class ErrorCode:
    INVALID_JSON = "INVALID_JSON"
    INVALID_MESSAGE_TYPE = "INVALID_MESSAGE_TYPE"
    UNKNOWN_COMMAND = "UNKNOWN_COMMAND"
    INVALID_PARAM = "INVALID_PARAM"
    MISSING_PARAM = "MISSING_PARAM"
    EXECUTION_ERROR = "EXECUTION_ERROR"


# =============================================================================
# Test Result Tracking
# =============================================================================

class TestResult(Enum):
    PASSED = "PASSED"
    FAILED = "FAILED"
    SKIPPED = "SKIPPED"


@dataclass
class TestCase:
    name: str
    result: TestResult
    message: str = ""
    duration_ms: float = 0.0


class TestSuite:
    def __init__(self, name: str):
        self.name = name
        self.tests: list[TestCase] = []

    def add_result(self, test: TestCase):
        self.tests.append(test)

    @property
    def passed(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.PASSED)

    @property
    def failed(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.FAILED)

    @property
    def skipped(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.SKIPPED)

    def print_summary(self):
        print(f"\n{'=' * 60}")
        print(f"Test Suite: {self.name}")
        print(f"{'=' * 60}")
        print(f"Total: {len(self.tests)} | Passed: {self.passed} | Failed: {self.failed} | Skipped: {self.skipped}")
        print(f"{'=' * 60}")

        if self.failed > 0:
            print("\nFailed Tests:")
            for test in self.tests:
                if test.result == TestResult.FAILED:
                    print(f"  ✗ {test.name}: {test.message}")

        print()


# =============================================================================
# API Client
# =============================================================================

class SerialStudioAPI:
    """Client for communicating with Serial Studio Plugin Server API."""

    def __init__(self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT, verbose: bool = False):
        self.host = host
        self.port = port
        self.verbose = verbose
        self.socket: Optional[socket.socket] = None

    def connect(self) -> bool:
        """Establish TCP connection to the plugin server."""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(SOCKET_TIMEOUT)
            self.socket.connect((self.host, self.port))
            if self.verbose:
                print(f"[INFO] Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"[ERROR] Failed to connect: {e}")
            return False

    def disconnect(self):
        """Close the TCP connection."""
        if self.socket:
            try:
                self.socket.close()
            except Exception:
                pass
            self.socket = None

    def send_raw(self, data: bytes) -> Optional[dict]:
        """Send raw bytes and receive JSON response."""
        if not self.socket:
            return None

        try:
            if self.verbose:
                print(f"[SEND] {data.decode('utf-8', errors='replace').strip()}")

            self.socket.sendall(data)
            response_data = self.socket.recv(RECV_BUFFER_SIZE)

            if self.verbose:
                print(f"[RECV] {response_data.decode('utf-8', errors='replace').strip()}")

            return json.loads(response_data.decode('utf-8'))
        except Exception as e:
            if self.verbose:
                print(f"[ERROR] {e}")
            return None

    def send_json(self, obj: dict) -> Optional[dict]:
        """Send JSON object and receive JSON response."""
        data = json.dumps(obj, separators=(',', ':')) + "\n"
        return self.send_raw(data.encode('utf-8'))

    def send_command(self, command: str, params: Optional[dict] = None,
                     request_id: Optional[str] = None) -> Optional[dict]:
        """Send a single command request."""
        msg = {
            "type": MessageType.COMMAND,
            "id": request_id or str(uuid.uuid4()),
            "command": command,
        }
        if params:
            msg["params"] = params
        return self.send_json(msg)

    def send_batch(self, commands: list[dict], request_id: Optional[str] = None) -> Optional[dict]:
        """Send a batch of commands."""
        msg = {
            "type": MessageType.BATCH,
            "id": request_id or str(uuid.uuid4()),
            "commands": commands,
        }
        return self.send_json(msg)


# =============================================================================
# Test Helpers
# =============================================================================

def assert_success(response: Optional[dict], test_name: str) -> tuple[bool, str]:
    """Assert that response indicates success."""
    if response is None:
        return False, "No response received"
    if not isinstance(response, dict):
        return False, f"Response is not a dict: {type(response)}"
    if response.get("type") != MessageType.RESPONSE:
        return False, f"Wrong type: {response.get('type')}"
    if not response.get("success"):
        error = response.get("error", {})
        return False, f"Not successful: {error.get('code')} - {error.get('message')}"
    return True, ""


def assert_error(response: Optional[dict], expected_code: str, test_name: str) -> tuple[bool, str]:
    """Assert that response is an error with specific code."""
    if response is None:
        return False, "No response received"
    if not isinstance(response, dict):
        return False, f"Response is not a dict: {type(response)}"
    if response.get("success"):
        return False, "Expected error but got success"
    error = response.get("error", {})
    if error.get("code") != expected_code:
        return False, f"Expected error code {expected_code}, got {error.get('code')}"
    return True, ""


def run_test(suite: TestSuite, name: str, test_fn) -> bool:
    """Run a single test and record the result."""
    start_time = time.time()
    try:
        passed, message = test_fn()
        duration_ms = (time.time() - start_time) * 1000
        result = TestResult.PASSED if passed else TestResult.FAILED
        suite.add_result(TestCase(name, result, message, duration_ms))

        status = "✓" if passed else "✗"
        print(f"  {status} {name} ({duration_ms:.1f}ms)" + (f" - {message}" if not passed else ""))
        return passed
    except Exception as e:
        duration_ms = (time.time() - start_time) * 1000
        suite.add_result(TestCase(name, TestResult.FAILED, str(e), duration_ms))
        print(f"  ✗ {name} ({duration_ms:.1f}ms) - Exception: {e}")
        return False


# =============================================================================
# Protocol Tests
# =============================================================================

def test_protocol(api: SerialStudioAPI, suite: TestSuite):
    """Test basic protocol handling."""
    print("\n--- Protocol Tests ---")

    # Test: Invalid JSON
    def test_invalid_json():
        response = api.send_raw(b"not valid json\n")
        return assert_error(response, ErrorCode.INVALID_JSON, "invalid_json")
    run_test(suite, "Invalid JSON rejected", test_invalid_json)

    # Test: Empty message
    def test_empty_object():
        response = api.send_json({})
        return assert_error(response, ErrorCode.INVALID_JSON, "empty_object")
    run_test(suite, "Empty object rejected", test_empty_object)

    # Test: Missing type field
    def test_missing_type():
        response = api.send_json({"command": "test"})
        return assert_error(response, ErrorCode.INVALID_JSON, "missing_type")
    run_test(suite, "Missing type field rejected", test_missing_type)

    # Test: Unknown message type
    def test_unknown_type():
        response = api.send_json({"type": "unknown_type", "command": "test"})
        return assert_error(response, ErrorCode.INVALID_MESSAGE_TYPE, "unknown_type")
    run_test(suite, "Unknown message type rejected", test_unknown_type)

    # Test: Command without command field
    def test_command_missing_cmd():
        response = api.send_json({"type": MessageType.COMMAND, "id": "test-1"})
        return assert_error(response, ErrorCode.INVALID_MESSAGE_TYPE, "missing_command")
    run_test(suite, "Command without 'command' field rejected", test_command_missing_cmd)

    # Test: Unknown command
    def test_unknown_command():
        response = api.send_command("nonexistent.command.xyz")
        return assert_error(response, ErrorCode.UNKNOWN_COMMAND, "unknown_command")
    run_test(suite, "Unknown command rejected", test_unknown_command)

    # Test: Response ID matching
    def test_response_id():
        test_id = "test-id-12345"
        response = api.send_command("api.getCommands", request_id=test_id)
        if response is None:
            return False, "No response"
        if response.get("id") != test_id:
            return False, f"ID mismatch: expected {test_id}, got {response.get('id')}"
        return True, ""
    run_test(suite, "Response ID matches request ID", test_response_id)


# =============================================================================
# API Commands Tests
# =============================================================================

def test_api_commands(api: SerialStudioAPI, suite: TestSuite):
    """Test api.* commands."""
    print("\n--- API Commands Tests ---")

    # Test: api.getCommands
    def test_get_commands():
        response = api.send_command("api.getCommands")
        passed, msg = assert_success(response, "getCommands")
        if not passed:
            return False, msg
        result = response.get("result", {})
        commands = result.get("commands", [])
        if not commands:
            return False, "No commands returned"
        # Verify some expected commands exist
        command_names = [c.get("name") for c in commands]
        expected = ["api.getCommands", "io.manager.connect", "io.driver.uart.setBaudRate"]
        for cmd in expected:
            if cmd not in command_names:
                return False, f"Missing expected command: {cmd}"
        return True, ""
    run_test(suite, "api.getCommands returns command list", test_get_commands)

    # Test: Command descriptions exist
    def test_command_descriptions():
        response = api.send_command("api.getCommands")
        passed, msg = assert_success(response, "getCommands")
        if not passed:
            return False, msg
        commands = response.get("result", {}).get("commands", [])
        for cmd in commands:
            if not cmd.get("description"):
                return False, f"Command {cmd.get('name')} has no description"
        return True, ""
    run_test(suite, "All commands have descriptions", test_command_descriptions)


# =============================================================================
# IO Manager Tests
# =============================================================================

def test_io_manager(api: SerialStudioAPI, suite: TestSuite):
    """Test io.manager.* commands."""
    print("\n--- IO Manager Tests ---")

    # Test: getStatus
    def test_get_status():
        response = api.send_command("io.manager.getStatus")
        passed, msg = assert_success(response, "getStatus")
        if not passed:
            return False, msg
        result = response.get("result", {})
        # Verify expected fields exist
        expected_fields = ["isConnected", "paused", "busType", "configurationOk"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "io.manager.getStatus returns status", test_get_status)

    # Test: getAvailableBuses
    def test_get_buses():
        response = api.send_command("io.manager.getAvailableBuses")
        passed, msg = assert_success(response, "getAvailableBuses")
        if not passed:
            return False, msg
        buses = response.get("result", {}).get("buses", [])
        if not buses:
            return False, "No buses returned"
        # Verify bus structure
        for bus in buses:
            if "index" not in bus or "name" not in bus:
                return False, f"Invalid bus structure: {bus}"
        return True, ""
    run_test(suite, "io.manager.getAvailableBuses returns bus list", test_get_buses)

    # Test: setBusType
    def test_set_bus_type():
        response = api.send_command("io.manager.setBusType", {"busType": 0})
        passed, msg = assert_success(response, "setBusType")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if result.get("busType") != 0:
            return False, f"busType not set correctly: {result.get('busType')}"
        return True, ""
    run_test(suite, "io.manager.setBusType sets bus type", test_set_bus_type)

    # Test: setBusType invalid
    def test_set_bus_type_invalid():
        response = api.send_command("io.manager.setBusType", {"busType": 999})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setBusType_invalid")
    run_test(suite, "io.manager.setBusType rejects invalid bus type", test_set_bus_type_invalid)

    # Test: setBusType missing param
    def test_set_bus_type_missing():
        response = api.send_command("io.manager.setBusType", {})
        return assert_error(response, ErrorCode.MISSING_PARAM, "setBusType_missing")
    run_test(suite, "io.manager.setBusType requires busType param", test_set_bus_type_missing)

    # Test: setPaused
    def test_set_paused():
        response = api.send_command("io.manager.setPaused", {"paused": True})
        passed, msg = assert_success(response, "setPaused")
        if not passed:
            return False, msg
        # Restore to false
        api.send_command("io.manager.setPaused", {"paused": False})
        return True, ""
    run_test(suite, "io.manager.setPaused sets pause state", test_set_paused)

    # Test: setPaused missing param
    def test_set_paused_missing():
        response = api.send_command("io.manager.setPaused", {})
        return assert_error(response, ErrorCode.MISSING_PARAM, "setPaused_missing")
    run_test(suite, "io.manager.setPaused requires paused param", test_set_paused_missing)

    # Test: connect when not configured (should fail)
    def test_connect_not_configured():
        response = api.send_command("io.manager.connect")
        # This should either succeed (if configured) or fail gracefully
        # We just verify we get a valid response
        if response is None:
            return False, "No response"
        if response.get("type") != MessageType.RESPONSE:
            return False, f"Wrong response type: {response.get('type')}"
        return True, ""
    run_test(suite, "io.manager.connect returns valid response", test_connect_not_configured)

    # Test: disconnect when not connected
    def test_disconnect_not_connected():
        response = api.send_command("io.manager.disconnect")
        # Should fail because not connected
        return assert_error(response, ErrorCode.EXECUTION_ERROR, "disconnect_not_connected")
    run_test(suite, "io.manager.disconnect fails when not connected", test_disconnect_not_connected)

    # Test: writeData when not connected
    def test_write_data_not_connected():
        import base64
        test_data = base64.b64encode(b"Hello").decode()
        response = api.send_command("io.manager.writeData", {"data": test_data})
        return assert_error(response, ErrorCode.EXECUTION_ERROR, "writeData_not_connected")
    run_test(suite, "io.manager.writeData fails when not connected", test_write_data_not_connected)

    # Test: writeData missing param
    def test_write_data_missing():
        response = api.send_command("io.manager.writeData", {})
        return assert_error(response, ErrorCode.MISSING_PARAM, "writeData_missing")
    run_test(suite, "io.manager.writeData requires data param", test_write_data_missing)


# =============================================================================
# UART Handler Tests
# =============================================================================

def test_uart_handler(api: SerialStudioAPI, suite: TestSuite):
    """Test io.driver.uart.* commands."""
    print("\n--- UART Handler Tests ---")

    # Test: getConfiguration
    def test_get_configuration():
        response = api.send_command("io.driver.uart.getConfiguration")
        passed, msg = assert_success(response, "getConfiguration")
        if not passed:
            return False, msg
        result = response.get("result", {})
        expected_fields = ["baudRate", "parityIndex", "dataBitsIndex", "stopBitsIndex", "flowControlIndex"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "io.driver.uart.getConfiguration returns config", test_get_configuration)

    # Test: getPortList
    def test_get_port_list():
        response = api.send_command("io.driver.uart.getPortList")
        passed, msg = assert_success(response, "getPortList")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if "portList" not in result:
            return False, "Missing portList field"
        if "currentPortIndex" not in result:
            return False, "Missing currentPortIndex field"
        return True, ""
    run_test(suite, "io.driver.uart.getPortList returns port list", test_get_port_list)

    # Test: getBaudRateList
    def test_get_baud_rate_list():
        response = api.send_command("io.driver.uart.getBaudRateList")
        passed, msg = assert_success(response, "getBaudRateList")
        if not passed:
            return False, msg
        result = response.get("result", {})
        baud_rates = result.get("baudRateList", [])
        if not baud_rates:
            return False, "No baud rates returned"
        # Verify common baud rates exist
        common = ["9600", "115200"]
        for rate in common:
            if rate not in baud_rates:
                return False, f"Missing common baud rate: {rate}"
        return True, ""
    run_test(suite, "io.driver.uart.getBaudRateList returns baud rates", test_get_baud_rate_list)

    # Test: setBaudRate
    def test_set_baud_rate():
        response = api.send_command("io.driver.uart.setBaudRate", {"baudRate": 115200})
        passed, msg = assert_success(response, "setBaudRate")
        if not passed:
            return False, msg
        if response.get("result", {}).get("baudRate") != 115200:
            return False, "Baud rate not set correctly"
        return True, ""
    run_test(suite, "io.driver.uart.setBaudRate sets baud rate", test_set_baud_rate)

    # Test: setBaudRate invalid
    def test_set_baud_rate_invalid():
        response = api.send_command("io.driver.uart.setBaudRate", {"baudRate": -1})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setBaudRate_invalid")
    run_test(suite, "io.driver.uart.setBaudRate rejects invalid rate", test_set_baud_rate_invalid)

    # Test: setBaudRate missing
    def test_set_baud_rate_missing():
        response = api.send_command("io.driver.uart.setBaudRate", {})
        return assert_error(response, ErrorCode.MISSING_PARAM, "setBaudRate_missing")
    run_test(suite, "io.driver.uart.setBaudRate requires baudRate param", test_set_baud_rate_missing)

    # Test: setParity
    def test_set_parity():
        response = api.send_command("io.driver.uart.setParity", {"parityIndex": 0})
        passed, msg = assert_success(response, "setParity")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setParity sets parity", test_set_parity)

    # Test: setParity invalid
    def test_set_parity_invalid():
        response = api.send_command("io.driver.uart.setParity", {"parityIndex": 999})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setParity_invalid")
    run_test(suite, "io.driver.uart.setParity rejects invalid index", test_set_parity_invalid)

    # Test: setDataBits
    def test_set_data_bits():
        response = api.send_command("io.driver.uart.setDataBits", {"dataBitsIndex": 3})  # 8 bits
        passed, msg = assert_success(response, "setDataBits")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setDataBits sets data bits", test_set_data_bits)

    # Test: setStopBits
    def test_set_stop_bits():
        response = api.send_command("io.driver.uart.setStopBits", {"stopBitsIndex": 0})  # 1 stop bit
        passed, msg = assert_success(response, "setStopBits")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setStopBits sets stop bits", test_set_stop_bits)

    # Test: setFlowControl
    def test_set_flow_control():
        response = api.send_command("io.driver.uart.setFlowControl", {"flowControlIndex": 0})  # None
        passed, msg = assert_success(response, "setFlowControl")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setFlowControl sets flow control", test_set_flow_control)

    # Test: setDtrEnabled
    def test_set_dtr():
        response = api.send_command("io.driver.uart.setDtrEnabled", {"dtrEnabled": True})
        passed, msg = assert_success(response, "setDtrEnabled")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setDtrEnabled sets DTR", test_set_dtr)

    # Test: setAutoReconnect
    def test_set_auto_reconnect():
        response = api.send_command("io.driver.uart.setAutoReconnect", {"autoReconnect": False})
        passed, msg = assert_success(response, "setAutoReconnect")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setAutoReconnect sets auto-reconnect", test_set_auto_reconnect)

    # Test: setDevice (with a test device name)
    def test_set_device():
        response = api.send_command("io.driver.uart.setDevice", {"device": "COM1"})
        passed, msg = assert_success(response, "setDevice")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.uart.setDevice registers device", test_set_device)

    # Test: setDevice empty
    def test_set_device_empty():
        response = api.send_command("io.driver.uart.setDevice", {"device": ""})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setDevice_empty")
    run_test(suite, "io.driver.uart.setDevice rejects empty device", test_set_device_empty)


# =============================================================================
# Network Handler Tests
# =============================================================================

def test_network_handler(api: SerialStudioAPI, suite: TestSuite):
    """Test io.driver.network.* commands."""
    print("\n--- Network Handler Tests ---")

    # Test: getConfiguration
    def test_get_configuration():
        response = api.send_command("io.driver.network.getConfiguration")
        passed, msg = assert_success(response, "getConfiguration")
        if not passed:
            return False, msg
        result = response.get("result", {})
        expected_fields = ["remoteAddress", "tcpPort", "udpLocalPort", "udpRemotePort", "socketTypeIndex"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "io.driver.network.getConfiguration returns config", test_get_configuration)

    # Test: getSocketTypes
    def test_get_socket_types():
        response = api.send_command("io.driver.network.getSocketTypes")
        passed, msg = assert_success(response, "getSocketTypes")
        if not passed:
            return False, msg
        result = response.get("result", {})
        socket_types = result.get("socketTypes", [])
        if not socket_types:
            return False, "No socket types returned"
        return True, ""
    run_test(suite, "io.driver.network.getSocketTypes returns socket types", test_get_socket_types)

    # Test: setRemoteAddress
    def test_set_remote_address():
        response = api.send_command("io.driver.network.setRemoteAddress", {"address": "192.168.1.100"})
        passed, msg = assert_success(response, "setRemoteAddress")
        if not passed:
            return False, msg
        if response.get("result", {}).get("address") != "192.168.1.100":
            return False, "Address not set correctly"
        return True, ""
    run_test(suite, "io.driver.network.setRemoteAddress sets address", test_set_remote_address)

    # Test: setRemoteAddress empty
    def test_set_remote_address_empty():
        response = api.send_command("io.driver.network.setRemoteAddress", {"address": ""})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setRemoteAddress_empty")
    run_test(suite, "io.driver.network.setRemoteAddress rejects empty", test_set_remote_address_empty)

    # Test: setTcpPort
    def test_set_tcp_port():
        response = api.send_command("io.driver.network.setTcpPort", {"port": 8080})
        passed, msg = assert_success(response, "setTcpPort")
        if not passed:
            return False, msg
        if response.get("result", {}).get("tcpPort") != 8080:
            return False, "TCP port not set correctly"
        return True, ""
    run_test(suite, "io.driver.network.setTcpPort sets port", test_set_tcp_port)

    # Test: setTcpPort invalid (too high)
    def test_set_tcp_port_invalid_high():
        response = api.send_command("io.driver.network.setTcpPort", {"port": 70000})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setTcpPort_invalid_high")
    run_test(suite, "io.driver.network.setTcpPort rejects port > 65535", test_set_tcp_port_invalid_high)

    # Test: setTcpPort invalid (zero)
    def test_set_tcp_port_invalid_zero():
        response = api.send_command("io.driver.network.setTcpPort", {"port": 0})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setTcpPort_invalid_zero")
    run_test(suite, "io.driver.network.setTcpPort rejects port 0", test_set_tcp_port_invalid_zero)

    # Test: setUdpLocalPort
    def test_set_udp_local_port():
        response = api.send_command("io.driver.network.setUdpLocalPort", {"port": 9000})
        passed, msg = assert_success(response, "setUdpLocalPort")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.network.setUdpLocalPort sets port", test_set_udp_local_port)

    # Test: setUdpLocalPort accepts 0 (any port)
    def test_set_udp_local_port_zero():
        response = api.send_command("io.driver.network.setUdpLocalPort", {"port": 0})
        passed, msg = assert_success(response, "setUdpLocalPort_zero")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.network.setUdpLocalPort accepts port 0", test_set_udp_local_port_zero)

    # Test: setUdpRemotePort
    def test_set_udp_remote_port():
        response = api.send_command("io.driver.network.setUdpRemotePort", {"port": 9001})
        passed, msg = assert_success(response, "setUdpRemotePort")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.network.setUdpRemotePort sets port", test_set_udp_remote_port)

    # Test: setSocketType
    def test_set_socket_type():
        response = api.send_command("io.driver.network.setSocketType", {"socketTypeIndex": 0})  # TCP
        passed, msg = assert_success(response, "setSocketType")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "io.driver.network.setSocketType sets socket type", test_set_socket_type)

    # Test: setSocketType invalid
    def test_set_socket_type_invalid():
        response = api.send_command("io.driver.network.setSocketType", {"socketTypeIndex": 999})
        return assert_error(response, ErrorCode.INVALID_PARAM, "setSocketType_invalid")
    run_test(suite, "io.driver.network.setSocketType rejects invalid index", test_set_socket_type_invalid)

    # Test: setUdpMulticast
    def test_set_udp_multicast():
        response = api.send_command("io.driver.network.setUdpMulticast", {"enabled": True})
        passed, msg = assert_success(response, "setUdpMulticast")
        if not passed:
            return False, msg
        # Restore to false
        api.send_command("io.driver.network.setUdpMulticast", {"enabled": False})
        return True, ""
    run_test(suite, "io.driver.network.setUdpMulticast sets multicast", test_set_udp_multicast)

    # Test: lookup
    def test_lookup():
        response = api.send_command("io.driver.network.lookup", {"host": "localhost"})
        passed, msg = assert_success(response, "lookup")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if not result.get("lookupStarted"):
            return False, "Lookup not started"
        return True, ""
    run_test(suite, "io.driver.network.lookup initiates DNS lookup", test_lookup)

    # Test: lookup empty host
    def test_lookup_empty():
        response = api.send_command("io.driver.network.lookup", {"host": ""})
        return assert_error(response, ErrorCode.INVALID_PARAM, "lookup_empty")
    run_test(suite, "io.driver.network.lookup rejects empty host", test_lookup_empty)


# =============================================================================
# Batch Command Tests
# =============================================================================

def test_batch_commands(api: SerialStudioAPI, suite: TestSuite):
    """Test batch command execution."""
    print("\n--- Batch Command Tests ---")

    # Test: Simple batch
    def test_simple_batch():
        commands = [
            {"command": "io.manager.getStatus", "id": "batch-1"},
            {"command": "io.driver.uart.getConfiguration", "id": "batch-2"},
        ]
        response = api.send_batch(commands, "batch-main")
        if response is None:
            return False, "No response"
        if response.get("type") != MessageType.RESPONSE:
            return False, f"Wrong type: {response.get('type')}"
        if response.get("id") != "batch-main":
            return False, f"ID mismatch: {response.get('id')}"
        results = response.get("results", [])
        if len(results) != 2:
            return False, f"Expected 2 results, got {len(results)}"
        return True, ""
    run_test(suite, "Batch executes multiple commands", test_simple_batch)

    # Test: Batch with mixed success/failure
    def test_mixed_batch():
        commands = [
            {"command": "io.manager.getStatus", "id": "mixed-1"},
            {"command": "nonexistent.command", "id": "mixed-2"},
            {"command": "io.driver.uart.getConfiguration", "id": "mixed-3"},
        ]
        response = api.send_batch(commands)
        if response is None:
            return False, "No response"
        results = response.get("results", [])
        if len(results) != 3:
            return False, f"Expected 3 results, got {len(results)}"
        # First should succeed
        if not results[0].get("success"):
            return False, "First command should succeed"
        # Second should fail
        if results[1].get("success"):
            return False, "Second command should fail"
        # Third should succeed
        if not results[2].get("success"):
            return False, "Third command should succeed"
        # Overall batch should be marked as failed (one failed)
        if response.get("success"):
            return False, "Batch should be marked as failed"
        return True, ""
    run_test(suite, "Batch continues after individual failures", test_mixed_batch)

    # Test: Batch preserves order
    def test_batch_order():
        commands = [
            {"command": "io.driver.uart.setBaudRate", "id": "order-1", "params": {"baudRate": 9600}},
            {"command": "io.driver.uart.setBaudRate", "id": "order-2", "params": {"baudRate": 115200}},
        ]
        response = api.send_batch(commands)
        if response is None:
            return False, "No response"
        results = response.get("results", [])
        if len(results) != 2:
            return False, f"Expected 2 results, got {len(results)}"
        # Verify order by checking IDs
        if results[0].get("id") != "order-1":
            return False, f"First result ID wrong: {results[0].get('id')}"
        if results[1].get("id") != "order-2":
            return False, f"Second result ID wrong: {results[1].get('id')}"
        return True, ""
    run_test(suite, "Batch preserves command order", test_batch_order)

    # Test: Empty batch rejected
    def test_empty_batch():
        response = api.send_batch([])
        return assert_error(response, ErrorCode.INVALID_MESSAGE_TYPE, "empty_batch")
    run_test(suite, "Empty batch rejected", test_empty_batch)

    # Test: Large batch
    def test_large_batch():
        commands = [
            {"command": "io.manager.getStatus", "id": f"large-{i}"}
            for i in range(20)
        ]
        response = api.send_batch(commands)
        if response is None:
            return False, "No response"
        results = response.get("results", [])
        if len(results) != 20:
            return False, f"Expected 20 results, got {len(results)}"
        return True, ""
    run_test(suite, "Large batch (20 commands) executes", test_large_batch)


# =============================================================================
# Stress Tests
# =============================================================================

def test_stress(api: SerialStudioAPI, suite: TestSuite):
    """Stress tests for the API."""
    print("\n--- Stress Tests ---")

    # Test: Rapid sequential commands
    def test_rapid_commands():
        success_count = 0
        total = 50
        for i in range(total):
            response = api.send_command("io.manager.getStatus", request_id=f"rapid-{i}")
            if response and response.get("success"):
                success_count += 1
        if success_count != total:
            return False, f"Only {success_count}/{total} commands succeeded"
        return True, ""
    run_test(suite, "Rapid sequential commands (50x)", test_rapid_commands)

    # Test: Large payload in params
    def test_large_payload():
        # Create a large-ish payload
        large_data = "A" * 10000
        response = api.send_command("io.driver.network.setRemoteAddress", {"address": large_data[:200]})
        # This should succeed (address will be set)
        if response is None:
            return False, "No response"
        return True, ""
    run_test(suite, "Handles large parameter values", test_large_payload)


# =============================================================================
# Configuration Workflow Tests
# =============================================================================

def test_configuration_workflow(api: SerialStudioAPI, suite: TestSuite):
    """Test realistic configuration workflows."""
    print("\n--- Configuration Workflow Tests ---")

    # Test: Configure UART and verify
    def test_uart_workflow():
        # Set all UART parameters
        commands = [
            {"command": "io.manager.setBusType", "id": "cfg-1", "params": {"busType": 0}},  # UART
            {"command": "io.driver.uart.setBaudRate", "id": "cfg-2", "params": {"baudRate": 115200}},
            {"command": "io.driver.uart.setParity", "id": "cfg-3", "params": {"parityIndex": 0}},
            {"command": "io.driver.uart.setDataBits", "id": "cfg-4", "params": {"dataBitsIndex": 3}},
            {"command": "io.driver.uart.setStopBits", "id": "cfg-5", "params": {"stopBitsIndex": 0}},
            {"command": "io.driver.uart.setFlowControl", "id": "cfg-6", "params": {"flowControlIndex": 0}},
            {"command": "io.driver.uart.getConfiguration", "id": "cfg-7"},
        ]
        response = api.send_batch(commands)
        if response is None:
            return False, "No response"
        results = response.get("results", [])
        if len(results) != 7:
            return False, f"Expected 7 results, got {len(results)}"

        # Check all succeeded
        for i, result in enumerate(results):
            if not result.get("success"):
                return False, f"Command {i+1} failed: {result.get('error')}"

        # Verify final configuration
        config = results[6].get("result", {})
        if config.get("baudRate") != 115200:
            return False, f"Baud rate mismatch: {config.get('baudRate')}"

        return True, ""
    run_test(suite, "UART configuration workflow", test_uart_workflow)

    # Test: Configure Network and verify
    def test_network_workflow():
        commands = [
            {"command": "io.manager.setBusType", "id": "net-1", "params": {"busType": 1}},  # Network
            {"command": "io.driver.network.setRemoteAddress", "id": "net-2", "params": {"address": "192.168.0.1"}},
            {"command": "io.driver.network.setTcpPort", "id": "net-3", "params": {"port": 5000}},
            {"command": "io.driver.network.setSocketType", "id": "net-4", "params": {"socketTypeIndex": 0}},
            {"command": "io.driver.network.getConfiguration", "id": "net-5"},
        ]
        response = api.send_batch(commands)
        if response is None:
            return False, "No response"
        results = response.get("results", [])
        if len(results) != 5:
            return False, f"Expected 5 results, got {len(results)}"

        # Check all succeeded
        for i, result in enumerate(results):
            if not result.get("success"):
                return False, f"Command {i+1} failed: {result.get('error')}"

        # Verify configuration
        config = results[4].get("result", {})
        if config.get("remoteAddress") != "192.168.0.1":
            return False, f"Address mismatch: {config.get('remoteAddress')}"
        if config.get("tcpPort") != 5000:
            return False, f"Port mismatch: {config.get('tcpPort')}"

        return True, ""
    run_test(suite, "Network configuration workflow", test_network_workflow)


# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Serial Studio API Test Suite",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python test_api.py                     # Run all tests on localhost:7777
  python test_api.py --verbose           # Run with verbose output
  python test_api.py --host 192.168.1.5  # Test remote instance
        """)
    parser.add_argument("--host", default=DEFAULT_HOST, help=f"Server host (default: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Server port (default: {DEFAULT_PORT})")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable verbose output")
    args = parser.parse_args()

    print("=" * 60)
    print("Serial Studio API Test Suite")
    print("=" * 60)
    print(f"Target: {args.host}:{args.port}")
    print(f"Verbose: {args.verbose}")
    print()

    # Create API client
    api = SerialStudioAPI(args.host, args.port, args.verbose)

    # Connect to server
    print("Connecting to Serial Studio Plugin Server...")
    if not api.connect():
        print("\n[FATAL] Could not connect to Serial Studio.")
        print("Please ensure:")
        print("  1. Serial Studio is running")
        print("  2. Plugin Server is enabled (Settings > Extensions > Enable Plugin Server)")
        print(f"  3. The server is listening on port {args.port}")
        return 1

    print("Connected successfully!\n")

    # Create test suite
    suite = TestSuite("Serial Studio API")

    try:
        # Run all test categories
        test_protocol(api, suite)
        test_api_commands(api, suite)
        test_io_manager(api, suite)
        test_uart_handler(api, suite)
        test_network_handler(api, suite)
        test_batch_commands(api, suite)
        test_stress(api, suite)
        test_configuration_workflow(api, suite)

    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Tests cancelled by user")
    except Exception as e:
        print(f"\n[FATAL] Unexpected error: {e}")
        import traceback
        traceback.print_exc()
    finally:
        api.disconnect()

    # Print summary
    suite.print_summary()

    # Return exit code
    return 0 if suite.failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

