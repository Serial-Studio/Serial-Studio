---
spec: 0125-kiosk-launcher-control
title: Kiosk launcher control (screen, reserved strip, control channel)
status: in-progress     # draft -> approved -> in-progress -> done | shelved
created: 2026-11-18
author: Alex Spataru
---

# Spec 0125 — Kiosk launcher control

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

## Problem / Motivation

`--kiosk` is used by an external launcher on a two-screen test-cell Mac. The launcher is turning
into a small window manager: it draws a dock on the left edge of the leftmost screen, and the
operator uses that dock to switch between the launcher's windows and the Serial Studio dashboard
and to move any of them to either screen. On macOS both programs hold the kiosk presentation
lock, so there is no Dock, no menu bar and no Cmd-Tab. The launcher's dock is the only way to
change window.

Today the kiosk decides everything itself. The main window fills the screen it was last saved
on, and every external dashboard or widget window saved on another screen fills that one. The
launcher cannot choose the screen, cannot keep its dock strip uncovered, and cannot tell a
running kiosk to come forward, move or close.

## Goals

- A launcher can start the kiosk on a screen it names, with nothing of Serial Studio elsewhere.
- A launcher can keep a strip free at the left edge of the leftmost screen.
- A launcher can, at run time, learn where the kiosk is and whether it is active, bring it
  forward, move it to another screen and close it.
- `--kiosk` with none of the new options behaves exactly as it does today.
- A launcher can discover support by reading `--help`.

## Non-Goals

- Nothing specific to one launcher: no product names, no fixed server name, no dock drawing.
- No server side. Serial Studio is only ever the client of the launcher's channel.
- No authentication on the channel. It carries three window commands and no project data; the
  launcher that started the process can already terminate it.
- No per-window control. The channel addresses the kiosk as one unit named `dashboard`.
- No change to the API server, to deployment packages or to the non-kiosk window behavior.
- No right, top or bottom reservation.

## Requirements

1. **R1** — `--kiosk-screen <name>` confines the kiosk to the screen with that name: the main
   window fills it and no Serial Studio window appears on another screen. An unknown name falls
   back to the primary screen with one warning on stderr. When two screens report the same name,
   the first in the system's screen order is used.
2. **R2** — In a confined kiosk, external dashboard windows are neither restored nor openable;
   their workspaces stay reachable from the main window's workspace list. Floating widget
   windows never fill a screen and are kept inside the kiosk screen (one exception, open for
   the maintainer: deviation 14). The project's saved window list is not rewritten by a
   confined run.
3. **R3** — `--kiosk-reserve-left <pixels>` (default 0) makes every kiosk-filled window on the
   leftmost screen start that many device-independent pixels to the right of the screen's left
   edge. The leftmost screen is the one with the smallest left coordinate; on a tie the primary
   screen wins. On any other screen the value has no effect. A negative or non-numeric value is
   read as 0, and the value is capped at half the screen width.
4. **R4** — `--kiosk-control <server-name>` connects, once the main window is shown, to a local
   server of that name. A failed or dropped connection is retried every 2 s for the life of the
   process, silently. Without the option no socket is opened.
5. **R5** — The channel is UTF-8, one JSON object per `\n`-terminated line, both ways. Unknown
   keys and unknown commands are ignored. A line that is not a JSON object is ignored. Any line over
   64 KiB, terminated or not, drops the connection (the retry then applies).
6. **R6** — On every connect the kiosk sends
   `{"hello":1,"id":"dashboard","title":…,"pid":…,"screen":…,"can":["raise","screen","close"]}`
   where `title` is the main window's title and `screen` the name of the screen it is on.
7. **R7** — The kiosk sends `{"event":"state","screen":…,"active":…}` whenever the main window's
   screen changes or the application becomes active or inactive.
8. **R8** — `{"cmd":"raise"}` makes Serial Studio the active application and brings the main
   window to the front. On macOS the kiosk presentation lock is in force again once it is active.
9. **R9** — `{"cmd":"screen","name":…,"reserve_left":…}` moves the kiosk to the named screen
   exactly as R1 to R3 would have placed it, and from then on the kiosk is confined. A present
   `reserve_left` replaces the reserved width; an absent one keeps the current width. An unknown
   name does nothing and sends nothing. A known name always answers with a `state` event, also
   when the screen did not change.
10. **R10** — `{"cmd":"close"}` quits through the same path as the operator's own quit in runtime
    mode, so recordings and exports are finalized the same way.
11. **R11** — All three options need `--kiosk`; without it they are ignored. All three names
    appear verbatim in `--help`.
12. **R12** — The options and the channel work on Windows and Linux, where raise is a plain
    window raise and activation request and there is no presentation lock.
13. **R13** — A confined run leaves the main window's saved position as it found it, so a later
    plain `--kiosk` opens where it would have opened had the confined run never happened.

## Acceptance Criteria

- [ ] **AC1** — `--runtime --kiosk` alone: same windows on the same screens as before the change
  (maintainer, two screens, a project with a saved external window on the second screen).
- [ ] **AC2** — `--kiosk-screen "<second>"`: nothing of Serial Studio on the first screen
  (maintainer).
- [ ] **AC3** — `--kiosk-screen "<leftmost>" --kiosk-reserve-left 64`: a 64 px strip stays free;
  on the other screen the full screen is used (maintainer; the rectangle rule itself is a unit
  test).
- [ ] **AC4** — Against a test server the hello line arrives; `screen` moves the kiosk and a
  `state` event follows; `raise` brings it in front of another full-screen window; `close` quits
  (framing, hello, state, command decoding and the oversize drop are a unit test against an
  in-process server; the window effects are the maintainer's).
- [ ] **AC5** — Killing the server does not disturb Serial Studio; it reconnects when the server
  returns and sends hello again (unit test with a shortened retry interval; maintainer once live).
- [ ] **AC6** — On macOS, after `raise` under the lock, the Dock and menu bar stay hidden
  (maintainer, with the real launcher).
- [ ] **AC7** — `--help` lists `--kiosk-screen`, `--kiosk-reserve-left`, `--kiosk-control`
  (maintainer, one command on a commercial build).
- [ ] **AC8** — In a confined kiosk with a widget popped out, switching to the launcher leaves
  the pop-out behind the launcher's windows and off the reserved strip (maintainer).
- [ ] **AC9** — Plain `--kiosk` on screen 1, then `--kiosk --kiosk-screen "<screen 2>"`, then
  plain `--kiosk` again: the third run opens on screen 1 (maintainer).

## Constraints & Invariants

- **Deciding constraint:** with no new option the kiosk produces the result it produces today.
  The placement function returns before any new logic when the reserved width is 0; the pure
  rule behind it is unit-tested with zero-reserve rows, and the QML path is checked by AC1.
- Kiosk stays a commercial, runtime-mode feature; the new options live under the same gate and a
  deployment package does not carry them. A GPL build therefore does not list them in `--help`,
  which is the correct answer to "does this build support them".
- The channel never blocks the interface thread and never shows a dialog.
- Nothing here touches the acquisition pipeline, licensing or the API server's auth and consent.
- No new dependency.

## Premise Check

- **Question:** does a local-socket client that dials an absent server stay non-blocking, and
  does it come back and greet again when the server returns?
- **Measured:** headless PySide6 6.11.1 spike on this Mac. A dial to an absent server returns in
  0.03 ms and reports the failure in 0.06 ms at worst over 20 dials. With a server that appears,
  dies and reappears, the client greeted twice. One finding shapes the design: the failure is
  reported from inside the dial call, and dialing again from that report is refused by Qt
  ("called while not in unconnected state"), so every redial has to come from the retry timer.
- **Verdict:** the channel premise holds. The other premise, that one locked application can
  become active while another locked application is in front, was **not** measured: testing it
  means holding the presentation lock, with Force Quit disabled, on the maintainer's desktop.
  It rests on the launcher yielding activation first, and AC6 is where it is proven or falls.

## Design

### Approach

Two small objects, neither a singleton: `main.cpp` owns the session, and the session owns the
client as a member. A **kiosk
session** holds the mutable placement (confined or not, screen, reserved width), computes the
rectangle a kiosk window should take, and tracks the main window's screen and the application's
active state. A **kiosk control client** owns the local socket, the 2 s retry timer and the line
protocol, and knows nothing about windows: it turns lines into three signals and turns hello and
state into lines. The session forwards raise and close as signals; the main window's QML
answers them with the existing quit function and the native window helper. QML keeps doing the window work it does today; `showKiosk()` asks the session for
the rectangle instead of using the raw screen geometry, which with no new option returns that
same geometry.

Candidates considered:

- **A. Session + client, QML places (chosen).** Smallest change to the window code that already
  works, and both halves are testable without a display.
- **B. C++ places every window.** Moves `showKiosk()` out of QML. More uniform, but rewrites the
  path AC1 says must not change.
- **C. Reuse the API server** with three new commands instead of a new channel. Breaks the
  contract's assumption that the launcher is the server; it also drags in the consent and
  capability surface for what is three window verbs, and gives no push events.
- **D. Smallest: options only, no channel** (about a third of the size). Drops R4 to R10; the
  launcher could place the kiosk at start but never raise it, which under the lock strands the
  operator.

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `app/src/Misc/CLI.h` | three options in the commercial block, three accessors | +25 |
| `app/src/Misc/CLI.cpp` | register the options, accessors gated on `kiosk()`, reserve parsed with the existing spec parsers | +45 |
| `app/src/main.cpp` | own the session, configure it from the CLI, expose it as `CLI_KIOSK_SESSION` | +14 |
| `core/Ui/Platform/KioskSession.h` (new) | properties `confined`, `screenGeometry`; `fillRect()`, `moveToScreen()`, `setMainWindow()` | +95 |
| `core/Ui/Platform/KioskSession.cpp` (new) | leftmost rule, inset, live screen lookup, screen and active tracking | +210 |
| `core/Ui/Platform/KioskControl.h` (new) | client interface: three command signals, hello and state senders | +85 |
| `core/Ui/Platform/KioskControl.cpp` (new) | socket, retry timer, line framing with the 64 KiB cap, JSON in and out | +230 |
| `core/Ui/Platform/NativeWindow.h` | `activateApplication()` slot | +1 |
| `core/Ui/Platform/NativeWindow_macOS.mm` | `[NSApp activate]`; the deployment floor is macOS 14 | +14 |
| `core/Ui/Platform/NativeWindow_CSD.cpp` | no-op | +8 |
| `core/Ui/CMakeLists.txt` | four new sources | +4 |
| `app/qml/Widgets/SmartWindow.qml` | `showKiosk()` takes its rectangle from the session; a confined main window targets the session's screen and keeps its loaded saved position | +37 |
| `app/qml/MainWindow/MainWindow.qml` | hand the window to the session once shown; re-place on a move | +15 |
| `app/qml/MainWindow/Panes/Dashboard.qml` | confined: reconcile against an empty list, opener and saved-list writer return early | +14 |
| `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml` | keep a floating widget window inside the kiosk screen | +14 |
| `app/tests/tst_kiosk_session.cpp` (new) | rectangle rule | +140 |
| `app/tests/tst_kiosk_control.cpp` (new) | protocol against an in-process server | +220 |
| `app/tests/CMakeLists.txt` | two tests | +20 |
| `doc/help/Command-Line-Interface.md`, `doc/help/Operator-Deployments.md` | three rows each, the protocol in the first | +45 |
| `doc/claude/directory-map.md` | one line for the two new files | +2 |
| `app/rcc/ai/search_index.json` | regenerated by `sanitize-commit.py`, not by hand | generated |
| **Total** | code 819 / tests 380 / docs 47 | **+1246** |

### Hotpath & threading impact

- **Touches the hotpath?** No. Nothing on the driver, reader, builder or dashboard draw path.
- **New cross-thread signal/slot?** No. The socket, its timer and both objects live on the GUI
  thread; traffic is a handful of lines per operator action. No `waitFor*` call anywhere.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.

### Data model, API and UI

- No project or schema change, and the session's own state is never saved. Window geometry is
  another matter: every window saves its position on exit, kiosk-filled or not, and the
  unpinned kiosk picks its screen from that. A confined main window therefore saves under a
  category of its own (R13, below).
- In a confined kiosk `showKiosk()` fills only the main window. Every other window gets
  `false` back and floats, clamped into the kiosk screen.
- In a confined kiosk the external-window opener and the saved-list writer both return early,
  so neither the operator nor a reconcile can spawn a window or overwrite the project's list.
  Workspaces remain switchable from the start menu, which runtime mode keeps (checked in
  `StartMenu.qml`).
- No API-server command. The channel is its own surface, opened only by an explicit option.
- One new QML global, `CLI_KIOSK_SESSION`, set beside the other `CLI_*` values and present in
  every build; outside a kiosk it reports not confined and a reserved width of 0.
- **Amended during implementation (2026-11-18), smaller than approved:** the session was to be
  owned and wired by the module manager under a `Cpp_*` name. That source file sits exactly at
  the 1500-line cap, and a `Cpp_*` name must be registered there. Ownership moved to `main.cpp`,
  which already publishes the `CLI_*` values, and the module manager is not touched.
- R13 is met by a separate saved-geometry category for a confined main window, so the normal
  category is never written by a confined run, also when the launcher kills the process.
- **Known gap, needs a file outside this list:** a widget pop-out that already fills another
  screen (a kiosk started without `--kiosk-screen`) stays there when a run-time `screen`
  command confines the kiosk. Closing it from here would rewrite the project's saved pop-out
  set, or switch a tool off, through the canvas's close handler; doing it cleanly is a
  suppressed close in `DashboardCanvas.qml`. A kiosk started with `--kiosk-screen` never has
  such a window.
- Screen names are matched against the live screen list at the time of the command, not the
  start-up snapshot QML holds, so a monitor plugged in later can be targeted.
- Redials come only from the retry timer (see Premise Check). Hello is sent from the socket's
  connected signal, so every reconnect greets again.
- `raise` runs: activate the application, re-assert the lock, raise and activate the main
  window. The lock is also re-asserted whenever the application becomes active in a kiosk
  that has a control channel.
- A `state` line also follows every hello, so the launcher learns `active` on connect.
- `close` invokes the same root quit function the macOS quit interceptor already uses.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| External dashboard windows in a confined kiosk | (a) not opened, content stays in the main window; (b) opened as floating windows on the kiosk screen | (a). The contract allows it as the safer choice. (b) puts always-on-top windows over a screen the launcher also draws on. |
| What `kioskGeometry` stores | the raw screen rectangle; the inset rectangle | Raw. An external window decides "is that screen taken" by comparing origins, and an inset origin would make it fill the main window's screen. |
| Where the two classes live | `core/Ui/Platform` beside the native window helper; `app/src/Misc` | `core/Ui/Platform`. The composition root keeps only CLI and wiring, and the tests link the library. |
| Channel transport | new local-socket client; API server commands | Local socket, as the contract specifies (candidate C). |
| `screen` without `reserve_left` | keep the current width; reset to 0 | Keep. A launcher that only wants to move sends less, and the reset is one explicit key away. |
| Duplicate screen names | first in screen order; refuse | First. Refusing would make identical monitors unusable; the launcher can see the same order. |
| Reserved width bounds | clamp to 0..half the screen; trust the value | Clamp. A typo must not produce a zero-width kiosk nobody can close from inside. |
| Trust on the channel | none beyond the server name; a shared token | None. The verbs are raise, move and close, and the peer already owns the process. Named here because it is a new inbound surface. |

### Risks

- **Always-on-top widget windows over the launcher.** Floating widget windows are kept above a
  kiosk-filled dashboard today. On macOS that level may also sit above the launcher's windows
  when the launcher is in front. Not verified. AC8 checks it; if it fails, the fix is to hold
  that flag only while the application is active, about five lines in the dashboard canvas.
- **Assumed, confirmed at implementation:** the UI library reaches Qt Network (it already hosts
  network diagnostics), the unit tests can link the two new sources, and the translation-unit
  census needs `--accept` for two new files.
- **Activation refused.** On macOS 14+ a background application's request to become active is
  cooperative. If the launcher does not yield first, `raise` does nothing visible. The state
  event (`active` stays false) tells the launcher so.
- **Closing external windows on a run-time move.** Moving an unconfined kiosk confines it, which
  closes open external dashboard windows. Their close must not write an empty window list back
  into the project; the existing restore guard covers reconcile-driven closes, and the
  implementation verifies that before relying on it.
- **Mixed-DPI move.** A frameless window moved between screens of different scale relies on Qt
  re-laying it out; the geometry is assigned again after the screen change lands.
- **Stale start-up screen snapshot** in QML for the unpinned path: unchanged from today.

### Tests

- AC3 → `tst_kiosk_session`: leftmost selection, the primary tie-break, inset on the leftmost
  only, clamp, zero reserve returns the input rectangle unchanged (the AC1 guard in miniature).
- AC4, AC5 → `tst_kiosk_control`: hello on connect and on reconnect, state line, the three
  commands decoded, unknown command and unknown keys ignored, non-object line ignored, 64 KiB
  without a newline drops the link, a split line across two reads is reassembled.
- AC1, AC2, AC6, AC7 and the window half of AC4 → maintainer, in the built application. I can
  neither build nor launch it, so these stay unverified until then.
- Every edited or new C++ file goes through `scripts/syntax-check.py`; the new ones borrow a
  donor compile entry.

## Deviations from the launcher's contract

1. A commercial build and `--runtime` (or `--deployment`) are required, as for `--kiosk` itself.
   A GPL build does not list the options in `--help`.
2. Confined, external dashboard windows stay docked (the contract's allowed alternative).
3. `close` shows no save prompt: runtime mode is an ephemeral session with nothing to save.
   Recordings and exports are finalized as on any quit.
4. A `screen` command confines the kiosk for the rest of the run, also one that started
   unconfined.
5. `screen` without `reserve_left` keeps the current reserved width.
6. `screen` with an unknown name sends no `state` event.
7. Two screens with the same name: the first in system order is used.
8. The reserved width is clamped to 0..half the screen width.
9. The 64 KiB limit applies to terminated lines too.
10. A `state` event is sent right after every hello.
11. In a `screen` command a numeric `reserve_left` is truncated to whole pixels and held at 0 or
    above; a `reserve_left` that is not a number counts as absent.
12. A confined kiosk whose screen is unplugged moves to the primary screen and reports it.
13. A launcher that stops reading while commands keep arriving is dropped once 64 KiB of replies
    are queued; the 2 s retry then applies.
14. A widget pop-out that already fills another screen is not closed by a run-time `screen`
    command (see Design); start with `--kiosk-screen` to avoid it.

## Open Questions

- **Re-confirmation needed (amended during implementation, 2026-11-18):** session ownership
  in `main.cpp` as `CLI_KIOSK_SESSION`; R13 through a separate settings category; deviations
  10 to 14; and whether `DashboardCanvas.qml` joins the file list to close deviation 14.
- Settled with the maintainer on 2026-11-18: external dashboards stay docked when
  confined; the macOS activation premise is proven live by AC6, not by a spike; a `screen`
  command confines for the rest of the run; a confined run does not move the saved position.
