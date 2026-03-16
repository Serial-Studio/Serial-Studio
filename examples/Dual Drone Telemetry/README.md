# Dual Drone Telemetry

Multi-source drone telemetry simulator demonstrating Serial Studio's ability to receive and visualize data from two independent devices simultaneously, each with its own camera feed, GPS coordinates, flight dynamics, and battery monitoring.

![Dual Drone Telemetry Screenshot](doc/screenshot.png)

## Overview

Two simulated drones fly completely different flight profiles over the Nevada desert, each transmitting telemetry and synthetic camera imagery on separate UDP ports:

| Drone | Port | Flight Pattern | Altitude | Camera Style |
|-------|------|---------------|----------|-------------|
| **Alpha** | 9001 | Circular patrol (~330 m radius) | 120 m | Top-down terrain (green/brown palette) |
| **Bravo** | 9002 | Figure-8 survey (~550 m radius) | 200 m | Thermal/IR (iron-bow palette) |

### Camera Feeds

**Alpha Terrain** — top-down patrol camera view:
- Procedural green/brown terrain with position-based parallax
- Roads rendered as grid lines based on GPS position
- Targeting reticle circle with crosshair lines and heading arrow
- Green HUD overlay with altitude, heading, and GPS coordinates

**Bravo Thermal** — synthetic thermal/infrared view:
- Iron-bow color palette (black → purple → orange → yellow)
- Moving thermal hot spots simulating heat signatures
- Scanning line and grid overlay
- Amber HUD overlay with FLIR label, altitude, and GPS coordinates
- Simple crosshair at center

### Telemetry per drone (11 fields)

| Field | Units | Description |
|-------|-------|-------------|
| Latitude | deg | GPS latitude |
| Longitude | deg | GPS longitude |
| Heading | deg | Compass heading (0-360) |
| Altitude | m | Altitude above ground |
| Airspeed | m/s | Forward speed |
| Vertical Speed | m/s | Climb/descent rate |
| Roll | deg | Bank angle |
| Pitch | deg | Nose up/down angle |
| Battery Voltage | V | 6S LiPo (18-25.2 V) |
| Current Draw | A | Motor + avionics current |
| Battery % | % | Remaining charge (drains over time) |

## Quick Start

1. Open `Dual Drone Telemetry.ssproj` in Serial Studio
2. Connect both UDP sources (ports 9001 and 9002)
3. Run the simulator:

```bash
pip install opencv-python numpy   # for camera imagery
python3 dual_drone_telemetry.py
```

The simulator works without opencv -- you just won't get camera images.

## Command-Line Options

| Flag | Default | Description |
|------|---------|-------------|
| `--fps` | 10 | Update rate in Hz (telemetry + camera) |
| `--host` | 127.0.0.1 | UDP destination host |

## Protocol Details

Both drones use hexadecimal frame delimiters with JPEG images interleaved in the same byte stream:

- **Alpha**: `AB CD EF` (start) / `FE ED` (end)
- **Bravo**: `DE AD` (start) / `FE ED` (end)

Each cycle sends a raw JPEG frame first, then a delimited CSV telemetry frame.
The Image View widget auto-detects JPEG frames by scanning for `FF D8 FF` magic bytes, independent of the CSV telemetry path.

## Dashboard Widgets

Each drone has six widget groups on the dashboard:

| Widget | Type | Description |
|--------|------|-------------|
| Camera | Image View | Live terrain (Alpha) or thermal/IR (Bravo) camera feed |
| Position | Map | GPS track on interactive map |
| Heading | Compass | Compass heading indicator |
| Attitude | Gyroscope | Roll, pitch, and yaw (heading) visualization |
| Flight | Gauges + Bars | Airspeed, altitude, and vertical speed |
| Power | Gauges + Bars | Battery voltage, current draw, and charge level |

## Flight Models

**Drone Alpha** flies a steady circular patrol:
- Constant mild bank angle (~12 deg)
- Slow altitude oscillation (+/- 8 m)
- Battery drain: ~0.15%/sec

**Drone Bravo** flies an aggressive figure-8 survey:
- Dynamic banking (up to +/- 40 deg)
- Higher altitude variation (+/- 15 m)
- Faster speed, higher current draw
- Battery drain: ~0.18%/sec

Both drones have low-battery alarms configured at 20% charge and 21V.

## Requirements

- Python 3.6+
- `opencv-python` and `numpy` (optional, for camera imagery)
- Serial Studio Pro (multi-source + Image View features)

<!--
  SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
-->
