"""
Device Simulator

Simulates a device sending telemetry data over TCP/UDP.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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

    def start(self) -> None:
        """Start the simulator server."""
        if self._running:
            return

        if self.protocol == "tcp":
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._socket.bind((self.host, self.port))
            self._socket.listen(1)
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

        if self._client_socket:
            try:
                self._client_socket.close()
            except Exception:
                pass
            self._client_socket = None

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
                    self._client_socket = client
                except socket.timeout:
                    continue
                except Exception:
                    break
            else:
                break

    def send_frame(self, frame: bytes) -> None:
        """
        Send a single frame.

        Args:
            frame: Frame bytes to send
        """
        if not self._running:
            raise RuntimeError("Simulator not started")

        if self.protocol == "tcp":
            if not self._client_socket:
                raise RuntimeError("No client connected")
            try:
                self._client_socket.sendall(frame)
            except (BrokenPipeError, ConnectionResetError):
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
        Send multiple frames with delay between them.

        Args:
            frames: List of frames to send
            interval_seconds: Delay between frames
        """
        for frame in frames:
            self.send_frame(frame)
            time.sleep(interval_seconds)

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

        start_time = time.time()
        while time.time() - start_time < timeout:
            if self._client_socket:
                return True
            time.sleep(0.1)
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
