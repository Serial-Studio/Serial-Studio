"""
Device Simulator

Simulates a device sending telemetry data over TCP/UDP.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import socket
import threading
import time
from typing import Callable, Optional

from .data_generator import ChecksumType, DataGenerator


class DeviceSimulator:
    """
    Simulates a device sending telemetry data.

    Uses TCP or UDP to send data that Serial Studio can receive via
    the network driver.
    """

    def __init__(
        self,
        host: str = "127.0.0.1",
        port: int = 9000,
        protocol: str = "tcp",
    ):
        """
        Initialize device simulator.

        Args:
            host: Host to bind to (127.0.0.1 for localhost)
            port: Port to listen on
            protocol: "tcp" or "udp"
        """
        self.host = host
        self.port = port
        self.protocol = protocol.lower()

        self._socket: Optional[socket.socket] = None
        self._thread: Optional[threading.Thread] = None
        self._running = False
        self._client_socket: Optional[socket.socket] = None
        self._client_lock = threading.Lock()

    def start(self) -> None:
        """Start the simulator server."""
        if self._running:
            return

        if self.protocol == "tcp":
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._socket.bind((self.host, self.port))
            self._socket.listen(8)
            self._socket.settimeout(1.0)
        else:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._socket.bind((self.host, self.port))
            self._socket.settimeout(1.0)

        self._running = True
        self._thread = threading.Thread(target=self._server_loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        """Stop the simulator server."""
        self._running = False

        with self._client_lock:
            client = self._client_socket
            self._client_socket = None
        if client:
            try:
                client.close()
            except Exception:
                pass

        if self._socket:
            try:
                self._socket.close()
            except Exception:
                pass
            self._socket = None

        if self._thread:
            self._thread.join(timeout=2.0)
            self._thread = None

    def _server_loop(self) -> None:
        """Server loop for accepting connections."""
        while self._running:
            if self.protocol == "tcp":
                try:
                    client, addr = self._socket.accept()
                    if self._is_dead_peer(client):
                        try:
                            client.close()
                        except Exception:
                            pass
                        continue
                    with self._client_lock:
                        previous = self._client_socket
                        self._client_socket = client
                    if previous is not None:
                        try:
                            previous.close()
                        except Exception:
                            pass
                    self._monitor_client(client)
                except socket.timeout:
                    continue
                except Exception:
                    break
            else:
                break

    @staticmethod
    def _is_dead_peer(client: socket.socket) -> bool:
        """True when the freshly-accepted peer has already hung up.

        Serial Studio's TCP dial opens a throwaway probe connection before the
        real one (Network.cpp probeTcpOnce). The probe handshakes and closes
        immediately, so if we latched it as the client, the next accepted (real)
        connection would close it out from under an in-flight send -> EBADF /
        "No client connected" / silently-dropped frames. A peek that returns EOF
        means the peer already closed (the probe); BlockingIOError means a live
        client that simply has not sent yet -- keep that one. The non-blocking
        window comes from setblocking(False), not MSG_DONTWAIT: that flag does
        not exist on Windows, and the AttributeError it raised there escaped
        every except clause and killed the accept loop on the first connection.
        """
        try:
            client.setblocking(False)
            peeked = client.recv(1, socket.MSG_PEEK)
            return peeked == b""
        except BlockingIOError:
            return False
        except OSError:
            return True
        finally:
            try:
                client.setblocking(True)
            except OSError:
                pass

    def _monitor_client(self, client: socket.socket) -> None:
        """Spawn a thread that clears _client_socket when the remote side closes."""
        import select

        def _watch() -> None:
            try:
                while self._running and self._client_socket is client:
                    try:
                        readable, _, _ = select.select([client], [], [], 0.5)
                    except OSError:
                        break
                    if not readable:
                        continue
                    try:
                        data = client.recv(1, socket.MSG_PEEK)
                        if not data:
                            break
                    except Exception:
                        break
                    # The app wrote back (deviceWrite/ACK); we never consume, so
                    # the socket stays readable forever. Sleep instead of spinning
                    # a full core -- the GIL contention is what widens the probe
                    # race on loaded CI runners.
                    time.sleep(0.05)
            finally:
                with self._client_lock:
                    if self._client_socket is client:
                        self._client_socket = None

        t = threading.Thread(target=_watch, daemon=True)
        t.start()

    def send_frame(self, frame: bytes) -> None:
        """
        Send a single frame.

        Args:
            frame: Frame bytes to send
        """
        if not self._running:
            raise RuntimeError("Simulator not started")

        if self.protocol == "tcp":
            with self._client_lock:
                sock = self._client_socket
            if sock is None:
                raise RuntimeError("No client connected")
            try:
                sock.sendall(frame)
            except (BrokenPipeError, ConnectionResetError, OSError):
                with self._client_lock:
                    if self._client_socket is sock:
                        self._client_socket = None
                raise RuntimeError("Client disconnected")
        else:
            self._socket.sendto(frame, (self.host, self.port))

    def send_frames(
        self,
        frames: list[bytes],
        interval_seconds: float = 0.1,
    ) -> None:
        """
        Send multiple frames paced at a fixed interval.

        Pacing follows an absolute schedule rather than sleeping the interval
        after each frame: loaded CI runners overshoot short sleeps by tens of
        milliseconds, and a relative sleep accumulates that overshoot until
        wall-clock assertions measure the host scheduler instead of the app.
        When the schedule slips, frames are sent back-to-back to catch up, so
        total wall time stays close to len(frames) * interval_seconds.

        Args:
            frames: List of frames to send
            interval_seconds: Target spacing between frames
        """
        next_send = time.monotonic()
        for frame in frames:
            self.send_frame(frame)
            next_send += interval_seconds
            delay = next_send - time.monotonic()
            if delay > 0:
                time.sleep(delay)

    def stream_data(
        self,
        generator: Callable[[], bytes],
        rate_hz: float = 10.0,
        duration_seconds: Optional[float] = None,
    ) -> None:
        """
        Stream data continuously.

        Args:
            generator: Function that returns frame bytes
            rate_hz: Streaming rate in Hz
            duration_seconds: Duration to stream (None = infinite)
        """
        interval = 1.0 / rate_hz
        start_time = time.time()

        while self._running:
            if duration_seconds and (time.time() - start_time) >= duration_seconds:
                break

            try:
                frame = generator()
                self.send_frame(frame)
            except Exception:
                break

            time.sleep(interval)

    def wait_for_connection(self, timeout: float = 10.0) -> bool:
        """
        Wait for client to connect (TCP only).

        Args:
            timeout: Maximum time to wait

        Returns:
            True if connected, False if timeout
        """
        if self.protocol != "tcp":
            return True

        # Require the same socket to persist across one poll interval before
        # declaring the link up: the app's pre-dial probe connection can briefly
        # latch here, and returning on the immediate first sight of it would hand
        # the test a socket that is about to close.
        stable: Optional[socket.socket] = None
        start_time = time.time()
        while time.time() - start_time < timeout:
            current = self._client_socket
            if current is not None and current is stable:
                return True
            stable = current
            time.sleep(0.12)
        return False

    @staticmethod
    def create_json_device(
        host: str = "127.0.0.1",
        port: int = 9000,
        protocol: str = "tcp",
        checksum_type: ChecksumType = ChecksumType.CRC16,
    ) -> "DeviceSimulator":
        """
        Create a device that sends JSON frames.

        Returns a simulator configured to send realistic JSON telemetry.
        """
        sim = DeviceSimulator(host, port, protocol)
        sim._frame_generator = lambda: DataGenerator.wrap_frame(
            DataGenerator.generate_json_frame(),
            checksum_type=checksum_type,
            mode="project",
        )
        return sim

    @staticmethod
    def create_csv_device(
        host: str = "127.0.0.1",
        port: int = 9000,
        protocol: str = "tcp",
        checksum_type: ChecksumType = ChecksumType.CRC16,
    ) -> "DeviceSimulator":
        """
        Create a device that sends CSV frames.

        Returns a simulator configured to send realistic CSV telemetry.
        """
        sim = DeviceSimulator(host, port, protocol)
        sim._frame_generator = lambda: DataGenerator.wrap_frame(
            DataGenerator.generate_csv_frame(),
            checksum_type=checksum_type,
            mode="project",
        )
        return sim

    def get_frame_generator(self) -> Callable[[], bytes]:
        """Get the configured frame generator function."""
        return getattr(self, "_frame_generator", None)

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()
