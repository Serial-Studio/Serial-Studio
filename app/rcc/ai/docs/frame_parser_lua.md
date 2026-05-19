# Frame Parser API — Lua

Lua 5.4 mirror of the JavaScript frame-parser API. Functionally identical to
the JS version; choose Lua when your team is already Lua-fluent or when
you want Lua's table semantics for batched parsing.

## Contract

```lua
function parse(frame, separator)
  -- returns array-like table of numbers / strings
end
```

- `frame` is a Lua string (one logical frame).
- `separator` is the source's configured CSV separator string.
- Return a sequence (1-indexed table) whose entries map positionally to
  datasets.
- Return `{}` to skip a frame silently.

## Compatibility shim (LuaCompat)

Serial Studio injects a small compatibility layer so scripts written
against Lua 5.1/5.2 conventions still work:

- `math.log10(x)` — alias for `math.log(x, 10)`.
- `math.pow(a, b)` — alias for `a ^ b`.
- `bit32` — full library (band, bor, bxor, bnot, lshift, rshift, arshift,
  rrotate, lrotate, extract, replace).
- `unpack(t)` — alias for `table.unpack(t)`.

## Sandbox

- No `io.*`, no `os.execute`, no `package.loadlib`, no filesystem.
- `require` is disabled. Single-file scripts.
- Top-level `local` declarations stay scoped to your script (each source
  has its own Lua state).

## Performance

Native Lua is fast. Avoid building full string tables when an in-place
loop will do; avoid `string.format` in the hot path; precompute regex
patterns at script-top-level (they capture as upvalues).

## Examples

### CSV split

```lua
function parse(frame, separator)
  local out = {}
  for value in string.gmatch(frame, "([^" .. separator .. "]+)") do
    out[#out + 1] = tonumber(value) or value
  end
  return out
end
```

### Pattern extraction

```lua
local pattern = "^%$(%a+),(-?%d+%.%d+),(-?%d+%.%d+),(-?%d+)"

function parse(frame)
  local _, lat, lon, sats = string.match(frame, pattern)
  if not lat then
    return {}
  end
  return { tonumber(lat), tonumber(lon), tonumber(sats) }
end
```

### Bit-extraction with bit32

```lua
function parse(frame)
  local b = string.byte(frame, 1)
  return {
    bit32.band(b, 0x0F),
    bit32.rshift(b, 4),
  }
end
```

### Binary frame (escape with `string.byte`)

```lua
function parse(frame)
  if #frame < 8 then return {} end
  local x = string.byte(frame, 1) * 256 + string.byte(frame, 2)
  local y = string.byte(frame, 3) * 256 + string.byte(frame, 4)
  return { x, y }
end
```

## Device output — `deviceWrite()`

```lua
deviceWrite(data, sourceId?) -> { ok = true } | { ok = false, error = "..." }
```

- `data` is a Lua string (raw bytes are fine — Lua strings are 8-bit clean).
- `sourceId` is optional; default is the source this parser belongs to.
- Synchronous, fire-and-forget. Does not throw. Calls are logged as
  `[deviceWrite] source=N bytes=M written=K`.

Use for closed-loop control from inside `parse()`: ack a frame, raise an
alarm, request a status push. NOT for user-triggered commands — those
belong on an Output Widget.

```lua
function parse(frame)
  local v = tonumber(frame)
  if v and v > 80 then
    deviceWrite("ALARM=1\n")
  end
  return { v }
end
```

## Firing project actions — `actionFire()`

```lua
actionFire(actionId) -> { ok = true } | { ok = false, error = "..." }
```

Reuses an existing project Action (with its prebuilt payload + timer
mode). `actionId` is the action's stable id, NOT its index in the list.
Same shape as `deviceWrite`; never throws.

## Dashboard controls

Seven UI helpers, all returning `{ ok = true }` or `{ ok = false, error = "..." }`. None of them log. Call once on a state transition, NOT on every frame.

```lua
clearPlots()                          -- wipe line/multiplot/FFT/GPS/3D/waterfall buffers
setPlotPoints(n)                      -- horizontal sample window (n >= 1)
setTerminalVisible(visible)           -- bool
setNotificationLogVisible(visible)    -- bool
setClockVisible(visible)              -- bool
setStopwatchVisible(visible)          -- bool
setActiveWorkspace(idOrName)          -- workspaceId (int >= 1000) OR title (case-insensitive)
```

Typical use: reset a GPS trace the moment a valid fix arrives so the line from origin to first real sample disappears.

```lua
local hadFix = false
function parse(frame)
  local lat, lon, q = frame:match("([^,]+),([^,]+),([^,]+)")
  q = tonumber(q) or 0
  if not hadFix and q > 0 then
    clearPlots()
    hadFix = true
  end
  return { tonumber(lat), tonumber(lon), q }
end
```

These affect the active dashboard window only. They do NOT modify the project file or the user's persisted preferences.

## Errors

A Lua error logs a watchdog warning and skips the frame. The Lua state is
preserved across frames; module-level state survives until the source is
reconfigured.

## Choosing JS vs Lua

The two engines have parity for parser scripts. JS gets you a familiar
syntax and good regex; Lua gets you cheaper allocations on hot paths and
slightly faster startup. Pick one per source — you can mix engines across
sources in the same project.
