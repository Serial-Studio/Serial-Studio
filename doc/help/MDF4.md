# MDF4 export and playback (Pro)

Switch on **MDF4 Recording**, connect your device, let it run for ten minutes, and a file called `2026-03-17_15-30-05.mf4` is waiting in `Documents/Serial Studio/MDF4/<project name>/`. MDF4 is a Pro feature. Open it later in a tool such as `asammdf`, or replay it in Serial Studio through the same data pipeline as a live connection.

MDF4 (Measurement Data Format 4, `.mf4`) is an [ASAM](https://www.asam.net/standards/detail/mdf/) binary log format used in automotive and test tools. Each project group gets its own set of columns with its own timestamps, so a 10 kHz vibration signal and a 1 Hz GPS signal share one file without padding. Serial Studio writes it uncompressed.

For text logging and the "which file format?" comparison, see [CSV Export & Playback](CSV-Export-Playback.md). For queryable, project-scoped session storage, see the [Historian](Historian.md).

## MDF4 export

### Turning export on

MDF4 export (Pro) is the **MDF4 Recording** switch under **Data Export** in the Setup panel, next to the CSV switch.

1. Connect your device.
2. In the Setup panel, under **Data Export**, turn on **MDF4 Recording**. You can also turn it on before connecting.
3. Let data flow, then disconnect.
4. Open the MDF4 folder described in [File location](#file-location) and find the `.mf4` file.

The switch is greyed out in Console Only mode. Without a Pro license no file is created: the first time you switch it on, Serial Studio offers the 14-day trial (a Yes/No prompt; registering the trial needs an internet connection), and after the trial expires the prompt offers activation or purchase instead. See [Pro vs Free Features](Pro-vs-Free.md). The setting is remembered between runs.

### File location

Serial Studio writes MDF4 files to `Documents/Serial Studio/MDF4/<Project Name>/` by default, named after the session start time. To move the workspace, see [CSV Export & Playback](CSV-Export-Playback.md).

```text
Serial Studio/MDF4/<Project Name>/<yyyy-MM-dd_HH-mm-ss>.mf4
```

For a session started at 3:30:05 PM on March 17, 2026, in a project named "Vehicle Test":

```text
Serial Studio/MDF4/Vehicle Test/2026-03-17_15-30-05.mf4
```

If that name is taken, Serial Studio appends `_2`, `_3` and so on, up to 64 attempts. A pause and resume within the same second therefore gives `..._2.mf4` instead of overwriting the first file.

To turn a file into CSV with Python:

```python
# pip install asammdf
from asammdf import MDF
MDF("2026-03-17_15-30-05.mf4").export(fmt="csv", filename="out.csv")
```

### Channels and metadata

Each project group becomes a channel group and each dataset becomes a channel, except Image View groups, which are not exported. A dataset called Temperature in the group Sensors appears as `Temperature` in channel group `Sensors`, plus a `Temperature (raw)` channel with the value before any [dataset transform](Dataset-Transforms.md). Channel names are written as UTF-8.

| Item | In the file |
|------|-------------|
| Channel group name | The group's title; `SourceName / GroupName` in multi-source projects |
| Unit | The dataset's `units` field |
| Data type | 64-bit float (8 bytes) for numeric datasets; fixed 256-byte UTF-8 string for text datasets |
| Time | The timestamp channel (`Time`, the MDF4 master channel): seconds from the file's start time, one per group |
| File header | Author (`Serial Studio`), description, subject (the project title), project (`Telemetry Data`) and start time |

Serial Studio writes only these. MDF4 itself supports conversion formulas and comments, but the exporter writes none, so scaling does not travel with the file.

### File lifecycle

One file covers one connected, unpaused stretch. Serial Studio creates it on the first frame received after export is on, and closes it on disconnect, pause, or when you turn export off. Resuming or reconnecting starts a new file with a new timestamp. If no frame arrives, no file appears.

### Background writing

Writing happens on a background thread so it does not slow the dashboard; the queue holds 8192 items and flushes every 1000 ms or at 1024 items. The export path never re-stamps frames, so CSV and MDF4 carry the same source timestamp (see [Threading and Timing Guarantees](Threading-and-Timing.md)).

### API and command line

To start MDF4 export from a script, run Serial Studio with `--mdf-export` (Pro; see the [Command-Line Interface](Command-Line-Interface.md)), or enable the API Server (off by default, port 7777). The [API](API-Reference.md) then exposes `mdf4Export.getStatus`, `mdf4Export.setEnabled` and `mdf4Export.close`.

## MDF4 playback

### Opening an MDF4 file

MDF4 playback (Pro) replays a recorded file without a device connected.

1. Click **Open MDF4** in the toolbar. It is disabled while you are connected, while a file is already open, and in operator (runtime) mode. Without a Pro license it triggers the same trial or activation prompt as export.
2. Pick a `.mf4` (or `.dat`) file. The dialog starts in the `MDF4` folder of your workspace.
3. The **MDF4 Player** dialog opens.

### How playback works

The MDF4 Player (Pro) feeds each frame through the same pipeline as a live connection: Frame Builder, then Dashboard and widgets. The player keeps the original timing between frames.

File export (CSV and MDF4) is suppressed while any player is open, so replaying a file does not re-record it.

### Player controls

In the MDF4 Player dialog, Space plays or pauses, Left and Right Arrow step one frame, and the progress slider seeks. The current timestamp shows above the slider as `HH:MM:SS.mmm`.

### Playing files from other tools

MDF4 playback (Pro) needs no setup for files recorded by Serial Studio: the `Time` channel is the clock and the `(raw)` channels are skipped. For files captured by other tools, the player assigns each channel group's data channels to the loaded project's datasets in file order, so the first channel feeds the first dataset. Load a project whose datasets are in the same order as the file's channels.

## Common pitfalls

| Symptom | Cause and fix |
|---------|---------------|
| No file appears | Export needs a live connection; the file is created on the first frame, not on the toggle. Console Only mode greys out the switch, and without a Pro license no file is created. |
| Files get large fast | An audio source at 48 kHz or a full CAN bus produces gigabytes per hour. Watch disk space; pausing or disconnecting closes the file and the next frame starts a new one. |
| Non-ASCII titles look garbled | Channel names are UTF-8; a tool that decodes names as ASCII garbles them. `asammdf` reads them correctly. |

## Further reading

- [ASAM MDF Standard (official site)](https://www.asam.net/standards/detail/mdf/)
- [asammdf Python library documentation](https://asammdf.readthedocs.io/)

## See also

- [CSV Export & Playback](CSV-Export-Playback.md): text-based logging and replay.
- [Historian](Historian.md): SQLite-backed project archive with built-in replay.
- [Drivers: CAN Bus](Drivers-CAN-Bus.md): connecting to a live CAN bus.
- [Pro vs Free Features](Pro-vs-Free.md): MDF4 export and playback are Pro features.
