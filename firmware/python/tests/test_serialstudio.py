# Serial Studio device library
# https://serial-studio.com/
#
# Copyright (C) 2020-2026 Alex Spataru
#
# SPDX-License-Identifier: MIT

import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(HERE.parents[1] / "tests"))

import golden_sketches  # noqa: E402
import serialstudio  # noqa: E402
from serialstudio import _string  # noqa: E402


def make(**options):
    out = bytearray()
    return serialstudio.Device("T", out.extend, **options), out


class GoldenTests(unittest.TestCase):
    def test_every_sketch_reproduces_its_golden_bytes(self):
        for name in golden_sketches.SKETCHES:
            with self.subTest(name):
                self.assertEqual(golden_sketches.run(name), golden_sketches.expected(name))


class EscapeTests(unittest.TestCase):
    def test_separator_before_a_number_char_is_escaped(self):
        self.assertEqual(_string("a 5"), '"a\\u00205"')
        self.assertEqual(_string("a,5;-1"), '"a\\u002c5\\u003b-1"')
        self.assertEqual(_string("x=5"), '"x\\u003d5"')
        self.assertEqual(_string("t:.5"), '"t\\u003a.5"')

    def test_other_text_is_untouched(self):
        self.assertEqual(_string("a b"), '"a b"')
        self.assertEqual(_string("m/s²"), '"m/s²"')
        self.assertEqual(_string('q"\\'), '"q\\"\\\\"')

    def test_control_characters_escape(self):
        self.assertEqual(_string("a\nb"), '"a\\u000ab"')


class RequestTests(unittest.TestCase):
    def test_declares_once_then_frames(self):
        device, out = make()
        for value in (1, 2):
            device.group("G")
            device.channel("A", value)
            device.send()

        lines = bytes(out).split(b"\n")
        self.assertTrue(lines[0].startswith(b"#ss1{"))
        self.assertEqual(lines[1], b"2")

    def test_request_redeclares_at_next_pass(self):
        device, out = make()
        for _ in range(2):
            device.group("G")
            device.channel("A", 1)
            device.send()

        device.feed(b"#ss?\r\n")
        self.assertEqual(device.poll(), 1)
        device.group("G")
        device.channel("A", 1)
        device.send()
        self.assertEqual(bytes(out).count(b"#ss1{"), 2)

    def test_request_mid_pass_does_not_corrupt_the_frame(self):
        device, out = make()
        device.group("G")
        device.channel("A", 1)
        device.send()
        device.group("G")
        device.channel("A", 7)
        device.feed(b"#ss?\n")
        device.poll()
        device.channel("B", 8)
        device.send()
        self.assertTrue(bytes(out).endswith(b"\n7,8\n"))


class CommandTests(unittest.TestCase):
    def test_command_reaches_the_callback_and_ignores_the_token(self):
        seen, lines = [], []
        device, _ = make(on_control=lambda n, v: seen.append((n, v)), on_line=lines.append)
        device.feed(b"#ss!Setpoint=128\n#ss!Gain=2.5@17\nhello\n#ss!Bad\n")
        device.poll()
        self.assertEqual(seen, [("Setpoint", 128), ("Gain", 2.5)])
        self.assertEqual(lines, ["hello"])

    def test_partial_lines_wait_for_the_newline(self):
        seen = []
        device, _ = make(on_control=lambda n, v: seen.append((n, v)))
        device.feed(b"#ss!A=")
        self.assertEqual(device.poll(), 0)
        device.feed(b"1\n")
        device.poll()
        self.assertEqual(seen, [("A", 1)])


class CapTests(unittest.TestCase):
    def test_string_cap_raises(self):
        device, _ = make()
        device.group("G")
        with self.assertRaises(ValueError):
            device.channel("x" * 65, 1)

    def test_dataset_cap_raises(self):
        device, _ = make()
        device.group("G")
        with self.assertRaises(ValueError):
            for n in range(513):
                device.channel("c%d" % n, 1)

    def test_unknown_property_raises(self):
        device, _ = make()
        device.group("G")
        with self.assertRaises(ValueError):
            device.channel("A", 1, transform="x")

    def test_channel_needs_a_group(self):
        device, _ = make()
        with self.assertRaises(ValueError):
            device.channel("A", 1)

    def test_label_cap_raises(self):
        device, _ = make()
        device.group("G")
        with self.assertRaises(ValueError):
            device.channel("A", 1, labels={n: "l" for n in range(257)})

    def test_line_cap_raises(self):
        device, _ = make()
        device.group("G")
        for n in range(300):
            device.channel("c%d" % n, 1, units="a" * 60)

        with self.assertRaises(ValueError):
            device.send()


class BinaryTests(unittest.TestCase):
    def pass_once(self, device, values):
        device.group("G")
        for n, value in enumerate(values):
            device.channel("c%d" % n, value)

        device.send()

    def test_declaration_carries_the_source_block(self):
        device, out = make()
        device.binary("u8,i16", "big", b"\x7e", None)
        self.pass_once(device, [1, 2])
        line = bytes(out).decode()
        self.assertIn('"sources":[{"frameStart":"7E","hexadecimalDelimiters":true', line)
        self.assertIn('"endianness":"big"', line)
        self.assertNotIn("checksumAlgorithm", line)

    def test_frame_is_start_payload_crc(self):
        device, out = make()
        device.binary("u8,i16", "big", b"\x7e", "CRC-16")
        self.pass_once(device, [1, 2])
        del out[:]
        self.pass_once(device, [1, -2])
        self.assertEqual(bytes(out[:4]), b"\x7e\x01\xff\xfe")
        self.assertEqual(len(out), 6)

    def test_wrong_value_count_raises(self):
        device, _ = make()
        device.binary("u8,u8")
        self.pass_once(device, [1, 2])
        with self.assertRaises(ValueError):
            self.pass_once(device, [1])

    def test_unknown_field_type_raises(self):
        device, _ = make()
        with self.assertRaises(ValueError):
            device.binary("f16")


class ReplyTests(unittest.TestCase):
    def frame(self, device, out):
        del out[:]
        device.group("G")
        device.channel("A", 1)
        device.send()
        return bytes(out)

    def test_tokened_command_appends_the_reply(self):
        device, out = make(on_control=lambda n, v: (3, "no, sorry"))
        device.group("G")
        device.channel("A", 1)
        device.control_group("C")
        device.control("Go", "button")
        device.send()
        self.assertEqual(self.frame(device, out), b"1\n")
        device.feed(b"#ss!Go=1@9\n")
        device.poll()
        self.assertEqual(self.frame(device, out), b"1,9,3,no  sorry\n")

    def test_command_without_a_token_leaves_frames_alone(self):
        device, out = make(on_control=lambda n, v: None)
        self.frame(device, out)
        device.feed(b"#ss!Go=1\n")
        device.poll()
        self.assertEqual(self.frame(device, out).count(b","), 0)


if __name__ == "__main__":
    unittest.main()
