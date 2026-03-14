#!/usr/bin/env python3
"""
Dual Drone Telemetry Simulator for Serial Studio

Simulates two drones transmitting flight telemetry and synthetic camera
imagery over separate UDP connections:

  Drone Alpha (port 9001) — Circular patrol at 120 m altitude
    Hex delimiters: AB CD EF … FE ED
    11 CSV fields + JPEG camera frames

  Drone Bravo (port 9002) — Figure-8 survey at 200 m altitude
    Hex delimiters: DE AD … FE ED
    11 CSV fields + JPEG camera frames

Each drone generates a unique procedural camera image every frame:
  Alpha — top-down terrain view with green/brown landscape and a moving
          crosshair reticle (simulating a patrol camera).
  Bravo — synthetic thermal/infrared style view in purple/orange tones
          with a scanning grid overlay (simulating a survey camera).

Open "Dual Drone Telemetry.ssproj" in Serial Studio and connect both
UDP sources before running this script.

Usage:
  python3 dual_drone_telemetry.py
  python3 dual_drone_telemetry.py --fps 15
  python3 dual_drone_telemetry.py --host 192.168.1.100

Dependencies:
  pip install opencv-python numpy
"""

import argparse
import math
import random
import socket
import sys
import threading
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

DEFAULT_HOST = "127.0.0.1"
DEFAULT_FPS = 10

PORT_ALPHA = 9001
PORT_BRAVO = 9002

IMG_WIDTH = 320
IMG_HEIGHT = 240
JPEG_QUALITY = 70

# Frame delimiters (hex)
DELIM_ALPHA_START = b"\xab\xcd\xef"
DELIM_ALPHA_END = b"\xfe\xed"
DELIM_BRAVO_START = b"\xde\xad"
DELIM_BRAVO_END = b"\xfe\xed"

# Base GPS coordinates (somewhere over Nevada desert)
BASE_LAT = 36.236
BASE_LON = -115.805

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def noise(amp=0.1):
    return random.gauss(0, amp)


# ---------------------------------------------------------------------------
# Synthetic image generators
# ---------------------------------------------------------------------------


def make_alpha_image(t, lat, lon, heading, alt):
    """
    Generate a top-down terrain camera view for Drone Alpha.
    Green/brown procedural landscape with a targeting reticle.
    """
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)

    # Procedural terrain — shifting noise based on GPS position
    for y in range(0, IMG_HEIGHT, 4):
        for x in range(0, IMG_WIDTH, 4):
            # Use position offsets to create terrain parallax
            fx = (x / IMG_WIDTH + lon * 100 + t * 0.02) * 8.0
            fy = (y / IMG_HEIGHT + lat * 100 + t * 0.015) * 8.0
            val = (math.sin(fx) * math.cos(fy) + 1.0) * 0.5
            val += (math.sin(fx * 3.7 + 1.3) * math.cos(fy * 2.9 + 0.7) + 1.0) * 0.15

            # Green/brown palette
            g = int(clamp(val * 140 + 40, 30, 180))
            r = int(clamp(val * 80 + 30, 20, 120))
            b = int(clamp(val * 30 + 10, 5, 50))
            img[y : y + 4, x : x + 4] = (b, g, r)

    # Roads — thin lines based on grid
    road_x = int((math.sin(lat * 500) * 0.5 + 0.5) * IMG_WIDTH)
    road_y = int((math.cos(lon * 500) * 0.5 + 0.5) * IMG_HEIGHT)
    cv2.line(img, (road_x, 0), (road_x, IMG_HEIGHT), (60, 60, 70), 2)
    cv2.line(img, (0, road_y), (IMG_WIDTH, road_y), (60, 60, 70), 2)

    # Targeting reticle — moves with heading
    cx, cy = IMG_WIDTH // 2, IMG_HEIGHT // 2
    r_size = 30 + int(10 * math.sin(t * 2))
    cv2.circle(img, (cx, cy), r_size, (0, 255, 0), 1)
    cv2.line(img, (cx - r_size - 10, cy), (cx - r_size + 5, cy), (0, 255, 0), 1)
    cv2.line(img, (cx + r_size - 5, cy), (cx + r_size + 10, cy), (0, 255, 0), 1)
    cv2.line(img, (cx, cy - r_size - 10), (cx, cy - r_size + 5), (0, 255, 0), 1)
    cv2.line(img, (cx, cy + r_size - 5), (cx, cy + r_size + 10), (0, 255, 0), 1)
    cv2.circle(img, (cx, cy), 3, (0, 255, 0), -1)

    # HUD overlay text
    cv2.putText(img, f"ALT {alt:.0f}m", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1)
    cv2.putText(
        img, f"HDG {heading:.0f}", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1
    )
    cv2.putText(
        img,
        f"{lat:.4f} {lon:.4f}",
        (10, IMG_HEIGHT - 15),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.35,
        (0, 255, 0),
        1,
    )

    # Heading indicator line from center
    hx = int(cx + 25 * math.sin(math.radians(heading)))
    hy = int(cy - 25 * math.cos(math.radians(heading)))
    cv2.arrowedLine(img, (cx, cy), (hx, hy), (0, 255, 0), 1, tipLength=0.3)

    return img


def make_bravo_image(t, lat, lon, heading, alt):
    """
    Generate a synthetic thermal/IR camera view for Drone Bravo.
    Purple-orange heat map with a scanning grid overlay.
    """
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)

    # Thermal noise field — different frequency from Alpha
    for y in range(0, IMG_HEIGHT, 4):
        for x in range(0, IMG_WIDTH, 4):
            fx = (x / IMG_WIDTH + lon * 80 + t * 0.03) * 6.0
            fy = (y / IMG_HEIGHT + lat * 80 + t * 0.025) * 6.0
            val = (math.sin(fx * 1.3 + fy * 0.7) + 1.0) * 0.3
            val += (math.cos(fx * 2.1 - fy * 1.5 + t * 0.5) + 1.0) * 0.2
            val += (math.sin((fx + fy) * 0.9 + t * 0.8) + 1.0) * 0.15

            # Hot spots — simulate thermal targets
            dx = (x / IMG_WIDTH - 0.5) * 2
            dy = (y / IMG_HEIGHT - 0.5) * 2
            for sx, sy in [
                (math.sin(t * 0.4) * 0.6, math.cos(t * 0.3) * 0.4),
                (math.cos(t * 0.5 + 2) * 0.5, math.sin(t * 0.6 + 1) * 0.5),
            ]:
                dist = math.sqrt((dx - sx) ** 2 + (dy - sy) ** 2)
                val += max(0, 0.4 - dist) * 1.5

            val = clamp(val, 0, 1)

            # Iron-bow thermal palette (black → purple → orange → yellow)
            if val < 0.33:
                t2 = val / 0.33
                r = int(t2 * 120)
                g = 0
                b = int(t2 * 180)
            elif val < 0.66:
                t2 = (val - 0.33) / 0.33
                r = int(120 + t2 * 135)
                g = int(t2 * 80)
                b = int(180 - t2 * 180)
            else:
                t2 = (val - 0.66) / 0.34
                r = 255
                g = int(80 + t2 * 175)
                b = 0

            img[y : y + 4, x : x + 4] = (b, g, r)

    # Scanning grid overlay
    scan_phase = int(t * 40) % IMG_HEIGHT
    cv2.line(img, (0, scan_phase), (IMG_WIDTH, scan_phase), (255, 255, 255), 1)

    grid_spacing = 40
    for gx in range(0, IMG_WIDTH, grid_spacing):
        cv2.line(img, (gx, 0), (gx, IMG_HEIGHT), (100, 100, 100), 1)
    for gy in range(0, IMG_HEIGHT, grid_spacing):
        cv2.line(img, (0, gy), (IMG_WIDTH, gy), (100, 100, 100), 1)

    # HUD overlay (cyan for IR theme)
    cv2.putText(img, f"FLIR", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 200, 0), 1)
    cv2.putText(
        img, f"ALT {alt:.0f}m", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 200, 0), 1
    )
    cv2.putText(
        img,
        f"{lat:.4f} {lon:.4f}",
        (10, IMG_HEIGHT - 15),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.35,
        (255, 200, 0),
        1,
    )

    # Crosshair
    cx, cy = IMG_WIDTH // 2, IMG_HEIGHT // 2
    cv2.line(img, (cx - 20, cy), (cx + 20, cy), (255, 200, 0), 1)
    cv2.line(img, (cx, cy - 20), (cx, cy + 20), (255, 200, 0), 1)

    return img


def make_fallback_image(label, t):
    """Simple colored rectangle when opencv is not available — never sent."""
    return None


# ---------------------------------------------------------------------------
# Drone flight simulators
# ---------------------------------------------------------------------------


class DroneAlpha:
    """Circular patrol flight profile at 120 m."""

    def __init__(self):
        self._t = 0.0
        self._battery_pct = 100.0
        self._radius = 0.003  # ~330 m GPS radius
        self._alt_base = 120.0

    def step(self, dt):
        self._t += dt
        t = self._t

        # Circular patrol — completes one orbit every ~40 seconds
        angle = t * 0.157  # ~2*pi/40
        lat = BASE_LAT + self._radius * math.cos(angle)
        lon = BASE_LON + self._radius * math.sin(angle)

        # Heading follows the tangent of the circle
        heading = (math.degrees(angle) + 90) % 360

        # Altitude: gentle oscillation
        alt = self._alt_base + 8 * math.sin(t * 0.3) + noise(0.5)
        alt = clamp(alt, 80, 160)

        # Airspeed: mostly constant cruise with turbulence
        airspeed = 15.0 + 2.0 * math.sin(t * 0.5) + noise(0.3)
        airspeed = clamp(airspeed, 10, 25)

        # Vertical speed from altitude derivative
        vspeed = 8 * 0.3 * math.cos(t * 0.3) + noise(0.1)

        # Roll: banked turn (constant mild bank for circular flight)
        roll = 12.0 + 3.0 * math.sin(t * 0.8) + noise(0.5)

        # Pitch: nose slightly down in cruise
        pitch = -2.0 + 1.5 * math.sin(t * 0.4) + noise(0.3)

        # Battery: slow linear drain ~0.15%/sec
        self._battery_pct = clamp(self._battery_pct - dt * 0.15, 0, 100)
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        current = 12.0 + abs(roll) * 0.3 + noise(0.5)

        return {
            "lat": round(lat, 6),
            "lon": round(lon, 6),
            "heading": round(heading % 360, 2),
            "alt": round(alt, 2),
            "airspeed": round(airspeed, 2),
            "vspeed": round(vspeed, 2),
            "roll": round(roll, 2),
            "pitch": round(pitch, 2),
            "voltage": round(clamp(voltage, 18, 25.2), 2),
            "current": round(clamp(current, 5, 35), 2),
            "battery_pct": round(self._battery_pct, 1),
        }


class DroneBravo:
    """Figure-8 survey flight profile at 200 m."""

    def __init__(self):
        self._t = 0.0
        self._battery_pct = 95.0  # starts slightly used
        self._alt_base = 200.0

    def step(self, dt):
        self._t += dt
        t = self._t

        # Figure-8 (lemniscate of Bernoulli) — completes in ~60 seconds
        omega = t * 0.105  # ~2*pi/60
        scale = 0.005  # ~550 m GPS radius
        lat = BASE_LAT + 0.008 + scale * math.sin(omega)
        lon = BASE_LON + 0.008 + scale * math.sin(omega) * math.cos(omega)

        # Heading from velocity direction
        dlat = scale * math.cos(omega) * 0.105
        dlon = scale * (math.cos(omega) * math.cos(omega) - math.sin(omega) * math.sin(omega)) * 0.105
        heading = math.degrees(math.atan2(dlon, dlat)) % 360

        # Altitude: higher base, periodic variation
        alt = self._alt_base + 15 * math.sin(t * 0.2) + noise(0.8)
        alt = clamp(alt, 150, 250)

        # Airspeed: faster survey speed
        airspeed = 22.0 + 4.0 * math.sin(t * 0.35) + noise(0.4)
        airspeed = clamp(airspeed, 15, 35)

        # Vertical speed
        vspeed = 15 * 0.2 * math.cos(t * 0.2) + noise(0.15)

        # Roll: aggressive banking for figure-8 turns
        roll = 20.0 * math.sin(omega) + noise(1.0)
        roll = clamp(roll, -40, 40)

        # Pitch
        pitch = -3.0 + 2.0 * math.sin(t * 0.6) + noise(0.4)

        # Battery: slightly faster drain due to higher speed
        self._battery_pct = clamp(self._battery_pct - dt * 0.18, 0, 100)
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        current = 16.0 + abs(roll) * 0.2 + noise(0.6)

        return {
            "lat": round(lat, 6),
            "lon": round(lon, 6),
            "heading": round(heading % 360, 2),
            "alt": round(alt, 2),
            "airspeed": round(airspeed, 2),
            "vspeed": round(vspeed, 2),
            "roll": round(clamp(roll, -40, 40), 2),
            "pitch": round(clamp(pitch, -20, 20), 2),
            "voltage": round(clamp(voltage, 18, 25.2), 2),
            "current": round(clamp(current, 5, 35), 2),
            "battery_pct": round(self._battery_pct, 1),
        }


# ---------------------------------------------------------------------------
# UDP sender
# ---------------------------------------------------------------------------


class UDPSender:
    def __init__(self, host, port):
        self.addr = (host, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.chunk_size = 8192

    def send_raw(self, data: bytes):
        for offset in range(0, len(data), self.chunk_size):
            self.sock.sendto(data[offset : offset + self.chunk_size], self.addr)

    def close(self):
        self.sock.close()


# ---------------------------------------------------------------------------
# Station threads
# ---------------------------------------------------------------------------

_display_lock = threading.Lock()
_display = {"alpha": "", "bravo": ""}


def _refresh_display():
    with _display_lock:
        line = f"  A: {_display['alpha']}   B: {_display['bravo']}"
    sys.stdout.write(f"\r{line:<140}")
    sys.stdout.flush()


def encode_jpeg(frame_bgr):
    ok, buf = cv2.imencode(".jpg", frame_bgr, [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY])
    if not ok:
        return None
    return bytes(buf)


def run_alpha(host, fps, stop_event):
    dt = 1.0 / fps
    sender = UDPSender(host, PORT_ALPHA)
    drone = DroneAlpha()

    print(f"[Alpha] Sending to {host}:{PORT_ALPHA} at {fps} Hz")

    try:
        while not stop_event.is_set():
            t_start = time.monotonic()

            data = drone.step(dt)

            # Send JPEG image if opencv available
            if HAS_CV2:
                img = make_alpha_image(
                    drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
                )
                jpeg = encode_jpeg(img)
                if jpeg:
                    sender.send_raw(jpeg)

            # Send CSV telemetry with hex delimiters
            csv = ",".join(
                str(data[k])
                for k in [
                    "lat", "lon", "heading", "alt", "airspeed",
                    "vspeed", "roll", "pitch", "voltage", "current",
                    "battery_pct",
                ]
            )
            frame = DELIM_ALPHA_START + csv.encode() + DELIM_ALPHA_END
            sender.send_raw(frame)

            with _display_lock:
                _display["alpha"] = f"lat={data['lat']:.4f} alt={data['alt']:.0f}m hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}%"

            _refresh_display()

            elapsed = time.monotonic() - t_start
            sleep_time = dt - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
    finally:
        sender.close()


def run_bravo(host, fps, stop_event):
    dt = 1.0 / fps
    sender = UDPSender(host, PORT_BRAVO)
    drone = DroneBravo()

    print(f"[Bravo] Sending to {host}:{PORT_BRAVO} at {fps} Hz")

    try:
        while not stop_event.is_set():
            t_start = time.monotonic()

            data = drone.step(dt)

            # Send JPEG image if opencv available
            if HAS_CV2:
                img = make_bravo_image(
                    drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
                )
                jpeg = encode_jpeg(img)
                if jpeg:
                    sender.send_raw(jpeg)

            # Send CSV telemetry with hex delimiters
            csv = ",".join(
                str(data[k])
                for k in [
                    "lat", "lon", "heading", "alt", "airspeed",
                    "vspeed", "roll", "pitch", "voltage", "current",
                    "battery_pct",
                ]
            )
            frame = DELIM_BRAVO_START + csv.encode() + DELIM_BRAVO_END
            sender.send_raw(frame)

            with _display_lock:
                _display["bravo"] = f"lat={data['lat']:.4f} alt={data['alt']:.0f}m hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}%"

            _refresh_display()

            elapsed = time.monotonic() - t_start
            sleep_time = dt - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
    finally:
        sender.close()


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(
        description="Dual Drone Telemetry Simulator for Serial Studio",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--fps",
        type=float,
        default=DEFAULT_FPS,
        metavar="FPS",
        help=f"Frame rate in Hz (default: {DEFAULT_FPS})",
    )
    parser.add_argument(
        "--host",
        default=DEFAULT_HOST,
        metavar="HOST",
        help=f"UDP destination host (default: {DEFAULT_HOST})",
    )
    args = parser.parse_args()

    if args.fps <= 0:
        print("Error: --fps must be a positive number.", file=sys.stderr)
        sys.exit(1)

    print("Dual Drone Telemetry Simulator")
    print(f"FPS: {args.fps}  |  Host: {args.host}")
    if HAS_CV2:
        print(f"Camera: {IMG_WIDTH}x{IMG_HEIGHT} synthetic JPEG @ quality {JPEG_QUALITY}")
    else:
        print("WARNING: opencv-python not installed — no camera images will be sent")
        print("         Install with: pip install opencv-python numpy")
    print("Press Ctrl+C to stop.\n")

    stop_event = threading.Event()

    t_alpha = threading.Thread(
        target=run_alpha,
        args=(args.host, args.fps, stop_event),
        daemon=True,
        name="alpha",
    )
    t_bravo = threading.Thread(
        target=run_bravo,
        args=(args.host, args.fps, stop_event),
        daemon=True,
        name="bravo",
    )

    t_alpha.start()
    t_bravo.start()

    try:
        while t_alpha.is_alive() and t_bravo.is_alive():
            time.sleep(0.1)
    except KeyboardInterrupt:
        stop_event.set()
        print("\nStopping simulation...")

    t_alpha.join(timeout=2.0)
    t_bravo.join(timeout=2.0)
    print("Done.")


if __name__ == "__main__":
    main()
