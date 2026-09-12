"""
Dataset Transform Integration Tests

Covers per-dataset value transforms (Dataset::transformCode):
 * Lua and JavaScript numeric transforms
 * String in → number out transforms
 * Transform isolation (per-dataset upvalues / IIFE closures)
 * Persistence round-trip through project export/import
 * Watchdog behaviour on infinite loops (Lua instruction hook + JS cross-thread watchdog)

All tests use a TCP loopback device and verify outputs via
dashboard.getData, which returns the post-transform Frame.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import APIError

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

# Passthrough parsers — split comma-separated frames, returning fields as-is.
LUA_PASSTHROUGH = (
    "function parse(frame)\n"
    "  local result = {}\n"
    "  for field in frame:gmatch('([^,]+)') do\n"
    "    result[#result + 1] = field\n"
    "  end\n"
    "  return result\n"
    "end\n"
)

JS_PASSTHROUGH = "function parse(frame) { return frame.split(','); }"


def _setup_project(
    api_client,
    parser_code: str,
    parser_language: int,
    dataset_count: int = 1,
) -> None:
    """
    Build a ProjectFile-mode project with `dataset_count` plot datasets and
    the given parser installed.
    """
    api_client.create_new_project()
    time.sleep(0.2)

    gid = api_client.add_group("G", widget_type=0)
    for _ in range(dataset_count):
        api_client.add_dataset(gid, options=1)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )

    api_client.set_frame_parser_code(parser_code, language=parser_language, source_id=0)
    time.sleep(0.15)


def _set_transform(api_client, group_id: int, dataset_id: int, code: str) -> None:
    api_client.command(
        "project.dataset.setTransformCode",
        {"groupId": group_id, "datasetId": dataset_id, "code": code},
    )
    time.sleep(0.1)


def _send_csv_and_read(
    api_client,
    device_simulator,
    payloads: list[bytes],
    settle_seconds: float = 1.2,
) -> dict:
    """Connect the loopback, stream payloads, return dashboard.getData result."""
    assert api_client.command("project.activate").get("loaded")

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [b"/*" + p + b"*/" for p in payloads]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(settle_seconds)

    data = api_client.get_dashboard_data()
    api_client.disconnect_device()
    time.sleep(0.2)
    return data


def _dataset_value(data: dict, dataset_index: int) -> tuple[float, str]:
    """Extract (numericValue, value) for the Nth dataset in the first group."""
    groups = data.get("frame", {}).get("groups", [])
    assert len(groups) >= 1, "frame has no groups"
    datasets = groups[0].get("datasets", [])
    assert (
        len(datasets) > dataset_index
    ), f"group has {len(datasets)} datasets, need index {dataset_index}"
    d = datasets[dataset_index]
    return d.get("numericValue", 0.0), d.get("value", "")


# ---------------------------------------------------------------------------
# Persistence round-trip (no device required)
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_transform_code_persists_round_trip(api_client, clean_state):
    """Setting transformCode survives a project export → reload cycle."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    lua_xform = "function transform(v) return v * 2 end"
    _set_transform(api_client, 0, 0, lua_xform)

    exported = api_client.command("project.exportJson")
    project_json = exported["config"]

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)

    datasets = api_client.command("project.dataset.list").get("datasets", [])
    assert len(datasets) >= 1
    restored_code = datasets[0].get("transformCode", "")
    assert "transform" in restored_code
    assert restored_code.strip() == lua_xform


@pytest.mark.project
def test_transform_code_clear_on_empty_string(api_client, clean_state):
    """Setting an empty transformCode clears the field."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    _set_transform(api_client, 0, 0, "function transform(v) return v + 1 end")

    datasets = api_client.command("project.dataset.list").get("datasets", [])
    assert datasets[0].get("transformCode", "") != ""

    _set_transform(api_client, 0, 0, "")

    datasets = api_client.command("project.dataset.list").get("datasets", [])
    assert datasets[0].get("transformCode", "") == ""


# ---------------------------------------------------------------------------
# Lua numeric transforms (end-to-end)
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_lua_numeric_transform_end_to_end(api_client, device_simulator, clean_state):
    """A Lua transform doubling the input shows up in dashboard.getData."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)
    _set_transform(api_client, 0, 0, "function transform(v) return v * 2 end")

    data = _send_csv_and_read(
        api_client, device_simulator, [b"21", b"42", b"3.5"], settle_seconds=1.2
    )

    num, _ = _dataset_value(data, 0)
    # Latest frame is 3.5 → transform → 7.0
    assert num == pytest.approx(7.0, rel=1e-6)


@pytest.mark.project
def test_lua_transform_per_dataset_isolation(api_client, device_simulator, clean_state):
    """
    Two datasets in the same source maintain independent Lua upvalues.

    Each dataset's chunk defines `local prev = 0; function transform(v)
    local result = v - prev; prev = v; return result end`. If upvalues
    aren't isolated, the second dataset would reset the first's `prev`.
    """
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=2)

    diff_code = (
        "local prev = 0\n"
        "function transform(v)\n"
        "  local r = v - prev\n"
        "  prev = v\n"
        "  return r\n"
        "end\n"
    )
    _set_transform(api_client, 0, 0, diff_code)
    _set_transform(api_client, 0, 1, diff_code)

    # First frame initialises both prev=0 → outputs (10, 100).
    # Second frame raises both → outputs (5, 50).
    data = _send_csv_and_read(
        api_client, device_simulator, [b"10,100", b"15,150"], settle_seconds=1.4
    )

    v0, _ = _dataset_value(data, 0)
    v1, _ = _dataset_value(data, 1)
    assert v0 == pytest.approx(5.0)
    assert v1 == pytest.approx(50.0)


@pytest.mark.project
def test_lua_transform_returns_string(api_client, device_simulator, clean_state):
    """Lua transforms that return strings propagate into dataset.value."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    label_code = (
        "function transform(v)\n"
        "  if v > 50 then return 'high' else return 'low' end\n"
        "end\n"
    )
    _set_transform(api_client, 0, 0, label_code)

    data = _send_csv_and_read(
        api_client, device_simulator, [b"25", b"75"], settle_seconds=1.2
    )

    # Last frame → 75 → 'high'
    _, value = _dataset_value(data, 0)
    assert value == "high"


# ---------------------------------------------------------------------------
# JavaScript numeric transforms
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_js_numeric_transform_end_to_end(api_client, device_simulator, clean_state):
    """A JS transform squaring the input reaches dashboard.getData."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)
    _set_transform(api_client, 0, 0, "function transform(v) { return v * v; }")

    data = _send_csv_and_read(
        api_client, device_simulator, [b"3", b"4", b"5"], settle_seconds=1.2
    )

    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(25.0)


@pytest.mark.project
def test_js_transform_iife_isolation(api_client, device_simulator, clean_state):
    """
    Two JS datasets using the same `var state = 0` pattern must not clobber
    each other — the FrameBuilder wraps user code in an IIFE at compile time
    so top-level `var` declarations become closure-scoped per dataset.
    """
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=2)

    ema_code = (
        "var alpha = 0.5;\n"
        "var ema = null;\n"
        "function transform(v) {\n"
        "  ema = (ema === null) ? v : alpha * v + (1 - alpha) * ema;\n"
        "  return ema;\n"
        "}"
    )
    _set_transform(api_client, 0, 0, ema_code)
    _set_transform(api_client, 0, 1, ema_code)

    # First frame initialises each ema independently.
    # 10 → ema0 = 10.0; 100 → ema1 = 100.0
    # Second frame 20,200 → ema0 = 0.5*20 + 0.5*10 = 15.0; ema1 = 150.0
    data = _send_csv_and_read(
        api_client, device_simulator, [b"10,100", b"20,200"], settle_seconds=1.4
    )

    v0, _ = _dataset_value(data, 0)
    v1, _ = _dataset_value(data, 1)
    assert v0 == pytest.approx(15.0)
    assert v1 == pytest.approx(150.0)


# ---------------------------------------------------------------------------
# Error paths — malformed transforms fall through to raw value
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_lua_transform_runtime_error_falls_back(
    api_client, device_simulator, clean_state
):
    """A Lua transform that errors at runtime must return the raw value."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    # error() always throws at runtime
    _set_transform(api_client, 0, 0, "function transform(v) error('boom') end")

    data = _send_csv_and_read(api_client, device_simulator, [b"42"], settle_seconds=1.0)

    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(42.0)  # untransformed


@pytest.mark.project
def test_lua_transform_non_numeric_return_falls_back(
    api_client, device_simulator, clean_state
):
    """A Lua transform returning nil falls back to raw."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    _set_transform(api_client, 0, 0, "function transform(v) return nil end")

    data = _send_csv_and_read(api_client, device_simulator, [b"17"], settle_seconds=1.0)

    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(17.0)


# ---------------------------------------------------------------------------
# Watchdog — a runaway transform must be interrupted, never hang the app
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_js_transform_infinite_loop_does_not_hang(
    api_client, device_simulator, clean_state
):
    """A JS transform stuck in `while(true){}` is interrupted per frame so the
    dataset falls back to its raw value and the app stays responsive.

    Before the cross-thread JsWatchdogThread, the same-thread QTimer could not
    fire while the engine blocked the event loop, so this hung the main thread
    on the first frame. pytest-timeout (see pytest.ini) turns a regression here
    into a fast failure instead of a frozen run.
    """
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)
    _set_transform(api_client, 0, 0, "function transform(v) { while (true) {} }")

    data = _send_csv_and_read(api_client, device_simulator, [b"42"], settle_seconds=1.5)

    # Transform timed out -> raw value passes through unmodified.
    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(42.0)

    # The app answered get_dashboard_data and still answers afterwards.
    assert api_client.command("project.dataset.list").get("datasets")


# ---------------------------------------------------------------------------
# Shared Lua library + dataset parameters (spec 0083)
# ---------------------------------------------------------------------------


def _set_library(api_client, code: str, language: str = "lua") -> None:
    api_client.command(
        "project.transformLibrary.set", {"code": code, "language": language}
    )
    time.sleep(0.1)


def _set_params(api_client, group_id: int, dataset_id: int, params: dict) -> None:
    api_client.update_dataset(group_id, dataset_id, transformParams=params)
    time.sleep(0.1)


@pytest.mark.project
def test_shared_library_persists_round_trip(api_client, clean_state):
    """The shared Lua library survives a project export → reload cycle and clears on empty."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    library = "function scale(v, p)\n  return v * p.k + p.o\nend\n"
    _set_library(api_client, library)
    assert api_client.command("project.transformLibrary.get").get("code") == library

    project_json = api_client.command("project.exportJson")["config"]
    assert project_json.get("transformLibrary") == library

    api_client.create_new_project()
    time.sleep(0.2)
    assert api_client.command("project.transformLibrary.get").get("code", "") == ""

    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)
    assert api_client.command("project.transformLibrary.get").get("code") == library

    _set_library(api_client, "")
    exported = api_client.command("project.exportJson")["config"]
    assert "transformLibrary" not in exported


@pytest.mark.project
def test_js_library_persists_round_trip(api_client, clean_state):
    """The JavaScript library is its own key and round-trips independently of the Lua one."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    library = "function scale(v, p) { return v * p.k + p.o; }\n"
    _set_library(api_client, library, language="js")
    got = api_client.command("project.transformLibrary.get", {"language": "js"})
    assert got.get("code") == library
    assert got.get("language") == "js"
    assert api_client.command("project.transformLibrary.get").get("code", "") == ""

    project_json = api_client.command("project.exportJson")["config"]
    assert project_json.get("transformLibraryJs") == library
    assert "transformLibrary" not in project_json

    api_client.create_new_project()
    time.sleep(0.2)
    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)
    got = api_client.command("project.transformLibrary.get", {"language": "js"})
    assert got.get("code") == library

    with pytest.raises(APIError):
        api_client.command("project.transformLibrary.get", {"language": "python"})


@pytest.mark.project
def test_transform_params_persist_round_trip(api_client, clean_state):
    """transformParams keep their scalar types across export → reload; nested values drop."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    _set_params(
        api_client,
        0,
        0,
        {"k": 2.5, "label": "rtd", "enabled": True, "nested": {"dropped": 1}},
    )

    datasets = api_client.command("project.dataset.list").get("datasets", [])
    params = datasets[0].get("transformParams", {})
    assert params == {"k": 2.5, "label": "rtd", "enabled": True}

    project_json = api_client.command("project.exportJson")["config"]
    api_client.create_new_project()
    time.sleep(0.2)
    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)

    datasets = api_client.command("project.dataset.list").get("datasets", [])
    assert datasets[0].get("transformParams", {}) == {
        "k": 2.5,
        "label": "rtd",
        "enabled": True,
    }


@pytest.mark.project
def test_library_dry_run_reports_lua_errors(api_client, clean_state):
    """dryRun accepts a valid chunk and names the error of a broken one, installing neither."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    ok = api_client.command(
        "project.transformLibrary.dryRun", {"code": "function f(v) return v end"}
    )
    assert ok.get("valid") is True

    bad = api_client.command(
        "project.transformLibrary.dryRun", {"code": "function f( return"}
    )
    assert bad.get("valid") is False
    assert bad.get("error")

    assert api_client.command("project.transformLibrary.get").get("code", "") == ""


@pytest.mark.project
def test_js_library_dry_run_reports_errors(api_client, clean_state):
    """The JavaScript dryRun accepts a valid chunk and names the line of a broken one."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    ok = api_client.command(
        "project.transformLibrary.dryRun",
        {"code": "function f(v) { return v; }", "language": "js"},
    )
    assert ok.get("valid") is True
    assert ok.get("language") == "js"

    bad = api_client.command(
        "project.transformLibrary.dryRun",
        {"code": "function f(v) {\n  return v +;\n}", "language": "js"},
    )
    assert bad.get("valid") is False
    assert "Line" in bad.get("error", "")

    got = api_client.command("project.transformLibrary.get", {"language": "js"})
    assert got.get("code", "") == ""


@pytest.mark.project
def test_js_library_function_shared_with_params(
    api_client, device_simulator, clean_state
):
    """Two JavaScript datasets call one library function with their own params."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=2)

    _set_library(
        api_client, "function scale(v, p) { return v * p.k + p.o; }", language="js"
    )
    code = "function transform(v) { return scale(v, params); }"
    _set_transform(api_client, 0, 0, code)
    _set_transform(api_client, 0, 1, code)
    _set_params(api_client, 0, 0, {"k": 2, "o": 0})
    _set_params(api_client, 0, 1, {"k": 3, "o": 1})

    data = _send_csv_and_read(
        api_client, device_simulator, [b"10,10", b"10,10"], settle_seconds=1.4
    )

    v0, _ = _dataset_value(data, 0)
    v1, _ = _dataset_value(data, 1)
    assert v0 == pytest.approx(20.0)
    assert v1 == pytest.approx(31.0)


@pytest.mark.project
def test_lua_library_function_shared_with_params(
    api_client, device_simulator, clean_state
):
    """Two datasets call one library function with their own params and get different values."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=2)

    _set_library(api_client, "function scale(v, p)\n  return v * p.k + p.o\nend\n")
    code = "function transform(v)\n  return scale(v, params)\nend\n"
    _set_transform(api_client, 0, 0, code)
    _set_transform(api_client, 0, 1, code)
    _set_params(api_client, 0, 0, {"k": 2, "o": 0})
    _set_params(api_client, 0, 1, {"k": 3, "o": 1})

    data = _send_csv_and_read(
        api_client, device_simulator, [b"10,10", b"10,10"], settle_seconds=1.4
    )

    v0, _ = _dataset_value(data, 0)
    v1, _ = _dataset_value(data, 1)
    assert v0 == pytest.approx(20.0)
    assert v1 == pytest.approx(31.0)


@pytest.mark.project
def test_js_transform_reads_params(api_client, device_simulator, clean_state):
    """A JavaScript transform receives the same params object as its closure argument."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    _set_transform(
        api_client, 0, 0, "function transform(v) { return v * params.k + params.o; }"
    )
    _set_params(api_client, 0, 0, {"k": 4, "o": 2})

    data = _send_csv_and_read(
        api_client, device_simulator, [b"10", b"10"], settle_seconds=1.2
    )

    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(42.0)


@pytest.mark.project
def test_broken_library_leaves_transforms_intact(
    api_client, device_simulator, clean_state
):
    """A library that fails to compile is reported, and transforms that never call it still run."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    _set_library(api_client, "function broken(\n")
    _set_transform(api_client, 0, 0, "function transform(v) return v * 2 end")

    data = _send_csv_and_read(
        api_client, device_simulator, [b"21", b"21"], settle_seconds=1.2
    )

    num, _ = _dataset_value(data, 0)
    assert num == pytest.approx(42.0)
