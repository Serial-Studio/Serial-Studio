"""
Virtual Serial Port Utility

Creates PTY pairs that simulate physical serial ports for integration testing.
Works on macOS (/dev/ttysNNN) and Linux (/dev/pts/N).

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import os
import time
from typing import Optional

# Probe PTY support at import time
PTY_AVAILABLE = True
try:
    import pty
    import tty

    _probe_master, _probe_slave = pty.openpty()
    os.close(_probe_master)
    os.close(_probe_slave)
except (OSError, ImportError, AttributeError):
    PTY_AVAILABLE = False


class VirtualSerialPort:
    """
    Creates a PTY pair: test writes to master_fd, Serial Studio opens slave path.
    """

    def __init__(self, name: str = "VirtualPort"):
        self.name = name
        self._master_fd: Optional[int] = None
        self._slave_fd: Optional[int] = None
        self._device_path: Optional[str] = None
        self._closed = True

    def open(self) -> None:
        """Create the PTY pair and configure raw mode."""
        self._master_fd, self._slave_fd = pty.openpty()
        tty.setraw(self._master_fd)
        tty.setraw(self._slave_fd)
        self._device_path = os.ttyname(self._slave_fd)
        self._closed = False

    def close(self) -> None:
        """Close both file descriptors safely."""
        if self._closed:
            return

        self._closed = True
        for fd in (self._master_fd, self._slave_fd):
            if fd is not None:
                try:
                    os.close(fd)
                except OSError:
                    pass

        self._master_fd = None
        self._slave_fd = None

    @property
    def device_path(self) -> str:
        """Device path that Serial Studio should connect to."""
        if self._device_path is None:
            raise RuntimeError(f"{self.name}: PTY not open, call open() first")
        return self._device_path

    @property
    def is_open(self) -> bool:
        return not self._closed

    def write(self, data: bytes) -> int:
        """Write raw bytes to the master side of the PTY."""
        if self._master_fd is None or self._closed:
            raise RuntimeError(f"{self.name}: PTY not open")
        return os.write(self._master_fd, data)

    def write_csv_frame(
        self,
        values: list,
        start_delimiter: str = "/*",
        end_delimiter: str = "*/",
    ) -> None:
        """Write a delimited CSV frame with the given values."""
        csv_str = ",".join(str(v) for v in values)
        frame = f"{start_delimiter}{csv_str}{end_delimiter}\n".encode()
        self.write(frame)

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *exc):
        self.close()


class DualSerialPorts:
    """Manages two VirtualSerialPort instances for dual-source testing."""

    def __init__(
        self,
        name_a: str = "PortAlpha",
        name_b: str = "PortBravo",
    ):
        self.alpha = VirtualSerialPort(name_a)
        self.bravo = VirtualSerialPort(name_b)

    @property
    def device_path_a(self) -> str:
        return self.alpha.device_path

    @property
    def device_path_b(self) -> str:
        return self.bravo.device_path

    def open(self) -> None:
        """Open both PTY pairs."""
        self.alpha.open()
        self.bravo.open()

    def close(self) -> None:
        """Close both PTY pairs."""
        self.alpha.close()
        self.bravo.close()

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, *exc):
        self.close()
