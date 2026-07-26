"""
Assistant Memory Proposal Integration Tests

Tests for assistant.memory.propose (spec 0008): the command must be registered,
validate its inputs, and stay side-effect-free -- it only queues a proposal for
the user's confirmation chip and never stores anything by itself. Persistence
happens exclusively through the in-app confirmation UI, which these API-level
tests deliberately cannot reach.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import pytest

from utils.api_client import APIError


@pytest.mark.integration
def test_memory_propose_is_registered(api_client, clean_state):
    """Verify assistant.memory.propose exists in the command registry."""
    assert api_client.command_exists("assistant.memory.propose")


@pytest.mark.integration
def test_memory_propose_queues_without_storing(api_client, clean_state):
    """Verify a valid proposal returns queued=true and reports no storage."""
    result = api_client.command(
        "assistant.memory.propose",
        {"category": "feedback", "text": "Prefer Lua for frame parsers"},
    )

    assert result.get("queued") is True
    assert "note" in result
    assert "path" not in result, "Proposal must not write anything to disk"
    assert "stored" not in result, "Proposal must not report a stored fact"


@pytest.mark.integration
@pytest.mark.parametrize("category", ["", "unknown", "secrets", "USER"])
def test_memory_propose_rejects_bad_category(api_client, clean_state, category):
    """Verify unknown categories are rejected with an error."""
    with pytest.raises(APIError):
        api_client.command(
            "assistant.memory.propose",
            {"category": category, "text": "some fact"},
        )


@pytest.mark.integration
def test_memory_propose_rejects_empty_text(api_client, clean_state):
    """Verify an empty fact is rejected."""
    with pytest.raises(APIError):
        api_client.command(
            "assistant.memory.propose",
            {"category": "user", "text": "   "},
        )


@pytest.mark.integration
def test_memory_propose_rejects_oversized_text(api_client, clean_state):
    """Verify a fact above the 400-character cap is rejected."""
    with pytest.raises(APIError):
        api_client.command(
            "assistant.memory.propose",
            {"category": "user", "text": "x" * 500},
        )


@pytest.mark.integration
def test_memory_propose_repeat_is_idempotent(api_client, clean_state):
    """Verify repeated proposals keep queueing without error or side effects."""
    for _ in range(3):
        result = api_client.command(
            "assistant.memory.propose",
            {"category": "project", "text": "Telemetry frames are CSV at 115200 baud"},
        )
        assert result.get("queued") is True
