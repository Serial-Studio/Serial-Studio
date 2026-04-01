"""
Serial Studio Test Utilities

Shared utilities for integration testing.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

from .api_client import SerialStudioClient, APIError
from .device_simulator import DeviceSimulator
from .data_generator import DataGenerator, ChecksumType
from .validators import validate_csv_export, validate_frame_structure
from .virtual_serial import VirtualSerialPort, DualSerialPorts, PTY_AVAILABLE

__all__ = [
    "SerialStudioClient",
    "APIError",
    "DeviceSimulator",
    "DataGenerator",
    "ChecksumType",
    "validate_csv_export",
    "validate_frame_structure",
    "VirtualSerialPort",
    "DualSerialPorts",
    "PTY_AVAILABLE",
]
