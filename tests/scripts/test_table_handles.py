"""
Contract tests for the data-table handle API (tableHandle / tableHandleMany /
tableGetH / tableSetH).

The host bridge is not available in Node.js, so these exercise the API contract
against the TABLE_API_SHIM model of the C++ DataTableStore (see conftest.py):
generation-tagged handles, the computed-register write guard, the -1 sentinel,
and safe no-ops for stale/invalid handles. End-to-end correctness against the
real store and Lua parity are covered by tests/integration/test_table_handles.py.
"""

import json

from .conftest import TABLE_API_SHIM, run_js


def _run(setup_js: str):
    """Run the shim + a setup snippet that console.logs a JSON result object."""
    return run_js(TABLE_API_SHIM + "\n" + setup_js)


# ---------------------------------------------------------------------------
# AC1 — handle and name accessors agree (R6)
# ---------------------------------------------------------------------------


class TestHandleNameAgreement:
    def test_write_by_handle_read_by_name(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 0);
            var h = tableHandle('DAQ', 'ch1');
            tableSetH(h, 42);
            console.log(JSON.stringify({ byName: tableGet('DAQ', 'ch1'), handle: h }));
            """)
        assert result["byName"] == 42
        assert result["handle"] > 0

    def test_write_by_name_read_by_handle(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 0);
            var h = tableHandle('DAQ', 'ch1');
            tableSet('DAQ', 'ch1', 7);
            console.log(JSON.stringify({ byHandle: tableGetH(h) }));
            """)
        assert result["byHandle"] == 7

    def test_string_value_round_trips(self):
        result = _run("""
            __defineRegister('Meta', 'label', true, '');
            var h = tableHandle('Meta', 'label');
            tableSetH(h, 'ok');
            console.log(JSON.stringify({ byName: tableGet('Meta', 'label'),
                                         byHandle: tableGetH(h) }));
            """)
        assert result["byName"] == "ok"
        assert result["byHandle"] == "ok"


# ---------------------------------------------------------------------------
# AC2 — computed-register write guard (R4)
# ---------------------------------------------------------------------------


class TestWriteGuard:
    def test_non_computed_handle_write_ignored(self):
        result = _run("""
            __defineRegister('K', 'constant', false, 99);
            var h = tableHandle('K', 'constant');
            tableSetH(h, 1234);
            console.log(JSON.stringify({ value: tableGetH(h) }));
            """)
        assert result["value"] == 99

    def test_guard_matches_name_path(self):
        result = _run("""
            __defineRegister('K', 'constant', false, 5);
            var h = tableHandle('K', 'constant');
            tableSet('K', 'constant', 11);
            tableSetH(h, 22);
            console.log(JSON.stringify({ value: tableGet('K', 'constant') }));
            """)
        assert result["value"] == 5


# ---------------------------------------------------------------------------
# AC3 — invalid / stale handles are safe no-ops (R5, R7)
# ---------------------------------------------------------------------------


class TestInvalidHandleSafety:
    def test_resolve_miss_returns_sentinel(self):
        result = _run("""
            console.log(JSON.stringify({ h: tableHandle('Nope', 'missing') }));
            """)
        assert result["h"] == -1

    def test_read_write_with_sentinel_is_noop(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 3);
            tableSetH(-1, 999);
            console.log(JSON.stringify({ read: tableGetH(-1) === undefined ? null : tableGetH(-1),
                                         untouched: tableGet('DAQ', 'ch1') }));
            """)
        assert result["read"] is None
        assert result["untouched"] == 3

    def test_out_of_range_handle_is_noop(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 0);
            var bogus = tableHandle('DAQ', 'ch1') + 99999;
            tableSetH(bogus, 5);
            console.log(JSON.stringify({ read: tableGetH(bogus) === undefined ? null : 0,
                                         ch1: tableGet('DAQ', 'ch1') }));
            """)
        assert result["read"] is None
        assert result["ch1"] == 0

    def test_stale_handle_after_rebuild_is_noop(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 1);
            var h = tableHandle('DAQ', 'ch1');
            __bumpGeneration();
            tableSetH(h, 777);
            console.log(JSON.stringify({ stale: tableGetH(h) === undefined ? null : 0,
                                         unchanged: tableGet('DAQ', 'ch1') }));
            """)
        assert result["stale"] is None
        assert result["unchanged"] == 1


# ---------------------------------------------------------------------------
# Batch resolve (Q2)
# ---------------------------------------------------------------------------


class TestBatchResolve:
    def test_handle_many_one_per_name(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 0);
            __defineRegister('DAQ', 'ch2', true, 0);
            __defineRegister('DAQ', 'ch3', true, 0);
            var H = tableHandleMany('DAQ', ['ch1', 'ch2', 'ch3']);
            for (var i = 0; i < H.length; i++) tableSetH(H[i], (i + 1) * 10);
            console.log(JSON.stringify({ n: H.length,
                                         vals: [tableGet('DAQ','ch1'),
                                                tableGet('DAQ','ch2'),
                                                tableGet('DAQ','ch3')] }));
            """)
        assert result["n"] == 3
        assert result["vals"] == [10, 20, 30]

    def test_handle_many_unknown_is_sentinel(self):
        result = _run("""
            __defineRegister('DAQ', 'ch1', true, 0);
            var H = tableHandleMany('DAQ', ['ch1', 'nope']);
            console.log(JSON.stringify({ ok: H[0] > 0, miss: H[1] }));
            """)
        assert result["ok"] is True
        assert result["miss"] == -1


# ---------------------------------------------------------------------------
# BADAQ-shaped equivalence — handle parser == name parser
# ---------------------------------------------------------------------------


class TestBadaqEquivalence:
    def test_handle_and_name_versions_agree(self):
        # Eight channels written every frame; the handle version must leave the
        # store identical to the name version for the same input.
        result = _run("""
            var CH = ['ch1','ch2','ch3','ch4','ch5','ch6','ch7','ch8'];
            for (var i = 0; i < CH.length; i++) {
              __defineRegister('A', CH[i], true, 0);
              __defineRegister('B', CH[i], true, 0);
            }
            var frame = [11, 22, 33, 44, 55, 66, 77, 88];

            // Name version into table A.
            for (var i = 0; i < CH.length; i++) tableSet('A', CH[i], frame[i]);

            // Handle version into table B (resolve once, write by handle).
            var H = tableHandleMany('B', CH);
            for (var i = 0; i < CH.length; i++) tableSetH(H[i], frame[i]);

            var a = [], b = [];
            for (var i = 0; i < CH.length; i++) {
              a.push(tableGet('A', CH[i]));
              b.push(tableGet('B', CH[i]));
            }
            console.log(JSON.stringify({ a: a, b: b }));
            """)
        assert result["a"] == result["b"]
        assert result["a"] == [11, 22, 33, 44, 55, 66, 77, 88]
