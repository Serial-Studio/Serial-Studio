# Output Widgets

Output widgets (Pro) are interactive controls that transmit bytes back to
the device. Each one belongs to a group and has its own JavaScript
"transmit function" that converts UI state into bytes.

**Output transmit functions are JavaScript-only** — the Lua-first
guidance in the `frame_parsers` and `transforms` skills does NOT apply
here. A small set of Modbus/CAN protocol helpers is JS-bound
(see [The transmit function](#the-transmit-function)); author transmit
functions in JS, Lua will not compile.

## Five widget types

Output widgets live in a group's output panel. Each has a `type`:

- **0 = Button**: click-to-send. Most common. Use for command triggers
  ("home", "calibrate", "reset"). Set `checkable: true` to make it latch
  instead: it stays pressed and sends `1` (on) / `0` (off) like a toggle,
  with `onLabel` / `offLabel` as the captions.
- **1 = Slider**: a value in `[minValue, maxValue]` with `stepSize` and
  `initialValue`. Sends on drag-end.
- **2 = Toggle**: boolean. Sends one of two payloads depending on state.
- **3 = TextField**: string input + Send button. With `inputKind` 1 (Number)
  it reads the text as a number and enforces `minValue`..`maxValue` first.
- **4 = Knob**: rotary equivalent of slider; same numeric range fields.

## Anatomy

```
project.outputWidget.add{groupId, type}     // create
project.outputWidget.update{groupId, widgetId, title?, icon?,
                            transmitFunction?}  // configure
project.outputWidget.get{groupId, widgetId}    // read current config
```

Each widget has:

- `title`, `icon`: UI labels
- `checkable`: button latches on/off instead of pulsing a single value
- `style`: button only. 0 = standard, 1 = illuminated push button, 2 = emergency
  stop (always latches; one press sends `1` at once, a later press sends `0`).
  An on-screen emergency stop is a command over the data link, never a
  substitute for a hardwired one
- `style` 3 on a knob is a rotary selector: one position per `stepSize` from
  `minValue` to `maxValue`, named by `labels` (comma-separated). With
  `springReturn` true it holds a position only while pressed and, when let go,
  returns to `initialValue` and sends it once. State feedback matches a
  position's value or label; anything else shows unknown
- `color`: hex fill override for the control; empty = group accent color
- `size`: 0 = small, 1 = normal, 2 = large, 3 = extra large
- `onLabel`, `offLabel`: captions for a latching button or a toggle
- `minValue`, `maxValue`, `stepSize`, `initialValue`: numeric range
  (sliders, knobs); `initialValue != 0` starts a latching button on
- `inputKind` (text field: 0 text, 1 number), `decimals` (-1 automatic),
  `units`, `quantize` (number field: snap to `stepSize` from `minValue`)
- `replyTokenDatasetId`, `replyCodeDatasetId`, `replyMessageDatasetId`
  (dataset uniqueIds, -1 = unbound; token and code must both be set) with
  `stateConfirmMs`: the control sends a token as `transmit(value, token)`
  and waits for the device to echo it with a result code
- `transmitFunction`: JS source that returns the bytes to send

Sliders, knobs and Number text fields enforce `minValue`..`maxValue` before
`transmit()` runs; a value outside it is out of range and never reaches the
script, so do not write a range clamp in the function. Read the range with
`project.outputWidget.get` before driving a control.

After driving a control, `ui.automation.listElements` for
`dashboard.output.<groupId>.<widgetId>` returns a `state` object: compare
`state.token` before and after to know the answer is yours, then read
`state.outcome` (`acknowledged`, `refused` with `state.message`, or
`unanswered`).

## The transmit function

The function name is `transmit` (NOT `output` / `send`). It receives a
single scalar `value` — numeric for slider/knob, `1`/`0` for toggle, the
string for textfield, and `1` for a button press (`1`/`0` when the button
is `checkable`). There is no `state`
object. It returns either a string (encoded with the widget's TX encoding)
or a byte array.

The only injected protocol globals are `modbusWriteRegister`,
`modbusWriteRegisters`, `modbusWriteCoil`, `modbusWriteFloat`, `canSendFrame`,
and `canSendValue`. Every Modbus helper takes an optional trailing `unit`
(1-247) to write to another device on the same bus; the payload then
carries a `0xFF 0x83 <unit>` prefix, so set the control's `txEncoding` to
Latin-1 or the bytes above 127 are re-encoded and the driver refuses them.
Checksums are globals too: `checksum(algorithm, data)` returns the digest as
a binary string in wire order and `checksumHex` the hex form, for `XOR-8`,
`MOD-256`, `CRC-8`, `CRC-16`, `CRC-16-MODBUS`, `CRC-16-CCITT`, `Fletcher-16`,
`CRC-32` and `Adler-32`. NMEA, GRBL, GCode, SCPI, SLCAN, and binary-packet
framing are NOT globals — adapt a bundled reference script (next section)
that self-contains that code. Full details:
`meta.fetchScriptingDocs{kind: "output_widget_js"}`.

```js
// Simple: send a Modbus-shaped command on button click
function transmit() {
  return ":01030000000AF1\r\n";
}

// Slider: send a 16-bit big-endian value
function transmit(value) {
  const v = Math.round(value) & 0xFFFF;
  return new Uint8Array([0xA5, (v >> 8) & 0xFF, v & 0xFF]);
}

// Toggle: send different payload per state
function transmit(checked) {
  return checked ? "RELAY ON\n" : "RELAY OFF\n";
}

// TextField: prepend AT prefix
function transmit(text) {
  return "AT+" + text + "\r\n";
}
```

## Iteration workflow

1. Get current config: `project.outputWidget.get{groupId, widgetId}` —
   preserve user-set ranges/labels when rewriting `transmitFunction`.
2. Read scripting reference:
   `meta.fetchScriptingDocs{kind: "output_widget_js"}`.
3. Adapt a real example: `scripts.list{kind: "output_widget_js"}` ships
   reference scripts for NMEA checksums, Modbus framing, SLCAN, GRBL,
   GCode, SCPI, AT commands, JSON, and binary packets — each self-contains
   its protocol logic in a `transmit(value)` you copy and tweak.
4. Validate BEFORE pushing:
   `assistant.script.dryRun{kind: "output_widget", code, inputValue?, hex?}`.
   It compiles `transmit()` in the real protocol-helper + table-API
   environment and returns `{ok, compileError?, line?}`. Pass `inputValue`
   (and `hex: true` for hex byte input) to also run `transmit()` once and
   get `sampleRun.outputHex` + `byteCount` — the only way to catch runtime
   errors and wrong byte output before the widget ships.
5. Push: `project.outputWidget.update{groupId, widgetId,
   transmitFunction: "..."}`.

## Pinning to a workspace

Output widgets render in the OutputPanel widget. To pin a panel to a
workspace:

```
project.workspace.setCustomizeMode{enabled: true}
project.workspace.addWidget{workspaceId, widgetType: "output-panel",
                            groupId}
// integer form (back-compat): widgetType: 19
// relativeIndex defaults to next free slot when omitted
```

The OutputPanel shows ALL output widgets in that group on one tile.
Don't add one tile per output widget — that's not how it's structured.

## Hardware writes go through the user, not you

You cannot transmit bytes yourself. `console.send`, `io.writeData`,
`io.connect`, and `io.disconnect` are deviceGated at the dispatcher:
blocked while the user's "Allow device control" toggle is off; with it
on they resolve to AlwaysConfirm, so each call still needs explicit
approval. Tools that hit those paths while blocked return
`category: "hardware_write_blocked"`
(distinct from `permission_denied`, which is OS-level only).

The whole point of an output widget is that the *user* presses the
button and the JS transmit function you generated runs on their
behalf. Frame your help around that: write a clean transmit() that
produces the right bytes, and let the user be the one who fires it.

## Why JavaScript-only

Output widget transmit functions and Canvas scripts are
JavaScript-only on purpose:

- Output widgets inject a JS-specific Modbus/CAN helper set and ship a
  JS reference-script library (NMEA, GRBL, GCode, SCPI, SLCAN, binary
  packet, etc.). Porting that to Lua is a large effort for marginal
  benefit.
- Canvas scripts draw via Canvas2D, which is bound to QJSEngine; there is
  no Lua canvas API.

Frame parsers and per-dataset transforms support BOTH Lua (preferred
for hotpath performance) and JavaScript. The Lua nudge does NOT
extend to output widgets or canvas scripts.

## When this skill does not match what the user sees

The output-widget host, the transmit script editor and the control
state feedback live in `source/core/Ui`; the write itself goes through
the driver in `source/core/Devices`. If a button's script behaves unlike
the description above, or the user quotes a script error verbatim,
`fs.search` that text with `path:"source/core/Ui"` and read the one file
it names; the `debugging` skill has the rules for citing it.
