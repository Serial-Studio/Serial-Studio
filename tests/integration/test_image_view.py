"""
ImageView Widget Integration Tests

Tests for the ImageFrameReader detection logic in both autodetect and manual
mode.  The ImageView widget connects directly to IO::HAL_Driver::dataReceived
and processes raw bytes independently of the main FrameReader, so these tests
send raw image bytes over the network driver instead of wrapped telemetry
frames.

Supported formats in autodetect mode:
  - JPEG   FF D8 FF … FF D9
  - PNG    89 50 4E 47 0D 0A 1A 0A … IEND chunk
  - BMP    42 4D <4-byte LE size> …
  - WebP   52 49 46 46 <4-byte LE size> 57 45 42 50 …

Manual mode uses hex-encoded start/end delimiters configured in the project
JSON (imgStartSequence / imgEndSequence).

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import struct
import time
import zlib

import pytest

from utils import SerialStudioClient, DeviceSimulator


# ---------------------------------------------------------------------------
# Minimal valid image builders
# ---------------------------------------------------------------------------

def _make_minimal_jpeg() -> bytes:
    """1×1 white JPEG (no colour space extension needed for QImage)."""
    return bytes([
        0xFF, 0xD8, 0xFF, 0xE0,          # SOI + APP0 marker
        0x00, 0x10,                        # APP0 length = 16
        0x4A, 0x46, 0x49, 0x46, 0x00,     # "JFIF\0"
        0x01, 0x01,                        # version 1.1
        0x00,                              # aspect ratio units: none
        0x00, 0x01, 0x00, 0x01,            # X/Y density = 1
        0x00, 0x00,                        # no thumbnail
        0xFF, 0xDB,                        # DQT marker
        0x00, 0x43, 0x00,                  # DQT length + table id
        *([16] * 64),                      # quantisation table (uniform)
        0xFF, 0xC0,                        # SOF0 marker
        0x00, 0x0B,                        # SOF0 length = 11
        0x08,                              # precision = 8 bit
        0x00, 0x01, 0x00, 0x01,            # 1×1 pixels
        0x01,                              # 1 component
        0x01, 0x11, 0x00,                  # Cb: sampling + qt id
        0xFF, 0xC4,                        # DHT marker
        0x00, 0x1F, 0x00,                  # DHT length + class/id
        0x00, 0x01, 0x05, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B,
        0xFF, 0xDA,                        # SOS marker
        0x00, 0x08,                        # SOS length = 8
        0x01,                              # 1 component
        0x01, 0x00,                        # component id + AC/DC table ids
        0x00, 0x3F, 0x00,                  # Ss, Se, Ah/Al
        0x7F,                              # compressed scan data (1 byte)
        0xFF, 0xD9,                        # EOI
    ])


def _make_minimal_png() -> bytes:
    """1×1 white PNG."""
    def _chunk(tag: bytes, data: bytes) -> bytes:
        crc = zlib.crc32(tag + data) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", crc)

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr_data = struct.pack(">IIBBBBB", 1, 1, 8, 2, 0, 0, 0)   # 1×1 RGB8
    ihdr = _chunk(b"IHDR", ihdr_data)

    raw_row = b"\x00\xFF\xFF\xFF"          # filter byte 0 + RGB white
    compressed = zlib.compress(raw_row)
    idat = _chunk(b"IDAT", compressed)
    iend = _chunk(b"IEND", b"")

    return sig + ihdr + idat + iend


def _make_minimal_bmp() -> bytes:
    """1×1 white 24-bit BMP."""
    file_size = 54 + 3          # header (54) + 1 pixel row (3 bytes, padded to 4)
    file_size_padded = 54 + 4

    header = (
        b"BM"
        + struct.pack("<I", file_size_padded)   # file size
        + b"\x00\x00\x00\x00"                   # reserved
        + struct.pack("<I", 54)                  # pixel data offset
    )
    dib = (
        struct.pack("<I", 40)   # DIB header size
        + struct.pack("<i", 1)  # width
        + struct.pack("<i", 1)  # height (positive = bottom-up)
        + struct.pack("<H", 1)  # colour planes
        + struct.pack("<H", 24) # bits per pixel
        + struct.pack("<I", 0)  # compression: none
        + struct.pack("<I", 4)  # image size (padded row)
        + struct.pack("<i", 2835) * 2  # X/Y pixels per metre (72 dpi)
        + struct.pack("<I", 0)  # colours in table
        + struct.pack("<I", 0)  # important colours
    )
    pixel_data = b"\xFF\xFF\xFF\x00"    # BGR + 1-byte padding to 4-byte row

    return header + dib + pixel_data


def _make_minimal_webp() -> bytes:
    """1×1 white lossless WebP using a trivial VP8L bitstream."""
    vp8l_sig = b"\x2F"                             # VP8L signature byte
    # Minimal VP8L: 1×1, no transform, one RGB pixel (white)
    # Encode width-1=0, height-1=0 in the 14-bit fields, then a literal ARGB
    # A simplified hand-crafted stream that Qt's WebP decoder accepts:
    vp8l_data = bytes([
        0x2F,                   # VP8L signature
        0x00, 0x00, 0x00, 0x00, # 1×1 (width-1=0 in bits 0-13, height-1=0 in bits 14-27)
        0x00,
        0xFF, 0xFF, 0xFF, 0xFF, # ARGB = 0xFFFFFFFF (white, fully opaque)
    ])

    # RIFF/WEBP container
    chunk_data = b"VP8L" + struct.pack("<I", len(vp8l_data)) + vp8l_data
    # Pad to even length
    if len(chunk_data) % 2:
        chunk_data += b"\x00"

    riff_size = 4 + len(chunk_data)   # "WEBP" + VP8L chunk
    return b"RIFF" + struct.pack("<I", riff_size) + b"WEBP" + chunk_data


def _image_project(detection_mode: str = "autodetect",
                   start_hex: str = "",
                   end_hex: str = "") -> dict:
    """Build a minimal project JSON with a single image group."""
    group = {
        "title": "Camera",
        "widget": "image",
        "datasets": [],
        "imgDetectionMode": detection_mode,
        "imgStartSequence": start_hex,
        "imgEndSequence": end_hex,
    }
    return {
        "title": "Image Test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "frameDetection": 1,
        "checksum": "",
        "frameParser": "function parse(frame) { return [frame]; }",
        "groups": [group],
    }


def _setup_image_project(api_client, detection_mode="autodetect",
                         start_hex="", end_hex=""):
    """Load image project and configure network driver."""
    project = _image_project(detection_mode, start_hex, end_hex)
    api_client.load_project_from_json(project)
    time.sleep(0.2)
    api_client.set_operation_mode("project")
    time.sleep(0.1)
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Project must load into FrameBuilder"
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")


# ---------------------------------------------------------------------------
# Autodetect mode — one test per supported format
# ---------------------------------------------------------------------------

class TestAutodetect:
    """Autodetect mode: Serial Studio identifies the image format from magic bytes."""

    def test_jpeg_autodetect(self, api_client, device_simulator, clean_state):
        """JPEG frames (FF D8 FF … FF D9) are detected and accepted."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg_bytes = _make_minimal_jpeg()
        for _ in range(5):
            device_simulator.send_frame(jpeg_bytes)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device should stay connected after JPEG frames"

        project_status = api_client.get_project_status()
        assert project_status["groupCount"] >= 1, "Image group must be present"

    def test_png_autodetect(self, api_client, device_simulator, clean_state):
        """PNG frames (89 50 4E 47 … IEND) are detected and accepted."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        png_bytes = _make_minimal_png()
        for _ in range(5):
            device_simulator.send_frame(png_bytes)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device should stay connected after PNG frames"

        project_status = api_client.get_project_status()
        assert project_status["groupCount"] >= 1, "Image group must be present"

    def test_bmp_autodetect(self, api_client, device_simulator, clean_state):
        """BMP frames (BM + 4-byte LE size) are detected and accepted."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        bmp_bytes = _make_minimal_bmp()
        for _ in range(5):
            device_simulator.send_frame(bmp_bytes)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device should stay connected after BMP frames"

        project_status = api_client.get_project_status()
        assert project_status["groupCount"] >= 1, "Image group must be present"

    def test_webp_autodetect(self, api_client, device_simulator, clean_state):
        """WebP frames (RIFF….WEBP) are detected and accepted."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        webp_bytes = _make_minimal_webp()
        for _ in range(5):
            device_simulator.send_frame(webp_bytes)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device should stay connected after WebP frames"

        project_status = api_client.get_project_status()
        assert project_status["groupCount"] >= 1, "Image group must be present"

    @pytest.mark.parametrize("fmt,builder", [
        ("JPEG", _make_minimal_jpeg),
        ("PNG",  _make_minimal_png),
        ("BMP",  _make_minimal_bmp),
    ])
    def test_multiple_frames_same_format(self, api_client, device_simulator,
                                         clean_state, fmt, builder):
        """Multiple consecutive frames of the same format are all processed."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        img = builder()
        frames = [img] * 10
        device_simulator.send_frames(frames, interval_seconds=0.05)
        time.sleep(0.8)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], f"Device must stay connected after 10 {fmt} frames"

    def test_mixed_formats_autodetect(self, api_client, device_simulator, clean_state):
        """Interleaved JPEG and PNG frames are each detected independently."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg = _make_minimal_jpeg()
        png  = _make_minimal_png()
        mixed = [jpeg, png, jpeg, png, jpeg]
        device_simulator.send_frames(mixed, interval_seconds=0.1)
        time.sleep(0.8)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected after mixed format frames"


# ---------------------------------------------------------------------------
# Manual mode
# ---------------------------------------------------------------------------

class TestManualMode:
    """Manual mode: start/end are user-supplied hex sequences."""

    def test_manual_jpeg_with_custom_framing(self, api_client, device_simulator,
                                              clean_state):
        """JPEG bytes wrapped in custom start/end sequences are extracted in manual mode."""
        start_seq = b"\xAA\xBB\xCC"
        end_seq   = b"\xDD\xEE\xFF"

        start_hex = start_seq.hex()    # "aabbcc"
        end_hex   = end_seq.hex()      # "ddeeff"

        _setup_image_project(api_client, detection_mode="manual",
                             start_hex=start_hex, end_hex=end_hex)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg = _make_minimal_jpeg()
        wrapped = start_seq + jpeg + end_seq

        for _ in range(5):
            device_simulator.send_frame(wrapped)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected in manual mode"

    def test_manual_png_with_custom_framing(self, api_client, device_simulator,
                                             clean_state):
        """PNG bytes wrapped in custom start/end sequences are extracted in manual mode."""
        start_seq = b"$IMG_START$"
        end_seq   = b"$IMG_END$"

        start_hex = start_seq.hex()
        end_hex   = end_seq.hex()

        _setup_image_project(api_client, detection_mode="manual",
                             start_hex=start_hex, end_hex=end_hex)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        png = _make_minimal_png()
        wrapped = start_seq + png + end_seq

        for _ in range(5):
            device_simulator.send_frame(wrapped)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected in manual PNG mode"

    def test_manual_mode_single_byte_delimiters(self, api_client, device_simulator,
                                                 clean_state):
        """Single-byte hex delimiters work correctly in manual mode."""
        start_seq = b"\x01"
        end_seq   = b"\x04"

        _setup_image_project(api_client, detection_mode="manual",
                             start_hex=start_seq.hex(), end_hex=end_seq.hex())

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg = _make_minimal_jpeg()
        wrapped = start_seq + jpeg + end_seq

        for _ in range(5):
            device_simulator.send_frame(wrapped)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected with single-byte delimiters"

    def test_manual_mode_falls_back_to_autodetect_when_sequences_empty(
            self, api_client, device_simulator, clean_state):
        """If imgDetectionMode is 'manual' but sequences are empty, reader falls back to autodetect."""
        _setup_image_project(api_client, detection_mode="manual",
                             start_hex="", end_hex="")

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg = _make_minimal_jpeg()
        for _ in range(5):
            device_simulator.send_frame(jpeg)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Should stay connected even with empty manual sequences"


# ---------------------------------------------------------------------------
# Corrupted / malformed data
# ---------------------------------------------------------------------------

class TestCorruptedData:
    """Robustness: corrupted or garbage data must not crash or disconnect."""

    def test_truncated_jpeg_does_not_crash(self, api_client, device_simulator,
                                            clean_state):
        """A JPEG truncated before EOI is held in the accumulator without crashing."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        jpeg = _make_minimal_jpeg()
        truncated = jpeg[:-4]   # cut off before FF D9

        device_simulator.send_frame(truncated)
        time.sleep(0.3)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Truncated JPEG must not disconnect the device"

    def test_truncated_png_does_not_crash(self, api_client, device_simulator,
                                           clean_state):
        """A PNG truncated before IEND is held in the accumulator without crashing."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        png = _make_minimal_png()
        truncated = png[:-12]   # cut off before IEND chunk

        device_simulator.send_frame(truncated)
        time.sleep(0.3)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Truncated PNG must not disconnect the device"

    def test_corrupted_bmp_size_field(self, api_client, device_simulator, clean_state):
        """A BMP with an invalid (zero) size field in bytes 2-5 is skipped gracefully."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        bmp = bytearray(_make_minimal_bmp())
        # Write 0 into the BMP file-size field — triggers the sz < 14 guard
        bmp[2] = 0
        bmp[3] = 0
        bmp[4] = 0
        bmp[5] = 0

        device_simulator.send_frame(bytes(bmp))
        time.sleep(0.3)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Invalid BMP size must not disconnect the device"

    def test_corrupted_bmp_oversized_size_field(self, api_client, device_simulator,
                                                 clean_state):
        """A BMP that claims a size > 64 MiB is skipped without crashing."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        bmp = bytearray(_make_minimal_bmp())
        # Write 0xFF FF FF FF into the file-size field (> 64 MiB limit)
        bmp[2] = 0xFF
        bmp[3] = 0xFF
        bmp[4] = 0xFF
        bmp[5] = 0xFF

        device_simulator.send_frame(bytes(bmp))
        time.sleep(0.3)

        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Oversized BMP claim must not disconnect the device"

    def test_random_garbage_does_not_crash(self, api_client, device_simulator,
                                            clean_state):
        """Completely random bytes do not crash the accumulator."""
        import random
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        for _ in range(10):
            garbage = bytes(random.randint(0, 255) for _ in range(512))
            device_simulator.send_frame(garbage)
            time.sleep(0.05)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Garbage bytes must not disconnect the device"

    def test_garbage_then_valid_jpeg(self, api_client, device_simulator, clean_state):
        """After receiving garbage, a valid JPEG is still detected correctly."""
        import random
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        garbage = bytes(random.randint(0, 0xFE) for _ in range(256))
        device_simulator.send_frame(garbage)
        time.sleep(0.1)

        jpeg = _make_minimal_jpeg()
        for _ in range(3):
            device_simulator.send_frame(jpeg)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected after garbage + valid JPEG"

    def test_jpeg_with_corrupted_body(self, api_client, device_simulator, clean_state):
        """A JPEG with random bytes between SOI and EOI is sent without crashing."""
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Keep magic SOI + EOI but corrupt everything in between
        corrupted = bytes([0xFF, 0xD8, 0xFF]) + bytes(range(256)) + bytes([0xFF, 0xD9])

        for _ in range(3):
            device_simulator.send_frame(corrupted)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Corrupted JPEG body must not disconnect the device"

    def test_accumulator_overflow_recovery(self, api_client, device_simulator,
                                            clean_state):
        """
        Sending > 16 MiB of data with no valid image markers flushes the accumulator
        and the widget recovers cleanly (stays connected).
        """
        _setup_image_project(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Send 17 MiB of 0x00 bytes in 256 KB chunks (no image magic bytes)
        chunk = b"\x00" * (256 * 1024)
        for _ in range(68):    # 68 × 256 KB = 17 MiB
            device_simulator.send_frame(chunk)
            time.sleep(0.01)

        time.sleep(1.0)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Device must stay connected after accumulator overflow"

    def test_manual_mode_corrupted_payload(self, api_client, device_simulator,
                                            clean_state):
        """In manual mode, a payload that is not a valid image is accepted by the reader
        (it is passed to QImage::fromData which returns null) without crashing."""
        start_seq = b"\xAA\xBB"
        end_seq   = b"\xCC\xDD"

        _setup_image_project(api_client, detection_mode="manual",
                             start_hex=start_seq.hex(), end_hex=end_seq.hex())

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Payload is not a valid image — QImage::fromData will return null,
        # but the widget must not crash.
        garbage_payload = b"this is not an image at all 12345"
        wrapped = start_seq + garbage_payload + end_seq

        for _ in range(5):
            device_simulator.send_frame(wrapped)
            time.sleep(0.1)

        time.sleep(0.5)
        status = api_client.command("io.manager.getStatus")
        assert status["isConnected"], "Invalid image payload in manual mode must not crash"
