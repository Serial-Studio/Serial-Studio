# CSV export and playback

Your board prints values with `Serial.println` and you want them in a spreadsheet, or you logged a bench test last week and want it back on the dashboard. Serial Studio writes incoming frames to CSV and replays CSV files through the same pipeline as a live device. CSV export and playback are in both the free edition and Pro. For high-rate binary logging with per-channel sample rates and metadata, see [MDF4 Export & Playback](MDF4.md) (Pro).

## Quickstart

1. Pick Quick Plot as the operation mode ([Operation Modes](Operation-Modes.md)).
2. In the Setup panel, under **Data Export**, turn on **CSV Spreadsheet**.
3. Connect, and let a few lines arrive.
4. Open the folder shown in [File location](#file-location). To replay the file, follow [Opening a CSV file](#opening-a-csv-file).

## CSV export

### Turning export on

A frame is one line your board prints, for example `21.4,48`. CSV export is a switch in the Setup panel of the main window. Under **Data Export**, turn on **CSV Spreadsheet**, before or during a live connection. Serial Studio then writes every incoming frame to a CSV file on a background thread. The setting is remembered across launches.

The writer queues up to 8192 frames and flushes at 1024 rows or every second, so the display thread never blocks on disk.

The switch is greyed out in Console Only mode, which has no frame structure to export, and switching to Console Only turns export off. Pick Quick Plot or Project File first ([Operation Modes](Operation-Modes.md)).

In Quick Plot mode each channel is exported twice, because it feeds both the data grid and the plot. Three channels give six value columns.

### File location

Exported files land under your workspace folder (Documents/Serial Studio by default; configurable in Settings):

```text
<Workspace>/CSV/<Project Name>/<YYYY-MM-DD_HH-MM-SS>.csv
```

A session started at 3:30:05 PM on March 17, 2026 for a project named "Weather Station" writes:

```text
Documents/Serial Studio/CSV/Weather Station/2026-03-17_15-30-05.csv
```

A project with no title exports to a folder named `Untitled`.

### File format

A two-channel project produces a file like this:

```text
Elapsed (s),Weather Station/Temp/Celsius,Weather Station/Temp/Humidity
0.000000000,21.4,48
0.016384512,21.5,48
```

The header row is `Elapsed (s)` followed by `GroupName/DatasetName` for each dataset, in dataset order. `Elapsed (s)` is seconds since the session started, with nanosecond precision. A `SourceName/` prefix is added whenever the owning source has a non-empty title, including in a single-source project. In the sample, `Weather Station` is the source title, `Temp` the group and `Celsius` the dataset.

The file opens with a UTF-8 byte-order mark. Most tools handle it; pandas needs `encoding="utf-8-sig"` or the first header cell keeps a BOM character. Any cell starting with `=`, `+`, `-`, `@`, a tab or a carriage return gets an apostrophe prefix (unless it is purely numeric), so spreadsheets do not evaluate it as a formula.

For a single-source project, each row is one complete frame. If your project has several devices, rows are sparse: one row per distinct sample instant, with a source's columns left empty where that source produced no sample. Rows are merged inside a 250 ms reorder window, so a file you tail during recording lags the live data by up to 250 ms; closing the file flushes the rest.

### Row interval

**Row Interval (ms)** is in [Preferences](Preferences.md) (the **Preferences** button on the main toolbar), **Export** tab, under CSV Export. The field accepts any integer of 0 or more, with presets 0, 10, 100 and 1000. The value is remembered across launches.

- `0` (default): one row per received frame.
- `N` above 0: one row every N milliseconds, each a snapshot of the latest value of every dataset. It applies live, so a change takes effect on the next snapshot.

A 10 ms interval on a sensor that updates once a second writes 100 near-identical rows for every distinct value.

### File lifecycle

- Export opens the file on the first frame received after export is turned on.
- Disconnecting, turning export off, or pausing the dashboard closes the file.
- Reconnecting, resuming or re-enabling export opens a new file with a new timestamp.

Names have one-second resolution. If the name exists, Serial Studio appends `_2`, `_3` and so on, up to 64 tries, and never overwrites a file.

## CSV playback

### Opening a CSV file

1. Only a multi-source file needs its project loaded first (see [Multi-source CSV files](#multi-source-csv-files)). For a single-device file, skip to step 2.
2. Click **Open CSV** in the toolbar, or press Ctrl+O (Cmd+O on macOS). Both are disabled while a CSV file is already open or a device is connected. Dropping a file on the window works too.
3. Pick a `*.csv` file.

The CSV Player dialog opens and the dashboard shows the first row. Press Space to play.

Opening a file while a device is connected shows **Device Connection Active**; disconnect first. A file with no data row is rejected with **Insufficient Data in CSV File**. Playback indexes up to 2,000,000,000 rows, then shows **CSV Row Limit Reached**.

### Timestamp handling

Files exported by Serial Studio need no prompt. For any other CSV, Serial Studio reads the first column to work out the timestamp format, and a file from a spreadsheet or another tool usually lands in one of the prompt cases.

| First column | What happens |
|--------------|--------------|
| Numeric, unit in the header | A bracketed unit (`time(ms)`, `t [us]`) or a `_unit` suffix (`time_ms`) sets the scale: seconds, `ms`, `us` or `ns`, by name or common abbreviation. Serial Studio's own `Elapsed (s)` is this case. |
| Numeric, no recognizable unit | A **Timestamp Units** prompt offers Seconds (s), Milliseconds (ms) and Microseconds (us); seconds is preselected. It offers no nanoseconds, so put `ns` in the header instead. |
| Date/time strings, such as `2026/03/17 15:30:05` | Parsed into absolute timestamps, then converted to elapsed offsets. |
| Anything else | A **CSV Date/Time Selection** prompt offers **Select a date/time column** or **Set interval manually** (a fixed gap of 1 to 1,000,000 ms between rows, 1000 preselected), for tools that record no timestamps. The date/time option opens a **Select Date/Time Column** picker, and the chosen column is parsed as in the date/time row. |

### Player controls

| Control          | Action                           | Shortcut       |
|------------------|----------------------------------|----------------|
| Play / Pause     | Start or pause playback          | Space, media Play/Pause |
| Previous frame   | Step back one frame              | Left or Down Arrow, media Previous |
| Next frame       | Step forward one frame           | Right or Up Arrow, media Next |
| Progress slider  | Seek to any position in the file | Drag or click  |

The current timestamp shows next to the slider as `HH:MM:SS.mmm`.

### How playback works

The CSV Player feeds each row through the same Frame Builder and Dashboard path as a live connection, so widgets, plots and gauges render as they would with a device.

```mermaid
flowchart LR
    A["CSV File"] --> B["CSV Player"]
    B --> C["Frame Builder"]
    C --> D["Dashboard"]
```

Playback runs in real time, using the gaps between timestamps. There is no speed multiplier; drag the progress slider to skip ahead.

### Multi-source CSV files

Load the project that produced the file before opening a multi-source CSV. The header carries labels, not machine-readable IDs, so the player maps columns positionally: it builds the export schema of the loaded project in unique-ID order and matches it against the file's data columns in that order. A project with an identical dataset and unique-ID layout works as well. Reconstructed per-source frames go back into the pipeline at the right routing.

## Analyzing exported data

CSV opens in every common analysis tool:

- Excel / LibreOffice Calc: open directly.
- Python: `df = pandas.read_csv('file.csv', encoding='utf-8-sig')`.
- MATLAB (`readtable`) and R (`read.csv`): check the first header cell for a BOM character.

## Automation

The [API](API-Reference.md) exposes `csvExport.getStatus`, `csvExport.setEnabled`, `csvExport.setInterval` and `csvExport.close` for export, and the `csvPlayer.*` family (`open`, `setPaused`, `setProgress`, `getStatus`, and `csvPlayer.step` for frame stepping) for playback. The API Server is off by default and listens on port 7777 once enabled.

The `--csv-export` flag in the [Command-Line Interface](Command-Line-Interface.md) turns export on at startup. The flag exists only in Pro builds; the Setup switch works in the free edition.

## See also

- [MDF4 Export & Playback](MDF4.md): binary logging with per-channel sample rates and channel metadata (Pro).
- [Historian](Historian.md): SQLite-backed project archive with built-in replay (Pro).
- [Operation Modes](Operation-Modes.md): Quick Plot vs Project File mode.
- [API Reference](API-Reference.md): the `csvExport.*` and `csvPlayer.*` commands.
