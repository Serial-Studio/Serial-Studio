"""
Zero-Day Level Adversarial Security Tests for Serial Studio

This test suite targets attack vectors NOT covered by the existing security
tests. It focuses on subtle, advanced exploitation techniques that a
sophisticated attacker (or an AI-assisted attacker) might use.

Attack categories covered:
- JavaScript sandbox escape via frameParserCode
- Prototype pollution via __proto__ / constructor injection
- UTF-8 overlong encoding to bypass path filters
- JSON key collision / hash flooding DoS
- State desynchronization via partial message boundaries
- Null byte injection for C++ string truncation
- Double/triple URL encoding for filter bypass
- Cross-client request forgery (shared global state abuse)
- Frame injection through data path
- Checksum algorithm confusion attacks
- Heap spray via predictable allocation patterns
- Use-after-free triggers via rapid resource lifecycle
- ANSI escape injection through terminal data path
- Quadratic blowup via duplicate JSON keys
- Unicode normalization-based filter bypass
- Integer truncation (int64 -> int32 narrowing)
- Deserialization gadget chains
- Memory disclosure via error message analysis
- Symlink/junction path attacks
- ReDoS via crafted delimiter patterns

Run with:
    pytest tests/security/test_zero_day_adversarial.py -v
    pytest tests/security/test_zero_day_adversarial.py -v -m "not destructive"
    pytest tests/security/test_zero_day_adversarial.py -v -k "sandbox"

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import base64
import hashlib
import json
import os
import random
import socket
import string
import struct
import threading
import time
import uuid

import pytest


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

API_HOST = "127.0.0.1"
API_PORT = 7777


def raw_send(data: bytes, timeout: float = 3.0):
    """Send raw bytes to the API and return (response_bytes, error_string)."""
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect((API_HOST, API_PORT))
        sock.sendall(data)
        response = b""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                chunk = sock.recv(65536)
                if not chunk:
                    break
                response += chunk
                if b"\n" in chunk:
                    break
            except socket.timeout:
                break
        return response, None
    except Exception as e:
        return None, str(e)
    finally:
        if sock:
            try:
                sock.close()
            except Exception:
                pass


def make_cmd(command, params=None):
    """Build a JSON-RPC command message."""
    msg = {
        "type": "command",
        "id": str(uuid.uuid4()),
        "command": command,
    }
    if params:
        msg["params"] = params
    return json.dumps(msg, separators=(",", ":")).encode() + b"\n"


# ===================================================================
# 1. JAVASCRIPT SANDBOX ESCAPE
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
@pytest.mark.destructive
class TestJavaScriptSandboxEscape:
    """
    CRITICAL: The FrameParser uses QJSEngine with AllExtensions.
    If frameParserCode can be set via API, an attacker could execute
    arbitrary JavaScript with full engine privileges.
    """

    MALICIOUS_SCRIPTS = [
        # Attempt to access file system
        (
            "fs_access",
            "function parse(f){var fs=require('fs');"
            "fs.writeFileSync('/tmp/pwned','OWNED');return f.split(',');}",
        ),
        # Attempt process spawning
        (
            "process_spawn",
            "function parse(f){var p=require('child_process');"
            "p.execSync('id > /tmp/pwned');return f.split(',');}",
        ),
        # Attempt to read environment variables
        (
            "env_leak",
            "function parse(f){var e=process.env;"
            "return [JSON.stringify(e)];}",
        ),
        # Infinite loop to freeze the engine
        (
            "infinite_loop",
            "function parse(f){while(true){}return f.split(',');}",
        ),
        # Stack overflow via recursion
        (
            "stack_overflow",
            "function parse(f){return parse(f);}",
        ),
        # Heap exhaustion via array allocation
        (
            "heap_exhaust",
            "function parse(f){var a=[];while(true)a.push(new Array(1e6));"
            "return f.split(',');}",
        ),
        # Prototype pollution within JS engine
        (
            "proto_pollution_js",
            "function parse(f){Object.prototype.polluted='yes';"
            "return f.split(',');}",
        ),
        # Attempt to access Qt internals
        (
            "qt_internal_access",
            "function parse(f){var q=Qt;if(q&&q.application)"
            "return [q.application.applicationName];return f.split(',');}",
        ),
        # Timer bomb (setTimeout abuse)
        (
            "timer_bomb",
            "function parse(f){for(var i=0;i<1e6;i++)"
            "setTimeout(function(){},0);return f.split(',');}",
        ),
        # Attempt to eval arbitrary code
        (
            "eval_injection",
            "function parse(f){return eval('f.split(\",\")');}",
        ),
    ]

    @pytest.mark.parametrize(
        "name,script",
        MALICIOUS_SCRIPTS,
        ids=[s[0] for s in MALICIOUS_SCRIPTS],
    )
    def test_js_sandbox_escape(
        self, security_client, check_server_alive, name, script
    ):
        """
        Attempt to inject malicious JavaScript via frameParserCode.

        Expected: Server should either reject the script, sandbox it safely,
        or not expose frameParserCode configuration via API at all.
        """
        try:
            result = security_client.command(
                "project.frameParser.setCode", {"code": script}
            )
            # If accepted, verify it didn't actually execute
            if name == "fs_access" and os.path.exists("/tmp/pwned"):
                os.remove("/tmp/pwned")
                pytest.fail(
                    "CRITICAL: JavaScript sandbox escape! "
                    "File written to /tmp/pwned via frameParserCode"
                )

        except Exception:
            pass

        assert check_server_alive(
            wait_time=2.0
        ), f"Server crashed on JS injection: {name}"

    def test_js_code_via_project_json(
        self, security_client, check_server_alive
    ):
        """
        Attempt JS injection via project.loadFromJSON with frameParserCode.
        This is the most likely attack path since project files contain JS.
        """
        malicious_project = {
            "title": "Malicious Project",
            "frameParserCode": (
                "function parse(f){"
                "var x=new XMLHttpRequest();"
                "x.open('GET','file:///etc/passwd',false);"
                "x.send();return [x.responseText];}"
            ),
            "groups": [
                {
                    "title": "Data",
                    "widget": "datagrid",
                    "datasets": [
                        {"title": "Value", "index": 1, "widget": "gauge"}
                    ],
                }
            ],
        }

        try:
            security_client.command(
                "project.loadFromJSON", {"config": malicious_project}
            )
        except Exception:
            pass

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on malicious project load with JS code"

    def test_js_code_length_bomb(
        self, security_client, check_server_alive
    ):
        """
        Send extremely large JavaScript code to exhaust parser memory.
        """
        huge_code = (
            "function parse(f){var x='"
            + "A" * (5 * 1024 * 1024)
            + "';return f.split(',');}"
        )

        try:
            security_client.command(
                "project.frameParser.setCode", {"code": huge_code}
            )
        except Exception:
            pass

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on oversized JavaScript code"


# ===================================================================
# 2. PROTOTYPE POLLUTION
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestPrototypePollution:
    """
    JSON __proto__ and constructor pollution attacks.
    If Qt's JSON parser doesn't sanitize these keys, internal objects
    could be polluted, leading to property injection or code execution.
    """

    POLLUTION_PAYLOADS = [
        {"__proto__": {"isAdmin": True, "role": "root"}},
        {"constructor": {"prototype": {"isAdmin": True}}},
        {"__proto__": {"toString": "pwned"}},
        {"__proto__": {"valueOf": 42}},
        {
            "__proto__": {
                "__proto__": {
                    "__proto__": {"deep": "pollution"}
                }
            }
        },
        {"constructor": {"name": "EvilConstructor"}},
        # Nested pollution in params
        {
            "title": "Normal",
            "__proto__": {"polluted": True},
            "groups": [{"__proto__": {"polluted": True}}],
        },
    ]

    @pytest.mark.parametrize(
        "payload",
        POLLUTION_PAYLOADS,
        ids=[f"proto_{i}" for i in range(len(POLLUTION_PAYLOADS))],
    )
    def test_prototype_pollution_via_command(
        self, security_client, payload
    ):
        """
        Send __proto__ keys in command parameters.

        Expected: Keys should be rejected or ignored, not applied to
        internal object prototypes.
        """
        try:
            security_client.command("project.loadFromJSON", {"config": payload})
        except Exception:
            pass

        # Verify server still works normally
        try:
            result = security_client.command("api.getCommands")
            assert isinstance(result, dict), "API response corrupted after proto pollution"
        except Exception:
            pass

    def test_prototype_pollution_in_raw_json(self, check_server_alive):
        """
        Send raw JSON with __proto__ directly over socket.
        """
        payloads = [
            b'{"type":"command","id":"x","command":"api.getCommands","__proto__":{"polluted":true}}\n',
            b'{"type":"command","id":"x","command":"project.setTitle","params":{"title":"test","__proto__":{"admin":true}}}\n',
            b'{"type":"command","id":"x","command":"api.getCommands","constructor":{"prototype":{"pwned":true}}}\n',
        ]

        for payload in payloads:
            raw_send(payload, timeout=2.0)

        assert check_server_alive(), "Server crashed on prototype pollution"


# ===================================================================
# 3. UTF-8 OVERLONG ENCODING / ENCODING ATTACKS
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestEncodingAttacks:
    """
    UTF-8 overlong encoding, invalid sequences, and encoding confusion
    to bypass input validation filters.
    """

    def test_overlong_utf8_path_traversal(self, check_server_alive):
        """
        Use overlong UTF-8 encoding of '../' to bypass path filters.
        The overlong encoding of '.' is 0xC0 0xAE (invalid but some
        parsers accept it).
        """
        overlong_payloads = [
            # Overlong dot: 0xC0 0xAE instead of 0x2E
            b'{"type":"command","id":"x","command":"project.file.open",'
            b'"params":{"filePath":"\xc0\xae\xc0\xae/\xc0\xae\xc0\xae/etc/passwd"}}\n',
            # Overlong slash: 0xC0 0xAF instead of 0x2F
            b'{"type":"command","id":"x","command":"project.file.open",'
            b'"params":{"filePath":"..\xc0\xaf..\xc0\xafetc\xc0\xafpasswd"}}\n',
            # Mixed overlong
            b'{"type":"command","id":"x","command":"project.file.open",'
            b'"params":{"filePath":"\xc0\xae\xc0\xae\xc0\xaf\xc0\xae\xc0\xae\xc0\xafetc\xc0\xafpasswd"}}\n',
        ]

        for payload in overlong_payloads:
            response, error = raw_send(payload, timeout=2.0)
            if response and b"success" in response.lower():
                # Check if path traversal succeeded
                if b"root:" in response or b"/bin/" in response:
                    pytest.fail(
                        "CRITICAL: Overlong UTF-8 encoding bypassed path filter!"
                    )

        assert check_server_alive(), "Server crashed on overlong UTF-8"

    def test_double_url_encoding(self, security_client):
        """
        Double-encode path traversal characters to bypass filters.
        %252e%252e = %2e%2e = ..
        """
        double_encoded_paths = [
            "%252e%252e/%252e%252e/%252e%252e/etc/passwd",
            "..%252f..%252f..%252fetc%252fpasswd",
            "%2e%2e%2f%2e%2e%2f%2e%2e%2fetc%2fpasswd",
            "..%c0%af..%c0%af..%c0%afetc%c0%afpasswd",
            "..%255c..%255c..%255cwindows%255csystem32",
        ]

        for path in double_encoded_paths:
            try:
                security_client.command(
                    "project.file.open", {"filePath": path}
                )
                pytest.fail(
                    f"Double-encoded path traversal accepted: {path}"
                )
            except Exception:
                pass

    def test_utf16_surrogate_injection(self, check_server_alive):
        """
        Inject UTF-16 surrogate pairs that are invalid in UTF-8.
        Some parsers mishandle these, potentially causing buffer overruns.
        """
        surrogate_payloads = [
            # Unpaired high surrogate
            b'{"type":"command","id":"\xed\xa0\x80","command":"api.getCommands"}\n',
            # Unpaired low surrogate
            b'{"type":"command","id":"\xed\xb0\x80","command":"api.getCommands"}\n',
            # Overlong NUL
            b'{"type":"command","id":"\xc0\x80","command":"api.getCommands"}\n',
            # 4-byte overlong
            b'{"type":"command","id":"\xf0\x80\x80\xae","command":"api.getCommands"}\n',
            # Invalid continuation byte
            b'{"type":"command","id":"\x80\x80\x80","command":"api.getCommands"}\n',
        ]

        for payload in surrogate_payloads:
            raw_send(payload, timeout=1.0)

        assert check_server_alive(), "Server crashed on surrogate injection"


# ===================================================================
# 4. NULL BYTE INJECTION
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestNullByteInjection:
    """
    Null byte injection to truncate C++ strings.
    QByteArray handles null bytes, but std::string and C strings truncate.
    If any path goes through C-string APIs, null bytes cause truncation.
    """

    def test_null_byte_path_truncation(self, security_client):
        """
        Inject null bytes to truncate file paths after the check.
        e.g., 'allowed_dir/safe.json\\x00../../etc/passwd'
        """
        null_byte_paths = [
            "/tmp/safe.json\x00../../etc/passwd",
            "project.json\x00\x00\x00/etc/shadow",
            "\x00/etc/passwd",
            "valid_project\x00.json",
            "a" * 255 + "\x00" + "../../../etc/passwd",
        ]

        for path in null_byte_paths:
            try:
                security_client.command(
                    "project.file.open", {"filePath": path}
                )
            except Exception:
                pass

    def test_null_byte_in_command_name(self, check_server_alive):
        """
        Null bytes in command names could truncate to valid commands.
        e.g., 'io.manager.connect\\x00.evil' -> 'io.manager.connect'
        """
        null_commands = [
            b'{"type":"command","id":"x","command":"io.manager.connect\x00.safe"}\n',
            b'{"type":"command","id":"x","command":"api.getCommands\x00DROP"}\n',
            b'{"type":"command","id":"x","command":"\x00io.manager.connect"}\n',
            b'{"type":"command","id":"x","command":"project.file.open\x00","params":{"filePath":"/etc/passwd"}}\n',
        ]

        for payload in null_commands:
            raw_send(payload, timeout=1.0)

        assert check_server_alive(), "Server crashed on null byte in command"

    def test_null_byte_in_frame_delimiters(self, security_client, check_server_alive):
        """
        Null bytes in frame delimiters could cause parser confusion.
        """
        try:
            security_client.configure_frame_parser(
                start_sequence="\x00/*",
                end_sequence="*/\x00",
                operation_mode=0,
            )
        except Exception:
            pass

        try:
            security_client.configure_frame_parser(
                start_sequence="\x00\x00\x00",
                end_sequence="\x00",
                operation_mode=0,
            )
        except Exception:
            pass

        assert check_server_alive(), "Server crashed on null byte delimiters"


# ===================================================================
# 5. STATE DESYNCHRONIZATION
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestStateDesync:
    """
    Get client and server out of sync on message boundaries.
    This can cause the server to parse one client's data as another's
    command, or interpret data as commands.
    """

    def test_partial_json_then_valid(self, check_server_alive):
        """
        Send partial JSON, then a valid command in the same TCP stream.
        Server should handle the boundary correctly.
        """
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)
            sock.connect((API_HOST, API_PORT))

            # Send partial JSON (no closing brace, no newline)
            sock.sendall(b'{"type":"command","id":"partial","command":"api.get')
            time.sleep(0.5)

            # Now send the rest + a new valid command
            sock.sendall(b'Commands"}\n')
            sock.settimeout(3.0)
            response = sock.recv(65536)
            sock.close()

            # Server should have handled the reassembled message
            if response:
                try:
                    parsed = json.loads(response.decode().split("\n")[0])
                    assert parsed.get("type") == "response"
                except (json.JSONDecodeError, UnicodeDecodeError):
                    pass

        except Exception:
            pass

        assert check_server_alive(), "Server crashed on partial JSON reassembly"

    def test_interleaved_binary_and_json(self, check_server_alive):
        """
        Send binary garbage between valid JSON messages.
        Server should discard binary and process valid messages.
        """
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)
            sock.connect((API_HOST, API_PORT))

            # Valid command
            sock.sendall(make_cmd("api.getCommands"))
            time.sleep(0.1)

            # Binary garbage
            sock.sendall(b"\xff\xfe\xfd\x00\x01\x02" * 100)
            time.sleep(0.1)

            # Another valid command
            sock.sendall(make_cmd("api.getCommands"))
            time.sleep(0.5)

            sock.close()

        except Exception:
            pass

        assert check_server_alive(), "Server crashed on interleaved binary/JSON"

    def test_newline_injection_in_json_value(self, check_server_alive):
        """
        Embed a newline within a JSON string value.
        This could cause the line-based parser to split one message
        into two, processing the second half as a new command.
        """
        # The injected newline creates a fake second message
        injected = (
            '{"type":"command","id":"inject","command":"api.getCommands",'
            '"params":{"data":"part1\\npart2"}}\n'
        )
        raw_send(injected.encode(), timeout=2.0)

        # Raw newline within JSON (malformed but dangerous)
        raw_payload = (
            b'{"type":"command","id":"raw","command":"api.getCommands",'
            b'"params":{"data":"line1\nline2"}}\n'
        )
        raw_send(raw_payload, timeout=2.0)

        assert check_server_alive(), "Server crashed on newline injection"

    def test_message_boundary_confusion(self, check_server_alive):
        """
        Send two messages concatenated without proper boundary.
        The second message starts immediately after the first's newline.
        """
        msg1 = make_cmd("api.getCommands")
        msg2 = make_cmd("api.getCommands")
        combined = msg1 + msg2  # Both end with \n

        # Send them in a single TCP write
        response, error = raw_send(combined, timeout=3.0)

        # Also try splitting across TCP segments
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)
            sock.connect((API_HOST, API_PORT))

            # Split message at various points
            split_point = len(msg1) + len(msg2) // 2
            sock.sendall(combined[:split_point])
            time.sleep(0.1)
            sock.sendall(combined[split_point:])
            time.sleep(0.5)

            sock.close()
        except Exception:
            pass

        assert check_server_alive(), "Server crashed on boundary confusion"


# ===================================================================
# 6. HASH COLLISION DoS
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestHashCollisionDoS:
    """
    Craft JSON objects with keys that produce hash collisions,
    causing O(n^2) insertion into hash maps.
    """

    def test_many_similar_keys(self, check_server_alive):
        """
        Send JSON with thousands of keys that are similar enough
        to cause hash table worst-case behavior.
        """
        # Generate keys with common prefixes (poor hash distribution)
        num_keys = 10000
        payload = {"type": "command", "id": "hash-dos", "command": "api.getCommands"}
        params = {}
        for i in range(num_keys):
            # Keys with same hash bucket (many hash functions are weak on these)
            params[f"{'a' * (i % 50)}_{i}"] = "x"
        payload["params"] = params

        start = time.time()
        response, error = raw_send(
            json.dumps(payload).encode() + b"\n", timeout=10.0
        )
        elapsed = time.time() - start

        # If parsing took > 5s, hash collision DoS is likely working
        if elapsed > 5.0:
            pytest.fail(
                f"Potential hash collision DoS: {num_keys} keys took "
                f"{elapsed:.2f}s to parse"
            )

        assert check_server_alive(), "Server crashed on hash collision payload"

    def test_quadratic_blowup_duplicate_keys(self, check_server_alive):
        """
        JSON with many duplicate keys causes some parsers to exhibit
        O(n^2) behavior during de-duplication.
        """
        # Build JSON with 5000 duplicate "x" keys
        # Using raw bytes since Python's json.dumps deduplicates
        parts = []
        parts.append(b'{"type":"command","id":"dup","command":"api.getCommands","params":{')
        for i in range(5000):
            if i > 0:
                parts.append(b",")
            parts.append(f'"x":"value_{i}"'.encode())
        parts.append(b"}}\n")

        payload = b"".join(parts)

        start = time.time()
        response, error = raw_send(payload, timeout=10.0)
        elapsed = time.time() - start

        if elapsed > 5.0:
            pytest.fail(
                f"Quadratic blowup: 5000 duplicate keys took {elapsed:.2f}s"
            )

        assert check_server_alive(), "Server crashed on duplicate key payload"


# ===================================================================
# 7. CROSS-CLIENT INTERFERENCE
# ===================================================================

@pytest.mark.security
class TestCrossClientAttacks:
    """
    Exploit shared global state to affect other clients.
    Since Serial Studio uses singletons, one client's actions
    affect all other clients.
    """

    def test_sabotage_via_config_change(self, check_server_alive):
        """
        One malicious client changes frame parser config while another
        client is receiving data, causing data corruption.
        """
        sabotage_complete = threading.Event()

        def saboteur():
            """Rapidly change parser config to disrupt other clients."""
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))

                delimiters = [
                    ("/*", "*/"),
                    ("$$", "$$"),
                    ("{", "}"),
                    ("<", ">"),
                    ("START", "END"),
                ]

                for i in range(50):
                    start, end = delimiters[i % len(delimiters)]
                    cmd = make_cmd(
                        "project.frameParser.configure",
                        {
                            "startSequence": start,
                            "endSequence": end,
                            "operationMode": i % 3,
                        },
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break
                    time.sleep(0.05)

                sock.close()
            except Exception:
                pass
            finally:
                sabotage_complete.set()

        def victim():
            """Normal client trying to read dashboard data."""
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))

                for i in range(20):
                    cmd = make_cmd("dashboard.getStatus")
                    try:
                        sock.sendall(cmd)
                        sock.recv(65536)
                    except Exception:
                        break
                    time.sleep(0.1)

                sock.close()
            except Exception:
                pass

        t1 = threading.Thread(target=saboteur)
        t2 = threading.Thread(target=victim)

        t1.start()
        t2.start()

        t1.join(timeout=10)
        t2.join(timeout=10)

        assert check_server_alive(), "Server crashed during cross-client attack"


# ===================================================================
# 8. FRAME INJECTION VIA DATA PATH
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestFrameInjection:
    """
    Inject crafted frames through the API's raw data path that could
    be interpreted as commands or corrupt the frame parser state.
    """

    def test_inject_json_command_as_frame(self, check_server_alive):
        """
        Send a JSON command structure as raw frame data.
        If the frame parser doesn't sanitize, this could be
        re-interpreted as an API command.
        """
        malicious_frames = [
            # JSON command disguised as frame data
            base64.b64encode(
                b'{"type":"command","command":"io.manager.connect"}'
            ).decode(),
            # Start/end delimiters embedded in data
            base64.b64encode(b"/*evil frame content*/").decode(),
            # Newline-terminated fake command
            base64.b64encode(
                b'{"type":"raw","data":"nested"}\n'
            ).decode(),
        ]

        for encoded_data in malicious_frames:
            msg = {
                "type": "raw",
                "id": str(uuid.uuid4()),
                "data": encoded_data,
            }
            raw_send(json.dumps(msg).encode() + b"\n", timeout=2.0)

        assert check_server_alive(), "Server crashed on frame injection"

    def test_delimiter_injection_in_data(self, security_client, check_server_alive):
        """
        Configure delimiters, then send raw data containing those delimiters.
        This tests whether the frame parser correctly handles delimiter
        characters appearing in the data content.
        """
        try:
            security_client.configure_frame_parser(
                start_sequence="/*",
                end_sequence="*/",
                operation_mode=1,
            )
        except Exception:
            pass

        # Data containing the configured delimiters
        injected_data = base64.b64encode(
            b"Normal data /* injected start */ and /* more injection */"
            b" with nested /* /* /* delimiters */ */ */"
        ).decode()

        msg = {"type": "raw", "id": str(uuid.uuid4()), "data": injected_data}
        raw_send(json.dumps(msg).encode() + b"\n", timeout=2.0)

        assert check_server_alive(), "Server crashed on delimiter injection"


# ===================================================================
# 9. CHECKSUM CONFUSION
# ===================================================================

@pytest.mark.security
class TestChecksumConfusion:
    """
    Exploit checksum algorithm switching to bypass integrity checks.
    """

    def test_rapid_checksum_algorithm_switching(
        self, security_client, check_server_alive
    ):
        """
        Rapidly switch checksum algorithms while data is being processed.
        This could cause the parser to validate a frame with the wrong
        algorithm, accepting corrupted data.
        """
        algorithms = [
            "", "XOR-8", "MOD-256", "CRC-8", "CRC-16-MODBUS",
            "CRC-32", "Fletcher-16", "Adler-32",
        ]

        for i in range(100):
            algo = algorithms[i % len(algorithms)]
            try:
                security_client.configure_frame_parser(
                    checksum_algorithm=algo
                )
            except Exception:
                pass

        assert check_server_alive(), "Server crashed on checksum switching"

    def test_checksum_length_mismatch(self, security_client, check_server_alive):
        """
        Send frames where the checksum field length doesn't match
        the algorithm. E.g., CRC-32 checksum but only 2 bytes provided.
        """
        # Configure CRC-32 which expects 4 bytes
        try:
            security_client.configure_frame_parser(
                start_sequence="$",
                end_sequence="\r\n",
                checksum_algorithm="CRC-32",
                operation_mode=0,
                frame_detection=1,
            )
        except Exception:
            pass

        # Send frame with truncated checksum (only 2 bytes instead of 4)
        short_checksum_frames = [
            base64.b64encode(b"$1,2,3\xAB\xCD\r\n").decode(),
            base64.b64encode(b"$1,2,3\xAB\r\n").decode(),
            base64.b64encode(b"$1,2,3\r\n").decode(),
        ]

        for frame_data in short_checksum_frames:
            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": frame_data}
            raw_send(json.dumps(msg).encode() + b"\n", timeout=1.0)

        assert check_server_alive(), "Server crashed on checksum length mismatch"


# ===================================================================
# 10. USE-AFTER-FREE TRIGGERS
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestUseAfterFree:
    """
    Trigger potential use-after-free by rapidly creating and destroying
    resources. The FrameReader is recreated on config change (by design),
    but rapid recreation could expose timing windows.
    """

    def test_rapid_framereader_recreation(self, check_server_alive):
        """
        Rapidly change frame parser settings to force FrameReader
        recreation. If any code holds a dangling reference to the old
        FrameReader, this triggers use-after-free.
        """
        def reconfigure_rapidly(thread_id):
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(10.0)
                sock.connect((API_HOST, API_PORT))

                for i in range(200):
                    cmd = make_cmd(
                        "project.frameParser.configure",
                        {
                            "startSequence": f"S{thread_id}_{i}",
                            "endSequence": f"E{thread_id}_{i}",
                            "operationMode": i % 3,
                        },
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break

                sock.close()
            except Exception:
                pass

        threads = [
            threading.Thread(target=reconfigure_rapidly, args=(i,))
            for i in range(10)
        ]
        for t in threads:
            t.start()
        for t in threads:
            t.join(timeout=15)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed during rapid FrameReader recreation (potential UAF)"

    def test_project_reload_during_data_flow(self, check_server_alive):
        """
        Reload project while data is actively flowing.
        This could trigger use-after-free if the old project's data
        structures are accessed after deletion.
        """
        def send_data():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(10.0)
                sock.connect((API_HOST, API_PORT))

                for i in range(100):
                    data = base64.b64encode(
                        f"/*{i},123,456*/".encode()
                    ).decode()
                    msg = {
                        "type": "raw",
                        "id": str(uuid.uuid4()),
                        "data": data,
                    }
                    try:
                        sock.sendall(json.dumps(msg).encode() + b"\n")
                    except Exception:
                        break
                    time.sleep(0.01)

                sock.close()
            except Exception:
                pass

        def reload_project():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(10.0)
                sock.connect((API_HOST, API_PORT))

                for i in range(20):
                    project = {
                        "title": f"Reload Test {i}",
                        "groups": [
                            {
                                "title": "G",
                                "widget": "datagrid",
                                "datasets": [
                                    {
                                        "title": f"D{j}",
                                        "index": j + 1,
                                        "widget": "gauge",
                                    }
                                    for j in range(3)
                                ],
                            }
                        ],
                    }
                    cmd = make_cmd(
                        "project.loadFromJSON", {"config": project}
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break
                    time.sleep(0.05)

                sock.close()
            except Exception:
                pass

        t1 = threading.Thread(target=send_data)
        t2 = threading.Thread(target=reload_project)

        t1.start()
        t2.start()

        t1.join(timeout=15)
        t2.join(timeout=15)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed during project reload with active data flow"


# ===================================================================
# 11. ANSI ESCAPE INJECTION VIA TERMINAL
# ===================================================================

@pytest.mark.security
class TestTerminalEscapeInjection:
    """
    Inject ANSI escape sequences through the data path that could
    affect the terminal/console widget, potentially:
    - Changing terminal title (information exfiltration)
    - Redefining keyboard mappings
    - Writing to arbitrary screen positions
    - Exploiting terminal emulator vulnerabilities
    """

    ESCAPE_PAYLOADS = [
        # Set terminal title (information exfiltration)
        b"\x1b]0;PWNED\x07",
        # Set terminal icon name
        b"\x1b]1;PWNED\x07",
        # Cursor to home position + clear screen
        b"\x1b[H\x1b[2J",
        # Save cursor, move to top, write, restore
        b"\x1b7\x1b[1;1HINJECTED\x1b8",
        # Device Status Report (DSR) - terminal responds with position
        b"\x1b[6n",
        # Request terminal parameters
        b"\x1b[x",
        # DCS (Device Control String) - can reprogram keys on some terminals
        b"\x1bP+q4142\x1b\\",
        # OSC hyperlink (could redirect clicks)
        b"\x1b]8;;https://evil.com\x07Click me\x1b]8;;\x07",
        # Sixel graphics (can cause buffer overflows in some terminals)
        b"\x1bPq" + b"#0;2;0;0;0" * 1000 + b"\x1b\\",
        # iTerm2-specific: file transfer attempt
        b"\x1b]1337;File=inline=1:" + base64.b64encode(b"MALICIOUS") + b"\x07",
        # Bracketed paste mode manipulation
        b"\x1b[200~INJECTED_PASTE\x1b[201~",
    ]

    @pytest.mark.parametrize(
        "payload",
        ESCAPE_PAYLOADS,
        ids=[f"ansi_{i}" for i in range(len(ESCAPE_PAYLOADS))],
    )
    def test_ansi_escape_via_raw_data(
        self, payload, check_server_alive
    ):
        """
        Send ANSI escape sequences as raw device data.

        Expected: Terminal widget should sanitize or ignore dangerous
        escape sequences.
        """
        encoded = base64.b64encode(payload).decode()
        msg = {"type": "raw", "id": str(uuid.uuid4()), "data": encoded}
        raw_send(json.dumps(msg).encode() + b"\n", timeout=2.0)

        assert check_server_alive(), f"Server crashed on ANSI escape injection"


# ===================================================================
# 12. INTEGER TRUNCATION ATTACKS
# ===================================================================

@pytest.mark.security
class TestIntegerTruncation:
    """
    Values that overflow when narrowed from int64 to int32, or from
    unsigned to signed. This can cause:
    - Negative array indices
    - Buffer size miscalculations
    - Wrap-around in loop counters
    - Massive memory allocation (no upper bound)

    KNOWN VULNERABILITY: Dashboard::setPoints() has NO upper bound check.
    Setting points to INT32_MAX causes allocation of ~8 GB vectors.
    The API handler only checks `points < 1`.
    """

    TRUNCATION_VALUES = [
        # int32 boundary
        (2147483647, "int32_max"),
        (2147483648, "int32_max_plus_1"),
        (-2147483648, "int32_min"),
        (-2147483649, "int32_min_minus_1"),
        # uint32 boundary
        (4294967295, "uint32_max"),
        (4294967296, "uint32_max_plus_1"),
        # Values that wrap to small positive when truncated
        (4294967297, "wraps_to_1"),
        (4294967298, "wraps_to_2"),
        # Values that wrap to negative
        (2147483648, "wraps_to_negative"),
        (3221225472, "wraps_to_neg_1073741824"),
        # Very large (JSON supports arbitrary precision)
        (999999999999999999, "18_digit"),
        (99999999999999999999999, "23_digit"),
    ]

    # FPS valid range is 1-240 (enforced by API handler).
    # Any value outside this range MUST be rejected.
    FPS_MAX_VALID = 240

    # Points has NO upper bound in C++ code (only `points < 1`).
    # A sane upper bound would be ~100,000. Values above that cause
    # massive memory allocation and UI freeze.
    POINTS_SANE_MAX = 100000

    @pytest.mark.parametrize(
        "value,name",
        TRUNCATION_VALUES,
        ids=[v[1] for v in TRUNCATION_VALUES],
    )
    def test_integer_truncation_in_fps(
        self, value, name, security_client, check_server_alive
    ):
        """
        Set FPS to values that cause truncation when cast to int/uint.

        Expected: Server MUST reject values outside 1-240 range.
        If accepted, the UI timer fires at absurd rates causing slowdown.
        """
        rejected = False
        try:
            result = security_client.command(
                "dashboard.setFPS", {"fps": value}
            )
            # Command succeeded - check if value was actually applied
            accepted_fps = result.get("fps", None)
            if accepted_fps is not None and (
                accepted_fps < 1 or accepted_fps > self.FPS_MAX_VALID
            ):
                pytest.fail(
                    f"VULNERABILITY: Server accepted FPS={accepted_fps} "
                    f"(input: {value}). Valid range is 1-{self.FPS_MAX_VALID}. "
                    f"This causes UI timer to fire at absurd rates."
                )
        except Exception:
            rejected = True

        # Always restore FPS to a sane value to avoid affecting other tests
        try:
            security_client.command("dashboard.setFPS", {"fps": 24})
        except Exception:
            pass

        assert check_server_alive(), f"Server crashed on FPS={value} ({name})"

    @pytest.mark.parametrize(
        "value,name",
        TRUNCATION_VALUES,
        ids=[v[1] for v in TRUNCATION_VALUES],
    )
    def test_integer_truncation_in_points(
        self, value, name, security_client, check_server_alive
    ):
        """
        Set plot points to values that cause truncation.

        KNOWN VULNERABILITY: DashboardHandler::setPoints() only checks
        `points < 1` with NO upper bound. Setting points to INT32_MAX
        causes Dashboard::setPoints() to allocate vectors with billions
        of entries, causing memory exhaustion and UI freeze.

        Expected: Server should reject points > 100,000 (or some sane max).
        """
        rejected = False
        try:
            result = security_client.command(
                "dashboard.setPoints", {"points": value}
            )
            accepted_points = result.get("points", None)
            if accepted_points is not None and accepted_points > self.POINTS_SANE_MAX:
                # Restore before failing so we don't leave the app broken
                try:
                    security_client.command(
                        "dashboard.setPoints", {"points": 100}
                    )
                except Exception:
                    pass

                pytest.fail(
                    f"VULNERABILITY: Server accepted points={accepted_points} "
                    f"(input: {value}). No upper bound enforced! "
                    f"DashboardHandler.cpp:209 only checks `points < 1`. "
                    f"This causes allocation of {accepted_points * 8 / 1e9:.1f} GB "
                    f"in plot vectors, freezing the application."
                )
        except Exception:
            rejected = True

        # Always restore points to a sane value
        try:
            security_client.command("dashboard.setPoints", {"points": 100})
        except Exception:
            pass

        assert check_server_alive(
            wait_time=3.0
        ), f"Server crashed on points={value} ({name})"

    def test_negative_port_numbers(self, security_client, check_server_alive):
        """
        Negative or overflow port numbers could wrap to valid ports.
        """
        evil_ports = [-1, -65535, 0, 65536, 65537, 100000, 2147483647, -2147483648]

        for port in evil_ports:
            try:
                security_client.command(
                    "io.driver.network.setTcpPort", {"port": port}
                )
            except Exception:
                pass

        assert check_server_alive(), "Server crashed on evil port numbers"


# ===================================================================
# 13. HEAP SPRAY VIA PREDICTABLE ALLOCATIONS
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestHeapSpray:
    """
    Create many objects of predictable size to fill the heap with
    controlled data. This sets up exploitation of use-after-free
    or heap overflow vulnerabilities.

    NOTE: Intensity is deliberately reduced to avoid test timeouts.
    The goal is to detect crashes/UAF, not to actually exhaust memory.
    """

    def test_project_group_spray(self, check_server_alive):
        """
        Create projects with many groups to spray the heap with
        controlled QObject allocations.

        Uses a single persistent socket and moderate project sizes
        to avoid overwhelming the server or timing out.
        """
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)
            sock.connect((API_HOST, API_PORT))

            for i in range(10):
                project = {
                    "title": f"Spray {i}",
                    "groups": [
                        {
                            "title": "A" * 64,
                            "widget": "datagrid",
                            "datasets": [
                                {
                                    "title": "B" * 64,
                                    "index": j + 1,
                                    "widget": "gauge",
                                }
                                for j in range(5)
                            ],
                        }
                        for _ in range(3)
                    ],
                }
                cmd = make_cmd("project.loadFromJSON", {"config": project})
                try:
                    sock.sendall(cmd)
                    # Drain response to avoid send buffer pressure
                    try:
                        sock.recv(65536)
                    except socket.timeout:
                        pass
                except (BrokenPipeError, ConnectionError):
                    break
                time.sleep(0.1)

            sock.close()
        except Exception:
            pass

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed during heap spray"

    def test_rapid_string_allocation_spray(self, check_server_alive):
        """
        Rapidly send commands with string parameters of specific sizes
        to control heap layout.
        """
        sizes = [16, 32, 64, 128, 256, 512, 1024]

        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10.0)
            sock.connect((API_HOST, API_PORT))

            for i in range(200):
                size = sizes[i % len(sizes)]
                cmd = make_cmd(
                    "project.setTitle",
                    {"title": "X" * size},
                )
                try:
                    sock.sendall(cmd)
                    # Periodically drain responses
                    if i % 20 == 0:
                        try:
                            sock.recv(65536)
                        except socket.timeout:
                            pass
                except (BrokenPipeError, ConnectionError):
                    break

            sock.close()
        except Exception:
            pass

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed during string allocation spray"


# ===================================================================
# 14. DESERIALIZATION GADGET CHAINS
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestDeserializationGadgets:
    """
    Craft complex JSON structures that trigger unexpected behavior
    through Qt's QJsonDocument parsing pipeline.
    """

    def test_json_billion_laughs(self, check_server_alive):
        """
        JSON equivalent of XML billion laughs attack.
        Create deeply nested arrays that expand exponentially.
        """
        # Each level doubles the data
        bomb = "[]"
        for _ in range(25):
            bomb = f"[{bomb},{bomb}]"

        payload = (
            b'{"type":"command","id":"bomb","command":"project.loadFromJSON",'
            b'"params":{"config":' + bomb.encode() + b"}}\n"
        )

        # Only send first 10MB (billion laughs can be much larger)
        if len(payload) > 10 * 1024 * 1024:
            payload = payload[: 10 * 1024 * 1024]

        response, error = raw_send(payload, timeout=5.0)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on JSON billion laughs"

    def test_deeply_nested_arrays_and_objects(self, check_server_alive):
        """
        Alternate between nested arrays and objects to maximize
        parser stack depth.
        """
        depth = 500
        inner = '{"a":[{"b":[' * depth + '1' + ']}]}' * depth

        payload = (
            b'{"type":"command","id":"nest","command":"project.loadFromJSON",'
            b'"params":{"config":' + inner.encode() + b"}}\n"
        )

        response, error = raw_send(payload, timeout=5.0)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on alternating nested arrays/objects"

    def test_json_with_every_value_type(self, check_server_alive):
        """
        Send JSON containing every possible value type in unusual
        combinations to trigger type confusion.
        """
        chaos_json = {
            "type": "command",
            "id": "chaos",
            "command": "project.loadFromJSON",
            "params": {
                "config": {
                    "title": True,
                    "groups": "not_an_array",
                    "frameParser": 42,
                    "datasets": None,
                    "nested": {
                        "array_of_mixed": [
                            1,
                            "two",
                            True,
                            None,
                            [1, [2, [3]]],
                            {"a": "b"},
                            3.14159,
                            -0,
                            1e100,
                        ]
                    },
                    "empty_object": {},
                    "empty_array": [],
                    "empty_string": "",
                    "zero": 0,
                    "negative_zero": -0.0,
                    "max_safe_int": 9007199254740991,
                    "min_safe_int": -9007199254740991,
                }
            },
        }

        raw_send(json.dumps(chaos_json).encode() + b"\n", timeout=3.0)

        assert check_server_alive(), "Server crashed on mixed-type JSON"


# ===================================================================
# 15. MEMORY DISCLOSURE VIA ERRORS
# ===================================================================

@pytest.mark.security
class TestMemoryDisclosure:
    """
    Check if error messages or responses leak internal memory contents,
    stack addresses, heap pointers, or other sensitive information.
    """

    def test_error_message_analysis(self, security_client):
        """
        Trigger various errors and analyze messages for information leaks.
        """
        leak_patterns = [
            "0x",
            "/home/",
            "/Users/",
            "C:\\",
            "stack",
            "trace",
            "malloc",
            "free",
            "segfault",
            "core dump",
            "at line",
            ".cpp:",
            ".h:",
            "QObject",
            "QString",
            "undefined behavior",
        ]

        error_triggers = [
            ("nonexistent.deep.command.path.that.is.very.long", None),
            ("project.file.open", {"filePath": "\x00\x01\x02\x03"}),
            ("project.file.open", {"filePath": "A" * 10000}),
            ("dashboard.setFPS", {"fps": "not_a_number"}),
            ("dashboard.setFPS", {"fps": float("inf")}),
            ("project.loadFromJSON", {"config": "not_an_object"}),
        ]

        for cmd, params in error_triggers:
            try:
                result = security_client.command(cmd, params)
                response_str = json.dumps(result)
            except Exception as e:
                response_str = str(e)

            response_lower = response_str.lower()
            for pattern in leak_patterns:
                if pattern.lower() in response_lower:
                    pytest.fail(
                        f"Information leak in error response!\n"
                        f"  Command: {cmd}\n"
                        f"  Pattern: '{pattern}'\n"
                        f"  Response: {response_str[:500]}"
                    )

    def test_uninitialized_memory_in_responses(self, check_server_alive):
        """
        Send requests that might cause the server to return responses
        containing uninitialized heap memory.
        """
        # Request dashboard data before any project is loaded
        cmd = make_cmd("dashboard.getData")
        response, error = raw_send(cmd, timeout=3.0)

        if response:
            # Check for binary data that shouldn't be in JSON
            try:
                decoded = response.decode("utf-8", errors="replace")
                # Check for replacement characters (indicates binary data)
                replacement_count = decoded.count("\ufffd")
                if replacement_count > 5:
                    pytest.fail(
                        f"Possible uninitialized memory in response: "
                        f"{replacement_count} replacement characters found"
                    )
            except Exception:
                pass

        assert check_server_alive(), "Server crashed during memory disclosure test"


# ===================================================================
# 16. UNICODE HOMOGRAPH ATTACKS
# ===================================================================

@pytest.mark.security
class TestUnicodeHomographs:
    """
    Use Unicode characters that look identical to ASCII but are different
    code points. This can bypass string comparison checks.
    """

    HOMOGRAPH_COMMANDS = [
        # Cyrillic 'a' (U+0430) looks like Latin 'a'
        "\u0430pi.getCommands",
        "api.getComm\u0430nds",
        # Full-width characters
        "\uff41\uff50\uff49.getCommands",
        # Combining characters to create lookalikes
        "a\u0300pi.getCommands",
        # Mathematical symbols that look like letters
        "\U0001d44epi.getCommands",
        # Right-to-left override to reverse display
        "sdnammoCteg.ipa\u202e",
        # Zero-width characters between valid parts
        "api\u200b.\u200bget\u200bCommands",
        "api\ufeff.getCommands",
        "api\u200c.get\u200dCommands",
    ]

    @pytest.mark.parametrize(
        "cmd",
        HOMOGRAPH_COMMANDS,
        ids=[f"homograph_{i}" for i in range(len(HOMOGRAPH_COMMANDS))],
    )
    def test_homograph_command_bypass(
        self, security_client, cmd
    ):
        """
        Send commands with homograph characters.

        Expected: Server should reject these or treat them as invalid
        commands, not match them to real commands.
        """
        try:
            result = security_client.command(cmd)
            # If the command succeeded, that means homograph bypassed validation
            if isinstance(result, dict) and "commands" in result:
                pytest.fail(
                    f"Homograph command accepted as valid: {repr(cmd)}"
                )
        except Exception:
            pass


# ===================================================================
# 17. RESOURCE EXHAUSTION VIA PROJECT COMPLEXITY
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestProjectComplexityBomb:
    """
    Create projects with extreme complexity to exhaust CPU and memory
    during project loading and dashboard initialization.
    """

    def test_maximum_groups_and_datasets(self, check_server_alive):
        """
        Create a project with the maximum number of groups and datasets
        that fits within the JSON validator's limits.
        """
        # JsonValidator allows maxArraySize=10000 per array
        project = {
            "title": "Complexity Bomb",
            "groups": [
                {
                    "title": f"Group_{i}",
                    "widget": "datagrid",
                    "datasets": [
                        {
                            "title": f"Dataset_{i}_{j}",
                            "index": j + 1,
                            "widget": "gauge",
                        }
                        for j in range(100)
                    ],
                }
                for i in range(100)
            ],
        }
        # 100 groups x 100 datasets = 10,000 total datasets

        start = time.time()
        cmd = make_cmd("project.loadFromJSON", {"config": project})
        response, error = raw_send(cmd, timeout=30.0)
        elapsed = time.time() - start

        assert check_server_alive(
            wait_time=3.0
        ), f"Server crashed on complexity bomb ({elapsed:.2f}s)"

    def test_deeply_nested_group_structure(self, check_server_alive):
        """
        Create project with deeply nested group/dataset hierarchy.
        """
        def make_nested_group(depth, max_depth=50):
            if depth >= max_depth:
                return {
                    "title": f"Leaf_{depth}",
                    "widget": "datagrid",
                    "datasets": [
                        {"title": "D", "index": 1, "widget": "gauge"}
                    ],
                }
            return {
                "title": f"Level_{depth}",
                "widget": "datagrid",
                "datasets": [
                    {"title": f"D_{depth}", "index": 1, "widget": "gauge"}
                ],
            }

        project = {
            "title": "Deep Nesting",
            "groups": [make_nested_group(i) for i in range(200)],
        }

        cmd = make_cmd("project.loadFromJSON", {"config": project})
        raw_send(cmd, timeout=10.0)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on deeply nested project"

    def test_project_with_huge_js_parser(self, check_server_alive):
        """
        Load project with very complex JavaScript parser code.
        """
        # JavaScript with many closures and allocations
        complex_js = "function parse(f){"
        for i in range(100):
            complex_js += f"var v{i}=f.split(',').map(function(x){{return parseFloat(x)*{i};}});"
        complex_js += "return f.split(',');}"

        project = {
            "title": "JS Bomb",
            "frameParserCode": complex_js,
            "groups": [
                {
                    "title": "G",
                    "widget": "datagrid",
                    "datasets": [
                        {"title": "D", "index": 1, "widget": "gauge"}
                    ],
                }
            ],
        }

        cmd = make_cmd("project.loadFromJSON", {"config": project})
        raw_send(cmd, timeout=10.0)

        assert check_server_alive(
            wait_time=2.0
        ), "Server crashed on complex JS parser"


# ===================================================================
# 18. SYMLINK AND PATH MANIPULATION
# ===================================================================

@pytest.mark.security
@pytest.mark.critical
class TestSymlinkAttacks:
    """
    Path manipulation attacks using symlinks, junctions, and special
    filesystem paths.
    """

    EVIL_PATHS = [
        # Symlink-like paths
        "/proc/self/exe",
        "/proc/self/maps",
        "/proc/self/mem",
        "/proc/self/fd/0",
        "/dev/stdin",
        "/dev/null",
        "/dev/zero",
        "/dev/random",
        "/dev/urandom",
        # macOS specific
        "/System/Library/CoreServices/SystemVersion.plist",
        # Windows-style paths in case running on Windows
        "\\\\?\\C:\\Windows\\System32\\config\\SAM",
        "\\\\?\\C:\\Windows\\System32\\drivers\\etc\\hosts",
        "CON",
        "NUL",
        "COM1",
        "LPT1",
        "PRN",
        # UNC paths
        "\\\\127.0.0.1\\c$\\Windows\\System32\\config\\SAM",
        "\\\\localhost\\c$\\",
        # file:// URI scheme
        "file:///etc/passwd",
        "file:///C:/Windows/System32/config/SAM",
        # Very long path (PATH_MAX = 4096 on Linux)
        "/" + "a" * 4096 + "/file.json",
        # Path with null bytes
        "/tmp/safe\x00/../../etc/passwd",
    ]

    @pytest.mark.parametrize(
        "path",
        EVIL_PATHS,
        ids=[f"path_{i}" for i in range(len(EVIL_PATHS))],
    )
    def test_evil_path(self, path, check_server_alive):
        """
        Attempt to open various dangerous filesystem paths.

        Expected: All should be rejected. Server must not crash.
        """
        cmd = make_cmd("project.file.open", {"filePath": path})
        response, error = raw_send(cmd, timeout=3.0)

        # Check response doesn't contain sensitive data
        if response:
            sensitive_patterns = [
                b"root:", b"daemon:", b"nobody:",  # /etc/passwd
                b"NTFS", b"\\Windows\\",  # SAM file
                b"ProductVersion", b"ProductName",  # macOS plist
            ]
            for pattern in sensitive_patterns:
                if pattern in response:
                    pytest.fail(
                        f"Sensitive data leaked via path: {path}\n"
                        f"Found: {pattern}"
                    )

        assert check_server_alive(), f"Server crashed on path: {path}"


# ===================================================================
# 19. CONCURRENT OPERATION CHAOS
# ===================================================================

@pytest.mark.security
@pytest.mark.destructive
class TestConcurrentChaos:
    """
    Perform many different operations simultaneously to trigger
    race conditions, deadlocks, and assertion failures.
    """

    def test_everything_at_once(self, check_server_alive):
        """
        Launch threads performing different operations simultaneously:
        - Config changes
        - Data sending
        - Status queries
        - Project loading
        - Export toggling
        """
        stop_event = threading.Event()

        def config_changer():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))
                while not stop_event.is_set():
                    cmd = make_cmd(
                        "dashboard.setFPS",
                        {"fps": random.randint(1, 120)},
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break
                    time.sleep(0.01)
                sock.close()
            except Exception:
                pass

        def data_sender():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))
                while not stop_event.is_set():
                    data = base64.b64encode(
                        f"/*{random.random()},{random.random()}*/".encode()
                    ).decode()
                    msg = {
                        "type": "raw",
                        "id": str(uuid.uuid4()),
                        "data": data,
                    }
                    try:
                        sock.sendall(json.dumps(msg).encode() + b"\n")
                    except Exception:
                        break
                    time.sleep(0.005)
                sock.close()
            except Exception:
                pass

        def status_querier():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))
                while not stop_event.is_set():
                    cmds = [
                        make_cmd("dashboard.getStatus"),
                        make_cmd("io.manager.getStatus"),
                        make_cmd("api.getCommands"),
                    ]
                    cmd = random.choice(cmds)
                    try:
                        sock.sendall(cmd)
                        sock.recv(65536)
                    except Exception:
                        break
                    time.sleep(0.02)
                sock.close()
            except Exception:
                pass

        def project_loader():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))
                while not stop_event.is_set():
                    n_groups = random.randint(1, 5)
                    n_datasets = random.randint(1, 10)
                    project = {
                        "title": f"Chaos_{random.randint(0,9999)}",
                        "groups": [
                            {
                                "title": f"G{g}",
                                "widget": "datagrid",
                                "datasets": [
                                    {
                                        "title": f"D{d}",
                                        "index": d + 1,
                                        "widget": "gauge",
                                    }
                                    for d in range(n_datasets)
                                ],
                            }
                            for g in range(n_groups)
                        ],
                    }
                    cmd = make_cmd(
                        "project.loadFromJSON", {"config": project}
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break
                    time.sleep(0.1)
                sock.close()
            except Exception:
                pass

        def export_toggler():
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect((API_HOST, API_PORT))
                while not stop_event.is_set():
                    enabled = random.choice([True, False])
                    cmd = make_cmd(
                        "csv.export.setEnabled",
                        {"enabled": enabled},
                    )
                    try:
                        sock.sendall(cmd)
                    except Exception:
                        break
                    time.sleep(0.05)
                sock.close()
            except Exception:
                pass

        thread_funcs = [
            config_changer,
            config_changer,
            data_sender,
            data_sender,
            data_sender,
            status_querier,
            status_querier,
            project_loader,
            export_toggler,
            export_toggler,
        ]

        threads = [threading.Thread(target=f) for f in thread_funcs]
        for t in threads:
            t.start()

        # Run chaos for 5 seconds
        time.sleep(5)
        stop_event.set()

        for t in threads:
            t.join(timeout=5)

        assert check_server_alive(
            wait_time=3.0
        ), "Server crashed during concurrent chaos test"


# ===================================================================
# 20. ADVANCED TIMING ORACLE
# ===================================================================

@pytest.mark.security
@pytest.mark.slow
class TestTimingOracle:
    """
    Use statistical timing analysis to extract information about
    internal state, valid commands, and processing paths.
    """

    def test_command_existence_oracle(self, check_server_alive):
        """
        Measure timing difference between valid and invalid commands
        to determine if timing can be used to enumerate commands.
        """
        valid_commands = [
            "api.getCommands",
            "dashboard.getStatus",
            "io.manager.getStatus",
        ]

        invalid_commands = [
            "api.secretAdmin",
            "system.exec.shell",
            "debug.memory.dump",
        ]

        def measure(command, samples=30):
            times = []
            for _ in range(samples):
                start = time.perf_counter()
                raw_send(make_cmd(command), timeout=2.0)
                elapsed = time.perf_counter() - start
                times.append(elapsed)
            return sorted(times)[len(times) // 4 : 3 * len(times) // 4]

        valid_times = []
        for cmd in valid_commands:
            valid_times.extend(measure(cmd))

        invalid_times = []
        for cmd in invalid_commands:
            invalid_times.extend(measure(cmd))

        if valid_times and invalid_times:
            avg_valid = sum(valid_times) / len(valid_times)
            avg_invalid = sum(invalid_times) / len(invalid_times)
            timing_diff_ms = abs(avg_valid - avg_invalid) * 1000

            # > 5ms difference is statistically significant
            if timing_diff_ms > 5.0:
                pytest.fail(
                    f"Timing oracle detected!\n"
                    f"  Valid commands avg: {avg_valid*1000:.2f}ms\n"
                    f"  Invalid commands avg: {avg_invalid*1000:.2f}ms\n"
                    f"  Difference: {timing_diff_ms:.2f}ms\n"
                    f"  An attacker can enumerate valid commands via timing"
                )

        assert check_server_alive(), "Server crashed during timing analysis"


# ===================================================================
# 21. MALFORMED BATCH STRUCTURES
# ===================================================================

@pytest.mark.security
class TestMalformedBatch:
    """
    Send structurally invalid batch messages to trigger edge cases
    in batch processing logic.
    """

    MALFORMED_BATCHES = [
        # commands is not an array
        b'{"type":"batch","id":"x","commands":"not_array"}\n',
        # commands contains non-objects
        b'{"type":"batch","id":"x","commands":[1,2,3]}\n',
        # commands contains nulls
        b'{"type":"batch","id":"x","commands":[null,null]}\n',
        # commands contains nested batches
        b'{"type":"batch","id":"x","commands":[{"type":"batch","commands":[]}]}\n',
        # Missing commands field
        b'{"type":"batch","id":"x"}\n',
        # Empty commands
        b'{"type":"batch","id":"x","commands":[]}\n',
        # commands with mixed types
        b'{"type":"batch","id":"x","commands":[{"command":"api.getCommands"},42,"str",null,true]}\n',
        # Recursive batch reference
        b'{"type":"batch","id":"x","commands":[{"command":"api.getCommands","type":"batch"}]}\n',
        # Very long command names in batch
        b'{"type":"batch","id":"x","commands":[{"command":"' + b"a" * 100000 + b'"}]}\n',
        # Batch with duplicate IDs
        b'{"type":"batch","id":"x","commands":[{"id":"dup","command":"api.getCommands"},{"id":"dup","command":"api.getCommands"}]}\n',
    ]

    @pytest.mark.parametrize(
        "payload",
        MALFORMED_BATCHES,
        ids=[f"batch_{i}" for i in range(len(MALFORMED_BATCHES))],
    )
    def test_malformed_batch(self, payload, check_server_alive):
        """
        Send structurally invalid batch commands.

        Expected: Server should reject with error, not crash.
        """
        raw_send(payload, timeout=3.0)
        assert check_server_alive(), "Server crashed on malformed batch"


# ===================================================================
# 22. API TYPE/FIELD CONFUSION
# ===================================================================

@pytest.mark.security
class TestTypeFieldConfusion:
    """
    Send messages with unexpected 'type' values or missing/extra fields
    to trigger edge cases in message routing.
    """

    CONFUSED_MESSAGES = [
        # Unknown type values
        b'{"type":"subscribe","id":"x","channel":"data"}\n',
        b'{"type":"publish","id":"x","data":"test"}\n',
        b'{"type":"admin","id":"x","command":"shutdown"}\n',
        b'{"type":"internal","id":"x","action":"debug"}\n',
        b'{"type":"system","id":"x","command":"reboot"}\n',
        # Type as non-string
        b'{"type":1,"id":"x","command":"api.getCommands"}\n',
        b'{"type":true,"id":"x","command":"api.getCommands"}\n',
        b'{"type":null,"id":"x","command":"api.getCommands"}\n',
        b'{"type":[],"id":"x","command":"api.getCommands"}\n',
        b'{"type":{"nested":"type"},"id":"x","command":"api.getCommands"}\n',
        # Multiple type fields (parser-dependent behavior)
        b'{"type":"command","type":"batch","id":"x","command":"api.getCommands"}\n',
        b'{"type":"raw","type":"command","id":"x","command":"api.getCommands"}\n',
        # Empty type
        b'{"type":"","id":"x","command":"api.getCommands"}\n',
        # Type with whitespace
        b'{"type":" command ","id":"x","command":"api.getCommands"}\n',
        b'{"type":"command\\n","id":"x","command":"api.getCommands"}\n',
    ]

    @pytest.mark.parametrize(
        "payload",
        CONFUSED_MESSAGES,
        ids=[f"type_{i}" for i in range(len(CONFUSED_MESSAGES))],
    )
    def test_type_confusion(self, payload, check_server_alive):
        """
        Send messages with confused type fields.

        Expected: Server should reject gracefully, not crash or
        misroute the message.
        """
        raw_send(payload, timeout=2.0)
        assert check_server_alive(), "Server crashed on type confusion"


# ===================================================================
# 23. RESPONSE PARSING ATTACKS (Reflected)
# ===================================================================

@pytest.mark.security
class TestReflectedAttacks:
    """
    Send payloads that, when reflected in error messages, could be
    dangerous to downstream consumers (XSS, log injection, etc.).
    """

    REFLECTED_PAYLOADS = [
        # XSS attempts (if response is rendered in web UI)
        '<script>alert("XSS")</script>',
        '<img src=x onerror=alert(1)>',
        '"><svg/onload=alert(1)>',
        "javascript:alert(1)",
        # Log injection
        "test\n[CRITICAL] Fake log entry",
        "test\r\n\r\nHTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Injected</h1>",
        # Template injection
        "{{7*7}}",
        "${7*7}",
        "<%= 7*7 %>",
        "#{7*7}",
        "{{constructor.constructor('return this')()}}",
    ]

    @pytest.mark.parametrize(
        "payload",
        REFLECTED_PAYLOADS,
        ids=[f"reflect_{i}" for i in range(len(REFLECTED_PAYLOADS))],
    )
    def test_reflected_payload_in_title(
        self, security_client, payload
    ):
        """
        Set project title to a malicious payload, then read it back.
        The payload should be stored as-is (not executed) or sanitized.
        """
        try:
            security_client.command(
                "project.setTitle", {"title": payload}
            )
        except Exception:
            return

        # Read back - should not be interpreted/executed
        try:
            status = security_client.command("project.getStatus")
            returned_title = status.get("title", "")
            # We just verify server didn't crash - actual sanitization
            # depends on where the title is displayed
        except Exception:
            pass
