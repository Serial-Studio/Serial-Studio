"""
Licensing API Integration Tests

Tests for the licensing.* and licensing.trial.* API commands, covering:
- Read-only status queries (smoke tests that verify the app is alive)
- Static-init crash regression: the app must survive rapid repeated connects
  that stress the LemonSqueezy/Trial singleton initialization path (the bug
  that caused SIGABRT via __cxa_guard_acquire on macOS and a hang on Windows
  when showMessageBox was called from inside the constructor).
- Input validation: missing / malformed params must return errors, not crash.
- Idempotency: calling validate/deactivate in bad states returns graceful errors.

These tests require Serial Studio Pro (BUILD_COMMERCIAL) to be running.
If the licensing.getStatus command is absent the tests are skipped.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _licensing_available(api_client) -> bool:
    """Return True if the licensing API commands are registered."""
    return api_client.command_exists("licensing.getStatus")


def _trial_available(api_client) -> bool:
    return api_client.command_exists("licensing.trial.getStatus")


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(autouse=True)
def require_licensing(api_client):
    """Skip entire module when running against a GPL build."""
    if not _licensing_available(api_client):
        pytest.skip("licensing API not available (GPL build or API server not enabled)")


# ---------------------------------------------------------------------------
# Smoke / status tests
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_licensing_get_status_shape(api_client):
    """licensing.getStatus returns the expected fields without crashing."""
    status = api_client.command("licensing.getStatus")

    assert isinstance(status.get("busy"), bool)
    assert isinstance(status.get("isActivated"), bool)
    assert isinstance(status.get("canActivate"), bool)
    assert isinstance(status.get("appName"), str)
    assert isinstance(status.get("license"), str)
    assert isinstance(status.get("instanceId"), str)
    assert isinstance(status.get("variantName"), str)
    assert isinstance(status.get("instanceName"), str)
    assert isinstance(status.get("customerName"), str)
    assert isinstance(status.get("customerEmail"), str)
    assert isinstance(status.get("seatLimit"), int)
    assert isinstance(status.get("seatUsage"), int)


@pytest.mark.integration
def test_trial_get_status_shape(api_client):
    """licensing.trial.getStatus returns the expected fields without crashing."""
    if not _trial_available(api_client):
        pytest.skip("trial API not available")

    status = api_client.command("licensing.trial.getStatus")

    assert isinstance(status.get("busy"), bool)
    assert isinstance(status.get("firstRun"), bool)
    assert isinstance(status.get("trialEnabled"), bool)
    assert isinstance(status.get("trialExpired"), bool)
    assert isinstance(status.get("trialAvailable"), bool)
    assert isinstance(status.get("daysRemaining"), int)


@pytest.mark.integration
def test_licensing_status_consistent(api_client):
    """isActivated and canActivate are consistent with license field."""
    status = api_client.command("licensing.getStatus")

    license_key = status.get("license", "")
    can_activate = status.get("canActivate", False)

    # canActivate requires exactly 36-char license key (UUID)
    if len(license_key) == 36:
        assert can_activate is True
    else:
        assert can_activate is False


@pytest.mark.integration
def test_licensing_app_name_nonempty(api_client):
    """appName must always be a non-empty string."""
    status = api_client.command("licensing.getStatus")
    assert len(status.get("appName", "")) > 0


# ---------------------------------------------------------------------------
# Input validation
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_set_license_missing_param(api_client):
    """licensing.setLicense without licenseKey must return a validation error."""
    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.setLicense", {})

    assert exc_info.value.code in ("MISSING_PARAM", "INVALID_PARAM", "VALIDATION_ERROR",
                                   "missing_param", "invalid_param")


@pytest.mark.integration
def test_set_license_empty_string(api_client):
    """Setting an empty license key is accepted but canActivate becomes False."""
    api_client.command("licensing.setLicense", {"licenseKey": ""})
    status = api_client.command("licensing.getStatus")
    assert status.get("canActivate") is False


@pytest.mark.integration
def test_set_license_too_short(api_client):
    """A license key shorter than 36 chars means canActivate is False."""
    api_client.command("licensing.setLicense", {"licenseKey": "short-key"})
    status = api_client.command("licensing.getStatus")
    assert status.get("canActivate") is False


@pytest.mark.integration
def test_set_license_valid_uuid_format(api_client):
    """A 36-char UUID-shaped key sets canActivate to True (format check only)."""
    dummy_key = "00000000-0000-0000-0000-000000000000"
    api_client.command("licensing.setLicense", {"licenseKey": dummy_key})
    status = api_client.command("licensing.getStatus")
    assert status.get("canActivate") is True

    # Restore empty key so we don't leave junk state
    api_client.command("licensing.setLicense", {"licenseKey": ""})


# ---------------------------------------------------------------------------
# Idempotency / graceful error states
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_activate_without_license_key(api_client):
    """Calling activate with no key set returns an error, not a crash."""
    api_client.command("licensing.setLicense", {"licenseKey": ""})

    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.activate")

    assert exc_info.value.code in (
        "INVALID_PARAM", "EXECUTION_ERROR", "MISSING_PARAM"
    )


@pytest.mark.integration
def test_deactivate_when_not_activated(api_client):
    """Calling deactivate when not activated returns an error, not a crash."""
    status = api_client.command("licensing.getStatus")
    if status.get("isActivated"):
        pytest.skip("Machine has an active license; skipping deactivate-when-inactive test")

    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.deactivate")

    assert exc_info.value.code in (
        "INVALID_PARAM", "EXECUTION_ERROR", "MISSING_PARAM"
    )


@pytest.mark.integration
def test_trial_enable_when_not_available(api_client):
    """Calling trial.enable when trial is unavailable returns an error."""
    if not _trial_available(api_client):
        pytest.skip("trial API not available")

    trial_status = api_client.command("licensing.trial.getStatus")
    if trial_status.get("trialAvailable"):
        pytest.skip("Trial IS available on this machine; skipping unavailability test")

    with pytest.raises(APIError) as exc_info:
        api_client.command("licensing.trial.enable")

    assert exc_info.value.code in (
        "INVALID_PARAM", "EXECUTION_ERROR", "MISSING_PARAM"
    )


# ---------------------------------------------------------------------------
# Regression: static-init crash (SIGABRT/__cxa_guard_acquire deadlock)
#
# The bug: LemonSqueezy::readValidationResponse() called showMessageBox()
# (→ QDialog::exec()) while still inside the static singleton constructor,
# with the __cxa_guard held. Any re-entrant access to the same singleton
# from the nested event loop → abort().
#
# We can't reproduce the exact constructor path via the API (the singleton
# is already initialized by the time the server accepts connections), but we
# can:
#  1. Hammer status queries across many connections to flush out any deferred
#     init paths that might be triggered by rapid reconnects.
#  2. Drive validate() which re-enters the same validation path that formerly
#     showed dialogs, ensuring it completes without hanging or crashing.
#  3. Interleave set/activate/status calls from concurrent connections, which
#     stress the same signal paths the constructor used to trigger.
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_rapid_status_queries_no_crash(serial_studio_running):
    """
    Regression: open/close 20 independent connections in quick succession and
    query licensing.getStatus on each.  The app must remain responsive
    throughout and return valid responses.
    """
    failures = []

    for i in range(20):
        try:
            with SerialStudioClient() as c:
                status = c.command("licensing.getStatus")
                assert isinstance(status.get("isActivated"), bool), \
                    f"Unexpected response on iteration {i}: {status}"
        except Exception as e:
            failures.append(f"iteration {i}: {e}")

        # No sleep — we want maximum stress on init paths
        # (but yield to the event loop between iterations)

    assert not failures, f"Rapid status queries failed:\n" + "\n".join(failures)


@pytest.mark.integration
def test_validate_does_not_hang(api_client):
    """
    Regression: licensing.validate() must return promptly without hanging.
    Before the fix, the validate path called showMessageBox() in some error
    branches, which could deadlock on Windows.

    We fire validate() and then immediately query status — if the app were
    deadlocked we'd get a timeout here.
    """
    # validate() is fire-and-forget (async network call); it should return
    # immediately (the result comes later via signal). An error is fine.
    try:
        api_client.command("licensing.validate")
    except APIError:
        pass  # "operation in progress" or "no key" — both are fine

    # The key assertion: the app is still responsive after validate()
    status = api_client.command("licensing.getStatus", timeout=3.0)
    assert isinstance(status.get("isActivated"), bool)


@pytest.mark.integration
def test_interleaved_set_and_status(api_client):
    """
    Regression: rapidly interleave setLicense + getStatus calls to stress
    the signal paths that formerly ran through showMessageBox in the
    constructor. App must survive without crashing or hanging.
    """
    dummy = "00000000-0000-0000-0000-000000000000"

    for _ in range(10):
        api_client.command("licensing.setLicense", {"licenseKey": dummy})
        status = api_client.command("licensing.getStatus")
        assert status.get("canActivate") is True

        api_client.command("licensing.setLicense", {"licenseKey": ""})
        status = api_client.command("licensing.getStatus")
        assert status.get("canActivate") is False

    # Final state: leave license empty
    api_client.command("licensing.setLicense", {"licenseKey": ""})


@pytest.mark.integration
def test_concurrent_connections_status(serial_studio_running):
    """
    Open several connections simultaneously and query licensing status on each.
    Tests that the singleton survives concurrent read access from multiple
    API clients without corruption or crash.
    """
    import threading

    results = {}
    errors = {}

    def query(idx):
        try:
            with SerialStudioClient() as c:
                results[idx] = c.command("licensing.getStatus")
        except Exception as e:
            errors[idx] = str(e)

    threads = [threading.Thread(target=query, args=(i,)) for i in range(8)]
    for t in threads:
        t.start()
    for t in threads:
        t.join(timeout=10)

    assert not errors, f"Concurrent queries produced errors: {errors}"
    assert len(results) == 8

    for idx, status in results.items():
        assert isinstance(status.get("isActivated"), bool), \
            f"Thread {idx} got unexpected result: {status}"
