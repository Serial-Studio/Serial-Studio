#!/usr/bin/env python3
"""
Serial Studio API Client & Test Suite
======================================

A versatile tool for interacting with the Serial Studio API Server.
Can be used as a command-line client, interactive shell, or test suite.

Usage:
    # Send a single command
    python test_api.py send io.manager.getStatus

    # Send command with parameters (key=value format - works on all shells)
    python test_api.py send io.driver.uart.setBaudRate -p baudRate=115200

    # Multiple parameters
    python test_api.py send io.driver.network.setTcpPort -p port=8080

    # JSON format (use on bash/zsh, tricky on PowerShell)
    python test_api.py send io.driver.uart.setBaudRate --params '{"baudRate": 115200}'

    # List all available commands
    python test_api.py list

    # Interactive mode (REPL)
    python test_api.py interactive

    # Live monitor (real-time status updates)
    python test_api.py monitor [--interval 500] [--compact] [--show-raw-data]

    # Run test suite
    python test_api.py test [--verbose]

    # Send batch from JSON file
    python test_api.py batch commands.json

    # Pipe JSON output (for scripting)
    python test_api.py send io.manager.getStatus --json | jq '.result'

Common options for all modes:
    --host HOST    Server host (default: 127.0.0.1)
    --port PORT    Server port (default: 7777)

Requirements:
    - Serial Studio running with API Server enabled (port 7777)
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
import select
import base64
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
    """Client for communicating with Serial Studio API Server."""

    def __init__(self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT, verbose: bool = False):
        self.host = host
        self.port = port
        self.verbose = verbose
        self.socket: Optional[socket.socket] = None
        self.receive_buffer = b""

    def connect(self) -> bool:
        """Establish TCP connection to the API server."""
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
        self.receive_buffer = b""

    def recv_message(self, timeout: float = SOCKET_TIMEOUT) -> Optional[dict]:
        """
        Receive a single JSON message from the socket.
        Handles newline-delimited JSON messages and buffering.
        Returns None on timeout or error.
        """
        if not self.socket:
            return None

        try:
            end_time = time.time() + timeout
            while True:
                newline_pos = self.receive_buffer.find(b'\n')
                if newline_pos != -1:
                    line = self.receive_buffer[:newline_pos]
                    self.receive_buffer = self.receive_buffer[newline_pos + 1:]

                    if line.strip():
                        if self.verbose:
                            print(f"[RECV] {line.decode('utf-8', errors='replace')}")
                        return json.loads(line.decode('utf-8'))
                    continue

                remaining_time = end_time - time.time()
                if remaining_time <= 0:
                    return None

                ready = select.select([self.socket], [], [], min(remaining_time, 0.1))
                if ready[0]:
                    chunk = self.socket.recv(RECV_BUFFER_SIZE)
                    if not chunk:
                        return None
                    self.receive_buffer += chunk
        except Exception as e:
            if self.verbose:
                print(f"[ERROR] recv_message: {e}")
            return None

    def send_raw(self, data: bytes, expected_id: Optional[str] = None, timeout: float = SOCKET_TIMEOUT) -> Optional[dict]:
        """
        Send raw bytes and receive JSON response.

        If expected_id is provided, will wait for a response with that ID,
        discarding any push notifications (data/frames) in the meantime.
        """
        if not self.socket:
            return None

        try:
            if self.verbose:
                print(f"[SEND] {data.decode('utf-8', errors='replace').strip()}")

            self.socket.sendall(data)

            # If we're expecting a specific response ID, wait for it
            if expected_id:
                end_time = time.time() + timeout
                while True:
                    remaining = end_time - time.time()
                    if remaining <= 0:
                        if self.verbose:
                            print(f"[ERROR] Timeout waiting for response ID {expected_id}")
                        return None

                    msg = self.recv_message(timeout=remaining)
                    if not msg:
                        return None

                    # Check if this is the response we're waiting for
                    if msg.get("type") == MessageType.RESPONSE and msg.get("id") == expected_id:
                        return msg

                    # Otherwise, it's a push notification - discard and continue waiting
                    if self.verbose:
                        print(f"[DEBUG] Discarding push notification while waiting for {expected_id}")
            else:
                # No expected ID, just return the next message
                return self.recv_message(timeout=timeout)

        except Exception as e:
            if self.verbose:
                print(f"[ERROR] {e}")
            return None

    def send_json(self, obj: dict) -> Optional[dict]:
        """Send JSON object and receive JSON response."""
        data = json.dumps(obj, separators=(',', ':')) + "\n"
        expected_id = obj.get("id")
        return self.send_raw(data.encode('utf-8'), expected_id=expected_id, timeout=SOCKET_TIMEOUT)

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

    def has_data_available(self, timeout: float = 0.0) -> bool:
        """Check if data is available to read from the socket."""
        if not self.socket:
            return False
        if self.receive_buffer:
            return True
        try:
            ready = select.select([self.socket], [], [], timeout)
            return bool(ready[0])
        except Exception:
            return False


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

        # Clean up: Disconnect if we connected successfully
        if response.get("success"):
            api.send_command("io.manager.disconnect")

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

def test_bluetoothle_handler(api: SerialStudioAPI, suite: TestSuite):
    """Test io.driver.ble.* commands."""
    print("\n--- Bluetooth LE Handler Tests ---")

    # Test: getStatus
    def test_get_status():
        response = api.send_command("io.driver.ble.getStatus")
        passed, msg = assert_success(response, "getStatus")
        if not passed:
            return False, msg
        result = response.get("result", {})
        expected_fields = ["operatingSystemSupported", "adapterAvailable", "isOpen", "deviceCount"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "io.driver.ble.getStatus returns status", test_get_status)

    # Test: getConfiguration
    def test_get_configuration():
        response = api.send_command("io.driver.ble.getConfiguration")
        passed, msg = assert_success(response, "getConfiguration")
        if not passed:
            return False, msg
        result = response.get("result", {})
        expected_fields = ["deviceIndex", "characteristicIndex", "isOpen", "configurationOk"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "io.driver.ble.getConfiguration returns config", test_get_configuration)

    # Test: getDeviceList
    def test_get_device_list():
        response = api.send_command("io.driver.ble.getDeviceList")
        passed, msg = assert_success(response, "getDeviceList")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if "deviceList" not in result:
            return False, "Missing deviceList field"
        return True, ""
    run_test(suite, "io.driver.ble.getDeviceList returns device list", test_get_device_list)

    # Test: getServiceList
    def test_get_service_list():
        response = api.send_command("io.driver.ble.getServiceList")
        passed, msg = assert_success(response, "getServiceList")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if "serviceList" not in result:
            return False, "Missing serviceList field"
        return True, ""
    run_test(suite, "io.driver.ble.getServiceList returns service list", test_get_service_list)

    # Test: getCharacteristicList
    def test_get_characteristic_list():
        response = api.send_command("io.driver.ble.getCharacteristicList")
        passed, msg = assert_success(response, "getCharacteristicList")
        if not passed:
            return False, msg
        result = response.get("result", {})
        if "characteristicList" not in result:
            return False, "Missing characteristicList field"
        return True, ""
    run_test(suite, "io.driver.ble.getCharacteristicList returns list", test_get_characteristic_list)


def test_csv_export_handler(api: SerialStudioAPI, suite: TestSuite):
    """Test csv.export.* commands."""
    print("\n--- CSV Export Handler Tests ---")

    # Test: getStatus
    def test_get_status():
        response = api.send_command("csv.export.getStatus")
        passed, msg = assert_success(response, "getStatus")
        if not passed:
            return False, msg
        result = response.get("result", {})
        expected_fields = ["enabled", "isOpen"]
        for field in expected_fields:
            if field not in result:
                return False, f"Missing field: {field}"
        return True, ""
    run_test(suite, "csv.export.getStatus returns status", test_get_status)

    # Test: setEnabled
    def test_set_enabled():
        response = api.send_command("csv.export.setEnabled", {"enabled": True})
        passed, msg = assert_success(response, "setEnabled")
        if not passed:
            return False, msg
        # Restore to false
        api.send_command("csv.export.setEnabled", {"enabled": False})
        return True, ""
    run_test(suite, "csv.export.setEnabled sets export state", test_set_enabled)

    # Test: setEnabled missing param
    def test_set_enabled_missing():
        response = api.send_command("csv.export.setEnabled", {})
        return assert_error(response, ErrorCode.MISSING_PARAM, "setEnabled_missing")
    run_test(suite, "csv.export.setEnabled requires enabled param", test_set_enabled_missing)

    # Test: close
    def test_close():
        response = api.send_command("csv.export.close")
        passed, msg = assert_success(response, "close")
        if not passed:
            return False, msg
        return True, ""
    run_test(suite, "csv.export.close executes", test_close)


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
# CLI Modes
# =============================================================================

def parse_params(param_list: list[str]) -> Optional[dict]:
    """
    Parse parameters from command line.
    Supports two formats:
      - JSON: '{"key": "value"}'
      - Key=Value pairs: key=value key2=value2

    For key=value, attempts to auto-convert types:
      - "true"/"false" -> bool
      - integers -> int
      - floats -> float
      - everything else -> string
    """
    if not param_list:
        return None

    # If first param looks like JSON, parse it as JSON
    joined = " ".join(param_list)
    stripped = joined.strip()
    if stripped.startswith("{"):
        # Try to parse as JSON, with some fixups for shell escaping issues
        try:
            return json.loads(stripped)
        except json.JSONDecodeError:
            # Try fixing common PowerShell issues (unquoted keys)
            # Convert {key:value} to {"key":value}
            import re
            fixed = re.sub(r'(\{|,)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*:', r'\1"\2":', stripped)
            try:
                return json.loads(fixed)
            except json.JSONDecodeError as e:
                raise ValueError(f"Invalid JSON: {e}\nTip: On PowerShell, use key=value format instead: -p baudRate=115200")

    # Parse as key=value pairs
    params = {}
    for param in param_list:
        if "=" not in param:
            raise ValueError(f"Invalid parameter format: '{param}'. Use key=value or JSON format.")
        key, value = param.split("=", 1)
        key = key.strip()
        value = value.strip()

        # Auto-convert types
        if value.lower() == "true":
            params[key] = True
        elif value.lower() == "false":
            params[key] = False
        else:
            # Try int, then float, then keep as string
            try:
                params[key] = int(value)
            except ValueError:
                try:
                    params[key] = float(value)
                except ValueError:
                    params[key] = value

    return params


def cmd_send(api: SerialStudioAPI, args) -> int:
    """Send a single command and print the result."""
    # Parse parameters if provided
    params = None
    if args.params:
        try:
            params = parse_params(args.params)
        except ValueError as e:
            print(f"[ERROR] {e}", file=sys.stderr)
            return 1

    # Send the command
    response = api.send_command(args.command, params)

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    # Output the response
    if args.json:
        print(json.dumps(response, indent=2))
    else:
        if response.get("success"):
            result = response.get("result", {})
            if result:
                print(json.dumps(result, indent=2))
            else:
                print("[OK] Command executed successfully")
        else:
            error = response.get("error", {})
            print(f"[ERROR] {error.get('code')}: {error.get('message')}", file=sys.stderr)
            return 1

    return 0


def cmd_batch(api: SerialStudioAPI, args) -> int:
    """Send batch commands from a JSON file."""
    try:
        with open(args.file, 'r') as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"[ERROR] File not found: {args.file}", file=sys.stderr)
        return 1
    except json.JSONDecodeError as e:
        print(f"[ERROR] Invalid JSON in file: {e}", file=sys.stderr)
        return 1

    # Handle both formats: array of commands or {"commands": [...]}
    if isinstance(data, list):
        commands = data
    elif isinstance(data, dict) and "commands" in data:
        commands = data["commands"]
    else:
        print("[ERROR] Expected array of commands or {\"commands\": [...]}", file=sys.stderr)
        return 1

    response = api.send_batch(commands)

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(response, indent=2))
    else:
        results = response.get("results", [])
        success_count = sum(1 for r in results if r.get("success"))
        print(f"Executed {len(results)} commands: {success_count} succeeded, {len(results) - success_count} failed")
        for i, result in enumerate(results):
            status = "✓" if result.get("success") else "✗"
            cmd_id = result.get("id", f"#{i+1}")
            if result.get("success"):
                print(f"  {status} {cmd_id}")
            else:
                error = result.get("error", {})
                print(f"  {status} {cmd_id}: {error.get('code')} - {error.get('message')}")

    return 0 if response.get("success") else 1


def cmd_list(api: SerialStudioAPI, args) -> int:
    """List all available commands."""
    response = api.send_command("api.getCommands")

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    if not response.get("success"):
        error = response.get("error", {})
        print(f"[ERROR] {error.get('code')}: {error.get('message')}", file=sys.stderr)
        return 1

    commands = response.get("result", {}).get("commands", [])

    if args.json:
        print(json.dumps(commands, indent=2))
    else:
        # Group commands by prefix
        groups = {}
        for cmd in commands:
            name = cmd.get("name", "")
            parts = name.split(".")
            group = ".".join(parts[:-1]) if len(parts) > 1 else "other"
            if group not in groups:
                groups[group] = []
            groups[group].append(cmd)

        print(f"Available Commands ({len(commands)} total):\n")
        for group in sorted(groups.keys()):
            print(f"  {group}.*")
            for cmd in sorted(groups[group], key=lambda c: c.get("name", "")):
                name = cmd.get("name", "")
                desc = cmd.get("description", "")
                # Truncate description if too long
                if len(desc) > 50:
                    desc = desc[:47] + "..."
                print(f"    {name:<40} {desc}")
            print()

    return 0


def cmd_interactive(api: SerialStudioAPI, args) -> int:
    """Interactive REPL mode."""
    print("Serial Studio Interactive Mode")
    print("Type 'help' for commands, 'quit' to exit\n")

    while True:
        try:
            line = input("ss> ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nGoodbye!")
            break

        if not line:
            continue

        # Built-in commands
        if line.lower() in ("quit", "exit", "q"):
            print("Goodbye!")
            break

        if line.lower() == "help":
            print("""
Interactive Mode Commands:
  help                          Show this help
  quit, exit, q                 Exit interactive mode
  list                          List all available API commands
  <command>                     Execute a command (e.g., io.manager.getStatus)
  <command> <json_params>       Execute with params (e.g., io.driver.uart.setBaudRate {"baudRate":115200})

Examples:
  io.manager.getStatus
  io.driver.uart.setBaudRate {"baudRate": 115200}
  io.driver.network.setRemoteAddress {"address": "192.168.1.100"}
""")
            continue

        if line.lower() == "list":
            response = api.send_command("api.getCommands")
            if response and response.get("success"):
                commands = response.get("result", {}).get("commands", [])
                for cmd in sorted(commands, key=lambda c: c.get("name", "")):
                    print(f"  {cmd.get('name')}")
            else:
                print("[ERROR] Could not fetch command list")
            continue

        # Parse command and optional JSON params
        parts = line.split(None, 1)
        command = parts[0]
        params = None

        if len(parts) > 1:
            try:
                params = json.loads(parts[1])
            except json.JSONDecodeError as e:
                print(f"[ERROR] Invalid JSON parameters: {e}")
                continue

        # Send command
        response = api.send_command(command, params)

        if response is None:
            print("[ERROR] No response received")
            continue

        if response.get("success"):
            result = response.get("result", {})
            if result:
                print(json.dumps(result, indent=2))
            else:
                print("[OK]")
        else:
            error = response.get("error", {})
            print(f"[ERROR] {error.get('code')}: {error.get('message')}")

    return 0


def cmd_monitor(api: SerialStudioAPI, args) -> int:
    """Monitor Serial Studio in real-time."""
    print("=" * 60)
    print("Serial Studio Live Monitor")
    print("=" * 60)
    print("Press Ctrl+C to exit\n")

    update_interval = args.interval / 1000.0
    frame_count = 0
    raw_data_count = 0
    last_status = {}
    last_update_time = time.time()

    try:
        while True:
            current_time = time.time()

            # Process any pending push notifications (raw data, frames)
            while api.has_data_available(timeout=0.0):
                msg = api.recv_message(timeout=0.1)
                if msg:
                    if "data" in msg and "frames" not in msg:
                        raw_data_count += 1
                        if args.show_raw_data:
                            try:
                                decoded = base64.b64decode(msg["data"]).decode('utf-8', errors='replace')
                                display_text = decoded.replace('\n', '\\n').replace('\r', '\\r')
                                if len(display_text) > 100:
                                    display_text = display_text[:97] + "..."
                                print(f"\rRaw [{raw_data_count}]: {display_text}".ljust(120), end='', flush=True)
                            except Exception:
                                pass
                    elif "frames" in msg:
                        frame_count += len(msg.get("frames", []))

            # Send status query at regular intervals
            if current_time - last_update_time >= update_interval:
                last_update_time = current_time

                response = api.send_command("io.manager.getStatus")
                if response and response.get("success"):
                    status = response.get("result", {})

                    # Clear screen and reposition cursor (only on success)
                    if not args.compact and not args.show_raw_data:
                        print("\033[2J\033[H", end="")
                    elif not args.show_raw_data:
                        print()

                    # Print header
                    if not args.show_raw_data:
                        print("=" * 60)
                        print(f"Serial Studio Live Monitor (Update: {args.interval}ms)")
                        print("=" * 60)
                        print()

                    # Connection status with color
                    connected = status.get("isConnected", False)
                    paused = status.get("paused", False)
                    conn_icon = "🟢" if connected else "🔴"
                    pause_icon = "⏸️ " if paused else ""

                    if not args.show_raw_data:
                        print(f"Status: {conn_icon} {'CONNECTED' if connected else 'DISCONNECTED'} {pause_icon}")
                        print(f"Bus Type: {status.get('busTypeName', 'Unknown')}")
                        print(f"Configuration: {'✓ OK' if status.get('configurationOk', False) else '✗ Not Ready'}")
                        print(f"Mode: {'Read/Write' if status.get('readWrite', False) else 'Read-Only' if status.get('readOnly', False) else 'None'}")

                        # Frame statistics (if connected)
                        if connected:
                            print()
                            print("─" * 60)

                            # Get driver-specific info based on bus type
                            bus_type = status.get("busType", 0)

                            if bus_type == 0:  # UART
                                uart_resp = api.send_command("io.driver.uart.getConfiguration")
                                if uart_resp and uart_resp.get("success"):
                                    uart_cfg = uart_resp.get("result", {})
                                    print(f"Port: {uart_cfg.get('port', 'N/A')}")
                                    print(f"Baud Rate: {uart_cfg.get('baudRate', 'N/A')}")
                                    print(f"Config: {uart_cfg.get('dataBits', '?')}{uart_cfg.get('parity', '?')[0]}{uart_cfg.get('stopBits', '?')}")

                            elif bus_type == 1:  # Network
                                net_resp = api.send_command("io.driver.network.getConfiguration")
                                if net_resp and net_resp.get("success"):
                                    net_cfg = net_resp.get("result", {})
                                    socket_types = ["TCP", "UDP"]
                                    socket_idx = net_cfg.get("socketTypeIndex", 0)
                                    socket_type = socket_types[socket_idx] if socket_idx < len(socket_types) else "Unknown"
                                    print(f"Type: {socket_type}")
                                    print(f"Address: {net_cfg.get('remoteAddress', 'N/A')}")
                                    if socket_idx == 0:
                                        print(f"Port: {net_cfg.get('tcpPort', 'N/A')}")
                                    else:
                                        print(f"Local Port: {net_cfg.get('udpLocalPort', 'N/A')}")
                                        print(f"Remote Port: {net_cfg.get('udpRemotePort', 'N/A')}")

                            elif bus_type == 2:  # Bluetooth LE
                                ble_resp = api.send_command("io.driver.ble.getConfiguration")
                                if ble_resp and ble_resp.get("success"):
                                    ble_cfg = ble_resp.get("result", {})
                                    print(f"Device: {ble_cfg.get('deviceName', 'N/A')}")
                                    print(f"Service: {ble_cfg.get('serviceName', 'N/A')}")
                                    print(f"Characteristic: {ble_cfg.get('characteristicName', 'N/A')}")

                        # Export status
                        print()
                        print("─" * 60)
                        csv_resp = api.send_command("csv.export.getStatus")
                        if csv_resp and csv_resp.get("success"):
                            csv_status = csv_resp.get("result", {})
                            csv_enabled = csv_status.get("enabled", False)
                            csv_open = csv_status.get("isOpen", False)
                            csv_icon = "📝" if csv_enabled and csv_open else "💾" if csv_enabled else "⏹️ "
                            print(f"CSV Export: {csv_icon} {'Active' if csv_enabled and csv_open else 'Enabled' if csv_enabled else 'Disabled'}")

                        # Detect changes
                        if last_status and not args.compact:
                            if status.get("isConnected") != last_status.get("isConnected"):
                                print("\n🔔 Connection state changed!")
                            if status.get("paused") != last_status.get("paused"):
                                print("\n🔔 Pause state changed!")

                        print()
                        print("─" * 60)
                        print(f"Frames: {frame_count} | Raw Data Packets: {raw_data_count} | Press Ctrl+C to exit")

                    last_status = status

                else:
                    # Only show errors in compact mode or verbose to avoid screen clutter
                    if api.verbose or args.compact:
                        print("[ERROR] Failed to get status (will retry)")

            # Small sleep to prevent CPU spinning
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n\nMonitoring stopped.")
        print(f"Total frames: {frame_count}")
        print(f"Total raw data packets: {raw_data_count}")
        return 0


def cmd_test(api: SerialStudioAPI, args) -> int:
    """Run the test suite."""
    print("=" * 60)
    print("Serial Studio API Test Suite")
    print("=" * 60)
    print()

    suite = TestSuite("Serial Studio API")

    try:
        test_protocol(api, suite)
        test_api_commands(api, suite)
        test_io_manager(api, suite)
        test_uart_handler(api, suite)
        test_network_handler(api, suite)
        test_bluetoothle_handler(api, suite)
        test_csv_export_handler(api, suite)
        test_batch_commands(api, suite)
        test_stress(api, suite)
        test_configuration_workflow(api, suite)
    except KeyboardInterrupt:
        print("\n[INTERRUPTED] Tests cancelled by user")
    except Exception as e:
        print(f"\n[FATAL] Unexpected error: {e}")
        import traceback
        traceback.print_exc()

    suite.print_summary()
    return 0 if suite.failed == 0 else 1


# =============================================================================
# Main Entry Point
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Serial Studio API Client & Test Suite",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s send io.manager.getStatus                            # Send a command
  %(prog)s send io.driver.uart.setBaudRate -p baudRate=115200   # With key=value params
  %(prog)s send io.driver.uart.setDtrEnabled -p dtrEnabled=true # Boolean param
  %(prog)s list                                                 # List commands
  %(prog)s interactive                                          # Interactive mode
  %(prog)s monitor --interval 500                               # Live monitor
  %(prog)s batch commands.json                                  # Batch from file
  %(prog)s test --verbose                                       # Run test suite
        """)

    parser.add_argument("--host", default=DEFAULT_HOST, help=f"Server host (default: {DEFAULT_HOST})")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Server port (default: {DEFAULT_PORT})")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable verbose output")
    parser.add_argument("--json", "-j", action="store_true", help="Output raw JSON (for scripting)")

    subparsers = parser.add_subparsers(dest="mode", help="Operation mode")

    # send subcommand
    send_parser = subparsers.add_parser("send", help="Send a single command")
    send_parser.add_argument("command", help="Command to send (e.g., io.manager.getStatus)")
    send_parser.add_argument("--params", "-p", nargs="+", metavar="PARAM",
                             help="Parameters as key=value pairs or JSON. Examples: baudRate=115200 or '{\"baudRate\": 115200}'")

    # batch subcommand
    batch_parser = subparsers.add_parser("batch", help="Send batch commands from JSON file")
    batch_parser.add_argument("file", help="JSON file containing commands")

    # list subcommand
    subparsers.add_parser("list", help="List all available commands")

    # interactive subcommand
    subparsers.add_parser("interactive", aliases=["i", "repl"], help="Interactive REPL mode")

    # monitor subcommand
    monitor_parser = subparsers.add_parser("monitor", aliases=["m", "live"], help="Live data monitor")
    monitor_parser.add_argument("--interval", type=int, default=1000, metavar="MS",
                                help="Update interval in milliseconds (default: 1000)")
    monitor_parser.add_argument("--compact", action="store_true",
                                help="Compact mode (append updates instead of clearing screen)")
    monitor_parser.add_argument("--show-raw-data", action="store_true",
                                help="Show raw device data on a single line (uses \\r for continuous update)")

    # test subcommand
    subparsers.add_parser("test", help="Run the API test suite")

    args = parser.parse_args()

    # Default to interactive mode if no subcommand given
    if args.mode is None:
        parser.print_help()
        print("\nTip: Use 'interactive' for a REPL, or 'send <command>' to execute a command.")
        return 0

    # Create API client
    api = SerialStudioAPI(args.host, args.port, args.verbose)

    # Connect to server
    if not args.json:
        print(f"Connecting to {args.host}:{args.port}...", file=sys.stderr)

    if not api.connect():
        if args.json:
            print(json.dumps({"error": "Connection failed"}))
        else:
            print("\n[ERROR] Could not connect to Serial Studio.", file=sys.stderr)
            print("Please ensure:", file=sys.stderr)
            print("  1. Serial Studio is running", file=sys.stderr)
            print("  2. API Server is enabled (Settings > Miscellaneous > Enable API Server)", file=sys.stderr)
            print(f"  3. The server is listening on port {args.port}", file=sys.stderr)
        return 1

    if not args.json:
        print("Connected!\n", file=sys.stderr)

    try:
        # Dispatch to appropriate handler
        if args.mode == "send":
            return cmd_send(api, args)
        elif args.mode == "batch":
            return cmd_batch(api, args)
        elif args.mode == "list":
            return cmd_list(api, args)
        elif args.mode in ("interactive", "i", "repl"):
            return cmd_interactive(api, args)
        elif args.mode in ("monitor", "m", "live"):
            return cmd_monitor(api, args)
        elif args.mode == "test":
            return cmd_test(api, args)
        else:
            parser.print_help()
            return 1
    finally:
        api.disconnect()


if __name__ == "__main__":
    sys.exit(main())

