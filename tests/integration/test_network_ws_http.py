"""
Network Driver WebSocket / HTTP Client Integration Tests (spec 0068)

Covers the API surface of the two URL transports: the socket-type enumeration, the
round-trip of every new property through io.network.getConfig, scheme validation, and
the poll counters exposed by io.network.getStatus.

Requires a running Serial Studio with the API server enabled (Preferences -> API &
Plugins -> Enable API Server).

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import pytest

SOCKET_TYPES = [(0, "TCP"), (1, "UDP"), (2, "WebSocket"), (3, "HTTP")]


@pytest.mark.integration
def test_socket_types_are_appended_not_renumbered(api_client, clean_state):
    """TCP and UDP keep indices 0 and 1; the URL transports are appended after them."""
    result = api_client.command("io.network.listSocketTypes")
    types = result["socketTypes"]

    assert len(types) == len(SOCKET_TYPES)
    for entry, (index, name) in zip(types, SOCKET_TYPES):
        assert entry["index"] == index
        assert entry["name"] == name


@pytest.mark.integration
def test_websocket_properties_round_trip(api_client, clean_state):
    """Every WebSocket setting written through the API reads back from getConfig."""
    api_client.command("io.setBusType", {"busType": 1})
    api_client.command("io.network.setSocketType", {"socketTypeIndex": 2})
    api_client.command(
        "io.network.setWebSocketUrl", {"url": "ws://127.0.0.1:8123/feed"}
    )
    api_client.command("io.network.setIgnoreTlsErrors", {"enabled": True})

    config = api_client.command("io.network.getConfig")
    assert config["socketTypeIndex"] == 2
    assert config["webSocketUrl"] == "ws://127.0.0.1:8123/feed"
    assert config["ignoreTlsErrors"] is True

    api_client.command("io.network.setIgnoreTlsErrors", {"enabled": False})
    assert api_client.command("io.network.getConfig")["ignoreTlsErrors"] is False


@pytest.mark.integration
def test_http_properties_round_trip(api_client, clean_state):
    """Every HTTP setting written through the API reads back from getConfig."""
    api_client.command("io.setBusType", {"busType": 1})
    api_client.command("io.network.setSocketType", {"socketTypeIndex": 3})
    api_client.command(
        "io.network.setHttpUrl", {"url": "http://127.0.0.1:8124/telemetry"}
    )
    api_client.command("io.network.setHttpMethod", {"method": "POST"})
    api_client.command("io.network.setHttpBody", {"body": '{"probe":1}'})
    api_client.command(
        "io.network.setHttpHeaders",
        {"headers": "X-Api-Key: secret\nAccept: application/json"},
    )
    api_client.command("io.network.setHttpInterval", {"interval": 250})

    config = api_client.command("io.network.getConfig")
    assert config["socketTypeIndex"] == 3
    assert config["httpUrl"] == "http://127.0.0.1:8124/telemetry"
    assert config["httpMethod"] == "POST"
    assert config["httpBody"] == '{"probe":1}'
    assert "X-Api-Key: secret" in config["httpHeaders"]
    assert config["httpInterval"] == 250


@pytest.mark.integration
def test_http_interval_zero_means_manual_only(api_client, clean_state):
    """Interval 0 is preserved verbatim; it is the 'send only on write' mode."""
    api_client.command("io.setBusType", {"busType": 1})
    api_client.command("io.network.setSocketType", {"socketTypeIndex": 3})
    api_client.command("io.network.setHttpInterval", {"interval": 0})

    assert api_client.command("io.network.getConfig")["httpInterval"] == 0


@pytest.mark.integration
@pytest.mark.parametrize(
    "command,url",
    [
        ("io.network.setWebSocketUrl", "http://127.0.0.1:8080/"),
        ("io.network.setWebSocketUrl", "not-a-url"),
        ("io.network.setHttpUrl", "ws://127.0.0.1:8080/"),
        ("io.network.setHttpUrl", "not-a-url"),
    ],
)
def test_mismatched_url_scheme_is_rejected(api_client, clean_state, command, url):
    """A URL for the wrong transport is refused rather than stored."""
    api_client.command("io.setBusType", {"busType": 1})

    before = api_client.command("io.network.getConfig")
    key = "webSocketUrl" if "WebSocket" in command else "httpUrl"

    with pytest.raises(Exception):
        api_client.command(command, {"url": url})

    after = api_client.command("io.network.getConfig")
    assert after[key] == before[key]


@pytest.mark.integration
def test_status_reports_poll_counters(api_client, clean_state):
    """getStatus is read-only and always answers with the counter fields."""
    api_client.command("io.setBusType", {"busType": 1})

    status = api_client.command("io.network.getStatus")
    for key in ("pollsOk", "pollsFailed", "pollsSkipped", "consecutiveFailures"):
        assert key in status
        assert status[key] >= 0

    assert "isOpen" in status
    assert "isConnecting" in status
