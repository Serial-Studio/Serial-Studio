#!/usr/bin/env python3
"""
Vibration Test Rig - UDP simulator for the Painter and Waterfall widgets.

Simulates a small electric motor on a test bench. The motor sweeps its RPM
up and down through a fixed band; an accelerometer mounted on the bracket
picks up the resulting vibration, and a stereo microphone in the enclosure
captures the airborne noise.

Each UDP frame carries four comma-separated values:

    vibration,rpm,mic_left,mic_right\n

  * vibration  - g, single high-rate accelerometer channel
  * rpm        - current motor speed (Hz expressed as RPM)
  * mic_left   - left microphone level in dB FS
  * mic_right  - right microphone level in dB FS

Frames are emitted at 1 kHz so the Waterfall widget (256-sample FFT at
1 kHz sampling) renders a clean order-tracking spectrogram with RPM as
the Y axis. The Painter widgets read the same datasets:

  * Audio VU meter   reads mic_left + mic_right
  * Dial gauge       reads peak vibration with a smoothed needle
  * Bars + peak hold reads vibration broken into 4 frequency bands

Usage:
    python3 vibration_test_rig.py

Serial Studio:
  1. Open Serial Studio.
  2. Load VibrationTestRig.ssproj.
  3. The project is preconfigured for UDP on port 9000.
  4. Hit Connect.

Author: Alex Spataru
"""

import sys

# Force UTF-8 console output: Windows defaults to cp1252, which cannot encode
# the Unicode characters this script prints (e.g. arrows / check marks).
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8")
    except (AttributeError, ValueError):
        pass

import argparse
import math
import random
import socket
import time


def main():
    parser = argparse.ArgumentParser(description="Vibration test rig UDP simulator")
    parser.add_argument(
        "--host", default="127.0.0.1", help="UDP destination host (default: 127.0.0.1)"
    )
    parser.add_argument(
        "--port", type=int, default=9000, help="UDP destination port (default: 9000)"
    )
    parser.add_argument(
        "--rate",
        type=float,
        default=1000.0,
        help="Sample/emission rate in Hz (default: 1000)",
    )
    parser.add_argument(
        "--rpm-min", type=float, default=900.0, help="Minimum motor RPM (default: 900)"
    )
    parser.add_argument(
        "--rpm-max",
        type=float,
        default=4800.0,
        help="Maximum motor RPM (default: 4800)",
    )
    parser.add_argument(
        "--sweep-period",
        type=float,
        default=18.0,
        help="Seconds for one full RPM sweep up + down (default: 18)",
    )
    parser.add_argument(
        "--imbalance",
        type=float,
        default=1.4,
        help="1x rotational imbalance amplitude in g (default: 1.4)",
    )
    parser.add_argument(
        "--bearing-amp",
        type=float,
        default=0.45,
        help="Bearing-defect harmonic amplitude in g (default: 0.45)",
    )
    parser.add_argument(
        "--noise",
        type=float,
        default=0.08,
        help="Broadband noise amplitude in g (default: 0.08)",
    )
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    addr = (args.host, args.port)

    dt = 1.0 / args.rate
    omega_sweep = 2.0 * math.pi / args.sweep_period

    # Motor harmonic structure:
    #   1x  : rotational imbalance (dominant line in the waterfall)
    #   2x  : misalignment ghost
    #   3x  : minor coupling artefact
    #   bearing defect at ~5.43x (non-integer ratio, characteristic of
    #   a rolling-element bearing fault)
    harmonics = [
        (1.0, args.imbalance),
        (2.0, args.imbalance * 0.35),
        (3.0, args.imbalance * 0.18),
        (5.43, args.bearing_amp),
    ]

    print("Vibration Test Rig (Painter + Waterfall demo)")
    print(f"  Target        : {args.host}:{args.port}")
    print(f"  Sample rate   : {args.rate:.0f} Hz")
    print(f"  RPM range     : {args.rpm_min:.0f} - {args.rpm_max:.0f}")
    print(f"  Sweep period  : {args.sweep_period:.1f} s")
    print(f"  Press Ctrl+C to stop\n")

    t = 0.0
    next_send = time.perf_counter()
    phases = [0.0] * len(harmonics)
    last_log = time.perf_counter()
    sent = 0

    try:
        while True:
            sweep = 0.5 * (1.0 - math.cos(omega_sweep * t))
            rpm = args.rpm_min + (args.rpm_max - args.rpm_min) * sweep
            f1 = rpm / 60.0

            vib = 0.0
            for i, (mult, amp) in enumerate(harmonics):
                phases[i] += 2.0 * math.pi * f1 * mult * dt
                if phases[i] > 2.0 * math.pi:
                    phases[i] -= 2.0 * math.pi
                vib += amp * math.sin(phases[i])

            vib += args.noise * (random.random() - 0.5) * 2.0

            # Audio: louder when vibration is high; left vs right slightly
            # decorrelated to give the VU meters something to do.
            mic_env = 0.55 * abs(vib) + 0.45 * sweep
            mic_jitter_l = 0.10 * (random.random() - 0.5)
            mic_jitter_r = 0.10 * (random.random() - 0.5)
            level_l = mic_env + mic_jitter_l
            level_r = mic_env + mic_jitter_r
            mic_l_db = clamp_db(amplitude_to_db(level_l))
            mic_r_db = clamp_db(amplitude_to_db(level_r))

            payload = f"{vib:.4f},{rpm:.1f},{mic_l_db:.2f},{mic_r_db:.2f}\n"
            sock.sendto(payload.encode("ascii"), addr)
            sent += 1

            t += dt
            next_send += dt
            sleep_for = next_send - time.perf_counter()
            if sleep_for > 0:
                time.sleep(sleep_for)
            else:
                next_send = time.perf_counter()

            now = time.perf_counter()
            if now - last_log >= 2.0:
                rate = sent / (now - last_log)
                print(
                    f"  {sent:6d} frames sent  |  {rate:7.1f} Hz  |  "
                    f"rpm={rpm:6.0f}  vib={vib:+5.2f} g"
                )
                sent = 0
                last_log = now

    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        sock.close()


def amplitude_to_db(level):
    if level <= 1e-6:
        return -80.0
    return 20.0 * math.log10(level)


def clamp_db(value, lo=-60.0, hi=6.0):
    if value < lo:
        return lo
    if value > hi:
        return hi
    return value


if __name__ == "__main__":
    main()
