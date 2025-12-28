#!/usr/bin/env python3
"""
UDP ADC Data Sender

Sends simulated ADC data to UDP port 9000 on localhost.
Replicates the data format from HexadecimalADC.ino:
- Start delimiter: 0xC0 0xDE
- 6 sensor values (1 byte each, 0-255)
- CRC-16-CCITT checksum (2 bytes, big-endian)
"""

import socket
import time
import random
import struct


def crc16_ccitt(data):
    """
    Calculate CRC-16-CCITT checksum.
    Polynomial: 0x1021, Initial value: 0x0000
    """
    crc = 0x0000
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def create_data_frame():
    """
    Create a data frame with start delimiter, 6 ADC values, and CRC checksum.
    Returns bytes ready to send over UDP.
    """
    payload = bytearray(6)
    for i in range(6):
        payload[i] = random.randint(0, 255)

    crc = crc16_ccitt(payload)

    frame = bytearray()
    frame.append(0xC0)
    frame.append(0xDE)
    frame.extend(payload)
    frame.append((crc >> 8) & 0xFF)
    frame.append(crc & 0xFF)

    return bytes(frame)


def main():
    """
    Main function to send ADC data frames via UDP.
    """
    host = 'localhost'
    port = 9000

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    print(f"Sending ADC data to UDP {host}:{port}")
    print("Press Ctrl+C to stop")
    print("-" * 50)

    frame_count = 0

    try:
        while True:
            frame = create_data_frame()
            sock.sendto(frame, (host, port))

            frame_count += 1

            hex_string = ' '.join(f'{b:02X}' for b in frame)
            print(f"Frame {frame_count}: {hex_string}")

            time.sleep(0.1)

    except KeyboardInterrupt:
        print(f"\n\nStopped. Sent {frame_count} frames.")
    finally:
        sock.close()


if __name__ == '__main__':
    main()
