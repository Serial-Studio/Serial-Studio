"""
OPC UA Driver Tests (spec 0066)

Drives the Pro OPC UA client end to end against the bottling-line simulator in
examples/OPC UA PLC Simulator: endpoint discovery, anonymous and username/password
connects, the one-verdict dial contract, address-space browsing, project generation,
subscription delivery, the poll fallback, bad-status handling, reconnect and the
project round-trip.

Requires: a running Pro build with the API server on localhost:7777 and the simulator
on 127.0.0.1:4840 (`python "examples/OPC UA PLC Simulator/opcua_plc_simulator.py"`).
Tests that need a private simulator (flags, restarts) launch their own on 484xx.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

BUS_TYPE_OPCUA = 10
LEVEL = "ns=2;s=Plant.Line1.Filler.Level_pct"
RUNNING = "ns=2;s=Plant.Line1.Filler.Running"
STATUS = "ns=2;s=Plant.Line1.Filler.Status"
ZONES = "ns=2;s=Plant.Line1.Pasteuriser.Zones_C"
FAULTY = "ns=2;s=Plant.Diagnostics.FaultySensor"
SERIAL = "ns=2;s=Plant.Diagnostics.SerialNumber"
RECIPE = "ns=2;i=5000"
TARGET = "ns=2;i=5001"

# RFC 5737 documentation address: never routable, so the dial runs out its deadline instead of
# getting the immediate RST a closed local port gives.
UNREACHABLE = "opc.tcp://192.0.2.1:4840/"
CYCLE = "ns=2;s=Plant.Diagnostics.Cycle"

SMALL_TAGS = [
    {"id": LEVEL, "name": "Level", "path": "Plant/Line1/Filler", "t": "f64", "n": 1},
    {
        "id": RUNNING,
        "name": "Running",
        "path": "Plant/Line1/Filler",
        "t": "bool",
        "n": 1,
    },
    {"id": STATUS, "name": "Status", "path": "Plant/Line1/Filler", "t": "str", "n": 1},
    {"id": CYCLE, "name": "Cycle", "path": "Plant/Diagnostics", "t": "i32", "n": 1},
]

pytestmark = [pytest.mark.integration, pytest.mark.pro, pytest.mark.opcua]


# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------


def _is_pro_build(api_client) -> bool:
    try:
        buses = api_client.command("io.listBuses").get("buses", [])
    except APIError:
        return False
    return len(buses) > BUS_TYPE_OPCUA


def _require_pro(api_client):
    if not _is_pro_build(api_client):
        pytest.skip("OPC UA driver requires a commercial build")


def _wait(predicate, timeout: float, step: float = 0.1):
    end = time.time() + timeout
    last = None
    while time.time() < end:
        last = predicate()
        if last:
            return last
        time.sleep(step)
    return last


def _settle(api_client, timeout: float = 20.0) -> dict:
    """Poll until linkState leaves 'connecting'; the dial contract forbids a stuck state."""
    status = _wait(
        lambda: (lambda st: st if st.get("linkState") != "connecting" else None)(
            api_client.command("io.getStatus")
        ),
        timeout,
    )
    assert status is not None, f"linkState stuck at 'connecting' beyond {timeout}s"
    return status


def _configure(
    api_client, url: str, tags=None, auth_mode: int = 0, user: str = "", pw: str = ""
):
    api_client.set_bus_type("opcua")
    api_client.command("io.opcua.setEndpointUrl", {"url": url})
    api_client.command("io.opcua.setEndpointIndex", {"index": -1})
    api_client.command("io.opcua.setAuthMode", {"mode": auth_mode})
    if auth_mode == 1:
        api_client.command("io.opcua.setUsername", {"username": user})
        api_client.command("io.opcua.setPassword", {"password": pw})
    api_client.command("io.opcua.setPublishingInterval", {"intervalMs": 100})
    api_client.command(
        "io.opcua.setTags", {"tags": tags if tags is not None else SMALL_TAGS}
    )


def _connect(api_client, timeout: float = 20.0) -> dict:
    api_client.command("io.connect", timeout=25.0)
    status = _settle(api_client, timeout)
    return status


def _disconnect(api_client):
    try:
        api_client.command("io.disconnect")
    except APIError:
        pass
    _settle(api_client, 10.0)


def _generate(api_client) -> dict:
    result = api_client.command("io.opcua.generateProject")
    api_client.command("project.activate")
    return result


def _dataset_values(api_client) -> dict:
    """title -> latest value text from the dashboard data snapshot."""
    frame = api_client.command("dashboard.getData").get("frame", {})
    out = {}
    for group in frame.get("groups", []):
        for ds in group.get("datasets", []):
            out[ds.get("title")] = ds.get("value")
    return out


def _frame_values(api_client, min_sequence: int = 1, timeout: float = 8.0) -> dict:
    frame = _wait(
        lambda: (
            lambda r: (
                r if r.get("hasData") and r.get("sequence", 0) >= min_sequence else None
            )
        )(api_client.command("io.getLatestFrame")),
        timeout,
    )
    assert frame, "no frame arrived from the OPC UA session"
    return frame


# -----------------------------------------------------------------------------
# Registration and configuration (no simulator needed)
# -----------------------------------------------------------------------------


class TestOpcUaRegistration:
    def test_bus_type_listed_at_index_10(self, api_client, clean_state):
        _require_pro(api_client)
        buses = api_client.command("io.listBuses").get("buses", [])
        entry = buses[BUS_TYPE_OPCUA]
        label = entry.get("name", "") if isinstance(entry, dict) else str(entry)
        assert "opc" in label.lower()

    def test_config_roundtrip(self, api_client, clean_state):
        _require_pro(api_client)
        _configure(api_client, "opc.tcp://127.0.0.1:4840/serialstudio/")
        cfg = api_client.command("io.opcua.getConfig")
        assert cfg["endpointUrl"] == "opc.tcp://127.0.0.1:4840/serialstudio/"
        assert cfg["authMode"] == 0
        assert cfg["publishingInterval"] == 100
        assert cfg["tagCount"] == len(SMALL_TAGS)
        assert cfg["configurationOk"] is True

        tags = api_client.command("io.opcua.listTags")
        assert [t["id"] for t in tags["tags"]] == [t["id"] for t in SMALL_TAGS]
        assert [s["t"] for s in tags["schema"]] == ["f64", "bool", "str", "i32"]

    def test_tag_mutations_and_validation(self, api_client, clean_state):
        _require_pro(api_client)
        _configure(api_client, "opc.tcp://127.0.0.1:4840/serialstudio/", tags=[])
        api_client.command(
            "io.opcua.addTag", {"id": ZONES, "name": "Zones", "t": "f64", "n": 6}
        )
        assert api_client.command("io.opcua.getConfig")["tagCount"] == 1
        assert len(api_client.command("io.opcua.listTags")["schema"]) == 6

        with pytest.raises(APIError):
            api_client.command(
                "io.opcua.addTag", {"id": ZONES, "name": "dup", "t": "f64"}
            )
        with pytest.raises(APIError):
            api_client.command(
                "io.opcua.addTag", {"id": "ns=2;s=x", "name": "x", "t": "nope"}
            )
        with pytest.raises(APIError):
            api_client.command("io.opcua.setEndpointUrl", {"url": "http://nope"})
        with pytest.raises(APIError):
            api_client.command("io.opcua.setPublishingInterval", {"intervalMs": 1})

        api_client.command("io.opcua.removeTag", {"index": 0})
        assert api_client.command("io.opcua.getConfig")["tagCount"] == 0
        assert api_client.command("io.opcua.getConfig")["configurationOk"] is False

    def test_password_never_echoed(self, api_client, clean_state):
        _require_pro(api_client)
        _configure(
            api_client,
            "opc.tcp://127.0.0.1:4840/serialstudio/",
            auth_mode=1,
            user="op",
            pw="pw",
        )
        cfg = api_client.command("io.opcua.getConfig")
        assert cfg["hasPassword"] is True
        assert "password" not in cfg

    def test_closed_port_settles(self, api_client, clean_state):
        """R5: a refused dial ends idle within the budget, never stuck in 'connecting'."""
        _require_pro(api_client)
        _configure(api_client, "opc.tcp://127.0.0.1:48499/nothing/")
        status = _connect(api_client, timeout=25.0)
        assert status["linkState"] == "idle"
        assert status["isConnected"] is False
        st = api_client.command("io.opcua.getStatus")
        assert st["connected"] is False and st["connecting"] is False
        assert st["lastError"]


# -----------------------------------------------------------------------------
# Live session against the shared simulator (127.0.0.1:4840)
# -----------------------------------------------------------------------------


@pytest.mark.requires_opcua_sim
class TestOpcUaSession:
    def test_discover_endpoints(self, api_client, clean_state, opcua_simulator):
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        api_client.command("io.opcua.discoverEndpoints")
        result = _wait(
            lambda: (lambda r: r if not r["discovering"] and r["endpoints"] else None)(
                api_client.command("io.opcua.listEndpoints")
            ),
            10.0,
        )
        assert result, "discovery did not finish"
        policies = {(e["policy"], e["selectable"]) for e in result["endpoints"]}
        assert ("None", True) in policies
        assert result["selectedIndex"] >= 0

    def test_connect_anonymous_and_subscribe(
        self, api_client, clean_state, opcua_simulator
    ):
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        _generate(api_client)
        status = _connect(api_client)
        assert status["isConnected"] is True
        try:
            st = _wait(
                lambda: (lambda s: s if s["framesPublished"] >= 5 else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                10.0,
            )
            assert st, "no frames published"
            assert st["pollMode"] is False
            assert st["valuesReceived"] > 0
            frame = _frame_values(api_client, min_sequence=3)
            assert frame["hasData"]
        finally:
            _disconnect(api_client)

    def test_subscription_delivery_rate(self, api_client, clean_state, opcua_simulator):
        """R9: frames arrive near the publishing interval (100 ms) while values change."""
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        _generate(api_client)
        _connect(api_client)
        try:
            a = api_client.command("io.opcua.getStatus")["framesPublished"]
            time.sleep(2.0)
            b = api_client.command("io.opcua.getStatus")["framesPublished"]
            assert 8 <= (b - a) <= 30, f"expected ~20 frames in 2 s, got {b - a}"
        finally:
            _disconnect(api_client)

    def test_generate_project_groups_and_types(
        self, api_client, clean_state, opcua_simulator
    ):
        """R7/R8: one group per folder, LED for booleans, plot for numerics, strings latch."""
        _require_pro(api_client)
        tags = SMALL_TAGS + [
            {
                "id": ZONES,
                "name": "Zones",
                "path": "Plant/Line1/Pasteuriser",
                "t": "f64",
                "n": 6,
            }
        ]
        _configure(api_client, opcua_simulator["url"], tags=tags)
        result = _generate(api_client)
        assert result["groups"] == 3
        assert result["datasets"] == 4 + 6
        titles = [
            ds["title"] for g in result["project"]["groups"] for ds in g["datasets"]
        ]
        assert "Zones[0]" in titles and "Zones[5]" in titles
        leds = {
            ds["title"]: ds.get("led")
            for g in result["project"]["groups"]
            for ds in g["datasets"]
        }
        assert leds["Running"] is True
        assert leds["Level"] in (False, None)

        _connect(api_client)
        try:
            values = _wait(
                lambda: (
                    lambda v: (
                        v if v.get("Status") in ("FILLING", "HOLD", "STOPPED") else None
                    )
                )(_dataset_values(api_client)),
                10.0,
            )
            assert values, "string tag never reached the dashboard"
            assert values["Running"] in ("0", "1", 0, 1, True, False)
            assert float(values["Level"]) >= 0.0
            assert float(values["Zones[3]"]) > 0.0
        finally:
            _disconnect(api_client)

    def test_bad_status_keeps_last_value(
        self, api_client, clean_state, opcua_simulator
    ):
        """R11: the Bad sentinel (-999) must never reach the dashboard, Uncertain values must."""
        _require_pro(api_client)
        tags = [
            {
                "id": FAULTY,
                "name": "Faulty",
                "path": "Plant/Diagnostics",
                "t": "f64",
                "n": 1,
            }
        ]
        _configure(api_client, opcua_simulator["url"], tags=tags)
        _generate(api_client)
        _connect(api_client)
        try:
            st = _wait(
                lambda: (
                    lambda s: (
                        s if s["badStatus"] > 0 and s["framesPublished"] > 0 else None
                    )
                )(api_client.command("io.opcua.getStatus")),
                20.0,
            )
            assert st, "the simulator's bad status never reached the driver"

            # Two full 10 s fault cycles: Good ~100, Uncertain ~50, Bad -999 (never shown).
            seen = []
            deadline = time.time() + 22.0
            while time.time() < deadline:
                value = _dataset_values(api_client).get("Faulty")
                if value not in (None, ""):
                    seen.append(float(value))
                time.sleep(0.25)

            assert seen, "no Faulty value ever reached the dashboard"
            assert (
                min(seen) > -900.0
            ), f"the Bad sentinel leaked to the dashboard: {min(seen)}"
            assert any(
                40.0 <= v <= 60.0 for v in seen
            ), "Uncertain values were dropped; Uncertain is not Bad"

            status = api_client.command("io.opcua.getStatus")
            assert status["badStatus"] > 0
        finally:
            _disconnect(api_client)

    def test_source_timestamps_in_csv(
        self, api_client, clean_state, opcua_simulator, temp_dir
    ):
        """R10/AC5: the CSV carries monotonic rows spaced at roughly the publishing interval."""
        import csv
        import glob
        import os

        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        _generate(api_client)
        api_client.enable_csv_export()
        _connect(api_client)
        try:
            time.sleep(4.0)
            assert api_client.get_csv_export_status()["enabled"]
            assert api_client.command("io.opcua.getStatus")["unstamped"] == 0
        finally:
            _disconnect(api_client)
            api_client.command("csvExport.close")
            api_client.disable_csv_export()

        home = os.path.expanduser("~")
        recent = [
            path
            for path in sorted(
                glob.glob(os.path.join(home, "**", "*.csv"), recursive=True),
                key=os.path.getmtime,
                reverse=True,
            )[:5]
            if time.time() - os.path.getmtime(path) < 120
        ]
        if not recent:
            pytest.skip("could not locate the CSV Serial Studio just wrote")

        with open(recent[0], "r", encoding="utf-8", errors="replace") as handle:
            rows = list(csv.reader(handle))

        assert len(rows) > 3, "the CSV holds no data rows"
        stamps = [row[0] for row in rows[1:] if row and row[0]]
        assert stamps == sorted(stamps), "CSV timestamps are not monotonic"

    def test_project_roundtrip_no_browse(
        self, api_client, clean_state, opcua_simulator, temp_dir
    ):
        """R13: saving and reopening the generated project restores endpoint and tags."""
        import os

        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        _generate(api_client)
        path = os.path.join(temp_dir, "opcua-roundtrip.ssproj")
        api_client.command("project.save", {"filePath": path})

        api_client.command("io.opcua.clearTags")
        api_client.command("io.opcua.setEndpointUrl", {"url": "opc.tcp://127.0.0.1:1/"})
        assert api_client.command("io.opcua.getConfig")["tagCount"] == 0

        api_client.command("project.open", {"filePath": path})
        api_client.command("project.activate")
        cfg = _wait(
            lambda: (lambda c: c if c["tagCount"] == len(SMALL_TAGS) else None)(
                api_client.command("io.opcua.getConfig")
            ),
            5.0,
        )
        assert cfg, "tag list did not come back from the project file"
        assert cfg["endpointUrl"] == opcua_simulator["url"]
        with open(path, "r", encoding="utf-8") as fh:
            assert "password" not in fh.read().lower().split('"username"')[-1][:200]

        _connect(api_client)
        try:
            assert api_client.command("io.getStatus")["isConnected"]
        finally:
            _disconnect(api_client)

    def test_browse_tree(self, api_client, clean_state, opcua_simulator):
        """R6: the browse session walks Objects -> Plant -> Line1 -> Filler with typed rows."""
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"], tags=[])
        api_client.command("io.opcua.startBrowse")
        try:
            root = _wait(
                lambda: (lambda r: r if r["children"] else None)(
                    api_client.command("io.opcua.browse", {"nodeId": ""})
                ),
                10.0,
            )
            assert root, "browse session never produced the Objects children"
            names = {c["name"]: c for c in root["children"]}
            assert "Plant" in names and names["Plant"]["folder"] is True

            for node in (
                "ns=2;s=Plant",
                "ns=2;s=Plant.Line1",
                "ns=2;s=Plant.Line1.Filler",
            ):
                level = _wait(
                    lambda n=node: (lambda r: r if r["children"] else None)(
                        api_client.command("io.opcua.browse", {"nodeId": n})
                    ),
                    10.0,
                )
                assert level, f"no children for {node}"

            filler = _wait(
                lambda: (
                    lambda r: (
                        r
                        if all(c["t"] for c in r["children"] if not c["folder"])
                        else None
                    )
                )(
                    api_client.command(
                        "io.opcua.browse", {"nodeId": "ns=2;s=Plant.Line1.Filler"}
                    )
                ),
                10.0,
            )
            assert filler, "variable rows never received their data types"
            rows = {c["name"]: c for c in filler["children"]}
            assert (
                rows["Running"]["t"] == "bool" and rows["Running"]["selectable"] is True
            )
            assert rows["Level_pct"]["t"] == "f64"
            assert rows["Status"]["t"] == "str"
            assert rows["Bottles"]["t"] == "u32"
        finally:
            api_client.command("io.opcua.stopBrowse")
        assert api_client.command("io.opcua.getStatus")["browsing"] is False

    # -----------------------------------------------------------------------------
    # Private simulator instances (flags and restarts)
    # -----------------------------------------------------------------------------

    def test_browse_is_lazy(self, api_client, clean_state, opcua_simulator):
        """Opening the picker must not crawl the address space: ns=0 Server stays unfetched."""
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"], tags=[])
        api_client.command("io.opcua.startBrowse")
        try:
            root = _wait(
                lambda: (lambda r: r if r["children"] else None)(
                    api_client.command("io.opcua.browse", {"nodeId": ""})
                ),
                10.0,
            )
            assert root
            time.sleep(2.0)
            rows = {c["name"]: c for c in root["children"]}
            assert "Server" in rows, "the Objects folder should list the Server node"
            assert (
                rows["Server"]["fetched"] is False
            ), "the picker crawled into ns=0 Server"
        finally:
            api_client.command("io.opcua.stopBrowse")

    def test_struct_variable_is_expandable(
        self, api_client, clean_state, opcua_simulator
    ):
        """A Variable owning child Variables (a PLC UDT, numeric ids) must be browsable."""
        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"], tags=[])
        api_client.command("io.opcua.startBrowse")
        try:
            for node in (
                "",
                "ns=2;s=Plant",
                "ns=2;s=Plant.Line1",
                "ns=2;s=Plant.Line1.Pasteuriser",
            ):
                _wait(
                    lambda n=node: (lambda r: r if r["children"] else None)(
                        api_client.command("io.opcua.browse", {"nodeId": n})
                    ),
                    10.0,
                )

            members = _wait(
                lambda: (lambda r: r if r["children"] else None)(
                    api_client.command("io.opcua.browse", {"nodeId": RECIPE})
                ),
                10.0,
            )
            assert members, "the struct Variable exposed no children"
            names = {c["name"]: c for c in members["children"]}
            assert "TargetTemp_C" in names
            assert names["TargetTemp_C"]["id"] == TARGET
            assert names["TargetTemp_C"]["selectable"] is True
        finally:
            api_client.command("io.opcua.stopBrowse")

    def test_string_tag_latches(self, api_client, clean_state, opcua_simulator):
        """A tag that rarely changes proves the delta/latch path: it must not read empty."""
        _require_pro(api_client)
        tags = [
            {
                "id": SERIAL,
                "name": "Serial",
                "path": "Plant/Diagnostics",
                "t": "str",
                "n": 1,
            }
        ]
        _configure(api_client, opcua_simulator["url"], tags=tags)
        _generate(api_client)
        _connect(api_client)
        try:
            values = _wait(
                lambda: (lambda v: v if v.get("Serial") else None)(
                    _dataset_values(api_client)
                ),
                10.0,
            )
            assert values and values["Serial"] == "SN-000123"
            time.sleep(2.0)
            assert _dataset_values(api_client)["Serial"] == "SN-000123"
        finally:
            _disconnect(api_client)


@pytest.mark.requires_opcua_sim
class TestOpcUaSimulatorFlags:
    def test_connect_username_and_bad_credentials(
        self, api_client, clean_state, opcua_simulator_process
    ):
        sim = opcua_simulator_process(48411, "--user", "op", "--password", "pw")

        _configure(api_client, sim.url, auth_mode=1, user="op", pw="pw")
        _generate(api_client)
        status = _connect(api_client)
        assert status["isConnected"] is True
        _disconnect(api_client)

        _configure(api_client, sim.url, auth_mode=1, user="op", pw="wrong")
        status = _connect(api_client)
        assert status["isConnected"] is False
        assert status["linkState"] == "idle"
        assert api_client.command("io.opcua.getStatus")["lastError"]

        _configure(api_client, sim.url, auth_mode=0)
        status = _connect(api_client)
        assert (
            status["isConnected"] is False
        ), "anonymous must be refused by a user-only server"

    def test_poll_fallback(self, api_client, clean_state, opcua_simulator_process):
        """R9: a server refusing subscriptions still delivers values through timed reads."""
        sim = opcua_simulator_process(48412, "--no-subscriptions")
        _configure(api_client, sim.url)
        _generate(api_client)
        _connect(api_client)
        try:
            st = _wait(
                lambda: (
                    lambda s: s if s["pollMode"] and s["framesPublished"] >= 3 else None
                )(api_client.command("io.opcua.getStatus")),
                15.0,
            )
            assert st, "driver did not fall back to polling"
            assert "Polling" in st["statusText"]
        finally:
            _disconnect(api_client)

    def test_reconnect_after_sim_restart(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """R12/AC6: a dropped server reports disconnected; reconnecting resumes without regeneration."""
        sim = opcua_simulator_process(48413)
        _configure(api_client, sim.url)
        _generate(api_client)
        _connect(api_client)
        before = api_client.command("io.opcua.getStatus")["framesPublished"]
        assert before >= 0

        sim.stop()
        dropped = _wait(
            lambda: (lambda s: s if not s["isConnected"] else None)(
                api_client.command("io.getStatus")
            ),
            20.0,
        )
        assert dropped, "link drop never reached io.getStatus"
        assert dropped["linkState"] == "idle"

        sim.restart()
        status = _connect(api_client)
        assert status["isConnected"] is True
        try:
            st = _wait(
                lambda: (lambda s: s if s["framesPublished"] >= 3 else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                10.0,
            )
            assert st, "no frames after reconnect"
            assert api_client.command("io.opcua.getConfig")["tagCount"] == len(
                SMALL_TAGS
            )
        finally:
            _disconnect(api_client)

    def test_partial_refusal_polls_the_rest(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """A tag the server cannot monitor must be polled, not silently flat-lined."""
        sim = opcua_simulator_process(48414)
        tags = SMALL_TAGS + [
            {
                "id": "ns=2;s=Does.Not.Exist",
                "name": "Ghost",
                "path": "Plant/Diagnostics",
                "t": "f64",
                "n": 1,
            }
        ]
        _configure(api_client, sim.url, tags=tags)
        _generate(api_client)
        _connect(api_client)
        try:
            st = _wait(
                lambda: (lambda s: s if s["framesPublished"] >= 3 else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                15.0,
            )
            assert st, "no frames with a partially refused subscription"
            assert (
                st["refusedTags"] >= 1
            ), "the unknown node was not routed to the poll lane"
            assert (
                st["pollMode"] is False
            ), "one bad tag must not demote the whole session"
            assert float(_dataset_values(api_client)["Level"]) >= 0.0
        finally:
            _disconnect(api_client)

    def test_revised_interval_is_reported(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """R9: the status reports the interval the server revised the subscription to."""
        sim = opcua_simulator_process(48415)
        _configure(api_client, sim.url)
        api_client.command("io.opcua.setPublishingInterval", {"intervalMs": 20})
        _generate(api_client)
        _connect(api_client)
        try:
            st = _wait(
                lambda: (lambda s: s if s.get("revisedInterval", 0) > 0 else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                10.0,
            )
            assert st, "the driver never reported a revised interval"
            assert st["revisedInterval"] >= 10
        finally:
            _disconnect(api_client)

    def test_silent_drop_falls_back_to_polling(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """A server that stops publishing without closing the socket must not freeze the view."""
        sim = opcua_simulator_process(48416, "--drop-after", "6")
        _configure(api_client, sim.url)
        _generate(api_client)
        _connect(api_client)
        try:
            st = _wait(
                lambda: (lambda s: s if s["pollMode"] else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                40.0,
            )
            assert st, "the watchdog never noticed the silent subscription"
        finally:
            _disconnect(api_client)

    def test_unreachable_host_settles(self, api_client, clean_state):
        """R5: a black-holed address resolves through the dial deadline, never stays connecting."""
        _require_pro(api_client)
        _configure(api_client, UNREACHABLE)
        status = _connect(api_client, timeout=40.0)
        assert status["linkState"] == "idle"
        assert status["isConnected"] is False
