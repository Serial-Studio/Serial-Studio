"""
Spec 0083 AC4: the register-map importer's generated Lua parser routes each Modbus reply by
the unit, function code and byte count the reply itself carries, never by poll position.

The decode machinery is a raw string literal inside ``ModbusMapImporter.cpp``; this test lifts
that literal verbatim, prepends a ``BLOCKS`` spec shaped the way ``luaEntryLine`` emits it for
``tests/fixtures/modbus/two_units.csv``, and runs the result under ``luajit`` with ``tableSet``
stubbed. A reply sequence with one reply dropped must still land every register in its own
table, an ``rbit`` row must follow its bit, and a ``cdab`` float must decode after the word
swap is undone. The C++ spec emitter is pinned by name so a rename of the fields the parser
matches on is loud here.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import re
import shutil
import struct
import subprocess
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
IMPORTER = (
    ROOT / "core" / "Pipeline" / "DataModel" / "Importers" / "ModbusMapImporter.cpp"
)
FIXTURE = ROOT / "tests" / "fixtures" / "modbus" / "two_units.csv"

LUAJIT = shutil.which("luajit")
pytestmark = pytest.mark.skipif(LUAJIT is None, reason="luajit not on PATH")

PLC_TABLE = "Unit 1 Holding Registers @ 0"
STATUS_TABLE = "Unit 1 Holding Registers @ 100"
METER_TABLE = "Unit 2 Holding Registers @ 0"
SETPOINT_TABLE = "Unit 1 Holding Registers @ 384"

BLOCKS = f"""
local BLOCKS = {{
  {{
    func = 0x03,
    unit = 1,
    bytes = 4,
    table = "{PLC_TABLE}",
    entries = {{
      {{ name = "PLC Temperature", offset = 0, type = "uint16", scale = 0.1 }},
      {{ name = "PLC Pressure", offset = 1, type = "uint16" }},
    }},
  }},
  {{
    func = 0x03,
    unit = 1,
    bytes = 2,
    table = "{STATUS_TABLE}",
    entries = {{
      {{ name = "PLC Status", offset = 0, type = "uint16" }},
      {{ name = "Ready", offset = 0, type = "rbit", bit = 3, width = 2 }},
      {{ name = "Fan", offset = 0, type = "rbit", bit = 5, width = 2, table = "{STATUS_TABLE}", word = "Fan (word)" }},
    }},
  }},
  {{
    func = 0x03,
    unit = 1,
    bytes = 2,
    table = "{SETPOINT_TABLE}",
    entries = {{
      {{ name = "Load Setpoint", offset = 0, type = "uint16" }},
    }},
  }},
  {{
    func = 0x03,
    unit = 2,
    bytes = 8,
    table = "{METER_TABLE}",
    entries = {{
      {{ name = "Meter Power", offset = 0, type = "float32", order = "cdab" }},
      {{ name = "Meter Energy", offset = 2, type = "uint32", scale = 0.001 }},
    }},
  }},
}}
"""

HARNESS = """
local log = {}
function tableSet(table_name, register, value)
  log[#log + 1] = { table_name, register, value }
end
table.unpack = table.unpack or unpack

local frames = FRAMES
for _, frame in ipairs(frames) do
  local bytes = {}
  for i = 1, #frame do bytes[i] = frame[i] end
  parse(bytes)
end

local out = {}
for _, entry in ipairs(log) do
  out[#out + 1] = string.format('["%s","%s",%.10g]', entry[1], entry[2], entry[3])
end
io.write("[" .. table.concat(out, ",") .. "]")
"""


def _parser_body() -> str:
    source = IMPORTER.read_text(encoding="utf-8")
    literals = re.findall(r'R"LUA\((.*?)\)LUA"', source, re.S)
    bodies = [lit for lit in literals if "local SIZES" in lit]
    assert len(bodies) == 1, "the parser body literal moved; update the extractor"
    return bodies[0]


def _reply(unit: int, payload: bytes) -> list[int]:
    """One RTU reply as the driver publishes it: [unit, FC03, byteCount, data..., crc, crc]."""
    frame = bytes([unit, 0x03, len(payload)]) + payload
    return list(frame) + [0x00, 0x00]


def _run(frames: list[list[int]]) -> list[list]:
    lua_frames = (
        "{" + ",".join("{" + ",".join(str(b) for b in f) + "}" for f in frames) + "}"
    )
    script = BLOCKS + _parser_body() + HARNESS.replace("FRAMES", lua_frames)
    result = subprocess.run(
        [LUAJIT, "-e", script], capture_output=True, text=True, timeout=10
    )
    assert result.returncode == 0, result.stderr
    return json.loads(result.stdout)


def _writes(rows: list[list], table: str) -> dict:
    return {register: value for t, register, value in rows if t == table}


def test_fixture_and_emitter_still_agree():
    """The fixture carries the four spec-0083 columns and the C++ emits the fields matched on."""
    header = FIXTURE.read_text(encoding="utf-8").splitlines()[1]
    assert (
        header
        == "address,name,type,dataType,units,min,max,scale,offset,slave,bit,rw,order"
    )

    source = IMPORTER.read_text(encoding="utf-8")
    for field in ("unit = %2", "bytes = %3", 'QStringLiteral("rbit")', ", order = %1"):
        assert field in source, f"luaEntryLine no longer emits {field!r}"


def test_two_units_route_by_unit_and_one_dropped_reply_does_not_shift():
    """Replies from units 1 and 2 land in their own tables even when unit 2's reply is dropped."""
    plc = _reply(1, struct.pack(">HH", 235, 1200))
    status = _reply(1, struct.pack(">H", 0b0010_1000))
    setpoint = _reply(1, struct.pack(">H", 42))
    meter_words = struct.pack(">f", 123.5)
    meter = _reply(2, meter_words[2:4] + meter_words[0:2] + struct.pack(">I", 987654))

    rows = _run(
        [
            plc,
            status,
            setpoint,
            meter,
            plc,
            status,
            setpoint,
            plc,
            status,
            setpoint,
            meter,
        ]
    )

    plc_values = _writes(rows, PLC_TABLE)
    assert plc_values["PLC Temperature"] == pytest.approx(23.5)
    assert plc_values["PLC Pressure"] == 1200

    assert _writes(rows, SETPOINT_TABLE)["Load Setpoint"] == 42

    meter_values = _writes(rows, METER_TABLE)
    assert meter_values["Meter Power"] == pytest.approx(123.5)
    assert meter_values["Meter Energy"] == pytest.approx(987.654)

    tables_in_order = [t for t, _, _ in rows]
    assert tables_in_order.count(METER_TABLE) == 2 * 2
    assert tables_in_order.count(SETPOINT_TABLE) == 3


def test_register_bit_rows_follow_their_bit_and_publish_the_word():
    """An rbit row reads one bit of the word; a writable one also publishes the raw word."""
    rows = _run([_reply(1, struct.pack(">H", 0b0010_1000))])

    status = _writes(rows, STATUS_TABLE)
    assert status["PLC Status"] == 40
    assert status["Ready"] == 1
    assert status["Fan"] == 1
    assert status["Fan (word)"] == 40

    rows = _run([_reply(1, struct.pack(">H", 0b0000_1000))])
    status = _writes(rows, STATUS_TABLE)
    assert status["Ready"] == 1
    assert status["Fan"] == 0


def test_exception_and_unknown_replies_are_ignored():
    """An exception response or a reply no block matches publishes nothing."""
    exception = [1, 0x83, 0x02, 0x00, 0x00]
    unknown_unit = _reply(9, struct.pack(">HH", 1, 2))
    wrong_length = _reply(1, struct.pack(">HHH", 1, 2, 3))
    assert _run([exception, unknown_unit, wrong_length]) == []
