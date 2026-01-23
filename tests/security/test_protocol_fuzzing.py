#!/usr/bin/env python3
"""
Protocol Fuzzing Test Suite for Serial Studio API

This test uses fuzzing techniques to find crashes, hangs, and unexpected
behavior in the API protocol implementation.

Attack vectors:
- Malformed protocol messages
- Invalid state sequences
- Boundary condition exploitation
- Binary data injection
- Encoding confusion attacks

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import random
import socket
import string
import time
import uuid
from typing import List, Any


class ProtocolFuzzer:
    """Protocol-level fuzzer for Serial Studio API"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.crashes = []
        self.hangs = []
        self.unexpected = []

    def send_raw(self, data: bytes, timeout: float = 2.0) -> tuple:
        """
        Send raw data and return (success, response, error)
        """
        sock = None
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(timeout)
            sock.connect((self.host, self.port))
            sock.sendall(data)

            response = b""
            start = time.time()
            while time.time() - start < timeout:
                try:
                    chunk = sock.recv(4096)
                    if not chunk:
                        break
                    response += chunk
                    if b"\n" in chunk:
                        break
                except socket.timeout:
                    break

            return (True, response, None)

        except socket.timeout:
            return (False, None, "TIMEOUT")
        except ConnectionRefusedError:
            return (False, None, "CONNECTION_REFUSED")
        except BrokenPipeError:
            return (False, None, "BROKEN_PIPE")
        except Exception as e:
            return (False, None, str(e))
        finally:
            if sock:
                try:
                    sock.close()
                except:
                    pass

    def fuzz_json_structure(self):
        """Fuzz JSON structure with malformed syntax"""
        print("\n[*] Fuzzing JSON structure...")

        test_cases = [
            # Missing fields
            b'{"type":"command"}\n',
            b'{"id":"test"}\n',
            b'{"command":"api.getCommands"}\n',
            # Duplicate fields
            b'{"type":"command","type":"batch","id":"x","command":"api.getCommands"}\n',
            b'{"type":"command","id":"1","id":"2","command":"test"}\n',
            # Wrong types
            b'{"type":123,"id":"x","command":"test"}\n',
            b'{"type":"command","id":null,"command":"test"}\n',
            b'{"type":"command","id":"x","command":["array"]}\n',
            b'{"type":"command","id":"x","params":"not_an_object"}\n',
            # Truncated JSON
            b'{"type":"command","id":"x",\n',
            b'{"type":"command"\n',
            b'{"type":\n',
            b'{"\n',
            b'{\n',
            # Extra commas
            b'{"type":"command",,,"id":"x"}\n',
            b'{"type":"command","id":"x",}\n',
            # Comments (not valid in JSON)
            b'{"type":"command", /* comment */ "id":"x"}\n',
            b'{"type":"command", // comment\n"id":"x"}\n',
            # Single quotes (not valid in JSON)
            b"{'type':'command','id':'x'}\n",
            # Unescaped control characters
            b'{"type":"command\t\r\n","id":"x"}\n',
            b'{"type":"command\x00","id":"x"}\n',
            # Very long field names
            b'{"' + b"x" * 100000 + b'":"value"}\n',
            # Very long string values
            b'{"type":"command","id":"' + b"y" * 100000 + b'"}\n',
            # Floating point edge cases
            b'{"type":"command","id":"x","params":{"val":1e308}}\n',  # Near max float
            b'{"type":"command","id":"x","params":{"val":-1e308}}\n',
            b'{"type":"command","id":"x","params":{"val":1e-308}}\n',  # Near min float
            b'{"type":"command","id":"x","params":{"val":Infinity}}\n',
            b'{"type":"command","id":"x","params":{"val":NaN}}\n',
            # Unicode edge cases
            b'{"type":"command","id":"\xed\xa0\x80"}\n',  # Surrogate half
            b'{"type":"command","id":"\xf4\x90\x80\x80"}\n',  # Invalid UTF-8
            # BOMs (Byte Order Marks)
            b'\xef\xbb\xbf{"type":"command","id":"x"}\n',  # UTF-8 BOM
            b'\xff\xfe{"type":"command","id":"x"}\n',  # UTF-16 LE BOM
            # Empty variations
            b"{}\n",
            b'{"type":""}\n',
            b'{"type":"","id":"","command":""}\n',
        ]

        for i, payload in enumerate(test_cases):
            success, response, error = self.send_raw(payload, timeout=1.0)

            if error == "TIMEOUT":
                self.hangs.append(("JSON Structure", payload))
                print(f"  [HANG] Test case {i+1}: {repr(payload[:50])}")
            elif error and "REFUSED" not in error:
                self.crashes.append(("JSON Structure", payload, error))
                print(f"  [CRASH] Test case {i+1}: {error}")
            elif success and response and b"error" not in response.lower():
                self.unexpected.append(("JSON Structure", payload, response))
                print(f"  [UNEXPECTED] Test case {i+1}: {repr(response[:100])}")

    def fuzz_field_values(self):
        """Fuzz with boundary values in JSON fields"""
        print("\n[*] Fuzzing field values...")

        boundary_values = [
            0,
            -1,
            1,
            2147483647,  # Max int32
            -2147483648,  # Min int32
            9223372036854775807,  # Max int64
            -9223372036854775808,  # Min int64
            "",
            " ",
            "null",
            "undefined",
            "true",
            "false",
            "\x00",
            "\n",
            "\r\n",
            "\\",
            '"',
            "'",
            "../../../etc/passwd",
            "C:\\Windows\\System32",
            "<?xml version='1.0'?>",
            "<script>alert(1)</script>",
            "${USER}",
            "$(whoami)",
            "`id`",
        ]

        for value in boundary_values:
            payload = {
                "type": "command",
                "id": value if isinstance(value, str) else str(value),
                "command": "api.getCommands",
            }

            try:
                data = json.dumps(payload).encode() + b"\n"
                success, response, error = self.send_raw(data, timeout=1.0)

                if error == "TIMEOUT":
                    self.hangs.append(("Field Values", payload))
                    print(f"  [HANG] Value: {repr(value)}")
            except Exception as e:
                print(f"  [ERROR] Encoding failed for {repr(value)}: {e}")

    def fuzz_protocol_violations(self):
        """Test protocol-level violations"""
        print("\n[*] Fuzzing protocol violations...")

        # Test 1: Messages without newline terminators
        print("  - Testing messages without newlines...")
        success, response, error = self.send_raw(
            b'{"type":"command","id":"x","command":"api.getCommands"}', timeout=3.0
        )
        if error == "TIMEOUT":
            self.hangs.append(("Protocol", "No newline"))
            print("    [HANG] Server waiting for newline")

        # Test 2: Multiple newlines
        print("  - Testing multiple consecutive newlines...")
        success, response, error = self.send_raw(b"\n\n\n\n\n\n\n\n\n\n", timeout=1.0)

        # Test 3: Mixed line endings
        print("  - Testing mixed line endings...")
        payloads = [
            b'{"type":"command","id":"1","command":"api.getCommands"}\r\n',
            b'{"type":"command","id":"2","command":"api.getCommands"}\r',
            b'{"type":"command","id":"3","command":"api.getCommands"}\n\r',
            b'{"type":"command","id":"4","command":"api.getCommands"}\r\r\n',
        ]
        for payload in payloads:
            self.send_raw(payload, timeout=1.0)

        # Test 4: Pipelined requests (multiple messages at once)
        print("  - Testing pipelined requests...")
        pipelined = b""
        for i in range(100):
            msg = {
                "type": "command",
                "id": f"pipe-{i}",
                "command": "api.getCommands",
            }
            pipelined += json.dumps(msg).encode() + b"\n"

        success, response, error = self.send_raw(pipelined, timeout=5.0)
        if error == "TIMEOUT":
            self.hangs.append(("Protocol", "Pipelined requests"))
            print("    [HANG] Server stalled on pipelined requests")

        # Test 5: Interleaved JSON and raw data
        print("  - Testing interleaved JSON and raw data...")
        interleaved = b'{"type":"command","id":"1","command":"api.getCommands"}\n'
        interleaved += b"RANDOM RAW DATA HERE\n"
        interleaved += b'{"type":"command","id":"2","command":"api.getCommands"}\n'
        self.send_raw(interleaved, timeout=2.0)

    def fuzz_batch_commands(self):
        """Fuzz batch command handling"""
        print("\n[*] Fuzzing batch commands...")

        # Test 1: Empty batch
        empty_batch = {
            "type": "batch",
            "id": str(uuid.uuid4()),
            "commands": [],
        }
        self.send_raw(json.dumps(empty_batch).encode() + b"\n")

        # Test 2: Batch with invalid commands
        invalid_batch = {
            "type": "batch",
            "id": str(uuid.uuid4()),
            "commands": [
                {"id": "1"},  # Missing command field
                {"command": ""},  # Empty command
                {"command": None},  # Null command
                {"command": 123},  # Wrong type
            ],
        }
        self.send_raw(json.dumps(invalid_batch).encode() + b"\n")

        # Test 3: Deeply nested batch
        nested = {
            "type": "batch",
            "id": str(uuid.uuid4()),
            "commands": [
                {
                    "id": f"cmd-{i}",
                    "command": "project.loadFromJSON",
                    "params": {
                        "config": {"nested": {"level": i}},
                    },
                }
                for i in range(1000)
            ],
        }
        success, response, error = self.send_raw(
            json.dumps(nested).encode() + b"\n", timeout=10.0
        )
        if error == "TIMEOUT":
            self.hangs.append(("Batch", "1000 nested commands"))
            print("  [HANG] Large batch caused timeout")

    def fuzz_raw_message_type(self):
        """Fuzz raw message type handling"""
        print("\n[*] Fuzzing raw message type...")

        # Test 1: Raw message without data field
        no_data = {
            "type": "raw",
            "id": str(uuid.uuid4()),
        }
        self.send_raw(json.dumps(no_data).encode() + b"\n")

        # Test 2: Raw message with invalid base64
        invalid_b64 = {
            "type": "raw",
            "id": str(uuid.uuid4()),
            "data": "!!!INVALID_BASE64!!!",
        }
        self.send_raw(json.dumps(invalid_b64).encode() + b"\n")

        # Test 3: Raw message with massive base64 data
        import base64

        huge_data = base64.b64encode(b"\x00" * (10 * 1024 * 1024)).decode()  # 10MB
        huge_raw = {
            "type": "raw",
            "id": str(uuid.uuid4()),
            "data": huge_data,
        }
        success, response, error = self.send_raw(
            json.dumps(huge_raw).encode() + b"\n", timeout=5.0
        )
        if error == "TIMEOUT":
            self.hangs.append(("Raw Type", "10MB base64 data"))
            print("  [HANG] Huge raw data caused timeout")

    def fuzz_random_data(self, iterations=100):
        """Send completely random binary data"""
        print(f"\n[*] Fuzzing with random binary data ({iterations} iterations)...")

        for i in range(iterations):
            # Generate random binary data
            length = random.randint(1, 10000)
            random_bytes = bytes(random.randint(0, 255) for _ in range(length))

            success, response, error = self.send_raw(random_bytes, timeout=1.0)

            if error == "TIMEOUT":
                self.hangs.append(("Random Data", random_bytes[:100]))
                print(f"  [HANG] Iteration {i+1}: {repr(random_bytes[:50])}")
            elif error and "REFUSED" not in error and "PIPE" not in error:
                self.crashes.append(("Random Data", random_bytes[:100], error))
                print(f"  [CRASH] Iteration {i+1}: {error}")

    def fuzz_command_names(self):
        """Fuzz command name handling"""
        print("\n[*] Fuzzing command names...")

        # Generate variations of command names
        base_commands = ["api.getCommands", "io.manager.connect", "project.setTitle"]

        mutations = []

        for cmd in base_commands:
            # Case variations
            mutations.append(cmd.upper())
            mutations.append(cmd.lower())
            mutations.append(cmd.title())

            # Dot variations
            mutations.append(cmd.replace(".", ".."))
            mutations.append(cmd.replace(".", ""))
            mutations.append(cmd + ".")
            mutations.append("." + cmd)

            # Injection attempts
            mutations.append(cmd + "; DROP TABLE commands")
            mutations.append(cmd + "\x00")
            mutations.append(cmd + "\n")

            # Length variations
            mutations.append(cmd * 100)
            mutations.append("x" * 10000)

        for cmd_name in mutations:
            payload = {
                "type": "command",
                "id": str(uuid.uuid4()),
                "command": cmd_name,
            }
            self.send_raw(json.dumps(payload).encode() + b"\n", timeout=1.0)

    def report(self):
        """Generate fuzzing report"""
        print("\n" + "=" * 80)
        print("PROTOCOL FUZZING REPORT")
        print("=" * 80)

        print(f"\nCrashes found: {len(self.crashes)}")
        for category, payload, error in self.crashes[:10]:  # Show first 10
            print(f"  [{category}] {error}")
            print(f"    Payload: {repr(payload[:100])}")

        print(f"\nHangs/Timeouts found: {len(self.hangs)}")
        for category, payload in self.hangs[:10]:
            print(f"  [{category}]")
            if isinstance(payload, bytes):
                print(f"    Payload: {repr(payload[:100])}")
            else:
                print(f"    Payload: {repr(str(payload)[:100])}")

        print(f"\nUnexpected responses: {len(self.unexpected)}")
        for category, payload, response in self.unexpected[:10]:
            print(f"  [{category}]")
            print(f"    Payload: {repr(payload[:50])}")
            print(f"    Response: {repr(response[:50])}")

        print("\n" + "=" * 80)


def main():
    """Run protocol fuzzing tests"""
    print("=" * 80)
    print("Serial Studio Protocol Fuzzing Suite")
    print("=" * 80)
    print("\nThis will send malformed data to find crashes and hangs.")
    print()

    fuzzer = ProtocolFuzzer()

    try:
        # Check server connectivity
        success, _, error = fuzzer.send_raw(
            b'{"type":"command","id":"test","command":"api.getCommands"}\n'
        )
        if not success:
            print(f"[ERROR] Cannot connect to API: {error}")
            return 1

        print("[+] Connected to Serial Studio API\n")

        # Run fuzzing campaigns
        fuzzer.fuzz_json_structure()
        fuzzer.fuzz_field_values()
        fuzzer.fuzz_protocol_violations()
        fuzzer.fuzz_batch_commands()
        fuzzer.fuzz_raw_message_type()
        fuzzer.fuzz_command_names()
        fuzzer.fuzz_random_data(iterations=50)

    except KeyboardInterrupt:
        print("\n\n[!] Fuzzing interrupted by user")

    finally:
        fuzzer.report()

    return 0


if __name__ == "__main__":
    exit(main())
