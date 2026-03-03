#!/usr/bin/env python3
"""
System Monitor — Serial Studio Process I/O Example

Streams real-time CPU, RAM, disk, network, temperature, per-core usage, and
top-process metrics to stdout in Serial Studio's key=value frame format.
Serial Studio launches this script via the Process driver (Pro) and parses
its output as live telemetry.

Output phases:
  1. Header  — one-shot static system information (OS, CPU model, RAM total,
               disk capacity). The JS parser persists these values so they
               remain visible while live frames keep updating the rest.
  2. Live    — frames at ~30 Hz with real-time metrics. Process list and
               disk stats refresh at 1 Hz to avoid overhead.

Frame format (ProjectFile mode, frameStart='$', frameEnd='\n'):
    $KEY1=value1|KEY2=value2|...\n

Index layout (1-based, matches .ssproj dataset indices):
   1  OS              8  CPU_USAGE      15  CPU_TEMP
   2  HOSTNAME        9  RAM_USAGE      16  SWAP_USED
   3  CPU_MODEL      10  RAM_USED       17  NET_SENT
   4  CPU_CORES      11  DISK_USAGE     18  NET_RECV
   5  CPU_THREADS    12  DISK_USED      19  PROCESSES
   6  RAM_TOTAL      13  CORE0          ...
   7  DISK_TOTAL     14  CORE1          44  CORE31
                                        45  PROC0
                                        ...
                                        54  PROC9

Dependencies:
    pip install psutil py-cpuinfo

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys
import time
import platform
import socket
import threading

try:
    import psutil
except ImportError:
    print("ERROR: psutil is not installed.", file=sys.stderr)
    print("Run:  pip install psutil py-cpuinfo", file=sys.stderr)
    sys.exit(1)

try:
    import cpuinfo
    _CPUINFO_AVAILABLE = True
except ImportError:
    _CPUINFO_AVAILABLE = False

MAX_CORES = 32
TOP_PROCS = 10

# ---------------------------------------------------------------------------
# Background CPU sampler
#
# psutil.cpu_percent(interval=N) blocks for N seconds while measuring.
# On macOS interval=None always returns 0.0 on the first call per process.
# Running the blocking call on a dedicated thread keeps the main loop fast
# while still getting accurate per-core measurements.
# ---------------------------------------------------------------------------

_cpu_lock   = threading.Lock()
_cpu_total  = 0.0
_cpu_cores  = [0.0] * MAX_CORES

def _cpu_sampler() -> None:
    global _cpu_total, _cpu_cores
    while True:
        cores = psutil.cpu_percent(interval=1.0, percpu=True)
        total = sum(cores) / len(cores) if cores else 0.0
        with _cpu_lock:
            _cpu_total = round(total, 1)
            _cpu_cores = [round(c, 1) for c in cores[:MAX_CORES]]


def _start_cpu_sampler() -> None:
    t = threading.Thread(target=_cpu_sampler, daemon=True)
    t.start()
    # Wait for the first real measurement before emitting any live frames.
    time.sleep(1.05)


def _get_cpu() -> tuple[float, list[float]]:
    with _cpu_lock:
        return _cpu_total, list(_cpu_cores)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _cpu_model() -> str:
    """Return a human-readable CPU model string, cross-platform."""
    if _CPUINFO_AVAILABLE:
        try:
            brand = cpuinfo.get_cpu_info().get("brand_raw", "")
            if brand:
                return brand[:48]
        except Exception:
            pass

    processor = platform.processor()
    machine   = platform.machine()
    if processor and processor not in ("", machine):
        return processor[:48]

    return machine or "Unknown"


def _disk_root() -> "tuple[str, object] | tuple[None, None]":
    """Return (path, disk_usage) for the root / primary partition."""
    for path in ("/", "C:\\"):
        try:
            return path, psutil.disk_usage(path)
        except Exception:
            continue
    return None, None


def _cpu_temp() -> str:
    """Return CPU temperature in °C, or 'N/A' if unavailable (e.g. macOS)."""
    if not hasattr(psutil, "sensors_temperatures"):
        return "N/A"
    try:
        temps = psutil.sensors_temperatures()
        if not temps:
            return "N/A"
        for key in ("coretemp", "cpu_thermal", "k10temp", "zenpower",
                    "cpu-thermal", "acpitz"):
            if key in temps and temps[key]:
                return str(round(temps[key][0].current, 1))
        for entries in temps.values():
            if entries:
                return str(round(entries[0].current, 1))
    except Exception:
        pass
    return "N/A"


_net_prev: tuple[int, int] = (0, 0)
_net_ts: float = 0.0

def _net_mbps() -> tuple[float, float]:
    """Return (sent MB/s, recv MB/s) since the last call."""
    global _net_prev, _net_ts
    c    = psutil.net_io_counters()
    now  = time.monotonic()
    dt   = max(now - _net_ts, 0.001)
    sent = round((c.bytes_sent - _net_prev[0]) / 1048576 / dt, 3)
    recv = round((c.bytes_recv - _net_prev[1]) / 1048576 / dt, 3)
    _net_prev = (c.bytes_sent, c.bytes_recv)
    _net_ts   = now
    return sent, recv


def _top_processes(n: int) -> list[str]:
    """
    Return top-n processes by CPU as formatted strings: "Name (nn% CPU)".

    Each entry is a single value so it fits cleanly in a datagrid cell.
    Slots with no process are filled with '---'.
    """
    procs = []
    for p in psutil.process_iter(["name", "cpu_percent"]):
        try:
            name = (p.info["name"] or "?")[:20].strip()
            cpu  = p.info["cpu_percent"] or 0.0
            procs.append((cpu, name))
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass

    procs.sort(reverse=True)
    result = [f"{name} ({cpu:.1f}% CPU)" for cpu, name in procs[:n]]
    while len(result) < n:
        result.append("---")
    return result


def _emit(frame: str) -> None:
    """Write a single framed line to stdout and flush immediately."""
    sys.stdout.write(f"${frame}\n")
    sys.stdout.flush()


# ---------------------------------------------------------------------------
# Header — static system information (emitted once)
# ---------------------------------------------------------------------------

def emit_header() -> None:
    """
    Emit one frame of static system metadata.

    The persistent JS parser stores these values and keeps them visible
    while subsequent live frames update only the dynamic keys.
    """
    _, disk   = _disk_root()
    ram       = psutil.virtual_memory()
    cpu_cores = psutil.cpu_count(logical=False) or 1
    cpu_threads = psutil.cpu_count(logical=True) or 1

    fields = [
        f"OS={platform.system()} {platform.release()}",
        f"HOSTNAME={socket.gethostname()[:24]}",
        f"CPU_MODEL={_cpu_model()}",
        f"CPU_CORES={cpu_cores}",
        f"CPU_THREADS={cpu_threads}",
        f"RAM_TOTAL={round(ram.total / 1e9, 1)} GB",
        f"DISK_TOTAL={round(disk.total / 1e9, 1) if disk else 0.0} GB",
    ]
    _emit("|".join(fields))


# ---------------------------------------------------------------------------
# Live loop
# ---------------------------------------------------------------------------

def run_live(interval: float) -> None:
    """
    Emit live telemetry frames at the requested rate.

    CPU percentages come from the background sampler thread (1 Hz accuracy,
    non-blocking reads). Process list and disk stats refresh every second
    to keep per-frame cost low at high frame rates.
    """
    disk_path, _ = _disk_root()

    # Prime network counter.
    c = psutil.net_io_counters()
    global _net_prev, _net_ts
    _net_prev = (c.bytes_sent, c.bytes_recv)
    _net_ts   = time.monotonic()

    # Slow-refresh cache (updated at ~1 Hz regardless of frame rate).
    _slow_cache: dict = {}
    _last_slow  = 0.0

    def _refresh_slow() -> None:
        disk_pct  = 0.0
        disk_used = 0.0
        if disk_path:
            try:
                du        = psutil.disk_usage(disk_path)
                disk_pct  = du.percent
                disk_used = round(du.used / 1e9, 1)
            except Exception:
                pass

        swap = psutil.swap_memory()

        _slow_cache["disk_pct"]  = disk_pct
        _slow_cache["disk_used"] = disk_used
        _slow_cache["swap_used"] = round(swap.used / 1e9, 2)
        _slow_cache["cpu_temp"]  = _cpu_temp()
        _slow_cache["procs"]     = _top_processes(TOP_PROCS)
        _slow_cache["n_procs"]   = len(psutil.pids())

    # Populate cache before first frame.
    _refresh_slow()
    _last_slow = time.monotonic()

    while True:
        t0 = time.monotonic()

        # Refresh slow metrics at ~1 Hz.
        if t0 - _last_slow >= 1.0:
            _refresh_slow()
            _last_slow = t0

        cpu_pct, core_pcts = _get_cpu()
        ram      = psutil.virtual_memory()
        sent_mbps, recv_mbps = _net_mbps()

        fields = [
            f"CPU_USAGE={cpu_pct}",
            f"RAM_USAGE={ram.percent}",
            f"RAM_USED={round(ram.used / 1e9, 2)}",
            f"DISK_USAGE={_slow_cache['disk_pct']}",
            f"DISK_USED={_slow_cache['disk_used']}",
            f"CPU_TEMP={_slow_cache['cpu_temp']}",
            f"SWAP_USED={_slow_cache['swap_used']}",
            f"NET_SENT={sent_mbps}",
            f"NET_RECV={recv_mbps}",
            f"PROCESSES={_slow_cache['n_procs']}",
        ]

        for i in range(MAX_CORES):
            pct = core_pcts[i] if i < len(core_pcts) else -1
            fields.append(f"CORE{i}={pct}")

        for i, label in enumerate(_slow_cache["procs"]):
            fields.append(f"PROC{i}={label}")

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
        default=1 / 30,
        metavar="SECONDS",
        help="Seconds between live frames (default: 1/30 ≈ 0.033)",
    )
    args = parser.parse_args()

    _start_cpu_sampler()
    emit_header()
    try:
        run_live(interval=args.interval)
    except KeyboardInterrupt:
        pass
