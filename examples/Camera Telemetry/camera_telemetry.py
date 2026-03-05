#!/usr/bin/env python3
"""
Camera Telemetry Simulator for Serial Studio

Captures frames from the default system camera at ~24 Hz and streams them over
a single UDP connection alongside computed image analytics (luminance, contrast,
sharpness, motion, dominant hue).

Everything goes through the same byte stream to 127.0.0.1:9000:
  - JPEG image frames (raw bytes, autodetected by Serial Studio's Image widget)
  - CSV telemetry frames ($metric1,metric2,...;\\n) for normal dashboard widgets

Serial Studio's FrameReader handles CSV frames (delimited by $ / ;) while the
Image widget's ImageFrameReader independently scans the same raw byte stream for
JPEG magic bytes (FF D8 FF ... FF D9), completely bypassing the telemetry path.

Usage:
  python3 camera_telemetry.py               # camera index 0, port 9000
  python3 camera_telemetry.py --camera 1    # alternate camera
  python3 camera_telemetry.py --port 9001   # alternate port
  python3 camera_telemetry.py --fps 10      # lower frame rate
  python3 camera_telemetry.py --no-camera   # synthetic test pattern (no hardware)

Dependencies:
  pip install opencv-python numpy
"""

import argparse
import math
import socket
import sys
import time

# ---------------------------------------------------------------------------
# Optional imports — degrade gracefully when no camera / no OpenCV
# ---------------------------------------------------------------------------

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
TARGET_FPS = 24
JPEG_QUALITY = 70       # lower = smaller packets, less latency
MAX_FRAME_WIDTH = 640   # resize if camera returns larger frames


# ---------------------------------------------------------------------------
# Image analytics
# ---------------------------------------------------------------------------

def compute_metrics(frame_bgr):
    """Return (luminance, contrast, sharpness, motion_score, hue_deg) from a BGR frame."""
    gray = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)
    hsv = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2HSV)

    # Luminance: mean Y in YCrCb (perceptual)
    ycrcb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2YCrCb)
    luminance = float(np.mean(ycrcb[:, :, 0]))

    # Contrast: standard deviation of grayscale
    contrast = float(np.std(gray))

    # Sharpness: variance of Laplacian (higher = sharper)
    laplacian = cv2.Laplacian(gray, cv2.CV_64F)
    sharpness = float(np.var(laplacian))

    # Dominant hue: circular mean of hue channel (0–180 in OpenCV, scale to 0–360)
    hue = hsv[:, :, 0].astype(np.float32) * 2.0   # → degrees
    # Circular mean using unit-vector method
    sin_mean = float(np.mean(np.sin(np.deg2rad(hue))))
    cos_mean = float(np.mean(np.cos(np.deg2rad(hue))))
    hue_deg = float(math.degrees(math.atan2(sin_mean, cos_mean))) % 360.0

    return luminance, contrast, sharpness, hue_deg


def compute_motion(prev_gray, curr_gray):
    """Optical-flow magnitude between two grayscale frames (0–100 scale)."""
    if prev_gray is None:
        return 0.0
    flow = cv2.calcOpticalFlowFarneback(
        prev_gray, curr_gray,
        None, 0.5, 3, 15, 3, 5, 1.2, 0
    )
    magnitude = np.sqrt(flow[..., 0] ** 2 + flow[..., 1] ** 2)
    return float(np.mean(magnitude) * 5.0)   # scale to ~0–100 range


# ---------------------------------------------------------------------------
# Synthetic test pattern (when no camera available)
# ---------------------------------------------------------------------------

def make_synthetic_frame(t, width=640, height=480):
    """Generate a synthetic colour gradient frame that changes over time."""
    frame = np.zeros((height, width, 3), dtype=np.uint8)
    # Animated horizontal gradient shifting over time
    for x in range(width):
        hue = int((x / width * 180 + t * 30) % 180)
        frame[:, x] = [hue, 200, 200]
    frame = cv2.cvtColor(frame, cv2.COLOR_HSV2BGR)
    # Add a moving bright circle
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
        # macOS loopback UDP is capped at ~9 KB; fragment all images so they
        # reassemble correctly in Serial Studio's ImageFrameReader accumulator.
        self.chunk_size = 8_192

    def send(self, data: bytes):
        """Send raw bytes, fragmenting if needed."""
        if len(data) <= self.chunk_size:
            self.sock.sendto(data, self.addr)
            return
        # Fragment: send sequentially so the receiver accumulates them
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
                   help="Target frame rate (default: 24)")
    p.add_argument("--no-camera", action="store_true",
                   help="Use synthetic test pattern instead of camera")
    return p.parse_args()


def open_camera(index):
    if not HAS_CV2:
        return None
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        return None
    # Request a reasonable resolution; camera may ignore this
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, MAX_FRAME_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    cap.set(cv2.CAP_PROP_FPS, TARGET_FPS)
    return cap


def encode_jpeg(frame_bgr, quality=JPEG_QUALITY):
    """Encode a BGR frame as JPEG bytes."""
    # Resize if wider than MAX_FRAME_WIDTH
    h, w = frame_bgr.shape[:2]
    if w > MAX_FRAME_WIDTH:
        scale = MAX_FRAME_WIDTH / w
        frame_bgr = cv2.resize(frame_bgr, (MAX_FRAME_WIDTH, int(h * scale)))
    ok, buf = cv2.imencode(".jpg", frame_bgr, [cv2.IMWRITE_JPEG_QUALITY, quality])
    if not ok:
        return None
    return bytes(buf)


def main():
    args = parse_args()

    if not HAS_CV2:
        print("ERROR: opencv-python is not installed.")
        print("       Install with:  pip install opencv-python numpy")
        sys.exit(1)

    use_camera = not args.no_camera
    cap = None

    if use_camera:
        cap = open_camera(args.camera)
        if cap is None:
            print(f"WARNING: could not open camera {args.camera}, falling back to synthetic pattern.")
            use_camera = False

    sender = UDPSender(UDP_HOST, args.port)
    interval = 1.0 / args.fps

    print(f"Camera Telemetry → udp://{UDP_HOST}:{args.port}  @ {args.fps:.0f} Hz")
    print("Source:", f"camera {args.camera}" if use_camera else "synthetic pattern")
    print("Press Ctrl+C to stop.\n")
    print(f"{'Frame':>6}  {'Lum':>6}  {'Contrast':>8}  {'Sharpness':>10}  {'Hue°':>6}  {'Motion':>7}  {'JPEG kB':>8}")
    print("-" * 62)

    frame_count = 0
    prev_gray = None
    t_start = time.monotonic()

    try:
        while True:
            t0 = time.monotonic()
            t = t0 - t_start

            # --- Acquire frame ---
            if use_camera:
                ok, frame_bgr = cap.read()
                if not ok:
                    print("Camera read failed, switching to synthetic pattern.")
                    use_camera = False
                    frame_bgr = make_synthetic_frame(t)
            else:
                frame_bgr = make_synthetic_frame(t)

            # --- Compute analytics ---
            curr_gray = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)
            luminance, contrast, sharpness, hue_deg = compute_metrics(frame_bgr)
            motion = compute_motion(prev_gray, curr_gray)
            prev_gray = curr_gray
            frame_count += 1

            # --- Clamp sharpness for display (log scale) ---
            sharpness_display = min(math.log10(max(sharpness, 1.0)) * 25.0, 100.0)

            # --- Send JPEG image frame (raw bytes — autodetected by Image widget) ---
            jpeg_bytes = encode_jpeg(frame_bgr)
            if jpeg_bytes:
                sender.send(jpeg_bytes)

            # --- Send CSV telemetry frame ---
            # Format: $lum,contrast,sharpness,hue,motion,fps,frame_count;\n
            fps_actual = frame_count / max(t, 0.001)
            csv = (
                f"${luminance:.2f},{contrast:.2f},{sharpness_display:.2f},"
                f"{hue_deg:.1f},{motion:.2f},{fps_actual:.1f},{frame_count};\n"
            )
            sender.send_text(csv)

            # --- Console stats ---
            jpeg_kb = len(jpeg_bytes) / 1024.0 if jpeg_bytes else 0.0
            print(
                f"{frame_count:>6}  {luminance:>6.1f}  {contrast:>8.2f}  "
                f"{sharpness_display:>10.2f}  {hue_deg:>6.1f}  {motion:>7.2f}  {jpeg_kb:>8.1f}",
                end="\r",
            )

            # --- Timing ---
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
