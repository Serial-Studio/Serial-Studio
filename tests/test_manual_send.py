#!/usr/bin/env python3
"""
Manual test script to send frames to Serial Studio.

This script:
1. Connects to Serial Studio API (port 7777) to configure it
2. Sets operation mode to "Device Sends JSON"
3. Configures network device (TCP server on port 9000)
4. Connects to the device port and sends telemetry frames
"""

import json
import socket
import time
from utils.api_client import SerialStudioClient
from utils.data_generator import DataGenerator, ChecksumType


def main():
    # Configuration
    API_HOST = "127.0.0.1"
    API_PORT = 7777
    DEVICE_HOST = "127.0.0.1"
    DEVICE_PORT = 9000
    # NOTE: DeviceSendsJSON mode doesn't use checksums - they're ignored
    DELAY = 1.0

    print("Step 1: Configure Serial Studio via API...")
    try:
        with SerialStudioClient(API_HOST, API_PORT) as api:
            # Set operation mode to "Device Sends JSON"
            print("  - Setting operation mode to 'Device Sends JSON'")
            api.set_operation_mode("json")

            # Configure network device
            print(f"  - Configuring TCP server on port {DEVICE_PORT}")
            api.configure_network(host=DEVICE_HOST, port=DEVICE_PORT, socket_type="tcp")

            # Connect device
            print("  - Connecting device...")
            api.connect_device()
            time.sleep(1.0)

            status = api.get_status()
            if not status["isConnected"]:
                print("ERROR: Device failed to connect!")
                return

            print("  - Device connected successfully!")
    except Exception as e:
        print(f"ERROR configuring Serial Studio: {e}")
        return

    print()
    print(f"Step 2: Send frames to device port {DEVICE_PORT}...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((DEVICE_HOST, DEVICE_PORT))
        print("Connected to device port!")
        print()

        # Generate and send frames
        for i in range(10):
            # Generate realistic telemetry
            frame_data = DataGenerator.generate_json_frame()
            payload = json.dumps(frame_data)

            # Wrap frame (DeviceSendsJSON mode = pure JSON + newline)
            frame = DataGenerator.wrap_frame(payload)

            print(f"Frame {i+1}:")
            print(f"  Payload length: {len(payload)} bytes")
            print(f"  Frame length: {len(frame)} bytes")
            print(f"  First 100 chars: {frame[:100]}")
            print(f"  Last 20 chars: {repr(frame[-20:])}")
            print()

            # Send frame
            sock.sendall(frame)
            time.sleep(DELAY)

        print("Done sending frames.")

    except ConnectionRefusedError:
        print(f"ERROR: Could not connect to {HOST}:{PORT}")
        print("Make sure Serial Studio is running and configured for TCP Server on port 9000")
    except Exception as e:
        print(f"ERROR: {e}")
    finally:
        sock.close()


if __name__ == "__main__":
    main()
