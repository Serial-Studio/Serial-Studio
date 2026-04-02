#!/usr/bin/env python3
"""
Dual Drone Telemetry Simulator for Serial Studio

Simulates two drones transmitting flight telemetry and real satellite
camera imagery over separate TCP connections from the Swiss Alps
(Zermatt area):

  Drone Alpha (port 9001) — Circular patrol at 120 m AGL
    CSV delimiters:   A1 01 A1 01 … A1 02 A1 02 (hex)
    Image delimiters: A1 CA FE 01 … A1 FE ED 01
    11 CSV fields + JPEG camera frames (satellite view + HUD)

  Drone Bravo (port 9002) — Figure-8 survey at 200 m AGL
    CSV delimiters:   B2 03 B2 03 … B2 04 B2 04 (hex)
    Image delimiters: B2 CA FE 02 … B2 FE ED 02
    11 CSV fields + JPEG camera frames (thermal/IR remap of satellite)

Both drones start grounded at their helipad near Zermatt. Use the TKO
command to launch, and RTH to bring them back. On landing the battery is
recharged and the drone is ready for another flight.

Camera images are fetched from ArcGIS World Imagery satellite tiles
(free, no API key required) and cached locally. Each drone's view is
centered on its GPS position and rotated to match heading:
  Alpha — photorealistic satellite view with green HUD overlay.
  Bravo — same satellite data remapped to iron-bow thermal palette.

Output Controls (requires Serial Studio Pro):
  Both drones respond to commands sent back over the same TCP link:
    TKO            Launch from helipad (only when grounded)
    THR <0-100>    Set throttle percentage (affects airspeed)
    HDG <-180-180> Apply heading offset in degrees
    CAM ON|OFF     Enable/disable camera image transmission
    RTH            Return to helipad, land, and recharge

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
import io
import math
import os
import random
import socket
import sys
import threading
import time
import urllib.request

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
JPEG_QUALITY = 60

# Frame delimiters (hex) — telemetry CSV frames
DELIM_ALPHA_START = b"\xa1\x01\xa1\x01"
DELIM_ALPHA_END = b"\xa1\x02\xa1\x02"
DELIM_BRAVO_START = b"\xb2\x03\xb2\x03"
DELIM_BRAVO_END = b"\xb2\x04\xb2\x04"

# Image delimiters (hex) — wrap JPEG camera frames
IMG_ALPHA_START = b"\xa1\xca\xfe\x01"
IMG_ALPHA_END = b"\xa1\xfe\xed\x01"
IMG_BRAVO_START = b"\xb2\xca\xfe\x02"
IMG_BRAVO_END = b"\xb2\xfe\xed\x02"

# Swiss Alps: Zermatt area — two helipads near the Matterhorn
# Alpha helipad: Zermatt village (north pad)
ALPHA_HOME_LAT = 46.0207
ALPHA_HOME_LON = 7.7491
# Bravo helipad: Trockener Steg plateau (south pad, higher elevation)
BRAVO_HOME_LAT = 46.0035
BRAVO_HOME_LON = 7.7465

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def noise(amp=0.1):
    return random.gauss(0, amp)


# Degrees-per-metre (approximate, Zermatt latitude ~46°N)
_DEG_PER_M_LAT = 1.0 / 111_320.0
_DEG_PER_M_LON = 1.0 / (111_320.0 * math.cos(math.radians(46.01)))


def bearing_and_distance(lat1, lon1, lat2, lon2):
    """Return (bearing_deg, distance_m) from (lat1,lon1) to (lat2,lon2)."""
    dlat = lat2 - lat1
    dlon = lon2 - lon1
    dist_m = math.sqrt((dlat / _DEG_PER_M_LAT) ** 2 + (dlon / _DEG_PER_M_LON) ** 2)
    bearing = math.degrees(math.atan2(dlon, dlat)) % 360.0
    return bearing, dist_m


def steer_toward(current_hdg, target_hdg, max_rate, dt):
    """Turn current_hdg toward target_hdg at max_rate deg/s. Return new heading."""
    diff = (target_hdg - current_hdg + 540) % 360 - 180
    turn = clamp(diff * 2.0, -max_rate, max_rate)
    return (current_hdg + turn * dt) % 360.0, turn


# ---------------------------------------------------------------------------
# Satellite tile fetcher & image generators
# ---------------------------------------------------------------------------

# ArcGIS World Imagery — free satellite tiles, no API key required
_TILE_URL = (
    "https://server.arcgisonline.com/ArcGIS/rest/services/"
    "World_Imagery/MapServer/tile/{z}/{y}/{x}"
)
_TILE_SIZE = 256
_TILE_CACHE = {}
_TILE_CACHE_LOCK = threading.Lock()
_TILE_CACHE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".tile_cache")

# Map altitude to zoom level — higher altitude = wider view = lower zoom
_ALT_ZOOM_TABLE = [
    (0, 18), (50, 17), (100, 17), (150, 16),
    (200, 16), (300, 15), (500, 15), (1000, 14),
]

# Per-drone patch cache: skip re-rendering when position barely changed
_PATCH_CACHE = {}
_PATCH_CACHE_LOCK = threading.Lock()


def _alt_to_zoom(alt):
    """Convert altitude (m) to an appropriate tile zoom level."""
    for threshold, zoom in _ALT_ZOOM_TABLE:
        if alt <= threshold:
            return zoom
    return 14


def _lat_lon_to_tile(lat, lon, zoom):
    """Convert lat/lon to fractional tile coordinates at given zoom."""
    n = 2 ** zoom
    x = (lon + 180.0) / 360.0 * n
    lat_rad = math.radians(lat)
    y = (1.0 - math.log(math.tan(lat_rad) + 1.0 / math.cos(lat_rad)) / math.pi) / 2.0 * n
    return x, y


def _fetch_tile(tx, ty, zoom):
    """Fetch a single 256x256 grayscale tile, using memory + disk cache."""
    key = (zoom, tx, ty)

    with _TILE_CACHE_LOCK:
        if key in _TILE_CACHE:
            return _TILE_CACHE[key]

    # Check disk cache
    cache_path = os.path.join(_TILE_CACHE_DIR, f"{zoom}_{tx}_{ty}.jpg")
    if os.path.exists(cache_path):
        tile_img = cv2.imread(cache_path, cv2.IMREAD_GRAYSCALE)
        if tile_img is not None:
            with _TILE_CACHE_LOCK:
                _TILE_CACHE[key] = tile_img
            return tile_img

    # Fetch from network
    url = _TILE_URL.format(z=zoom, x=tx, y=ty)
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "SerialStudio-DroneDemo/1.0"})
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()

        arr = np.frombuffer(data, dtype=np.uint8)
        tile_img = cv2.imdecode(arr, cv2.IMREAD_GRAYSCALE)

        if tile_img is not None:
            # Save to disk cache
            os.makedirs(_TILE_CACHE_DIR, exist_ok=True)
            cv2.imwrite(cache_path, tile_img)

            with _TILE_CACHE_LOCK:
                _TILE_CACHE[key] = tile_img
            return tile_img
    except Exception:
        pass

    # Fallback: dark grey tile
    fallback = np.full((_TILE_SIZE, _TILE_SIZE), 40, dtype=np.uint8)
    with _TILE_CACHE_LOCK:
        _TILE_CACHE[key] = fallback
    return fallback


def _get_satellite_patch(lat, lon, heading, alt, width, height, cache_key=None):
    """
    Build a grayscale satellite image patch centered on (lat, lon), rotated
    by heading. Uses a per-drone cache to skip re-rendering when position
    has barely changed (< ~1 pixel of movement).
    """
    zoom = _alt_to_zoom(alt)
    fx, fy = _lat_lon_to_tile(lat, lon, zoom)

    # Quantize inputs to skip redundant renders — ~1 pixel threshold
    qfx = round(fx * _TILE_SIZE)
    qfy = round(fy * _TILE_SIZE)
    qhdg = round(heading)

    if cache_key is not None:
        snap = (zoom, qfx, qfy, qhdg)
        with _PATCH_CACHE_LOCK:
            prev = _PATCH_CACHE.get(cache_key)
            if prev is not None and prev[0] == snap:
                return prev[1].copy()

    # Only need a 3x3 tile grid for 320x240 output + rotation margin
    half = 2
    cx_tile = int(fx)
    cy_tile = int(fy)

    # Build mosaic (grayscale — single channel)
    span = 2 * half + 1
    mosaic = np.zeros((span * _TILE_SIZE, span * _TILE_SIZE), dtype=np.uint8)

    for dy in range(-half, half + 1):
        for dx in range(-half, half + 1):
            tile = _fetch_tile(cx_tile + dx, cy_tile + dy, zoom)
            px = (dx + half) * _TILE_SIZE
            py = (dy + half) * _TILE_SIZE
            if tile.shape[0] == _TILE_SIZE and tile.shape[1] == _TILE_SIZE:
                mosaic[py : py + _TILE_SIZE, px : px + _TILE_SIZE] = tile

    # Sub-tile pixel offset for exact centering
    off_x = (fx - cx_tile + half) * _TILE_SIZE
    off_y = (fy - cy_tile + half) * _TILE_SIZE

    # Rotate around the drone's position
    rot_mat = cv2.getRotationMatrix2D((off_x, off_y), heading, 1.0)
    rot_mat[0, 2] += width / 2 - off_x
    rot_mat[1, 2] += height / 2 - off_y
    result = cv2.warpAffine(mosaic, rot_mat, (width, height), borderValue=40)

    # Cache the result
    if cache_key is not None:
        snap = (zoom, qfx, qfy, qhdg)
        with _PATCH_CACHE_LOCK:
            _PATCH_CACHE[cache_key] = (snap, result.copy())

    return result


def _draw_hud_alpha(img, lat, lon, heading, alt):
    """Draw green military-style HUD overlay on Alpha's grayscale view."""
    h, w = img.shape[:2]

    # Grid overlay
    for gx in range(0, w, 40):
        cv2.line(img, (gx, 0), (gx, h), (0, 180, 0), 1)
    for gy in range(0, h, 40):
        cv2.line(img, (0, gy), (w, gy), (0, 180, 0), 1)

    # Targeting reticle
    cx, cy = w // 2, h // 2
    cv2.circle(img, (cx, cy), 30, (0, 255, 0), 1)
    cv2.circle(img, (cx, cy), 10, (0, 255, 0), 1)
    cv2.line(img, (cx - 40, cy), (cx - 20, cy), (0, 255, 0), 1)
    cv2.line(img, (cx + 20, cy), (cx + 40, cy), (0, 255, 0), 1)
    cv2.line(img, (cx, cy - 40), (cx, cy - 20), (0, 255, 0), 1)
    cv2.line(img, (cx, cy + 20), (cx, cy + 40), (0, 255, 0), 1)

    # Text info
    cv2.putText(img, f"ALT {alt:.0f}m", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1)
    cv2.putText(
        img, f"HDG {heading:.0f}", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 0), 1
    )
    cv2.putText(
        img,
        f"{lat:.4f} {lon:.4f}",
        (10, h - 15),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.35,
        (0, 255, 0),
        1,
    )


def _apply_thermal_palette(grey):
    """Convert a grayscale satellite image to iron-bow thermal palette."""
    # Invert so snow (bright) becomes cold (dark) and valleys become warm
    inv = 255 - grey

    # Apply COLORMAP_INFERNO for a realistic thermal/FLIR look
    return cv2.applyColorMap(inv, cv2.COLORMAP_INFERNO)


def _draw_hud_bravo(img, lat, lon, heading, alt):
    """Draw amber FLIR-style HUD overlay on Bravo's thermal view."""
    h, w = img.shape[:2]

    # Grid overlay
    for gx in range(0, w, 40):
        cv2.line(img, (gx, 0), (gx, h), (100, 100, 100), 1)
    for gy in range(0, h, 40):
        cv2.line(img, (0, gy), (w, gy), (100, 100, 100), 1)

    # Text info
    cv2.putText(img, "FLIR", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 200, 0), 1)
    cv2.putText(
        img, f"ALT {alt:.0f}m", (10, 40), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 200, 0), 1
    )
    cv2.putText(
        img,
        f"{lat:.4f} {lon:.4f}",
        (10, h - 15),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.35,
        (255, 200, 0),
        1,
    )


def make_alpha_image(lat, lon, heading, alt):
    """Grayscale satellite camera view for Drone Alpha with green HUD."""
    grey = _get_satellite_patch(lat, lon, heading, alt, IMG_WIDTH, IMG_HEIGHT, cache_key="alpha")
    img = cv2.cvtColor(grey, cv2.COLOR_GRAY2BGR)
    _draw_hud_alpha(img, lat, lon, heading, alt)
    return img


def make_bravo_image(lat, lon, heading, alt):
    """Thermal/IR remap of satellite imagery for Drone Bravo."""
    grey = _get_satellite_patch(lat, lon, heading, alt, IMG_WIDTH, IMG_HEIGHT, cache_key="bravo")
    img = _apply_thermal_palette(grey)
    _draw_hud_bravo(img, lat, lon, heading, alt)
    return img


def make_camera_off_image(label):
    """Static 'camera disabled' placeholder image."""
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)
    img[:] = (30, 30, 30)

    cx, cy = IMG_WIDTH // 2, IMG_HEIGHT // 2
    cv2.circle(img, (cx, cy - 20), 30, (80, 80, 80), 2)
    cv2.line(img, (cx - 22, cy + 2), (cx + 22, cy - 42), (80, 80, 80), 2)

    text = f"{label} CAMERA OFF"
    ts = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 1)[0]
    cv2.putText(
        img, text,
        (cx - ts[0] // 2, cy + 30),
        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (100, 100, 100), 1,
    )

    return img


# ---------------------------------------------------------------------------
# Flight state machine
# ---------------------------------------------------------------------------

GROUNDED = "grounded"
TAKEOFF = "takeoff"
CRUISE = "cruise"
RETURNING = "returning"
LANDING = "landing"


# ---------------------------------------------------------------------------
# Drone flight simulators
# ---------------------------------------------------------------------------


class DroneAlpha:
    """Circular patrol flight profile at 120 m AGL.

    Starts grounded at its helipad near Zermatt village. Use TKO to launch,
    RTH to return. On landing, battery recharges and drone is ready
    for another flight.
    """

    def __init__(self):
        self._t = 0.0
        self._state = GROUNDED
        self._battery_pct = 100.0
        self._cruise_alt = 120.0
        self._lat = ALPHA_HOME_LAT
        self._lon = ALPHA_HOME_LON
        self._heading = 90.0
        self._alt = 0.0
        self._prev_alt = 0.0

    def _reset(self):
        """Reset to grounded state with full battery at helipad."""
        self._state = GROUNDED
        self._battery_pct = 100.0
        self._lat = ALPHA_HOME_LAT
        self._lon = ALPHA_HOME_LON
        self._alt = 0.0
        self._prev_alt = 0.0

    def step(self, dt, cmds=None):
        self._t += dt

        # State transitions from commands
        if cmds and cmds.tko and self._state == GROUNDED:
            self._state = TAKEOFF

        if cmds and cmds.rth and self._state == CRUISE:
            self._state = RETURNING

        if self._state == GROUNDED:
            return self._step_grounded(dt)
        elif self._state == TAKEOFF:
            return self._step_takeoff(dt, cmds)
        elif self._state == CRUISE:
            return self._step_cruise(dt, cmds)
        elif self._state == RETURNING:
            return self._step_returning(dt)
        elif self._state == LANDING:
            return self._step_landing(dt)

    def _make_telemetry(self, airspeed, roll, pitch, current):
        vspeed = (self._alt - self._prev_alt) / 0.1 if self._prev_alt is not None else 0.0
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        return {
            "lat": round(self._lat, 6),
            "lon": round(self._lon, 6),
            "heading": round(self._heading % 360, 2),
            "alt": round(max(0, self._alt), 2),
            "airspeed": round(clamp(airspeed, 0, 40), 2),
            "vspeed": round(vspeed, 2),
            "roll": round(roll, 2),
            "pitch": round(pitch, 2),
            "voltage": round(clamp(voltage, 18, 25.2), 2),
            "current": round(clamp(current, 0, 35), 2),
            "battery_pct": round(self._battery_pct, 1),
        }

    def _step_grounded(self, dt):
        """Sitting on the helipad, motors off."""
        self._prev_alt = self._alt
        self._alt = 0.0
        return self._make_telemetry(0, 0, 0, 0.5)

    def _step_takeoff(self, dt, cmds):
        """Rocket climb to cruise altitude — intentionally fast for demo visibility."""
        self._prev_alt = self._alt
        climb_rate = 60.0
        self._alt += climb_rate * dt + noise(0.5)
        self._alt = max(0, self._alt)

        # Heavy battery drain during aggressive climb
        self._battery_pct = clamp(self._battery_pct - dt * 1.5, 0, 100)

        # Reached cruise altitude
        if self._alt >= self._cruise_alt:
            self._alt = self._cruise_alt
            self._state = CRUISE

        pitch = clamp(30.0 + noise(1.0), 10, 30)
        current = 35.0 + noise(1.5)
        return self._make_telemetry(8.0 + noise(0.5), noise(3.0), pitch, current)

    def _step_cruise(self, dt, cmds):
        """Circular patrol pattern."""
        t = self._t
        self._prev_alt = self._alt

        base_turn_rate = 9.0
        base_airspeed = 15.0

        throttle_frac = cmds.throttle / 50.0 if cmds else 1.0
        airspeed = base_airspeed * throttle_frac + noise(0.3)
        airspeed = clamp(airspeed, 0, 40)

        heading_offset = cmds.heading_offset if cmds else 0.0
        self._heading += (base_turn_rate + heading_offset * 0.15) * dt
        self._heading %= 360.0

        turn_rate = base_turn_rate + heading_offset * 0.15
        roll = clamp(turn_rate * 1.3 + noise(0.5), -45, 45)

        hdg_rad = math.radians(self._heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * _DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * _DEG_PER_M_LON

        self._alt = self._cruise_alt + 8 * math.sin(t * 0.3) + noise(0.5)
        self._alt = clamp(self._alt, 80, 160)

        pitch = clamp(-2.0 + (1.0 - throttle_frac) * 4.0 + noise(0.3), -15, 15)

        drain = 0.15 * (0.5 + throttle_frac * 0.5)
        self._battery_pct = clamp(self._battery_pct - dt * drain, 0, 100)
        current = 8.0 + airspeed * 0.8 + abs(roll) * 0.2 + noise(0.5)

        return self._make_telemetry(airspeed, roll, pitch, current)

    def _step_returning(self, dt):
        """Fly back toward helipad, then transition to landing."""
        self._prev_alt = self._alt

        bearing, dist_m = bearing_and_distance(
            self._lat, self._lon, ALPHA_HOME_LAT, ALPHA_HOME_LON
        )

        self._heading, turn = steer_toward(self._heading, bearing, 30, dt)
        roll = clamp(turn * 1.0, -30, 30)

        # Slow down as we approach
        airspeed = clamp(min(12.0, dist_m * 0.3), 2, 12) + noise(0.2)

        hdg_rad = math.radians(self._heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * _DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * _DEG_PER_M_LON

        # Hold altitude during return
        self._alt += noise(0.3)
        self._alt = clamp(self._alt, 60, 200)

        self._battery_pct = clamp(self._battery_pct - dt * 0.1, 0, 100)
        current = 5.0 + airspeed * 0.5 + noise(0.3)
        pitch = clamp(1.0 + noise(0.2), -5, 10)

        # Close enough — start landing
        if dist_m < 15:
            self._state = LANDING

        return self._make_telemetry(airspeed, roll, pitch, current)

    def _step_landing(self, dt):
        """Descend vertically and land. Recharge on touchdown."""
        self._prev_alt = self._alt

        # Snap position to helipad
        self._lat += (ALPHA_HOME_LAT - self._lat) * min(1.0, 2.0 * dt)
        self._lon += (ALPHA_HOME_LON - self._lon) * min(1.0, 2.0 * dt)

        descent_rate = 3.0
        self._alt -= descent_rate * dt
        self._alt = max(0, self._alt + noise(0.1))

        self._battery_pct = clamp(self._battery_pct - dt * 0.08, 0, 100)
        pitch = clamp(3.0 + noise(0.2), 0, 8)
        current = 4.0 + noise(0.3)

        # Touchdown
        if self._alt <= 0.5:
            self._reset()

        return self._make_telemetry(0.5, noise(0.3), pitch, current)


class DroneBravo:
    """Figure-8 survey flight profile at 200 m AGL.

    Starts grounded at Trockener Steg plateau near the Matterhorn.
    Same state machine as Alpha: TKO to launch, RTH to return and recharge.
    """

    def __init__(self):
        self._t = 0.0
        self._state = GROUNDED
        self._battery_pct = 100.0
        self._cruise_alt = 200.0
        self._lat = BRAVO_HOME_LAT
        self._lon = BRAVO_HOME_LON
        self._heading = 0.0
        self._alt = 0.0
        self._prev_alt = 0.0

    def _reset(self):
        """Reset to grounded state with full battery at helipad."""
        self._state = GROUNDED
        self._battery_pct = 100.0
        self._lat = BRAVO_HOME_LAT
        self._lon = BRAVO_HOME_LON
        self._alt = 0.0
        self._prev_alt = 0.0

    def step(self, dt, cmds=None):
        self._t += dt

        if cmds and cmds.tko and self._state == GROUNDED:
            self._state = TAKEOFF

        if cmds and cmds.rth and self._state == CRUISE:
            self._state = RETURNING

        if self._state == GROUNDED:
            return self._step_grounded(dt)
        elif self._state == TAKEOFF:
            return self._step_takeoff(dt, cmds)
        elif self._state == CRUISE:
            return self._step_cruise(dt, cmds)
        elif self._state == RETURNING:
            return self._step_returning(dt)
        elif self._state == LANDING:
            return self._step_landing(dt)

    def _make_telemetry(self, airspeed, roll, pitch, current):
        vspeed = (self._alt - self._prev_alt) / 0.1 if self._prev_alt is not None else 0.0
        voltage = 21.0 + (self._battery_pct / 100.0) * 4.2 + noise(0.05)
        return {
            "lat": round(self._lat, 6),
            "lon": round(self._lon, 6),
            "heading": round(self._heading % 360, 2),
            "alt": round(max(0, self._alt), 2),
            "airspeed": round(clamp(airspeed, 0, 50), 2),
            "vspeed": round(vspeed, 2),
            "roll": round(roll, 2),
            "pitch": round(pitch, 2),
            "voltage": round(clamp(voltage, 18, 25.2), 2),
            "current": round(clamp(current, 0, 35), 2),
            "battery_pct": round(self._battery_pct, 1),
        }

    def _step_grounded(self, dt):
        self._prev_alt = self._alt
        self._alt = 0.0
        return self._make_telemetry(0, 0, 0, 0.5)

    def _step_takeoff(self, dt, cmds):
        """Rocket climb to cruise altitude — intentionally fast for demo visibility."""
        self._prev_alt = self._alt
        climb_rate = 80.0
        self._alt += climb_rate * dt + noise(0.5)
        self._alt = max(0, self._alt)

        # Heavy battery drain during aggressive climb
        self._battery_pct = clamp(self._battery_pct - dt * 1.8, 0, 100)

        if self._alt >= self._cruise_alt:
            self._alt = self._cruise_alt
            self._state = CRUISE

        pitch = clamp(30.0 + noise(1.0), 10, 30)
        current = 35.0 + noise(1.5)
        return self._make_telemetry(10.0 + noise(0.5), noise(3.0), pitch, current)

    def _step_cruise(self, dt, cmds):
        t = self._t
        self._prev_alt = self._alt

        base_turn_rate = 12.0 * math.sin(t * 0.105)
        base_airspeed = 22.0

        throttle_frac = cmds.throttle / 50.0 if cmds else 1.0
        airspeed = base_airspeed * throttle_frac + noise(0.4)
        airspeed = clamp(airspeed, 0, 50)

        heading_offset = cmds.heading_offset if cmds else 0.0
        self._heading += (base_turn_rate + heading_offset * 0.15) * dt
        self._heading %= 360.0

        turn_rate = base_turn_rate + heading_offset * 0.15
        roll = clamp(turn_rate * 1.6 + noise(1.0), -45, 45)

        hdg_rad = math.radians(self._heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * _DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * _DEG_PER_M_LON

        self._alt = self._cruise_alt + 15 * math.sin(t * 0.2) + noise(0.8)
        self._alt = clamp(self._alt, 150, 250)

        pitch = clamp(-3.0 + (1.0 - throttle_frac) * 5.0 + noise(0.4), -20, 20)

        drain = 0.18 * (0.5 + throttle_frac * 0.5)
        self._battery_pct = clamp(self._battery_pct - dt * drain, 0, 100)
        current = 10.0 + airspeed * 0.6 + abs(roll) * 0.2 + noise(0.6)

        return self._make_telemetry(airspeed, roll, pitch, current)

    def _step_returning(self, dt):
        self._prev_alt = self._alt

        bearing, dist_m = bearing_and_distance(
            self._lat, self._lon, BRAVO_HOME_LAT, BRAVO_HOME_LON
        )

        self._heading, turn = steer_toward(self._heading, bearing, 30, dt)
        roll = clamp(turn * 1.0, -30, 30)

        airspeed = clamp(min(15.0, dist_m * 0.3), 2, 15) + noise(0.2)

        hdg_rad = math.radians(self._heading)
        self._lat += math.cos(hdg_rad) * airspeed * dt * _DEG_PER_M_LAT
        self._lon += math.sin(hdg_rad) * airspeed * dt * _DEG_PER_M_LON

        self._alt += noise(0.3)
        self._alt = clamp(self._alt, 80, 260)

        self._battery_pct = clamp(self._battery_pct - dt * 0.12, 0, 100)
        current = 6.0 + airspeed * 0.5 + noise(0.3)
        pitch = clamp(1.0 + noise(0.2), -5, 10)

        if dist_m < 15:
            self._state = LANDING

        return self._make_telemetry(airspeed, roll, pitch, current)

    def _step_landing(self, dt):
        self._prev_alt = self._alt

        self._lat += (BRAVO_HOME_LAT - self._lat) * min(1.0, 2.0 * dt)
        self._lon += (BRAVO_HOME_LON - self._lon) * min(1.0, 2.0 * dt)

        descent_rate = 4.0
        self._alt -= descent_rate * dt
        self._alt = max(0, self._alt + noise(0.1))

        self._battery_pct = clamp(self._battery_pct - dt * 0.1, 0, 100)
        pitch = clamp(3.0 + noise(0.2), 0, 8)
        current = 5.0 + noise(0.4)

        if self._alt <= 0.5:
            self._reset()

        return self._make_telemetry(0.5, noise(0.3), pitch, current)


# ---------------------------------------------------------------------------
# Command parser & TCP server
# ---------------------------------------------------------------------------


class CommandState:
    """Mutable state driven by output-control commands from Serial Studio."""

    def __init__(self):
        self.throttle = 50.0      # 0-100 %
        self.heading_offset = 0.0 # -180 to +180 deg
        self.camera_on = True
        self.rth = False
        self.tko = False

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
            elif cmd == "TKO":
                self.tko = True


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
            if not server.wait_for_client(stop_event):
                break

            drone = DroneAlpha()
            cmds = CommandState()
            print(f"[Alpha] Streaming at {fps} Hz — drone grounded, send TKO to launch")

            while not stop_event.is_set() and server.client:
                t_start = time.monotonic()

                while True:
                    raw = server.recv()
                    if raw is None:
                        break
                    cmds.parse(raw)

                if not server.client:
                    break

                data = drone.step(dt, cmds)

                # Reset one-shot command flags after each step
                cmds.rth = False
                cmds.tko = False

                if HAS_CV2:
                    if cmds.camera_on:
                        img = make_alpha_image(
                            data["lat"], data["lon"], data["heading"], data["alt"]
                        )
                    else:
                        img = make_camera_off_image("ALPHA")

                    jpeg = encode_jpeg(img)
                    if jpeg:
                        server.send_raw(IMG_ALPHA_START + jpeg + IMG_ALPHA_END)

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
                        f"[{drone._state:>9s}] "
                        f"lat={data['lat']:.4f} alt={data['alt']:.0f}m "
                        f"hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}%"
                    )

                _refresh_display()

                elapsed = time.monotonic() - t_start
                sleep_time = dt - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)

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
            if not server.wait_for_client(stop_event):
                break

            drone = DroneBravo()
            cmds = CommandState()
            print(f"[Bravo] Streaming at {fps} Hz — drone grounded, send TKO to launch")

            while not stop_event.is_set() and server.client:
                t_start = time.monotonic()

                while True:
                    raw = server.recv()
                    if raw is None:
                        break
                    cmds.parse(raw)

                if not server.client:
                    break

                data = drone.step(dt, cmds)

                # Reset one-shot command flags after each step
                cmds.rth = False
                cmds.tko = False

                if HAS_CV2:
                    if cmds.camera_on:
                        img = make_bravo_image(
                            data["lat"], data["lon"], data["heading"], data["alt"]
                        )
                    else:
                        img = make_camera_off_image("BRAVO")

                    jpeg = encode_jpeg(img)
                    if jpeg:
                        server.send_raw(IMG_BRAVO_START + jpeg + IMG_BRAVO_END)

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
                        f"[{drone._state:>9s}] "
                        f"lat={data['lat']:.4f} alt={data['alt']:.0f}m "
                        f"hdg={data['heading']:.0f} bat={data['battery_pct']:.0f}%"
                    )

                _refresh_display()

                elapsed = time.monotonic() - t_start
                sleep_time = dt - elapsed
                if sleep_time > 0:
                    time.sleep(sleep_time)

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

    print("Dual Drone Telemetry Simulator — Swiss Alps (Zermatt)")
    print(f"FPS: {args.fps}  |  Host: {args.host}")
    print(f"Alpha helipad: {ALPHA_HOME_LAT:.4f}, {ALPHA_HOME_LON:.4f} (Zermatt village)")
    print(f"Bravo helipad: {BRAVO_HOME_LAT:.4f}, {BRAVO_HOME_LON:.4f} (Trockener Steg)")
    if HAS_CV2:
        print(f"Camera: {IMG_WIDTH}x{IMG_HEIGHT} satellite JPEG @ quality {JPEG_QUALITY}")
        print(f"Tile cache: {_TILE_CACHE_DIR}")
    else:
        print("WARNING: opencv-python not installed — no camera images will be sent")
        print("         Install with: pip install opencv-python numpy")
    print("Drones start grounded. Send TKO to launch, RTH to return.")
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
