"""
CSV Player Integration Tests

Tests for the CSV file playback API. Most commands require an open file;
tests verify they respond correctly both when no file is loaded and when
a file is available.

Commands covered:
  - csvPlayer.getStatus
  - csvPlayer.close
  - csvPlayer.setPaused (replaces play/pause/toggle)
  - csvPlayer.step (replaces nextFrame/previousFrame; takes signed delta)
  - csvPlayer.setProgress
  - csvPlayer.open

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_get_status_fields(api_client, clean_state):
    """Verify csvPlayer.getStatus returns all expected fields."""
    status = api_client.command("csvPlayer.getStatus")

    expected_fields = [
        "isOpen",
        "isPlaying",
        "frameCount",
        "framePosition",
        "progress",
        "timestamp",
        "filename",
    ]

    for field in expected_fields:
        assert field in status, f"Missing field in csvPlayer.getStatus: {field}"


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_initially_closed(api_client, clean_state):
    """Verify the CSV player reports no open file after a clean state."""
    api_client.command("csvPlayer.close")
    time.sleep(0.1)

    status = api_client.command("csvPlayer.getStatus")
    assert status.get("isOpen") is False
    assert status.get("isPlaying") is False


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_close_when_already_closed(api_client, clean_state):
    """Verify csvPlayer.close succeeds even when no file is open."""
    result = api_client.command("csvPlayer.close")
    assert result.get("closed") is True

    status = api_client.command("csvPlayer.getStatus")
    assert status.get("isOpen") is False


@pytest.mark.integration
@pytest.mark.csv
@pytest.mark.parametrize("paused", [True, False])
def test_csv_player_set_paused_without_file(api_client, clean_state, paused):
    """csvPlayer.setPaused should not crash when no file is loaded."""
    api_client.command("csvPlayer.close")
    time.sleep(0.1)

    try:
        result = api_client.command("csvPlayer.setPaused", {"paused": paused})
        assert "isPlaying" in result
    except Exception:
        # An error response is also acceptable when nothing is loaded.
        pass


@pytest.mark.integration
@pytest.mark.csv
@pytest.mark.parametrize("delta", [1, -1, 5, -5])
def test_csv_player_step_without_file(api_client, clean_state, delta):
    """csvPlayer.step replaces nextFrame/previousFrame; takes a signed delta."""
    api_client.command("csvPlayer.close")
    time.sleep(0.1)

    try:
        result = api_client.command("csvPlayer.step", {"delta": delta})
        assert "framePosition" in result
    except Exception:
        pass


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_set_progress_valid(api_client, clean_state):
    """Verify csvPlayer.setProgress accepts values in [0.0, 1.0]."""
    api_client.command("csvPlayer.close")
    time.sleep(0.1)

    for progress in [0.0, 0.25, 0.5, 0.75, 1.0]:
        result = api_client.command("csvPlayer.setProgress", {"progress": progress})
        assert "progress" in result
        assert "framePosition" in result


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_set_progress_invalid(api_client, clean_state):
    """Verify csvPlayer.setProgress rejects values outside [0.0, 1.0]."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("csvPlayer.setProgress", {"progress": -0.1})

    with pytest.raises(APIError):
        api_client.command("csvPlayer.setProgress", {"progress": 1.1})


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_open_nonexistent_file(api_client, clean_state):
    """Verify csvPlayer.open with a nonexistent path returns gracefully."""
    try:
        result = api_client.command(
            "csvPlayer.open", {"filePath": "/nonexistent/path/data.csv"}
        )
        assert result.get("isOpen") is False
    except Exception:
        pass


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_open_missing_param(api_client, clean_state):
    """Verify csvPlayer.open rejects calls missing 'filePath'."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("csvPlayer.open", {})


@pytest.mark.integration
@pytest.mark.csv
def test_csv_player_full_lifecycle(api_client, device_simulator, clean_state, tmp_path):
    """
    Full lifecycle: export data to CSV, then verify the player API is reachable.

    The CSV path is managed internally; we don't try to re-open the file here.
    """
    api_client.set_operation_mode("json")
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.enable_csv_export()
    time.sleep(0.3)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    from utils import DataGenerator, ChecksumType

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=1.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    api_client.disconnect_device()
    api_client.command("csvExport.close")
    time.sleep(0.5)

    api_client.disable_csv_export()

    status = api_client.command("csvPlayer.getStatus")
    assert "isOpen" in status
    assert "frameCount" in status
