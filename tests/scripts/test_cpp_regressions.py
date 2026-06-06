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
    assert re.search(
        r"\[\[nodiscard\]\] virtual qint64 write\(const QByteArray& data\)\s*=\s*0;",
        hal,
    )

    for rel in [
        "app/src/IO/Drivers/UART.h",
        "app/src/IO/Drivers/Network.h",
        "app/src/IO/Drivers/BluetoothLE.h",
        "app/src/IO/Drivers/CANBus.h",
        "app/src/IO/Drivers/Audio.h",
        "app/src/IO/Drivers/Modbus.h",
    ]:
        assert "[[nodiscard]] qint64 write(const QByteArray& data) override;" in _read(
            rel
        )


def test_io_manager_bounds_written_byte_count():
    text = _read("app/src/IO/ConnectionManager.cpp")

    assert (
        "const qint64 boundedBytes = qMin<qint64>(bytes, writtenData.size());" in text
    )
    assert "writtenData.chop(writtenData.length() - boundedBytes);" in text


def test_csv_player_catchup_uses_next_frame_timestamps():
    text = _read("app/src/CSV/Player.cpp")

    assert re.search(
        r"while \(m_framePos < frameCount\(\) - 1.*?recomputeMsUntilNext\(msUntilNext\)",
        text,
        re.DOTALL,
    )
    assert "m_timestampCache[m_framePos + 1]" in text
    assert "getDateTime(m_framePos + 1)" in text


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

    assert 'QStringLiteral("io.network.setRemoteAddress")' in network
    assert 'QStringLiteral("io.network.setUdpMulticast")' in network
    assert 'QStringLiteral("project.open")' in project
    assert 'QStringLiteral("csvExport.setEnabled")' in csv
    assert 'QStringLiteral("dataBitsIndex")' in uart
    assert 'QStringLiteral("parityIndex")' in uart
    assert 'QStringLiteral("stopBitsIndex")' in uart
    assert 'QStringLiteral("socketTypeIndex")' in network
    assert 'QStringLiteral("address")' in network

    assert 'QStringLiteral("io.driver.network.setTcpHost")' not in network
    assert 'QStringLiteral("io.driver.network.setUdpMulticastEnabled")' not in network
    assert 'QStringLiteral("project.openFromFile")' not in project
    assert 'QStringLiteral("csv.export.start")' not in csv


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

    # FrameReader::processData receives an IO::CapturedDataPtr whose `data`
    # field is now a QByteArray (the shared_ptr indirection was removed
    # because QByteArray is already COW with an atomic refcount).
    assert "if (!data || data->data.isEmpty())" in frame_reader
    # ServerWorker::writeRawData takes a QByteArray directly; guard skips the
    # broadcast if the payload or the client list is empty.
    assert "if (data.isEmpty() || m_sockets.isEmpty())" in server
    # Console::Handler::hotpathRxData accepts a QByteArray reference and
    # short-circuits on an empty payload.
    assert "if (data.isEmpty())" in console


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
    text = _read("app/src/DataModel/Editors/JsCodeEditor.cpp")

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
    """IO::Drivers::MQTT::open() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/IO/Drivers/MQTT.cpp")

    assert "SS_LICENSE_GUARD()" in text
    assert re.search(
        r"!token\.isValid\(\)\s*\|\|\s*!SS_LICENSE_GUARD\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from MQTT input driver open() guard"


def test_license_guard_present_in_mqtt_hotpath():
    """MQTT::Publisher::licenseValid() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/MQTT/Publisher.cpp")

    assert re.search(
        r"token\.isValid\(\)\s*\n?\s*&&\s*SS_LICENSE_GUARD\(\)\s*\n?\s*&&\s*token\.featureTier\(\)",
        text,
    ), "SS_LICENSE_GUARD() missing from MQTT publisher license check"


def test_license_guard_present_in_mdf4_export():
    """MDF4::ExportWorker::createFile() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/MDF4/Export.cpp")

    assert (
        text.count("SS_LICENSE_GUARD()") >= 2
    ), "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in MDF4::Export"


def test_license_guard_present_in_console_export():
    """Console::ExportWorker::createFile() must check SS_LICENSE_GUARD()."""
    text = _read("app/src/Console/Export.cpp")

    assert (
        text.count("SS_LICENSE_GUARD()") >= 2
    ), "SS_LICENSE_GUARD() should appear in both createFile and setExportEnabled in Console::Export"


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
    """The CMake generator must dispatch across every distinct check style."""
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
    """The generator must produce at least 20 guard functions."""
    text = _read("cmake/GenLicenseGuards.cmake")

    match = re.search(r"set\(_LG_COUNT\s+(\d+)\)", text)
    assert match, "_LG_COUNT not found in GenLicenseGuards.cmake"
    assert (
        int(match.group(1)) >= 20
    ), f"Guard count too low: {match.group(1)} (minimum 20)"


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
    # Captured frames are stamped and enqueued through the slot-pool seam
    # (claimCapturedSlot -> fill -> enqueueCaptured), not a value-returning
    # buildFrame helper. enqueueCaptured owns the timestamp at the source.
    assert (
        "void enqueueCaptured(IO::CapturedDataPtr&& ptr, IO::CapturedData* cd, qsizetype frameEndPos);"
        in reader_h
    )
    assert (
        "[[nodiscard]] IO::CapturedData* claimCapturedSlot(IO::CapturedDataPtr& ptr);"
        in reader_h
    )
    assert "cd->timestamp         = frameTimestamp(frameEndPos);" in reader_cpp
    assert "m_queue.try_enqueue(std::move(ptr))" in reader_cpp

    assert "void hotpathRxFrame(const IO::CapturedDataPtr& data);" in builder_h
    assert (
        "void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);" in builder_h
    )
    assert "dashboard.hotpathRxFrame(frame);" in builder_cpp
    assert "const auto frameTs = data->timestamp + step * i;" in builder_cpp
    # Hotpath fan-out draws each TimestampedFramePtr from a fixed-size slot
    # pool (acquireFrame) so we never heap-allocate per frame.
    assert "hotpathTxFrame(acquireFrame(m_frame, frameTs));" in builder_cpp
    assert (
        "std::make_shared<DataModel::TimestampedFrame>(m_frame, frameTs)"
        not in builder_cpp
    ), "FrameBuilder hotpath must use acquireFrame() pool, not std::make_shared"
    assert "updateTimestampedFramesEnabled" not in builder_cpp
    assert "nextTimestampedFrameTime" not in builder_cpp

    assert (
        "void hotpathRxFrame(const DataModel::TimestampedFramePtr& frame);"
        in dashboard_h
    )
    assert (
        "void hotpathTxRawBytes(int deviceId, const IO::CapturedDataPtr& data);"
        in sessions_h
    )


# ----------------------------------------------------------------------------------
# R1 -- license storage stays on encrypted QSettings, never an OS keystore
# ----------------------------------------------------------------------------------


def test_license_storage_uses_simplecrypt_qsettings_not_keystore():
    """The OS-keystore experiment was reverted (f8c02f83): license and trial secrets
    persist through SimpleCrypt-encrypted QSettings. Headless servers, slim
    containers, and SSH-only hosts often have no running keyring daemon, and a
    license loss on those systems would regress against the pre-keystore release.
    Guard that the revert stays reverted -- no keystore on the licensing path, and
    the encrypted-QSettings round trip remains in place."""
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

    # Trial secrets round-trip through SimpleCrypt + QSettings.
    assert "m_crypt.encryptToString(" in trial
    assert "m_crypt.decryptToString(" in trial
    assert 'm_settings.setValue("expiry", m_crypt.encryptToString(' in trial
    assert 'm_settings.beginGroup("trial");' in trial

    # License key + metadata round-trip through SimpleCrypt + QSettings.
    assert "m_simpleCrypt.encryptToString(" in lemon
    assert 'm_settings.setValue("license", m_simpleCrypt.encryptToString(' in lemon
    assert 'm_settings.beginGroup("licensing");' in lemon


# ----------------------------------------------------------------------------------
# R6 -- hotpath logging is throttled, frame pool scan avoids per-slot atomic ops
# ----------------------------------------------------------------------------------


def test_frame_reader_dropped_frame_log_is_throttled():
    """qWarning() inside the dropped-frame slot must sit behind the same 5 s gate
    as the user notification. Previously it fired unconditionally on every drop
    and flooded stderr at 10 kHz saturation."""
    text = _read("app/src/IO/FrameReader.cpp")

    # The throttle gate now precedes the qWarning. We assert order by requiring
    # the gate to appear before the warning string in the noteDroppedFrame body.
    body = re.search(
        r"void IO::FrameReader::noteDroppedFrame\(\)\s*\{[\s\S]*?\n\}",
        text,
    )
    assert body is not None, "noteDroppedFrame body must be present"
    gate_pos = body.group(0).find("now - m_lastDropNotify < std::chrono::seconds(5)")
    warn_pos = body.group(0).find('qWarning() << "[FrameReader] Frame queue full')
    assert gate_pos != -1 and warn_pos != -1
    assert gate_pos < warn_pos, "qWarning must sit AFTER the 5 s throttle gate"


def test_frame_reader_buffer_overflow_log_is_throttled():
    """Buffer-overflow qWarning() must also throttle (separate timer) so a saturated
    ring buffer does not log per chunk."""
    text = _read("app/src/IO/FrameReader.cpp")
    header = _read("app/src/IO/FrameReader.h")

    assert "IO::CapturedData::SteadyTimePoint m_lastOverflowLog;" in header
    assert re.search(
        r"if \(now - m_lastOverflowLog >= std::chrono::seconds\(5\)\) \{\s*"
        r"m_lastOverflowLog = now;\s*"
        r'qWarning\(\) << "\[FrameReader\] Buffer overflow:"',
        text,
    )


def test_frame_builder_pool_scan_copies_no_shared_ptr():
    """The pool-slot scan must not copy a slot's shared_ptr per iteration. The
    allocator probes free slots via use_count() == 1 (the pool holds the only
    reference) inside claimPoolSlot(), and acquireFrame binds the chosen slot by
    const reference and returns through the aliasing TimestampedFramePtr ctor --
    no per-frame control block, no copy in the scan."""
    text = _read("app/src/DataModel/FrameBuilder.cpp")

    scan = re.search(
        r"size_t DataModel::FrameBuilder::claimPoolSlot\(\) noexcept\s*\{[\s\S]*?\n\}",
        text,
    )
    assert scan is not None, "claimPoolSlot body must be present"
    scan_body = scan.group(0)

    # The scan probes the pool's own reference count -- no shared_ptr copy, no CAS.
    assert "if (m_framePool[idx].use_count() != 1)" in scan_body
    assert "compare_exchange_strong" not in scan_body, "scan must not CAS a slot flag"
    assert (
        "auto slotPtr = m_framePool[idx];" not in scan_body
    ), "scan must not copy a slot shared_ptr"

    body = re.search(
        r"DataModel::TimestampedFramePtr DataModel::FrameBuilder::acquireFrame\("
        r"\s*const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts\)"
        r"\s*\{[\s\S]*?\n\}",
        text,
    )
    assert body is not None, "acquireFrame body must be present"
    snippet = body.group(0)

    # The claimed slot is bound by reference, mutated through a raw pointer, and
    # returned via the aliasing ctor -- the only incref happens at construction.
    assert "const auto& slotOwner = m_framePool[idx];" in snippet
    assert "auto* slotRaw         = slotOwner.get();" in snippet
    assert "return TimestampedFramePtr(slotOwner, &slotRaw->frame);" in snippet


# ----------------------------------------------------------------------------------
# R7 -- BackupManager snapshots carry a version stamp; CrashTracker stays local-only
# ----------------------------------------------------------------------------------


def test_backup_manager_writes_versioned_wrapper():
    text = _read("app/src/Misc/BackupManager.cpp")

    assert "static constexpr int kSnapshotFormat = 1;" in text
    assert 'static const QLatin1StringView kBackupMetaKey("_backupMeta");' in text

    # snapshot() injects a _backupMeta object with format/takenAt/label/sha1.
    assert 'meta.insert(QStringLiteral("format"), kSnapshotFormat);' in text
    assert 'meta.insert(QStringLiteral("takenAt"),' in text
    assert 'meta.insert(QStringLiteral("label"), label);' in text
    assert 'meta.insert(QStringLiteral("sha1"),' in text
    assert "serialised.insert(kBackupMetaKey, meta);" in text


def test_backup_manager_restore_refuses_newer_format():
    text = _read("app/src/Misc/BackupManager.cpp")

    # restore() reads _backupMeta and refuses snapshots written by a newer build.
    assert re.search(
        r"if \(obj\.contains\(kBackupMetaKey\)\) \{[\s\S]*?"
        r'const int fmt   = meta\.value\(QStringLiteral\("format"\)\)\.toInt\(0\);[\s\S]*?'
        r"if \(fmt > kSnapshotFormat\) \{[\s\S]*?return false;",
        text,
    )

    # The wrapper must be stripped before handing the doc to ProjectModel so the
    # wrapper is invisible to ProjectModel's loader.
    assert "obj.remove(kBackupMetaKey);" in text
    assert "pm.loadFromJsonDocument(projectDoc, originalPath);" in text


def test_backup_manager_summarize_surfaces_wrapper():
    text = _read("app/src/Misc/BackupManager.cpp")

    assert "if (project.contains(kBackupMetaKey))" in text
    assert re.search(
        r'out\.insert\(QStringLiteral\("backupMeta"\),\s*'
        r"project\.value\(kBackupMetaKey\)\.toObject\(\)\.toVariantMap\(\)\);",
        text,
    )


def test_backup_manager_snapshots_parser_only_edits_on_empty_project():
    """A parser-only edit on a structurally empty project (no groups/datasets) must still
    reach the snapshot path. setModified() suppresses the dirty flag there, so it emits
    contentTouched instead, and BackupManager arms the debounce off that signal. The whole-
    project hash in snapshot() is the sole arbiter of whether anything is actually written.
    """
    model_h = _read("app/src/DataModel/ProjectModel.h")
    model_cpp = _read("app/src/DataModel/ProjectModel.cpp")

    # The decoupling signal exists and fires from the empty-project branch of setModified().
    assert "void contentTouched();" in model_h
    setmod = re.search(
        r"void DataModel::ProjectModel::setModified\(const bool modified\)[\s\S]*?\n\}",
        model_cpp,
    )
    assert setmod is not None
    body = setmod.group(0)
    # contentTouched is emitted inside the suppressed branch, before the early return.
    assert re.search(
        r"if \(modified && m_groups\.empty\(\)[\s\S]*?"
        r"Q_EMIT contentTouched\(\);[\s\S]*?return;",
        body,
    ), "setModified must emit contentTouched in the empty-project branch"

    # BackupManager listens to it and arms the debounce (hash-only decision in flushDebounced).
    backup_cpp = _read("app/src/Misc/BackupManager.cpp")
    assert (
        "&DataModel::ProjectModel::contentTouched, this, "
        "&BackupManager::onProjectContentTouched" in backup_cpp
    )
    slot = re.search(
        r"void Misc::BackupManager::onProjectContentTouched\(\)[\s\S]*?\n\}",
        backup_cpp,
    )
    assert slot is not None
    assert "m_debounceTimer->start();" in slot.group(0)


def test_crash_tracker_documents_local_only_telemetry():
    """The CrashTracker header @brief reminds reviewers that the class has no telemetry.
    Adding a network sink here without re-reviewing the contract is the failure we want
    to catch."""
    text = _read("app/src/Misc/CrashTracker.h")

    # @brief sentence carries the contract as a one-liner.
    assert "(no telemetry)" in text

    # No network / outbound transport headers exist in the cpp.
    cpp = _read("app/src/Misc/CrashTracker.cpp")
    for forbidden in ("QNetworkAccessManager", "QTcpSocket", "QUdpSocket", "curl_easy"):
        assert (
            forbidden not in cpp
        ), f"CrashTracker.cpp introduced a network sink: {forbidden}"


# ----------------------------------------------------------------------------------
# R8 -- legacy "JSON Projects" migration is copy + marker, never destructive move
# ----------------------------------------------------------------------------------


def test_workspace_migration_copies_with_atomic_marker():
    text = _read("app/src/Misc/WorkspaceManager.cpp")

    # No destructive rename of the legacy folder.
    assert (
        'parent.rename(QStringLiteral("JSON Projects"), QStringLiteral("Projects"))'
        not in text
    )
    # No QFile::rename of individual files in the migration body.
    assert "QFile::rename(src, dst)" not in text
    # No rmdir of the legacy folder.
    assert 'QDir(m_path).rmdir(QStringLiteral("JSON Projects"))' not in text

    # Files are copied, not moved.
    assert "if (!QFile::copy(src, dst))" in text

    # Marker file path matches the in-source constant.
    assert (
        'static const QLatin1StringView kMarkerFileName(".migrated-from-json-projects");'
        in text
    )

    # The marker file is written AFTER every copy succeeded.
    body = re.search(
        r"void Misc::WorkspaceManager::migrateLegacyProjectsFolder\(\)[\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)
    fail_pos = snippet.find("if (!allCopied)\n    return;")
    marker_pos = snippet.find("QFile m(marker);")
    flag_pos = snippet.rfind("m_settings.setValue(kMigratedKey, true);")
    assert fail_pos != -1 and marker_pos != -1 and flag_pos != -1
    assert (
        fail_pos < marker_pos < flag_pos
    ), "marker write must come after the all-copies check and before the settings flag"

    # Re-entry skip: a present marker short-circuits the migration on next launch.
    assert "if (QFileInfo::exists(marker)) {" in text


# ----------------------------------------------------------------------------------
# R9 -- FrameParserPipeline is the single bytes-to-channels seam (hotpath + dialog + dryRun)
# ----------------------------------------------------------------------------------


def test_frame_parser_pipeline_module_exists_with_shared_seam():
    """The shared module exposes the decoder seam, the QuickPlot splitter, and the two
    runners that the dialog and the dryRun call into. Anything else fragmenting that
    surface re-introduces the divergence we just removed."""
    header = _read("app/src/DataModel/Scripting/FrameParserPipeline.h")

    # Two-overload decoder seam: live-parser (FrameParser&) and engine-override (IScriptEngine&).
    assert (
        "void decodeAndParseFrame(const QByteArray& rawFrame,\n"
        "                         SerialStudio::DecoderMethod decoderMethod,\n"
        "                         DataModel::FrameParser& parser,\n"
        "                         int sourceId,\n"
        "                         QList<QStringList>& outChannels);"
    ) in header
    assert (
        "void decodeAndParseFrame(const QByteArray& rawFrame,\n"
        "                         SerialStudio::DecoderMethod decoderMethod,\n"
        "                         DataModel::IScriptEngine& engine,\n"
        "                         QList<QStringList>& outChannels);"
    ) in header

    # QuickPlot CSV split lives here too -- the live FrameBuilder calls it.
    assert (
        "void splitQuickPlotChannels(const QByteArray& rawFrame, QList<QStringList>& outChannels);"
        in header
    )

    # Two public runners (live engine for dialog, throwaway engine for dryRun).
    assert "PipelineResult runFrameParserPipeline(" in header
    assert "PipelineResult runFrameParserPipelineWithCode(" in header


def test_frame_builder_uses_shared_seam_not_inline_decoder_switch():
    """FrameBuilder::decodeProjectChannels must delegate to the shared decoder seam.
    A re-inlined `case SerialStudio::Hexadecimal: parser.parseMultiFrame(...)` switch
    here means the hotpath has drifted from the dialog/dryRun path again."""
    text = _read("app/src/DataModel/FrameBuilder.cpp")

    # Must include the pipeline header.
    assert '#include "DataModel/Scripting/FrameParserPipeline.h"' in text

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

    # Player short-circuit also goes through the shared QuickPlot helper.
    assert (
        "DataModel::splitQuickPlotChannels(data->data, outChannels);" in snippet
    ), "Player playback must reuse the same comma split as QuickPlot."


def test_frame_builder_quick_plot_uses_shared_splitter():
    """parseQuickPlotFrame must rely on the shared splitter so dialog / dryRun split
    bytes the same way the live QuickPlot mode does."""
    text = _read("app/src/DataModel/FrameBuilder.cpp")

    body = re.search(
        r"void DataModel::FrameBuilder::parseQuickPlotFrame\(const IO::CapturedDataPtr& data\)"
        r"[\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    assert "DataModel::splitQuickPlotChannels(data->data, splitRows);" in snippet
    # The retired inline helper must not have been added back.
    assert "parseCsvValues" not in snippet
    # And the retired helper itself must be gone from the file entirely.
    assert "void parseCsvValues(" not in text


def test_frame_parser_pipeline_dispatches_quick_plot_branch():
    """Both runners must comma-split on QuickPlot and only invoke the parser
    in non-QuickPlot modes."""
    text = _read("app/src/DataModel/Scripting/FrameParserPipeline.cpp")

    # The per-frame helper short-circuits QuickPlot to splitQuickPlotChannels.
    helper = re.search(
        r"DataModel::PipelineFrame buildPipelineFrame\([\s\S]*?\n\}",
        text,
    )
    assert helper is not None
    assert re.search(
        r"if \(spec\.operationMode == SerialStudio::QuickPlot\) \{\s*"
        r"DataModel::splitQuickPlotChannels\(frame\.rawBytes, frame\.rows\);",
        helper.group(0),
    )

    # The dryRun runner skips engine compilation entirely for QuickPlot.
    code_runner = re.search(
        r"DataModel::PipelineResult DataModel::runFrameParserPipelineWithCode\([\s\S]*?\n\}",
        text,
    )
    assert code_runner is not None
    assert (
        "const bool needsEngine = (spec.operationMode != SerialStudio::QuickPlot);"
        in code_runner.group(0)
    )


# ----------------------------------------------------------------------------------
# R10 -- dryRun + dispatcher require bytes; parser-only fallback is gone
# ----------------------------------------------------------------------------------


def test_frame_parser_dry_run_requires_input_bytes():
    """project.frameParser.dryRun must reject calls without inputBytes / inputBytesHex
    and must not silently fall back to sampleFrame/sampleFrames anymore."""
    text = _read("app/src/API/Handlers/ProjectHandler.cpp")

    body = re.search(
        r"API::CommandResponse API::Handlers::ProjectHandler::frameParserDryRun"
        r"\([\s\S]*?\n\}\n",
        text,
    )
    assert body is not None
    snippet = body.group(0)

    # The required-param check for inputBytes / inputBytesHex is present.
    assert re.search(
        r'if \(!params\.contains\(QStringLiteral\("inputBytes"\)\)\s*'
        r'&& !params\.contains\(QStringLiteral\("inputBytesHex"\)\)\)\s*'
        r"return CommandResponse::makeError\(\s*id,\s*ErrorCode::MissingParam,",
        snippet,
    )

    # Legacy sampleFrame / sampleFrames branches are gone.
    assert "sampleFrame" not in snippet
    assert "sampleFrames" not in snippet

    # Drives the shared pipeline runner.
    assert "DataModel::runFrameParserPipelineWithCode(" in snippet

    # QuickPlot operationMode gets line-based defaults so the call still extracts frames.
    assert (
        "if (spec.operationMode == SerialStudio::QuickPlot && spec.finishSequences.isEmpty())"
        in snippet
    )


def test_tool_dispatcher_routes_pipeline_inputs():
    """ToolDispatcher.frameParserDryRunCommand must forward the pipeline keys when bytes
    are supplied, and fall back to dryCompile when they aren't."""
    text = _read("app/src/AI/ToolDispatcher.cpp")

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
    """assistant.script.apply must strip the dryRun-only pipeline keys before
    forwarding to setCode; otherwise setCode rejects them with InvalidParam."""
    text = _read("app/src/API/Handlers/AssistantHandler.cpp")

    # The fallback is keyed off inputBytes(Hex), not the old sampleFrame(s) check.
    assert (
        'if (kind == QStringLiteral("frame_parser") && !inner.contains(QStringLiteral("inputBytes"))'
        in text
    )
    assert '&& !inner.contains(QStringLiteral("inputBytesHex")))' in text

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
    """The per-source frame parser bridge (FrameParserModel) must call the shared
    pipeline runners and must write delimiter / decoder / detection / checksum edits
    back to ProjectModel so the live driver reconfigures.
    """
    text = _read("app/src/DataModel/Editors/FrameParserModel.cpp")

    assert '#include "DataModel/Scripting/FrameParserPipeline.h"' in text

    # Both pipeline runners are exercised from dryRun: the live engine for JS/Lua
    # sources and the throwaway native-template engine for Native sources.
    assert "runFrameParserPipeline(bytes, spec, m_sourceId)" in text
    assert "runNativeTemplatePipeline(bytes, spec," in text

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
        assert "ProjectModel::instance().updateSource(m_sourceId, src);" in body.group(
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
    sure we did not accidentally rename that one too."""
    text = _read("app/src/DataModel/Importers/ProtoImporter.cpp")

    # emitDecoderParseMsg uses endPos for the loop guard.
    parse_msg = re.search(
        r"void DataModel::ProtoImporter::emitDecoderParseMsg\(QString& code\)"
        r"[\s\S]*?\n\}",
        text,
    )
    assert parse_msg is not None
    snippet = parse_msg.group(0)
    assert (
        'code += QStringLiteral("    if p <= before or p > endPos then break end\\n");'
        in snippet
    ), "parseMsg loop guard must use endPos (not endP)"
    assert (
        'code += QStringLiteral("    if p <= before or p > endP then break end\\n");'
        not in snippet
    ), "parseMsg must not regress to the broken endP name"

    # emitScoreDispatcher legitimately declares its own local endP -- keep that as-is.
    score = re.search(
        r"void DataModel::ProtoImporter::emitScoreDispatcher\(QString& code\)"
        r"[\s\S]*?\n\}",
        text,
    )
    assert score is not None
    score_snippet = score.group(0)
    assert "local endP = bufLen + 1" in score_snippet
    assert "while p < endP do" in score_snippet
    assert "if p <= before or p > endP then break end" in score_snippet


def test_protobuf_example_embedded_parser_was_repaired():
    """The shipped Protobuf example carries a regenerated parser script in its
    .ssproj. Confirm the same endPos fix is applied there so users running the
    example out-of-the-box see all datasets light up, not just the first field."""
    text = _read("examples/Protobuf Example/Protobuf Example.ssproj")

    # The buggy form must be gone from parseMsg (it appeared inside a "\\n"-escaped
    # Lua string, after the closing "end" of the float branch).
    assert "p > endP then break end\\n  end\\nend\\n\\n-- Dispatch table" not in text
    # The repaired guard must be present.
    assert "p > endPos then break end\\n  end\\nend\\n\\n-- Dispatch table" in text
    # scoreDispatcher's legitimate local endP must still be there.
    assert "local endP = bufLen + 1" in text


# ----------------------------------------------------------------------------------
# R13 -- Dashboard preserves in-flight time-ring data across project mutations
# ----------------------------------------------------------------------------------


def test_dashboard_snapshots_and_restores_time_rings_on_reconfigure():
    """Project mutations (frame detection change, dataset rename, plot min/max edit,
    etc.) used to wipe plots until reconnect because reconfigureDashboard runs
    resetData(false), which cleared every TimeRing. The snapshot/restore pair now
    preserves per-uniqueId ring contents across reconfigures."""
    text = _read("app/src/UI/Dashboard.cpp")
    header = _read("app/src/UI/Dashboard.h")

    # The four helpers must be declared and defined.
    for sig in (
        "QHash<int, DSP::TimeRing> snapshotPlotTimeRings() const;",
        "QHash<int, std::vector<DSP::TimeRing>> snapshotMultiplotTimeRings() const;",
        "void restorePlotTimeRings(QHash<int, DSP::TimeRing>& snapshot);",
        "void restoreMultiplotTimeRings(QHash<int, std::vector<DSP::TimeRing>>& snapshot);",
    ):
        assert sig in header, f"missing in Dashboard.h: {sig}"

    # restorePlotTimeRings handles both same-shape splice and different-shape replay.
    plot_restore = re.search(
        r"void UI::Dashboard::restorePlotTimeRings\([\s\S]*?\n\}",
        text,
    )
    assert plot_restore is not None
    snippet = plot_restore.group(0)
    assert "live.time.capacity() == kept.time.capacity()" in snippet
    assert "qFuzzyCompare(live.interval, kept.interval)" in snippet
    assert "live = std::move(kept);" in snippet
    assert "replayTimeRing(kept, live);" in snippet

    # Same for the multiplot variant (per-curve splice/replay).
    multi_restore = re.search(
        r"void UI::Dashboard::restoreMultiplotTimeRings\([\s\S]*?\n\}",
        text,
    )
    assert multi_restore is not None
    multi_snippet = multi_restore.group(0)
    assert "live[j].time.capacity() == kept[j].time.capacity()" in multi_snippet
    assert "qFuzzyCompare(live[j].interval, kept[j].interval)" in multi_snippet
    assert "live[j] = std::move(kept[j]);" in multi_snippet
    assert "replayTimeRing(kept[j], live[j]);" in multi_snippet


# Matches the snapshot declaration regardless of clang-format's alignment padding.
# The `=` aligns to the widest declarator in its block, so the gap varies per site
# (setPlotTimeRange also snapshots the sweep configs, widening the column).
_SNAPSHOT_DECL = re.compile(r"auto savedPlotRings += snapshotPlotTimeRings\(\);")


def test_dashboard_snapshots_around_every_clearing_trigger():
    """The three sites that previously dropped time-ring data on a project edit must
    snapshot before the rebuild and restore after."""
    text = _read("app/src/UI/Dashboard.cpp")

    # reconfigureDashboard: snapshot before resetData, restore after updateDataSeries.
    body = re.search(
        r"void UI::Dashboard::reconfigureDashboard[\s\S]*?\n\}",
        text,
    )
    assert body is not None
    snippet = body.group(0)
    snap_match = _SNAPSHOT_DECL.search(snippet)
    snap_pos = snap_match.start() if snap_match else -1
    reset_pos = snippet.find("resetData(false);")
    update_pos = snippet.find("updateDataSeries();")
    restore_pos = snippet.find("restorePlotTimeRings(savedPlotRings);")
    assert snap_pos != -1 and reset_pos != -1
    assert update_pos != -1 and restore_pos != -1
    assert (
        snap_pos < reset_pos < update_pos < restore_pos
    ), "reconfigureDashboard must snapshot before resetData and restore after updateDataSeries"

    # setPlotTimeRange: snapshot around configureLineSeries / configureMultiLineSeries.
    range_body = re.search(
        r"void UI::Dashboard::setPlotTimeRange[\s\S]*?\n\}",
        text,
    )
    assert range_body is not None
    range_snippet = range_body.group(0)
    assert _SNAPSHOT_DECL.search(range_snippet) is not None
    assert "restorePlotTimeRings(savedPlotRings);" in range_snippet
    assert range_snippet.index("snapshotPlotTimeRings") < range_snippet.index(
        "configureLineSeries"
    )
    assert range_snippet.index("configureMultiLineSeries") < range_snippet.index(
        "restorePlotTimeRings"
    )

    # setPoints: same shape.
    pts_body = re.search(
        r"void UI::Dashboard::setPoints[\s\S]*?\n  \}\n\}",
        text,
    )
    assert pts_body is not None
    pts_snippet = pts_body.group(0)
    assert _SNAPSHOT_DECL.search(pts_snippet) is not None
    assert "restorePlotTimeRings(savedPlotRings);" in pts_snippet


# ----------------------------------------------------------------------------------
# R14 -- Modbus map importer: a bool on a holding/input register decodes the whole
#        16-bit word, not a packed coil bit (PLC e-stop LED never lit otherwise)
# ----------------------------------------------------------------------------------


def test_modbus_register_bool_decodes_whole_word():
    """A holding/input-register bool used to be funneled to decodeBit(), which reads
    one LSB-packed bit out of the byte stream -- correct for coils/discrete inputs
    (register type >= 2) but wrong for holding/input registers, whose response is
    2-byte big-endian words. For E-Stop at register offset 0, decodeBit() read the
    register's HIGH byte (always 0x00) and the LED never turned on.

    The fix routes a register-block bool to a new RegBool entry type, decoded as a
    full word with 0/1 truthiness. Coil/discrete blocks still decode as packed bits.
    """
    text = _read("app/src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp")

    # The RegBool entry type exists and is dispatched through decodeRegister().
    assert "RegBool," in text, "ModbusEntryType::RegBool must exist"
    assert re.search(
        r"if \(entry\.type == ModbusEntryType::RegBool\)\s*\{\s*"
        r"storeAt\(entry\.channel, QString::number\(raw != 0 \? 1 : 0\)\);",
        text,
    ), "decodeRegister must short-circuit RegBool to a 0/1 word truthiness"

    # Only bit blocks (coils/discrete) force the packed-bit path now; a bool on a
    # register block must NOT be collapsed into Bit.
    assert (
        'if (bitBlock || name == QLatin1String("bool"))' not in text
    ), "the bug was the combined `bitBlock || bool` guard; it must be split"
    assert re.search(r"if \(bitBlock\)\s*return ModbusEntryType::Bit;", text)
    assert re.search(
        r'if \(name == QLatin1String\("bool"\)\)\s*return ModbusEntryType::RegBool;',
        text,
    )


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
    and 0 to 0. The old packed-bit path returns 0 for value 1 -- the actual bug."""
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
    """signalToJson() emitted bigEndian = (dataEndian == LittleEndian), an inversion.
    Qt reports DBC @0 (Motorola) as QSysInfo::BigEndian and @1 (Intel) as
    LittleEndian, verbatim -- so the flag was backwards for BOTH endiannesses and
    every signal hit the wrong extractor branch."""
    text = _read("app/src/DataModel/Importers/DBCImporter.cpp")

    assert (
        'json.insert(QStringLiteral("bigEndian"), signal.dataEndian() == QSysInfo::BigEndian);'
        in text
    ), "bigEndian must compare against BigEndian"
    assert "QSysInfo::LittleEndian" not in re.search(
        r"DBCImporter::signalToJson[\s\S]*?\n\}", text
    ).group(0), "signalToJson must not regress to the inverted LittleEndian compare"


def test_dbc_extract_signal_uses_motorola_sawtooth():
    """The big-endian branch of extractCanSignal walked bit_pos = start_bit + i
    monotonically, which is not the DBC Motorola layout. The fix steps the in-byte
    bit index DOWN and jumps +15 across byte boundaries (Qt's sawtooth)."""
    text = _read("app/src/DataModel/Scripting/NativeTemplates/MapTemplates.cpp")

    be_branch = re.search(r"if \(signal\.big_endian\)\s*\{[\s\S]*?\n  \} else \{", text)
    assert be_branch is not None
    snippet = be_branch.group(0)

    # The sawtooth step is the heart of the fix.
    assert "bit_pos = ((bit_pos % 8) == 0) ? (bit_pos + 15) : (bit_pos - 1);" in snippet
    # The old monotonic walk and its complemented shift must be gone.
    assert "const int bit_pos  = signal.start_bit + i;" not in snippet
    assert "7 - (bit_pos % 8)" not in snippet


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
    both decode back. Also pin a byte0-MSB 16-bit signal (7|16@0) of 0x1234."""

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
    creeping back into a signal line."""
    dbc = _read("examples/CAN Bus Example/ecu_simulator.dbc")

    # Real signal definitions are "SG_ <name> : <bits>@<order><sign> ..." -- the
    # NS_ header's bare "SG_MUL_VAL_" symbol token must not be mistaken for one.
    signal_lines = [ln for ln in dbc.splitlines() if re.match(r"\s*SG_\s+\w+\s*:", ln)]
    assert signal_lines, "expected SG_ signal definitions in the example DBC"

    motorola = [ln for ln in signal_lines if re.search(r"@0[+-]", ln)]
    assert not motorola, f"example DBC must be Intel (@1), found @0 in: {motorola}"
    assert all(re.search(r"@1[+-]", ln) for ln in signal_lines)
