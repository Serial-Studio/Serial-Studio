---
spec: 0048-csv-separator-detection
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-09
---

# Tasks 0048 — CSV player separator auto-detection

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — Parameterize the replay row splitter

- **Files:** `app/src/DataModel/Scripting/FrameParserPipeline.h`,
  `app/src/DataModel/Scripting/FrameParserPipeline.cpp`
- **Does:** `splitReplayRowSpans` gains trailing `char separator = ','`; the byte state
  machine's cell boundary (`c == ','`, .cpp:489) compares against it. Binding invariants:
  quote/trim/guard semantics unchanged for every separator; all existing callers compile
  unchanged via the default; `splitReplayRow`, `splitQuickPlotChannels`, `joinReplayRow`
  stay comma-only — this is the only function touched in the pipeline TU.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back confirms no
  other function changed.
- **Deps:** none
- [x] done

### T2 — Separator-aware QuickPlot slice helper in the player

- **Files:** `app/src/CSV/Player.cpp`
- **Does:** Generalize `firstTopLevelComma(row)` (line 114) to
  `firstTopLevelSeparator(row, separator)`, same quote machine, caller at line 1386
  passes the player separator (still `','` until T3 lands — behavior identical).
- **Verify:** `python scripts/code-verify.py --check app/src/CSV/Player.cpp`.
- **Deps:** none
- [x] done

### T3 — Sniff separator in the quick pass and store it

- **Files:** `app/src/CSV/Player.h`, `app/src/CSV/Player.cpp`
- **Does:** Add `char m_separator` (ctor init list per no-in-header-init rule; reset to
  `','` in `closeFile()`); add file-local `sniffSeparator(header_row, data_row)` scoring
  candidates `, ; \t |` by summed quote-aware cell count (comma wins ties; winner needs
  >= 2 cells on the data row else comma — R6 keeps single-column files on today's flow).
  `runQuickPass()` sniffs after locating the first two non-empty raw rows and *before*
  capturing header cells / detecting timestamp mode, then all five
  `splitReplayRowSpans` sites (587, 619, 1009, 1285, 1326) and the T2 helper use
  `m_separator`. Binding invariant: sniff reads raw rows only — no dependency on cells
  split before the separator is known.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back: every
  split call in the player passes `m_separator`; grep confirms no site still splits with
  the bare default.
- **Deps:** T1, T2
- [x] done

### T4 — Thread separator through the loader worker

- **Files:** `app/src/CSV/PlayerLoaderWorker.h`, `app/src/CSV/PlayerLoaderWorker.cpp`,
  `app/src/CSV/Player.cpp`
- **Does:** `PlayerIndexRequest` gains `char separator = ','` (aggregate default,
  existing struct style); `processRow()` passes `request.separator` (.cpp:209);
  `startIndexing()` fills it from `m_separator`. Binding invariant: separator crosses
  the thread by value inside the request — no member reads from the worker, no new
  signal/slot.
- **Verify:** `python scripts/code-verify.py --check` on all three files; read-back of
  the request construction site.
- **Deps:** T3
- [x] done

### T5 — Normalize non-comma QuickPlot payloads

- **Files:** `app/src/CSV/Player.cpp`
- **Does:** `quickPlotPayload()` (line 1359) takes the existing rebuild path
  (`splitDataCells` + `joinReplayRow`) whenever `m_separator != ','`, covering Interval /
  Numeric / DateTime modes; DateTimeColumn already rebuilds. Binding invariants: comma
  files keep the zero-copy raw-slice paths byte-for-byte; every payload entering
  `injectFrame` is an RFC-4180 comma row, so downstream `splitReplayChannels` /
  multi-source `splitReplayRow` stay comma-only.
- **Verify:** `python scripts/code-verify.py --check app/src/CSV/Player.cpp`; read-back:
  comma path provably unchanged (diff shows only the added branch).
- **Deps:** T3
- [x] done

### T6 — Integration tests + fixtures

- **Files:** `tests/integration/test_csv_separator_detection.py`
- **Does:** New pytest module generating fixtures in tmp: Mazda-shaped semicolon file
  (numeric ms column, `-` gaps, `°C` header), same data as comma and tab, quoted-cell
  fixtures per separator, quoted Serial Studio-style comma export. Tests: AC1 (semicolon
  opens, `frameCount`, channel structure, sampled values), AC2 (identical results across
  comma/semicolon/tab), AC3 (quoted comma file unchanged), AC4 (quoted separators do not
  split). Marked `integration` + `csv`; requires running app with API server.
- **Verify:** `pytest tests/integration/test_csv_separator_detection.py -v` against a
  running app (`nc -z 127.0.0.1 7777` first); existing
  `tests/integration/test_csv_player.py` stays green.
- **Deps:** T5 (full pipeline behavior in place), plus maintainer rebuild.
- **Status 2026-08-10:** 14 tests green against the rebuilt app (combined with
  `test_csv_player.py`: 28/28). Assertions rewritten onto `dashboard.getData` frame
  titles/values — `datasetCount` probes (project-status and dashboard-status) were both
  wrong instruments for QuickPlot player runs.
- [x] done

### T7 — Live check with the real Mazda file

- **Does:** Drive the running app: `csvPlayer.open` on
  `~/Desktop/Mazda/Parking Acc.csv`; assert seven channels named from the header, numeric
  timeline from `time(ms)`, no interval/date-time prompt (AC5). Maintainer confirms
  visually; API-side checks run from here.
- **Verify:** `tests/utils/api_client.py` session transcript + maintainer observation.
- **Deps:** T6, maintainer rebuild + app launch.
- **Status 2026-08-10:** verified over the API — 777 rows indexed, six channels named from
  the header (degree sign intact), numeric timeline, no prompt, sane last-row values.
- [x] done

### T8 — Numeric timestamp unit scaling (R7 addendum)

- **Files:** `app/src/CSV/Player.h`, `app/src/CSV/Player.cpp`,
  `app/src/CSV/PlayerLoaderWorker.h`, `app/src/CSV/PlayerLoaderWorker.cpp`,
  `tests/integration/test_csv_separator_detection.py`
- **Does:** `timestampUnitScale(header)` (file-local, conservative: bracketed unit or
  `_unit` suffix only, returns optional) + `promptTimestampUnitScale()` (unitless header
  asks, seconds preselected, Enter/cancel keep 1.0 — maintainer decision 2026-08-10);
  `m_timeScale` member (ctor init list, reset in `closeFile()`), set in the Numeric
  branch of `runQuickPass()` after header split; `PlayerIndexRequest::timeScale = 1.0`
  passed by value; `secondsForRow` Numeric branch multiplies. Binding invariants:
  explicit-unit headers are silent; prompt cancel = legacy seconds; scale crosses the
  thread only inside the request; no other consumer of row-seconds changes.
- **Verify:** `code-verify.py --check` on the four C++ files; pytest fixture with
  `time(ms)` header asserts end-of-file timestamp equals the data's real duration.
- **Deps:** T3, T4
- **Status 2026-08-10:** found live on the real Mazda file — ms column read as seconds,
  play paced one row per 16+ s.
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC6 =
      maintainer's `--benchmark-hotpath` run stays green).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted (quoted-cell sniff
      leak fixed via `topLevelSeparatorCount` + grid-consistency gate; validity-desync
      edge accepted and documented in `plan.md` risks).
- [x] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done` (pending AC verification after maintainer rebuild).
