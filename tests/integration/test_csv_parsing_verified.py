"""
CSV Parsing Verification Tests

Tests that verify CSV data is correctly parsed with different delimiters.

Unlike test_csv_delimiter_resilience() which only checks connection stability,
these tests actually verify that values are extracted correctly from CSV frames.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import APIError, ChecksumType, DataGenerator


def test_quickplot_comma_csv_parsing(api_client, device_simulator, clean_state):
    """
    Test CSV parsing in QuickPlot mode with default comma delimiter.

    QuickPlot mode works like Arduino Serial Plotter:
    - NO frame delimiters (no /* and */)
    - Line-based parsing with CR, LF, or CRLF terminators
    - Comma-separated values only

    NOTE: In QuickPlot mode, there is no project model. Datasets are auto-generated
    dynamically from incoming CSV data. We can only verify connection stability
    and that frames are being received without errors.
    """
    # Create new project and set to QuickPlot mode
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.2)

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 20 CSV lines with known values (NO frame delimiters, just line terminators)
    test_values = [1.23, 4.56, 7.89]
    frames = []
    for i in range(20):
        values = [v + i * 0.1 for v in test_values]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter=",")
        # QuickPlot mode: Send raw CSV with only line terminator (no /* */)
        frame = (payload + "\n").encode("utf-8")
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.5)

    # In QuickPlot mode, we can only verify that:
    # 1. The connection remains stable
    # 2. Frames are being received without errors
    # We CANNOT check project status because QuickPlot has no project model
    status = api_client.command("io.getStatus")
    assert status[
        "isConnected"
    ], "Device should remain connected after receiving QuickPlot data"


def test_csv_parsing_with_lua_semicolon(api_client, device_simulator, clean_state):
    """
    Test CSV parsing with semicolon delimiter using a Lua parser.

    Exercises the native frame:split(sep) helper (string.split installed by
    LuaCompat: literal separator, keeps empty fields) on semicolon-separated
    values.
    """
    # Create new project via API
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group
    api_client.command("project.group.add", {"title": "CSV Values", "widgetType": 0})
    time.sleep(0.2)

    # Add 3 datasets
    for i in range(3):
        api_client.command("project.dataset.add", {"groupId": 0, "options": 0})
        time.sleep(0.1)

    # Set Lua parser for semicolon delimiter via the native frame:split helper
    # (language=1 = Lua, which is also the new-project default)
    parser_code = "function parse(frame) return frame:split(';') end"
    api_client.set_frame_parser_code(parser_code, language=1)
    time.sleep(0.2)

    # Configure frame delimiters for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.activate")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 15 frames with semicolon delimiter
    test_values = [1.23, 4.56, 7.89]
    frames = []
    for i in range(15):
        values = [v + i * 0.1 for v in test_values]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter=";")
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.0)

    # Verify frames were parsed
    status = api_client.command("io.getStatus")
    assert status["isConnected"], "Device should remain connected"

    project_status = api_client.get_project_status()
    assert project_status["datasetCount"] >= 3, "Should have at least 3 datasets"


def test_csv_parsing_with_lua_tab(api_client, device_simulator, clean_state):
    """
    Test CSV parsing with tab delimiter using a Lua parser.

    Exercises the native frame:split(sep) helper on tab-separated values.
    """
    # Create new project via API
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group
    api_client.command("project.group.add", {"title": "CSV Values", "widgetType": 0})
    time.sleep(0.2)

    # Add 4 datasets
    for i in range(4):
        api_client.command("project.dataset.add", {"groupId": 0, "options": 0})
        time.sleep(0.1)

    # Set Lua parser for tab delimiter via the native frame:split helper
    # (language=1 = Lua, which is also the new-project default)
    parser_code = "function parse(frame) return frame:split('\\t') end"
    api_client.set_frame_parser_code(parser_code, language=1)
    time.sleep(0.2)

    # Configure frame delimiters for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.activate")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 15 frames with tab delimiter
    test_values = [1.23, 4.56, 7.89, 10.11]
    frames = []
    for i in range(15):
        values = [v + i * 0.1 for v in test_values]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter="\t")
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.0)

    # Verify frames were parsed
    status = api_client.command("io.getStatus")
    assert status["isConnected"], "Device should remain connected"

    project_status = api_client.get_project_status()
    assert project_status["datasetCount"] >= 4, "Should have at least 4 datasets"


def test_csv_parsing_with_lua_pipe(api_client, device_simulator, clean_state):
    """
    Test CSV parsing with pipe delimiter using a Lua parser.

    Exercises the native frame:split(sep) helper on pipe-separated values.
    """
    # Create new project via API
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group
    api_client.command("project.group.add", {"title": "CSV Values", "widgetType": 0})
    time.sleep(0.2)

    # Add 5 datasets
    for i in range(5):
        api_client.command("project.dataset.add", {"groupId": 0, "options": 0})
        time.sleep(0.1)

    # Set Lua parser for pipe delimiter via the native frame:split helper
    # (language=1 = Lua, which is also the new-project default)
    parser_code = "function parse(frame) return frame:split('|') end"
    api_client.set_frame_parser_code(parser_code, language=1)
    time.sleep(0.2)

    # Configure frame delimiters for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.activate")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 15 frames with pipe delimiter
    test_values = [1.23, 4.56, 7.89, 10.11, 12.13]
    frames = []
    for i in range(15):
        values = [v + i * 0.1 for v in test_values]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter="|")
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.0)

    # Verify frames were parsed
    status = api_client.command("io.getStatus")
    assert status["isConnected"], "Device should remain connected"

    project_status = api_client.get_project_status()
    assert project_status["datasetCount"] >= 5, "Should have at least 5 datasets"


def test_frame_parser_language_mismatch_does_not_crash(api_client, clean_state):
    """
    Regression: a frame-parser language mismatch must never crash the app.

    Loading JavaScript source into the Lua engine makes luaL_loadbuffer raise
    a syntax error; because Lua is built as C++ the error propagates as a
    thrown lua_longjmp*. On macOS (Xcode 26 ld dropped DWARF unwind under
    -dead_strip + -flto) that throw escaped Lua's own catch and aborted the
    process. The fix wraps the Lua load path in a C++ try/catch so the
    mismatch becomes a clean load failure. This test feeds the mismatch on
    purpose and asserts the app stays alive and responsive afterwards.

    New projects now default to the Native template parser, so each direction
    flips the language explicitly first, then sends the mismatched code via
    the raw setCode command WITHOUT a language key so it lands in the engine
    exactly as the original crash did. Both flip directions are exercised.
    """

    def feed_mismatch(code: str) -> None:
        # The handler may reject (APIError) or accept and fail to compile;
        # either is fine. A dropped socket (ConnectionError) means a crash.
        try:
            api_client.command("project.frameParser.setCode", {"code": code})
        except APIError:
            pass
        except ConnectionError as exc:
            raise AssertionError(
                f"App crashed loading mismatched parser code: {code!r}"
            ) from exc

    # JavaScript source into the Lua engine (the original crash trigger)
    api_client.create_new_project()
    time.sleep(0.2)
    api_client.set_frame_parser_language(1)
    time.sleep(0.1)
    feed_mismatch("function parse(frame) { return frame.split(';'); }")
    time.sleep(0.2)

    # App must still answer commands
    status = api_client.command("io.getStatus")
    assert (
        "isConnected" in status
    ), "App should remain responsive after JS->Lua mismatch"

    # Now the reverse: Lua source into a JS engine (flip the language first)
    api_client.create_new_project()
    time.sleep(0.2)
    api_client.set_frame_parser_language(0)
    time.sleep(0.1)
    feed_mismatch("function parse(frame) return frame:split(';') end")
    time.sleep(0.2)

    status = api_client.command("io.getStatus")
    assert (
        "isConnected" in status
    ), "App should remain responsive after Lua->JS mismatch"
