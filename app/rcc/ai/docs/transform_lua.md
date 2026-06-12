# Per-Dataset Value Transform (Lua)

Lua 5.4 mirror of the JS transform API. Use Lua when your team is
Lua-fluent or when you want closure-captured local state semantics.

## Contract

```lua
function transform(value)
  return value
end
```

- `value` is a Lua number (or string for string datasets).
- Return a finite number. NaN/Inf or non-numeric returns fall back to the
  raw value silently.

## Virtual vs non-virtual

Set `virtual: true` on the dataset **only** when the transform has no
parser-supplied `value`, i.e. its output is built purely from peers,
tables, or constants (e.g. `Power = Voltage * Current`). A transform
that USES `value` (unit conversion like `km/h = m/s * 3.6`, EMA
smoothing, calibration, deadband) stays non-virtual. Rule of thumb: if
the body references the `value` argument, leave `virtual` alone.

## Isolation

Each dataset's chunk is loaded with its own per-dataset environment table
(an `_ENV` sandbox), so even globals — including `function transform` —
are private per dataset; top-level `local` declarations are additionally
chunk-private. Two datasets that define `local ema = 0` get independent
state, even though the Lua state itself is shared across the source.

```lua
local ema = 0
local alpha = 0.2

function transform(value)
  ema = alpha * value + (1 - alpha) * ema
  return ema
end
```

## Data table API

Functions injected into the Lua state:

```lua
tableGet(tableName, registerName)      -- -> number | string
tableSet(tableName, registerName, v)    -- user tables only
datasetGetRaw(uniqueId)                 -- raw value, this frame
datasetGetFinal(uniqueId)               -- final value of an earlier dataset
```

User-table registers are either `Constant` (immutable, set at project
load) or `Computed` (writable from transforms). Computed registers hold
the last value written **indefinitely**; there is no per-frame reset.
That's what makes them the natural place for filter state, integrators,
edge counters, and latched flags. The register's `defaultValue` is the
starting value at project load, not a recurring reset.

## Compatibility shim (LuaCompat)

Same shim as the parser API:

- `math.log10(x)` and `math.pow(a, b)` aliases.
- Full `bit32` library.
- `unpack(t)` for `table.unpack(t)`.

## Console logging

Same API as the parser: `print(...)` plus `console.log/debug/info/warn/error`
all land in the application console (`error` always raises an app
notification; `warn` notifies only when *Route warnings to notifications*
is enabled, off by default); script log lines are also mirrored to stdout
by the application's message handler. Transforms run on every frame for
every dataset — latch or rate-limit logging with a local flag, and remove it
from shipped projects.

## Examples

### EMA smoothing

```lua
local ema = 0
local alpha = 0.2

function transform(value)
  ema = alpha * value + (1 - alpha) * ema
  return ema
end
```

### Calibration

```lua
function transform(value)
  local offset = tableGet("Calibration", "offset")
  local scale  = tableGet("Calibration", "scale")
  return (value - offset) * scale
end
```

### Cross-dataset compute

```lua
function transform(dx)
  local dt = datasetGetRaw("dt_ms")
  if dt and dt > 0 then
    return (dx / dt) * 1000
  end
  return 0
end
```

### Hysteresis

```lua
local last = 0
local threshold = 0.05

function transform(value)
  if math.abs(value - last) >= threshold then
    last = value
  end
  return last
end
```

### Running integrator via Computed register

```lua
-- Computed register Trip.litresUsed (defaultValue 0) persists across
-- frames, so the running total accumulates session-long.
function transform(litresPerHour, info)
  if not info then return tableGet("Trip", "litresUsed") or 0 end

  local prevTs = tableGet("Trip", "lastTsMs") or info.timestampMs
  local dtMs   = info.timestampMs - prevTs
  tableSet("Trip", "lastTsMs", info.timestampMs)

  local delta = (litresPerHour / 3600.0) * (dtMs / 1000.0)
  local total = (tableGet("Trip", "litresUsed") or 0) + delta
  tableSet("Trip", "litresUsed", total)
  return total
end
```

## Performance

Lua is fast at the call boundary. Avoid `string.format` in the hot path,
avoid `pcall` unless you expect failures, and prefer arithmetic
to table lookups when you can.

## Frame metadata: second `info` argument

```lua
function transform(value, info)
  -- info.frameNumber : integer, per-source counter (1-based, monotonic)
  -- info.sourceId    : integer
  -- info.timestampMs : integer, monotonic ms (steady clock, NOT wall clock)
end
```

Existing one-arg transforms keep working unchanged (Lua ignores extra args).
`timestampMs` is a monotonic counter; use it for deltas, not absolute time.

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

## Firing project actions: `actionFire()`

```lua
actionFire(actionId) -> { ok = true } | { ok = false, error = "..." }
```

Triggers an existing project Action by its stable `actionId` (NOT its
index). Reuses the action's prebuilt payload, encoding, and timer mode.
Calls are logged `[actionFire] id=N index=M ok`.

## Device output: `deviceWrite()`

```lua
deviceWrite(data, sourceId?) -> { ok = true } | { ok = false, error = "..." }
```

- `data` is a Lua string (8-bit clean).
- `sourceId` is optional; default is the source the dataset belongs to.
- Synchronous, fire-and-forget. Does not throw. Calls are logged as
  `[deviceWrite] source=N bytes=M written=K`.

Use for **closed-loop control**: compute a setpoint from a sensor value and
push it back to the device on the same frame.

```lua
local kp = 4.0
function transform(temperature)
  local sp = tableGet("Control", "setpoint") or 25.0
  local pwm = math.max(0, math.min(255, kp * (sp - temperature) + 128))
  deviceWrite(string.format("PWM=%d\n", math.floor(pwm + 0.5)))
  return temperature
end
```

Transforms run on every frame, so be conservative: latch repeated actions
with a local flag, or rate-limit with a counter, to avoid saturating the
link.

## Dashboard controls

Seven UI helpers, all returning `{ ok = true }` or `{ ok = false, error = "..." }`. None of them log. Latch with a local flag so each call fires once per state transition, not every frame.

```lua
clearPlots()                          -- wipe plot/multiplot/FFT/GPS/3D/waterfall
setPlotPoints(n)                      -- horizontal sample window (n >= 1)
setTerminalVisible(visible)           -- bool
setNotificationLogVisible(visible)    -- bool
setClockVisible(visible)              -- bool
setStopwatchVisible(visible)          -- bool
setActiveWorkspace(idOrName)          -- workspaceId (int >= 1000) OR title
```

Example: any reading at the device's reboot sentinel value (>= 9999) wipes the plot history so the new boot draws cleanly.

```lua
function transform(value)
  if value >= 9999 then
    clearPlots()
    return 0
  end
  return value
end
```

These affect the active dashboard window only. They do NOT modify the project file or the user's persisted preferences.

## Errors

A Lua error logs a watchdog warning and the raw value is used. Returning a
non-number falls back silently. Don't rely on errors for control flow.
