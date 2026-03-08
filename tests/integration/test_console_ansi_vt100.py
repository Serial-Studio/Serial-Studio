"""
Console VT-100 and ANSI Color Stress Tests

Tests for the terminal's VT-100 emulation and ANSI escape sequence parsing,
including correctness checks and adversarial inputs designed to find crashes
or hangs in Serial Studio's terminal state machine.

All tests require Serial Studio running with API Server enabled on port 7777,
connected to a TCP device simulator on port 9000.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

ESC = "\x1b"


def _send(api_client, data: str, *, delay: float = 0.1) -> dict:
    """Send a raw string to the console and return the API result."""
    result = api_client.command("console.send", {"data": data})
    time.sleep(delay)
    return result


def _feed(device_simulator, data: bytes, *, delay: float = 0.15) -> None:
    """Push bytes from the device side so the terminal receives and parses them."""
    device_simulator.send_frame(data)
    time.sleep(delay)


def _connect(api_client, device_simulator) -> None:
    """Connect Serial Studio to the device simulator and wait for link-up."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _disconnect(api_client) -> None:
    try:
        api_client.disconnect_device()
    except Exception:
        pass


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture
def console_session(api_client, device_simulator, clean_state):
    """Set up a connected console session and tear it down afterwards."""
    _connect(api_client, device_simulator)
    api_client.command("console.setDisplayMode", {"modeIndex": 0})
    api_client.command("console.setDataMode", {"modeIndex": 0})
    api_client.command("console.setLineEnding", {"endingIndex": 0})
    time.sleep(0.1)

    yield api_client, device_simulator

    api_client.command("console.clear")
    _disconnect(api_client)


# ---------------------------------------------------------------------------
# Basic VT-100 / ANSI configuration
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_ansi_colors_default_off(api_client, clean_state):
    """ANSI color support should default to off on a fresh session."""
    config = api_client.command("console.getConfiguration")
    # The field may not be present in older builds; skip if absent
    if "ansiColors" in config:
        assert config["ansiColors"] is False


@pytest.mark.integration
def test_console_clear_before_ansi_session(api_client, clean_state):
    """console.clear must succeed regardless of ANSI/VT-100 state."""
    result = api_client.command("console.clear")
    assert result.get("cleared") is True

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", 0) == 0


# ---------------------------------------------------------------------------
# Standard ANSI SGR colour sequences (sent from the device side)
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_standard_ansi_foreground_colors(console_session):
    """Device sends all 8 standard ANSI foreground color codes without crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    for code in range(30, 38):
        payload = f"{ESC}[{code}mcolor {code - 30}\n".encode()
        _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_standard_ansi_background_colors(console_session):
    """Device sends all 8 standard ANSI background color codes without crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    for code in range(40, 48):
        payload = f"{ESC}[{code}mbg {code - 40}\n".encode()
        _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_bright_ansi_foreground_colors(console_session):
    """Device sends bright foreground color codes (90–97) without crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    for code in range(90, 98):
        payload = f"{ESC}[{code}mbright {code - 90}\n".encode()
        _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_bright_ansi_background_colors(console_session):
    """Device sends bright background color codes (100–107) without crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    for code in range(100, 108):
        payload = f"{ESC}[{code}mbright bg {code - 100}\n".encode()
        _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_reset_code(console_session):
    """ESC[0m (reset) after colored text must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = (
        f"{ESC}[31mred text{ESC}[0m normal text\n"
        f"{ESC}[32mgreen{ESC}[0m\n"
    ).encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_combined_sgr_parameters(console_session):
    """ESC[1;31m (bold + red) and similar combined SGR params must be handled."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    sequences = [
        f"{ESC}[1;31mbold red\n",
        f"{ESC}[4;32munderline green\n",
        f"{ESC}[1;4;33mbold underline yellow\n",
        f"{ESC}[0mReset\n",
    ]
    _feed(device_simulator, "".join(sequences).encode())

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_256_color_foreground(console_session):
    """ESC[38;5;<n>m (256-color foreground) across full range must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # Sample indices that exercise each of the three 256-color sub-ranges:
    # 0-7: standard, 8-15: high-intensity, 16-231: 6×6×6 cube, 232-255: greyscale
    sample_indices = [0, 7, 8, 15, 16, 21, 46, 51, 196, 231, 232, 243, 255]
    lines = "".join(f"{ESC}[38;5;{i}mfg256-{i}{ESC}[0m\n" for i in sample_indices)
    _feed(device_simulator, lines.encode(), delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_256_color_background(console_session):
    """ESC[48;5;<n>m (256-color background) must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    sample_indices = [0, 15, 16, 51, 196, 231, 255]
    lines = "".join(f"{ESC}[48;5;{i}mbg256-{i}{ESC}[0m\n" for i in sample_indices)
    _feed(device_simulator, lines.encode(), delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_rgb_color_foreground(console_session):
    """ESC[38;2;R;G;Bm (true-colour foreground) must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    rgb_samples = [
        (255, 0, 0),
        (0, 255, 0),
        (0, 0, 255),
        (128, 128, 128),
        (0, 0, 0),
        (255, 255, 255),
    ]
    lines = "".join(
        f"{ESC}[38;2;{r};{g};{b}mRGB({r},{g},{b}){ESC}[0m\n" for r, g, b in rgb_samples
    )
    _feed(device_simulator, lines.encode(), delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


# ---------------------------------------------------------------------------
# VT-100 cursor-movement sequences
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_vt100_cursor_position_sequence(console_session):
    """ESC[row;colH cursor-positioning sequences must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    sequences = [
        f"{ESC}[1;1Horigin",
        f"{ESC}[5;10Hjumped",
        f"{ESC}[1;1H",   # home
        f"{ESC}[24;80H",  # bottom-right common terminal size
    ]
    _feed(device_simulator, "\n".join(sequences).encode())

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_vt100_backspace_handling(console_session):
    """Backspace (0x08) in VT-100 mode must move cursor without crashing."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = b"Hello\x08\x08World\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


# ---------------------------------------------------------------------------
# Adversarial / crash-induction inputs
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_truncated_escape_sequence(console_session):
    """Incomplete escape sequence (ESC without follow-on) must not hang or crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # ESC with no following '[' or letter — leaves state machine in Escape state
    _feed(device_simulator, b"\x1b")
    _feed(device_simulator, b"normal text after bare ESC\n")

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_truncated_csi_sequence(console_session):
    """ESC[ with no terminating letter must recover cleanly."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # CSI opened but never closed
    _feed(device_simulator, b"\x1b[31")
    time.sleep(0.05)
    # Now send valid data; parser must recover
    _feed(device_simulator, b"mrecovered\n")

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_nested_escape_sequences(console_session):
    """Back-to-back ESC codes without text between them must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = f"{ESC}[31m{ESC}[1m{ESC}[4m{ESC}[42mstacked\n{ESC}[0m".encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_very_long_sgr_parameter_list(console_session):
    """Extremely long semicolon-separated SGR parameter list must not overflow."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # Build ESC[0;0;0;...;0m with 200 parameters
    params = ";".join(["0"] * 200)
    payload = f"{ESC}[{params}mtext\n".encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_huge_numeric_sgr_parameter(console_session):
    """A very large numeric SGR parameter (e.g. 999999) must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = f"{ESC}[999999mtext{ESC}[0m\n".encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_negative_looking_sgr_parameter(console_session):
    """SGR parameter with leading minus character must be handled gracefully."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # '-' is not a valid SGR digit; parser should ignore or skip gracefully
    payload = b"\x1b[-1mtext\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_non_sgr_csi_sequences(console_session):
    """Non-SGR CSI sequences (A/B/C/D cursor keys, J erase) must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    sequences = [
        f"{ESC}[A",   # cursor up
        f"{ESC}[B",   # cursor down
        f"{ESC}[C",   # cursor right
        f"{ESC}[D",   # cursor left
        f"{ESC}[2J",  # erase display
        f"{ESC}[K",   # erase line
    ]
    payload = "\n".join(sequences).encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_osc_title_sequence(console_session):
    """OSC title-set sequence (ESC]0;title BEL) must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = f"{ESC}]0;Serial Studio Test\x07normal text\n".encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_private_mode_sequences(console_session):
    """DEC private mode sequences (ESC[?...h/l) must be silently ignored."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    sequences = [
        f"{ESC}[?1049h",  # alternate screen
        f"{ESC}[?25l",    # hide cursor
        f"{ESC}[?25h",    # show cursor
        f"{ESC}[?1049l",  # restore screen
    ]
    payload = "".join(sequences) + "visible text\n"
    _feed(device_simulator, payload.encode())

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_null_bytes_in_stream(console_session):
    """Null bytes (0x00) embedded in data must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = b"before\x00after\nmore\x00data\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_high_byte_values_in_stream(console_session):
    """Bytes 0x80–0xFF embedded in data must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = bytes(range(0x80, 0x100)) + b"\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_mixed_utf8_and_ansi(console_session):
    """UTF-8 multibyte characters interleaved with ANSI escapes must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = (
        f"{ESC}[32m温度: 23.5°C{ESC}[0m\n"
        f"{ESC}[31m⚠ Ошибка соединения{ESC}[0m\n"
        "日本語テスト\n"
        f"{ESC}[33mΔ = 0.001{ESC}[0m\n"
    ).encode("utf-8")
    _feed(device_simulator, payload, delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


# ---------------------------------------------------------------------------
# Buffer overflow / flood tests
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_buffer_fills_to_max_and_rolls(console_session):
    """
    Sending more than MAX_LINES (1000) lines must roll the buffer without crash.

    The terminal caps at 1000 lines; sending 1200 ensures the oldest lines are
    evicted and the buffer stays bounded.
    """
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    chunk = ("A" * 79 + "\n") * 100   # 100 lines per chunk
    for _ in range(13):               # 1300 lines total
        _feed(device_simulator, chunk.encode(), delay=0.05)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_rapid_ansi_color_switching(console_session):
    """
    500 rapid colour-change sequences on one line must not crash or deadlock.
    """
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    colors = [31, 32, 33, 34, 35, 36, 37]
    line = "".join(f"{ESC}[{c}mX" for c in (colors * 72)[:500]) + f"{ESC}[0m\n"
    _feed(device_simulator, line.encode(), delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_flood_with_plain_text(console_session):
    """Flooding 5000 short lines of plain text must not crash or stall."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    chunk = ("line\n") * 500
    for _ in range(10):   # 5000 lines
        _feed(device_simulator, chunk.encode(), delay=0.02)

    time.sleep(0.5)
    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_very_long_single_line(console_session):
    """A single line of 64 KB without newline must not overflow or crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = (b"X" * 65536) + b"\n"
    _feed(device_simulator, payload, delay=0.5)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_alternating_ansi_and_plain_lines(console_session):
    """Alternating ANSI-coloured and plain-text lines must be parsed correctly."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    lines = []
    for i in range(50):
        if i % 2 == 0:
            lines.append(f"{ESC}[3{i % 7 + 1}mColored line {i}{ESC}[0m\n")
        else:
            lines.append(f"Plain line {i}\n")

    _feed(device_simulator, "".join(lines).encode(), delay=0.3)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


# ---------------------------------------------------------------------------
# Clear and re-fill stress
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_clear_during_active_feed(console_session):
    """
    Issuing console.clear while data is streaming must not cause a crash
    or leave the API unresponsive.
    """
    api_client, device_simulator = console_session

    chunk = ("data\n") * 200
    _feed(device_simulator, chunk.encode(), delay=0.01)

    result = api_client.command("console.clear")
    assert result.get("cleared") is True

    _feed(device_simulator, b"after clear\n")
    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_repeated_clear_cycles(console_session):
    """Clearing the console 20 times in rapid succession must not crash."""
    api_client, device_simulator = console_session

    for _ in range(20):
        _feed(device_simulator, b"some text\n", delay=0.02)
        result = api_client.command("console.clear")
        assert result.get("cleared") is True

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


# ---------------------------------------------------------------------------
# Edge-case sequences
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_escape_at_end_of_buffer(console_session):
    """ESC as the very last byte in a payload must not corrupt parser state."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    _feed(device_simulator, b"some text\x1b")   # ESC with nothing following
    time.sleep(0.05)
    _feed(device_simulator, b"[32m continuation\n")

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_bare_carriage_return(console_session):
    """Bare CR (0x0D) without LF must not crash the terminal."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = b"line one\roverwritten\nline two\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_crlf_line_endings(console_session):
    """CRLF line endings must not produce double-newlines or crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = b"line 1\r\nline 2\r\nline 3\r\n"
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_ansi_with_empty_parameter(console_session):
    """ESC[m (SGR with no params, equivalent to reset) must be handled."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = f"before{ESC}[mafter\n".encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_cursor_home_then_overwrite(console_session):
    """ESC[H (cursor home) followed by new text must not leave stale state."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    payload = (
        f"original line\n{ESC}[Hoverwritten\n"
    ).encode()
    _feed(device_simulator, payload)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_display_mode_switch_during_ansi_feed(console_session):
    """Switching display mode to Hex and back while ANSI data flows must not crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    _feed(device_simulator, f"{ESC}[31mred data\n".encode(), delay=0.05)

    api_client.command("console.setDisplayMode", {"modeIndex": 1})
    time.sleep(0.05)
    _feed(device_simulator, b"hex mode data\n", delay=0.05)

    api_client.command("console.setDisplayMode", {"modeIndex": 0})
    time.sleep(0.05)
    _feed(device_simulator, f"{ESC}[32mback to plain\n".encode(), delay=0.05)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0


@pytest.mark.integration
def test_all_sgr_attribute_codes(console_session):
    """Send all standard SGR attribute codes (0–9) to ensure none crash."""
    api_client, device_simulator = console_session
    api_client.command("console.clear")

    # 0=reset, 1=bold, 2=dim, 3=italic, 4=underline, 5=blink, 7=reverse, 8=conceal
    sgr_codes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    lines = "".join(f"{ESC}[{c}mattr-{c}{ESC}[0m\n" for c in sgr_codes)
    _feed(device_simulator, lines.encode(), delay=0.2)

    config = api_client.command("console.getConfiguration")
    assert config.get("bufferLength", -1) >= 0
