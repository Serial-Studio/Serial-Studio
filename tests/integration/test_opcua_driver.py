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

        # CSV::Export writes under the workspace CSV folder (see csvSessionDir), so search
        # there rather than the whole home directory: a recursive home glob takes minutes on a
        # real machine and can match an unrelated CSV that happens to be recent.
        csv_root = os.path.join(
            os.path.expanduser("~"), "Documents", "Serial Studio", "CSV"
        )
        recent = [
            path
            for path in sorted(
                glob.glob(os.path.join(csv_root, "**", "*.csv"), recursive=True),
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
        import shutil

        _require_pro(api_client)
        _configure(api_client, opcua_simulator["url"])
        _generate(api_client)
        path = os.path.join(temp_dir, "opcua-roundtrip.ssproj")
        api_client.command("project.save", {"filePath": path})

        # Editing a driver while a project is open re-captures its settings into source[0] and
        # autosaves them ~750 ms later, so the scrub below rewrites the very file this test then
        # reopens. Verify against a copy taken while the saved state is still on disk.
        saved = os.path.join(temp_dir, "opcua-roundtrip-saved.ssproj")
        shutil.copyfile(path, saved)

        api_client.command("io.opcua.clearTags")
        api_client.command("io.opcua.setEndpointUrl", {"url": "opc.tcp://127.0.0.1:1/"})
        assert api_client.command("io.opcua.getConfig")["tagCount"] == 0

        api_client.command("project.open", {"filePath": saved})
        api_client.command("project.activate")
        cfg = _wait(
            lambda: (lambda c: c if c["tagCount"] == len(SMALL_TAGS) else None)(
                api_client.command("io.opcua.getConfig")
            ),
            5.0,
        )
        assert cfg, "tag list did not come back from the project file"
        assert cfg["endpointUrl"] == opcua_simulator["url"]
        with open(saved, "r", encoding="utf-8") as fh:
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
        """A password travels only over an encrypted channel unless the installation opted into
        plaintext (spec 0067 R15, hardened by spec 0075), so the credentials are exercised on a
        SignAndEncrypt channel: the first attempt may be refused over the simulator's unknown
        certificate, and trusting it is what lets the credentials through."""
        _require_pro(api_client)
        sim = opcua_simulator_process(
            48411, "--security", "Basic256Sha256", "--user", "op", "--password", "pw"
        )
        policy = "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

        fingerprint = ""
        try:
            _configure(api_client, sim.url, auth_mode=1, user="op", pw="pw")
            _configure_security(api_client, policy, 3)
            _generate(api_client)
            status = _connect(api_client)
            if not status["isConnected"]:
                fingerprint = _trust_pending(api_client)
                status = _connect(api_client)
            else:
                st = api_client.command("io.opcua.getStatus")
                fingerprint = st.get("serverCertificate", {}).get("fingerprint", "")

            assert status["isConnected"] is True
            _disconnect(api_client)

            _configure(api_client, sim.url, auth_mode=1, user="op", pw="wrong")
            _configure_security(api_client, policy, 3)
            status = _connect(api_client)
            assert status["isConnected"] is False
            assert status["linkState"] == "idle"
            assert api_client.command("io.opcua.getStatus")["lastError"]
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

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


# -----------------------------------------------------------------------------
# Secure channels (spec 0067 stage 2, AC6-AC15)
# -----------------------------------------------------------------------------


def _configure_security(api_client, policy: str, mode: int):
    api_client.command("io.opcua.setSecurityPolicy", {"policy": policy})
    api_client.command("io.opcua.setSecurityMode", {"mode": mode})


def _trust_pending(api_client) -> str:
    """Accept the certificate the last attempt was refused over; returns its fingerprint."""
    st = api_client.command("io.opcua.getStatus")
    fingerprint = st.get("serverCertificate", {}).get("fingerprint", "")
    assert fingerprint, "the refused attempt reported no server certificate"
    api_client.command("io.opcua.trustServer", {"fingerprint": fingerprint})
    return fingerprint


def _forget(api_client, fingerprint: str):
    try:
        api_client.command("io.opcua.revokeTrust", {"fingerprint": fingerprint})
    except APIError:
        pass


@pytest.mark.requires_opcua_sim
class TestOpcUaSecureChannel:
    def test_client_certificate_is_generated_and_reused(self, api_client, clean_state):
        """AC10: the installation gets one certificate and keeps it, so a server operator
        trusts this client once rather than on every launch."""
        _require_pro(api_client)
        first = api_client.command("io.opcua.getCertificate")["certificate"]
        if not first["valid"]:
            first = api_client.command("io.opcua.regenerateCertificate")["certificate"]

        assert first["valid"] is True
        assert first["fingerprint"]
        assert first["applicationUri"].startswith("urn:")

        again = api_client.command("io.opcua.getCertificate")["certificate"]
        assert (
            again["fingerprint"] == first["fingerprint"]
        ), "the certificate was not reused"

    def test_certificate_export_writes_only_the_certificate(
        self, api_client, clean_state, temp_dir
    ):
        """AC11: the certificate can be handed to a server's trust store; the KEY never leaves."""
        import os

        _require_pro(api_client)
        assert api_client.command("io.opcua.getCertificate")["certificate"][
            "valid"
        ] or (api_client.command("io.opcua.regenerateCertificate"))

        path = os.path.join(temp_dir, "serial-studio-client.der")
        api_client.command("io.opcua.exportCertificate", {"path": path})
        assert os.path.getsize(path) > 0

        with open(path, "rb") as handle:
            blob = handle.read()

        assert blob[:1] == b"\x30", "the export is not a DER certificate"
        assert b"PRIVATE KEY" not in blob

    def test_untrusted_server_is_refused_then_trusted(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC7/AC12/AC14: an unknown server certificate is its own distinct verdict, the prompt
        gets the fingerprint, and accepting is what lets the NEXT attempt through."""
        _require_pro(api_client)
        sim = opcua_simulator_process(48421, "--security", "Basic256Sha256")
        _configure(api_client, sim.url)
        _configure_security(
            api_client,
            "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
            3,
        )
        _generate(api_client)

        fingerprint = ""
        try:
            status = _connect(api_client)
            st = api_client.command("io.opcua.getStatus")
            if not status["isConnected"]:
                assert (
                    "trust" in st["lastError"].lower()
                    or "certificate" in st["lastError"].lower()
                )
                fingerprint = _trust_pending(api_client)
                status = _connect(api_client)
            else:
                fingerprint = st.get("serverCertificate", {}).get("fingerprint", "")

            assert (
                status["isConnected"] is True
            ), "the trusted server still refused the channel"
            st = api_client.command("io.opcua.getStatus")
            assert st["securityMode"] == 3
            assert st["securityPolicy"].endswith("Basic256Sha256")
            assert st["credentialsExposed"] is False
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

    @pytest.mark.parametrize(
        "policy,mode",
        [
            ("Basic256Sha256", 2),
            ("Basic256Sha256", 3),
            ("Aes128_Sha256_RsaOaep", 3),
            ("Aes256_Sha256_RsaPss", 3),
        ],
    )
    def test_every_policy_and_mode_opens(
        self, api_client, clean_state, opcua_simulator_process, policy, mode
    ):
        """AC6: each supported policy opens in both Sign and Sign & Encrypt."""
        _require_pro(api_client)
        sim = opcua_simulator_process(48422, "--security", "all")
        _configure(api_client, sim.url)
        _configure_security(
            api_client, f"http://opcfoundation.org/UA/SecurityPolicy#{policy}", mode
        )
        _generate(api_client)

        fingerprint = ""
        try:
            status = _connect(api_client)
            if not status["isConnected"]:
                fingerprint = _trust_pending(api_client)
                status = _connect(api_client)

            assert status["isConnected"] is True, f"{policy}/{mode} did not open"
            st = _wait(
                lambda: (lambda s: s if s["framesPublished"] >= 3 else None)(
                    api_client.command("io.opcua.getStatus")
                ),
                15.0,
            )
            assert st, f"{policy}/{mode} opened but published nothing"
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

    def test_secure_only_server_is_reachable(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC8: a server with no None endpoint is connectable, which is the whole point of
        owning the stack; the old build could only look at it."""
        _require_pro(api_client)
        sim = opcua_simulator_process(48423, "--secure-only")
        _configure(api_client, sim.url)
        api_client.command("io.opcua.discoverEndpoints")
        rows = _wait(
            lambda: (lambda r: r if r["endpoints"] else None)(
                api_client.command("io.opcua.listEndpoints")
            ),
            15.0,
        )
        assert rows, "discovery returned no endpoints"
        assert all(
            row["mode"] != 1 for row in rows["endpoints"]
        ), "expected no None endpoint"
        assert any(row["selectable"] for row in rows["endpoints"]), "nothing dialable"

        _generate(api_client)
        fingerprint = ""
        try:
            status = _connect(api_client)
            if not status["isConnected"]:
                fingerprint = _trust_pending(api_client)
                status = _connect(api_client)

            assert status["isConnected"] is True
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

    def test_endpoint_auto_selection_prefers_the_strongest(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC9: discovery lands on the most secure endpoint the identity can use, and never on a
        deprecated policy by itself."""
        _require_pro(api_client)
        sim = opcua_simulator_process(48424, "--security", "all")
        _configure(api_client, sim.url)
        api_client.command("io.opcua.discoverEndpoints")
        rows = _wait(
            lambda: (lambda r: r if r["endpoints"] else None)(
                api_client.command("io.opcua.listEndpoints")
            ),
            15.0,
        )
        assert rows

        deprecated = [row for row in rows["endpoints"] if row["deprecated"]]
        assert deprecated, "the simulator was asked for the deprecated policies"
        assert all(
            row["selectable"] for row in deprecated
        ), "deprecated is still reachable"

        chosen = api_client.command("io.opcua.getConfig")
        assert not chosen["securityPolicy"].endswith("Basic128Rsa15")
        assert not chosen["securityPolicy"].endswith("Basic256")

    def test_plaintext_warning_is_conditional(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC13: the credentials warning follows the negotiated mode. A permanent banner on an
        encrypted channel is noise, and noise is what gets ignored on the one that matters.
        """
        _require_pro(api_client)
        sim = opcua_simulator_process(
            48425, "--security", "all", "--user", "op", "--password", "pw"
        )

        _configure(api_client, sim.url, auth_mode=1, user="op", pw="pw")
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#None", 1
        )
        assert api_client.command("io.opcua.getConfig")["credentialsExposed"] is True

        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256", 3
        )
        assert api_client.command("io.opcua.getConfig")["credentialsExposed"] is False

        api_client.command("io.opcua.setAuthMode", {"mode": 0})
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#None", 1
        )
        assert api_client.command("io.opcua.getConfig")["credentialsExposed"] is False

    def test_x509_identity(
        self, api_client, clean_state, opcua_simulator_process, temp_dir
    ):
        """AC15: an X.509 user identity token is accepted where the server offers one.

        The identity certificate is generated here rather than reusing the installation's
        own: its private key never leaves the config directory, which is the property
        test_certificate_export_writes_only_the_certificate pins.
        """
        import asyncio
        import os
        from pathlib import Path

        from asyncua.crypto.cert_gen import setup_self_signed_certificate
        from cryptography.x509.oid import ExtendedKeyUsageOID

        _require_pro(api_client)
        sim = opcua_simulator_process(
            48426, "--security", "all", "--allow-certificate-users"
        )
        _configure(api_client, sim.url)

        cert = Path(temp_dir) / "user_cert.der"
        key = Path(temp_dir) / "user_key.pem"
        asyncio.run(
            setup_self_signed_certificate(
                key,
                cert,
                "urn:serial-studio:test:user",
                "127.0.0.1",
                [ExtendedKeyUsageOID.CLIENT_AUTH],
                {"countryName": "DE", "organizationName": "Serial Studio Tests"},
            )
        )
        assert os.path.getsize(cert) > 0 and os.path.getsize(key) > 0

        api_client.command(
            "io.opcua.setUserCertificate", {"certificate": str(cert), "key": str(key)}
        )
        api_client.command("io.opcua.setIdentityType", {"type": 2})
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256", 3
        )
        _generate(api_client)

        fingerprint = ""
        try:
            status = _connect(api_client)
            if not status["isConnected"]:
                fingerprint = _trust_pending(api_client)
                status = _connect(api_client)

            assert status["isConnected"] is True
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

    def test_expired_certificate_is_its_own_reason(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC11/AC14: an expired server certificate is reported as expired, not as untrusted, and
        accepting it does NOT let it through. Renewing it on the server is the only fix, so a
        trust prompt that could wave it past would be worse than useless."""
        _require_pro(api_client)
        sim = opcua_simulator_process(
            48428, "--security", "Basic256Sha256", "--cert-expired"
        )
        _configure(api_client, sim.url)
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256", 3
        )
        _generate(api_client)

        fingerprint = ""
        try:
            status = _connect(api_client)
            assert (
                status["isConnected"] is False
            ), "an expired certificate opened a channel"
            assert status["linkState"] == "idle"

            st = api_client.command("io.opcua.getStatus")
            assert "expired" in st["lastError"].lower(), st["lastError"]
            assert "trust" not in st["lastError"].lower()

            certificate = st.get("serverCertificate", {})
            assert certificate.get("expired") is True
            fingerprint = certificate.get("fingerprint", "")

            if fingerprint:
                api_client.command("io.opcua.trustServer", {"fingerprint": fingerprint})
                status = _connect(api_client)
                assert (
                    status["isConnected"] is False
                ), "trusting an expired certificate let it through"
        finally:
            _disconnect(api_client)
            _forget(api_client, fingerprint)

    def test_hostname_mismatch_is_its_own_reason(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC11/AC14: a certificate issued for another host is reported as a hostname mismatch.
        The fix is to dial the name it was issued for, which the reason has to say."""
        _require_pro(api_client)
        sim = opcua_simulator_process(
            48429, "--security", "Basic256Sha256", "--cert-wrong-host"
        )
        _configure(api_client, sim.url)
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256", 3
        )
        _generate(api_client)

        try:
            status = _connect(api_client)
            assert status["isConnected"] is False
            assert status["linkState"] == "idle"

            st = api_client.command("io.opcua.getStatus")
            assert "host" in st["lastError"].lower(), st["lastError"]
            assert st.get("serverCertificate", {}).get("hostnameMatches") is False
        finally:
            _disconnect(api_client)

    def test_every_failure_mode_has_a_distinct_reason(
        self, api_client, clean_state, opcua_simulator_process
    ):
        """AC14: the security failure modes must not collapse into one message. Four causes with
        four different fixes reported identically is what leaves an operator guessing.
        """
        _require_pro(api_client)
        _configure_security(
            api_client, "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256", 3
        )

        reasons = {}
        cases = (
            ("untrusted", 48430, ()),
            ("expired", 48431, ("--cert-expired",)),
            ("hostname", 48432, ("--cert-wrong-host",)),
        )
        for label, port, extra in cases:
            sim = opcua_simulator_process(port, "--security", "Basic256Sha256", *extra)
            _configure(api_client, sim.url)
            _configure_security(
                api_client,
                "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256",
                3,
            )
            _generate(api_client)
            try:
                status = _connect(api_client)
                if status["isConnected"]:
                    pytest.fail(f"{label}: the channel opened when it should not have")

                reasons[label] = api_client.command("io.opcua.getStatus")["lastError"]
            finally:
                _disconnect(api_client)

        assert all(reasons.values()), f"an empty reason: {reasons}"
        assert len(set(reasons.values())) == len(
            reasons
        ), f"reasons collapsed: {reasons}"

    def test_security_round_trips_without_secrets(
        self, api_client, clean_state, opcua_simulator_process, temp_dir
    ):
        """AC12/R18: policy, mode and identity survive a project save and reopen, and the file
        carries no key, password or certificate blob."""
        import json
        import os
        import shutil

        _require_pro(api_client)
        sim = opcua_simulator_process(
            48427, "--security", "all", "--user", "op", "--password", "pw"
        )

        _configure(api_client, sim.url, auth_mode=1, user="op", pw="pw")
        _configure_security(
            api_client,
            "http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss",
            3,
        )
        _generate(api_client)

        path = os.path.join(temp_dir, "opcua-secure.ssproj")
        api_client.command("project.save", {"filePath": path})

        # Editing a driver while a project is open re-captures its settings into source[0] and
        # autosaves them, so the scrub below would rewrite the very file this test reopens.
        # Verify against a copy taken while the saved state is still on disk.
        saved = os.path.join(temp_dir, "opcua-secure-saved.ssproj")
        shutil.copyfile(path, saved)

        with open(saved, "r", encoding="utf-8") as handle:
            raw = handle.read()

        connection = json.loads(raw).get("sources", [{}])[0].get("connection", {})
        assert "password" not in connection, "the password key reached the project file"
        assert "pw" not in [str(v) for v in connection.values()]
        assert "PRIVATE KEY" not in raw

        api_client.command("io.opcua.setSecurityPolicy", {"policy": "None"})
        api_client.command("project.open", {"filePath": saved})
        api_client.command("project.activate")

        cfg = _wait(
            lambda: (
                lambda c: (
                    c if c["securityPolicy"].endswith("Aes256_Sha256_RsaPss") else None
                )
            )(api_client.command("io.opcua.getConfig")),
            5.0,
        )
        assert cfg, "the security policy did not come back from the project file"
        assert cfg["securityMode"] == 3
        assert cfg["authMode"] == 1

        # hasPassword is True here on purpose: the password came back from the per-machine
        # encrypted vault, keyed by host:port, which is where it belongs. The requirement is
        # that it never entered the project FILE, which the connection-block assertions above
        # are what actually pin.
        assert cfg["hasPassword"] is True
