"""
Pytest configuration and fixtures for frame-parser script tests.

Each test runs the parser script in a fresh Node.js subprocess, so stateful
parsers (those that maintain a 'parsedValues' array across calls) always start
from a clean zero-filled state.
"""

import json
import subprocess
from pathlib import Path

import pytest

SCRIPTS_DIR = (
    Path(__file__).parent.parent.parent / "app" / "rcc" / "scripts" / "parser" / "js"
)


def run_parser(script_name: str, frame_input) -> list:
    """
    Execute a parser script's parse() function via Node.js.

    The frame_input is serialised as JSON and injected directly into the
    generated wrapper snippet, so both string and array inputs work without
    any type-specific encoding gymnastics.

    Args:
        script_name: Filename inside app/rcc/scripts/parser/js/ (e.g. "comma_separated.js")
        frame_input: String or list to pass as the frame argument to parse()

    Returns:
        The value returned by parse(), deserialised from JSON.

    Raises:
        RuntimeError: If Node.js exits with a non-zero status.
        json.JSONDecodeError: If the output cannot be parsed.
    """
    script_path = SCRIPTS_DIR / script_name
    script_source = script_path.read_text(encoding="utf-8")

    runner = f"""{script_source}
var _result = parse({json.dumps(frame_input)});
console.log(JSON.stringify(_result));
"""
    # Feed the program over stdin ("node -") instead of "node -e <src>": passing a
    # large script as an argv string trips the Windows 32767-char command-line limit
    # (subprocess surfaces it as "environment variable is longer than 32767").
    #
    # Pin encoding="utf-8": the parser scripts carry non-ASCII characters (e.g. the
    # U+2192 arrow in comments). Without this, text mode decodes/encodes with the
    # locale codec, which is cp1252 on the Windows runners and raises
    # UnicodeEncodeError when the script source crosses the subprocess boundary.
    result = subprocess.run(
        ["node", "-"],
        input=runner,
        capture_output=True,
        text=True,
        encoding="utf-8",
        timeout=30,
    )

    if result.returncode != 0:
        raise RuntimeError(
            f"Node.js exited {result.returncode} for {script_name}:\n{result.stderr}"
        )

    # Take the last non-empty line; earlier lines may be console.log() debug output
    last_line = result.stdout.strip().splitlines()[-1]
    return json.loads(last_line)


@pytest.fixture
def parse_script():
    """Fixture that exposes run_parser() to tests."""
    return run_parser


# ---------------------------------------------------------------------------
# Data-table handle API contract harness
# ---------------------------------------------------------------------------
#
# The host bridge (tableGet/tableSet/tableHandle/...) does not exist in plain
# Node.js, so handle-API tests run against this JS shim, a faithful model of the
# C++ DataTableStore semantics: a flat register store, generation-tagged handles
# (gen << 24 | index), the computed-register write guard, the -1 sentinel, and
# safe no-ops for stale/invalid handles. __defineRegister(table, reg, computed,
# default) sets registers up; __bumpGeneration() simulates a table-def rebuild.

TABLE_API_SHIM = r"""
(function () {
  var REG = {}, ORDER = [], GEN = 1;
  var SHIFT = Math.pow(2, 24);
  function key(t, r) { return t + " " + r; }
  function pack(idx) { return GEN * SHIFT + idx; }
  function unpack(h) {
    if (h < 0) return -1;
    var gen = Math.floor(h / SHIFT), idx = h - gen * SHIFT;
    if (gen !== GEN || idx >= ORDER.length) return -1;
    return idx;
  }
  function read(rec) { return rec.isNumeric ? rec.numeric : rec.str; }
  function write(rec, v) {
    if (typeof v === "string") { rec.str = v; rec.isNumeric = false; }
    else { rec.numeric = v; rec.isNumeric = true; }
  }
  globalThis.__defineRegister = function (t, r, computed, def) {
    var k = key(t, r);
    if (REG[k]) return;
    var rec = { idx: ORDER.length, numeric: 0, str: "", isNumeric: true,
                computed: !!computed };
    if (def !== undefined) write(rec, def);
    REG[k] = rec;
    ORDER.push(k);
  };
  globalThis.__bumpGeneration = function () { GEN += 1; };
  globalThis.tableGet = function (t, r) {
    var rec = REG[key(t, r)];
    return rec ? read(rec) : undefined;
  };
  globalThis.tableSet = function (t, r, v) {
    var rec = REG[key(t, r)];
    if (rec && rec.computed) write(rec, v);
  };
  globalThis.tableHandle = function (t, r) {
    var rec = REG[key(t, r)];
    return rec ? pack(rec.idx) : -1;
  };
  globalThis.tableHandleMany = function (t, regs) {
    var out = [];
    for (var i = 0; i < regs.length; i++) out.push(tableHandle(t, regs[i]));
    return out;
  };
  globalThis.tableGetH = function (h) {
    var idx = unpack(h);
    return idx < 0 ? undefined : read(REG[ORDER[idx]]);
  };
  globalThis.tableSetH = function (h, v) {
    var idx = unpack(h);
    if (idx < 0) return;
    var rec = REG[ORDER[idx]];
    if (rec.computed) write(rec, v);
  };
})();
"""


def run_js(program: str):
    """Run an arbitrary JS program under Node and return its last JSON line."""
    result = subprocess.run(
        ["node", "-"],
        input=program,
        capture_output=True,
        text=True,
        encoding="utf-8",
        timeout=30,
    )

    if result.returncode != 0:
        raise RuntimeError(f"Node.js exited {result.returncode}:\n{result.stderr}")

    last_line = result.stdout.strip().splitlines()[-1]
    return json.loads(last_line)
