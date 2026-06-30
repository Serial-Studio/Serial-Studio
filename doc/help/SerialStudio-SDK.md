# SDK Reference

Every script you write inside Serial Studio (a frame parser, a dataset transform, a Control Loop, an Output widget) runs with the **Serial Studio SDK** already loaded. The SDK is a set of natural, namespaced functions that let a script reach back into the application: write to the connected device, read the latest frame, move values through the shared data tables, fire actions, post notifications, reshape the dashboard, and encode wire-level protocol frames. You call these functions directly, with positional arguments, and the code editor offers them all in autocomplete.

This page is the single reference for that surface. The scripting pages ([Frame Parser Reference](JavaScript-API.md), [Dataset Value Transforms](Dataset-Transforms.md), [Control Loop](Control-Script.md), [Output Controls](Output-Controls.md)) describe *where* and *when* each script runs; this page describes *what they can call* once they are running.

## What the SDK is

The SDK is one auto-generated file per language, `SerialStudio.js` and `SerialStudio.lua`, evaluated into every script engine at startup. It is a thin, friendly layer over a single primitive, `apiCall(method, params)`, which routes to Serial Studio's command registry: the very same command surface the [network API](API-Reference.md) exposes over TCP. So `io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex)` is just a readable spelling of:

```javascript
apiCall("io.ble.writeCharacteristic",
        { characteristicUuid: "fff1", data: "<base64 of 5600030C>" });
```

You almost never call `apiCall` by hand. You call the wrapper, pass plain values, and the SDK builds the parameter object, base64-encodes any byte payload, and returns the result.

Two consequences follow from "the SDK is the API, in-process":

- **Anything the network API can do, a script can do**: connect a source, edit the project, start an export, query the dashboard. The full command catalog is the [API Reference](API-Reference.md); this page covers the ergonomic helpers layered on top.
- **The same `{ ok, result, error }` discipline applies.** Every call can fail (a wrong UUID, a disconnected device, a Pro-only command on a Free build), so you check the result rather than assume success.

## The result envelope

Every SDK call that reaches the command registry returns the same object. Check `ok` before trusting `result`.

| Field | When set | Meaning |
|---|---|---|
| `ok` | always | `true` on success, `false` otherwise. |
| `result` | success | The command-specific payload. Absent when the command returns nothing. |
| `error` | failure | Human-readable message. |
| `errorCode` | failure | Machine-readable code (`INVALID_PARAM`, `MISSING_PARAM`, `UNKNOWN_COMMAND`, ...). |

```javascript
var r = io.getStatus();
if (!r.ok) {
  console.log("getStatus failed: " + r.error);
  return;
}
if (r.result.isConnected)
  io.writeData("PING\n", SerialStudio.Text);
```

The pure helpers (the protocol encoders `modbusWriteRegister`, `canSendFrame`, and `delay()`) do not reach the registry and so do not return an envelope; they return their value directly. The table below notes which is which.

## Byte payloads and the encoding enum

Any argument that carries **bytes to the device** takes a trailing encoding constant so you never base64-encode by hand:

| Constant | Value | You pass | Example |
|---|---|---|---|
| `SerialStudio.Hex` | 1 | a hex string | `"5600030C"` |
| `SerialStudio.Text` | 2 | a UTF-8 string | `"READ\r\n"` |
| `SerialStudio.Base64` | 0 | a base64 string | `"VgADDA=="` |

```javascript
io.writeData("READ\r\n", SerialStudio.Text);             // text bytes
io.writeData("DEADBEEF", SerialStudio.Hex);              // 4 raw bytes
io.ble.writeCharacteristic("fff1", "01FF", SerialStudio.Hex);
```

The SDK converts to base64 internally (in pure JS/Lua, with no host dependency), because the wire format under the API is base64. You just declare which form your string is in.

## The call surface

### Core gateway

| Function | Returns | Notes |
|---|---|---|
| `apiCall(method, params)` | envelope | The primitive every wrapper routes through. Use it for commands that have no dedicated wrapper yet. |
| `apiCallList()` | array | Every command name available in the current engine. |
| `delay(ms)` | none | Blocks **only the calling worker thread**. Control Loop only (the frame-parser/transform hotpath has no `delay`). |

### Device I/O (`io.*`)

The wrappers mirror the bus tree. The ones you reach for most:

| Function | Purpose |
|---|---|
| `io.writeData(data, encoding)` | Write raw bytes to the active connection (any bus). |
| `io.getStatus()` | `{ isConnected, paused, ... }`. |
| `io.getLatestFrame(options?)` | The most recent raw frame (see [`newFrame()`](#see-decide-act-helpers) for the deduplicated control-loop form). |
| `io.connect()` / `io.disconnect()` | Open / close the configured connection. |
| `io.setPaused(paused)` | Pause/resume the data stream. |
| `io.ble.writeCharacteristic(uuid, data, encoding)` | Write to a specific BLE characteristic. |
| `io.ble.selectServiceByUuid(uuid)` / `io.ble.setNotifyCharacteristic(uuid)` | Wire up a BLE service/notify by UUID from a script. |

Every other `io.*` command (audio, CAN, Modbus, network, process, USB, HID configuration) is also a wrapper: type `io.` in the editor and autocomplete lists the namespace.

### `deviceWrite()` and `deviceWriteAndWait()`

`deviceWrite(data, sourceId?)` is the data-first form of `io.writeData`, available in parsers and transforms as well as the Control Loop. `sourceId` defaults to the first source.

`deviceWriteAndWait(data, timeoutMs, until, source?)` (Control Loop only) writes, then **blocks the worker thread** (never the GUI) until a reply satisfies `until` or `timeoutMs` elapses. `until` is a terminator string, an expected byte count (number), or `undefined` for the first non-empty reply. It returns `{ ok, data, bytesRead, timedOut }`, the clean way to drive a request/response instrument without juggling `newFrame()` state.

```javascript
var reply = deviceWriteAndWait("*IDN?\n", 1000, "\n");   // SCPI query
if (reply.ok && !reply.timedOut)
  console.log("Instrument: " + reply.data);
```

### Data tables (`tableGet` / `tableSet`)

The shared data tables are the cross-script blackboard (see [Data Tables](Data-Tables.md)): constants and computed registers that hold their value across frames. All four script kinds read and write them with the same two calls:

| Function | Returns | Notes |
|---|---|---|
| `tableGet(table, register)` | value or `undefined` | `tableGet(t, r) || fallback` works for missing registers. |
| `tableSet(table, register, value)` | none | Stages the new value into the live runtime. |
| `datasetGetRaw(uniqueId)` / `datasetGetFinal(uniqueId)` | value | Read another dataset's pre/post-transform value by unique id. |

In a Control Loop these calls marshal to the GUI thread, so each costs a round-trip: read once per `loop()` pass, not in a tight inner loop.

#### Handles: the fast path for table-heavy parsers

`tableGet`/`tableSet` look a register up by name on every call. For a parser that touches many registers every frame, resolving a name once and reusing a **handle** is much cheaper: a handle is a number that points straight at the register, with no name lookup on read or write.

| Function | Returns | Notes |
|---|---|---|
| `tableHandle(table, register)` | handle (number), or `-1` | Resolve once, at script load. `-1` means the register does not exist. |
| `tableHandleMany(table, registers)` | array of handles | Resolve several registers of one table in a single call. |
| `tableGetH(handle)` | value or `undefined` | Read by handle. A stale or invalid handle returns `undefined`. |
| `tableSetH(handle, value)` | none | Write by handle. A stale, invalid, or constant-register handle is ignored. |

Resolve handles **once**, in the top-level body of the script (which runs when the project loads), and keep them in a variable or array. Then use only `tableGetH`/`tableSetH` inside `parse()` / `transform()` / `loop()`:

```javascript
// Top level (runs once at load): resolve names to handles.
var H = tableHandleMany("DAQ/BRD-1", ["ch1", "ch2", "ch3", "ch4"]);

function parse(frame) {
  // Hot path: write by handle, no name lookup.
  for (var i = 0; i < H.length; i++)
    tableSetH(H[i], frame[i]);
  return [0];
}
```

If a handle comes back as `-1` at the top level, the data-table store was not built yet when the script first ran; resolve on the first `parse()` / `transform()` call instead (cache it in a variable, then reuse), which always runs after the store exists.

Handles are valid only while the project is loaded. If you edit the project's table definitions, the registers are rebuilt and old handles stop matching; the script re-resolves them automatically the next time it loads, and in the meantime a stale handle is a safe no-op (read returns `undefined`, write does nothing) rather than touching the wrong register. The handle calls are available to every script kind, including the Control Loop, where they still marshal to the GUI thread (so the round-trip rule above still applies; the win there is the elided name lookup, not the round-trip).

### Dashboard control

Available from any in-process script (the bridge is present in parsers, transforms, and the Control Loop):

| Function | Effect |
|---|---|
| `clearPlots()` | Wipe plot history. |
| `setPlotPoints(n)` | Set the visible plot window length. |
| `setTerminalVisible(v)` / `setNotificationLogVisible(v)` | Toggle dashboard panels. |
| `setClockVisible(v)` / `setStopwatchVisible(v)` | Toggle the clock / stopwatch. |
| `setActiveWorkspace(idOrName)` | Switch the active workspace. |

Control Loops additionally get the **See/Decide/Act** helpers, documented below.

### Actions

`actionFire(actionId)` triggers a configured [Action](Actions.md) (a saved one-shot command) by id. Use it to reuse a command you already defined in the UI instead of re-encoding the bytes in script.

### Notifications (Pro)

`notify(level, ...)`, `notifyInfo(...)`, `notifyWarning(...)`, `notifyCritical(...)`, and `notifyClear(...)` post to the [Notifications](Notifications.md) center. Each accepts `(title)`, `(title, subtitle)`, or `(channel, title, subtitle)`. `level` is `Info`, `Warning`, or `Critical` (globals `0/1/2`). On a Free build these throw with a licensing message, so guard with a Pro check if your script must run everywhere.

```javascript
notifyCritical("Watchdog", "No frames for 2 s");
notifyClear("Watchdog");   // resolve the same channel/title
```

### MQTT (Pro)

`mqttPublish(topic, payload, qos, retain)` publishes to the configured broker from inside a parser or transform. See [MQTT Publisher](MQTT-Publisher.md).

### Protocol encoders (pure helpers)

These build a wire-format byte string and **return it directly** (no envelope). They are how Output widgets and parsers craft Modbus/CAN frames without bit-twiddling. Pair them with `deviceWrite()` / `io.writeData(..., SerialStudio.Base64)` to send, or `return` them from an Output widget's `transmit()`.

| Function | Produces |
|---|---|
| `modbusWriteRegister(address, value)` | 16-bit holding-register write (4 bytes). |
| `modbusWriteCoil(address, on)` | Coil ON=`0xFF00` / OFF=`0x0000` (4 bytes). |
| `modbusWriteFloat(address, value)` | IEEE-754 float across two registers (6 bytes). |
| `canSendFrame(id, payload)` | CAN frame from a byte array or string payload. |
| `canSendValue(id, value, bytes?)` | CAN frame carrying a big-endian numeric value (default 2 bytes). |

## Where each surface lives

Not every helper exists in every engine; the SDK guards each block on whether the host installed the matching bridge. This table is the quick lookup. An absent function is undefined rather than throwing, so a `typeof` guard (JS) or `if name then` (Lua) keeps a shared snippet portable.

| Helper group | Frame parser | Transform | Control Loop | Output widget |
|---|---|---|---|---|
| `apiCall` / `apiCallList` | yes | yes | yes | no |
| `delay`, See/Decide/Act, `deviceWriteAndWait` | no | no | yes | no |
| `io.*`, `deviceWrite` | yes | yes | yes | no |
| `tableGet` / `tableSet`, dataset reads | yes | yes | yes | no |
| Dashboard control, `actionFire` | yes | yes | yes | no |
| `notify*`, `mqttPublish` (Pro) | yes | yes | yes | no |
| Protocol encoders (`modbus*`, `can*`) | yes | yes | yes | yes |

Output widget `transmit()` runs a smaller sandbox: it returns bytes for Serial Studio to send, so it gets the pure encoders but not the live `io.*`/table bridges. Language note: `transmit()` is **always JavaScript**; the Lua/JavaScript choice applies only to the frame parser.

## See/Decide/Act helpers (Control Loop)

The Control Loop adds four helpers for closed-loop automation: read the latest data, decide in plain code, act through the SDK. Full treatment is in the [Control Loop](Control-Script.md) page; the SDK surface is:

| Function | Returns | Purpose |
|---|---|---|
| `newFrame(sourceId?)` | frame or `null` | The latest frame, **once per arrival** (`null` if nothing new). Carries `text`, `values`, `sourceId`, `timestampMs`, `ageMs`, `sequence`. |
| `refreshDashboard()` | envelope | Re-run dataset transforms from the last values and repaint (**view only**, no export). |
| `dashboardTick()` | envelope | Same render, but also fans the synthesized frame to the export sinks (CSV/MDF4/Session/MQTT). Seeds structure before the first device frame. |
| `ensureDashboard(spec)` | summary | Declaratively create any group/dataset in `spec` that does not exist yet; existing items are untouched, and a repeated identical spec is free. |

## Worked examples

### 1. Control Loop: poll a SCPI instrument, no parser state machine

`deviceWriteAndWait` turns a request/response exchange into a single blocking call on the worker thread.

```javascript
function loop() {
  var r = deviceWriteAndWait("MEAS:VOLT:DC?\n", 1000, "\n");
  if (r.ok && !r.timedOut)
    tableSet("DMM", "voltage", parseFloat(r.data));   // feed a virtual dataset
  refreshDashboard();
  delay(500);                                          // ~2 Hz
}
```

### 2. Control Loop: adaptive sampling driven by the data

See the frame, decide in JavaScript, act through the SDK: speed up polling when the signal moves fast, slow down when it is quiet.

```javascript
var last = null;

function loop() {
  var f = newFrame();
  if (!f) { delay(50); return; }

  var v = parseFloat(f.values[0]);
  var fast = last !== null && Math.abs(v - last) > 1.0;
  last = v;

  if (fast && v > 90)
    notifyWarning("Telemetry", "Channel 1 high and rising: " + v.toFixed(1));

  io.writeData("READ\n", SerialStudio.Text);
  delay(fast ? 20 : 200);
}
```

### 3. Frame parser: close the loop from inside parse()

A parser is not write-only: after decoding the frame it can command the device. Here it parses two channels and, if temperature crosses a limit, fires a pre-configured cooling Action and trips a critical notification, exactly once per crossing.

```javascript
var hot = false;

function parse(frame) {
  var v = frame.split(",");          // "temp,rpm"
  var temp = parseFloat(v[0]);

  if (temp > 80 && !hot) {
    hot = true;
    actionFire("cooling_on");        // reuse an Action defined in the UI
    notifyCritical("Thermal", "Over-temp: " + temp.toFixed(1) + " C");
  } else if (temp < 70 && hot) {
    hot = false;
    notifyClear("Thermal");
  }

  return v;                          // datasets still get both channels
}
```

> Do the SDK call only on the **state transition** (the `if`), never on every frame: the parser is the hotpath. `apiCall` is rate-limited and a per-frame call will throttle or stall the stream.

### 4. Dataset transform: calibrate against a live table register

A transform reads a calibration slope another script wrote into a data table, so one place owns the constant and every channel stays in sync.

```javascript
function transform(value) {
  var slope = tableGet("Calibration", "adc_slope") || 1.0;   // missing -> 1.0
  return value * slope;
}
```

### 5. Output widget: a knob that drives a Modbus register

`transmit(value)` returns the wire bytes; the protocol encoder builds them. No `io.*` needed, since Serial Studio sends whatever you return.

```javascript
var SETPOINT_REGISTER = 0x0040;

function transmit(value) {
  // value is the knob position in its configured [Min, Max] range
  return modbusWriteFloat(SETPOINT_REGISTER, value);   // float over two registers
}
```

### 6. Output widget: a toggle that emits a CAN frame

```javascript
var RELAY_CAN_ID = 0x200;

function transmit(value) {
  // Toggle hands value as 0 or 1
  return canSendFrame(RELAY_CAN_ID, [0x01, value ? 0xFF : 0x00]);
}
```

### 7. Calling a command with no dedicated wrapper

When a helper does not exist for what you need, drop to `apiCall` with the raw method name and params. `apiCallList()` enumerates everything reachable in the current engine.

```javascript
var fps = apiCall("dashboard.getFps", {});
if (fps.ok)
  console.log("Dashboard at " + fps.result.fps + " FPS");
```

## Lua notes

The SDK is identical in spirit in Lua; only the spellings change:

- The envelope is a Lua table: `local r = io.getStatus(); if r.ok then ... end`.
- The encoding enum is `SerialStudio.Hex` / `.Text` / `.Base64`, same as JS.
- Wrappers are global functions with dotted names (`io.writeData`, `io.ble.writeCharacteristic`); optional arguments go in a trailing table.
- Bridge availability differs per engine; a portable snippet guards with `if tableGet then ... end`. (`delay`, the See/Decide/Act helpers, and `deviceWriteAndWait` are Control-Loop-only and JavaScript-only, since the Control Loop runs JavaScript.)

```lua
function transform(value)
  local slope = tableGet("Calibration", "adc_slope") or 1.0
  return value * slope
end
```

## How the SDK is built

`SerialStudio.js` and `SerialStudio.lua` are generated from the live command registry by `scripts/generate-sdk.py` (the schema snapshot lives in `app/rcc/api/api-schema.json`, refreshed with `SerialStudio --dump-api-schema`). The hand-written helpers (the dashboard/notification/encoder glue and the See/Decide/Act helpers) live in `app/rcc/api/prelude.js` and `prelude.lua` and are prepended to the generated wrappers. You never edit the SDK files directly; add a command to the registry (or the prelude) and regenerate. This is why every new API command shows up in scripts for free, with autocomplete, the moment it lands.

## Related

- [Frame Parser Reference](JavaScript-API.md): turning device bytes into dataset values; where `parse()` runs.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset conditioning; where `transform()` runs.
- [Control Loop](Control-Script.md): `setup()`/`loop()` automation and the See/Decide/Act model in depth.
- [Output Controls](Output-Controls.md): Button/Slider/Toggle/Knob widgets and their `transmit()` function.
- [Data Tables](Data-Tables.md): the shared blackboard behind `tableGet`/`tableSet`.
- [API Reference](API-Reference.md): the full command catalog the SDK wraps, and the same surface over the network.
- [Actions](Actions.md) and [Notifications](Notifications.md): what `actionFire()` and `notify*()` drive.
