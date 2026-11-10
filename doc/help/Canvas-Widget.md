# Canvas Widget (Pro)

A water-level sensor prints one number, and no built-in widget draws a tank. The Canvas widget is a blank rectangle on the dashboard that a few lines of JavaScript fill in from your live values: a tank, a custom dial, a radar. If a built-in widget already shows your data, use that instead. The built-in templates need no coding; a custom drawing needs a little JavaScript.

Canvas is a Pro widget (see [Pro vs Free](Pro-vs-Free.md)); the first use offers the free 14-day trial with a Yes/No prompt, and registering it needs an internet connection. In the free edition, a Canvas group falls back to a data grid. (Formerly the Painter widget; the project-file key stays `"painter"`.)

A whole script, drawing one bar that follows the first value. `paint` runs on every screen refresh; `ctx` is the drawing surface, and `w` and `h` are the widget size in pixels.

```javascript
function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  if (datasets.length === 0) return;

  const ds = datasets[0];
  const v  = (ds.value - ds.min) / (ds.max - ds.min || 1);

  ctx.fillStyle = theme.widget_highlight;
  ctx.fillRect(8, h - v * (h - 16) - 8, w - 16, v * (h - 16));
}
```

## Adding a Canvas widget to a project

Canvas draws datasets, and datasets come from your project's frame parser. Start from a project that already shows your `Serial.println` values (see [Getting Started](Getting-Started.md) and [Frame Parser Scripting](Frame-Parser.md)). Add one dataset per value, so `Serial.println(level);` needs one and `Serial.print(a); Serial.print(','); Serial.println(b);` needs two. [Project Editor](Project-Editor.md) covers adding datasets.

Fastest start: add the group, click **Template**, and pick **Dial gauge** or **Strip chart**. If the group has fewer datasets than the template expects, Serial Studio offers to add the missing ones with the template's defaults.

1. Open the **Project Editor** from the main toolbar.
2. In the Project Editor toolbar, click **Canvas**. A group titled "Canvas Widget" is added to the project.
3. Add datasets to the group. The script reads them through the `datasets` global.
4. Select the group in the tree and click **Edit Code**. The **Canvas Widget Code Editor** opens.
5. Click **Template** to load one of the [built-in templates](#built-in-templates). The **Add datasets for this template?** prompt appears when the group has too few datasets.
6. Edit the script. Changes compile and reload live. The **Canvas Live Preview** dialog shows the result without a device connected.

## Script structure

A Canvas script defines `paint()` (required), `onFrame()` (optional) and optional [pointer handlers](#pointer-handlers). `datasets[0]` is the first dataset in the group, for example the first comma-separated value your Arduino prints. `min` and `max` are the **Minimum Value** and **Maximum Value** set on that dataset in the Project Editor; when those are unset the script sees the fallback range described under [`datasets`](#datasets). The timing limits are in [Time limits](#time-limits).

## Pipeline

A dashboard tick is one screen refresh: 60 per second by default, set with **UI Refresh Rate (Hz)** on the **Plotting** tab of [Preferences](Preferences.md) (1 to 240). A data-bearing tick is a tick that carried new frames. Each Canvas widget is bound to one project group. On every tick:

1. Serial Studio updates the group's datasets with the latest parsed values.
2. If new data arrived since the last tick and the script defines `onFrame()`, it runs once. Use it for history buffers, peak-hold decay and accumulated angles.
3. `paint(ctx, w, h)` runs while the widget is visible. `ctx` is a Canvas2D-style context that uses the same drawing calls as the HTML canvas, `w` and `h` are the widget's pixel dimensions.

`onFrame()` runs when new data arrives; `paint()` runs on every refresh, so the widget repaints at the refresh rate regardless of the data rate. Put history and counters in `onFrame()`, drawing in `paint()`.

### `paint(ctx, w, h)`

Runs on every tick while the widget is visible. The bitmap is cleared to transparent before the call, and the path is empty. All other context state (`fillStyle`, `strokeStyle`, `font`, `globalAlpha`, transform, line styles) persists from the previous frame. Set every property on which the frame depends at the top of `paint()`, or wrap mid-frame changes in `save()` / `restore()` pairs. Bitmap pixels are wiped between frames; context state is not.

Aim for under 10 ms per call. The hard limits are in [Time limits](#time-limits).

### `onFrame()`

Called once per data-bearing tick, immediately before `paint()`. Optional. It is not invoked on refresh ticks that carry no new data, so state that must advance on every refresh, such as a sweep angle, cannot live there: advance it in `paint()` from `Date.now()`. `onFrame()` runs even when the widget is hidden (for example on another workspace tab), so history buffers keep filling and `deviceWrite()` / `actionFire()` calls still fire. `paint()` runs only while the widget is visible.

`onFrame()` is the place for time-domain bookkeeping: pushing samples into history buffers, decaying peak holds, integrating a phase angle. Keeping that logic out of `paint()` makes both functions easier to read and review.

```javascript
const HISTORY = 256;
const trace   = [];

function onFrame() {
  if (datasets.length === 0) return;
  trace.push(datasets[0].value);
  if (trace.length > HISTORY) trace.shift();
}

function paint(ctx, w, h) {
  const ds = datasets[0];
  const span = (ds.max - ds.min) || 1;

  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  if (trace.length < 2) return;

  ctx.strokeStyle = theme.widget_highlight;
  ctx.lineWidth   = 2;
  ctx.beginPath();
  for (let i = 0; i < trace.length; ++i) {
    const x = (i / (HISTORY - 1)) * w;
    const y = h - ((trace[i] - ds.min) / span) * h;
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
  }
  ctx.stroke();
}
```

### Pointer handlers

A script can react to the mouse by defining any of these optional functions. Coordinates are widget pixels.

| Handler                  | Arguments            | Fires when |
|--------------------------|----------------------|------------|
| `onPress(x, y, button)`  | position, button number (1 left, 2 right, 4 middle) | A mouse button goes down. |
| `onDrag(x, y, dx, dy)`   | position, movement since the last event | The pointer moves while a button is held. |
| `onRelease(x, y)`        | position             | The button is released. |
| `onMove(x, y)`           | position             | The pointer hovers over the widget. |
| `onWheel(x, y, delta)`   | position, signed wheel notches (one notch is 1; trackpads give fractions) | The wheel turns. |
| `onDoubleClick(x, y)`    | position             | A double-click, for reset-style gestures. |

### Persistent state

Variables declared at the top of the script (`const`, `let`, `var`) live for the lifetime of the widget and retain their values between calls. State is reset when the script is recompiled (after an edit in the editor), when the user switches between light and dark themes, or when the widget is destroyed (project closed, group deleted). A theme switch clears history buffers; a disconnect and reconnect keeps them.

A widget has one set of variables, shared by all its datasets. A top-level `let trace = []` is one buffer for every dataset. Use an array indexed by dataset index instead:

```javascript
const traces = [];
function onFrame() {
  while (traces.length < datasets.length) traces.push([]);
  for (let i = 0; i < datasets.length; ++i) {
    traces[i].push(datasets[i].value);
  }
}
```

## Globals

Five globals are available inside `paint()` and `onFrame()`: `datasets`, `group`, `frame`, `theme` and `console`. The control functions are listed in [Control APIs](#control-apis--devicewrite-and-actionfire).

### `datasets`

An array-like view of the group's datasets. `datasets.length` returns the count, `datasets[i]` returns the i-th dataset. Each dataset is a frozen object with the fields below. The fields are getters and always return the current value.

| Field        | Type    | Description |
|--------------|---------|-------------|
| `index`      | number  | Position of the dataset in the frame (not `frame.number`). |
| `uniqueId`   | number  | Stable unique ID across project edits. |
| `title`      | string  | Display name. |
| `units`      | string  | Measurement units (`"degC"`, `"rpm"`, ...). |
| `widget`     | string  | Dataset widget type, if any (`"bar"`, `"gauge"`, `"compass"`, `"meter"`). |
| `value`      | number  | Post-transform numeric value. `NaN` if the dataset is invalid. |
| `rawValue`   | number  | Pre-transform numeric value. |
| `text`       | string  | Post-transform string value. |
| `rawText`    | string  | Pre-transform string value. |
| `min`, `max` | number  | Effective range. Falls back through widget bounds, plot bounds, then FFT bounds. |
| `widgetMin`, `widgetMax`, `plotMin`, `plotMax`, `fftMin`, `fftMax` | number | The widget, plot and FFT bounds that `min` / `max` fall back through. |
| `alarmLow`, `alarmHigh`  | number | Alarm thresholds. Derived from `alarmBands`: the lower edge of the first, and the upper edge of the last, Warning-or-higher band. `NaN` when no such band exists. |
| `alarmBands`             | array of object | The dataset's alarm bands: `{min, max, severity, color, label, blink}`. `severity` is 0=Info, 1=OK, 2=Warning, 3=Critical. `alarmLow`/`alarmHigh` are a legacy view of this same data. |
| `ledHigh`, `hasPlot`, `hasFft` | number, boolean | LED activation threshold, and the plot and FFT visualization flags from the project. |

`datasets` is implemented as a Proxy. Out-of-range indices return `undefined`; entries cannot be added or removed.

### `group`

Metadata about the group bound to this Canvas.

| Field      | Type    | Description |
|------------|---------|-------------|
| `id`       | number  | Group ID. |
| `title`    | string  | Group title. |
| `columns`  | number  | Configured column count for the group. |
| `sourceId` | number  | Source feeding this group (for multi-source projects). |

### `frame`

Metadata about the last data-bearing tick.

| Field          | Type    | Description |
|----------------|---------|-------------|
| `number`       | number  | Monotonic frame counter, starting from 1. |
| `timestampMs`  | number  | Wall-clock timestamp in milliseconds since epoch. |

Both fields advance only when new data arrives; on a refresh-only tick `paint()` sees the previous values. Read `frame.timestampMs` in `onFrame()` for timestamps that match the data. For an animation that must advance on every refresh, read `Date.now()` in `paint()`, otherwise it freezes between frames.

### `theme`

The active color palette, refreshed automatically whenever the user switches themes. Every bundled template reads its colors from `theme` instead of hardcoding hex values.

| Field              | Type   | Description |
|--------------------|--------|-------------|
| `widget_base`      | string | Canvas / card background. |
| `widget_border`    | string | Grid lines, frames, and borders. |
| `alternate_base`   | string | Card body / recessed panel fill. |
| `widget_text`      | string | Primary value labels and titles. |
| `placeholder_text` | string | Muted secondary labels. |
| `widget_highlight` | string | Primary signal / value color. |
| `accent`           | string | Secondary accent color. |
| `alarm`            | string | Alarm color. |
| `widget_colors`    | array of string | Per-channel palette; index with `theme.widget_colors[i % theme.widget_colors.length]`. |

Other palette keys exist (`text`, `window`, `mid`, `groupbox_background`, `groupbox_border`, `highlighted_text`, `pane_section_label`, ...) mirroring the palette the QML dashboard widgets draw from, but the fields above cover what a Canvas script needs. `theme` is a plain object, not frozen — avoid mutating it. A theme switch replaces the global `theme` object and recompiles the script, which resets its state (see [Persistent state](#persistent-state)).

### `console`

`console.log`, `console.info`, `console.debug`, `console.warn`, and `console.error` route to the Canvas widget's console output, which is visible in the script editor. Each call has the same signature as the browser console. A `console.log` inside `paint()` emits one line per tick (60 lines per second at the default refresh rate): useful during development, remove it from shipped projects.

## Control APIs — `deviceWrite()` and `actionFire()`

Canvas scripts can write back to the device or fire any existing project [Action](Actions.md):

```javascript
deviceWrite(data, sourceId?)     // -> { ok: true } | { ok: false, error: "..." }
actionFire(actionId)             // -> { ok: true } | { ok: false, error: "..." }
```

Both are synchronous, fire-and-forget, and never throw. `deviceWrite` defaults `sourceId` to the canvas widget's group `sourceId`; pass an explicit one to target a different source.

**Important:** `paint()` runs at the refresh rate, 60 times a second by default. Calling `deviceWrite` or `actionFire` from `paint()` sends 60 writes a second to a device that wanted one. `onFrame()` runs at most once per data-bearing tick (frames batched at high rates), never on refresh ticks with no new data, so move the calls there and guard them on a state transition. `actionFire` takes the numeric ID of an existing action in the project (the `7` below):

```javascript
var alarmRaised = false;
function onFrame() {
  var high = datasets[0].value > 100;
  if (high && !alarmRaised) actionFire(7);   // existing "Alarm" action
  alarmRaised = high;
}
```

The dashboard helpers (`clearPlots()`, `setPlotPoints(n)`, `setTerminalVisible(bool)`, `setNotificationLogVisible(bool)`, `setClockVisible(bool)`, `setStopwatchVisible(bool)`, `setActiveWorkspace(idOrName)`) are also available, with the same `{ ok, error }` return shape and the same "fire once on a state transition" guidance. Do not call them from `paint()`.

See [Frame Parser Scripting](Frame-Parser.md) for full signatures and failure modes shared across parsers, transforms, and canvas scripts.

## Canvas code on a remote dashboard

On a dashboard mirrored from another instance (see [Remote Dashboard](Remote-Dashboard.md)), the publisher's Canvas widgets still draw on the viewer, but their code can only draw. The script runs on a draw-only surface: `apiCall()`, `deviceWrite()`, `actionFire()` and the dashboard helpers answer `{ ok: false, error: "dryRun" }`, `console` output is dropped, and nothing the script does reaches the viewer's devices, files, or processes. `datasets`, `group`, `frame`, `theme` and `ctx` work as usual, so `paint()` and `onFrame()` draw the same picture they draw on the publisher.

## Drawing API

The context exposes a Canvas2D-style API backed by Qt's QPainter, drawing into an off-screen bitmap. The sections below cover the commonly used methods and properties. Additional Canvas2D members are implemented (`roundRect`, `arcTo`, `ellipse`, `transform` / `setTransform` / `getTransform`, `setLineDash` / `getLineDash`, `isPointInPath`, `isPointInStroke`, `measureText`, `globalCompositeOperation`, `miterLimit`, `lineDashOffset`, the `shadow*` properties, and image-smoothing controls). Calling a member that does not exist throws.

### State

CSS `rgb()` and `rgba()` strings are not supported. For transparency, use the alpha-first hex form (`"#80ff0000"` is 50% red).

| Property       | Notes |
|----------------|-------|
| `fillStyle`    | Color string: hex (`"#22c55e"`, or `"#80ff0000"` with the alpha byte first) and named colors. |
| `strokeStyle`  | Same syntax as `fillStyle`. |
| `lineWidth`    | Pixels. |
| `lineCap`      | `"butt"`, `"round"`, `"square"`. |
| `lineJoin`     | `"miter"`, `"round"`, `"bevel"`. |
| `font`         | CSS font shorthand (`"bold 14px monospace"`, `"12px sans-serif"`). |
| `textAlign`    | `"left"` / `"start"`, `"center"`, `"right"` / `"end"`. |
| `textBaseline` | `"alphabetic"`, `"top"`, `"middle"`, `"bottom"`, `"hanging"`. |
| `globalAlpha`  | 0.0 to 1.0. |

`save()` and `restore()` push and pop the full state stack, including the current transform.

Gradient and pattern objects are supported. `createLinearGradient(x0, y0, x1, y1)`, `createRadialGradient(x0, y0, r0, x1, y1, r1)`, `createConicGradient(startRad, cx, cy)`, and `createPattern(src, repetition)` each return a handle that can be assigned to `fillStyle` or `strokeStyle`; gradient handles take stops via `addColorStop(offset, color)`. A gradient needs at least two `addColorStop()` entries to show visible color, and a pattern whose `src` falls outside the image sandbox resolves to an empty tile. The bundled templates favor stacks of solid-color rectangles or arcs for a flat look, but a gradient is available when a smooth ramp is wanted.

### Transforms

`translate(x, y)`, `rotate(radians)`, `scale(sx, sy)`, `resetTransform()`. Rotations are in radians. Convert from degrees with `* Math.PI / 180`.

### Paths

`beginPath()`, `closePath()`, `moveTo(x, y)`, `lineTo(x, y)`, `rect(x, y, w, h)`, `arc(x, y, r, startRad, endRad, counterClockwise=false)`, `quadraticCurveTo(cpx, cpy, x, y)`, `bezierCurveTo(c1x, c1y, c2x, c2y, x, y)`. Commit with `fill()`, `stroke()`, or `clip()`.

`arc()` requires a preceding `moveTo()` to the arc's start point. The implementation maps to `QPainterPath::arcTo`, which connects the path's current cursor to the arc's start with an implicit line. Without the `moveTo`, the cursor is at the origin (0, 0) and the chord becomes part of the path. A subsequent `stroke()` strokes the chord, `fill()` encloses it, and `clip()` removes a wedge from the clipping region.

```javascript
ctx.beginPath();
ctx.moveTo(cx + Math.cos(startA) * r, cy + Math.sin(startA) * r);
ctx.arc(cx, cy, r, startA, endA);
ctx.stroke();
```

For full circles, `moveTo(cx + r, cy)` is sufficient.

Pixel coordinates address the boundaries between pixels. A 1-pixel line at integer Y is split between two rows and rendered as a 2-pixel anti-aliased band. Offset by 0.5 (`ctx.moveTo(x, y + 0.5)`) to get a crisp line.

### Direct shapes

`fillRect(x, y, w, h)`, `strokeRect(x, y, w, h)`, `clearRect(x, y, w, h)`. These bypass the path and are appropriate for backgrounds and grids.

### Text

`fillText(text, x, y)`, `strokeText(text, x, y)`, `measureTextWidth(text)`, `measureText(text)`. Honors the current `font`, `textAlign`, and `textBaseline`. `measureTextWidth` returns the advance width as a plain number; `measureText` returns a metrics object with `width`, `actualBoundingBoxAscent`, `actualBoundingBoxDescent`, `fontBoundingBoxAscent`, and `fontBoundingBoxDescent`. For centered text, prefer `textAlign = "center"` over measuring.

To place a value and a unit label side by side, measure the value with the value's own font:

```javascript
ctx.font = "bold 18px sans-serif";
const valueW = ctx.measureTextWidth(value);
ctx.fillText(value, x, y);

ctx.fillStyle = theme.placeholder_text;
ctx.font      = "10px sans-serif";
ctx.fillText(units, x + valueW + 6, y);
```

### Images

`drawImage(src, x, y)` and `drawImageScaled(src, x, y, w, h)`. The `src` string is resolved through a sandbox: `qrc:/` resources and paths inside the project file's directory are accepted. Other paths are rejected, and a rejected path is silently skipped: nothing is drawn and no exception is raised.

```javascript
ctx.drawImage("logo.png", 12, 12);                                // relative to the project
ctx.drawImage("qrc:/iconpacks/fine-line/widgets/48/painter.svg",  // bundled resource
              0, 0);
```

## Built-in templates

Eighteen templates are bundled with Serial Studio and load from the **Template** button in the code editor. Every one draws through the [`theme`](#theme) global. The instrument-style templates (oscilloscope, radar sweep, artificial horizon) use a recessed screen with a scan grid inside the same card frame.

| Template                | Datasets needed | What it draws |
|-------------------------|-----------------|---------------|
| Default template        | 3 (X, Y, Z)     | Position indicator with a planar dot and a Z bar. |
| Audio VU meter          | 1+              | Per-channel VU bars with peak markers, any channel count. |
| Bars with peak hold     | 1+              | Vertical bars with decaying peak-hold lines. |
| Clock face              | 0 or 1 (seconds value) | Analog clock driven by the dataset value; falls back to wall-clock time when the dataset is absent. |
| Dial gauge              | 1               | Single arc gauge with a needle. |
| Heatmap                 | N               | Color-mapped grid, one cell per dataset. |
| Artificial horizon      | 2 (pitch, roll) | Aviation attitude indicator. |
| LED matrix              | N               | Discrete LED grid driven by per-row values. |
| Oscilloscope            | 1+              | Phosphor-style traces with a CRT background. |
| Polar plot              | 2 per trace (angle, mag) | Multi-trace polar coordinate plot. |
| Progress rings          | 1+              | Concentric ring gauges. |
| Radar sweep             | 2 per target (az, range) | Sweeping radar PPI with target blips. |
| 7-segment display       | 1+              | Segmented numeric readout. |
| Sparkline grid          | N               | One mini line chart per dataset. |
| Status grid             | 1+              | Tile grid with values and color-coded states. |
| Strip chart             | 1+              | Multi-trace rolling line chart. |
| Vector field            | 2 per vector (Vx, Vy) | Vector arrows on a grid. |
| XY scope (Lissajous)    | 2 per pair      | XY mode oscilloscope. |

N means any number of datasets, with the template adapting to the group size. 1+ means at least one. Templates are stored as plain `.js` files under `app/rcc/scripts/painter/`.

## Layout tips

None of this is enforced by the engine. It keeps a custom widget looking like the built-in ones.

A card with a soft shadow and a 1-pixel border holds the content. The 0.5-pixel offset on `strokeRect` aligns the stroke to a single pixel column.

```javascript
ctx.fillStyle = theme.widget_base;
ctx.fillRect(0, 0, w, h);

const pad = 14;
ctx.fillStyle = theme.widget_border;      // shadow
ctx.fillRect(pad + 1, pad + 2, w - pad * 2, h - pad * 2);
ctx.fillStyle = theme.alternate_base;     // card body
ctx.fillRect(pad, pad, w - pad * 2, h - pad * 2);
ctx.strokeStyle = theme.widget_border;
ctx.lineWidth = 1;
ctx.strokeRect(pad + 0.5, pad + 0.5, w - pad * 2 - 1, h - pad * 2 - 1);
```

Size from the available area. Compute it first, fit content into the smaller dimension, and center horizontally. Fixed fractions like `cy = h * 0.55` fit one aspect ratio and break at others.

```javascript
const labelH = 36;                          // reserved for bottom labels
const margin = 12;
const availW = w - margin * 2;
const availH = h - margin * 2 - labelH;
const r      = Math.max(8, Math.min(availW, availH) * 0.5 - 8);
const cx     = w / 2;
const cy     = margin + r + 8;
```

## Examples

### Tank level with alarm coloring

The fill color switches from the highlight color to the accent color near the top of the range, and to the alarm color once the value reaches the dataset's `alarmHigh`.

```javascript
function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  if (datasets.length === 0) return;

  const ds   = datasets[0];
  const v    = Number.isFinite(ds.value) ? ds.value : ds.min;
  const span = (ds.max - ds.min) || 1;
  const norm = Math.max(0, Math.min(1, (v - ds.min) / span));

  let color = theme.widget_highlight;
  if (Number.isFinite(ds.alarmHigh) && v >= ds.alarmHigh) color = theme.alarm;
  else if (norm > 0.85) color = theme.accent;

  const pad = 16;
  const th  = h - pad * 2 - 24;
  ctx.fillStyle = theme.alternate_base;
  ctx.fillRect(pad, pad, w - pad * 2, th);

  ctx.fillStyle = color;
  ctx.fillRect(pad, pad + th * (1 - norm), w - pad * 2, th * norm);

  ctx.fillStyle    = theme.widget_text;
  ctx.font         = "bold 16px sans-serif";
  ctx.textAlign    = "center";
  ctx.textBaseline = "alphabetic";
  ctx.fillText(v.toFixed(1) + " " + (ds.units || ""), w / 2, h - 8);
}
```

### Polar bug indicator

Two datasets, bearing in degrees and range in 0..1, plotted as a marker on a polar grid. Each `arc()` is preceded by a `moveTo()` to its start point.

```javascript
function paint(ctx, w, h) {
  ctx.fillStyle = theme.widget_base;
  ctx.fillRect(0, 0, w, h);

  if (datasets.length < 2) return;

  const cx = w / 2;
  const cy = h / 2;
  const r  = Math.min(w, h) * 0.42;

  // Concentric range rings.
  ctx.strokeStyle = theme.widget_border;
  ctx.lineWidth   = 1;
  for (let i = 1; i <= 4; ++i) {
    const rr = (r * i) / 4;
    ctx.beginPath();
    ctx.moveTo(cx + rr, cy);
    ctx.arc(cx, cy, rr, 0, Math.PI * 2);
    ctx.stroke();
  }

  // Bearing spokes every 30 degrees.
  ctx.strokeStyle = theme.widget_border;
  for (let deg = 0; deg < 360; deg += 30) {
    const a = (deg - 90) * Math.PI / 180;
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(cx + Math.cos(a) * r, cy + Math.sin(a) * r);
    ctx.stroke();
  }

  // Marker.
  const bearing = datasets[0].value;
  const range   = Math.max(0, Math.min(1, datasets[1].value));
  const a       = (bearing - 90) * Math.PI / 180;
  const x       = cx + Math.cos(a) * r * range;
  const y       = cy + Math.sin(a) * r * range;

  ctx.fillStyle = theme.widget_highlight;
  ctx.beginPath();
  ctx.moveTo(x + 6, y);
  ctx.arc(x, y, 6, 0, Math.PI * 2);
  ctx.fill();

  // Readout.
  ctx.fillStyle    = theme.widget_text;
  ctx.font         = "bold 12px sans-serif";
  ctx.textAlign    = "center";
  ctx.textBaseline = "alphabetic";
  ctx.fillText(bearing.toFixed(0) + "°  /  " + (range * 100).toFixed(0) + "%",
               cx, h - 10);
  ctx.textAlign = "start";
}
```

## Performance

The Canvas pipeline targets the dashboard refresh rate (60 Hz by default) for moderately complex scenes: a few hundred line segments, a few hundred filled shapes, and about ten text labels per frame.

### Time limits

| Limit  | Applies to | What happens |
|--------|------------|--------------|
| 10 ms  | `paint()`  | A target, not enforced. |
| 30 ms  | `paint()`, two consecutive ticks | The message `warning: paint() took N ms (>30 ms budget)` appears next to the widget, once per script compile. |
| 250 ms | `paint()`, `onFrame()` and the initial compile, per call | The call is aborted, the widget stops updating and shows the error. See [Widget renders once and then freezes](#widget-renders-once-and-then-freezes). |

Common causes of slow paints:

- **Allocation in the paint path.** Iterating with `for (const ds of datasets)` is fine. `datasets.map(d => d.value)` allocates a new array each frame. So does `ds.title.toUpperCase()` inside a tight loop. Move the work into `onFrame()` and cache the result if the cost is non-trivial.
- **`measureTextWidth()` per label.** Each call is a real metrics call into Qt. Measure fixed labels once at compile time and cache the width.
- **One `stroke()` per point.** A single `beginPath()` followed by many `lineTo()` calls and one `stroke()` is one path-stroke call. A `beginPath()`, `lineTo()`, `stroke()` per point is N calls.
- **`drawImage` from disk every frame.** `drawImage` reloads the image from disk on every call with no internal caching, so per-frame disk reads are not free. Pre-resolve to `qrc:/` paths or read the image once at the top of the script and reuse it.

## Common errors

### Widget renders once and then freezes

`paint()` or `onFrame()` threw an exception. The widget stops updating and shows the error message next to it. Common causes: indexing `datasets[0]` when the group is empty; dividing by `(ds.max - ds.min)` when both are zero; calling `.toFixed()` on `undefined`. A call that runs past the 250 ms limit stops the widget the same way.

### A diagonal line crosses the arc

`arc()` without a preceding `moveTo()` draws a chord from (0, 0). Add the `moveTo()`; see [Paths](#paths).

### Gradient or pattern fill shows nothing

The handle must be assigned to `fillStyle` or `strokeStyle` before drawing, and a gradient needs two color stops. See [State](#state).

### Lines look fuzzy

A 1-pixel line at an integer coordinate renders as a 2-pixel band. Offset it by 0.5; see [Paths](#paths).

### Colors do not match what was set

Context state carries over between `paint()` calls. If a previous frame ended with `globalAlpha = 0.5`, the next frame starts with the same value. See [`paint(ctx, w, h)`](#paintctx-w-h).

### State leaks between channels

A top-level `let trace = []` is shared by every dataset. See [Persistent state](#persistent-state).

### `drawImage` shows nothing

A path outside `qrc:/` and the project file's directory is silently skipped. See [Images](#images).

## See also

- [Widget Reference](Widget-Reference.md): the full list of built-in dashboard widgets.
- [Frame Parser Scripting](Frame-Parser.md): the JavaScript engine on the receiving side of the data pipeline.
- [Output Controls](Output-Controls.md): user-scripted widgets that send data to the device.
- [Dataset Value Transforms](Dataset-Transforms.md): per-dataset scripts for calibration and filtering.
- [Project Editor](Project-Editor.md): adding groups, datasets, and Canvas widgets to a project.
- [Preferences](Preferences.md): the UI Refresh Rate setting.
