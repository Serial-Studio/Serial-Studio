# CSV Import and Export

Serial Studio can export incoming telemetry data to CSV files during a live session and replay previously saved CSV files through the full data pipeline. This page covers both workflows, the file format, and the MDF4 alternative available in Pro.

## Export and Playback Pipeline

The following diagram shows how CSV export runs on a background thread during live data, and how CSV playback feeds recorded data back through the same pipeline.

```svg
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 720 560" font-family="monospace" font-size="13">
  <!-- Background -->
  <rect width="720" height="560" fill="#f8f9fa" rx="8"/>

  <!-- Title: EXPORT PATH -->
  <text x="200" y="25" text-anchor="middle" fill="#2d3748" font-weight="bold" font-size="14">Live Export</text>

  <!-- Device -->
  <rect x="100" y="40" width="200" height="36" rx="6" fill="#2d3748" stroke="#1a202c" stroke-width="1.5"/>
  <text x="200" y="63" text-anchor="middle" fill="#fff" font-weight="bold">Connected Device</text>

  <line x1="200" y1="76" x2="200" y2="106" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>
  <text x="215" y="96" fill="#718096" font-size="10">raw bytes</text>

  <!-- Frame Builder -->
  <rect x="100" y="106" width="200" height="36" rx="6" fill="#2b6cb0" stroke="#2c5282" stroke-width="1.5"/>
  <text x="200" y="129" text-anchor="middle" fill="#fff" font-weight="bold">Frame Builder</text>

  <!-- Branch: Main thread to Dashboard -->
  <line x1="200" y1="142" x2="200" y2="172" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <rect x="100" y="172" width="200" height="36" rx="6" fill="#6b46c1" stroke="#553c9a" stroke-width="1.5"/>
  <text x="200" y="195" text-anchor="middle" fill="#fff" font-weight="bold">Dashboard (Main Thread)</text>

  <!-- Branch: Export worker -->
  <line x1="300" y1="124" x2="360" y2="124" stroke="#ed8936" stroke-width="1.5"/>
  <line x1="360" y1="124" x2="360" y2="172" stroke="#ed8936" stroke-width="1.5" marker-end="url(#arr)"/>
  <text x="335" y="152" fill="#ed8936" font-size="10">lock-free</text>
  <text x="340" y="163" fill="#ed8936" font-size="10">enqueue</text>

  <!-- Export Worker box -->
  <rect x="310" y="172" width="100" height="100" rx="6" fill="#fff" stroke="#ed8936" stroke-width="1.5" stroke-dasharray="6,3"/>
  <text x="360" y="192" text-anchor="middle" fill="#c05621" font-weight="bold" font-size="11">Worker</text>
  <text x="360" y="210" text-anchor="middle" fill="#c05621" font-size="11">Thread</text>
  <line x1="320" y1="218" x2="400" y2="218" stroke="#e2e8f0" stroke-width="1"/>
  <text x="360" y="238" text-anchor="middle" fill="#2d3748" font-size="10">CSV</text>
  <text x="360" y="253" text-anchor="middle" fill="#2d3748" font-size="10">MDF4 [Pro]</text>
  <text x="360" y="268" text-anchor="middle" fill="#2d3748" font-size="10">API :7777</text>

  <!-- Arrow to disk -->
  <line x1="360" y1="272" x2="360" y2="310" stroke="#718096" stroke-width="1.5" marker-end="url(#arr)"/>

  <!-- Disk -->
  <rect x="300" y="310" width="120" height="36" rx="6" fill="#2f855a" stroke="#276749" stroke-width="1.5"/>
  <text x="360" y="333" text-anchor="middle" fill="#fff" font-weight="bold" font-size="11">CSV / MDF4 File</text>

  <!-- File path -->
  <rect x="60" y="240" width="220" height="48" rx="4" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="170" y="258" text-anchor="middle" fill="#2d3748" font-size="10">Documents/Serial Studio/CSV/</text>
  <text x="170" y="273" text-anchor="middle" fill="#718096" font-size="10">&lt;Project&gt;/&lt;Year&gt;/&lt;Month&gt;/&lt;Day&gt;/&lt;Time&gt;.csv</text>

  <!-- Separator -->
  <line x1="20" y1="376" x2="700" y2="376" stroke="#cbd5e0" stroke-width="1" stroke-dasharray="4,4"/>

  <!-- Title: PLAYBACK PATH -->
  <text x="360" y="400" text-anchor="middle" fill="#2d3748" font-weight="bold" font-size="14">CSV Playback</text>

  <!-- CSV File -->
  <rect x="60" y="420" width="120" height="36" rx="6" fill="#2f855a" stroke="#276749" stroke-width="1.5"/>
  <text x="120" y="443" text-anchor="middle" fill="#fff" font-weight="bold" font-size="11">CSV File</text>

  <line x1="180" y1="438" x2="230" y2="438" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <!-- CSV Player -->
  <rect x="230" y="420" width="140" height="36" rx="6" fill="#d69e2e" stroke="#b7791f" stroke-width="1.5"/>
  <text x="300" y="443" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">CSV Player</text>

  <line x1="370" y1="438" x2="420" y2="438" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>
  <text x="395" y="428" fill="#718096" font-size="10">timed</text>

  <!-- Frame Builder (playback) -->
  <rect x="420" y="420" width="140" height="36" rx="6" fill="#2b6cb0" stroke="#2c5282" stroke-width="1.5"/>
  <text x="490" y="443" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">Frame Builder</text>

  <line x1="560" y1="438" x2="600" y2="438" stroke="#718096" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Dashboard (playback) -->
  <rect x="600" y="420" width="100" height="36" rx="6" fill="#6b46c1" stroke="#553c9a" stroke-width="1.5"/>
  <text x="650" y="443" text-anchor="middle" fill="#fff" font-weight="bold" font-size="12">Dashboard</text>

  <!-- Player controls -->
  <rect x="230" y="470" width="340" height="26" rx="4" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="400" y="488" text-anchor="middle" fill="#718096" font-size="10">◀ Prev  |  ▶ Play/Pause  |  ▶▶ Next  |  ━━━━━━━ Seek Bar</text>

  <!-- Legend -->
  <rect x="40" y="515" width="640" height="28" rx="4" fill="#edf2f7" stroke="#cbd5e0" stroke-width="1"/>
  <text x="360" y="534" text-anchor="middle" fill="#718096" font-size="10">Export: 8192 queue capacity  •  1024 frame flush threshold  •  1 s timer  •  zero dashboard impact</text>

  <!-- Arrow marker -->
  <defs>
    <marker id="arr" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
      <polygon points="0 0, 10 3.5, 0 7" fill="#718096"/>
    </marker>
  </defs>
</svg>
```

---

## CSV Export

### Enabling Export

CSV export is toggled in the **Setup Panel** of the main window. Enable the **CSV Export** switch before or during a live connection. Once enabled, Serial Studio writes every incoming frame to a CSV file on a background thread, so dashboard performance is unaffected.

### File Location

Exported CSV files are saved under the user's Documents directory in a structured hierarchy:

```
Documents/Serial Studio/CSV/<Project Name>/<Year>/<Month>/<Day>/<Time>.csv
```

For example, a session started at 3:30:05 PM on March 17, 2026, for a project named "Weather Station" would produce:

```
Documents/Serial Studio/CSV/Weather Station/2026/03/17/15-30-05.csv
```

### File Format

The CSV file contains a header row followed by one row per received frame.

**Header row:**

```
RX Date/Time,GroupName/DatasetName,GroupName/DatasetName,...
```

The first column is the elapsed time in seconds since the session started, with nanosecond precision (e.g., `0.000000000`, `0.016384512`). The remaining columns correspond to each dataset defined in the project, ordered by their unique ID. The header labels are formed as `GroupName/DatasetName` so you can trace each column back to the project structure.

**Data rows:**

Each row represents one complete frame. Cells contain the numeric or string values of each dataset at that point in time.

### File Lifecycle

- The file is created on the first frame received after export is enabled.
- The file auto-closes when the device disconnects or when export is disabled.
- If you disconnect and reconnect during the same session, a new file is created with a new timestamp.

### Background Thread

The `CSV::ExportWorker` runs on a dedicated background thread. Frames are enqueued from the main thread in a lock-free manner and written to disk in batches. This design ensures that disk I/O latency never blocks the dashboard or data pipeline.

---

## CSV Playback

### Opening a CSV File

To replay a previously recorded CSV file:

1. Click the **Open CSV** button in the toolbar (or use the keyboard shortcut Ctrl+O / Cmd+O).
2. Select the CSV file in the file dialog.
3. The CSV Player dialog appears.

### Timestamp Handling

When a CSV file is opened, Serial Studio examines the first column to determine the timestamp format:

- **Decimal seconds** (e.g., `0.0`, `1.5`, `3.016`): Used directly as elapsed time. This is the format Serial Studio's own export produces.
- **Date/time strings** (e.g., `2026-03-17 15:30:05`): Parsed into absolute timestamps and converted to elapsed offsets.
- **No recognizable format**: Serial Studio prompts you to specify a fixed interval between rows (in milliseconds). This is useful for CSV files generated by other tools that lack timestamps.

### Player Controls

The CSV Player provides the following controls:

| Control | Action | Shortcut |
|---------|--------|----------|
| Play / Pause | Start or pause playback | Space |
| Previous Frame | Step back one frame | Left Arrow |
| Next Frame | Step forward one frame | Right Arrow |
| Progress Slider | Seek to any position in the file | Drag or click |

The current timestamp is displayed next to the slider, formatted as `HH:MM:SS.mmm`.

### How Playback Works

During playback, the CSV Player feeds each row through the same data pipeline used for live connections: Frame Builder, then Dashboard. This means all widgets, plots, and gauges render exactly as they would with a live device. The player respects the original timing between frames, so playback speed matches the original recording rate.

### Multi-Source CSV Files

For projects with multiple data sources, the CSV Player maps columns back to their respective source IDs. Each column is associated with a source based on the dataset's unique ID recorded in the header. The player reconstructs per-source frames and injects them into the pipeline at the correct source routing.

### Speed Control

Playback runs at real-time speed by default. The player uses the timestamp differences between consecutive rows to schedule frame delivery, preserving the original data rate.

---

## MDF4 Export and Playback (Pro)

Serial Studio Pro can also export and replay MDF4 (Measurement Data Format version 4) files, an ASAM standard for storing measurement data in a compact binary format.

### When to Use MDF4 vs. CSV

| Aspect | CSV | MDF4 (Pro) |
|--------|-----|------------|
| File size | Larger (text-based) | Smaller (binary, compressed) |
| Write speed | Adequate for most rates | Better for high-frequency data |
| Compatibility | Universal (Excel, Python, MATLAB, R) | Specialized (CANape, DIAdem, asammdf) |
| Metadata | Column headers only | Rich: channel names, units, conversions |
| Best for | General analysis, sharing | Automotive, industrial, high-rate logging |

### Analyzing Exported Data

**CSV files:**

- Excel / LibreOffice Calc: Open directly.
- Python: `import pandas; df = pandas.read_csv('file.csv')`
- MATLAB: `data = readtable('file.csv');`
- R: `data <- read.csv('file.csv')`

**MDF4 files (Pro):**

- Vector CANape: Professional automotive analysis.
- NI DIAdem: Industrial data management.
- MATLAB: Vehicle Network Toolbox.
- Python: `from asammdf import MDF; mdf = MDF('file.mf4')`

---

## See Also

- [Getting Started](Getting-Started.md) — Initial setup and first connection
- [Operation Modes](Operation-Modes.md) — Quick Plot vs. Project File mode
- [Project Editor](Project-Editor.md) — Define datasets and dashboard layout
- [Data Flow](Data-Flow.md) — How data moves through the pipeline
- [Pro vs Free Features](Pro-vs-Free.md) — MDF4 export is a Pro feature
