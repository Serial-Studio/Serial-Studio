"""Spec 0103 -- a UART Connection Sequence brings a device up before any byte becomes data.

The test owns the far end of a socat pty pair and plays the device: it answers the sequence's
probe after a delay (sending noise meanwhile), or never answers, then streams CSV lines. The
noise uses a value no data line carries, so a handshake byte that leaked into the frame path
shows up in the latest frame or the CSV export.

Requires the app up with the API server enabled (see tests/README.md). Skips on Windows (no pty
pairs) and without socat. BREAK is not exercised here: a pty does not hold a line condition, so
BREAK timing stays a maintainer observation on real adapters (AC2).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import os
import shutil
import subprocess
import sys
import threading
import time
from pathlib import Path

import pytest

from utils.api_client import APIError

pytestmark = [pytest.mark.integration, pytest.mark.uart]

NOISE_LINE = b"31337,31337\n"
DATA_LINE = b"7,8\n"
PROBE = b"AT\r"
REPLY = b"OK\r\n"

# Discard, probe, wait for "OK"; the budget leaves room for the device's deliberate delay
PROBE_SEQUENCE = {
    "budgetMs": 5000,
    "steps": [
        {"kind": "discard"},
        {"kind": "send", "data": "AT", "hex": False, "eol": "\\r", "checksum": ""},
        {"kind": "expect", "patterns": ["4F 4B"], "timeoutMs": 3000, "optional": False},
    ],
}


class _PtyDevice:
    """Far end of the pty pair: waits for the probe, sends noise for `reply_delay` seconds,
    then answers and streams data lines. With `reply=None` it never answers."""

    def __init__(self, path: str, reply=REPLY, reply_delay: float = 1.5):
        self._fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self._reply = reply
        self._reply_delay = reply_delay
        self._running = True
        self.probed = threading.Event()
        self.answered = threading.Event()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _read(self) -> bytes:
        try:
            return os.read(self._fd, 512)
        except (BlockingIOError, OSError):
            return b""

    def _write(self, data: bytes) -> None:
        try:
            os.write(self._fd, data)
        except OSError:
            pass

    def _run(self) -> None:
        rx = b""
        probed_at = None
        while self._running:
            rx += self._read()
            if probed_at is None and PROBE in rx:
                probed_at = time.time()
                self.probed.set()

            if probed_at is None:
                time.sleep(0.02)
                continue

            if self._reply is None or time.time() - probed_at < self._reply_delay:
                self._write(NOISE_LINE)
                time.sleep(0.1)
                continue

            if not self.answered.is_set():
                self._write(self._reply)
                self.answered.set()

            self._write(DATA_LINE)
            time.sleep(0.05)

    def stop(self) -> None:
        self._running = False
        self._thread.join(timeout=2.0)
        os.close(self._fd)


@pytest.fixture
def pty_pair():
    """A disposable serial pair; the app opens one end, the test plays the device on the other."""
    if sys.platform.startswith("win"):
        pytest.skip("no pty pairs on Windows")

    if not shutil.which("socat"):
        pytest.skip("socat is required to create a disposable serial pair")

    workdir = os.environ.get("TMPDIR", "/tmp").rstrip("/")
    left = f"{workdir}/ss-seq-a"
    right = f"{workdir}/ss-seq-b"
    proc = subprocess.Popen(
        ["socat", f"PTY,link={left},raw,echo=0", f"PTY,link={right},raw,echo=0"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    deadline = time.time() + 5.0
    while not (os.path.exists(left) and os.path.exists(right)):
        if time.time() > deadline:
            proc.kill()
            pytest.skip("socat did not create the pty pair")
        time.sleep(0.1)

    yield left, right

    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()


@pytest.fixture
def uart_session(api_client, clean_state, pty_pair):
    """Quick Plot on the app's end of the pair; the sequence is cleared again afterwards, since
    the Setup Panel's sequence persists across sessions."""
    if not api_client.command_exists("io.uart.setSequence"):
        pytest.skip("this build has no Connection Sequence commands")

    left, right = pty_pair
    api_client.set_operation_mode("quickplot")
    api_client.set_bus_type("uart")
    api_client.command("io.uart.setBaudRate", {"baudRate": 9600})
    api_client.command("io.uart.setDevice", {"device": left})
    time.sleep(1.5)

    ports = api_client.command("io.uart.listPorts").get("portList", [])
    index = next((p["index"] for p in ports if p.get("name") == left), None)
    if index is None:
        pytest.skip("the registered custom port did not appear in the port list")

    api_client.command("io.uart.setPortIndex", {"portIndex": index})
    yield right

    try:
        api_client.command("io.disconnect")
    except Exception:
        pass
    api_client.command("io.uart.setSequence", {"sequence": {"steps": []}})


def _status(api_client) -> dict:
    return api_client.command("io.getStatus")


def _wait_for(predicate, budget_s: float, what: str):
    deadline = time.time() + budget_s
    last = None
    while time.time() < deadline:
        last = predicate()
        if last:
            return last
        time.sleep(0.1)
    pytest.fail(f"{what} did not happen within {budget_s:.1f}s")


def _latest_frame(api_client) -> dict:
    return api_client.command("io.getLatestFrame", {"encoding": "text"})


def _frame_sequence(api_client) -> int:
    latest = _latest_frame(api_client)
    return latest.get("sequence", -1) if latest.get("hasData") else -1


def _workspace_roots(subdir: str) -> list:
    roots = []
    env = os.environ.get("SS_WORKSPACE")
    if env:
        roots.append(Path(env) / subdir)
    roots.append(Path.home() / "Documents" / "Serial Studio" / subdir)
    roots.append(Path("/tmp") / "Serial Studio" / subdir)
    return roots


def _newest_since(subdir: str, suffix: str, start_time: float):
    newest, newest_mtime = None, start_time - 1.0
    for root in _workspace_roots(subdir):
        if not root.is_dir():
            continue
        for path in root.rglob(f"*{suffix}"):
            mtime = path.stat().st_mtime
            if mtime >= start_time and mtime > newest_mtime:
                newest, newest_mtime = path, mtime
    return newest


def test_connects_only_after_the_device_answers(api_client, uart_session):
    """AC4, AC5, AC6: connecting until the reply, no frame from the handshake, data after it,
    and none of the handshake bytes in the CSV export of the session."""
    device = _PtyDevice(uart_session, reply=REPLY, reply_delay=1.5)
    started = time.time()
    try:
        api_client.command("io.uart.setSequence", {"sequence": PROBE_SEQUENCE})
        api_client.enable_csv_export()
        before = _frame_sequence(api_client)

        api_client.command("io.connect", timeout=15.0)
        assert device.probed.wait(timeout=5.0), "the sequence never sent its probe"

        time.sleep(0.5)
        status = _status(api_client)
        assert (
            status.get("isConnected") is False
        ), f"connected before the reply: {status}"
        assert status.get("linkState") == "connecting", status
        assert (
            _frame_sequence(api_client) == before
        ), "a frame arrived during the handshake"

        _wait_for(
            lambda: _status(api_client).get("isConnected"), 8.0, "connect after reply"
        )
        assert device.answered.is_set()

        _wait_for(
            lambda: "7,8" in _latest_frame(api_client).get("text", ""),
            5.0,
            "a data frame after the sequence",
        )
        assert "31337" not in _latest_frame(api_client).get("text", "")

        api_client.command("io.disconnect")
        api_client.command("csvExport.close")
        api_client.disable_csv_export()
        time.sleep(1.0)
    finally:
        device.stop()

    csv = _newest_since("CSV", ".csv", started)
    if csv is not None:
        assert "31337" not in csv.read_text(
            errors="replace"
        ), f"handshake bytes in {csv}"


def test_unanswered_sequence_fails_within_the_budget(api_client, uart_session):
    """AC4 (negative): a silent device fails the connect within the budget, never connects, and
    the exported console names the expect step."""
    device = _PtyDevice(uart_session, reply=None)
    started = time.time()
    console_export = api_client.command_exists("consoleExport.setEnabled")
    try:
        sequence = dict(PROBE_SEQUENCE, budgetMs=2000)
        sequence["steps"] = [dict(s) for s in PROBE_SEQUENCE["steps"]]
        sequence["steps"][2]["timeoutMs"] = 1500
        api_client.command("io.uart.setSequence", {"sequence": sequence})
        if console_export:
            api_client.command("consoleExport.setEnabled", {"enabled": True})

        api_client.command("io.connect", timeout=15.0)
        deadline = time.time() + 5.0
        settled = None
        while time.time() < deadline:
            status = _status(api_client)
            assert status.get("isConnected") is not True, "connected to a silent device"
            if status.get("linkState") == "idle":
                settled = status
                break
            time.sleep(0.1)

        assert settled is not None, "the failed sequence never settled the connect"
    finally:
        device.stop()
        if console_export:
            api_client.command("consoleExport.close")
            api_client.command("consoleExport.setEnabled", {"enabled": False})

    if not console_export:
        return

    time.sleep(0.5)
    log = _newest_since("Console", ".txt", started)
    if log is not None:
        text = log.read_text(errors="replace")
        assert "[Connection Sequence]" in text
        assert "step 3" in text and "Expect Reply" in text, text[-2000:]


def test_live_baud_change_keeps_the_connection(api_client, uart_session):
    """AC3: a live baud change keeps the link connected and leaves the saved rate alone."""
    device = _PtyDevice(uart_session, reply=REPLY, reply_delay=0.1)
    try:
        api_client.command("io.uart.setSequence", {"sequence": PROBE_SEQUENCE})
        api_client.command("io.connect", timeout=15.0)
        _wait_for(lambda: _status(api_client).get("isConnected"), 8.0, "connect")

        try:
            reply = api_client.command("io.uart.setLiveBaudRate", {"baudRate": 115200})
        except APIError as error:
            pytest.skip(f"this pty refuses a live baud change: {error}")

        assert reply["liveBaudRate"] == 115200
        assert reply["baudRate"] == 9600
        assert _status(api_client).get("isConnected") is True

        before = _frame_sequence(api_client)
        _wait_for(
            lambda: _frame_sequence(api_client) > before,
            5.0,
            "frames after the baud change",
        )
        assert api_client.command("io.uart.getConfig")["baudRate"] == 9600
    finally:
        device.stop()


def test_line_control_commands_refuse_a_closed_port(api_client, uart_session):
    """The live-control commands act on a connected port only, and say why they refuse."""
    for name, params in (
        ("io.uart.sendBreak", {"durationMs": 55}),
        ("io.uart.setLiveBaudRate", {"baudRate": 115200}),
        ("io.uart.getLineStates", {}),
        ("io.uart.runSequence", {}),
    ):
        with pytest.raises(APIError):
            api_client.command(name, params)


def test_sequence_round_trips_through_a_project(api_client, clean_state, temp_dir):
    """AC7: a sequence set on a project source survives save and reopen; a project saved before
    spec 0103 opens with no sequence, whatever the Setup Panel held."""
    if not api_client.command_exists("io.uart.setSequence"):
        pytest.skip("this build has no Connection Sequence commands")

    try:
        api_client.set_operation_mode("project")
        api_client.create_new_project(title="Sequence Round Trip")
        api_client.source_update(0, busType=0)
        api_client.source_set_property(0, "connectionSequence", PROBE_SEQUENCE)

        path = Path(temp_dir) / "sequence.ssproj"
        api_client.command("project.save", {"filePath": str(path)})
        api_client.load_project(str(path))

        connection = api_client.source_get_configuration(0).get("connection", {})
        steps = connection.get("connectionSequence", {}).get("steps", [])
        assert [s.get("kind") for s in steps] == ["discard", "send", "expect"]

        api_client.command("io.uart.setSequence", {"sequence": PROBE_SEQUENCE})
        api_client.load_project_from_json(
            {
                "title": "Pre-0103",
                "sources": [
                    {
                        "sourceId": 0,
                        "title": "Adapter",
                        "busType": 0,
                        "frameEnd": "\n",
                        "frameDetection": 0,
                        "connection": {"baudRate": 115200, "dtr": True},
                    }
                ],
                "groups": [],
            }
        )
        time.sleep(0.5)
        assert api_client.command("io.uart.getSequence")["stepCount"] == 0
    finally:
        api_client.command("io.uart.setSequence", {"sequence": {"steps": []}})
