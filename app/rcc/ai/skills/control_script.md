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
  `controlscript.getStatus()`, etc.
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

The `controlscript.*` commands let tools and other scripts inspect and replace
the script:

- `controlscript.get` -> `{ code }`: the current script source.
- `controlscript.set { code }`: replace the script. It is persisted in the
  project and applied live; if a device is connected it recompiles and restarts.
- `controlscript.getStatus` -> `{ running }`: whether the script is running now.

## BLE service / characteristic selection from a script

For a split read/write BLE device, a control script can wire up the whole GATT
layout by UUID (robust against discovery order):

- `io.ble.selectServiceByUuid("fff0")` -- pick the service.
- `io.ble.setNotifyCharacteristic("fff2")` -- subscribe to incoming data.
- `io.ble.writeCharacteristic("fff1", "<hex>", SerialStudio.Hex)` -- send commands.

## Worked example: BLE handshake

Many BLE devices expose a service with separate write and notify
characteristics, and only start streaming after a command sequence is written.
`setup()` selects the service, subscribes to the notify characteristic, then
writes the enable commands; readings then arrive through the frame parser.

```javascript
function setup() {
  io.ble.selectServiceByUuid("fff0");
  io.ble.setNotifyCharacteristic("fff2");
  io.ble.writeCharacteristic("fff1", "0102030405", SerialStudio.Hex);
}
```

For the full help page, fetch `Control-Script`.
