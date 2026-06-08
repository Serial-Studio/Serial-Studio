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
  ("home", "calibrate", "reset").
- **1 = Slider**: a value in `[minValue, maxValue]` with `stepSize` and
  `initialValue`. Sends on drag-end.
- **2 = Toggle**: boolean. Sends one of two payloads depending on state.
- **3 = TextField**: free-form string input + Send button.
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
- `minValue`, `maxValue`, `stepSize`, `initialValue`: numeric range
  (sliders, knobs)
- `transmitFunction`: JS source that returns the bytes to send

## The transmit function

The function name is `transmit` (NOT `output` / `send`). It receives a
single scalar `value` — numeric for slider/knob, `1`/`0` for toggle, the
string for textfield, and `1` for a button press. There is no `state`
object. It returns either a string (encoded with the widget's TX encoding)
or a byte array.

The only injected protocol globals are `modbusWriteRegister`,
`modbusWriteCoil`, `modbusWriteFloat`, `canSendFrame`, and `canSendValue`.
NMEA, GRBL, GCode, SCPI, SLCAN, CRC, and binary-packet logic are NOT
globals — adapt a bundled reference script (next section) that self-contains
that code. Full details: `meta.fetchScriptingDocs{kind: "output_widget_js"}`.

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
// integer form (back-compat): widgetType: 15
// relativeIndex defaults to next free slot when omitted
```

The OutputPanel shows ALL output widgets in that group on one tile.
Don't add one tile per output widget — that's not how it's structured.

## Hardware writes go through the user, not you

You cannot transmit bytes yourself. `console.send`, `io.writeData`,
`io.connect`, and `io.disconnect` are Blocked at the dispatcher.
Tools that hit those paths return `category: "hardware_write_blocked"`
(distinct from `permission_denied`, which is OS-level only).

The whole point of an output widget is that the *user* presses the
button and the JS transmit function you generated runs on their
behalf. Frame your help around that: write a clean transmit() that
produces the right bytes, and let the user be the one who fires it.

## Why JavaScript-only

Output widget transmit functions and Painter scripts are
JavaScript-only on purpose:

- Output widgets inject a JS-specific Modbus/CAN helper set and ship a
  JS reference-script library (NMEA, GRBL, GCode, SCPI, SLCAN, binary
  packet, etc.). Porting that to Lua is a large effort for marginal
  benefit.
- Painters draw via Canvas2D, which is bound to QJSEngine; there is
  no Lua canvas API.

Frame parsers and per-dataset transforms support BOTH Lua (preferred
for hotpath performance) and JavaScript. The Lua nudge does NOT
extend to output widgets or painters.
