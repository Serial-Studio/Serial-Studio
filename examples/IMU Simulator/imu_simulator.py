#!/usr/bin/env python3
"""
IMU Batched Data Simulator

Simulates an IMU sensor that batches high-frequency accelerometer readings
before transmission.

Use case: A sensor samples accelerometer at 120 Hz but transmits
data packets at 1 Hz, each containing:
- Scalar values (battery, temperature)
- Batched accelerometer readings (120 samples)

This demonstrates Serial Studio's multi-frame parsing where one packet
automatically generates 120 frames for smooth visualization.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import math
import socket
import time
from datetime import datetime


class IMU_Simulator:
    """Simulates an IMU device with batched sensor readings."""

    def __init__(self, host="127.0.0.1", port=9000, sample_rate=120):
        """
        Initialize the simulator.

        Args:
            host: UDP destination host
            port: UDP destination port
            sample_rate: Accelerometer sampling rate (Hz)
        """
        self.host = host
        self.port = port
        self.sample_rate = sample_rate
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Simulation state
        self.time = 0.0
        self.battery_voltage = 3.7

    def generate_motion_pattern(self, t, axis="x"):
        """
        Generate realistic accelerometer motion pattern.

        Simulates a combination of:
        - Gravity (1g baseline)
        - Walking motion (sinusoidal)
        - Random noise
        """
        if axis == "x":
            # Lateral sway during walking
            base = 0.2 * math.sin(2.0 * math.pi * 0.8 * t)
        elif axis == "y":
            # Forward-backward motion
            base = 0.3 * math.sin(2.0 * math.pi * 1.2 * t)
        else:  # z-axis
            # Vertical motion + gravity
            base = 9.81 + 0.5 * math.sin(2.0 * math.pi * 1.5 * t)

        # Add high-frequency vibration
        vibration = 0.05 * math.sin(2.0 * math.pi * 20.0 * t)

        return base + vibration

    def generate_batched_packet(self):
        """
        Generate one batched sensor packet.

        Returns a JSON object containing:
        - timestamp: Current Unix timestamp
        - battery: Battery voltage (3.0-4.2V)
        - temperature: Temperature in Celsius
        - accel_x: Array of 120 accelerometer X samples
        - accel_y: Array of 120 accelerometer Y samples
        - accel_z: Array of 120 accelerometer Z samples
        """
        dt = 1.0 / self.sample_rate

        accel_x = []
        accel_y = []
        accel_z = []

        for i in range(self.sample_rate):
            t = self.time + i * dt
            accel_x.append(round(self.generate_motion_pattern(t, "x"), 3))
            accel_y.append(round(self.generate_motion_pattern(t, "y"), 3))
            accel_z.append(round(self.generate_motion_pattern(t, "z"), 3))

        # Simulate battery discharge
        self.battery_voltage -= 0.0001
        if self.battery_voltage < 3.0:
            self.battery_voltage = 4.2

        # Simulate temperature variation
        temperature = 25.0 + 5.0 * math.sin(2.0 * math.pi * self.time / 60.0)

        packet = {
            "timestamp": int(time.time()),
            "battery": round(self.battery_voltage, 2),
            "temperature": round(temperature, 1),
            "accel_x": accel_x,
            "accel_y": accel_y,
            "accel_z": accel_z,
        }

        self.time += 1.0
        return packet

    def send_packet(self, packet):
        """Send packet via UDP with Serial Studio framing."""
        payload = json.dumps(packet)
        frame = f"/*{payload}*/\n"
        self.sock.sendto(frame.encode("utf-8"), (self.host, self.port))

    def run(self, duration=None, packet_rate=1.0):
        """
        Run the simulator.

        Args:
            duration: Run duration in seconds (None = infinite)
            packet_rate: Packets per second (default 1 Hz)
        """
        print(f"IMU Batched Data Simulator")
        print(f"Sending to {self.host}:{self.port}")
        print(f"Sample rate: {self.sample_rate} Hz")
        print(f"Packet rate: {packet_rate} Hz")
        print(f"Samples per packet: {self.sample_rate}")
        print()
        print("Configure Serial Studio:")
        print("1. Set Bus Type: UDP Client")
        print("2. Host: 127.0.0.1, Port: 9000")
        print("3. Load project: imu_batched.ssproj")
        print("4. Connect")
        print()

        start_time = time.time()
        packet_interval = 1.0 / packet_rate
        packet_count = 0

        try:
            while True:
                if duration and (time.time() - start_time) > duration:
                    break

                packet = self.generate_batched_packet()
                self.send_packet(packet)
                packet_count += 1

                if packet_count % 10 == 0:
                    print(
                        f"Sent {packet_count} packets "
                        f"({packet_count * self.sample_rate} frames total)"
                    )

                time.sleep(packet_interval)

        except KeyboardInterrupt:
            print("\nSimulator stopped by user")
        finally:
            self.sock.close()
            print(f"\nTotal packets sent: {packet_count}")
            print(f"Total frames generated: {packet_count * self.sample_rate}")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="IMU Batched Data Simulator")
    parser.add_argument(
        "--host", default="127.0.0.1", help="UDP destination host (default: 127.0.0.1)"
    )
    parser.add_argument(
        "--port", type=int, default=9000, help="UDP destination port (default: 9000)"
    )
    parser.add_argument(
        "--sample-rate",
        type=int,
        default=120,
        help="Accelerometer sample rate in Hz (default: 120)",
    )
    parser.add_argument(
        "--packet-rate",
        type=float,
        default=1.0,
        help="Packet transmission rate in Hz (default: 1.0)",
    )
    parser.add_argument(
        "--duration", type=float, help="Run duration in seconds (default: infinite)"
    )

    args = parser.parse_args()

    simulator = IMU_Simulator(
        host=args.host, port=args.port, sample_rate=args.sample_rate
    )

    simulator.run(duration=args.duration, packet_rate=args.packet_rate)
