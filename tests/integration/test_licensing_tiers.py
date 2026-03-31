"""
Licensing Tier Integration Tests

Tests for the per-feature tier gating system introduced with the Hobbyist tier.
Verifies that licensing.getStatus returns the new featureTier and
featureTierValue fields, and that tier names are correctly mapped.

These tests require Serial Studio Pro (BUILD_COMMERCIAL) to be running.
If the licensing.getStatus command is absent the tests are skipped.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

VALID_TIER_NAMES = {"None", "Hobbyist", "Trial", "Pro", "Enterprise"}

TIER_VALUES = {
    "None": 0,
    "Hobbyist": 1,
    "Trial": 2,
    "Pro": 3,
    "Enterprise": 4,
}


def _licensing_available(api_client) -> bool:
    """Return True if the licensing API commands are registered."""
    return api_client.command_exists("licensing.getStatus")


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(autouse=True)
def require_licensing(api_client):
    """Skip entire module when running against a GPL build."""
    if not _licensing_available(api_client):
        pytest.skip("licensing API not available (GPL build)")


# ---------------------------------------------------------------------------
# Feature tier field tests
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_get_status_includes_feature_tier(api_client):
    """licensing.getStatus must include featureTier and featureTierValue."""
    status = api_client.command("licensing.getStatus")

    assert "featureTier" in status, "Missing featureTier field"
    assert "featureTierValue" in status, "Missing featureTierValue field"
    assert isinstance(status["featureTier"], str)
    assert isinstance(status["featureTierValue"], int)


@pytest.mark.integration
def test_feature_tier_is_valid_name(api_client):
    """featureTier must be one of the recognized tier names."""
    status = api_client.command("licensing.getStatus")
    tier = status["featureTier"]
    assert tier in VALID_TIER_NAMES, \
        f"Unexpected tier name: '{tier}', expected one of {VALID_TIER_NAMES}"


@pytest.mark.integration
def test_feature_tier_value_matches_name(api_client):
    """featureTierValue must be consistent with featureTier name."""
    status = api_client.command("licensing.getStatus")
    tier_name = status["featureTier"]
    tier_value = status["featureTierValue"]

    expected_value = TIER_VALUES.get(tier_name)
    assert expected_value is not None, f"No expected value for tier: {tier_name}"
    assert tier_value == expected_value, \
        f"Tier '{tier_name}' should have value {expected_value}, got {tier_value}"


@pytest.mark.integration
def test_feature_tier_value_in_range(api_client):
    """featureTierValue must be between 0 and 4."""
    status = api_client.command("licensing.getStatus")
    tier_value = status["featureTierValue"]
    assert 0 <= tier_value <= 4, \
        f"featureTierValue out of range: {tier_value}"


# ---------------------------------------------------------------------------
# Tier-to-activation consistency
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_activated_implies_nonzero_tier(api_client):
    """If isActivated is true, featureTier must not be None."""
    status = api_client.command("licensing.getStatus")
    if status["isActivated"]:
        assert status["featureTier"] != "None", \
            "Activated license should have a non-None tier"
        assert status["featureTierValue"] > 0, \
            "Activated license should have tier value > 0"


@pytest.mark.integration
def test_variant_name_consistent_with_tier(api_client):
    """variantName should be consistent with featureTier mapping."""
    status = api_client.command("licensing.getStatus")

    if not status["isActivated"]:
        pytest.skip("No active license to check variant-tier mapping")

    variant = status.get("variantName", "").lower()
    tier = status["featureTier"]

    # Check known mappings
    if variant.startswith("enterprise"):
        assert tier == "Enterprise"
    elif variant.startswith("hobbyist"):
        assert tier == "Hobbyist"
    elif variant.startswith("pro") or variant.startswith("team"):
        assert tier == "Pro"


# ---------------------------------------------------------------------------
# Trial tier consistency
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_trial_tier_is_trial(api_client):
    """When trial is enabled, featureTier should be Trial (value 2)."""
    if not api_client.command_exists("licensing.trial.getStatus"):
        pytest.skip("trial API not available")

    trial = api_client.command("licensing.trial.getStatus")
    status = api_client.command("licensing.getStatus")

    if trial.get("trialEnabled") and not status.get("isActivated"):
        assert status["featureTier"] == "Trial", \
            f"Active trial should show Trial tier, got {status['featureTier']}"
        assert status["featureTierValue"] == TIER_VALUES["Trial"]


# ---------------------------------------------------------------------------
# Guard integrity with new tiers
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_guards_still_pass_with_new_tiers(api_client):
    """All license guards must still pass after the tier enum change."""
    result = api_client.command("licensing.guardStatus")

    assert result.get("allOk") is True, \
        f"License guards failed after tier changes: {result.get('failures')}"
