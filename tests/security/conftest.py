"""
Pytest Configuration for Security Tests

Fixtures and configuration for Serial Studio security testing suite.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils.api_client import SerialStudioClient, APIError


class VulnerabilityTracker:
    """Track discovered vulnerabilities during security tests"""

    def __init__(self):
        self.vulnerabilities = []
        self.exploits = []
        self.crashes = []

    def log_vulnerability(self, severity, category, description, payload=None):
        """Log a discovered vulnerability"""
        vuln = {
            "severity": severity,
            "category": category,
            "description": description,
            "payload": payload,
        }
        self.vulnerabilities.append(vuln)

    def log_exploit(self, name, success, details):
        """Log an exploitation attempt"""
        exploit = {"name": name, "success": success, "details": details}
        self.exploits.append(exploit)

    def log_crash(self, category, payload, error):
        """Log a server crash"""
        crash = {"category": category, "payload": payload, "error": error}
        self.crashes.append(crash)

    def get_summary(self):
        """Get vulnerability summary"""
        return {
            "total_vulnerabilities": len(self.vulnerabilities),
            "total_exploits": len(self.exploits),
            "total_crashes": len(self.crashes),
            "successful_exploits": len(
                [e for e in self.exploits if e["success"]]
            ),
            "by_severity": self._count_by_severity(),
        }

    def _count_by_severity(self):
        """Count vulnerabilities by severity"""
        counts = {}
        for vuln in self.vulnerabilities:
            severity = vuln["severity"]
            counts[severity] = counts.get(severity, 0) + 1
        return counts


@pytest.fixture(scope="session")
def api_server_required():
    """
    Verify Serial Studio API server is running.

    Security tests MUST have the API server enabled.
    """
    try:
        client = SerialStudioClient(timeout=2.0)
        client.connect()
        client.disconnect()
        return True
    except ConnectionError:
        pytest.fail(
            "\n\n"
            "=" * 70 + "\n"
            "Serial Studio API Server is NOT running!\n\n"
            "Security tests require the API server to be enabled.\n\n"
            "Please:\n"
            "  1. Start Serial Studio\n"
            "  2. Enable API Server (Settings → API → Enable)\n"
            "  3. Verify it's listening on localhost:7777\n"
            "=" * 70
        )


@pytest.fixture
def security_client(api_server_required):
    """
    Provide an API client for security testing.

    Does NOT automatically clean state - security tests may want
    to test dirty/invalid states.

    Skips test if server is unavailable (e.g., crashed by previous test).
    """
    client = SerialStudioClient(timeout=5.0)

    try:
        client.connect()
    except ConnectionError as e:
        pytest.skip(
            f"Server unavailable (may have crashed from previous test): {e}"
        )

    yield client

    # Cleanup - but don't fail if server is unresponsive
    try:
        client.disconnect()
    except Exception:
        pass


@pytest.fixture
def vuln_tracker():
    """Provide a vulnerability tracker for test session"""
    return VulnerabilityTracker()


@pytest.fixture
def check_server_alive(api_server_required):
    """
    Check if server is still alive after a test.

    Use this fixture to verify the server didn't crash.

    IMPORTANT: This checks if server is RESPONSIVE, not just if connection
    was closed. Defensive connection closures are GOOD security behavior,
    not crashes!
    """

    def _check(wait_time=1.0):
        """
        Try to establish a NEW connection and execute a simple command.

        Args:
            wait_time: Time to wait before checking (allows server to recover)

        Returns:
            True if server is alive and responsive
            False if server crashed or is unresponsive
        """
        # Wait for server to recover from rate limiting / stress
        time.sleep(wait_time)

        # Try up to 3 times with increasing delays
        for attempt in range(3):
            try:
                client = SerialStudioClient(timeout=5.0)
                client.connect()

                # Execute simple command
                result = client.command("api.getCommands")

                client.disconnect()

                # Verify we got a valid response
                if isinstance(result, dict) or isinstance(result, list):
                    return True

            except ConnectionRefusedError:
                # Server not accepting connections - likely crashed
                if attempt == 2:  # Last attempt
                    return False
                time.sleep(2.0 * (attempt + 1))  # Exponential backoff

            except (ConnectionError, TimeoutError, Exception) as e:
                # Other errors might be rate limiting or temp issues
                if attempt == 2:  # Last attempt
                    return False
                time.sleep(2.0 * (attempt + 1))  # Exponential backoff

        return False

    return _check


@pytest.fixture(autouse=True)
def security_test_warning(request):
    """
    Display warning before destructive security tests.

    Only shown for tests marked as 'destructive'.
    """
    if "destructive" in [mark.name for mark in request.node.iter_markers()]:
        print(
            "\n⚠️  WARNING: This test may crash or hang the server!\n"
        )
        # Add delay before destructive tests to let server recover
        time.sleep(2.0)


@pytest.fixture(autouse=True)
def test_isolation(request):
    """
    Add delays between tests to prevent overwhelming the server.

    This helps prevent cascading failures where one stress test
    causes the next test to fail due to rate limiting or resource exhaustion.
    """
    yield  # Run the test

    # Add delay after each test
    time.sleep(0.5)

    # Longer delay after stress/destructive tests
    if any(mark.name in ["destructive", "dos", "exploit"] for mark in request.node.iter_markers()):
        time.sleep(2.0)


def pytest_configure(config):
    """Configure pytest with security-specific markers"""
    config.addinivalue_line(
        "markers", "security: security and penetration tests"
    )
    config.addinivalue_line(
        "markers",
        "exploit: active exploitation attempts (may crash server)",
    )
    config.addinivalue_line("markers", "dos: denial of service attacks")
    config.addinivalue_line("markers", "fuzzing: fuzzing tests")
    config.addinivalue_line(
        "markers", "critical: tests for critical vulnerabilities"
    )
    config.addinivalue_line(
        "markers", "high: tests for high severity vulnerabilities"
    )
    config.addinivalue_line(
        "markers", "medium: tests for medium severity vulnerabilities"
    )
    config.addinivalue_line(
        "markers",
        "destructive: tests that may crash or hang the server (run with caution)",
    )


def pytest_collection_modifyitems(config, items):
    """
    Automatically mark security tests and handle test ordering.
    """
    for item in items:
        # Auto-mark all tests in security directory
        if "security" in str(item.fspath):
            item.add_marker(pytest.mark.security)

        # Mark by test function name patterns
        test_name = item.name.lower()

        if any(
            pattern in test_name
            for pattern in ["dos", "flood", "exhaust", "bomb"]
        ):
            item.add_marker(pytest.mark.dos)
            item.add_marker(pytest.mark.destructive)

        if any(
            pattern in test_name
            for pattern in ["fuzz", "random", "malformed"]
        ):
            item.add_marker(pytest.mark.fuzzing)

        if any(
            pattern in test_name
            for pattern in [
                "exploit",
                "weaponized",
                "race",
                "overflow",
                "corruption",
            ]
        ):
            item.add_marker(pytest.mark.exploit)
            item.add_marker(pytest.mark.destructive)

        # Mark slow tests
        if any(
            pattern in test_name
            for pattern in [
                "slowloris",
                "timing",
                "persistence",
                "connection_exhaustion",
            ]
        ):
            item.add_marker(pytest.mark.slow)


def pytest_report_header(config):
    """Add security test suite header to pytest output"""
    if config.getoption("verbose") > 0:
        return [
            "",
            "Serial Studio Security Test Suite",
            "⚠️  These tests actively attempt to exploit vulnerabilities",
            "=" * 70,
        ]


@pytest.fixture
def exploiter(api_server_required):
    """
    Provide an AdvancedExploiter instance for exploit technique tests.

    Used by test_exploit_techniques.py
    """
    from security.test_exploit_techniques import AdvancedExploiter

    return AdvancedExploiter()


@pytest.fixture
def tester(request, api_server_required):
    """
    Provide the appropriate Tester instance for security tests.

    Dynamically instantiates the correct tester class based on which
    test module is requesting the fixture.
    """
    module_name = request.module.__name__

    if "test_api_vulnerabilities" in module_name:
        from security.test_api_vulnerabilities import SecurityTester

        return SecurityTester()
    elif "test_access_control" in module_name:
        from security.test_access_control import AuthBypassTester

        return AuthBypassTester()
    elif "test_denial_of_service" in module_name:
        from security.test_denial_of_service import DoSTester

        return DoSTester()
    else:
        # Default fallback
        from security.test_api_vulnerabilities import SecurityTester

        return SecurityTester()


@pytest.fixture
def fuzzer(api_server_required):
    """
    Provide a ProtocolFuzzer instance for protocol fuzzing tests.

    Used by test_protocol_fuzzing.py
    """
    from security.test_protocol_fuzzing import ProtocolFuzzer

    return ProtocolFuzzer()


def pytest_terminal_summary(terminalreporter, exitstatus, config):
    """Display security test summary at the end"""
    if "security" in config.getoption("file_or_dir"):
        terminalreporter.section("Security Test Summary")

        passed = len(terminalreporter.stats.get("passed", []))
        failed = len(terminalreporter.stats.get("failed", []))

        terminalreporter.write_line(
            f"Tests passed: {passed} (vulnerabilities defended)"
        )
        terminalreporter.write_line(
            f"Tests failed: {failed} (vulnerabilities exploited)"
        )

        if failed > 0:
            terminalreporter.write_line(
                "\n⚠️  VULNERABILITIES FOUND - Review failed tests for details",
                red=True,
                bold=True,
            )
        else:
            terminalreporter.write_line(
                "\n✅  All security tests passed - No exploitable vulnerabilities found",
                green=True,
                bold=True,
            )
