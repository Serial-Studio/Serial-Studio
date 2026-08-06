"""
Connection Diagnostics Integration Tests (spec 0035)

Drives the diagnostics.* API against a running Serial Studio and asserts the
three distinct reachability verdicts (name did not resolve, connection
refused, connection timed out), that a reachable endpoint is probed without
carrying a single application byte, that diagnostics.run acknowledges
immediately and diagnostics.status settles inside the declared worst case,
and that an automatic run stays scoped to the failing bus.

The reachability probes read the network source's configured host and port,
so every test configures the network driver first. The blackhole address is
an RFC 1918 host that drops SYNs, which is what makes the timeout verdict
different from a refusal.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import functools
import socket
import threading
import time

import pytest

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

NETWORK_CHECKER = "diagnostics.network"
OTHER_CHECKERS = ("diagnostics.serial", "diagnostics.bluetooth", "diagnostics.audio")

REACHABILITY_CODES = ("host-not-resolved", "connection-refused", "endpoint-timed-out")

# The .invalid TLD is reserved by RFC 2606 and can never resolve.
UNRESOLVABLE_HOST = "serial-studio-diagnostics.invalid"

# RFC 1918 address that is routed nowhere on a normal LAN: SYNs are dropped
# rather than refused, which is exactly the timeout case.
BLACKHOLE_HOST = "10.255.255.1"
BLACKHOLE_PORT = 9

# Discard port on loopback: resolves instantly and refuses the connection.
REFUSED_HOST = "127.0.0.1"
REFUSED_PORT = 9


@functools.lru_cache(maxsize=1)
def refusal_is_observable() -> bool:
    """Whether this host answers a closed loopback port with an RST.

    Windows Firewall stealth mode (on by default, including on GitHub CI
    runners) silently drops the SYN instead, so a refusal is indistinguishable
    from a timeout and the connection-refused verdict cannot be produced.
    """
    probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    probe.settimeout(2.0)
    try:
        probe.connect((REFUSED_HOST, REFUSED_PORT))
        return False
    except ConnectionRefusedError:
        return True
    except OSError:
        return False
    finally:
        probe.close()


# Slack over the runner's declared worst case, for a loaded CI runner.
SETTLE_SLACK_S = 4.0


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


class CountingListener:
    """A loopback TCP listener that records connections and bytes received."""

    def __init__(self):
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind(("127.0.0.1", 0))
        self._server.listen(4)
        self._server.settimeout(0.25)
        self._running = True
        self.connections = 0
        self.bytes_received = 0
        self.port = self._server.getsockname()[1]
        self._thread = threading.Thread(target=self._accept_loop, daemon=True)
        self._thread.start()

    def _accept_loop(self) -> None:
        while self._running:
            try:
                client, _ = self._server.accept()
            except (socket.timeout, OSError):
                continue

            self.connections += 1
            client.settimeout(0.5)
            try:
                data = client.recv(4096)
                self.bytes_received += len(data)
            except (socket.timeout, OSError):
                pass
            finally:
                try:
                    client.close()
                except OSError:
                    pass

    def close(self) -> None:
        self._running = False
        self._thread.join(timeout=2.0)
        try:
            self._server.close()
        except OSError:
            pass


@pytest.fixture
def counting_listener():
    listener = CountingListener()

    yield listener

    listener.close()


def _run(api_client, bus: str = None) -> dict:
    """Start a diagnostics run, optionally scoped to one bus."""
    return api_client.command("diagnostics.run", {"bus": bus} if bus else None)


def _status(api_client) -> dict:
    return api_client.command("diagnostics.status")


def _findings(api_client, checker_id: str) -> list:
    result = api_client.command("problems.list", {"checkerId": checker_id})
    return result.get("findings", [])


def _codes(findings: list) -> list:
    return [f.get("code") for f in findings]


def _wait_until_settled(api_client, budget_s: float) -> float:
    """Poll diagnostics.status until no probe is in flight; return the wait."""
    started = time.time()
    deadline = started + budget_s
    while time.time() < deadline:
        if not _status(api_client)["running"]:
            return time.time() - started
        time.sleep(0.1)

    pytest.fail(f"Diagnostics still running after {budget_s:.1f} s")


def _configure_and_probe(api_client, host: str, port: int) -> dict:
    """Point the network source at an endpoint and run the network checks."""
    api_client.configure_network(host=host, port=port, socket_type="tcp")
    time.sleep(0.3)

    started = _run(api_client, bus="network")
    budget = started["estimatedMs"] / 1000.0 + SETTLE_SLACK_S
    _wait_until_settled(api_client, budget)
    return started


def _reachability_finding(api_client, code: str) -> dict:
    findings = _findings(api_client, NETWORK_CHECKER)
    matches = [f for f in findings if f.get("code") == code]
    assert len(matches) == 1, f"Expected one '{code}' finding, got {_codes(findings)}"
    return matches[0]


# ---------------------------------------------------------------------------
# AC5 -- three distinct reachability verdicts
# ---------------------------------------------------------------------------


@pytest.mark.network
def test_unresolvable_host_reports_a_name_resolution_failure(api_client, clean_state):
    """A host name that cannot resolve is reported as such, not as a timeout."""
    _configure_and_probe(api_client, UNRESOLVABLE_HOST, 9000)

    finding = _reachability_finding(api_client, "host-not-resolved")
    assert finding["severity"] == "error"
    assert finding["checkerId"] == NETWORK_CHECKER
    assert UNRESOLVABLE_HOST in finding["explanation"]
    assert finding["remedy"]


@pytest.mark.network
def test_closed_port_reports_a_refused_connection(api_client, clean_state):
    """A resolvable host with nothing listening is reported as a refusal."""
    if not refusal_is_observable():
        pytest.skip("this host swallows the RST for closed loopback ports")

    _configure_and_probe(api_client, REFUSED_HOST, REFUSED_PORT)

    finding = _reachability_finding(api_client, "connection-refused")
    assert finding["severity"] == "error"
    assert f"{REFUSED_HOST}:{REFUSED_PORT}" in finding["explanation"]
    assert finding["remedy"]


@pytest.mark.network
@pytest.mark.slow
def test_blackholed_address_times_out_inside_the_declared_budget(
    api_client, clean_state
):
    """A dropped SYN produces the timeout verdict within the declared budget."""
    api_client.configure_network(
        host=BLACKHOLE_HOST, port=BLACKHOLE_PORT, socket_type="tcp"
    )
    time.sleep(0.3)

    started = _run(api_client, bus="network")
    budget = started["estimatedMs"] / 1000.0
    assert budget > 0, "A configured endpoint must declare a probing budget"

    elapsed = _wait_until_settled(api_client, budget + SETTLE_SLACK_S)
    assert elapsed <= budget + SETTLE_SLACK_S

    finding = _reachability_finding(api_client, "endpoint-timed-out")
    assert finding["severity"] == "error"
    assert BLACKHOLE_HOST in finding["explanation"]
    assert finding["remedy"]


@pytest.mark.network
@pytest.mark.slow
def test_the_three_verdicts_carry_three_different_remedies(api_client, clean_state):
    """Each reachability failure states a different fix, not one generic line."""
    remedies = {}

    legs = [
        (UNRESOLVABLE_HOST, 9000, "host-not-resolved"),
        (BLACKHOLE_HOST, BLACKHOLE_PORT, "endpoint-timed-out"),
    ]
    if refusal_is_observable():
        legs.insert(1, (REFUSED_HOST, REFUSED_PORT, "connection-refused"))

    for host, port, code in legs:
        _configure_and_probe(api_client, host, port)
        remedies[code] = _reachability_finding(api_client, code)["remedy"]

    assert len(set(remedies.values())) == len(
        legs
    ), f"Remedies are not distinct: {remedies}"


# ---------------------------------------------------------------------------
# AC6 -- a reachable endpoint is probed without sending anything
# ---------------------------------------------------------------------------


@pytest.mark.network
def test_reachable_endpoint_is_probed_without_application_bytes(
    api_client, clean_state, counting_listener
):
    """The probe connects and closes: no finding, no byte on the wire."""
    _configure_and_probe(api_client, "127.0.0.1", counting_listener.port)

    codes = _codes(_findings(api_client, NETWORK_CHECKER))
    for code in REACHABILITY_CODES:
        assert code not in codes, f"A reachable endpoint reported '{code}'"

    time.sleep(0.5)
    assert counting_listener.connections >= 1, "The probe never connected"
    assert counting_listener.bytes_received == 0, "The probe sent application bytes"


# ---------------------------------------------------------------------------
# AC8 -- the API surface: ack, poll, then read through problems.list
# ---------------------------------------------------------------------------


@pytest.mark.network
def test_run_acknowledges_immediately_with_the_instant_results(api_client, clean_state):
    """diagnostics.run returns the completed instant results without blocking."""
    api_client.configure_network(
        host=BLACKHOLE_HOST, port=BLACKHOLE_PORT, socket_type="tcp"
    )
    time.sleep(0.3)

    started_at = time.time()
    result = _run(api_client)
    elapsed = time.time() - started_at

    assert elapsed < 1.0, f"diagnostics.run blocked for {elapsed:.2f} s"
    assert result["started"] is True
    for key in ("running", "buses", "probing", "instant", "estimatedMs", "hint"):
        assert key in result, f"diagnostics.run result is missing '{key}'"

    assert "serial" in result["buses"]
    assert "network" in result["buses"]
    assert "network" in result["probing"]

    for row in result["instant"]:
        for key in ("bus", "verdict", "code", "title", "explanation", "remedy"):
            assert key in row, f"Instant result is missing '{key}'"

    _wait_until_settled(api_client, result["estimatedMs"] / 1000.0 + SETTLE_SLACK_S)


@pytest.mark.network
def test_status_reports_completion_and_findings_reach_problems_list(
    api_client, clean_state
):
    """diagnostics.status settles, and problems.list then serves the findings."""
    _configure_and_probe(api_client, REFUSED_HOST, REFUSED_PORT)

    status = _status(api_client)
    assert status["running"] is False
    assert status["lastRun"]
    assert "network" in status["buses"]
    assert set(status["counts"]) == {"info", "warning", "failure"}

    findings = _findings(api_client, NETWORK_CHECKER)
    assert findings, "The diagnostics findings never reached the problem center"
    assert all(f["checkerId"] == NETWORK_CHECKER for f in findings)
    assert all(f["title"] for f in findings)


@pytest.mark.network
def test_unknown_bus_slug_is_rejected(api_client, clean_state):
    """An unknown scope names the valid values instead of running everything."""
    with pytest.raises(Exception) as error:
        _run(api_client, bus="teleporter")

    message = str(error.value)
    assert "teleporter" in message
    assert "serial" in message and "network" in message


# ---------------------------------------------------------------------------
# AC9 -- a failed open runs the failing bus only, and is rate limited
# ---------------------------------------------------------------------------


@pytest.mark.network
@pytest.mark.slow
def test_failed_open_diagnoses_only_the_failing_bus(api_client, clean_state):
    """A refused network open publishes network findings and probes nothing else."""
    api_client.configure_network(
        host=REFUSED_HOST, port=REFUSED_PORT, socket_type="tcp"
    )
    time.sleep(0.3)

    baseline = {c: len(_findings(api_client, c)) for c in OTHER_CHECKERS}

    try:
        api_client.connect_device()
    except Exception:
        pass

    _wait_until_settled(api_client, 10.0)

    assert _findings(api_client, NETWORK_CHECKER), "The failed open produced no finding"
    for checker, count in baseline.items():
        assert (
            len(_findings(api_client, checker)) == count
        ), f"A network failure changed {checker} findings"

    try:
        api_client.disconnect_device()
    except Exception:
        pass


@pytest.mark.network
@pytest.mark.slow
def test_repeated_failures_do_not_accumulate_findings(api_client, clean_state):
    """
    A second failure inside the rate-limit window re-reports, never duplicates.

    The suppressed probing run is internal state; what is observable from the
    API is that the standing findings stay identical, which is the property
    the rate limit exists to protect.
    """
    api_client.configure_network(
        host=REFUSED_HOST, port=REFUSED_PORT, socket_type="tcp"
    )
    time.sleep(0.3)

    try:
        api_client.connect_device()
    except Exception:
        pass

    _wait_until_settled(api_client, 10.0)
    first = _codes(_findings(api_client, NETWORK_CHECKER))
    assert first, "The first failed open produced no finding"

    try:
        api_client.disconnect_device()
    except Exception:
        pass
    try:
        api_client.connect_device()
    except Exception:
        pass

    _wait_until_settled(api_client, 10.0)
    second = _codes(_findings(api_client, NETWORK_CHECKER))

    assert second == first, "Repeated failures accumulated findings"

    try:
        api_client.disconnect_device()
    except Exception:
        pass
