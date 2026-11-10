# Variables (Pro)

Your Arduino prints a raw ADC count, three datasets need the same volts-per-count slope, and you would rather not paste that number into three scripts. Variables, a Pro feature of the Project Editor (Project Editor > Variables), hold it once, in a table saved with the project. A variable has a name and a default value, and stores a number or a string:

| Table `calibration` | Permissions | Default |
|---------------------|-------------|---------|
| `slope`             | Read-Only   | `0.01`  |

A **transform** is a small Lua or JavaScript function attached to a dataset in the Project Editor. It receives the parsed value and returns the value to display; see [Dataset Value Transforms](Dataset-Transforms.md). This one multiplies every value by the slope:

```lua
function transform(value)
  return value * tableGet("calibration", "slope")
end
```

You create the table and the variable in the Project Editor ([steps below](#defining-tables-in-the-project-editor)), then paste the function into a dataset's transform. Variables are what transforms share: project-wide constants (calibration factors, thresholds) and values that flow between transforms, within a frame or across frames (running filters, integrators, latched state).

**Edition.** Creating, duplicating or importing a table, and adding a variable, need a Pro license or an active trial. So does connecting with a project that defines a table. The first time you use a Pro feature without a license, Serial Studio asks whether to start the Pro trial (it needs an internet connection to register); declining leaves the feature locked, and once a trial has ended the prompt offers activation or purchase instead. Deleting the table is free and lets the project connect again. See [Pro vs Free](Pro-vs-Free.md#dataset-transforms-variables-and-custom-workspaces).

## Defining tables in the Project Editor

Variables (Pro), Project Editor > Variables. **Add Variables Table** and **Add Variable** need a Pro license or trial (see Edition above).

1. Open the project in the Project Editor.
2. Select the **Variables** node in the tree.
3. Click **Add Variables Table** and give it a name, for example `calibration` or `runtime`.
4. Select the table and click **Add Variable**. Type a name; the new variable is Read/Write with a default of `0`. Then set:

| Field             | Accepts                                    | Effect                                                                                                                      |
|-------------------|--------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------|
| **Variable Name** | text; a duplicate gets a `_2` suffix       | The name transforms pass to `tableGet()` and `tableSet()`.                                                                  |
| **Permissions**   | Read-Only, Read/Write                      | Read-Only is a constant; Read/Write is a computed variable ([Variable types](#variable-types)).                                                     |
| **Default Value** | a number or a string                       | The value at project load. A constant keeps it all session; a computed variable starts there and holds what transforms write. |

Tables are saved with the project file, so anyone who opens it gets the same tables and defaults.

The table toolbar also has **Insert Constant**, which adds a physical constant (speed of light, standard gravity, gas constants) as a Read-Only variable, and **Import** and **Export**, which move a table to and from CSV. The file starts with the header `name,permissions,value`. Permissions accepts `read-only` or `read-write` (also `read/write`, `rw`, `computed`). Import needs Pro.

The same edits are scriptable: the `project.dataTable.*` commands add, rename and delete tables and variables, and read or write a variable's live runtime value from outside a transform. See the [API Reference](API-Reference.md#data-table-commands-13).

### Naming rules

- Table and variable names are case-sensitive and free-form.
- Runs of whitespace collapse to one space, so a name typed with two spaces does not match the string in your script. Short ASCII names without spaces keep scripts readable.
- A duplicate table name gets a numeric suffix (`Calibration 2`); a duplicate variable name gets `_2`.
- A new table's name has any `/` removed, because `/` joins a folder to a table in its full path.
- Do not name a table `__datasets__`; the name belongs to the system table.

## Variable types

A variable is one of two types, picked with the **Permissions** field. This manual calls them a **constant** (Read-Only) and a **computed variable** (Read/Write); the editor only shows Read-Only and Read/Write, and a computed variable has nothing to do with a [computed dataset](Dataset-Transforms.md#computed-datasets).

A constant is written at project load and is fixed for the session. It is for configuration: sensor slopes, offsets, thresholds, full-scale ranges. `tableSet()` cannot modify it.

A computed variable behaves like ordinary memory. Once a transform writes a value it stays until the next write, which matches how state lives in embedded systems: filter states, integrators, edge counters, latched flags. To start each frame from a known value, write it with `tableSet()` at the top of an early transform.

## The transform API

Four functions read and write variables. Lua and JavaScript have the same names and semantics; a missing value is `nil` in Lua and `undefined` in JavaScript. For transforms that touch the same variables on every value, `tableGetH` and `tableSetH` resolve a name once to a handle; see [Fast table access with handles](SerialStudio-SDK.md#handles-the-fast-path-for-table-heavy-parsers).

| Function                             | Arguments                                   | Returns                                                         |
|--------------------------------------|---------------------------------------------|-----------------------------------------------------------------|
| `tableGet(table, variable)`          | table name, variable name                   | the value, or nil/undefined if the variable doesn't exist       |
| `tableSet(table, variable, value)`   | table name, variable name, number or string | nothing; ignored if the variable is a constant or doesn't exist |
| `datasetGetRaw(uniqueId \| alias)`   | integer uniqueId or alias string            | the value, or nil/undefined for an unknown dataset              |
| `datasetGetFinal(uniqueId \| alias)` | integer uniqueId or alias string            | the value, or nil/undefined for an unknown dataset              |

A `uniqueId` is the integer shown next to each dataset in the Project Editor tree; see the [Dataset Identity Model](Identity-Model.md).

- A table filed in a folder is addressed by its full path: `tableGet("sensors/calibration", "slope")`.
- A string argument to `datasetGetRaw` and `datasetGetFinal` is always an alias, even if it looks like a number, so `datasetGetFinal("10")` looks for an alias named `10`.
- An unknown ID, alias or variable returns nil and logs a warning once.

Publish a value from one transform and consume it in another:

```lua
-- Earlier dataset writes a shared value
function transform(value)
  tableSet("runtime", "pack_current", value)
  return value
end
```

```lua
-- Later dataset reads it
function transform(value)
  local i = tableGet("runtime", "pack_current")
  return value * i
end
```

See [Dataset Value Transforms](Dataset-Transforms.md#table-api) for the per-language reference, including the JavaScript equivalents.

## Processing order and visibility

A dataset sees the final values of the datasets listed above it in the Project Editor tree: earlier groups first, then earlier datasets inside the same group. For everything below it, it sees whatever the previous frame left behind. If dataset B depends on dataset A, list A before B.

Each dataset is processed in one pass before the next: its raw value is written to the system table, its transform runs, and its final value is written. So `datasetGetRaw(uid)` is current only for the running dataset and those above it. Computed variables written by earlier transforms are visible to later ones in the same frame and on every later frame, until the next write.

## The system table

Serial Studio builds one table called `__datasets__` from your project. You don't define it. The Project Editor shows it as a read-only **Dataset Values** node under Variables, listing each dataset's unique ID and its `datasetGetFinal()` access code. It mirrors every dataset as two variables:

| Variable           | Contents                                                             |
|--------------------|----------------------------------------------------------------------|
| `raw:<uniqueId>`   | Raw value from the frame parser, before the dataset's transform runs |
| `final:<uniqueId>` | Final value after the dataset's transform has run                    |

A dataset with a **Script Alias** is also exposed as `raw:<alias>` and `final:<alias>`. Outside a transform, read it with `tableGet("__datasets__", "final:ATAM1-CH1")`; inside one, use `datasetGetFinal("ATAM1-CH1")`, which returns the same value.

## Common use cases

### Sensor calibration constants

Several datasets share one calibration, and every firmware revision, lab recalibration or field swap would otherwise mean editing each transform by hand. Store the calibration in constants, so a recalibration is one edit in the Project Editor. The same pattern covers per-deployment unit factors such as `m_to_ft` or `counts_per_rev`. This recipe is also the shipped calibration transform template: open the transform editor's template picker and start there.

Table `calibration`:

| Variable | Permissions | Default | Purpose              |
|----------|-------------|---------|----------------------|
| `slope`  | Read-Only   | `0.01`  | V per ADC count      |
| `offset` | Read-Only   | `0.0`   | Zero-offset in volts |

```lua
function transform(value)
  local slope  = tableGet("calibration", "slope")
  local offset = tableGet("calibration", "offset")
  return slope * value + offset
end
```

### Thresholds and alarm limits

Store warning and critical levels in constants and let a transform return a status code (0 ok, 1 warning, 2 critical) for you to plot or display. For coloring a widget by level, see [Alarm bands](Widget-Reference.md#alarm-bands).

Table `limits`:

| Variable  | Permissions | Default |
|-----------|-------------|---------|
| `warn_lo` | Read-Only   | `10`    |
| `warn_hi` | Read-Only   | `80`    |
| `crit_lo` | Read-Only   | `0`     |
| `crit_hi` | Read-Only   | `95`    |

```lua
function transform(value)
  local wl = tableGet("limits", "warn_lo")
  local wh = tableGet("limits", "warn_hi")
  local cl = tableGet("limits", "crit_lo")
  local ch = tableGet("limits", "crit_hi")
  if value <= cl or value >= ch then return 2  -- critical
  elseif value <= wl or value >= wh then return 1  -- warning
  else return 0 end  -- ok
end
```

Pair this with a [computed dataset](Dataset-Transforms.md#computed-datasets) so the status gets its own channel without consuming a frame index.

### Derived quantities from other datasets

To compute power from voltage and current, create a computed dataset whose transform reads both channels with `datasetGetFinal()`. Both source datasets must be listed above it in the Project Editor tree (see [Processing order and visibility](#processing-order-and-visibility)). Here voltage has unique ID 10 and current has 11:

```lua
function transform(value)
  local v = datasetGetFinal(10)
  local i = datasetGetFinal(11)
  if v == nil or i == nil then return 0 end
  return v * i
end
```

If the source datasets have aliases, the transform reads them by name, which stays readable in a large project: `datasetGetFinal("voltage")` and `datasetGetFinal("current")`. The same shape fits IMU acceleration magnitude, RMS of three phase currents and torque times angular velocity.

### Cross-dataset scratch pad

When two transforms need the output of an expensive calculation (an FFT peak, a filtered value, a CRC), compute it once in an earlier dataset, publish it to a computed variable and read it from the later datasets. The writing dataset must be listed above the readers. The value persists, so readers see what the earlier transform last wrote, in this frame or carried over from the previous one.

The same pattern carries a "data valid" flag: one transform sets `probe_ok` to 0 or 1 after a range check, the others branch on it, and one out-of-range channel taints several derived channels. Declare `probe_ok` with a default of `1`, or every reader sees invalid data until the probe's transform has run once.

Table `runtime`:

| Variable     | Permissions | Default |
|--------------|-------------|---------|
| `filtered_v` | Read/Write  | `0`     |

A variable always reads as its default before the first write, never as nil. Seed on `info.frameNumber == 1`, not on a nil test. `info.frameNumber` counts frames per source, so in a multi-source project seed a shared variable from one source only.

```lua
-- Earlier dataset: compute and publish
function transform(value, info)
  local f = tableGet("runtime", "filtered_v")
  if info.frameNumber == 1 then f = value end
  local smoothed = 0.9 * f + 0.1 * value
  tableSet("runtime", "filtered_v", smoothed)
  return smoothed
end
```

```lua
-- Later dataset: consume without recomputing
function transform(value)
  return value - tableGet("runtime", "filtered_v")
end
```

### Tunable filters and cross-frame state

Put the knob (cutoff, alpha, window size) in a constant and the filter's running state in a computed variable. Configuration stays separate from state, and you re-tune the filter without touching its code. The same split serves integrators, derivatives, edge counters, peak detectors and latched alarms, because a computed variable keeps its value between frames.

An exponential moving average, with `ema_alpha` Read-Only (default `0.1`) and `ema_state` Read/Write in table `filters`:

```lua
function transform(value, info)
  local alpha = tableGet("filters", "ema_alpha")
  local ema   = tableGet("filters", "ema_state")
  if info.frameNumber == 1 then ema = value end
  ema = alpha * value + (1 - alpha) * ema
  tableSet("filters", "ema_state", ema)
  return ema
end
```

A discrete-time derivative `dT/dt` of the dataset with unique ID 2 (the temperature), using `info.timestampMs` and `info.frameNumber`, which restarts at 1 after a disconnect (see [Dataset Value Transforms](Dataset-Transforms.md)). Table `runtime` holds `last_temp` and `last_t_ms`, both Read/Write with default `0`:

```lua
function transform(_, info)
  local T = datasetGetFinal(2)
  if T == nil then return 0 end

  local prevT  = tableGet("runtime", "last_temp")
  local prevTs = tableGet("runtime", "last_t_ms")

  tableSet("runtime", "last_temp", T)
  tableSet("runtime", "last_t_ms", info.timestampMs)

  if info.frameNumber == 1 then return 0 end
  local dt = info.timestampMs - prevTs
  if dt <= 0 then return 0 end
  return (T - prevT) / (dt / 1000)
end
```

## Rules and limitations

1. `tableSet()` on a constant is ignored and Serial Studio logs a `[DataTableStore]` warning to the application log. Constants are frozen at project load.
2. A variable holds one number or one string, such as a device serial number. For a vector, use several variables or several datasets.
3. A Variables table is not a settings dialog. For a knob that changes while you watch the data, use an Output widget.
4. Tables are shared across all sources in a project. Each source processes its frames on its own, so a transform reading a value another source writes gets the latest write, with no ordering guarantee between sources.
5. Variables were called Data Tables before the rename. The API namespace is still `project.dataTable.*` and the project file still stores them under the `registers` key.

## See also

- [Dataset Value Transforms](Dataset-Transforms.md): how to write the `transform(value)` function and call the table API.
- [Project Editor](Project-Editor.md): where tables are defined.
- [Data Flow](Data-Flow.md): where transforms and tables sit in the overall pipeline.
- [Frame Parser Scripting](Frame-Parser.md): `parse(frame)` produces the raw values that transforms consume.
- [API Reference](API-Reference.md#data-table-commands-13): the `project.dataTable.*` commands.
