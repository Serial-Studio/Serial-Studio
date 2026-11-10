# Plots

Your board prints `23.5`, then `23.6`, once a second. A **Plot** turns that stream into a line that scrolls across the screen, newest reading at the right edge.

```cpp
Serial.println(temp);   // sends 23.5 and a newline: one frame, one value, one moving line
```

If you have no project yet, follow [Getting Started](Getting-Started.md) first. Then, in the [Project Editor](Project-Editor.md), select the dataset that holds the value and tick its **Plot** widget (`graph: true` on the dataset in the project file).

## Plot and Multi-Plot

A **Plot** shows one dataset (enable it with `graph: true`). A **Multi-Plot** overlays several signals from one group on a shared axis with a color legend, which suits comparing channels such as the three axes of an accelerometer (set the group's widget to Multi-Plot). Both are configured in the Project Editor.

| You want to                           | Use                                                            |
|---------------------------------------|----------------------------------------------------------------|
| Watch one signal over time            | Plot                                                           |
| Compare several signals on one chart  | Multi-Plot                                                     |
| Plot one value against another        | Plot with a custom X axis                                      |
| Hold a repeating waveform still       | Plot or Multi-Plot with Sweep (Pro)                            |
| See a frequency spectrum              | FFT Plot (see [Widget Reference](Widget-Reference.md#fft-plot)) |

## How the X axis works

Time is the default. Change it with the **X-Axis Source** field in the Project Editor (`xAxis` in the project file): on a dataset it offers Time, Samples, or another dataset; on a Multi-Plot group it offers Time or Samples.

| X axis      | Set by X-Axis Source                              | Reads as                                  |
|-------------|---------------------------------------------------|-------------------------------------------|
| **Time**    | Time (default)                                    | Seconds, newest sample at the right       |
| **Samples** | Samples                                           | Sample index, 0 to N, newest at the right |
| **Custom**  | Another dataset (Plot only)                       | That dataset's value (XY / scatter)       |

A **time** plot scrolls: it shows the most recent window of data, as wide as the dashboard time range, with the newest reading pinned at the right edge. A **custom X axis** plots one dataset against another instead of against time, which gives XY traces such as a phase plot or an I-V curve.

Two settings control how much history a plot keeps:

| Setting         | Meaning                                         | Range and default                                                            | Where                                                                                 |
|-----------------|-------------------------------------------------|------------------------------------------------------------------------------|---------------------------------------------------------------------------------------|
| **Time Range**  | Width of the scrolling time window              | Presets: 1, 2, 5, 10, 20, 50, 100, 200, 500 ms; 1, 2, 5, 10, 20, 30, 60, 120, 300 s. Typed values snap to the nearest. Default 10 s.  | Project Editor toolbar (saved as `plotTimeRange`) and Preferences, Plotting tab       |
| **Point Count** | Samples kept per signal (window width on a Samples plot) | 10 to 100000 in steps of 10. Default 1000.                           | Project Editor toolbar (saved as `pointCount`) and Preferences, Plotting tab          |

The Project Editor values are saved into the project file; the Preferences dialog applies them to the live dashboard.

The Y axis auto-scales to the data by default. Pin it with `plotMin` / `plotMax` in the project file (the Y-axis minimum and maximum fields in the Project Editor), or from the **Axis Range** dialog on the toolbar. **Logarithmic Y Axis** scales it in decades, and values at or below zero are drawn at the bottom. **Logarithmic X Axis** (Project Editor, dataset form) is available when X-Axis Source is Samples or another dataset, not Time.

## Toolbar controls

When a plot is large enough to show its toolbar, these controls appear left to right:

| Control | Icon | What it does |
|---------|------|--------------|
| **Interpolation** | <img src="icon:commands/interpolate-on" alt="interpolate on" width="16" height="16"> <img src="icon:commands/interpolate-off" alt="interpolate off" width="16" height="16"> | Cycles line style: none (raw points), linear, ZOH (stepped: holds the last value until the next sample), or stem (a vertical line from zero to each point). |
| **Area** | <img src="icon:commands/area" alt="area" width="16" height="16"> | Plot only. Fills the region under the curve. Enabled for linear and ZOH; disabled for none and stem. |
| **X / Y label** | <img src="icon:commands/x" alt="x" width="16" height="16"> <img src="icon:commands/y" alt="y" width="16" height="16"> | Shows or hides each axis label. |
| **Crosshair** | <img src="icon:commands/crosshair" alt="crosshair" width="16" height="16"> | Shows a tracking crosshair that follows the cursor. |
| **Pause / Resume** | <img src="icon:commands/pause" alt="pause" width="16" height="16"> <img src="icon:commands/resume" alt="resume" width="16" height="16"> | Freezes this plot. Readings that arrive while paused are not drawn after Resume. Other widgets keep updating. |
| **Sweep / Trigger Mode** | <img src="icon:commands/sweep" alt="sweep" width="16" height="16"> | Turns [Sweep / Trigger mode](#sweep--trigger-mode-pro) on or off. A repeating waveform holds still, like on an oscilloscope (Pro, time-axis plots only). |
| **Trigger Settings** | <img src="icon:commands/trigger" alt="trigger" width="16" height="16"> | Opens the trigger settings (Pro). Enabled only while Sweep is on. |
| **Past sweeps** | A stepper with `‹` and `›` arrows | Plot only. Shown next to Trigger Settings while Sweep is on. Keeps 0, 1, 2, 4, 8, 16, 32 or 64 earlier sweeps drawn behind the live trace, dimmed by age. |
| **Reset View** | <img src="icon:commands/return" alt="return" width="16" height="16"> | Returns pan and zoom to the default view. |
| **Axis Range Settings** | <img src="cmd:app.preferences" alt="Preferences" width="16" height="16"> | Opens the Axis Range dialog to set fixed minimum and maximum values for the axes. The button shows the gear (Preferences) icon. |

Drag inside the plot to pan and scroll to zoom; **Reset View** undoes both. These buttons and the toolbars of every other widget are catalogued in the [Toolbar & Button Reference](Toolbar-Reference.md#plot).

## Sweep / Trigger mode (Pro)

A scrolling sine wave slides past too fast to read, like carriage numbers on a passing train. **Sweep / Trigger mode** works the way a benchtop oscilloscope does: every sweep starts at the same point on the waveform, so the trace holds still and you can read it.

Sweep / Trigger is Pro and works only on plots with a Time X axis; otherwise, or without a license, the Sweep and Trigger Settings buttons are hidden. The first Pro-feature use shows a Yes/No prompt offering a trial, which needs an internet connection to register. See [Pro vs Free](Pro-vs-Free.md).

Turn it on with the **Sweep** button, then open **Trigger Settings**. For a first stable trace, set **Level** to the middle of your signal (0 for a sine that swings from -1 to 1), **Slope** to Rising, **Mode** to Auto, and **Timebase** to two or three cycles of the signal.

### Trigger: what starts each sweep

In Sweep / Trigger mode (Pro), the Trigger Settings dialog sets the **trigger**, the event that begins a new sweep. You choose the value the signal must cross (the **Level**) and the direction of the crossing (the **Slope**, Rising or Falling). Each time the signal crosses that level in that direction, a fresh sweep starts there, which keeps successive sweeps aligned. While the dialog is open, a horizontal line on the plot marks the Level; drag it to change Level and the dialog field follows.

There are three **modes**:

| Mode       | Behavior                                                                  |
|------------|---------------------------------------------------------------------------|
| **Auto**   | Triggers on a crossing, and free-runs anyway if none is found. Always moving. |
| **Normal** | Triggers only on a crossing. The trace holds the last sweep until the next one. |
| **Single** | Captures one sweep, then stops. Use **Capture Next** to arm another.      |

Auto is the everyday choice. Use Normal for an intermittent signal you don't want redrawn by noise, and Single to freeze one event.

On a **Multi-Plot**, the **Source** setting picks which curve the trigger watches. All curves are captured together and stay aligned to that one signal.

### Timing: width and rate

In Sweep / Trigger mode (Pro), the **Timebase:** field in Trigger Settings sets how much time one sweep shows, in milliseconds. Leave it empty (the field reads "Match time range") to use the plot's full time range. Lower it to zoom in: a shorter timebase shows fewer cycles but draws them larger, which is the setting to reach for when a fast waveform looks like a solid band. The trace begins at the left edge, time zero, and runs for one timebase to the right.

A Timebase longer than the plot's time range is capped at the time range.

The **Holdoff:** field, also in milliseconds (default 0), ignores new triggers for that long after each one. Raise it when a complex waveform crosses the level several times per cycle and you want to lock onto just the first crossing.

## Performance

A plot redraws at the **UI Refresh Rate** (Preferences, Plotting tab; 1 to 240 Hz).
