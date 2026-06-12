# Painter Widget

The Painter widget (Pro) provides a Canvas-2D drawing surface. One per
group with `widgetType: 8` (GroupWidget enum at create time). Bind code
via `project.painter.setCode{groupId, code}`.

**Painter scripts are JavaScript-only.** The Lua-first guidance in
the `frame_parsers` and `transforms` skills does NOT apply here. The
reason is concrete: painters draw via Canvas2D, which only exists as a
QJSEngine API surface. There is no Lua canvas API to expose; porting
one would be a large effort for marginal benefit. Don't try to author
painter code in Lua; it will not compile.

To pin a painter to a workspace, use `widgetType: "painter"`
(preferred; the back-compat integer enum is `21`) on `addWidget`.

## Decision: when to use a painter

- **Don't** for standard widgets (gauge, plot, bar, LED, compass); those
  are dataset options, not painters. Painter is one or two orders of
  magnitude more code.
- **Do** for visualizations the standard widgets can't express: status
  grids combining many datasets, custom dial faces, schematics with live
  values, oscilloscope-style traces, polar/radar plots, vector fields,
  segmented displays, pixel-pushed image outputs.

A painter group can be **empty** (zero datasets). Read peer datasets via
`datasetGetFinal(uniqueId)` instead of duplicating data into the painter
group.

## Entry points

- **`paint(ctx, w, h)`: REQUIRED.** Called every UI tick (60 Hz
  default, configurable 1-240 via `dashboard.setFps`) to
  redraw the canvas. The function name is `paint`, not `draw`, not
  `render`. The engine looks up `globalThis.paint` by name.
- **`onFrame()`: optional.** Called immediately before
  each `paint(ctx, w, h)`. No arguments. This is where you do
  expensive per-tick computation so `paint` stays cheap.
- `bootstrap()` does NOT exist. Top-level statements at the script's
  outer scope run once when the script compiles: that is your
  bootstrap.

### When `onFrame` actually fires (timing reality)

`onFrame` runs at the **dashboard's UI tick rate (60 Hz default,
configurable 1-240)**, NOT once per parsed frame. If frames arrive at
1 kHz, `onFrame` still fires only ~60 times per second, sampling
whichever dataset values were latest at tick time. So:

- **Per-tick work (FFT, downsampling, formatting, computing visible
  ranges)** → put it in `onFrame`. Doing it in `paint` makes you pay
  the cost on every redraw, including idle redraws when no new data
  arrived.
- **Per-frame work (rolling EMA, per-sample integration, alarm
  detection that must see every value)** → DOES NOT belong in
  `onFrame`. The painter will skip frames between two ticks. Move that
  computation into a per-dataset **transform**, where every parsed
  frame fires; the painter then reads the transform's output via
  `datasetGetFinal(uniqueId)`.
- **Pure layout / drawing** → keep in `paint`. It runs against
  whatever `onFrame` last computed.

### Recipe: spectrum analyzer painter

```js
// Top-level: bootstrap once at compile time
var SAMPLES = 1024;
var ring    = new Array(SAMPLES).fill(0);
var spectrum = new Array(SAMPLES / 2).fill(0);
var head    = 0;

function onFrame() {
  // Cheap: copy the latest sample into the ring
  var v = datasetGetFinal(/*uniqueId*/ 0);
  if (typeof v !== 'number') return;
  ring[head] = v;
  head = (head + 1) % SAMPLES;

  // Run FFT on tick (60 Hz default), not on every parsed frame -- a 48 kHz
  // audio source emits 48000 frames/s; we only render ~60/s, so doing
  // FFT here gets us a ~2 kHz redraw rate against a recent window.
  spectrum = computeFFT(ring, head);
}

function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);
  ctx.strokeStyle = theme.widget_highlight;
  ctx.beginPath();
  ctx.moveTo(0, h);
  for (var i = 0; i < spectrum.length; ++i) {
    var x = (i / spectrum.length) * w;
    var y = h - spectrum[i] * h;
    ctx.lineTo(x, y);
  }
  ctx.stroke();
}
```

Without `onFrame`, that FFT would run inside `paint` and recompute on
every redraw, including ones triggered by mouse hover and theme
changes. With it, `paint` only laces lines through the cached
`spectrum` array.

## Iteration workflow

1. Get current code: `project.painter.getCode{groupId}` (returns empty
   string if the group has no painter set yet).
2. Get the peer dataset uniqueIds you'll read:
   `project.dataset.list` returns `uniqueId` per dataset.
3. Read tables you'll reference: `project.dataTable.list` then
   `project.dataTable.get{name}` for each.
4. Read scripting reference: `meta.fetchScriptingDocs{kind: "painter_js"}`.
5. Adapt a real example: `scripts.list{kind: "painter"}` then
   `scripts.get{kind: "painter", id: "<closest match>"}`.
6. Dry-run: `assistant.script.dryRun{kind:"painter", code}` verifies compile + that
   `paint(ctx, w, h)` is defined. Doesn't render; runtime errors inside
   `paint` only surface when the live widget mounts.
7. Push: `assistant.script.apply{kind:"painter", groupId, code}`. It
   dry-runs first, then calls `project.painter.setCode`.

## Globals you can use

```js
datasets[i]      // proxy; .length, .value, .rawValue, .title, .units,
                 // .uniqueId, .min, .max, .widgetMin/Max, etc.
group            // .id, .title, .columns, .sourceId
frame            // .number, .timestampMs
theme            // ThemeManager palette; widget_base, widget_border,
                 // widget_text, widget_highlight, alarm, etc.
                 // theme.widget_colors is an array of per-channel colors.
console          // log/warn/error to the editor status pane
tableGet, tableSet, datasetGetRaw, datasetGetFinal
deviceWrite(data, sourceId?)  // -> {ok, error?}  -- defaults to group.sourceId
actionFire(actionId)          // -> {ok, error?}  -- triggers a project Action

// Dashboard helpers (shared with parsers and transforms; never log)
clearPlots()                  // wipe plot/multiplot/FFT/GPS/3D/waterfall
setPlotPoints(n)              // horizontal sample window (n >= 1)
setTerminalVisible(bool)
setNotificationLogVisible(bool)
setClockVisible(bool)
setStopwatchVisible(bool)
setActiveWorkspace(idOrName)  // workspaceId (>= 1000) OR title (case-insensitive)
```

`deviceWrite`, `actionFire`, and the dashboard helpers: call from
`onFrame()`, NOT `paint()`. `paint` runs every UI tick (60 Hz default);
writing or `clearPlots`-ing on every tick saturates the link / yanks
the plot. Use `onFrame()` (once per parsed frame) or guard with state.

## Color from theme

Don't hard-code hex. Use `theme.widget_base` for background,
`theme.widget_border` for grid/frame, `theme.widget_text` for labels,
`theme.widget_highlight` for the primary signal, `theme.widget_colors[i]`
for per-channel colors, `theme.alarm` for red/critical states. The
canvas tracks light/dark theme switches automatically.

## Design: readability before cleverness

A painter that looks technically impressive but takes the operator three
seconds to read is a worse painter than a boring DataGrid. Apply these
before you tune visuals:

- **One primary signal per canvas.** If you're tempted to overlay four
  traces, two needles, and a status grid on one painter, split it into
  two groups. Hick's Law: each extra independently-readable element adds
  decision time. Cap visible primitives (distinct shape kinds, not
  draw calls) at ~5 before you ask whether a second tile would be clearer.
- **State must NEVER be color-only.** Roughly 8% of male operators have
  red/green color deficiency. An alarm shown as a red dot turning green
  is invisible to them. Always pair color with a second cue: shape
  (circle vs triangle), position (a needle moving into a marked zone),
  or text (`"FAULT"` vs `"OK"`). `theme.alarm` is fine for emphasis, NOT
  for the sole signal.
- **Contrast against `theme.widget_base`, not against your assumption of
  dark mode.** Users flip themes. Foreground colors must read on both.
  Use `theme.widget_text` for labels and `theme.widget_highlight` for
  the primary signal; those are designed for the theme's background.
  Hardcoded `#FFFFFF` text vanishes on the light theme.
- **Anchor numbers with units and a reference range.** `87` floating
  alone is unreadable; `87 °C  (idle 70-90)` tells the operator both
  the value AND whether it's normal in one glance. The dataset's
  `.units`, `.widgetMin`, `.widgetMax` already carry this; read them
  off the proxy.
- **Doherty threshold: stay under 400 ms.** `paint` runs at 60 Hz by
  default (~16 ms budget) and `onFrame` runs immediately before each
  `paint`. If a single tick's `onFrame + paint` cost exceeds ~16 ms,
  the UI feels laggy regardless of how good the visualization is. Push heavy math
  into transforms (per-frame, runs on the frame thread); keep `onFrame`
  bounded; keep `paint` to layout + draw against cached arrays.
- **Peak-End on time-series.** When the painter renders a rolling window,
  the operator's memory of "is this healthy" is anchored by the most
  recent value AND the worst value in view. Highlight both (a faint
  marker on the current sample and a labeled max/min) instead of
  leaving them to read off an unmarked trace.
- **Negative space is signal.** Don't fill 100% of the canvas. Leave
  ~10% margin so axis labels, units, and current-value readouts have
  room to render without clipping when the tile is resized.

These are guidance, not lint rules. The Painter dryRun won't catch a
color-only alarm or a 600 ms onFrame; the operator will. If you're
unsure whether a design choice helps, ask the user before pushing.

## arc / moveTo discipline

The Painter renderer requires `moveTo` BEFORE `arc` to start a new
subpath. Without `moveTo`, the arc is implicitly connected to the
previous path endpoint and may render a stray chord:

```js
ctx.beginPath();
ctx.moveTo(cx + r, cy);          // explicit start
ctx.arc(cx, cy, r, 0, Math.PI);  // arc opens cleanly
ctx.stroke();
```

## Canvas subset

Pen/fill: `strokeStyle`, `fillStyle`, `lineWidth`, `lineCap`, `lineJoin`,
`miterLimit`, `setLineDash`.
Path: `beginPath`, `moveTo`, `lineTo`, `arc`, `arcTo`, `quadraticCurveTo`,
`bezierCurveTo`, `closePath`.
Fill/stroke: `fill`, `stroke`, `clearRect`, `fillRect`, `strokeRect`.
Transforms: `save`, `restore`, `translate`, `rotate`, `scale`,
`setTransform`, `resetTransform`.
Text: `font`, `textAlign`, `textBaseline`, `fillText`, `strokeText`,
`measureText` (returns `{width, actualBoundingBoxAscent,
actualBoundingBoxDescent, fontBoundingBoxAscent, fontBoundingBoxDescent}`),
`measureTextWidth` (returns just a number).
Gradients: `createLinearGradient`, `createRadialGradient`,
`gradient.addColorStop`.
Images/patterns: `createPattern`, plus `drawImage` / `drawImageScaled`
for sandbox-resolved local paths (not URLs).

NOT supported: `putImageData`, `getImageData`.

## Performance

Paint runs at 60 Hz by default (configurable 1-240 via
`dashboard.setFps`). Cheap: `lineTo`, `arc`, `fillText`, arithmetic.
Avoid: `JSON.stringify`, allocating arrays per frame, creating new
gradients per call (cache in a top-level `var`).

Throwing inside `paint` logs a watchdog warning and the canvas keeps the
last successful frame on screen.
