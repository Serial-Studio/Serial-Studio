# Frame Parser API (Lua)

LuaJIT (Lua 5.1 syntax and semantics) mirror of the JavaScript frame-parser
API. Functionally identical to the JS version; choose Lua when your team is
already Lua-fluent or when you want Lua's table semantics for batched parsing.

Not available (syntax errors or nil): integer division `//`, the bitwise
operators `& | ~ << >>` (use `bit.band`/`bit.bor`/`bit.bxor`/`bit.lshift`/
`bit.rshift`), `string.pack`/`string.unpack`, `utf8`, `table.move`,
`math.tointeger`, and `os`. There is no integer subtype: every number is a
double.

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

- No `io.*`, no `os` at all (no `os.time`/`os.clock`; a restricted `os` exists
  only for MQTT publisher scripts), no `package.loadlib`, no filesystem.
  Only `base`, `table`, `string`, `math` and `bit` are loaded.
- `require` is disabled. Single-file scripts.
- Top-level `local` declarations stay scoped to your script (each source
  has its own Lua state).

## Variables tables

The shared-table API is injected into the parser too: `tableGet(table, reg)` /
`tableSet(table, reg, v)` by name, plus the handle fast path `tableHandle(table, reg)` /
`tableHandleMany(table, regs)` / `tableGetH(handle)` / `tableSetH(handle, v)`. For a parser that
writes many variables per frame, resolve handles once in a top-level `local` and use
`tableGetH`/`tableSetH` on the hot path; a stale handle (after a table-definition edit) is a safe
no-op. A table inside folders is addressed by its full `/`-joined folder path (top-level tables
are bare names); `project.dataTable.list` reports that path. Full reference in the transform and
SDK docs.

## Checksums

`checksum(algorithm, data)` returns the digest as a byte string in wire order (`CRC-16-MODBUS`
little-endian, the rest big-endian) and `checksumHex(algorithm, data)` the same as lowercase hex;
`checksumAlgorithms()` lists the names (`XOR-8`, `MOD-256`, `CRC-8`, `CRC-16`, `CRC-16-MODBUS`,
`CRC-16-CCITT`, `Fletcher-16`, `CRC-32`, `Adler-32`). `data` is a string or a table of byte
values; an unknown algorithm raises a Lua error. A trailer check is one line:
`if checksumHex("XOR-8", body) ~= trailer:lower() then return {} end`.

## Notifications (Pro)

`notify`, `notifyInfo`, `notifyWarning`, `notifyCritical` and `notifyClear` (with
the `Info`/`Warning`/`Critical` constants) are Pro-only. Without a Pro license or
trial the Lua calls are silent no-ops (one warning is logged per session).

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

There is no `string.unpack`. Decode multi-byte integers from the byte table
with the `bit` library and arithmetic (little-endian signed int16 from bytes 5
and 6 shown):

```lua
function parse(frame)
  if #frame < 6 then return {} end
  local raw = bit.lshift(frame[6], 8) + frame[5]
  if raw >= 32768 then raw = raw - 65536 end
  return { raw }
end
```

A float32 field needs a manual IEEE-754 decode (big-endian shown; reverse the
byte order for little-endian):

```lua
local function readFloat32(b1, b2, b3, b4)
  local sign = bit.band(b1, 0x80) ~= 0 and -1 or 1
  local expo = bit.band(b1, 0x7F) * 2 + bit.rshift(b2, 7)
  local mant = bit.band(b2, 0x7F) * 65536 + b3 * 256 + b4
  if expo == 0 then return sign * mant * 2 ^ (-126 - 23) end
  return sign * (1 + mant / 2 ^ 23) * 2 ^ (expo - 127)
end

function parse(frame)
  if #frame < 4 then return {} end
  return { readFloat32(frame[1], frame[2], frame[3], frame[4]) }
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
