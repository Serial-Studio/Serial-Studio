"""
Serial Studio API Client

Clean wrapper around the Serial Studio TCP API for testing.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import time
import uuid
from typing import Any, Optional


class APIError(Exception):
    """Exception raised when API returns an error."""

    def __init__(self, code: str, message: str):
        self.code = code
        self.message = message
        super().__init__(f"{code}: {message}")


class SerialStudioClient:
    """
    High-level client for Serial Studio API.

    Provides clean interface for testing without worrying about
    low-level socket communication details.
    """

    DEFAULT_HOST = "127.0.0.1"
    DEFAULT_PORT = 7777
    TIMEOUT = 5.0

    def __init__(
        self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT, timeout: float = TIMEOUT
    ):
        self.host = host
        self.port = port
        self.timeout = timeout
        self._socket: Optional[socket.socket] = None
        self._buffer = b""

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.disconnect()

    def connect(self) -> None:
        """Establish connection to Serial Studio API."""
        if self._socket:
            return

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.settimeout(self.timeout)
        try:
            self._socket.connect((self.host, self.port))
        except (ConnectionRefusedError, socket.timeout) as e:
            raise ConnectionError(
                f"Could not connect to Serial Studio at {self.host}:{self.port}. "
                f"Ensure Serial Studio is running with API Server enabled."
            ) from e

    def disconnect(self) -> None:
        """Close the connection."""
        if self._socket:
            try:
                self._socket.close()
            except Exception:
                pass
            self._socket = None
        self._buffer = b""

    def _recv_message(self, timeout: Optional[float] = None) -> dict:
        """Receive a single JSON message from the socket."""
        if not self._socket:
            raise RuntimeError("Not connected")

        timeout = timeout or self.timeout
        end_time = time.time() + timeout

        while True:
            newline_pos = self._buffer.find(b"\n")
            if newline_pos != -1:
                line = self._buffer[:newline_pos]
                self._buffer = self._buffer[newline_pos + 1 :]

                if line.strip():
                    return json.loads(line.decode("utf-8"))
                continue

            remaining = end_time - time.time()
            if remaining <= 0:
                raise TimeoutError("Timeout waiting for response")

            self._socket.settimeout(min(remaining, 0.1))
            try:
                chunk = self._socket.recv(65536)
                if not chunk:
                    raise ConnectionError("Connection closed by server")
                self._buffer += chunk
            except socket.timeout:
                continue

    def _send_message(self, message: dict) -> None:
        """Send a JSON message to the server."""
        if not self._socket:
            raise RuntimeError("Not connected")

        data = json.dumps(message, separators=(",", ":")) + "\n"
        self._socket.sendall(data.encode("utf-8"))

    def command(
        self, command: str, params: Optional[dict] = None, timeout: Optional[float] = None
    ) -> dict:
        """
        Send a command and return the result.

        Args:
            command: Command name (e.g., "io.manager.getStatus")
            params: Optional parameters dict
            timeout: Optional timeout override

        Returns:
            Result dict on success

        Raises:
            APIError: If command fails
            TimeoutError: If response not received in time
        """
        request_id = str(uuid.uuid4())
        message = {"type": "command", "id": request_id, "command": command}

        if params:
            message["params"] = params

        self._send_message(message)

        while True:
            response = self._recv_message(timeout)

            if response.get("type") == "response" and response.get("id") == request_id:
                if response.get("success"):
                    return response.get("result", {})
                else:
                    error = response.get("error", {})
                    raise APIError(error.get("code", "UNKNOWN"), error.get("message", "Unknown error"))

    def batch(
        self, commands: list[dict], timeout: Optional[float] = None
    ) -> list[dict]:
        """
        Send batch of commands.

        Args:
            commands: List of dicts with "command" and optional "params"
            timeout: Optional timeout override

        Returns:
            List of results (may include errors)

        Raises:
            TimeoutError: If response not received in time
        """
        request_id = str(uuid.uuid4())

        batch_commands = []
        for i, cmd in enumerate(commands):
            batch_cmd = {
                "id": f"{request_id}-{i}",
                "command": cmd["command"],
            }
            if "params" in cmd:
                batch_cmd["params"] = cmd["params"]
            batch_commands.append(batch_cmd)

        message = {"type": "batch", "id": request_id, "commands": batch_commands}

        self._send_message(message)

        while True:
            response = self._recv_message(timeout)

            if response.get("type") == "response" and response.get("id") == request_id:
                return response.get("results", [])

    def set_bus_type(self, bus_type: str) -> None:
        """
        Set the bus type.

        Args:
            bus_type: "uart", "network", "ble", "audio", "modbus", or "canbus"
        """
        bus_map = {
            "uart": 0,
            "network": 1,
            "ble": 2,
            "audio": 3,
            "modbus": 4,
            "canbus": 5,
        }

        if bus_type.lower() not in bus_map:
            raise ValueError(f"Invalid bus type: {bus_type}")

        self.command("io.manager.setBusType", {"busType": bus_map[bus_type.lower()]})

    def configure_network(
        self,
        host: str = "127.0.0.1",
        port: int = 9000,
        socket_type: str = "tcp",
    ) -> None:
        """Configure network driver for device simulation."""
        socket_type_map = {"tcp": 0, "udp": 1}

        commands = [
            {"command": "io.manager.setBusType", "params": {"busType": 1}},
            {"command": "io.driver.network.setRemoteAddress", "params": {"address": host}},
            {
                "command": "io.driver.network.setSocketType",
                "params": {"socketTypeIndex": socket_type_map[socket_type.lower()]},
            },
        ]

        if socket_type.lower() == "tcp":
            commands.append({"command": "io.driver.network.setTcpPort", "params": {"port": port}})
        else:
            commands.append({"command": "io.driver.network.setUdpRemotePort", "params": {"port": port}})

        self.batch(commands)

    def connect_device(self) -> None:
        """Connect to the configured device."""
        self.command("io.manager.connect")

    def disconnect_device(self) -> None:
        """Disconnect from the device."""
        self.command("io.manager.disconnect")

    def is_connected(self) -> bool:
        """Check if device is connected."""
        status = self.command("io.manager.getStatus")
        return status.get("isConnected", False)

    def enable_csv_export(self) -> None:
        """Enable CSV export."""
        self.command("csv.export.setEnabled", {"enabled": True})

    def disable_csv_export(self) -> None:
        """Disable CSV export."""
        self.command("csv.export.setEnabled", {"enabled": False})

    def get_csv_export_status(self) -> dict:
        """Get CSV export status."""
        return self.command("csv.export.getStatus")

    def load_project(self, file_path: str) -> None:
        """Load a project file."""
        self.command("project.file.open", {"filePath": file_path})

    def create_new_project(self) -> None:
        """Create a new empty project."""
        self.command("project.file.new")

    def get_project_status(self) -> dict:
        """Get project information."""
        return self.command("project.getStatus")

    def set_dashboard_fps(self, fps: int) -> None:
        """
        Set dashboard UI rendering/refresh rate.

        Args:
            fps: Frames per second for UI updates (typically 30-60)
                 Note: This is NOT the data processing rate.
                 Serial Studio can process data at much higher rates.
        """
        self.command("dashboard.setFPS", {"fps": fps})

    def set_dashboard_points(self, points: int) -> None:
        """
        Set maximum data points visible in plot widgets.

        Args:
            points: Number of data points to display (e.g., 500, 1000)
        """
        self.command("dashboard.setPoints", {"points": points})

    def get_dashboard_status(self) -> dict:
        """Get dashboard configuration."""
        return self.command("dashboard.getStatus")


    def set_operation_mode(self, mode: str) -> None:
        """
        Set dashboard operation mode and configure appropriate delimiters.

        Args:
            mode: "project", "json", or "quickplot"
        """
        mode_map = {
            "project": 0,  # ProjectFile mode
            "json": 1,  # DeviceSendsJSON mode
            "quickplot": 2,  # QuickPlot mode
        }

        if mode.lower() not in mode_map:
            raise ValueError(f"Invalid operation mode: {mode}")

        mode_index = mode_map[mode.lower()]

        # Configure delimiters based on mode
        if mode.lower() == "json":
            # DeviceSendsJSON: Set /* */ delimiters, no checksum
            self.configure_frame_parser(
                start_sequence="/*",
                end_sequence="*/",
                checksum_algorithm="",
                operation_mode=mode_index
            )
        else:
            # Just set the operation mode for other modes
            self.command("dashboard.setOperationMode", {"mode": mode_index})

    def wait_for_connection(self, timeout: float = 10.0) -> bool:
        """
        Wait for device to connect.

        Returns:
            True if connected within timeout, False otherwise
        """
        end_time = time.time() + timeout
        while time.time() < end_time:
            if self.is_connected():
                return True
            time.sleep(0.1)
        return False

    def load_project_from_json(self, config: dict) -> dict:
        """
        Load project configuration from JSON object.

        Args:
            config: Project configuration dict with frameParser, groups, datasets, etc.

        Returns:
            Result dict with loaded project info
        """
        return self.command("project.loadFromJSON", {"config": config})

    def configure_frame_parser(
        self,
        start_sequence: str = None,
        end_sequence: str = None,
        checksum_algorithm: str = None,
        operation_mode: int = None,
        frame_detection: int = None,
    ) -> dict:
        """
        Configure frame parser settings.

        Args:
            start_sequence: Frame start delimiter (e.g., "$", "/*")
            end_sequence: Frame end delimiter (e.g., "\\r\\n", "*/", ";")
            checksum_algorithm: Checksum name (e.g., "None", "CRC-16", "CRC-32", "Adler-32")
            operation_mode: Operation mode (0=ProjectFile, 1=DeviceSendsJSON, 2=QuickPlot)
            frame_detection: Frame detection mode (0=EndDelimiterOnly, 1=StartAndEndDelimiter, 2=NoDelimiters, 3=StartDelimiterOnly)

        Returns:
            Result dict
        """
        params = {}
        if start_sequence is not None:
            params["startSequence"] = start_sequence
        if end_sequence is not None:
            params["endSequence"] = end_sequence
        if checksum_algorithm is not None:
            params["checksumAlgorithm"] = checksum_algorithm
        if operation_mode is not None:
            params["operationMode"] = operation_mode
        if frame_detection is not None:
            params["frameDetection"] = frame_detection

        return self.command("project.frameParser.configure", params)

    def get_frame_parser_config(self) -> dict:
        """
        Get current frame parser configuration.

        Returns:
            Dict with startSequence, endSequence, checksumAlgorithm, operationMode
        """
        return self.command("project.frameParser.getConfig")

    def get_dashboard_data(self) -> dict:
        """
        Get current dashboard dataset values.

        Returns:
            Dict with groups, datasets, and their current values
        """
        return self.command("dashboard.getData")

    def get_dataset_count(self) -> int:
        """
        Get total number of datasets across all groups in current frame.

        Returns:
            Total dataset count
        """
        status = self.get_project_status()
        return status.get("datasetCount", 0)

    def get_widget_count(self) -> int:
        """
        Get total number of dashboard widgets currently active.

        This works in all modes (ProjectFile, DeviceSendsJSON, QuickPlot).

        Returns:
            Total widget count
        """
        status = self.get_dashboard_status()
        return status.get("widgetCount", 0)
