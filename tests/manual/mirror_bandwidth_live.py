"""
Spec-0040 AC6 harness: mirror bandwidth vs device rate, measured live.

MAINTAINER-RUN. Needs a running Serial Studio with the API server enabled and
the spec-0040 mirror implemented (T7-T13), so it cannot run before the 0039 M2
gate. The fixture-based math in `tests/fixtures/mirror/measure_bandwidth.py`
predicts the answer; this proves it on a real pair.

AC6 asks for bytes/second on the mirror channel at a low device rate and at a
rate at least two orders of magnitude higher, on the same project, and that the
two land within the same order of magnitude. This drives both legs and prints
the comparison plus a verdict.

    # terminal 1: the capture
    serial-studio --headless --api-server --api-external project.ssproj

    # terminal 2: the measurement
    python3 tests/manual/mirror_bandwidth_live.py \
        --host 192.168.1.20 --token <hex> --low-hz 10 --high-hz 2000

The device is driven through the ordinary integration-test loopback
(`utils.device_simulator`), so the capture side sees a real driver and a real
parse pipeline, not an injected frame.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

from __future__ import annotations

import argparse
import socket
import sys
import threading
import time
from pathlib import Path

TESTS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TESTS_ROOT))
sys.path.insert(0, str(TESTS_ROOT / "utils"))

from mirror_client import MirrorClient  # noqa: E402
from utils.api_client import SerialStudioClient  # noqa: E402
from utils.data_generator import ChecksumType, DataGenerator  # noqa: E402
from utils.device_simulator import DeviceSimulator  # noqa: E402


def local_address(peer_host: str) -> str:
    """The address the capture side should dial back on, seen from this machine."""
    probe = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        probe.connect((peer_host, 7777))
        return probe.getsockname()[0]
    finally:
        probe.close()


def drive(
    host: str,
    token: str,
    mirror_hz: int,
    device_hz: int,
    seconds: float,
    simulator: DeviceSimulator,
    fields: int,
) -> dict:
    """
    Runs one leg: stream continuously at `device_hz` on a producer thread while
    the viewer measures for `seconds`. The producer must run concurrently --
    `DeviceSimulator.send_frames` is paced and blocking, so sending first and
    measuring afterwards would measure an idle link.
    """
    viewer = MirrorClient(host=host, token=token, hz=mirror_hz)
    viewer.attach()

    batch = [
        DataGenerator.wrap_frame(
            ",".join(f"{(i + j) % 100}.{i % 1000:03d}" for j in range(fields)),
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(max(1, min(device_hz, 2000)))
    ]

    stop = threading.Event()

    def produce() -> None:
        while not stop.is_set():
            simulator.send_frames(batch, interval_seconds=1.0 / device_hz)

    producer = threading.Thread(target=produce, daemon=True)
    producer.start()
    time.sleep(1.0)

    before_bytes = viewer.stats.bytes_in
    before_snapshots = viewer.stats.snapshots
    started = time.monotonic()

    while time.monotonic() - started < seconds:
        viewer.pump(timeout=0.25)

    elapsed = time.monotonic() - started
    snapshots = viewer.stats.snapshots - before_snapshots
    measured_bytes = viewer.stats.bytes_in - before_bytes

    stop.set()
    producer.join(timeout=5.0)
    viewer.close()

    return {
        "deviceHz": device_hz,
        "mirrorHz": mirror_hz,
        "seconds": elapsed,
        "bytes": measured_bytes,
        "bytesPerSecond": measured_bytes / elapsed,
        "snapshots": snapshots,
        "snapshotsPerSecond": snapshots / elapsed,
        "meanSnapshotBytes": measured_bytes / max(1, snapshots),
        "datasetCount": (
            0 if viewer.structure is None else viewer.structure.dataset_count
        ),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1", help="capture-side host")
    parser.add_argument("--port", type=int, default=7777)
    parser.add_argument("--token", default="", help="API auth token for a remote hop")
    parser.add_argument("--mirror-hz", type=int, default=20)
    parser.add_argument("--low-hz", type=int, default=10, help="low device rate")
    parser.add_argument("--high-hz", type=int, default=2000, help="high device rate")
    parser.add_argument("--seconds", type=float, default=20.0, help="per-leg duration")
    parser.add_argument("--sim-port", type=int, default=9000)
    parser.add_argument(
        "--sim-host",
        default="",
        help="address the capture side dials back on (default: auto-detected)",
    )
    parser.add_argument("--fields", type=int, default=8)
    args = parser.parse_args()

    if args.high_hz < args.low_hz * 100:
        print(
            "AC6 requires the high rate to be at least two orders of magnitude "
            f"above the low rate; got {args.low_hz} and {args.high_hz}.",
            file=sys.stderr,
        )
        return 2

    control = SerialStudioClient(host=args.host, port=args.port)
    control.connect()
    project = control.get_project_status()
    print(
        f"remote project: {project.get('title', '?')} "
        f"({project.get('datasetCount', '?')} datasets)"
    )

    dial_back = args.sim_host or local_address(args.host)
    simulator = DeviceSimulator(host="0.0.0.0", port=args.sim_port)
    simulator.start()
    print(
        f"simulator listening on 0.0.0.0:{args.sim_port}; "
        f"capture side will dial {dial_back}:{args.sim_port}"
    )

    try:
        control.configure_network(host=dial_back, port=args.sim_port)
        control.connect_device()
        if not simulator.wait_for_connection(timeout=10.0):
            print("capture side never connected to the simulator", file=sys.stderr)
            return 1

        legs = [
            drive(
                args.host,
                args.token,
                args.mirror_hz,
                hz,
                args.seconds,
                simulator,
                args.fields,
            )
            for hz in (args.low_hz, args.high_hz)
        ]
    finally:
        try:
            control.disconnect_device()
        except Exception:
            pass
        simulator.stop()
        control.disconnect()

    print("\n| device Hz | mirror Hz | snapshots/s | mean snapshot | mirror B/s |")
    print("|-----------|-----------|-------------|---------------|------------|")
    for leg in legs:
        print(
            f"| {leg['deviceHz']:9d} | {leg['mirrorHz']:9d} | "
            f"{leg['snapshotsPerSecond']:11.1f} | "
            f"{leg['meanSnapshotBytes']:13.0f} | {leg['bytesPerSecond']:10.0f} |"
        )

    low, high = legs[0]["bytesPerSecond"], legs[1]["bytesPerSecond"]
    ratio = high / max(1.0, low)
    rate_ratio = args.high_hz / max(1, args.low_hz)

    print(f"\ndevice rate x{rate_ratio:.0f}  ->  mirror bytes x{ratio:.2f}")
    verdict = "PASS" if 0.1 <= ratio <= 10.0 else "FAIL"
    print(
        f"AC6 {verdict}: the two rates are "
        f"{'within' if verdict == 'PASS' else 'NOT within'} one order of magnitude."
    )
    print("Record this table in doc/claude/specs/0040-remote-dashboard/bandwidth.md.")

    return 0 if verdict == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
