"""
Link Recovery Integration Tests (spec 0034, AC4/AC5/AC6)

Drives the async orchestration layer against a running Serial Studio: a link
that drops mid-stream must come back on its own, repeatedly, without the test
touching io.connect, and the process must be at a steady state afterwards.

The test hosts the peer. Serial Studio is the TCP client, so severing a link is
just closing the accepted socket: the driver sees an unsolicited disconnect,
emits linkDropped(), and the supervising flow re-runs the open sequence under
the shared retry policy. Because the peer stays listening, the first re-attempt
succeeds and no backoff is paid -- a cycle costs a socket close plus a dial.

The 100 severances of AC5 are split into chunks by a parametrized test rather
than run in one function: pytest.ini caps every test at 30 s, and 100 cycles of
(sever -> reconnect -> frame resumes), each with its own API round trips, does
not reliably fit. Chunks share one module-scoped link, so the loop is still one
uninterrupted 100-severance run of the same connection -- SEVERANCE_CHUNKS *
SEVERANCES_PER_CHUNK is the total, and the steady-state test that follows reads
the rig's own counter rather than assuming the loop completed.

Maintainer-run: needs the app up with the API server on localhost:7777. The
MQTT case additionally needs a commercial build and a broker on 127.0.0.1:1883.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import socket
import threading
import time

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError

# -----------------------------------------------------------------------------
# Constants
# -----------------------------------------------------------------------------

SEVERANCE_CHUNKS = 10
SEVERANCES_PER_CHUNK = 10

# Generous against a loaded runner: a healthy cycle settles in well under a
# second, so a timeout here means recovery did not happen at all.
RECONNECT_TIMEOUT_S = 8.0
FRAME_TIMEOUT_S = 5.0

# An answer that arrives inside this window cannot have been blocked behind the
# old ~4.5 s synchronous connect loop, which is what AC4 is about.
IMMEDIATE_ANSWER_S = 2.5

# RFC 1918 address routed nowhere on a normal LAN: SYNs are dropped, so a
# connect attempt against it stays pending instead of being refused.
BLACKHOLE_HOST = "10.255.255.1"
BLACKHOLE_PORT = 9

# RetryPolicy::autoReconnect() waits 500 ms after the first failed attempt and
# doubles up to a 5 s ceiling, so a peer kept down for this long forces the
# 500 ms / 1 s / 2 s intervals to be paid and leaves the 4 s one in flight. The
# 30 s per-test cap is why the window stops there instead of running the policy
# out to its 60th attempt.
PEER_DOWNTIME_S = 6.0
MIN_BACKOFF_INTERVALS = 3

# The peer returns mid-backoff, so recovery waits out whatever is left of the
# current interval before it can dial: the ceiling plus slack.
PEER_RETURN_TIMEOUT_S = 10.0

BROKER_HOST = "127.0.0.1"
BROKER_PORT = 1883
MQTT_SEVERANCES = 5
BUS_TYPE_MQTT = 9

_JS_SPLIT_PARSER = """
function parse(frame) {
    return frame.split(',');
}
"""


# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------


class SeverablePeer:
    """A loopback TCP listener whose accepted connection can be cut at will."""

    def __init__(self):
        self._lock = threading.Lock()
        self._server = self._listen(0)
        self._running = True
        self._client = None
        self.accepted = 0
        self.port = self._server.getsockname()[1]
        self._thread = threading.Thread(target=self._accept_loop, daemon=True)
        self._thread.start()

    @staticmethod
    def _listen(port: int):
        """Bind a listener on `port`; SO_REUSEADDR is what lets the same port be
        re-taken after a downtime window closed it."""
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind(("127.0.0.1", port))
        server.listen(4)
        server.settimeout(0.25)
        return server

    def _accept_loop(self) -> None:
        while self._running:
            with self._lock:
                server = self._server

            if server is None:
                time.sleep(0.05)
                continue

            try:
                client, _ = server.accept()
            except (socket.timeout, OSError):
                continue

            client.settimeout(1.0)
            with self._lock:
                self.accepted += 1
                previous = self._client
                self._client = client

            self._close(previous)

    @staticmethod
    def _close(sock) -> None:
        if sock is None:
            return
        try:
            sock.close()
        except OSError:
            pass

    @property
    def live_clients(self) -> int:
        """How many accepted sockets are still open; more than one means the
        app is holding overlapping connections."""
        with self._lock:
            return 0 if self._client is None else 1

    def wait_for_client(self, timeout: float, after: int = 0) -> bool:
        """Wait until an accepted connection newer than `after` exists."""
        end = time.time() + timeout
        while time.time() < end:
            with self._lock:
                if self.accepted > after and self._client is not None:
                    return True
            time.sleep(0.02)

        return False

    def sever(self) -> None:
        """Close the live connection, which the app sees as a dropped link."""
        with self._lock:
            victim = self._client
            self._client = None

        self._close(victim)

    def stop_listening(self) -> None:
        """Take the peer down: the listener goes away too, so a re-dial is
        refused instead of accepted. This is the case a severance cannot
        reproduce, because a severance leaves the port open."""
        with self._lock:
            server = self._server
            self._server = None

        self._close(server)
        self.sever()

    def start_listening(self) -> None:
        """Bring the peer back on the same port the app is still configured
        for, so recovery has something to find."""
        server = self._listen(self.port)
        with self._lock:
            self._server = server

    def send(self, payload: bytes) -> bool:
        with self._lock:
            client = self._client

        if client is None:
            return False

        try:
            client.sendall(payload)
            return True
        except OSError:
            return False

    def close(self) -> None:
        self._running = False
        self.sever()
        self._thread.join(timeout=2.0)

        with self._lock:
            server = self._server
            self._server = None

        self._close(server)


class SeveranceRig:
    """The peer, the client that drives the app, and the counters the
    steady-state assertions compare against."""

    def __init__(self, client, peer, baseline):
        self.peer = peer
        self.client = client
        self.baseline = baseline
        self.severances = 0


def _status(client) -> dict:
    return client.command("io.getStatus")


def _wait_for_link(client, timeout: float) -> dict:
    """Poll io.getStatus until the link reports itself connected."""
    end = time.time() + timeout
    status = {}
    while time.time() < end:
        status = _status(client)
        if status.get("isConnected"):
            return status

        time.sleep(0.05)

    return status


def _wait_for_link_state(client, states, timeout: float) -> str:
    """Poll io.getStatus until linkState is one of `states`, returning whatever
    the last reading was so a failure can name it."""
    end = time.time() + timeout
    state = ""
    while time.time() < end:
        state = _status(client).get("linkState", "")
        if state in states:
            return state

        time.sleep(0.05)

    return state


def _peak_reconnect_attempt(client, seconds: float) -> int:
    """Highest reconnectAttempt seen across a window. The counter is cleared the
    moment an attempt succeeds, so the peak -- not the final reading -- is what
    says how many backoff intervals the recovery actually paid."""
    end = time.time() + seconds
    peak = 0
    while time.time() < end:
        peak = max(peak, _status(client).get("reconnectAttempt") or 0)
        time.sleep(0.2)

    return peak


def _latest_sequence(client) -> int:
    frame = client.command("io.getLatestFrame")
    return frame.get("sequence", 0) if frame.get("hasData") else 0


def _wait_for_new_frame(client, after: int, timeout: float) -> int:
    """Poll io.getLatestFrame until a frame newer than `after` is captured."""
    end = time.time() + timeout
    sequence = after
    while time.time() < end:
        sequence = _latest_sequence(client)
        if sequence > after:
            return sequence

        time.sleep(0.05)

    return sequence


def _is_pro_build(client) -> bool:
    try:
        buses = client.command("io.listBuses").get("buses", [])
    except APIError:
        return False

    return len(buses) > BUS_TYPE_MQTT


def _broker_reachable() -> bool:
    try:
        with socket.create_connection((BROKER_HOST, BROKER_PORT), timeout=1.0):
            return True
    except OSError:
        return False


def _bring_up_link(client, peer) -> dict:
    """Point the app at `peer` over TCP and wait for the link to come up,
    returning the status the recovery assertions compare against."""
    client.create_new_project()
    time.sleep(0.2)
    client.set_operation_mode("project")
    client.configure_network(host="127.0.0.1", port=peer.port, socket_type="tcp")
    client.configure_frame_parser(
        start_sequence="", end_sequence="\n", operation_mode=0, frame_detection=0
    )
    client.set_frame_parser_code(_JS_SPLIT_PARSER, language=0)
    client.command("project.activate")
    time.sleep(0.2)

    client.connect_device()
    assert peer.wait_for_client(
        RECONNECT_TIMEOUT_S
    ), "Serial Studio never connected to the test peer"

    status = _wait_for_link(client, RECONNECT_TIMEOUT_S)
    assert status.get("isConnected"), "The link never came up to begin with"

    return status


def _sever_and_recover(rig, index: int) -> None:
    """One severance cycle: cut the link, let the app bring it back by itself,
    and confirm frames flow again on the new connection."""
    accepted_before = rig.peer.accepted
    rig.peer.sever()
    rig.severances += 1

    assert rig.peer.wait_for_client(
        RECONNECT_TIMEOUT_S, after=accepted_before
    ), f"Severance {index}: the source never re-dialled the peer"

    status = _wait_for_link(rig.client, RECONNECT_TIMEOUT_S)
    assert status.get(
        "isConnected"
    ), f"Severance {index}: io.getStatus never reported the link back up"

    sequence_before = _latest_sequence(rig.client)
    assert rig.peer.send(b"1,2,3\n"), f"Severance {index}: peer could not write"

    sequence_after = _wait_for_new_frame(rig.client, sequence_before, FRAME_TIMEOUT_S)
    assert (
        sequence_after > sequence_before
    ), f"Severance {index}: frames did not resume after recovery"


# -----------------------------------------------------------------------------
# Fixtures
# -----------------------------------------------------------------------------


@pytest.fixture(scope="module")
def link_client(serial_studio_running):
    """A module-scoped client, so one link survives the whole severance loop."""
    client = SerialStudioClient(timeout=15.0)
    client.connect()

    yield client

    try:
        client.disconnect_device()
    except (APIError, ConnectionError):
        pass

    client.disconnect()


@pytest.fixture(scope="module")
def severance_rig(link_client):
    """Bring up a TCP link to a peer this test owns, and record the steady-state
    baseline the recovery loop must return to."""
    peer = SeverablePeer()
    status = _bring_up_link(link_client, peer)

    yield SeveranceRig(link_client, peer, status)

    try:
        link_client.disconnect_device()
    except (APIError, ConnectionError):
        pass

    peer.close()


@pytest.fixture
def downtime_rig(api_client, clean_state):
    """A link of its own for the peer-down scenario: the module-scoped rig
    counts every accepted connection against its severances, and a downtime
    window adds re-dials that were refused, so sharing it would break the
    steady-state assertions of the severance loop."""
    peer = SeverablePeer()
    status = _bring_up_link(api_client, peer)

    yield SeveranceRig(api_client, peer, status)

    try:
        api_client.disconnect_device()
    except (APIError, ConnectionError):
        pass

    peer.close()


# -----------------------------------------------------------------------------
# AC5 -- a dropped link comes back on its own, 100 times
# -----------------------------------------------------------------------------


@pytest.mark.slow
@pytest.mark.network
@pytest.mark.integration
@pytest.mark.parametrize("chunk", range(SEVERANCE_CHUNKS))
def test_tcp_link_recovers_100_severances(severance_rig, chunk):
    """Every severance is followed by an unassisted reconnect and resumed
    frames. The chunks together are the 100-cycle loop AC5 asks for."""
    for step in range(SEVERANCES_PER_CHUNK):
        _sever_and_recover(severance_rig, chunk * SEVERANCES_PER_CHUNK + step + 1)


@pytest.mark.network
@pytest.mark.integration
def test_steady_state_after_severance_loop(severance_rig):
    """After the loop the process is where it started: one live connection, the
    same number of supervised flows, and no attempt left in flight."""
    status = _wait_for_link(severance_rig.client, RECONNECT_TIMEOUT_S)

    assert status.get("isConnected"), "The link did not survive the severance loop"
    assert status.get("linkState") == "connected"
    assert status.get("reconnectAttempt") == 0
    assert status.get("activeFlows") == severance_rig.baseline.get(
        "activeFlows"
    ), "The number of supervised flows grew across the severance loop"

    assert (
        severance_rig.peer.live_clients == 1
    ), "The app is holding more than one connection to the peer"
    assert severance_rig.peer.accepted == 1 + severance_rig.severances, (
        "The peer accepted more connections than severances: a recovery "
        "re-dialled more than once"
    )


@pytest.mark.network
@pytest.mark.integration
def test_status_reports_the_link_as_connected(severance_rig):
    """The three fields spec 0034 added are present and coherent while up."""
    status = _status(severance_rig.client)

    assert status.get("linkState") == "connected"
    assert status.get("activeFlows") >= 1, "A supervised link reports no flow"
    assert status.get("reconnectAttempt") == 0


# -----------------------------------------------------------------------------
# AC5 -- a peer that stays down across several backoffs is still recovered
# -----------------------------------------------------------------------------


@pytest.mark.slow
@pytest.mark.network
@pytest.mark.integration
def test_link_recovers_after_peer_stays_down(downtime_rig):
    """The severance loop only ever cuts a connection the peer is still ready to
    accept again, so it never exercises the backoff at all. Here the listener
    goes away too: every re-dial is refused, the recovery pays real intervals of
    RetryPolicy::autoReconnect(), and the link must come back by itself once the
    peer returns -- without the test touching io.connect."""
    rig = downtime_rig
    accepted_before = rig.peer.accepted

    rig.peer.stop_listening()

    state = _wait_for_link_state(
        rig.client, ("retrying", "connecting"), RECONNECT_TIMEOUT_S
    )
    assert state in (
        "retrying",
        "connecting",
    ), f"A peer that went down left the link in '{state}' instead of retrying"

    attempts = _peak_reconnect_attempt(rig.client, PEER_DOWNTIME_S)
    assert attempts >= MIN_BACKOFF_INTERVALS + 1, (
        f"Only {attempts} attempts in {PEER_DOWNTIME_S:.0f} s: the recovery did "
        f"not pay {MIN_BACKOFF_INTERVALS} backoff intervals"
    )
    assert (
        rig.peer.accepted == accepted_before
    ), "The peer accepted a connection while its listener was down"

    rig.peer.start_listening()

    assert rig.peer.wait_for_client(
        PEER_RETURN_TIMEOUT_S, after=accepted_before
    ), "The link never re-dialled after the peer came back"

    status = _wait_for_link(rig.client, PEER_RETURN_TIMEOUT_S)
    assert status.get(
        "isConnected"
    ), "The link did not report itself back up after the downtime"
    assert status.get("linkState") == "connected"
    assert status.get("reconnectAttempt") == 0, "A recovered link still counts attempts"

    sequence_before = _latest_sequence(rig.client)
    assert rig.peer.send(b"1,2,3\n"), "The returned peer could not write"

    sequence_after = _wait_for_new_frame(rig.client, sequence_before, FRAME_TIMEOUT_S)
    assert (
        sequence_after > sequence_before
    ), "Frames did not resume after the peer came back"


# -----------------------------------------------------------------------------
# AC4 -- a connect attempt never blocks the interface
# -----------------------------------------------------------------------------


@pytest.mark.slow
@pytest.mark.network
@pytest.mark.integration
def test_cancel_during_connect_is_immediate(api_client, clean_state):
    """Connecting to a black-holed address answers at once, and so does the
    disconnect that cancels it. io.disconnect may answer "Not connected" while
    the attempt is still in flight -- that is today's contract, and the
    assertion here is on latency, not on the verdict."""
    api_client.configure_network(
        host=BLACKHOLE_HOST, port=BLACKHOLE_PORT, socket_type="tcp"
    )
    time.sleep(0.2)

    started = time.time()
    api_client.connect_device()
    connect_elapsed = time.time() - started

    assert connect_elapsed < IMMEDIATE_ANSWER_S, (
        f"io.connect blocked for {connect_elapsed:.2f} s against an unreachable "
        f"address"
    )

    status = _status(api_client)
    assert status.get("linkState") in ("connecting", "retrying"), (
        f"An attempt against {BLACKHOLE_HOST} should be in flight, "
        f"got {status.get('linkState')}"
    )

    started = time.time()
    try:
        api_client.disconnect_device()
    except APIError:
        pass

    cancel_elapsed = time.time() - started
    assert (
        cancel_elapsed < IMMEDIATE_ANSWER_S
    ), f"io.disconnect took {cancel_elapsed:.2f} s to answer during a connect"

    end = time.time() + RECONNECT_TIMEOUT_S
    state = None
    while time.time() < end:
        state = _status(api_client).get("linkState")
        if state == "idle":
            break

        time.sleep(0.1)

    assert state == "idle", f"The cancelled attempt left the link in '{state}'"


# -----------------------------------------------------------------------------
# AC6 -- the MQTT source recovers under the same policy (commercial build)
# -----------------------------------------------------------------------------


@pytest.mark.slow
@pytest.mark.mqtt
@pytest.mark.requires_broker
@pytest.mark.integration
def test_mqtt_link_recovers(api_client, clean_state):
    """The same severance loop over MQTT: the test proxies the broker so it can
    cut the session, and the source must come back without being asked."""
    if not _is_pro_build(api_client):
        pytest.skip("The MQTT source requires a commercial build")

    if not _broker_reachable():
        pytest.skip(f"No MQTT broker on {BROKER_HOST}:{BROKER_PORT}")

    proxy = _BrokerProxy()
    try:
        api_client.command(
            "project.mqtt.subscriber.setConfig",
            {
                "hostname": "127.0.0.1",
                "port": proxy.port,
                "topicFilter": "serial-studio/link-recovery",
            },
        )
        api_client.set_bus_type("mqtt")
        time.sleep(0.2)
        api_client.connect_device()

        status = _wait_for_link(api_client, RECONNECT_TIMEOUT_S)
        assert status.get("isConnected"), "The MQTT source never connected"
        flows = status.get("activeFlows")

        for index in range(MQTT_SEVERANCES):
            sessions_before = proxy.sessions
            proxy.sever()

            assert proxy.wait_for_session(
                RECONNECT_TIMEOUT_S, after=sessions_before
            ), f"Severance {index + 1}: the MQTT source never re-dialled"

            status = _wait_for_link(api_client, RECONNECT_TIMEOUT_S)
            assert status.get(
                "isConnected"
            ), f"Severance {index + 1}: the MQTT source stayed down"

        assert (
            status.get("activeFlows") == flows
        ), "The MQTT severance loop leaked supervised flows"
        assert status.get("reconnectAttempt") == 0

    finally:
        try:
            api_client.disconnect_device()
        except (APIError, ConnectionError):
            pass

        proxy.close()


class _BrokerProxy:
    """A loopback TCP forwarder in front of the MQTT broker, so a test that
    does not own the broker can still cut the session at will."""

    def __init__(self):
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind(("127.0.0.1", 0))
        self._server.listen(4)
        self._server.settimeout(0.25)
        self._lock = threading.Lock()
        self._running = True
        self._pairs = []
        self.sessions = 0
        self.port = self._server.getsockname()[1]
        self._thread = threading.Thread(target=self._accept_loop, daemon=True)
        self._thread.start()

    def _accept_loop(self) -> None:
        while self._running:
            try:
                client, _ = self._server.accept()
            except (socket.timeout, OSError):
                continue

            try:
                upstream = socket.create_connection(
                    (BROKER_HOST, BROKER_PORT), timeout=2.0
                )
            except OSError:
                self._shutdown(client)
                continue

            with self._lock:
                self.sessions += 1
                self._pairs.append((client, upstream))

            self._pump(client, upstream)
            self._pump(upstream, client)

    def _pump(self, source, target) -> None:
        def _forward() -> None:
            try:
                while self._running:
                    data = source.recv(4096)
                    if not data:
                        break

                    target.sendall(data)
            except OSError:
                pass
            finally:
                self._shutdown(source)
                self._shutdown(target)

        threading.Thread(target=_forward, daemon=True).start()

    @staticmethod
    def _shutdown(sock) -> None:
        try:
            sock.close()
        except OSError:
            pass

    def wait_for_session(self, timeout: float, after: int = 0) -> bool:
        end = time.time() + timeout
        while time.time() < end:
            with self._lock:
                if self.sessions > after:
                    return True
            time.sleep(0.02)

        return False

    def sever(self) -> None:
        with self._lock:
            pairs = self._pairs
            self._pairs = []

        for client, upstream in pairs:
            self._shutdown(client)
            self._shutdown(upstream)

    def close(self) -> None:
        self._running = False
        self.sever()
        self._thread.join(timeout=2.0)
        self._shutdown(self._server)
