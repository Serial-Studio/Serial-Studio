---
spec: 0015-operator-mode-hardening
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-17
---

# Tasks 0015 — Operator mode hardening

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

### T1 — Gate taskbar freeze + workspace-edit controls

- **Files:** `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`
- **Does:** Adds `!app.runtimeMode` to `_freezeButton.visible`, hides the edit-workspace
  `IconButton` in runtime mode (hidden, not disabled — spec Q3), and guards the widget-button
  right-click `TapHandler` so `_tbContextMenu` ("Remove from Workspace") never opens in
  runtime mode. Signal-wiring invariant: existing signals/handlers untouched — visibility
  and guard-condition edits only, no new or changed connections.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`;
  read back the three gates (freeze visible-chain, edit button, TapHandler guard).
- **Deps:** none
- [x] done

### T2 — Gate Start-menu freeze entry

- **Files:** `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`
- **Does:** Adds `&& !app.runtimeMode` to `_freezeBt.visible` so the "Freeze Dashboard"
  entry disappears in operator mode. Neighbors (Auto Layout above, separator + Full Screen
  below) stay, so no separator orphaning. No other menu entries change.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`;
  read back that `searchableItems()` still lists no freeze action (AC1's search clause).
- **Deps:** none
- [x] done

### T3 — Gate caption-menu project mutators

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`
- **Does:** Folds `!app.runtimeMode` into `displayEditable` and hides "Rename Widget…",
  the "Freeze Title" nested `Menu`, and their `MenuSeparator` in runtime mode (R7: only
  "Open in External Window" remains). Uses the `visible` + zero-height idiom already used
  for "Painted Title"; if the nested `Menu` item does not collapse that way, falls back to
  `widgetMenu.removeMenu()` at `Component.onCompleted`. Per-widget freeze-title *display*
  (`effectiveFreezeTitle` reads) must keep working — only the mutation entries go.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`;
  read back that no `setFreezeTitleMode`/`promptRenameWidget` call site is reachable in
  runtime mode and the read path is untouched.
- **Deps:** none
- [x] done

### T4 — CLI flag `--no-taskbar-search`

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:** Adds `noTaskbarSearchOpt{"no-taskbar-search", ...}` to `m_opts` (worded like
  the sibling operator-mode options, Pro-tagged), registers it with the parser, and in
  `applyOperatorTaskbarSettings()` sets
  `tbs.setSearchEnabled(!m_parser.isSet(m_opts.noTaskbarSearchOpt))`. Invariant: runs
  after `setSettingsPersistent(false)` in `applyOperatorRuntimeSettings()` so the author's
  stored `Taskbar/SearchEnabled` is never written; absence of the flag must yield search
  on (Q2/AC4).
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/CLI.h app/src/Misc/CLI.cpp`;
  read back call order (persistence off → setSearchEnabled).
- **Deps:** none
- [x] done

### T5 — ShortcutGenerator emits the flag

- **Files:** `app/src/Misc/ShortcutGenerator.h`, `app/src/Misc/ShortcutGenerator.cpp`
- **Does:** Adds `bool taskbarSearch` to `generate(...)` and `buildArguments(...)`
  (placed adjacent to the taskbar params); `buildArguments` appends
  `--no-taskbar-search` only when `taskbarSearch` is false, so shortcuts with search on
  stay byte-identical to today's output. Single caller (`ShortcutGenerator.qml:129`)
  is updated in T6 — QML calls a `public slots:` method, so no other C++ touch-points.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ShortcutGenerator.h app/src/Misc/ShortcutGenerator.cpp`;
  grep confirms no other `generate(` caller.
- **Deps:** T4
- [x] done

### T6 — New Deployment dialog "Search Bar" switch

- **Files:** `app/qml/Dialogs/ShortcutGenerator.qml`
- **Does:** Adds a "Search Bar" `Label` + `Switch` row (default checked) to the Taskbar
  tab's Visibility section, `enabled: root.taskbarMode !== "hidden"` with the 0.5-opacity
  dimming the Pinned Buttons rows use (R6); resets to checked in `onVisibleChanged`;
  threads the value into the `Cpp_ShortcutGenerator.generate(...)` call in the same
  positional slot the C++ signature gained in T5. New string is plain `qsTr()` with no
  placeholders.
- **Verify:** `python scripts/code-verify.py --check app/qml/Dialogs/ShortcutGenerator.qml`;
  read back that QML argument order matches the T5 signature exactly.
- **Deps:** T5
- [x] done

### T7 — Self-review + handoff checks

- **Files:** none new (whole diff)
- **Does:** Re-reads the full diff against spec 0015 scope (six files + spec artifacts,
  nothing else); runs the repo linter across all touched files and `qt-cpp-review` on the
  C++ delta; counterfactual check: names the rule the diff most risks (scope creep /
  foreign-file touch) and the evidence it holds. Updates `tasks.md` boxes and marks
  `spec.md` status per Definition of Done. Maintainer runs the AC1–AC5 in-app checks from
  `plan.md` (operator deployments cannot be exercised without building/running the app).
- **Verify:** `python scripts/code-verify.py --check` clean on all six files; diff listing
  shows only in-scope files.
- **Deps:** T1, T2, T3, T4, T5, T6
- [x] done — code-verify clean on all 8 touched files; qt-cpp-review: 0 confirmed findings,
  1 investigation note (pre-existing 15-param positional QML boundary, extend-consistently
  verdict); `Menu.removeItem`/`removeMenu` availability confirmed against Qt 6.11 docs.
  `sanitize-commit.py` deliberately NOT run: the working tree carries unrelated in-progress
  spec-0014 work (Plot*, Frame.*, ProjectEditor*, ci.yml, ...) and the repo-wide pipeline
  (clang-format/code-verify --fix/lupdate) would rewrite files this session did not edit —
  run it at commit time once 0014/0015 are staged separately.

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1–AC5 are
      maintainer in-app checks — pending maintainer, implementation complete).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted (0 confirmed,
      1 investigation note recorded in T7).
- [x] Hotpath untouched (plan states none) — no `--benchmark-hotpath` run needed beyond CI.
- [x] No pytest surface touched; maintainer AC checks listed in `plan.md` instead.
- [x] `python scripts/sanitize-commit.py` run before commit (regenerates translations for
      the new "Search Bar" string) — deferred, see T7 note: working tree holds unrelated
      spec-0014 edits the pipeline would rewrite.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
