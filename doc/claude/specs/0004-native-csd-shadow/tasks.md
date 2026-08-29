---
spec: 0004-native-csd-shadow
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-07
---

# Tasks 0004 — Native OS/WM window shadows for CSD windows

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

### T1 — Strip the shadow layer from the CSD decorator

- **Files:** `app/src/Platform/CSD.h`, `app/src/Platform/CSD.cpp`
- **Does:** Deletes `ShadowRadius`, the shadow atlas generators (`generateShadowCorner`,
  `buildShadowAtlas`), `ShadowImageProvider`, `setupFrame()`, `updateFrameGeometry()`,
  `m_frame`, `m_shadowEnabled`, the `shadow` ctor parameter, and the alpha-buffer /
  `Qt::transparent` surface setup. `shadowMargin()` is removed.
- **Verify:** `python scripts/code-verify.py --check app/src/Platform/CSD.h
  app/src/Platform/CSD.cpp`; grep confirms no reference to `ShadowRadius`, `shadowMargin`,
  `m_frame`, or `csdshadow` remains in `CSD.*`.
- **Deps:** none
- [x] done

### T2 — Re-anchor CSD layout at the true window rect

- **Files:** `app/src/Platform/CSD.cpp`
- **Does:** Titlebar at (0,0) spanning the window width; content container at
  (0, titleBarHeight()) filling the rest; border sized to the full window rect; `edgeAt()`
  resize band = `ResizeMargin` from the real edges; `updateMinimumSize()` /
  `onMinimumSizeChanged()` lose their `2 * margin` terms; the `windowStateChanged` handler
  keeps hiding border (and sizing titlebar) for maximized/fullscreen without any frame
  bookkeeping.
- **Verify:** `python scripts/code-verify.py --check app/src/Platform/CSD.cpp`; read-back:
  no geometry expression in `CSD.cpp` references a margin other than `ResizeMargin`.
- **Deps:** T1
- [x] done

### T3 — Purge the shadow preference and frameMargin from NativeWindow

- **Files:** `app/src/Platform/NativeWindow.h`, `app/src/Platform/NativeWindow_CSD.cpp`,
  `app/src/Platform/NativeWindow_macOS.mm`
- **Does:** Removes the `csdShadowEnabled` Q_PROPERTY, getter/setter, signal, member, and
  the `Window/CSDShadowEnabled` QSettings read/write; removes `frameMargin()` from header
  and both implementations; `frameTopInset()` returns the decorator titlebar height only
  (pre-show fallback: `CSD::TitleBarHeight`); `CSD::Window` construction drops the shadow
  argument.
- **Verify:** `python scripts/code-verify.py --check` on the three files; grep confirms
  `csdShadowEnabled` and `frameMargin` appear nowhere under `app/src/`.
- **Deps:** T1
- [x] done

### T4 — Windows 10 DWM shadow enabler

- **Files:** `app/src/Platform/NativeWindow_CSD.cpp`
- **Does:** Adds the `Q_OS_WIN`-only native-shadow path: an `enableNativeShadow(QWindow*)`
  helper that restores `WS_THICKFRAME | WS_CAPTION` on the HWND and issues
  `SWP_FRAMECHANGED`, plus a lazily-installed process-wide `QAbstractNativeEventFilter`
  tracking decorated HWNDs that answers `WM_NCCALCSIZE` with the full window rect (inset by
  `SM_CXSIZEFRAME`/`SM_CYSIZEFRAME` + `SM_CXPADDEDBORDER` when maximized; monitor rect
  behavior preserved for fullscreen). Wired from `addWindow()` for decorated (non-Win11)
  windows; unregistered on window destruction.
- **Verify:** `python scripts/code-verify.py --check app/src/Platform/NativeWindow_CSD.cpp`;
  read-back of the filter against the plan's maximized/fullscreen risk notes (AC5/AC8 smoke
  is the maintainer's runtime gate).
- **Deps:** T2, T3
- [x] done

### T5 — Remove the Window Shadow toggle from Settings

- **Files:** `app/qml/Dialogs/Settings.qml`
- **Does:** Deletes the "Window Shadow" Label + Switch rows (the `csdShadowEnabled`
  bindings); the "Custom Window Decorations" switch and restart note stay.
- **Verify:** `python scripts/code-verify.py --check app/qml/Dialogs/Settings.qml`; grep
  confirms `csdShadowEnabled` appears nowhere under `app/qml/`.
- **Deps:** T3
- [x] done

### T6 — Drop frameMargin from SmartDialog sizing

- **Files:** `app/qml/Widgets/SmartDialog.qml`
- **Does:** Removes the `frameMargin` property, its `Cpp_NativeWindow.frameMargin(root)`
  refreshes, and its terms in the width/height/min/max formulas; `titlebarHeight` +
  `frameTopInset` remain and now describe the exact chrome.
- **Verify:** `python scripts/code-verify.py --check app/qml/Widgets/SmartDialog.qml`; grep
  confirms `frameMargin` appears nowhere under `app/qml/`.
- **Deps:** T3
- [x] done

### T7 — Documentation sync

- **Files:** `doc/claude/directory-map.md`, `doc/claude/specs/0004-native-csd-shadow/tasks.md`
- **Does:** Updates the `Platform/` role notes (CSD no longer paints a shadow; Win10 uses a
  DWM native shadow; Linux uses the thin border) and keeps this checklist current. CLAUDE.md
  needs no change (no rule/architecture contract altered).
- **Verify:** `python scripts/documentation-verify.py` via sanitize pipeline (doc lint is
  read-only); read-back.
- **Deps:** T1-T6
- [x] done

### T8 — Self-review, static gates, and handoff

- **Files:** (no new edits — review pass)
- **Does:** Re-reads the full diff against the plan's file table (lane check), runs
  `qt-cpp-review` on the `Platform/` C++ diff, addresses or notes findings, then runs
  `python scripts/sanitize-commit.py`. Lists the maintainer's runtime checklist (AC1-AC8
  observations + `pytest tests/integration/ -v` on a CSD platform) in the handoff message.
- **Verify:** sanitize pipeline clean; diff contains only the files in the plan's table.
  qt-cpp-review findings: OWN-1 (broken visibleChanged deferral — fixed by calling
  enableNativeShadow directly from addWindow), A4-1/EH-1 (per-monitor DPI metrics — fixed
  with GetSystemMetricsForDpi), stale-HWND cluster (fixed by tracking QWindow* in a QSet and
  matching live winId() in the filter). All fixes re-linted clean.
- **Deps:** T1-T7
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met or explicitly handed to the maintainer
      as a runtime observation (AC1-AC9 listed in the handoff).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; three finding clusters fixed (see T8), rest clean.
- [x] Hotpath untouched (plan says none) — no `--benchmark-hotpath` run required.
- [x] Relevant `pytest` targets identified for the maintainer (`tests/integration/` on a CSD
      platform).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (ModuleManager.cpp is the maintainer's own pending edit, untouched).
- [x] `spec.md` status set to `done` after maintainer runtime verification.
