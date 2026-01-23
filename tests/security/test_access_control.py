#!/usr/bin/env python3
"""
Authentication and Authorization Bypass Test Suite

Tests for authentication weaknesses, authorization bypasses, and
privilege escalation vulnerabilities in the Serial Studio API.

Attack categories:
- No authentication testing
- Authorization bypass attempts
- Privilege escalation
- Session manipulation
- API key bypass
- CORS and origin validation

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import uuid
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError


class AuthBypassTester:
    """Authentication and authorization bypass testing"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.vulnerabilities = []

    def log_vulnerability(self, severity, category, description):
        """Log a discovered vulnerability"""
        vuln = {"severity": severity, "category": category, "description": description}
        self.vulnerabilities.append(vuln)
        print(f"[{severity}] {category}: {description}")


def test_no_authentication(tester):
    """Test if API requires authentication"""
    print("\n[*] Testing authentication requirements...")

    # Test 1: Direct connection without credentials
    print("  - Testing unauthenticated access...")
    try:
        with SerialStudioClient() as client:
            result = client.command("api.getCommands")

            tester.log_vulnerability(
                "HIGH",
                "No Authentication",
                "API accepts connections without any authentication",
            )

            # List available commands
            commands = result.get("commands", [])
            print(f"    Accessible commands: {len(commands)}")

            # Check for sensitive commands
            sensitive_commands = []
            for cmd in commands:
                cmd_name = cmd.get("name", "")
                if any(
                    keyword in cmd_name.lower()
                    for keyword in [
                        "disconnect",
                        "connect",
                        "open",
                        "close",
                        "write",
                        "set",
                        "delete",
                        "remove",
                    ]
                ):
                    sensitive_commands.append(cmd_name)

            if sensitive_commands:
                tester.log_vulnerability(
                    "CRITICAL",
                    "Authorization",
                    f"Unauthenticated access to {len(sensitive_commands)} sensitive commands",
                )
                print(f"    Sensitive commands exposed: {len(sensitive_commands)}")
                for cmd in sensitive_commands[:10]:
                    print(f"      - {cmd}")

    except Exception as e:
        print(f"    Authentication may be required: {e}")


def test_network_binding(tester):
    """Test network binding restrictions"""
    print("\n[*] Testing network binding...")

    # Test 1: Check if bound to localhost only
    print("  - Checking bind address...")
    try:
        # Try to connect from localhost
        with SerialStudioClient("127.0.0.1", 7777) as client:
            client.command("api.getCommands")
            print("    Accessible from 127.0.0.1 (localhost)")

        # Try to connect from 0.0.0.0 (would work if bound to all interfaces)
        try:
            with SerialStudioClient("0.0.0.0", 7777) as client:
                client.command("api.getCommands")
                tester.log_vulnerability(
                    "CRITICAL",
                    "Network Exposure",
                    "API accessible from all network interfaces (0.0.0.0)",
                )
        except:
            print("    Not accessible from 0.0.0.0 (GOOD)")

    except Exception as e:
        print(f"    Error: {e}")


def test_origin_validation(tester):
    """Test origin/referrer validation"""
    print("\n[*] Testing origin validation...")

    # Test 1: Send requests with fake origins
    print("  - Testing origin header validation...")

    malicious_origins = [
        "http://attacker.com",
        "http://evil.example.com",
        "http://192.168.1.100",
        "file:///etc/passwd",
    ]

    for origin in malicious_origins:
        try:
            # TCP doesn't have HTTP headers, but test at protocol level
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))

            # Send command (would work if no origin checking)
            msg = {
                "type": "command",
                "id": str(uuid.uuid4()),
                "command": "api.getCommands",
            }
            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(4096)
            sock.close()

            if b"success" in response:
                print(f"    Accepted connection (no origin validation at TCP level)")
                break

        except Exception as e:
            pass


def test_command_authorization(tester):
    """Test command-level authorization"""
    print("\n[*] Testing command authorization...")

    # Test 1: Attempt privileged operations
    print("  - Testing privileged operations...")

    privileged_operations = [
        ("io.manager.connect", {}, "Connect to device"),
        ("io.manager.disconnect", {}, "Disconnect device"),
        ("csv.export.setEnabled", {"enabled": True}, "Enable CSV export"),
        ("project.file.open", {"filePath": "/tmp/test.json"}, "Open file"),
    ]

    accessible_privileged = []

    for cmd, params, description in privileged_operations:
        try:
            with SerialStudioClient() as client:
                result = client.command(cmd, params)
                accessible_privileged.append((cmd, description))
                print(f"    [ACCESSIBLE] {cmd}: {description}")
        except APIError as e:
            # Expected - should require authorization
            pass

    if accessible_privileged:
        tester.log_vulnerability(
            "CRITICAL",
            "Authorization Bypass",
            f"Unauthenticated access to {len(accessible_privileged)} privileged commands",
        )


def test_data_injection(tester):
    """Test data injection via API"""
    print("\n[*] Testing data injection capabilities...")

    # Test 1: Check if we can inject raw data to device
    print("  - Testing raw data injection...")

    try:
        import base64

        with SerialStudioClient() as client:
            # Try to configure network driver
            client.configure_network(host="127.0.0.1", port=9999, socket_type="tcp")

            # Try to send raw data
            malicious_data = b"INJECTED_DATA\r\n"
            encoded = base64.b64encode(malicious_data).decode()

            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))

            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": encoded}

            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(4096)
            sock.close()

            if b"success" in response or b"bytesWritten" in response:
                tester.log_vulnerability(
                    "HIGH",
                    "Data Injection",
                    "Unauthenticated raw data injection to device",
                )
                print("    Successfully injected raw data")

    except Exception as e:
        print(f"    Data injection blocked or failed: {e}")


def test_configuration_tampering(tester):
    """Test configuration tampering"""
    print("\n[*] Testing configuration tampering...")

    # Test 1: Modify dashboard settings
    print("  - Testing dashboard configuration modification...")

    try:
        with SerialStudioClient() as client:
            # Get current FPS
            status = client.get_dashboard_status()
            original_fps = status.get("fps", 24)

            # Try to modify
            client.set_dashboard_fps(1)

            # Verify modification
            new_status = client.get_dashboard_status()
            new_fps = new_status.get("fps", 24)

            if new_fps == 1:
                tester.log_vulnerability(
                    "MEDIUM",
                    "Configuration Tampering",
                    "Unauthenticated modification of dashboard settings",
                )
                print("    Successfully modified dashboard FPS")

                # Restore original
                client.set_dashboard_fps(original_fps)

    except Exception as e:
        print(f"    Configuration tampering blocked: {e}")

    # Test 2: Modify frame parser settings
    print("  - Testing frame parser modification...")

    try:
        with SerialStudioClient() as client:
            # Try to modify critical parsing settings
            result = client.configure_frame_parser(
                start_sequence="HACKED", end_sequence="OWNED", operation_mode=1
            )

            tester.log_vulnerability(
                "HIGH",
                "Configuration Tampering",
                "Unauthenticated modification of frame parser settings",
            )
            print("    Successfully modified frame parser")

    except Exception as e:
        print(f"    Frame parser modification blocked: {e}")


def test_project_manipulation(tester):
    """Test project file manipulation"""
    print("\n[*] Testing project manipulation...")

    # Test 1: Load arbitrary project files
    print("  - Testing arbitrary file loading...")

    try:
        with SerialStudioClient() as client:
            # Try to load files outside project directory
            dangerous_paths = [
                "/etc/passwd",
                "C:\\Windows\\System32\\config\\SAM",
                "../../../sensitive.json",
            ]

            for path in dangerous_paths:
                try:
                    client.load_project(path)
                    tester.log_vulnerability(
                        "CRITICAL",
                        "Path Traversal",
                        f"Successfully loaded file: {path}",
                    )
                except APIError as e:
                    # Good - should be blocked
                    pass

    except Exception as e:
        print(f"    File loading test error: {e}")

    # Test 2: Create malicious project
    print("  - Testing malicious project creation...")

    try:
        with SerialStudioClient() as client:
            malicious_project = {
                "title": "<script>alert('XSS')</script>",
                "frameParser": {
                    "startSequence": "'; DROP TABLE datasets; --",
                    "endSequence": "\x00\x00\x00",
                },
                "groups": [],
            }

            try:
                result = client.load_project_from_json(malicious_project)
                print("    Loaded project with malicious content")
                # Check if it's properly sanitized
            except APIError as e:
                print(f"    Malicious project rejected: {e}")

    except Exception as e:
        print(f"    Project creation test error: {e}")


def test_information_disclosure(tester):
    """Test information disclosure via API"""
    print("\n[*] Testing information disclosure...")

    # Test 1: Extract system information
    print("  - Testing system information extraction...")

    try:
        with SerialStudioClient() as client:
            # Get IO manager status
            status = client.command("io.manager.getStatus")

            # Check for sensitive info
            sensitive_keys = ["path", "file", "directory", "user", "home", "system"]

            leaked_info = []
            for key in status.keys():
                if any(s in key.lower() for s in sensitive_keys):
                    leaked_info.append(key)

            if leaked_info:
                tester.log_vulnerability(
                    "LOW",
                    "Information Disclosure",
                    f"Status command leaks {len(leaked_info)} potentially sensitive fields",
                )

    except Exception as e:
        print(f"    Status query error: {e}")

    # Test 2: Error message information leakage
    print("  - Testing error message information leakage...")

    try:
        with SerialStudioClient() as client:
            # Trigger various errors and check messages
            try:
                client.command("nonexistent.command.that.does.not.exist.at.all")
            except APIError as e:
                # Check if error reveals internal details
                error_msg = e.message.lower()
                if any(
                    leak in error_msg
                    for leak in ["path", "line", "file", "stack", "trace", "version"]
                ):
                    tester.log_vulnerability(
                        "LOW",
                        "Information Disclosure",
                        f"Error messages leak internal details: {e.message[:100]}",
                    )

    except Exception as e:
        print(f"    Error message test error: {e}")


def test_cross_client_interference(tester):
    """Test cross-client interference"""
    print("\n[*] Testing cross-client interference...")

    # Test 1: Check if one client can affect another
    print("  - Testing client isolation...")

    try:
        client1 = SerialStudioClient()
        client2 = SerialStudioClient()

        client1.connect()
        client2.connect()

        # Client 1 sets configuration
        client1.set_dashboard_fps(10)

        # Client 2 tries to read it
        status = client2.get_dashboard_status()
        fps = status.get("fps", 0)

        if fps == 10:
            tester.log_vulnerability(
                "MEDIUM",
                "Client Isolation",
                "No client isolation - shared global state",
            )
            print("    Clients share global state (no isolation)")
        else:
            print("    Clients appear to be isolated (GOOD)")

        client1.disconnect()
        client2.disconnect()

    except Exception as e:
        print(f"    Client isolation test error: {e}")


def main():
    """Run authentication and authorization tests"""
    print("=" * 80)
    print("Serial Studio Authentication & Authorization Test Suite")
    print("=" * 80)
    print("\nTesting for authentication bypasses and authorization flaws.")
    print()

    tester = AuthBypassTester()

    try:
        # Check server connectivity
        with SerialStudioClient() as client:
            print("[+] Connected to Serial Studio API\n")

        # Run authentication tests
        test_no_authentication(tester)
        test_network_binding(tester)
        test_origin_validation(tester)
        test_command_authorization(tester)
        test_data_injection(tester)
        test_configuration_tampering(tester)
        test_project_manipulation(tester)
        test_information_disclosure(tester)
        test_cross_client_interference(tester)

    except ConnectionError as e:
        print(f"[ERROR] Cannot connect to Serial Studio API: {e}")
        print("Make sure Serial Studio is running with API Server enabled.")
        return 1

    except KeyboardInterrupt:
        print("\n\n[!] Tests interrupted by user")

    finally:
        # Generate report
        print("\n" + "=" * 80)
        print("AUTHENTICATION & AUTHORIZATION REPORT")
        print("=" * 80)

        print(f"\nTotal issues found: {len(tester.vulnerabilities)}")

        by_severity = {}
        for vuln in tester.vulnerabilities:
            sev = vuln["severity"]
            by_severity[sev] = by_severity.get(sev, 0) + 1

        for severity in ["CRITICAL", "HIGH", "MEDIUM", "LOW"]:
            count = by_severity.get(severity, 0)
            if count > 0:
                print(f"  {severity}: {count}")

        print("\nDetailed findings:")
        for i, vuln in enumerate(tester.vulnerabilities, 1):
            print(f"{i}. [{vuln['severity']}] {vuln['category']}")
            print(f"   {vuln['description']}")

        print("\n" + "=" * 80)

    return 0


if __name__ == "__main__":
    exit(main())
