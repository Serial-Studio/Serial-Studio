#!/usr/bin/env python3
"""
Lorenz Attractor Data Generator - UDP Version

This program simulates the Lorenz system, a set of three chaotic differential
equations, and transmits the generated data (x, y, z) over UDP to port 9000.
The output is ideal for visualizing the Lorenz attractor in real-time using
Serial Studio.

Lorenz System Parameters:
- σ (sigma): 10.0      (controls the rate of rotation in the attractor)
- ρ (rho):   28.0      (sets the "height" of the attractor)
- β (beta):  8.0 / 3.0 (controls the damping)

Usage:
    python3 lorenz_udp.py

Serial Studio Configuration:
1. Open Serial Studio
2. Select Network (UDP) as the data source
3. Set port to 9000
4. Load the LorenzAttractor.json project file
5. Start receiving data

Author: Alex Spataru
"""

import socket
import time


class LorenzAttractor:
    def __init__(self, sigma=10.0, rho=28.0, beta=8.0/3.0, dt=0.01):
        self.sigma = sigma
        self.rho = rho
        self.beta = beta
        self.dt = dt

        self.x = 0.1
        self.y = 0.0
        self.z = 0.0

    def step(self):
        dx = self.sigma * (self.y - self.x) * self.dt
        dy = (self.x * (self.rho - self.z) - self.y) * self.dt
        dz = (self.x * self.y - self.beta * self.z) * self.dt

        self.x += dx
        self.y += dy
        self.z += dz

        return self.x, self.y, self.z


def main():
    host = '127.0.0.1'
    port = 9000
    transmission_interval = 0.001

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    lorenz = LorenzAttractor()

    print(f"Lorenz Attractor UDP Sender")
    print(f"Sending data to {host}:{port}")
    print(f"Transmission interval: {transmission_interval*1000:.1f}ms")
    print(f"Press Ctrl+C to stop\n")

    try:
        packet_count = 0
        while True:
            x, y, z = lorenz.step()

            message = f"{x:.6f},{y:.6f},{z:.6f}\n"

            sock.sendto(message.encode('utf-8'), (host, port))

            packet_count += 1
            if packet_count % 1000 == 0:
                print(f"Sent {packet_count} packets - Current: x={x:.3f}, y={y:.3f}, z={z:.3f}")

            time.sleep(transmission_interval)

    except KeyboardInterrupt:
        print(f"\nStopped. Total packets sent: {packet_count}")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
