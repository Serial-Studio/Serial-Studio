# Widget Reference

## Overview

Serial Studio provides 15+ widget types for real-time data visualization. Widgets are organized into two categories: **group widgets** (display multiple datasets from a group) and **dataset widgets** (display a single dataset value).

## Widget Type Hierarchy

The following diagram shows all widget types organized by category, with their configuration keys and dataset requirements.

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 720 520" font-family="monospace" font-size="13">
  <!-- Background -->
  <rect width="720" height="520" fill="#f8f9fa" rx="8"/>

  <!-- Root -->
  <rect x="270" y="16" width="180" height="36" rx="6" fill="#2d3748" stroke="#1a202c" stroke-width="1.5"/>
  <text x="360" y="40" text-anchor="middle" fill="#fff" font-weight="bold">Dashboard Widgets</text>

  <!-- Branch lines -->
  <line x1="270" y1="52" x2="180" y2="80" stroke="#718096" stroke-width="1.5"/>
  <line x1="450" y1="52" x2="540" y2="80" stroke="#718096" stroke-width="1.5"/>

  <!-- Group Widgets header -->
  <rect x="60" y="80" width="240" height="32" rx="5" fill="#2b6cb0" stroke="#2c5282" stroke-width="1.5"/>
  <text x="180" y="101" text-anchor="middle" fill="#fff" font-weight="bold">Group Widgets (multi-dataset)</text>

  <!-- Dataset Widgets header -->
  <rect x="420" y="80" width="240" height="32" rx="5" fill="#2f855a" stroke="#276749" stroke-width="1.5"/>
  <text x="540" y="101" text-anchor="middle" fill="#fff" font-weight="bold">Dataset Widgets (single)</text>

  <!-- Group widget items -->
  <line x1="100" y1="112" x2="100" y2="440" stroke="#2b6cb0" stroke-width="1" stroke-dasharray="3,3"/>

  <!-- Data Grid -->
  <rect x="40" y="126" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="148" fill="#2b6cb0" font-weight="bold" font-size="11">Data Grid</text>
  <text x="195" y="148" fill="#718096" font-size="10">"datagrid" · 1+</text>

  <!-- MultiPlot -->
  <rect x="40" y="168" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="190" fill="#2b6cb0" font-weight="bold" font-size="11">Multiple Plot</text>
  <text x="195" y="190" fill="#718096" font-size="10">"multiplot" · 1+</text>

  <!-- GPS Map -->
  <rect x="40" y="210" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="232" fill="#2b6cb0" font-weight="bold" font-size="11">GPS Map</text>
  <text x="195" y="232" fill="#718096" font-size="10">"map" · 2-3 ds</text>

  <!-- Gyroscope -->
  <rect x="40" y="252" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="274" fill="#2b6cb0" font-weight="bold" font-size="11">Gyroscope</text>
  <text x="195" y="274" fill="#718096" font-size="10">"gyro" · 3 ds</text>

  <!-- Accelerometer -->
  <rect x="40" y="294" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="316" fill="#2b6cb0" font-weight="bold" font-size="11">Accelerometer</text>
  <text x="195" y="316" fill="#718096" font-size="10">"accelerometer" · 3</text>

  <!-- LED Panel -->
  <rect x="40" y="336" width="220" height="34" rx="4" fill="#ebf8ff" stroke="#2b6cb0" stroke-width="1"/>
  <text x="60" y="358" fill="#2b6cb0" font-weight="bold" font-size="11">LED Panel</text>
  <text x="195" y="358" fill="#718096" font-size="10">auto · led:true</text>

  <!-- 3D Plot -->
  <rect x="40" y="378" width="220" height="34" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="60" y="400" fill="#6b46c1" font-weight="bold" font-size="11">3D Plot [Pro]</text>
  <text x="195" y="400" fill="#718096" font-size="10">"plot3d" · 3 ds</text>

  <!-- Image View -->
  <rect x="40" y="420" width="220" height="34" rx="4" fill="#faf5ff" stroke="#6b46c1" stroke-width="1"/>
  <text x="60" y="442" fill="#6b46c1" font-weight="bold" font-size="11">Image View [Pro]</text>
  <text x="195" y="442" fill="#718096" font-size="10">"image" · 0 ds</text>

  <!-- Dataset widget items -->
  <line x1="500" y1="112" x2="500" y2="300" stroke="#2f855a" stroke-width="1" stroke-dasharray="3,3"/>

  <!-- Plot -->
  <rect x="420" y="126" width="240" height="34" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="440" y="148" fill="#2f855a" font-weight="bold" font-size="11">Plot</text>
  <text x="600" y="148" fill="#718096" font-size="10">graph:true</text>

  <!-- FFT Plot -->
  <rect x="420" y="168" width="240" height="34" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="440" y="190" fill="#2f855a" font-weight="bold" font-size="11">FFT Plot</text>
  <text x="600" y="190" fill="#718096" font-size="10">fft:true</text>

  <!-- Bar -->
  <rect x="420" y="210" width="240" height="34" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="440" y="232" fill="#2f855a" font-weight="bold" font-size="11">Bar</text>
  <text x="600" y="232" fill="#718096" font-size="10">"bar" · min/max</text>

  <!-- Gauge -->
  <rect x="420" y="252" width="240" height="34" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="440" y="274" fill="#2f855a" font-weight="bold" font-size="11">Gauge</text>
  <text x="600" y="274" fill="#718096" font-size="10">"gauge" · min/max</text>

  <!-- Compass -->
  <rect x="420" y="294" width="240" height="34" rx="4" fill="#e6fffa" stroke="#2f855a" stroke-width="1"/>
  <text x="440" y="316" fill="#2f855a" font-weight="bold" font-size="11">Compass</text>
  <text x="600" y="316" fill="#718096" font-size="10">"compass" · 0-360</text>

  <!-- Data type → widget guide -->
  <rect x="40" y="470" width="640" height="36" rx="4" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="360" y="493" text-anchor="middle" fill="#718096" font-size="10">Temp/Pressure → Plot+Gauge  |  GPS → Map  |  IMU → Accel+Gyro  |  Audio → FFT  |  Status → LED  |  Heading → Compass</text>
</svg>
```

## Group Widgets

### Data Grid

- Widget key: `"datagrid"`
- Displays all datasets in a tabular format with titles, values, and units
- Best for: overview of multiple channels, status monitoring
- Configuration: none required beyond adding datasets to the group
- Supports pause/resume

### Multiple Plot (MultiPlot)

- Widget key: `"multiplot"`
- Overlays multiple dataset curves on shared axes
- Individual curve visibility toggles
- Auto-scaling Y-axis
- Best for: comparing related signals (e.g., 3-axis accelerometer over time)
- Configuration: add datasets with `graph: true` to the group

### GPS Map

- Widget key: `"map"` (also accepts `"gps"`)
- Tile-based map with real-time position tracking
- Plots trajectory path
- Shows latitude, longitude, altitude
- Auto-center on latest position
- Zoom and pan with mouse
- Best for: vehicle tracking, drone telemetry, field measurements
- Requires: datasets providing latitude, longitude, and optionally altitude
- Internet connection required for map tiles; previously viewed areas are cached

### Gyroscope

- Widget key: `"gyro"` (also accepts `"gyroscope"`)
- 3D attitude indicator showing yaw, pitch, roll
- Best for: IMU visualization, drone orientation, robotics
- Requires: exactly 3 datasets (yaw, pitch, roll)
- OpenGL required for 3D rendering

### Accelerometer

- Widget key: `"accelerometer"`
- 3-axis acceleration visualization with computed total G-force
- Shows pitch, roll, peak G, magnitude
- Configurable max G range
- Best for: vibration analysis, impact detection, motion sensing
- Requires: exactly 3 datasets (X, Y, Z acceleration)
- OpenGL required for 3D rendering

### LED Panel

- Group widget (auto-created for datasets with `led: true`)
- Multiple boolean indicator LEDs
- LED turns on when value exceeds `ledHigh` threshold (default: 80)
- Best for: status flags, limit indicators, digital I/O states
- Configuration: set `led: true` and `ledHigh` threshold on each dataset

### Terminal

- Special widget — always available when console is enabled
- VT-100 emulation with ANSI color support
- Shows raw text data stream
- Configurable font and display settings
- Keyboard input forwarded to connected device

### 3D Plot [Pro]

- Widget key: `"plot3d"`
- 3D scatter/trajectory visualization
- Orbit or free-camera navigation
- Optional anaglyph stereo (red/cyan 3D glasses)
- Interpolation support
- Best for: 3D position tracking, spatial data, point clouds
- Requires: exactly 3 datasets (X, Y, Z coordinates)
- OpenGL required
- Pro license required

### Image View [Pro]

- Widget key: `"image"`
- Live JPEG/PNG/BMP/WebP image streaming from device
- Auto-detect or manual frame delimiter configuration
- Independent frame reader per widget — image frames and CSV telemetry coexist in the same byte stream
- Export, zoom, and image filter toolbar controls
- Best for: camera feeds, thermal imaging, visual inspection
- Group-level configuration fields:
  - `imgDetectionMode`: `"autodetect"` (default, uses format magic bytes) or `"manual"` (user-defined delimiters)
  - `imgStartSequence`: hex start delimiter (manual mode only)
  - `imgEndSequence`: hex end delimiter (manual mode only)
- No datasets required inside the group — the widget reads raw image bytes directly from the transport stream
- Pro license required

## Dataset Widgets

### Plot

- Auto-created for datasets with `graph: true` (field name `plt`)
- Single-curve 2D time-series line chart
- Auto-scaling Y-axis
- Configurable plot range via `pltMin` and `pltMax`
- Optional custom X-axis from another dataset via `xAxisId` (enables XY/scatter plots)
- Pause/resume
- Best for: individual signal monitoring, trend analysis

### FFT Plot

- Auto-created for datasets with `fft: true`
- Real-time frequency spectrum analysis via Fast Fourier Transform (KissFFT)
- Configurable FFT window size: 8 to 16384 samples (powers of 2), default 256
- Configurable sampling rate determines frequency axis (default 100 Hz)
- Configurable frequency range via `fftMin` and `fftMax`
- Best for: vibration frequency analysis, audio spectrum, signal quality
- Configuration fields: `fftSamples` (window size), `fftSamplingRate` (Hz), `fftMin`, `fftMax`

### Bar

- Dataset widget key: `"bar"`
- Horizontal bar gauge with min/max range
- Alarm thresholds with visual trigger state
- Shows current value and units
- Best for: level indicators, resource usage, bounded values
- Configuration fields: `wgtMin` (default 0), `wgtMax` (default 100), `alarmLow` (default 20), `alarmHigh` (default 80)
- Alarms must be enabled via `alarmEnabled: true`

### Gauge

- Dataset widget key: `"gauge"`
- Circular/arc gauge display
- Same configuration as Bar (min/max, alarms)
- Best for: speedometers, RPM, pressure, temperature
- Configuration fields: `wgtMin`, `wgtMax`, `alarmLow`, `alarmHigh`

### Compass

- Dataset widget key: `"compass"`
- Heading indicator (0-360 degrees)
- Auto-converts numeric heading to cardinal direction (N, NE, E, SE, S, SW, W, NW)
- Best for: heading/bearing, wind direction, orientation

## Widget Configuration Summary Table

| Widget | Type | Key | Min Datasets | Key Settings |
|--------|------|-----|--------------|--------------|
| Data Grid | Group | `datagrid` | 1+ | — |
| Multiple Plot | Group | `multiplot` | 1+ | `graph: true` on datasets |
| GPS Map | Group | `map` | 2-3 | lat, lon, (alt) datasets |
| Gyroscope | Group | `gyro` | 3 | yaw, pitch, roll |
| Accelerometer | Group | `accelerometer` | 3 | x, y, z accel |
| LED Panel | Group | auto | 1+ | `led: true`, `ledHigh` |
| 3D Plot | Group | `plot3d` | 3 | x, y, z coords [Pro] |
| Image View | Group | `image` | 0 | binary stream [Pro] |
| Plot | Dataset | auto | — | `graph: true`, pltMin/Max |
| FFT Plot | Dataset | auto | — | `fft: true`, fftSamples, fftSamplingRate |
| Bar | Dataset | `bar` | — | wgtMin/Max, alarmLow/High |
| Gauge | Dataset | `gauge` | — | wgtMin/Max, alarmLow/High |
| Compass | Dataset | `compass` | — | value 0-360 |

## Dataset Fields Reference

Every dataset in a project file supports the following visualization-related fields:

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `index` | int | 0 | Frame offset index (column position in CSV data) |
| `title` | string | — | Human-readable display name |
| `units` | string | — | Measurement units (e.g., "m/s", "degC") |
| `widget` | string | `""` | Dataset widget type: `"bar"`, `"gauge"`, or `"compass"` |
| `plt` (graph) | bool | false | Enable time-series plot |
| `fft` | bool | false | Enable FFT spectrum plot |
| `led` | bool | false | Enable LED indicator |
| `log` | bool | false | Enable logging to file |
| `overviewDisplay` | bool | false | Show in overview/status bar |
| `pltMin` | double | 0 | Plot Y-axis minimum (0 = auto-scale) |
| `pltMax` | double | 0 | Plot Y-axis maximum (0 = auto-scale) |
| `wgtMin` | double | 0 | Widget (bar/gauge) minimum |
| `wgtMax` | double | 100 | Widget (bar/gauge) maximum |
| `ledHigh` | double | 80 | LED activation threshold |
| `alarmEnabled` | bool | false | Enable alarm thresholds on bar/gauge |
| `alarmLow` | double | 20 | Low alarm threshold |
| `alarmHigh` | double | 80 | High alarm threshold |
| `fftSamples` | int | 256 | FFT window size (power of 2, 8-16384) |
| `fftSamplingRate` | int | 100 | FFT sampling rate in Hz |
| `fftMin` | double | 0 | FFT frequency axis minimum |
| `fftMax` | double | 0 | FFT frequency axis maximum |
| `xAxisId` | int | -1 | Reference dataset for custom X-axis (-1 = use time) |

## Dashboard Layout

- Widgets appear as mini-windows on the dashboard canvas
- Drag title bars to reposition
- Resize from edges and corners
- Minimize individual widgets to the taskbar
- Maximize to fill the canvas area
- Right-click canvas for context menu: tile windows, set wallpaper
- Widget positions and sizes are saved per-project via `widgetSettings` and persist across sessions
- Actions panel (if the project defines actions) appears as a horizontal bar above the widgets
- Dashboard render order follows the `DashboardWidget` enum: Terminal, DataGrid, MultiPlot, Accelerometer, Gyroscope, GPS, Plot3D, FFT, LED, Plot, Bar, Gauge, Compass, ImageView

## Selecting the Right Widget

| Data Type | Recommended Widget |
|-----------|--------------------|
| Temperature, Pressure, Voltage | Plot, Gauge, or Bar |
| GPS Coordinates | GPS Map (group) |
| Acceleration (X, Y, Z) | Accelerometer (group) |
| Rotation (X, Y, Z) | Gyroscope (group) |
| Heading/Bearing | Compass |
| Audio/Vibration frequency | FFT Plot |
| Boolean/status flags | LED Panel (group) |
| Mixed numeric and text values | Data Grid (group) |
| 3D trajectory or position | 3D Plot (group, Pro) |
| Live camera or image stream | Image View (group, Pro) |

## See Also

- [Project Editor](Project-Editor.md) — How to configure widgets in your project
- [Operation Modes](Operation-Modes.md) — Dashboard modes and frame detection
- [Data Flow](Data-Flow.md) — How data reaches widgets from drivers through the pipeline
