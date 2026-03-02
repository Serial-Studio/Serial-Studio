#!/usr/bin/env python3
"""
System Monitor — Serial Studio Process I/O Example

Streams real-time CPU, RAM, disk, network, and temperature metrics to stdout
in Serial Studio's key=value frame format. Serial Studio launches this script
directly via the Process driver (Pro) and parses its output as live telemetry.

Two output phases:
  1. Header  — one-shot static system information (OS, CPU model, RAM total,
               disk capacity). Each key=value line is a separate frame so the
               datagrid widget populates immediately on connect.
  2. Live    — continuous 1 Hz frames with real-time usage metrics.

Frame format (ProjectFile mode, frameStart='$', frameEnd='\n'):
    $KEY1=value1|KEY2=value2|KEY3=value3\n

The JavaScript parser in the .ssproj file splits on '|' and then on '=' to
build a lookup table, returning values in the fixed index order the dashboard
widgets expect.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys
import time
import platform
import socket

try:
    import psutil
except ImportError:
    print("ERROR: psutil is not installed.", file=sys.stderr)
    print("Run:  pip install psutil", file=sys.stderr)
    sys.exit(1)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _cpu_model() -> str:
    """Return a human-readable CPU model string."""
    machine = platform.machine()
    processor = platform.processor()
    system = platform.system()

    if processor and processor not in ("", machine):
        return processor[:48]

    if system == "Darwin":
        try:
            import subprocess
            result = subprocess.run(
                ["sysctl", "-n", "machdep.cpu.brand_string"],
                capture_output=True, text=True, timeout=2
            )
            brand = result.stdout.strip()
            if brand:
                return brand[:48]
        except Exception:
            pass

    if system == "Linux":
        try:
            with open("/proc/cpuinfo") as f:
                for line in f:
                    if line.startswith("model name"):
                        return line.split(":", 1)[1].strip()[:48]
        except Exception:
            pass

    return machine or "Unknown"


def _disk_root() -> "tuple[str, object] | tuple[None, None]":
    """Return (path, disk_usage) for the root / primary partition."""
    for path in ("/", "C:\\"):
        try:
            return path, psutil.disk_usage(path)
        except Exception:
            continue
    return None, None


def _cpu_temp() -> float:
    """Return CPU temperature in °C, or -1 if unavailable."""
    try:
        temps = psutil.sensors_temperatures()
        if not temps:
            return -1.0

        for key in ("coretemp", "cpu_thermal", "k10temp", "zenpower",
                    "cpu-thermal", "acpitz"):
            if key in temps and temps[key]:
                return round(temps[key][0].current, 1)

        for entries in temps.values():
            if entries:
                return round(entries[0].current, 1)
    except Exception:
        pass

    return -1.0


def _net_bytes() -> tuple[int, int]:
    """Return (bytes_sent, bytes_recv) cumulative counters."""
    c = psutil.net_io_counters()
    return c.bytes_sent, c.bytes_recv


def _emit(frame: str) -> None:
    """Write a single framed line to stdout and flush immediately."""
    print(f"${frame}", flush=True)


# ---------------------------------------------------------------------------
# Header phase — static system information
# ---------------------------------------------------------------------------

def emit_header() -> None:
    """
    Print one frame of static system metadata.

    Serial Studio receives this as a regular frame; the datagrid group
    displays these values and they do not change during the session.
    """
    _, disk = _disk_root()
    disk_total_gb = round(disk.total / 1e9, 1) if disk else 0.0

    ram_total_gb  = round(psutil.virtual_memory().total / 1e9, 1)
    cpu_cores     = psutil.cpu_count(logical=False) or 1
    cpu_threads   = psutil.cpu_count(logical=True)  or 1

    fields = [
        f"OS={platform.system()} {platform.release()}",
        f"HOSTNAME={socket.gethostname()[:24]}",
        f"CPU_MODEL={_cpu_model()}",
        f"CPU_CORES={cpu_cores}",
        f"CPU_THREADS={cpu_threads}",
        f"RAM_TOTAL={ram_total_gb} GB",
        f"DISK_TOTAL={disk_total_gb} GB",
    ]

    _emit("|".join(fields))


# ---------------------------------------------------------------------------
# Live phase — real-time metrics
# ---------------------------------------------------------------------------

def run_live(interval: float = 1.0) -> None:
    """
    Loop forever, emitting one frame per interval with live metrics.

    Args:
        interval: Seconds between frames (default 1.0).
    """
    disk_path, _ = _disk_root()

    # Prime the network counter so the first delta is meaningful.
    prev_sent, prev_recv = _net_bytes()
    time.sleep(min(interval, 1.0))

    while True:
        t0 = time.monotonic()

        cpu_pct   = psutil.cpu_percent(interval=None)
        ram       = psutil.virtual_memory()
        ram_pct   = ram.percent
        ram_used  = round(ram.used  / 1e9, 2)

        disk_pct  = 0.0
        disk_used = 0.0
        if disk_path:
            try:
                du = psutil.disk_usage(disk_path)
                disk_pct  = du.percent
                disk_used = round(du.used / 1e9, 1)
            except Exception:
                pass

        cur_sent, cur_recv = _net_bytes()
        net_sent_kb = round((cur_sent - prev_sent) / 1024, 1)
        net_recv_kb = round((cur_recv - prev_recv) / 1024, 1)
        prev_sent, prev_recv = cur_sent, cur_recv

        cpu_temp     = _cpu_temp()
        process_count = len(psutil.pids())

        fields = [
            f"CPU_USAGE={cpu_pct}",
            f"RAM_USAGE={ram_pct}",
            f"RAM_USED={ram_used}",
            f"DISK_USAGE={disk_pct}",
            f"DISK_USED={disk_used}",
            f"NET_SENT={net_sent_kb}",
            f"NET_RECV={net_recv_kb}",
            f"CPU_TEMP={cpu_temp}",
            f"PROCESSES={process_count}",
        ]

        _emit("|".join(fields))

        elapsed = time.monotonic() - t0
        time.sleep(max(0.0, interval - elapsed))


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="System Monitor — Serial Studio Process I/O example"
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=1.0,
        metavar="SECONDS",
        help="Seconds between live frames (default: 1.0)",
    )
    args = parser.parse_args()

    try:
        emit_header()
        run_live(interval=args.interval)
    except KeyboardInterrupt:
        pass
