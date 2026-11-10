# Control Loop (setup / loop)

Your Arduino prints a reading only after you send it `READ`. Without firmware changes, the Control Loop sends it for you:

```javascript
function loop() {
  io.writeData("READ\r\n", SerialStudio.Text);   // the host-side Serial.println("READ")
  delay(500);                                    // twice a second
}
```

`io.writeData` sends the bytes `READ` followed by CR LF to the open connection. The reply comes back through the [frame parser](Frame-Parser.md) like any other data. To try it, paste the script into **Project Scripts > Control Loop** in the Project Editor, then connect the device.

The Control Loop is a script of up to three functions that Serial Studio calls over the life of a connection: `setup()` once after the device connects, `loop()` repeatedly while it stays connected, and the optional `onConnect()` once just before the connection opens. It can send wake-up handshakes, send commands on a timer, issue keep-alives, or react to what the device prints.

The Control Loop is free; nothing in it checks a license. Three things a script can call need Pro: the `notify*()` functions, Variables tables, and `ensureDashboard()` creating a waterfall, 3D plot or image widget (see [Pro vs Free Features](Pro-vs-Free.md)).

Older versions and the API call it the Control Script (`controlScript.*` commands).

It automates a device you have **already set up and connected**. It is not where you create or connect a device: choose the Connection, the device, and its settings in the Device Setup pane first.

A project has one Control Loop. With several devices, the calls that talk to a device take an optional `{ sourceId: n }`; the default is source 0.

Use it when:

- A device needs a command sequence to start streaming (many BLE probes and lab instruments only report data after a "begin measurement" handshake).
- You want to poll a device on a timer: request a Modbus register, send an `AT` query, ask for the next sample.
- A device needs a periodic keep-alive to stay awake.
- The dashboard should build itself from whatever the device sends ([self-building dashboard](#6-self-building-dashboard)).

## Where to configure it

Open the Project Editor and select **Project Scripts > Control Loop** in the left tree. It sits next to the two transform libraries and opens a code editor.

```
Project
├─ Project Scripts
│   ├─ Control Loop        <-- here
│   ├─ Lua Library
│   └─ JavaScript Library
├─ Actions
├─ Device 1 (Bluetooth LE)
├─ Groups
└─ ...
```

The editor toolbar mirrors the Frame Parser editor: **Reset** loads the starter template, **Open** imports a `.js` file, then the usual undo / redo / clipboard actions, **Help** opens this page, and **Validate** compiles the script and reports the first syntax error (or confirms it is clean) without running it. Errors appear in a banner below the editor.

The Control Loop runs only while a project file is loaded and a device is connected. It does not run in Quick Plot or console-only mode, or while a CSV, MDF4 or session file is open for replay.

## The lifecycle functions

```javascript
function onConnect() {
  // Optional. Runs once, just BEFORE the connection opens.
}

function setup() {
  // Runs once, right AFTER the device connects.
}

function loop() {
  // Runs repeatedly while the device stays connected. Pace it with delay().
}
```

Define `setup()`, `loop()`, or both. A device that streams on its own after a one-time handshake needs only `setup()`; a device that must be polled needs `loop()`. `onConnect()` is optional and cannot stand alone: a script with neither `setup()` nor `loop()` fails with "Script must define setup() and/or loop()." The script is restarted on each new connection, so `setup()` runs again every time you reconnect.

Each call to `setup()` and `loop()` must return within 2000 ms, and `onConnect()` has its own 2000 ms limit. An overrun stops the script and shows "Control script loop() exceeded 2000 ms budget; stopped." in the editor's status bar (for `setup()` the message reads "Control script setup() exceeded 2000 ms budget."). A stopped script does not restart until the next connection. `delay()` does not count against the limit.

Top-level variables declared in the file persist across `loop()` iterations, so the script can keep counters, buffers, or a small state machine between ticks:

```javascript
var tick = 0;

function loop() {
  tick += 1;
  // ... use tick ...
}
```

`onConnect()` is for the rare case where something must be running before Serial Studio opens the connection, such as a helper process; see [Launching helper processes](#launching-helper-processes-the-system-api).

### loop() pacing with delay()

Call `delay(ms)` at the end of `loop()`, as in an Arduino sketch. Without it, `loop()` is queued again 1 ms after each call returns, and every I/O, table, or dashboard call in it crosses to the GUI thread, so an unpaced loop that writes on every pass can make the interface sluggish.

`delay(ms)` blocks only the script's own worker thread, never the dashboard or frame parsing. It is clamped to 0 to 3,600,000 ms (one hour), and the 2000 ms limit starts over when it returns.

## Talking to the device: the SerialStudio SDK

Every script runs with the `SerialStudio` SDK loaded: namespaced functions over Serial Studio's I/O API. You call them directly, with positional arguments, and the editor offers them in autocomplete.

```javascript
io.writeData("READ\r\n", SerialStudio.Text);          // send text
io.ble.writeCharacteristic("fff1", "5600030C", SerialStudio.Hex);  // send hex bytes
var status = io.getStatus();                           // read connection state
```

Rules:

- Functions are dotted namespaces: `io.writeData`, `io.ble.writeCharacteristic`, `io.connect`, `controlScript.getStatus`.
- Required arguments are positional; any optional ones go in a trailing object.
- A byte-payload argument takes a trailing **encoding**: `SerialStudio.Hex`, `SerialStudio.Text`, or `SerialStudio.Base64`. The SDK does the base64 conversion, so you pass hex or text directly and never encode by hand.
- Every namespaced call (`io.*`, `project.*`, `system.*`) returns `{ ok, result, error, errorCode }`; `ok` is `false` on failure. Helpers such as `newFrame()`, `tableGet()` and `delay()` return plain values.

```javascript
var r = io.writeData("READ\r\n", SerialStudio.Text);
if (!r.ok)
  console.log("Write failed: " + r.error);
```

`console.log()` output goes to the Console pane (and to the terminal, if you started Serial Studio from one).

The most useful I/O calls for a control loop:

- `io.writeData(data, encoding)` -- write bytes to the active connection (any bus).
- `io.ble.selectServiceByUuid(uuid)` -- select a BLE service (e.g. `"fff0"`).
- `io.ble.setNotifyCharacteristic(uuid)` -- subscribe to incoming notifications (e.g. `"fff2"`).
- `io.ble.writeCharacteristic(uuid, data, encoding)` -- write to a specific BLE characteristic (e.g. `"fff1"`), independent of the notify one.
- `io.getStatus()` -- returns `{ isConnected, paused, ... }` plus the bus type, the link state and one entry per source.

## Examples

### 1. Periodic keep-alive

A device that disconnects when idle gets a heartbeat byte every second.

```javascript
function loop() {
  io.writeData("00", SerialStudio.Hex);   // single 0x00 byte
  delay(1000);
}
```

### 2. Init commands that need spacing

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

### 3. Writing only while the link is up

`loop()` runs only while the device is connected, but the link can drop or be paused between two ticks. Check `io.getStatus()` before writing and skip the pass when the link is down.

```javascript
function loop() {
  var status = io.getStatus();
  if (!status.ok || !status.result.isConnected)
    return;

  io.writeData("POLL", SerialStudio.Text);
  delay(500);
}
```

### 4. BLE wake-up handshake

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

The characteristic UUID is a plain string and the payload a hex string with `SerialStudio.Hex`; do not wrap the arguments in an object and do not hand-encode base64. For a binary BLE device, set the source's decoder to **Binary (raw bytes)** with **No delimiters (whole chunk)** so the frame parser receives each notification as an array of byte values; see [Frame Parser Reference](Frame-Parser.md) for reading bytes and decoding floats.

### 5. Bringing a serial device up again (advanced)

This one is only for UART devices that need a special start-up sequence. A UART source with a [Connection Sequence](Drivers-UART.md#connection-sequence) runs it on every connect, before `setup()`. A device that resets itself while the port stays open, for example on a supply dip, goes silent until it is brought up again. The loop below watches the frame age and re-runs the sequence on the open port; `io.uart.sendBreak(55)` alone serves a device that needs only a BREAK reset.

```javascript
function loop() {
  var latest = io.getLatestFrame();
  if (latest.ok && latest.result.hasData && latest.result.ageMs > 3000) {
    var r = io.uart.runSequence();
    if (!r.ok)
      console.log("Re-run refused: " + r.error);

    delay(5000);
    return;
  }

  delay(500);
}
```

`io.uart.runSequence()` returns as soon as the run starts, so it never counts against the 2000 ms limit; the sequence reports its outcome in the console. Give it time with `delay()` before checking again, longer than the sequence's time budget. `io.uart.runSequence()` and `io.uart.sendBreak()` each take an optional `{ sourceId: n }` for a Device other than the first (source 0).

### 6. Self-building dashboard

This example uses `newFrame()` and `ensureDashboard()`, both explained under [Reading data](#reading-data-see-decide-act). A logger that streams a variable number of comma-separated channels gets a plotted dataset per channel, created the moment a new channel first appears. Widgets that already exist are left alone, so the script is safe to leave running.

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

## Reading data: See, Decide, Act

A control loop can also read what the device sends and act on it: **see** the latest data, **decide** in plain JavaScript, **act** through the SDK.

```javascript
function loop() {
  var f = newFrame();
  if (f && f.values[0] > 80)
    io.writeData("FAN ON\n", SerialStudio.Text);

  delay(200);
}
```

- **See**: `newFrame()` returns the latest frame received from the device, or `null` when nothing new has arrived since the last call. It also returns `null` before the first frame of a connection, so check for that before reading `values`. `newFrame(sourceId)` takes an optional source, but the SDK remembers one sequence number for all sources, so poll a single source per script, or compare `f.sequence` yourself using `io.getLatestFrame({ sourceId })`.
- **Decide**: plain JavaScript. Compare, count, branch; top-level variables persist across `loop()` calls.
- **Act**: write to the device (`io.writeData`), post notifications, or reshape the dashboard ([`ensureDashboard()`](#ensuredashboardspec-declarative-widgets), or any `project.*` call).

The frame object carries these fields:

| Field | Meaning |
|-------|---------|
| `text` | The raw payload. |
| `values` | The numbers the parser extracted, in parser order. Empty when the parser has not yet produced channels for this frame, so check `values.length` before indexing. |
| `sourceId` | The device the frame came from. |
| `timestampMs` | A clock in milliseconds that only counts up. Use it for deltas between frames; it is not wall-clock time. |
| `ageMs` | Milliseconds since the frame was captured. |
| `sequence` | A number that counts up with each frame. |

`newFrame()` returns the raw channels even if no dataset reads them yet, which is what you need to detect a new channel and create a widget for it. (`dashboard.getData()`, by contrast, returns only data already mapped to datasets.) The raw command underneath is `io.getLatestFrame`; it also accepts `{ encoding: "base64" }` for binary payloads. Pace any polling loop with `delay()`; 50-250 ms is plenty for channel detection.

The next three parts are advanced: Variables tables, the refresh and tick calls, and the watchdog matter for scripts that compute values themselves. If your device streams real data, skip to [ensureDashboard()](#ensuredashboardspec-declarative-widgets).

### Variables tables: tableGet() / tableSet()

Control loops can read and write the same table variables the frame parser and dataset transforms use (see [Variables](Variables.md); Variables are a Pro feature). `tableGet(table, variable)` returns the live runtime value (or `undefined` when the table or variable does not exist, so `tableGet(t, r) || fallback` works), and `tableSet(table, variable, value)` writes one. Both are marshalled to the GUI thread, so each call costs a thread round-trip: read what you need once per `loop()` pass, not in a tight inner loop.

### refreshDashboard() and dashboardTick()

Dataset transforms only re-run when a frame arrives. If your script writes table variables while the device is silent (a communication-loss watchdog marking sensors invalid, say), the dashboard keeps showing the last rendered values until the next frame. Two calls close that gap, and the difference between them is whether the result is exported:

- **`refreshDashboard()` is view only.** It re-runs every dataset transform from the last received values and republishes to the dashboard. Nothing is appended to CSV/MDF4/Historian/MQTT. Use it for a purely visual refresh.
- **`dashboardTick()` renders and exports.** It runs the same transform pass but publishes the synthesized frame through the export fan-out as well, so a script-owned (table-driven) simulation is recorded to CSV/MDF4/Historian/MQTT just like real device data. It also seeds the frame structure when no device frame has arrived yet, so it works from the very first `loop()`. Exports are still gated on a sink being enabled.

> **Choosing between them, and the parser's role.**
>
> | Who owns the values | Call from the script | `parse()` returns |
> |---------------------|----------------------|-------------------|
> | The script writes the tables; the device is silent or a dummy source | `dashboardTick()` | An empty list `[]` or `{}`, so the parser exports no frame |
> | `parse()` writes the tables; real frames arrive | No extra call | Its normal frame, for example `[23.5, 1013]` |
>
> Never do both. If `parse()` returns a non-empty frame and the script also calls `dashboardTick()`, every value is exported twice, and the two paths race in the export records. Keep `refreshDashboard()` for refreshes you explicitly do *not* want exported.

### Staleness watchdog

`ageMs` makes a communication-loss watchdog short: no clock mixing, no timestamp bookkeeping in the parser. Mark the failure state in the Variables tables, then `refreshDashboard()` renders it without waiting for a frame:

```javascript
var commLost = false;

function loop() {
  var r = io.getLatestFrame();
  var stale = r.ok && r.result.hasData && r.result.ageMs > 1000;

  if (stale && !commLost) {
    commLost = true;
    tableSet("BRD-1", "boot_selftest", 0xCC);  // project-specific: this project's transforms treat 0xCC as comm loss
    refreshDashboard();
    notifyCritical("Watchdog", "No frames for " + r.result.ageMs + " ms");
  } else if (!stale && commLost) {
    commLost = false;
    notifyInfo("Watchdog", "Communication restored");
  }

  delay(100);
}
```

`notifyInfo()`, `notifyWarning()` and `notifyCritical()` need Pro. Without a license they throw an error, so drop the two notify calls to run this example in the free edition.

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

- Groups are matched by `title`; datasets are matched by their parser `index` within the group. `index` is the 1-based position of the value in the parser output, the same as a dataset's **Frame Index**. If you rename a generated group in the Project Editor, the script no longer finds it and recreates it under the original name; treat the spec as the source of truth for the generated parts of a project.
- Dataset flags map to widgets: `plot`, `fft`, `bar`, `gauge`, `compass`, `waterfall` (or pass a raw `options` bitfield instead, which is also how you request an LED). `waterfall` needs Pro.
- Group `widget` accepts `datagrid`, `accelerometer`, `gyroscope`, `gps`, `multiplot`, `none`, `plot3d`, `image`, `painter`, `webview`, `barpanel`, or `ledpanel`. `plot3d` and `image` need Pro; without a license, a `plot3d` group renders as a Multi-Plot titled "*(title)* (Fallback)". See [Pro vs Free Features](Pro-vs-Free.md).
- Created groups and datasets are real project edits: they are saved with the project exactly like changes made in the Project Editor, so the dashboard the script builds is still there the next time the project opens.

## Launching helper processes: the system.* API

A control loop can launch a helper program and let Serial Studio manage its lifetime. This is how the example projects start their Python data generators automatically: you click **Connect** and the generator is running, with nothing to type in a terminal. The example projects are downloaded from the **Examples** toolbar button the first time you open it, which needs an internet connection. The calls live under the `system.` namespace and are available to in-process scripts, including the control loop. For safety, `system.exec`, `system.kill`, and `system.runningProcesses` are rejected over the network API; `system.projectDir` is allowed everywhere.

- `system.projectDir()` -- returns `{ directory, filePath, fileName }` for the loaded project. Use `directory` to build a path to a script that sits next to the `.ssproj` file.
- `system.exec(program, { args, workingDir })` -- launch `program` with optional `args` (an array) and `workingDir` (defaults to the project directory). Returns `{ processId }`.
- `system.kill(processId)` -- stop one launched process.
- `system.runningProcesses()` -- list the processes Serial Studio is currently managing.

Serial Studio stops every process it launched when the device disconnects, when the project changes, and when the app quits, so a helper never lingers after you are done with it.

The first time a project's script asks to run a program, Serial Studio shows a prompt naming the program and its arguments and asks whether this project's scripts may launch programs. Until you answer, `system.exec` returns `ok: false` with `errorCode: "CONSENT_REQUIRED"`.

- **Yes**: the Control Loop restarts so `setup()` runs again with the permission in place, and the answer is remembered for that project file. A launch from `onConnect()` needs one more **Connect** after you allowed it.
- **No**: launches stay refused until you restart Serial Studio.
- **Unsaved project**: Serial Studio asks once per session instead of remembering the answer.

Open projects you do not trust with this in mind: a Control Loop runs with your user account's privileges. Headless runs cannot show the prompt; set the environment variable `SERIAL_STUDIO_API_AUTO_CONSENT=1` to grant it for that process.

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

Put the launch in `setup()` when Serial Studio listens for the data it generates (a UDP source). Put it in `onConnect()` when Serial Studio connects to it as a client (a TCP socket or a Modbus master), because the server must already be listening before the connection opens and `setup()` only runs once the connection is open. `onConnect()` runs before any device is attached, so it cannot read frames or write to the device.

## Saving and lifecycle

The script is saved inside the project file, so it travels with the project. Editing it while connected restarts it immediately; otherwise it starts the next time the device connects and stops when the device disconnects. Use **Validate** before connecting to catch syntax errors early; runtime errors (an exception inside `setup()` or `loop()`) are shown in the editor's status bar and stop the script.

Every connection starts a fresh script engine: all top-level variables reset, exactly like an Arduino reset. Do not design around state surviving a connect/disconnect cycle. The latest-frame store also clears on each connection edge, so `io.getLatestFrame()` reports no data until the first frame of the current connection arrives; a watchdog built on `ageMs` can never trip from a previous connection's frame.

Tools and scripts can manage the control loop through the API: `controlScript.get`/`controlScript.getCode` read the source, `controlScript.dryRun` compile-checks source without installing or running it (syntax errors come back with line numbers), `controlScript.set`/`controlScript.setCode` install it, and `controlScript.getStatus` reports whether it is running. Each one is listed under [Control Script Commands](API-Reference.md#control-script-commands-6) in the API Reference.

## Related

- [SDK Reference](SerialStudio-SDK.md) for the full call surface a control loop drives: `io.*`, `tableGet`/`tableSet`, `deviceWriteAndWait`, dashboard control, notifications, and the See/Decide/Act helpers.
- [Frame Parser Reference](Frame-Parser.md) for turning the bytes a device streams into dataset values.
- [Actions](Actions.md) for user-triggered one-shot commands, the manual counterpart to an automated Control Loop.
- [Bluetooth Low Energy](Drivers-Bluetooth-LE.md) for selecting the device, service, and notify characteristic the script writes alongside.
- [API Reference](API-Reference.md#system-commands-4) for `system.exec` and the helper-process commands a control loop may launch.
