"""
Temporary source-level guards for shipped C++ regressions.

This file is a holding pen, not a contract. Reading source text is the weakest kind of
test there is: it rejects harmless refactors and it accepts behaviourally wrong code that
happens to spell things the same way. Every guard here is therefore provisional, and the
policy is:

1. A guard exists only while NOTHING behavioural or mechanical covers its invariant. The
   moment a `scripts/code-verify.py` rule, an `app/tests/` ctest, a `tests/integration/`
   case, or the compiler covers it, the guard is deleted -- not kept "for safety".
2. Every guard names the incident or spec it protects, and carries a `retire when:` line
   naming the behavioural test or lint that would replace it. A guard without one is
   unfinished work.
3. Assertions check intent at the coarsest level that still fails when the invariant
   breaks. Exact local names, whitespace, statement order and helper spellings are
   incidental; pinning them buys nothing and costs every future refactor.
4. Reading a component means `_component_text()`. Spec 0070 moved cohesive concerns out of
   the god files into sibling TUs, and which TU a function landed in is not an invariant.

Adding a guard here is a decision to owe a real test later. Write the real test instead
whenever it is cheap: a JS-parser unit next door in `tests/scripts/`, a ctest under
`app/tests/`, or an API-driven case under `tests/integration/`.
"""

from pathlib import Path
import json
import re

ROOT = Path(__file__).resolve().parents[2]


_SPEC_0070_SPLITS = {
    "core/Ui/AI/ToolDispatcher.cpp": "core/Ui/AI/Tools",
    "core/Ui/AI/Conversation.cpp": "core/Ui/AI/Conversation",
    "core/Storage/Sessions/DatabaseManager.cpp": "core/Storage/Sessions/DatabaseManager",
    "core/Storage/Sessions/Player.cpp": "core/Storage/Sessions/Player",
    "core/Devices/IO/Drivers/BluetoothLE.cpp": "core/Devices/IO/Drivers/BluetoothLE",
    "core/Devices/IO/Drivers/USB.cpp": "core/Devices/IO/Drivers/USB",
    "core/Devices/IO/Drivers/Audio.cpp": "core/Devices/IO/Drivers/Audio",
    "core/Devices/IO/Drivers/OpcUa.cpp": "core/Devices/IO/Drivers/OpcUa",
    "core/Devices/IO/Drivers/Modbus.cpp": "core/Devices/IO/Drivers/Modbus",
    "core/Devices/IO/ConnectionManager.cpp": "core/Devices/IO/ConnectionManager",
    "core/Ui/UI/Widgets/Waterfall.cpp": "core/Ui/UI/Widgets/Waterfall",
    "core/Ui/UI/Widgets/Terminal.cpp": "core/Ui/UI/Widgets/Terminal",
    "core/Ui/UI/Taskbar.cpp": "core/Ui/UI/Taskbar",
    "core/Ui/UI/WindowManager.cpp": "core/Ui/UI/WindowManager",
    "core/Ui/UI/Dashboard.cpp": "core/Ui/UI/Dashboard",
    "core/Ui/UI/Widgets/PainterContext.cpp": "core/Ui/UI/Widgets/Painter",
    "core/Api/API/Server.cpp": "core/Api/API/Server",
    "core/Storage/CSV/Player.cpp": "core/Storage/CSV/Player",
    "core/Pipeline/DataModel/FrameBuilder.cpp": "core/Pipeline/DataModel/FrameBuilder",
    "core/Devices/MQTT/Publisher.cpp": "core/Devices/MQTT",
    "core/Ui/Misc/ExtensionManager.cpp": "core/Ui/Misc/Extensions",
}


def _component_text(path: str) -> str:
    """A component's source: the named file plus the concern TUs extracted from it.

    Spec 0070 moved cohesive concerns out of the god files into sibling TUs. These
    checks assert on a component's behaviour, not on which of its files a given
    function ended up in, so they read the whole component.
    """
    p = ROOT / path
    parts = [p.read_text(encoding="utf-8")] if p.exists() else []
    d = ROOT / _SPEC_0070_SPLITS.get(path, "")
    if path in _SPEC_0070_SPLITS and d.is_dir():
        for f in sorted(d.iterdir()):
            if f.suffix in (".cpp", ".h"):
                parts.append(f.read_text(encoding="utf-8"))
    return "\n".join(parts)


def _read(path: str) -> str:
    if path.endswith((".cpp", ".h")):
        return _component_text(path)
    return (ROOT / path).read_text(encoding="utf-8")


def test_project_model_save_returns_real_result():
    """The on-disk save path reports the real outcome of the write, never a hardcoded
    success. The explanatory in-body comment was stripped by the comment-cleanup pass,
    so the behaviour is pinned here instead.

    retire when: an integration case makes the write fail (read-only target) and asserts
    `project.save` reports the failure over the API.
    """
    text = _read("core/Pipeline/DataModel/Project/ProjectPersistence.cpp")

    body = re.search(
        r"bool DataModel::ProjectPersistence::saveJsonFile\(const bool askPath\)[\s\S]*?\n\}",
        text,
    )
    assert body is not None, "saveJsonFile body must be present"
    snippet = body.group(0)

    returns = re.findall(r"\breturn\s+([^;]+);", snippet)
    assert returns, "saveJsonFile must return something"
    assert "true" not in [
        value.strip() for value in returns
    ], "saveJsonFile must not hardcode a successful save"
    assert any(
        "(" in value for value in returns
    ), "success must come out of the write path, not out of a bare flag"


def test_hal_write_api_uses_signed_sizes():
    """The driver write API returns a SIGNED count so a driver can report failure as -1;
    an unsigned return turns every failure into a huge apparent success.

    The overrides are compiler-enforced (a mismatched signature leaves the pure virtual
    unimplemented and the driver abstract), so only the base declaration is pinned.

    retire when: a ctest driver double asserts `write()` returns a negative value on a
    closed link -- that makes the signedness observable instead of declared.
    """
    hal = _read("core/Core/IO/HAL_Driver.h")

    assert re.search(
        r"virtual qint64 write\(const QByteArray&\s*\w+\)\s*=\s*0;", hal
    ), "HAL_Driver::write must stay a signed-count pure virtual"


def test_io_manager_bounds_written_byte_count():
    """A driver that over-reports its write count used to chop() past the start of the
    echoed payload; the count is clamped to what was actually handed over.

    retire when: a ctest drives the device IO router with a driver double that reports
    more bytes written than it was given.
    """
    text = _read("core/Devices/IO/ConnectionManager.cpp")

    assert re.search(
        r"qMin<qint64>\(\s*\w+,\s*\w+\.size\(\)\)", text
    ), "the written-byte count must be clamped to the payload size"
    assert re.search(
        r"\.chop\(\w+\.length\(\) - \w+\);", text
    ), "the echo is trimmed to the clamped count"


def test_csv_player_catchup_uses_next_frame_timestamps():
    """Playback pacing derives from the NEXT indexed row's recorded time, never from a
    re-stamped wall clock (spec 0022).

    retire when: a ctest drives CSV::Player over a fixture with uneven row spacing --
    tst_replay_playback_engine.cpp covers the window/epoch half of replay, not pacing.
    """
    text = _read("core/Storage/CSV/Player.cpp")

    body = re.search(
        r"bool CSV::Player::recomputeMsUntilNext\(qint64& msUntilNext\)[\s\S]*?\n\}",
        text,
    )
    assert body is not None, "recomputeMsUntilNext must exist (spec 0022 pacing)"
    pacing = body.group(0)
    assert re.search(r"m_framePos \+ 1", pacing), "the delay must target the NEXT row"
    assert (
        "rowSecondsSinceStart(" in pacing
    ), "the delay must read the recording's own time, not a wall clock"

    catchup = re.search(
        r"int CSV::Player::catchUpTargetRow\(double target\) const[\s\S]*?\n\}",
        text,
    )
    assert catchup is not None, "catchUpTargetRow must exist (spec 0022 catch-up)"
    assert re.search(
        r"rowSecondsSinceStart\(\w+ \+ 1\)", catchup.group(0)
    ), "the catch-up scan must compare against the next row's recorded time"
    assert "catchUpTargetRow(target)" in text, "the catch-up branch must be reachable"


def test_mdf4_player_catchup_uses_next_frame_timestamp():
    """Same invariant for the MDF4 player: the playback delay and the catch-up scan read
    the recording's next-frame time, never a re-stamped clock (spec 0022).

    retire when: a ctest drives MDF4::Player over an unevenly spaced fixture.
    """
    text = _read("core/Storage/MDF4/Player.cpp")

    update = re.search(r"void MDF4::Player::updateData\(\)[\s\S]*?\n\}", text)
    assert update is not None
    assert re.search(
        r"m_timestamps\[[^\]]*\+ 1[^\]]*\]", update.group(0)
    ), "the delay must read the next frame's recorded timestamp"

    catchup = re.search(
        r"void MDF4::Player::catchUpToTarget\(double targetTime\)[\s\S]*?\n\}",
        text,
    )
    assert catchup is not None, "catchUpToTarget must exist (spec 0022 catch-up)"
    assert re.search(
        r"m_timestamps\[[^\]]*\+ 1[^\]]*\] > targetTime", catchup.group(0)
    ), "the catch-up scan must stop at the first frame due after the target"


def test_api_server_enabled_state_tracks_listen_failure():
    """A refused listen() must leave the server disabled and still notify, so the toggle
    in Preferences springs back instead of lying about a bound port.

    retire when: an integration case binds the API port from outside, toggles the server
    on, and asserts the reported state stays disabled.
    """
    text = _read("core/Api/API/Server.cpp")

    body = re.search(
        r"void API::Server::setEnabled\(const bool enabled\)[\s\S]*?\n\}", text
    )
    assert body is not None, "setEnabled body must be present"
    setter = body.group(0)

    assert "startListening()" in setter, "the setter must consult the real bind"
    assert (
        "m_enabled = enabled;" not in setter
    ), "the stored flag must follow the listen outcome, not the request"
    stored = re.search(r"m_enabled = (\w+);", setter)
    assert stored is not None and stored.group(1) != "enabled"
    assert (
        setter.count("Q_EMIT enabledChanged();") == 2
    ), "a refused listen must still notify so the UI reverts"


def test_ble_characteristic_path_guards_index_before_at():
    """The selected-characteristic index is range-checked before it indexes the list; a
    stale index used to call .at() out of bounds after a service rescan.

    retire when: a ctest exercises the BLE characteristic selection with a stale index.
    """
    text = _read("core/Devices/IO/Drivers/BluetoothLE.cpp")

    assert "if (m_selectedCharacteristic == -1)" in text
    assert "m_selectedCharacteristic < m_characteristics.count()" in text


def test_macos_native_window_validates_qwindow_before_winid():
    """Every QWindow -> NSView bridge sits behind a null guard on both halves: winId() on
    a destroyed window, or an NSView that is not yet realised, crashes the GUI thread.

    retire when: the macOS native-window layer gets a ctest (it needs a live NSWindow,
    which is why it has none today).
    """
    text = _read("core/Ui/Platform/NativeWindow_macOS.mm")

    bridges = 0
    for fn in re.split(r"\n(?=[A-Za-z_][^\n]*NativeWindow::)", text):
        for m in re.finditer(
            r"NSView \*(\w+) = reinterpret_cast<NSView \*>\((\w+)->winId\(\)\);", fn
        ):
            bridges += 1
            view, win = m.group(1), m.group(2)
            assert re.search(
                r"if \(!%s[\s)|]" % win, fn[: m.start()]
            ), f"{win}->winId() must sit behind a null guard"
            after = fn[m.end() : m.end() + 300]
            assert re.search(r"if \(!%s\)" % view, after) or re.search(
                r"\b%s \?" % view, after
            ), f"{view} must be null-checked before it is used"

    assert bridges, "expected at least one QWindow -> NSView bridge"


def test_hotpath_byte_array_ptrs_are_null_guarded():
    """The three hotpath entry points short-circuit on an absent or empty payload before
    touching it. Each takes its bytes by reference or pointer from a different thread.

    retire when: the hotpath ingest seam gets a ctest that feeds each entry point an
    empty payload (tst_frame_consumer.cpp covers the drain side only).
    """
    frame_reader = _read("core/Pipeline/IO/FrameReader.cpp")
    server = _read("core/Api/API/Server.cpp")
    console = _read("core/Ui/Console/Handler.cpp")

    reader_body = re.search(
        r"void IO::FrameReader::processData\([\s\S]*?\n\}", frame_reader
    )
    assert reader_body is not None
    assert re.search(
        r"if \(!\w+ \|\|[^)]*isEmpty\(\)\)", reader_body.group(0)
    ), "processData must reject a null or empty capture"

    server_body = re.search(r"::writeRawData\([\s\S]*?\n\}", server)
    assert server_body is not None
    broadcast = server_body.group(0)
    assert (
        "isEmpty()" in broadcast
    ), "the raw broadcast must skip an empty payload / client list"

    console_body = re.search(r"::hotpathRxData\([\s\S]*?\n\}", console)
    assert console_body is not None
    tap = console_body.group(0)
    assert "isEmpty()" in tap, "the console tap must skip an empty payload"


def test_window_manager_taskbar_access_is_guarded():
    """The taskbar arrives through a qobject_cast that can yield null, so every reset of
    the active window is guarded; an unguarded one crashed on teardown.

    retire when: a ctest constructs a WindowManager with no taskbar bound and closes a
    window through it.
    """
    text = _read("core/Ui/UI/WindowManager.cpp")

    assert "qobject_cast<UI::Taskbar*>(taskbar)" in text
    calls = list(re.finditer(r"m_taskbar->setActiveWindow\(nullptr\);", text))
    assert calls, "expected the active-window reset on teardown"
    for call in calls:
        assert re.search(
            r"if \(m_taskbar\)", text[max(0, call.start() - 160) : call.start()]
        ), "every active-window reset must be null-guarded"


def test_usb_close_joins_threads_with_quit_then_wait():
    """The USB threads run on the started + DirectConnection idiom, so quit() must
    precede wait() to drop out of exec(); terminate() corrupts libusb mid-transfer.

    retire when: a ctest opens and closes the USB pump against a stub transfer source.
    """
    text = _read("core/Devices/IO/Drivers/USB.cpp")

    joins = list(re.finditer(r"(\bm_\w+)\.wait\(\)", text))
    assert joins, "expected the worker threads to be joined on close"
    for join in joins:
        name = join.group(1)
        assert (
            f"{name}.quit();" in text[: join.start()]
        ), f"{name}.wait() must follow {name}.quit()"

    assert ".terminate();" not in text, "terminate() corrupts libusb mid-transfer"


def test_frame_parser_uses_qcoreapplication_event_forwarding():
    """The embedded editor forwards events through QCoreApplication rather than the
    vendored DW_EXEC_EVENT macro, which re-entered the widget's own event loop.

    retire when: nothing behavioural can see this -- delete it once the vendored editor
    is replaced, since DW_EXEC_EVENT goes with it.
    """
    text = _read("core/Ui/ProjectEditor/Editors/JsCodeEditor.cpp")
    bridge = _read("core/Ui/ProjectEditor/Editors/EmbeddedCodeEditor.cpp")

    assert re.search(r"QCoreApplication::sendEvent\(", bridge)
    assert "DW_EXEC_EVENT" not in text
    assert "DW_EXEC_EVENT" not in bridge


def test_project_editor_bounds_checks_combo_indices():
    """Every combo index from QML is range-checked against its own domain before it is
    used: an out-of-range index otherwise falls back to the domain's first entry and
    silently rewrites the user's choice (spec 0036).

    retire when: an integration case drives each of these combos over the API with an
    out-of-range index and asserts the stored value is unchanged.
    """
    text = ""
    for tu in sorted((ROOT / "core/Ui/ProjectEditor").rglob("*.cpp")):
        text += tu.read_text(encoding="utf-8")

    # The editor's sub-objects (spec 0077) reach the facade's state through `m_editor.`; the
    # guards are what this test pins, not which class holds the member.
    text = text.replace("m_editor.", "")

    for domain in (
        "keys",
        "eolKeys",
        "checksums",
        "m_frameDetectionMethodsValues",
        "m_datasetWidgets",
        "m_plotOptions",
        "m_fftSamples",
    ):
        assert re.search(
            r"\w+ < 0 \|\| \w+ >= %s\.size\(\)|\w+ >= 0 && \w+ < %s\.size\(\)"
            % (domain, domain),
            text,
        ), f"the {domain} combo index must be range-checked"


# ---------------------------------------------------------------------------
# License guard regression tests
#
# SS_LICENSE_GUARD() is anti-tamper: the point is that the guard call SITS in
# the source at every commercial gate, so a patched-out guard is visible in a
# diff. These are deliberately source-level and have no behavioural twin --
# a test that exercised the gate at runtime would prove the guard runs, not
# that it is still written down. They retire only if the guard scheme is
# replaced, or when registry-verify.py grows a commercial-gate census that
# covers the same list mechanically.
# ---------------------------------------------------------------------------


def test_license_guard_macro_defined_in_commercial_token_header():
    """CommercialToken.h defines SS_LICENSE_GUARD and includes the generated header.

    retire when: the guard scheme is replaced (see the block comment above).
    """
    text = _read("core/Core/Licensing/CommercialToken.h")

    assert '#  include "LicenseGuards.generated.h"' in text
    assert "#  define SS_LICENSE_GUARD()" in text
    assert "Licensing::Guards::runGuard(__LINE__)" in text


def test_license_guard_present_in_license_flag_publisher():
    """The root's publishLicenseState() (the Core::License writer) runs the guard.

    retire when: the guard scheme is replaced.
    """
    text = _read("app/src/Misc/ModuleManager.cpp")

    assert re.search(
        r"Core::License::set\([^;]*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from publishLicenseState()"


def test_license_guard_present_in_mqtt_connect():
    """The MQTT input driver's open() runs the guard before it dials.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Devices/IO/Drivers/MQTT.cpp")

    assert re.search(
        r"!SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from MQTT input driver open() guard"


def test_license_guard_present_in_mqtt_hotpath():
    """MQTT::Publisher reads the root-derived Core::License flag (spec 0077) instead of
    re-deriving the token on the publish path. The guard itself is pinned where the flag
    is derived, in publishLicenseState().

    retire when: the guard scheme is replaced -- tst_license_state.cpp already covers the
    Core::License flag's own semantics.
    """
    text = _read("core/Storage/MQTT/Publisher.cpp")

    assert (
        "Core::License::activated()" in text
    ), "MQTT publisher license check must read Core::License::activated()"
    assert (
        "CommercialToken" not in text
    ), "the publisher must not re-derive the licence token"


def test_license_guard_present_in_mdf4_export():
    """Both the MDF4 file creation and its enable setter run the guard.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Storage/MDF4/Export.cpp")

    assert (
        text.count("SS_LICENSE_GUARD()") >= 2
    ), "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in MDF4::Export"


def test_license_guard_present_in_console_export():
    """Both the console export's file creation and its enable setter run the guard.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Ui/Console/Export.cpp")

    assert (
        text.count("SS_LICENSE_GUARD()") >= 2
    ), "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in Console::Export"


def test_license_guard_present_in_connection_manager():
    """Connecting a commercially-gated device runs the guard.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Devices/IO/ConnectionManager.cpp")

    assert "SS_LICENSE_GUARD()" in text


def test_license_guard_present_in_dashboard():
    """The Dashboard's pro-feature detection runs the guard alongside the token check.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Ui/UI/Dashboard.cpp")

    assert re.search(
        r"\w+\.isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from Dashboard pro detection"


def test_license_guard_present_in_gps():
    """The GPS map-type gate runs the guard alongside the token check.

    retire when: the guard scheme is replaced.
    """
    text = _read("core/Ui/UI/Widgets/GPS.cpp")

    assert re.search(
        r"\w+\.isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from GPS map type guard"


def test_license_guard_cmake_generator_exists():
    """The CMake guard generator exists and the root build includes it.

    retire when: the guard scheme is replaced.
    """
    cmake_script = _read("cmake/GenLicenseGuards.cmake")
    root_cmake = _read("CMakeLists.txt")

    assert "GenLicenseGuards.cmake" in root_cmake
    assert "_LG_COUNT" in cmake_script
    assert "runGuard" in cmake_script
    assert "guardTable" in cmake_script


def test_license_guard_cmake_generates_all_styles():
    """The generator dispatches across every distinct check style, so the emitted guards
    do not share one recognisable shape.

    retire when: the guard scheme is replaced.
    """
    text = _read("cmake/GenLicenseGuards.cmake")

    style_match = re.search(r"math\(EXPR _style \"\$\{_i\} % (\d+)\"\)", text)
    assert style_match, "guard style dispatch modulus not found"
    style_count = int(style_match.group(1))
    assert style_count >= 6, f"Too few guard styles: {style_count} (minimum 6)"

    explicit = set(re.findall(r"_style EQUAL (\d+)", text))
    assert explicit, "no explicit guard style branches found"

    # Every style index below the modulus must be reachable: either an
    # explicit `_style EQUAL N` branch or the trailing else() fallback.
    for style in range(style_count):
        if str(style) in explicit:
            continue
        assert "else()" in text, f"style {style} has no branch and no else() fallback"


def test_license_guard_minimum_count():
    """The generator produces at least 20 guard functions.

    retire when: the guard scheme is replaced.
    """
    text = _read("cmake/GenLicenseGuards.cmake")

    match = re.search(r"set\(_LG_COUNT\s+(\d+)\)", text)
    assert match, "_LG_COUNT not found in GenLicenseGuards.cmake"
    assert (
        int(match.group(1)) >= 20
    ), f"Guard count too low: {match.group(1)} (minimum 20)"


def test_license_guard_build_dir_is_gitignored():
    """The generated header stays out of the tree, so the guard table differs per build.

    retire when: the guard scheme is replaced.
    """
    text = _read(".gitignore")

    assert "LicenseGuards.generated.h" in text


def test_session_report_series_preserve_raw_points_under_budget():
    """Session report charts keep raw samples up to the 10k budget: a decimated series
    hides the spikes the report exists to show.

    retire when: a ctest feeds the report chart builder a series above and below the
    budget and asserts the emitted sample count (tst_sessions_export_worker.cpp covers
    row digests only).
    """
    report = _read("core/Storage/Sessions/ReportData.cpp")
    manager = _read("core/Storage/Sessions/DatabaseManager.cpp")
    runtime = _read("app/rcc/templates/reports/session-report.js")

    assert (
        "series.values.size() == std::min<std::size_t>(count, maxSamples)" in report
    ), "the builder must assert it emits min(count, budget) samples"
    assert "Q_ARG(int, 10000)" in manager, "the chart sample budget must stay 10k"
    assert (
        runtime.count("decimation: { enabled: false }") >= 2
    ), "the report runtime must not decimate on top of the budget"


def test_timestamp_pipeline_starts_in_driver_and_shares_parsed_frames():
    """Source owns time, and spec 0055 left exactly one publication payload.

    The positive declarations this once pinned are compiler-enforced (their callers would
    not build), so what stays is the set of things that would compile perfectly while
    being wrong: a builder-side clock re-stamping rows, a revived per-frame
    TimestampedFrame TX path, or a direct dashboard call from the frame path.

    retire when: a ctest asserts per-row stamps against a fixture driver timestamp
    (tst_frame_builder_staging.cpp covers staging/flush, not the stamp derivation).
    """
    builder_h = _read("core/Pipeline/DataModel/FrameBuilder.h")
    builder_cpp = _read("core/Pipeline/DataModel/FrameBuilder.cpp")
    publisher_cpp = _read("core/Pipeline/DataModel/FrameBuilder/BlockPublisher.cpp")
    dashboard_h = _read("core/Ui/UI/Dashboard.h")

    # Source owns time: each row's stamp derives from the driver's timestamp, never a
    # builder-side clock.
    assert re.search(
        r"\w+\s*= data->timestamp \+ \w+ \* \w+;", builder_cpp
    ), "each replayed row's stamp must derive from the driver timestamp, not a builder clock"
    assert "updateTimestampedFramesEnabled" not in builder_cpp
    assert "nextTimestampedFrameTime" not in builder_cpp

    # One publication payload: the per-frame TimestampedFrame TX path is gone, and the
    # fan-out hop lives in BlockPublisher (spec 0075 R12.8), never a direct dashboard call.
    assert "void hotpathTxFrame" not in builder_h
    assert "void hotpathTxFrame" not in dashboard_h
    assert "publishBlockToDashboard(" in publisher_cpp
    assert "publishFrameToDashboard" not in publisher_cpp
    assert "dashboard.hotpathRxFrame(" not in publisher_cpp
    assert "dashboard.hotpathRxFrame(" not in builder_cpp


# ----------------------------------------------------------------------------------
# R1 -- license storage stays on encrypted QSettings, never an OS keystore
# ----------------------------------------------------------------------------------


def test_license_storage_uses_simplecrypt_qsettings_not_keystore():
    """The OS-keystore experiment was reverted (f8c02f83): license and trial secrets
    persist through SimpleCrypt-encrypted QSettings. Headless servers, slim
    containers, and SSH-only hosts often have no running keyring daemon, and a
    license loss on those systems would regress against the pre-keystore release.
    Guard that the revert stays reverted -- no keystore on the licensing path, and
    the encrypted-QSettings round trip remains in place.

    retire when: never, while the decision stands -- this guards a SHELVED design, and
    no runtime test can observe "the keystore was not re-adopted". Delete it only if the
    decision is reversed.
    """
    trial = _read("app/src/Licensing/Trial.cpp")
    lemon = _read("app/src/Licensing/LemonSqueezy.cpp")

    # The reverted keystore class is gone for good.
    assert not (
        ROOT / "app/src/Licensing/SecretStorage.cpp"
    ).exists(), "SecretStorage.cpp was reverted; it must not return"
    assert not (ROOT / "app/src/Licensing/SecretStorage.h").exists()

    # Neither licensing path may reach for the OS keystore.
    for name, text in (("Trial.cpp", trial), ("LemonSqueezy.cpp", lemon)):
        assert "SecretStore" not in text, f"{name} must not use the OS keystore"
        assert "SecretStorage" not in text, f"{name} must not use SecretStorage"

    # Both secrets round-trip through SimpleCrypt into their own QSettings group.
    assert re.search(r"encryptToString\(", trial) and 'beginGroup("trial")' in trial
    assert re.search(r"decryptToString\(", trial)
    assert re.search(r"encryptToString\(", lemon) and 'beginGroup("licensing")' in lemon


# ----------------------------------------------------------------------------------
# R6 -- hotpath logging is throttled, frame pool scan avoids per-slot atomic ops
# ----------------------------------------------------------------------------------


def test_frame_reader_dropped_frame_log_is_throttled():
    """The dropped-frame qWarning() sits behind the same 5 s gate as the user
    notification; it used to fire per drop and flood stderr at 10 kHz saturation.

    retire when: `perf-log-on-hotpath` is promoted from advisory to a blocking rule --
    today it reports the site but cannot fail CI, so the throttle has no mechanical gate.
    """
    text = _read("core/Pipeline/IO/FrameReader.cpp")

    body = re.search(
        r"void IO::FrameReader::noteDroppedFrame\(\)\s*\{[\s\S]*?\n\}",
        text,
    )
    assert body is not None, "noteDroppedFrame body must be present"
    gate = re.search(r"m_lastDropNotify[^\n]*seconds\(5\)", body.group(0))
    warn = re.search(r"qWarning\(\)", body.group(0))
    assert gate is not None, "the drop notification must sit behind a 5 s gate"
    assert warn is not None, "the drop warning must still exist"
    assert gate.start() < warn.start(), "qWarning must sit AFTER the throttle gate"


def test_frame_reader_buffer_overflow_log_is_throttled():
    """The buffer-overflow qWarning() throttles on its own timer, so a saturated ring
    does not log per chunk.

    retire when: `perf-log-on-hotpath` is promoted from advisory to a blocking rule.
    """
    text = _read("core/Pipeline/IO/FrameReader.cpp")
    header = _read("core/Pipeline/IO/FrameReader.h")

    assert (
        "m_lastOverflowLog" in header
    ), "the overflow log needs its own timer, separate from the drop gate"

    gate = re.search(r"m_lastOverflowLog[^\n]*seconds\(5\)", text)
    assert gate is not None, "the overflow warning must sit behind a 5 s gate"
    warn = text.find("Buffer overflow", gate.start())
    assert warn != -1, "the overflow warning must sit after its gate"


def test_frame_builder_pool_scan_copies_no_shared_ptr():
    """The pool-slot allocator probes free slots through the pool's own reference count
    and never copies a slot's shared_ptr: a copy is an atomic refcount bump per frame on
    the publish path.

    `--benchmark-hotpath` now carries a per-frame allocation column (spec 0084), but it counts
    only in a `-DSS_ALLOC_STATS=ON` build, so this source-level scan stays as the check that runs
    without a build. Retire when `perf-shared-ptr-by-value` is promoted to a blocking rule.
    """
    text = _read("core/Pipeline/DataModel/FrameBuilder.cpp")

    scan = re.search(
        r"size_t DataModel::FrameBuilder::claimPoolSlot\([^)]*\) noexcept\s*\{[\s\S]*?\n\}",
        text,
    )
    assert scan is not None, "claimPoolSlot body must be present"
    scan_body = scan.group(0)

    assert re.search(
        r"m_framePool\[\w+\]\.use_count\(\) == 1", scan_body
    ), "the free-slot probe must read the pool's own reference count"
    assert "compare_exchange_strong" not in scan_body, "scan must not CAS a slot flag"
    assert not re.search(
        r"auto \w+ = m_framePool\[", scan_body
    ), "scan must not copy a slot shared_ptr"

    assert re.search(
        r"const auto& \w+ = m_framePool\[\w+\];", text
    ), "the claimed slot must be bound by const reference, not copied"


# ----------------------------------------------------------------------------------
# R7 -- BackupManager snapshots carry a version stamp; CrashTracker stays local-only
# ----------------------------------------------------------------------------------


def test_backup_manager_writes_versioned_wrapper():
    """Every snapshot carries a version stamp plus the metadata the restore UI shows;
    an unstamped snapshot cannot be refused by a future build that changes the format.

    retire when: a ctest round-trips BackupManager snapshot -> restore and asserts the
    wrapper fields.
    """
    text = _read("core/Ui/Misc/BackupManager.cpp")

    assert re.search(r"kSnapshotFormat = \d+;", text), "snapshots need a format version"
    assert re.search(
        r'kBackupMetaKey\("_backupMeta"\)', text
    ), "the wrapper key is persisted in user files and must not change"

    for field in ("format", "takenAt", "label", "sha1"):
        assert re.search(
            r'insert\(QStringLiteral\("%s"\)' % field, text
        ), f"the snapshot wrapper must carry {field}"
    assert re.search(r"insert\(kBackupMetaKey,", text)


def test_backup_manager_restore_refuses_newer_format():
    """restore() refuses a snapshot written by a newer build, and strips the wrapper
    before handing the document to the loader, which knows nothing about it.

    retire when: a ctest restores a snapshot stamped with a future format and asserts the
    refusal.
    """
    text = _read("core/Ui/Misc/BackupManager.cpp")

    assert re.search(
        r"if \(\w+ > kSnapshotFormat\)", text
    ), "restore must refuse a newer snapshot format"
    assert re.search(
        r"remove\(kBackupMetaKey\);", text
    ), "the wrapper must be stripped before the document reaches the loader"
    assert "loadFromJsonDocument(" in text


def test_backup_manager_summarize_surfaces_wrapper():
    """The snapshot listing surfaces the wrapper, which is what lets the restore UI show
    when a backup was taken and under what label.

    retire when: an integration case lists checkpoints over the API and asserts the
    metadata reaches the caller.
    """
    text = _read("core/Ui/Misc/BackupManager.cpp")

    assert "kBackupMetaKey" in text
    assert re.search(
        r'insert\(QStringLiteral\("backupMeta"\)', text
    ), "the summary must expose the wrapper to the restore UI"


def test_backup_manager_snapshots_parser_only_edits_on_empty_project():
    """A parser-only edit on a structurally empty project (no groups/datasets) must still
    reach the snapshot path. setModified() suppresses the dirty flag there, so it emits
    contentTouched instead, and BackupManager arms the debounce off that signal. The whole-
    project hash in snapshot() is the sole arbiter of whether anything is actually written.

    retire when: an integration case edits only the frame parser of an empty project and
    asserts a checkpoint appears.
    """
    model_h = _read("core/Pipeline/DataModel/ProjectModel.h")
    model_cpp = _read("core/Pipeline/DataModel/ProjectModel.cpp")

    # The decoupling signal exists and fires from the empty-project branch of setModified().
    assert "void contentTouched();" in model_h
    setmod = re.search(
        r"void DataModel::ProjectModel::setModified\(const bool modified\)[\s\S]*?\n\}",
        model_cpp,
    )
    assert setmod is not None
    body = setmod.group(0)
    assert re.search(
        r"m_groups\.empty\(\)[\s\S]*?Q_EMIT contentTouched\(\);[\s\S]*?return;",
        body,
    ), "setModified must emit contentTouched in the empty-project branch"

    # BackupManager listens to it and arms the debounce (hash-only decision in flushDebounced).
    backup_cpp = _read("core/Ui/Misc/BackupManager.cpp")
    assert (
        "ProjectModel::contentTouched" in backup_cpp
    ), "BackupManager must arm off contentTouched, not the dirty flag"
    slot = re.search(
        r"void Misc::BackupManager::onProjectContentTouched\(\)[\s\S]*?\n\}",
        backup_cpp,
    )
    assert slot is not None
    assert "start();" in slot.group(0), "the debounce must be armed from that signal"


def test_crash_tracker_documents_local_only_telemetry():
    """CrashTracker has no outbound transport, and the header says so in its @brief so a
    reviewer sees the contract before adding one.

    retire when: never, while the promise stands -- no runtime test can observe "no
    telemetry was added". This is a policy scan, deliberately.
    """
    text = _read("core/Ui/Misc/CrashTracker.h")

    assert "telemetry" in text, "the header @brief must carry the no-telemetry contract"

    # No network / outbound transport headers exist in the cpp.
    cpp = _read("core/Ui/Misc/CrashTracker.cpp")
    for forbidden in ("QNetworkAccessManager", "QTcpSocket", "QUdpSocket", "curl_easy"):
        assert (
            forbidden not in cpp
        ), f"CrashTracker.cpp introduced a network sink: {forbidden}"


# ----------------------------------------------------------------------------------
# R8 -- legacy "JSON Projects" migration is copy + marker, never destructive move
# ----------------------------------------------------------------------------------


def test_workspace_migration_copies_with_atomic_marker():
    """The legacy "JSON Projects" folder is COPIED, never moved or removed: a failed
    migration must leave the user's originals where they were. The completion marker is
    written only after every copy succeeded, and a present marker skips the migration.

    retire when: a ctest runs migrateLegacyProjectsFolder() against a temp workspace,
    including a failure injected mid-copy.
    """
    text = _read("core/Core/WorkspaceManager.cpp")

    body = re.search(
        r"void Misc::WorkspaceManager::migrateLegacyProjectsFolder\(\)[\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    # Nothing destructive touches the legacy tree.
    for destructive in ("QFile::rename(", ".rmdir(", ".removeRecursively("):
        assert destructive not in snippet, f"the migration must not {destructive}"
    assert "QFile::copy(" in snippet, "files are copied, not moved"

    # The marker file path is persisted on disk; changing it re-runs the migration.
    assert '".migrated-from-json-projects"' in text

    fail_pos = snippet.find("if (!allCopied)")
    marker_write = re.search(r"QFile \w+\(marker\);", snippet)
    flag_pos = snippet.rfind("kMigratedKey")
    assert fail_pos != -1 and marker_write is not None and flag_pos != -1
    marker_pos = marker_write.start()
    assert (
        fail_pos < marker_pos < flag_pos
    ), "marker write must come after the all-copies check and before the settings flag"

    # Re-entry skip: a present marker short-circuits the migration on next launch.
    assert re.search(r"QFileInfo::exists\(marker\)", text)


# ----------------------------------------------------------------------------------
# R9 -- FrameParserPipeline is the single bytes-to-channels seam (hotpath + dialog + dryRun)
# ----------------------------------------------------------------------------------


def test_frame_builder_uses_shared_seam_not_inline_decoder_switch():
    """FrameBuilder::decodeProjectChannels delegates to the shared decoder seam. A
    re-inlined per-decoder switch here means the hotpath has drifted from the
    dialog/dryRun path again, which is how the two last diverged.

    retire when: tst_cframe_parser.cpp / tst_script_frame_shaping.cpp grow a case that
    decodes the same bytes through both the builder and the dryRun runner and compares.
    """
    text = _read("core/Pipeline/DataModel/FrameBuilder.cpp")

    # decodeProjectChannels delegates to the seam.
    body = re.search(
        r"void DataModel::FrameBuilder::decodeProjectChannels\([\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    assert (
        "decodeAndParseFrame(" in snippet
    ), "FrameBuilder must route through decodeAndParseFrame"
    # No re-inlined per-decoder switch (which would have one of these case statements).
    assert "case SerialStudio::Hexadecimal:" not in snippet
    assert "case SerialStudio::Base64:" not in snippet
    assert "case SerialStudio::Binary:" not in snippet

    # Player short-circuit also goes through the shared replay splitter (the
    # quote-aware twin of the QuickPlot split, declared in FrameParserPipeline.h).
    assert (
        "splitReplayChannels(" in snippet
    ), "Player playback must reuse the shared replay splitter."


def test_frame_builder_quick_plot_uses_shared_splitter():
    """parseQuickPlotFrame relies on the shared splitter, so dialog / dryRun split bytes
    the same way the live QuickPlot mode does.

    retire when: a ctest splits the same QuickPlot frame through the builder and through
    the pipeline runner and compares the rows.
    """
    text = _read("core/Pipeline/DataModel/FrameBuilder.cpp")

    body = re.search(
        r"void DataModel::FrameBuilder::parseQuickPlotFrame\([\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    assert "splitQuickPlotChannels(" in snippet
    # The retired inline helper must not have been added back.
    assert "parseCsvValues" not in text


def test_frame_parser_pipeline_dispatches_quick_plot_branch():
    """Both runners comma-split on QuickPlot and invoke the parser only in the other
    modes: compiling an engine for QuickPlot is what made the dryRun diverge.

    retire when: an integration case dry-runs a QuickPlot frame with a deliberately
    broken parser and asserts the rows still come out.
    """
    text = _read("core/Pipeline/DataModel/Scripting/FrameParserPipeline.cpp")

    helper = re.search(
        r"DataModel::PipelineFrame buildPipelineFrame\([\s\S]*?\n\}",
        text,
    )
    assert helper is not None
    assert re.search(
        r"operationMode == SerialStudio::QuickPlot\)[\s\S]{0,120}?splitQuickPlotChannels\(",
        helper.group(0),
    ), "QuickPlot must short-circuit to the shared splitter"

    code_runner = re.search(
        r"DataModel::PipelineResult DataModel::runFrameParserPipelineWithCode\([\s\S]*?\n\}",
        text,
    )
    assert code_runner is not None
    assert re.search(
        r"\w+ = \(spec\.operationMode != SerialStudio::QuickPlot\);",
        code_runner.group(0),
    ), "the dryRun runner must skip engine compilation for QuickPlot"


# ----------------------------------------------------------------------------------
# R10 -- dryRun + dispatcher require bytes; parser-only fallback is gone
#
# The dryRun half of R10 is now behavioural: tests/integration/test_frame_parsing.py
# drives project.frameParser.dryRun over the API and asserts the refusal and the
# QuickPlot defaults. Only the AI-side routing, which has no API surface of its own,
# is still guarded here.
# ----------------------------------------------------------------------------------


def test_tool_dispatcher_routes_pipeline_inputs():
    """ToolDispatcher.frameParserDryRunCommand forwards the pipeline keys when bytes are
    supplied, and falls back to dryCompile when they are not: routing a byte-less call to
    dryRun makes the assistant see a MissingParam it cannot act on.

    retire when: an integration case drives the assistant tool lane end to end (the
    dispatcher is not reachable from the command API).
    """
    text = _read("core/Ui/AI/ToolDispatcher.cpp")

    body = re.search(
        r"static QString frameParserDryRunCommand\([\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    # Either KeyView("foo") / "foo" literal or Keys::Symbol is acceptable.
    for literal, symbol in (
        ('"inputBytes"', None),
        ('"inputBytesHex"', None),
        ('"decoderMethod"', "Keys::DecoderMethod"),
        ('"frameDetection"', "Keys::FrameDetection"),
        ('"frameStart"', "Keys::FrameStart"),
        ('"frameEnd"', "Keys::FrameEnd"),
        ('"hexadecimalDelimiters"', "Keys::HexadecimalDelimiters"),
        ('"checksumAlgorithm"', "Keys::ChecksumAlgorithm"),
        ('"operationMode"', None),
    ):
        ok = literal in snippet
        if not ok and symbol:
            ok = symbol in snippet
        assert ok, f"pipeline key {literal} must be forwarded"

    # No more sampleFrame / sampleFrames routing through this command.
    assert "sampleFrame" not in snippet
    assert "sampleFrames" not in snippet

    # The fallback for "no bytes" is dryCompile.
    assert 'return QStringLiteral("project.frameParser.dryCompile");' in snippet


def test_assistant_script_apply_strips_pipeline_keys():
    """assistant.script.apply strips the dryRun-only pipeline keys before forwarding to
    setCode, which rejects unknown params with InvalidParam.

    retire when: an integration case posts an assistant.script.apply carrying the dryRun
    keys and asserts the code lands.
    """
    text = _read("core/Ui/ApiHandlers/AssistantHandler.cpp")

    # The fallback is keyed off inputBytes(Hex), not the old sampleFrame(s) check.
    assert re.search(
        r'contains\(QStringLiteral\("inputBytes"\)\)[\s\S]{0,160}?'
        r'contains\(QStringLiteral\("inputBytesHex"\)\)',
        text,
    ), "the frame_parser fallback must key off the pipeline byte params"

    # Either QStringLiteral("key") or Keys::SymbolName form is acceptable for the strip.
    for literal, symbol in (
        ('"inputBytes"', None),
        ('"inputBytesHex"', None),
        ('"decoderMethod"', "Keys::DecoderMethod"),
        ('"frameDetection"', "Keys::FrameDetection"),
        ('"frameStart"', "Keys::FrameStart"),
        ('"frameEnd"', "Keys::FrameEnd"),
        ('"hexadecimalDelimiters"', "Keys::HexadecimalDelimiters"),
        ('"checksumAlgorithm"', "Keys::ChecksumAlgorithm"),
        ('"operationMode"', None),
    ):
        ok = f"setParams.remove(QStringLiteral({literal}))" in text
        if not ok and symbol:
            ok = f"setParams.remove({symbol})" in text
        assert ok, f"{literal} must be stripped before setCode"


# ----------------------------------------------------------------------------------
# R11 -- the parser tester bridge drives the shared pipeline and writes back to the source
# ----------------------------------------------------------------------------------


def test_tester_runs_pipeline_and_writes_back_to_source():
    """The per-source frame parser bridge (FrameParserModel) calls the shared pipeline
    runners and writes delimiter / decoder / detection / checksum edits back to
    ProjectModel, so the live driver reconfigures instead of testing against stale
    settings.

    retire when: an integration case edits a delimiter through the parser tester and
    asserts the project source reports the new value.
    """
    text = _read("core/Ui/ProjectEditor/Editors/FrameParserModel.cpp")

    # Both pipeline runners are exercised from dryRun: the live engine for JS/Lua
    # sources and the throwaway native-template engine for Native sources.
    assert "runFrameParserPipeline(" in text
    assert "runNativeTemplatePipeline(" in text

    # All pipeline setters write back through updateSource.
    for slot in (
        "setDecoderIndex",
        "setDetectionIndex",
        "setChecksumIndex",
        "setFrameStart",
        "setFrameEnd",
        "setHexDelimiters",
    ):
        body = re.search(
            r"void DataModel::FrameParserModel::" + slot + r"\([\s\S]*?\n\}",
            text,
        )
        assert body is not None, f"{slot} body must exist"
        assert "updateSource(" in body.group(
            0
        ), f"{slot} must persist edits via updateSource"

    # Bridge reacts to live source mutations so it stays in sync with external edits.
    assert "&ProjectModel::sourceChanged" in text


# ----------------------------------------------------------------------------------
# R12 -- ProtoImporter parseMsg uses the correct endPos parameter, not a stray endP
# ----------------------------------------------------------------------------------


def test_proto_importer_parse_msg_uses_endpos_not_endp():
    """parseMsg's outer loop ran its post-iteration guard against an undefined `endP`
    instead of its `endPos` parameter. Lua raised on the very first comparison and
    the pcall in parse() swallowed the error, so the script only ever populated the
    first field of any frame -- which surfaces as 'the dashboard only shows one
    value per sensor message' in the Protobuf example.

    The fix is a one-character rename. This test pins the loop guard to endPos.
    scoreDispatcher is a separate function with its OWN local endP, so we make
    sure we did not accidentally rename that one too.

    retire when: a ctest EXECUTES the generated Lua over a fixture payload and asserts
    every field decodes -- tst_proto_importer.cpp only checks the parser is attached.
    """
    text = _read("core/Pipeline/DataModel/Importers/ProtoImporter.cpp")

    # emitDecoderParseMsg emits a loop guard bound to its endPos parameter.
    parse_msg = re.search(
        r"void DataModel::ProtoImporter::emitDecoderParseMsg\([\s\S]*?\n\}",
        text,
    )
    assert parse_msg is not None
    snippet = parse_msg.group(0)
    assert re.search(
        r"p > endPos then break end", snippet
    ), "parseMsg loop guard must use endPos (not endP)"
    assert not re.search(
        r"p > endP then break end", snippet
    ), "parseMsg must not regress to the broken endP name"

    # emitScoreDispatcher legitimately declares its own local endP -- keep that as-is.
    score = re.search(
        r"void DataModel::ProtoImporter::emitScoreDispatcher\([\s\S]*?\n\}",
        text,
    )
    assert score is not None
    assert "local endP = bufLen + 1" in score.group(0)
    assert "p > endP then break end" in score.group(0)


def test_protobuf_example_embedded_parser_was_repaired():
    """The shipped Protobuf example carries a regenerated parser script in its .ssproj,
    so a user running the example out of the box sees every dataset light up rather than
    the first field of each message.

    retire when: an integration case loads the example and asserts its dataset count from
    a replayed frame.
    """
    text = _read("examples/Protobuf Example/Protobuf Example.ssproj")

    assert (
        "p > endPos then break end" in text
    ), "the example's parseMsg guard must carry the endPos fix"
    # scoreDispatcher's legitimate local endP must still be there.
    assert "local endP = bufLen + 1" in text


# ----------------------------------------------------------------------------------
# R13 -- Dashboard preserves in-flight time-ring data across project mutations
# ----------------------------------------------------------------------------------


def test_dashboard_snapshots_around_every_clearing_trigger():
    """Project mutations (frame detection change, dataset rename, plot min/max edit) used
    to wipe every plot until the next reconnect, because the rebuild runs
    resetData(false) and that clears every time ring. Each clearing site snapshots the
    rings before the rebuild and restores them after.

    The restore itself is covered behaviourally by
    app/tests/tst_replay_seek_engine.cpp; what has no other enforcer is that the
    Dashboard call sites still bracket their rebuild with it.

    retire when: the Dashboard facade becomes constructible under ctest, so the rebuild
    can be driven directly and the ring contents asserted after it.
    """
    text = _read("core/Ui/UI/Dashboard.cpp")

    # reconfigureDashboard: snapshot before resetData, restore after updateDataSeries.
    body = re.search(
        r"void UI::Dashboard::reconfigureDashboard[\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)
    snap_pos = snippet.find("snapshotPlotTimeRings()")
    reset_pos = snippet.find("resetData(false);")
    update_pos = snippet.find("updateDataSeries();")
    restore_pos = snippet.find("restorePlotTimeRings(")
    assert snap_pos != -1 and reset_pos != -1
    assert update_pos != -1 and restore_pos != -1
    assert (
        snap_pos < reset_pos < update_pos < restore_pos
    ), "reconfigureDashboard must snapshot before resetData and restore after updateDataSeries"

    # setPlotTimeRange and setPoints share one rebuild helper (spec 0075 F4) that snapshots
    # before configureLineSeries / configureMultiLineSeries and restores after both.
    helper_body = re.search(
        r"void UI::Dashboard::rebuildLineSeriesPreservingState[\s\S]*?\n\}",
        text,
    )
    assert helper_body is not None
    helper = helper_body.group(0)
    assert helper.index("snapshotPlotTimeRings") < helper.index("configureLineSeries")
    assert helper.index("configureMultiLineSeries") < helper.index(
        "restorePlotTimeRings"
    )
    assert (
        "restorePlotSweepConfig(" in helper
    ), "the sweep configuration is part of the same preserved state"

    for fn in ("setPlotTimeRange", "setPoints"):
        body = re.search(r"void UI::Dashboard::%s\b[\s\S]*?\n\}" % fn, text)
        assert body is not None, fn
        assert "rebuildLineSeriesPreservingState();" in body.group(0), fn


# ----------------------------------------------------------------------------------
# R14 -- Modbus map importer: a bool on a holding/input register decodes the whole
#        16-bit word, not a packed coil bit (PLC e-stop LED never lit otherwise)
# ----------------------------------------------------------------------------------


def test_modbus_register_bool_decodes_whole_word():
    """A holding/input-register bool used to be funneled to the packed-bit path, which
    reads one LSB-packed bit out of the byte stream -- correct for coils/discrete
    inputs (register type >= 2) but wrong for holding/input registers, whose response
    is 2-byte big-endian words. For E-Stop at register offset 0, the bit path read the
    register's HIGH byte (always 0x00) and the LED never turned on.

    The decode now lives in the Lua parser generated by the Modbus map importer: a
    register-block bool decodes the whole 16-bit word with 0/1 truthiness, and only
    coil/discrete blocks use the packed-bit path.

    retire when: a ctest runs the map importer's generated Lua over a holding-register
    reply and asserts the bool dataset -- tst_modbus_generation.cpp covers the DRIVER's
    generator, which is a different one.
    """
    text = _read("core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp")

    # The generated Lua decodes a register-block bool as whole-word truthiness.
    assert (
        "(raw ~= 0) and 1 or 0" in text
    ), "the generated Lua must decode bool registers as 0/1 word truthiness"

    # Only bit blocks (coils/discrete) force the packed-bit path; a bool on a register
    # block keeps its "bool" spec type, which is the whole-word decode.
    assert re.search(
        r'if \(bitBlock\)\s*return QStringLiteral\("bit"\);', text
    ), "luaEntryType must reserve the packed-bit path for coil/discrete blocks"
    assert re.search(
        r"bool\s*=\s*2,", text
    ), "the generated Lua SIZES table must decode bool as a full 2-byte word"
    assert (
        'string.unpack("' not in text
    ), "the generated Lua must not call string.unpack (absent in LuaJIT)"


def _decode_modbus_reg_bool(reg_value: int) -> int:
    """Mirror of the fixed RegBool path: read the full 16-bit register, report 0/1."""
    hi = (reg_value >> 8) & 0xFF
    lo = reg_value & 0xFF
    frame = bytes([0x01, 0x03, 0x02, hi, lo])  # [slave, FC03, byteCount, data...]
    byte_off = 3  # reg_offset 0 -> 3 + 0 * 2
    raw = (frame[byte_off] << 8) | frame[byte_off + 1]
    return 1 if raw != 0 else 0


def _decode_modbus_old_bit(reg_value: int) -> int:
    """Mirror of the OLD buggy decodeBit() path for a holding-register bool."""
    hi = (reg_value >> 8) & 0xFF
    lo = reg_value & 0xFF
    frame = bytes([0x01, 0x03, 0x02, hi, lo])
    byte_idx = 3 + (0 // 8)  # reg_offset 0 as a bit index
    return (frame[byte_idx] >> (0 % 8)) & 0x01


def test_modbus_register_bool_roundtrip_lights_estop():
    """Behavioural check: an E-Stop register reading 1 must decode to 1 (LED on),
    and 0 to 0. The old packed-bit path returns 0 for value 1 -- the actual bug.

    retire when: the ctest above exists -- this mirrors the algorithm in Python, which
    proves the decode is right but not that the shipped Lua implements it.
    """
    assert _decode_modbus_reg_bool(0) == 0
    assert _decode_modbus_reg_bool(1) == 1
    assert _decode_modbus_reg_bool(0xFF00) == 1  # any nonzero word is "on"

    # Prove the test discriminates the bug: the old path mis-reads value 1 as 0.
    assert _decode_modbus_old_bit(1) == 0
    assert _decode_modbus_reg_bool(1) != _decode_modbus_old_bit(1)


# ----------------------------------------------------------------------------------
# R15 -- DBC importer: Motorola (big-endian) signals decode correctly. Two bugs --
#        an inverted endian flag and a non-sawtooth big-endian bit walk.
# ----------------------------------------------------------------------------------


def test_dbc_importer_endian_flag_not_inverted():
    """The DBC importer once emitted bigEndian = (dataEndian == LittleEndian), an
    inversion. Qt reports DBC @0 (Motorola) as QSysInfo::BigEndian and @1 (Intel) as
    LittleEndian, verbatim -- so the flag was backwards for BOTH endiannesses and
    every signal hit the wrong extractor branch. The flag now feeds the generated
    Lua spec's `be` field via signalSpecLine().

    retire when: tst_dbc_importer.cpp grows a Motorola fixture -- its
    generatedLuaMatchesRangesAtRuntime() covers Intel signals only.
    """
    text = _read("core/Pipeline/DataModel/Importers/DBCImporter.cpp")

    spec_line = re.search(r"DBCImporter::signalSpecLine[\s\S]*?\n\}", text)
    assert spec_line is not None, "signalSpecLine must emit the Lua signal spec"
    assert "signal.dataEndian() == QSysInfo::BigEndian" in spec_line.group(
        0
    ), "the Lua spec's `be` flag must compare against BigEndian"
    assert "QSysInfo::LittleEndian" not in spec_line.group(
        0
    ), "signalSpecLine must not regress to the inverted LittleEndian compare"


def test_dbc_extract_signal_uses_motorola_sawtooth():
    """The big-endian extractor once walked bit_pos = start_bit + i monotonically,
    which is not the DBC Motorola layout. The fix steps the in-byte bit index DOWN
    and jumps +15 across byte boundaries (Qt's sawtooth). The extractor now lives in
    the Lua parser generated by the DBC importer.

    retire when: tst_dbc_importer.cpp grows a Motorola fixture and runs the generated
    Lua against the expected values (test_dbc_motorola_sawtooth_matches_qt_doc_example
    below proves the algorithm, not the generated code).
    """
    text = _read("core/Pipeline/DataModel/Importers/DBCImporter.cpp")

    # The sawtooth step is the heart of the fix (Lua spelling).
    assert (
        "(bit_pos + 15) or (bit_pos - 1)" in text
    ), "the big-endian walk must step down in-byte and jump +15 across bytes"
    # The old complemented shift of the monotonic walk must be gone.
    assert "7 - (bit_pos % 8)" not in text


def _extract_be(frame: bytes, start_bit: int, length: int) -> int:
    """Mirror of the fixed Motorola (big-endian) sawtooth extractor."""
    value = 0
    bit_pos = start_bit
    for _ in range(length):
        byte_idx = bit_pos // 8
        if byte_idx < len(frame):
            bit = (frame[byte_idx] >> (bit_pos % 8)) & 1
            value = (value << 1) | bit
        bit_pos = (bit_pos + 15) if (bit_pos % 8 == 0) else (bit_pos - 1)
    return value


def _extract_be_old(frame: bytes, start_bit: int, length: int) -> int:
    """Mirror of the OLD buggy monotonic big-endian walk."""
    value = 0
    for i in range(length):
        bit_pos = start_bit + i
        byte_idx = bit_pos // 8
        if byte_idx >= len(frame):
            continue
        bit = (frame[byte_idx] >> (7 - (bit_pos % 8))) & 1
        value = (value << 1) | bit
    return value


def test_dbc_motorola_sawtooth_matches_qt_doc_example():
    """Qt's QCanSignalDescription docs give a canonical big-endian example: two
    12-bit values in a 3-byte payload as signal1(startBit=7,len=12) and
    signal2(startBit=11,len=12). Pack 0xABC and 0x123 per the sawtooth and confirm
    both decode back. Also pin a byte0-MSB 16-bit signal (7|16@0) of 0x1234.

    retire when: tst_dbc_importer.cpp grows the same fixture against the generated Lua.
    """

    def build_be(frame_len, placements):
        bits = [0] * (frame_len * 8)
        for start_bit, length, value in placements:
            bp = start_bit
            for i in range(length):
                bits[bp] = (value >> (length - 1 - i)) & 1
                bp = (bp + 15) if (bp % 8 == 0) else (bp - 1)
        frame = bytearray(frame_len)
        for idx, b in enumerate(bits):
            if b:
                frame[idx // 8] |= 1 << (idx % 8)
        return bytes(frame)

    frame = build_be(3, [(7, 12, 0xABC), (11, 12, 0x123)])
    assert _extract_be(frame, 7, 12) == 0xABC
    assert _extract_be(frame, 11, 12) == 0x123

    f16 = build_be(8, [(7, 16, 0x1234)])
    assert f16[0] == 0x12 and f16[1] == 0x34  # byte 0 is the MSB
    assert _extract_be(f16, 7, 16) == 0x1234

    # The old monotonic walk does not reproduce these layouts.
    assert _extract_be_old(f16, 7, 16) != 0x1234


# ----------------------------------------------------------------------------------
# R16 -- CAN Bus example DBC matches its little-endian simulator wire format
# ----------------------------------------------------------------------------------


def test_can_example_dbc_is_intel_endian():
    """ecu_simulator.dbc declared every signal @0 (Motorola) but ecu_simulator.py
    packs multi-byte signals with struct.pack('<...') (Intel). The DBC was flipped
    to @1 so the shipped example is internally consistent. Guard against any @0
    creeping back into a signal line.

    retire when: an integration case replays the example's simulator output through the
    example project and asserts the decoded values.
    """
    dbc = _read("examples/CAN Bus Example/ecu_simulator.dbc")

    # Real signal definitions are "SG_ <name> : <bits>@<order><sign> ..." -- the
    # NS_ header's bare "SG_MUL_VAL_" symbol token must not be mistaken for one.
    signal_lines = [ln for ln in dbc.splitlines() if re.match(r"\s*SG_\s+\w+\s*:", ln)]
    assert signal_lines, "expected SG_ signal definitions in the example DBC"

    motorola = [ln for ln in signal_lines if re.search(r"@0[+-]", ln)]
    assert not motorola, f"example DBC must be Intel (@1), found @0 in: {motorola}"
    assert all(re.search(r"@1[+-]", ln) for ln in signal_lines)


# ----------------------------------------------------------------------------------
# R17 -- CAN driver publishes full 29-bit extended ids (flagged 4-byte header)
# ----------------------------------------------------------------------------------


def test_can_driver_extended_id_header():
    """The driver used to publish only the low 16 bits of every CAN id, so 29-bit
    extended ids aliased and could never be decoded. Standard frames keep the legacy
    [ID_hi, ID_lo, DLC, ...] layout (11-bit ids, byte 0 top bit always clear);
    extended frames now carry the full id as [0x80|ID28..24, ID23..16, ID15..8,
    ID7..0, DLC, ...], write() mirrors the header, and the DBC importer's generated
    Lua decodes both forms.

    retire when: a ctest publishes a 29-bit frame through the CAN driver's plugin path
    and asserts the emitted header -- tst_serial_can_backend.cpp covers the SLCAN and
    Seeed backends, which build their frames elsewhere.
    """
    text = _read("core/Devices/IO/Drivers/CANBus.cpp")

    # RX: the id is masked per frame format and the extended header is flagged.
    assert "hasExtendedFrameFormat()" in text
    assert (
        "0x1FFFFFFF" in text
    ), "extended ids must be masked to 29 bits, not truncated to 16"
    assert re.search(
        r"0x80 \| \(\(\w+ >> 24\) & 0x1F\)", text
    ), "the extended header must carry the flag plus the top id bits"

    # TX: bit 7 of byte 0 selects the 4-byte id form.
    assert re.search(
        r"\(static_cast<quint8>\(data\[0\]\) & 0x80\)", text
    ), "write() must read the same extended-header flag"

    # The generated DBC Lua decodes both header forms and keys on 29-bit ids.
    dbc = _read("core/Pipeline/DataModel/Importers/DBCImporter.cpp")
    assert "local function frame_id(frame)" in dbc
    # Spec 0051 migrated the generator off 5.3 bitwise syntax: the 29-bit id is now
    # assembled with bit.band + arithmetic shifts (values exceed bit.*'s 32-bit range).
    assert "16777216" in dbc, "the 29-bit id must be assembled arithmetically"
    assert (
        "bit.band(b1, 0x80)" in dbc
    ), "the generated Lua must test the extended-header flag with bit.band, not 5.3 `&`"


def _encode_can_header(can_id: int, extended: bool, payload: bytes) -> bytes:
    """Mirror of the driver's publish header (incl. the fixed-size zero padding)."""
    if extended:
        head = bytes(
            [
                0x80 | ((can_id >> 24) & 0x1F),
                (can_id >> 16) & 0xFF,
                (can_id >> 8) & 0xFF,
                can_id & 0xFF,
                len(payload),
            ]
        )
        pad = 13
    else:
        head = bytes([(can_id >> 8) & 0xFF, can_id & 0xFF, len(payload)])
        pad = 11

    data = head + payload
    return data + bytes(max(0, pad - len(data)))


def _decode_can_header(data: bytes):
    """Mirror of the generated Lua frame_id(): returns (id, dlc_index)."""
    if data[0] & 0x80:
        cid = ((data[0] & 0x1F) << 24) | (data[1] << 16) | (data[2] << 8) | data[3]
        return cid, 4
    return ((data[0] << 8) | data[1]), 2


def test_can_extended_id_roundtrip():
    """A 29-bit id must round-trip through the wire header; the old 16-bit
    truncation aliased it. Standard frames keep the legacy 2-byte header.

    retire when: the ctest above exists -- this mirrors the header codec in Python.
    """
    eid = 0x18FF50E5  # typical J1939 extended id
    frame = _encode_can_header(eid, True, bytes([1, 2, 3, 4]))
    cid, dlc_index = _decode_can_header(frame)
    assert cid == eid
    assert dlc_index == 4 and frame[dlc_index] == 4
    assert cid != (eid & 0xFFFF)  # the old truncation provably loses the id

    sid = 0x1A0
    frame = _encode_can_header(sid, False, bytes([9]))
    cid, dlc_index = _decode_can_header(frame)
    assert cid == sid
    assert dlc_index == 2 and frame[dlc_index] == 1
    assert len(frame) == 11  # legacy padding preserved for standard frames


# ----------------------------------------------------------------------------------
# R18 -- AI conversation history keeps the tool_use/tool_result pairing invariant
# ----------------------------------------------------------------------------------


def test_ai_history_sanitizer_strips_orphan_tool_results():
    """The Anthropic API rejects the whole request (400: unexpected tool_use_id) when a
    tool_result block has no matching tool_use in the immediately preceding assistant
    message. Orphans were created by interrupted tool batches (reply error with an async
    tool still in flight), stale confirmation dialogs, and history pruning cutting
    between a tool_use and its results. The fixes: a bidirectional sanitizer that strips
    orphan results before every send, prune running BEFORE reconcile, a no-turn-in-flight
    guard on recordToolResult, and confirmation state cleared on reply errors.

    HistorySurgery itself is covered behaviourally by app/tests/tst_conversation_history.cpp
    (stripOrphans_*, firstFreshUserTurn_*, prune_*, reconcile_*). What has no other
    enforcer is the Conversation-side call ORDER and the two lifecycle guards, which sit
    on a class no ctest can construct.

    retire when: Conversation's request lifecycle becomes drivable under ctest with a
    stub provider.
    """
    text = _read("core/Ui/AI/Conversation.cpp")

    # Prune must run before reconcile so a prune cut cannot ship unpaired blocks.
    issue = re.search(
        r"void AI::Conversation::issueRequest\(\)[\s\S]*?m_provider->sendMessage", text
    )
    assert issue is not None
    snippet = issue.group(0)
    assert snippet.index("pruneHistory();") < snippet.index(
        "reconcileHistoryToolPairs();"
    ), "issueRequest must prune before reconciling tool pairs"

    # Late async tool completions (after error/cancel) must not record results.
    record = re.search(
        r"void AI::Conversation::recordToolResult\([\s\S]*?\n\{[\s\S]*?\n  if \(!m_busy\)",
        text,
    )
    assert (
        record is not None
    ), "recordToolResult must drop results when no turn is in flight"

    # Reply errors must clear pending confirmations so a later approval can't resume.
    # The table itself moved into ToolTurnRunner (spec 0075 J7); the rule is the same.
    error_fn = re.search(r"void AI::Conversation::onReplyError[\s\S]*?\n\}", text)
    assert error_fn is not None
    assert "m_tools.clearPending();" in error_fn.group(0)


# ----------------------------------------------------------------------------------
# R19 -- control-script lifecycle is per-connection; agent surface stays discoverable
# ----------------------------------------------------------------------------------


def test_control_script_connection_lifecycle():
    """A connect/disconnect/connect cycle must give the control script a fresh engine and
    fresh latest-frame data. Regressions here are silent: a stale engine keeps watchdog
    state (false 'comm loss' alarms after reconnect), a worker/GUI running-flag desync
    keeps an old engine alive, and a retained pre-disconnect frame leaks into the new
    connection's io.getLatestFrame when the API server keeps the capture flag on.

    retire when: an integration case cycles connect/disconnect/connect against the
    device simulator and asserts the control script's watchdog state and the latest-frame
    store both come back empty.
    """
    cs = _read("core/Pipeline/DataModel/Scripting/ControlScript.cpp")

    # Rising edge force-restarts (stop then start) via edge tracking.
    on_changed = re.search(
        r"void DataModel::ControlScript::onConnectedChanged\(\)[\s\S]*?\n\}", cs
    )
    assert on_changed is not None
    body = on_changed.group(0)
    assert body.index("stopWorker();") < body.index(
        "startWorker();"
    ), "a rising edge must stop the old worker before starting a fresh one"

    # stopWorker always queues the idempotent worker stop (no running-flag early return).
    stop_fn = re.search(
        r"void DataModel::ControlScript::stopWorker\(\)[\s\S]*?\n\}", cs
    )
    assert stop_fn is not None
    assert re.search(
        r'invokeMethod\(m_worker, "stop"', stop_fn.group(0)
    ), "stopWorker must always queue the worker stop"
    assert not re.search(
        r"if \(!m_running\)\s*\n\s*return;", stop_fn.group(0)
    ), "a GUI/worker flag desync must not skip the stop"

    # A setup() exception stops the worker so the loop never arms while the GUI shows
    # the script as stopped.
    worker = _read("core/Pipeline/DataModel/Scripting/ControlScriptWorker.cpp")
    setup_err = re.search(r"setup\(\) line %1: %2[\s\S]*?\n  \}", worker)
    assert setup_err is not None and "stop();" in setup_err.group(0)

    # FrameBuilder clears the latest-frame store on BOTH connection edges, through a
    # helper that also advances the sequence so the GUI mirror republishes the empty map.
    fb = _read("core/Pipeline/DataModel/FrameBuilder.cpp")
    connected = re.search(
        r"void DataModel::FrameBuilder::onConnectedChanged\(\)[\s\S]*?\n\}", fb
    )
    assert connected is not None
    assert (
        connected.group(0).count("clearLatestFrames();") >= 2
    ), "both connection edges must clear the latest-frame store"

    clear_fn = re.search(
        r"void DataModel::FrameBuilder::clearLatestFrames\(\)[\s\S]*?\n\}", fb
    )
    assert clear_fn is not None
    assert "m_latestFrames.clear();" in clear_fn.group(0)
    assert re.search(
        r"\+\+m_latestFrameSeq;", clear_fn.group(0)
    ), "the sequence must advance so the mirror republishes the empty map"


def test_control_script_agent_surface():
    """Agents find the control-script commands by their conventional names, can validate
    before committing, and can fetch the focused runtime reference. A command that exists
    but is undiscoverable is, to an agent, a command that does not exist.

    retire when: an integration case asserts these commands appear in the tools listing
    and that the doc kind resolves (tests/integration/test_mcp.py is the model).
    """
    handler = _read("core/Api/API/Handlers/ControlScriptHandler.cpp")
    for cmd in (
        "controlScript.get",
        "controlScript.getCode",
        "controlScript.set",
        "controlScript.setCode",
        "controlScript.dryRun",
        "controlScript.getStatus",
    ):
        assert f'QStringLiteral("{cmd}")' in handler, f"missing command: {cmd}"

    # dryRun mirrors the worker: control flag + SDK prelude, watchdogged evaluation.
    assert "__ss_control" in handler
    assert ":/api/SerialStudio.js" in handler
    assert "JsWatchdog" in handler or "ScriptDryRun" in handler

    # The control_script_js doc kind is registered everywhere it must be: the
    # ContextBuilder doc roster (scriptingDocFor) and the meta.fetchScriptingDocs
    # surface, which moved from ToolDispatcher into Conversation.cpp (2026-07 AI
    # hardening refactor).
    assert "control_script_js" in _read("core/Ui/AI/ContextBuilder.cpp")
    assert "control_script_js" in _read("core/Ui/AI/Conversation.cpp")
    assert "ai/docs/control_script_js.md" in _read("app/rcc/rcc.qrc")
    doc = _read("app/rcc/ai/docs/control_script_js.md")
    assert "controlScript.dryRun" in doc and "ageMs" in doc

    # Safety tiers: reads + dryRun auto-execute; both install spellings always confirm.
    tiers = json.loads(_read("app/rcc/ai/command_safety.json"))
    assert "controlScript.dryRun" in tiers["safe"]
    assert "controlScript.getCode" in tiers["safe"]
    assert "controlScript.setCode" in tiers["alwaysConfirm"]
    assert "controlScript.set" in tiers["alwaysConfirm"]


# ----------------------------------------------------------------------------------
# R20 -- Modbus group attribution survives a failed poll, and the published bytes are
#        a real RTU frame (spec 0075 E3/E12)
# ----------------------------------------------------------------------------------


def test_modbus_failed_poll_keeps_group_attribution():
    """The generated Lua parser infers a reply's register group by counting frames, so a
    poll the driver silently skipped shifted every later frame onto the wrong group for
    the rest of the session -- a whole dashboard of plausible, wrong readings.

    The driver now publishes a zero-length placeholder [unit, fc, 0] for the failed
    group, and the generated parser skips a zero byte-count frame while still advancing
    the cycle. Old generated parsers see a zero-length payload and decode nothing, which
    is why no header byte was added.

    The generated parser's half is covered by app/tests/tst_modbus_generation.cpp
    (parserSkipsTheFailedPollPlaceholder, parserIdentifiesGroupsByCodeAndSize). What is
    guarded here is the DRIVER's obligation to publish the placeholder at all.

    retire when: a ctest drives the Modbus poll loop with a failing reply and asserts the
    published bytes.
    """
    driver = _read("core/Devices/IO/Drivers/Modbus.cpp")

    fn = re.search(
        r"void IO::Drivers::Modbus::advanceAfterFailedPoll\(\)[\s\S]*?\n\}", driver
    )
    assert (
        fn is not None
    ), "a failed reply must publish the placeholder instead of silently skipping"
    body = fn.group(0)
    assert "functionCodeForType" in body, "the placeholder must carry the group's fc"
    assert re.search(
        r"publishReceivedData\(", body
    ), "the placeholder must actually reach the pipeline"
    assert re.search(
        r"\+\+m_currentGroupIndex;", body
    ), "the cycle must advance so later frames keep their group"

    # Both failure exits go through it: a reply error and an empty/invalid data unit.
    on_ready = re.search(
        r"void IO::Drivers::Modbus::onReadReady\(\)[\s\S]*?\n\}", driver
    )
    assert on_ready is not None
    assert (
        on_ready.group(0).count("advanceAfterFailedPoll();") == 2
    ), "both failure exits must publish the placeholder"


def test_modbus_rtu_frames_carry_a_checksum_and_the_responding_unit():
    """buildRtuFrame published [slave, fc, byteCount, data...] with no CRC and echoed the
    REQUESTED unit id, so the bytes were a header-shaped fragment and a gateway's reply
    was labelled with the address the poll asked for rather than the one that answered.

    The CRC itself is covered by app/tests/tst_modbus_register_groups.cpp
    (theChecksumIsTheModbusCrc); what is guarded here is the driver appending it and
    reading the responding unit off the reply.

    retire when: a ctest builds an RTU frame from a stub reply with a differing server
    address and asserts the published bytes.
    """
    driver = _read("core/Devices/IO/Drivers/Modbus.cpp")

    fn = re.search(
        r"QByteArray IO::Drivers::Modbus::buildRtuFrame\([\s\S]*?\n\}", driver
    )
    assert fn is not None
    body = fn.group(0)
    assert body.count("appendCrc(") == 2, "both frame shapes need the CRC"
    assert "serverAddress" in body, "the responding unit id must reach the frame"
    assert re.search(
        r"buildRtuFrame\(\w+, \w+->serverAddress\(\)\)", driver
    ), "the caller must pass the address that answered, not the one polled"


def test_modbus_bit_reads_get_their_own_request_cap():
    """The poll-interval floor the driver advertises must match the UI's own validator
    (50 ms); advertising a lower floor the setter then clamps away made the spin box
    silently disagree with the driver.

    The request caps themselves are covered by app/tests/tst_modbus_register_groups.cpp
    (registerReadsStopAtTheWordCap, bitReadsGetTheirOwnCap).

    retire when: the property registry gains a bound check that both the QML validator
    and the driver property read from one place.
    """
    driver = _read("core/Devices/IO/Drivers/Modbus.cpp")
    poll = re.search(
        r"poll\.value = m_pollInterval;\s*\n\s*poll\.min\s*=\s*(\d+);", driver
    )
    assert poll is not None and poll.group(1) == "50"
    qml = _read("app/qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml")
    assert "IntValidator { bottom: 50; top: 60000 }" in qml


# ----------------------------------------------------------------------------------
# R21 -- OPC UA: Trust overrides a name/clock refusal, and a password never crosses a
#        clear channel unasked (spec 0075 E11)
# ----------------------------------------------------------------------------------


def test_opcua_trust_is_checked_after_the_clock_and_before_the_name():
    """verifyServerCertificate refused on expiry, then not-yet-valid, then hostname, and
    only reached the trust store last. A self-signed controller dialed by IP therefore
    reported HostnameMismatch forever: accepting it in the trust prompt changed nothing,
    because the next attempt was refused before the trust store was consulted.

    Trust pins the exact bytes by SHA-256, so it answers Good ahead of the hostname check.
    It stays BEHIND the validity window: trusting an expired certificate must not open the
    channel (spec 0067 AC11, and what the manual promises), so renewal is the only fix.

    app/tests/tst_opcua_security.cpp covers what inspect() reports; the REFUSAL ORDER
    lives in the session, which needs a live server to drive.

    retire when: a ctest exercises verifyServerCertificate() against a stubbed session
    state (expired + trusted, valid + untrusted + wrong host, and so on).
    """
    session = _read("core/Devices/IO/Drivers/OpcUaSession.cpp")
    fn = re.search(
        r"IO::Drivers::OpcUaTypes::StatusCode "
        r"IO::Drivers::OpcUaSession::verifyServerCertificate\([\s\S]*?\n\}",
        session,
    )
    assert fn is not None
    body = fn.group(0)

    trusted = body.index("m_serverCertificate.trusted")
    for earlier in (
        "m_serverCertificate.valid",
        "m_serverCertificate.expired",
        "m_serverCertificate.notYetValid",
    ):
        assert (
            body.index(earlier) < trusted
        ), f"{earlier} must be refused before trust is read"

    assert trusted < body.index(
        "m_serverCertificate.hostnameMatches"
    ), "trust must be read before the hostname check"


def test_opcua_plaintext_password_needs_an_explicit_grant():
    """allowNonePolicyPassword was set unconditionally for username mode, so every
    None-policy login shipped its password in the clear whether or not anyone agreed to
    it. It now follows a per-installation acknowledgement that starts off, and the
    acknowledgement is deliberately NOT a driver property: a security grant that
    travelled inside a project file would be given by opening the file.

    The acknowledgement's default is covered by app/tests/tst_opcua_security.cpp
    (plaintextPasswordIsOffUntilGranted). What is guarded here is the session reading it
    rather than hardcoding the allowance, and the grant staying out of the project file.

    retire when: a ctest builds the session config from an identity and asserts the flag
    follows the grant.
    """
    session = _read("core/Devices/IO/Drivers/OpcUaSession.cpp")
    assert re.search(
        r"allowNonePolicyPassword = \w+\.allowPlaintextPassword;", session
    ), "the session must read the grant, not decide it"
    assert (
        "allowNonePolicyPassword = true;" not in session
    ), "the plaintext allowance must never be hardcoded on"

    driver = _read("core/Devices/IO/Drivers/OpcUa.cpp")
    props = re.search(
        r"QList<IO::DriverProperty> IO::Drivers::OpcUa::driverProperties\(\)[\s\S]*?\n\}",
        driver,
    )
    assert props is not None
    assert "allowPlaintextPassword" not in props.group(0)


def test_opcua_write_reports_failure_like_its_siblings():
    """Read-only drivers return -1 from write() so a caller sees a hard failure; OPC UA
    returned 0, which reads as 'wrote nothing, no error'.

    retire when: a ctest asserts write() < 0 on each read-only driver.
    """
    header = (ROOT / "core/Devices/IO/Drivers/OpcUa.h").read_text(encoding="utf-8")
    fn = re.search(
        r"qint64 write\(const QByteArray& data\) override\s*\{[\s\S]*?\}", header
    )
    assert fn is not None and "return -1;" in fn.group(0)


# ----------------------------------------------------------------------------------
# R22 -- IEC 104 slot identity is (address, type id), and the credential store is
#        never called encrypted (spec 0075 E16, E14/K6)
# ----------------------------------------------------------------------------------


def test_iec104_slots_are_keyed_by_address_and_type():
    """The point table was keyed on the information-object address alone, so a station
    reporting a single-point input and a measurand at the same address latched the second
    into the first one's slot and published it with the first one's wire type. The key is
    now the (address, type id) pair, and a report's LIVE kind overwrites the restored one
    because the station is the authority on what it is sending.

    slotKey() itself is covered by app/tests/tst_iec104_slots.cpp
    (oneAddressUnderTwoTypesIsTwoSlots, keysNeverCollideAcrossTheAddressRange). What is
    guarded here is the driver using it, and the live kind winning over the restored one.

    retire when: a ctest drives the driver's point table through a restore plus two
    reports at one address.
    """
    driver = _read("core/Devices/IO/Drivers/Iec104.cpp")
    assert "m_slotForIoa" not in driver, "the address-only index must be gone"
    assert re.search(
        r"slotKey\(\w+\.ioa, \w+\.typeId\)", driver
    ), "the driver must look points up by the (address, type id) pair"

    fn = re.search(r"int IO::Drivers::Iec104::slotForPoint\([\s\S]*?\n\}", driver)
    assert fn is not None
    assert re.search(
        r"m_points\[\w+\]\.kind = \w+\.kind;", fn.group(0)
    ), "a live report's kind must overwrite the restored one"


def test_influx_counts_one_error_per_failed_write():
    """onSslErrors counted a failure and the finished handler counted a second one, so a
    single refused certificate read as two HTTP errors. The TLS reason is now recorded and
    reported once, by the finished handler. The wall-clock offset is also re-sampled when
    the sink re-opens: sampling it once at bootstrap shifted every later point after an
    NTP step.

    app/tests/tst_influx_lineprotocol.cpp covers the line protocol and the skip tally,
    not the reply lifecycle, which needs a live QNetworkAccessManager.

    retire when: a ctest drives the export worker against a stub network reply that fails
    its TLS handshake and asserts the error count is 1.
    """
    influx = _read("core/Storage/InfluxDB/Export.cpp")

    ssl = re.search(r"void InfluxDB::ExportWorker::onSslErrors\([\s\S]*?\n\}", influx)
    assert ssl is not None
    assert "noteHttpFailure" not in ssl.group(
        0
    ), "the TLS handler must not count its own"
    assert re.search(
        r"m_sslFailure = \w+;", ssl.group(0)
    ), "the TLS reason must be recorded for the finished handler to report"

    finished = re.search(
        r"void InfluxDB::ExportWorker::onReplyFinished\(\)[\s\S]*?\n\}", influx
    )
    assert finished is not None
    reply = finished.group(0)
    assert (
        reply.count("noteHttpFailure(") == 1
    ), "one failed write counts exactly one error"
    assert (
        "sampleEpochOffset();" in reply
    ), "the wall-clock offset must be re-sampled, not fixed at bootstrap"


def test_no_user_facing_string_calls_the_credential_store_encrypted():
    """SimpleCrypt under a machine-derived key is obfuscation, not encryption. The OS
    keychain migration is a shelved decision, so the wording is what has to be true.

    retire when: never, while the decision stands -- this is a claims scan over
    user-facing strings, which is exactly the kind of thing no runtime test can see.
    Fold it into documentation-verify.py if that linter ever grows a claims rule.
    """
    claims = (
        "stored encrypted",
        "encrypted at rest",
        "are encrypted",
        "encrypted vault",
        "encrypted storage",
        "encrypted on this",
    )
    for rel in ("app/qml", "core/Devices/MQTT", "core/Ui/AI"):
        for path in sorted((ROOT / rel).rglob("*")):
            if path.suffix not in (".qml", ".cpp", ".h") or not path.is_file():
                continue

            text = path.read_text(encoding="utf-8", errors="replace")
            for claim in claims:
                assert (
                    claim not in text
                ), f"{path} still calls the credential store encrypted"


def test_startup_failure_runs_the_same_teardown_ladder():
    """A UI that fails to load must not skip the session teardown (spec 0075, K4).

    The old shape returned EXIT_FAILURE from inside the ModuleManager scope, so the
    pipeline thread kept running and nine adopted modules were released by the static
    SessionContext destructor -- after ~QApplication. Both exits now leave the scope and
    run one ladder: workers joined, drivers stopped, message handler removed, context
    shut down, all with qApp alive (INV-6).

    retire when: a headless run with a deliberately broken QML root is added to CI and
    asserted to exit cleanly under a sanitizer.
    """
    main = _read("app/src/main.cpp")

    assert "static void shutdownSession()" in main
    assert main.count("SessionContext::current().shutdown()") == 1
    assert main.count("shutdownSession();") == 1

    ladder = main.split("static void shutdownSession()", 1)[1]
    ladder = ladder.split("\n}", 1)[0]
    order = [
        ladder.index("stopFrameConsumerWorkers"),
        ladder.index("shutdownDrivers"),
        ladder.index("qInstallMessageHandler(nullptr)"),
        ladder.index("SessionContext::current().shutdown()"),
    ]
    assert order == sorted(order), "teardown ladder is out of order"

    # The failed-bootstrap path returns a status instead of escaping the scope.
    session = main.split("static int runConfiguredSession", 1)[1].split("\n}", 1)[0]
    assert "Critical QML error" in session
    assert "return EXIT_FAILURE;" in session

    run_app = main.split("static int runApplication", 1)[1]
    assert "Critical QML error" not in run_app


def test_cli_license_commands_wait_on_the_request_verdict():
    """A refused deactivation must keep the license cache (K1).

    The CLI half -- --activate / --deactivate reporting the server's verdict rather than
    an entitlement flip -- is covered behaviourally by
    tests/integration/test_cli_licensing.py, which runs the binary. What is guarded here
    is the refusal path inside the reply reader: a refused deactivation that cleared the
    cache would strand a paying user offline, and no test drives a server refusal.

    retire when: test_cli_licensing.py gains a stub endpoint that refuses a deactivation
    and asserts the license survives.
    """
    lemon = _read("app/src/Licensing/LemonSqueezy.cpp")
    deactivation = lemon.split("readDeactivationResponse(const QByteArray& data)", 1)[1]
    deactivation = deactivation.split("\n}", 1)[0]
    refusal = deactivation.split("if (!deactivated)", 1)[1].split("return;", 1)[0]
    assert (
        "clearLicenseCache" not in refusal
    ), "a refused deactivation must keep the cache"
    assert deactivation.count("clearLicenseCache(true)") == 1


def test_api_token_can_be_supplied_without_argv():
    """A token on the command line is readable by every process on the box (K14), so the
    file and the environment variable both take precedence over the argv form.

    retire when: test_cli_licensing.py (or a sibling) launches the binary with each of
    the three token sources and asserts the precedence.
    """
    cli = _read("app/src/Misc/CLI.cpp")
    header = _read("app/src/Misc/CLI.h")
    assert "api-token-file" in header
    assert "SS_API_TOKEN" in cli

    body = cli.split("QString CLI::resolveApiToken()", 1)[1].split("\n}", 1)[0]
    assert (
        body.index("apiTokenFileOpt")
        < body.index("SS_API_TOKEN")
        < body.index("apiTokenOpt")
    )


def test_reply_handlers_do_not_open_modal_dialogs():
    """A modal spins a nested loop under the reply's stack, which can outlive it (K13).

    retire when: never, realistically -- a test that caught this would have to survive
    the crash it is testing for. Promote it to a code-verify rule if the pattern recurs
    outside the licensing handlers.
    """
    for path in (
        "app/src/Licensing/LemonSqueezy.cpp",
        "app/src/Licensing/Trial.cpp",
    ):
        text = _read(path)
        assert (
            "Utilities::showMessageBox" not in text
        ), f"{path} still opens a modal inline"
        assert "Utilities::postMessageBox" in text

    utils = _read("core/Ui/Misc/Utilities.cpp")
    assert "void Misc::Utilities::postMessageBox" in utils
    assert "Qt::QueuedConnection" in utils


def test_assistant_checkpoints_instead_of_writing_the_project():
    """With auto-approve on, an assistant edit must not reach the .ssproj (J2).

    The Conversation-side timer was converted to a checkpoint, but every tool call still ran
    through API::CommandRegistry::execute, which arms the document's own 1.5 s autosave after
    any mutating command: the file was written behind the user anyway. The synchronous tool
    lane therefore runs under an autosave hold that autoSave() itself honours, so a batch's
    end-of-op flush cannot slip past it either.

    tests/integration/test_assistant_autosave.py pins the other half of this contract over
    the API and names this test as the source-level owner of the no-write half: the raw
    API autosaves a file-backed project by design, so "the assistant did not write" is not
    observable from outside.

    retire when: the assistant tool lane becomes drivable over the API with a real
    provider stub, making the hold observable as an unchanged file hash.
    """
    conversation = _read("core/Ui/AI/Conversation.cpp")
    assert (
        "saveJsonFile" not in conversation
    ), "the assistant must not save the document"
    assert 'QStringLiteral("assistant.checkpoint")' in conversation
    hold = conversation.index("m_project.setAutoSaveHeld(true);")
    release = conversation.index("m_project.setAutoSaveHeld(held);")
    dispatch = conversation.index("m_dispatcher->executeCommand(name, arguments)")
    assert (
        hold < release < dispatch
    ), "the hold and its scope guard precede the dispatch"

    persistence = _read("core/Pipeline/DataModel/Project/ProjectPersistence.cpp")
    auto_save = re.search(
        r"void DataModel::ProjectPersistence::autoSave\(\)\n\{[\s\S]*?\n\}", persistence
    )
    assert auto_save is not None
    assert "m_autoSaveHeld" in auto_save.group(
        0
    ), "a held autosave must not write on flush"
    schedule = re.search(
        r"void DataModel::ProjectPersistence::scheduleAutoSave\(\)\n\{[\s\S]*?\n\}",
        persistence,
    )
    assert schedule is not None
    assert "m_autoSaveHeld" in schedule.group(
        0
    ), "a held autosave must not arm the timer"

    tiers = json.loads(_read("app/rcc/ai/command_safety.json"))
    assert (
        "assistant.checkpoint" in tiers["safe"]
    ), "the debounced checkpoint must stay Safe"
    assert (
        "project.save" in tiers["confirm"]
    ), "project.save stays the one explicit disk write"


# ---------------------------------------------------------------------------
# Spec 0078: build identity (commit stamp) plumbing
# ---------------------------------------------------------------------------


def test_build_commit_reaches_about_dialog():
    """The CI-supplied commit hash is registered next to the version and the About
    dialog renders it; a build without the property would silently show only the
    version again (spec 0078, AC1).

    retire when: registry-verify.py resolves every ContextRegistry registration against
    the value table -- the missing table entry caught on 2026-09-11 is exactly the kind
    of drift it already checks for icons and commands.
    """
    module_manager = _read("app/src/Misc/ModuleManager.cpp")
    assert 'registry.add("Cpp_AppVersion"' in module_manager
    assert (
        'registry.add("Cpp_AppCommit", QVariant(QStringLiteral(APP_COMMIT)))'
        in module_manager
    )

    # ContextRegistry::add asserts the name against its value table; a registration
    # without the table entry logs an assertion at every startup (caught 2026-09-11).
    registry = _read("core/Ui/Misc/ContextRegistry.cpp")
    assert 'QStringLiteral("Cpp_AppCommit"),' in registry

    app_info = _read("core/Core/AppInfo.h")
    assert "#define APP_COMMIT      PROJECT_COMMIT" in app_info

    root_cmake = _read("CMakeLists.txt")
    assert 'add_definitions(-DPROJECT_COMMIT="${SS_BUILD_COMMIT}")' in root_cmake

    about = _read("app/qml/Dialogs/About.qml")
    assert "Cpp_AppCommit" in about
    assert 'qsTr("local build")' in about
    assert 'qsTr("Version %1 (%2)")' in about


def test_ci_configures_pass_the_build_commit():
    """Every shipping configure in ci.yml passes SS_BUILD_COMMIT, so a new build job
    cannot ship a binary whose About dialog and help fetch fall back to master
    (spec 0078, R1/R4). The unit-tier configures under build/unit-ci are exempt.

    retire when: the shipping configures share one composite action or reusable
    workflow, so the flag is written once and cannot be forgotten per job.
    """
    text = _read(".github/workflows/ci.yml")
    lines = text.splitlines()
    sites = [
        i
        for i, line in enumerate(lines)
        if line.strip().startswith("cmake -B build -G Ninja")
    ]
    assert len(sites) >= 9, "expected the nine shipping configure sites"
    for i in sites:
        window = " ".join(lines[i : i + 3])
        assert (
            "-DSS_BUILD_COMMIT=${{ github.sha }}" in window
        ), f"ci.yml line {i + 1}: configure step lacks SS_BUILD_COMMIT"
