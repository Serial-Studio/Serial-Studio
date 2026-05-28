# Frame Parsers

Frame parsers turn one logical frame (the bytes between `frameStart` and
`frameEnd`) into an array of dataset values. Each Source has its own parser.

## Decision: do you need a parser?

- **Yes**: framing is line-delimited, comma-separated, or has any
  structure beyond raw bytes. The default project template includes a
  `parse(frame) { return frame.split(','); }` parser already.
- **No**: you're reading raw uniform bytes (audio, image stream) where
  the dashboard widget consumes the unframed payload directly. Set
  `frameDetection = NoDelimiters (2)` and skip the parser.

## Pick Lua first

- **Lua** (language=1): **recommended default.** Embedded Lua 5.4. The
  interpreter is faster than `QJSEngine` on the parser hotpath, the
  per-call overhead is lower, and at high frame rates (audio at
  48 kHz, multi-channel UART at 1 kHz+) that compounds. We ship a
  `LuaCompat` shim so 5.1/5.2-era idioms (`math.log10`, `math.pow`,
  `bit32`, `unpack`) keep working.
- **JS** (language=0): use only when you need a JS-specific feature
  (regex flavors, JSON.stringify, etc.). QJSEngine + ConsoleExtension,
  watchdog protected.

ALWAYS pass `language` on `project.frameParser.setCode`. A mismatch
(JS code under language=Lua or vice versa) is a silent compile failure
(the dashboard receives no data and there's no popup). The API now
returns a `warning` field in the response when the syntax doesn't
match the declared language; do not ignore it.

`project.frameParser.setLanguage` WIPES existing code and replaces it
with the new language's default template. Read the existing source with
`getCode` first if you want to preserve it; or just pass `language`
directly to `setCode` to flip and replace in one call.

## Iteration workflow

1. Get the current parser: `project.frameParser.getCode{sourceId}`.
2. Get a sample of what the device emits:
   - If connected: `dashboard.tailFrames{count: 4}` to see recent
     parsed values, then ask the user to paste an example raw frame
     (raw stream bytes, not pre-extracted).
   - If disconnected: ask for a sample. Hex (`01 A2 FF`) is the
     binary-safe form; UTF-8 text is fine for ASCII / CSV protocols.
3. **Compile-only**: `assistant.script.dryRun{kind:"frame_parser", code,
   language}` routes to `project.frameParser.dryCompile`. Verifies the
   script parses and `parse(frame)` is defined without executing.
   Cheapest way to catch the wrong-language silent failure (Lua code
   with `language: 0`, or the reverse).
4. **Dry-run (pipeline)**: `assistant.script.dryRun{kind:"frame_parser",
   code, language, inputBytesHex, decoderMethod, frameDetection,
   frameStart, frameEnd, hexadecimalDelimiters, checksumAlgorithm}`.
   The bytes are driven through the same FrameReader + decoder switch
   the live FrameBuilder uses, then handed to `parse()`. Returns
   per-frame `rawHex` + `decoderOutput` + `rows`, plus
   `extractedCount` / `consumedBytes` / `remainingBytes` /
   `droppedFrames`. Stateful parsers (top-level closures, EMA, frame
   assembly buffers) run sequentially through one engine, so state
   evolves frame-by-frame.

   This is the ONLY dryRun mode now. There is no `sampleFrame` /
   `sampleFrames` fallback; passing pre-extracted frames cannot be
   distinguished from a real byte stream, and that ambiguity hid every
   extraction / decoder bug from past iterations.
5. Push: `assistant.script.apply{kind:"frame_parser", code, language,
   sourceId, inputBytesHex, decoderMethod, ...}`. It dry-runs first,
   then calls `project.frameParser.setCode`. Without bytes, apply
   falls back to dryCompile (syntax only).
6. Auto-save will write to disk within ~1s.

## Read the stage outputs, not just the rows

The pipeline response is structured intentionally:

```json
{
  "extractedCount":  2,
  "consumedBytes":   24,
  "remainingBytes":  3,
  "droppedFrames":   0,
  "frames": [
    { "rawHex": "...", "decoderOutput": "...", "rows": [["1.23"]] },
    { "rawHex": "...", "decoderOutput": "...", "rows": [[]] }
  ]
}
```

Common failure shapes and what they mean:

- `extractedCount == 0` -- delimiters / detection mode never matched.
  The bytes never reached the parser. Check `frameStart` / `frameEnd`
  and `hexadecimalDelimiters`; consider `frameDetection: 2`
  (NoDelimiters) for fixed-size or self-framing protocols (COBS,
  protobuf, length-prefixed).
- `extractedCount > 0` but `rows: []`: the parser ran and returned
  nothing useful. Either `parse()` returned `nil` / non-array, or it
  threw an error (Lua's `pcall` wrapper would swallow it). Look at
  `decoderOutput`: if it's full of `U+FFFD` for a binary protocol, the
  decoder is wrong (see the decoder table further down).
- `remainingBytes > 0`: bytes were left in the buffer because no
  closing delimiter / checksum boundary was found. Either the test
  input is incomplete, or the delimiter is wrong.
- `droppedFrames > 0`: input was so large the FrameReader queue
  saturated. Use a smaller test slice.

## How decoder + detection settings affect what `parse()` sees

The dryRun pipeline runs in two stages before `parse()` is called:

```
   raw stream bytes
         |
         v
   FrameReader (frame detection + delimiters + checksum)
         |
         v
   one extracted frame's bytes
         |
         v
   decoder method
         |
         v
   parse(frame)
```

Getting either stage wrong is the difference between "shows nothing"
and "shows garbage". Knowing the matrix below saves an iteration.

**Frame detection** (`frameDetection`) slices the byte stream:

| Int | Mode | Frame body is... | Pick it for |
|-----|------|------------------|-------------|
| 0 | EndDelimiterOnly | Everything up to the next end delimiter. | Line-terminated text, CSV with `\n`, NMEA with `\r\n`. |
| 1 | StartAndEndDelimiter | Everything between start and end delimiters; outside bytes are dropped. | Framed text protocols like `/* ... */`, `<...>`. |
| 2 | NoDelimiters | The whole captured chunk (the driver's chunk boundary IS the frame boundary). | Fixed-size binary, COBS (parser splits on `0x00`), protobuf over UDP, length-prefixed binary. |
| 3 | StartDelimiterOnly | Between this start marker and the next one. | Protocols that lead with a sync word but have no terminator. |

When `frameDetection` requires a delimiter that's empty or missing,
extraction silently downgrades to NoDelimiters. That's usually NOT
what the user wants: if you set `frameStart: ""` you almost always
need to flip detection mode too.

**Decoder method** (`decoderMethod`) decides what `parse()` receives:

| Int | Decoder | Parser receives | Trap |
|-----|---------|-----------------|------|
| 0 | PlainText | A UTF-8 string built by `QString::fromUtf8(bytes)`. | **Binary bytes are corrupted.** Any non-UTF-8 byte (most of `0x80..0xFF`, every `0x00`) becomes `U+FFFD` and the original byte is lost. Picking PlainText for COBS / Modbus / protobuf is the #1 silent failure. |
| 1 | Hexadecimal | A hex string, no spaces (`48656C6C6F`). | Doubles the byte count; works on any input. Use when you want to read bytes from a string-only parser. |
| 2 | Base64 | A Base64 string. | When the device already emits Base64; otherwise just a coding overhead. |
| 3 | Binary | Raw bytes: a 1-indexed byte table in Lua, a length-keyed object in JS. | The only decoder that round-trips arbitrary binary. Pick this for every non-text protocol. |

Rule of thumb: **if the user mentions COBS, Modbus, protobuf, CAN,
audio, or "binary", `decoderMethod: 3` is right.** If they mention
NMEA, AT-commands, CSV, line-based, or "text", `decoderMethod: 0` is
right. Hex and Base64 are niche choices for cases where the device
itself ships an encoded payload.

## Common patterns

```js
// CSV (the default)
function parse(frame) { return frame.split(','); }

// Key=value pairs (e.g. "speed=42.1,heading=180")
function parse(frame) {
  const kv = {};
  frame.split(',').forEach(p => {
    const [k, v] = p.split('=');
    kv[k] = v;
  });
  return [kv.speed || 0, kv.heading || 0];
}

// Binary little-endian (frame is a byte string)
function parse(frame) {
  const buf = new Uint8Array(frame.length);
  for (let i = 0; i < frame.length; i++) buf[i] = frame.charCodeAt(i);
  const view = new DataView(buf.buffer);
  return [view.getFloat32(0, true), view.getInt16(4, true)];
}
```

## NMEA / hex / domain protocols

For NMEA, AT-commands, hex-encoded, base64, COBS, TLV, see
`scripts.list{kind: "frame_parser_js"}`: the codebase includes ~10
reference parsers for these patterns. Adapt the closest match.

## Gotchas

- `parse()` must return an **array** (JS) or **table** (Lua). Returning
  an object / map / `nil` silently fails; the dashboard sees zero
  datasets.
- Each return-array index `i` maps to the dataset whose `index` is
  `i + 1`. Off-by-one errors are common; verify with a dryRun first.
- The string vs binary distinction is set by `decoderMethod`, NOT by
  the console display mode. `console.setDataMode` only changes how the
  terminal renders bytes; the parser keeps receiving whatever
  `decoderMethod` produced. For binary protocols, set
  `decoderMethod: 3` (Binary) on the source.
- Lua `pcall` inside `parse()` will silently swallow runtime errors and
  return a partial result. If a Lua parser populates only the first
  dataset of an N-dataset frame, suspect an exception mid-body
  (typos, nil arithmetic, off-by-one indexing). Run dryRun and
  scrutinize `decoderOutput` vs the rows it produced.
- Top-level `var` / `local` persists across calls; that's how you keep
  buffers, state, EMAs across frames.

## Closed-loop control: `deviceWrite()` and `actionFire()`

Parsers can drive output without going through a widget:

- `deviceWrite(data, sourceId?)`: synchronous fire-and-forget byte write.
  Returns `{ ok, error? }`, never throws. `data` is a Lua string or JS
  string / byte array. `sourceId` defaults to the source this parser
  belongs to. Logged `[deviceWrite] source=N bytes=M written=K`.
- `actionFire(actionId)`: fires an existing project Action by its
  stable `actionId` (NOT its index). Reuses the action's payload,
  encoding, and timer mode. Same return shape. Logged
  `[actionFire] id=N index=M ok`.

Use for acks, alarms, or status requests decided by the incoming frame.
For user-triggered commands, build an Output Widget instead.

## Dashboard controls

Seven runtime UI helpers, all `{ ok, error? }`, NO logging:

- `clearPlots()`: wipe line / multiplot / FFT / GPS / 3D / waterfall buffers without rebuilding widgets, datasets, or actions. Typical use: reset a GPS trace the moment the first valid fix arrives.
- `setPlotPoints(n)`: horizontal sample window for line plots (n >= 1).
- `setTerminalVisible(bool)`, `setNotificationLogVisible(bool)`, `setClockVisible(bool)`, `setStopwatchVisible(bool)`: show/hide the corresponding dashboard widgets.
- `setActiveWorkspace(idOrName)`: switch the active workspace tab. Pass a numeric `workspaceId` (>= 1000) or a case-insensitive title string.

Latch every call behind a state transition (a top-level `var` / `local` flag). Calling them per frame produces empty plots, flicker, or workspace-yank. They affect the active dashboard window only and do NOT persist to the project file or QSettings.
