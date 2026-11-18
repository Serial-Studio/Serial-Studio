---
spec: 0125-kiosk-launcher-control
phase: tasks
updated: 2026-11-18
---

# Tasks 0125 — Kiosk launcher control

> **The ordered checklist.** `/ss-implement` decomposes the Design in [`spec.md`](./spec.md)
> into units that are small, ordered, and *individually verifiable*, works the list top to
> bottom and keeps the boxes current. No approval gate: the spec was the gate.

## Tasks

### T1 — Control client

- **Files:** `core/Ui/Platform/KioskControl.h`, `core/Ui/Platform/KioskControl.cpp`
- **Does:** Local-socket client: 2 s retry timer, line framing with the 64 KiB cap, command
  decoding, hello and state lines. Redials come only from the timer; no `waitFor*`.
- **Verify:** `code-verify.py --check`, `syntax-check.py` with a donor entry.
- **Deps:** none
- [x] done

### T2 — Session

- **Files:** `core/Ui/Platform/KioskSession.h`, `core/Ui/Platform/KioskSession.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Placement state, the leftmost and inset rule, live screen lookup, state publishing.
  Zero reserve returns the input rectangle before any other logic.
- **Verify:** as T1.
- **Deps:** T1
- [x] done

### T3 — Application activation

- **Files:** `core/Ui/Platform/NativeWindow.h`, `NativeWindow_macOS.mm`, `NativeWindow_CSD.cpp`
- **Does:** `activateApplication()` slot; no-op off macOS.
- **Verify:** as T1.
- **Deps:** none
- [x] done

### T4 — Options and ownership

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`, `app/src/main.cpp`
- **Does:** Three options under the commercial gate, accessors gated on `kiosk()`; `main.cpp`
  owns the session, outliving the QML engine, and publishes `CLI_KIOSK_SESSION`.
- **Verify:** as T1.
- **Deps:** T2
- [x] done

### T5 — Window placement in QML

- **Files:** `app/qml/Widgets/SmartWindow.qml`, `app/qml/MainWindow/MainWindow.qml`,
  `app/qml/MainWindow/Panes/Dashboard.qml`,
  `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml`
- **Does:** `showKiosk()` takes its rectangle from the session; confined main window, docked
  external dashboards, clamped pop-outs, raise and close handlers, separate settings category.
  `kioskGeometry` stays the raw screen rectangle.
- **Verify:** `code-verify.py --check`; read-back of the unconfined path against the old one.
- **Deps:** T3, T4
- [x] done

### T6 — Unit tests

- **Files:** `app/tests/tst_kiosk_session.cpp`, `app/tests/tst_kiosk_control.cpp`,
  `app/tests/CMakeLists.txt`
- **Does:** The rectangle rule and the protocol against an in-process server.
- **Verify:** `syntax-check.py`; a PySide6 mirror of the protocol cases where it helps.
- **Deps:** T1, T2
- [x] done

### T7 — Documentation

- **Files:** `doc/help/Command-Line-Interface.md`, `doc/help/Operator-Deployments.md`,
  `doc/claude/directory-map.md`
- **Does:** Three option rows each, the protocol, one directory-map line.
- **Verify:** `documentation-verify.py`.
- **Deps:** T4
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath not touched.
- [x] `python scripts/sanitize-commit.py` run, only if a generator input changed.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
