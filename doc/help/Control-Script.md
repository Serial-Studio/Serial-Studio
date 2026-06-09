# Control Script (setup / loop)

The Control Script automates a connected device the way an Arduino sketch does. You write two functions, `setup()` and `loop()`, and Serial Studio calls them over the life of the connection: `setup()` once when the device connects, `loop()` repeatedly at a fixed interval while it stays connected. Both functions can drive the connection through Serial Studio's I/O API, so the script can send wake-up handshakes, poll registers, issue keep-alives, or step a state machine without any firmware changes on the device.

This is a per-project script, not a per-source one. A project has exactly one Control Script; if you run several devices in one project, the script branches on the device itself.

The script runs on its own worker thread, separate from the data hotpath, so a slow or blocking script never stalls frame parsing, the dashboard, or the UI. Each call is bounded by a runtime watchdog; a function that runs too long is stopped and reported as an error rather than freezing the application.

Use this when:

- A device needs a command sequence to start streaming (many BLE probes and lab instruments only report data after a "begin measurement" handshake).
- You want to poll a device on a timer (request a Modbus register, send an `AT` query, ask for the next sample).
- A device needs a periodic keep-alive to stay awake or stay connected.

## Where to configure it

The Control Script has its own node in the project editor's left tree, directly under the project root. Selecting it opens the code editor.

```
Project
├─ Device 1 (Bluetooth LE)
├─ Control Script          <-- here
├─ Actions
├─ Groups
└─ ...
```

The editor toolbar mirrors the Frame Parser editor: **Reset** loads the starter template, **Open** imports a `.js` file, then the usual undo / redo / clipboard actions, and **Validate** compiles the script and reports the first syntax error (or confirms it is clean) without running it. A status dot shows whether the script is currently running, and **Help** opens this page.

## The two functions

```javascript
function setup() {
  // Runs once, right after the device connects.
}

function loop() {
  // Runs every loop() interval while the device stays connected.
}
```

You may define either or both. A device that streams on its own after a one-time handshake needs only `setup()`. A device that must be polled needs `loop()`. The script is restarted on each new connection, so `setup()` runs again every time you reconnect.

Top-level variables declared in the file persist across `loop()` iterations, so the script can keep counters, buffers, or a small state machine between ticks:

```javascript
var tick = 0;

function loop() {
  tick += 1;
  // ... use tick ...
}
```

### loop() pacing with delay()

By default `loop()` runs as fast as the worker can schedule it (the next call is queued only after the current one returns, so it never stacks up). To pace it, call `delay(ms)` exactly like an Arduino sketch:

```javascript
function loop() {
  apiCall("io.writeData", { data: "UkVBRA0K" });   // ask for a sample
  delay(500);                                       // ~2 Hz
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

- Functions are dotted namespaces: `io.writeData`, `io.ble.writeCharacteristic`, `io.connect`, `controlscript.getStatus`, and so on. Type a name in the editor and autocomplete shows the rest.
- Required arguments are positional; any optional ones go in a trailing object.
- A byte-payload argument takes a trailing **encoding**: `SerialStudio.Hex`, `SerialStudio.Text`, or `SerialStudio.Base64`. The SDK does the base64 conversion, so you pass hex or text directly and never encode by hand.
- Every call returns `{ ok, result, error, errorCode }`. Check `ok`.

The most useful I/O calls for a control script:

- `io.writeData(data, encoding)` -- write bytes to the active connection (any bus).
- `io.ble.selectServiceByUuid(uuid)` -- select a BLE service (e.g. `"fff0"`).
- `io.ble.setNotifyCharacteristic(uuid)` -- subscribe to incoming notifications (e.g. `"fff2"`).
- `io.ble.writeCharacteristic(uuid, data, encoding)` -- write to a specific BLE characteristic (e.g. `"fff1"`), independent of the notify one.
- `io.getStatus()` -- returns `{ isConnected, paused, ... }`.

## Examples

### 1. BLE wake-up handshake

Many BLE devices expose a service (here `FFF0`) with separate write (`FFF1`) and notify (`FFF2`) characteristics, and only start streaming after a command sequence is written. `setup()` selects the service, subscribes to the notify characteristic, and writes the enable sequence; readings then arrive through the frame parser like any other data.

```javascript
function setup() {
  io.ble.selectServiceByUuid("fff0");
  io.ble.setNotifyCharacteristic("fff2");

  var cmds = ["0102030405", "0607080900", "0A0B0C0D0E"];
  for (var i = 0; i < cmds.length; ++i) {
    var r = io.ble.writeCharacteristic("fff1", cmds[i], SerialStudio.Hex);
    if (!r.ok)
      console.log("Handshake step " + i + " failed: " + r.error);
  }
}
```

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

## Saving and lifecycle

The script is saved inside the project file, so it travels with the project. Editing it while connected restarts it immediately; otherwise it starts the next time the device connects and stops when the device disconnects. Use **Validate** before connecting to catch syntax errors early; runtime errors (an exception inside `setup()` or `loop()`) are shown in the editor's status bar and stop the script.

## Related

- [Bluetooth Low Energy](Drivers-Bluetooth-LE.md) for selecting the device, service, and notify characteristic the script writes alongside.
- [Frame Parser Reference](JavaScript-API.md) for turning the bytes a device streams into dataset values.
- [Actions](Actions.md) for user-triggered one-shot commands, the manual counterpart to an automated Control Script.
