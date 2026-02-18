"""
Console Configuration Integration Tests

Tests for console display settings, data modes, font options, and message
transmission via the API. All commands require Serial Studio to be running
with API Server enabled on port 7777.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


@pytest.mark.integration
def test_console_echo_toggle(api_client, clean_state):
    """Verify console echo can be enabled and disabled."""
    api_client.command("console.setEcho", {"enabled": True})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("echo") is True

    api_client.command("console.setEcho", {"enabled": False})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("echo") is False


@pytest.mark.integration
def test_console_timestamp_toggle(api_client, clean_state):
    """Verify timestamp visibility can be toggled via API."""
    api_client.command("console.setShowTimestamp", {"enabled": True})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("showTimestamp") is True

    api_client.command("console.setShowTimestamp", {"enabled": False})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("showTimestamp") is False


@pytest.mark.integration
def test_console_display_mode(api_client, clean_state):
    """Verify console display mode switches between PlainText (0) and Hex (1)."""
    api_client.command("console.setDisplayMode", {"modeIndex": 0})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("displayMode") == 0

    api_client.command("console.setDisplayMode", {"modeIndex": 1})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("displayMode") == 1

    # Restore default
    api_client.command("console.setDisplayMode", {"modeIndex": 0})


@pytest.mark.integration
def test_console_display_mode_invalid(api_client, clean_state):
    """Verify invalid display mode index is rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("console.setDisplayMode", {"modeIndex": 99})


@pytest.mark.integration
def test_console_data_mode(api_client, clean_state):
    """Verify console data send mode switches between UTF-8 (0) and Hex (1)."""
    api_client.command("console.setDataMode", {"modeIndex": 0})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("dataMode") == 0

    api_client.command("console.setDataMode", {"modeIndex": 1})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert config.get("dataMode") == 1

    # Restore default
    api_client.command("console.setDataMode", {"modeIndex": 0})


@pytest.mark.integration
def test_console_line_ending(api_client, clean_state):
    """Verify all line ending options can be set: None(0), LF(1), CR(2), CRLF(3)."""
    for ending_index in range(4):
        api_client.command("console.setLineEnding", {"endingIndex": ending_index})
        time.sleep(0.1)

        config = api_client.command("console.getConfiguration")
        assert config.get("lineEnding") == ending_index


@pytest.mark.integration
def test_console_line_ending_invalid(api_client, clean_state):
    """Verify invalid line ending index is rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("console.setLineEnding", {"endingIndex": 99})


@pytest.mark.integration
def test_console_font_size(api_client, clean_state):
    """Verify console font size can be changed and persists."""
    for size in [10, 12, 14, 16]:
        api_client.command("console.setFontSize", {"fontSize": size})
        time.sleep(0.1)

        config = api_client.command("console.getConfiguration")
        assert config.get("fontSize") == size


@pytest.mark.integration
def test_console_font_size_invalid(api_client, clean_state):
    """Verify font size of 0 or negative is rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("console.setFontSize", {"fontSize": 0})


@pytest.mark.integration
def test_console_font_family(api_client, clean_state):
    """Verify console font family can be set."""
    api_client.command("console.setFontFamily", {"fontFamily": "Monospace"})
    time.sleep(0.2)

    config = api_client.command("console.getConfiguration")
    assert "fontFamily" in config
    assert len(config["fontFamily"]) > 0


@pytest.mark.integration
def test_console_checksum_method(api_client, clean_state):
    """Verify console checksum method index 0 is always valid."""
    api_client.command("console.setChecksumMethod", {"methodIndex": 0})
    time.sleep(0.1)

    config = api_client.command("console.getConfiguration")
    assert "checksumMethod" in config


@pytest.mark.integration
def test_console_checksum_method_invalid(api_client, clean_state):
    """Verify out-of-range checksum method index is rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("console.setChecksumMethod", {"methodIndex": 9999})


@pytest.mark.integration
def test_console_configuration_fields(api_client, clean_state):
    """Verify console.getConfiguration returns all required fields."""
    config = api_client.command("console.getConfiguration")

    expected_fields = [
        "echo",
        "showTimestamp",
        "displayMode",
        "dataMode",
        "lineEnding",
        "fontFamily",
        "fontSize",
        "checksumMethod",
        "bufferLength",
    ]

    for field in expected_fields:
        assert field in config, f"Missing field in console.getConfiguration: {field}"


@pytest.mark.integration
def test_console_configuration_roundtrip(api_client, clean_state):
    """Set multiple console settings and verify all persist via getConfiguration."""
    api_client.command("console.setEcho", {"enabled": True})
    api_client.command("console.setShowTimestamp", {"enabled": True})
    api_client.command("console.setDisplayMode", {"modeIndex": 0})
    api_client.command("console.setDataMode", {"modeIndex": 0})
    api_client.command("console.setLineEnding", {"endingIndex": 1})
    api_client.command("console.setFontSize", {"fontSize": 13})
    time.sleep(0.3)

    config = api_client.command("console.getConfiguration")
    assert config["echo"] is True
    assert config["showTimestamp"] is True
    assert config["displayMode"] == 0
    assert config["dataMode"] == 0
    assert config["lineEnding"] == 1
    assert config["fontSize"] == 13


@pytest.mark.integration
def test_console_send_data(api_client, device_simulator, clean_state):
    """Verify console.send transmits data to a connected device without error."""
    api_client.command("console.setDataMode", {"modeIndex": 0})
    api_client.command("console.setLineEnding", {"endingIndex": 0})
    time.sleep(0.1)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    result = api_client.command("console.send", {"data": "hello"})
    assert result.get("sent") is True
    assert result.get("dataLength") == 5

    time.sleep(0.2)
    api_client.disconnect_device()


@pytest.mark.integration
def test_console_send_requires_data_param(api_client, clean_state):
    """Verify console.send rejects calls missing the 'data' parameter."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("console.send", {})


@pytest.mark.integration
def test_console_clear_empties_buffer(api_client, device_simulator, clean_state):
    """Verify console.clear resets the buffer length to zero."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frame(b"some data for the console buffer\n")

    # Wait for buffer to fill
    end_time = time.time() + 2.0
    while time.time() < end_time:
        config = api_client.command("console.getConfiguration")
        if config.get("bufferLength", 0) > 0:
            break
        time.sleep(0.05)

    api_client.command("console.clear")

    # Wait for buffer to drain
    end_time = time.time() + 2.0
    while time.time() < end_time:
        config = api_client.command("console.getConfiguration")
        if config.get("bufferLength", 0) == 0:
            break
        time.sleep(0.05)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) == 0

    api_client.disconnect_device()
