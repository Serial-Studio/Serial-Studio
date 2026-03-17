#!/usr/bin/env python3
"""
Electromagnetic Wave Simulator - UDP Version

Simulates a propagating EM plane wave with orthogonal electric and magnetic
field vectors, as described by Maxwell's equations.

Physical model (linearly polarized plane wave):

    E(x,t) = E0 * sin(k*x - omega*t)   along Y (electric field)
    B(x,t) = B0 * sin(k*x - omega*t)   along Z (magnetic field)

    E and B are perpendicular to each other and to the propagation axis (X).
    The wave propagates at c = omega / k.

The simulator samples the wave spatially at each time step and sends one
point per UDP packet. The sliding window keeps the wave centered in view.

Usage:
    python3 em_wave_udp.py

Serial Studio Configuration:
1. Open Serial Studio
2. Select Network (UDP) as the data source
3. Set port to 9000
4. Load the EMWaveSimulator.ssproj project file
5. Start receiving data

Author: Alex Spataru
"""

import math
import socket
import time
import argparse


def main():
    parser = argparse.ArgumentParser(description="EM Wave UDP Simulator")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--wavelength", type=float, default=40.0,
                        help="Wavelength in arbitrary units (default: 40)")
    parser.add_argument("--speed", type=float, default=8.0,
                        help="Wave propagation speed (units/s)")
    parser.add_argument("--amplitude", type=float, default=10.0,
                        help="E-field amplitude")
    parser.add_argument("--samples", type=int, default=60,
                        help="Spatial sample points per frame")
    parser.add_argument("--interval", type=float, default=0.03,
                        help="Time between frames in seconds")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Wave parameters
    c = args.speed
    k = 2.0 * math.pi / args.wavelength
    omega = c * k
    E0 = args.amplitude

    # Spatial window: 2 full wavelengths
    window = 2.0 * args.wavelength
    dx = window / args.samples

    print("EM Wave Simulator")
    print(f"  Target     : {args.host}:{args.port}")
    print(f"  Wavelength : {args.wavelength}")
    print(f"  Speed      : {c} units/s")
    print(f"  Frequency  : {omega / (2 * math.pi):.3f} Hz")
    print(f"  E0         : {E0}")
    print(f"  Samples    : {args.samples} pts/frame")
    print(f"  Frame rate : {1.0 / args.interval:.0f} fps")
    print("Press Ctrl+C to stop\n")

    t = 0.0
    frame = 0

    try:
        while True:
            # Slide the window so the wave stays centered
            x_start = c * t - window * 0.5

            for i in range(args.samples):
                x = x_start + i * dx
                phase = k * x - omega * t
                ey = E0 * math.sin(phase)
                bz = E0 * math.sin(phase)

                msg = f"{x:.4f},{ey:.4f},{bz:.4f}\n"
                sock.sendto(msg.encode("utf-8"), (args.host, args.port))

            frame += 1
            t += args.interval

            if frame % 100 == 0:
                print(f"Frame {frame} — t={t:.2f}s, "
                      f"window [{x_start:.1f}, {x_start + window:.1f}]")

            time.sleep(args.interval)

    except KeyboardInterrupt:
        print(f"\nStopped after {frame} frames.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
