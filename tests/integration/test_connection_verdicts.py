"""Spec 0050 -- connection verdict unification.

Every connect attempt must end in exactly one observable outcome, "connecting" must always
resolve, cycling must leave no residue, and identical-settings re-application must be a
complete no-op. Requires the app up with the API server enabled (see tests/README.md).
"""

import base64
import functools
import socket
import threading
import time

import pytest

REFUSED_HOST = "127.0.0.1"
REFUSED_PORT = 9
VERDICT_BUDGET_S = 6.0


@functools.lru_cache(maxsize=1)
def refusal_is_observable() -> bool:
    """Whether this host answers a closed loopback port with an RST.

    Windows Firewall stealth mode (default on GitHub CI) drops the SYN instead,
    turning "refused" into the OS connect timeout -- the verdict budget cannot
    hold there (same guard as test_connection_diagnostics.py).
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


def _status(api_client) -> dict:
    return api_client.command("io.getStatus")


def _wait_settled(api_client, budget_s: float = VERDICT_BUDGET_S) -> dict:
    """Poll until linkState leaves 'connecting'; fail if it never resolves."""
    deadline = time.time() + budget_s
    while time.time() < deadline:
        st = _status(api_client)
        if st.get("linkState") != "connecting":
            return st
        time.sleep(0.2)

    pytest.fail(f"linkState stuck at 'connecting' beyond {budget_s:.1f}s")


def _connect_and_settle(api_client, budget_s: float = VERDICT_BUDGET_S) -> dict:
    """Issue io.connect and wait out the dial.

    Since spec 0075 the TCP dial is asynchronous like WebSocket, HTTP, BLE, Modbus and MQTT: the
    io.connect response says the attempt STARTED, and the verdict arrives through the status. A
    test that reads the response flag alone is testing the old synchronous dial.
    """
    api_client.command("io.connect", timeout=15.0)
    return _wait_settled(api_client, budget_s)


class _FeedServer:
    """Multi-accept local TCP server streaming CSV lines to every client."""

    def __init__(self, port: int = 0):
        self._srv = socket.socket()
        self._srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._srv.bind(("127.0.0.1", port))
        self._srv.listen(8)
        self.port = self._srv.getsockname()[1]
        self._running = True
        self._lock = threading.Lock()
        self._received = bytearray()
        threading.Thread(target=self._accept_loop, daemon=True).start()

    def _accept_loop(self):
        while self._running:
            try:
                conn, _ = self._srv.accept()
            except OSError:
                return
            threading.Thread(target=self._feed, args=(conn,), daemon=True).start()

    def _feed(self, conn):
        n = 0
        conn.settimeout(0.05)
        try:
            while self._running:
                conn.sendall(f"{n},{n % 100}\n".encode())
                n += 1
                try:
                    chunk = conn.recv(4096)
                    if chunk:
                        with self._lock:
                            self._received += chunk
                except (TimeoutError, BlockingIOError):
                    pass
        except OSError:
            pass

    def received(self) -> bytes:
        """Everything the clients have sent us so far."""
        with self._lock:
            return bytes(self._received)

    def stop(self):
        self._running = False
        self._srv.close()


@pytest.fixture
def feed_server():
    srv = _FeedServer()
    yield srv
    srv.stop()


@pytest.mark.network
def test_network_dead_port_settles_with_one_verdict(api_client, clean_state):
    """AC1: a refused TCP dial ends disconnected within the budget, never stuck."""
    if not refusal_is_observable():
        pytest.skip("this host swallows the RST for closed loopback ports")

    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host=REFUSED_HOST, port=REFUSED_PORT, socket_type="tcp"
    )
    time.sleep(0.3)

    st = _connect_and_settle(api_client)
    assert st.get("isConnected") is False
    assert st.get("linkState") == "idle"


@pytest.mark.network
def test_modbus_dead_port_settles_with_one_verdict(api_client, clean_state):
    """AC1/AC2: a refused Modbus TCP dial resolves; 'connecting' cannot persist."""
    if not refusal_is_observable():
        pytest.skip("this host swallows the RST for closed loopback ports")

    api_client.set_bus_type("modbus")
    time.sleep(0.3)
    api_client.command("io.modbus.setHost", {"host": REFUSED_HOST})
    api_client.command("io.modbus.setPort", {"port": REFUSED_PORT})
    time.sleep(0.3)

    api_client.command("io.connect", timeout=15.0)
    st = _wait_settled(api_client)
    assert st.get("isConnected") is False
    assert st.get("linkState") == "idle"


@pytest.mark.network
def test_network_cycle_20x_leaves_no_residue(api_client, clean_state, feed_server):
    """AC7: 20 connect/disconnect cycles end in a state identical to fresh."""
    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host="127.0.0.1", port=feed_server.port, socket_type="tcp"
    )
    time.sleep(0.3)

    fresh = _status(api_client)
    for _ in range(20):
        st = _connect_and_settle(api_client)
        assert st.get("isConnected") is True
        api_client.command("io.disconnect")

    final = _wait_settled(api_client)
    assert final.get("isConnected") == fresh.get("isConnected") is False
    assert final.get("linkState") == "idle"


@pytest.mark.network
def test_identical_settings_reapply_is_noop(api_client, clean_state, feed_server):
    """AC5: re-applying the identical endpoint never bounces a live connection."""
    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host="localhost", port=feed_server.port, socket_type="tcp"
    )
    time.sleep(0.5)

    st = _connect_and_settle(api_client)
    assert st.get("isConnected") is True

    for _ in range(10):
        api_client.configure_network(
            host="localhost", port=feed_server.port, socket_type="tcp"
        )
        time.sleep(0.1)

    time.sleep(1.0)
    st = _status(api_client)
    assert st.get("isConnected") is True, "identical-settings echo bounced the link"
    api_client.command("io.disconnect")


@pytest.mark.network
def test_connect_then_write_reaches_the_peer(api_client, clean_state, feed_server):
    """A script's io.connect() + writeData() sequence still lands on the wire.

    The dial no longer blocks inside open(), so bytes written while it is in flight are held by
    the driver and flushed the moment the socket connects (spec 0050's promise, kept across the
    spec-0075 async dial).
    """
    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host="127.0.0.1", port=feed_server.port, socket_type="tcp"
    )
    time.sleep(0.3)

    api_client.command("io.connect", timeout=15.0)
    api_client.command("io.writeData", {"data": base64.b64encode(b"PING\n").decode()})

    st = _wait_settled(api_client)
    assert st.get("isConnected") is True

    deadline = time.time() + 5.0
    while time.time() < deadline and b"PING" not in feed_server.received():
        time.sleep(0.1)

    assert (
        b"PING" in feed_server.received()
    ), "the write issued during the dial was lost"
    api_client.command("io.disconnect")
