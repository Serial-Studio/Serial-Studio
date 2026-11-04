"""
Device Declaration Tests (spec 0111)

A device that declares its dashboard on the wire builds it in Quick Plot with no
project file. The Python firmware library serves the declaration over a loopback
TCP link, and the dashboard structure is read back over the API.

Requires a running Serial Studio instance with the API server enabled.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import sys
import threading
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "firmware" / "python"))

import serialstudio  # noqa: E402
from utils.api_client import APIError  # noqa: E402

PORT = 9000
DEGREE_C = "℃"
MAIN = "main"
NOTICE_CHANNEL = "Device Declaration"
SLIDER = "dashboard.output.1.0.slider"
CONSENT_WAIT = 30.0


class Sketch:
    """The MPU6050 sketch, looped on a thread the way a microcontroller runs it."""

    def __init__(self, device):
        self.device = device
        self._running = True
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def _loop(self):
        d = self.device
        while self._running:
            d.group("Accelerometer", "accelerometer")
            for name, axis, value in (
                ("Accelerometer X", "x", 0.12),
                ("Accelerometer Y", "y", -0.34),
                ("Accelerometer Z", "z", 9.81),
            ):
                d.channel(name, value, units="m/s²", widget=axis)
            d.group("Gyroscope", "gyro")
            for name, axis, value in (
                ("Gyro X", "x", 0.01),
                ("Gyro Y", "y", -0.02),
                ("Gyro Z", "z", 0.03),
            ):
                d.channel(name, value, units="deg/s", widget=axis)
            d.group("Temperature")
            d.channel(
                "Temperature",
                24.5,
                units=DEGREE_C,
                widget="bar",
                range=(0, 100),
                graph=False,
            )
            d.send()
            time.sleep(0.05)

    def stop(self):
        self._running = False
        self._thread.join(timeout=2.0)


def dashboard_groups(api_client):
    frame = api_client.command("dashboard.getData").get("frame", {})
    return frame.get("groups", [])


def wait_for_groups(api_client, count, timeout=10.0):
    deadline = time.time() + timeout
    groups = []
    while time.time() < deadline:
        groups = dashboard_groups(api_client)
        if len(groups) >= count:
            return groups
        time.sleep(0.1)
    return groups


@pytest.fixture
def declared_device():
    """The Python library serving one device on the port the app connects to."""
    device = serialstudio.Device("MPU6050")
    server = serialstudio.tcp_server(device, port=PORT)
    sketch = Sketch(device)
    yield device
    sketch.stop()
    server.close()


@pytest.mark.integration
class TestDeviceDeclaration:
    """A declaring device in Quick Plot."""

    def test_declared_dashboard(self, api_client, declared_device, clean_state):
        """AC1: groups, widgets, titles, units and ranges come from the declaration."""
        api_client.set_operation_mode("quickplot")
        time.sleep(0.5)  # Let queued rebuildDevices settle before configuring
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        time.sleep(0.3)
        api_client.connect_device()
        assert wait_until(api_client.is_connected), "the app never connected"
        declared_device.declare_due = True  # a device booting after the link opens

        groups = wait_for_groups(api_client, 3)
        api_client.disconnect_device()

        by_title = {g.get("title"): g for g in groups}
        assert {"Accelerometer", "Gyroscope", "Temperature"} <= set(by_title), groups
        assert by_title["Accelerometer"].get("widget") == "accelerometer"
        assert by_title["Gyroscope"].get("widget") == "gyro"

        accel = {
            d.get("title"): d for d in by_title["Accelerometer"].get("datasets", [])
        }
        assert set(accel) == {"Accelerometer X", "Accelerometer Y", "Accelerometer Z"}
        assert all(d.get("units") == "m/s²" for d in accel.values()), accel

        temp = by_title["Temperature"]["datasets"][0]
        assert temp.get("title") == "Temperature"
        assert temp.get("units") == DEGREE_C
        assert temp.get("widget") == "bar"
        assert float(temp["widgetMax"]) == pytest.approx(100.0), temp
        assert float(temp["plotMax"]) == pytest.approx(100.0), temp

    def test_declaration_in_project_mode(
        self, api_client, declared_device, clean_state
    ):
        """AC3: a differing declaration posts one notice, leaves the project, and adopts."""
        api_client.load_project_from_json(loaded_project())
        time.sleep(0.3)
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        api_client.set_operation_mode("project")
        try:
            api_client.command("project.activate")
        except APIError:
            pass

        api_client.connect_device()
        assert wait_until(api_client.is_connected), "the app never connected"
        time.sleep(0.5)
        before = exported(api_client)
        notices = len(declaration_notices(api_client))

        declared_device.declare_due = True
        assert wait_until(
            lambda: len(declaration_notices(api_client)) > notices
        ), "no notice for the differing declaration"
        time.sleep(1.0)
        assert len(declaration_notices(api_client)) == notices + 1
        assert (
            exported(api_client) == before
        ), "the declaration changed the loaded project"

        status = api_client.command("project.declaration.get")
        assert status.get("status") == "differs", status
        assert status.get("pendingDifference") is True, status

        adopted = api_client.command(
            "project.declaration.adopt", {"discardChanges": True}
        )
        assert adopted.get("title") == "MPU6050", adopted
        assert adopted.get("groups") == 3, adopted
        assert adopted.get("modified") is True, adopted
        assert not api_client.is_connected(), "adopt left the link up"

        titles = {g.get("title") for g in exported(api_client).get("groups", [])}
        assert {"Accelerometer", "Gyroscope", "Temperature"} <= titles, titles
        status = api_client.command("project.declaration.get")
        assert status.get("pendingDifference") is False, status

    def test_keep_dismisses_the_offer(self, api_client, declared_device, clean_state):
        """R3: Keep Project dismisses the offer and leaves the loaded project as it was."""
        api_client.load_project_from_json(loaded_project())
        time.sleep(0.3)
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        api_client.set_operation_mode("project")
        api_client.connect_device()
        assert wait_until(api_client.is_connected), "the app never connected"
        time.sleep(0.5)
        before = exported(api_client)

        declared_device.declare_due = True
        assert wait_until(
            lambda: api_client.command("project.declaration.get").get(
                "pendingDifference"
            )
            is True
        ), "the differing declaration raised no offer"

        kept = api_client.command("project.declaration.keep")
        assert kept.get("kept") is True, kept
        status = api_client.command("project.declaration.get")
        assert status.get("status") == "kept", status
        assert status.get("pendingDifference") is False, status
        assert exported(api_client) == before, "keep changed the loaded project"
        api_client.disconnect_device()

    def test_reconnect_request(self, api_client, declared_device, clean_state):
        """AC4: a device that declared once is rebuilt after a reconnect by the app's request."""
        status = api_client.command("project.declaration.get")
        if not status.get("requestOnConnect"):
            pytest.skip(
                "Ask Devices for Their Dashboard is off (Preferences > Plotting, off by "
                "default); turn it on to run this test"
            )

        requests = RequestLog(declared_device)
        api_client.set_operation_mode("quickplot")
        time.sleep(0.5)
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        time.sleep(0.3)
        api_client.connect_device()
        assert wait_until(
            lambda: quick_plot_declared(api_client)
        ), "the first connect built no declared dashboard"

        api_client.disconnect_device()
        time.sleep(0.5)
        assert not declared_device.declare_due
        assert not quick_plot_declared(
            api_client
        ), "the declaration survived the disconnect"
        first = requests.count

        api_client.connect_device()
        assert wait_until(
            lambda: quick_plot_declared(api_client)
        ), "the reconnect did not rebuild the declared dashboard"
        titles = {g.get("title") for g in wait_for_groups(api_client, 3)}
        api_client.disconnect_device()

        assert requests.count == first + 1, "the reconnect sent no single request"
        assert {"Accelerometer", "Gyroscope", "Temperature"} <= titles, titles

    @pytest.mark.gui
    def test_declared_slider(self, api_client, clean_state):
        """AC5: a declared slider driven by its control name reaches the sketch's callback."""
        if not api_client.command_exists("licensing.getStatus"):
            pytest.skip("Commercial build required for output controls")
        if not api_client.command("ui.automation.listWindows")["windows"]:
            pytest.skip("no visible window: the app runs headless or offscreen")

        received = []
        device = serialstudio.Device(
            "Pump", on_control=lambda name, value: received.append((name, value))
        )
        server = serialstudio.tcp_server(device, port=PORT)
        sketch = ControlSketch(device)
        try:
            api_client.set_operation_mode("quickplot")
            time.sleep(0.5)
            api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
            time.sleep(0.3)
            api_client.connect_device()
            assert wait_until(
                lambda: element(api_client, SLIDER) is not None
            ), "the declared slider never appeared"

            drive(
                api_client,
                "ui.automation.key",
                {"window": MAIN, "target": SLIDER, "keys": "Up"},
            )
            assert wait_until(lambda: received, 3.0), "the sketch received no command"
            assert received[-1] == ("Speed", 1), received
        finally:
            try:
                if api_client.is_connected():
                    api_client.disconnect_device()
            except APIError:
                pass
            sketch.stop()
            server.close()


class ControlSketch:
    """A one-channel sketch with a declared Speed slider, looped on a thread."""

    def __init__(self, device):
        self.device = device
        self._running = True
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def _loop(self):
        d = self.device
        while self._running:
            d.group("Status")
            d.channel("Level", 1.0)
            d.control_group("Controls")
            d.control("Speed", "slider", min=0, max=255, step=1)
            d.send()
            time.sleep(0.05)

    def stop(self):
        self._running = False
        self._thread.join(timeout=2.0)


class RequestLog:
    """Counts the declaration requests the app sends, by wrapping the device's line dispatch."""

    def __init__(self, device):
        self.count = 0
        self._dispatch = device._dispatch
        device._dispatch = self._record

    def _record(self, line):
        if line.startswith("#ss?"):
            self.count += 1
        self._dispatch(line)


def loaded_project():
    """A one-group project over the loopback device that differs from the MPU6050 declaration."""
    return {
        "title": "Loaded Project",
        "frameEnd": "\\n",
        "frameDetection": 0,
        "decoder": 0,
        "frameParserCode": "function parse(frame) {\n  return frame.split(',');\n}\n",
        "frameParserLanguage": 0,
        "groups": [
            {
                "title": "Raw",
                "widget": "datagrid",
                "datasets": [
                    {"title": "First", "value": "%1", "index": 1, "widget": ""}
                ],
            }
        ],
        "actions": [],
    }


def exported(api_client):
    config = api_client.command("project.exportJson")["config"]
    return json.loads(config) if isinstance(config, str) else config


def declaration_notices(api_client):
    events = api_client.command("notifications.list").get("events", [])
    return [e for e in events if e.get("channel") == NOTICE_CHANNEL]


def quick_plot_declared(api_client):
    status = api_client.command("project.declaration.get")
    return bool(status.get("quickPlot", {}).get("declared"))


def wait_until(predicate, timeout=10.0):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return True
        time.sleep(0.1)
    return False


def element(api_client, element_id):
    params = {"window": MAIN, "id": element_id}
    for item in api_client.command("ui.automation.listElements", params)["elements"]:
        if item["id"] == element_id:
            return item
    return None


def drive(api_client, command, params):
    """Retries while the user has not yet answered the driving consent."""
    deadline = time.time() + CONSENT_WAIT
    while True:
        try:
            return api_client.command(command, params)
        except APIError as error:
            if error.code != "CONSENT_REQUIRED" or time.time() >= deadline:
                raise
            time.sleep(0.5)


REPLY_CELL = "dashboard.output.1.0"


def disconnect(api_client):
    try:
        if api_client.is_connected():
            api_client.disconnect_device()
    except APIError:
        pass


def declared_mpu_values(api_client, binary):
    """Title -> (units, value) of the MPU6050 sketch, sent as text lines or packed frames."""
    device = serialstudio.Device("MPU6050")
    if binary:
        device.binary("f32,f32,f32,f32,f32,f32,f32", "little", b"\xaa\x55", "CRC-16")

    server = serialstudio.tcp_server(device, port=PORT)
    sketch = Sketch(device)
    try:
        api_client.set_operation_mode("quickplot")
        time.sleep(0.5)
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        time.sleep(0.3)
        api_client.connect_device()

        def seen():
            out = {}
            for group in dashboard_groups(api_client):
                for dataset in group.get("datasets", []):
                    out[dataset.get("title")] = dataset
            return out

        assert wait_until(
            lambda: len(seen()) >= 7
            and all(d.get("value") not in (None, "") for d in seen().values()),
            15.0,
        ), f"the declared dashboard never filled: {seen()}"
        return {t: (d.get("units"), float(d["value"])) for t, d in seen().items()}
    finally:
        disconnect(api_client)
        sketch.stop()
        server.close()


@pytest.mark.integration
class TestDeclaredBinary:
    """A packed binary device builds the same dashboard as its text variant."""

    def test_declared_binary(self, api_client, clean_state):
        """AC10: titles, units and values over packed frames equal the text variant's."""
        text = declared_mpu_values(api_client, binary=False)
        packed = declared_mpu_values(api_client, binary=True)

        assert set(packed) == set(text) and len(text) == 7, (text, packed)
        for title, (units, value) in text.items():
            assert packed[title][0] == units, (title, text, packed)
            assert packed[title][1] == pytest.approx(value), (title, text, packed)


@pytest.mark.integration
class TestDeclaredReply:
    """A declared control shows the library's reply: acknowledged, then refused."""

    def test_declared_reply(self, api_client, clean_state):
        """Spec 0110 through 0111: the echoed token and code settle the control's state."""
        if not api_client.command_exists("licensing.getStatus"):
            pytest.skip("Commercial build required for output controls")
        if not api_client.command("ui.automation.listWindows")["windows"]:
            pytest.skip("no visible window: the app runs headless or offscreen")

        answers = iter([None, (3, "too hot")])
        device = serialstudio.Device(
            "Pump", on_control=lambda name, value: next(answers)
        )
        server = serialstudio.tcp_server(device, port=PORT)
        sketch = ControlSketch(device)

        def outcome():
            return (element(api_client, REPLY_CELL) or {}).get("state", {})

        try:
            api_client.set_operation_mode("quickplot")
            time.sleep(0.5)
            api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
            time.sleep(0.3)
            api_client.connect_device()
            assert wait_until(lambda: element(api_client, SLIDER) is not None)

            key = {"window": MAIN, "target": SLIDER, "keys": "Up"}
            drive(api_client, "ui.automation.key", key)
            assert wait_until(
                lambda: outcome().get("outcome") == "acknowledged", 5.0
            ), outcome()

            drive(api_client, "ui.automation.key", key)
            assert wait_until(
                lambda: outcome().get("outcome") == "refused", 5.0
            ), outcome()
            assert outcome().get("code") == 3
            assert outcome().get("message") == "too hot"
        finally:
            disconnect(api_client)
            sketch.stop()
            server.close()
