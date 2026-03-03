# System Monitor

## Overview

This example turns Serial Studio into a live system dashboard — like a minimal
htop — by using the **Process I/O driver** (Pro) to launch a Python script and
read its `stdout` directly as a telemetry stream.

No network sockets, no serial cables, no external hardware required.
Serial Studio spawns `system-monitor.py` (or the platform launcher), and the
script continuously writes metrics to `stdout` at ~30 Hz. Serial Studio parses
each line as a frame and drives gauges, multiplots, bar graphs, and data grids
in real time.

**Note:** The Process I/O driver requires a Serial Studio Pro license.
Visit [serial-studio.com](https://serial-studio.com/) for details.

---

## TODO

- **Dynamic dashboard model via JS API** — the frame parser currently targets a
  fixed index layout baked into the `.ssproj` file. A JS-based API that lets the
  parser itself declare groups, datasets, and widget types at runtime would allow
  the dashboard to adapt automatically to the machine (e.g. show only as many
  CPU core curves as the host actually has, without needing a `MAX_CORES = 32`
  sentinel approach). This requires a Serial Studio-side API extension.

---

## What It Monitors

| Metric | Widget | Notes |
|---|---|---|
| CPU Usage | Gauge + line plot | Overall utilisation (%) |
| CPU Temperature | Bar | °C; `N/A` if unavailable (e.g. macOS) |
| Per-Core CPU | Multiplot | One curve per logical core; cores beyond the machine's count are hidden via `-1` sentinel |
| RAM Usage | Gauge + line plot | Memory pressure (%) |
| RAM Used | Bar | Absolute consumption (GB) |
| Swap Used | Bar | Page-file / swap used (GB) |
| Disk Usage | Gauge + line plot | Root partition (%) |
| Disk Used | Bar | Space consumed (GB) |
| Network Upload / Download | Multiplot | MB/s delta since last frame |
| Process Count | Bar | Total running processes |
| Top 10 Processes | Data grid | Sorted by CPU; format: `Name (nn.n% CPU)` |
| System Info | Data grid | OS, hostname, CPU model, cores, RAM, disk — static |

---

## Frame Format

The script emits two types of frames to `stdout`:

### 1 — Header frame (emitted once at startup)

Static system information that populates the System Info data grid:

```
$OS=Darwin 25.3.0|HOSTNAME=myhost|CPU_MODEL=Apple M2 Pro|CPU_CORES=12|CPU_THREADS=12|RAM_TOTAL=17.2 GB|DISK_TOTAL=494.4 GB
```

### 2 — Live frames (emitted at ~30 Hz)

Real-time resource usage, per-core CPU, and top processes:

```
$CPU_USAGE=9.4|RAM_USAGE=62.1|RAM_USED=8.50|DISK_USAGE=18.3|DISK_USED=12.3|CPU_TEMP=N/A|SWAP_USED=1.7|NET_SENT=0.124|NET_RECV=0.871|PROCESSES=682|CORE0=17.2|CORE1=16.0|...|CORE31=-1|PROC0=WindowServer (12.5% CPU)|PROC1=python3 (8.1% CPU)|...
```

All frames use `$` as `frameStart` and `\n` as `frameEnd`.

#### Core sentinel value

The project always declares 32 core slots (`CORE0`–`CORE31`). Slots beyond the
machine's actual logical core count are emitted as `-1`. The multiplot widget
treats `-1` as out-of-range and hides those curves automatically.

---

## How to Run

### Step 1 — Install dependencies

```bash
pip install psutil py-cpuinfo
```

- **`psutil`** — cross-platform CPU, memory, disk, network, and process counters
- **`py-cpuinfo`** — reliable CPU brand string on all platforms (including Apple
  Silicon, where `platform.processor()` returns only `"arm"`)

The platform launchers (`system-monitor.sh` / `system-monitor.bat`) auto-install
both packages via `pip` if they are missing.

### Step 2 — Configure Serial Studio

1. Open Serial Studio and load **`system-monitor.ssproj`**.
2. In the **Setup** panel, set **Bus Type** to **Process**.
3. Set **Mode** to **Launch**.
4. Set **Executable** to one of:
   - **macOS / Linux:** full path to `system-monitor.sh`
   - **Windows:** full path to `system-monitor.bat`
   - Or point directly to `python3` and set **Arguments** to the full path of
     `system-monitor.py`.
5. Leave **Working Directory** empty (the script resolves its own path).
6. Click **Connect**.

### Optional — Run from the terminal

```bash
# macOS / Linux
./system-monitor.sh

# Windows
system-monitor.bat

# Direct Python (any platform)
python3 system-monitor.py --interval 0.033
```

Press `Ctrl+C` to stop.

**Arguments:**

| Argument | Default | Description |
|---|---|---|
| `--interval SECONDS` | `1/30 ≈ 0.033` | Seconds between live frames |

---

## How the Frame Parser Works

The JavaScript parser inside `system-monitor.ssproj` uses a **persistent value
array** declared at module scope. Keys absent from a frame keep their last value,
so the static header fields (OS, hostname, CPU model…) remain visible while live
metric frames keep updating the rest.

```javascript
// Module-scope globals — persist across parse() calls in QJSEngine
var _map  = { "OS": 0, "CPU_USAGE": 7, "CORE0": 17, ... };
var _vals = new Array(59).fill('');

function parse(frame) {
    var pairs = frame.split('|');
    for (var i = 0; i < pairs.length; i++) {
        var eq  = pairs[i].indexOf('=');
        var key = pairs[i].substring(0, eq).trim();
        var val = pairs[i].substring(eq + 1).trim();
        if (_map.hasOwnProperty(key))
            _vals[_map[key]] = val;
    }
    return _vals;
}
```

Dataset index `N` in the `.ssproj` maps to `_vals[N - 1]` (Serial Studio uses
1-based dataset indices, the JS array is 0-based).

---

## Index Layout

| Index | Key | Description |
|---|---|---|
| 1–7 | `OS` … `DISK_TOTAL` | Static system info |
| 8 | `CPU_USAGE` | Overall CPU % |
| 9 | `RAM_USAGE` | RAM % |
| 10 | `RAM_USED` | RAM used (GB) |
| 11 | `DISK_USAGE` | Disk % |
| 12 | `DISK_USED` | Disk used (GB) |
| 13 | `CPU_TEMP` | CPU temperature (°C or `N/A`) |
| 14 | `SWAP_USED` | Swap/page-file used (GB) |
| 15 | `NET_SENT` | Upload MB/s |
| 16 | `NET_RECV` | Download MB/s |
| 17 | `PROCESSES` | Total process count |
| 18–49 | `CORE0`–`CORE31` | Per-core CPU % (`-1` = core absent) |
| 50–59 | `PROC0`–`PROC9` | Top processes by CPU |

---

## Architecture Notes

- **CPU sampling** runs on a dedicated background thread using
  `psutil.cpu_percent(interval=1.0, percpu=True)`. This is necessary on macOS
  where `interval=None` always returns `0.0` until at least one blocking
  measurement has completed. The main loop reads the latest snapshot
  non-blocking via a lock.
- **Slow metrics** (disk, temperature, process list) are refreshed at 1 Hz
  regardless of frame rate to avoid I/O overhead at 30 Hz.
- **Fast metrics** (RAM, network) are sampled every frame.

---

## Dependencies

- Python 3.8 or later
- [`psutil`](https://pypi.org/project/psutil/) — `pip install psutil`
- [`py-cpuinfo`](https://pypi.org/project/py-cpuinfo/) — `pip install py-cpuinfo`

---

## License

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
