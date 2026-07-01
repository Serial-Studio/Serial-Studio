"""
Offline Activation API Integration Tests

Tests for the licensing.activateOffline command and the offline fields of
licensing.getStatus, covering:
- Status shape: machineId + offline* fields are always present.
- Input validation: missing param / unreadable file / garbage cert return
  distinct errors, never a crash and never a partial activation.
- Happy path (opt-in): when SS_OFFLINE_TEST_CERT points at a .sslic minted for
  THIS machine (with the production signing key), importing it activates Pro
  with no online license involved.

Minting a valid certificate requires the Ed25519 private key held by the
operator (never in the repo), so the positive path is gated behind an env var
the maintainer sets after minting a cert with tools/ss-license-sign. The
negative and shape cases run unconditionally.

These tests require Serial Studio Pro (BUILD_COMMERCIAL) to be running with the
API server enabled. If licensing.activateOffline is absent they are skipped.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import os

import pytest

from utils.api_client import APIError

# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture(autouse=True)
def require_offline(api_client):
    """Skip the module when offline activation is unavailable (GPL build)."""
    if not api_client.command_exists("licensing.activateOffline"):
        pytest.skip(
            "offline activation API not available (GPL build or API server off)"
        )


# ---------------------------------------------------------------------------
# Status shape
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_status_exposes_offline_fields(api_client):
    """licensing.getStatus exposes the machine id and offline license fields."""
    status = api_client.command("licensing.getStatus")

    assert isinstance(status.get("machineId"), str)
    assert len(status["machineId"]) > 0
    assert isinstance(status.get("offlineActivated"), bool)
    assert isinstance(status.get("offlineDaysRemaining"), int)
    assert isinstance(status.get("offlineExpiresAt"), str)


# ---------------------------------------------------------------------------
# Input validation / rejection taxonomy
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_activate_offline_missing_param(api_client):
    """activateOffline without a path returns a validation error, not a crash."""
    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.activateOffline", {})

    assert exc_info.value.code in ("MISSING_PARAM", "INVALID_PARAM", "VALIDATION_ERROR")


@pytest.mark.integration
def test_activate_offline_unreadable_file(api_client):
    """A path that does not exist returns an error and does not activate."""
    with pytest.raises(APIError) as exc_info:
        api_client.command(
            "licensing.activateOffline", {"path": "/nonexistent/does-not-exist.sslic"}
        )

    assert exc_info.value.code in ("EXECUTION_ERROR", "INVALID_PARAM")
    status = api_client.command("licensing.getStatus")
    assert status.get("offlineActivated") is False


@pytest.mark.integration
def test_activate_offline_garbage_cert(api_client, tmp_path):
    """A file that is not a valid certificate is rejected without activating."""
    bogus = tmp_path / "bogus.sslic"
    bogus.write_text("this is not a certificate", encoding="utf-8")

    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.activateOffline", {"path": str(bogus)})

    assert exc_info.value.code == "EXECUTION_ERROR"
    assert len(str(exc_info.value)) > 0
    status = api_client.command("licensing.getStatus")
    assert status.get("offlineActivated") is False


@pytest.mark.integration
def test_activate_offline_truncated_cert(api_client, tmp_path):
    """A base64 blob too short to hold a signature is rejected as malformed."""
    short = tmp_path / "short.sslic"
    short.write_text("AAAA", encoding="utf-8")

    with pytest.raises(APIError):
        api_client.command("licensing.activateOffline", {"path": str(short)})

    status = api_client.command("licensing.getStatus")
    assert status.get("offlineActivated") is False


# ---------------------------------------------------------------------------
# Happy path (opt-in: requires a cert minted for this machine)
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_activate_offline_valid_cert_unlocks_pro(api_client):
    """
    With a certificate minted for THIS machine, importing it unlocks Pro with
    no online license involved.

    Set SS_OFFLINE_TEST_CERT to the path of a .sslic file signed with the
    production key for the machineId reported by licensing.getStatus. To also
    verify persistence across restarts, run this, restart the app, and confirm
    offlineActivated is still true (not automated here — the fixture keeps one
    app instance for the session).
    """
    cert_path = os.environ.get("SS_OFFLINE_TEST_CERT")
    if not cert_path:
        pytest.skip("set SS_OFFLINE_TEST_CERT to a .sslic minted for this machine")

    before = api_client.command("licensing.getStatus")
    api_client.command("licensing.activateOffline", {"path": cert_path})
    after = api_client.command("licensing.getStatus")

    assert after.get("offlineActivated") is True
    assert after.get("offlineDaysRemaining", 0) > 0
    assert after.get("featureTierValue", 0) >= 3  # Pro (3) or Enterprise (4)

    # Offline activation must not have gone through the online LemonSqueezy path.
    assert after.get("isActivated") == before.get("isActivated")
