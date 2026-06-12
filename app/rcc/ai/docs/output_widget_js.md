# Output Widgets: JavaScript "transmit function" API

Output widgets (Pro) translate UI state into bytes the device receives.
Each widget instance has one user-authored JS function that runs on every
state change (button press, slider release, toggle flip, textfield submit).

## Contract

The entry point is `transmit`, **not** `output` / `send`. It takes a single
scalar `value` and returns the bytes to send:

```js
function transmit(value) {
  return "CMD " + value + "\r\n";
}
```

`value` is a single scalar whose type depends on the widget kind — there is
**no `state` object**:

| Widget kind   | `value` passed to `transmit()`        |
|---------------|---------------------------------------|
| Button (0)    | the number `1` on each press          |
| Slider (1)    | the current numeric value (after step rounding) |
| Toggle (2)    | the number `1` (on) or `0` (off)      |
| TextField (3) | the submitted string                  |
| Knob (4)      | the current numeric value (like slider) |

Return either:

- a **String**: encoded using the widget's configured TX encoding and sent.
  For binary payloads build a byte-string with `String.fromCharCode(...)`
  the way the bundled modbus/can helpers do — plain `Array<number>` returns
  are dropped (they convert to an empty byte array),
- `null` / `undefined` / an empty result: nothing is sent.

There is a maximum payload size; oversized returns are dropped with an error.

## Injected protocol helpers (globals)

The Modbus/CAN helpers below are injected as globals, alongside the full
host API: `apiCall`, `tableGet`/`tableSet`, `datasetGetRaw`/`datasetGetFinal`,
`deviceWrite`, `actionFire`, `notify*`, the dashboard helpers, and the
generated `SerialStudio` SDK. Protocol logic beyond these helpers you build
yourself in the function body (see the bundled reference
scripts below — they self-contain their protocol logic, they are NOT globals).

```js
modbusWriteRegister(address, value)  // 16-bit holding register -> 4-byte payload
modbusWriteCoil(address, on)         // coil ON=0xFF00 / OFF=0x0000 -> 4-byte payload
modbusWriteFloat(address, value)     // IEEE-754 float over two registers -> 6-byte payload
canSendFrame(id, payload)            // payload = string or Array<number> -> [id_hi,id_lo,dlc,...data]
canSendValue(id, value, bytes = 2)   // big-endian integer over canSendFrame
```

These produce raw binary payloads (returned as strings). There is **no**
`crc16` / `nmea` / `slcan` / `gcode` / `scpi` / `packet` global — those names
do not exist. Author that logic inline or adapt a bundled reference script.

## Bundled reference scripts

`scripts.list{kind: "output_widget_js"}` enumerates ready-to-adapt examples;
`scripts.get{kind: "output_widget_js", id}` returns the full source. Available
templates include: `at_command`, `binary_packet`, `canbus_frame`,
`gcode_command`, `grbl_command`, `json_command`, `modbus_write`,
`nmea_sentence`, `pid_setpoint`, `pwm_control`, `relay_toggle`,
`scpi_command`, `simple_command`, `slcan_command`. Each defines its own
`transmit(value)` and computes any checksum/framing inline — adapt one
instead of writing protocol code from scratch.

## Examples

### Toggle a relay (textual command)

```js
function transmit(value) {
  return value ? "RELAY ON\r\n" : "RELAY OFF\r\n";
}
```

### Slider value over Modbus (built-in helper)

```js
function transmit(value) {
  return modbusWriteRegister(0x0010, Math.round(value));
}
```

### NMEA sentence with inline XOR checksum (no helper exists)

```js
function transmit(value) {
  var body = (typeof value === "string") ? value : "GPCMD," + value;
  var chk = 0;
  for (var i = 0; i < body.length; i++)
    chk ^= body.charCodeAt(i);

  var hex = ("0" + chk.toString(16)).slice(-2).toUpperCase();
  return "$" + body + "*" + hex + "\r\n";
}
```

### Slider over CAN (built-in helper)

```js
function transmit(value) {
  return canSendValue(0x123, Math.round(value), 2);
}
```

## Validate before applying

Compile and test the function without touching the live widget:

```
assistant.script.dryRun{kind: "output_widget", code, inputValue?, hex?}
```

It compiles `transmit()` in the same injected-helper + table-API environment
the live widget uses and returns `{ok, compileError?, line?}`. Pass
`inputValue` (and `hex: true` to treat it as hex bytes) to also run
`transmit()` once and get `sampleRun.outputHex` + `byteCount`.

## Sandbox

Same isolation as parser scripts — no network, no filesystem, no Qt access.
Transmit scripts run on UI events, not on the frame hot path, so allocations
are not a concern.
