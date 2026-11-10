# Widget reference

## Overview

Your board prints `21.5,48` once a second. In the [Project Editor](Project-Editor.md) each value becomes a **dataset**, and datasets live in a **group**. A dataset shows one value and a group shows several together. To give the first dataset a gauge, select it in the Project Editor and set its **Widget** to Gauge, with a range. The project file stores that as the JSON below. You never have to edit it by hand, and the keys on this page are project-file keys.

```json
{ "title": "Temperature", "units": "C", "widget": "gauge", "widgetMin": 0, "widgetMax": 100 }
```

The gauge sweeps to 21.5 on a 0 to 100 dial, with C beneath it.

Widgets come in three kinds:

- **Group widgets** display the datasets of one group, such as a GPS map or a data grid. Choose the widget type when you add the group with **Add Group**.
- **Dataset widgets** display a single value: Plot, FFT Plot, Waterfall (Pro), Bar, Gauge, Compass, Meter. Click **Plot**, **FFT Plot** or **Waterfall** on the dataset toolbar in the Project Editor. Bar, Gauge, Compass and Meter are chosen in the dataset's **Widget** setting.
- **Utility widgets** (Clock, Stopwatch, Terminal) belong to no group. Turn them on from the dashboard taskbar or the Start menu.

New to Serial Studio? [Getting Started](Getting-Started.md) takes you from a first connection to a first dashboard.

## Picking the right widget

For a single number from your Arduino, start with [Plot](#plot); add a [Gauge](#gauge) once you know the range. Group widgets need several datasets in one group.

| Data type                                                                     | Recommended widget         |
|-------------------------------------------------------------------------------|----------------------------|
| Temperature                                                                   | [Gauge](#gauge), [Plot](#plot), or [Bar](#bar) |
| Pressure, voltage, signal level                                               | Plot, Gauge, [Meter](#meter), or Bar |
| Several related signals overlaid on one chart                                 | [Multi-Plot](#multi-plot) (group) |
| GPS coordinates                                                               | [GPS Map](#gps-map) (group) |
| Acceleration (X, Y, Z)                                                        | [Accelerometer](#accelerometer) (group) |
| Orientation angles (yaw, pitch, roll); for angular rate (deg/s) see Gyroscope | [Gyroscope](#gyroscope) (group) |
| Heading or bearing                                                            | [Compass](#compass)        |
| Audio or vibration frequency                                                  | [FFT Plot](#fft-plot)      |
| Time-frequency / spectrogram                                                  | [Waterfall](#waterfall-pro) (Pro) |
| Boolean status flags                                                          | [LED Panel](#led-panel) (group) |
| Many related channels, color by severity                                      | [Bar Panel](#bar-panel) (group) |
| Mixed numeric and text values                                                 | [Data Grid](#data-grid) (group) |
| 3D trajectory or position                                                     | [3D Plot](#3d-plot-pro) (group, Pro) |
| Live camera or image stream                                                   | [Image View](#image-view-pro) (group, Pro) |
| Custom instrument mimic or layout no widget covers                            | [Canvas](#canvas-pro) (group, Pro) |
| Embedded web page / map / 3D model                                            | [Web View](#web-view) (group) |
| Wall-clock time / session timestamp                                           | [Clock](#clock) (utility)  |
| Manual elapsed-time / lap timing                                              | [Stopwatch](#stopwatch) (utility) |
| Raw text stream, typing to the device                                         | [Terminal](#terminal) (utility) |

## Group widgets

### Data Grid

- Widget key: `"datagrid"`.
- Displays all datasets in a tabular format with titles, values, and units.
- Best for: overview of multiple channels, status monitoring.
- Configuration: none required beyond adding datasets to the group.
- Pause freezes the grid. Resume shows the current values immediately and discards what arrived while paused.

### Bar Panel

- Widget key: `"barpanel"`.
- One labeled bar per dataset in the group: title, alarm-band-zoned track, live value.
- Alarm bands are always drawn in full as muted zones on each track; the fill takes the active band's color (green while normal, yellow/red as the value crosses band edges).
- **Bar Style** (group key `barPanelStyle`: empty = Auto, `horizontal`, or `vertical`) in the Project Editor: Auto (default, picks by panel shape), Horizontal (labeled rows), or Vertical (rake-style columns).
- Datasets with **Hold Min/Max Markers** enabled show tick marks at the lowest and highest values observed since the last data reset.
- Best for: monitoring many related channels at a glance (temperatures, pressures, EGT rakes) where color should mean severity and nothing else.
- Configuration: set `widgetMin`/`widgetMax` per dataset for the track range; define alarm bands for severity coloring.

### Multi-Plot

- Widget key: `"multiplot"`.
- Overlays multiple dataset curves on shared axes, with per-curve visibility toggles and an auto-scaling Y-axis.
- Best for: comparing related signals (for example a 3-axis accelerometer over time).
- Configuration: switch **Plot** on for each dataset in the group (project-file key `graph: true`).

### GPS Map

- Widget key: `"map"` (the Project Editor writes this for new GPS Map groups; `"gps"` is also accepted).
- Tile-based map that plots the trajectory path and shows latitude, longitude, and altitude. The view follows the latest position; zoom and pan with the mouse.
- Best for: vehicle tracking, drone telemetry, field measurements.
- Requires: datasets with latitude, longitude, and optionally altitude.
- Needs an internet connection for map tiles. Previously viewed areas are cached.
- Only the first base map style is free; the additional styles in the **Base Map** dropdown require Pro.

### Gyroscope

- Widget key: `"gyro"` (also accepts `"gyroscope"`).
- Attitude indicator showing yaw, pitch, roll.
- Best for: IMU visualization, drone orientation, robotics.
- Requires: exactly 3 datasets, in group order yaw, pitch, roll.
- Expects absolute angles in degrees. If your sensor only provides angular rates (deg/s), integrate them with the **Integrate Rate to Angle** transform template.

### Accelerometer

- Widget key: `"accelerometer"`.
- 3-axis acceleration visualization with a computed total G-force: pitch, roll, peak G and magnitude, over a configurable max G range.
- Best for: vibration analysis, impact detection, motion sensing.
- Requires: exactly 3 datasets, in group order X, Y, Z acceleration.

### LED Panel

- Widget key: `"ledpanel"`.
- One multi-state indicator light per dataset in the group, with the dataset title beside it.
- With alarm bands defined, each light takes the color of the band the value sits in, shows the band's `label` next to the title, and flashes while a band marked `blink` is active. Outside every band, the light is off (annunciator-panel behavior).
- Without bands, the light falls back to a single threshold: on (dataset color) when the value meets or exceeds `ledHigh` (default 80).
- Every dataset in the group is a light, **Hide on Dashboard** included, exactly as every dataset in a Bar Panel group is a row. To show a value both in a Data Grid and as a light, add it to both groups; datasets may share a Frame Index.
- Best for: status flags, limit indicators, digital I/O states, annunciator panels.
- Configuration: set the group widget to **LED Panel**, then either define `alarmBands` (see [Alarm bands](#alarm-bands)) or set `ledHigh` on each dataset.
- Projects saved by an earlier version flagged individual datasets with `led: true` instead. On load, each group that had lights gains a `<Group> LEDs` group right after it holding copies of those datasets, saved workspaces keep pointing at the panel, and the project is marked modified so you can save the new form. A flagged dataset that was also hidden showed no light and is not copied.

### 3D Plot (Pro)

- Widget key: `"plot3d"`.
- 3D scatter and trajectory visualization.
- Orbit or free-camera navigation.
- Optional anaglyph stereo (red/cyan 3D glasses).
- Interpolation support.
- Best for: 3D position tracking, spatial data, point clouds.
- Recommended: 3 datasets tagged x/y/z; any axis without a tagged dataset renders at 0.
- Renders through Serial Studio's custom QPainter-based 3D pipeline. No GPU or OpenGL driver required. Runs on low-end hardware including Raspberry Pi.
- Pro. The first use offers a free trial (internet needed); see [Pro vs Free](Pro-vs-Free.md).

### Image View (Pro)

- Widget key: `"image"`.
- Live JPEG/PNG/BMP/WebP image streaming from the device.
- Autodetect or manual frame delimiter configuration.
- Independent frame reader per widget. Image frames and CSV telemetry coexist in the same byte stream.
- Export, zoom, and image filter toolbar controls.
- Best for: camera feeds, thermal imaging, visual inspection.
- Group-level configuration fields:
  - `imgDetectionMode`: `"autodetect"` (default, uses format magic bytes) or `"manual"` (user-defined delimiters).
  - `imgStartSequence`: hex start delimiter (manual mode only).
  - `imgEndSequence`: hex end delimiter (manual mode only).
- No datasets required inside the group. The widget reads raw image bytes directly from the transport stream.
- Pro. The first use offers a free trial (internet needed); see [Pro vs Free](Pro-vs-Free.md).

### Canvas (Pro)

- Widget key: `"painter"`.
- User-scripted dashboard widget. The script defines a JavaScript `paint(ctx, w, h)` callback (and an optional `onFrame()` callback) that renders directly into the widget's bitmap on every dashboard tick.
- Eighteen templates ship, one of them blank. Examples: Oscilloscope, Dial gauge, Polar plot, Radar sweep, Artificial horizon and Heatmap; the full list is under [Built-in templates](Canvas-Widget.md#built-in-templates).
- The script reads the group's datasets through a `datasets` global and dashboard tick metadata through `frame.number` / `frame.timestampMs`.
- Best for visualizations not covered by any built-in widget: instrument mimics, project-specific layouts, lab-equipment-style readouts.
- Repaints at the dashboard refresh rate (60 Hz by default, configurable 1-240 Hz). A 250 ms watchdog terminates the script if a single call does not return.
- See the [Canvas Widget](Canvas-Widget.md) reference for the full API.
- Pro. The first use offers a free trial (internet needed); see [Pro vs Free](Pro-vs-Free.md).

### Web View

- Widget key: `"webview"`.
- Embeds a web page inside a dashboard group, backed by Qt WebEngine (Chromium).
- The only configuration is a URL, set in the group's settings in the Project Editor.
- Group-level configuration fields:
  - `webViewUrl`: the address to load.
- No datasets required inside the group.
- Best for: embedding a live map, a 3D model viewer, a hardware vendor's web dashboard, or any HTML content alongside your telemetry. The ISS Tracker example uses it to show a NASA glTF model of the station.
- Requires a build compiled with Qt WebEngine. On builds without it, the widget shows a "Web View Unavailable" placeholder instead of failing to load the project.

## Dataset widgets

Bar, Gauge, Compass and Meter each have two pages; see their sections.

### Plot

- One per dataset with **Plot** switched on in the Project Editor (project-file key `graph: true`).
- Single-curve 2D time-series line chart with an auto-scaling Y-axis, or a fixed range with `plotMin` and `plotMax`.
- Optional custom X-axis from another dataset (`xAxis`), which enables XY/scatter plots.
- Pause stops appending samples and keeps the history you already have.
- Best for: individual signal monitoring, trend analysis.

### FFT Plot

- One per dataset with **FFT Plot** switched on in the Project Editor (`fft: true`).
- Real-time frequency spectrum analysis via Fast Fourier Transform (KissFFT).
- Configurable FFT window size: 8 to 262144 samples (powers of 2). Default 256.
- Selectable window function applied before the transform to reduce spectral leakage (15 options, from Rectangular to Flat Top). Default Blackman-Harris. The same window is used by the Waterfall widget.
- Configurable sampling rate determines the frequency axis (default 100 Hz).
- Configurable frequency range via `fftMin` and `fftMax`.
- [Frequency markers](#frequency-markers) name frequencies or ranges on the plot and show their live level.
- Best for: vibration frequency analysis, audio spectrum, signal quality.
- Configuration fields: `fftSamples` (window size), `fftWindow` (window function), `fftSamplingRate` (Hz), `fftMin`, `fftMax`.

### Waterfall (Pro)

- One per dataset with **Waterfall** switched on in the Project Editor (`waterfall: true`).
- Scrolling time-frequency plot (spectrogram). Each row is one FFT magnitude spectrum, with the newest row drawn at the top and older rows scrolling down.
- Reuses the dataset's [FFT Plot](#fft-plot) settings (`fftSamples`, `fftWindow`, `fftSamplingRate`, `fftMin`, `fftMax`). The Waterfall caps the transform at 65536 samples; a larger `fftSamples` is clamped. Enable both `fft: true` and `waterfall: true` if you want the FFT plot alongside the waterfall.
- Magnitude is converted to dB. The dynamic range (`minDb` / `maxDb`) is adjustable from the widget toolbar.
- Built-in color maps, sequential: Viridis, Inferno, Magma, Plasma, Turbo, Jet, Hot, Cividis, Cubehelix, SDR Classic, Grayscale, Grayscale (Inverted). Diverging (mid-scale neutral, so an idle spectrogram reads as the low-end color rather than black): Spectral, Red/Blue, Coolwarm.
- Mouse wheel to zoom, drag to pan, hover for a frequency/time readout. Reset view from the toolbar.
- Configurable history depth: 16 to 4096 stored rows.
- Draws the same [frequency markers](#frequency-markers) as the FFT Plot.
- **Y-axis source.** Defaults to elapsed time. Set `waterfallYAxis` to another dataset's `uniqueId` to drive the Y axis from that dataset's value instead. This is typically used for order-tracking plots (for example RPM vs. frequency).
- Best for: vibration order tracking, audio spectrograms, RF band monitoring, transient frequency events.
- Configuration fields: `waterfall: true`, `waterfallYAxis` (0 = time; otherwise the `uniqueId` of the dataset to use as the Y axis), plus the FFT fields listed in this section.
- Pro. The first use offers a free trial (internet needed); see [Pro vs Free](Pro-vs-Free.md).

### Bar

- Widget key: `"bar"`.
- Horizontal bar gauge with min/max range, current value and units, and color-banded alarm zones with per-band severity (Info / OK / Warning / Critical).
- Two pages: analog display and large digital readout. Swipe horizontally or use the page dots at the bottom; each tile remembers its page in the project file.
- Best for: bounded values such as a tank level or a battery percentage.
- Configuration fields (project-file keys): `widgetMin` (default 0), `widgetMax` (default 0), `alarmBands` (array; see [Alarm bands](#alarm-bands)). Swapped min and max values are put in order.

### Gauge

- Widget key: `"gauge"`.
- Circular or arc gauge display with colored outer-rim arc segments for each alarm band.
- Two pages: analog display and large digital readout; swipe or use the page dots.
- Configuration fields (project-file keys): `widgetMin` (default 0), `widgetMax` (default 0), `alarmBands`.

### Compass

- Widget key: `"compass"`.
- Heading indicator (0 to 360 degrees) that converts a numeric heading to a cardinal direction (N, NE, E, SE, S, SW, W, NW).
- Two pages: analog display and large digital readout, which adds the cardinal direction; swipe or use the page dots.
- Configuration: none beyond the value itself (0 to 360).

### Meter

- Widget key: `"meter"`.
- Analog half-arc meter with a sweeping needle, tick marks, colored arc bands, and value readout.
- Two pages: analog display and large digital readout; swipe or use the page dots.
- Configuration fields (project-file keys): `widgetMin` (default 0), `widgetMax` (default 0), `alarmBands`.

## Alarm bands

An alarm band is a value range with a color and a severity. This gauge draws an 80 to 100 Warning zone on the dial and raises a Warning when the value gets there:

```json
{ "title": "Coolant", "widget": "gauge", "widgetMin": 0, "widgetMax": 120,
  "alarmBands": [ { "min": 80, "max": 100, "severity": 2, "label": "Hot" } ] }
```

Edit bands with the **Alarm Bands** button in the dataset toolbar (next to **Transform**). It opens a dialog with presets (Tachometer, Speedometer, Engine Temperature, Pressure, Battery Voltage, Fuel Level, Signal Strength, CPU / System Load, OK / Warning / Critical, Indicator (On / Off), Fault Indicator), a per-band color picker, severity selector, blink toggle, and a live preview strip.

Bar, Gauge, Meter, Bar Panel, and LED Panel widgets render alarm bands. Bands are always drawn in full: muted zones along the bar track (Bar / Bar Panel) or arc segments on the dial rim (Gauge / Meter). The fill or needle takes the active band's color, so a value in the normal range reads as a green signal. A value outside every band takes the nearest band's severity, so overrange data renders as critical rather than unclassified. LED Panel entries light in the active band's color. The tachometer convention (white below normal, green operating range, yellow caution, red redline) is one setup; any combination of ranges and colors is allowed.

Bands take effect only on a dataset that one of these widgets shows: a dataset with a Bar, Gauge, or Meter widget, or any dataset in a Bar Panel or LED Panel group. **Hide on Dashboard** removes the first case, because a hidden dataset gets no widget of its own; Bar Panel rows and LED Panel lights are unaffected. On any other dataset (a plot-only dataset, for example) the bands are kept in the project file but are not drawn, post no notification, and raise no [alarm point](Aural-Alerts.md). Assigning one of the widgets again brings them back.

Three dataset options in the Widget Settings control when bands alert:

| Option (project-file key)                      | Effect | Default |
|------------------------------------------------|--------|---------|
| **Suppress Alarms** (`suppressAlarms`)         | Bands stay drawn, but the dataset posts no notification, raises no alarm point and plays no sound. A redline that should be seen but not heard is the usual case. | Off |
| **Alarms only while visible** (`alarmsWhileVisible`) | The dataset alerts only while a widget showing it is on screen. **Suppress Alarms** wins when both are on. | Off |
| **Alarm Deadband** (`alarmDeadband`)           | Hysteresis in the dataset's units: an active band holds until the value leaves it by more than the deadband. | 0 |

**Alarms only while visible.** The bands stay drawn and the dataset keeps recording, but a notification, an alarm point and a sound happen only while a widget showing the dataset is on screen. A widget showing the dataset counts as on screen when all of these hold:

- It sits on the active tab of the main dashboard (Dashboard pane shown) or of an external dashboard window.
- The window is not minimized.
- The widget itself is neither minimized nor closed.

An open pop-out counts, and so does a frozen dashboard. Revealing the dataset while its value already sits in a Warning or Critical band counts as entering the band, so the point and the notification arrive at once. Hiding it removes the point, and nothing is recorded for the time it was hidden. One project can carry several device profiles on their own workspaces this way: only the profile the operator is looking at alerts.

**Alarm Deadband.** A value that hovers on a band edge would enter and leave the band on every frame, raising a point each time. The hold only ever delays a step down: a move into a more severe band takes effect immediately, so a deadband never postpones a Critical. A temperature with a Warning band at 80 and a deadband of 1 alarms at 80.0 and clears at 78.9, not at 79.9.

**Band schema.** Under the dataset's `alarmBands` array, each entry is an object:

| Field      | Type   | Required | Notes |
|------------|--------|----------|-------|
| `min`      | double | yes      | Lower bound of the band (inclusive). |
| `max`      | double | yes      | Upper bound of the band (inclusive: a value is in the band when `min <= value <= max`). |
| `severity` | int    | yes      | `0` = Info, `1` = OK, `2` = Warning, `3` = Critical. Drives the default color. Only `2` and `3` raise a notification on entry. |
| `color`    | string | no       | Hex override (`"#rrggbb"`). When empty, the severity's theme color is used (theme switches re-tint live). |
| `label`    | string | no       | Optional human-readable name. Surfaces in the band-edge notification subtitle and next to the dataset title on LED panels. |
| `blink`    | bool   | no       | When `true`, LED panels flash the LED while the value sits in this band. Defaults to `false`. |
| `sound`    | string | no       | WAV file played instead of the priority's default when the value enters this band (see [Aural Alerts](Aural-Alerts.md)). Relative paths resolve against the project file's folder. Omitted when empty. |

Severity is a separate scale from [notification](Notifications.md) `level` (`0` = Info, `1` = Warning, `2` = Critical): band severity 2 posts level `Warning` (1) and severity 3 posts level `Critical` (2).

Bands may have gaps, may overlap, and need not cover the full range. Overlaps resolve to the FIRST matching band in list order, not the last. A gap only leaves the painted track/background uncolored: the bar fill or needle itself still takes a band's color, clamped to whichever band is nearest.
**Notifications and aural alerts.** Entering a Warning or Critical band also raises an alarm point on the master annunciator, the bell in the taskbar tray (see [Aural Alerts](Aural-Alerts.md)); leaving it returns the point to normal. When the value enters a band with severity ≥ Warning, Serial Studio posts a notification on the "Alarms" channel, with the band's `label` in the subtitle. Alarm tracking runs per dataset at the dashboard level, so notifications fire even when the widget displaying the dataset is hidden or not instantiated. The 3 second cooldown is tracked per dataset and per severity tier: a dataset's Warning is suppressed by its own Warning or Critical within 3 seconds, a Critical only by its own Critical, so escalating from Warning to Critical always posts.
**Legacy compatibility.** Project files written by older Serial Studio releases carry `alarmEnabled` / `alarmLow` / `alarmHigh` instead of `alarmBands`. On load, those are converted to two `Warning`-severity bands (`[widgetMin..alarmLow]` and `[alarmHigh..widgetMax]`). The legacy keys are not written back; re-saved projects carry only `alarmBands`. For canvas scripts (Pro), `dataset.alarmLow` and `dataset.alarmHigh` remain readable as derived values (first / last `Warning+` band edges) so existing scripts keep working.

## Frequency markers

A frequency marker names a frequency or a range on a dataset's FFT Plot and Waterfall and shows the live level there, which keeps a known line in view: mains hum, a resonance, the band where a bearing defect appears. Markers are free on the FFT Plot. The Waterfall is Pro (see [Pro vs Free](Pro-vs-Free.md)).

In the [Project Editor](Project-Editor.md), select a dataset with FFT or Waterfall enabled and click **Freq. Markers** on the dataset toolbar. The **Frequency Markers** dialog holds one row per marker, and a strip at the bottom previews them from 0 Hz to the top of the range.

| Column | Meaning |
|--------|---------|
| **Start (Hz)** | Frequency of a point marker, or the low edge of a band. A row left at 0 is dropped on **Apply**. |
| **End (Hz)** | High edge of a band. Left empty, or not above **Start (Hz)**, the row is a point marker. |
| **Warn (dB)**, **Alarm (dB)** | Levels from -200 to 100 at which the marker changes color. Left empty, the level is off. |
| **Color** | Click to choose a color; right-click to return to the automatic color. |
| **Label** | Name shown on the marker. Without one, the marker shows its frequency. |

Frequencies stop at half the FFT sampling rate, shown as **Frequency range** at the top of the dialog. **Preset** replaces the table with a watchlist and skips any entry above that range: **Mains Hum (50 Hz + harmonics)**, **Mains Hum (60 Hz + harmonics)**, or **Octave Bands (31.5 Hz - 16 kHz)**.

On the dashboard a point marker is a vertical line and a band is a shaded region. Each marker carries a chip with its name and its peak level in dB, measured on the spectrum as displayed: inside the band, or within two FFT bins either side of a point marker. On the FFT Plot, clicking a chip highlights that marker and dims the others. **Show Frequency Markers** on the widget toolbar hides or shows them all; the FFT Plot offers this button only when the dataset has markers.

**The levels only change the color.** A peak at or above **Warn (dB)** draws the marker in the theme's Warning color, and at or above **Alarm (dB)** in its Critical color. A marker raises no alarm point, notification, or sound. For an alert you can hear, use [alarm bands](#alarm-bands) on a Bar, Gauge, or Meter.

In the project file, markers are the dataset's `fftMarkers` array. This dataset is sampled at 2000 Hz, so its range ends at 1000 Hz:

```json
"fftMarkers": [
  { "freq": 50, "label": "Mains", "color": "#ffb300" },
  { "freq": 120, "endFreq": 180, "label": "Bearing", "warningDb": -40, "alarmDb": -20 }
]
```

On load, an entry whose `freq` is not above 0 is dropped, an `endFreq` not above `freq` makes a point marker, and a `warningDb` above `alarmDb` swaps the two. The API commands are `project.dataset.getFFTMarkers` and `project.dataset.setFFTMarkers` in the [API Reference](API-Reference.md).

## Utility widgets

Terminal, Clock and Stopwatch are dashboard-level utility widgets, free in every build. They are not attached to any group or dataset. Toggle them with the **Console**, **Clock** and **Stopwatch** buttons on the dashboard taskbar, or from the **Start menu**. Each remembers between sessions whether it was on; all three start off.

### Terminal

- Shows the raw text stream from the device, with VT-100 emulation and ANSI colors.
- Keystrokes typed into it go to the connected device.
- The taskbar calls it **Console**.

### Clock

- Page 0 is an analog face, page 1 a digital readout (12-hour time and date). Driven by the system clock, ticks once per second.
- The active page is remembered between sessions.
- Best for: visible session timestamps in screen recordings, lab/operator dashboards.

### Stopwatch

- Single-page widget: large `HH:MM:SS.mmm` readout, **Start/Stop**, **Lap**, **Reset** buttons, and a scrollable lap table.
- Local-only timing; data is not persisted to the project, transmitted, or sent to the device.
- Best for: bench testing, run/test timing during a session.

## Widget configuration summary

| Widget        | Type    | Key             | Edition                   | Datasets | Key settings                                               |
|---------------|---------|-----------------|---------------------------|----------|------------------------------------------------------------|
| Data Grid     | Group   | `datagrid`      | Free                      | 1+       | (none)                                                     |
| Bar Panel     | Group   | `barpanel`      | Free                      | 1+       | `widgetMin`/`widgetMax`, `alarmBands[]`, `barPanelStyle`   |
| Multi-Plot    | Group   | `multiplot`     | Free                      | 1+       | `graph: true` on datasets                                  |
| GPS Map       | Group   | `map`           | Free; extra base maps Pro | 2-3      | lat, lon, (alt) datasets                                   |
| Gyroscope     | Group   | `gyro`          | Free                      | 3        | yaw, pitch, roll                                           |
| Accelerometer | Group   | `accelerometer` | Free                      | 3        | x, y, z accel                                              |
| LED Panel     | Group   | `ledpanel`      | Free                      | 1+       | `alarmBands[]` or `ledHigh` per dataset                    |
| 3D Plot       | Group   | `plot3d`        | Pro                       | 0+       | x, y, z coords                                             |
| Image View    | Group   | `image`         | Pro                       | 0        | binary stream                                              |
| Canvas        | Group   | `painter`       | Pro                       | 0+       | user `paint(ctx, w, h)` JS script                          |
| Web View      | Group   | `webview`       | Free                      | 0        | `webViewUrl` (Qt WebEngine build)                          |
| Plot          | Dataset | flag `graph`    | Free                      | any      | `graph: true`, `plotMin`/`plotMax`                         |
| FFT Plot      | Dataset | flag `fft`      | Free                      | any      | `fft: true`, `fftSamples`, `fftSamplingRate`               |
| Waterfall     | Dataset | flag `waterfall` | Pro                       | any      | `waterfall: true`, FFT fields, `waterfallYAxis`            |
| Bar           | Dataset | `bar`           | Free                      | any      | `widgetMin`/`widgetMax`, `alarmBands[]`                    |
| Gauge         | Dataset | `gauge`         | Free                      | any      | `widgetMin`/`widgetMax`, `alarmBands[]`                    |
| Compass       | Dataset | `compass`       | Free                      | any      | value 0-360                                                |
| Meter         | Dataset | `meter`         | Free                      | any      | `widgetMin`/`widgetMax`, `alarmBands[]`                    |
| Terminal      | Utility | (toggle)        | Free                      | 0        | raw text stream with VT-100 emulation; taskbar **Console** |
| Clock         | Utility | (toggle)        | Free                      | 0        | system clock; analog face / digital readout                |
| Stopwatch     | Utility | (toggle)        | Free                      | 0        | Start/Stop/Lap/Reset with lap table                        |

## Dataset fields reference

Every dataset in a project file supports these visualization-related fields:

| Field              | Type   | Default | Description |
|--------------------|--------|---------|-------------|
| `index`            | int    | 0       | Frame offset index (column position in CSV data). |
| `title`            | string | (none)  | Human-readable display name. |
| `units`            | string | (none)  | Measurement units (for example "m/s", "degC"). |
| `widget`           | string | `""`    | Dataset widget type: `"bar"`, `"gauge"`, `"compass"`, or `"meter"`. |
| `graph`            | bool   | false   | Enable time-series plot. |
| `fft`              | bool   | false   | Enable FFT spectrum plot. |
| `waterfall`        | bool   | false   | Enable waterfall (spectrogram) plot. Pro. |
| `waterfallYAxis`   | int    | 0       | Waterfall Y-axis source: 0 = time, otherwise the `uniqueId` of another dataset (order tracking). |
| `log`              | bool   | false   | Enable logging to file. |
| `overviewDisplay`  | bool   | false   | Show in the overview/status bar. |
| `plotMin`          | double | 0       | Plot Y-axis minimum (0 = auto-scale). |
| `plotMax`          | double | 0       | Plot Y-axis maximum (0 = auto-scale). |
| `widgetMin`        | double | 0       | Widget (bar/gauge/meter) minimum. |
| `widgetMax`        | double | 0       | Widget (bar/gauge/meter) maximum. |
| `displayTickCount` | int    | 5       | Major-tick count on the dial scale (0 = auto-fit to widget size). Editor label "Tick Count". |
| `displayFormat`    | string | `"0d"`  | Decimal places on analog widget ticks and captions; Hexadecimal and Binary apply wherever the value is shown as text. Editor label "Label Format". |
| `decimalPoints`    | int    | -1      | Fixed decimal places for the value display; overrides `displayFormat` (-1 = auto). Editor label "Decimal Points". |
| `extremeHold`      | bool   | false   | Shows tick marks at the lowest and highest values observed since the last data reset. Editor label "Hold Min/Max Markers". |
| `ledHigh`          | double | 80      | Light-on threshold for a dataset in an LED Panel group (used only when `alarmBands` is empty). |
| `alarmBands`       | array  | `[]`    | Colored value bands for bar/gauge/meter widgets, LED panels, and bar panels; ignored on any other dataset. Each entry: `{min, max, severity, color?, label?, blink?}`; see [Alarm bands](#alarm-bands). Legacy `alarmEnabled` / `alarmLow` / `alarmHigh` keys from older releases are still read and migrated to bands on load, but no longer written. |
| `suppressAlarms`   | bool   | false   | Keeps the dataset's alarm bands drawn but stops its band notifications, alarm points, and sounds. Written only when true. Editor label "Suppress Alarms". |
| `alarmsWhileVisible` | bool | false   | The dataset's alarm bands notify, raise alarm points, and sound only while a widget showing it is on screen (see [Alarm bands](#alarm-bands)); `suppressAlarms` wins when both are set. Written only when true. Editor label "Alarms only while visible". |
| `alarmDeadband`    | double | 0       | Hysteresis in the dataset's units: the active alarm band holds until the value leaves it by more than this; escalation is never delayed. Must be >= 0; written only when positive. Editor label "Alarm Deadband". |
| `fftSamples`       | int    | 256     | FFT window size (power of 2, 8 to 262144). The Waterfall clamps it to 65536. |
| `fftWindow`        | int    | 5       | FFT window function, applied to both the FFT plot and the waterfall: 0 = Rectangular, 1 = Bartlett, 2 = Hann, 3 = Hamming, 4 = Blackman, 5 = Blackman-Harris, 6 = Nuttall, 7 = Blackman-Nuttall, 8 = Flat Top, 9 = Welch, 10 = Bartlett-Hann, 11 = Bohman, 12 = Cosine, 13 = Lanczos, 14 = Parzen. |
| `fftSamplingRate`  | int    | 100     | FFT sampling rate in Hz. |
| `fftMin`           | double | 0       | FFT frequency axis minimum. |
| `fftMax`           | double | 0       | FFT frequency axis maximum. |
| `fftMarkers`       | array  | none    | [Frequency markers](#frequency-markers) drawn on the FFT Plot and Waterfall. Written only when not empty. |
| `labels`           | array  | none    | Value-to-text table, entries `{"value": <integer>, "text": <string>}`. Editor button "Labels"; see the [Project Editor](Project-Editor.md). |
| `labelsDefault`    | string | ""      | Text shown for a value no `labels` entry lists; empty shows the number. Editor row "Any other". |
| `xAxis`            | int    | -2      | X-axis source: `-2` = time (default), `-1` = sample count (the **Samples** choice in the X-Axis Source combo), or the `uniqueId` of another dataset for an XY plot. |

## Dashboard layout

Widgets are windows on the dashboard canvas. Drag the title bar to move one, resize from edges and corners, minimize to the taskbar or maximize to fill the canvas. Right-click the canvas to tile windows or set a wallpaper. Positions and sizes are saved per project via `widgetSettings`. For the Actions panel, see [Actions](Actions.md).

Most widgets carry a small toolbar of icon buttons along their top edge that appears once the widget is large enough to fit it. Every button on every widget toolbar is listed in the [Toolbar & Button Reference](Toolbar-Reference.md#dashboard-widget-toolbars).

## See also

- [Project Editor](Project-Editor.md): how to configure widgets in your project.
- [Toolbar & Button Reference](Toolbar-Reference.md): every button on every widget toolbar and across the app.
- [Operation Modes](Operation-Modes.md): dashboard modes and frame detection.
- [Data Flow](Data-Flow.md): how data reaches widgets from drivers through the pipeline.
