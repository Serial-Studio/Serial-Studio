"""
Unit tests for all frame-parser scripts in app/rcc/scripts/.

Every test invokes the script's parse() function through a fresh Node.js
subprocess (via the run_parser helper defined in conftest.py), which means:
  - The shared 'parsedValues' state starts from zeros for every test case.
  - No QJSEngine or Qt dependency is needed; the scripts are pure JS.

Test organisation: one class per script, prefixed with the script name.
"""

import math
import struct

import pytest

from .conftest import run_parser


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def approx(value, rel=1e-5):
    """Return a pytest.approx wrapper with a relative tolerance."""
    return pytest.approx(value, rel=rel)


def make_tlv(tag: int, value_bytes: bytes) -> list[int]:
    """Build a minimal TLV triplet as a byte list."""
    return [tag, len(value_bytes)] + list(value_bytes)


def crc24q(data: bytes) -> int:
    """Compute CRC-24Q as used by RTCM3."""
    poly = 0x1864CFB
    crc = 0
    for byte in data:
        crc ^= byte << 16
        for _ in range(8):
            crc <<= 1
            if crc & 0x1000000:
                crc ^= poly
    return crc & 0xFFFFFF


def build_rtcm3(msg_type: int, payload_bits: bytes) -> list[int]:
    """
    Wrap a raw RTCM3 payload in a valid RTCM3 frame.
    payload_bits should already contain the 12-bit message type at the front.
    """
    header = bytes([0xD3, 0x00, len(payload_bits)])
    body = header + payload_bits
    crc = crc24q(body)
    return list(body + bytes([(crc >> 16) & 0xFF, (crc >> 8) & 0xFF, crc & 0xFF]))


# ---------------------------------------------------------------------------
# at_commands
# ---------------------------------------------------------------------------

class TestAtCommands:
    SCRIPT = "at_commands.js"

    def test_csq_basic(self):
        result = run_parser(self.SCRIPT, "+CSQ: 25,99")
        assert result[0] == 25
        assert result[1] == 99

    def test_creg(self):
        result = run_parser(self.SCRIPT, "+CREG: 0,1")
        assert result[2] == 0   # numeric 0, not string "0"
        assert result[3] == 1

    def test_cgatt(self):
        result = run_parser(self.SCRIPT, "+CGATT: 1")
        assert result[4] == 1

    def test_cops(self):
        result = run_parser(self.SCRIPT, "+COPS: 0,2,26201")
        assert result[5] == 0
        assert result[6] == 2
        assert result[7] == 26201

    def test_ok_response_returns_zeros(self):
        result = run_parser(self.SCRIPT, "OK")
        assert result == [0] * 8

    def test_unknown_command_ignored(self):
        result = run_parser(self.SCRIPT, "+UNKNOWN: 1,2,3")
        assert result == [0] * 8

    def test_output_length(self):
        result = run_parser(self.SCRIPT, "+CSQ: 10,20")
        assert len(result) == 8

    def test_whitespace_trimmed(self):
        result = run_parser(self.SCRIPT, "  +CSQ: 5,0  ")
        assert result[0] == 5


# ---------------------------------------------------------------------------
# base64_encoded
# ---------------------------------------------------------------------------

class TestBase64Encoded:
    SCRIPT = "base64_encoded.js"

    def test_hello_world(self):
        import base64
        expected = list(b"Hello World")
        b64 = base64.b64encode(b"Hello World").decode()
        result = run_parser(self.SCRIPT, b64)
        assert result == expected

    def test_single_byte(self):
        import base64
        b64 = base64.b64encode(bytes([0x42])).decode()
        result = run_parser(self.SCRIPT, b64)
        assert result == [0x42]

    def test_all_zeros(self):
        import base64
        b64 = base64.b64encode(bytes(4)).decode()
        result = run_parser(self.SCRIPT, b64)
        assert result == [0, 0, 0, 0]

    def test_invalid_base64_returns_empty(self):
        result = run_parser(self.SCRIPT, "!!!invalid!!!")
        assert result == []

    def test_binary_values(self):
        import base64
        data = bytes([0x00, 0xFF, 0x7F, 0x80])
        b64 = base64.b64encode(data).decode()
        result = run_parser(self.SCRIPT, b64)
        assert result == list(data)


# ---------------------------------------------------------------------------
# binary_tlv
# ---------------------------------------------------------------------------

class TestBinaryTlv:
    SCRIPT = "binary_tlv.js"

    def test_single_tag(self):
        frame = make_tlv(0x01, bytes([42]))
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 42

    def test_multiple_tags(self):
        frame = (
            make_tlv(0x01, bytes([10]))
            + make_tlv(0x02, bytes([20]))
            + make_tlv(0x03, bytes([30]))
        )
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 10
        assert result[1] == 20
        assert result[2] == 30

    def test_two_byte_value_big_endian(self):
        frame = make_tlv(0x04, bytes([0x01, 0x00]))
        result = run_parser(self.SCRIPT, frame)
        assert result[3] == 256

    def test_unknown_tag_ignored(self):
        frame = make_tlv(0xFF, bytes([99]))
        result = run_parser(self.SCRIPT, frame)
        assert result == [0] * 5

    def test_output_length(self):
        result = run_parser(self.SCRIPT, [])
        assert len(result) == 5

    def test_truncated_frame_does_not_crash(self):
        result = run_parser(self.SCRIPT, [0x01])
        assert len(result) == 5


# ---------------------------------------------------------------------------
# cobs_encoded
# ---------------------------------------------------------------------------

class TestCobsEncoded:
    SCRIPT = "cobs_encoded.js"

    def test_example_from_spec(self):
        # [0x03, 0x11, 0x22, 0x02, 0x33] → [0x11, 0x22, 0x00, 0x33]
        result = run_parser(self.SCRIPT, [0x03, 0x11, 0x22, 0x02, 0x33])
        assert result == [0x11, 0x22, 0x00, 0x33]

    def test_no_zero_in_data(self):
        # data [0x01, 0x02, 0x03] → COBS [0x04, 0x01, 0x02, 0x03]
        result = run_parser(self.SCRIPT, [0x04, 0x01, 0x02, 0x03])
        assert result == [0x01, 0x02, 0x03]

    def test_leading_zero(self):
        # data [0x00, 0x01] → COBS [0x01, 0x02, 0x01]
        result = run_parser(self.SCRIPT, [0x01, 0x02, 0x01])
        assert result == [0x00, 0x01]

    def test_zero_code_terminates(self):
        result = run_parser(self.SCRIPT, [0x00, 0x01, 0x02])
        assert result == []

    def test_empty_frame(self):
        result = run_parser(self.SCRIPT, [])
        assert result == []

    def test_max_block_no_zero_inserted(self):
        # code = 0xFF means 254 data bytes with no zero appended at the end of block
        block = [0xFF] + [0xAA] * 254
        result = run_parser(self.SCRIPT, block)
        assert len(result) == 254
        assert all(b == 0xAA for b in result)


# ---------------------------------------------------------------------------
# comma_separated
# ---------------------------------------------------------------------------

class TestCommaSeparated:
    SCRIPT = "comma_separated.js"

    def test_basic(self):
        assert run_parser(self.SCRIPT, "a,b,c") == ["a", "b", "c"]

    def test_numbers(self):
        assert run_parser(self.SCRIPT, "1,2,3") == ["1", "2", "3"]

    def test_single_value(self):
        assert run_parser(self.SCRIPT, "hello") == ["hello"]

    def test_empty_fields(self):
        assert run_parser(self.SCRIPT, "a,,c") == ["a", "", "c"]


# ---------------------------------------------------------------------------
# fixed_width_fields
# ---------------------------------------------------------------------------

class TestFixedWidthFields:
    SCRIPT = "fixed_width_fields.js"

    def test_basic(self):
        result = run_parser(self.SCRIPT, "John      25  Engineer  ABC123")
        assert result == ["John", "25", "Engineer", "ABC123"]

    def test_single_word(self):
        assert run_parser(self.SCRIPT, "hello") == ["hello"]

    def test_leading_trailing_spaces_ignored(self):
        result = run_parser(self.SCRIPT, "  a  b  ")
        assert result == ["a", "b"]

    def test_tab_separated(self):
        result = run_parser(self.SCRIPT, "a\tb\tc")
        assert result == ["a", "b", "c"]


# ---------------------------------------------------------------------------
# hexadecimal_bytes
# ---------------------------------------------------------------------------

class TestHexadecimalBytes:
    SCRIPT = "hexadecimal_bytes.js"

    def test_no_spaces(self):
        result = run_parser(self.SCRIPT, "1A2B3C")
        assert result == [0x1A, 0x2B, 0x3C]

    def test_with_spaces(self):
        result = run_parser(self.SCRIPT, "1A 2B 3C")
        assert result == [0x1A, 0x2B, 0x3C]

    def test_zeros(self):
        result = run_parser(self.SCRIPT, "000000")
        assert result == [0, 0, 0]

    def test_ff(self):
        result = run_parser(self.SCRIPT, "FF")
        assert result == [255]

    def test_empty_string(self):
        result = run_parser(self.SCRIPT, "")
        assert result == []


# ---------------------------------------------------------------------------
# ini_config
# ---------------------------------------------------------------------------

class TestIniConfig:
    SCRIPT = "ini_config.js"

    def test_temperature(self):
        result = run_parser(self.SCRIPT, "temperature=25.5")
        assert result[0] == approx(25.5)

    def test_multiple_keys(self):
        result = run_parser(self.SCRIPT, "temperature=20\nhumidity=60\npressure=1013")
        assert result[0] == 20
        assert result[1] == 60
        assert result[2] == 1013

    def test_comment_lines_ignored(self):
        frame = "; a comment\n# another comment\ntemperature=99"
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 99

    def test_spaces_around_equals(self):
        result = run_parser(self.SCRIPT, "temperature = 42")
        assert result[0] == 42

    def test_unknown_key_ignored(self):
        result = run_parser(self.SCRIPT, "unknown_key=999")
        assert result == [0] * 5

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "")) == 5


# ---------------------------------------------------------------------------
# json_data
# ---------------------------------------------------------------------------

class TestJsonData:
    SCRIPT = "json_data.js"

    def test_full_object(self):
        result = run_parser(self.SCRIPT, '{"time":"12:05","speed":48,"angle":4.0,"distance":87}')
        assert result[0] == "12:05"
        assert result[1] == 48
        assert result[2] == approx(4.0)
        assert result[3] == 87

    def test_partial_object(self):
        result = run_parser(self.SCRIPT, '{"speed":100}')
        assert result[1] == 100
        assert result[0] == 0

    def test_empty_string_returns_defaults(self):
        result = run_parser(self.SCRIPT, "")
        assert result == [0, 0, 0, 0]

    def test_invalid_json_returns_defaults(self):
        result = run_parser(self.SCRIPT, "{bad json}")
        assert result == [0, 0, 0, 0]

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "{}")) == 4


# ---------------------------------------------------------------------------
# key_value_pairs
# ---------------------------------------------------------------------------

class TestKeyValuePairs:
    SCRIPT = "key_value_pairs.js"

    def test_all_keys(self):
        result = run_parser(self.SCRIPT, "temperature=22.5,humidity=60.3,pressure=1013")
        assert result[0] == approx(22.5)
        assert result[1] == approx(60.3)
        assert result[2] == approx(1013.0)

    def test_single_key(self):
        result = run_parser(self.SCRIPT, "humidity=75")
        assert result[1] == 75

    def test_unknown_key_ignored(self):
        result = run_parser(self.SCRIPT, "unknown=999")
        assert result == [0, 0, 0]

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "")) == 3


# ---------------------------------------------------------------------------
# mavlink
# ---------------------------------------------------------------------------

class TestMavlink:
    SCRIPT = "mavlink.js"

    def _make_v1_frame(self, msg_id: int, payload: bytes) -> list[int]:
        header = bytes([0xFE, len(payload), 0, 1, 1, msg_id])
        return list(header + payload + bytes([0x00, 0x00]))

    def _pack_float(self, value: float) -> bytes:
        return struct.pack("<f", value)

    def test_attitude_message(self):
        # Message ID 30: time_boot_ms(4) + roll(4) + pitch(4) + yaw(4) + ...
        time_ms = struct.pack("<I", 1000)
        roll    = self._pack_float(0.1)
        pitch   = self._pack_float(0.2)
        yaw     = self._pack_float(0.3)
        # Pad to at least 13 bytes (need indices 4,8,12 for roll/pitch/yaw)
        payload = time_ms + roll + pitch + yaw
        frame   = self._make_v1_frame(30, payload)
        result  = run_parser(self.SCRIPT, frame)
        assert result[0] == approx(0.1, rel=1e-4)
        assert result[1] == approx(0.2, rel=1e-4)
        assert result[2] == approx(0.3, rel=1e-4)

    def test_wrong_start_marker_returns_defaults(self):
        frame = [0xAA] + [0] * 10
        result = run_parser(self.SCRIPT, frame)
        assert result == [0] * 16

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0xFE, 0x00])
        assert result == [0] * 16

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, [0xFE] + [0] * 10)) == 16


# ---------------------------------------------------------------------------
# messagepack
# ---------------------------------------------------------------------------

class TestMessagepack:
    SCRIPT = "messagepack.js"

    def test_fixarray_of_integers(self):
        # fixarray of 3 positive fixints: [0x93, 1, 2, 3]
        result = run_parser(self.SCRIPT, [0x93, 0x01, 0x02, 0x03])
        assert result == [1, 2, 3]

    def test_single_positive_fixint(self):
        result = run_parser(self.SCRIPT, [0x2A])
        assert result == [42]

    def test_negative_fixint(self):
        # 0xFF = -1 in negative fixint range
        result = run_parser(self.SCRIPT, [0xFF])
        assert result == [-1]

    def test_nil(self):
        result = run_parser(self.SCRIPT, [0xC0])
        assert result == [None]

    def test_bool_true(self):
        result = run_parser(self.SCRIPT, [0xC3])
        assert result == [True]

    def test_bool_false(self):
        result = run_parser(self.SCRIPT, [0xC2])
        assert result == [False]

    def test_uint8(self):
        result = run_parser(self.SCRIPT, [0xCC, 0xE5])
        assert result == [0xE5]

    def test_uint16(self):
        result = run_parser(self.SCRIPT, [0xCD, 0x01, 0x00])
        assert result == [256]


# ---------------------------------------------------------------------------
# modbus
# ---------------------------------------------------------------------------

class TestModbus:
    SCRIPT = "modbus.js"

    def test_read_holding_registers(self):
        # FC 0x03, 2 registers, values 100 and 200
        frame = [0x01, 0x03, 0x04, 0x00, 0x64, 0x00, 0xC8]
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 100
        assert result[1] == 200

    def test_read_coils(self):
        # FC 0x01, 1 byte, bits: 0b10110101
        frame = [0x01, 0x01, 0x01, 0b10110101]
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 1
        assert result[1] == 0
        assert result[2] == 1

    def test_exception_response_returns_defaults(self):
        # FC 0x83 (exception for FC 0x03), exception code 0x02
        # modbus.js emits a console.log for exceptions; conftest takes the last line
        frame = [0x01, 0x83, 0x02]
        result = run_parser(self.SCRIPT, frame)
        assert result == [0] * 9

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0x01, 0x03])
        assert result == [0] * 9

    def test_write_single_coil(self):
        # FC 0x05, coil address 0, value ON (0xFF00)
        frame = [0x01, 0x05, 0x00, 0x00, 0xFF, 0x00]
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 1

    def test_flow_rate_scaling(self):
        # Register 6 has scale=10, raw value 550 → 55.0
        frame = [0x01, 0x03, 0x0E] + [0x00] * 12 + [0x02, 0x26]
        result = run_parser(self.SCRIPT, frame)
        assert result[6] == approx(55.0)

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, [0x01, 0x03, 0x00])) == 9


# ---------------------------------------------------------------------------
# nmea_0183
# ---------------------------------------------------------------------------

class TestNmea0183:
    SCRIPT = "nmea_0183.js"

    def test_gpgga(self):
        sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47"
        result = run_parser(self.SCRIPT, sentence)
        assert result[0] == approx(48.1173, rel=1e-3)
        assert result[1] == approx(11.5167, rel=1e-3)
        assert result[2] == approx(545.4)

    def test_gprmc(self):
        sentence = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A"
        result = run_parser(self.SCRIPT, sentence)
        assert result[0] == approx(48.1173, rel=1e-3)

    def test_gpvtg(self):
        sentence = "$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48"
        result = run_parser(self.SCRIPT, sentence)
        assert result[6] == approx(5.5)
        assert result[7] == approx(10.2)

    def test_no_dollar_returns_defaults(self):
        result = run_parser(self.SCRIPT, "GPGGA,123519")
        assert result == [0] * 16

    def test_south_latitude_is_negative(self):
        # No checksum suffix so validation is skipped; S hemisphere gives negative lat
        sentence = "$GPGLL,3346.000,S,07030.000,W"
        result = run_parser(self.SCRIPT, sentence)
        assert result[0] < 0

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "$GPGGA,,,,,,,,,,,,,*1E")) == 16


# ---------------------------------------------------------------------------
# nmea_2000
# ---------------------------------------------------------------------------

class TestNmea2000:
    SCRIPT = "nmea_2000.js"

    def _make_frame(self, pgn: int, data: list[int]) -> list[int]:
        # Encode PGN into CAN ID (simplified: PGN in bits 8-25, PDU2 format)
        can_id = (pgn & 0x1FFFF) << 8
        return [
            can_id & 0xFF,
            (can_id >> 8) & 0xFF,
            (can_id >> 16) & 0xFF,
            (can_id >> 24) & 0xFF,
            len(data),
        ] + data

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0x00] * 4)
        assert result == [0] * 20

    def test_output_length(self):
        result = run_parser(self.SCRIPT, [0x00] * 6)
        assert len(result) == 20


# ---------------------------------------------------------------------------
# pipe_delimited
# ---------------------------------------------------------------------------

class TestPipeDelimited:
    SCRIPT = "pipe_delimited.js"

    def test_basic(self):
        assert run_parser(self.SCRIPT, "a|b|c") == ["a", "b", "c"]

    def test_numbers(self):
        assert run_parser(self.SCRIPT, "1|2|3") == ["1", "2", "3"]

    def test_single_value(self):
        assert run_parser(self.SCRIPT, "hello") == ["hello"]


# ---------------------------------------------------------------------------
# raw_bytes
# ---------------------------------------------------------------------------

class TestRawBytes:
    SCRIPT = "raw_bytes.js"

    def test_passthrough(self):
        frame = [0x01, 0x02, 0x03, 0x04]
        assert run_parser(self.SCRIPT, frame) == frame

    def test_all_byte_values(self):
        frame = list(range(256))
        assert run_parser(self.SCRIPT, frame) == frame

    def test_empty(self):
        assert run_parser(self.SCRIPT, []) == []

    def test_single_byte(self):
        assert run_parser(self.SCRIPT, [0xFF]) == [255]


# ---------------------------------------------------------------------------
# rtcm_corrections
# ---------------------------------------------------------------------------

class TestRtcmCorrections:
    SCRIPT = "rtcm_corrections.js"

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0xD3, 0x00, 0x01, 0x00])
        assert result == [0] * 16

    def test_wrong_preamble_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0xAA] + [0] * 10)
        assert result == [0] * 16

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, [0xD3] + [0] * 5)) == 16

    def test_crc_failure_returns_defaults(self):
        # Valid preamble and length but wrong CRC bytes
        frame = [0xD3, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        result = run_parser(self.SCRIPT, frame)
        assert result == [0] * 16


# ---------------------------------------------------------------------------
# semicolon_separated
# ---------------------------------------------------------------------------

class TestSemicolonSeparated:
    SCRIPT = "semicolon_separated.js"

    def test_basic(self):
        assert run_parser(self.SCRIPT, "a;b;c") == ["a", "b", "c"]

    def test_numbers(self):
        assert run_parser(self.SCRIPT, "1;2;3") == ["1", "2", "3"]

    def test_single_value(self):
        assert run_parser(self.SCRIPT, "hello") == ["hello"]


# ---------------------------------------------------------------------------
# sirf_binary
# ---------------------------------------------------------------------------

class TestSirfBinary:
    SCRIPT = "sirf_binary.js"

    def _make_sirf_frame(self, msg_id: int, data: bytes) -> list[int]:
        payload = bytes([msg_id]) + data
        length  = len(payload)
        checksum = sum(payload) & 0x7FFF
        header   = bytes([
            (length >> 8) & 0x7F,
            length & 0xFF,
        ])
        trailer = bytes([
            (checksum >> 8) & 0x7F,
            checksum & 0xFF,
        ])
        return list(header + payload + trailer)

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0x00, 0x01, 0x04])
        assert result == [0] * 20

    def test_bad_checksum_returns_defaults(self):
        frame = self._make_sirf_frame(4, bytes(10))
        frame[-1] ^= 0xFF
        result = run_parser(self.SCRIPT, frame)
        assert result == [0] * 20

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, [0x00] * 5)) == 20

    def test_tracker_data_message(self):
        # SiRF Message ID 4 layout (after msg ID byte is stripped by .slice(1)):
        #   byte[0]:   reserved
        #   bytes[1-2]: GPS week (big-endian)
        #   bytes[3-6]: GPS TOW * 100 (big-endian)
        #   byte[7]:    number of tracked channels
        reserved = bytes([0])
        week     = struct.pack(">H", 2300)
        tow      = struct.pack(">I", 123400)   # 1234.00 s * 100
        payload  = reserved + week + tow + bytes([8]) + bytes(20)
        frame    = self._make_sirf_frame(4, payload)
        result   = run_parser(self.SCRIPT, frame)
        assert result[14] == 2300
        assert result[15] == approx(1234.0)
        assert result[16] == 8


# ---------------------------------------------------------------------------
# slip_encoded
# ---------------------------------------------------------------------------

class TestSlipEncoded:
    SCRIPT = "slip_encoded.js"

    def test_example_from_docstring(self):
        encoded  = [0x01, 0x02, 0xDB, 0xDC, 0x03, 0xDB, 0xDD, 0x04]
        expected = [0x01, 0x02, 0xC0, 0x03, 0xDB, 0x04]
        assert run_parser(self.SCRIPT, encoded) == expected

    def test_no_special_bytes(self):
        frame = [0x01, 0x02, 0x03]
        assert run_parser(self.SCRIPT, frame) == frame

    def test_end_bytes_skipped(self):
        # END byte (0xC0) should be removed from output
        result = run_parser(self.SCRIPT, [0xC0, 0x01, 0x02, 0xC0])
        assert result == [0x01, 0x02]

    def test_invalid_escape_emits_both_bytes(self):
        # ESC followed by unknown byte → both bytes emitted
        result = run_parser(self.SCRIPT, [0xDB, 0xAA])
        assert result == [0xDB, 0xAA]

    def test_empty(self):
        assert run_parser(self.SCRIPT, []) == []

    def test_trailing_esc_emits_esc(self):
        result = run_parser(self.SCRIPT, [0x01, 0xDB])
        assert result == [0x01, 0xDB]


# ---------------------------------------------------------------------------
# tab_separated
# ---------------------------------------------------------------------------

class TestTabSeparated:
    SCRIPT = "tab_separated.js"

    def test_basic(self):
        assert run_parser(self.SCRIPT, "a\tb\tc") == ["a", "b", "c"]

    def test_numbers(self):
        assert run_parser(self.SCRIPT, "1\t2\t3") == ["1", "2", "3"]

    def test_single_value(self):
        assert run_parser(self.SCRIPT, "hello") == ["hello"]


# ---------------------------------------------------------------------------
# ubx_ublox
# ---------------------------------------------------------------------------

class TestUbxUblox:
    SCRIPT = "ubx_ublox.js"

    def _ubx_checksum(self, msg_class: int, msg_id: int, payload: bytes) -> tuple[int, int]:
        ck_a, ck_b = 0, 0
        for b in [msg_class, msg_id, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF] + list(payload):
            ck_a = (ck_a + b) & 0xFF
            ck_b = (ck_b + ck_a) & 0xFF
        return ck_a, ck_b

    def _make_ubx_frame(self, msg_class: int, msg_id: int, payload: bytes) -> list[int]:
        length = len(payload)
        ck_a, ck_b = self._ubx_checksum(msg_class, msg_id, payload)
        return list(bytes([msg_class, msg_id, length & 0xFF, (length >> 8) & 0xFF])
                    + payload + bytes([ck_a, ck_b]))

    def test_too_short_returns_defaults(self):
        result = run_parser(self.SCRIPT, [0x01, 0x07, 0x00])
        assert result == [0] * 20

    def test_bad_checksum_returns_defaults(self):
        payload = bytes(92)
        frame   = self._make_ubx_frame(0x01, 0x07, payload)
        frame[-1] ^= 0xFF
        result  = run_parser(self.SCRIPT, frame)
        assert result == [0] * 20

    def test_nav_posllh(self):
        # 0x01 0x02: [iTOW(4), lon(4), lat(4), height(4), hMSL(4), hAcc(4), vAcc(4)]
        iTOW   = struct.pack("<I", 0)
        lon    = struct.pack("<i", int(11.5 * 1e7))
        lat    = struct.pack("<i", int(48.2 * 1e7))
        height = struct.pack("<i", 500000)
        hMSL   = struct.pack("<i", 450000)
        hAcc   = struct.pack("<I", 1000)
        vAcc   = struct.pack("<I", 2000)
        payload = iTOW + lon + lat + height + hMSL + hAcc + vAcc
        frame   = self._make_ubx_frame(0x01, 0x02, payload)
        result  = run_parser(self.SCRIPT, frame)
        assert result[0] == approx(48.2, rel=1e-4)
        assert result[1] == approx(11.5, rel=1e-4)
        assert result[2] == approx(450.0, rel=1e-3)

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, [0x01, 0x07] + [0] * 4)) == 20


# ---------------------------------------------------------------------------
# url_encoded
# ---------------------------------------------------------------------------

class TestUrlEncoded:
    SCRIPT = "url_encoded.js"

    def test_basic(self):
        result = run_parser(self.SCRIPT, "temperature=25.5&humidity=60&pressure=1013")
        assert result[0] == approx(25.5)
        assert result[1] == 60
        assert result[2] == 1013

    def test_percent_encoding(self):
        result = run_parser(self.SCRIPT, "temperature=25%2E5")
        assert result[0] == approx(25.5)

    def test_plus_decoded_as_space(self):
        # '+' decodes to space: "25+5" → "25 5"; parseFloat("25 5") = 25
        result = run_parser(self.SCRIPT, "temperature=25+5")
        assert result[0] == 25

    def test_unknown_key_ignored(self):
        result = run_parser(self.SCRIPT, "unknown=999")
        assert result == [0] * 6

    def test_empty_frame(self):
        result = run_parser(self.SCRIPT, "")
        assert result == [0] * 6

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "")) == 6


# ---------------------------------------------------------------------------
# xml_data
# ---------------------------------------------------------------------------

class TestXmlData:
    SCRIPT = "xml_data.js"

    def test_basic_tags(self):
        xml    = "<root><temperature>25.5</temperature><humidity>60</humidity></root>"
        result = run_parser(self.SCRIPT, xml)
        assert result[0] == approx(25.5)
        assert result[1] == approx(60.0)

    def test_nested_tags(self):
        xml    = "<data><sensors><pressure>1013</pressure></sensors></data>"
        result = run_parser(self.SCRIPT, xml)
        assert result[2] == approx(1013.0)

    def test_unknown_tag_ignored(self):
        result = run_parser(self.SCRIPT, "<unknown>999</unknown>")
        assert result == [0] * 8

    def test_empty_frame(self):
        assert run_parser(self.SCRIPT, "") == [0] * 8

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "<root/>")) == 8


# ---------------------------------------------------------------------------
# yaml_data
# ---------------------------------------------------------------------------

class TestYamlData:
    SCRIPT = "yaml_data.js"

    def test_basic(self):
        result = run_parser(self.SCRIPT, "temperature: 25.5\nhumidity: 60\npressure: 1013")
        assert result[0] == approx(25.5)
        assert result[1] == 60
        assert result[2] == 1013

    def test_inline_comment_stripped(self):
        result = run_parser(self.SCRIPT, "temperature: 30 # measured in Celsius")
        assert result[0] == 30

    def test_document_separator_skipped(self):
        result = run_parser(self.SCRIPT, "---\ntemperature: 55\n...")
        assert result[0] == 55

    def test_null_value_keeps_previous(self):
        result = run_parser(self.SCRIPT, "temperature: null")
        assert result[0] == 0

    def test_boolean_true(self):
        result = run_parser(self.SCRIPT, "temperature: true")
        assert result[0] is True

    def test_unknown_key_ignored(self):
        result = run_parser(self.SCRIPT, "unknown: 999")
        assert result == [0] * 7

    def test_output_length(self):
        assert len(run_parser(self.SCRIPT, "")) == 7


# ---------------------------------------------------------------------------
# batched_sensor_data
# ---------------------------------------------------------------------------

class TestBatchedSensorData:
    SCRIPT = "batched_sensor_data.js"

    def test_basic_json(self):
        frame = '{"device_id":42,"battery":3.7,"temperature":25.5,"samples":[1.0,2.0,3.0]}'
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 42
        assert result[1] == approx(3.7)
        assert result[2] == approx(25.5)
        assert result[3] == approx([1.0, 2.0, 3.0])

    def test_missing_samples_is_empty(self):
        frame  = '{"device_id":1,"battery":4.0,"temperature":20.0}'
        result = run_parser(self.SCRIPT, frame)
        assert result[3] == []

    def test_defaults_for_missing_scalars(self):
        frame  = '{"samples":[5.0]}'
        result = run_parser(self.SCRIPT, frame)
        assert result[0] == 0
        assert result[1] == approx(0.0)
        assert result[2] == approx(0.0)


# ---------------------------------------------------------------------------
# time_series_2d
# ---------------------------------------------------------------------------

class TestTimeSeries2D:
    SCRIPT = "time_series_2d.js"

    def test_basic(self):
        frame = '{"records":[{"timestamp":1000,"temp":25.5,"humidity":60.0},{"timestamp":2000,"temp":25.7,"humidity":59.8}]}'
        result = run_parser(self.SCRIPT, frame)
        assert len(result) == 2
        assert result[0] == [1000, approx(25.5), approx(60.0)]
        assert result[1] == [2000, approx(25.7), approx(59.8)]

    def test_empty_records(self):
        result = run_parser(self.SCRIPT, '{"records":[]}')
        assert result == []

    def test_missing_records_key(self):
        result = run_parser(self.SCRIPT, '{}')
        assert result == []

    def test_defaults_for_missing_fields(self):
        frame  = '{"records":[{"timestamp":500}]}'
        result = run_parser(self.SCRIPT, frame)
        assert result[0][0] == 500
        assert result[0][1] == approx(0.0)
        assert result[0][2] == approx(0.0)
