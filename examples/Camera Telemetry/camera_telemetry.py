#!/usr/bin/env python3
"""
Camera Telemetry for Serial Studio

Captures frames from the system camera and streams them over UDP as fast as
possible.  Only FPS and frame count are sent as telemetry — no analytics.

Everything goes through the same byte stream to 127.0.0.1:9000:
  - JPEG image frames (raw bytes, autodetected by Serial Studio's Image widget)
  - CSV telemetry frames ($fps,frame_count;\\n)

Serial Studio's FrameReader handles CSV frames (delimited by $ / ;) while the
Image widget's ImageFrameReader independently scans the same raw byte stream for
JPEG magic bytes (FF D8 FF ... FF D9), completely bypassing the telemetry path.

Usage:
  python3 camera_telemetry.py               # camera index 0, port 9000
  python3 camera_telemetry.py --camera 1    # alternate camera
  python3 camera_telemetry.py --port 9001   # alternate port
  python3 camera_telemetry.py --fps 30      # target frame rate
  python3 camera_telemetry.py --quality 85  # JPEG quality (1-100)
  python3 camera_telemetry.py --no-camera   # synthetic test pattern (no hardware)

Dependencies:
  pip install opencv-python
"""

import argparse
import math
import socket
import sys
import time

try:
    import cv2
    import numpy as np
    HAS_CV2 = True
except ImportError:
    HAS_CV2 = False


# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

UDP_HOST = "127.0.0.1"
UDP_PORT = 9000
TARGET_FPS = 30
JPEG_QUALITY = 85
MAX_FRAME_WIDTH = 640


# ---------------------------------------------------------------------------
# Synthetic test pattern (when no camera available)
# ---------------------------------------------------------------------------

def make_synthetic_frame(t, width=640, height=480):
    """Generate a synthetic colour gradient frame that changes over time."""
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    for x in range(width):
        hue = int((x / width * 180 + t * 30) % 180)
        frame[:, x] = [hue, 200, 200]
    frame = cv2.cvtColor(frame, cv2.COLOR_HSV2BGR)
    cx = int(width / 2 + width / 3 * math.sin(t * 0.7))
    cy = int(height / 2 + height / 4 * math.cos(t * 1.1))
    cv2.circle(frame, (cx, cy), 40, (255, 255, 255), -1)
    return frame


# ---------------------------------------------------------------------------
# UDP sender
# ---------------------------------------------------------------------------

class UDPSender:
    def __init__(self, host, port):
        self.addr = (host, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.chunk_size = 8_192

    def send(self, data: bytes):
        """Send raw bytes, fragmenting if needed."""
        for offset in range(0, len(data), self.chunk_size):
            self.sock.sendto(data[offset:offset + self.chunk_size], self.addr)

    def send_text(self, text: str):
        self.sock.sendto(text.encode(), self.addr)

    def close(self):
        self.sock.close()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def parse_args():
    p = argparse.ArgumentParser(description="Camera Telemetry for Serial Studio")
    p.add_argument("--camera", type=int, default=0, metavar="INDEX",
                   help="Camera device index (default: 0)")
    p.add_argument("--port", type=int, default=UDP_PORT, metavar="PORT",
                   help="UDP destination port (default: 9000)")
    p.add_argument("--fps", type=float, default=TARGET_FPS, metavar="FPS",
                   help="Target frame rate (default: 30)")
    p.add_argument("--quality", type=int, default=JPEG_QUALITY, metavar="Q",
                   help="JPEG quality 1-100 (default: 85)")
    p.add_argument("--no-camera", action="store_true",
                   help="Use synthetic test pattern instead of camera")
    return p.parse_args()


def open_camera(index, fps):
    if not HAS_CV2:
        return None
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        return None
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, MAX_FRAME_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cap.set(cv2.CAP_PROP_FPS, fps)
    return cap


def encode_jpeg(frame_bgr, quality):
    """Encode a BGR frame as JPEG bytes, downscaling if needed."""
    h, w = frame_bgr.shape[:2]
    if w > MAX_FRAME_WIDTH:
        scale = MAX_FRAME_WIDTH / w
        frame_bgr = cv2.resize(frame_bgr, (MAX_FRAME_WIDTH, int(h * scale)),
                               interpolation=cv2.INTER_LINEAR)
    ok, buf = cv2.imencode(".jpg", frame_bgr, [cv2.IMWRITE_JPEG_QUALITY, quality])
    if not ok:
        return None
    return bytes(buf)


def main():
    args = parse_args()

    if not HAS_CV2:
        print("ERROR: opencv-python is not installed.")
        print("       Install with:  pip install opencv-python")
        sys.exit(1)

    use_camera = not args.no_camera
    cap = None

    if use_camera:
        cap = open_camera(args.camera, args.fps)
        if cap is None:
            print(f"WARNING: could not open camera {args.camera}, falling back to synthetic pattern.")
            use_camera = False

    sender = UDPSender(UDP_HOST, args.port)
    interval = 1.0 / args.fps

    print(f"Camera Telemetry → udp://{UDP_HOST}:{args.port}  @ {args.fps:.0f} Hz  quality={args.quality}")
    print("Source:", f"camera {args.camera}" if use_camera else "synthetic pattern")
    print("Press Ctrl+C to stop.\n")
    print(f"{'Frame':>8}  {'FPS':>6}  {'JPEG kB':>8}")
    print("-" * 28)

    frame_count = 0
    t_start = time.monotonic()

    try:
        while True:
            t0 = time.monotonic()
            t = t0 - t_start

            if use_camera:
                ok, frame_bgr = cap.read()
                if not ok:
                    print("Camera read failed, switching to synthetic pattern.")
                    use_camera = False
                    frame_bgr = make_synthetic_frame(t)
            else:
                frame_bgr = make_synthetic_frame(t)

            frame_count += 1

            jpeg_bytes = encode_jpeg(frame_bgr, args.quality)
            if jpeg_bytes:
                sender.send(jpeg_bytes)

            fps_actual = frame_count / max(t, 0.001)
            sender.send(b"\xab\xcd\xef" + f"{fps_actual:.1f},{frame_count}".encode() + b"\xfe\xed")

            jpeg_kb = len(jpeg_bytes) / 1024.0 if jpeg_bytes else 0.0
            print(f"{frame_count:>8}  {fps_actual:>6.1f}  {jpeg_kb:>8.1f}", end="\r")

            elapsed = time.monotonic() - t0
            sleep_time = interval - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)

    except KeyboardInterrupt:
        print(f"\n\nStopped after {frame_count} frames.")
    finally:
        if cap is not None:
            cap.release()
        sender.close()


if __name__ == "__main__":
    main()
