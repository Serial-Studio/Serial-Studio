#!/usr/bin/env python3
#
# gping.py
#
# Pings a host repeatedly with the operating system's own `ping` command and
# streams the round-trip time over UDP for real-time plotting in Serial Studio,
# in the spirit of the `gping` terminal tool.
#
# --- How It Works ---
# One `ping` process is launched per sample (one echo request each), so the
# script needs no raw-socket privileges and no third-party packages. The reply
# line is parsed for its `time=12.3 ms` field; the flags differ per platform:
#
#   Windows : ping -n 1 -w <milliseconds> host
#   macOS   : ping -c 1 -W <milliseconds> host
#   Linux   : ping -c 1 -W <seconds> host
#
# --- What It Streams (UDP, CSV Format) ---
#   host      : Target as typed (string)
#   ip        : Resolved IPv4 address (string)
#   last      : Latest round-trip time in ms (0 when the request timed out)
#   min       : Smallest round-trip time in the statistics window
#   max       : Largest round-trip time in the statistics window
#   avg       : Mean round-trip time in the statistics window
#   jitter    : Mean absolute difference between consecutive replies
#   p95       : 95th percentile round-trip time in the statistics window
#   timeouts  : Requests without a reply since the script started
#   lost      : 1 when the latest request timed out, 0 otherwise
#
# Example frame:
#   $google.com,142.250.78.14,5.143,4.191,95.912,13.291,15.473,73.657,0,0
#
# --- Use In Serial Studio ---
# - Load graphical-ping.ssproj and click Connect; the bundled control loop
#   launches this script. To run it by hand:
#
#     python3 gping.py serial-studio.com --interval 0.2
#

import argparse
import platform
import re
import socket
import subprocess
import sys
import time
from collections import deque

RTT_PATTERN = re.compile(r"[=<]\s*(\d+(?:[.,]\d+)?)\s*ms", re.IGNORECASE)


def build_command(host, timeout_s):
    """Returns the single-echo ping command for the current platform."""
    system = platform.system()
    if system == "Windows":
        return ["ping", "-n", "1", "-w", str(int(timeout_s * 1000)), host]

    if system == "Darwin":
        return ["ping", "-c", "1", "-W", str(int(timeout_s * 1000)), host]

    return ["ping", "-c", "1", "-W", str(max(1, int(round(timeout_s)))), host]


def ping_once(command, timeout_s):
    """Returns the round-trip time in ms, or None when no reply arrived."""
    flags = 0
    if platform.system() == "Windows":
        flags = subprocess.CREATE_NO_WINDOW

    try:
        result = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            timeout=timeout_s + 2.0,
            creationflags=flags,
        )
    except (subprocess.TimeoutExpired, OSError):
        return None

    if result.returncode != 0:
        return None

    match = RTT_PATTERN.search(result.stdout.decode(errors="replace"))
    if match is None:
        return None

    return float(match.group(1).replace(",", "."))


def resolve(host):
    """Returns the IPv4 address of the host, or a dash when DNS fails."""
    try:
        return socket.gethostbyname(host)
    except OSError:
        return "-"


def percentile(samples, fraction):
    """Returns the nearest-rank percentile of a non-empty sample list."""
    ordered = sorted(samples)
    rank = int(round(fraction * (len(ordered) - 1)))
    return ordered[rank]


def statistics(window):
    """Returns (min, max, avg, jitter, p95) over the (time, rtt) window."""
    samples = [rtt for _, rtt in window]
    if not samples:
        return 0.0, 0.0, 0.0, 0.0, 0.0

    deltas = [abs(b - a) for a, b in zip(samples, samples[1:])]
    jitter = sum(deltas) / len(deltas) if deltas else 0.0
    average = sum(samples) / len(samples)
    return min(samples), max(samples), average, jitter, percentile(samples, 0.95)


def main():
    parser = argparse.ArgumentParser(description="Graphical ping for Serial Studio")
    parser.add_argument("host", nargs="?", default="google.com")
    parser.add_argument(
        "--interval", type=float, default=0.2, help="seconds between pings"
    )
    parser.add_argument(
        "--timeout", type=float, default=1.0, help="seconds to wait for a reply"
    )
    parser.add_argument(
        "--window", type=float, default=60.0, help="statistics window in seconds"
    )
    parser.add_argument("--udp-host", default="127.0.0.1")
    parser.add_argument("--udp-port", type=int, default=9000)
    args = parser.parse_args()

    ip = resolve(args.host)
    command = build_command(args.host, args.timeout)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print("Pinging {} ({}) every {} s".format(args.host, ip, args.interval))

    timeouts = 0
    window = deque()
    while True:
        started = time.monotonic()
        rtt = ping_once(command, args.timeout)
        if rtt is None:
            timeouts += 1
            print("Request timed out ({} so far)".format(timeouts))
        else:
            window.append((started, rtt))

        while window and started - window[0][0] > args.window:
            window.popleft()

        if ip == "-":
            ip = resolve(args.host)

        low, high, average, jitter, p95 = statistics(window)
        frame = "${},{},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{:.3f},{},{}\n".format(
            args.host,
            ip,
            rtt if rtt is not None else 0.0,
            low,
            high,
            average,
            jitter,
            p95,
            timeouts,
            1 if rtt is None else 0,
        )
        sock.sendto(frame.encode(), (args.udp_host, args.udp_port))

        remaining = args.interval - (time.monotonic() - started)
        if remaining > 0:
            time.sleep(remaining)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(0)
