#!/usr/bin/env python3
"""
Electromagnetic Wave Simulator - UDP Version

Simulates a propagating EM wave packet ("photon") with orthogonal electric
and magnetic field vectors, as described by Maxwell's equations.

Physical model (Gaussian-enveloped linearly polarized wave packet):

    E(x,t) = E0 * exp(-((x - x0 - c*t) / sigma)^2) * sin(k*(x - c*t))
    B(x,t) = B0 * exp(-((x - x0 - c*t) / sigma)^2) * sin(k*(x - c*t))

    E oscillates along Y, B along Z — mutually perpendicular and
    perpendicular to the propagation direction X. The Gaussian envelope
    localizes the wave into a packet that travels at speed c.

    B0 = E0 / c (SI), but for visualization both are scaled equally.

The packet starts off-screen left, propagates rightward through the view,
exits right, then loops back — giving the appearance of a photon flying by.

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
    parser.add_argument("--wavelength", type=float, default=12.0,
                        help="Carrier wavelength in arbitrary units")
    parser.add_argument("--speed", type=float, default=20.0,
                        help="Wave propagation speed (units/s)")
    parser.add_argument("--amplitude", type=float, default=10.0,
                        help="E-field peak amplitude")
    parser.add_argument("--sigma", type=float, default=20.0,
                        help="Gaussian envelope width (spatial std dev)")
    parser.add_argument("--samples", type=int, default=80,
                        help="Spatial sample points per frame")
    parser.add_argument("--interval", type=float, default=0.025,
                        help="Time between frames in seconds")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Wave parameters
    c = args.speed
    k = 2.0 * math.pi / args.wavelength
    E0 = args.amplitude
    sigma = args.sigma

    # Spatial view window: wide enough to see the full packet
    window = 6.0 * sigma
    dx = window / args.samples

    # The packet center starts off-screen left and travels right.
    # Total travel distance includes margins so the packet fully
    # enters and exits the view.
    margin = 3.0 * sigma
    travel = window + 2.0 * margin
    cycle_time = travel / c

    print("EM Wave Simulator (Photon Wave Packet)")
    print(f"  Target      : {args.host}:{args.port}")
    print(f"  Wavelength  : {args.wavelength}")
    print(f"  Speed       : {c} units/s")
    print(f"  Frequency   : {c / args.wavelength:.3f} Hz")
    print(f"  E0          : {E0}")
    print(f"  Envelope σ  : {sigma}")
    print(f"  View window : {window:.1f} units")
    print(f"  Samples     : {args.samples} pts/frame")
    print(f"  Frame rate  : {1.0 / args.interval:.0f} fps")
    print(f"  Cycle time  : {cycle_time:.2f} s")
    print("Press Ctrl+C to stop\n")

    t = 0.0
    frame = 0

    try:
        while True:
            # View window is fixed
            x_start = 0.0

            # Packet center position (loops after full traversal)
            t_mod = math.fmod(t, cycle_time)
            packet_center = -margin + c * t_mod

            for i in range(args.samples):
                x = x_start + i * dx

                # Distance from packet center
                d = x - packet_center

                # Gaussian envelope
                envelope = math.exp(-(d * d) / (2.0 * sigma * sigma))

                # Carrier oscillation
                phase = k * (x - c * t)
                carrier = math.sin(phase)

                # E-field along Y, B-field along Z (perpendicular)
                ey = E0 * envelope * carrier
                bz = E0 * envelope * carrier

                msg = f"{x:.4f},{ey:.4f},{bz:.4f}\n"
                sock.sendto(msg.encode("utf-8"), (args.host, args.port))

            frame += 1
            t += args.interval

            if frame % 100 == 0:
                print(f"Frame {frame} — t={t:.2f}s, "
                      f"packet center={packet_center:.1f}")

            time.sleep(args.interval)

    except KeyboardInterrupt:
        print(f"\nStopped after {frame} frames.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
