"""
Plot Cost Under Pressure (spec 0097, Scenario P, AC6 / AC9)

Streams CSV frames at 1 kHz from a local TCP feeder into the running Serial
Studio and measures footprint growth plus the plot-history bytes reported by
dashboard.getStatus (plotMemory.historyBytes) for N single-curve time plots
and for 4 multiplots x 8 curves.

WARNING: every phase REPLACES the project open in the running app and
connects/disconnects it. Run it deliberately, never with unsaved work.
Needs the API server on 127.0.0.1:7777. If the app shows a consent prompt for
project.loadJson, approve it, or start the app with
SERIAL_STUDIO_API_AUTO_CONSENT=1 (headless).

Footprint: macOS `vmmap --summary` "Physical footprint"; elsewhere psutil RSS.
Optional AC9 profile: SS_PLOT_PRESSURE_PROFILE=1 (macOS) runs `sample` during
the 30-plot phase and prints the QQuickWindow / displayString wait shares.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import math
import os
import re
import socket
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils import SerialStudioClient
from utils.api_client import APIError

try:
    import psutil
except ImportError:
    psutil = None

RATE_HZ = 1000
SETTLE_S = 15.0
MB = {"K": 1 / 1024, "M": 1, "G": 1024}


def _api_up():
    try:
        socket.create_connection(("127.0.0.1", 7777), timeout=1.0).close()
        return True
    except OSError:
        return False


pytestmark = [
    pytest.mark.performance,
    pytest.mark.skipif(
        not _api_up(), reason="Serial Studio API (127.0.0.1:7777) is down"
    ),
]


class Feeder:
    """TCP server that streams `/*v1,...,vN*/` frames at RATE_HZ to one client."""

    def __init__(self, ncols):
        self.ncols = ncols
        self.stop = False
        self.srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.srv.bind(("127.0.0.1", 0))
        self.srv.listen(1)
        self.port = self.srv.getsockname()[1]
        self.thread = threading.Thread(target=self._accept, daemon=True)
        self.thread.start()

    def _accept(self):
        self.srv.settimeout(0.5)
        while not self.stop:
            try:
                conn, _ = self.srv.accept()
            except OSError:
                continue
            self._stream(conn)

    def _stream(self, conn):
        i, per_tick, nxt = 0, max(1, RATE_HZ // 100), time.perf_counter()
        while not self.stop:
            buf = []
            for _ in range(per_tick):
                vals = ",".join(
                    f"{math.sin(i * 0.01 + k):.4f}" for k in range(self.ncols)
                )
                buf.append(f"/*{vals}*/\n")
                i += 1
            try:
                conn.sendall("".join(buf).encode())
            except OSError:
                return
            nxt += 0.01
            time.sleep(max(0.0, nxt - time.perf_counter()))

    def close(self):
        self.stop = True
        self.thread.join(timeout=2.0)
        self.srv.close()


def _project(mode, n):
    ds = [
        {
            "title": f"D{k}",
            "value": "",
            "index": k + 1,
            "graph": mode == "plot",
            "units": "",
        }
        for k in range(n)
    ]
    if mode == "plot":
        groups = [{"title": "Data", "widget": "", "sourceId": 0, "datasets": ds}]
    else:
        groups = [
            {
                "title": f"MP{g}",
                "widget": "multiplot",
                "sourceId": 0,
                "datasets": ds[g : g + 8],
            }
            for g in range(0, n, 8)
        ]
    return {
        "title": f"PlotPressure {mode} {n}",
        "frameStart": "/*",
        "frameEnd": "*/",
        "frameDetection": 1,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Sim",
                "busType": 1,
                "frameStart": "",
                "frameEnd": "\n",
                "frameDetection": 1,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "connectionSettings": {},
            }
        ],
        "groups": groups,
        "actions": [],
    }


def _find_pid():
    """PID of the running Serial Studio app, or None."""
    if psutil is not None:
        for p in psutil.process_iter(["name", "pid"]):
            if "serial-studio" in (p.info["name"] or "").lower().replace(" ", "-"):
                return p.info["pid"]
    out = subprocess.run(
        ["pgrep", "-fi", "Serial[- ]Studio.*/MacOS/|^Serial-Studio"],
        capture_output=True,
        text=True,
    ).stdout.split()
    pids = [int(x) for x in out if int(x) != os.getpid()]
    return pids[0] if pids else None


def _footprint_mb(pid):
    """Physical footprint in MB (vmmap on macOS, RSS elsewhere), or None."""
    if sys.platform == "darwin":
        out = subprocess.run(
            ["vmmap", "--summary", str(pid)], capture_output=True, text=True
        ).stdout
        m = re.search(r"Physical footprint:\s+([\d.]+)([KMG])", out)
        if m:
            return float(m.group(1)) * MB[m.group(2)]
    if psutil is not None:
        return psutil.Process(pid).memory_info().rss / 1024 / 1024
    return None


def _profile_shares(path):
    """Share of main-thread samples under QQuickWindow waits / displayString."""
    stack, total, wait, disp, in_main = [], 0, 0, 0, False
    for line in Path(path).read_text(errors="replace").splitlines():
        if "Thread_" in line:
            in_main = "main-thread" in line
            stack, total = [], 0
        m = re.match(r"^([ +!:|]*)(\d+) (.*)$", line)
        if not in_main or not m:
            continue
        col, count, name = len(m.group(1)), int(m.group(2)), m.group(3)
        while stack and stack[-1][0] >= col:
            stack.pop()
        stack.append((col, name))
        total = total or count
        if "displayString" in name:
            disp += count
        if "QWaitCondition" in name and any("QQuickWindow" in s[1] for s in stack):
            wait += count
    return (wait / total, disp / total) if total else (None, None)


def _run_phase(mode, n, profile=False):
    """Load, connect, stream SETTLE_S, return a result dict. Always disconnects."""
    feeder = Feeder(n)
    pid = _find_pid()
    try:
        with SerialStudioClient() as api:
            try:
                api.command("io.disconnect")
            except APIError:
                pass
        time.sleep(1.5)
        with SerialStudioClient() as api:
            try:
                api.load_project_from_json(_project(mode, n))
            except APIError as e:
                if e.code == "CONSENT_REQUIRED":
                    pytest.skip(
                        "project.loadJson needs consent: approve the prompt in the "
                        "app, or run it with SERIAL_STUDIO_API_AUTO_CONSENT=1"
                    )
                raise
            time.sleep(0.5)
            api.configure_network(host="127.0.0.1", port=feeder.port, socket_type="tcp")
            api.set_operation_mode("project")
            api.command("project.activate")
        time.sleep(1.0)
        before = _footprint_mb(pid) if pid else None
        with SerialStudioClient() as api:
            api.connect_device()
        sample_out = None
        if profile and pid and sys.platform == "darwin":
            sample_out = tempfile.mktemp(suffix=".sample.txt")
            subprocess.run(
                ["sample", str(pid), "10", "-file", sample_out], capture_output=True
            )
            time.sleep(max(0.0, SETTLE_S - 10))
        else:
            time.sleep(SETTLE_S)
        during = _footprint_mb(pid) if pid else None
        with SerialStudioClient() as api:
            plot = api.command("dashboard.getStatus").get("plotMemory", {})
        res = {
            "phase": f"{mode}-{n}",
            "rate_hz": RATE_HZ,
            "footprint_before_mb": before,
            "footprint_during_mb": during,
            "historyBytes": plot.get("historyBytes"),
            "inexactCurves": plot.get("inexactCurves"),
        }
        res["growth_mb"] = (during - before) if None not in (before, during) else None
        if sample_out and Path(sample_out).exists():
            res["gui_wait_in_QQuickWindow"], res["gui_displayString"] = _profile_shares(
                sample_out
            )
        print(json.dumps(res), flush=True)
        return res
    finally:
        try:
            with SerialStudioClient() as api:
                api.command("io.disconnect")
        except Exception:
            pass
        feeder.close()


def _check_growth(res, limit_mb):
    if res["growth_mb"] is None:
        print(
            "footprint unavailable (no PID / vmmap / psutil): growth assertion skipped"
        )
        return
    assert res["growth_mb"] <= limit_mb, f"footprint growth {res['growth_mb']:.1f} MB"


def _check_history(res):
    assert (
        res["historyBytes"] is not None
    ), "dashboard.getStatus lacks plotMemory.historyBytes"
    assert (
        res["historyBytes"] <= 15 * 1024 * 1024
    ), f"historyBytes {res['historyBytes']}"


def test_plot_pressure_10_plots():
    """AC6: 10 single-curve plots grow the footprint by <= 45 MB."""
    _check_growth(_run_phase("plot", 10), 45)


def test_plot_pressure_30_plots():
    """AC6 + AC9: 30 plots grow <= 120 MB, plot history <= 15 MB."""
    profile = os.environ.get("SS_PLOT_PRESSURE_PROFILE") == "1"
    res = _run_phase("plot", 30, profile=profile)
    _check_growth(res, 120)
    _check_history(res)


def test_plot_pressure_multiplots_4x8():
    """AC6: 4 multiplots x 8 curves keep history <= 15 MB and every curve exact."""
    res = _run_phase("multiplot", 32)
    _check_history(res)
    assert res["inexactCurves"] == 0, f"inexactCurves {res['inexactCurves']}"
