#!/usr/bin/env python3
"""
Dual Drone Telemetry Simulator for Serial Studio

Simulates two drones transmitting flight telemetry and synthetic camera
imagery over separate TCP connections:

  Drone Alpha (port 9001) — Circular patrol at 120 m altitude
    CSV delimiters:   A1 01 A1 01 … A1 02 A1 02 (hex)
    Image delimiters: A1 CA FE 01 … A1 FE ED 01
    11 CSV fields + JPEG camera frames

  Drone Bravo (port 9002) — Figure-8 survey at 200 m altitude
    CSV delimiters:   B2 03 B2 03 … B2 04 B2 04 (hex)
    Image delimiters: B2 CA FE 02 … B2 FE ED 02
    11 CSV fields + JPEG camera frames

Each drone generates a unique procedural camera image every frame:
  Alpha — top-down terrain view with green/brown landscape and a moving
          crosshair reticle (simulating a patrol camera).
  Bravo — synthetic thermal/infrared style view in purple/orange tones
          with a scanning grid overlay (simulating a survey camera).

Output Controls (requires Serial Studio Pro):
  Both drones respond to commands sent back over the same TCP link:
    THR <0-100>    Set throttle percentage (affects airspeed)
    HDG <-180-180> Apply heading offset in degrees
    CAM ON|OFF     Enable/disable camera image transmission
    RTH            Trigger return-to-home (placeholder)

Run this script first (it starts two TCP servers), then open
"Dual Drone Telemetry.ssproj" in Serial Studio and connect both sources.

Usage:
  python3 dual_drone_telemetry.py
  python3 dual_drone_telemetry.py --fps 15
  python3 dual_drone_telemetry.py --host 0.0.0.0

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

# Frame delimiters (hex) — telemetry CSV frames
# Must NOT occur inside JPEG image data sent on the same UDP port.
# Using FF D0 … sequences: FF D0-D7 are JPEG restart markers (RST0-RST7)
# that only appear at specific intervals in progressive JPEGs.  Our images
# are baseline, so these never appear.  Combined with a unique second pair
# they form 4-byte sequences that are collision-free.
DELIM_ALPHA_START = b"\xa1\x01\xa1\x01"
DELIM_ALPHA_END = b"\xa1\x02\xa1\x02"
DELIM_BRAVO_START = b"\xb2\x03\xb2\x03"
DELIM_BRAVO_END = b"\xb2\x04\xb2\x04"

# Image delimiters (hex) — wrap JPEG camera frames
IMG_ALPHA_START = b"\xa1\xca\xfe\x01"
IMG_ALPHA_END = b"\xa1\xfe\xed\x01"
IMG_BRAVO_START = b"\xb2\xca\xfe\x02"
IMG_BRAVO_END = b"\xb2\xfe\xed\x02"

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


def make_camera_off_image(label):
    """Static 'camera disabled' placeholder image."""
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)
    img[:] = (30, 30, 30)

    # Draw a camera-off icon: circle with diagonal line
    cx, cy = IMG_WIDTH // 2, IMG_HEIGHT // 2
    cv2.circle(img, (cx, cy - 20), 30, (80, 80, 80), 2)
    cv2.line(img, (cx - 22, cy + 2), (cx + 22, cy - 42), (80, 80, 80), 2)

    # Label
    text = f"{label} CAMERA OFF"
    ts = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)[0]
    cv2.putText(
        img, text,
        (cx - ts[0] // 2, cy + 30),
        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (100, 100, 100), 1,
    )

    return img


# ---------------------------------------------------------------------------
# Drone flight simulators
# ---------------------------------------------------------------------------


class DroneAlpha:
    """Circular patrol flight profile at 120 m.

    The base flight path is a circle, but throttle and heading commands
    modify the actual speed and course.  Position is integrated from
    heading + airspeed each tick so controls visibly steer the drone.
    """

    # Degrees-per-metre (approximate, Nevada latitude)
    _DEG_PER_M_LAT = 1.0 / 111_320.0
    _DEG_PER_M_LON = 1.0 / (111_320.0 * math.cos(math.radians(BASE_LAT)))

    def __init__(self):
        self._t = 0.0
        self._battery_pct = 100.0
        self._alt_base = 120.0
        self._lat = BASE_LAT
        self._lon = BASE_LON
        self._heading = 90.0
        self._prev_alt = self._alt_base

    def step(self, dt, cmds=None):
        self._t += dt
        t = self._t

        # Base circular turn rate: ~9 deg/s → full circle in ~40 s
        base_turn_rate = 9.0
        base_airspeed = 15.0

        # Throttle scales airspeed (50% = normal cruise)
        throttle_frac = cmds.throttle / 50.0 if cmds else 1.0
        airspeed = base_airspeed * throttle_frac + noise(0.3)
        airspeed = clamp(airspeed, 0, 40)

        # Heading: base circular turn + user offset drives actual heading
        heading_offset = cmds.heading_offset if cmds else 0.0
        self._heading += (base_turn_rate + heading_offset * 0.15) * dt
        self._heading %= 360.0
        heading = self._heading

        # Roll proportional to turn rate (heading_offset adds bank)
        turn_rate = base_turn_rate + heading_offset * 0.15
        roll = clamp(turn_rate * 1.3 + noise(0.5), -45, 45)

        # Integrate position from heading + airspeed
        hdg_rad = math.radians(heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * self._DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * self._DEG_PER_M_LON

        # Altitude: gentle oscillation
        alt = self._alt_base + 8 * math.sin(t * 0.3) + noise(0.5)
        alt = clamp(alt, 80, 160)
        vspeed = (alt - self._prev_alt) / dt
        self._prev_alt = alt

        # Pitch: nose down in fast flight, up when slow
        pitch = clamp(-2.0 + (1.0 - throttle_frac) * 4.0 + noise(0.3), -15, 15)

        # Battery: drain scales with throttle
        drain = 0.15 * (0.5 + throttle_frac * 0.5)
        self._battery_pct = clamp(self._battery_pct - dt * drain, 0, 100)
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        current = 8.0 + airspeed * 0.8 + abs(roll) * 0.2 + noise(0.5)

        return {
            "lat": round(self._lat, 6),
            "lon": round(self._lon, 6),
            "heading": round(heading, 2),
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
    """Figure-8 survey flight profile at 200 m.

    Same integrated-position model as Alpha but with a figure-8 base
    turn pattern (alternating left/right turns).
    """

    _DEG_PER_M_LAT = 1.0 / 111_320.0
    _DEG_PER_M_LON = 1.0 / (111_320.0 * math.cos(math.radians(BASE_LAT)))

    def __init__(self):
        self._t = 0.0
        self._battery_pct = 95.0
        self._alt_base = 200.0
        self._lat = BASE_LAT + 0.008
        self._lon = BASE_LON + 0.008
        self._heading = 0.0
        self._prev_alt = self._alt_base

    def step(self, dt, cmds=None):
        self._t += dt
        t = self._t

        # Figure-8 base turn: sinusoidal turn rate → alternating circles
        base_turn_rate = 12.0 * math.sin(t * 0.105)
        base_airspeed = 22.0

        # Throttle scales airspeed
        throttle_frac = cmds.throttle / 50.0 if cmds else 1.0
        airspeed = base_airspeed * throttle_frac + noise(0.4)
        airspeed = clamp(airspeed, 0, 50)

        # Heading: base figure-8 turn + user offset
        heading_offset = cmds.heading_offset if cmds else 0.0
        self._heading += (base_turn_rate + heading_offset * 0.15) * dt
        self._heading %= 360.0
        heading = self._heading

        # Roll from turn rate
        turn_rate = base_turn_rate + heading_offset * 0.15
        roll = clamp(turn_rate * 1.6 + noise(1.0), -45, 45)

        # Integrate position
        hdg_rad = math.radians(heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * self._DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * self._DEG_PER_M_LON

        # Altitude
        alt = self._alt_base + 15 * math.sin(t * 0.2) + noise(0.8)
        alt = clamp(alt, 150, 250)
        vspeed = (alt - self._prev_alt) / dt
        self._prev_alt = alt

        # Pitch
        pitch = clamp(-3.0 + (1.0 - throttle_frac) * 5.0 + noise(0.4), -20, 20)

        # Battery
        drain = 0.18 * (0.5 + throttle_frac * 0.5)
        self._battery_pct = clamp(self._battery_pct - dt * drain, 0, 100)
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        current = 10.0 + airspeed * 0.6 + abs(roll) * 0.2 + noise(0.6)

        return {
            "lat": round(self._lat, 6),
            "lon": round(self._lon, 6),
            "heading": round(heading, 2),
            "alt": round(alt, 2),
            "airspeed": round(airspeed, 2),
            "vspeed": round(vspeed, 2),
            "roll": round(clamp(roll, -45, 45), 2),
            "pitch": round(clamp(pitch, -20, 20), 2),
            "voltage": round(clamp(voltage, 18, 25.2), 2),
            "current": round(clamp(current, 5, 35), 2),
            "battery_pct": round(self._battery_pct, 1),
        }


# ---------------------------------------------------------------------------
# UDP sender
# ---------------------------------------------------------------------------


class CommandState:
    """Mutable state driven by output-control commands from Serial Studio."""

    def __init__(self):
        self.throttle = 50.0      # 0-100 %
        self.heading_offset = 0.0 # -180 to +180 deg
        self.camera_on = True
        self.rth = False

    def parse(self, raw: bytes):
        """Parse one or more newline-delimited commands."""
        for line in raw.decode("utf-8", errors="ignore").splitlines():
            line = line.strip()
            if not line:
                continue

            parts = line.split(None, 1)
            cmd = parts[0].upper()
            arg = parts[1] if len(parts) > 1 else ""

            if cmd == "THR":
                self.throttle = clamp(float(arg), 0, 100)
            elif cmd == "HDG":
                self.heading_offset = clamp(float(arg), -180, 180)
            elif cmd == "CAM":
                self.camera_on = arg.strip().upper() == "ON"
            elif cmd == "RTH":
                self.rth = True


class TCPServer:
    """TCP server that accepts one client and provides send/recv."""

    def __init__(self, host, port, label):
        self.label = label
        self.client = None
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server.bind((host, port))
        self.server.listen(1)
        self.server.settimeout(0.5)
        print(f"[{label}] TCP server listening on {host}:{port}")

    def wait_for_client(self, stop_event):
        """Block until a client connects or stop_event is set."""
        print(f"[{self.label}] Waiting for Serial Studio to connect...")
        while not stop_event.is_set():
            try:
                self.client, addr = self.server.accept()
                self.client.setblocking(False)
                print(f"[{self.label}] Client connected from {addr[0]}:{addr[1]}")
                return True
            except socket.timeout:
                continue
        return False

    def send_raw(self, data: bytes):
        if not self.client:
            return
        try:
            self.client.sendall(data)
        except (BrokenPipeError, ConnectionResetError, OSError):
            self.client = None

    def recv(self):
        """Non-blocking receive. Returns bytes or None."""
        if not self.client:
            return None
        try:
            data = self.client.recv(4096)
            if not data:
                self.client = None
                return None
            return data
        except BlockingIOError:
            return None
        except (ConnectionResetError, OSError):
            self.client = None
            return None

    def close(self):
        if self.client:
            self.client.close()
        self.server.close()


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
    server = TCPServer(host, PORT_ALPHA, "Alpha")

    try:
        while not stop_event.is_set():
            # Wait for Serial Studio to connect
            if not server.wait_for_client(stop_event):
                break

            # Fresh state for each connection
            drone = DroneAlpha()
            cmds = CommandState()
            print(f"[Alpha] Streaming at {fps} Hz")

            while not stop_event.is_set() and server.client:
                t_start = time.monotonic()

                # Process incoming commands
                while True:
                    raw = server.recv()
                    if raw is None:
                        break
                    cmds.parse(raw)

                # Client disconnected during recv
                if not server.client:
                    break

                data = drone.step(dt, cmds)

                # Send JPEG image wrapped in image delimiters
                if HAS_CV2:
                    if cmds.camera_on:
                        img = make_alpha_image(
                            drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
                        )
                    else:
                        img = make_camera_off_image("ALPHA")

                    jpeg = encode_jpeg(img)
                    if jpeg:
                        server.send_raw(IMG_ALPHA_START + jpeg + IMG_ALPHA_END)

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
                server.send_raw(frame)

                with _display_lock:
                    _display["alpha"] = (
                        f"lat={data['lat']:.4f} alt={data['alt']:.0f}m "
                        f"hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}% "
                        f"thr={cmds.throttle:.0f}% cam={'on' if cmds.camera_on else 'off'}"
                    )

                _refresh_display()

                elapsed = time.monotonic() - t_start
                sleep_time = dt - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)

            # Client disconnected — reset display and wait for reconnect
            print(f"\n[Alpha] Client disconnected, waiting for reconnect...")
            with _display_lock:
                _display["alpha"] = "disconnected"
            _refresh_display()
    finally:
        server.close()


def run_bravo(host, fps, stop_event):
    dt = 1.0 / fps
    server = TCPServer(host, PORT_BRAVO, "Bravo")

    try:
        while not stop_event.is_set():
            # Wait for Serial Studio to connect
            if not server.wait_for_client(stop_event):
                break

            # Fresh state for each connection
            drone = DroneBravo()
            cmds = CommandState()
            print(f"[Bravo] Streaming at {fps} Hz")

            while not stop_event.is_set() and server.client:
                t_start = time.monotonic()

                # Process incoming commands
                while True:
                    raw = server.recv()
                    if raw is None:
                        break
                    cmds.parse(raw)

                # Client disconnected during recv
                if not server.client:
                    break

                data = drone.step(dt, cmds)

                # Send JPEG image wrapped in image delimiters
                if HAS_CV2:
                    if cmds.camera_on:
                        img = make_bravo_image(
                            drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
                        )
                    else:
                        img = make_camera_off_image("BRAVO")

                    jpeg = encode_jpeg(img)
                    if jpeg:
                        server.send_raw(IMG_BRAVO_START + jpeg + IMG_BRAVO_END)

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
                server.send_raw(frame)

                with _display_lock:
                    _display["bravo"] = (
                        f"lat={data['lat']:.4f} alt={data['alt']:.0f}m "
                        f"hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}% "
                        f"thr={cmds.throttle:.0f}% cam={'on' if cmds.camera_on else 'off'}"
                    )

                _refresh_display()

                elapsed = time.monotonic() - t_start
                sleep_time = dt - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)

            # Client disconnected — reset display and wait for reconnect
            print(f"\n[Bravo] Client disconnected, waiting for reconnect...")
            with _display_lock:
                _display["bravo"] = "disconnected"
            _refresh_display()
    finally:
        server.close()


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
        help=f"TCP listen address (default: {DEFAULT_HOST})",
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
