#!/usr/bin/env python3
"""
Serial Studio Project Tutorial simulator.

Sends the same frames as tutorial.ino over UDP, for readers without a board:

    $temperature,humidity,ax,ay,az;

ten times a second to 127.0.0.1:9000. In Serial Studio, set the device's Connection to
Network, choose UDP, and set the local port to 9000.

Usage:
    python3 tutorial_sim.py [--host 127.0.0.1] [--port 9000] [--rate 10]

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import argparse
import math
import socket
import time


def frame(t: float) -> bytes:
    """Builds one tutorial frame for time t (seconds)."""
    temperature = 24.0 + 3.0 * math.sin(t * 0.2)
    humidity = 55.0 + 10.0 * math.sin(t * 0.1 + 1.0)
    ax = 2.0 * math.sin(t * 1.5)
    ay = 2.0 * math.cos(t * 1.5)
    az = 9.81 + 0.3 * math.sin(t * 3.0)
    text = f"${temperature:.2f},{humidity:.2f},{ax:.3f},{ay:.3f},{az:.3f};\n"
    return text.encode("ascii")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Serial Studio Project Tutorial simulator"
    )
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--rate", type=float, default=10.0, help="frames per second")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    period = 1.0 / max(args.rate, 0.1)
    start = time.monotonic()
    print(f"Sending tutorial frames to {args.host}:{args.port} (Ctrl+C to stop)")
    try:
        while True:
            sock.sendto(frame(time.monotonic() - start), (args.host, args.port))
            time.sleep(period)
    except KeyboardInterrupt:
        pass
    finally:
        sock.close()


if __name__ == "__main__":
    main()
