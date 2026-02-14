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
  - Serial Studio running with API server enabled (Settings → Enable API Server)

License:
  Dual-licensed under GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
  See Serial Studio's LICENSE.md for details
"""

import sys
import socket
import threading
import time

# Configuration
SERIAL_STUDIO_HOST = "localhost"
SERIAL_STUDIO_PORT = 7777
RECONNECT_DELAY = 2  # seconds
MAX_RECONNECT_ATTEMPTS = 5


def log(message):
    """Log messages to stderr (visible in Claude Desktop logs)"""
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] [Serial Studio Bridge] {message}", file=sys.stderr, flush=True)


def tcp_to_stdout(sock, should_stop):
    """
    Forward messages from Serial Studio (TCP) to Claude Desktop (stdout)

    Args:
        sock: TCP socket connected to Serial Studio
        should_stop: Threading event to signal shutdown
    """
    buffer = b""
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
                while b'\n' in buffer:
                    line, buffer = buffer.split(b'\n', 1)
                    if line.strip():
                        # Forward to Claude Desktop via stdout
                        sys.stdout.buffer.write(line + b'\n')
                        sys.stdout.buffer.flush()

            except socket.timeout:
                continue  # Check should_stop flag
            except Exception as e:
                log(f"TCP→stdout error: {e}")
                break

    except Exception as e:
        log(f"Fatal TCP→stdout error: {e}")
    finally:
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


def connect_to_serial_studio(max_attempts=MAX_RECONNECT_ATTEMPTS):
    """
    Connect to Serial Studio TCP server with retry logic

    Args:
        max_attempts: Maximum number of connection attempts

    Returns:
        socket.socket: Connected socket, or None on failure
    """
    for attempt in range(1, max_attempts + 1):
        try:
            log(f"Connecting to Serial Studio at {SERIAL_STUDIO_HOST}:{SERIAL_STUDIO_PORT} (attempt {attempt}/{max_attempts})...")
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((SERIAL_STUDIO_HOST, SERIAL_STUDIO_PORT))
            log("✓ Connected to Serial Studio")
            return sock

        except ConnectionRefusedError:
            if attempt < max_attempts:
                log(f"Connection refused, retrying in {RECONNECT_DELAY} seconds...")
                time.sleep(RECONNECT_DELAY)
            else:
                log("✗ Failed to connect to Serial Studio")
                log("  Make sure Serial Studio is running and API server is enabled")
                log("  (Settings → Enable API Server)")

        except Exception as e:
            log(f"✗ Connection error: {e}")
            if attempt < max_attempts:
                time.sleep(RECONNECT_DELAY)

    return None


def main():
    """Main bridge process"""
    log("Serial Studio MCP Bridge starting...")

    # Connect to Serial Studio
    sock = connect_to_serial_studio()
    if sock is None:
        sys.exit(1)

    # Create stop event for graceful shutdown
    should_stop = threading.Event()

    # Start bidirectional forwarding threads
    tcp_thread = threading.Thread(
        target=tcp_to_stdout,
        args=(sock, should_stop),
        daemon=True,
        name="TCP→stdout"
    )
    tcp_thread.start()

    stdin_thread = threading.Thread(
        target=stdin_to_tcp,
        args=(sock, should_stop),
        daemon=True,
        name="stdin→TCP"
    )
    stdin_thread.start()

    log("✓ Bridge active - Claude can now control Serial Studio")
    log(f"  Available tools: 182+ commands")
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
