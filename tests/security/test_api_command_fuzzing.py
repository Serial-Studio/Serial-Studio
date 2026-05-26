"""
API Command-Parameter Fuzzing

Existing fuzz coverage targets protocol syntax (test_protocol_fuzzing.py) and
device-side frame parsing (test_fuzzy.py). This suite fills the remaining gap:
the *semantic* layer -- well-formed JSON commands carrying hostile parameter
payloads (wrong types, out-of-range numbers, injection strings, malformed
project configs, abusive batches).

The invariant under test is resilience, not correctness: every input must be
handled without crashing or hanging the server. A clean rejection (APIError) or
a defensive disconnect (ConnectionError) is a pass; a TimeoutError -- meaning the
main thread hung -- propagates and fails the test. After each barrage a known-good
command must still succeed, proving the server is still alive.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import random

import pytest

from utils.api_client import APIError

pytestmark = [pytest.mark.security, pytest.mark.fuzzing]

# ---------------------------------------------------------------------------
# Hostile parameter values
# ---------------------------------------------------------------------------

FUZZ_VALUES = [
    None,
    True,
    False,
    0,
    -1,
    1,
    2**31,
    -(2**31),
    2**63,
    -(2**63),
    2**63 + 1,
    0.0,
    -1.5,
    float("1e308"),
    "",
    "x" * 10000,
    "../../../etc/passwd",
    "$(rm -rf /)",
    "1; DROP TABLE users;--",
    "%s%n%x",
    "{{7*7}}",
    "\x00\x01\x02",
    [],
    [1, 2, 3],
    [None] * 100,
    {"nested": {"deep": {"deeper": [1, 2, 3]}}},
]

# Known commands present in every (GPL) build, with their documented param keys.
TARGET_COMMANDS = [
    ("io.setBusType", ["busType"]),
    ("io.setPaused", ["paused"]),
    ("dashboard.setFps", ["fps"]),
    ("dashboard.setPoints", ["points"]),
    ("dashboard.setOperationMode", ["mode"]),
    ("io.network.setTcpPort", ["port"]),
    ("io.network.setRemoteAddress", ["address"]),
    ("io.uart.setBaudRate", ["baudRate"]),
    ("io.uart.setPortIndex", ["portIndex"]),
    ("project.group.add", ["title", "widgetType"]),
    ("project.dataset.add", ["options"]),
]


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _fuzz_send(api_client, command, params):
    """Send one fuzz command.

    APIError       -> server rejected the input cleanly (pass).
    ConnectionError -> server defensively dropped the client (pass); reconnect.
    TimeoutError    -> not caught here; it means a hang and must fail the test.
    """
    try:
        api_client.command(command, params)
    except APIError:
        pass
    except ConnectionError:
        api_client.disconnect()
        api_client.connect()


def _assert_server_alive(api_client):
    """A known-good command must still succeed, proving the server survived."""
    try:
        result = api_client.command("api.getCommands")
    except ConnectionError:
        api_client.disconnect()
        api_client.connect()
        result = api_client.command("api.getCommands")

    assert "commands" in result, "server stopped answering after fuzzing"


# ---------------------------------------------------------------------------
# loadJson schema fuzzing
# ---------------------------------------------------------------------------


@pytest.mark.high
def test_loadjson_malformed_configs(api_client, clean_state):
    """project.loadJson with garbage `config` payloads must never crash the app."""
    bad_configs = [
        {},  # missing config
        {"config": None},
        {"config": "not-an-object"},
        {"config": 12345},
        {"config": []},
        {"config": {"groups": "not-an-array"}},
        {"config": {"groups": [None, 1, "x"]}},
        {"config": {"groups": [{"datasets": 42}]}},
        {"config": {"groups": [{"datasets": [{"index": "abc"}]}]}},
        {"config": {"groups": [{}] * 5000}},
        {"config": {"title": "x" * 100000}},
        {"config": {"frameParser": "while(true){}"}},
    ]

    for payload in bad_configs:
        _fuzz_send(api_client, "project.loadJson", payload)

    _assert_server_alive(api_client)


# ---------------------------------------------------------------------------
# Parameter type / range abuse
# ---------------------------------------------------------------------------


@pytest.mark.high
def test_command_param_type_abuse(api_client):
    """Each known command gets every hostile value in each of its param slots."""
    for command, keys in TARGET_COMMANDS:
        for key in keys:
            for value in FUZZ_VALUES:
                _fuzz_send(api_client, command, {key: value})

        # Also throw an unexpected extra key alongside a plausible one.
        _fuzz_send(api_client, command, {"unexpected": "fuzz", keys[0]: None})

    _assert_server_alive(api_client)


# ---------------------------------------------------------------------------
# Batch fuzzing
# ---------------------------------------------------------------------------


@pytest.mark.medium
def test_batch_edge_cases(api_client):
    """Empty, malformed, oversized, and unknown-command batches stay contained."""
    batches = [
        [],
        [{"command": "api.getCommands"}] * 256,
        [{"command": "does.not.exist"}],
        [{"command": "io.setBusType", "params": {"busType": v}} for v in FUZZ_VALUES],
        [{"command": "dashboard.setFps", "params": {"fps": 2**40}}],
    ]

    for cmds in batches:
        try:
            api_client.batch(cmds)
        except (APIError, ConnectionError):
            api_client.disconnect()
            api_client.connect()

    _assert_server_alive(api_client)


# ---------------------------------------------------------------------------
# Randomized fuzzing
# ---------------------------------------------------------------------------


@pytest.mark.medium
def test_random_param_fuzz(api_client):
    """Randomized command/param combinations over many iterations stay contained."""
    rng = random.Random(0xC0FFEE)

    for i in range(150):
        command, keys = rng.choice(TARGET_COMMANDS)

        params = {}
        for key in keys:
            if rng.random() < 0.8:
                params[key] = rng.choice(FUZZ_VALUES)

        # Sometimes inject a random extra key.
        if rng.random() < 0.3:
            params["k%d" % rng.randint(0, 999)] = rng.choice(FUZZ_VALUES)

        _fuzz_send(api_client, command, params)

        # Periodically confirm the server is still responsive.
        if i % 50 == 0:
            _assert_server_alive(api_client)

    _assert_server_alive(api_client)
