#!/usr/bin/env python3
"""
Serial Studio MCP Bridge for Claude Desktop

This script creates a bidirectional bridge between Claude Desktop (stdio)
and Serial Studio's TCP API server (port 7777), enabling Claude to control
Serial Studio using the Model Context Protocol.

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

Distribution:
  The Serial Studio plugin for the Claude directory (Serial-Studio/claude-plugin)
  ships this same file as bridge.py; keep the two byte-identical.
  SERIAL_STUDIO_API_PORT overrides the port (the unit test aims it at a closed one).

License:
  Dual-licensed under GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
RECONNECT_DELAY = 2  # seconds
GIVE_UP_AFTER = 6  # seconds before the last attempt; see connect_to_serial_studio


def log(message):
    """Log messages to stderr (visible in Claude Desktop logs)"""
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    print(
        f"[{timestamp}] [Serial Studio Bridge] {message}", file=sys.stderr, flush=True
    )


def is_jsonrpc(line):
    """True for a JSON-RPC 2.0 message or batch; anything else is not for the MCP client"""
    try:
        message = json.loads(line)
    except ValueError:
        return False
    if isinstance(message, list):
        return True
    return isinstance(message, dict) and message.get("jsonrpc") == "2.0"


def tcp_to_stdout(sock, should_stop):
    """
    Forward messages from Serial Studio (TCP) to Claude Desktop (stdout)

    Args:
        sock: TCP socket connected to Serial Studio
        should_stop: Threading event to signal shutdown
    """
    buffer = b""
    dropped = 0
    try:
        while not should_stop.is_set():
            try:
                # Non-blocking receive with timeout
                sock.settimeout(0.5)
                chunk = sock.recv(4096)

                if not chunk:
                    log("Serial Studio closed connection")
                    break

                buffer += chunk

                # Process complete lines (newline-delimited JSON-RPC messages)
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    if not line.strip():
                        continue

                    if not is_jsonrpc(line):
                        dropped += 1
                        continue

                    sys.stdout.buffer.write(line + b"\n")
                    sys.stdout.buffer.flush()

            except socket.timeout:
                continue  # Check should_stop flag
            except Exception as e:
                log(f"TCP→stdout error: {e}")
                break

    except Exception as e:
        log(f"Fatal TCP→stdout error: {e}")
    finally:
        if dropped:
            log(f"Dropped {dropped} non-JSON-RPC line(s) from Serial Studio")
        should_stop.set()


def stdin_to_tcp(sock, should_stop):
    """
    Forward messages from Claude Desktop (stdin) to Serial Studio (TCP)

    Args:
        sock: TCP socket connected to Serial Studio
        should_stop: Threading event to signal shutdown
    """
    try:
        # Read from stdin line by line
        for line in sys.stdin.buffer:
            if should_stop.is_set():
                break

            if line.strip():
                try:
                    # Forward to Serial Studio via TCP
                    sock.sendall(line)
                except Exception as e:
                    log(f"stdin→TCP send error: {e}")
                    break

    except Exception as e:
        log(f"Fatal stdin→TCP error: {e}")
    finally:
        should_stop.set()


def connect_to_serial_studio(give_up_after=GIVE_UP_AFTER):
    """
    Connect to Serial Studio's TCP server, retrying until a deadline passes.

    The bound is a deadline rather than an attempt count because Windows spends
    about two seconds on every refused loopback connect; with the last attempt
    starting before the deadline, the bridge gives up within ten seconds on
    every platform (what an MCP client needs to show the cause instead of a
    hung server).

    Args:
        give_up_after: Seconds after which no new attempt starts

    Returns:
        socket.socket: Connected socket, or None on failure
    """
    deadline = time.monotonic() + give_up_after
    attempt = 0
    while True:
        attempt += 1
        log(
            f"Connecting to Serial Studio at {SERIAL_STUDIO_HOST}:{SERIAL_STUDIO_PORT} (attempt {attempt})..."
        )
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.connect((SERIAL_STUDIO_HOST, SERIAL_STUDIO_PORT))
            log("✓ Connected to Serial Studio")
            return sock
        except ConnectionRefusedError:
            log("Connection refused")
        except Exception as e:
            log(f"✗ Connection error: {e}")
        sock.close()

        remaining = deadline - time.monotonic()
        if remaining <= 0:
            log("✗ Failed to connect to Serial Studio")
            log("  Make sure Serial Studio is running and API server is enabled")
            return None
        time.sleep(min(RECONNECT_DELAY, remaining))


def main():
    """Main bridge process"""
    log("Serial Studio MCP Bridge starting...")

    # Connect to Serial Studio
    sock = connect_to_serial_studio()
    if sock is None:
        log(
            "Enable the API server: Preferences → General → API & Plugins → Enable API Server"
        )
        sys.exit(1)

    # Create stop event for graceful shutdown
    should_stop = threading.Event()

    # Start bidirectional forwarding threads
    tcp_thread = threading.Thread(
        target=tcp_to_stdout, args=(sock, should_stop), daemon=True, name="TCP→stdout"
    )
    tcp_thread.start()

    stdin_thread = threading.Thread(
        target=stdin_to_tcp, args=(sock, should_stop), daemon=True, name="stdin→TCP"
    )
    stdin_thread.start()

    log("✓ Bridge active - Claude can now control Serial Studio")
    log("  Tools: every listed API command (tools/list)")
    log(f"  Resources: serialstudio://frame/current, serialstudio://frame/history")

    # Wait for threads to complete or stop signal
    try:
        # Keep main thread alive until one of the worker threads stops
        while not should_stop.is_set():
            time.sleep(0.5)

    except KeyboardInterrupt:
        log("Bridge stopped by user (Ctrl+C)")

    finally:
        # Clean shutdown
        should_stop.set()
        sock.close()
        log("Bridge closed")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        log(f"Fatal error: {e}")
        sys.exit(1)
