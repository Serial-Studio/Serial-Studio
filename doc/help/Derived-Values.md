# Deriving values

## Overview

A dashboard often needs a value the device does not send directly: a calibrated reading, an average of two channels, a running total, or a value a script produces with no device frame at all. Serial Studio has five mechanisms for this. They overlap, so this page says which one fits which job.

| Mechanism | Use it for | Runs | Edition |
|-----------|-----------|------|---------|
| [Value transform](#value-transform) | Calibrating, converting, or filtering one dataset's own value | Once per frame, per dataset | Pro |
| [Computed dataset](#computed-dataset) | A new channel built from other datasets, such as an average or a ratio | Once per frame | Pro |
| [Variables](#variables) | Constants and state shared between datasets: calibration tables, integrators, flags | Read and written by transforms and scripts | Pro |
| [Control Loop](#control-loop) | Driving the device and reacting to data over time, outside the frame cycle | On its own thread, paced by `delay()` | Free; Variables it uses are Pro |
| [`dashboardTick()`](#dashboardtick) | Values a script owns, with no device frame, that must also be recorded | When the Control Loop calls it | Free; Variables it uses are Pro |

Start at the top of the table and stop at the first row that does the job: each row costs more setup than the one before it.

## Value transform

A value transform rewrites one dataset's value after parsing and before display and export. The cheapest form is an **Expression**, one arithmetic line with no script engine:

```text
v * 0.01 + 273.15
```

Lua and JavaScript transforms define `transform(value)` and can keep state between frames:

```lua
local avg = nil
function transform(value)
  avg = (avg == nil) and value or (0.9 * avg + 0.1 * value)
  return avg
end
```

See [Dataset Value Transforms](Dataset-Transforms.md).

## Computed dataset

A computed dataset has no frame index. Its transform produces the whole value, typically from other datasets' final values:

```lua
function transform(value)
  return (datasetGetFinal(10) + datasetGetFinal(11)) / 2
end
```

A transform that never reads `value` makes its dataset computed automatically when the project loads; the editor greys out the Frame Index field and says why. Transforms run in project-tree order, so place a computed dataset after the datasets it reads. See [Computed datasets](Dataset-Transforms.md#computed-datasets).

## Variables

Variables tables hold named values that parsers, transforms and scripts share. **Constant** variables are set in the Project Editor and fixed for the session; **computed** variables are written with `tableSet()` and keep their last value:

```lua
function transform(value)
  local total = (tableGet("energy", "total") or 0) + value * 0.1
  tableSet("energy", "total", total)
  return total
end
```

Use Variables for calibration constants that change per deployment, and for state that more than one dataset reads. See [Variables](Variables.md).

## Control Loop

The Control Loop is one `setup()` / `loop()` script per project, running beside the frame pipeline. It suits work that is not tied to a single frame: wake-up handshakes, polling, watchdogs, and values that depend on time rather than on arriving data. It reads and writes Variables with `tableGet()` and `tableSet()`. Transforms only re-run when a frame arrives, so after writing variables while the device is silent, call `refreshDashboard()` to update the view. See [Control Loop](Control-Loop.md).

## dashboardTick()

When a script owns the values, for example a simulation that writes Variables while the device sends nothing useful, call `dashboardTick()` from the Control Loop. It re-runs the transforms and publishes the result to the dashboard and to the enabled exports (CSV, MDF4, Historian, MQTT), so the script's values are recorded like device data. Make the frame parser return an empty list in this setup; otherwise every value is exported twice. See [Variables tables](Control-Loop.md#variables-tables-tableget--tableset).

## See also

- [Concepts](Concepts.md): where these mechanisms sit in the chain from device to dashboard.
- [Frame Parser Reference](Frame-Parser.md): when the right fix is to change what the parser returns.
