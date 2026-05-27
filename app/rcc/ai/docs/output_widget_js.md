# Output Widgets — JavaScript "output script" API

Output widgets (Pro) translate UI state into bytes the device receives.
Each widget instance has one user-authored JS script that runs on every
state change.

## Contract

```js
function output(state) {
  return new Uint8Array([0x01, state.value & 0xFF]);
}
```

- `state` is an object whose shape depends on the widget kind (see
  "Widget state shapes").
- Return a `Uint8Array`, an `ArrayBuffer`, a `String`, or `null`.
  - `Uint8Array` / `ArrayBuffer` → bytes are written verbatim.
  - `String` → encoded UTF-8 and written.
  - `null` / `undefined` → nothing is sent.

## Widget state shapes

### Button
```js
{ kind: "button", label: string, pressed: boolean }
```
`pressed` flips true on press, false on release.

### Toggle
```js
{ kind: "toggle", label: string, on: boolean }
```

### Slider
```js
{ kind: "slider", label: string, value: number, min: number, max: number }
```
`value` is the current numeric value (after step rounding).

### TextField
```js
{ kind: "textfield", label: string, text: string, submitted: boolean }
```
`submitted` is true once on Enter; false during typing.

### Panel
```js
{ kind: "panel", controls: Array<state> }
```
A panel batches several controls; `controls` is the array of child states.

## Protocol helpers (injected globals)

The output engine injects helpers so you don't reimplement common protocols:

### Checksums and CRCs
```js
crc8(bytes, poly = 0x07, init = 0)        // -> number
crc16(bytes, poly = 0x1021, init = 0xFFFF) // -> number  (CRC-16/CCITT-FALSE default)
crc32(bytes)                               // -> number  (IEEE 802.3)
xor8(bytes)                                // -> number  (NMEA / SLCAN)
```

Bytes can be `Uint8Array`, `Array<number>`, or `String` (UTF-8).

### NMEA-0183
```js
nmea(sentence)                             // -> "$<sentence>*<XOR>\r\n"
```

### Modbus RTU
```js
modbusReadHolding(slave, addr, count)      // -> Uint8Array (FC=0x03)
modbusWriteSingle(slave, addr, value)      // -> Uint8Array (FC=0x06)
modbusWriteMultiple(slave, addr, values)   // -> Uint8Array (FC=0x10)
```
All include the trailing CRC-16 (Modbus polynomial).

### CAN bus / SLCAN
```js
slcan(id, data, extended = false)          // -> "T<id><len><data>\r" or "t..."
canFrame(id, data, extended = false)       // -> Uint8Array (id_len_data layout for raw CAN drivers)
```

### G-code / GRBL
```js
gcode(command, params = {})                // e.g. gcode('G1', {X: 10, Y: 20, F: 1500})
```

### SCPI
```js
scpi(query)                                // -> "<query>\n"  (terminator-aware)
```

### Binary packet builder
```js
packet([
  { type: 'u8', value: 0xAA },
  { type: 'u16le', value: 1234 },
  { type: 'i32be', value: -7 },
  { type: 'f32le', value: 3.14 },
  { type: 'cstring', value: 'hello' },
])                                         // -> Uint8Array
```
Supported types: `u8 i8 u16le u16be i16le i16be u32le u32be i32le i32be f32le f32be f64le f64be cstring bytes`.

## Examples

### Toggle a relay (single byte)

```js
function output(state) {
  return new Uint8Array([state.on ? 0x01 : 0x00]);
}
```

### NMEA $PXSEND on submit

```js
function output(state) {
  if (!state.submitted)
    return null;
  return nmea('PXSEND,' + state.text);
}
```

### PWM duty (slider) over Modbus

```js
function output(state) {
  const duty = Math.round(state.value);
  return modbusWriteSingle(0x01, 0x0010, duty);
}
```

### Binary command frame with CRC-16

```js
function output(state) {
  if (!state.pressed)
    return null;

  const body = new Uint8Array([0xAA, 0x55, 0x01]);
  const checksum = crc16(body);
  const out = new Uint8Array(body.length + 2);
  out.set(body, 0);
  out[body.length]     = checksum & 0xFF;
  out[body.length + 1] = (checksum >> 8) & 0xFF;
  return out;
}
```

## Sandbox

Same isolation as parser scripts — no network, no filesystem, no Qt access.
Output scripts run on UI events, not on the frame hot path, so allocations
are not a concern.
