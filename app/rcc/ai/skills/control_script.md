# Control Script and the SerialStudio SDK

Serial Studio projects can carry an Arduino-style **control script** that
automates a connected device: a `setup()` function runs once when the device
connects, and a `loop()` function runs repeatedly while it stays connected.
This is the place to send wake-up handshakes, poll a device on a timer, or send
keep-alives. It is one script per project, runs on its own worker thread (off
the data hotpath), and is stored in the project file.

## When to reach for it

- A device only streams after a command handshake (many BLE probes, lab
  instruments). Put the handshake in `setup()`.
- A device must be polled or kept alive. Put the request in `loop()` and pace it
  with `delay(ms)`.
- The user says "send X when it connects", "poll the sensor", "wake up the
  probe", "start the measurement stream".
- The user wants widgets created automatically from incoming data ("build the
  dashboard from whatever arrives"). Use `newFrame()` + `ensureDashboard()`
  (below).
- The user wants a communication-loss watchdog ("show Disconnected when the
  device goes silent"). Use `io.getLatestFrame().result.ageMs` + `tableSet()` +
  `refreshDashboard()` (below).

## The SerialStudio SDK (natural API wrappers)

Every script engine (control script, frame parser, transforms) has a generated
`SerialStudio` library on top of the raw `apiCall(method, params)` gateway. It
turns verbose calls into natural, positional ones:

```javascript
// Instead of:
apiCall("io.ble.writeCharacteristic", { characteristicUuid: "fff1", data: "<base64>" });

// Write:
io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex);
```

Rules of the SDK:

- Commands are dotted namespaces: `io.writeData(...)`, `io.ble.writeCharacteristic(...)`,
  `controlScript.getStatus()`, etc.
- Required parameters are positional, in the order the command declares them.
- Optional parameters go in a trailing `{ }` object.
- Byte-payload parameters (`data`) take a trailing **encoding** argument:
  `SerialStudio.Hex`, `SerialStudio.Text`, or `SerialStudio.Base64`. The wrapper
  converts to base64 for you, so the user never writes base64 by hand.
- Every wrapper returns the same `{ ok, result, error, errorCode }` shape as
  `apiCall`. Check `ok`.

The SDK is generated from the live command registry by
`scripts/generate-sdk.py` (refreshed in `sanitize-commit`), reading
`app/rcc/api/api-schema.json`. The output `SerialStudio.js` / `SerialStudio.lua`
in `app/rcc/api/` is the single, authoritative listing of the scripting
surface. To refresh the schema after adding or changing commands, run the app
with `--dump-api-schema <file>`.

To confirm an exact signature before writing a script, fetch the generated SDK
source itself: `meta.fetchScriptingDocs{kind: 'sdk_js'}` (or `'sdk_lua'`). It
lists every callable wrapper, so you never have to guess a function name or
argument order. All SDK symbols also appear in the code-editor autocomplete.

## Reading data: newFrame(), refreshDashboard(), ensureDashboard() (control script ONLY)

These prelude helpers exist only inside the control script engine (they are
gated on its bridge; frame parsers and transforms do not have them). Together
they close the See -> Decide -> Act loop:

- `newFrame(sourceId?)` -- returns the latest frame received from the device
  exactly once per arrival, `null` when nothing new. Fields: `text` (raw
  payload), `values` (the parser's channel tokens BEFORE dataset mapping, so it
  includes channels no dataset reads yet), `sourceId`, `timestampMs` (monotonic
  milliseconds, for deltas only, not Unix time), `ageMs` (milliseconds since
  the frame was captured: the staleness-watchdog primitive; never compare
  `timestampMs` against `Date.now()`), `sequence`. The raw command
  is `io.getLatestFrame` (`{encoding: "base64"}` for binary). `dashboard.getData` shows only mapped data; this is the
  pre-mapping view.
- `refreshDashboard()` -- re-runs every dataset transform from the last
  received values and republishes the frames to the dashboard (raw command:
  `dashboard.reprocess`; no export side effects, nothing lands in CSV/MDF4).
  Transforms normally run only when a device frame arrives, so `tableSet()`
  writes made while the device is silent do not render until you call this.
  Call it on state transitions, not every `loop()` pass (each call appends one
  point to plot-enabled datasets). **It updates the dashboard ONLY: exports
  (CSV/MDF4/Session DB/MQTT/gRPC) are fed exclusively by frames the frame
  parser publishes, and the parser publishes a frame only when `parse()`
  returns a non-empty result.** If the dashboard is driven from data tables and
  the parser would otherwise have nothing to return, make `parse()` return
  dummy data (e.g. `return [0];`) so a frame still flows to the export
  fan-out. A parser that returns `[]` keeps the dashboard alive via
  `tableSet()` + `refreshDashboard()`, but exports stay completely empty.
- `ensureDashboard(spec)` -- declarative, idempotent widget builder. Spec is an
  array of groups: `{title, widget, datasets: [{title, index, plot, gauge,
  units, ...}]}`. Groups match by title, datasets by parser `index` within the
  group; only missing items are created, existing ones are never modified, and
  the last satisfied spec is memoized so calling it every `loop()` is free.
  Group `widget`: datagrid | accelerometer | gyroscope | gps | multiplot |
  none | plot3d | image | painter. Dataset flags: plot, fft, bar, gauge,
  compass, led, waterfall (or a raw `options` bitfield). Created items are real
  project edits and persist with the project.

Control scripts also get the data-table globals `tableGet(table, register)` and
`tableSet(table, register, value)` (`table` is the full `/`-joined folder
path for a table inside folders; a top-level table is its bare name),
marshalled through
`project.dataTable.getValue` / `setValue` (each call is a thread round-trip:
read once per `loop()` pass, not in tight inner loops). `tableGet` returns
`undefined` for unknown registers, so `tableGet(t, r) || fallback` works.
`datasetGetRaw` / `datasetGetFinal` remain parser/transform-only.

Canonical communication-loss watchdog (tables + refreshDashboard):

```javascript
var commLost = false;

function loop() {
  var r = io.getLatestFrame();
  var hasData = r.ok && r.result.hasData;
  var stale   = hasData && r.result.ageMs > 1000;
  var fresh   = hasData && r.result.ageMs <= 1000;

  if (stale && !commLost) {
    commLost = true;
    tableSet("BRD-1", "boot_selftest", 0xCC);  // transforms map 0xCC -> "Comm Loss"
    refreshDashboard();                        // render it without waiting for a frame
    notifyCritical("Watchdog", "No frames for " + r.result.ageMs + " ms");
  } else if (fresh && commLost) {
    commLost = false;
    notifyInfo("Watchdog", "Communication restored");
  }

  delay(100);
}
```

Canonical auto-dashboard loop:

```javascript
function loop() {
  var f = newFrame();
  if (!f) { delay(100); return; }

  var spec = { title: "Telemetry", widget: "multiplot", datasets: [] };
  for (var i = 0; i < f.values.length; ++i)
    spec.datasets.push({ title: "Channel " + (i + 1), index: i + 1, plot: true });

  ensureDashboard([spec]);
  delay(100);
}
```

Rules: always pace a loop with `delay()` (50-250 ms for polling; ~20 ms floor for
dashboard-feeding simulations — every `tableSet`/`apiCall` is a blocking GUI-thread
hop, and a free-running loop that writes on each pass stalls the dashboard); do not
hand-roll exists-checks with `project.group.list` loops (ensureDashboard does
the diff); renaming a generated group in the editor makes the script recreate
it under the original title (the spec is the source of truth); ConsoleOnly mode
yields `text` but an empty `values`.

## Mistakes that will not work (do not write these)

Before writing any `io.ble.*` or byte-payload call, fetch `sdk_js` and copy the
real signature. The following all fail; they are the failure modes seen most
often:

- **Object arguments.** `io.ble.setNotifyCharacteristic({ characteristicUuid: "fff2" })`
  is wrong. Arguments are POSITIONAL: `io.ble.setNotifyCharacteristic("fff2")`.
  Passing an object stuffs the whole object into the first parameter, so a UUID
  becomes `[object Object]` and you get `No characteristic matches UUID ''`.
- **`.then()` / await.** SDK calls are SYNCHRONOUS. `io.getStatus().then(...)` and
  `await io.ble.writeCharacteristic(...)` are wrong; there are no promises. Read
  the returned `{ ok, result, error }` object directly.
- **Hand-rolled base64.** Never write your own hex-to-base64 (it is easy to get
  wrong, and it is unnecessary). Pass a hex string with `SerialStudio.Hex` and
  the SDK encodes it: `io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex)`.
- **Guessed command names.** There is no `io.ble.discoverCharacteristics`, no
  `io.ble.setNotify`, no `dashboard.tailFrames(16)`-as-promise. The real BLE
  commands are `selectServiceByUuid`, `setNotifyCharacteristic`,
  `writeCharacteristic`, `listServices`, `listCharacteristics`. If you cannot
  find a command in `sdk_js`, it does not exist.
- **Calling script functions from the console.** A function you define in the
  control script (e.g. a `zeroSensor()` helper) is NOT reachable as
  `SerialStudio.controlScript.zeroSensor()` or from the console. Control-script
  functions only run via `setup()`/`loop()`. For a user-triggerable one-shot,
  use an Action, not a console call.
- **Double-exporting from a control-loop project.** When the script owns the
  values (writes tables, calls `dashboardTick()` to render AND export), the frame
  parser MUST return empty (`[]` / `{}`). A non-empty parser return publishes its
  own frame, so every value lands in CSV/MDF4/Session DB/MQTT twice and the two
  paths race in the export records. `refreshDashboard()` is the view-only variant
  (never exports), and works from the first `loop()`. The opposite, parser-driven
  pattern — `parse()` writes the tables and returns `[0]` so each real arriving
  frame flows to exports — is correct only when the device sends frames, and must
  NOT also call `dashboardTick()`.

## Pacing loop() with delay()

`loop()` runs as fast as it can be scheduled. Call `delay(ms)` (control script
only) to pace it, exactly like Arduino. `delay` blocks only the script's worker
thread, never the dashboard or frame parsing, and the runtime watchdog is paused
for the sleep so a long wait is not flagged as a hang.

```javascript
function loop() {
  io.writeData("READ\r\n", SerialStudio.Text);
  delay(500);   // ~2 Hz
}
```

## Managing the control script from the API

The `controlScript.*` commands let tools and other scripts inspect, validate,
and replace the script:

- `controlScript.get` / `controlScript.getCode` -> `{ code }`: the current
  script source (aliases; `getCode` matches the `project.frameParser.getCode`
  naming convention).
- `controlScript.dryRun { code }` -> `{ valid, hasSetup, hasLoop, error?,
  line? }`: compile-check WITHOUT installing or running anything. Syntax errors
  come back with line numbers; `setup()`/`loop()` never execute and
  `apiCall`/`tableSet` have no effect. **Always dry-run before set.**
- `controlScript.set { code }` / `controlScript.setCode { code }`: replace the
  script (aliases). It is persisted in the project and applied live; if a
  device is connected it recompiles and restarts.
- `controlScript.getStatus` -> `{ running }`: whether the script is running now.

For the focused runtime reference (all injectable globals, lifecycle rules,
the connect-cycle-safe watchdog), fetch
`meta.fetchScriptingDocs{kind: 'control_script_js'}`.

## Lifecycle: fresh engine on every connection

The script starts when the device connects and stops when it disconnects.
**Every connection gets a brand-new engine**: all top-level variables reset
and `setup()` re-runs on each reconnect. Never design around state surviving
a connect/disconnect cycle. The latest-frame store is also cleared on every
connection edge, so `io.getLatestFrame()` reports `hasData: false` until the
first frame of the *current* connection arrives: an `ageMs`-based watchdog
can never fire from a previous connection's frame.

## Leave source and connection setup to the user

A control script automates a device that the user has **already configured and
connected** in the I/O panel. Do not try to create, configure, or connect a data
source from a script, and especially not a BLE source: picking the adapter,
scanning, choosing the device, and connecting is interactive and unreliable to
drive programmatically. Assume the connection is open by the time `setup()`
runs (it only runs after the device connects). When the user asks for a wake-up
or polling script, write only the `setup()`/`loop()` logic and tell them to set
up and connect the source themselves first.

The one exception is GATT layout *within* an already-connected BLE device:
selecting which service and notify characteristic the script talks to (below)
is fine, because the device is connected and the choice is by UUID.

## BLE GATT is owned by the project, not the script

The BLE **service and notify characteristic are configured on the source and
saved in the project** (by UUID). The driver applies them after discovery and
only then reports "connected", so by the time `setup()` runs, the service is
selected and the notify characteristic is subscribed. **Do not select the
service or notify characteristic from a control script.** A script that calls
`io.ble.selectServiceByUuid(...)` / `io.ble.setNotifyCharacteristic(...)` fights
the driver for ownership and races discovery; leave both to the project.

A control script's only BLE job is the **write handshake**:

- `io.ble.writeCharacteristic("fff1", "<hex>", SerialStudio.Hex)` -- send commands.

Pass byte payloads as a **hex string** with `SerialStudio.Hex`, never a JS array
of numbers; the wrapper hex-decodes the string for you. When a user asks for a
BLE wake-up/streaming script, write only the `writeCharacteristic` handshake and
tell them to pick the service + notify characteristic on the BLE source (it is
saved with the project). `selectServiceByUuid` / `setNotifyCharacteristic` still
exist for advanced one-off cases, but are not the path for a normal project.

## Worked example: testo 549i BLE pressure probe (canonical)

A real pattern for a split read/write BLE probe (service `FFF0`, write `FFF1`,
notify `FFF2`) that streams only after a vendor command sequence. The service
and notify characteristic are set on the source and saved in the project, so the
control script is just the handshake; a binary frame parser turns the
notifications into datasets.

Control script (handshake only):

```javascript
const WRITE_UUID  = "fff1";
const ENABLE_CMDS = ["5600030000000C69023E81", "200000000000077B", "110000000000035A"];

function setup() {
  for (var i = 0; i < ENABLE_CMDS.length; ++i) {
    if (!io.ble.writeCharacteristic(WRITE_UUID, ENABLE_CMDS[i], SerialStudio.Hex).ok) return;
    delay(100);
  }
}

function loop() { delay(1000); }
```

Matching frame parser. Binary BLE data needs the source set to **decoder
Binary** and **No Delimiters**; with the Binary decoder, `parse(frame)` receives
an **Array of byte values** (not a string), so read bytes by index and decode
floats with `DataView` -- never `charCodeAt` on the frame:

```javascript
function floatLE(frame, at) {
  var v = new DataView(new ArrayBuffer(4));
  for (var i = 0; i < 4; ++i) v.setUint8(i, frame[at + i] & 0xff);
  return v.getFloat32(0, true);
}
function markerEnd(frame, name) {
  for (var i = 0; i + name.length <= frame.length; ++i) {
    var hit = true;
    for (var j = 0; j < name.length; ++j)
      if (frame[i + j] !== name.charCodeAt(j)) { hit = false; break; }
    if (hit) return i + name.length;
  }
  return -1;
}
var bar = 0, psi = 0, batt = 0;
function parse(frame) {
  var p = markerEnd(frame, "DifferentialPressure");
  if (p !== -1 && p + 4 <= frame.length) { bar = floatLE(frame, p) * 1e-5; psi = bar * 14.5037738; }
  var b = markerEnd(frame, "BatteryLevel");
  if (b !== -1 && b + 4 <= frame.length) batt = floatLE(frame, b);
  return [bar, psi, batt];
}
```

For the full help page, fetch `Control-Script`.
