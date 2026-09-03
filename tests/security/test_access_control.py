#!/usr/bin/env python3
"""
Authentication and Authorization Bypass Test Suite

Tests for authentication weaknesses, authorization bypasses, and
privilege escalation weaknesses in the Serial Studio API.

Probe categories:
- No authentication testing
- Authorization bypass attempts
- Privilege escalation
- Session manipulation
- API key bypass
- CORS and origin validation

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import uuid
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError


class AuthBypassTester:
    """Authentication and authorization bypass testing"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.weaknesses = []

    def log_weakness(self, severity, category, description):
        """Log a discovered weakness"""
        finding = {
            "severity": severity,
            "category": category,
            "description": description,
        }
        self.weaknesses.append(finding)
        print(f"[{severity}] {category}: {description}")


def test_no_authentication(tester):
    """Test if API requires authentication"""
    print("\n[*] Testing authentication requirements...")

    # Test 1: Direct connection without credentials
    print("  - Testing unauthenticated access...")
    try:
        with SerialStudioClient() as client:
            result = client.command("api.getCommands")

            tester.log_weakness(
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
                tester.log_weakness(
                    "CRITICAL",
                    "Authorization",
                    f"Unauthenticated access to {len(sensitive_commands)} sensitive commands",
                )
                print(f"    Sensitive commands exposed: {len(sensitive_commands)}")
                for cmd in sensitive_commands[:10]:
                    print(f"      - {cmd}")

    except Exception as e:
        print(f"    Authentication may be required: {e}")


def test_origin_validation(tester):
    """Test origin/referrer validation"""
    print("\n[*] Testing origin validation...")

    # Test 1: Send requests with fake origins
    print("  - Testing origin header validation...")

    untrusted_origins = [
        "http://client.com",
        "http://evil.example.com",
        "http://192.168.1.100",
        "file:///etc/passwd",
    ]

    for origin in untrusted_origins:
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
        ("io.connect", {}, "Connect to device"),
        ("io.disconnect", {}, "Disconnect device"),
        ("csvExport.setEnabled", {"enabled": True}, "Enable CSV export"),
        ("project.open", {"filePath": "/tmp/test.json"}, "Open file"),
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
        tester.log_weakness(
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
            untrusted_data = b"INJECTED_DATA\r\n"
            encoded = base64.b64encode(untrusted_data).decode()

            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))

            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": encoded}

            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(4096)
            sock.close()

            # The server rejects the unknown "raw" message type, but its error
            # response still carries "success": false — a substring match on
            # b"success" therefore false-positives. Parse the reply and treat
            # only a genuine success/bytesWritten as an accepted injection.
            try:
                parsed = json.loads(response.decode())
            except (ValueError, UnicodeDecodeError):
                parsed = {}

            if parsed.get("success") is True or "bytesWritten" in parsed:
                tester.log_weakness(
                    "HIGH",
                    "Data Injection",
                    "Unauthenticated raw data injection to device",
                )
                print("    Successfully injected raw data")
                pytest.fail("Unauthenticated raw data injection to device succeeded")

    except Exception as e:
        print(f"    Data injection blocked or failed: {e}")


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
                    tester.log_weakness(
                        "CRITICAL",
                        "Path Traversal",
                        f"Successfully loaded file: {path}",
                    )
                    pytest.fail(f"Successfully loaded file outside project dir: {path}")
                except APIError as e:
                    # Good - should be blocked
                    pass

    except Exception as e:
        print(f"    File loading test error: {e}")

    # Test 2: Create untrusted project
    print("  - Testing untrusted project creation...")

    try:
        with SerialStudioClient() as client:
            untrusted_project = {
                "title": "<script>alert('XSS')</script>",
                "frameParser": {
                    "startSequence": "'; DROP TABLE datasets; --",
                    "endSequence": "\x00\x00\x00",
                },
                "groups": [],
            }

            try:
                result = client.load_project_from_json(untrusted_project)
                print("    Loaded project with untrusted content")
                # Check if it's properly sanitized
            except APIError as e:
                print(f"    Untrusted project rejected: {e}")

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
            status = client.command("io.getStatus")

            # Check for sensitive info
            sensitive_keys = ["path", "file", "directory", "user", "home", "system"]

            leaked_info = []
            for key in status.keys():
                if any(s in key.lower() for s in sensitive_keys):
                    leaked_info.append(key)

            if leaked_info:
                tester.log_weakness(
                    "LOW",
                    "Information Disclosure",
                    f"Status command leaks {len(leaked_info)} potentially sensitive fields",
                )
                pytest.fail(
                    f"Status command leaks potentially sensitive fields: {leaked_info}"
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
                    tester.log_weakness(
                        "LOW",
                        "Information Disclosure",
                        f"Error messages leak internal details: {e.message[:100]}",
                    )
                    pytest.fail(
                        f"Error message leaks internal details: {e.message[:100]}"
                    )

    except Exception as e:
        print(f"    Error message test error: {e}")


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
        test_origin_validation(tester)
        test_command_authorization(tester)
        test_data_injection(tester)
        test_project_manipulation(tester)
        test_information_disclosure(tester)

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

        print(f"\nTotal issues found: {len(tester.weaknesses)}")

        by_severity = {}
        for finding in tester.weaknesses:
            sev = finding["severity"]
            by_severity[sev] = by_severity.get(sev, 0) + 1

        for severity in ["CRITICAL", "HIGH", "MEDIUM", "LOW"]:
            count = by_severity.get(severity, 0)
            if count > 0:
                print(f"  {severity}: {count}")

        print("\nDetailed findings:")
        for i, finding in enumerate(tester.weaknesses, 1):
            print(f"{i}. [{finding['severity']}] {finding['category']}")
            print(f"   {finding['description']}")

        print("\n" + "=" * 80)

    return 0


if __name__ == "__main__":
    exit(main())
