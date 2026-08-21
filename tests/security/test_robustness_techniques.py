#!/usr/bin/env python3
"""
Advanced Probing Suite for Serial Studio

This module contains sophisticated probe techniques designed to bypass
security controls and probe subtle weaknesses.

Probe categories:
- Race conditions and TOCTOU probes
- Integer overflows and underflows
- Memory corruption attempts
- Parser state confusion
- Side-channel timing probes
- Thread exhaustion
- File descriptor exhaustion
- Logic bugs in state machines

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import threading
import time
import uuid
import sys
import struct
from pathlib import Path
from typing import List
import base64
import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError


class AdvancedProber:
    """Advanced probing techniques"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.probes = []
        self.crashes = []

    def log_probe(self, name, success, details):
        """Log probing attempt"""
        result = {"name": name, "success": success, "details": details}
        self.probes.append(result)
        if success:
            print(f"[FLAGGED] {name}: {details}")
        else:
            print(f"[BLOCKED] {name}: {details}")


@pytest.mark.timeout(60)
def test_race_conditions(prober):
    """Probe race conditions in connection/disconnection"""
    print("\n[*] Testing race condition probes...")

    # Probe 1: TOCTOU - Connect/Disconnect race
    print("  - TOCTOU probe on connect/disconnect state...")

    stop_flag = threading.Event()

    def rapid_toggle():
        try:
            with SerialStudioClient(timeout=1.0) as client:
                for _ in range(500):
                    if stop_flag.is_set():
                        break

                    try:
                        client.command("io.connect")
                        client.command("io.disconnect")
                    except:
                        pass
        except:
            pass

    # Launch 20 threads to create race condition
    threads = [threading.Thread(target=rapid_toggle, daemon=True) for _ in range(20)]

    for t in threads:
        t.start()

    time.sleep(3)
    stop_flag.set()

    # Check if server crashed
    try:
        with SerialStudioClient() as client:
            client.command("api.getCommands")
            prober.log_probe(
                "TOCTOU Connect/Disconnect",
                False,
                "Server survived race condition",
            )
    except:
        prober.log_probe(
            "TOCTOU Connect/Disconnect", True, "Server crashed or unresponsive!"
        )

    for t in threads:
        t.join(timeout=5)

    # Probe 2: Race on configuration changes
    print("  - Racing configuration changes...")

    def race_config():
        try:
            with SerialStudioClient() as client:
                for i in range(100):
                    client.command("dashboard.setFps", {"fps": i % 60 + 1})
                    client.command("dashboard.setTimeRange", {"seconds": i % 60 + 1})
        except:
            pass

    threads = [threading.Thread(target=race_config) for _ in range(10)]
    for t in threads:
        t.start()
    for t in threads:
        t.join(timeout=5)

    # Probe 3: Race on frame parser reset
    print("  - Racing frame parser resets...")

    def race_parser():
        try:
            with SerialStudioClient() as client:
                sequences = [b"/*", b"*/", b"@@", b"##", b"{{", b"}}"]
                for i in range(50):
                    start = sequences[i % len(sequences)]
                    end = sequences[(i + 1) % len(sequences)]
                    client.configure_frame_parser(
                        start_sequence=start.decode(),
                        end_sequence=end.decode(),
                        operation_mode=i % 3,
                    )
        except:
            pass

    threads = [threading.Thread(target=race_parser) for _ in range(5)]
    for t in threads:
        t.start()
    for t in threads:
        t.join(timeout=10)

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


@pytest.mark.timeout(90)
def test_integer_overflow(prober):
    """Test integer overflow weaknesses"""
    print("\n[*] Testing integer overflow probes...")

    # Probe 1: Overflow FPS value
    print("  - Testing FPS integer overflow...")
    overflow_values = [
        2147483647,  # Max int32
        2147483648,  # Max int32 + 1
        4294967295,  # Max uint32
        4294967296,  # Max uint32 + 1
        9223372036854775807,  # Max int64
        -2147483648,  # Min int32
        -9223372036854775808,  # Min int64
    ]

    for val in overflow_values:
        try:
            with SerialStudioClient(timeout=3.0) as client:
                result = client.command("dashboard.setFps", {"fps": val})
                prober.log_probe("Integer Overflow FPS", True, f"Accepted value: {val}")
        except (APIError, TimeoutError, ConnectionError):
            # Server rejected or timed out - expected for invalid values
            pass

    # Probe 2: Overflow time range value
    print("  - Testing time range integer overflow...")
    for val in overflow_values:
        try:
            with SerialStudioClient(timeout=3.0) as client:
                result = client.command("dashboard.setTimeRange", {"seconds": val})
                prober.log_probe(
                    "Integer Overflow TimeRange", True, f"Accepted value: {val}"
                )
        except (APIError, TimeoutError, ConnectionError):
            # Server rejected or timed out - expected for invalid values
            pass

    # Probe 3: Negative array indices
    print("  - Testing negative array access...")
    try:
        with SerialStudioClient() as client:
            # Try to access negative indices through batch
            batch = [{"command": "api.getCommands"} for _ in range(-1, 10)]
            client.batch(batch)
    except:
        pass

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_memory_corruption(prober):
    """Attempt memory corruption through buffer overflows"""
    print("\n[*] Testing memory corruption probes...")

    # Probe 1: Overflow frame delimiters
    print("  - Overflowing frame delimiter buffers...")
    huge_delimiter = "A" * (1024 * 1024 * 10)  # 10MB delimiter

    try:
        with SerialStudioClient() as client:
            client.configure_frame_parser(
                start_sequence=huge_delimiter,
                end_sequence=huge_delimiter,
                operation_mode=0,
            )
            prober.log_probe(
                "Delimiter Buffer Overflow", True, "10MB delimiter accepted"
            )
    except:
        prober.log_probe("Delimiter Buffer Overflow", False, "Huge delimiter rejected")

    # Probe 2: Corrupt JSON with extreme nesting
    print("  - Deep JSON nesting to overflow stack...")
    depth = 100000
    nested = '{"a":' * depth + "1" + "}" * depth

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((prober.host, prober.port))
        sock.sendall(nested.encode() + b"\n")
        sock.settimeout(2.0)
        response = sock.recv(4096)
        sock.close()

        if not response or b"error" not in response.lower():
            prober.log_probe(
                "JSON Stack Overflow", True, f"{depth} level nesting accepted"
            )
    except:
        pass

    # Probe 3: Unicode buffer overflow
    print("  - Unicode normalization expansion...")
    # Characters that expand significantly during normalization
    unicode_amplification = "\u0061\u0301" * 100000  # á repeated

    try:
        with SerialStudioClient() as client:
            result = client.command(
                "project.setTitle", {"title": unicode_amplification}
            )
            prober.log_probe(
                "Unicode Normalization expansion",
                True,
                "Unicode amplification accepted",
            )
    except:
        pass

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_parser_confusion(prober):
    """Probe parser state machine bugs"""
    print("\n[*] Testing parser confusion probes...")

    # Probe 1: Delimiter injection
    print("  - Injecting delimiters in data stream...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((prober.host, prober.port))

        # Configure parser with /* */ delimiters
        with SerialStudioClient() as client:
            client.configure_frame_parser(
                start_sequence="/*", end_sequence="*/", operation_mode=0
            )

        # Send data with embedded delimiters to confuse parser
        confused = b"/*FRAME1*//*FRAME2*//*INCOMPLETE"
        confused += b"/**//**/"  # Empty frames
        confused += b"/*" * 1000 + b"*/"  # Many opens, one close

        sock.sendall(confused)
        time.sleep(1)

        # Check if server is confused
        msg = '{"type":"command","id":"test","command":"api.getCommands"}\n'
        sock.sendall(msg.encode())
        sock.settimeout(2.0)
        response = sock.recv(4096)
        sock.close()

        if response:
            prober.log_probe(
                "Parser Delimiter Confusion", False, "Parser handled confusion"
            )
    except:
        prober.log_probe("Parser Delimiter Confusion", True, "Parser crashed or hung!")

    # Probe 2: Newline injection
    print("  - Newline injection in JSON strings...")
    payloads = [
        '{"type":"command","id":"test\nINJECTED","command":"api.getCommands"}\n',
        '{"type":"command","id":"test\r\nINJECTED","command":"api.getCommands"}\n',
        '{"type":"command","id":"test\x00NULL","command":"api.getCommands"}\n',
    ]

    for payload in payloads:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            sock.sendall(payload.encode())
            sock.settimeout(1.0)
            response = sock.recv(4096)
            sock.close()
        except:
            pass

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_timing_probes(prober):
    """Side-channel timing probes"""
    print("\n[*] Testing timing side-channel probes...")

    # Probe 1: Command enumeration via timing
    print("  - Command enumeration via timing analysis...")

    def measure_timing(command):
        # Reuse one connection across all samples: opening a fresh socket per
        # iteration churns 50 connect/close cycles, which stalls under TIME_WAIT
        # on macOS runners and trips the global test timeout.
        times = []
        with SerialStudioClient() as client:
            for _ in range(50):
                start = time.perf_counter()
                try:
                    client.command(command)
                except:
                    pass
                elapsed = time.perf_counter() - start
                times.append(elapsed)
        return sum(times) / len(times)

    # Valid vs invalid commands should have different timing
    valid_time = measure_timing("api.getCommands")
    invalid_time = measure_timing("nonexistent.command.12345")

    timing_diff = abs(valid_time - invalid_time)
    if timing_diff > 0.01:  # 10ms difference, well above loopback jitter
        prober.log_probe(
            "Timing Side Channel",
            True,
            f"Command existence leaked via timing: {timing_diff*1000:.2f}ms diff",
        )

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


@pytest.mark.timeout(120)
def test_resource_exhaustion(prober):
    """Advanced resource exhaustion"""
    print("\n[*] Testing advanced resource exhaustion...")

    # Probe 1: File descriptor exhaustion
    print("  - Exhausting file descriptors...")
    sockets = []
    try:
        for i in range(1000):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            sockets.append(sock)
            # Don't send data, just hold connections open
    except Exception as e:
        print(f"    Opened {len(sockets)} sockets before: {e}")

    # Check if server still accepts new connections
    try:
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        test_sock.connect((prober.host, prober.port))
        test_sock.close()
        prober.log_probe(
            "FD Exhaustion", False, f"Server survived {len(sockets)} connections"
        )
    except:
        prober.log_probe("FD Exhaustion", True, "Server refusing connections!")

    # Cleanup
    for sock in sockets:
        try:
            sock.close()
        except:
            pass

    # Probe 2: Thread exhaustion via concurrent requests
    print("  - Thread pool exhaustion...")

    def blocking_request():
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            # Send partial message to tie up a thread
            sock.sendall(b'{"type":"command","id":"block",')
            time.sleep(30)  # Hold for 30 seconds
            sock.close()
        except:
            pass

    threads = [threading.Thread(target=blocking_request) for _ in range(100)]
    for t in threads:
        t.start()

    time.sleep(2)

    # Check if server is still responsive
    try:
        with SerialStudioClient() as client:
            client.command("api.getCommands")
        prober.log_probe("Thread Exhaustion", False, "Server still responsive")
    except:
        prober.log_probe("Thread Exhaustion", True, "Server starved of threads!")

    for t in threads:
        t.join(timeout=35)

    # Probe 3: Queue overflow with message flooding
    print("  - Queue overflow probe...")

    def flood_queue():
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            for i in range(10000):
                msg = {
                    "type": "command",
                    "id": f"flood-{i}",
                    "command": "api.getCommands",
                }
                sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.close()
        except:
            pass

    # Multiple threads flooding simultaneously
    threads = [threading.Thread(target=flood_queue) for _ in range(10)]
    for t in threads:
        t.start()
    for t in threads:
        t.join(timeout=10)

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_deserialization_probes(prober):
    """JSON deserialization probing"""
    print("\n[*] Testing deserialization probes...")

    # Probe 1: Circular references
    print("  - Testing circular reference handling...")
    try:
        # Can't create true circular ref in JSON, but can create self-similar structures
        circular = {"type": "command", "id": "test", "command": "api.getCommands"}
        circular["params"] = {"self": circular}

        # Will fail to serialize, but try anyway
        with SerialStudioClient() as client:
            # Create deeply recursive structure
            recursive = {"a": {}}
            current = recursive["a"]
            for i in range(1000):
                current["b"] = {}
                current = current["b"]

            msg = {
                "type": "command",
                "id": "recursive",
                "command": "project.loadJson",
                "params": {"config": recursive},
            }

            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(5.0)
            response = sock.recv(4096)
            sock.close()

    except Exception as e:
        print(f"    Recursive structure: {e}")

    # Probe 2: Type confusion
    print("  - Testing type confusion...")
    type_confusion_payloads = [
        {"type": ["array", "instead", "of", "string"]},
        {"type": {"nested": "object"}},
        {"type": 12345},
        {"type": True},
        {"type": None},
        {"id": ["array", "id"]},
        {"command": {"nested": "command"}},
    ]

    for payload in type_confusion_payloads:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            sock.sendall(json.dumps(payload).encode() + b"\n")
            sock.settimeout(1.0)
            response = sock.recv(4096)
            sock.close()
        except:
            pass

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_logic_edge_cases(prober):
    """Probe state machine logic errors"""
    print("\n[*] Testing logic edge case probes...")

    # Probe 1: Invalid state transitions
    print("  - Forcing invalid state transitions...")

    try:
        with SerialStudioClient() as client:
            # Try to export before connecting
            client.command("csvExport.setEnabled", {"enabled": True})

            # Try to disconnect before connecting
            client.command("io.disconnect")

            # Try to write data without device
            import base64

            data = base64.b64encode(b"INJECT").decode()
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": data}
            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(2.0)
            response = sock.recv(4096)
            sock.close()

    except Exception as e:
        print(f"    State validation: {e}")

    # Probe 2: Rapid mode switching
    print("  - Rapid operation mode switching...")

    try:
        with SerialStudioClient() as client:
            for i in range(500):
                mode = i % 3
                client.configure_frame_parser(
                    start_sequence="/*",
                    end_sequence="*/",
                    operation_mode=mode,
                )
    except Exception as e:
        print(f"    Mode switching: {e}")

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_compression_amplification(prober):
    """Test compression amplification / decompression amplification equivalent"""
    print("\n[*] Testing compression-like probes...")

    # Probe: Send highly repetitive data that might trigger compression
    print("  - Sending highly repetitive payload...")

    try:
        # Create a message with extreme redundancy
        redundant = {
            "type": "command",
            "id": "A" * 1000000,
            "command": "api.getCommands",
        }

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((prober.host, prober.port))
        payload = json.dumps(redundant).encode() + b"\n"
        print(f"    Payload size: {len(payload) / 1024 / 1024:.2f} MB")
        sock.sendall(payload)
        sock.settimeout(5.0)
        response = sock.recv(4096)
        sock.close()

        if response:
            prober.log_probe(
                "Compression amplification", False, "Redundant data handled"
            )
    except Exception as e:
        print(f"    Exception: {e}")

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


@pytest.mark.timeout(90)
def test_batch_probes(prober):
    """Advanced batch command probing"""
    print("\n[*] Testing advanced batch probes...")

    # Probe 1: Exactly at batch limit (256)
    print("  - Testing batch size limit bypass...")
    try:
        with SerialStudioClient(timeout=10.0) as client:
            # Try exactly 256 (at limit)
            batch = [{"command": "api.getCommands"} for _ in range(256)]
            result = client.batch(batch, timeout=30.0)
            print("    256 commands: ACCEPTED")

    except (APIError, TimeoutError, ConnectionError) as e:
        print(f"    256 commands: REJECTED ({e})")

    # Try 257 in a new connection (over limit)
    try:
        with SerialStudioClient(timeout=10.0) as client:
            batch = [{"command": "api.getCommands"} for _ in range(257)]
            result = client.batch(batch, timeout=30.0)

            # Check if server returned error response
            if isinstance(result, dict) and result.get("error"):
                prober.log_probe(
                    "Batch Limit Bypass", False, "Limit enforced via error response"
                )
            else:
                prober.log_probe("Batch Limit Bypass", True, "257 commands accepted!")

    except (APIError, ConnectionError) as e:
        # Server rejected and closed connection - CORRECT behavior
        prober.log_probe("Batch Limit Bypass", False, f"Limit enforced: {e}")
    except TimeoutError:
        prober.log_probe("Batch Limit", True, "Server hung on batch processing!")

    # Probe 2: Batch within batch (if possible)
    print("  - Testing nested batch commands...")
    try:
        # This might not work, but worth trying
        nested_batch = {
            "type": "batch",
            "id": str(uuid.uuid4()),
            "commands": [
                {
                    "type": "batch",
                    "id": str(uuid.uuid4()),
                    "commands": [{"command": "api.getCommands"} for _ in range(100)],
                }
                for _ in range(10)
            ],
        }

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((prober.host, prober.port))
        sock.sendall(json.dumps(nested_batch).encode() + b"\n")
        sock.settimeout(10.0)
        response = sock.recv(65536)
        sock.close()

    except Exception as e:
        print(f"    Nested batch: {e}")

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def test_binary_injection(prober):
    """Inject binary data through various channels"""
    print("\n[*] Testing binary data injection...")

    # Probe 1: Binary in JSON strings
    print("  - Injecting binary in JSON strings...")
    binary_payloads = [
        b"\x00\x01\x02\x03\x04\x05",  # NULLs and control chars
        b"\xff\xfe\xfd\xfc",  # High bytes
        b"\x1b[31mRED\x1b[0m",  # ANSI escape codes
        b"\x7f" * 100,  # DEL characters
    ]

    for binary in binary_payloads:
        try:
            # Encode as base64 for raw message
            encoded = base64.b64encode(binary).decode()
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((prober.host, prober.port))
            msg = {"type": "raw", "id": str(uuid.uuid4()), "data": encoded}
            sock.sendall(json.dumps(msg).encode() + b"\n")
            sock.settimeout(1.0)
            response = sock.recv(4096)
            sock.close()
        except:
            pass

    successful = [e for e in prober.probes if e["success"]]
    assert not successful, f"Probe succeeded: {successful}"


def main():
    """Run advanced probing suite"""
    print("=" * 80)
    print("Serial Studio Advanced Probing Suite")
    print("=" * 80)
    print("\nRED TEAM MODE: Attempting sophisticated probing techniques")
    print("This will try to bypass security controls and crash the server.\n")

    prober = AdvancedProber()

    try:
        # Check connectivity
        with SerialStudioClient() as client:
            print("[+] Target acquired: Serial Studio API\n")

        # Run advanced probes
        test_race_conditions(prober)
        test_integer_overflow(prober)
        test_memory_corruption(prober)
        test_parser_confusion(prober)
        test_timing_probes(prober)
        test_resource_exhaustion(prober)
        test_deserialization_probes(prober)
        test_logic_edge_cases(prober)
        test_compression_amplification(prober)
        test_batch_probes(prober)
        test_binary_injection(prober)

    except KeyboardInterrupt:
        print("\n\n[!] Probing interrupted")

    finally:
        # Report
        print("\n" + "=" * 80)
        print("PROBING REPORT")
        print("=" * 80)

        successful = [e for e in prober.probes if e["success"]]
        blocked = [e for e in prober.probes if not e["success"]]

        print(f"\n🎯 Successful probes: {len(successful)}")
        for probe in successful:
            print(f"  [FLAGGED] {probe['name']}: {probe['details']}")

        print(f"\n🛡️  Blocked probes: {len(blocked)}")
        for probe in blocked[:10]:  # Show first 10
            print(f"  [DEFENDED] {probe['name']}: {probe['details']}")

        # Final check
        print("\n[*] Final server health check...")
        try:
            with SerialStudioClient() as client:
                client.command("api.getCommands")
                print("  ✅ Server still alive and responsive")
        except:
            print("  ❌ SERVER DOWN - PROBING SUCCESSFUL!")

        print("\n" + "=" * 80)

    return 0


if __name__ == "__main__":
    exit(main())
