"""
Batch API Integration Tests

Tests for the batch command feature: sending multiple commands in a single
request, verifying results, and exercising size/error boundaries.

The server accepts up to 256 commands per batch. Batches are sent as:
  {"type": "batch", "id": "<uuid>", "commands": [...]}

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


@pytest.mark.integration
def test_batch_read_only_queries(api_client, clean_state):
    """Verify a batch of read-only commands returns one result per command."""
    commands = [
        {"command": "io.manager.getStatus"},
        {"command": "dashboard.getStatus"},
        {"command": "csv.export.getStatus"},
        {"command": "console.getConfiguration"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list), "Batch should return a list of results"
    assert len(results) == len(commands), "Result count must match command count"

    for result in results:
        assert result.get("success") is True, f"All batch results should succeed: {result}"


@pytest.mark.integration
def test_batch_mixed_read_write(api_client, clean_state):
    """Verify a batch that sets and then reads values returns consistent state."""
    commands = [
        {"command": "dashboard.setFPS", "params": {"fps": 20}},
        {"command": "dashboard.setPoints", "params": {"points": 600}},
        {"command": "dashboard.getStatus"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list)
    assert len(results) == 3

    # All should succeed
    for result in results:
        assert result.get("success") is True

    # getStatus result is the third
    status = results[2].get("result", {})
    assert status.get("fps") == 20
    assert status.get("points") == 600


@pytest.mark.integration
def test_batch_result_order_preserved(api_client, clean_state):
    """Verify batch results appear in the same order commands were sent."""
    commands = [
        {"command": "dashboard.setFPS", "params": {"fps": 15}},
        {"command": "dashboard.setFPS", "params": {"fps": 30}},
        {"command": "dashboard.setFPS", "params": {"fps": 60}},
        {"command": "dashboard.getFPS"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list)
    assert len(results) == 4

    # The final getFPS should return the last set value (60)
    final = results[3].get("result", {})
    assert final.get("fps") == 60


@pytest.mark.integration
def test_batch_single_command(api_client, clean_state):
    """Verify a batch with a single command is accepted and returns one result."""
    results = api_client.batch([{"command": "io.manager.getStatus"}])
    assert isinstance(results, list)
    assert len(results) == 1
    assert results[0].get("success") is True


@pytest.mark.integration
def test_batch_console_settings(api_client, clean_state):
    """Verify a batch can configure multiple console settings atomically."""
    commands = [
        {"command": "console.setEcho", "params": {"enabled": False}},
        {"command": "console.setShowTimestamp", "params": {"enabled": False}},
        {"command": "console.setDisplayMode", "params": {"modeIndex": 0}},
        {"command": "console.setDataMode", "params": {"modeIndex": 0}},
        {"command": "console.setLineEnding", "params": {"endingIndex": 0}},
        {"command": "console.getConfiguration"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list)
    assert len(results) == len(commands)

    for result in results:
        assert result.get("success") is True

    config = results[-1].get("result", {})
    assert config.get("echo") is False
    assert config.get("showTimestamp") is False
    assert config.get("displayMode") == 0
    assert config.get("dataMode") == 0
    assert config.get("lineEnding") == 0


@pytest.mark.integration
def test_batch_too_large_rejected(api_client, clean_state):
    """Verify batches exceeding 256 commands are rejected by the server."""
    commands = [{"command": "io.manager.getStatus"}] * 257

    result = api_client.batch(commands, timeout=10.0)

    # The server should reject this batch; either:
    # - Returns an error dict (batch rejected)
    # - Or raises APIError
    if isinstance(result, dict):
        assert result.get("error") is True or result.get("success") is False
    else:
        # If returned as list, it should not have 257 results from an invalid batch
        assert len(result) <= 256


@pytest.mark.integration
def test_batch_at_limit(api_client, clean_state):
    """Verify a batch of exactly 256 commands is accepted."""
    commands = [{"command": "io.manager.getStatus"}] * 256

    results = api_client.batch(commands, timeout=30.0)
    assert isinstance(results, list)
    assert len(results) == 256

    for result in results:
        assert result.get("success") is True


@pytest.mark.integration
def test_batch_network_configuration(api_client, clean_state):
    """Verify network driver can be fully configured via a single batch."""
    commands = [
        {"command": "io.manager.setBusType", "params": {"busType": 1}},
        {"command": "io.driver.network.setRemoteAddress", "params": {"address": "127.0.0.1"}},
        {"command": "io.driver.network.setSocketType", "params": {"socketTypeIndex": 0}},
        {"command": "io.driver.network.setTcpPort", "params": {"port": 9000}},
        {"command": "io.driver.network.getConfiguration"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list)
    assert len(results) == len(commands)

    for result in results:
        assert result.get("success") is True, f"Batch command failed: {result}"

    config = results[-1].get("result", {})
    assert config.get("remoteAddress") == "127.0.0.1"
    assert config.get("tcpPort") == 9000
    assert config.get("socketTypeIndex") == 0


@pytest.mark.integration
def test_batch_partial_failure_does_not_abort(api_client, clean_state):
    """Verify that one failing command in a batch does not abort remaining commands."""
    commands = [
        {"command": "dashboard.setFPS", "params": {"fps": 30}},
        # This will fail: invalid FPS
        {"command": "dashboard.setFPS", "params": {"fps": 9999}},
        {"command": "dashboard.getFPS"},
    ]

    results = api_client.batch(commands)
    assert isinstance(results, list)
    assert len(results) == 3

    # First command should succeed
    assert results[0].get("success") is True

    # Second command should fail
    assert results[1].get("success") is False

    # Third command should still succeed and reflect first command's value
    assert results[2].get("success") is True
    assert results[2].get("result", {}).get("fps") == 30
