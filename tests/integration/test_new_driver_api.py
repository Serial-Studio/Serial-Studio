"""
API Tests for HID, USB, and Process Drivers (Pro features)

Validates that all io.driver.hid.*, io.driver.usb.*, and io.driver.process.*
API commands exist, accept valid parameters, reject invalid parameters, and
round-trip configuration correctly.

All tests are skipped gracefully when the Pro build is not available.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import pytest
import time

from utils.api_client import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _skip_if_missing(api_client, command_name):
    """Skip the test if a command is not registered (non-Pro build)."""
    if not api_client.command_exists(command_name):
        pytest.skip(f"Command '{command_name}' not available (Pro build required)")


def _skip_if_headless(api_client):
    """Skip the test when running without a GUI session (headless mode)."""
    status = api_client.command("ui.window.getStatus")
    if not status.get("sessionActive", True):
        pytest.skip("Skipped in headless mode (no GUI to confirm dialogs)")


# ---------------------------------------------------------------------------
# HID Driver tests
# ---------------------------------------------------------------------------

class TestHIDDriver:
    """Tests for io.driver.hid.* commands."""

    @pytest.mark.integration
    def test_hid_commands_registered(self, api_client, clean_state):
        """All HID commands should be present in a Pro build."""
        _skip_if_missing(api_client, "io.driver.hid.getDeviceList")

        expected = [
            "io.driver.hid.setDeviceIndex",
            "io.driver.hid.getDeviceList",
            "io.driver.hid.getConfiguration",
        ]
        for cmd in expected:
            assert api_client.command_exists(cmd), f"Missing command: {cmd}"

    @pytest.mark.integration
    def test_hid_get_device_list(self, api_client, clean_state):
        """getDeviceList returns a list with at least one entry and a selectedIndex."""
        _skip_if_missing(api_client, "io.driver.hid.getDeviceList")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.2)

        result = api_client.command("io.driver.hid.getDeviceList")
        assert "devices" in result, "Expected 'devices' key"
        assert "selectedIndex" in result, "Expected 'selectedIndex' key"
        assert isinstance(result["devices"], list), "devices should be a list"
        # Index 0 is always the placeholder "Select Device"
        assert len(result["devices"]) >= 1, "Should have at least the placeholder entry"

    @pytest.mark.integration
    def test_hid_get_configuration(self, api_client, clean_state):
        """getConfiguration returns deviceIndex, usagePage, and usage."""
        _skip_if_missing(api_client, "io.driver.hid.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.2)

        config = api_client.command("io.driver.hid.getConfiguration")
        assert "deviceIndex" in config
        assert "usagePage" in config
        assert "usage" in config

    @pytest.mark.integration
    def test_hid_set_device_index_placeholder(self, api_client, clean_state):
        """Setting deviceIndex to 0 (placeholder) should succeed."""
        _skip_if_missing(api_client, "io.driver.hid.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.2)

        result = api_client.command("io.driver.hid.setDeviceIndex", {"deviceIndex": 0})
        assert result["deviceIndex"] == 0

        config = api_client.command("io.driver.hid.getConfiguration")
        assert config["deviceIndex"] == 0

    @pytest.mark.integration
    def test_hid_set_device_index_missing_param(self, api_client, clean_state):
        """setDeviceIndex without deviceIndex should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.hid.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.hid.setDeviceIndex", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_hid_set_device_index_out_of_range(self, api_client, clean_state):
        """setDeviceIndex with an out-of-range value should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.hid.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.hid.setDeviceIndex", {"deviceIndex": 9999})
        assert exc_info.value.code == "INVALID_PARAM"

    @pytest.mark.integration
    def test_hid_set_device_index_negative(self, api_client, clean_state):
        """setDeviceIndex with a negative value should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.hid.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 7})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.hid.setDeviceIndex", {"deviceIndex": -1})
        assert exc_info.value.code == "INVALID_PARAM"


# ---------------------------------------------------------------------------
# USB Driver tests
# ---------------------------------------------------------------------------

class TestUSBDriver:
    """Tests for io.driver.usb.* commands."""

    @pytest.mark.integration
    def test_usb_commands_registered(self, api_client, clean_state):
        """All USB commands should be present in a Pro build."""
        _skip_if_missing(api_client, "io.driver.usb.getDeviceList")

        expected = [
            "io.driver.usb.setDeviceIndex",
            "io.driver.usb.setTransferMode",
            "io.driver.usb.setInEndpointIndex",
            "io.driver.usb.setOutEndpointIndex",
            "io.driver.usb.setIsoPacketSize",
            "io.driver.usb.getDeviceList",
            "io.driver.usb.getConfiguration",
        ]
        for cmd in expected:
            assert api_client.command_exists(cmd), f"Missing command: {cmd}"

    @pytest.mark.integration
    def test_usb_get_device_list(self, api_client, clean_state):
        """getDeviceList returns a device list and selectedIndex."""
        _skip_if_missing(api_client, "io.driver.usb.getDeviceList")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.2)

        result = api_client.command("io.driver.usb.getDeviceList")
        assert "devices" in result
        assert "selectedIndex" in result
        assert isinstance(result["devices"], list)
        assert len(result["devices"]) >= 1

    @pytest.mark.integration
    def test_usb_get_configuration(self, api_client, clean_state):
        """getConfiguration returns all expected keys."""
        _skip_if_missing(api_client, "io.driver.usb.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.2)

        config = api_client.command("io.driver.usb.getConfiguration")
        for key in ("deviceIndex", "transferMode", "inEndpointIndex",
                    "outEndpointIndex", "isoPacketSize"):
            assert key in config, f"Missing key: {key}"

    @pytest.mark.integration
    def test_usb_set_device_index_placeholder(self, api_client, clean_state):
        """Setting deviceIndex to 0 (placeholder) should succeed."""
        _skip_if_missing(api_client, "io.driver.usb.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.2)

        result = api_client.command("io.driver.usb.setDeviceIndex", {"deviceIndex": 0})
        assert result["deviceIndex"] == 0

        config = api_client.command("io.driver.usb.getConfiguration")
        assert config["deviceIndex"] == 0

    @pytest.mark.integration
    def test_usb_set_transfer_mode_all_valid(self, api_client, clean_state):
        """All three transfer modes (0-2) should be accepted."""
        _skip_if_missing(api_client, "io.driver.usb.setTransferMode")
        _skip_if_headless(api_client)

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.2)

        for mode in (0, 1, 2):
            result = api_client.command("io.driver.usb.setTransferMode", {"mode": mode})
            assert result["mode"] == mode

            config = api_client.command("io.driver.usb.getConfiguration")
            assert config["transferMode"] == mode

    @pytest.mark.integration
    def test_usb_set_transfer_mode_invalid(self, api_client, clean_state):
        """Transfer mode out of range should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.usb.setTransferMode")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.usb.setTransferMode", {"mode": 3})
        assert exc_info.value.code == "INVALID_PARAM"

    @pytest.mark.integration
    def test_usb_set_device_index_missing_param(self, api_client, clean_state):
        """setDeviceIndex without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.usb.setDeviceIndex")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.usb.setDeviceIndex", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_usb_set_transfer_mode_missing_param(self, api_client, clean_state):
        """setTransferMode without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.usb.setTransferMode")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.usb.setTransferMode", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_usb_set_iso_packet_size_valid(self, api_client, clean_state):
        """Valid ISO packet sizes should be accepted and round-trip correctly."""
        _skip_if_missing(api_client, "io.driver.usb.setIsoPacketSize")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.2)

        for size in (64, 192, 1024, 49152):
            result = api_client.command("io.driver.usb.setIsoPacketSize", {"size": size})
            assert result["size"] == size

            config = api_client.command("io.driver.usb.getConfiguration")
            assert config["isoPacketSize"] == size

    @pytest.mark.integration
    def test_usb_set_iso_packet_size_invalid(self, api_client, clean_state):
        """ISO packet size outside 1-49152 should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.usb.setIsoPacketSize")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.1)

        for bad_size in (0, -1, 49153, 65536):
            with pytest.raises(APIError) as exc_info:
                api_client.command("io.driver.usb.setIsoPacketSize", {"size": bad_size})
            assert exc_info.value.code == "INVALID_PARAM", (
                f"Expected INVALID_PARAM for size={bad_size}, "
                f"got {exc_info.value.code}"
            )

    @pytest.mark.integration
    def test_usb_set_iso_packet_size_missing_param(self, api_client, clean_state):
        """setIsoPacketSize without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.usb.setIsoPacketSize")

        api_client.command("io.manager.setBusType", {"busType": 6})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.usb.setIsoPacketSize", {})
        assert exc_info.value.code == "MISSING_PARAM"


# ---------------------------------------------------------------------------
# Process Driver tests
# ---------------------------------------------------------------------------

class TestProcessDriver:
    """Tests for io.driver.process.* commands."""

    @pytest.mark.integration
    def test_process_commands_registered(self, api_client, clean_state):
        """All Process commands should be present in a Pro build."""
        _skip_if_missing(api_client, "io.driver.process.getConfiguration")

        expected = [
            "io.driver.process.setMode",
            "io.driver.process.setExecutable",
            "io.driver.process.setArguments",
            "io.driver.process.setWorkingDir",
            "io.driver.process.setPipePath",
            "io.driver.process.getRunningProcesses",
            "io.driver.process.getConfiguration",
        ]
        for cmd in expected:
            assert api_client.command_exists(cmd), f"Missing command: {cmd}"

    @pytest.mark.integration
    def test_process_get_configuration(self, api_client, clean_state):
        """getConfiguration returns all expected keys."""
        _skip_if_missing(api_client, "io.driver.process.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        config = api_client.command("io.driver.process.getConfiguration")
        for key in ("mode", "executable", "arguments", "workingDir", "pipePath"):
            assert key in config, f"Missing key: {key}"

    @pytest.mark.integration
    def test_process_set_mode_launch(self, api_client, clean_state):
        """Mode 0 (Launch) should be accepted and persisted."""
        _skip_if_missing(api_client, "io.driver.process.setMode")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        result = api_client.command("io.driver.process.setMode", {"mode": 0})
        assert result["mode"] == 0

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["mode"] == 0

    @pytest.mark.integration
    def test_process_set_mode_named_pipe(self, api_client, clean_state):
        """Mode 1 (NamedPipe) should be accepted and persisted."""
        _skip_if_missing(api_client, "io.driver.process.setMode")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        result = api_client.command("io.driver.process.setMode", {"mode": 1})
        assert result["mode"] == 1

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["mode"] == 1

    @pytest.mark.integration
    def test_process_set_mode_invalid(self, api_client, clean_state):
        """Mode values other than 0/1 should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setMode")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        for bad_mode in (2, -1, 99):
            with pytest.raises(APIError) as exc_info:
                api_client.command("io.driver.process.setMode", {"mode": bad_mode})
            assert exc_info.value.code == "INVALID_PARAM"

    @pytest.mark.integration
    def test_process_set_mode_missing_param(self, api_client, clean_state):
        """setMode without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setMode")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setMode", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_set_executable(self, api_client, clean_state):
        """setExecutable should accept any non-empty path and persist it."""
        _skip_if_missing(api_client, "io.driver.process.setExecutable")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        test_path = "/usr/bin/python3"
        result = api_client.command("io.driver.process.setExecutable",
                                    {"executable": test_path})
        assert result["executable"] == test_path

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["executable"] == test_path

    @pytest.mark.integration
    def test_process_set_executable_missing_param(self, api_client, clean_state):
        """setExecutable without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setExecutable")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setExecutable", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_set_arguments(self, api_client, clean_state):
        """setArguments should accept a shell-style argument string and persist it."""
        _skip_if_missing(api_client, "io.driver.process.setArguments")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        test_args = "--port 9600 --verbose"
        result = api_client.command("io.driver.process.setArguments",
                                    {"arguments": test_args})
        assert result["arguments"] == test_args

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["arguments"] == test_args

    @pytest.mark.integration
    def test_process_set_arguments_empty(self, api_client, clean_state):
        """setArguments with an empty string should succeed (no args)."""
        _skip_if_missing(api_client, "io.driver.process.setArguments")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        result = api_client.command("io.driver.process.setArguments", {"arguments": ""})
        assert result["arguments"] == ""

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["arguments"] == ""

    @pytest.mark.integration
    def test_process_set_arguments_missing_param(self, api_client, clean_state):
        """setArguments without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setArguments")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setArguments", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_set_working_dir(self, api_client, clean_state):
        """setWorkingDir should persist the directory path."""
        _skip_if_missing(api_client, "io.driver.process.setWorkingDir")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        test_dir = "/tmp"
        result = api_client.command("io.driver.process.setWorkingDir",
                                    {"workingDir": test_dir})
        assert result["workingDir"] == test_dir

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["workingDir"] == test_dir

    @pytest.mark.integration
    def test_process_set_working_dir_missing_param(self, api_client, clean_state):
        """setWorkingDir without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setWorkingDir")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setWorkingDir", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_set_pipe_path(self, api_client, clean_state):
        """setPipePath should persist the pipe path."""
        _skip_if_missing(api_client, "io.driver.process.setPipePath")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        test_path = "/tmp/test.fifo"
        result = api_client.command("io.driver.process.setPipePath",
                                    {"pipePath": test_path})
        assert result["pipePath"] == test_path

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["pipePath"] == test_path

    @pytest.mark.integration
    def test_process_set_pipe_path_missing_param(self, api_client, clean_state):
        """setPipePath without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setPipePath")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setPipePath", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_set_output_capture_stdout(self, api_client, clean_state):
        """setOutputCapture with 0 (StdOut) should be accepted and persisted."""
        _skip_if_missing(api_client, "io.driver.process.setOutputCapture")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        result = api_client.command("io.driver.process.setOutputCapture", {"capture": 0})
        assert result["capture"] == 0

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["outputCapture"] == 0

    @pytest.mark.integration
    def test_process_set_output_capture_stderr(self, api_client, clean_state):
        """setOutputCapture with 1 (StdErr) should be accepted and persisted."""
        _skip_if_missing(api_client, "io.driver.process.setOutputCapture")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        result = api_client.command("io.driver.process.setOutputCapture", {"capture": 1})
        assert result["capture"] == 1

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["outputCapture"] == 1

    @pytest.mark.integration
    def test_process_set_output_capture_both(self, api_client, clean_state):
        """setOutputCapture with 2 (Both) should be accepted and persisted."""
        _skip_if_missing(api_client, "io.driver.process.setOutputCapture")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        result = api_client.command("io.driver.process.setOutputCapture", {"capture": 2})
        assert result["capture"] == 2

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["outputCapture"] == 2

    @pytest.mark.integration
    def test_process_set_output_capture_invalid(self, api_client, clean_state):
        """Capture values other than 0/1/2 should return INVALID_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setOutputCapture")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        for bad_capture in (3, -1, 99):
            with pytest.raises(APIError) as exc_info:
                api_client.command("io.driver.process.setOutputCapture",
                                   {"capture": bad_capture})
            assert exc_info.value.code == "INVALID_PARAM"

    @pytest.mark.integration
    def test_process_set_output_capture_missing_param(self, api_client, clean_state):
        """setOutputCapture without required param should return MISSING_PARAM."""
        _skip_if_missing(api_client, "io.driver.process.setOutputCapture")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.1)

        with pytest.raises(APIError) as exc_info:
            api_client.command("io.driver.process.setOutputCapture", {})
        assert exc_info.value.code == "MISSING_PARAM"

    @pytest.mark.integration
    def test_process_get_running_processes(self, api_client, clean_state):
        """getRunningProcesses should return a list with at least one entry."""
        _skip_if_missing(api_client, "io.driver.process.getRunningProcesses")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        result = api_client.command("io.driver.process.getRunningProcesses")
        assert "processes" in result, "Expected 'processes' key"
        assert "count" in result, "Expected 'count' key"
        assert isinstance(result["processes"], list)

        # There should always be at least one process running (the app itself)
        assert result["count"] > 0, "Should have at least one running process"
        assert len(result["processes"]) == result["count"]

    @pytest.mark.integration
    def test_process_running_processes_format(self, api_client, clean_state):
        """Each running process entry should match 'name [PID]' format."""
        _skip_if_missing(api_client, "io.driver.process.getRunningProcesses")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        result = api_client.command("io.driver.process.getRunningProcesses")
        processes = result.get("processes", [])
        assert len(processes) > 0

        import re
        pattern = re.compile(r".+\s\[\d+\]$")
        for entry in processes[:10]:  # Check first 10 entries
            assert pattern.match(entry), (
                f"Process entry does not match 'name [PID]' format: {entry!r}"
            )

    @pytest.mark.integration
    def test_process_full_launch_config_roundtrip(self, api_client, clean_state):
        """Setting all Launch mode fields and reading them back should match."""
        _skip_if_missing(api_client, "io.driver.process.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        api_client.command("io.driver.process.setMode", {"mode": 0})
        api_client.command("io.driver.process.setExecutable",
                           {"executable": "/usr/bin/python3"})
        api_client.command("io.driver.process.setArguments",
                           {"arguments": "-c \"import sys; sys.stdout.write('hello\\n')\""})
        api_client.command("io.driver.process.setWorkingDir", {"workingDir": "/tmp"})

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["mode"] == 0
        assert config["executable"] == "/usr/bin/python3"
        assert config["workingDir"] == "/tmp"

    @pytest.mark.integration
    def test_process_full_pipe_config_roundtrip(self, api_client, clean_state):
        """Setting all NamedPipe mode fields and reading them back should match."""
        _skip_if_missing(api_client, "io.driver.process.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        api_client.command("io.driver.process.setMode", {"mode": 1})
        api_client.command("io.driver.process.setPipePath", {"pipePath": "/tmp/myapp.fifo"})

        config = api_client.command("io.driver.process.getConfiguration")
        assert config["mode"] == 1
        assert config["pipePath"] == "/tmp/myapp.fifo"

    @pytest.mark.integration
    def test_process_bus_type_index(self, api_client, clean_state):
        """Setting busType to 8 should switch to the Process driver."""
        _skip_if_missing(api_client, "io.driver.process.getConfiguration")

        api_client.command("io.manager.setBusType", {"busType": 8})
        time.sleep(0.2)

        status = api_client.command("io.manager.getStatus")
        assert status["busType"] == 8, (
            f"Expected busType 8 (Process), got {status['busType']}"
        )


# ---------------------------------------------------------------------------
# Cross-driver: bus-type switching
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_switch_to_all_new_bus_types(api_client, clean_state):
    """Switching to USB (6), HID (7), and Process (8) bus types should work."""
    if not api_client.command_exists("io.driver.usb.getDeviceList"):
        pytest.skip("Pro build required")

    for bus_type in (6, 7, 8):
        api_client.command("io.manager.setBusType", {"busType": bus_type})
        time.sleep(0.2)

        status = api_client.command("io.manager.getStatus")
        assert status["busType"] == bus_type, (
            f"Failed to switch to bus type {bus_type}: got {status['busType']}"
        )

    # Return to UART so other tests are not affected
    api_client.command("io.manager.setBusType", {"busType": 0})


@pytest.mark.integration
def test_available_buses_includes_new_drivers(api_client, clean_state):
    """getAvailableBuses should list USB, HID, and Process when Pro is enabled."""
    if not api_client.command_exists("io.driver.usb.getDeviceList"):
        pytest.skip("Pro build required")

    buses = api_client.command("io.manager.getAvailableBuses").get("buses", [])
    indices = [b["index"] for b in buses]

    assert 6 in indices, "USB Device bus (index 6) should be in available buses"
    assert 7 in indices, "HID Device bus (index 7) should be in available buses"
    assert 8 in indices, "Process bus (index 8) should be in available buses"
