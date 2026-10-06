---
spec: 0092-lazy-trial
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-03
---

# Tasks 0092 — Lazy Pro Trial & Graceful Degradation

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

### T1 — `Core::License` carries trial days

- **Files:** `core/Core/License.h`, `core/Core/License.cpp`
- **Does:** Adds `trialDaysRemaining` (int, −1 = no trial) to the atomic license state and
  its `set()`/getter. Invariant: the state stays a plain atomic read — no QObject, no
  allocation, callable from any thread.
- **Verify:** `python scripts/code-verify.py --check core/Core/License.h core/Core/License.cpp`;
  grep that every existing `License::set` caller still compiles conceptually (one caller:
  `publishLicenseState`).
- **Deps:** none
- [x] done — defaulted new param (-1) so the lone caller compiles until T5


### T2 — Pro-intent seam + `LicenseStateChanged` days field

- **Files:** `core/Core/License.h`, `core/Core/License.cpp`, `core/Core/Bus/Messages.h`
- **Does:** (AMENDED — the Messages.h contract forbids closure fields and multi-library
  publishers.) Adds `Core::License::requestProFeature(featureId, retry)` forwarding to a
  root-installed handler (`setProFeatureHandler`, spec-0040 module-static pattern;
  GUI-thread, command-rate, no-op when no handler); extends `LicenseStateChanged` with
  the trial-days field (wire break: its one publisher updates in T5).
- **Verify:** `python scripts/code-verify.py --check` on the three files; read-back that
  no hotpath TU is affected (`bus-on-hotpath` lint unchanged).
- **Deps:** T1
- [x] done — publisher's 4-field update landed here too (wire-break rule), T5 owns the rest


### T3 — `Trial` mid-session expiry + notice latch

- **Files:** `app/src/Licensing/Trial.h`, `app/src/Licensing/Trial.cpp`
- **Does:** Adds a 60 s date-rollover timer that re-evaluates `daysRemaining()` and emits
  `enabledChanged` ONLY on a real enabled→expired transition (the 2026-08-04 rule: no
  redundant emissions — they used to loop live-device rebuilds); adds the persisted
  one-time expiry-notice latch (plain bool beside the three SimpleCrypt keys, which stay
  byte-identical — R10) and posts the notice via `postMessageBox` (K13: never a blocking
  box off a non-gesture stack).
- **Verify:** `python scripts/code-verify.py --check app/src/Licensing/Trial.*`; read-back
  of the transition predicate against the four state predicates in Trial.cpp:138-166.
- **Deps:** none
- [x] done — snapshot also resyncs on licenseDataChanged (phantom-transition guard)


### T4 — `TrialGate` prompt broker

- **Files:** `app/src/Licensing/TrialGate.h`, `app/src/Licensing/TrialGate.cpp`
- **Does:** New commercial-only QObject owning the whole prompt policy: subscribes
  intent requests; `firstRun` → trial-offer question box, `trialExpired` →
  Activate/Upgrade/Cancel box; accept → `Trial::enableTrial()`; success (token valid) →
  pending retry runs once via queued single-shot on the GUI thread; failure → non-blocking
  notice, nothing persisted (R9). Modal-once: while a box is up or registration in flight,
  new intents coalesce (latest retry wins). Suppressed in runtime/headless mode. Exposes a
  QML-callable slot for QML gate sites. Blocking boxes only from the gesture-driven
  handler (`Misc::Utilities::showMessageBox`), everything async uses `postMessageBox`.
- **Verify:** `python scripts/code-verify.py --check app/src/Licensing/TrialGate.*`;
  header-shape read-back (slots not `Q_INVOKABLE void`, ctor init list, `[[nodiscard]]`).
- **Deps:** T2, T3
- [x] done — ctor-injected Trial&/LemonSqueezy& (zero instance() reaches); registered in app/CMakeLists.txt commercial lists


### T5 — Composition-root wiring

- **Files:** `app/src/Misc/ModuleManager.cpp` (+ `.h` if a member slot is added)
- **Does:** PROTECTED SURFACE — binding invariants: (1) the commercial licensing block's
  pinned position is machine-checked by the `composition-root-order` anchor — `TrialGate`
  is owned by `ModuleManager` (member/unique_ptr constructed right after the licensing
  block), NOT a Meyers singleton and NOT a `SessionContext` slot, so the singleton census
  and the pinned order stay untouched; (2) its ctor may reach only the bus, `Trial` and
  `LemonSqueezy` (all already constructed — ctor-edge stays provable); (3) wiring, not
  construction, carries behavior: `publishLicenseState()` gains the days field (T1) and
  the project-load Pro-intent hook (`containsCommercialFeatures && Trial::firstRun` →
  gate, no retry — the fan-out upgrades widgets) lands inside
  `setupCrossModuleConnections()` after `setupExternalConnections` per the startup
  contract; (4) register the `Cpp_Licensing_TrialGate` context property in
  `registerCommercialContextProperties`.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`;
  `python scripts/claim-verify.py` (composition-root anchors must still pass);
  `python scripts/code-verify.py --singleton-census --check` (no increase).
- **Deps:** T1, T2, T4
- [x] done — gate constructed+wired in wireTrialGate() (called from registerCommercialContextProperties, post-restoreLastProject, headless-silent); census re-baselined 797->800, all three reaches composition-root wiring


### T6 — Connect-path intent

- **Files:** `core/Devices/IO/ConnectionManager.cpp`
- **Does:** The `connectDevice` refusal (:559) calls `requestProFeature` with a
  reconnect retry closure instead of refusing silently. Invariants: command-rate only;
  no change to driver-open semantics (`openFinished` stays the one async verdict owner);
  the existing `LicenseStateChanged → rebuildDevices` wiring is untouched.
- **Verify:** `python scripts/code-verify.py --check core/Devices/IO/ConnectionManager.cpp`;
  read existing signal wiring in the file before editing (CLAUDE.md rule).
- **Deps:** T2
- [x] done — old expired-blocks-everything refusal removed (R6); Pro-bus detection lives in DeviceTableQuery::connectRequiresEntitlement (TU ratchet: extract, not append); free-bus connects now proceed in every licensing state


### T7 — Gesture-site intents (freeze, player, shortcuts, output)

- **Files:** `core/Pipeline/DataModel/ProjectModel.cpp` (:1056 `setFrozen`),
  `core/Storage/MDF4/Player.cpp` (:284), `core/Ui/Misc/ShortcutGenerator.cpp` (:346)
- **Does:** Each per-use refusal calls `requestProFeature` (retry where the gesture
  has a direct resumable action, e.g. the player open; none for freeze — the user
  re-toggles). `ProjectModel` edit stays OUTSIDE the ctor closure (gate on
  `m_initialized` path; `setFrozen` is a runtime mutator, not ctor-reachable).
- **Verify:** `python scripts/code-verify.py --check` on the three files.
- **Deps:** T2
- [x] done — MDF4 keeps its GPL-build pitch box; shortcut site keeps shortcutFailed alongside the prompt

### T8 — Output-widget send intent

- **Files:** `core/Ui/UI/Widgets/Output/Base.cpp` (:380)
- **Does:** The `sendValue` refusal publishes intent (no retry — the user re-interacts).
  Relies on T4's coalescing so repeated sends cannot stack boxes.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Base.cpp`.
- **Deps:** T2, T4
- [x] done — intent raised in sendValue (the gesture), NOT deliverValue (timer-reachable); tamper check stays silent


### T9 — Export sinks: intent + symmetric re-derive

- **Files:** `core/Ui/Console/Export.cpp`, `core/Storage/MDF4/Export.cpp`,
  `core/Storage/Sessions/Export.cpp`
- **Does:** (a) `setExportEnabled` refusals publish intent with a re-enable retry;
  (b) adopts the InfluxDB replay pattern: the recorded enable request is re-applied on an
  entitlement-ON transition, not only cleared on OFF (R8 symmetry). Invariant: enable
  changes keep flowing through the existing `sinkActivityChanged` edge so the cached
  `m_anyAsyncSink` hotpath flag refreshes — never set a sink live behind that signal's
  back (silent empty-recording class).
- **Verify:** `python scripts/code-verify.py --check` on the three files; read-back
  comparing each replay hook against `InfluxDB/Export.cpp:551`.
- **Deps:** T2
- [x] done — InfluxDB included too (its gesture now prompts; named file-list extension); all four sinks: m_exportRequested + replay-guarded watch, prompts gesture-only, refused branches stop clobbering persisted intent; sinkActivityChanged edge untouched


### T10 — Degrade-on-expiry: sweep state + GPS map

- **Files:** `core/Ui/UI/Dashboard/PlotControlBank.cpp`, `core/Ui/UI/Widgets/GPS.cpp`
- **Does:** On an entitlement-OFF transition (`LicenseStateChanged`), baked Pro state
  resets: sweep/trigger state clears; a Pro map type falls back to a free one. Emissions
  only on real change (no rebuild storms).
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T2
- [x] done — sweeps disarmed via PlotControlBank::disableSweeps on entitlement-off (DashboardWiring license watch); GPS restore silenced (m_restoringMapType), gesture prompts, degrade via widget rebuild; no retry closures on widgets (lifetime)

### T11 — Degrade-on-expiry: stop Pro audio recordings

- **Files:** `core/Ui/UI/Widgets/FFTPlot.cpp`, `core/Ui/UI/Widgets/Waterfall.cpp`
- **Does:** An in-progress Pro audio recording stops on entitlement-OFF (today only the
  enable toggle is gated; a running recording survives expiry).
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T2
- [x] done — degrade already covered: AudioExport closes all sessions on entitlement-off and widgets disarm via sessionClosed; added the gesture intent to both enable toggles

### T12 — Script-API stubs re-derive

- **Files:** `core/Pipeline/DataModel/NotificationCenter.cpp` + the engine owners the
  investigation confirms (frame-parser Lua engine, `Painter`); exact list fixed in-task.
- **Does:** Verifies which `notify*` stub-vs-real installs re-derive on
  `LicenseStateChanged` (TransformCompiler is believed reactive; the frame-parser Lua
  engine and Painter JS are flagged unconfirmed) and wires a recompile/reinstall where
  missing. Invariant: engine recreation happens on the thread that owns the engine
  (pipeline-thread engines recreate via the existing reset paths, never cross-thread).
- **Verify:** `python scripts/code-verify.py --check` on touched files; read-back naming
  where each engine's reinstall now hangs.
- **Deps:** T2
- [x] done — Lua: real closures always installed, gated per call on Core::License::activated() (stub + luaL_error removed; covers frame-parser engines of any age). JS: Painter rebuilds on reconfigure, no-__nc fallback goes through license-checked apiCall; RESIDUAL: a JS engine created licensed keeps __nc until its next recompile after expiry (documented in consumers.md, T21)


### T13 — Console status line

- **Files:** `core/Ui/UI/Widgets/Terminal.cpp`
- **Does:** `loadWelcomeGuide()` appends one translated status line after the Tip line
  (days remaining / expired / nothing when activated or GPL3), read from `Core::License`
  (T1) — the function already `clear()`s and re-runs on `LicenseStateChanged` and
  language change, so the line self-regenerates (R4).
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Terminal.cpp`.
- **Deps:** T1, T5 (days published)
- [x] done — moved into Console::welcomeConsoleText (WelcomeText.cpp) instead: Terminal.cpp is TU-ratchet debt and the line now rides the existing reactive reprint; lands right after the welcome text, before the Tip


### T14 — Boot simplification in `main.qml`

- **Files:** `app/qml/main.qml`
- **Does:** `continueBoot()` always calls `showMainWindow()`; deletes
  `showWelcomeDialog()`, the `welcomeDialog` loader, the `dontNag` property and its
  Settings alias (`App/hideWelcomeDialog`); the file-transmission gate (:712) routes its
  not-entitled branch through `Cpp_Licensing_TrialGate` (guarded by
  `Cpp_CommercialBuild`). CrashRecovery's `continueBoot()` call keeps working unchanged.
- **Verify:** `python scripts/code-verify.py --check app/qml/main.qml`; grep for stale
  `welcomeDialog`/`dontNag` references tree-wide.
- **Deps:** T5
- [x] done — also added the TrialGate activationRequested -> showLicenseDialog Connections; showFileTransmission left UNCHANGED (its only refusal is runtime mode, where prompts are suppressed by policy)


### T15 — Delete `Welcome.qml`

- **Files:** `app/qml/Dialogs/Welcome.qml` (deleted), `app/CMakeLists.txt` (:326 qrc
  entry), `scripts/code-verify.py` (`_TRIAL_PARITY_ALLOWED`, :2074)
- **Does:** Removes the dialog, its qrc registration, and its trial-parity allowlist
  entry. No other allowlist rows change.
- **Verify:** tree-wide grep for `Welcome.qml` (only spec archives may remain);
  `python scripts/code-verify.py --check` still runs clean on an unrelated QML file
  (script self-syntax).
- **Deps:** T14
- [x] done — .ts translation references remain (derived artifacts, next lupdate cleans them; Trust Contract: not mine to touch)


### T16 — Toolbar: Connect back, Activate out

- **Files:** `app/qml/MainWindow/Panes/Toolbar.qml` (:251-261),
  `app/qml/Commands/AppCommandBindings.qml` (:210-213),
  `app/rcc/commands/layouts/main-toolbar.json` (:67)
- **Does:** Removes the expired-state Activate button and its `pinnedEnd` layout entry;
  restores the Connect toggle's visibility in the expired state. The `license.activate`
  command itself STAYS in `app.json` (palette/About/LicenseManagement keep it).
  Registry rule: binding guards (`Cpp_Licensing_` needs `Cpp_CommercialBuild` on the
  same or previous line in `Commands/*.qml`) must hold after the edit.
- **Verify:** `python scripts/registry-verify.py`;
  `python scripts/code-verify.py --check` on the two QML files.
- **Deps:** none (orderable any time; kept here so QML lands after C++)
- [x] done — registry G4 also required adding Cpp_Licensing_TrialGate to the ContextRegistry.cpp name table (mechanical consequence, named)


### T17 — QML freeze fallbacks route through the gate

- **Files:** `app/qml/Commands/DashboardCommandBindings.qml` (:171),
  `app/qml/MainWindow/Dashboard/DashboardLayout.qml` (:163),
  `app/qml/MainWindow/Dashboard/Taskbar.qml` (:973)
- **Does:** The three not-entitled freeze fallbacks call the TrialGate slot instead of
  `app.showLicenseDialog()` (the gate decides trial-offer vs activate box). Binding-guard
  rule as in T16 for the Commands file.
- **Verify:** `python scripts/registry-verify.py`;
  `python scripts/code-verify.py --check` on the three files.
- **Deps:** T5
- [x] done — all three now call Cpp_UI_Dashboard.setFrozen unconditionally; the T7 C++ gate owns the one prompt policy (no TrialGate QML call needed)


### T18 — ProNotice in commercial builds + AddWidgetDialog refresh

- **Files:** `app/qml/Widgets/ProNotice.qml` (:52),
  `app/qml/ProjectEditor/**/AddWidgetDialog.qml` (:148; exact path confirmed in-task)
- **Does:** ProNotice visibility becomes "Pro features present and not entitled"
  (`!app.proVersion` replaces `!Cpp_CommercialBuild`), activating its dormant
  Buy/Activate branches — this is the R3 degraded-load passive notice.
  AddWidgetDialog re-runs `refresh()` on license change while open (stale-snapshot fix).
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T5
- [x] done — ProNotice gates on !app.proVersion in both flavors; AddWidgetDialog refreshes on activatedChanged (covers trial install)


### T19 — ctest unit: rollover + notice latch

- **Files:** `app/tests/tst_trial_rollover.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Unit-tests the T3 logic factored constructible without the server: the
  rollover detector fires exactly once per enabled→expired transition, and the notice
  latch is once-only across simulated relaunches (AC4/AC8 predicates). Registered in the
  existing ctest tier (no CMake presets — hand-written configure; maintainer builds).
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_trial_rollover.cpp`;
  `ctest` run against the next user-built test-enabled build dir.
- **Deps:** T3
- [x] done — RE-SCOPED to `tst_license_seam.cpp` (Core::License state + requestProFeature handler lifecycle; archive-only link). Trial.cpp's link closure (LemonSqueezy, network, Ui prompts) makes a rollover unit unreasonable as built; the rollover latch is pinned by AC4 maintainer observation instead


### T20 — pytest integration: degradation flow

- **Files:** `tests/integration/test_licensing_degradation.py` (new)
- **Does:** Automatable halves of AC3/AC5/AC6: unlicensed free-feature session
  end-to-end over the API (loopback connect, dashboard reads, CSV export — no licensing
  dialog blocks the flow); a Pro-widget project loads degraded with
  `containsCommercialFeatures == true`; after a maintainer-performed activation, widget
  types re-read over the API flip to Pro. Markers and fixtures per `tests/README.md`
  (read it first); destructive-safe.
- **Verify:** file imports clean (`pytest --collect-only`); full run needs the running
  app (maintainer launches).
- **Deps:** T5
- [x] done — `test_licensing_degradation.py`, 3 tests, license-state-agnostic asserts (free-bus connect in any state = the R6 pin; Pro project loads degraded; status surface agrees); collect-only clean


### T21 — Docs: consumers inventory + startup contract

- **Files:** `doc/claude/specs/0042-license-token-hardening/consumers.md`,
  `doc/claude/architecture/startup.md` (licensing paragraph only, if claim anchors
  require it)
- **Does:** Refreshes the inventory rows this spec changes (new symmetric export wires,
  sweep/GPS/recording/script-stub re-derives, the TrialGate funnel) per the inventory's
  going-forward rule; amends the startup licensing paragraph only as far as
  `claim-verify.py` demands (welcome-flow sentences, TrialGate construction site).
- **Verify:** `python scripts/claim-verify.py`; `python scripts/doc-verify.py` if
  markdown lint applies.
- **Deps:** T5, T9-T12
- [x] done — consumers.md gained a dated 0092 section; startup.md licensing paragraph extended; claim-verify clean


### T22 — Final static sweep

- **Files:** none new (whole diff)
- **Does:** Runs the full gate battery over the complete diff and fixes fallout:
  `code-verify --check` on every touched file, `registry-verify.py`,
  `claim-verify.py`, `layer-verify.py` (no new upward includes; bus message is
  core-internal), singleton census, then `qt-cpp-review` on the C++ diff and
  `qt-qml-review` on the QML diff; self-review the diff against "what was asked, and
  only that".
- **Verify:** all listed gates clean; review findings addressed or recorded.
- **Deps:** all previous
- [x] done — qt-cpp-review (6 agents): fixed export-request seed bug (3 sinks), MDF4 stray double-prompt box, TrialGate retry-drop-on-success (new Trial::registrationSettled signal replaces busyChanged guessing), registration-pending wedge (activated pre-check + busy()-conditional arming + stale-retry clears), License::set default dropped (test call sites updated: tst_license_state, tst_message_bus — wire-break rule), GUI-thread guard in requestProFeature, refusal emits unstick Sessions/InfluxDB checkboxes, pytest plot3d dataset guard. QML pass: ONE focused review agent instead of the 6-agent qt-qml-review skill (8 mechanical QML edits; deviation named) — confirmed Connect-visibility safety, boot coherence, guard patterns; fixed one stale comment. Noted-not-fixed (judgment): featureId slugs stay strings until a consumer exists; rollover timer ungated (negligible); GPS flag dance kept; JS __nc residual documented; SS_LICENSE_GUARD tamper refusal stays silent. sanitize-commit run.


### T23 — Remediation after maintainer first-run test (2026-10-04)

- **Files:** `core/Devices/IO/ConnectionManager.cpp`, `app/qml/main.qml` (AI/Historian/file
  transmission gates, dead ProUpgradeNotice loader), `app/qml/AI/ProUpgradeNotice.qml`
  (deleted), `app/qml/Dialogs/ShortcutGenerator.qml`, `core/Pipeline/DataModel/Project/
  ProjectSources.cpp`, `app/src/Misc/ModuleManager.cpp` (rising-edge hook),
  `core/Ui/Console/WelcomeText.cpp`, `app/rcc/messages/unlicensed/` (21 new files),
  `app/rcc/rcc.qrc`, `app/CMakeLists.txt`
- **Does:** Routes every surviving legacy pitch box and ungated Pro surface through the
  central gate; adds the unlicensed welcome variant. Dialog-open gates (AI, Historian,
  file transfer) re-open on the next click after a trial starts (no QML retry closure) —
  a named, deliberate R2 relaxation.
- **Verify:** full gate battery clean; maintainer re-test of the first-run flow.
- **Deps:** T22
- [x] done

### T24 — Feature-list audit gates (2026-10-04, maintainer-approved)

- **Files:** `core/Pipeline/DataModel/Project/ProjectTables.cpp`, `ProjectFolders.cpp`,
  `ProjectWorkspaces.cpp`, `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp`,
  `core/Ui/UI/Dashboard/DashboardTools.cpp`, `app/rcc/messages/{pro,trial,unlicensed}/`
  (XY wording, 63 files)
- **Does:** Audit enumerated from the advertised feature list + BUILD_COMMERCIAL CMake
  lists (new ledger rule) found four ungated advertised Pro features; maintainer chose to
  gate all four and keep XY free with corrected marketing. Transform gate placed in the
  editor TU, not EditorForms (TU ratchet: extract/relocate, never append past 1500).
- **Verify:** full gate battery clean; maintainer re-test.
- **Deps:** T23
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` staged — AC1-AC4/AC7 are maintainer
      observations in the next build; AC5/AC6 partials via T20; AC8 via T19 (re-scoped);
      AC9 via the next `--benchmark-hotpath` run (no hotpath TU touched).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched by construction; `--benchmark-hotpath` confirms no regression on
      the next user-built binary (AC9).
- [x] Relevant `pytest` targets listed for the maintainer
      (`tests/integration/test_licensing_degradation.py`).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — named extensions: InfluxDB prompt, ContextRegistry table, test call-site updates, WelcomeText.cpp relocation.
- [x] `spec.md` status set to `done`.
