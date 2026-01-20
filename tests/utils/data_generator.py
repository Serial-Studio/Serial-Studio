"""
Telemetry Data Generator

Generates realistic telemetry data with proper checksums for testing.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import random
import struct
import zlib
from enum import Enum
from typing import Any, Optional


class ChecksumType(Enum):
    """Supported checksum algorithms matching Serial Studio."""

    NONE = "none"
    XOR = "xor"
    SUM = "sum"
    CRC8 = "crc8"
    CRC16 = "crc16"
    CRC32 = "crc32"
    FLETCHER16 = "fletcher16"
    ADLER32 = "adler32"


class DataGenerator:
    """Generates realistic telemetry data for testing."""

    # Standard CSV parser template for ProjectFile mode tests
    CSV_PARSER_TEMPLATE = """/**
 * Comma-Separated Values (CSV) Parser
 *
 * Parses comma-separated data into an array of values.
 *
 * INPUT FORMAT: "a,b,c,d,e,f"
 * OUTPUT ARRAY: ["a", "b", "c", "d", "e", "f"]
 *
 * Note: Frame delimiters are automatically removed by Serial Studio.
 */

//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a CSV frame into an array of values.
 *
 * @param {string} frame - The CSV data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
  return frame.split(',');
}
"""

    @staticmethod
    def calculate_checksum(data: bytes, checksum_type: ChecksumType) -> int:
        """
        Calculate checksum for data.

        Args:
            data: Raw bytes to checksum
            checksum_type: Checksum algorithm

        Returns:
            Checksum value as integer
        """
        if checksum_type == ChecksumType.NONE:
            return 0

        elif checksum_type == ChecksumType.XOR:
            result = 0
            for byte in data:
                result ^= byte
            return result

        elif checksum_type == ChecksumType.SUM:
            return sum(data) & 0xFF

        elif checksum_type == ChecksumType.CRC8:
            crc = 0xFF
            for byte in data:
                crc ^= byte
                for _ in range(8):
                    if crc & 0x80:
                        crc = (crc << 1) ^ 0x31
                    else:
                        crc <<= 1
                    crc &= 0xFF
            return crc

        elif checksum_type == ChecksumType.CRC16:
            crc = 0xFFFF
            for byte in data:
                x = (crc >> 8) ^ byte
                x ^= (x >> 4)
                crc = ((crc << 8) ^ (x << 12) ^ (x << 5) ^ x) & 0xFFFF
            return crc

        elif checksum_type == ChecksumType.CRC32:
            return zlib.crc32(data) & 0xFFFFFFFF

        elif checksum_type == ChecksumType.FLETCHER16:
            sum1 = 0
            sum2 = 0
            for byte in data:
                sum1 = (sum1 + byte) % 255
                sum2 = (sum2 + sum1) % 255
            return (sum2 << 8) | sum1

        elif checksum_type == ChecksumType.ADLER32:
            return zlib.adler32(data) & 0xFFFFFFFF

        else:
            raise ValueError(f"Unknown checksum type: {checksum_type}")

    @staticmethod
    def generate_json_frame(
        groups: Optional[list[dict]] = None,
        title: str = "Test Frame",
    ) -> dict:
        """
        Generate a JSON frame matching Serial Studio DeviceSendsJSON format.

        Uses proper Frame keys as defined in Frame.h: "title", "groups", "datasets", etc.
        NOT the abbreviated format ("t", "g") - that's for internal serialization only.

        IMPORTANT: Includes widget definitions so dashboard actually displays data.

        Args:
            groups: List of group dicts (auto-generated if None)
            title: Frame title

        Returns:
            JSON frame dict
        """
        if groups is None:
            groups = [
                {
                    "title": "Sensors",
                    "widget": "datagrid",
                    "datasets": [
                        {
                            "title": "Temperature",
                            "value": f"{20 + random.uniform(-5, 5):.2f}",
                            "units": "°C",
                            "widget": "gauge",
                            "graph": True,
                        },
                        {
                            "title": "Humidity",
                            "value": f"{50 + random.uniform(-10, 10):.1f}",
                            "units": "%",
                            "widget": "bar",
                            "graph": True,
                        },
                        {
                            "title": "Pressure",
                            "value": f"{1013 + random.uniform(-20, 20):.1f}",
                            "units": "hPa",
                            "widget": "gauge",
                            "graph": True,
                        },
                    ],
                },
                {
                    "title": "GPS",
                    "widget": "gps",
                    "datasets": [
                        {
                            "title": "Latitude",
                            "value": f"{37.7749 + random.uniform(-0.01, 0.01):.6f}",
                            "units": "°",
                            "widget": "lat",
                        },
                        {
                            "title": "Altitude",
                            "value": f"{100 + random.uniform(-10, 10):.1f}",
                            "units": "m",
                            "widget": "alt",
                        },
                        {
                            "title": "Longitude",
                            "value": f"{-122.4194 + random.uniform(-0.01, 0.01):.6f}",
                            "units": "°",
                            "widget": "lon",
                        },
                    ],
                },
            ]

        return {"title": title, "groups": groups}

    @staticmethod
    def generate_csv_frame(
        values: Optional[list[float]] = None,
        delimiter: str = ",",
    ) -> str:
        """
        Generate a CSV-formatted frame.

        Args:
            values: List of values (auto-generated if None)
            delimiter: CSV delimiter

        Returns:
            CSV string
        """
        if values is None:
            values = [
                20 + random.uniform(-5, 5),
                50 + random.uniform(-10, 10),
                1013 + random.uniform(-20, 20),
                37.7749 + random.uniform(-0.01, 0.01),
                -122.4194 + random.uniform(-0.01, 0.01),
                100 + random.uniform(-10, 10),
            ]

        return delimiter.join(f"{v:.3f}" for v in values)

    @staticmethod
    def wrap_frame(
        payload: str,
        start_delimiter: str = "/*",
        end_delimiter: str = "*/",
        checksum_type: ChecksumType = ChecksumType.NONE,
        line_terminator: str = "\n",
        mode: str = "json",
    ) -> bytes:
        """
        Wrap payload with delimiters and optional binary checksum.

        IMPORTANT: Both DeviceSendsJSON and ProjectFile modes use delimiters!
        - DeviceSendsJSON mode: /* */ delimiters, NO checksums
        - ProjectFile mode: /* */ delimiters AND binary checksums

        FrameReader uses delimiters to extract frames, then FrameBuilder parses the payload.

        Args:
            payload: Frame payload (string)
            start_delimiter: Start delimiter (default: "/*")
            end_delimiter: End delimiter (default: "*/")
            checksum_type: Checksum algorithm (ignored in JSON mode)
            line_terminator: Line terminator (default: "\n")
            mode: "json" for DeviceSendsJSON, "project" for ProjectFile mode

        Returns:
            Complete frame as bytes

        Examples:
            DeviceSendsJSON mode: /*{"title":"Test","groups":[]}*/\n
            ProjectFile mode: /*{"title":"Test","groups":[]}*/\xAB\xCD\r\n
        """
        payload_bytes = payload.encode("utf-8")
        frame_bytes = (start_delimiter + payload + end_delimiter).encode("utf-8")

        # Only add checksums in ProjectFile mode
        if mode.lower() != "json" and checksum_type != ChecksumType.NONE:
            checksum = DataGenerator.calculate_checksum(payload_bytes, checksum_type)

            if checksum_type in (ChecksumType.CRC32, ChecksumType.ADLER32):
                checksum_bytes = struct.pack(">I", checksum)
            elif checksum_type == ChecksumType.CRC16:
                checksum_bytes = struct.pack(">H", checksum)
            elif checksum_type == ChecksumType.FLETCHER16:
                checksum_bytes = struct.pack(">H", checksum)
            else:
                checksum_bytes = struct.pack("B", checksum & 0xFF)

            frame_bytes += checksum_bytes

        frame_bytes += line_terminator.encode("utf-8")
        return frame_bytes

    @staticmethod
    def generate_sensor_reading(
        sensor_type: str = "temperature",
        timestamp: Optional[float] = None,
    ) -> dict:
        """
        Generate realistic sensor reading.

        Args:
            sensor_type: Type of sensor (temperature, pressure, humidity, etc.)
            timestamp: Optional timestamp

        Returns:
            Sensor reading dict
        """
        import time

        if timestamp is None:
            timestamp = time.time()

        readings = {
            "temperature": {
                "value": 20 + random.gauss(0, 5),
                "units": "°C",
                "range": (-40, 85),
            },
            "humidity": {
                "value": 50 + random.gauss(0, 15),
                "units": "%",
                "range": (0, 100),
            },
            "pressure": {
                "value": 1013 + random.gauss(0, 30),
                "units": "hPa",
                "range": (950, 1050),
            },
            "voltage": {
                "value": 3.3 + random.gauss(0, 0.1),
                "units": "V",
                "range": (0, 5),
            },
            "current": {
                "value": 0.5 + random.gauss(0, 0.05),
                "units": "A",
                "range": (0, 3),
            },
            "acceleration": {
                "value": random.gauss(0, 1),
                "units": "g",
                "range": (-8, 8),
            },
            "gyro": {
                "value": random.gauss(0, 10),
                "units": "°/s",
                "range": (-250, 250),
            },
            "gps_lat": {
                "value": 37.7749 + random.gauss(0, 0.001),
                "units": "°",
                "range": (-90, 90),
            },
            "gps_lon": {
                "value": -122.4194 + random.gauss(0, 0.001),
                "units": "°",
                "range": (-180, 180),
            },
        }

        if sensor_type not in readings:
            raise ValueError(f"Unknown sensor type: {sensor_type}")

        reading = readings[sensor_type]
        value = max(reading["range"][0], min(reading["range"][1], reading["value"]))

        return {
            "sensor": sensor_type,
            "value": value,
            "units": reading["units"],
            "timestamp": timestamp,
        }

    @staticmethod
    def generate_binary_frame(
        values: Optional[list[float]] = None,
        format_string: str = "ffffff",
    ) -> bytes:
        """
        Generate binary frame using struct format.

        Args:
            values: List of values to pack
            format_string: struct format string (default: 6 floats)

        Returns:
            Binary frame
        """
        if values is None:
            values = [
                20 + random.uniform(-5, 5),
                50 + random.uniform(-10, 10),
                1013 + random.uniform(-20, 20),
                37.7749 + random.uniform(-0.01, 0.01),
                -122.4194 + random.uniform(-0.01, 0.01),
                100 + random.uniform(-10, 10),
            ]

        return struct.pack(format_string, *values)

    @staticmethod
    def generate_realistic_telemetry(
        duration_seconds: float = 1.0,
        frequency_hz: float = 10.0,
        frame_format: str = "json",
        checksum_type: ChecksumType = ChecksumType.NONE,
        mode: str = "json",
    ) -> list[bytes]:
        """
        Generate realistic telemetry stream.

        Args:
            duration_seconds: Duration of data to generate
            frequency_hz: Sample rate in Hz
            frame_format: "json" or "csv"
            checksum_type: Checksum algorithm (ignored in "json" mode)
            mode: "json" for DeviceSendsJSON mode, "project" for ProjectFile mode

        Returns:
            List of frames
        """
        num_samples = int(duration_seconds * frequency_hz)
        frames = []

        for i in range(num_samples):
            if frame_format == "json":
                payload = json.dumps(DataGenerator.generate_json_frame())
            else:
                payload = DataGenerator.generate_csv_frame()

            frame = DataGenerator.wrap_frame(
                payload,
                checksum_type=checksum_type,
                mode=mode,
            )
            frames.append(frame)

        return frames

    @staticmethod
    def generate_malformed_json() -> list[str]:
        """
        Generate various malformed JSON payloads for fuzzy testing.

        Returns:
            List of malformed JSON strings
        """
        return [
            "{",
            "}",
            "{{}",
            '{"unclosed": "string',
            '{"missing": }',
            '{"trailing": "comma",}',
            '{"duplicate":"key","duplicate":"key"}',
            '{"nested": {"unclosed": }',
            '{"unescaped": "quotes"inside"}',
            '{"invalid\\escape": "sequence"}',
            '{"number": 123.456.789}',
            '{"array": [1, 2, 3,]}',
            '{"array": [1, 2, , 3]}',
            "{'single': 'quotes'}",
            '{"null": null}{"extra":"object"}',
            '{"comment": /* not allowed */ "value"}',
            '{"unicode": "\\uDEAD"}',
            '{"control": "chars\x00\x01\x02"}',
            '{"very_long_key_' + "x" * 10000 + '": "value"}',
            '{"value": "very_long_' + "x" * 100000 + '"}',
            "{" * 1000 + "}" * 1000,
            '{"inf": Infinity}',
            '{"nan": NaN}',
            '{"undefined": undefined}',
            "",
            "null",
            "[]",
            "[[[[[",
            '"\\"',
        ]

    @staticmethod
    def generate_edge_case_values() -> list[dict]:
        """
        Generate JSON frames with edge case numeric values.

        Returns:
            List of JSON frame dicts with edge cases
        """
        edge_cases = []

        edge_values = [
            ("zero", "0"),
            ("negative_zero", "-0"),
            ("max_int", str(2**63 - 1)),
            ("min_int", str(-(2**63))),
            ("very_large", "1" + "0" * 308),
            ("very_small", "1e-308"),
            ("max_float", "1.7976931348623157e+308"),
            ("negative", "-999999"),
            ("scientific", "1.23e-10"),
            ("many_decimals", "3.14159265358979323846"),
        ]

        for name, value in edge_values:
            edge_cases.append(
                {
                    "title": f"Edge Case: {name}",
                    "groups": [
                        {
                            "title": "Edge Values",
                            "datasets": [
                                {"title": name, "value": value, "units": "units"}
                            ],
                        }
                    ],
                }
            )

        return edge_cases

    @staticmethod
    def generate_random_garbage(min_size: int = 1, max_size: int = 1024) -> bytes:
        """
        Generate random binary garbage data.

        Args:
            min_size: Minimum size in bytes
            max_size: Maximum size in bytes

        Returns:
            Random bytes
        """
        size = random.randint(min_size, max_size)
        return bytes(random.randint(0, 255) for _ in range(size))

    @staticmethod
    def corrupt_frame(frame: bytes, corruption_rate: float = 0.1) -> bytes:
        """
        Corrupt a valid frame by randomly flipping bits.

        Args:
            frame: Valid frame bytes
            corruption_rate: Probability of corrupting each byte (0.0 to 1.0)

        Returns:
            Corrupted frame
        """
        frame_array = bytearray(frame)
        for i in range(len(frame_array)):
            if random.random() < corruption_rate:
                frame_array[i] = random.randint(0, 255)
        return bytes(frame_array)

    @staticmethod
    def generate_partial_frame(complete_frame: bytes) -> bytes:
        """
        Generate a partial/truncated frame.

        Args:
            complete_frame: Complete valid frame

        Returns:
            Truncated frame
        """
        if len(complete_frame) <= 1:
            return complete_frame

        cut_point = random.randint(1, len(complete_frame) - 1)
        return complete_frame[:cut_point]

    @staticmethod
    def generate_oversized_frame(multiplier: int = 100) -> bytes:
        """
        Generate an extremely large frame to test buffer limits.

        Args:
            multiplier: Size multiplier for groups/datasets

        Returns:
            Very large frame
        """
        groups = []
        for i in range(multiplier):
            datasets = [
                {
                    "title": f"Dataset_{j}_" + "x" * 100,
                    "value": str(random.random() * 1000),
                    "units": "unit_" + "y" * 50,
                }
                for j in range(multiplier)
            ]
            groups.append({"title": f"Group_{i}_" + "z" * 100, "datasets": datasets})

        payload = json.dumps({"title": "Oversized Frame", "groups": groups})
        return DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)

    @staticmethod
    def generate_unicode_stress() -> list[str]:
        """
        Generate various Unicode edge cases.

        Returns:
            List of Unicode test strings
        """
        return [
            "Simple ASCII",
            "Émojis: 😀🎉🚀",
            "中文字符",
            "العربية",
            "עברית",
            "Кириллица",
            "ΕλληνικÞ",
            "🔥💯✨",
            "\u0000\u0001\u0002",
            "Zero\x00width",
            "Right‏to‏Left‏",
            "Combining: é vs é",
            "Zalgo: H̷̡̢̨̛̛̖̣̗̰̮̦̬̺͚̯͓̰̣͔̮̲̤̼̭͕̗̝̹̘̪͉͚̗̳̗̘̼̟̞̼̜̻̫̪̱̥̤̝̦̱͓̮̰̠̳̗͖̮̻̲̥̼̭̪͖̾̂͋̊̐͊̇̓̓̀̾̄́̌̍̑̓́̐̔̾͒̊͂̎̈́̀̾̈̌̈́̌͂̑͘͘͝͝͝ͅͅͅę̸̢̧̛̪̯͚̟̬̣̰̼̤̞̗̩̙̪̰̘͉̳̟̻̺̺̰̼̗̣̦͔͚͇̟̹̬̩̦̗̖̹͚̰̞̼̖͓̫̦̰͚͉̟̣͙̮̟̰̪͓̺͉͉̤̻̻̏̆̈́̏͐̒̂̐̽̈́̾̈́̆̄̎̈́͊̇̇͋͑̋͗̀̿̏̈́͌̂̑͗̐̍͆̈́̌̄̾̋̔̾̀̿̎̄̾͛̕͜͜͝͝͝͠͝l̸̢̨̢̧̨̛͚̞͇̺̬̻̞͙͎̘̬̹͇̦͕̝͈̤̞̱̦̠̦̝̰̟͙͕͓̖̝̲̗̳̩̗͉̣̱̺̭̙̬̦͙̦͍̳̜̭̥̖͙̘̻̻̃̾̈͋̏̃̇̄̀̾̋̓͆̊́̆̍̆̽͑́̏̆͊̂͒̀̄̀̑͊͆̐̆̾̎̂̊́̍͋̑̽͌̌̎̇̆̑͋͐̀̍̎͒͗͊͂͘̚͘͜͜͝͝͠͝͝͝ͅͅl̴̡̨̢̧̧̧̛̛̛̛̛͖̬̤̯̖̩̖̙̥̠̤̤̻̼̼̺̼̻̰̹̮̳̙̯̪͚̥̪̜̫͓̟̜̺̦̻͍̹̩̮̠̜̩͖̩̦̭͚̩͙̰͇͖̫̮̘̬̖̮̮̠̠̝̙̤̍̋̈́͗̔̈́̀̈́̀̿͑̂͆̿̉̑͋̀͐̿͋̀̋͑̀͊̽̊̈́̂̈́̿̐̒̐̇̐̇͒̀̈́̍̿̄̂̆͐̓̿͂̂̾̎̒̅̏͗̄̕̚̕͘͜͜͜͝͠͝͝͝͝͠ͅǭ̸̢̧̢̡̢̨̧̛̛̛̞̯͈̦͓̩̻̯̩͙̙̪̹̰̫̤̞͓̣̬͕͖̦̙̦͙̥͈̹̞̪̤̪̹̪̗̗̥̺̥̮̭̺͍̖̲̪̬̞̠̯̺̝̖̳̖̦̱̆̓̇̀̀̉͛̐͑͑̉͐̄̔̑̓̀̌̎̆̐̓͂̈̔͂̐̎̋̆̎̊̄̌̓̔͂̎͋̌̽̈́͋̈́̐͊͗͌̌͋̏̾̐̈́̓̎̓̉̔̔͊̕͘̚̚͘͜͜͜͝͝͝͝͠͝͝",
            "Byte Order Mark: \ufeff",
        ]

    @staticmethod
    def generate_delimiter_confusion() -> list[bytes]:
        """
        Generate frames designed to confuse delimiter detection.

        Returns:
            List of confusing frame bytes
        """
        return [
            b"/**//**//**/\n",
            b"/*/*nested*/*/\n",
            b"/*start*/middle/*end*/\n",
            b"no delimiters at all\n",
            b"*/backwards/*\n",
            b"/**/\r\n/**/\n/**/\r",
            b"/*" * 100 + b"\n",
            b"*/" * 100 + b"\n",
            b"/*mixed\r\nline\nendings\r*/\n",
            b"/*binary\x00null\x00bytes*/\n",
            b"/**/" + b"\x00" * 10 + b"\n",
        ]

    @staticmethod
    def generate_rapid_frames(count: int = 1000) -> list[bytes]:
        """
        Generate many small frames for rapid-fire testing.

        Args:
            count: Number of frames to generate

        Returns:
            List of minimal frames
        """
        frames = []
        for i in range(count):
            payload = json.dumps({"title": f"F{i}", "groups": []})
            frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
            frames.append(frame)
        return frames

    @staticmethod
    def generate_project_with_frame_delimiters(
        start: str = "/*",
        end: str = "*/",
        detection_mode: int = 0,
        title: str = "Test Project",
        checksum_algorithm: str = "None",
    ) -> dict:
        """
        Generate a complete project JSON configuration with custom frame delimiters.

        Uses CSV parser and creates datasets with plot widgets enabled.

        Args:
            start: Frame start delimiter
            end: Frame end delimiter
            detection_mode: Frame detection mode (0=EndDelimiterOnly, 1=StartAndEndDelimiter, 2=NoDelimiters, 3=StartDelimiterOnly)
            title: Project title
            checksum_algorithm: Checksum algorithm name

        Returns:
            Project configuration dict suitable for load_project_from_json()
        """
        return {
            "title": title,
            "frameStart": start,
            "frameEnd": end,
            "frameDetection": detection_mode,
            "checksum": checksum_algorithm if checksum_algorithm != "None" else "",
            "frameParser": DataGenerator.CSV_PARSER_TEMPLATE,
            "groups": [
                {
                    "title": "Test Group",
                    "widget": "datagrid",
                    "datasets": [
                        {
                            "title": f"Dataset{i}",
                            "graph": True,
                            "units": "",
                            "value": "0",
                        }
                        for i in range(6)
                    ],
                }
            ],
        }
