#!/usr/bin/env python3
"""
Dual Drone Telemetry Simulator for Serial Studio

Simulates two drones transmitting flight telemetry and synthetic camera
imagery over separate TCP connections from the Swiss Alps (Zermatt area):

  Drone Alpha (port 9001) — Circular patrol at 120 m AGL
    CSV delimiters:   A1 01 A1 01 … A1 02 A1 02 (hex)
    Image delimiters: A1 CA FE 01 … A1 FE ED 01
    11 CSV fields + JPEG camera frames

  Drone Bravo (port 9002) — Figure-8 survey at 200 m AGL
    CSV delimiters:   B2 03 B2 03 … B2 04 B2 04 (hex)
    Image delimiters: B2 CA FE 02 … B2 FE ED 02
    11 CSV fields + JPEG camera frames

Both drones start grounded at their helipad near Zermatt. Use the TKO
command to launch, and RTH to bring them back. On landing the battery is
recharged and the drone is ready for another flight.

Each drone generates a unique procedural camera image every frame:
  Alpha — top-down alpine terrain view with snow-capped peaks, green
          valleys, and rock ridges plus a targeting reticle.
  Bravo — synthetic thermal/IR style view showing terrain heat
          signatures with a scanning grid overlay.

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
# Synthetic image generators
# ---------------------------------------------------------------------------


def make_alpha_image(t, lat, lon, heading, alt):
    """
    Generate a top-down alpine terrain camera view for Drone Alpha.
    Snow-capped peaks, green valleys, and rock ridges with a targeting reticle.
    """
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)

    # Alpine terrain — elevation-based coloring with position-dependent features
    for y in range(0, IMG_HEIGHT, 4):
        for x in range(0, IMG_WIDTH, 4):
            fx = (x / IMG_WIDTH + lon * 80 + t * 0.01) * 6.0
            fy = (y / IMG_HEIGHT + lat * 80 + t * 0.008) * 6.0

            # Multi-octave terrain height
            h = (math.sin(fx * 1.1) * math.cos(fy * 0.9) + 1.0) * 0.3
            h += (math.sin(fx * 2.7 + 1.5) * math.cos(fy * 3.1 + 0.8) + 1.0) * 0.15
            h += (math.sin(fx * 5.3 + 3.0) * math.cos(fy * 4.7 + 2.1) + 1.0) * 0.07

            # Ridge lines
            ridge = abs(math.sin(fx * 1.5 + fy * 0.7))
            h += ridge * 0.1

            h = clamp(h, 0, 1)

            # Color by elevation: valley green → rock grey → snow white
            if h < 0.3:
                # Valley — dark green meadow
                g = int(80 + h / 0.3 * 60)
                r = int(40 + h / 0.3 * 20)
                b = int(20 + h / 0.3 * 10)
            elif h < 0.55:
                # Mid-altitude — rock and sparse vegetation
                t2 = (h - 0.3) / 0.25
                g = int(140 - t2 * 50)
                r = int(60 + t2 * 60)
                b = int(30 + t2 * 40)
            elif h < 0.75:
                # High rock — grey granite
                t2 = (h - 0.55) / 0.2
                grey = int(110 + t2 * 40)
                r, g, b = grey, grey - 5, grey - 10
            else:
                # Snow cap — bright white with blue tint
                t2 = (h - 0.75) / 0.25
                r = int(200 + t2 * 55)
                g = int(200 + t2 * 55)
                b = int(210 + t2 * 45)

            img[y : y + 4, x : x + 4] = (b, g, r)

    # River/stream through the valley
    for px in range(0, IMG_WIDTH, 2):
        river_y = int(
            IMG_HEIGHT * 0.5
            + 30 * math.sin(px / IMG_WIDTH * 4.0 + lon * 200)
            + 15 * math.sin(px / IMG_WIDTH * 7.0 + lat * 150)
        )
        river_y = clamp(river_y, 2, IMG_HEIGHT - 3)
        cv2.line(img, (px, river_y - 1), (px + 2, river_y + 1), (160, 120, 50), 2)

    # Static grid overlay
    grid_spacing = 40
    for gx in range(0, IMG_WIDTH, grid_spacing):
        cv2.line(img, (gx, 0), (gx, IMG_HEIGHT), (0, 180, 0), 1)
    for gy in range(0, IMG_HEIGHT, grid_spacing):
        cv2.line(img, (0, gy), (IMG_WIDTH, gy), (0, 180, 0), 1)

    # HUD overlay
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

    return img


def make_bravo_image(t, lat, lon, heading, alt):
    """
    Generate a synthetic thermal/IR camera view for Drone Bravo.
    Terrain-aware heat map: snow is cold (dark), exposed rock and valleys are warm.
    """
    img = np.zeros((IMG_HEIGHT, IMG_WIDTH, 3), dtype=np.uint8)

    for y in range(0, IMG_HEIGHT, 4):
        for x in range(0, IMG_WIDTH, 4):
            fx = (x / IMG_WIDTH + lon * 80 + t * 0.015) * 6.0
            fy = (y / IMG_HEIGHT + lat * 80 + t * 0.012) * 6.0

            # Same terrain shape as Alpha (correlated terrain)
            h = (math.sin(fx * 1.1) * math.cos(fy * 0.9) + 1.0) * 0.3
            h += (math.sin(fx * 2.7 + 1.5) * math.cos(fy * 3.1 + 0.8) + 1.0) * 0.15
            h += (math.sin(fx * 5.3 + 3.0) * math.cos(fy * 4.7 + 2.1) + 1.0) * 0.07

            # Thermal value — low terrain = warm, high terrain (snow) = cold
            # Add time-varying solar heating on south-facing slopes
            solar = (math.sin(fx * 1.5 + t * 0.3) + 1.0) * 0.1
            val = clamp(1.0 - h + solar, 0, 1)

            # Warm spots (e.g. buildings, animals in the valley)
            dx = (x / IMG_WIDTH - 0.5) * 2
            dy = (y / IMG_HEIGHT - 0.5) * 2
            for sx, sy in [
                (math.sin(t * 0.3) * 0.5, math.cos(t * 0.25) * 0.3),
                (math.cos(t * 0.4 + 2) * 0.4, math.sin(t * 0.35 + 1) * 0.4),
            ]:
                dist = math.sqrt((dx - sx) ** 2 + (dy - sy) ** 2)
                val += max(0, 0.3 - dist) * 1.2

            val = clamp(val, 0, 1)

            # Iron-bow thermal palette
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

    # HUD overlay
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
                            drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
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
                            drone._t, data["lat"], data["lon"], data["heading"], data["alt"]
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
        print(f"Camera: {IMG_WIDTH}x{IMG_HEIGHT} synthetic JPEG @ quality {JPEG_QUALITY}")
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
