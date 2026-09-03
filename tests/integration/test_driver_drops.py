"""Spec 0075 -- an established link that drops must reach the UI.

Every driver family loses its link differently (a peer that closes, an adapter that vanishes,
a process that exits), but the observable contract is one: within a few seconds the app reports
disconnected, the session is still usable, and a reconnect works. A drop that leaves the toolbar
"connected" on a dead link is the defect these cover (findings D1, D5, D10, D15).

Requires the app up with the API server enabled (see tests/README.md).
"""

import os
import shutil
import socket
import subprocess
import sys
import threading
import time

import pytest

DROP_BUDGET_S = 8.0
SETTLE_BUDGET_S = 8.0


def _status(api_client) -> dict:
    return api_client.command("io.getStatus")


def _wait_state(api_client, predicate, budget_s: float, what: str) -> dict:
    """Poll io.getStatus until predicate holds; fail with the last state if it never does."""
    deadline = time.time() + budget_s
    last = {}
    while time.time() < deadline:
        last = _status(api_client)
        if predicate(last):
            return last
        time.sleep(0.2)

    pytest.fail(f"{what} did not happen within {budget_s:.1f}s (last status: {last})")


def _wait_connected(api_client) -> dict:
    return _wait_state(
        api_client,
        lambda st: st.get("isConnected") is True,
        SETTLE_BUDGET_S,
        "connect",
    )


def _wait_disconnected(api_client) -> dict:
    return _wait_state(
        api_client,
        lambda st: st.get("isConnected") is False and st.get("linkState") == "idle",
        DROP_BUDGET_S,
        "drop",
    )


class _OneShotServer:
    """TCP server that accepts one client, feeds it CSV, and can close on command."""

    def __init__(self):
        self._srv = socket.socket()
        self._srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._srv.bind(("127.0.0.1", 0))
        self._srv.listen(4)
        self.port = self._srv.getsockname()[1]
        self._conns = []
        self._running = True
        threading.Thread(target=self._accept_loop, daemon=True).start()

    def _accept_loop(self):
        while self._running:
            try:
                conn, _ = self._srv.accept()
            except OSError:
                return

            self._conns.append(conn)
            threading.Thread(target=self._feed, args=(conn,), daemon=True).start()

    def _feed(self, conn):
        n = 0
        try:
            while self._running:
                conn.sendall(f"{n},{n % 100}\n".encode())
                n += 1
                time.sleep(0.05)
        except OSError:
            pass

    def drop_clients(self):
        """Close every accepted connection, which is the peer-side drop under test."""
        for conn in self._conns:
            try:
                conn.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            conn.close()

        self._conns.clear()

    def stop(self):
        self._running = False
        self.drop_clients()
        self._srv.close()


@pytest.fixture
def feed_server():
    srv = _OneShotServer()
    yield srv
    srv.stop()


@pytest.mark.network
def test_tcp_peer_close_reports_disconnected(api_client, clean_state, feed_server):
    """A TCP peer that closes the socket drops the session, and the app says so."""
    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host="127.0.0.1", port=feed_server.port, socket_type="tcp"
    )
    time.sleep(0.3)

    api_client.command("io.connect", timeout=15.0)
    _wait_connected(api_client)

    feed_server.drop_clients()
    _wait_disconnected(api_client)


@pytest.mark.network
def test_reconnect_after_peer_close_succeeds(api_client, clean_state, feed_server):
    """The session outlives a drop: the next connect works without restarting anything."""
    api_client.set_operation_mode("quickplot")
    api_client.configure_network(
        host="127.0.0.1", port=feed_server.port, socket_type="tcp"
    )
    time.sleep(0.3)

    api_client.command("io.connect", timeout=15.0)
    _wait_connected(api_client)
    feed_server.drop_clients()
    _wait_disconnected(api_client)

    # The drop's teardown is queued behind the published state, so a reconnect issued in the
    # same instant races the close of the device it is about to reopen.
    time.sleep(1.5)

    api_client.command("io.connect", timeout=15.0)
    _wait_connected(api_client)
    api_client.command("io.disconnect")


@pytest.mark.network
def test_websocket_server_close_reports_disconnected(api_client, clean_state):
    """The WebSocket transport reports a server-side close the same way TCP does."""
    websockets = pytest.importorskip("websockets")

    import asyncio

    port_holder = {}
    stop_event = threading.Event()

    def serve():
        async def main():
            async def handler(conn):
                try:
                    while not stop_event.is_set():
                        await conn.send("1,2\n")
                        await asyncio.sleep(0.05)
                except Exception:
                    pass

            async with websockets.serve(handler, "127.0.0.1", 0) as server:
                port_holder["port"] = server.sockets[0].getsockname()[1]
                while not stop_event.is_set():
                    await asyncio.sleep(0.05)

        asyncio.run(main())

    thread = threading.Thread(target=serve, daemon=True)
    thread.start()

    deadline = time.time() + 5.0
    while "port" not in port_holder and time.time() < deadline:
        time.sleep(0.05)

    if "port" not in port_holder:
        pytest.skip("the local WebSocket server did not come up")

    api_client.set_operation_mode("quickplot")
    api_client.configure_network(socket_type="websocket")
    api_client.command(
        "io.network.setWebSocketUrl",
        {"url": f"ws://127.0.0.1:{port_holder['port']}"},
    )
    time.sleep(0.3)

    api_client.command("io.connect", timeout=15.0)
    _wait_connected(api_client)

    stop_event.set()
    _wait_disconnected(api_client)


@pytest.mark.uart
def test_serial_pty_removal_reports_disconnected(api_client, clean_state):
    """A pty that goes away is a ResourceError, and a custom device path must honour it.

    Ignoring ResourceError on custom paths left the port "open" forever, with write() still
    returning byte counts into nothing (finding D5).
    """
    if sys.platform.startswith("win"):
        pytest.skip("no pty pairs on Windows")

    if not shutil.which("socat"):
        pytest.skip("socat is required to create a disposable serial pair")

    workdir = os.environ.get("TMPDIR", "/tmp").rstrip("/")
    left = f"{workdir}/ss-drop-a"
    right = f"{workdir}/ss-drop-b"

    proc = subprocess.Popen(
        ["socat", f"PTY,link={left},raw,echo=0", f"PTY,link={right},raw,echo=0"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    try:
        deadline = time.time() + 5.0
        while not (os.path.exists(left) and os.path.exists(right)):
            if time.time() > deadline:
                pytest.skip("socat did not create the pty pair")
            time.sleep(0.1)

        api_client.set_operation_mode("quickplot")
        api_client.set_bus_type("uart")
        api_client.command("io.uart.setDevice", {"device": left})
        time.sleep(1.5)

        ports = api_client.command("io.uart.listPorts").get("portList", [])
        index = next(
            (p["index"] for p in ports if p.get("name") == left),
            None,
        )
        if index is None:
            pytest.skip("the registered custom port did not appear in the port list")

        api_client.command("io.uart.setPortIndex", {"portIndex": index})
        api_client.command("io.connect", timeout=15.0)
        _wait_connected(api_client)

        proc.terminate()
        proc.wait(timeout=5)
        _wait_disconnected(api_client)

    finally:
        if proc.poll() is None:
            proc.kill()
            proc.wait(timeout=5)
