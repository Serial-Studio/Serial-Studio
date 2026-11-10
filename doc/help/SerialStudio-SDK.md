# SDK Reference

Your board prints `23.5,1013` once a second, and you want Serial Studio to send `READ` back whenever the first value passes 90. That is a script calling the **Serial Studio SDK**:

```javascript
var sent = false;

function parse(frame) {
  var v = frame.split(",");
  var hot = parseFloat(v[0]) > 90;
  if (hot && !sent)
    io.writeData("READ\n", SerialStudio.Text);   // once per crossing, not on every frame
  sent = hot;
  return v;
}
```

Every script in a project has the SDK already loaded, so `io.writeData` needs no import. The script kinds are a [frame parser](Frame-Parser.md), a [dataset transform](Dataset-Transforms.md), a [Control Loop](Control-Loop.md) (a script that runs on its own timer) and an [Output widget](Output-Controls.md)'s `transmit()`. You write them in the [Project Editor](Project-Editor.md); the Control Loop has its own node under **Project Scripts**. Plain plotting needs no script at all, so skip this page until you want to write to the device or react to a value.

Pro covers `notify*`, `mqttPublish`, dataset transforms, Variables tables (`tableGet`/`tableSet` and handles) and Output widgets (`transmit()`). Frame parsers, the Control Loop, `io.*`, `deviceWrite`, the encoders and the checksums are free. See [Pro vs Free](Pro-vs-Free.md).

## The result envelope

Every SDK call that reaches the command registry returns the same object. Check `ok` before trusting `result`.

| Field | When set | Meaning |
|---|---|---|
| `ok` | always | `true` on success, `false` otherwise. |
| `result` | success | The command-specific payload. Absent when the command returns nothing. |
| `error` | failure | Human-readable message. |
| `errorCode` | failure | Machine-readable code, for example `INVALID_PARAM`, `MISSING_PARAM`, `UNKNOWN_COMMAND` or `METHOD_NOT_ALLOWED`. |

```javascript
var r = io.getStatus();
if (!r.ok) {
  console.log("getStatus failed: " + r.error);
  return;
}
if (r.result.isConnected)
  io.writeData("PING\n", SerialStudio.Text);
```

The protocol encoders and the checksum helpers never reach the registry, so they return their value directly instead of an envelope. `deviceWriteAndWait()` returns its own `{ ok, ... }` object (see [`deviceWrite()` and `deviceWriteAndWait()`](#devicewrite-and-devicewriteandwait)).

## Byte payloads and the encoding enum

Any argument that carries **bytes to the device** takes a trailing encoding constant so you never base64-encode by hand:

| Constant | Value | You pass | Example |
|---|---|---|---|
| `SerialStudio.Text` | 2 | a UTF-8 string | `"READ\r\n"` |
| `SerialStudio.Hex` | 1 | a hex string | `"5600030C"` |
| `SerialStudio.Base64` | 0 | a base64 string | `"VgADDA=="` |

```javascript
io.writeData("READ\r\n", SerialStudio.Text);             // text bytes
io.writeData("DEADBEEF", SerialStudio.Hex);              // 4 raw bytes
io.ble.writeCharacteristic("fff1", "01FF", SerialStudio.Hex);
```

## How a wrapper maps to a command

The SDK is one generated file per language, `SerialStudio.js` and `SerialStudio.lua`, loaded into every script engine at startup. It wraps a single primitive, `apiCall(method, params)`, which calls the same command registry the [network API](API-Reference.md) exposes over TCP. So `io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex)` is a readable spelling of:

```javascript
apiCall("io.ble.writeCharacteristic",
        { characteristicUuid: "fff1", data: "<base64 of 5600030C>" });
```

Any command in the [API Reference](API-Reference.md) works from a script. Every call can fail (a wrong UUID, a disconnected device, a Pro-only command on a Free build), so check `ok`. `apiCall` has no allow-list, rate limit or payload cap.

## The result envelope

Every SDK call that reaches the command registry returns the same object. Check `ok` before trusting `result`.

| Field | When set | Meaning |
|---|---|---|
| `ok` | always | `true` on success, `false` otherwise. |
| `result` | success | The command-specific payload. Absent when the command returns nothing. |
| `error` | failure | Human-readable message. |
| `errorCode` | failure | Machine-readable code, for example `INVALID_PARAM`, `MISSING_PARAM`, `UNKNOWN_COMMAND` or `METHOD_NOT_ALLOWED`. |

```javascript
var r = io.getStatus();
if (!r.ok) {
  console.log("getStatus failed: " + r.error);
  return;
}
if (r.result.isConnected)
  io.writeData("PING\n", SerialStudio.Text);
```

## The call surface

### Core gateway

| Function | Returns | Notes |
|---|---|---|
| `apiCall(method, params)` | envelope | The primitive every wrapper routes through. Use it for commands that have no dedicated wrapper yet. |
| `apiCallList()` | array | Every command name available in the current engine. |
| `delay(ms)` | none | Blocks the worker thread, not the GUI, so the dashboard keeps moving while your script waits on a slow instrument. Works in the Control Loop and [Macros](Macros.md); in a parser, transform or Output widget it is defined but does nothing. |

### Device I/O (`io.*`)

The wrappers mirror the bus tree. The ones you reach for most:

| Function | Purpose |
|---|---|
| `io.writeData(data, encoding)` | Write raw bytes to the active connection (any bus). |
| `io.getStatus()` | `{ isConnected, paused, ... }`. |
| `io.getLatestFrame(options?)` | The most recent raw frame (see [`newFrame()`](#seedecideact-helpers) for the deduplicated once-per-arrival form). |
| `io.connect()` / `io.disconnect()` | Open / close the configured connection. |
| `io.setPaused(paused)` | Pause/resume the data stream. |
| `io.ble.writeCharacteristic(uuid, data, encoding)` | Write to a specific BLE characteristic. |
| `io.ble.selectServiceByUuid(uuid)` / `io.ble.setNotifyCharacteristic(uuid)` | Wire up a BLE service/notify by UUID from a script. |

Every other `io.*` command (audio, CAN, Modbus, network, process, USB, HID configuration) is also a wrapper: type `io.` in the editor and autocomplete lists the namespace.

### `deviceWrite()` and `deviceWriteAndWait()`

`deviceWrite(data, sourceId?)` is the data-first form of `io.writeData`. It exists in the frame parser, transforms and Output widgets, but **not in the Control Loop**, which uses `io.writeData` instead (see [Where each surface lives](#where-each-surface-lives)). `sourceId` defaults to the first source.

`deviceWriteAndWait(data, timeoutMs, until, source?)` (Control Loop and [Macros](Macros.md); elsewhere it returns `{ ok: false, error: "unsupported here" }`) writes, then **blocks the worker thread** (never the GUI) until a reply satisfies `until` or `timeoutMs` elapses. `timeoutMs` is clamped to 0..30000. `source` defaults to `0`. `until` is a terminator string, an expected byte count (number), or `undefined` for the first non-empty reply. It returns `{ ok, data, bytesRead, timedOut }`, or `{ ok: false, error }` when the write fails. It is the clean way to drive a request/response instrument without juggling `newFrame()` state.

```javascript
var reply = deviceWriteAndWait("*IDN?\n", 1000, "\n");   // SCPI query
if (reply.ok && !reply.timedOut)
  console.log("Instrument: " + reply.data);
```

### Variables tables (`tableGet` / `tableSet`)

The Variables tables are the cross-script blackboard (see [Variables](Variables.md)): constants and computed variables that hold their value across frames. All four script kinds read and write them with the same two calls:

| Function | Returns | Notes |
|---|---|---|
| `tableGet(table, variable)` | value or `undefined` | `tableGet(t, r) || fallback` works for missing variables. |
| `tableSet(table, variable, value)` | none | Stages the new value into the live runtime. An undefined/nil value is ignored. |
To read another dataset, `datasetGetRaw(uniqueId | alias)` and `datasetGetFinal(uniqueId | alias)` return its pre- and post-transform value. A number argument is a uniqueId; a string argument is the dataset's alias. They are not available in the Control Loop.

In a Control Loop these calls marshal to the GUI thread, so each costs a round-trip: read once per `loop()` pass, not in a tight inner loop.

### Dashboard control

Available in the frame parser, transforms and Output widgets. The Control Loop reaches the same commands through `apiCall("dashboard....")`; the direct functions below do not exist there.

| Function | Effect |
|---|---|
| `clearPlots()` | Wipe plot history. |
| `setPlotPoints(n)` | Set the visible plot window length. |
| `setTerminalVisible(v)` / `setNotificationLogVisible(v)` | Toggle dashboard panels. |
| `setClockVisible(v)` / `setStopwatchVisible(v)` | Toggle the clock / stopwatch. |
| `setActiveWorkspace(idOrName)` | Switch the active workspace. |

The JavaScript-only [See/Decide/Act helpers](#seedecideact-helpers) are available in every script kind.

### Actions

`actionFire(actionId)` triggers a configured [Action](Actions.md) (a saved one-shot command) by id. Use it to reuse a command you already defined in the UI instead of re-encoding the bytes in script. It is not available in the Control Loop; there, call `apiCall` with the method name from the [API Reference](API-Reference.md).

### Notifications (Pro)

`notify(level, ...)`, `notifyInfo(...)`, `notifyWarning(...)`, `notifyCritical(...)`, and `notifyClear(...)` post to the [Notifications](Notifications.md) center. Each accepts `(title)`, `(title, subtitle)`, or `(channel, title, subtitle)`. `level` is `Info`, `Warning`, or `Critical` (globals `0/1/2`). With one or two arguments the channel is `Dashboard`.

On a build without Pro (see [Pro vs Free](Pro-vs-Free.md)) the result depends on the script kind:

- JavaScript frame parser, transform and Output widget: the call throws `Error("notify() requires a Pro license. ...")`. Wrap it in `try`/`catch` if the script must run everywhere.
- Control Loop: the call returns `{ ok: false }`, so check `ok`.
- Lua: the call does nothing and logs a warning once.

```javascript
notifyCritical("Watchdog", "No frames for 2 s");
notifyClear("Watchdog");   // clears the Watchdog title on the Dashboard channel
```

### MQTT (Pro)

`mqttPublish(topic, payload, qos, retain)` publishes to the configured broker from the frame parser, transforms and Output widgets. It does not exist in the Control Loop. See [MQTT Publisher](MQTT-Publisher.md) and [Pro vs Free](Pro-vs-Free.md).

### Protocol encoders (pure helpers)

These build a wire-format byte string and **return it directly** (no envelope). They are how Output widgets and parsers craft Modbus/CAN frames without bit-twiddling. Pair them with `deviceWrite()` / `io.writeData(..., SerialStudio.Base64)` to send, or `return` them from an Output widget's `transmit()`.

| Function | Produces |
|---|---|
| `modbusWriteRegister(address, value, unit?)` | 16-bit holding-register write (4 bytes). `value` is rounded and truncated to 16 bits. |
| `modbusWriteRegisters(address, values, unit?)` | Several consecutive registers: the address, then 2 bytes per value. |
| `modbusWriteCoil(address, on, unit?)` | Coil ON=`0xFF00` / OFF=`0x0000` (4 bytes). |
| `modbusWriteFloat(address, value, unit?)` | Big-endian IEEE-754 float across two registers (6 bytes). |
| `canSendFrame(id, payload)` | CAN frame from a byte array or string payload. An `id` up to `0x7FF` is a standard frame: 2 id bytes, a length byte, the payload. A larger `id` is a 29-bit extended frame: 4 id bytes (the first has bit `0x80` set), a length byte, the payload. An extended `id` is masked to 29 bits. The payload length is not checked: you supply at most 8 bytes. |
| `canSendValue(id, value, bytes?)` | CAN frame carrying a big-endian numeric value, rounded to an integer (default 2 bytes; above 4 the high bytes come out wrong). |

The optional `unit` (1 to 247) adds a 3-byte `FF 83 <unit>` prefix that sends the write to that Modbus unit instead of the connection's own. An out-of-range `unit` is ignored. The byte counts above are without the prefix.

### Checksums (pure helpers)

The frame reader's checksum library is exposed to every engine, so a script never carries its own CRC table. `data` is a binary string (one character per byte, the same convention the encoders above return) or an array of byte values; in Lua a plain string is already bytes, and a table of numbers works too.

| Function | Produces |
|---|---|
| `checksum(algorithm, data)` | The digest bytes as a binary string in wire order: `CRC-16-MODBUS` little-endian, every other multi-byte algorithm big-endian. Append it to a payload as-is. |
| `checksumHex(algorithm, data)` | The same digest as lowercase hex, for comparing against a text frame's trailer. |
| `checksumAlgorithms()` | The algorithm names: `XOR-8`, `MOD-256`, `CRC-8`, `CRC-16`, `CRC-16-MODBUS`, `CRC-16-CCITT`, `Fletcher-16`, `CRC-32`, `Adler-32`. |

An unknown algorithm or a `data` argument that is neither a string nor an array raises an error in the script (a `TypeError` in JavaScript, a Lua error in Lua). Data longer than 1 MiB (1048576 bytes) is rejected the same way.

```js
// Output widget: Modbus RTU write with its CRC appended
function transmit(value) {
  var pdu = String.fromCharCode(0x01, 0x06) + modbusWriteRegister(0x10, value);
  return pdu + checksum('CRC-16-MODBUS', pdu);
}
```

```lua
-- Frame parser: drop a frame whose "*XX" trailer does not match
function parse(frame)
  local body, trailer = frame:match("^(.+)%*(%x%x)$")
  if not body or checksumHex("XOR-8", body) ~= trailer:lower() then return {} end
  return body:split(",")   -- string.split is built into every Lua script engine
end
```

## Where each surface lives

A function that a script kind does not support is undefined instead of throwing, so a `typeof` guard (JS) or `if name then` (Lua) keeps a shared snippet portable. `delay` and `deviceWriteAndWait` are the exception: they are always defined, so a guard on them is always true.

| Helper group | Frame parser | Transform | Control Loop | Output widget |
|---|---|---|---|---|
| `apiCall` / `apiCallList` | yes | yes | yes | yes |
| `delay` | no-op | no-op | yes | no-op |
| `deviceWriteAndWait` | returns `{ ok: false }` | returns `{ ok: false }` | yes | returns `{ ok: false }` |
| See/Decide/Act (JavaScript only) | yes | yes | yes | yes |
| `io.*` | yes | yes | yes | yes |
| `deviceWrite` | yes | yes | no | yes |
| `tableGet` / `tableSet`, handles | yes | yes | yes (marshalled) | yes |
| `datasetGetRaw` / `datasetGetFinal` | yes | yes | no | yes |
| Dashboard control, `actionFire` | yes | yes | no | yes |
| `notify*` (Pro) | yes | yes | yes (returns an envelope) | yes |
| `mqttPublish` (Pro) | yes | yes | no | yes |
| Protocol encoders (`modbus*`, `can*`) | yes | yes | yes | yes |
| Checksums (`checksum`, `checksumHex`, `checksumAlgorithms`) | yes | yes | yes | yes |

The Control Loop runs on its own worker thread, so it reaches the application only through `apiCall`, which is marshalled to the GUI thread. That is why its direct helpers are missing; use `io.writeData` and `apiCall` instead. [Macros](Macros.md) get the same worker-thread surface as the Control Loop.

### Output widget scripts

- `transmit()` runs from a user action on the widget (a knob turn, a button press), not from an arriving frame, so `newFrame()` has nothing new to read.
- `delay()` does nothing and `deviceWriteAndWait()` returns `{ ok: false, error: "unsupported here" }`.
- `transmit()` is always JavaScript. The Lua/JavaScript choice applies only to the frame parser.

## See/Decide/Act helpers

Four JavaScript-only helpers for closed-loop automation: read the latest data, decide in plain code, act through the SDK. They have no Lua equivalent. The [Control Loop](Control-Loop.md) page covers the model in full; the SDK surface is:

| Function | Returns | Purpose |
|---|---|---|
| `newFrame(sourceId?)` | frame or `null` | The latest frame, **once per arrival** (`null` if nothing new). Carries `text`, `values`, `sourceId`, `timestampMs`, `ageMs`, `sequence`. |
| `refreshDashboard()` | envelope | Re-run dataset transforms from the last values and repaint (**view only**, no export). |
| `dashboardTick()` | envelope | Same render, but also fans the synthesized frame to the export sinks (CSV/MDF4/Session/MQTT). Seeds structure before the first device frame. |
| `ensureDashboard(spec)` | summary | Declaratively create any group/dataset in `spec` that does not exist yet; existing items are untouched, and a repeated identical spec is free. |

## Worked examples

### 1. Control Loop: poll a SCPI instrument, no parser state machine

Paste into the **Control Loop** node under **Project Scripts**. `deviceWriteAndWait` turns a request/response exchange into a single blocking call on the worker thread.

```javascript
function loop() {
  var r = deviceWriteAndWait("MEAS:VOLT:DC?\n", 1000, "\n");
  if (r.ok && !r.timedOut)
    tableSet("DMM", "voltage", parseFloat(r.data));   // feed a computed dataset
  refreshDashboard();
  delay(500);                                          // about 2 Hz plus the reply wait
}
```

### 2. Control Loop: poll faster when the signal moves

Also a Control Loop script. See the frame, decide in JavaScript, act through the SDK: only the delay changes, so `READ` goes out every pass, quickly while the signal moves and slowly while it is quiet. `notifyWarning` needs Pro; on Free it returns `{ ok: false }` and the loop continues.

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

This goes in the frame parser. A parser is not write-only: after decoding the frame it can command the device. Here it parses two channels and, if temperature crosses a limit, fires a pre-configured cooling Action and trips a critical notification, exactly once per crossing.

```javascript
var hot = false;

function parse(frame) {
  var v = frame.split(",");          // "temp,rpm"
  var temp = parseFloat(v[0]);

  if (temp > 80 && !hot) {
    hot = true;
    actionFire(7);                    // 7 stands for the actionId of an Action you defined
    notifyCritical("Thermal", "Over-temp: " + temp.toFixed(1) + " C");
  } else if (temp < 70 && hot) {
    hot = false;
    notifyClear("Thermal");
  }

  return v;                          // datasets still get both channels
}
```

> `notifyCritical` and `notifyClear` need Pro. On a Free build they throw (see [Notifications (Pro)](#notifications-pro)), so wrap them in `try`/`catch` or leave them out.
>
> Call the SDK only on the **state transition** (the `if`), never on every frame. `parse()` runs on the acquisition pipeline's processing thread, so each `apiCall` adds time to every frame and can make the stream fall behind. There is no rate limit to save you, and a parser that fires on every frame is a quick way to build a busy pipeline.

### 4. Dataset transform: calibrate against a live table variable

In a dataset's transform, `transform(value)` reads a calibration slope another script wrote into a Variables table, so one place owns the constant and every channel stays in sync.

```javascript
function transform(value) {
  var slope = tableGet("Calibration", "adc_slope") || 1.0;   // missing -> 1.0
  return value * slope;
}
```

### 5. Output widget: a knob that drives a Modbus register

Needs Pro: Output widgets are a Pro feature (see [Output Controls](Output-Controls.md)). `transmit(value)` returns the wire bytes; the protocol encoder builds them. No `io.*` needed, since Serial Studio sends whatever you return.

```javascript
var SETPOINT_REGISTER = 0x0040;

function transmit(value) {
  // value is the knob position, within the widget's Min and Max
  return modbusWriteFloat(SETPOINT_REGISTER, value);   // float over two registers
}
```

### 6. Output widget: a toggle that emits a CAN frame

Needs Pro, like every Output widget.

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

Lua matches the JavaScript surface except for the points below.

- The envelope is a Lua table: `local r = io.getStatus(); if r.ok then ... end`.
- The encoding enum is `SerialStudio.Hex` / `.Text` / `.Base64`, the same constants as in JavaScript.
- Wrappers are global functions with dotted names (`io.writeData`, `io.ble.writeCharacteristic`); optional arguments go in a trailing table.
- The See/Decide/Act helpers (`newFrame`, `refreshDashboard`, `dashboardTick`, `ensureDashboard`) do not exist in Lua.
- The checksum helpers are native globals in every Lua engine: `checksum(algorithm, data)` returns the digest as a byte string (`s:byte(1, -1)` unpacks it), `checksumHex` the hex form, and `data` may be a string or a table of byte values.

```lua
function transform(value)
  local slope = tableGet("Calibration", "adc_slope") or 1.0
  return value * slope
end
```

## Handles: the fast path for table-heavy parsers

`tableGet` and `tableSet` look a variable up by name on every call. A **handle** is a number that points straight at the variable, so resolving a name once and reusing the handle skips the lookup on every read and write.

| Function | Returns | Notes |
|---|---|---|
| `tableHandle(table, variable)` | handle (number), or `-1` | Resolve once, at script load. `-1` means the variable does not exist. |
| `tableHandleMany(table, variables)` | array of handles | Resolve several variables of one table in a single call. |
| `tableGetH(handle)` | value or `undefined` | Read by handle. A stale or invalid handle returns `undefined`. |
| `tableSetH(handle, value)` | none | Write by handle. A stale, invalid, or constant-variable handle is ignored, as is an undefined/nil value. |

Resolve handles once, in the top-level body of the script (which runs when the project loads), and use only `tableGetH`/`tableSetH` inside `parse()` / `transform()` / `loop()`:

```javascript
var H = tableHandleMany("DAQ/BRD-1", ["ch1", "ch2", "ch3", "ch4"]);

function parse(frame) {
  var v = frame.split(",");
  for (var i = 0; i < H.length; i++)
    tableSetH(H[i], parseFloat(v[i]));
  return [0];   // the values live in the table, so the parser hands back one placeholder
}
```

If a handle is `-1` at the top level, the table store was not built yet when the script first ran. Resolve on the first `parse()` / `transform()` call instead and cache it. Handles are valid only while the project is loaded; after you edit the table definitions an old handle is a safe no-op. In the Control Loop each handle call still costs a round-trip to the GUI thread, so the saving there is the name lookup only.

## How the SDK is built

`SerialStudio.js` and `SerialStudio.lua` are generated from the live command registry by `scripts/generate-sdk.py`, with the hand-written helpers in `app/rcc/api/prelude.js` and `prelude.lua` prepended. Add a command to the registry and regenerate; scripts see it as a wrapper.

## Related

- [Frame Parser Reference](Frame-Parser.md): turning device bytes into dataset values; where `parse()` runs.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset conditioning; where `transform()` runs.
- [Control Loop](Control-Loop.md): `setup()`/`loop()` automation and the See/Decide/Act model in depth.
- [Output Controls](Output-Controls.md): Button/Slider/Toggle/Knob widgets and their `transmit()` function.
- [Variables](Variables.md): the shared blackboard behind `tableGet`/`tableSet`.
- [API Reference](API-Reference.md): the full command catalog the SDK wraps, and the same surface over the network.
- [Macros](Macros.md): the in-app terminal and script editor for running `apiCall` and the wrappers interactively, outside any project.
- [Actions](Actions.md) and [Notifications](Notifications.md): what `actionFire()` and `notify*()` drive.
