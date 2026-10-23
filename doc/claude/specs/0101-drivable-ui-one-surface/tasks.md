---
spec: 0101-drivable-ui-one-surface
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-19
---

# Tasks 0101 — Drivable UI and One Command Surface (Part A)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

This list covers **Part A only**: the driving surface, its gating, the identifier sweep, the
linter rules, the tests and the docs. Parts E, C and D get their own addendum and checklist
after Part A lands (spec R27).

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
  Every C++ task also runs `python scripts/syntax-check.py <files>`; a SKIP means "unverified"
  and is reported as such.
- **Deps** lists task IDs that must land first.
- **Build points** are the maintainer's: the tree is built after T9 (ctest spike), after T11
  (handler registered, schema dump possible) and before T20 (pytest). Nothing here invokes
  cmake or a compiler.
- Tasks marked **delegable** are mechanical sweeps the main session briefs to a cheaper
  executor (Sonnet; Haiku for the pure one-line sweeps) and reviews; design tasks stay in the
  main session.

## Tasks

### T1 — `DrivesUi` capability and the two tables

- **Files:** `core/Api/API/CommandCapabilities.h`, `core/Api/API/CommandCapabilities.cpp`
- **Does:** Adds `DrivesUi = 0x0100` to the capability enum; `ui.automation.click` and
  `ui.automation.key` join `kCapable` with `DrivesUi` always and no argument-dependent part;
  `listWindows`, `listElements`, `screenshot`, `setReduceMotion` join `kPlain`. Both tables
  stay sorted (binary search) and the six names appear in exactly one table.
- **Verify:** `code-verify.py --check` and `syntax-check.py` on both files; read back the sort
  order around the `ui.` block. The registry ctest passes only after T12 (schema snapshot).
- **Deps:** none
- [x] done

### T2 — Sealed policy

- **Files:** `core/Api/API/SealedPolicy.cpp`
- **Does:** All six `ui.automation.*` names in `kDenied`, sorted. The sealed integration test
  derives its denied list from this file, so AC6 needs no test edit.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tst_sealed_policy_registry` after
  T12.
- **Deps:** none
- [x] done

### T3 — Third consent gate in the server auth half

- **Files:** `core/Api/API/Server/ServerAuth.h`, `core/Api/API/Server/ServerAuth.cpp`
- **Does:** Adds `ConsentGate m_uiDrive` with an **empty settings key** (session-only, like
  the two beside it), title "Allow API clients to drive the user interface?" and a body naming
  what a click can do; `authorizeRemoteCommand` routes `DrivesUi` to it and nothing else in the
  mapping moves; the auto-consent variable grants it in the ctor with the other two. Binding
  invariants, named here because this is a decision-visible security surface: remote origin
  stays a dispatch scope, the mapping lives only in `authorizeRemoteCommand`, a yes never
  reaches disk, the offscreen platform denies without the variable, and consents do not
  compose.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `term-verify.py` on the two new
  user-facing strings; T4's ctest.
- **Deps:** T1
- [x] done

### T4 — `tst_server_auth`: non-composition and the headless grant

- **Files:** `app/tests/tst_server_auth.cpp`
- **Does:** Two cases: granting the device-write gate alone leaves a `DrivesUi` command
  answered consent-required or denied (offscreen), never allowed; with the auto-consent
  variable set at construction the same command is allowed. Uses the existing fixture shape
  in the file.
- **Verify:** `code-verify.py --check`; `ctest -R tst_server_auth` after the maintainer's
  build at T9.
- **Deps:** T3
- [x] done

### T5 — `WindowInventory`

- **Files:** `core/Ui/UI/Automation/WindowInventory.h`, `core/Ui/UI/Automation/WindowInventory.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Lists visible `QQuickWindow`s from `topLevelWindows()`; resolves a window by
  `objectName` with `main` as the default; reports title, frame geometry, `devicePixelRatio`,
  active, `modal` (a visible overlay child whose popup object reports `modal`) and
  `focusedId`. Public Qt API only; no event loop; `SS_ASSERT` on every pointer boundary.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T9.
- **Deps:** none
- [x] done

### T6 — `ElementCatalog`

- **Files:** `core/Ui/UI/Automation/ElementCatalog.h`, `core/Ui/UI/Automation/ElementCatalog.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Iterative depth-first walk with an explicit stack, depth cap 64, item cap 16384
  (both named constants); the role table from the plan (Qt Quick Controls class-name prefixes
  plus the repository's Item-based controls by QML component name parsed up to `_QMLTYPE_`);
  entry fields `id, role, label, text, x, y, width, height, enabled, checked, focused` with
  the label read from the item's `text`, `title`, `placeholderText` or `toolTip` (the command
  manifest is loaded in QML, so no C++ title lookup exists); `id` and `label`
  substring filters; `byId()` (first in walk order) and `atPoint()` (last visible control
  containing the point). No allocation in the per-item step beyond the result vector reserve.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T9.
- **Deps:** none
- [x] done

### T7 — `InputSynthesizer`

- **Files:** `core/Ui/UI/Automation/InputSynthesizer.h`, `core/Ui/UI/Automation/InputSynthesizer.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** `click(window, point, button, modifiers, double)` delivers move, press, release
  (and the double-click pair) through `QWindowSystemInterface::handleMouseEvent` with the
  **synchronous delivery** template argument; `keys(window, QKeySequence)` and `text(window,
  QString)` through `handleKeyEvent` likewise, one press and release per key or character;
  `focus(item)` calls `forceActiveFocus()`. Binding invariant: synchronous delivery only, so no
  event is left queued and no event loop is spun inside a command.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T9.
- **Deps:** none
- [x] done

### T8 — `WindowCapture`

- **Files:** `core/Ui/UI/Automation/WindowCapture.h`, `core/Ui/UI/Automation/WindowCapture.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** `grabWindow()`; when the grab is an FP16 format, applies the inverse output
  transform with the constants from `Misc/HdrTransfer.h` (divide by SDR white, linear to sRGB)
  and converts to 8-bit; optional crop (element rect times `devicePixelRatio`) and `scale`
  (0.1 to 1.0); PNG into a `QBuffer`; halves the scale until the base64 reply is under the
  named 4 MiB cap (the socket's pending-write cap is 16 MiB). Returns width, height and the
  scale actually used.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T9.
- **Deps:** none
- [x] done

### T9 — `tst_ui_automation`: the offscreen spike (maintainer build point)

- **Files:** `app/tests/tst_ui_automation.cpp`, `app/tests/support/automation/Fixture.qml`,
  `app/tests/CMakeLists.txt`
- **Does:** A `QQuickView` on the offscreen platform loading a fixture with a named `Button`,
  `CheckBox`, `TextField`, `ComboBox`, a `ToolbarButton`-shaped Item control with properties,
  and a modal `Popup`. Cases: catalog ids, roles and geometry; `_QMLTYPE_` parsing; modal
  detection before and after `open()`; a click by id toggles the check box and a click on a
  disabled control is refused; typed text lands in the field and a `Ctrl+A` sequence selects
  it; `grabWindow()` returns a non-null image whose size is the view size times the ratio, and
  a crop equals the element rect. This is the spike AC16 asks for: if the grab is null on
  offscreen, the case records it and the GUI tier carries AC4.
- **Verify:** `code-verify.py --check`; the maintainer builds with `SS_BUILD_TESTS=ON`; then
  `ctest -R "tst_ui_automation|tst_server_auth"` against the build dir.
- **Deps:** T4, T5, T6, T7, T8
- [x] done

### T10 — `AutomationHandler` and registration

- **Files:** `core/Ui/ApiHandlers/AutomationHandler.h`, `core/Ui/ApiHandlers/AutomationHandler.cpp`,
  `core/Ui/ApiHandlers/UiHandlers.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** Registers the six commands with schemas (`enumProp` for `button`, `rangeProp` for
  `scale`) and descriptions that state the gating and the native-dialog caveat for `click`;
  validates params; maps T5 to T8 onto the reply shapes in the plan's API table; answers an
  execution error naming "no window" when the inventory is empty (headless);
  `setReduceMotion` calls the graphics backend setter and returns the previous value. Static
  members only, like `WindowHandler`; registered from `registerAll()`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T12's ctests; a manual
  `io.getStatus`-style probe of `ui.automation.listWindows` through `tests/utils/api_client.py`
  once the maintainer has the app up.
- **Deps:** T1, T2, T3, T9
- [x] done

### T11 — Assistant tiers and the static tests

- **Files:** `app/rcc/ai/command_safety.json`, `tests/scripts/test_ai_assistant_static.py`
  (the diagnostics static file pins only diagnostics commands and stays untouched)
- **Does:** `listWindows`, `listElements`, `setReduceMotion` in `safe`; `screenshot` in
  `confirm`; `click`, `key` in `deviceGated`. The static tests pin each of the six to exactly
  one tier, the tier named here.
- **Verify:** `pytest tests/scripts/test_ai_assistant_static.py -q` (runs without the app).
- **Deps:** T10
- [x] done

### T12 — Schema snapshot and generated surfaces (maintainer build point)

- **Files:** `app/rcc/api/api-schema.json`, `app/rcc/api/SerialStudio.js`,
  `app/rcc/api/SerialStudio.lua`, `app/rcc/api/sdk-symbols.json`,
  `app/rcc/api/proto-fields.json`, `doc/grpc/serialstudio-typed.proto`,
  `app/rcc/ai/search_index.json`
- **Does:** After the maintainer rebuilds with T10 in, run the built binary with
  `--dump-api-schema app/rcc/api/api-schema.json`, then `scripts/generate-sdk.py`,
  `scripts/generate-property-registry.py` (proto field numbers append only) and
  `app/rcc/ai/build_search_index.py`. Never hand-edit a generated file.
- **Verify:** `ctest -R "tst_command_capability_registry|tst_sealed_policy_registry|tst_path_policy_registry"`
  passes; `generate-sdk.py --check` and `generate-property-registry.py --check-snapshot --strict`
  are clean; `pytest tests/scripts/test_proto_ledger_static.py -q`.
- **Deps:** T10
- [ ] done

### T13 — Identifier sweep: manifest-driven delegates

- **Files:** `app/qml/Widgets/CommandToolbar.qml`, `app/qml/Widgets/CommandMenu.qml`,
  `app/qml/Widgets/CommandPalette.qml`, `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`
- **Does:** `objectName: node.id` on the toolbar command button and overflow entries;
  `objectName` passed in the `MenuItem` creation properties in `CommandMenu`; palette and
  start-menu delegates named `palette.tool.<id>`, `palette.workspace.<id>`,
  `palette.window.<id>`. No visual or behavioral change. Landed: toolbar buttons, menu items,
  start-menu command and toggle entries, palette workspace, folder, window and add cells; palette
  tool cells stay unnamed because `PaletteModel.qml` (outside the plan's lane) does not copy the
  command id into its tool nodes; a one-line `id: items[i].id` there completes them.
- **Verify:** `code-verify.py --check` on the four files; `qmllint` if available; read-back.
- **Deps:** none
- [x] done

### T14 — Identifier sweep: window roots (**delegable**)

- **Files:** `app/qml/MainWindow/MainWindow.qml` (`main`),
  `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml` (`external.<widgetIndex>`),
  every `SmartWindow`, `SmartDialog` and raw `Window` root instance under `app/qml/Dialogs/`,
  `app/qml/ProjectEditor/ProjectEditor.qml`, `app/qml/AI/AssistantPanel.qml`,
  `app/qml/DatabaseExplorer/` (about 50 files)
- **Does:** One `objectName` line per window root, the file's camelCase name; the external
  window folds in its `widgetIndex`. Brief: the list of files from a grep for the three root
  types, the naming rule, no other edits.
- **Verify:** `code-verify.py --check` on the touched files; a grep that every window root
  has an `objectName`; the main session spot-checks three files.
- **Deps:** none
- [x] done

### T15 — Identifier sweep: main-window chrome, console, tiles

- **Files:** `app/qml/MainWindow/Panes/Toolbar.qml`, `app/qml/MainWindow/Panes/Console.qml`,
  `app/qml/Widgets/Dashboard/Terminal.qml`, `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`,
  `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`,
  `app/qml/Widgets/Dashboard/WidgetTitleBar.qml`, `app/qml/Widgets/Dashboard/WidgetToolbar.qml`
- **Does:** Connect button `io.toggleConnection` and the remote-dashboard button
  `mirror.manage`; `console.send`, `console.sendButton` and the console toolbar controls;
  `taskbar.window.<windowId>`; tile chrome `dashboard.tile.<widgetIndex>.<control>` (title
  bar buttons and toolbar buttons). Split into two diffs if it exceeds three files per review.
  Landed: `io.toggleConnection`, `app.remoteAttach` (the remote-dashboard button's real
  command id), 27 `console.*` controls, `taskbar.start/search/searchIcon/mqttSettings`,
  `taskbar.pin.<name>`, `taskbar.window.<windowId>`, the tile root `dashboard.tile.<widgetIndex>`
  (role `tile` via the `WidgetDelegate` component) and its six context-menu entries. Not landed:
  the tile title-bar buttons (menu, minimize, maximize, close) are hand-built MouseAreas inside
  `app/qml/Widgets/MiniWindow.qml`, outside the plan's lane and without a widget identity; the
  proposed follow-up is a `property string automationId` on MiniWindow that the delegate sets to
  its tile id, with the four buttons named `automationId + ".close"` and so on.
- **Verify:** `code-verify.py --check`; read-back; T20's AC2 and AC3 exercise the ids.
- **Deps:** none
- [x] done

### T16 — Identifier sweep: connection setup panels (**delegable**)

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`, `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Drivers/*.qml` (14 driver panels plus their dialogs)
- **Does:** `setup.<driver>.<setting>` on every interactive control, the setting name taken
  from the bound driver property (so `setup.uart.baudRate` matches the `io.uart.setBaudRate`
  command Part C will dispatch). Brief: the naming rule, the list of panels, one objectName per
  control, no other edits.
- **Verify:** `code-verify.py --check` on the touched files; the `qml-control-unnamed` rule
  (T18) reports zero in this directory; the main session spot-checks UART and Network.
- **Deps:** none
- [x] done

### T17 — Identifier sweep: Preferences pages (**delegable**)

- **Files:** `app/qml/Dialogs/Settings/Settings.qml`, `app/qml/Dialogs/Settings/*.qml`
- **Does:** Window `settings`; `settings.<page>.<setting>` on every control, including
  `settings.general.language`, which AC2 drives.
- **Verify:** `code-verify.py --check`; zero `qml-control-unnamed` findings in the directory
  after T18.
- **Deps:** none
- [x] done

### T18 — Linter rules

- **Files:** `scripts/code-verify.py`, `scripts/tests/` (rule cases)
- **Does:** `qml-objectname-literal` (error everywhere): the expression bound to `objectName`
  is a string literal, a `node.id` / `modelData.id` / `entry.id` form, or a concatenation of
  literals with identity properties; any `qsTr(`, theme reference or `.title` / `.text` /
  `.label` read is an error. `qml-control-unnamed`: an instance of a shared interactive widget
  (`PushButton`, `IconButton`, `ToolbarButton`, `Toggle`, `Radio`, `Combo`, `LineField`,
  `BoundField`, `SearchField`, `SpinBox`, `SegmentedControl`, `MenuButton`, `BigButton`,
  `TaskbarButton`) or a raw control (`Button`, `CheckBox`, `Switch`, `TextField`, `ComboBox`,
  `SpinBox`, `Slider`) with no `objectName:` in its body; error under
  `app/qml/Widgets/Command*.qml`, `app/qml/MainWindow/**`, `app/qml/Dialogs/Settings/**`,
  advisory elsewhere. Rule cases in the scripts test folder for both. Landed: the advisory
  half is its own kind, `qml-control-unnamed-advisory`, because severity is per kind in the
  linter; `objectName` expressions spanning lines are read as one; string literals are
  blanked before the forbidden-property check. Turning the rule on surfaced twelve more
  in-scope controls (dashboard search, action bar, layout ratio picker, alarm bell, five
  start-menu submenus, palette search, close and back), named in the same pass.
- **Verify:** `pytest scripts/tests/ -q`; `python scripts/code-verify.py --check app/qml`
  shows zero errors from the two rules once T13 to T17 are in.
- **Deps:** T13, T14, T15, T16, T17
- [x] done

### T19 — Test harness plumbing

- **Files:** `tests/pytest.ini`, `tests/README.md`, `.github/workflows/test-suite.yml`
- **Does:** Registers the `gui` marker (strict markers are on); README rows for the two new
  test files and a note that the `gui` tier runs on a desk with the GUI app up, never headless;
  the CI marker expression excludes `gui`.
- **Verify:** `pytest tests/ --collect-only -q -m gui` collects nothing before T20 and the new
  file after; read-back of the workflow expression.
- **Deps:** none
- [x] done

### T20 — Integration tests (maintainer launches the GUI app)

- **Files:** `tests/integration/test_ui_automation.py`
- **Does:** Marker `gui`; a module fixture skips when `listWindows` is empty and restores the
  reduce-motion value it changes. Cases: AC1 (the main window is listed; opening a mini window
  through the start menu lists it; `focusedId` matches the last clicked control), AC2
  (toolbar ids by command id and `io.toggleConnection` present; the same ids after switching
  `settings.general.language` and back through click and key), AC3 (configure the network
  simulator, click Connect by id, connection reads back true; type into `console.send` and read
  it back in the catalog; clicking a disabled control errors), AC4 (PNG decodes, size equals
  geometry times scale, a crop equals the element rect). Uses `tests/utils/api_client.py` and
  the `device_simulator` fixture.
- **Verify:** `pytest tests/integration/test_ui_automation.py -m gui -v` with the GUI app up
  and the API server on.
- **Deps:** T10, T12, T13, T14, T15, T17, T19
- [x] done

### T21 — Security test

- **Files:** `tests/security/test_ui_automation_consent.py`
- **Does:** Under the existing `SS_TEST_AUTO_CONSENT` convention: without auto-consent, a
  remote `click` is refused with the consent-required code and the maintainer sees one prompt
  (docstring says so); with auto-consent it runs. Both skip when `listWindows` is empty.
- **Verify:** `pytest tests/security/test_ui_automation_consent.py -v` in each launch mode.
- **Deps:** T10, T19
- [x] done

### T22 — Manual and assistant docs

- **Files:** `doc/help/API-Reference.md`, `doc/help/AI-Assistant.md`
- **Does:** New "UI Automation Commands (6)" section in the reference's per-command shape,
  the identifier convention and roles stated once, the native-dialog caveat under `click`; the
  total and per-module counts; the third prompt in the consent table and the class table.
  `AI-Assistant.md` adds `click` and `key` to the device-gated list and names `screenshot` as
  confirm-tier.
- **Verify:** `python scripts/documentation-verify.py doc/help/API-Reference.md doc/help/AI-Assistant.md`;
  `term-verify.py`; the static assistant test's reference spot-check still passes.
- **Deps:** T10
- [x] done

### T23 — Architecture sub-doc and CLAUDE.md row

- **Files:** `doc/claude/architecture/automation.md` (new), `doc/claude/architecture.md`,
  `CLAUDE.md`
- **Does:** The surface's contract: the identifier convention, the role table, the gating
  story (third gate, `DrivesUi`, tiers, sealed), the headless behavior, and the Qt
  implementation names to re-verify per Qt bump (`_QMLTYPE_`, the overlay popup item class,
  the control class names) beside the existing `_qt_sg_hdr_format` note; an index row in
  `architecture.md`; one Subsystem Contracts row in `CLAUDE.md` pointing at it.
- **Verify:** `python scripts/claim-verify.py` resolves every path and symbol the new doc
  names; `documentation-verify.py` on the new file.
- **Deps:** T10
- [x] done

### T24 — Verification sweep and handoff

- **Files:** none new
- **Does:** `code-verify.py --check` on every touched file; `syntax-check.py` on every C++
  file; `registry-verify.py`, `term-verify.py`, `claim-verify.py`, `layer-verify.py`,
  `code-verify.py --singleton-census --check` and `--tu-census --check`; `qt-cpp-review` on
  the C++ diff and `qt-qml-review` on the QML diff; `sanitize-commit.py` (sanitize only; the
  commit waits for explicit permission). Self-review of the diff against the plan's file list;
  anything outside it is named in chat, not committed.
- **Verify:** every tool clean or its findings listed in the handoff message.
  Review outcome: twelve read-only agents (six C++ lenses, six QML lenses) over the diff.
  Fixed from their findings: mouse delivery now goes through the exported `qt_handleMouseEvent`
  so points are converted to native pixels on scaled displays; password-mode fields report no
  text or text-derived label through the catalog; click refuses non-finite or non-numeric
  coordinates and wrong-typed `button`, `modifiers` and `double`; `setReduceMotion` names a
  missing backend instead of a missing window; the auth ctest unbinds its prompter on every
  exit path; modifier phases are a named enum; the float-format check is a table; a palette
  entry without an id gets no name. Documented rather than changed: a click whose handler opens
  a modal dialog parks the command in a nested loop and nested dispatches skip the checkpoint.
- **Deps:** all
- [x] done

## Definition of Done

- [ ] AC1 to AC6, AC10, AC11 and AC16 in `spec.md` are met and checked off there; the Part E,
  C and D criteria stay open for their own parts.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors),
  including the two new rules over the swept directories.
- [x] `qt-cpp-review` run on the C++ diff and `qt-qml-review` on the QML diff; findings
  addressed or noted.
- [x] Hotpath untouched; `--benchmark-hotpath` not run because no hotpath file changed.
- [ ] (maintainer build point; no compile database existed, so every syntax check was a SKIP)
  ctests green against the maintainer's build: `tst_ui_automation`, `tst_server_auth`,
  the three registry sweeps, `tst_assistant_batch_tiers`.
- [x] pytest targets identified for the maintainer: the `gui` integration file, the security
  file in both launch modes, the sealed-session file, the static tier.
- [x] `python scripts/sanitize-commit.py` run; generated surfaces regenerated, not hand-edited.
- [x] Diff is *what was asked, and only that*: every file is in the plan's Part A table, with
  four named exceptions recorded in T13, T15, T18 and T19: the twelve in-scope controls the
  new rule surfaced (dashboard panes, start menu, palette), the `gui` exclusion placed in
  `test-suite.yml`'s shared pytest line because the per-job marker strings live in `ci.yml`,
  and two follow-ups left untouched outside the lane (`PaletteModel.qml` tool ids,
  `MiniWindow.qml` title-bar buttons).
- [x] `spec.md` status set to `in-progress` at the start of implementation; it moves to `done`
  only after Part D.
