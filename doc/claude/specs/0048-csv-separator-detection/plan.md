---
spec: 0048-csv-separator-detection
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-09
---

# Plan 0048 — CSV player separator auto-detection

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Sniff-and-parameterize. `CSV::Player::runQuickPass()` detects the separator once per opened
file by trial-splitting the header row and first data row with each candidate (`,` `;` `\t`
`|`) through the existing quote-aware splitter and scoring by total cell count (comma wins
ties). The winner is stored as a `char` member on the player, passed to every
`splitReplayRowSpans` call (the free function gains a `char separator = ','` parameter — all
other callers keep the default) and into `PlayerIndexRequest` for the loader worker.
QuickPlot-mode payloads for non-comma files are rebuilt as RFC-4180 comma rows via the
existing `splitDataCells` + `joinReplayRow` path (already used by date-time-column mode), so
everything downstream of `injectFrame` — `splitReplayChannels`, multi-source fan-out —
continues to see only comma rows and needs no change. Detection state never leaves the
player; Sessions/MDF4 replay and all synthesized rows are untouched by construction.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/Scripting/FrameParserPipeline.h` | `splitReplayRowSpans` gains trailing `char separator = ','`; doc updated |
| `app/src/DataModel/Scripting/FrameParserPipeline.cpp` | Thread `separator` through the byte state machine (replaces the literal `','` at the cell boundary, line 489) |
| `app/src/CSV/Player.h` | New `char m_separator` member (reset to `','` in ctor + `closeFile()`); decl for the sniffer if not file-local |
| `app/src/CSV/Player.cpp` | Sniff in `runQuickPass()` before header capture; pass `m_separator` at all five split sites (587, 619, 1009, 1285, 1326); generalize `firstTopLevelComma` to `firstTopLevelSeparator(row, sep)` (line 114); `quickPlotPayload()` takes the rebuild path whenever `m_separator != ','` (line 1359); `startIndexing()` fills `request->separator` |
| `app/src/CSV/PlayerLoaderWorker.h` | `PlayerIndexRequest` gains `char separator = ','` (aggregate default, existing struct style) |
| `app/src/CSV/PlayerLoaderWorker.cpp` | `processRow()` passes `request.separator` to `splitReplayRowSpans` (line 209) |
| `tests/integration/test_csv_separator_detection.py` | New: fixture-generated semicolon/tab/pipe/comma files, quoted-separator cases, regression vs comma behavior |

Confirmed by grep: `splitReplayRowSpans` has exactly three calling TUs (CSV/Player.cpp,
CSV/PlayerLoaderWorker.cpp, FrameParserPipeline.cpp itself). `Sessions/Player.cpp` and
`MDF4/Player.cpp` only call `joinReplayRow` (comma synthesis) — no change there.

## Architecture & data flow

Per `doc/claude/architecture/export.md` (spec 0022 streaming): the player mmaps the file,
`runQuickPass()` (main thread) captures header + timestamp mode from the first rows, then
`PlayerLoaderWorker` (own QThread) indexes row offsets + seconds. Detection slots into
`runQuickPass()` between "first valid row found" and "header cells captured":

1. Quick pass finds the first two non-empty rows (raw byte views, unchanged loop).
2. `sniffSeparator(header_row, first_data_row)`: each candidate is scored by its summed
   top-level occurrence count over both rows via `topLevelSeparatorCount`, a dedicated
   cell-position-independent quote scanner (any `"` toggles quoted mode, `""` escapes) —
   NOT the RFC splitter, whose cell-start quote rule leaks quoted content under a wrong
   candidate (qt-cpp-review finding, 2026-08-09). Highest score wins; tie order comma >
   semicolon > tab > pipe. A non-comma winner must appear in the data row AND match the
   header's count — the grid-consistency check that stops unquoted text-cell separators
   (`1,a;b;c` in a comma export) from outvoting comma. No candidate qualifying = comma
   (R6: single-column files keep today's behavior, including the prompts).
3. `m_separator` then drives: header-cell capture, timestamp-mode detection (first data
   cell is now the real first column), the worker's per-row split (via
   `PlayerIndexRequest::separator`, plain value copied into the request — no shared
   state), `splitDataCells()` during playback/seek, and `firstTopLevelSeparator` in
   `quickPlotPayload()`.
4. QuickPlot injection: comma files keep the zero-copy raw-slice paths byte-for-byte.
   Non-comma files reuse the date-time-column rebuild (split + `joinReplayRow`), so the
   payload entering `IO::ConnectionManager::processPayload` is always a comma row —
   `splitReplayChannels` / multi-source `splitReplayRow` downstream stay comma-only, per
   the spec's deciding constraint.

Multi-source replay only engages when the header matches the project's export schema
(Serial Studio's own comma exports), and its `injectFrame` split sees normalized comma
payloads anyway — no separate handling.

**R7 addendum (2026-08-10) — numeric timestamp unit scaling.** `runQuickPass()`'s Numeric
branch derives `m_timeScale` from the timestamp header cell via a conservative
unit parser (`timestampUnitScale`, returns optional): a parenthesized/bracketed unit
token or a `_unit` suffix maps ms→1e-3, us→1e-6, ns→1e-9, s/sec→1.0. No recognizable
unit → `promptTimestampUnitScale()` asks (seconds preselected; Enter/cancel keep 1.0) —
maintainer decision 2026-08-10, mirroring the interval/date-time prompt; API-driven
opens of unitless files therefore raise a desktop dialog, same as those prompts, so
test fixtures always carry unit markers. Serial Studio's own exports use date-time
cells and never hit this path. The scale rides `PlayerIndexRequest::timeScale` (same
by-value handoff as `separator`) and multiplies only the Numeric branch of
`secondsForRow`; everything downstream (pacing, seek windows, timestamp display)
already derives from the indexed row-seconds, so no other site changes.

## Hotpath & threading impact

- **Touches the hotpath?** No. `splitReplayRowSpans` runs only on the CSV replay path
  (player main-thread splits + loader-worker indexing), not in the live
  `FrameReader`/`parseUtf8Spans` lane. The added cost is one `char` parameter compare per
  byte-loop iteration, identical branch shape. `--benchmark-hotpath` still runs as the
  guard (AC6) since the benchmark harness shares FrameBuilder replay entry points.
- **New cross-thread signal/slot?** No. The separator rides the existing
  `PlayerIndexRequestPtr` handed to the worker at `startIndexing()` — value copy, no new
  connection, no shared mutable state.
- **New input to a cached hotpath flag?** No. `m_playerOpen` gating is unchanged.
- **Timestamp ownership** — unchanged: rows keep recorded times (numeric column /
  anchored date-time / interval), stamped exactly where they are today.

## Data model & persistence

None. No `Keys::`, no schema, no project-JSON, no settings. The detected separator is
transient per-open state, deliberately not persisted (spec non-goal: no override UI).

## API / SDK surface

None. `csvPlayer.open` behavior improves transparently; no new verbs, no schema change.

## QML / UI

None. Detection is silent (spec open question resolved: silent).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Overall shape | (a) sniff + parameterized splitter; (b) normalize file to comma temp copy at open; (c) push separator into FrameBuilder's replay splitters | **(a)** — (b) violates the spec-0022 streaming constraint (mmap, no materialization; multi-GB logs) which genuinely binds; (c) leaks per-file state into the shared pipeline the spec forbids touching |
| Detection input | header row only vs header + first data row | **Both, summed** — an unquoted comma inside a semicolon file's header label would otherwise outvote; data row is decisive noise-free evidence |
| Comma priority | hard "any comma => comma" vs max-count with comma winning ties | **Max-count, comma wins ties** — hard priority misdetects semicolon files whose labels contain commas; max-count still guarantees every well-formed comma file (R2) parses as before since comma dominates its own rows |
| QuickPlot non-comma payload | rebuild as comma row vs teach downstream splitters the separator | **Rebuild** — bounded cost, only on non-comma files, keeps the "downstream sees only commas" invariant checkable in one function |
| Splitter API | trailing default param vs separate overload vs struct options | **Trailing `char` default** — three call sites change, all others provably identical; single-byte covers every candidate |

## Risks & mitigations

- **Misdetection on a pathological comma file**: quoted separators never score (the
  generic quote scanner masks them for every candidate symmetrically), and unquoted
  text-cell separators fail the header==data count-consistency gate. Regression tests pin
  both shapes plus a quoted Serial Studio export (AC3).
- **Accepted edge (review finding, conf 65)**: the quick-pass row-validity filter runs
  with comma before the sniff, so degenerate rows of bare separators (`;;;` / `,,,`)
  between header and data can desync header selection from the worker's index. All paths
  degrade gracefully (bounds-checked, no crash), comma files are unaffected, and such
  rows are garbage input — not mitigated.
- **Silent divergence between quick pass, worker, and playback splits** (three consumers
  of the separator): single `m_separator` source, worker receives it by value in the
  request; test AC2 asserts identical structure across separators end-to-end.
- **`common-mistakes.md` exposure**: scope creep (lane is the seven files above);
  timestamp re-stamping (none added); macOS file-dialog reentrancy (untouched —
  `openFile(path)` overload only).
- **Behavior change for currently "loadable" garbage** (semicolon file that today prompts
  for interval and plays one channel): intentional per R3; R6 keeps genuinely
  single-column files on the old flow via the >= 2 cells rule.

## Test & verification plan

- **Integration (maintainer runs app with API server, then I run pytest):**
  `tests/integration/test_csv_separator_detection.py` —
  - AC1: generate a semicolon fixture (Mazda-shaped: numeric ms column, `-` gaps, degree
    sign in header), `csvPlayer.open`, assert `frameCount == rows`, header-derived
    channel structure, sampled values.
  - AC2: same data as comma/tab/semicolon fixtures produce identical `getStatus` +
    channel structure + values.
  - AC3: quoted comma fixture (RFC-4180, commas + quotes inside cells) matches current
    behavior; existing `test_csv_player.py` must stay green.
  - AC4: per-separator fixtures with the separator inside quoted cells; assert column
    count.
- **Maintainer observation:** AC5 — open the real `~/Desktop/Mazda/Parking Acc.csv`:
  seven channels, `time(ms)` timeline, no prompt.
- **Hotpath:** AC6 — maintainer's `--benchmark-hotpath` run stays green.
- **Static:** `python scripts/code-verify.py --check` on the six C++ files;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.
