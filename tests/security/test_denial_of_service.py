#!/usr/bin/env python3
"""
DoS and Resource Exhaustion Test Suite for Serial Studio API

This test suite attempts various Denial of Service attacks to verify
resource limits and system stability under malicious workloads.

Attack categories:
- CPU exhaustion
- Memory exhaustion
- Connection exhaustion
- Bandwidth exhaustion
- Queue overflow
- Thread starvation

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import socket
import threading
import time
import uuid
import psutil
import sys
from pathlib import Path
from typing import List

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import SerialStudioClient, APIError


class DoSTester:
    """DoS attack testing framework"""

    def __init__(self, host="127.0.0.1", port=7777):
        self.host = host
        self.port = port
        self.successful_attacks = []
        self.failed_attacks = []

    def log_success(self, attack_name, details):
        """Log successful DoS attack"""
        self.successful_attacks.append({"attack": attack_name, "details": details})
        print(f"[VULN] {attack_name}: {details}")

    def log_failure(self, attack_name, reason):
        """Log failed DoS attack (server defended)"""
        self.failed_attacks.append({"attack": attack_name, "reason": reason})
        print(f"[DEFENDED] {attack_name}: {reason}")

    def get_process_stats(self):
        """Get current process resource usage"""
        try:
            for proc in psutil.process_iter(["pid", "name", "memory_info", "cpu_percent"]):
                if "Serial-Studio" in proc.info["name"]:
                    return {
                        "pid": proc.info["pid"],
                        "memory_mb": proc.info["memory_info"].rss / 1024 / 1024,
                        "cpu_percent": proc.info["cpu_percent"],
                    }
        except:
            pass
        return None

    def check_server_responsive(self, timeout=2.0):
        """Check if server is still responsive"""
        try:
            with SerialStudioClient(timeout=timeout) as client:
                client.command("api.getCommands")
                return True
        except:
            return False


def test_memory_exhaustion(tester):
    """Test memory exhaustion attacks"""
    print("\n[*] Testing memory exhaustion attacks...")

    initial_stats = tester.get_process_stats()
    if initial_stats:
        print(f"  Initial memory: {initial_stats['memory_mb']:.2f} MB")

    # Attack 1: Huge JSON payloads
    print("  - Sending huge JSON payloads (100MB each)...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        for i in range(10):
            huge_payload = {
                "type": "command",
                "id": str(uuid.uuid4()),
                "command": "api.getCommands",
                "params": {"data" * 1000: "x" * (10 * 1024 * 1024)},  # 10MB param
            }
            try:
                sock.sendall(json.dumps(huge_payload).encode() + b"\n")
                time.sleep(0.1)
            except:
                break

        sock.close()

        final_stats = tester.get_process_stats()
        if final_stats:
            memory_increase = final_stats["memory_mb"] - initial_stats["memory_mb"]
            print(f"    Memory increased by {memory_increase:.2f} MB")

            if memory_increase > 500:  # More than 500MB increase
                tester.log_success(
                    "Memory Exhaustion", f"Memory grew {memory_increase:.2f} MB"
                )
            else:
                tester.log_failure("Memory Exhaustion", "Server limited memory growth")

    except Exception as e:
        print(f"    Exception: {e}")

    # Attack 2: Memory leak via repeated connections
    print("  - Testing memory leak via connection churn...")
    initial_stats = tester.get_process_stats()

    for i in range(1000):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))
            sock.sendall(
                b'{"type":"command","id":"x","command":"api.getCommands"}\n'
            )
            sock.close()
        except:
            pass

    time.sleep(2)
    final_stats = tester.get_process_stats()

    if initial_stats and final_stats:
        memory_increase = final_stats["memory_mb"] - initial_stats["memory_mb"]
        print(f"    Memory increased by {memory_increase:.2f} MB after 1000 connections")

        if memory_increase > 100:
            tester.log_success(
                "Memory Leak", f"Potential leak: {memory_increase:.2f} MB increase"
            )


def test_cpu_exhaustion(tester):
    """Test CPU exhaustion attacks"""
    print("\n[*] Testing CPU exhaustion attacks...")

    initial_stats = tester.get_process_stats()
    if initial_stats:
        print(f"  Initial CPU: {initial_stats['cpu_percent']:.1f}%")

    # Attack 1: Computationally expensive operations
    print("  - Flooding with rapid commands...")

    def flood_thread():
        try:
            with SerialStudioClient() as client:
                for i in range(1000):
                    try:
                        client.command("api.getCommands")
                    except:
                        pass
        except:
            pass

    threads = [threading.Thread(target=flood_thread) for _ in range(10)]
    start_time = time.time()

    for t in threads:
        t.start()

    time.sleep(2)
    final_stats = tester.get_process_stats()

    for t in threads:
        t.join(timeout=5)

    elapsed = time.time() - start_time

    if final_stats:
        print(f"    Peak CPU: {final_stats['cpu_percent']:.1f}%")
        print(f"    Duration: {elapsed:.2f}s")

        if final_stats["cpu_percent"] > 90:
            tester.log_success("CPU Exhaustion", f"CPU reached {final_stats['cpu_percent']:.1f}%")
        else:
            print(f"    Server maintained {final_stats['cpu_percent']:.1f}% CPU under load")

    # Attack 2: Complex JSON parsing
    print("  - Testing complex JSON parsing...")
    try:
        with SerialStudioClient() as client:
            complex_payload = {
                "type": "batch",
                "id": str(uuid.uuid4()),
                "commands": [
                    {
                        "id": f"cmd-{i}",
                        "command": "project.loadFromJSON",
                        "params": {
                            "config": {
                                "title": "Test",
                                "groups": [
                                    {
                                        "title": f"Group-{j}",
                                        "datasets": [
                                            {"title": f"DS-{k}", "value": "0"}
                                            for k in range(50)
                                        ],
                                    }
                                    for j in range(20)
                                ],
                            }
                        },
                    }
                    for i in range(100)
                ],
            }

            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))
            sock.sendall(json.dumps(complex_payload).encode() + b"\n")
            sock.settimeout(5.0)
            response = sock.recv(4096)
            sock.close()

    except Exception as e:
        print(f"    Exception: {e}")


def test_connection_flood(tester):
    """Test connection flooding"""
    print("\n[*] Testing connection flood attack...")

    # Attack 1: Rapid connection establishment
    print("  - Opening 500 connections in rapid succession...")
    connections = []
    successful = 0

    start_time = time.time()

    for i in range(500):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1.0)
            sock.connect((tester.host, tester.port))
            connections.append(sock)
            successful += 1
        except Exception as e:
            break

    elapsed = time.time() - start_time
    print(f"    Opened {successful} connections in {elapsed:.2f}s")

    if successful > 100:
        tester.log_success(
            "Connection Flood", f"No limit: {successful} simultaneous connections"
        )
    else:
        tester.log_failure("Connection Flood", f"Limited to {successful} connections")

    # Close all connections
    for sock in connections:
        try:
            sock.close()
        except:
            pass

    # Check server responsiveness
    time.sleep(1)
    if tester.check_server_responsive():
        print("    Server still responsive after flood (GOOD)")
    else:
        tester.log_success("Connection Flood", "Server unresponsive after flood")


def test_slowloris_attack(tester):
    """Test Slowloris-style slow connection attack"""
    print("\n[*] Testing Slowloris attack...")

    print("  - Opening slow connections...")
    slow_sockets = []

    # Open multiple slow connections
    for i in range(50):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((tester.host, tester.port))
            slow_sockets.append(sock)
        except:
            break

    print(f"    Opened {len(slow_sockets)} slow connections")

    # Send data very slowly on each connection
    start_time = time.time()
    payload = b'{"type":"command","id":"slow","command":"api.getCommands"}\n'

    for i in range(min(len(payload), 30)):  # Send ~30 bytes over time
        for sock in slow_sockets:
            try:
                sock.send(bytes([payload[i]]))
            except:
                slow_sockets.remove(sock)
        time.sleep(0.5)  # 2 bytes per second

    # Check if server times out slow connections
    elapsed = time.time() - start_time
    still_connected = sum(1 for s in slow_sockets if s.fileno() != -1)

    print(f"    After {elapsed:.1f}s: {still_connected}/{len(slow_sockets)} still connected")

    if still_connected > len(slow_sockets) * 0.5:
        tester.log_success(
            "Slowloris", f"No timeout: {still_connected} slow connections survived"
        )
    else:
        tester.log_failure("Slowloris", f"Server timed out slow connections")

    # Clean up
    for sock in slow_sockets:
        try:
            sock.close()
        except:
            pass


def test_bandwidth_exhaustion(tester):
    """Test bandwidth exhaustion"""
    print("\n[*] Testing bandwidth exhaustion...")

    # Attack: Send continuous stream of data
    print("  - Sending continuous data stream (10MB/sec)...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        chunk_size = 1024 * 1024  # 1MB chunks
        total_sent = 0
        start_time = time.time()

        # Send for 10 seconds
        while time.time() - start_time < 10:
            try:
                chunk = b"X" * chunk_size
                sock.sendall(chunk)
                total_sent += chunk_size
            except:
                break

        elapsed = time.time() - start_time
        bandwidth_mbps = (total_sent / elapsed) / (1024 * 1024)

        print(f"    Sent {total_sent / (1024*1024):.2f} MB in {elapsed:.2f}s ({bandwidth_mbps:.2f} MB/s)")

        sock.close()

        # Check if server is still responsive
        if tester.check_server_responsive(timeout=5.0):
            print("    Server responsive after bandwidth flood")
        else:
            tester.log_success("Bandwidth Exhaustion", "Server unresponsive after flood")

    except Exception as e:
        print(f"    Exception: {e}")


def test_queue_overflow(tester):
    """Test queue overflow attacks"""
    print("\n[*] Testing queue overflow...")

    # The API server uses FrameConsumer with queue capacity 2048
    # Try to overflow this queue

    print("  - Flooding with 10,000 rapid messages...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((tester.host, tester.port))

        for i in range(10000):
            msg = {
                "type": "command",
                "id": f"flood-{i}",
                "command": "api.getCommands",
            }
            try:
                sock.sendall(json.dumps(msg).encode() + b"\n")
            except:
                print(f"    Connection broke after {i} messages")
                break

        sock.close()

        # Check server responsiveness
        time.sleep(2)
        if tester.check_server_responsive():
            print("    Server still responsive")
        else:
            tester.log_success("Queue Overflow", "Server unresponsive after message flood")

    except Exception as e:
        print(f"    Exception: {e}")


def test_fork_bomb_simulation(tester):
    """Simulate fork bomb via rapid batch commands"""
    print("\n[*] Testing fork bomb simulation...")

    print("  - Sending nested batch commands...")

    # Try to cause exponential resource growth
    try:
        with SerialStudioClient() as client:
            for round_num in range(5):
                batch_size = 2 ** (round_num + 5)  # 32, 64, 128, 256, 512
                print(f"    Round {round_num+1}: {batch_size} commands...")

                batch = [{"command": "api.getCommands"} for _ in range(batch_size)]

                try:
                    start = time.time()
                    results = client.batch(batch, timeout=30.0)
                    elapsed = time.time() - start
                    print(f"      Completed in {elapsed:.2f}s")

                    if elapsed > 10.0:
                        tester.log_success(
                            "Batch DoS", f"Batch of {batch_size} took {elapsed:.2f}s"
                        )
                except (APIError, TimeoutError) as e:
                    print(f"      Server rejected: {e}")
                    break

    except Exception as e:
        print(f"    Exception: {e}")


def test_amplification_attack(tester):
    """Test amplification attacks (small request, large response)"""
    print("\n[*] Testing amplification attack...")

    # Try to get disproportionately large responses
    print("  - Requesting commands list repeatedly...")

    try:
        with SerialStudioClient() as client:
            # Small request
            request_size = len(b'{"type":"command","id":"x","command":"api.getCommands"}\n')

            responses = []
            for i in range(100):
                result = client.command("api.getCommands")
                response_size = len(json.dumps(result).encode())
                responses.append(response_size)

            avg_response = sum(responses) / len(responses)
            amplification_factor = avg_response / request_size

            print(f"    Request: {request_size} bytes")
            print(f"    Avg response: {avg_response:.0f} bytes")
            print(f"    Amplification: {amplification_factor:.1f}x")

            if amplification_factor > 100:
                tester.log_success(
                    "Amplification", f"{amplification_factor:.1f}x amplification factor"
                )

    except Exception as e:
        print(f"    Exception: {e}")


def test_recursive_operations(tester):
    """Test recursive or circular operations"""
    print("\n[*] Testing recursive operations...")

    # Try to create recursive project structures
    print("  - Testing recursive project loading...")

    try:
        with SerialStudioClient() as client:
            # Create a project that references itself
            recursive_config = {
                "title": "Recursive",
                "groups": [],
            }

            # Add circular reference if possible
            for i in range(100):
                try:
                    result = client.command(
                        "project.loadFromJSON", {"config": recursive_config}
                    )
                except APIError:
                    break

    except Exception as e:
        print(f"    Exception: {e}")


def main():
    """Run DoS attack tests"""
    print("=" * 80)
    print("Serial Studio DoS Attack Test Suite")
    print("=" * 80)
    print("\nWARNING: These tests will stress the Serial Studio API.")
    print("System resource usage will spike during testing.")
    print()

    tester = DoSTester()

    try:
        # Check server connectivity
        if not tester.check_server_responsive():
            print("[ERROR] Cannot connect to Serial Studio API")
            print("Make sure Serial Studio is running with API Server enabled.")
            return 1

        print("[+] Connected to Serial Studio API\n")

        # Run DoS attack categories
        test_memory_exhaustion(tester)
        test_cpu_exhaustion(tester)
        test_connection_flood(tester)
        test_slowloris_attack(tester)
        test_bandwidth_exhaustion(tester)
        test_queue_overflow(tester)
        test_fork_bomb_simulation(tester)
        test_amplification_attack(tester)
        test_recursive_operations(tester)

    except KeyboardInterrupt:
        print("\n\n[!] Tests interrupted by user")

    finally:
        # Generate report
        print("\n" + "=" * 80)
        print("DOS ATTACK TEST REPORT")
        print("=" * 80)

        print(f"\nSuccessful attacks: {len(tester.successful_attacks)}")
        for attack in tester.successful_attacks:
            print(f"  - {attack['attack']}: {attack['details']}")

        print(f"\nDefended attacks: {len(tester.failed_attacks)}")
        for attack in tester.failed_attacks:
            print(f"  - {attack['attack']}: {attack['reason']}")

        # Final server check
        print("\nFinal server status check...")
        if tester.check_server_responsive():
            print("  [OK] Server still responsive after all tests")
        else:
            print("  [CRITICAL] Server is unresponsive!")

        print("\n" + "=" * 80)

    return 0


if __name__ == "__main__":
    exit(main())
