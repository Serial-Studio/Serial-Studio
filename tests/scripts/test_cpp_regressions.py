"""
Source-level regression checks for critical C++ fixes.

These tests intentionally validate key implementation details directly from
source so regressions are caught even when integration tests are not running.
"""

from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]


def _read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def test_project_model_save_returns_real_result():
    text = _read("app/src/DataModel/ProjectModel.cpp")

    assert re.search(
        r"// File already on disk, just write new data to it\s+return finalizeProjectSave\(\);",
        text,
    )


def test_hal_write_api_uses_signed_sizes():
    hal = _read("app/src/IO/HAL_Driver.h")
    assert "[[nodiscard]] virtual qint64 write(const QByteArray& data) = 0;" in hal

    for rel in [
        "app/src/IO/Drivers/UART.h",
        "app/src/IO/Drivers/Network.h",
        "app/src/IO/Drivers/BluetoothLE.h",
        "app/src/IO/Drivers/CANBus.h",
        "app/src/IO/Drivers/Audio.h",
        "app/src/IO/Drivers/Modbus.h",
    ]:
        assert "[[nodiscard]] qint64 write(const QByteArray& data) override;" in _read(rel)


def test_io_manager_bounds_written_byte_count():
    text = _read("app/src/IO/ConnectionManager.cpp")

    assert "const qint64 boundedBytes = qMin<qint64>(bytes, writtenData.size());" in text
    assert "writtenData.chop(writtenData.length() - boundedBytes);" in text


def test_csv_player_catchup_uses_next_frame_timestamps():
    text = _read("app/src/CSV/Player.cpp")

    assert re.search(
        r"while \(m_framePos < frameCount\(\) - 1.*?m_timestampCache\[m_framePos \+ 1\]",
        text,
        re.DOTALL,
    )
    assert re.search(
        r"while \(m_framePos < frameCount\(\) - 1.*?getDateTime\(m_framePos \+ 1\)",
        text,
        re.DOTALL,
    )


def test_mdf4_player_catchup_uses_next_frame_timestamp():
    text = _read("app/src/MDF4/Player.cpp")

    assert re.search(
        r"while \(m_framePos < frameCount\(\) - 1.*?nextFrameTime = m_frameIndex\[m_framePos \+ 1\]\.timestamp",
        text,
        re.DOTALL,
    )


def test_api_server_enabled_state_tracks_listen_failure():
    text = _read("app/src/API/Server.cpp")

    assert "bool effectiveEnabled = enabled;" in text
    assert "effectiveEnabled = false;" in text
    assert "if (m_enabled != effectiveEnabled)" in text
    assert "else if (enabled != effectiveEnabled)" in text
    assert "m_enabled = enabled;" not in text


def test_mcp_schema_matches_registered_command_names():
    network = _read("app/src/API/Handlers/NetworkHandler.cpp")
    project = _read("app/src/API/Handlers/ProjectHandler.cpp")
    csv = _read("app/src/API/Handlers/CSVExportHandler.cpp")
    uart = _read("app/src/API/Handlers/UARTHandler.cpp")

    assert 'QStringLiteral("io.driver.network.setRemoteAddress")' in network
    assert 'QStringLiteral("io.driver.network.setUdpMulticast")' in network
    assert 'QStringLiteral("project.file.open")' in project
    assert 'QStringLiteral("csv.export.setEnabled")' in csv
    assert 'QStringLiteral("dataBitsIndex")' in uart
    assert 'QStringLiteral("parityIndex")' in uart
    assert 'QStringLiteral("stopBitsIndex")' in uart
    assert 'QStringLiteral("socketTypeIndex")' in network
    assert 'QStringLiteral("address")' in network

    assert 'QStringLiteral("io.driver.network.setTcpHost")' not in network
    assert 'QStringLiteral("io.driver.network.setUdpMulticastEnabled")' not in network
    assert 'QStringLiteral("project.openFromFile")' not in project
    assert 'QStringLiteral("csv.export.start")' not in csv


def test_declarative_widget_avoids_unsafe_table_cast():
    text = _read("app/src/UI/DeclarativeWidgets/DeclarativeWidget.cpp")

    assert "tableView->doItemsLayout();" in text
    assert "reinterpret_cast<PwnedWidget*>" not in text


def test_ble_characteristic_path_guards_index_before_at():
    text = _read("app/src/IO/Drivers/BluetoothLE.cpp")

    assert "if (m_selectedCharacteristic == -1)" in text
    assert "m_selectedCharacteristic < m_characteristics.count()" in text


def test_macos_native_window_validates_qwindow_before_winid():
    text = _read("app/src/Platform/NativeWindow_macOS.mm")

    assert "if (!win)" in text
    assert "if (!view)" in text


def test_hotpath_byte_array_ptrs_are_null_guarded():
    frame_reader = _read("app/src/IO/FrameReader.cpp")
    server = _read("app/src/API/Server.cpp")
    console = _read("app/src/Console/Handler.cpp")

    # FrameReader::processData receives an IO::CapturedDataPtr that wraps a
    # ByteArrayPtr in `data->data`, so the null-guard checks both layers.
    assert "if (!data || !data->data || data->data->isEmpty())" in frame_reader
    assert "if (!data || data->isEmpty() || m_sockets.isEmpty())" in server
    assert "if (!data)" in console


def test_window_manager_taskbar_access_is_guarded():
    text = _read("app/src/UI/WindowManager.cpp")

    assert "m_taskbar = qobject_cast<UI::Taskbar*>(taskbar);" in text
    assert "if (m_taskbar)\n      m_taskbar->setActiveWindow(nullptr);" in text


def test_usb_close_forces_thread_stop_before_handle_close():
    text = _read("app/src/IO/Drivers/USB.cpp")

    assert "if (m_readThread.isRunning())" in text
    assert "if (!m_readThread.wait(2000))" in text
    assert "m_readThread.terminate();" in text


def test_frame_parser_uses_qcoreapplication_event_forwarding():
    text = _read("app/src/DataModel/JsCodeEditor.cpp")

    assert "QCoreApplication::sendEvent(&m_widget, event);" in text
    assert "DW_EXEC_EVENT" not in text


def test_project_editor_bounds_checks_combo_indices():
    text = _read("app/src/DataModel/ProjectEditor.cpp")

    for expected in [
        "if (widgetIdx < 0 || widgetIdx >= keys.size())",
        "if (eolIdx < 0 || eolIdx >= eolKeys.size())",
        "if (checksumId < 0 || checksumId >= checksums.size())",
        "if (idx < 0 || idx >= m_frameDetectionMethodsValues.size())",
        "if (widgetIdx < 0 || widgetIdx >= datasetWidgetKeys.size())",
        "if (plotIdx < 0 || plotIdx >= plotOptionKeys.size())",
        "if (sampleIdx < 0 || sampleIdx >= m_fftSamples.size())",
    ]:
        assert expected in text


# ---------------------------------------------------------------------------
# License guard regression tests
#
# Verify that SS_LICENSE_GUARD() is present at every critical commercial
# feature gate. If someone (or an LLM) removes a guard, these tests catch
# it before the build ships.
# ---------------------------------------------------------------------------


def test_license_guard_macro_defined_in_commercial_token_header():
    """CommercialToken.h must define SS_LICENSE_GUARD and include the generated header."""
    text = _read("app/src/Licensing/CommercialToken.h")

    assert '#  include "LicenseGuards.generated.h"' in text
    assert "#  define SS_LICENSE_GUARD()" in text
    assert "Licensing::Guards::runGuard(__LINE__)" in text


def test_license_guard_present_in_serial_studio_activated():
    """SerialStudio::activated() must include SS_LICENSE_GUARD()."""
    text = _read("app/src/SerialStudio.cpp")

    assert re.search(
        r"return.*isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from SerialStudio::activated()"


def test_license_guard_present_in_mqtt_connect():
    """MQTT::Client::openConnection() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/MQTT/Client.cpp")

    assert "SS_LICENSE_GUARD()" in text
    assert re.search(
        r"token\.isValid\(\)\s*\|\|\s*!SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from MQTT openConnection guard"


def test_license_guard_present_in_mqtt_hotpath():
    """MQTT::Client::hotpathTxFrame() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/MQTT/Client.cpp")

    assert re.search(
        r"token\.isValid\(\)\s*\n?\s*&&\s*SS_LICENSE_GUARD\(\)\s*&&\s*token\.featureTier\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from MQTT hotpathTxFrame"


def test_license_guard_present_in_mdf4_export():
    """MDF4::ExportWorker::createFile() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/MDF4/Export.cpp")

    assert text.count("SS_LICENSE_GUARD()") >= 2, \
        "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in MDF4::Export"


def test_license_guard_present_in_console_export():
    """Console::ExportWorker::createFile() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/Console/Export.cpp")

    assert text.count("SS_LICENSE_GUARD()") >= 2, \
        "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in Console::Export"


def test_license_guard_present_in_connection_manager():
    """IO::ConnectionManager::connectDevice() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/IO/ConnectionManager.cpp")

    assert "SS_LICENSE_GUARD()" in text


def test_license_guard_present_in_dashboard():
    """UI::Dashboard pro feature gates must check SS_LICENSE_GUARD()."""
    text = _read("app/src/UI/Dashboard.cpp")

    # Dashboard binds the commercial token to a short local (`tk`), so accept
    # either name as long as the guard is actually wired in.
    assert re.search(
        r"\b(?:token|tk)\.isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from Dashboard pro detection"


def test_license_guard_present_in_gps():
    """Widgets::GPS::setMapType() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/UI/Widgets/GPS.cpp")

    assert re.search(
        r"tk\.isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from GPS map type guard"


def test_license_guard_present_in_plot():
    """Widgets::Plot custom X-axis must check SS_LICENSE_GUARD()."""
    text = _read("app/src/UI/Widgets/Plot.cpp")

    assert re.search(
        r"tk\.isValid\(\)\s*&&\s*SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from Plot custom X-axis guard"


def test_license_guard_cmake_generator_exists():
    """The CMake guard generator script must exist and be included."""
    cmake_script = _read("cmake/GenLicenseGuards.cmake")
    root_cmake = _read("CMakeLists.txt")

    assert "GenLicenseGuards.cmake" in root_cmake
    assert "_LG_COUNT" in cmake_script
    assert "runGuard" in cmake_script
    assert "guardTable" in cmake_script


def test_license_guard_cmake_generates_all_styles():
    """The CMake generator must produce all 6 check styles."""
    text = _read("cmake/GenLicenseGuards.cmake")

    for style_marker in [
        "Style 0: XOR check",
        "Style 1: Modular arithmetic",
        "Style 2: Bit rotation check",
        "Style 3: Additive hash",
        "Style 4: Multi-byte extraction",
        "Style 5: Polynomial check",
    ]:
        assert style_marker in text, f"Missing guard style: {style_marker}"


def test_license_guard_minimum_count():
    """The generator must produce at least 20 guard functions."""
    text = _read("cmake/GenLicenseGuards.cmake")

    match = re.search(r'set\(_LG_COUNT\s+(\d+)\)', text)
    assert match, "_LG_COUNT not found in GenLicenseGuards.cmake"
    assert int(match.group(1)) >= 20, \
        f"Guard count too low: {match.group(1)} (minimum 20)"


def test_license_guard_build_dir_is_gitignored():
    """The generated header must be in .gitignore."""
    text = _read(".gitignore")

    assert "LicenseGuards.generated.h" in text


def test_session_report_series_preserve_raw_points_under_budget():
    """Session report charts should keep raw samples up to the 10k budget."""
    report = _read("app/src/Sessions/ReportData.cpp")
    manager = _read("app/src/Sessions/DatabaseManager.cpp")
    worker = _read("app/src/Sessions/DatabaseWorker.cpp")
    runtime = _read("app/rcc/templates/reports/session-report.js")

    assert "x.clear();" in report
    assert "y.clear();" in report
    assert "writeReportSamples(x, y, count, maxSamples, series);" in report
    assert "appendBucketSamples(y, begin, end, target, indices);" in report
    assert "appendBudgetFillSamples(count, budget, indices);" in report
    assert "std::sort(indices.begin(), indices.end());" in report
    assert "series.values.size() == std::min<std::size_t>(count, maxSamples)" in report
    assert "Q_ARG(int, 10000)" in manager
    assert "loadChartSeries(m_db, sessionId, chartMaxSamples)" in worker
    assert runtime.count("tension: 0") >= 2
    assert runtime.count("decimation: { enabled: false }") >= 2


def test_timestamp_pipeline_starts_in_driver_and_shares_parsed_frames():
    """Driver timestamps should flow through FrameReader into one shared parsed frame object."""
    hal = _read("app/src/IO/HAL_Driver.h")
    reader_h = _read("app/src/IO/FrameReader.h")
    reader_cpp = _read("app/src/IO/FrameReader.cpp")
    builder_h = _read("app/src/DataModel/FrameBuilder.h")
    builder_cpp = _read("app/src/DataModel/FrameBuilder.cpp")
    dashboard_h = _read("app/src/UI/Dashboard.h")
    sessions_h = _read("app/src/Sessions/Export.h")

    assert "struct CapturedData" in hal
    assert "void dataReceived(const IO::CapturedDataPtr& data);" in hal
    assert "void publishReceivedData(" in hal
    assert "Q_DECLARE_METATYPE(IO::CapturedDataPtr)" not in hal

    assert "moodycamel::ReaderWriterQueue<IO::CapturedDataPtr>" in reader_h
    assert "void processData(const IO::CapturedDataPtr& data);" in reader_h
    assert "frameTimestamp(qsizetype endOffsetExclusive)" in reader_h
    assert "buildFrame(QByteArray&& data, qsizetype endOffsetExclusive)" in reader_h
    assert "m_queue.try_enqueue(buildFrame(std::move(frame), frameEndPos))" in reader_cpp

    assert "void hotpathRxFrame(const IO::CapturedDataPtr& data);" in builder_h
    assert "void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);" in builder_h
    assert "dashboard.hotpathRxFrame(frame);" in builder_cpp
    assert "std::make_shared<DataModel::TimestampedFrame>(m_frame, data->timestamp + step * i)" in builder_cpp
    assert "updateTimestampedFramesEnabled" not in builder_cpp
    assert "nextTimestampedFrameTime" not in builder_cpp

    assert "void hotpathRxFrame(const DataModel::TimestampedFramePtr& frame);" in dashboard_h
    assert "void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);" in sessions_h
