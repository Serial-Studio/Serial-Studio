# System Monitor

## Overview

This example turns Serial Studio into a live system dashboard — like a minimal
Task Manager — by using the **Process I/O driver** (Pro) to launch a Python
script and read its `stdout` directly as a telemetry stream.

No network sockets, no serial cables, no external hardware required.
Serial Studio spawns `system-monitor.py` (or the platform launcher), and the
script continuously writes CPU, RAM, disk, network, and temperature metrics to
`stdout`. Serial Studio parses each line as a frame and drives gauges, bar
graphs, and trend plots in real time.

**Note:** The Process I/O driver requires a Serial Studio Pro license.
Visit [serial-studio.com](https://serial-studio.com/) for details.

## What It Monitors

| Metric | Widget | Notes |
|---|---|---|
| CPU Usage | Gauge + line plot | Overall utilisation (%) |
| CPU Temperature | Bar graph | °C; shown as `-1` if unavailable |
| RAM Usage | Gauge + line plot | Memory pressure (%) |
| RAM Used | Bar graph | Absolute consumption (GB) |
| Disk Usage | Gauge + line plot | Root partition (%) |
| Disk Used | Bar graph | Space consumed (GB) |
| Network Upload | Bar graph + plot | KB/s delta since last frame |
| Network Download | Bar graph + plot | KB/s delta since last frame |
| Process Count | Bar graph | Total running processes |
| System Info | Data grid | OS, hostname, CPU model, cores, RAM, disk — static |

## Frame Format

The script emits two types of frames to `stdout`:

### 1 — Header frame (emitted once at startup)
Static system information, useful for the data-grid widget:

```
$OS=macOS 14.4|HOSTNAME=myhost|CPU_MODEL=Apple M2|CPU_CORES=8|CPU_THREADS=8|RAM_TOTAL=16.0 GB|DISK_TOTAL=494.4 GB
```

### 2 — Live frames (emitted every second)
Real-time resource usage:

```
$CPU_USAGE=34.2|RAM_USAGE=61.0|RAM_USED=9.76|DISK_USAGE=44.1|DISK_USED=218.3|NET_SENT=12.4|NET_RECV=87.1|CPU_TEMP=52.0|PROCESSES=413
```

All frames use `$` as `frameStart` and `\n` as `frameEnd`. The JavaScript
parser in the project file splits on `|` and `=` to build a key→value lookup,
so both frame types are handled by the same function — unknown keys are
silently ignored, and missing keys leave their slot empty.

## How to Run

### Step 1 — Install the dependency

```bash
pip install psutil
```

`psutil` provides cross-platform access to CPU, memory, disk, and network
counters. It is the only third-party dependency.

### Step 2 — Configure Serial Studio

1. Open Serial Studio and load **`system-monitor.ssproj`**.
2. In the **Setup** panel, set **Bus Type** to **Process**.
3. Set **Mode** to **Launch**.
4. Set **Executable** to one of:
   - **macOS / Linux:** full path to `system-monitor.sh`
   - **Windows:** full path to `system-monitor.bat`
   - Or point directly to `python3` / `python` and set **Arguments** to the
     full path of `system-monitor.py`.
5. Leave **Working Directory** empty (the script resolves its own path).
6. Click **Connect**.

Serial Studio launches the script, reads its `stdout`, and begins populating
the dashboard immediately.

### Optional — Run from the terminal

You can also run the script standalone to verify output before connecting
Serial Studio:

```bash
# macOS / Linux
./system-monitor.sh

# Windows
system-monitor.bat

# Direct Python (any platform)
python3 system-monitor.py --interval 0.5
```

Press `Ctrl+C` to stop.

**Arguments:**

| Argument | Default | Description |
|---|---|---|
| `--interval SECONDS` | `1.0` | Seconds between live frames |

## Files

| File | Description |
|---|---|
| `system-monitor.py` | Python script — data source |
| `system-monitor.sh` | Unix launcher (macOS / Linux) |
| `system-monitor.bat` | Windows launcher |
| `system-monitor.ssproj` | Serial Studio project file |
| `README.md` | This document |

## How the Frame Parser Works

The JavaScript parser inside `system-monitor.ssproj` splits each frame on
`|` to get individual `KEY=value` pairs, then indexes them by name into a
fixed-position array that the dashboard datasets reference by index:

```javascript
// Simplified view of the parser
function parse(frame) {
    var lookup = {};
    frame.split('|').forEach(function(pair) {
        var parts = pair.split('=');
        lookup[parts[0].trim()] = parts[1].trim();
    });

    return [
        lookup['OS'],          // index 1  — System Info grid
        lookup['HOSTNAME'],    // index 2
        lookup['CPU_MODEL'],   // index 3
        lookup['CPU_CORES'],   // index 4
        lookup['CPU_THREADS'], // index 5
        lookup['RAM_TOTAL'],   // index 6
        lookup['DISK_TOTAL'],  // index 7
        lookup['CPU_USAGE'],   // index 8  — CPU gauge / plot
        lookup['RAM_USAGE'],   // index 9  — Memory gauge / plot
        lookup['RAM_USED'],    // index 10
        lookup['DISK_USAGE'],  // index 11 — Disk gauge / plot
        lookup['DISK_USED'],   // index 12
        lookup['NET_SENT'],    // index 13 — Network bars / plots
        lookup['NET_RECV'],    // index 14
        lookup['CPU_TEMP'],    // index 15 — CPU temperature bar
        lookup['PROCESSES']    // index 16 — Process count bar
    ];
}
```

Because the parser returns an empty string `''` for any key not present in a
given frame, the header frame fills slots 1–7 and leaves 8–16 blank, while
live frames fill 8–16 and leave 1–7to whatever value Serial Studio last saw —
which is the static information from the header.

## Notes

- **CPU temperature** requires kernel access on some platforms. On Linux,
  `lm-sensors` may need to be installed. On macOS, temperature is read via
  `IOKit` (available on Apple Silicon; may be absent on Intel without third-
  party tools). If unavailable, `CPU_TEMP` is emitted as `-1`.
- **Network counters** are system-wide cumulative totals. The script computes
  the per-interval delta so the values represent KB transferred *since the
  previous frame*, not a running total.
- The `system-monitor.sh` and `system-monitor.bat` launchers auto-install
  `psutil` via `pip` if it is missing, so first-run setup is automatic when
  Serial Studio launches the script.

## Dependencies

- Python 3.8 or later
- [`psutil`](https://pypi.org/project/psutil/) — `pip install psutil`

## License

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
