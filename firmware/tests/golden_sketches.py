# Serial Studio device library
# https://serial-studio.com/
#
# Copyright (C) 2020-2026 Alex Spataru
#
# SPDX-License-Identifier: MIT

"""The golden sketches in Python: each one reproduces its .decl and .frames byte for byte."""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT.parent / "python"))

import serialstudio  # noqa: E402

GOLDEN = ROOT / "golden"


def _mpu6050(device, row):
    device.group("Accelerometer", "accelerometer")
    for name, value, axis in zip(("Accelerometer X", "Accelerometer Y", "Accelerometer Z"), row[0:3], "xyz"):
        device.channel(name, value, units="m/s²", widget=axis)
    device.group("Gyroscope", "gyro")
    for name, value, axis in zip(("Gyro X", "Gyro Y", "Gyro Z"), row[3:6], "xyz"):
        device.channel(name, value, units="deg/s", widget=axis)
    device.group("Temperature")
    device.channel("Temperature", row[6], units="℃", widget="bar", range=(0, 100), graph=False)


def _tinygps(device, row):
    device.group("GPS Map", "gps")
    device.channel("Latitude", row[0], places=4, units="°", widget="lat", graph=False)
    device.channel("Longitude", row[1], places=4, units="°", widget="lon", graph=False)
    device.channel("Altitude", row[2], places=1, units="m", widget="alt", graph=False)


def _hexadc(device, row):
    device.group("ADC Readings", "multiplot")
    for n in range(6):
        device.channel(
            "ADC %d" % n, row[n], units="Volts", plot_max=5, fft=True, fft_sampling_rate=200, graph=False
        )
    device.group("ADC Readings", "datagrid")
    for n in range(6):
        device.channel("ADC %d" % n, units="Volts", index=n + 1, graph=False)


def _controls(device, row):
    device.group("Status")
    device.channel(
        "Gear", row[0], labels={0: "Park", 1: "Reverse", 2: "Neutral", 3: "Drive"},
        labels_default="Unknown", graph=False,
    )
    device.channel(
        "Speed", row[1], places=1, units="km/h", widget="gauge", widget_min=0, widget_max=200,
        alarm_bands=[{"min": 150, "max": 200, "severity": 2, "label": "Overspeed"}], graph=False,
    )
    device.channel("Tank 2 Pressure", row[2], units="bar")
    device.control_group("Controls")
    device.control("Start", "button")
    device.control("Pump", "toggle", on_label="On", off_label="Off")
    device.control("Setpoint", "slider", min=0, max=255, step=1)
    device.control("Gain", "knob", min=0, max=10, step=0.5)


def _formats(device, row):
    device.group("Decoded")
    device.channel("Accel", row[0], units="g", format="fixed", format_width=16, format_signed=True,
                   format_fraction_bits=15)
    device.channel("Door", row[1], format="bits", format_width=8, format_bit_offset=3, format_bit_width=1,
                   labels={0: "Closed", 1: "Open"}, graph=False)
    device.channel("Temp", row[2], units="C", format="scale", format_scale=0.0625, format_offset=-40)


def _textfield(device, row):
    device.group("Status")
    device.channel("Level", row[0], units="m")
    device.control_group("Tuning")
    device.control("Frequency", "text", min=0, max=50, units="Hz", decimals=1)


def _tuner_command(name, value):
    return (2, "out of range") if value > 50 else None


def _binary_mpu(device):
    device.binary("f32,f32,f32,f32,f32,f32,f32", "little", b"\xaa\x55", "CRC-16")


SKETCHES = {
    "mpu6050": ("MPU6050", {}, _mpu6050,
                [[0.12, -0.34, 9.81, 0.01, -0.02, 0.03, 24.5], [0.15, -0.31, 9.79, 0.02, -0.01, 0.03, 24.56],
                 [-0.08, 0.22, 9.83, -0.05, 0.04, -0.01, 24.61]]),
    "tinygps": ("TinyGPS", {"named": True}, _tinygps,
                [[48.8584, 2.2945, 35.0], [48.8585, 2.2946, 35.4], [48.8586, 2.2947, None]]),
    "hexadc": ("Hexadecimal ADC", {"separator": ";"}, _hexadc,
               [["0,25", "1,50", "2,75", "3,00", "4,25", "4,90"], ["0,30", "1,55", "2,80", "3,05", "4,30", "4,95"],
                ["0,35", "1,60", "2,85", "3,10", "4,35", "5,00"]]),
    "controls": ("Pump Controller", {}, _controls,
                 [[3, 87.5, 1.2], [3, 92.0, 1.25], [1, 0.0, 1.25]]),
    "formats": ("Formats", {}, _formats, [[16384, 8, 1200], [8192, 0, 1210], [-4096, 8, 1190]]),
    "mpu6050_bin": ("MPU6050", {}, _mpu6050, None),
    "textfield": ("Tuner", {"on_control": _tuner_command}, _textfield,
                  [[1.5], [1.6], [1.7], [1.8]]),
}

SETUP = {"mpu6050_bin": _binary_mpu}
INPUTS = {"textfield": {1: b"#ss!Frequency=12@7\n", 2: b"#ss!Frequency=99@8\n"}}


def run(name):
    """The bytes the sketch writes: the boot declaration, then one frame per row.

    A binary frame is printed as a lowercase hex line, which is the golden form.
    """
    title, options, sketch, rows = SKETCHES[name]
    rows = rows or SKETCHES["mpu6050"][3]
    out = bytearray()

    def sink(data):
        out.extend(data if data.startswith(b"#ss") or name not in SETUP else data.hex().encode() + b"\n")

    device = serialstudio.Device(title, sink, **options)
    if name in SETUP:
        SETUP[name](device)

    sketch(device, rows[0])
    device.send()
    for index, row in enumerate(rows):
        if index in INPUTS.get(name, {}):
            device.feed(INPUTS[name][index])
            device.poll()

        sketch(device, row)
        device.send()

    return bytes(out)


def expected(name):
    return (GOLDEN / (name + ".decl")).read_bytes() + (GOLDEN / (name + ".frames")).read_bytes()
