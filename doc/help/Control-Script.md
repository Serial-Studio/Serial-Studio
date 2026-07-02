# Control Loop (setup / loop)

The Control Loop automates a connected device the way an Arduino sketch does. You write two functions, `setup()` and `loop()`, and Serial Studio calls them over the life of the connection: `setup()` once when the device connects, `loop()` repeatedly while it stays connected (paced by `delay()`, as the loop() pacing section explains). Both functions can drive the connection through Serial Studio's I/O API, so the loop can send wake-up handshakes, poll registers, issue keep-alives, or step a state machine without any firmware changes on the device.

The Control Loop automates a device you have **already set up and connected**. It is not where you create or connect a data source: choose the bus, device, and connection in the I/O panel first, then write the loop to drive that open connection. This matters most for Bluetooth LE, where scanning, selecting the device, and connecting are interactive steps best done by hand; once the BLE device is connected, the loop can select its service and notify characteristic by UUID and write commands (see the BLE example below).

This is a per-project loop, not a per-source one. A project has exactly one Control Loop; if you run several devices in one project, the loop branches on the device itself.

The loop runs on its own worker thread, separate from the data hotpath, so a slow or blocking loop never stalls frame parsing, the dashboard, or the UI. Each call is bounded by a runtime watchdog; a function that runs too long is stopped and reported as an error rather than freezing the application.

Use this when:

- A device needs a command sequence to start streaming (many BLE probes and lab instruments only report data after a "begin measurement" handshake).
- You want to poll a device on a timer (request a Modbus register, send an `AT` query, ask for the next sample).
- A device needs a periodic keep-alive to stay awake or stay connected.
- The dashboard should build itself from whatever the device sends (see the self-building dashboard example below).

## Where to configure it

The Control Loop has its own node in the project editor's left tree, directly under the project root. Selecting it opens the code editor.

```
Project
├─ Device 1 (Bluetooth LE)
├─ Control Loop            <-- here
├─ Actions
├─ Groups
└─ ...
```

The editor toolbar mirrors the Frame Parser editor: **Reset** loads the starter template, **Open** imports a `.js` file, then the usual undo / redo / clipboard actions, **Help** opens this page, and **Validate** compiles the script and reports the first syntax error (or confirms it is clean) without running it. Errors appear in a banner below the editor.

## The lifecycle functions

```javascript
function onConnect() {
  // Runs once, just BEFORE the connection opens.
}

function setup() {
  // Runs once, right AFTER the device connects.
}

function loop() {
  // Runs every loop() interval while the device stays connected.
}
```

You may define any combination of the three. A device that streams on its own after a one-time handshake needs only `setup()`. A device that must be polled needs `loop()`. The script is restarted on each new connection, so `setup()` runs again every time you reconnect.

`onConnect()` is the exception: it runs *before* Serial Studio opens the connection, so it is the place to start something the connection then depends on. When Serial Studio is the client (a TCP socket, a Modbus master) the server it connects to must already be listening, and the server may be a helper process you launch with `system.exec`. Putting that launch in `setup()` would be too late, because `setup()` only runs once the connection is already open. Because `onConnect()` runs before any device is attached, it cannot read frames or write to the device; use it only for setup that must precede the connection.

Top-level variables declared in the file persist across `loop()` iterations, so the script can keep counters, buffers, or a small state machine between ticks:

```javascript
var tick = 0;

function loop() {
  tick += 1;
  // ... use tick ...
}
```

### loop() pacing with delay()

By default `loop()` runs as fast as the worker can schedule it (the next call is queued only after the current one returns, so it never stacks up). Pacing matters: every I/O, table, or dashboard call in the loop crosses to the interface thread, so an unpaced loop that writes on every pass can starve the user interface. To pace it, call `delay(ms)` exactly like an Arduino sketch:

```javascript
function loop() {
  io.writeData("READ\r\n", SerialStudio.Text);   // ask for a sample
  delay(500);                                    // ~2 Hz
}
```

`delay(ms)` blocks only the script's own worker thread, never the dashboard or frame parsing, and the runtime watchdog is paused for its duration so a long wait is not mistaken for a hung script. A `loop()` with no `delay()` runs at full speed, which is fine for lightweight work.

## Talking to the device: the SerialStudio SDK

Every script runs with the `SerialStudio` SDK loaded: a set of natural,
namespaced functions over Serial Studio's I/O API. You call them directly,
with positional arguments, and the editor offers them all in autocomplete.

```javascript
io.writeData("READ\r\n", SerialStudio.Text);          // send text
io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex);  // send hex bytes
var status = io.getStatus();                           // read connection state
```

Rules:

- Functions are dotted namespaces: `io.writeData`, `io.ble.writeCharacteristic`, `io.connect`, `controlScript.getStatus`, and so on. Type a name in the editor and autocomplete shows the rest.
- Required arguments are positional; any optional ones go in a trailing object.
- A byte-payload argument takes a trailing **encoding**: `SerialStudio.Hex`, `SerialStudio.Text`, or `SerialStudio.Base64`. The SDK does the base64 conversion, so you pass hex or text directly and never encode by hand.
- Every call returns `{ ok, result, error, errorCode }`. Check `ok`.

The most useful I/O calls for a control loop:

- `io.writeData(data, encoding)` -- write bytes to the active connection (any bus).
- `io.ble.selectServiceByUuid(uuid)` -- select a BLE service (e.g. `"fff0"`).
- `io.ble.setNotifyCharacteristic(uuid)` -- subscribe to incoming notifications (e.g. `"fff2"`).
- `io.ble.writeCharacteristic(uuid, data, encoding)` -- write to a specific BLE characteristic (e.g. `"fff1"`), independent of the notify one.
- `io.getStatus()` -- returns `{ isConnected, paused, ... }`.

## Launching helper processes: the system.* API

A control loop can launch a helper program and let Serial Studio manage its lifetime. This is how the bundled examples start their Python data generators automatically: you click **Connect** and the generator is running, with nothing to type in a terminal. The calls live under the `system.` namespace and are available to in-process scripts, including the control loop. For safety, `system.exec`, `system.kill`, and `system.runningProcesses` are rejected over the network API; `system.projectDir` is allowed everywhere.

- `system.projectDir()` -- returns `{ directory, filePath, fileName }` for the loaded project. Use `directory` to build a path to a script that sits next to the `.ssproj` file.
- `system.exec(program, { args, workingDir })` -- launch `program` with optional `args` (an array) and `workingDir` (defaults to the project directory). Returns `{ processId }`.
- `system.kill(processId)` -- stop one launched process.
- `system.runningProcesses()` -- list the processes Serial Studio is currently managing.

Serial Studio stops every process it launched when the device disconnects, when the project changes, and when the app quits, so a helper never lingers after you are done with it.

The program name is resolved cross-platform: a bare `"python3"` or `"python"` is looked up on the system `PATH` plus the usual install locations a GUI app does not otherwise see (Homebrew, pyenv, and user-local directories), so the interpreter that has your installed packages is the one that runs. The launched program's standard output and standard error are captured and written to Serial Studio's console, tagged with the process id, so you can see what the helper prints.

A typical launcher pairs `system.projectDir()` with `system.exec()` and tries `python3` first, falling back to `python`:

```javascript
function startHelper() {
  var dir = system.projectDir();
  if (!dir.ok)
    return;

  var script = dir.result.directory + "/generator.py";
  var r = system.exec("python3", { args: ["-u", script], workingDir: dir.result.directory });
  if (!r.ok)
    r = system.exec("python", { args: ["-u", script], workingDir: dir.result.directory });
}
```

Pass `-u` (as above) when launching Python so its output is unbuffered and appears in the console line by line instead of in bursts.

Put the launch in `setup()` when Serial Studio listens for the data it generates (a UDP source), and in `onConnect()` when Serial Studio connects to it as a client (a TCP socket or a Modbus master), because the server has to be listening before the connection is opened.

## Reading data: See, Decide, Act

A control loop is not limited to sending commands: it can also read what the device sends and act on it. The mental model is the same loop a robot runs: **see** the latest data, **decide** in plain JavaScript, **act** through the SDK.

- **See**: `newFrame()` returns the latest frame received from the device, or `null` when nothing new has arrived since the last call. The returned object carries `text` (the raw payload), `values` (the parser's channel tokens, in parser order), `sourceId`, `timestampMs` (a monotonic clock in milliseconds, useful for deltas between frames, not wall-clock time), `ageMs` (milliseconds elapsed since the frame was captured), and a monotonic `sequence` number.
- **Decide**: plain JavaScript. Compare, count, branch; top-level variables persist across `loop()` calls.
- **Act**: write to the device (`io.writeData`), post notifications, or reshape the dashboard (`ensureDashboard()` below, or any `project.*` call).

`newFrame()` sees the data *before* dataset mapping, so it includes channels that no dataset reads yet, which is exactly what you need to detect a new channel and create a widget for it. (`dashboard.getData()`, by contrast, returns only data already mapped to datasets.) The raw command underneath is `io.getLatestFrame`, which also accepts `{ encoding: "base64" }` for binary payloads.

In ConsoleOnly mode frames are not parsed, so `values` is empty and only `text` carries the payload. Pace any polling loop with `delay()`; 50-250 ms is plenty for channel detection.

### Data tables: tableGet() / tableSet()

Control loops can read and write the same data-table registers the frame parser and dataset transforms use. `tableGet(table, register)` returns the live runtime value (or `undefined` when the table or register does not exist, so `tableGet(t, r) || fallback` works), and `tableSet(table, register, value)` writes one. Both are marshalled to the GUI thread, so each call costs a thread round-trip: read what you need once per `loop()` pass, not in a tight inner loop.

One caveat: dataset transforms only re-run when a frame arrives. If your script writes table registers while the device is silent (e.g. a communication-loss watchdog marking sensors invalid), the dashboard keeps showing the last rendered values until the next frame. Two calls close that gap, and the difference between them is whether the result is exported:

- **`refreshDashboard()` — view only.** Re-runs every dataset transform from the last received values and republishes to the dashboard. Nothing is appended to CSV/MDF4/Session Database/MQTT. Use it for a purely visual refresh, such as a comms-loss watchdog marking sensors invalid.
- **`dashboardTick()` — render *and* export.** Runs the same transform pass but publishes the synthesized frame through the export fan-out as well, so a script-owned (table-driven) simulation is recorded to CSV/MDF4/Session Database/MQTT just like real device data. It also seeds the frame structure when no device frame has arrived yet, so it works from the very first `loop()`. Exports are still gated on a sink being enabled.

> **Choosing between them — and the parser's role.** If the script owns the values (it writes data tables and the device is silent or only a dummy source), call `dashboardTick()` so the data both renders and records, and make the frame parser return **empty** (`return []` / `return {}`). This last part matters: if the parser also returns a non-empty frame, every value is exported twice — once by the parser-produced frame and once by the frame `dashboardTick()` synthesizes — and the two paths race in the export records. Reserve `refreshDashboard()` for refreshes you explicitly do *not* want exported. The opposite pattern is parser-driven: when real device frames arrive and `parse()` writes the tables itself, have it return dummy data (`return [0];`) so each arriving frame flows through the export pipeline, and do *not* also call `dashboardTick()` — either path alone exports each value exactly once.

### Staleness watchdog

`ageMs` makes a communication-loss watchdog short: no clock mixing, no timestamp bookkeeping in the parser. Mark the failure state in the data tables, then `refreshDashboard()` renders it without waiting for a frame:

```javascript
var commLost = false;

function loop() {
  var r = io.getLatestFrame();
  var stale = r.ok && r.result.hasData && r.result.ageMs > 1000;

  if (stale && !commLost) {
    commLost = true;
    tableSet("BRD-1", "boot_selftest", 0xCC);  // sensor transforms show "Comm Loss"
    refreshDashboard();
    notifyCritical("Watchdog", "No frames for " + r.result.ageMs + " ms");
  } else if (!stale && commLost) {
    commLost = false;
    notifyInfo("Watchdog", "Communication restored");
  }

  delay(100);
}
```

### ensureDashboard(spec): declarative widgets

`ensureDashboard(spec)` makes the described dashboard exist. You state what you want; it creates only what is missing and never modifies what is already there. Calling it on every `loop()` pass with the same spec is free (the last satisfied spec is remembered), so there is no bookkeeping to write:

```javascript
ensureDashboard([
  {
    title: "Telemetry",            // group, matched by title
    widget: "multiplot",           // datagrid | multiplot | gps | none | ...
    datasets: [
      { title: "Temperature", index: 1, plot: true, units: "C" },
      { title: "Pressure",    index: 2, plot: true, gauge: true }
    ]
  }
]);
```

- Groups are matched by `title`; datasets are matched by their parser `index` within the group. If you rename a generated group in the Project Editor, the script no longer finds it and recreates it under the original name; treat the spec as the source of truth for the generated parts of a project.
- Dataset flags map to widgets: `plot`, `fft`, `bar`, `gauge`, `compass`, `led`, `waterfall` (or pass a raw `options` bitfield instead).
- Group `widget` accepts `datagrid`, `accelerometer`, `gyroscope`, `gps`, `multiplot`, `none`, `plot3d`, `image`, or `painter`.
- Created groups and datasets are real project edits: they are saved with the project exactly like changes made in the Project Editor, so the dashboard the script builds is still there the next time the project opens.

## Examples

### 1. BLE wake-up handshake

Many BLE devices expose a service (here `FFF0`) with separate write (`FFF1`) and notify (`FFF2`) characteristics, and only start streaming after a command sequence is written. The service and notify characteristic are chosen on the BLE source and saved with the project; the driver wires them on connect, so the control loop only writes the enable sequence. Readings then arrive through the frame parser like any other data.

```javascript
function setup() {
  var cmds = ["0102030405", "0607080900", "0A0B0C0D0E"];
  for (var i = 0; i < cmds.length; ++i) {
    var r = io.ble.writeCharacteristic("fff1", cmds[i], SerialStudio.Hex);
    if (!r.ok)
      console.log("Handshake step " + i + " failed: " + r.error);
  }
}
```

The characteristic UUID is a plain string and the payload a hex string with `SerialStudio.Hex`; do not wrap the arguments in an object and do not hand-encode base64. Configure the service and notify characteristic on the source (they are saved in the project) rather than selecting them from the script. For a binary BLE device, set the source's decoder to **Binary** with **No Delimiters** so the frame parser receives each notification as an array of byte values; see [Frame Parser Reference](JavaScript-API.md) for reading bytes and decoding floats.

### 2. Periodic keep-alive

A device that disconnects when idle gets a heartbeat byte every second.

```javascript
function loop() {
  io.writeData("00", SerialStudio.Hex);   // single 0x00 byte
  delay(1000);
}
```

### 3. Polling a request / response sensor

Some sensors answer only when asked. Send the query, wait, and let the reply come back through the parser as a normal frame.

```javascript
function loop() {
  io.writeData("READ\r\n", SerialStudio.Text);
  delay(200);
}
```

### 4. Timed startup sequence in setup()

A device that needs its init commands spaced out can use `delay()` between them, right inside `setup()`.

```javascript
function setup() {
  var cmds = ["BEGIN\n", "MODE 1\n", "START\n"];
  for (var i = 0; i < cmds.length; ++i) {
    io.writeData(cmds[i], SerialStudio.Text);
    delay(250);
  }
}
```

### 5. Guarding on connection state

`setup()` runs after the connection is open, but you can still confirm before acting, which is useful in `loop()` if the device may drop mid-session.

```javascript
function loop() {
  var status = io.getStatus();
  if (!status.ok || !status.result.isConnected)
    return;

  io.writeData("POLL", SerialStudio.Text);
}
```

### 6. Self-building dashboard

A logger that streams a variable number of comma-separated channels gets a plotted dataset per channel, created the moment a new channel first appears. Widgets that already exist are left alone, so the script is safe to leave running.

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

## Saving and lifecycle

The script is saved inside the project file, so it travels with the project. Editing it while connected restarts it immediately; otherwise it starts the next time the device connects and stops when the device disconnects. Use **Validate** before connecting to catch syntax errors early; runtime errors (an exception inside `setup()` or `loop()`) are shown in the editor's status bar and stop the script.

Every connection starts a fresh script engine: all top-level variables reset and `setup()` runs again on each reconnect, exactly like an Arduino reset. Do not design around state surviving a connect/disconnect cycle. The latest-frame store also clears on each connection edge, so `io.getLatestFrame()` reports no data until the first frame of the current connection arrives; a watchdog built on `ageMs` can never trip from a previous connection's frame.

Tools and scripts can manage the control loop through the API: `controlScript.get`/`controlScript.getCode` read the source, `controlScript.dryRun` compile-checks source without installing or running it (syntax errors come back with line numbers), `controlScript.set`/`controlScript.setCode` install it, and `controlScript.getStatus` reports whether it is running.

## Related

- [SDK Reference](SerialStudio-SDK.md) for the full call surface a control loop drives: `io.*`, `tableGet`/`tableSet`, `deviceWriteAndWait`, dashboard control, notifications, and the See/Decide/Act helpers.
- [Bluetooth Low Energy](Drivers-Bluetooth-LE.md) for selecting the device, service, and notify characteristic the script writes alongside.
- [Frame Parser Reference](JavaScript-API.md) for turning the bytes a device streams into dataset values.
- [Actions](Actions.md) for user-triggered one-shot commands, the manual counterpart to an automated Control Loop.
