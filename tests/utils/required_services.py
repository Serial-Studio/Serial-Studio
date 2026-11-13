"""
Required Services

A test that needs an external service (the MQTT broker, the OPC UA simulator) skips when it
cannot reach it on a developer's machine, but must fail on a CI leg that started that service:
a skip there turned "the broker never came up" into a green run (spec 0120 R14).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import os

import pytest

#: Set to "1" by .github/workflows/test-suite.yml on legs that start the MQTT broker.
REQUIRE_BROKER = "SS_REQUIRE_BROKER"

#: Set to "1" by .github/workflows/test-suite.yml on legs that start the OPC UA simulator.
REQUIRE_OPCUA_SIM = "SS_REQUIRE_OPCUA_SIM"


def required(env_var: str) -> bool:
    """Whether this run promised the service behind `env_var`."""
    return os.environ.get(env_var, "") == "1"


def missing_service(message: str, env_var: str) -> None:
    """Skip when the service is optional here; fail when this leg started it."""
    if required(env_var):
        pytest.fail(
            f"{message} ({env_var}=1: this leg started it, so its absence is a failure)"
        )

    pytest.skip(message)
