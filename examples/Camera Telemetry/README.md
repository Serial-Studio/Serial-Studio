# Camera Telemetry

## Overview

This example streams live camera video and computed image analytics to Serial
Studio over a single UDP connection at ~24 Hz.  It exercises the **Image View**
widget (Pro) alongside standard dashboard widgets (gauge, bar, multiplot).

**Difficulty:** 🔴 Advanced | **License:** Pro (Image View widget required)

No microcontroller needed — a standard webcam or the built-in laptop camera is
all the hardware required.  A synthetic test pattern is also available when no
camera is connected (`--no-camera`).

---

## What It Streams

### Image data (Image View widget)
Raw JPEG frames captured from the camera, sent as-is over UDP.  The Image View
widget uses **autodetect** mode: it finds JPEG frames by their `FF D8 FF` magic
bytes and `FF D9` end-of-image marker, completely independently of the CSV
telemetry parser.

### Telemetry (normal dashboard widgets)

| Index | Metric | Widget | Description |
|-------|--------|--------|-------------|
| 1 | Luminance | Gauge | Perceptual brightness (Y channel, 0–255) |
| 2 | Contrast | Bar | Grayscale standard deviation |
| 3 | Sharpness | Bar | Log-scaled Laplacian variance (0–100) |
| 4 | Dominant Hue | Gauge | Circular mean of the H channel (0°–360°) |
| 5 | Motion | Bar | Mean optical-flow magnitude (0–100) |
| 6 | FPS | Gauge | Actual stream frame rate |
| 7 | Frame Count | Data | Total frames sent since start |

---

## Frame Format

Everything goes through a single UDP stream.  Two types of data co-exist in the
same byte flow:

### 1 — JPEG image frame (binary)
Raw JPEG bytes with no additional framing.  The Image View widget detects them
automatically by magic bytes.

### 2 — CSV telemetry frame (ASCII)

```
$luminance,contrast,sharpness,hue,motion,fps,frame_count;\n
```

Example:
```
$142.37,18.52,62.14,215.3,3.47,23.9,1024;\n
```

Both types co-exist in the same byte stream.  The FrameReader ignores JPEG
binary data (no `$` start marker); the ImageFrameReader ignores the ASCII CSV
frames (no JPEG magic bytes).

---

## How to Run

### Step 1 — Install dependencies

```bash
pip install opencv-python numpy
```

### Step 2 — Start the script

```bash
python3 camera_telemetry.py
```

**Options:**

| Option | Default | Description |
|--------|---------|-------------|
| `--camera INDEX` | `0` | Camera device index |
| `--port PORT` | `9000` | UDP destination port |
| `--fps FPS` | `24` | Target frame rate |
| `--no-camera` | off | Use synthetic test pattern |

```bash
python3 camera_telemetry.py --no-camera        # no hardware needed
python3 camera_telemetry.py --camera 1         # secondary camera
python3 camera_telemetry.py --fps 10           # lower rate / less CPU
```

### Step 3 — Configure Serial Studio

1. Open Serial Studio and load **`Camera Telemetry.ssproj`**.
2. In the **Setup** panel set **Bus Type** → **Network Socket**, **Socket Type** → **UDP**.
3. Set **Port** to `9000` (or whatever `--port` you used).
4. Click **Connect**.

---

## Dashboard Groups

| Group | Widget | Description |
|-------|--------|-------------|
| Luminance | Gauge | Real-time brightness meter |
| Frame Quality | Bar × 2 | Contrast and sharpness side-by-side |
| Colour | Gauge | Dominant hue angle |
| Motion | Bar | Optical-flow activity level |
| Performance | Gauge + data | Live FPS and total frame count |
| Trends | Multiplot | Overlaid time plots of all metrics |
| Camera Feed | Image View | Live JPEG camera stream (Pro) |

---

## Notes

- JPEG quality is set to 70 by default to keep UDP packets well under 64 KB.
  Raise `JPEG_QUALITY` in the script for sharper images at the cost of larger
  packets and possible fragmentation.
- Frames wider than 640 px are automatically downscaled before encoding.
- Optical-flow computation adds CPU load; use `--fps 10` on slow machines.
- The synthetic pattern (`--no-camera`) produces a scrolling colour gradient
  with a moving white circle, giving non-trivial values for all metrics.

---

## Dependencies

- Python 3.8 or later
- [`opencv-python`](https://pypi.org/project/opencv-python/) — `pip install opencv-python`
- [`numpy`](https://pypi.org/project/numpy/) — installed as an opencv-python dependency

---

## License

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
