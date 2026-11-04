#!/usr/bin/env python3
# Serial Studio device library
# https://serial-studio.com/
#
# Copyright (C) 2020-2026 Alex Spataru
#
# SPDX-License-Identifier: MIT

"""Byte-compare each library's golden-sketch output with firmware/tests/golden.

The Python half always runs. The C and Arduino halves run when their host binaries
exist (CI builds them, see .github/workflows/ci.yml): pass --c-bin / --arduino-bin,
or leave them to the default build/ locations. A binary is run as `<bin> <sketch>`
and prints the boot declaration followed by the frames.

Usage: python3 firmware/tests/check_golden.py [--c-bin PATH] [--arduino-bin PATH]
Exit codes: 0 all present halves match, 1 a mismatch.
"""

import argparse
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import golden_sketches  # noqa: E402


def first_difference(got, want):
    for offset, (a, b) in enumerate(zip(got, want)):
        if a != b:
            return "first difference at byte %d: got %r, want %r" % (offset, got[offset:offset + 24], want[offset:offset + 24])

    return "length differs: got %d bytes, want %d" % (len(got), len(want))


def check(label, produce):
    failures = 0
    for name in golden_sketches.SKETCHES:
        want = golden_sketches.expected(name)
        try:
            got = produce(name)
        except (OSError, subprocess.SubprocessError) as error:
            print("check_golden: FAIL %s %s: %s" % (label, name, error))
            failures += 1
            continue

        if got == want:
            print("check_golden: ok   %s %s" % (label, name))
        else:
            print("check_golden: FAIL %s %s: %s" % (label, name, first_difference(got, want)))
            failures += 1

    return failures


def runner(binary):
    return lambda name: subprocess.run([str(binary), name], capture_output=True, check=True, timeout=30).stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--c-bin", type=Path, default=HERE / "build" / "c_golden")
    parser.add_argument("--arduino-bin", type=Path, default=HERE / "build" / "arduino_golden")
    args = parser.parse_args()

    failures = check("python", golden_sketches.run)
    for label, binary in (("c", args.c_bin), ("arduino", args.arduino_bin)):
        if binary.is_file():
            failures += check(label, runner(binary))
        else:
            print("check_golden: skip %s (no host binary at %s)" % (label, binary))

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
