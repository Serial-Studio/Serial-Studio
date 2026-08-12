# Bug report: parse-load circuit breaker degrades the whole dashboard to 1 Hz

Date: 2026-08-11
Reporter: Alex (via Claude Code session on the TAM-Firmware/BADAQ project)
Area: `app/src/DataModel/FrameBuilder.cpp` (parser-load budget guard)
Severity: high for any project mixing a high-rate scripted source with normal sources

## Symptom

With the BADAQ project connected (CAN at ~10 Hz plus two audio sources at 48 kHz
running per-sample Lua transforms), every dashboard widget, including the APS500
painter and all CAN-fed values, visibly updates exactly once per second. The
painter repaints at 60 FPS (animateTick runs), but the values it samples only
advance once per second. The console logs:

    [FrameBuilder] Parser load exceeded budget ( ... ms / 1000 ms) ...dropping frames until parse load recovers.

## Root cause

`FrameBuilder` has a global parse-time circuit breaker:

- `FrameBuilder.h:154` - `kParseBudgetWindowMs = 1000`
- `FrameBuilder.h:155` - `kParseBudgetWarnLimitMs = 800`
- `FrameBuilder.cpp` - `parseBudgetSkipFrame()` (~line 1466) and
  `parseBudgetAccount()` (~line 1500)

Behavior: parse time is accumulated into `m_parseBudgetUsedNs` per fixed 1 s
window. Once it crosses 800 ms, `m_parseBudgetSkipping` latches and
`parseBudgetSkipFrame()` returns true for EVERY subsequent frame from EVERY
source until the window rolls over. Both `parseProjectFrame()` overloads check
it at the top (~lines 1030 and 1084).

The BADAQ audio path ran 8 transform-bearing datasets (4 raw sensors plus 4
"metrics engine" datasets) at 48 kHz, about 384k Lua call boundaries per
second. At roughly 1-2 us per boundary that is 400-800 ms of parse time per
second, right at the trip point. So each window: parse a burst, trip at ~800 ms
of work, drop everything (audio AND CAN) until the next window. The result is a
dashboard that advances in 1 s steps while the UI itself renders smoothly.

Two design problems compound here:

1. **The breaker is global.** One expensive source starves all sources. The CAN
   link contributed microseconds per second of parse time but its frames were
   dropped along with the audio.
2. **Hard cutoff creates a 1 Hz beat.** Burst-then-starve within fixed windows
   reads as "the app runs at 1 FPS" instead of reading as an overload. Nothing
   in the dashboard UI says frames are being dropped; the only signals are a
   console warning and a one-shot message box on first trip.

## What was ruled out while diagnosing

- Painter tick wiring is correct: `Dashboard::updated` -> `updateData()` (data
  ticks with onFrame) plus `uiTimeout` -> `animateTick()` (repaint-only), see
  `Painter.cpp` ~line 190.
- `DataTableStore` dataset mirrors bump the write clock on change, and the
  change-driven gating in `beginDatasetPass()` / `applyDatasetValue()` is
  sound; virtual datasets re-run when their read slots change.
- Transform engines are torn down and recompiled on project load
  (`compileTransforms()`), so no stale Lua state across reloads.
- Audio capture ticks at 10 ms (`Audio.cpp` ~line 678), so the driver is not
  the batching bottleneck.

## Minimal reproduction

1. Project with two sources: any slow periodic source (serial or CAN at 10 Hz)
   plus one audio source at 48 kHz.
2. Give the audio dataset a trivial per-sample Lua transform
   (`return value * 2`), then duplicate that dataset 6-8 times so total
   scripting cost exceeds 800 ms/s on the test machine.
3. Connect both. Observe: the parse-budget warning fires and BOTH sources'
   dashboard values advance once per second, in lockstep with the 1 s window.

## Suggested fixes (any subset helps)

1. **Per-source accounting.** Charge `m_parseBudgetUsedNs` per sourceId and
   skip only the offending source's frames. A cheap source must never be
   starved by an expensive one. This alone removes the worst of the symptom.
2. **Decimate instead of gate.** When a source is over budget, process every
   Nth frame (N derived from the overrun ratio) rather than dropping everything
   until the window ends. Load degrades smoothly and the 1 Hz beat disappears.
   For sample-stream sources this is principled decimation, not data loss: the
   dashboard cannot show 48k values/s anyway.
3. **Rolling window.** Replace the fixed 1 s window with a rolling or
   exponentially-decayed accumulator so recovery is continuous instead of
   phase-locked to window boundaries.
4. **Amortize the script call boundary for span-lane sources.** The audio span
   lane invokes the transform once per sample. Offering a batch call (pass the
   whole chunk as an array, or an optional native pre-scale/decimate stage on
   audio sources) would cut the per-boundary overhead by orders of magnitude
   and make per-sample DSP in Lua practical.
5. **Surface the state.** Raise a ProblemCenter entry with per-source parse
   load while skipping is active, and give the dashboard a visible stale or
   dropping-frames indicator. A silent 1 Hz dashboard looks like a rendering
   bug (that is exactly how this one was reported).

## Workaround applied in the BADAQ project (already shipped)

- Merged the 4 "metrics engine" datasets into the raw sensor transforms
  (halves the Lua boundary crossings).
- Dropped audio capture from 48 kHz to 16 kHz (Nyquist 8 kHz still covers the
  1057 Hz rotor band; FFT bins improve to 1.95 Hz at 8192 samples).

Net: ~64k calls/s, roughly 100-150 ms/s of parse time, comfortably under the
budget. This masks the symptom for BADAQ but the breaker design issue remains
for any future heavy-scripting project.
