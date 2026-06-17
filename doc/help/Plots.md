# Plots

Serial Studio's line plots turn a stream of numbers into a moving chart. This page covers the two plot widgets, how their axes work, the toolbar controls they share, and the oscilloscope-style Sweep / Trigger mode.

## The two plot widgets

| Widget       | Source  | Curves | Enable with                           |
|--------------|---------|--------|---------------------------------------|
| **Plot**     | Dataset | One    | `graph: true` on a dataset            |
| **MultiPlot**| Group   | Many   | Set the group's widget to MultiPlot (plots every dataset in the group) |

A **Plot** draws a single signal. A **MultiPlot** overlays several signals from the same group on one chart with a shared axis and a color legend, which is the right choice when you want to compare channels (the three axes of an accelerometer, several thermocouples, a setpoint against a measured value).

Both are configured in the Project Editor and behave the same way at runtime. Everything below applies to both unless noted.

## How the X axis works

A plot's X axis is one of three kinds, chosen automatically from the dataset configuration:

| X axis      | When                                        | Reads as                          |
|-------------|---------------------------------------------|-----------------------------------|
| **Time**    | Default for a plotted dataset               | Seconds, newest sample at the right |
| **Samples** | X axis set to Samples                        | Sample index, 0 to N (newest at the right) |
| **Custom**  | `xAxisId` points at another dataset (Pro)   | That dataset's value (XY / scatter) |

A **time** plot scrolls: it shows the most recent window of data, spanning `[-T, 0]` where `T` is the dashboard time range. The newest reading is always pinned at the right edge.

Two settings control how much history a plot keeps, and both sit next to each other in two places: the **Time Range** and **Point Count** fields in the Project Editor's project toolbar, and the same two fields on the **Dashboard** tab of the **Settings** dialog. **Time Range** is the width `T` of the scrolling time window, in seconds (it sets how far back a time plot looks). **Point Count** is the number of samples retained per signal, which sizes the history buffer and is the window width for a plot drawn against sample number rather than time. The Project Editor values are saved into the project file (as `plotTimeRange` and `pointCount`); the Settings dialog applies them to the live dashboard.

A **custom X axis** plots one dataset against another instead of against time, producing XY traces (a phase plot, an I-V curve, a Lissajous figure).

The Y axis auto-scales to fit the data by default. Pin it to a fixed range with `pltMin` / `pltMax` in the Project Editor, or from the **Axis Range** dialog on the toolbar.

## Toolbar controls

When a plot is large enough to show its toolbar, these controls appear left to right:

| Control | Icon | What it does |
|---------|------|--------------|
| **Interpolation** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-on.svg" width="16" height="16"> <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/interpolate-off.svg" width="16" height="16"> | Cycles line style: none (raw points), linear, ZOH (stepped), or stem. |
| **Area** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/area.svg" width="16" height="16"> | Fills the region under the curve. Available with line interpolation. Plot only. |
| **X / Y label** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/x.svg" width="16" height="16"> <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/y.svg" width="16" height="16"> | Shows or hides each axis label. |
| **Crosshair** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/crosshair.svg" width="16" height="16"> | Shows a tracking crosshair that follows the cursor. |
| **Pause** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/pause.svg" width="16" height="16"> <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/resume.svg" width="16" height="16"> | Freezes the plot without stopping data collection. |
| **Sweep** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/sweep.svg" width="16" height="16"> | Turns Sweep / Trigger mode on or off (Pro). |
| **Trigger** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/trigger.svg" width="16" height="16"> | Opens the trigger settings (Pro). Enabled only while Sweep is on. |
| **Reset View** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/dashboard-buttons/return.svg" width="16" height="16"> | Returns pan and zoom to the default view. |
| **Axis Range** | <img src="https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/refs/heads/master/app/rcc/icons/toolbar/settings.svg" width="16" height="16"> | Sets fixed minimum and maximum values for the axes. |

Drag inside the plot to pan and scroll to zoom; **Reset View** undoes both. These buttons and the toolbars of every other widget are catalogued in the [Toolbar & Button Reference](Toolbar-Reference.md#plot).

## Sweep / Trigger mode (Pro)

A scrolling plot is good for watching a signal drift over time, but it is poor for reading the shape of a repeating waveform: each cycle slides past before you can study it. **Sweep / Trigger mode** solves this the way a benchtop oscilloscope does. It starts every sweep at the same point on the waveform, so the trace holds still and you can read it.

Turn it on with the **Sweep** button, then open the **Trigger Settings** dialog to set it up.

### Trigger: what starts each sweep

A **trigger** is the event that begins a new sweep. You choose the value the signal must cross (the **Level**) and the direction of the crossing (the **Slope**, Rising or Falling). Each time the signal crosses that level in that direction, a fresh sweep starts at exactly that point, which is what keeps successive sweeps aligned on top of each other.

There are three **modes**:

| Mode       | Behavior                                                                  |
|------------|---------------------------------------------------------------------------|
| **Auto**   | Triggers on a crossing, and free-runs anyway if none is found. Always moving. |
| **Normal** | Triggers only on a crossing. The trace holds the last sweep until the next one. |
| **Single** | Captures one sweep, then stops. Use **Capture Next** to arm another.      |

Auto is the everyday choice and matches the Auto button on a scope. Use Normal for an intermittent signal you don't want redrawn by noise, and Single to freeze one event.

On a **MultiPlot**, the **Source** setting picks which curve the trigger watches. All curves are captured together and stay aligned to that one signal.

### Timing: width and rate

**Timebase** sets how much time one sweep shows. Leave it empty to use the plot's full time range. Lower it to zoom in: a shorter timebase shows fewer cycles but draws them larger, which is how you read the shape of a fast signal. This is the setting to reach for when a fast waveform looks like a solid band.

A short timebase also refreshes the trace many times per second and holds it stationary, like a scope. A long timebase (more than about a tenth of a second) instead draws the trace as it fills, growing left to right in real time so it never appears to stall.

**Holdoff** ignores new triggers for a set time after each one. Raise it when a complex waveform crosses the level several times per cycle and you want to lock onto just the first crossing.

### Reading the markers

While the Trigger Settings dialog is open, a horizontal line marks the trigger level on the plot. The trace begins at the left edge (time zero) and runs for one timebase to the right.

## Performance

Plots draw from a decimating ring buffer sized to the time range, so a plot stays responsive whether it holds a few hundred points or millions. Rendering happens on the UI thread at a fixed refresh rate and never blocks data collection; **Pause** stops only the drawing.

## Choosing a plot

| You want to                                  | Use                          |
|----------------------------------------------|------------------------------|
| Watch one signal over time                   | Plot                         |
| Compare several signals on one chart          | MultiPlot                    |
| Plot one value against another               | Plot with a custom X axis    |
| Read the shape of a repeating waveform        | Plot or MultiPlot with Sweep |
| See a frequency spectrum                      | FFT Plot (see Widget Reference) |
