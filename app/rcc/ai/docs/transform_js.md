# Per-Dataset Value Transform (JavaScript)

A transform turns a raw numeric (or string) dataset value into the value the
dashboard plots, displays, and exports. Transforms run on every frame, in
declared dataset order. They are independent per dataset.

## Contract

```js
function transform(value) {
  return value;  // must return a number (or QString-coercible value)
}
```

- `value` is the current frame's raw value for this dataset, after the parser
  ran. For numeric datasets it is a `Number`; for string datasets it is a
  `String`.
- Return a finite number. Returning `NaN`, `Infinity`, or `undefined`
  rejects the transform's output and the dashboard uses the **raw** value
  instead. (No exception, just a fallback.)

## Virtual vs non-virtual

Set `virtual: true` on the dataset **only** when the transform has no
parser-supplied `value`, i.e. its output is built purely from peers,
tables, or constants (e.g. `Power = Voltage × Current`). A transform
that USES `value` (unit conversion like `km/h = m/s × 3.6`, EMA
smoothing, calibration, deadband) stays non-virtual. Rule of thumb: if
the body references the `value` argument, leave `virtual` alone.

## Isolation

Your script is automatically wrapped in an IIFE at compile time:

```js
(function() {
  /* your code */
  return typeof transform === 'function' ? transform : null;
})()
```

That means top-level `var/let/const` are private per dataset, even when
several datasets in the same source share a JS engine. Two datasets can
both define `let alpha = 0.2` without clobbering each other.

## Data table API: the central data bus

Transforms read from and write to two kinds of registers.

**System table** (`__datasets__`, always present): two registers per
dataset, `raw:<uniqueId>` and `final:<uniqueId>`, populated by the
FrameBuilder during parsing. Read-only from a transform. You almost
never read it via `tableGet`; use the convenience helpers below, which
are faster and more legible.

**User tables**: defined in the project (create via
`project.dataTable.add` + `project.dataTable.addRegister`). Each register
is one of two types:

- `Constant`: single immutable value across the session. Set once at
  declaration; every `tableGet` returns it. Use for calibration
  coefficients, lookup tables, configuration flags.
- `Computed`: writable from transforms via `tableSet`. Holds the last
  value written **indefinitely** (no per-frame reset). Use for
  filter/integrator state, cross-frame counters, latched flags, and
  intermediate results another transform reads later. The
  `defaultValue` is the starting value at project load, not a
  recurring reset.

Functions injected into your scope:

```js
tableGet(tableName, registerName)              // -> number | string
tableSet(tableName, registerName, value)       // user-table writes only
datasetGetRaw(uniqueId)                        // raw value of any dataset
datasetGetFinal(uniqueId)                      // final value of an EARLIER dataset (this frame)
```

`uniqueId` is an OPAQUE integer that uniquely identifies a dataset
within the project. It comes back from `project.dataset.list` (under
`uniqueId`), from `project.snapshot`, and from the resolvers
`project.dataset.getByPath { path: "Group/Dataset" }` /
`getByTitle` / `getByUniqueId`.

**Treat the value as opaque.** It happens to be derived from
`(sourceId, groupId, datasetId)`, but reordering changes those
numbers. Resolve once via the API and pass the resulting integer into
`datasetGetRaw / datasetGetFinal`; never recompute it.

## Processing order

Datasets are processed in group-array then dataset-array order, in a
single pass. A transform can read:

- raw and final values of **earlier** datasets (in execution order):
  this frame's
- raw and final values of **later** datasets: the **previous** frame's

Reading `datasetGetRaw`/`datasetGetFinal` of a dataset processed later
silently returns the previous frame's (stale) value; the one-shot warning
fires only for an unknown `uniqueId`. Use
`project.dataset.getExecutionOrder` to confirm the order of execution
when peer reads return stale values.

## When to use a table vs a transform local

- **Per-dataset state across frames** (EMA, last-seen value, deadband
  hysteresis): use a top-level `let`/`var` in your transform script. The
  IIFE wrapping keeps each dataset's locals private. Cheaper than a
  table register and isolated to the one dataset.
- **State shared across datasets and frames** (a filter whose output
  several downstream channels read, a long-running integrator, a
  latched alarm): use a Computed register, which persists.
- **Shared *constants*** (calibration coefficients used by N channels,
  lookup tables, full-scale ranges): use a Constant register.
- **Cross-dataset derived values within one frame** (compute speed from
  dx and dt that arrive in the same frame): use `datasetGetRaw` to read a
  peer dataset, OR write to a Computed register in the earlier transform
  and `tableGet` it from the later one.

## Examples

### EMA smoothing (per-dataset state)

```js
let ema = 0;
const alpha = 0.2;

function transform(value) {
  ema = alpha * value + (1 - alpha) * ema;
  return ema;
}
```

Two datasets in the same project can both use this template; the IIFE
wrapping keeps each dataset's `ema` independent.

### Calibration from a constants table

```js
function transform(value) {
  const offset = tableGet('Calibration', 'offset');
  const scale = tableGet('Calibration', 'scale');
  return (value - offset) * scale;
}
```

### Cross-dataset compute (speed from dx and dt)

`uniqueId` is a number, not a name. Read it from
`project.dataset.list` and pin it in a top-level constant.

```js
const DT_MS_UID = 10003;       // <- from project.dataset.list

function transform(dx) {
  const dt = datasetGetRaw(DT_MS_UID);
  return dt > 0 ? (dx / dt) * 1000 : 0;
}
```

### Running integrator via Computed register

```js
// Computed register Trip.litresUsed (defaultValue 0) persists across
// frames, so the running total accumulates session-long.
function transform(litresPerHour, info) {
  if (!info) return tableGet('Trip', 'litresUsed') || 0;

  const prevTs = tableGet('Trip', 'lastTimestampMs') || info.timestampMs;
  const dtMs   = info.timestampMs - prevTs;
  tableSet('Trip', 'lastTimestampMs', info.timestampMs);

  const delta = (litresPerHour / 3600.0) * (dtMs / 1000.0);
  const total = (tableGet('Trip', 'litresUsed') || 0) + delta;
  tableSet('Trip', 'litresUsed', total);
  return total;
}
```

## Performance

Transforms run on every frame for every dataset that defines one. Cheap:
arithmetic, single `tableGet` calls, branchless math. Avoid:

- `JSON.parse`, `JSON.stringify`
- Allocating arrays / objects per call
- Try/catch in the hot path

## Frame metadata: second `info` argument

```js
function transform(value, info) {
  // info.frameNumber : number, per-source counter (1-based, monotonic)
  // info.sourceId    : number
  // info.timestampMs : number, monotonic ms (steady clock, NOT wall clock)
}
```

One-arg transforms keep working (JS ignores extra args). `timestampMs` is
a monotonic counter; use deltas, not absolute time. Per-dataset state
goes in a top-level `let` (IIFE-private):

```js
let lastTs = 0;
function transform(v, info) {
  if (info.timestampMs - lastTs >= 100) {
    lastTs = info.timestampMs;
    deviceWrite("PING\n");
  }
  return v;
}
```

## Firing project actions: `actionFire()`

```js
actionFire(actionId) -> { ok: true } | { ok: false, error: "..." }
```

Triggers an existing project Action by its stable `actionId` (NOT its
index). Reuses the action's prebuilt payload, encoding, and timer mode.
Calls are logged `[actionFire] id=N index=M ok`.

## Device output: `deviceWrite()`

```js
deviceWrite(data, sourceId?) -> { ok: true } | { ok: false, error: "..." }
```

- `data` is a string (UTF-8 on the wire) or an array of byte values (0–255).
- `sourceId` is optional; default is the source the dataset belongs to.
- Synchronous, fire-and-forget. Never throws. Logged as
  `[deviceWrite] source=N bytes=M written=K`.

Use for **closed-loop control**: react to a sensor reading by pushing a
setpoint, alarm, or request back to the device. Latch repeated actions in
a top-level `let` (the IIFE wrap keeps it private per dataset).

```js
let triggered = false;
function transform(value) {
  if (!triggered && value > 100) {
    const r = deviceWrite("ALARM=1\n");
    if (r.ok) triggered = true;
  }
  return value;
}
```

## Dashboard controls

Seven UI helpers, all returning `{ ok: true }` or `{ ok: false, error: "..." }`. None of them log. Latch with a top-level `let` so each call fires once per state transition, not every frame.

```js
clearPlots()                          // wipe plot/multiplot/FFT/GPS/3D/waterfall
setPlotPoints(n)                      // horizontal sample window (n >= 1)
setTerminalVisible(visible)           // bool
setNotificationLogVisible(visible)    // bool
setClockVisible(visible)              // bool
setStopwatchVisible(visible)          // bool
setActiveWorkspace(idOrName)          // workspaceId (int >= 1000) OR title
```

Example: any reading at the device's reboot sentinel value (>= 9999) wipes the plot history so the new boot draws cleanly.

```js
function transform(value) {
  if (value >= 9999) {
    clearPlots();
    return 0;
  }
  return value;
}
```

These affect the active dashboard window only. They do NOT modify the project file or the user's persisted preferences.

## Errors

Returning `NaN` or `Infinity` falls back to the raw value silently. Throwing
an exception also falls back to the raw value silently (no log); only
timeouts log. Don't rely on exceptions for control flow.
