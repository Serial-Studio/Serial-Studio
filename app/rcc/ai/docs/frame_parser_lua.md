# Frame Parser API (Lua)

Lua 5.4 mirror of the JavaScript frame-parser API. Functionally identical to
the JS version; choose Lua when your team is already Lua-fluent or when
you want Lua's table semantics for batched parsing.

## Contract

You define a single function taking exactly **one** parameter:

```lua
function parse(frame)
  -- returns array-like table of numbers / strings
end
```

- `frame` is a Lua string (one logical frame) with the PlainText,
  Hexadecimal, or Base64 decoder; with the Binary decoder it is a
  1-indexed table of byte integers (0–255), not a string.
- **Never write `parse(frame, separator)`.** That two-parameter signature is
  a Serial Studio v2 relic, removed years ago and unsupported. There is no
  separator argument — split the frame yourself with the delimiter your
  protocol uses (e.g. `frame:split(",")`).
- Return a sequence (1-indexed table) whose entries map positionally to
  datasets.
- Return `{}` to skip a frame silently.

## Compatibility shim (LuaCompat)

Serial Studio injects a small compatibility layer so scripts written
against Lua 5.1/5.2 conventions still work:

- `math.log10(x)`: alias for `math.log(x, 10)`.
- `math.pow(a, b)`: alias for `a ^ b`.
- `bit32`: full library (band, bor, bxor, bnot, lshift, rshift, arshift,
  rrotate, lrotate, extract, replace).
- `unpack(t)`: alias for `table.unpack(t)`.
- `string.split(s, sep)`: **native** split on the literal separator `sep`,
  keeping empty fields (`"a,,b"` -> `{"a", "", "b"}`) — same semantics as
  JavaScript's `String.split`. Runs in C; prefer it over a hand-written
  `gmatch`/`find` loop for delimited frames (it is the fast path for CSV-style
  data). `sep` is a literal, not a Lua pattern, so `"|"`, `"."`, etc. are safe.
- `string.trim(s)`, `string.startswith(s, p)`, `string.endswith(s, p)`,
  and `string.gfind` (alias for `string.gmatch`).

## Sandbox

- No `io.*`, no `os.execute`, no `package.loadlib`, no filesystem.
- `require` is disabled. Single-file scripts.
- Top-level `local` declarations stay scoped to your script (each source
  has its own Lua state).

## Data tables

The shared data-table API is injected into the parser too: `tableGet(table, reg)` /
`tableSet(table, reg, v)` by name, plus the handle fast path `tableHandle(table, reg)` /
`tableHandleMany(table, regs)` / `tableGetH(handle)` / `tableSetH(handle, v)`. For a parser that
writes many registers per frame, resolve handles once in a top-level `local` and use
`tableGetH`/`tableSetH` on the hot path; a stale handle (after a table-definition edit) is a safe
no-op. A table inside folders is addressed by its full `/`-joined folder path (top-level tables
are bare names); `project.dataTable.list` reports that path. Full reference in the transform and
SDK docs.

## Console logging

`print(...)` and a JS-style `console` table route to the application
console — the same sink as JS `console.log`. Script log lines are also
mirrored to stdout by the application's message handler.

```lua
print("frame:", frame)          -- debug level
console.log("parsed", n)        -- debug
console.debug("raw byte", b)    -- debug
console.info("fix acquired")    -- info
console.warn("crc mismatch")    -- warning (notifies only if "Route warnings
                                -- to notifications" is enabled; off by default)
console.error("bad header")     -- critical (always raises an app notification)
```

Arguments are joined tab-separated; `__tostring` metamethods are honored.
Logging formats and allocates on every call, so gate per-frame logging
behind a condition while debugging and remove it from parsers that run at
kHz rates.

## Performance

Native Lua is fast, but a hand-written split loop runs as interpreted
bytecode. For delimited frames use the native `frame:split(sep)` (see the
compatibility shim) instead of a `gmatch`/`find` loop — it does the whole
scan in C and matches JavaScript's native `split`. Avoid `string.format` in
the hot path; precompute regex patterns at script-top-level (they capture as
upvalues).

## Examples

### CSV split

```lua
function parse(frame)
  return frame:split(",")
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

### Binary frame (index the byte table)

With the Binary decoder, `frame` is a 1-indexed table of byte integers —
index it directly (`string.byte(frame, ...)` errors on a table):

```lua
function parse(frame)
  if #frame < 4 then return {} end
  local x = frame[1] * 256 + frame[2]
  local y = frame[3] * 256 + frame[4]
  return { x, y }
end
```

`string.unpack` (floats, multi-byte integers, endianness) needs a string —
convert the table once at the top of `parse()` (one small allocation per
frame; frames are short):

```lua
function parse(frame)
  if type(frame) == "table" then
    frame = string.char(table.unpack(frame))
  end
  if #frame < 6 then return {} end
  return { string.unpack("<f", frame, 1), string.unpack("<i2", frame, 5) }
end
```

## Device output: `deviceWrite()`

```lua
deviceWrite(data, sourceId?) -> { ok = true } | { ok = false, error = "..." }
```

- `data` is a Lua string (raw bytes are fine, Lua strings are 8-bit clean).
- `sourceId` is optional; default is the source this parser belongs to.
- Synchronous, fire-and-forget. Does not throw. Calls are logged as
  `[deviceWrite] source=N bytes=M written=K`.

Use for closed-loop control from inside `parse()`: ack a frame, raise an
alarm, request a status push. NOT for user-triggered commands; those
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

## Firing project actions: `actionFire()`

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
slightly faster startup. Pick one per source; you can mix engines across
sources in the same project.
