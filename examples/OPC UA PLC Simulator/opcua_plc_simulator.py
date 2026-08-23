#!/usr/bin/env python3
"""
Bottling Line Simulator - OPC UA Server
=======================================

Physics-flavoured simulation for Serial Studio demonstration and for the
OPC UA driver's integration tests:
- Filler with PID-controlled level and a conveyor speed ramp
- Capper with torque feedback and a jam counter
- Pasteuriser tunnel with a first-order thermal model
- Utilities: compressed air, chilled water, mains power
- A deliberately "bad" sensor cycling Good -> Bad (sentinel -999) -> Uncertain, so a client
  is caught both dropping Bad values and wrongly dropping Uncertain ones
- A float array (zone temperatures) and string status tags

OPC UA Server: opc.tcp://0.0.0.0:4840/serialstudio/  (policy None)

Address space (Objects folder; node ids are strings, e.g. ns=2;s=Plant.Line1.Filler.Level_pct):
  Plant/
    Line1/
      Filler/       Running(bool) Level_pct(f64) Speed_bpm(f64) Bottles(u32) Status(str)
      Capper/       Running(bool) Torque_Nm(f32) Jams(u16) Status(str)
      Pasteuriser/  Inlet_C(f64) Outlet_C(f64) Zones_C(f64[6]) Heater_pct(u8)
    Utilities/      Air_bar(f64) ChilledWater_C(f64) Power_kW(f64) Frequency_Hz(f32)
    Diagnostics/    Uptime_s(i64) Cycle(i32) Mood(i8) State(i32) StartedAt(DateTime)
                    SerialNumber(str, rarely changes: proves the delta/latch path)
                    FaultySensor(f64): Good 0-5 s, Bad -999 5-7.5 s, Uncertain 7.5-10 s
    Line1/Pasteuriser/Recipe (ns=2;i=5000, a Variable owning child Variables like a PLC UDT):
                    TargetTemp_C(f64, i=5001) HoldTime_s(u16, i=5002)
  Numeric node ids, EngineeringUnits/EURange properties and a DateTime are present so a client
  is exercised the way a real PLC or gateway exercises it.

Usage: python opcua_plc_simulator.py [--port 4840] [--user NAME --password PW]
                                     [--no-subscriptions] [--secure-only] [--rate 10]
Requires: pip install asyncua
"""

import argparse
import asyncio
import logging
import math
import random
import sys
import time

for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8")
    except (AttributeError, ValueError):
        pass

import datetime

from asyncua import Server, ua
from asyncua.crypto.permission_rules import User, UserRole

logging.basicConfig(
    level=logging.INFO, format="%(asctime)s %(message)s", datefmt="%H:%M:%S"
)
log = logging.getLogger("opcua-sim")
logging.getLogger("asyncua").setLevel(logging.WARNING)

ENDPOINT_PATH = "/serialstudio/"
NAMESPACE = "urn:serial-studio:bottling-line"


# =========================================================================
# Plant model
# =========================================================================


class BottlingLine:
    """Minimal but coupled plant dynamics so every tag moves for a reason."""

    def __init__(self):
        self.t0 = time.monotonic()
        self.cycle = 0
        self.running = True
        self.level = 45.0
        self.level_target = 60.0
        self.speed = 0.0
        self.speed_target = 300.0
        self.bottles = 0
        self.torque = 0.0
        self.jams = 0
        self.inlet = 22.0
        self.outlet = 22.0
        self.zones = [22.0] * 6
        self.heater = 0
        self.air = 6.5
        self.chilled = 6.0
        self.power = 12.0
        self.frequency = 50.0
        self.mood = 0
        self.integral = 0.0
        self.fault_phase = 0.0

    def step(self, dt: float):
        self.cycle += 1
        now = time.monotonic() - self.t0

        # Conveyor soft-start toward target, occasional stop/start cycles.
        if int(now) % 90 > 80:
            self.running = False
        else:
            self.running = True
        target = self.speed_target if self.running else 0.0
        self.speed += (target - self.speed) * min(1.0, dt * 0.4)

        # Filler level PID on a wandering setpoint with bottle draw-down.
        self.level_target = 60.0 + 10.0 * math.sin(now / 37.0)
        err = self.level_target - self.level
        self.integral = max(-50.0, min(50.0, self.integral + err * dt))
        fill = 2.0 * err + 0.3 * self.integral
        drain = self.speed / 300.0 * 8.0
        self.level = max(0.0, min(100.0, self.level + (fill - drain) * dt * 0.1))
        self.bottles += int(self.speed / 60.0 * dt + random.random())

        # Capper torque follows speed with jitter; jams when torque spikes.
        self.torque = 1.8 + self.speed / 300.0 * 0.6 + random.gauss(0, 0.05)
        if self.running and random.random() < 0.002:
            self.torque += 1.5
            self.jams += 1

        # Pasteuriser: heater duty chases 62 C at the outlet, zones lag inlet.
        self.heater = int(max(0, min(100, (62.0 - self.outlet) * 12)))
        self.inlet += (20.0 + 0.45 * self.heater - self.inlet) * dt * 0.05
        for i in range(6):
            prev = self.inlet if i == 0 else self.zones[i - 1]
            self.zones[i] += (prev - self.zones[i]) * dt * 0.3
        self.outlet = self.zones[-1]

        # Utilities wander slowly.
        self.air = 6.5 + 0.3 * math.sin(now / 11.0) + random.gauss(0, 0.01)
        self.chilled = 6.0 + 0.5 * math.sin(now / 53.0)
        self.power = 10.0 + self.speed / 300.0 * 18.0 + self.heater * 0.12
        self.frequency = 50.0 + random.gauss(0, 0.01)
        self.mood = int(5 * math.sin(now / 7.0))
        self.fault_phase = now % 10.0

    @property
    def faulty_sensor_bad(self) -> bool:
        return 5.0 <= self.fault_phase < 7.5

    @property
    def faulty_sensor_uncertain(self) -> bool:
        return self.fault_phase >= 7.5

    @property
    def filler_status(self) -> str:
        if not self.running:
            return "STOPPED"
        return "FILLING" if self.level < self.level_target else "HOLD"

    @property
    def capper_status(self) -> str:
        return "JAM" if self.torque > 3.0 else ("RUN" if self.running else "IDLE")


# =========================================================================
# Server
# =========================================================================


async def build_address_space(server: Server, idx: int):
    objects = server.nodes.objects

    async def folder(parent, path):
        return await parent.add_folder(ua.NodeId(path, idx), path.rsplit(".", 1)[-1])

    plant = await folder(objects, "Plant")
    line = await folder(plant, "Plant.Line1")
    filler = await folder(line, "Plant.Line1.Filler")
    capper = await folder(line, "Plant.Line1.Capper")
    past = await folder(line, "Plant.Line1.Pasteuriser")
    util = await folder(plant, "Plant.Utilities")
    diag = await folder(plant, "Plant.Diagnostics")

    async def var(parent, name, value, vtype, numeric=None, eu=None, rng=None):
        # String node ids ("ns=2;s=Plant.Line1.Filler.Level_pct") keep project files and tests
        # stable across restarts. One branch uses numeric ids on purpose: that is what Siemens,
        # Beckhoff and most embedded servers hand out.
        node_id = (
            ua.NodeId(numeric, idx)
            if numeric is not None
            else ua.NodeId(parent.nodeid.Identifier + "." + name, idx)
        )
        node = await parent.add_variable(node_id, name, ua.Variant(value, vtype))
        await node.set_writable(False)

        def prop_id(suffix, offset):
            if numeric is not None:
                return ua.NodeId(numeric + offset, idx)
            return ua.NodeId(f"{node_id.Identifier}.{suffix}", idx)

        if eu is not None:
            prop = await node.add_property(
                prop_id("EngineeringUnits", 900),
                "EngineeringUnits",
                ua.EUInformation(
                    NamespaceUri="http://www.opcfoundation.org/UA/units/un/cefact",
                    UnitId=0,
                    DisplayName=ua.LocalizedText(eu),
                    Description=ua.LocalizedText(eu),
                ),
            )
            await prop.set_writable(False)

        if rng is not None:
            prop = await node.add_property(
                prop_id("EURange", 950), "EURange", ua.Range(Low=rng[0], High=rng[1])
            )
            await prop.set_writable(False)

        return node

    tags = {
        "filler.running": await var(filler, "Running", True, ua.VariantType.Boolean),
        "filler.level": await var(
            filler, "Level_pct", 45.0, ua.VariantType.Double, eu="%", rng=(0.0, 100.0)
        ),
        "filler.speed": await var(
            filler, "Speed_bpm", 0.0, ua.VariantType.Double, eu="bpm", rng=(0.0, 400.0)
        ),
        "filler.bottles": await var(filler, "Bottles", 0, ua.VariantType.UInt32),
        "filler.status": await var(filler, "Status", "STOPPED", ua.VariantType.String),
        "capper.running": await var(capper, "Running", True, ua.VariantType.Boolean),
        "capper.torque": await var(capper, "Torque_Nm", 0.0, ua.VariantType.Float),
        "capper.jams": await var(capper, "Jams", 0, ua.VariantType.UInt16),
        "capper.status": await var(capper, "Status", "IDLE", ua.VariantType.String),
        "past.inlet": await var(past, "Inlet_C", 22.0, ua.VariantType.Double),
        "past.outlet": await var(past, "Outlet_C", 22.0, ua.VariantType.Double),
        "past.zones": await var(past, "Zones_C", [22.0] * 6, ua.VariantType.Double),
        "past.heater": await var(past, "Heater_pct", 0, ua.VariantType.Byte, eu="%"),
        "recipe.name": await var(
            past, "Recipe", "PASTEUR-A", ua.VariantType.String, numeric=5000
        ),
        "util.air": await var(util, "Air_bar", 6.5, ua.VariantType.Double),
        "util.chilled": await var(util, "ChilledWater_C", 6.0, ua.VariantType.Double),
        "util.power": await var(util, "Power_kW", 12.0, ua.VariantType.Double),
        "util.freq": await var(util, "Frequency_Hz", 50.0, ua.VariantType.Float),
        "diag.uptime": await var(diag, "Uptime_s", 0, ua.VariantType.Int64),
        "diag.cycle": await var(diag, "Cycle", 0, ua.VariantType.Int32),
        "diag.faulty": await var(diag, "FaultySensor", 0.0, ua.VariantType.Double),
        "diag.mood": await var(diag, "Mood", 0, ua.VariantType.SByte),
    }
    # A Variable that owns child Variables, the way a PLC exposes a struct/UDT.
    recipe = tags["recipe.name"]
    tags["recipe.target"] = await var(
        recipe,
        "TargetTemp_C",
        62.0,
        ua.VariantType.Double,
        numeric=5001,
        eu="degC",
        rng=(0.0, 100.0),
    )
    tags["recipe.hold"] = await var(
        recipe, "HoldTime_s", 30, ua.VariantType.UInt16, numeric=5002, eu="s"
    )

    tags["diag.state"] = await var(diag, "State", 0, ua.VariantType.Int32)
    tags["diag.started"] = await var(
        diag,
        "StartedAt",
        datetime.datetime.now(datetime.timezone.utc),
        ua.VariantType.DateTime,
    )
    tags["diag.serial"] = await var(
        diag, "SerialNumber", "SN-000123", ua.VariantType.String
    )
    return tags


async def publish(server: Server, tags, sim: BottlingLine):
    """Writes every tag with a source timestamp; the faulty sensor alternates Good/Bad."""
    now = datetime.datetime.now(datetime.timezone.utc)
    values = {
        "filler.running": (sim.running, ua.VariantType.Boolean),
        "filler.level": (round(sim.level, 3), ua.VariantType.Double),
        "filler.speed": (round(sim.speed, 2), ua.VariantType.Double),
        "filler.bottles": (sim.bottles, ua.VariantType.UInt32),
        "filler.status": (sim.filler_status, ua.VariantType.String),
        "capper.running": (sim.running, ua.VariantType.Boolean),
        "capper.torque": (round(sim.torque, 3), ua.VariantType.Float),
        "capper.jams": (sim.jams, ua.VariantType.UInt16),
        "capper.status": (sim.capper_status, ua.VariantType.String),
        "past.inlet": (round(sim.inlet, 2), ua.VariantType.Double),
        "past.outlet": (round(sim.outlet, 2), ua.VariantType.Double),
        "past.zones": ([round(z, 2) for z in sim.zones], ua.VariantType.Double),
        "past.heater": (sim.heater, ua.VariantType.Byte),
        "util.air": (round(sim.air, 3), ua.VariantType.Double),
        "util.chilled": (round(sim.chilled, 2), ua.VariantType.Double),
        "util.power": (round(sim.power, 2), ua.VariantType.Double),
        "util.freq": (round(sim.frequency, 3), ua.VariantType.Float),
        "diag.uptime": (int(time.monotonic() - sim.t0), ua.VariantType.Int64),
        "diag.cycle": (sim.cycle, ua.VariantType.Int32),
        "diag.mood": (sim.mood, ua.VariantType.SByte),
        "diag.state": (sim.cycle % 4, ua.VariantType.Int32),
        "recipe.target": (62.0, ua.VariantType.Double),
        "recipe.hold": (30, ua.VariantType.UInt16),
    }
    for key, (value, vtype) in values.items():
        dv = ua.DataValue(ua.Variant(value, vtype), SourceTimestamp=now)
        await server.write_attribute_value(tags[key].nodeid, dv)

    # The Bad phase writes a sentinel a correct client must never show; the Uncertain phase
    # writes a real value it MUST show, because Uncertain is not Bad.
    if sim.faulty_sensor_bad:
        status, value = ua.StatusCode(ua.StatusCodes.BadSensorFailure), -999.0
    elif sim.faulty_sensor_uncertain:
        status = ua.StatusCode(ua.StatusCodes.UncertainLastUsableValue)
        value = round(50.0 + 5.0 * math.sin(sim.cycle / 20.0), 3)
    else:
        status = ua.StatusCode(ua.StatusCodes.Good)
        value = round(100.0 + 5.0 * math.sin(sim.cycle / 20.0), 3)

    dv = ua.DataValue(
        ua.Variant(value, ua.VariantType.Double),
        StatusCode=status,
        SourceTimestamp=now,
    )
    await server.write_attribute_value(tags["diag.faulty"].nodeid, dv)


class SimpleUserManager:
    """asyncua user manager: one configured user, or anonymous when none is set."""

    def __init__(self, username, password):
        self.username = username
        self.password = password

    def get_user(self, iserver, username=None, password=None, certificate=None):
        if self.username is None:
            return User(role=UserRole.User)
        if username == self.username and password == self.password:
            return User(role=UserRole.User)
        return None


async def run(args):
    server = Server(user_manager=SimpleUserManager(args.user, args.password))
    await server.init()
    server.set_endpoint(f"opc.tcp://{args.host}:{args.port}{ENDPOINT_PATH}")
    server.set_server_name("Serial Studio Bottling Line Simulator")

    if args.secure_only:
        server.set_security_policy(
            [ua.SecurityPolicyType.Basic256Sha256_SignAndEncrypt]
        )
        await server.load_certificate(args.cert)
        await server.load_private_key(args.key)
    else:
        server.set_security_policy([ua.SecurityPolicyType.NoSecurity])

    if args.user is None:
        server.set_identity_tokens([ua.AnonymousIdentityToken])
    else:
        server.set_identity_tokens([ua.UserNameIdentityToken])

    if args.no_subscriptions:
        # Refuse every CreateSubscription (BadTooManySubscriptions) so clients fall back to polling.
        server.iserver.max_subscriptions = 0

    idx = await server.register_namespace(NAMESPACE)
    tags = await build_address_space(server, idx)
    sim = BottlingLine()

    dt = 1.0 / args.rate
    async with server:
        log.info(
            "OPC UA server ready at opc.tcp://%s:%d%s",
            args.host,
            args.port,
            ENDPOINT_PATH,
        )
        log.info(
            "auth: %s | subscriptions: %s | rate: %.1f Hz",
            "anonymous" if args.user is None else f"user '{args.user}'",
            "refused" if args.no_subscriptions else "enabled",
            args.rate,
        )
        started = time.monotonic()
        while True:
            if args.drop_after > 0 and time.monotonic() - started > args.drop_after:
                log.info("silent drop: no further publishes")
                await asyncio.sleep(3600)

            sim.step(dt)
            await publish(server, tags, sim)
            await asyncio.sleep(dt)


def main():
    parser = argparse.ArgumentParser(
        description="Serial Studio OPC UA bottling-line simulator"
    )
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=4840)
    parser.add_argument("--rate", type=float, default=10.0, help="update rate in Hz")
    parser.add_argument(
        "--user", default=None, help="require this username (with --password)"
    )
    parser.add_argument("--password", default="", help="password for --user")
    parser.add_argument(
        "--no-subscriptions",
        action="store_true",
        help="refuse CreateSubscription (poll fallback test)",
    )
    parser.add_argument(
        "--secure-only",
        action="store_true",
        help="advertise only a Basic256Sha256 endpoint (needs --cert/--key)",
    )
    parser.add_argument("--cert", default="server_cert.der")
    parser.add_argument("--key", default="server_key.pem")
    parser.add_argument(
        "--drop-after",
        type=float,
        default=0.0,
        help="stop publishing after N seconds without closing the socket (silent-drop test)",
    )
    args = parser.parse_args()

    try:
        asyncio.run(run(args))
    except KeyboardInterrupt:
        log.info("stopped")


if __name__ == "__main__":
    main()
