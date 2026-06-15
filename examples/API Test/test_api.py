#!/usr/bin/env python3
"""
Serial Studio API Client & Test Suite
======================================

A versatile tool for interacting with the Serial Studio API Server.
Can be used as a command-line client, interactive shell, or test suite.

Usage:
    # Send a single command
    python test_api.py send io.getStatus

    # Send command with parameters (key=value format - works on all shells)
    python test_api.py send io.uart.setBaudRate -p baudRate=115200

    # Multiple parameters
    python test_api.py send io.network.setTcpPort -p port=8080

    # JSON format (use on bash/zsh, tricky on PowerShell)
    python test_api.py send io.uart.setBaudRate --params '{"baudRate": 115200}'

    # List all available commands
    python test_api.py list

    # Interactive mode (REPL)
    python test_api.py interactive

    # Live monitor (real-time status updates)
    python test_api.py monitor [--interval 500] [--compact] [--show-raw-data]

    # Run smoke test suite (introspects the live server)
    python test_api.py test [--verbose]

    # Send batch from JSON file
    python test_api.py batch commands.json

    # Pipe JSON output (for scripting)
    python test_api.py send io.getStatus --json | jq '.result'

Common options for all modes:
    --host HOST    Server host (default: 127.0.0.1)
    --port PORT    Server port (default: 7777)

Requirements:
    - Serial Studio running with API Server enabled (port 7777)
    - Python 3.8+

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys

# Force UTF-8 console output: Windows defaults to cp1252, which cannot encode
# the Unicode characters this script prints (e.g. arrows / check marks).
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8")
    except (AttributeError, ValueError):
        pass

import json
import socket
import argparse
import time
import uuid
import select
import base64
import os
from typing import Optional
from dataclasses import dataclass
from enum import Enum

try:
    import readline

    READLINE_AVAILABLE = True
except ImportError:
    READLINE_AVAILABLE = False


# =============================================================================
# Configuration
# =============================================================================

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 7777
SOCKET_TIMEOUT = 5.0
RECV_BUFFER_SIZE = 65536


class Colors:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    RED = "\033[91m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    BLUE = "\033[94m"
    MAGENTA = "\033[95m"
    CYAN = "\033[96m"

    @staticmethod
    def is_supported():
        if not hasattr(sys.stdout, "isatty") or not sys.stdout.isatty():
            return False
        return os.name != "nt" or "ANSICON" in os.environ


COLORS_ENABLED = Colors.is_supported()


# =============================================================================
# Protocol Constants (matching C++ API::MessageType and API::ErrorCode)
# =============================================================================


class MessageType:
    COMMAND = "command"
    BATCH = "batch"
    RESPONSE = "response"


class ErrorCode:
    INVALID_JSON = "INVALID_JSON"
    INVALID_MESSAGE_TYPE = "INVALID_MESSAGE_TYPE"
    UNKNOWN_COMMAND = "UNKNOWN_COMMAND"
    INVALID_PARAM = "INVALID_PARAM"
    MISSING_PARAM = "MISSING_PARAM"
    EXECUTION_ERROR = "EXECUTION_ERROR"


# =============================================================================
# Color Helpers
# =============================================================================


def colorize(text: str, color: str) -> str:
    return f"{color}{text}{Colors.RESET}" if COLORS_ENABLED else text


def success(text: str) -> str:
    return colorize(text, Colors.GREEN)


def error(text: str) -> str:
    return colorize(text, Colors.RED)


def info(text: str) -> str:
    return colorize(text, Colors.BLUE)


def warning(text: str) -> str:
    return colorize(text, Colors.YELLOW)


def dim(text: str) -> str:
    return colorize(text, Colors.DIM)


def bold(text: str) -> str:
    return colorize(text, Colors.BOLD)


# =============================================================================
# Test Result Tracking
# =============================================================================


class TestResult(Enum):
    PASSED = "PASSED"
    FAILED = "FAILED"
    SKIPPED = "SKIPPED"


@dataclass
class TestCase:
    name: str
    result: TestResult
    message: str = ""
    duration_ms: float = 0.0


class TestSuite:
    def __init__(self, name: str):
        self.name = name
        self.tests: list[TestCase] = []

    def add_result(self, test: TestCase):
        self.tests.append(test)

    @property
    def passed(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.PASSED)

    @property
    def failed(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.FAILED)

    @property
    def skipped(self) -> int:
        return sum(1 for t in self.tests if t.result == TestResult.SKIPPED)

    def print_summary(self):
        print(bold(f"\n{'=' * 60}"))
        print(bold(f"Test Suite: {self.name}"))
        print(bold(f"{'=' * 60}"))

        total = len(self.tests)
        passed_str = (
            success(f"Passed: {self.passed}")
            if self.passed > 0
            else dim(f"Passed: {self.passed}")
        )
        failed_str = (
            error(f"Failed: {self.failed}")
            if self.failed > 0
            else dim(f"Failed: {self.failed}")
        )
        skipped_str = (
            warning(f"Skipped: {self.skipped}")
            if self.skipped > 0
            else dim(f"Skipped: {self.skipped}")
        )

        print(f"Total: {total} | {passed_str} | {failed_str} | {skipped_str}")
        print(bold(f"{'=' * 60}"))

        if self.failed > 0:
            print(error("\nFailed Tests:"))
            for test in self.tests:
                if test.result == TestResult.FAILED:
                    print(f"  {error(chr(10007))} {test.name}: {error(test.message)}")

        print()


# =============================================================================
# API Client
# =============================================================================


class SerialStudioAPI:
    """Client for communicating with Serial Studio API Server."""

    def __init__(
        self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT, verbose: bool = False
    ):
        self.host = host
        self.port = port
        self.verbose = verbose
        self.socket: Optional[socket.socket] = None
        self.receive_buffer = b""

    def connect(self) -> bool:
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(SOCKET_TIMEOUT)
            self.socket.connect((self.host, self.port))
            if self.verbose:
                print(f"[INFO] Connected to {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"[ERROR] Failed to connect: {e}")
            return False

    def disconnect(self):
        if self.socket:
            try:
                self.socket.close()
            except Exception:
                pass
            self.socket = None
        self.receive_buffer = b""

    def recv_message(self, timeout: float = SOCKET_TIMEOUT) -> Optional[dict]:
        """Receive a newline-delimited JSON message. Returns None on timeout/error."""
        if not self.socket:
            return None

        try:
            end_time = time.time() + timeout
            while True:
                newline_pos = self.receive_buffer.find(b"\n")
                if newline_pos != -1:
                    line = self.receive_buffer[:newline_pos]
                    self.receive_buffer = self.receive_buffer[newline_pos + 1 :]

                    if line.strip():
                        if self.verbose:
                            print(f"[RECV] {line.decode('utf-8', errors='replace')}")
                        return json.loads(line.decode("utf-8"))
                    continue

                remaining_time = end_time - time.time()
                if remaining_time <= 0:
                    return None

                ready = select.select([self.socket], [], [], min(remaining_time, 0.1))
                if ready[0]:
                    chunk = self.socket.recv(RECV_BUFFER_SIZE)
                    if not chunk:
                        return None
                    self.receive_buffer += chunk
        except Exception as e:
            if self.verbose:
                print(f"[ERROR] recv_message: {e}")
            return None

    def send_raw(
        self,
        data: bytes,
        expected_id: Optional[str] = None,
        timeout: float = SOCKET_TIMEOUT,
    ) -> Optional[dict]:
        """Send bytes; if expected_id is set, skip push notifications until that response arrives."""
        if not self.socket:
            return None

        try:
            if self.verbose:
                print(f"[SEND] {data.decode('utf-8', errors='replace').strip()}")

            self.socket.sendall(data)

            if expected_id:
                end_time = time.time() + timeout
                while True:
                    remaining = end_time - time.time()
                    if remaining <= 0:
                        if self.verbose:
                            print(
                                f"[ERROR] Timeout waiting for response ID {expected_id}"
                            )
                        return None

                    msg = self.recv_message(timeout=remaining)
                    if not msg:
                        return None

                    if (
                        msg.get("type") == MessageType.RESPONSE
                        and msg.get("id") == expected_id
                    ):
                        return msg

                    if self.verbose:
                        print(
                            f"[DEBUG] Discarding push notification while waiting for {expected_id}"
                        )
            else:
                return self.recv_message(timeout=timeout)

        except Exception as e:
            if self.verbose:
                print(f"[ERROR] {e}")
            return None

    def send_json(self, obj: dict) -> Optional[dict]:
        data = json.dumps(obj, separators=(",", ":")) + "\n"
        expected_id = obj.get("id")
        return self.send_raw(
            data.encode("utf-8"), expected_id=expected_id, timeout=SOCKET_TIMEOUT
        )

    def send_command(
        self,
        command: str,
        params: Optional[dict] = None,
        request_id: Optional[str] = None,
    ) -> Optional[dict]:
        msg = {
            "type": MessageType.COMMAND,
            "id": request_id or str(uuid.uuid4()),
            "command": command,
        }
        if params:
            msg["params"] = params
        return self.send_json(msg)

    def send_batch(
        self, commands: list[dict], request_id: Optional[str] = None
    ) -> Optional[dict]:
        msg = {
            "type": MessageType.BATCH,
            "id": request_id or str(uuid.uuid4()),
            "commands": commands,
        }
        return self.send_json(msg)

    def has_data_available(self, timeout: float = 0.0) -> bool:
        if not self.socket:
            return False
        if self.receive_buffer:
            return True
        try:
            ready = select.select([self.socket], [], [], timeout)
            return bool(ready[0])
        except Exception:
            return False


# =============================================================================
# Smoke-Test Suite
# =============================================================================
#
# The earlier hand-written per-handler test functions (~1700 lines) drifted
# behind the C++ registry every time a command was renamed. The suite below
# is intentionally small and self-discovering: it reads the live command
# list, exercises a handful of well-known read-only commands, and checks
# the protocol-level invariants. Drift is detected automatically -- if a
# command vanishes from the registry, the test that uses it fails with
# UNKNOWN_COMMAND, which is the right signal.
# =============================================================================

# Read-only commands that should always succeed and return a result. One per
# scope -- enough to prove each subsystem is wired and responsive without
# pretending to know the per-driver setter surface.
SMOKE_COMMANDS: list[tuple[str, str]] = [
    ("api.getCommands", "api"),
    ("io.getStatus", "io"),
    ("io.listBuses", "io"),
    ("io.uart.listPorts", "io.uart"),
    ("io.uart.listBaudRates", "io.uart"),
    ("io.network.listSocketTypes", "io.network"),
    ("console.getConfig", "console"),
    ("dashboard.getStatus", "dashboard"),
    ("dashboard.getOperationMode", "dashboard"),
    ("project.exportJson", "project"),
    ("project.template.list", "project"),
    ("project.group.list", "project"),
    ("project.dataset.list", "project"),
    ("project.action.list", "project"),
    ("project.workspace.list", "project"),
    ("project.source.list", "project"),
    ("project.dataTable.list", "project"),
    ("ui.window.getLayout", "ui"),
    ("ui.window.listGroups", "ui"),
    ("notifications.getUnreadCount", "notifications"),
    ("notifications.listChannels", "notifications"),
    ("extensions.listRepositories", "extensions"),
    ("scripts.list", "scripts"),
    ("licensing.getStatus", "licensing"),
    # Pro-only -- skipped automatically when license tier is below Pro
    ("io.modbus.listProtocols", "io.modbus (Pro)"),
    ("io.canbus.listPlugins", "io.canbus (Pro)"),
    ("io.audio.listSampleRates", "io.audio (Pro)"),
    ("mqtt.getConfig", "mqtt (Pro)"),
    ("sessions.getStatus", "sessions (Pro)"),
]


def _record(
    suite: TestSuite,
    name: str,
    result: TestResult,
    message: str = "",
    duration_ms: float = 0.0,
):
    suite.add_result(
        TestCase(name=name, result=result, message=message, duration_ms=duration_ms)
    )
    icon = (
        success(chr(10003))
        if result == TestResult.PASSED
        else (error(chr(10007)) if result == TestResult.FAILED else warning("~"))
    )
    suffix = f" ({duration_ms:.0f} ms)" if duration_ms else ""
    print(f"  {icon} {name}{suffix}{'  ' + dim(message) if message else ''}")


def test_protocol(api: SerialStudioAPI, suite: TestSuite):
    """Protocol-level checks that don't depend on the command surface."""
    print(bold(info("\n[ Protocol ]")))

    # 1) Unknown command must return UNKNOWN_COMMAND, not crash the server.
    t0 = time.time()
    response = api.send_command("does.not.exist." + uuid.uuid4().hex[:8])
    duration = (time.time() - t0) * 1000
    if response is None:
        _record(
            suite,
            "unknown_command -> server response",
            TestResult.FAILED,
            "no response received",
        )
    elif response.get("success"):
        _record(
            suite,
            "unknown_command -> error",
            TestResult.FAILED,
            "server returned success on a fake command name",
        )
    else:
        err_code = response.get("error", {}).get("code", "")
        if err_code == ErrorCode.UNKNOWN_COMMAND:
            _record(
                suite,
                "unknown_command -> UNKNOWN_COMMAND",
                TestResult.PASSED,
                duration_ms=duration,
            )
        else:
            _record(
                suite,
                "unknown_command -> UNKNOWN_COMMAND",
                TestResult.FAILED,
                f"got {err_code} instead",
            )

    # 2) Response IDs must match the request ID.
    rid = "test-id-" + uuid.uuid4().hex[:8]
    t0 = time.time()
    response = api.send_command("io.getStatus", request_id=rid)
    duration = (time.time() - t0) * 1000
    if response and response.get("id") == rid:
        _record(
            suite,
            "response.id matches request.id",
            TestResult.PASSED,
            duration_ms=duration,
        )
    else:
        actual = response.get("id") if response else None
        _record(
            suite,
            "response.id matches request.id",
            TestResult.FAILED,
            f"expected {rid!r}, got {actual!r}",
        )

    # 3) Batch responses must come back in order with one entry per command.
    cmds = [
        {"id": "b-1", "command": "io.getStatus"},
        {"id": "b-2", "command": "console.getConfig"},
        {"id": "b-3", "command": "dashboard.getStatus"},
    ]
    t0 = time.time()
    response = api.send_batch(cmds)
    duration = (time.time() - t0) * 1000
    if not response or "results" not in response:
        _record(
            suite, "batch returns results array", TestResult.FAILED, "no results field"
        )
    else:
        results = response["results"]
        if len(results) == len(cmds):
            ids_ok = all(
                results[i].get("id") == cmds[i]["id"] for i in range(len(cmds))
            )
            if ids_ok:
                _record(
                    suite,
                    "batch preserves order and IDs",
                    TestResult.PASSED,
                    duration_ms=duration,
                )
            else:
                _record(
                    suite,
                    "batch preserves order and IDs",
                    TestResult.FAILED,
                    "results not in submitted order",
                )
        else:
            _record(
                suite,
                "batch preserves order and IDs",
                TestResult.FAILED,
                f"expected {len(cmds)} results, got {len(results)}",
            )


def test_command_registry(api: SerialStudioAPI, suite: TestSuite):
    """Verify api.getCommands returns a populated registry."""
    print(bold(info("\n[ Command Registry ]")))

    t0 = time.time()
    response = api.send_command("api.getCommands")
    duration = (time.time() - t0) * 1000

    if not response or not response.get("success"):
        _record(
            suite,
            "api.getCommands succeeds",
            TestResult.FAILED,
            "no successful response",
        )
        return None

    commands = response.get("result", {}).get("commands", [])
    if not isinstance(commands, list) or len(commands) < 50:
        _record(
            suite,
            "api.getCommands returns >= 50 commands",
            TestResult.FAILED,
            f"got {len(commands)}",
        )
        return commands

    _record(
        suite,
        f"api.getCommands returns {len(commands)} commands",
        TestResult.PASSED,
        duration_ms=duration,
    )

    # Spot-check structure: each entry has name + description.
    bad = [
        c
        for c in commands
        if not isinstance(c, dict) or "name" not in c or "description" not in c
    ]
    if bad:
        _record(
            suite,
            "every command has name+description",
            TestResult.FAILED,
            f"{len(bad)} entries malformed",
        )
    else:
        _record(suite, "every command has name+description", TestResult.PASSED)

    return commands


def test_smoke(api: SerialStudioAPI, suite: TestSuite, available: set[str]):
    """Exercise one read-only command per scope."""
    print(bold(info("\n[ Smoke: read-only commands ]")))

    for cmd, scope in SMOKE_COMMANDS:
        if cmd not in available:
            _record(
                suite,
                f"{cmd} ({scope})",
                TestResult.SKIPPED,
                "not registered (probably GPL build or feature off)",
            )
            continue

        t0 = time.time()
        response = api.send_command(cmd)
        duration = (time.time() - t0) * 1000

        if response is None:
            _record(suite, f"{cmd} ({scope})", TestResult.FAILED, "no response")
            continue

        if response.get("success"):
            _record(suite, f"{cmd} ({scope})", TestResult.PASSED, duration_ms=duration)
        else:
            err = response.get("error", {})
            code = err.get("code", "?")
            # license_required is a soft pass -- the command exists, the
            # server just won't run it without a Pro key.
            if code == "license_required":
                _record(
                    suite, f"{cmd} ({scope})", TestResult.SKIPPED, "license_required"
                )
            else:
                _record(
                    suite,
                    f"{cmd} ({scope})",
                    TestResult.FAILED,
                    f"{code}: {err.get('message', '')}",
                )


def test_blocked_commands(api: SerialStudioAPI, suite: TestSuite, available: set[str]):
    """The legacy TCP API does not enforce AI safety tags, but unknown
    commands and bad params must still come back as structured errors."""
    print(bold(info("\n[ Error-shape checks ]")))

    # Missing required parameter.
    if "io.uart.setBaudRate" in available:
        response = api.send_command("io.uart.setBaudRate")
        if response and not response.get("success"):
            code = response.get("error", {}).get("code", "")
            if code in (ErrorCode.MISSING_PARAM, ErrorCode.INVALID_PARAM):
                _record(
                    suite,
                    "missing required param -> structured error",
                    TestResult.PASSED,
                )
            else:
                _record(
                    suite,
                    "missing required param -> structured error",
                    TestResult.FAILED,
                    f"got {code}",
                )
        else:
            _record(
                suite,
                "missing required param -> structured error",
                TestResult.FAILED,
                "server accepted the command without baudRate",
            )
    else:
        _record(
            suite,
            "missing required param -> structured error",
            TestResult.SKIPPED,
            "io.uart.setBaudRate not registered",
        )

    # Wrong parameter type.
    if "io.uart.setBaudRate" in available:
        response = api.send_command(
            "io.uart.setBaudRate", params={"baudRate": "not-a-number"}
        )
        if response and not response.get("success"):
            _record(
                suite, "type-mismatched param -> structured error", TestResult.PASSED
            )
        else:
            _record(
                suite,
                "type-mismatched param -> structured error",
                TestResult.FAILED,
                "server accepted non-numeric baudRate",
            )
    else:
        _record(
            suite,
            "type-mismatched param -> structured error",
            TestResult.SKIPPED,
            "io.uart.setBaudRate not registered",
        )


def cmd_test(api: SerialStudioAPI, args) -> int:
    """Run the smoke test suite. Self-discovers the live command surface."""
    print(bold("=" * 60))
    print(bold("Serial Studio API Smoke Test"))
    print(bold("=" * 60))
    print()
    print(dim("This suite exercises a handful of read-only commands per scope"))
    print(dim("and checks protocol-level invariants. It does NOT verify every"))
    print(dim("command's behavior -- use the live registry (api.getCommands)"))
    print(dim("for that."))

    suite = TestSuite("Serial Studio API")

    try:
        test_protocol(api, suite)
        commands = test_command_registry(api, suite)

        if commands is not None:
            available = {c.get("name", "") for c in commands}
            test_smoke(api, suite, available)
            test_blocked_commands(api, suite, available)
    except KeyboardInterrupt:
        print(error("\n[INTERRUPTED] Tests cancelled by user"))
    except Exception as e:
        print(error(f"\n[FATAL] Unexpected error: {e}"))
        import traceback

        traceback.print_exc()

    suite.print_summary()
    return 0 if suite.failed == 0 else 1


# =============================================================================
# CLI Modes
# =============================================================================


def parse_params(param_list: list[str]) -> Optional[dict]:
    """
    Parse parameters from command line.
    Supports two formats:
      - JSON: '{"key": "value"}'
      - Key=Value pairs: key=value key2=value2

    For key=value, attempts to auto-convert types:
      - "true"/"false" -> bool
      - integers -> int
      - floats -> float
      - everything else -> string
    """
    if not param_list:
        return None

    joined = " ".join(param_list)
    stripped = joined.strip()
    if stripped.startswith("{"):
        try:
            return json.loads(stripped)
        except json.JSONDecodeError:
            # PowerShell often strips quotes around keys -- fix {key:value}
            import re

            fixed = re.sub(
                r"(\{|,)\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*:", r'\1"\2":', stripped
            )
            try:
                return json.loads(fixed)
            except json.JSONDecodeError as e:
                raise ValueError(
                    f"Invalid JSON: {e}\n"
                    "Tip: On PowerShell, use key=value format instead: -p baudRate=115200"
                )

    params = {}
    for param in param_list:
        if "=" not in param:
            raise ValueError(
                f"Invalid parameter format: '{param}'. Use key=value or JSON format."
            )
        key, value = param.split("=", 1)
        key = key.strip()
        value = value.strip()

        if value.lower() == "true":
            params[key] = True
        elif value.lower() == "false":
            params[key] = False
        else:
            try:
                params[key] = int(value)
            except ValueError:
                try:
                    params[key] = float(value)
                except ValueError:
                    params[key] = value

    return params


def cmd_send(api: SerialStudioAPI, args) -> int:
    """Send a single command and print the result."""
    params = None
    if args.params:
        try:
            params = parse_params(args.params)
        except ValueError as e:
            print(f"[ERROR] {e}", file=sys.stderr)
            return 1

    response = api.send_command(args.command, params)

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(response, indent=2))
    else:
        if response.get("success"):
            result = response.get("result", {})
            if result:
                print(json.dumps(result, indent=2))
            else:
                print("[OK] Command executed successfully")
        else:
            err = response.get("error", {})
            print(f"[ERROR] {err.get('code')}: {err.get('message')}", file=sys.stderr)
            return 1

    return 0


def cmd_batch(api: SerialStudioAPI, args) -> int:
    """Send batch commands from a JSON file."""
    try:
        with open(args.file, "r") as f:
            data = json.load(f)
    except FileNotFoundError:
        print(f"[ERROR] File not found: {args.file}", file=sys.stderr)
        return 1
    except json.JSONDecodeError as e:
        print(f"[ERROR] Invalid JSON in file: {e}", file=sys.stderr)
        return 1

    if isinstance(data, list):
        commands = data
    elif isinstance(data, dict) and "commands" in data:
        commands = data["commands"]
    else:
        print(
            '[ERROR] Expected array of commands or {"commands": [...]}', file=sys.stderr
        )
        return 1

    response = api.send_batch(commands)

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    if args.json:
        print(json.dumps(response, indent=2))
    else:
        results = response.get("results", [])
        success_count = sum(1 for r in results if r.get("success"))
        print(
            f"Executed {len(results)} commands: {success_count} succeeded, "
            f"{len(results) - success_count} failed"
        )
        for i, result in enumerate(results):
            status = chr(10003) if result.get("success") else chr(10007)
            cmd_id = result.get("id", f"#{i+1}")
            if result.get("success"):
                print(f"  {status} {cmd_id}")
            else:
                err = result.get("error", {})
                print(
                    f"  {status} {cmd_id}: {err.get('code')} - " f"{err.get('message')}"
                )

    return 0 if response.get("success") else 1


def _print_command_list(commands: list[dict], use_color: bool):
    """Group commands by scope and print them aligned."""
    groups: dict[str, list[tuple[str, str]]] = {}
    for cmd in commands:
        name = cmd.get("name", "")
        desc = cmd.get("description", "")
        parts = name.split(".")
        scope = ".".join(parts[:-1]) if len(parts) > 1 else "other"
        groups.setdefault(scope, []).append((name, desc))

    print(f"Available Commands ({len(commands)} total):\n")
    for scope in sorted(groups.keys()):
        header = f"  {scope}.*"
        print(info(header) if use_color else header)
        for name, desc in sorted(groups[scope]):
            short = (desc[:50] + "...") if len(desc) > 53 else desc
            print(f"    {name:<45} {dim(short) if use_color else short}")
        print()


def cmd_list(api: SerialStudioAPI, args) -> int:
    """List all available commands."""
    response = api.send_command("api.getCommands")

    if response is None:
        print("[ERROR] No response received", file=sys.stderr)
        return 1

    if not response.get("success"):
        err = response.get("error", {})
        print(f"[ERROR] {err.get('code')}: {err.get('message')}", file=sys.stderr)
        return 1

    commands = response.get("result", {}).get("commands", [])

    if args.json:
        print(json.dumps(commands, indent=2))
    else:
        _print_command_list(commands, use_color=COLORS_ENABLED)

    return 0


def cmd_interactive(api: SerialStudioAPI, args) -> int:
    """Interactive REPL with command history and tab completion."""
    history_file = os.path.expanduser("~/.serial_studio_api_history")
    available_commands: list[str] = []

    if READLINE_AVAILABLE:
        try:
            readline.read_history_file(history_file)
        except FileNotFoundError:
            pass
        readline.set_history_length(1000)

        response = api.send_command("api.getCommands")
        if response and response.get("success"):
            available_commands = [
                c.get("name", "")
                for c in response.get("result", {}).get("commands", [])
            ]

            def completer(text, state):
                options = [c for c in available_commands if c.startswith(text)]
                return options[state] if state < len(options) else None

            readline.parse_and_bind("tab: complete")
            readline.set_completer(completer)

    print(bold("\n=========================================================="))
    print(bold("        Serial Studio Interactive Mode (REPL)"))
    print(bold("==========================================================\n"))

    print(dim("Features:"))
    if READLINE_AVAILABLE:
        print(dim(f"  - {success(chr(10003))} Command history (up/down arrows)"))
        print(
            dim(
                f"  - {success(chr(10003))} Tab completion ({len(available_commands)} commands)"
            )
        )
    else:
        print(dim(f"  - {warning('!')} Install 'readline' for history & completion"))
    print(dim(f"  - Type {bold('help')} for built-in commands"))
    print(dim(f"  - Type {bold('list')} to see all API commands"))
    print(dim(f"  - Type {bold('quit')} to exit\n"))

    while True:
        try:
            line = input(info("ss> ")).strip()
        except (EOFError, KeyboardInterrupt):
            print("\n" + dim("Goodbye!"))
            break

        if not line:
            continue

        if line.lower() in ("quit", "exit", "q"):
            print(dim("Goodbye!"))
            break

        if line.lower() == "help":
            print(bold("\n" + "=" * 60))
            print(bold("Interactive Mode Commands"))
            print(bold("=" * 60))
            print(f"{info('  help')}                          Show this help")
            print(f"{info('  quit, exit, q')}                 Exit interactive mode")
            print(
                f"{info('  list')}                          List all available API commands"
            )
            print(f"{info('  clear')}                         Clear the screen")
            print(f"{info('  <command>')}                     Execute a command")
            print(f"{info('  <command> <json_params>')}       Execute with params")
            print(bold("\nExamples:"))
            print(dim("  io.getStatus"))
            print(dim('  io.uart.setBaudRate {"baudRate": 115200}'))
            print(dim('  io.network.setRemoteAddress {"address": "192.168.1.100"}'))
            print()
            continue

        if line.lower() == "clear":
            os.system("clear" if os.name != "nt" else "cls")
            continue

        if line.lower() == "list":
            response = api.send_command("api.getCommands")
            if response and response.get("success"):
                cmds = response.get("result", {}).get("commands", [])
                _print_command_list(cmds, use_color=COLORS_ENABLED)
            else:
                print(error("[ERROR] Could not fetch command list"))
            continue

        # <command> [<json_params>]
        parts = line.split(None, 1)
        command = parts[0]
        params = None

        if len(parts) > 1:
            try:
                params = json.loads(parts[1])
            except json.JSONDecodeError as e:
                print(error(f"[ERROR] Invalid JSON parameters: {e}"))
                print(dim('Tip: Use double quotes for JSON: {"key": "value"}'))
                continue

        response = api.send_command(command, params)

        if response is None:
            print(error("[ERROR] No response received"))
            continue

        if response.get("success"):
            result = response.get("result", {})
            if result:
                print(info(json.dumps(result, indent=2)))
            else:
                print(success("[OK]"))
        else:
            err = response.get("error", {})
            print(error(f"[ERROR] {err.get('code')}: {err.get('message')}"))

    if READLINE_AVAILABLE:
        try:
            readline.write_history_file(history_file)
        except Exception:
            pass

    return 0


def cmd_monitor(api: SerialStudioAPI, args) -> int:
    """Monitor Serial Studio in real-time: connection status, frame counters,
    and (optionally) raw incoming bytes."""
    print("=" * 60)
    print("Serial Studio Live Monitor")
    print("=" * 60)
    print("Press Ctrl+C to exit\n")

    update_interval = args.interval / 1000.0
    frame_count = 0
    raw_data_count = 0
    last_status: dict = {}
    last_update_time = time.time()

    try:
        while True:
            current_time = time.time()

            # Drain push notifications (frames + raw data)
            while api.has_data_available(timeout=0.0):
                msg = api.recv_message(timeout=0.1)
                if not msg:
                    continue

                if "data" in msg and "frames" not in msg:
                    raw_data_count += 1
                    if args.show_raw_data:
                        try:
                            decoded = base64.b64decode(msg["data"]).decode(
                                "utf-8", errors="replace"
                            )
                            display_text = decoded.replace("\n", "\\n").replace(
                                "\r", "\\r"
                            )
                            if len(display_text) > 100:
                                display_text = display_text[:97] + "..."
                            print(
                                f"\rRaw [{raw_data_count}]: "
                                f"{display_text}".ljust(120),
                                end="",
                                flush=True,
                            )
                        except Exception:
                            pass
                elif "frames" in msg:
                    frame_count += len(msg.get("frames", []))

            if current_time - last_update_time >= update_interval:
                last_update_time = current_time

                response = api.send_command("io.getStatus")
                if not response or not response.get("success"):
                    if api.verbose or args.compact:
                        print("[ERROR] Failed to get status (will retry)")
                    time.sleep(0.01)
                    continue

                status = response.get("result", {})

                if not args.compact and not args.show_raw_data:
                    print("\033[2J\033[H", end="")
                elif not args.show_raw_data:
                    print()

                if not args.show_raw_data:
                    print("=" * 60)
                    print(f"Serial Studio Live Monitor " f"(Update: {args.interval}ms)")
                    print("=" * 60)
                    print()

                connected = status.get("isConnected", False)
                paused = status.get("paused", False)
                conn_label = "CONNECTED" if connected else "DISCONNECTED"
                pause_label = " (paused)" if paused else ""

                if not args.show_raw_data:
                    print(f"Status: {conn_label}{pause_label}")
                    print(f"Bus Type: {status.get('busTypeName', 'Unknown')}")
                    config_ok = status.get("configurationOk", False)
                    print(f"Configuration: " f"{'OK' if config_ok else 'Not Ready'}")
                    rw = (
                        "Read/Write"
                        if status.get("readWrite", False)
                        else "Read-Only" if status.get("readOnly", False) else "None"
                    )
                    print(f"Mode: {rw}")

                    if connected:
                        print()
                        print("-" * 60)
                        bus_type = status.get("busType", 0)

                        if bus_type == 0:  # UART
                            uart = api.send_command("io.uart.getConfig")
                            if uart and uart.get("success"):
                                cfg = uart.get("result", {})
                                print(f"Port: {cfg.get('port', 'N/A')}")
                                print(f"Baud Rate: " f"{cfg.get('baudRate', 'N/A')}")
                                parity = (cfg.get("parity", "?") or "?")[0]
                                print(
                                    f"Config: {cfg.get('dataBits', '?')}"
                                    f"{parity}{cfg.get('stopBits', '?')}"
                                )

                        elif bus_type == 1:  # Network
                            net = api.send_command("io.network.getConfig")
                            if net and net.get("success"):
                                cfg = net.get("result", {})
                                socket_types = ["TCP", "UDP"]
                                idx = cfg.get("socketTypeIndex", 0)
                                stype = (
                                    socket_types[idx]
                                    if idx < len(socket_types)
                                    else "Unknown"
                                )
                                print(f"Type: {stype}")
                                print(f"Address: " f"{cfg.get('remoteAddress', 'N/A')}")
                                if idx == 0:
                                    print(f"Port: " f"{cfg.get('tcpPort', 'N/A')}")
                                else:
                                    print(
                                        f"Local Port: "
                                        f"{cfg.get('udpLocalPort', 'N/A')}"
                                    )
                                    print(
                                        f"Remote Port: "
                                        f"{cfg.get('udpRemotePort', 'N/A')}"
                                    )

                        elif bus_type == 2:  # Bluetooth LE
                            ble = api.send_command("io.ble.getConfig")
                            if ble and ble.get("success"):
                                cfg = ble.get("result", {})
                                print(f"Device: " f"{cfg.get('deviceName', 'N/A')}")
                                print(f"Service: " f"{cfg.get('serviceName', 'N/A')}")
                                print(
                                    f"Characteristic: "
                                    f"{cfg.get('characteristicName', 'N/A')}"
                                )

                    print()
                    print("-" * 60)

                    csv = api.send_command("csvExport.getStatus")
                    if csv and csv.get("success"):
                        cs = csv.get("result", {})
                        en = cs.get("enabled", False)
                        op = cs.get("isOpen", False)
                        label = (
                            "Active" if en and op else "Enabled" if en else "Disabled"
                        )
                        print(f"CSV Export: {label}")

                    if last_status and not args.compact:
                        if status.get("isConnected") != last_status.get("isConnected"):
                            print("\n[!] Connection state changed")
                        if status.get("paused") != last_status.get("paused"):
                            print("\n[!] Pause state changed")

                    print()
                    print("-" * 60)
                    print(
                        f"Frames: {frame_count} | Raw Data Packets: "
                        f"{raw_data_count} | Press Ctrl+C to exit"
                    )

                last_status = status

            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\n\nMonitoring stopped.")
        print(f"Total frames: {frame_count}")
        print(f"Total raw data packets: {raw_data_count}")
        return 0


# =============================================================================
# Main Entry Point
# =============================================================================


def main():
    parser = argparse.ArgumentParser(
        description="Serial Studio API Client & Test Suite",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s send io.getStatus                              # Send a command
  %(prog)s send io.uart.setBaudRate -p baudRate=115200    # With key=value params
  %(prog)s send io.uart.setDtrEnabled -p dtrEnabled=true  # Boolean param
  %(prog)s list                                           # List commands
  %(prog)s interactive                                    # Interactive mode
  %(prog)s monitor --interval 500                         # Live monitor
  %(prog)s batch commands.json                            # Batch from file
  %(prog)s test --verbose                                 # Run smoke tests
        """,
    )

    parser.add_argument(
        "--host", default=DEFAULT_HOST, help=f"Server host (default: {DEFAULT_HOST})"
    )
    parser.add_argument(
        "--port",
        type=int,
        default=DEFAULT_PORT,
        help=f"Server port (default: {DEFAULT_PORT})",
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose output"
    )
    parser.add_argument(
        "--json", "-j", action="store_true", help="Output raw JSON (for scripting)"
    )

    subparsers = parser.add_subparsers(dest="mode", help="Operation mode")

    send_parser = subparsers.add_parser("send", help="Send a single command")
    send_parser.add_argument("command", help="Command to send (e.g., io.getStatus)")
    send_parser.add_argument(
        "--params",
        "-p",
        nargs="+",
        metavar="PARAM",
        help="Parameters as key=value pairs or JSON. "
        "Examples: baudRate=115200 or '{\"baudRate\": 115200}'",
    )

    batch_parser = subparsers.add_parser(
        "batch", help="Send batch commands from JSON file"
    )
    batch_parser.add_argument("file", help="JSON file containing commands")

    subparsers.add_parser("list", help="List all available commands")

    subparsers.add_parser(
        "interactive", aliases=["i", "repl"], help="Interactive REPL mode"
    )

    monitor_parser = subparsers.add_parser(
        "monitor", aliases=["m", "live"], help="Live data monitor"
    )
    monitor_parser.add_argument(
        "--interval",
        type=int,
        default=1000,
        metavar="MS",
        help="Update interval in milliseconds (default: 1000)",
    )
    monitor_parser.add_argument(
        "--compact",
        action="store_true",
        help="Compact mode (append updates instead of clearing screen)",
    )
    monitor_parser.add_argument(
        "--show-raw-data",
        action="store_true",
        help="Show raw device data on a single line "
        "(uses \\r for continuous update)",
    )

    subparsers.add_parser("test", help="Run the API smoke test suite")

    args = parser.parse_args()

    if args.mode is None:
        parser.print_help()
        print(
            "\nTip: Use 'interactive' for a REPL, or "
            "'send <command>' to execute a command."
        )
        return 0

    api = SerialStudioAPI(args.host, args.port, args.verbose)

    if not args.json:
        print(f"Connecting to {args.host}:{args.port}...", file=sys.stderr)

    if not api.connect():
        if args.json:
            print(json.dumps({"error": "Connection failed"}))
        else:
            print("\n[ERROR] Could not connect to Serial Studio.", file=sys.stderr)
            print("Please ensure:", file=sys.stderr)
            print("  1. Serial Studio is running", file=sys.stderr)
            print(
                "  2. API Server is enabled "
                "(Settings -> Miscellaneous -> Enable API Server)",
                file=sys.stderr,
            )
            print(f"  3. The server is listening on port {args.port}", file=sys.stderr)
        return 1

    if not args.json:
        print("Connected!\n", file=sys.stderr)

    try:
        if args.mode == "send":
            return cmd_send(api, args)
        elif args.mode == "batch":
            return cmd_batch(api, args)
        elif args.mode == "list":
            return cmd_list(api, args)
        elif args.mode in ("interactive", "i", "repl"):
            return cmd_interactive(api, args)
        elif args.mode in ("monitor", "m", "live"):
            return cmd_monitor(api, args)
        elif args.mode == "test":
            return cmd_test(api, args)
        else:
            parser.print_help()
            return 1
    finally:
        api.disconnect()


if __name__ == "__main__":
    sys.exit(main())
