#!/usr/bin/env python3
"""
Serial Studio MCP Bridge

This script creates a bidirectional bridge between an MCP client on stdio
(Claude Code, Cowork, Claude Desktop or any other host) and Serial Studio's TCP
API server (port 7777), enabling Claude to control Serial Studio using the
Model Context Protocol.

Usage:
  Add this to your Claude Desktop configuration file:

  macOS: ~/Library/Application Support/Claude/claude_desktop_config.json
  Windows: %APPDATA%\\Claude\\claude_desktop_config.json
  Linux: ~/.config/Claude/claude_desktop_config.json

  {
    "mcpServers": {
      "serial-studio": {
        "command": "python3",
        "args": ["/absolute/path/to/claude_desktop_bridge.py"]
      }
    }
  }

Requirements:
  - Python 3.7 or later
  - Serial Studio running with API server enabled
    (Preferences → General → API & Plugins → Enable API Server)

While Serial Studio is away:
  The bridge keeps running and keeps dialing, so starting Serial Studio after the
  MCP client, or restarting it mid-session, needs no reconnect on the client side.
  Until the link is up the bridge answers the MCP handshake itself, lists only
  bridge.getStatus and fails every other call with the fix, so the client shows
  the cause instead of a hung server. Once Serial Studio answers, the bridge
  replays the handshake and sends the list_changed notifications.

Distribution:
  The Serial Studio plugin for the Claude directory (Serial-Studio/claude-plugin)
  ships this same file as bridge.py; keep the two byte-identical.
  SERIAL_STUDIO_API_PORT overrides the port (the unit tests aim it at fake servers).

License:
  Dual-licensed under GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
  See Serial Studio's LICENSE.md for details
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
import os
import socket
import threading
import time

# Configuration
SERIAL_STUDIO_HOST = "localhost"
SERIAL_STUDIO_PORT = int(os.environ.get("SERIAL_STUDIO_API_PORT", "7777"))
RECONNECT_DELAY = 2  # seconds between attempts while Serial Studio is away
LINK_TIMEOUT = 30  # seconds a send or a reconnect handshake may stall
LATEST_PROTOCOL_VERSION = "2025-06-18"

FIX = "Enable the API server: Preferences → General → API & Plugins → Enable API Server"
UNREACHABLE = (
    f"Serial Studio is not reachable on port {SERIAL_STUDIO_PORT}. Start it if it is "
    f"closed. {FIX}. Its tools appear automatically once it answers."
)

STATUS_TOOL = {
    "name": "bridge.getStatus",
    "title": "Bridge status",
    "description": (
        "Report whether the bridge can reach Serial Studio, and how to fix it when it "
        "cannot. Listed only while Serial Studio is away."
    ),
    "inputSchema": {"type": "object", "properties": {}},
    "annotations": {
        "readOnlyHint": True,
        "destructiveHint": False,
        "openWorldHint": False,
    },
}

OFFLINE_LISTS = {
    "tools/list": {"tools": [STATUS_TOOL]},
    "resources/list": {"resources": []},
    "resources/templates/list": {"resourceTemplates": []},
    "prompts/list": {"prompts": []},
}
OFFLINE_FAILURES = ("resources/read", "prompts/get", "completion/complete")


def log(message):
    """Log messages to stderr (visible in the MCP client's logs)"""
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    print(
        f"[{timestamp}] [Serial Studio Bridge] {message}", file=sys.stderr, flush=True
    )


def encode(message):
    """One JSON-RPC message as compact UTF-8 bytes, without the line terminator"""
    return json.dumps(message, separators=(",", ":")).encode("utf-8")


def parse_jsonrpc(line):
    """The JSON-RPC 2.0 message or batch in line; None is not for the MCP client"""
    try:
        message = json.loads(line)
    except ValueError:
        return None
    if isinstance(message, list):
        return message
    if isinstance(message, dict) and message.get("jsonrpc") == "2.0":
        return message
    return None


def success(request_id, result):
    return encode({"jsonrpc": "2.0", "id": request_id, "result": result})


def failure(request_id, code, message):
    error = {"code": code, "message": message}
    return encode({"jsonrpc": "2.0", "id": request_id, "error": error})


def text_result(text, is_error=False):
    return {"content": [{"type": "text", "text": text}], "isError": is_error}


def offline_initialize(params):
    """The initialize result the bridge gives while Serial Studio is away"""
    offered = params.get("protocolVersion") if isinstance(params, dict) else None
    listing = {"listChanged": True}
    return {
        "protocolVersion": offered or LATEST_PROTOCOL_VERSION,
        "capabilities": {"tools": listing, "resources": listing, "prompts": listing},
        "serverInfo": {"name": "Serial Studio bridge", "version": "waiting"},
        "instructions": UNREACHABLE,
    }


def offline_reply(message):
    """The answer to a client request while Serial Studio is away, None for no answer"""
    method = message.get("method")
    if method is None or "id" not in message:
        return None

    request_id = message["id"]
    if method == "initialize":
        return success(request_id, offline_initialize(message.get("params")))
    if method == "ping":
        return success(request_id, {})
    if method in OFFLINE_LISTS:
        return success(request_id, OFFLINE_LISTS[method])
    if method == "tools/call":
        return success(request_id, text_result(UNREACHABLE, True))
    if method in OFFLINE_FAILURES:
        return failure(request_id, -32000, UNREACHABLE)
    return failure(request_id, -32601, f"Unknown method: {method}")


def advertise_list_changes(message):
    """Re-encode an initialize reply so the client re-reads its lists after a reconnect"""
    result = message.get("result")
    capabilities = result.get("capabilities") if isinstance(result, dict) else None
    for name in ("tools", "resources", "prompts"):
        if isinstance(capabilities, dict) and isinstance(capabilities.get(name), dict):
            capabilities[name]["listChanged"] = True
    return encode(message)


def try_connect():
    """One attempt at Serial Studio's TCP server: the socket, or None"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(LINK_TIMEOUT)
    try:
        sock.connect((SERIAL_STUDIO_HOST, SERIAL_STUDIO_PORT))
    except OSError:
        sock.close()
        return None
    return sock


def abandon(sock):
    """Wake the link thread, which alone closes the socket and reports the loss"""
    try:
        sock.shutdown(socket.SHUT_RDWR)
    except OSError:
        pass


class Bridge:
    """Owns the link to Serial Studio and what the MCP client sees of it"""

    def __init__(self):
        self.lock = threading.Lock()
        self.stdout_lock = threading.Lock()
        self.sock = None  # published only once Serial Studio can take requests
        self.init_params = None  # the client's initialize, replayed on reconnect
        self.init_key = None  # the client's initialize while Serial Studio answers
        self.replay_id = None  # the bridge's own initialize during a reconnect
        self.replays = 0
        self.pending = {}  # encoded id of each forwarded request → its method
        self.dropped = 0

    def write_to_client(self, data):
        """Write one message to the MCP client; a closed pipe ends the bridge"""
        try:
            with self.stdout_lock:
                sys.stdout.buffer.write(data + b"\n")
                sys.stdout.buffer.flush()
        except (OSError, ValueError):
            log("MCP client closed its pipe")
            os._exit(0)

    def report_dropped(self):
        if self.dropped:
            log(f"Dropped {self.dropped} non-JSON-RPC line(s) from Serial Studio")
            self.dropped = 0

    # --- MCP client → Serial Studio (main thread) ---------------------------------

    def run_client(self):
        """Forward the client's messages until it closes stdin"""
        for line in sys.stdin.buffer:
            if line.strip():
                self.handle_client_line(line)
        self.report_dropped()

    def answer_status(self, request):
        """Answer bridge.getStatus locally; True when request was that call"""
        params = request.get("params")
        name = params.get("name") if isinstance(params, dict) else None
        if request.get("method") != "tools/call" or name != STATUS_TOOL["name"]:
            return False

        text = "Connected to Serial Studio." if self.sock else UNREACHABLE
        self.write_to_client(success(request.get("id"), text_result(text)))
        return True

    def handle_client_line(self, line):
        message = parse_jsonrpc(line)
        request = message if isinstance(message, dict) else {}
        if self.answer_status(request):
            return

        method = request.get("method")
        with self.lock:
            sock = self.sock
            if method == "initialize":
                params = request.get("params")
                self.init_params = params if isinstance(params, dict) else {}
                if sock is not None:
                    self.init_key = json.dumps(request.get("id"))
            if sock is not None and method is not None and "id" in request:
                self.pending[json.dumps(request["id"])] = method

        if sock is None:
            reply = offline_reply(request)
            if reply is not None:
                self.write_to_client(reply)
            return

        try:
            sock.sendall(line)
        except OSError as e:
            # Part of the line may have gone out, so this stream cannot be reused
            log(f"stdin→TCP send error: {e}")
            abandon(sock)

    # --- Serial Studio → MCP client (link thread) ---------------------------------

    def run_link(self, sock):
        """Serve the link for the life of the process, redialing whenever it drops"""
        while True:
            if sock is None:
                sock = self.dial()
                self.begin_session(sock)
            self.pump(sock)
            sock = None
            time.sleep(RECONNECT_DELAY)

    @staticmethod
    def dial():
        while True:
            sock = try_connect()
            if sock is not None:
                log("✓ Connected to Serial Studio")
                return sock
            time.sleep(RECONNECT_DELAY)

    def begin_session(self, sock):
        """Publish a new link, replaying the client's handshake first if it had one"""
        with self.lock:
            if self.init_params is None:
                self.sock = sock
                return
            self.replays += 1
            self.replay_id = f"bridge-replay-{self.replays}"
            params = self.init_params

        request = {
            "jsonrpc": "2.0",
            "id": self.replay_id,
            "method": "initialize",
            "params": params,
        }
        try:
            sock.sendall(encode(request) + b"\n")
        except OSError as e:
            log(f"Reconnect handshake failed: {e}")
            abandon(sock)

    def pump(self, sock):
        """Forward Serial Studio's newline-delimited messages until the link ends"""
        buffer = b""
        try:
            while True:
                try:
                    chunk = sock.recv(65536)
                except socket.timeout:
                    if self.replay_id is None:
                        continue
                    log("Serial Studio did not answer the reconnect handshake")
                    break

                if not chunk:
                    log("Serial Studio closed connection")
                    break

                buffer += chunk
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    if line.strip():
                        self.handle_app_line(line, sock)
        except OSError as e:
            log(f"TCP→stdout error: {e}")
        finally:
            self.end_session(sock)

    def handle_app_line(self, line, sock):
        message = parse_jsonrpc(line)
        if message is None:
            self.dropped += 1
            return

        if isinstance(message, dict) and "method" not in message and "id" in message:
            if self.replay_id is not None and message["id"] == self.replay_id:
                self.finish_replay(message, sock)
                return
            line = self.settle_request(message, line)

        self.write_to_client(line)

    def settle_request(self, message, line):
        """Retire an answered request; the reply to initialize also gains listChanged"""
        key = json.dumps(message["id"])
        with self.lock:
            self.pending.pop(key, None)
            is_initialize = key == self.init_key
            if is_initialize:
                self.init_key = None
        return advertise_list_changes(message) if is_initialize else line

    def finish_replay(self, message, sock):
        """Open a reconnected link for requests and have the client re-read its lists"""
        if "error" in message:
            log(f"Serial Studio refused the reconnect handshake: {message['error']}")
            abandon(sock)
            return

        initialized = {"jsonrpc": "2.0", "method": "notifications/initialized"}
        sock.sendall(encode(initialized) + b"\n")
        with self.lock:
            self.replay_id = None
            self.sock = sock

        log("✓ MCP session restored")
        for kind in ("tools", "resources", "prompts"):
            notification = {
                "jsonrpc": "2.0",
                "method": f"notifications/{kind}/list_changed",
            }
            self.write_to_client(encode(notification))

    def end_session(self, sock):
        """Take a lost link out of service and fail whatever was waiting on it"""
        with self.lock:
            if self.sock is sock:
                self.sock = None
            self.replay_id = None
            self.init_key = None
            pending, self.pending = self.pending, {}

        sock.close()
        self.report_dropped()
        log(FIX)
        for key, method in pending.items():
            reply = offline_reply({"id": json.loads(key), "method": method})
            if reply is not None:
                self.write_to_client(reply)


def main():
    """Main bridge process"""
    log("Serial Studio MCP Bridge starting...")
    log(f"Connecting to Serial Studio at {SERIAL_STUDIO_HOST}:{SERIAL_STUDIO_PORT}...")

    bridge = Bridge()
    sock = try_connect()
    if sock is None:
        log("Serial Studio is not reachable yet; retrying until it is")
        log(FIX)
    else:
        log("✓ Connected to Serial Studio")
        log("✓ Bridge active - Claude can now control Serial Studio")
        log("  Tools: every listed API command (tools/list)")
        log("  Resources: serialstudio://frame/current, serialstudio://frame/history")
        bridge.begin_session(sock)

    threading.Thread(
        target=bridge.run_link, args=(sock,), daemon=True, name="TCP→stdout"
    ).start()

    bridge.run_client()
    log("Bridge closed")


if __name__ == "__main__":
    status = 0
    try:
        main()
    except KeyboardInterrupt:
        log("Bridge stopped by user (Ctrl+C)")
    except Exception as e:
        log(f"Fatal error: {e}")
        status = 1

    # A daemon thread still inside a buffered stream aborts a normal shutdown
    os._exit(status)
