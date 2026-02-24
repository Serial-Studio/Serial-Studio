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
    text = _read("app/src/IO/Manager.cpp")

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
    text = _read("app/src/API/MCPHandler.cpp")

    for expected in [
        'name == QStringLiteral("io.driver.network.setRemoteAddress")',
        'name == QStringLiteral("io.driver.network.setUdpMulticast")',
        'name == QStringLiteral("project.file.open")',
        'name == QStringLiteral("csv.export.setEnabled")',
        'properties[QStringLiteral("dataBitsIndex")]',
        'properties[QStringLiteral("parityIndex")]',
        'properties[QStringLiteral("stopBitsIndex")]',
        'properties[QStringLiteral("socketTypeIndex")]',
        'properties[QStringLiteral("address")]',
    ]:
        assert expected in text

    for obsolete in [
        'name == QStringLiteral("io.driver.network.setTcpHost")',
        'name == QStringLiteral("io.driver.network.setUdpMulticastEnabled")',
        'name == QStringLiteral("project.openFromFile")',
        'name == QStringLiteral("csv.export.start")',
    ]:
        assert obsolete not in text


def test_declarative_widget_avoids_unsafe_table_cast():
    text = _read("app/src/UI/DeclarativeWidgets/DeclarativeWidget.cpp")

    assert "tableView->doItemsLayout();" in text
    assert "reinterpret_cast<PwnedWidget*>" not in text
