"""
Spec-0040 Remote Dashboard Mirror -- Live Integration

Drives the mirror wire contract against a *running* Serial Studio capture
instance, exercising the viewer half of spec 0040 through the Python
reference client (tests/utils/mirror_client.py). The desktop viewer
(API::MirrorSession) is a GUI object and cannot be scripted here; the Python
MirrorClient stands in for it and asserts the same wire behaviour the C++
client depends on, so a protocol regression on the capture side is caught
without a second GUI.

MAINTAINER-RUN. Unlike the pure-codec suite (tests/unit/test_mirror_protocol.py,
agent-runnable), this file needs a live capture:

    * a Serial Studio instance with the API server enabled on 127.0.0.1:7777,
      a multi-group project loaded, and data flowing (so snapshots are live);
    * for the wrong-token / external-refusal legs, that instance provisioned
      with an auth token (headless: --api-token <hex> --api-external).

Configuration is environment-driven so the same file covers a loopback box and
a two-machine setup without editing:

    SS_MIRROR_HOST   capture host        (default 127.0.0.1)
    SS_MIRROR_PORT   capture API port    (default 7777)
    SS_MIRROR_TOKEN  auth token, if any  (default "": loopback pre-auth)

Every leg skips cleanly when its precondition is absent, so `pytest
--collect-only` and `python -m py_compile` are green with no instance, and a
partial setup runs the legs it can.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import os
import sys
import time
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]

# Imported by path, exactly as the pure-codec suite does: the mirror client has
# no third-party dependencies and the utils package __init__ pulls in numpy and
# pyserial, which the wire tests have no reason to require.
sys.path.insert(0, str(REPO_ROOT / "tests" / "utils"))

from mirror_client import (  # noqa: E402
    MIRROR_WIRE_VERSION,
    MirrorClient,
    MirrorCommandError,
    MirrorError,
    MirrorSnapshot,
    MirrorStructure,
)

pytestmark = [pytest.mark.integration, pytest.mark.mirror, pytest.mark.network]


# ----------------------------------------------------------------------
# Environment / reachability
# ----------------------------------------------------------------------

MIRROR_HOST = os.environ.get("SS_MIRROR_HOST", "127.0.0.1")
MIRROR_PORT = int(os.environ.get("SS_MIRROR_PORT", "7777"))
MIRROR_TOKEN = os.environ.get("SS_MIRROR_TOKEN", "")

# How long to wait for the first live snapshot on a healthy capture.
SNAPSHOT_TIMEOUT_S = 5.0


def _new_client(token: str = MIRROR_TOKEN, hz: int = 20) -> MirrorClient:
    return MirrorClient(host=MIRROR_HOST, port=MIRROR_PORT, token=token, hz=hz)


def _capture_reachable() -> bool:
    """True when a mirror-capable capture answers subscribe within the timeout."""
    try:
        client = _new_client()
        client.attach()
        client.close()
        return True
    except MirrorError:
        return False


@pytest.fixture(scope="module")
def require_capture():
    """Skip the whole module unless a live capture instance is reachable."""
    if not _capture_reachable():
        pytest.skip(
            f"no mirror-capable capture at {MIRROR_HOST}:{MIRROR_PORT} "
            "(start Serial Studio with the API server on and a project loaded)"
        )


@pytest.fixture
def attached(require_capture):
    """A subscribed client holding a verified structure; closed on teardown."""
    client = _new_client()
    client.attach()
    yield client
    client.close()


def _drain_for_snapshot(
    client: MirrorClient, timeout: float = SNAPSHOT_TIMEOUT_S
) -> bool:
    """Pumps until at least one snapshot lands or the timeout expires."""
    deadline = time.monotonic() + timeout
    baseline = client.stats.snapshots
    while time.monotonic() < deadline:
        client.pump(max_events=1, timeout=0.25)
        if client.stats.snapshots > baseline:
            return True
    return False


# ----------------------------------------------------------------------
# AC1 -- attach, structure, live values
# ----------------------------------------------------------------------


def test_attach_yields_a_verified_structure(attached):
    """The subscribe result is wire-compatible and a hashed structure adopts."""
    assert attached.info.get("wireVersion") == MIRROR_WIRE_VERSION
    assert isinstance(attached.structure, MirrorStructure)
    # The layout hash the codec recomputes must equal the announced one, or
    # _adopt would have raised: reaching here is the positional-safety proof.
    assert attached.structure.layout_hash
    if attached.structure.dataset_count == 0:
        pytest.skip("capture has an empty project loaded -- no datasets to mirror")


def test_live_values_flow_at_display_cadence(attached):
    """A capture producing data delivers snapshots the viewer marks live."""
    if not _drain_for_snapshot(attached):
        pytest.skip("capture reachable but idle -- no snapshots to assert liveness on")

    assert attached.stats.snapshots >= 1
    assert attached.stats.dropped_epoch == 0
    assert attached.stats.dropped_hash == 0
    assert attached.stats.dropped_length == 0
    assert attached.live


# ----------------------------------------------------------------------
# AC2 -- detach leaves the capture running; reattach works
# ----------------------------------------------------------------------


def test_detach_leaves_capture_running_and_reattach_recovers(require_capture):
    """Closing the viewer must not disturb the capture; a fresh attach recovers."""
    first = _new_client()
    first.attach()
    first_epoch = first.structure.epoch
    first.close()

    # The capture is untouched by our socket closing: a brand-new client sees a
    # structure again (same or a later epoch if the remote project changed).
    second = _new_client()
    second.attach()
    try:
        assert isinstance(second.structure, MirrorStructure)
        assert second.structure.epoch >= first_epoch
    finally:
        second.close()


# ----------------------------------------------------------------------
# AC3 -- two viewers agree, neither disturbs the other
# ----------------------------------------------------------------------


def test_two_viewers_agree_and_are_independent(require_capture):
    """Two simultaneous viewers hold the same epoch and dataset layout."""
    a = _new_client()
    b = _new_client()
    a.attach()
    b.attach()
    try:
        assert a.structure.epoch == b.structure.epoch
        assert a.structure.layout_hash == b.structure.layout_hash
        assert a.structure.dataset_ids == b.structure.dataset_ids

        # Closing one leaves the other fully functional.
        a.close()
        b.pump(max_events=1, timeout=1.0)
        assert isinstance(b.structure, MirrorStructure)
    finally:
        a.close()
        b.close()


# ----------------------------------------------------------------------
# AC4 -- staleness within a bounded time
# ----------------------------------------------------------------------


def test_stale_when_nothing_arrives_within_the_watchdog(attached):
    """With no pumping past the watchdog window, the link reads stale, not live."""
    if not _drain_for_snapshot(attached):
        pytest.skip("capture idle -- staleness bound is only meaningful once live")

    assert attached.live
    time.sleep(attached.watchdog_s + 0.25)
    # We deliberately stopped reading: the time-based staleness flag must trip.
    assert attached.stale
    assert not attached.live


# ----------------------------------------------------------------------
# AC8 -- auth refusal
# ----------------------------------------------------------------------


def test_wrong_token_is_refused(require_capture):
    """A bad token is refused; a viewer never streams unauthorized."""
    if not MIRROR_TOKEN:
        pytest.skip(
            "capture is loopback pre-authenticated; set SS_MIRROR_TOKEN to test refusal"
        )

    bad = _new_client(token="deadbeef" * 8)
    with pytest.raises(MirrorError):
        bad.attach()
    bad.close()


# ----------------------------------------------------------------------
# T6 regression -- an ordinary API client is unaffected by the mirror
# ----------------------------------------------------------------------


def test_plain_api_client_unaffected_while_a_viewer_is_attached(require_capture):
    """The mirror is additive: a non-mirror API client keys-sniffs past it."""
    try:
        from utils import SerialStudioClient
    except Exception:
        pytest.skip("utils package unavailable for the API regression leg")

    viewer = _new_client()
    viewer.attach()
    try:
        api = SerialStudioClient(host=MIRROR_HOST, port=MIRROR_PORT)
        api.connect()
        try:
            # A plain command still round-trips: the {"mirror":...} pushes on
            # other sockets do not corrupt this one's response framing.
            status = api.get_dashboard_status()
            assert isinstance(status, dict)
        finally:
            api.disconnect()
    finally:
        viewer.close()
