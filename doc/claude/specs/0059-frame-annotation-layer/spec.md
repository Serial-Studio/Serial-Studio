---
spec: 0059-frame-annotation-layer
title: Frame annotation layer
status: done          # closed 2026-08-20
created: 2026-08-16
author: Claude (overnight run, unattended; planned + implemented 2026-08-17 on maintainer request)
---

# Spec 0059 — Frame annotation layer

> **Phase 1 of 4 — the WHAT and the WHY.** Written 2026-08-16 as specify-only; on 2026-08-17 the
> maintainer asked for B to be planned and implemented — see `plan.md` for the decisions taken
> on the open questions and the deviations recorded there. Adapts the *idea* of a logic-analyzer decoder annotation model;
> no external code.

## Problem / Motivation

Serial Studio sees every byte a device sends but explains none of them. The Terminal shows
raw bytes (hex/text) and the parser turns delimited frames into datasets, but the middle
layer, "which bytes are the header, which are the CRC, which are the payload, and what does
that payload mean", exists only in the user's head or in a JS/Lua parser that throws the
structure away as it extracts values. When a device speaks a binary protocol (Modbus RTU,
MAVLink, a vendor packet with a length byte and a checksum), debugging it today means
counting bytes by hand in the console. Users of a logic-analyzer viewer get a stack of
decoder rows (bits -> bytes -> packets) over the same capture and reach for it constantly;
we cannot offer that on the byte stream we already own.

## Goals

- A user-authored decoder (existing JS or Lua engines, no new language) can attach
  annotations to byte ranges of the raw stream: `{startByte, endByte, texts[], class, row}`,
  where `texts` is a list of progressively shorter renderings of the same meaning
  ("Sync byte 0xFE" / "SYNC" / "S") so a narrow span can still show something.
- Annotations group into stacked rows (bits, bytes, fields, packets) that a decoder declares
  up front; rows stack in a Terminal overlay track and stay aligned to the byte positions the
  Terminal already renders.
- Three consumers off one model: (1) the Terminal overlay track, (2) a sortable tabular view
  (row, class, start, end, text) with CSV export, (3) an extracted-payload view that
  concatenates the bytes of a chosen class into a new byte stream (e.g. "show me only the
  payload bytes of every valid packet").
- Annotation text is interned: a decoder emits the same string millions of times over a
  session; the model stores a small integer id and one copy of the string.
- Bounded memory: annotations are a ring over the same retained span the Terminal keeps, not
  an unbounded log; a session recording may persist them (open question).

## Non-Goals

- A protocol decoder library. sigrok ships ~140 decoders; we ship zero on day one, and this
  spec does not change that. It ships the *substrate* plus one or two worked examples.
- A new scripting language or DSL for decoders.
- Running decoders on the frame pipeline thread; annotation is a Terminal-side feature at
  chunk cadence, never per byte on the hotpath.
- Editing annotations by hand.

## Requirements

1. **R1** — The Terminal owns an annotation model keyed by absolute byte offset in the raw
   stream it retains; when the Terminal drops old bytes, annotations that end before the
   retained window drop with them.
2. **R2** — A decoder is a JS or Lua function `decode(bytes, offset, ctx)` invoked with each
   new chunk plus a bounded carry-over of unconsumed bytes; it returns annotations and the
   number of bytes it consumed; it runs on the Terminal's thread, chunk-rate, never per byte on
   the pipeline.
3. **R3** — An annotation carries `startByte`, `endByte` (inclusive, absolute), `row` (index
   into the decoder's declared rows), `class` (index into declared classes; class carries a
   colour), and `texts` (ordered longest to shortest).
4. **R4** — Text is interned per decoder instance: the model stores text ids; identical
   strings share one entry; the intern table is bounded and a decoder exceeding it gets a
   fallback "…" id rather than growth.
5. **R5** — The Terminal renders one overlay row per declared row above/below the byte
   columns, choosing the longest `texts[i]` that fits the span's pixel width.
6. **R6** — The table view lists annotations (filterable by row/class, sortable by start),
   selecting one scrolls the Terminal to its bytes; CSV export writes the visible list.
7. **R7** — The payload view concatenates the bytes of all annotations of one class in stream
   order and shows them as a byte stream (hex/text) with copy/export.
8. **R8** — Decoder errors surface once per distinct message in the Problem Center, never as a
   per-chunk dialog; a decoder that fails is disabled until re-enabled.
9. **R9** — Everything is opt-in per project (a decoder list under the project JSON); no cost
   when no decoder is configured.

## Acceptance Criteria

- [ ] **AC1** — A worked example decoder for the app's own delimited frame format annotates
      start delimiter, payload, checksum, end delimiter as four classes on two rows, live, at
      the default UART rate with no visible Terminal slowdown.
- [ ] **AC2** — With 1M annotations of 8 distinct texts the intern table holds 8 strings and
      the model's memory is bounded by the Terminal's retained window.
- [ ] **AC3** — Table view sort/filter/export round-trips into a CSV that reopens in a
      spreadsheet with one row per annotation.
- [ ] **AC4** — Payload view of class "payload" reproduces the concatenated payload bytes
      byte-for-byte for a known capture.
- [ ] **AC5** — A decoder throwing on every chunk yields one Problem Center finding and a
      disabled decoder, not a dialog storm.

## Constraints & Invariants

- Nothing on the frame path (`FrameReader` / `FrameBuilder`); decoders consume the raw byte
  feed the Terminal already receives (`ConnectionManager::onRawDataReceived` path).
- Script execution follows the existing guardrails (`JsScriptEngine::guardedCall`, Lua
  Safe/Fast mode + watchdog); no new engine type.
- Memory bounded by the Terminal's retained span; no unbounded vectors.

## What this competes with, and the smallest first slice

sigrok has ~140 community decoders and a mature stacking model; a first release with zero
decoders is a substrate, not a feature. The smallest slice that is still useful: R1-R5 plus
R9 (model + JS/Lua decode hook + Terminal overlay), shipping with two example decoders (the
app's own delimited frame, and a generic length-prefixed packet with CRC). Table view and
payload view (R6, R7) are the second slice; a decoder gallery is a community/marketing task,
not an engineering one.

## Open Questions for Alex

- Is the Terminal the right home, or should annotations be a separate "Protocol" dashboard
  tool window? (Terminal already owns the byte model and hex rendering.)
- Persist annotations into the Session Database, or re-run decoders on replay? Re-running is
  cheaper and always consistent with the decoder version; persisting survives decoder edits.
- Pro-gate: substrate free + gallery Pro, or all Pro?
- Decoder distribution: bundled in the project file (like transforms), or installable like
  widget extensions (spec 0038 consent model)?
