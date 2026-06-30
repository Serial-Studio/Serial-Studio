# Frame Parser API — JavaScript

A frame parser converts raw bytes (or pre-split text) emitted by a device into
the array of dataset values the dashboard consumes.

## Contract

You define a single function taking exactly **one** parameter:

```js
function parse(frame) {
  // returns Array<number | string>
}
```

- `frame`'s shape depends on the source's selected decoder: PlainText gives
  a UTF-8 `String`, Hexadecimal/Base64 give the encoded text as a `String`,
  and Binary gives an `Array` of byte values (0–255).
- **Never write `parse(frame, separator)`.** That two-parameter signature is
  a Serial Studio v2 relic, removed years ago; the validator rejects it when
  the script is loaded for authoring or dry-run, and even where a legacy
  script slips through, `separator` is `undefined`. There is no separator
  argument — split the frame yourself with the delimiter your protocol uses
  (e.g. `frame.split(',')`).
- Return an `Array` whose values map **positionally** to the datasets in the
  project (in group/dataset declaration order).
- Returning `[]` or `null` skips the frame silently. Extra values are
  ignored; datasets whose index exceeds the returned length keep their
  previous value.

## Sandbox

The QJSEngine running parsers is isolated from the host system. Available:
`console`, `gc()`, plain JS built-ins, and the injected host API —
`deviceWrite`, `actionFire`, the seven dashboard helpers,
`tableGet`/`tableSet`/`datasetGetRaw`/`datasetGetFinal`, the handle fast path
`tableHandle`/`tableHandleMany`/`tableGetH`/`tableSetH`, `notify*`,
`apiCall`, and the generated `SerialStudio` SDK (`io.*`, `project.*`, ...):

- **No** `XMLHttpRequest`, `fetch`, `setTimeout`, `setInterval`.
- **No** filesystem, network, or Qt API access.
- **No** `import`/`require`. Single-file scripts only.

Each source runs its parser in its own engine; top-level `var` declarations
are real globals within that per-source engine, so they don't bleed into
other sources' parsers.

The data-table API is injected here too. A table inside folders is addressed
by its full `/`-joined folder path (top-level tables are bare names);
`project.dataTable.list` reports that path. For a parser that writes many
registers per frame, resolve handles once at load with `tableHandle` and use
`tableGetH`/`tableSetH` on the hot path. Full reference in the transform and
SDK docs.

## Performance

This function runs on every frame, sometimes at 10+ kHz. Cheap operations:

- `String.prototype.split`
- `parseInt`, `parseFloat`, `Number`
- `String.prototype.match` with a precompiled `RegExp`
- Numeric arithmetic

Avoid:

- Building intermediate objects per call
- Catching exceptions in the hot path — let parse errors bubble; the runtime
  watchdog handles them
- `JSON.parse` on every frame unless the protocol is JSON-line

## Examples

### CSV split (most common)

```js
function parse(frame) {
  return frame.split(',');
}
```

### Tab-delimited with numeric coercion

```js
function parse(frame) {
  const parts = frame.split('\t');
  const out = new Array(parts.length);
  for (let i = 0; i < parts.length; ++i)
    out[i] = parseFloat(parts[i]);
  return out;
}
```

### Regex extraction

```js
const RE = /^\$([A-Z]+),(-?\d+\.\d+),(-?\d+\.\d+),(-?\d+)\*([0-9A-F]{2})/;

function parse(frame) {
  const m = frame.match(RE);
  if (!m)
    return [];
  return [parseFloat(m[2]), parseFloat(m[3]), parseInt(m[4], 10)];
}
```

### JSON-line

```js
function parse(frame) {
  try {
    const obj = JSON.parse(frame);
    return [obj.x, obj.y, obj.z];
  } catch (_e) {
    return [];
  }
}
```

### Binary frame (index the byte array)

With the Binary decoder, `frame` is an `Array` of byte values (0–255), not a
string — string methods throw (`frame.split is not a function`). Index it
directly; wrap it in a `DataView` for multi-byte or float fields:

```js
function parse(frame) {
  if (frame.length < 6)
    return [];
  const view = new DataView(Uint8Array.from(frame).buffer);
  return [view.getFloat32(0, true), view.getInt16(4, true)];
}
```

### Batched samples (one frame yields N rows)

If your device emits multiple sensor rows per frame, return a 2-D array;
the FrameBuilder will flatten to multiple frames.

```js
function parse(frame) {
  const rows = frame.split(';');
  return rows.map(r => r.split(',').map(parseFloat));
}
```

## Device output — `deviceWrite()`

```js
deviceWrite(data, sourceId?) -> { ok: true } | { ok: false, error: "..." }
```

- `data` is a string (UTF-8 encoded on the wire) or an array of byte values
  (0–255).
- `sourceId` is optional; default is the source this parser belongs to.
- Synchronous, fire-and-forget. Never throws. Each call is logged as
  `[deviceWrite] source=N bytes=M written=K`.

Use for closed-loop control from inside `parse()`: ack a frame, raise an
alarm, request a status push. Not for user-triggered actions — those
belong on an Output Widget.

```js
function parse(frame) {
  const values = frame.split(",");
  if (parseFloat(values[0]) > 80) {
    const r = deviceWrite("ALARM=1\n");
    if (!r.ok) console.warn("alarm write failed:", r.error);
  }
  return values;
}
```

## Firing project actions — `actionFire()`

```js
actionFire(actionId) -> { ok: true } | { ok: false, error: "..." }
```

Reuses an existing project Action (with its prebuilt payload + timer
mode). `actionId` is the action's stable id, NOT its index in the list.
Same shape as `deviceWrite`; never throws.

## Dashboard controls

Seven UI helpers, all returning `{ ok: true }` or `{ ok: false, error: "..." }`. None of them log. Call once on a state transition, NOT on every frame.

```js
clearPlots()                          // wipe line/multiplot/FFT/GPS/3D/waterfall buffers
setPlotPoints(n)                      // horizontal sample window (n >= 1)
setTerminalVisible(visible)           // bool
setNotificationLogVisible(visible)    // bool
setClockVisible(visible)              // bool
setStopwatchVisible(visible)          // bool
setActiveWorkspace(idOrName)          // workspaceId (int >= 1000) OR title (case-insensitive)
```

Typical use: reset a GPS trace the moment a valid fix arrives so the line from origin to first real sample disappears.

```js
var hadFix = false;
function parse(frame) {
  const [lat, lon, q] = frame.split(",").map(parseFloat);
  if (!hadFix && q > 0) {
    clearPlots();
    hadFix = true;
  }
  return [lat, lon, q];
}
```

These affect the active dashboard window only. They do NOT modify the project file or the user's persisted preferences.

## Errors

Throwing an exception logs a watchdog warning and skips the frame. Returning
non-numeric strings for a numeric dataset coerces via `Number()`; `NaN`
becomes `0`.

## Multi-source projects

Each source has its own parser script; you cannot reach across sources from
a parser. To compute cross-source values, use a per-dataset transform with
the `tableGet`/`datasetGetFinal` API instead.
