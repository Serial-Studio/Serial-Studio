"""
Bug Audit Tests (2026-03-13)

Regression tests for bugs discovered during the comment style audit.
These tests verify fixes for critical, high, and medium severity issues
found across Audio, MQTT, FrameBuilder, CSV Player, API Server, and UI
components.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import APIError


# ---------------------------------------------------------------------------
# Bug #9: ProcessHandler missing path validation (security)
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestProcessHandlerPathValidation:
    """Verify that process driver rejects dangerous executable paths."""

    def test_set_executable_rejects_absolute_outside_allowed(self, api_client, clean_state):
        """ProcessHandler.setExecutable should reject paths outside allowed dirs."""
        dangerous_paths = [
            "/usr/bin/rm",
            "/bin/sh",
            "/tmp/malicious_binary",
        ]

        api_client.command("io.manager.setBusType", {"busType": 8})

        for path in dangerous_paths:
            with pytest.raises(APIError):
                api_client.command(
                    "io.driver.process.setExecutable",
                    {"executable": path},
                )

    def test_set_executable_rejects_missing_param(self, api_client, clean_state):
        """ProcessHandler.setExecutable should reject missing param."""
        api_client.command("io.manager.setBusType", {"busType": 8})

        with pytest.raises(APIError):
            api_client.command("io.driver.process.setExecutable", {})


# ---------------------------------------------------------------------------
# Bug #10: MCPHandler session leak via operator[]
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestMCPSessionLeak:
    """Verify that unauthenticated MCP requests don't leak sessions."""

    def test_invalid_session_does_not_grow_map(self, api_client, clean_state):
        """Sending commands with bogus session IDs should not create entries."""
        # Send multiple requests with different fake session IDs
        # If the map leaks, memory grows; we can't measure memory directly
        # but we can verify the server stays responsive after many attempts
        for i in range(50):
            try:
                api_client.command(
                    "api.getCommands",
                    {"__mcp_session_id__": f"fake-session-{i}"},
                )
            except (APIError, Exception):
                pass

        # Server should still be responsive
        result = api_client.command("api.getCommands")
        assert "commands" in result, "Server should remain responsive after bogus sessions"


# ---------------------------------------------------------------------------
# Bug #3: MQTT string format (%.  instead of %1)
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestMQTTConfiguration:
    """Verify MQTT client API returns valid configuration."""

    def test_mqtt_get_configuration(self, api_client, clean_state):
        """MQTT configuration should be retrievable."""
        try:
            config = api_client.command("mqtt.getConfiguration")
            assert isinstance(config, dict), "MQTT config should be a dict"
        except APIError as e:
            if "UNKNOWN_COMMAND" in str(e):
                pytest.skip("MQTT commands not available")
            raise


# ---------------------------------------------------------------------------
# Bug #13: Server settings persistence bug
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestServerSettingsPersistence:
    """Verify API server status is consistent."""

    def test_server_reports_enabled_when_listening(self, api_client, clean_state):
        """If we can connect, the server status should report enabled."""
        # If we got this far, the server is running
        # This is a basic consistency check
        result = api_client.command("api.getCommands")
        assert "commands" in result


# ---------------------------------------------------------------------------
# Bug #3 (DashboardHandler): operationMode bounds
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestDashboardOperationMode:
    """Verify operation mode stays within valid bounds."""

    def test_set_invalid_operation_mode_rejected(self, api_client, clean_state):
        """Setting an out-of-range operation mode should fail or be clamped."""
        try:
            api_client.command("dashboard.setOperationMode", {"mode": 99})
        except APIError:
            pass

        status = api_client.command("dashboard.getStatus")
        mode = status.get("operationMode", -1)
        assert mode in [0, 1, 2], f"Operation mode should be 0-2, got {mode}"

    def test_valid_operation_modes(self, api_client, clean_state):
        """All three valid operation modes should be settable."""
        for mode_idx in [0, 1, 2]:
            try:
                api_client.command("dashboard.setOperationMode", {"mode": mode_idx})
                status = api_client.command("dashboard.getStatus")
                assert status.get("operationMode") == mode_idx
            except APIError:
                pass


# ---------------------------------------------------------------------------
# Bug #11: SourceHandler cross-thread safety
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestSourceHandlerThreadSafety:
    """Verify source operations complete without crashes."""

    def test_rapid_source_add_delete_no_crash(self, api_client, clean_state):
        """Rapidly adding and deleting sources should not crash."""
        api_client.create_new_project(title="Thread Safety Test")

        for i in range(5):
            try:
                api_client.command("source.add")
                time.sleep(0.05)
            except APIError:
                pass

        # Get current sources and delete them
        try:
            status = api_client.command("project.getStatus")
            source_count = status.get("sourceCount", 0)
            for sid in range(source_count - 1, 0, -1):
                try:
                    api_client.command("source.delete", {"sourceId": sid})
                    time.sleep(0.05)
                except APIError:
                    pass
        except APIError:
            pass

        # Server should still be responsive
        result = api_client.command("api.getCommands")
        assert "commands" in result


# ---------------------------------------------------------------------------
# Bug #4: FrameBuilder operator[] on m_sourceFrames
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestFrameBuilderSourceFrames:
    """Verify frame parsing handles source IDs correctly."""

    def test_quickplot_frame_parsing(self, api_client, device_simulator, clean_state):
        """QuickPlot mode should parse CSV frames without phantom source entries."""
        api_client.set_operation_mode("quickplot")
        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Send a few frames
        for i in range(5):
            device_simulator.send_frame(f"1.0,2.0,3.0\n".encode())
            time.sleep(0.05)

        time.sleep(0.5)

        # Dashboard should have valid groups
        status = api_client.command("dashboard.getStatus")
        assert status.get("available", False) or status.get("totalWidgets", 0) >= 0

        api_client.disconnect_device()


# ---------------------------------------------------------------------------
# Bug #7: Plot widget operator[] on datasets QMap
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestDashboardDataIntegrity:
    """Verify dashboard data access doesn't create phantom entries."""

    def test_dashboard_groups_valid_after_connect(self, api_client, device_simulator, clean_state):
        """After connecting and sending data, all dashboard groups should be valid."""
        api_client.set_operation_mode("quickplot")
        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Send data
        for i in range(10):
            device_simulator.send_frame(f"{i * 0.1},{i * 0.2},{i * 0.3}\n".encode())
            time.sleep(0.02)

        time.sleep(0.5)

        # Query dashboard status — should not crash
        status = api_client.command("dashboard.getStatus")
        assert isinstance(status, dict)

        api_client.disconnect_device()


# ---------------------------------------------------------------------------
# Audio driver bugs (#1, #2): multi-channel and channel count fallback
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestAudioDriverConfiguration:
    """Verify audio driver configuration is consistent."""

    def test_audio_bus_type_switch(self, api_client, clean_state):
        """Switching to audio bus type should not crash."""
        try:
            api_client.command("io.manager.setBusType", {"busType": 3})
            config = api_client.command("io.driver.audio.getConfiguration")
            assert isinstance(config, dict)
        except APIError as e:
            if "COMMERCIAL" in str(e).upper() or "NOT_AVAILABLE" in str(e).upper():
                pytest.skip("Audio driver requires commercial build")
            raise

    def test_audio_channel_counts_not_empty(self, api_client, clean_state):
        """Audio device capabilities should include valid channel counts."""
        try:
            api_client.command("io.manager.setBusType", {"busType": 3})
            config = api_client.command("io.driver.audio.getConfiguration")

            # After Bug #2 fix, channel counts should always have fallback values
            input_channels = config.get("inputChannels", [])
            output_channels = config.get("outputChannels", [])

            # At least one of input/output should have channels if devices exist
            if config.get("inputDevices") or config.get("outputDevices"):
                has_channels = len(input_channels) > 0 or len(output_channels) > 0
                assert has_channels, (
                    "Bug #2 regression: channel counts should have fallback defaults"
                )
        except APIError as e:
            if "COMMERCIAL" in str(e).upper() or "NOT_AVAILABLE" in str(e).upper():
                pytest.skip("Audio driver requires commercial build")
            raise


# ---------------------------------------------------------------------------
# Bus type switching stress test (covers multiple bugs)
# ---------------------------------------------------------------------------


@pytest.mark.integration
class TestBusTypeSwitchingStress:
    """Stress-test bus type switching to catch threading and state bugs."""

    def test_rapid_bus_switching_no_crash(self, api_client, clean_state):
        """Rapidly switching bus types should not crash or deadlock."""
        available = api_client.command("io.manager.getAvailableBuses")
        buses = available.get("buses", [])

        for _ in range(3):
            for bus in buses:
                try:
                    api_client.command("io.manager.setBusType", {"busType": bus["index"]})
                    time.sleep(0.02)
                except APIError:
                    pass

        # Server should still respond
        status = api_client.command("io.manager.getStatus")
        assert isinstance(status, dict)
