# Deriving values

Your sensor sends hundredths of a degree Celsius, such as `2531` for 25.31 C, and the dashboard should show kelvin. A dataset is one channel of the device's output, such as one comma-separated value on a `Serial.println` line. Select the dataset in the Project Editor, open its transform, pick **Expression** in the language dropdown, and type:

```text
v * 0.01 + 273.15
```

`v` is the dataset's value. When one line is not enough, the table below picks the next mechanism.

| Mechanism | Use it for | Runs | Edition |
|-----------|-----------|------|---------|
| [Value transform](#value-transform) | Calibrating, converting, or filtering one dataset's own value | Once per frame, per dataset | Pro |
| [Computed dataset](#computed-dataset) | A new channel built from other datasets, such as an average or a ratio | Once per frame | Pro |
| [Variables](#variables) | Constants and state shared between datasets: calibration tables, integrators, flags | Read and written by transforms and scripts | Pro |
| [Control Loop](#control-loop) | Driving the device and reacting to data over time, outside the frame cycle | On its own thread, paced by `delay()`; JavaScript | Free; reading or writing Variables from it needs Pro |
| [`dashboardTick()`](#dashboardtick) | Values a script owns, with no device frame, that must also be recorded | When the Control Loop or a JavaScript macro calls it | Free; the Variables it reads need Pro |

If the value comes from one dataset, use a transform. If it comes from several, use a computed dataset. If it must be remembered between frames or shared, use Variables. If it depends on time, or the device has to be asked first, use the Control Loop. 

A project that contains a transform or a Variables table needs a license or an active trial to connect. The first use of a Pro feature shows a Yes/No prompt that offers the trial (it needs an internet connection to register); answer No to keep working in the free edition. See [Pro vs Free](Pro-vs-Free.md).

## Value transform

Value transforms are a Pro feature. In the Project Editor, select a dataset and use the transform editor's language dropdown: **Lua**, **JavaScript** or **Expression**. A transform rewrites that dataset's value after parsing and before display and export. An Expression is one arithmetic line with no script engine, and `v` is the dataset's value, as in the example at the top of this page.

Lua and JavaScript transforms define `transform(value)` instead, and can keep state between frames:

```lua
local avg = nil
function transform(value)
  avg = (avg == nil) and value or (0.9 * avg + 0.1 * value)
  return avg
end
```

See [Dataset Value Transforms](Dataset-Transforms.md).

## Computed dataset

A computed dataset is a Pro feature: a dataset with no Frame Index (the position of its value in the parsed frame), whose transform produces the whole value, typically from other datasets' final values. Serial Studio marks a dataset computed automatically, or you can tick the computed checkbox in the Project Editor. The final value is what a dataset holds after its own transform. `datasetGetFinal()` takes the integer unique ID shown next to a dataset in the Project Editor, or its **Script Alias**; replace 10 and 11 below with those of your two channels, or pass an alias string such as `datasetGetFinal("temp1")`:

```lua
function transform(value)
  return (datasetGetFinal(10) + datasetGetFinal(11)) / 2
end
```

A Lua or JavaScript transform that never reads `value` makes its dataset computed when the project loads, and the Frame Index field greys out. Inside a computed dataset `value` is always `0`. Transforms run in project-tree order, so place a computed dataset after the datasets it reads. See [Computed datasets](Dataset-Transforms.md#computed-datasets).

## Variables

Variables are a Pro feature. A Variables table holds named numbers that stay between frames, such as a running total or a calibration constant, and that parsers, transforms and scripts share. **Read-Only** (constant) variables are set in the Project Editor and fixed for the session; **Read/Write** (computed) variables are written with `tableSet()` and keep their last value. A computed variable stores a value; a computed dataset is a channel shown on the dashboard.

Define the table first. This example needs a table named `energy` with a Read/Write variable `total`, because `tableSet()` on a variable that does not exist is ignored:

```lua
function transform(value)
  local total = (tableGet("energy", "total") or 0) + value * 0.1
  tableSet("energy", "total", total)
  return total
end
```

Use them for calibration constants that change per deployment, and for state that more than one dataset reads. See [Variables](Variables.md).

## Control Loop

The Control Loop is a free feature: one `setup()` / `loop()` JavaScript script per project, found under **Project Scripts** in the Project Editor tree. `setup()` runs once and `loop()` repeats until you disconnect. It runs beside the frame pipeline and suits work that is not tied to a single frame: the handshake that makes a silent device start sending, polling, watchdogs, and values that depend on time. The example below only writes a Variable and does not touch the device; to send a command to the device, see [Control Loop](Control-Loop.md).

```javascript
function loop() {
  tableSet("sim", "temp", 20 + 5 * Math.sin(Date.now() / 5000));
  refreshDashboard();
  delay(100);
}
```

Transforms only re-run when a frame arrives. `refreshDashboard()` re-runs them from the last received values and repaints the dashboard only, so it does nothing until the device has sent at least one frame, and nothing reaches CSV, MDF4, Historian or MQTT. For a device that has never sent a frame, use `dashboardTick()`. `delay(100)` waits 100 ms. `refreshDashboard()` and `dashboardTick()` have no Lua equivalent. See [Control Loop](Control-Loop.md).

## dashboardTick()

When a script owns the values, for example a simulation that writes Variables while the device sends nothing useful, call `dashboardTick()` from the Control Loop instead of `refreshDashboard()`:

```javascript
function loop() {
  tableSet("sim", "temp", 20 + 5 * Math.sin(Date.now() / 5000));
  dashboardTick();
  delay(100);
}
```

It re-runs the transforms, publishes the result to the dashboard, and sends the frame to every enabled export sink (CSV, MDF4, Historian, MQTT, InfluxDB, the API stream), so the script's values are recorded like device data. A sink that is off records nothing. `dashboardTick()` is JavaScript only and is available in the Control Loop and in JavaScript macros. It publishes only in a project-file session with at least one group; in Quick Plot or Console mode, or when no dataset value changed since the last tick, it returns `published: false` and records nothing.

In this setup, make the frame parser (the Frame Parser script in the Project Editor) return an empty list `[]` or `{}`, or the device's frames and the ticks are both exported. See [Reading data: See, Decide, Act](Control-Loop.md#reading-data-see-decide-act) and [Frame Parser Reference](Frame-Parser.md).

## See also

- [Concepts](Concepts.md): where these mechanisms sit in the chain from device to dashboard.
- [Frame Parser Reference](Frame-Parser.md): when the right fix is to change what the parser returns.
