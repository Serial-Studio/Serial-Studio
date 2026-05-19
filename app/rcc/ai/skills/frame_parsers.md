# Frame Parsers

Frame parsers turn one logical frame (the bytes between `frameStart` and
`frameEnd`) into an array of dataset values. Each Source has its own parser.

## Decision: do you need a parser?

- **Yes**: framing is line-delimited, comma-separated, or has any
  structure beyond raw bytes. The default project template ships with a
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
— the dashboard receives no data and there's no popup. The API now
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
     parsed values, then ask the user to paste an example raw frame.
   - If disconnected: ask for a sample.
3. **Compile-only**: `project.frameParser.dryCompile{code, language}`
   verifies the script parses and `parse(frame)` is defined, without
   executing. Cheapest way to catch the wrong-language silent failure
   (Lua code with `language: 0`, or the reverse).
4. **Dry-run**: `project.frameParser.dryRun{code, language, sampleFrame}`
   compiles the script and runs `parse(sample)` in a throwaway engine.
   Returns the rows or compile/runtime errors. For STATEFUL parsers
   (top-level closures, EMA-style state, frame-assembly buffers), pass
   `sampleFrames: ["...","...",...]` to feed several frames sequentially
   through one engine instance and see how state evolves.
5. Push: `project.frameParser.setCode{code, language, sourceId}`.
6. Auto-save will write to disk within ~1s.

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
`scripts.list{kind: "frame_parser_js"}` — the codebase ships ~10
reference parsers for these patterns. Adapt the closest match.

## Gotchas

- `parse()` must return an **array**. Returning an object silently fails;
  the dashboard sees zero datasets.
- Each return-array index `i` maps to the dataset whose `index` is
  `i + 1`. Off-by-one errors are common; verify with a dryRun first.
- Strings are UTF-8 by default. For binary protocols, set
  `console.setDataMode = 1` (Hex) on the user's request, but the parser
  still receives the decoded bytes as a string in JS.
- Top-level `var` persists across calls — that's how you keep buffers,
  state, EMAs across frames.

## Closed-loop control: `deviceWrite()` and `actionFire()`

Parsers can drive output without going through a widget:

- `deviceWrite(data, sourceId?)` — synchronous fire-and-forget byte write.
  Returns `{ ok, error? }`, never throws. `data` is a Lua string or JS
  string / byte array. `sourceId` defaults to the source this parser
  belongs to. Logged `[deviceWrite] source=N bytes=M written=K`.
- `actionFire(actionId)` — fires an existing project Action by its
  stable `actionId` (NOT its index). Reuses the action's payload,
  encoding, and timer mode. Same return shape. Logged
  `[actionFire] id=N index=M ok`.

Use for acks, alarms, or status requests decided by the incoming frame.
For user-triggered commands, build an Output Widget instead.

## Dashboard controls

Seven runtime UI helpers, all `{ ok, error? }`, NO logging:

- `clearPlots()` — wipe line / multiplot / FFT / GPS / 3D / waterfall buffers without rebuilding widgets, datasets, or actions. Typical use: reset a GPS trace the moment the first valid fix arrives.
- `setPlotPoints(n)` — horizontal sample window for line plots (n >= 1).
- `setTerminalVisible(bool)`, `setNotificationLogVisible(bool)`, `setClockVisible(bool)`, `setStopwatchVisible(bool)` — show/hide the corresponding dashboard widgets.
- `setActiveWorkspace(idOrName)` — switch the active workspace tab. Pass a numeric `workspaceId` (>= 1000) or a case-insensitive title string.

Latch every call behind a state transition (a top-level `var` / `local` flag). Calling them per frame produces empty plots, flicker, or workspace-yank. They affect the active dashboard window only and do NOT persist to the project file or QSettings.
