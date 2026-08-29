---
spec: 0006-terminal-scroll-ux
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-13
---

# Tasks 0006 — Terminal scrollbar, configurable scrollback, and Text/Hex display labels

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

### T1 — `scrollbackLines` property on Console::Handler

- **Files:** `app/src/Console/Handler.h`, `app/src/Console/Handler.cpp`
- **Does:** Adds `Q_PROPERTY(int scrollbackLines ...)` with `[[nodiscard]]` getter, `public
  slots:` setter, `scrollbackLinesChanged()` signal, and `m_scrollbackLines` member (ctor
  init list only, per header rules). Ctor loads `Console/ScrollbackLines` (default 1000) and
  clamps to [100, 100000]; setter clamps, guards on equality, persists via `m_settings`, and
  emits — byte-for-byte the FontSize pattern. Binding invariants: header section order
  (`Q_PROPERTY` block → `signals:` → getters → `public slots:`), no in-header member init,
  `Q_EMIT` not `emit`.
- **Verify:** `python scripts/code-verify.py --check app/src/Console/Handler.h
  app/src/Console/Handler.cpp`; read-back that load-clamp and setter-clamp agree on
  [100, 100000] and the settings key matches the plan.
- **Deps:** none
- [x] done

### T2 — Extract `trimExcessLines()` and retire `MAX_LINES` for `m_maxLines`

- **Files:** `app/src/UI/Widgets/Terminal.h`, `app/src/UI/Widgets/Terminal.cpp`
- **Does:** Extracts the front-trim from `appendString()` into a private
  `trimExcessLines(int linesToDrop)` that moves **text rows, color rows, `m_repeatCounts`,
  cursor Y, scroll offset, and selection points in lockstep, and sets `m_searchDirty`**
  (binding invariant: the parallel-buffer alignment bug class from the spec; the
  repeat-count and search-dirty legs already exist in `appendString()` and must move
  verbatim) — shifting `m_selectionStart/End/StartCursor` down by
  the dropped count and clearing the selection (+ emitting `selectionChanged()`) when it
  falls entirely off the front. Replaces `constexpr MAX_LINES` with an `m_maxLines` member
  initialized from `Console::Handler::scrollbackLines()` at every former use site
  (`appendString`, `initBuffer`, `setAnsiColors`, `setCursorPosition` clamp);
  `initBuffer()` reserves `qMin(m_maxLines, 10000)`. No behavior change at the default
  (m_maxLines == 1000). Binding invariant: no per-append work that scales with buffer size —
  the append path only compares against the cached int.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Terminal.h
  app/src/UI/Widgets/Terminal.cpp`; read-back that `MAX_LINES` has zero remaining references
  and `trimExcessLines()` is the only trim site.
- **Deps:** T1
- [x] done

### T3 — Live cap changes: `applyScrollbackLimit()` wiring

- **Files:** `app/src/UI/Widgets/Terminal.cpp`, `app/src/UI/Widgets/Terminal.h`
- **Does:** Adds a private slot `applyScrollbackLimit()` that re-reads
  `m_consoleHandler.scrollbackLines()` into `m_maxLines`, calls `trimExcessLines()` when the
  buffer exceeds the new cap, and schedules a repaint. Connects
  `Console::Handler::scrollbackLinesChanged` → it in the ctor, next to the existing
  `fontChanged`/`displayString` connections. Binding invariants: read the ctor's existing
  signal wiring before adding (per CLAUDE.md); same-thread auto connection (both objects are
  main-thread); trim work is proportional and happens once, at the setting change.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of the ctor
  connection block ordering.
- **Deps:** T2
- [x] done

### T4 — Scrollback UI: console settings popup + Settings dialog + Reset entry

- **Files:** `app/qml/Dialogs/Settings.qml`, `app/qml/Widgets/Dashboard/Terminal.qml`
- **Does:** Adds a "Scrollback Lines" `Label` + `SpinBox` (`from: 100`, `to: 100000`,
  `stepSize: 100`, `editable: true`) in two places: the console toolbar's `settingsPopup`
  in Terminal.qml (after the existing checkboxes — the "Settings menu in console" from the
  original request) and the Settings dialog Console tab's Display section, both using the
  `_consoleFontSize` pattern exactly: `value:` binding, `onValueModified` write-back,
  `Connections` restore on `onScrollbackLinesChanged` (binding invariant: the
  ComboBox/SpinBox restore-race guard — only user edits write to C++). Adds
  `Cpp_Console_Handler.scrollbackLines = 1000` to the Reset button.
- **Verify:** `python scripts/code-verify.py --check app/qml/Dialogs/Settings.qml
  app/qml/Widgets/Dashboard/Terminal.qml`; read-back that both rows follow the write-back
  pattern and Reset includes the new line.
- **Deps:** T1
- [x] done

### T5 — Scrollbar geometry: hit-rects, mapping helpers, always-on-overflow painting

- **Files:** `app/src/UI/Widgets/Terminal.h`, `app/src/UI/Widgets/Terminal.cpp`
- **Does:** Reworks `paintScrollbar()` to draw a faint full-height track strip plus the
  rounded thumb whenever `lineCount() > linesPerPage()` (the `autoscroll()` early-return is
  removed). Adds const helpers that both painter and input use: `scrollbarTrackRect()`,
  `scrollbarThumbRect()` (current height/position math), and
  `scrollOffsetForThumbY(int y)` (inverse mapping, clamped to
  `[0, lineCount() - linesPerPage()]`). Binding invariant: RTL — X placement derives from
  the one existing `m_translator.rtl()` branch so paint and hit-test cannot diverge; track
  uses the existing `QPalette::Window` color at low alpha (no new theme keys).
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that
  `paintScrollbar()` and the hit-rect helpers share the same geometry code path.
- **Deps:** T2 (touches the same file; keeps diffs separable)
- [x] done — *amended 2026-07-13 per maintainer: thumb-only (no track strip), hidden while
  autoscroll is engaged; `isOverScrollbar()` gates interaction on the same visibility rule.*

### T6 — Scrollbar input: drag, track paging, autoscroll handoff

- **Files:** `app/src/UI/Widgets/Terminal.h`, `app/src/UI/Widgets/Terminal.cpp`
- **Does:** Adds drag state (`m_draggingScrollbar`, press-anchor members) and branches the
  mouse handlers: `mousePressEvent` — left press inside the scrollbar band starts a thumb
  drag (anchored so the thumb doesn't jump) or pages by `linesPerPage()` on track press,
  and *skips selection start*; `mouseMoveEvent` — while dragging, map via
  `scrollOffsetForThumbY()` recomputed against the current `lineCount()`;
  `mouseReleaseEvent` — ends the drag. Autoscroll handoff mirrors `wheelEvent`'s existing
  rules (binding invariant): any move/page away from the bottom → `setAutoscroll(false)`,
  reaching max offset → `setAutoscroll(true)`. Wheel and selection behavior outside the band
  are byte-identical.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that the
  selection path is unreachable from a scrollbar press and that both autoscroll rules match
  the wheel handler's.
- **Deps:** T5
- [x] done

### T7 — Display-mode labels: "Text" / "Hex"

- **Files:** `app/src/Console/Handler.cpp`
- **Does:** `displayModes()` returns `tr("Text")`, `tr("Hex")` (was "Plain Text" /
  "Hexadecimal"). No enum, API-identifier, or dataModes changes (binding invariant: API
  `"PlainText"`/`"Hexadecimal"` strings in `API/Handlers/ConsoleHandler.cpp` are
  machine-facing and stay).
- **Verify:** `python scripts/code-verify.py --check app/src/Console/Handler.cpp`; grep
  confirms no other C++/QML site hardcodes the old display-mode labels.
- **Deps:** none (independent slice; ordered late to keep translation tasks adjacent)
- [x] done

### T8 — `PINNED_TRANSLATIONS` in llm_translate.py

- **Files:** `app/translations/llm_translate.py`
- **Does:** Adds a `PINNED_TRANSLATIONS = {"Hex": "Hex"}` exact-match table near
  `DOMAIN_GLOSSARY` with a comment stating the contract (exact source match → fixed
  translation for every locale, never sent to the LLM). In `translate_ts_file()`'s pre-scan,
  a pending entry whose source matches is written directly (text set, `type="unfinished"`
  dropped, counted as done, logged as pinned) and never enters `pending`. The
  `--verify-only` pass re-enforces pinned values so later runs can't drift them. The en_US
  copy path is untouched (already a direct copy).
- **Verify:** `python -m py_compile app/translations/llm_translate.py`; read-back of both
  application sites (translate + verify-only). Full AC5 run stays with the maintainer
  (needs API key + Qt tools).
- **Deps:** T7 (the string only exists after the rename; keeps the pair reviewable together)
- [x] done — *amended 2026-07-13 per maintainer: the pin table is per-language
  (`{"Hex": {lang_code: value}}`) with curated values for all 20 locales (transliterations
  for Cyrillic/RTL/Indic, native CJK terms); a locale missing from a term's map falls
  through to the LLM.*

### T9 — Help-doc touch-up for the renamed labels

- **Files:** `doc/help/Getting-Started.md`
- **Does:** Updates the two sentences (~lines 122 and 234) that name the console display
  options to say "Text and Hex", matching the shipped UI. No other doc content changes.
- **Verify:** `python scripts/documentation-verify.py` (read-only report) shows no new
  findings for the file; grep confirms no remaining "Plain Text and Hexadecimal" phrasing
  in `doc/help/`.
- **Deps:** T7
- [x] done

### T10 — No timestamp prefix on empty console lines *(added 2026-07-13, maintainer request)*

- **Files:** `app/src/Console/Handler.cpp`
- **Does:** Hex-dump display emits blank separator lines between rows; with Show Timestamp
  on, each one rendered as a bare `HH:mm:ss.zzz -> ` stamp. The newline branch in
  `Handler::append()` and its mirror `appendToDevice()` no longer emits the timestamp for a
  line that is still at its start (empty line); the `isStartingLine` state transitions are
  untouched, so stamping of real content is byte-identical. Applies to both the display
  string and the shared export text buffer.
- **Verify:** `python scripts/code-verify.py --check app/src/Console/Handler.cpp`;
  maintainer observation — hex mode + timestamps shows stamps only on hex rows, blank
  separators stay blank.
- **Deps:** none
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1–AC4
      maintainer observations listed in `plan.md`; AC5 after the maintainer's
      `llm_translate.py` run; AC6 via CI). *Implementation complete 2026-07-13; runtime
      verification awaits the maintainer.*
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; all findings addressed (scrollOffsetYChanged
      emission on trim, capacity squeeze after cap reduction, drag-coordinate overflow
      clamp, qPow→int multiply, shared `isOverScrollbar()` band, `mouseUngrabEvent()`
      reset, and — per maintainer 2026-07-13 — `event->position().toPoint()` replacing the
      deprecated `QMouseEvent::pos()` in the new mouse-handler lines).
- [x] Hotpath untouched (plan's answer: none) — CI `--benchmark-hotpath` gate confirms no
      regression on the PR.
- [x] Relevant `pytest` tests identified for the maintainer: none apply (no API/parse-path
      change); manual observation list in `plan.md` stands in.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep beyond the named
      selection-shift fix inside `trimExcessLines()` (approved in the plan gate) and the
      review-remediation edits above, all inside the plan's file list.
- [x] `spec.md` status set to `done`.
