# Painter Widget API — JavaScript

The Painter widget (Pro) gives you a Canvas-style 2D drawing surface. Each
group with `widgetType: 8` (GroupWidget::Painter) gets its own painter
script. Bind by calling `project.painter.setCode {groupId, code}`.

A painter group can be **empty** (zero datasets). It will then draw using
peer datasets read via `datasetGetFinal(uniqueId)` and shared registers
read via `tableGet(table, register)`. Don't duplicate datasets just to
feed a painter — see "Reading peer datasets" below.

## Entry points

The script defines TWO functions. Names must match exactly.

- **`paint(ctx, w, h)` — REQUIRED.** Called every UI tick (24 Hz default)
  to redraw the canvas. `ctx` is a Canvas-2D-like context. `w` and `h` are
  the current widget size in pixels. Treat the canvas as ephemeral — clear
  what you need at the top of each call.

- **`onFrame()` — OPTIONAL.** Called once per parsed frame, before the
  next paint, with no arguments. Read live values via
  `datasets[i].value` (etc.) or via `datasetGetFinal(uniqueId)` and cache
  them in top-level `var`s if you need to do per-frame computation outside
  the per-tick `paint`.

`bootstrap()` does **not** exist. Top-level statements at the script's
outer scope run once when the script compiles; that is your "bootstrap".

The function name is **`paint`**, not `draw`, not `render`. The engine
looks up `globalThis.paint` by name and reports
`missing paint(ctx, w, h) function` if it's absent.

## Globals injected before your script runs

These are available as soon as your script starts; you don't import them.

```js
// datasets[i] -- read-only proxy. May be empty (length === 0) for a
// painter group with no own datasets. Numeric indexing + .length.
datasets[0].value         // numeric current value (post-transform)
datasets[0].rawValue      // numeric value before transforms
datasets[0].text          // string form of value
datasets[0].rawText       // string form of rawValue
datasets[0].title         // user-set title
datasets[0].units         // user-set units
datasets[0].uniqueId      // runtime id
datasets[0].widget        // widget hint string ("bar", "gauge", ...)
datasets[0].min           // dataset min (any of the *Min hints)
datasets[0].max           // dataset max
datasets[0].plotMin / .plotMax       // plot Y-axis bounds
datasets[0].widgetMin / .widgetMax   // gauge / bar bounds
datasets[0].fftMin / .fftMax         // FFT bounds
datasets[0].alarmLow / .alarmHigh    // alarm thresholds
datasets[0].ledHigh                  // LED activation threshold
datasets[0].hasPlot / .hasFft / .hasLed
datasets.length

// group -- the host group's project metadata
group.id
group.title
group.columns
group.sourceId

// frame -- the most recent parsed frame's metadata
frame.number      // monotonic frame counter
frame.timestampMs

// theme -- ThemeManager palette, always in sync with the active theme.
// PRIMARY keys for painters (these match what the QML dashboard widgets
// already use, so a painter-rendered widget visually fits the rest of the
// dashboard):
theme.widget_base       // canvas / card background
theme.widget_border     // grid lines, frames, outlines
theme.widget_text       // primary value labels and titles
theme.widget_highlight  // primary signal / value colour
theme.alarm             // red, used for alarm states / second hand / errors
theme.placeholder_text  // muted secondary labels
theme.pane_section_label // small caption text
theme.widget_colors     // ARRAY of per-channel distinct colors, indexable
                        // as theme.widget_colors[i % theme.widget_colors.length]
// Other keys also exist (text, window, base, mid, highlight, accent,
// link, error, groupbox_background, ...) but prefer the widget_* ones
// above for painter content -- they're tuned for in-widget rendering.

// console -- log/warn/error to the editor status pane
console.log("...")
console.warn("...")
console.error("...")

// Data-table API (shared with transforms; see "Tables and registers")
tableGet(tableName, registerName)              // -> number | string
tableSet(tableName, registerName, value)       // user-table writes only
datasetGetRaw(uniqueId)                        // raw value of any dataset
datasetGetFinal(uniqueId)                      // final value of any dataset

// Closed-loop control APIs (shared with parsers and transforms)
deviceWrite(data, sourceId?)                   // -> { ok, error? }
actionFire(actionId)                           // -> { ok, error? }

// Dashboard helpers (shared with parsers and transforms)
clearPlots()                                   // wipe plot/multiplot/FFT/GPS/3D/waterfall
setPlotPoints(n)                               // horizontal sample window (n >= 1)
setTerminalVisible(visible)                    // bool
setNotificationLogVisible(visible)             // bool
setClockVisible(visible)                       // bool
setStopwatchVisible(visible)                   // bool
setActiveWorkspace(idOrName)                   // workspaceId (>= 1000) OR title
```

`deviceWrite`'s default `sourceId` follows the painter's group sourceId
(updated automatically when the host group changes). `actionFire` fires a
project Action by its stable `actionId`. The dashboard helpers all return
`{ ok, error? }` and never log.

**Call these from `onFrame()`, NOT `paint()`.** `paint` runs on every
dashboard tick (24 Hz); a write or `clearPlots` per paint will saturate
the link / yank the plot. Use `onFrame()` (one call per parsed frame) or
guard with a state machine.

## Top-level state

Top-level `var` is preserved across `paint` calls; that's how you cache
geometry, gradients, and counters.

- Don't `Object.defineProperty` / `Object.freeze` your top-level state.
  The engine recompiles the script when you save edits and re-evaluation
  must be able to reassign the locals.
- Use `var` (not `const` / `let`) for anything you intend to mutate
  across calls.

## Reading peer datasets — DO NOT duplicate datasets for the painter

Painter groups address peer data by **uniqueId**, not by dataset index
inside the painter group. The right pattern:

1. Run `project.dataset.list` (or `project.group.list`'s `datasetSummary`)
   and read the `uniqueId` of each dataset you want to render. The id is
   stable for a given (sourceId, groupId, datasetId) triple.
2. Call `datasetGetFinal(uniqueId)` (post-transform) or
   `datasetGetRaw(uniqueId)` (pre-transform) inside `paint`/`onFrame`.

```js
// Peer-dataset uniqueIds discovered ahead of time via dataset.list.
var SPEED_UID  = 10001;
var RPM_UID    = 10002;

function paint(ctx, w, h) {
  var speed = datasetGetFinal(SPEED_UID) || 0;
  var rpm   = datasetGetFinal(RPM_UID)   || 0;
  // ...draw using speed and rpm without owning any datasets in this group
}
```

Why not duplicate datasets into the painter group? Because every dataset
in a project costs a frame-table register, an exporter column, a
dashboard row, and a project-file entry. Painters that just READ values
should not own them.

## Tables and registers — the central data bus

Data tables let multiple scripts (transforms, painters) share state.
Two register types:

- **Constant**: holds a single immutable value across the whole session.
  Set when you declare it; every `tableGet` thereafter returns the same
  value. Use for: calibration coefficients, lookup tables, configuration
  flags.
- **Computed**: resets to its `defaultValue` at the **start of every
  parsed frame**. Transforms (and only transforms) write to it via
  `tableSet`. Use for: per-frame accumulators, derived values, intermediate
  results that another transform will read later in the same frame.

A system-managed table named `__datasets__` is **always** present. It
holds two registers per dataset: `raw:<uniqueId>` and `final:<uniqueId>`,
populated by the FrameBuilder. You almost never read it through
`tableGet` — use `datasetGetRaw(uniqueId)` / `datasetGetFinal(uniqueId)`
instead, which are faster and more legible.

For your own tables:

```
project.dataTable.add {name: "Calibration"}
project.dataTable.addRegister {table: "Calibration", name: "offset",
                               computed: false, defaultValue: 0}
project.dataTable.addRegister {table: "Calibration", name: "scale",
                               computed: false, defaultValue: 1}
```

then in any script:

```js
var offset = tableGet("Calibration", "offset");
var scale  = tableGet("Calibration", "scale");
return raw * scale + offset;   // a transform body
```

Per-frame ordering: transforms run in group-array then dataset-array
order. Inside a transform you can read **raw values for ALL datasets**
and **final values only for datasets earlier in the order**. Painters
run AFTER all transforms, so painters see final values for everything.

## arc / moveTo discipline

The Painter renderer requires `moveTo` BEFORE `arc` whenever you want the
arc to start a new subpath. Without `moveTo`, the arc is implicitly
connected to the previous path endpoint and may render a stray chord:

```js
ctx.beginPath();
ctx.moveTo(cx + r, cy);          // explicit start point
ctx.arc(cx, cy, r, 0, Math.PI);  // arc opens cleanly
ctx.stroke();
```

For full circles, the order is the same — `moveTo` to the perimeter, then
`arc` to `2*Math.PI`.

## Color from theme — never hard-code hex

Use the `widget_*` family for in-painter content so the canvas matches
the rest of the dashboard:

```js
ctx.fillStyle   = theme.widget_base;       // background
ctx.strokeStyle = theme.widget_border;     // grid / frame
ctx.fillStyle   = theme.widget_text;       // value labels
ctx.fillStyle   = theme.widget_highlight;  // primary value / signal
ctx.fillStyle   = theme.alarm;             // alarm / red accent
```

For multi-channel painters (one color per dataset), index into the
`widget_colors` array — the dashboard's plot/multiplot widgets use the
same palette:

```js
const palette = theme.widget_colors;
for (let i = 0; i < datasets.length; i++) {
  ctx.strokeStyle = palette[i % palette.length];
  // ...trace dataset i
}
```

The `theme` global stays in sync when the user switches between light
and dark themes. Hard-coded hex breaks that.

## Worked example 1: progress rings (multi-dataset)

Lays out one circular ring per dataset. Demonstrates a square-ish grid,
arc + moveTo discipline, and a value/title centre.

```js
function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);
  ctx.strokeStyle = theme.widget_border;
  ctx.lineWidth = 2;
  ctx.strokeRect(1, 1, w - 2, h - 2);

  if (datasets.length === 0) return;

  var n    = datasets.length;
  var cols = Math.min(n, Math.max(1, Math.round(Math.sqrt(n * w / h))));
  var rows = Math.ceil(n / cols);
  var pad  = 12;
  var cw   = (w - pad * 2) / cols;
  var ch   = (h - pad * 2) / rows;

  for (var i = 0; i < n; i++) {
    var col = i % cols;
    var row = Math.floor(i / cols);
    var cx  = pad + cw * col + cw * 0.5;
    var cy  = pad + ch * row + ch * 0.5 - 6;
    var rr  = Math.max(10, Math.min(cw, ch) * 0.38 - 4);
    var ds  = datasets[i];
    var v   = Number.isFinite(ds.value) ? ds.value : 0;
    var span = (ds.max - ds.min) || 1;
    var norm = Math.max(0, Math.min(1, (v - ds.min) / span));

    // Track ring (full circle, the same color the dashboard uses for grid lines).
    ctx.strokeStyle = theme.widget_border;
    ctx.lineWidth   = 12;
    ctx.lineCap     = "butt";
    ctx.beginPath();
    ctx.moveTo(cx + rr, cy);
    ctx.arc(cx, cy, rr, 0, Math.PI * 2);
    ctx.stroke();

    // Value arc -- pick a per-channel color from theme.widget_colors so
    // multi-channel painters use the same palette as multiplots.
    if (norm > 0) {
      var startA = -Math.PI * 0.5;
      var endA   = startA + Math.PI * 2 * norm;
      var palette = theme.widget_colors || [theme.widget_highlight];
      ctx.strokeStyle = palette[i % palette.length];
      ctx.lineCap     = "round";
      ctx.beginPath();
      ctx.moveTo(cx + Math.cos(startA) * rr, cy + Math.sin(startA) * rr);
      ctx.arc(cx, cy, rr, startA, endA);
      ctx.stroke();
      ctx.lineCap = "butt";
    }

    // Centre value.
    ctx.fillStyle    = theme.widget_text;
    ctx.font         = "bold 18px sans-serif";
    ctx.textAlign    = "center";
    ctx.textBaseline = "middle";
    ctx.fillText((norm * 100).toFixed(0) + "%", cx, cy);

    // Title under the ring.
    ctx.fillStyle    = theme.placeholder_text || theme.widget_text;
    ctx.font         = "10px sans-serif";
    ctx.textBaseline = "alphabetic";
    ctx.fillText((ds.title || "").substring(0, 16), cx, cy + rr + 18);
  }

  ctx.textAlign    = "start";
  ctx.textBaseline = "alphabetic";
}
```

## Worked example 2: status grid (peer-dataset reads, no own datasets)

Demonstrates a painter group with `datasets.length === 0`. The script
reads **peer** datasets by uniqueId (discovered ahead of time via
`project.dataset.list`) and renders a coloured status tile per channel.

```js
// Peer dataset uniqueIds, discovered via project.dataset.list. Each
// constant is set once at script load; edit the script if your project
// layout changes.
var CHANNELS = [
  { uid: 10001, label: "Voltage",   warn: 11.0, alarm: 10.5 },
  { uid: 10002, label: "Current",   warn: 8.0,  alarm: 9.5  },
  { uid: 10003, label: "Temp",      warn: 70,   alarm: 85   },
  { uid: 10004, label: "Pressure",  warn: 110,  alarm: 130  }
];

function tileColor(value, c) {
  if (!Number.isFinite(value)) return theme.widget_border;
  if (value >= c.alarm) return theme.alarm;
  if (value >= c.warn)  return theme.accent;
  return theme.widget_highlight;
}

function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  var pad  = 8;
  var cols = 2;
  var rows = Math.ceil(CHANNELS.length / cols);
  var cw   = (w - pad * (cols + 1)) / cols;
  var ch   = (h - pad * (rows + 1)) / rows;

  for (var i = 0; i < CHANNELS.length; i++) {
    var c    = CHANNELS[i];
    var x    = pad + (i % cols) * (cw + pad);
    var y    = pad + Math.floor(i / cols) * (ch + pad);
    var v    = datasetGetFinal(c.uid);

    ctx.fillStyle = tileColor(v, c);
    ctx.fillRect(x, y, cw, ch);

    ctx.fillStyle    = theme.highlighted_text || theme.widget_text;
    ctx.font         = "bold 14px sans-serif";
    ctx.textAlign    = "left";
    ctx.textBaseline = "top";
    ctx.fillText(c.label, x + 8, y + 8);

    ctx.font         = "bold 22px sans-serif";
    ctx.textBaseline = "bottom";
    var label = Number.isFinite(v) ? v.toFixed(1) : "—";
    ctx.fillText(label, x + 8, y + ch - 8);
  }

  ctx.textBaseline = "alphabetic";
  ctx.textAlign    = "start";
}
```

## Worked example 3: clock face (zero datasets, frame.timestampMs)

Demonstrates a painter that uses NO dataset at all and just renders
something useful from `frame.timestampMs`.

```js
function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  var cx = w * 0.5;
  var cy = h * 0.5;
  var r  = Math.min(w, h) * 0.42;

  // Dial face.
  ctx.fillStyle = theme.alternate_base || theme.widget_base;
  ctx.beginPath();
  ctx.moveTo(cx + r, cy);
  ctx.arc(cx, cy, r, 0, Math.PI * 2);
  ctx.fill();

  ctx.strokeStyle = theme.widget_border;
  ctx.lineWidth   = 3;
  ctx.beginPath();
  ctx.moveTo(cx + r, cy);
  ctx.arc(cx, cy, r, 0, Math.PI * 2);
  ctx.stroke();

  // Hour numerals.
  ctx.fillStyle    = theme.widget_text;
  ctx.font         = "bold 14px serif";
  ctx.textAlign    = "center";
  ctx.textBaseline = "middle";
  for (var i = 1; i <= 12; i++) {
    var ang = (i / 12) * Math.PI * 2 - Math.PI / 2;
    ctx.fillText("" + i,
                 cx + Math.cos(ang) * r * 0.82,
                 cy + Math.sin(ang) * r * 0.82);
  }

  // Hands from wall-clock time.
  var now  = new Date(frame.timestampMs || Date.now());
  var sec  = now.getSeconds();
  var min  = now.getMinutes() + sec / 60;
  var hour = (now.getHours() % 12) + min / 60;

  function hand(angle, length, width, color) {
    ctx.strokeStyle = color;
    ctx.lineWidth   = width;
    ctx.lineCap     = "round";
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(cx + Math.cos(angle) * length, cy + Math.sin(angle) * length);
    ctx.stroke();
  }
  hand((hour / 12) * Math.PI * 2 - Math.PI / 2, r * 0.50, 6, theme.widget_text);
  hand((min  / 60) * Math.PI * 2 - Math.PI / 2, r * 0.72, 4, theme.widget_text);
  hand((sec  / 60) * Math.PI * 2 - Math.PI / 2, r * 0.85, 1, theme.alarm);

  ctx.lineCap = "butt";
  ctx.textAlign = "start";
  ctx.textBaseline = "alphabetic";
}
```

## More references

The Pro build ships ~18 reference painter scripts. Read them with:

```
scripts.list {kind: "painter"}    -> [{id, name, datasetCount}, ...]
scripts.get  {kind: "painter", id: "<id>"}   -> {body: "<full source>"}
```

Adapt the closest match instead of writing from scratch.

## Canvas context — supported API

The `ctx` argument behaves like a browser `CanvasRenderingContext2D`. The
list below is exhaustive: anything not listed is not implemented and
calling it from your script will throw or no-op.

**Style state**
- `fillStyle`, `strokeStyle` — accept a CSS color string OR a gradient
  object OR a pattern object (see "Gradients & patterns" below).
- `lineWidth`, `lineCap` ("butt" / "round" / "square"),
  `lineJoin` ("miter" / "round" / "bevel"), `miterLimit`.
- `setLineDash([on, off, ...])`, `getLineDash()`, `lineDashOffset`.
- `globalAlpha` (clamped to [0, 1]).
- `globalCompositeOperation`: source-over (default), source-in, source-out,
  source-atop, destination-over, destination-in, destination-out,
  destination-atop, lighter, copy, xor, multiply, screen, overlay, darken,
  lighten, color-dodge, color-burn, hard-light, soft-light, difference,
  exclusion.
- Shadows: `shadowColor`, `shadowBlur`, `shadowOffsetX`, `shadowOffsetY`.
  Applied to `fill`, `stroke`, `fillRect`, `strokeRect`, `fillText`,
  `strokeText`. A box-blur approximation is used for `shadowBlur > 0`;
  large blur radii are expensive — keep shadows out of per-frame hotpaths
  if you can.
- `imageSmoothingEnabled`, `imageSmoothingQuality` ("low" / "medium" /
  "high"; Qt has only one filter so quality is recorded for parity but not
  switched).

**Paths**
- `beginPath`, `closePath`, `moveTo`, `lineTo`, `rect`,
  `roundRect(x, y, w, h, radii)` (radii: number, [r], [tl, tr], [tl, tr, br],
  or [tl, tr, br, bl]), `arc(x, y, r, start, end, ccw?)`,
  `arcTo(x1, y1, x2, y2, r)`,
  `ellipse(x, y, rx, ry, rotation, start, end, ccw?)`,
  `quadraticCurveTo`, `bezierCurveTo`.
- Path consumers: `fill`, `stroke`, `clip`, `isPointInPath(x, y)`,
  `isPointInStroke(x, y)`.

**Rectangles**
- `fillRect`, `strokeRect`, `clearRect`.

**Transforms**
- `save`, `restore`.
- `translate`, `rotate(rad)`, `scale(sx, sy)`.
- `transform(a, b, c, d, e, f)` (multiply current matrix),
  `setTransform(a, b, c, d, e, f)` (replace),
  `resetTransform()`.
- `getTransform()` returns `{a, b, c, d, e, f}`.

**Text**
- `font` (CSS-like shorthand: "12px sans-serif", "bold 14px Arial",
  "italic 700 16px monospace"). Generic families (sans-serif, serif,
  monospace) map to the dashboard's CommonFonts.
- `textAlign` ("start" / "end" / "left" / "right" / "center"),
  `textBaseline` ("top" / "hanging" / "middle" / "alphabetic" / "bottom").
- `fillText(text, x, y)`, `strokeText(text, x, y)`.
- `measureText(s)` returns `{ width, actualBoundingBoxAscent,
  actualBoundingBoxDescent, fontBoundingBoxAscent,
  fontBoundingBoxDescent }`.
- `measureTextWidth(s)` is a fast shortcut returning just the width.

**Images**
- `drawImage(src, x, y)` and `drawImage(src, x, y, w, h)` — source must be
  a `qrc:/` resource OR a path inside the project file's directory tree.
  URLs (`http://`, `https://`, etc.) are rejected for sandbox reasons.
- `createPattern(src, repetition)` where repetition is "repeat" (default),
  "repeat-x", "repeat-y", or "no-repeat". Bind the returned pattern via
  `ctx.fillStyle = pattern`.

**Gradients**
- `createLinearGradient(x0, y0, x1, y1)` -> gradient.
- `createRadialGradient(x0, y0, r0, x1, y1, r1)` -> gradient.
- `createConicGradient(startRadians, cx, cy)` -> gradient.
- `gradient.addColorStop(offset, color)` (offset clamped to [0, 1]).
- Bind via `ctx.fillStyle = gradient` or `ctx.strokeStyle = gradient`.
- A single gradient handle can be reused across paint() calls — cache it
  in a top-level `var`.

**Geometry**
- `ctx.width`, `ctx.height` mirror the `w` / `h` arguments to `paint`.

**Not supported (will throw or no-op)**
- `drawImage` from network URLs.
- `putImageData`, `getImageData`, `createImageData`.
- `drawFocusIfNeeded`, `scrollPathIntoView`.
- `filter` (CSS filter strings).
- Pixel-level access in general — Canvas2D image-data APIs return
  `Uint8ClampedArray`, which QJSEngine cannot bridge cheaply.

Gradient/pattern example:

```js
var g = null;
function paint(ctx, w, h) {
  if (!g) {
    g = ctx.createLinearGradient(0, 0, 0, h);
    g.addColorStop(0,   theme.widget_highlight);
    g.addColorStop(0.5, theme.accent);
    g.addColorStop(1,   theme.alarm);
  }
  ctx.fillStyle = g;
  ctx.fillRect(0, 0, w, h);

  ctx.shadowColor   = "rgba(0,0,0,0.45)";
  ctx.shadowBlur    = 8;
  ctx.shadowOffsetY = 2;
  ctx.fillStyle     = theme.widget_text;
  ctx.font          = "bold 24px sans-serif";
  ctx.fillText("Live", 12, 32);
  ctx.shadowBlur = 0;
  ctx.shadowColor = "rgba(0,0,0,0)";
}
```

## Performance

`paint` runs at 24 Hz by default. Cheap: lineTo / arc / fillText calls,
arithmetic. Avoid: building intermediate strings every frame
(`toFixed(1)` is fine; `JSON.stringify` is not), creating new gradients
per call (cache in a top-level `var`).

## Errors

Throwing inside `paint` logs a watchdog warning and the canvas keeps the
last successful frame on screen. The error surfaces in the editor's
status pane.
