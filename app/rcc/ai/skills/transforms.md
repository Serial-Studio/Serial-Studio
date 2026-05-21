# Per-Dataset Value Transforms

Transforms run on every parsed frame, after the frame parser, before the
dashboard sees the value. They turn a raw value into the displayed value.

## Pick Lua first

Both Lua and JavaScript work, but **Lua is the recommended default** —
it's measurably faster on the hotpath at typical telemetry rates and
the embedded interpreter has a lighter per-call cost than `QJSEngine`.
Use JavaScript only when you need a JS-specific feature (regex
flavours, `JSON.stringify`, etc.).

When you push transform code, ALWAYS pass `language` so the dataset's
`transformLanguage` is locked to the syntax you wrote. A mismatch is a
silent compile failure — the dashboard will show the raw value with no
visible error. Two ways to set both at once:

```
project.dataset.setTransformCode {groupId, datasetId, code, language: 1}
project.dataset.update            {groupId, datasetId, transformCode, transformLanguage: 1}
```

## Virtual vs non-virtual: when to flip the flag

`virtual: true` means **compute-only**: the dataset has no slot in the
frame, the parser supplies no value for it, and its output is built
entirely from peer datasets, table registers, or constants. **Do not**
flip `virtual` on a regular dataset just because it has a transform.
Most transforms — unit conversion, calibration, smoothing, deadband,
hysteresis — operate on a parser-supplied `value` and stay
**non-virtual**.

**Rule of thumb.** If the transform USES its `value` argument, the
dataset is non-virtual. If it ignores `value` and reads only from
`datasetGetRaw` / `datasetGetFinal` / `tableGet`, it's virtual.

**Virtual** — Power [W] = Voltage × Current. No parser slot; the value
is computed from two peer datasets. Set `virtual: true`:

```lua
function transform(_v)
  return datasetGetFinal(VOLTAGE_UID) * datasetGetFinal(CURRENT_UID)
end
```

**Non-virtual** — km/h from a m/s sensor reading. The parser writes
m/s into `value`, the transform converts units. Leave `virtual: false`
(the default):

```lua
function transform(value)
  return value * 3.6
end
```

Same rule for EMA smoothing, ADC-counts → volts, gear-ratio applied
to RPM, deadband filters — anything that touches `value` is a regular
transform, not a virtual dataset.

```
project.dataset.add {groupId, options: ["plot"]}     -- creates dataset N
project.dataset.update {                              -- flip ONLY for compute-only
  groupId, datasetId: N,
  virtual: true,
  title: "Power",
  units: "W",
  transformLanguage: 1,                              -- 1 = Lua
  transformCode: "..."
}
```

**Auto-detect on save.** The save path inspects each dataset's transform
body. If `transformCode` is non-empty AND the body **never references
`value`**, the dataset is auto-flagged `virtual: true`. So a Power-style
transform is detected as virtual on save even if you forgot the flag,
but the dataset stays empty until that save — set `virtual` explicitly
when you push compute-only code.

If `virtual=false` and `index<=0` after a `setTransformCode`, the API
returns a `hint` field telling you to flip `virtual`. Listen to it — but
only when the transform really is compute-only. If you wrote a regular
`value`-using transform on a dataset that has no parser slot, the fix
is usually to assign `index` (give it a slot), not to flip `virtual`.

### Why prefer virtual datasets over an extra parser slot

You COULD parse-and-emit a derived value from the frame parser
directly (so the parser writes `channels[N] = computeSpeed(...)` and
a regular dataset reads slot N). Don't. Virtual datasets are the
right shape because:

- **Separation of concerns.** Frame parsers turn bytes into raw
  channels — that's their whole job. Computation, calibration, and
  derivation belong in transforms. Mixing both in the parser turns
  it into an opaque blob that's painful to debug with `dryRun`.
- **Testability.** `project.dataset.transform.dryRun{values, code}`
  exercises a transform in isolation. You can't dry-run a derived
  channel that lives inside `parse()`; you'd have to invent fake
  byte frames every iteration.
- **Cross-source reach.** A virtual dataset's transform can read
  `datasetGetFinal(uniqueId)` of any dataset, including peers from
  *other* sources via the shared data table. A parser only sees its
  own source's bytes.
- **Unit + range hygiene.** Virtual datasets carry their own
  `units`, `plotMin/Max`, `widgetMin/Max`, alarms, and widget
  bitflags. They show up cleanly in the project tree, the dashboard,
  and CSV/MDF4 exports. A computation hidden inside the parser has
  no presence in the schema.
- **Templates.** A virtual dataset (transform + units + widget
  config) is the unit you copy across projects. Parser code is
  source-bus-specific; transforms are portable.
- **Performance.** Virtual datasets share the source's transform
  engine — no extra QJSEngine / Lua state per derivation. A bloated
  parser, by contrast, runs every byte through one big function on
  every frame, allocating intermediate arrays.

Use a parser-emitted slot only when the derivation needs raw bytes
that aren't otherwise exposed (uncommon — most cases are downstream
arithmetic on already-extracted channels).

## Contract

```lua
function transform(value)
  return value  -- must return a finite number or string
end
```

```js
function transform(value) {
  return value;  // must return a finite number (or QString-coercible)
}
```

For numeric datasets, `value` is a `Number`. For string datasets, it's a
`String`. Returning `NaN`, `Infinity`, or `undefined` rejects the output
and the dashboard shows the raw value instead. No exception, just a
silent fallback.

## Per-dataset isolation

Your transform script is wrapped in an IIFE at compile time:

```js
(function() {
  /* your code */
  return typeof transform === 'function' ? transform : null;
})()
```

Top-level `var/let/const` are private per dataset, even when several
datasets in the same source share a JS engine. Two datasets can both
declare `let alpha = 0.2` without clobbering each other.

## Iteration workflow

1. Read the dataset's current transform:
   `project.dataset.list` returns `transformCode` per dataset; or fetch
   `project.frameParser.getCode` for context on what `value` looks like.
2. Dry-run: `assistant.script.dryRun{kind:"transform", code, language,
   values}`
   compiles + runs `transform()` against an array of sample inputs.
   Returns the per-input outputs. Iterate on the code until outputs look
   right.
3. Push: `assistant.script.apply{kind:"transform", groupId, datasetId,
   code, language, values, virtual}`. It dry-runs first, optionally marks
   compute-only datasets virtual, then calls `project.dataset.setTransformCode`.

## Tables: the central data bus

Transforms read and write two kinds of registers:

**System table** (`__datasets__`, always present): two registers per
dataset — `raw:<uniqueId>` and `final:<uniqueId>`. Read-only. Convenience
helpers: `datasetGetRaw(uniqueId)` and `datasetGetFinal(uniqueId)`. Use
those instead of `tableGet('__datasets__', 'raw:<uid>')`.

**User tables**: project-defined. Two register types:

- `Constant`: single value across the session. Set when declared. Use for
  calibration coefficients, lookup tables, configuration flags.
- `Computed`: resets to `defaultValue` at the start of every frame.
  Writable from transforms. Use for per-frame accumulators, derived
  values that another transform reads later in the same frame.

```js
tableGet(tableName, registerName)              // -> number | string
tableSet(tableName, registerName, value)       // user tables only
datasetGetRaw(uniqueId)                        // any dataset, this frame
datasetGetFinal(uniqueId)                      // EARLIER datasets only
```

`uniqueId` is a stable INTEGER, not a name. Read it from
`assistant.dataset.resolve`, `project.dataset.list`, or
`project.snapshot`. Treat it as opaque; do not compute it in chat.

## Processing order

Datasets are processed in group-array then dataset-array order. Inside a
transform you can read:

- raw values of **all** datasets in this frame (parser already ran)
- final values of **earlier** datasets only

Trying to read `datasetGetFinal` of a dataset processed later returns 0.

## When to use what

- **Per-dataset state across frames** (EMA, last value, deadband): use a
  top-level `let` in the transform. Cheaper than a table register.
- **Shared state across datasets** (calibration constants used by N
  channels, lookup tables): use a Constant register in a user table.
- **Cross-dataset compute within one frame** (speed from dx and dt that
  arrive together): use `datasetGetRaw` to read a peer, OR write to a
  Computed register in the earlier transform and `tableGet` it from the
  later one.

## Examples (Lua — preferred)

```lua
-- EMA smoothing (per-dataset state via upvalue)
local ema = 0
local alpha = 0.2
function transform(value)
  ema = alpha * value + (1 - alpha) * ema
  return ema
end

-- Calibration from constants table
function transform(value)
  local offset = tableGet("Calibration", "offset")
  local scale  = tableGet("Calibration", "scale")
  return (value - offset) * scale
end

-- Cross-dataset speed (DT_MS_UID from project.dataset.list)
local DT_MS_UID = 10003
function transform(dx)
  local dt = datasetGetRaw(DT_MS_UID)
  if dt and dt > 0 then return (dx / dt) * 1000 end
  return 0
end
```

## Examples (JavaScript — when Lua won't do)

```js
let ema = 0;
const alpha = 0.2;
function transform(value) {
  ema = alpha * value + (1 - alpha) * ema;
  return ema;
}
```

For ~20 more reference transforms (clamp, dead-zone, ADC-to-voltage,
celsius/fahrenheit, accumulator, autozero, bit extract, ...), call
`scripts.list{kind: "transform_lua"}` or
`scripts.list{kind: "transform_js"}`.

## Frame metadata — second `info` argument

Transforms receive `{frameNumber, sourceId, timestampMs}` as a second arg.
One-arg transforms keep working unchanged (both languages ignore extras).
`timestampMs` is a **monotonic** ms counter (steady clock), not wall
clock — use for deltas only.

```lua
local lastTs = 0
function transform(v, info)
  if info.timestampMs - lastTs >= 100 then
    lastTs = info.timestampMs
    deviceWrite("PING\n")
  end
  return v
end
```

## Closed-loop control: `deviceWrite()` and `actionFire()`

Transforms can drive output directly:

- `deviceWrite(data, sourceId?)` — synchronous fire-and-forget byte
  write. `{ ok, error? }`, never throws. `sourceId` defaults to the
  source the dataset belongs to. Logged
  `[deviceWrite] source=N bytes=M written=K`.
- `actionFire(actionId)` — fires an existing project Action by its
  stable `actionId` (NOT its index). Reuses the action's payload and
  timer mode. Same shape. Logged `[actionFire] id=N index=M ok`.

Transforms run on every frame. Always latch with a local flag or
rate-limit via `info.timestampMs` so repeated triggers don't saturate
the link:

```lua
local fired = false
function transform(v, info)
  if not fired and v > 100 then
    if deviceWrite("ALARM=1\n").ok then fired = true end
  end
  return v
end
```

Use for closed-loop control (setpoint write-back, alarm raise). For
user-triggered actions, use an Output Widget instead.

## Dashboard controls

Seven runtime UI helpers, all `{ ok, error? }`, NO logging:

- `clearPlots()` — wipe line / multiplot / FFT / GPS / 3D / waterfall buffers. Widgets, datasets, actions, axis bounds intact.
- `setPlotPoints(n)` — horizontal sample window for line plots (n >= 1).
- `setTerminalVisible(bool)`, `setNotificationLogVisible(bool)`, `setClockVisible(bool)`, `setStopwatchVisible(bool)` — show/hide the dashboard widgets.
- `setActiveWorkspace(idOrName)` — switch active workspace tab. Numeric `workspaceId` (>= 1000) or case-insensitive title.

Transforms run on every frame — gate every call behind a latch (`local fired = false` / `let fired = false`) or a sentinel-value match. Example: any reading at the device reboot sentinel (>= 9999) clears the plot history:

```lua
function transform(value)
  if value >= 9999 then
    clearPlots()
    return 0
  end
  return value
end
```

Affects the active dashboard window only. Does NOT persist to the project file or QSettings.
