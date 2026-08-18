---
spec: 0059-frame-annotation-layer
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-17 (unattended; maintainer re-reviews)
updated: 2026-08-17
---

# Plan 0059 — Frame annotation layer

## Approach (one paragraph)

`Console::AnnotationModel` (`app/src/Console/Annotations.{h,cpp}`) is the one model: a bounded
vector of `Annotation {start, end, row, cls, texts[3]}` over absolute stream offsets, an
interned text table (`kMaxTexts` 4096, overflow id 0 = "..."), the decoder-declared rows and
classes (with colours), and a bounded copy of the raw bytes (`kMaxRetainedBytes` 1 MiB) so a
class's payload can be extracted; it is also the `QAbstractTableModel` behind the table view.
`Console::AnnotationDecoder` runs a user JavaScript `decoder = { rows, classes, decode(bytes,
offset, ctx) }` in its own `QJSEngine` under a `JsWatchdog` (200 ms) at chunk cadence, with
bounded carry-over of unconsumed bytes; a throw or timeout disables it with the message exposed.
`Console::AnnotationFilter` is the row/class proxy. `Console::Handler` owns all three (ctor:
plain `new` with `this` parent, no singleton reach), feeds the decoder from `hotpathRxData` /
`hotpathRxDeviceData` (current device only) and exposes them as `annotations`,
`annotationDecoder`, `annotationFilter`. `ConsoleAnnotations.qml` (toggled from the console
ribbon) shows four tabs: a track strip (one lane per row over a byte window, spans proportional
to offset, longest fitting text), the filterable table with CSV export, the payload view
(hex/text of one class), and the decoder editor with Apply/Enable/Clear. Decoder code + enabled
persist under `widgetSettings("console")`.

## Decisions taken for the open questions

| Question | Decision |
|----------|----------|
| Home | Terminal (Console tool) — a panel below the console, toggle in the ribbon |
| Persist annotations in sessions | no; re-run decoders on replay (the Console feed replays too) |
| Pro gate | none |
| Decoder distribution | project-bundled via `widgetSettings("console")` (no schema work) |
| Lua decoders | **JS only in this slice** (Lua follow-up; the model/API is language-neutral) |
| Overlay on the VT100 grid | replaced by the byte-proportional track strip: the Terminal renders formatted text, not byte cells, so a per-cell overlay would need a byte->cell map that does not exist |
| Errors | decoder disabled + message in the panel + `qWarning`; Problem Center checker deferred |

## Hotpath & threading impact

None on the pipeline thread: `Console::Handler` already receives raw bytes on the GUI thread at
chunk cadence; the decoder runs there under a watchdog. Model growth is bounded (annotations,
texts, bytes, carry).

## Test & verification plan

- `tst_console_annotations` (ctest, unbuilt tonight): interning bound, retained-window trim,
  capacity trim, filter + CSV, payload extraction, JS decoder with carry-over, throw disables,
  layout validation.
- Spec AC1-AC5 in the running app (AC5 = decoder disabled with one message, no dialog storm).
