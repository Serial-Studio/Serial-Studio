---
spec: 0092-lazy-trial
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-03
---

# Plan 0092 — Lazy Pro Trial & Graceful Degradation

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Delete the first-run wall (`Welcome.qml` and its boot branch) and move the trial-start
moment to the gate sites that already refuse Pro use. Core gate sites call a new
command-rate seam, `Core::License::requestProFeature(featureId, retry)`, at their
explicit-gesture refusal points; the seam forwards to a handler the composition root
installs — a new commercial-only `Licensing::TrialGate` (app/src) that owns the one
message box (trial offer when `Trial::firstRun`, activate/purchase when expired), drives
`Trial::enableTrial()`, and runs the optional retry closure once the token lands.
(AMENDED 2026-10-03, was a `ProFeatureRequested` bus message: the Messages.h reviewer
checklist forbids non-value-type fields and multi-library publishers, so the intent seam
follows the spec-0040 root-bound module-static pattern instead; GPL3/headless installs no
handler and keeps today's silent refusal.) Unlocking rides the existing spec-0042 machinery unmodified: trial/license
transitions already funnel through `ModuleManager::publishLicenseState()` →
`Core::Bus::LicenseStateChanged`, which already rebuilds devices, re-resolves widgets,
regenerates workspaces, and reprints the console banner. The remaining work is closing
the known one-way or baked re-derive gaps so expiry mid-session degrades as cleanly as
activation upgrades, plus a minute-rate date-rollover check in `Trial` so expiry can
actually *happen* mid-session. This shape was chosen over QML-side interception (misses
C++-originated intents: project load, MDF4 open) and over prompting from inside the token
check itself (sample sites run per-publish/per-message; prompts must come only from
explicit gestures).

## Affected subsystems & files

Grep-confirmed touch-points. `(new)` marks created files.

| File | Change |
|------|--------|
| `app/src/Licensing/TrialGate.{h,cpp}` (new) | Prompt broker: the root-installed `requestProFeature` handler; shows trial-offer / expired box, calls `enableTrial()`, runs retry on success, posts non-blocking failure notice (R2, R5, R9). Suppressed in runtime/headless mode. |
| `app/src/Licensing/Trial.{h,cpp}` | Minute-rate date-rollover check (emit `enabledChanged` on expiry transition); one-time expiry-notice latch persisted beside the trial keys (R5, R8, R10). |
| `app/src/Misc/ModuleManager.cpp` | Construct `TrialGate` in the commercial licensing block; extend `publishLicenseState()` with `trialDaysRemaining`; wire project-load Pro-intent (project with `containsCommercialFeatures` + `firstRun` → gate). |
| `core/Core/License.{h,cpp}` | Add `trialDaysRemaining` to the atomic state (−1 = none) so core-side UI can print the console line without reaching app/src; add the `requestProFeature(featureId, retry)` seam with its root-installed handler slot (GUI-thread, command-rate). |
| `core/Core/Bus/Messages.h` | Extend `LicenseStateChanged` with the trial-days field (wire break: its one publisher updates in the same change). |
| `core/Devices/IO/ConnectionManager.cpp` | `connectDevice` refusal (:559) raises intent with a reconnect retry; remove the silent-refuse-only path. |
| `core/Devices/IO/ConnectionManager/DriverFactory.cpp` | No gate change (stays per-use); confirm refusals surface through the connect-site intent, not here. |
| `core/Ui/UI/Widgets/Terminal.cpp` | `loadWelcomeGuide()` appends the one status line (days remaining / expired / none) after the Tip line, from `Core::License` state (R4). |
| `core/Ui/Console/Export.cpp`, `core/Storage/MDF4/Export.cpp`, `core/Storage/Sessions/Export.cpp` | Make the deactivation-only re-derive symmetric: adopt the InfluxDB replay pattern (re-apply the recorded enable request on activation) (R8). |
| `core/Ui/UI/Dashboard/PlotControlBank.cpp` | Clear baked sweep/trigger state on an entitlement-off transition (R8). |
| `core/Ui/UI/Widgets/GPS.cpp` | Reset Pro map type to a free one on entitlement-off (R8). |
| `core/Ui/UI/Widgets/FFTPlot.cpp`, `core/Ui/UI/Widgets/Waterfall.cpp` | Stop an in-progress Pro audio recording on entitlement-off (R8). |
| `core/Pipeline/DataModel/NotificationCenter.cpp` + Lua/JS engine owners | Verify the `notify*` stub-vs-real install re-derives on `LicenseStateChanged` (frame-parser Lua engine, Painter JS); wire a recompile where it does not (R8; flagged unconfirmed in the inventory). |
| `core/Pipeline/DataModel/ProjectModel.cpp` | `setFrozen` refusal publishes intent (freeze is a named trigger). |
| Pro-intent gesture sites: `core/Storage/MDF4/Player.cpp` (:284), `core/Ui/Misc/ShortcutGenerator.cpp` (:346), `core/Ui/UI/Widgets/Output/Base.cpp` (:380), export `setExportEnabled` setters above | Refusal points call `Core::License::requestProFeature` (R3). |
| `app/qml/main.qml` | `continueBoot()` always `showMainWindow()`; delete `showWelcomeDialog()`, `welcomeDialog` loader, `dontNag` property + Settings alias; file-transmission gate (:712) routes through the gate. |
| `app/qml/Dialogs/Welcome.qml` | Deleted. |
| `app/CMakeLists.txt` | Remove `Welcome.qml` qrc entry (:326). |
| `app/qml/MainWindow/Panes/Toolbar.qml` | Remove the Activate button (:251-261). |
| `app/qml/Commands/AppCommandBindings.qml` | Restore Connect visibility in the expired state (:210-213); keep `license.activate` bound for palette/menus. |
| `app/rcc/commands/layouts/main-toolbar.json` | Remove `license.activate` from `pinnedEnd` (:67); the command itself stays in `app.json`. |
| QML freeze fallbacks: `app/qml/Commands/DashboardCommandBindings.qml` (:171), `app/qml/MainWindow/Dashboard/DashboardLayout.qml` (:163), `app/qml/MainWindow/Dashboard/Taskbar.qml` (:973) | Route the not-entitled branch through the gate (QML-visible slot) instead of `showLicenseDialog()`. |
| `app/qml/Widgets/ProNotice.qml` | Enable the dormant commercial branch: visible when Pro features present and not entitled (`!app.proVersion`), serving as the degraded-load passive notice (R3 decline path). |
| `app/qml/ProjectEditor/.../AddWidgetDialog.qml` | Refresh the widget-summary snapshot on license change (stale-snapshot fix, R8). |
| `scripts/code-verify.py` | Drop `Welcome.qml` from `_TRIAL_PARITY_ALLOWED` (:2074). |
| `app/tests/` + `tests/integration/` | New checks per the test plan below. |
| `doc/claude/specs/0042-license-token-hardening/consumers.md` | Refresh the inventory rows this plan changes (the going-forward rule says new/changed consumers are recorded there). |

## Architecture & data flow

Intent path (new, command-rate, GUI thread): user gesture → gate site refusal →
`Core::License::requestProFeature(featureId, retry)` → root-installed handler →
`TrialGate` (GUI thread) shows
`Misc::Utilities` message box → on accept `Trial::enableTrial()` → server verdict →
`Trial::enabledChanged` → `ModuleManager::publishLicenseState()` →
`Core::License::set(...)` + `LicenseStateChanged` → existing fan-out (DriverFactory
rebuild, FrameBuilder → Dashboard widget re-resolution, ProjectModel workspaces, console
banner reprint, export replay) → `TrialGate` runs the pending retry closure (queued, GUI
thread) exactly once, then clears it. Decline or failure: closure dropped, nothing
persisted (R9); the next gesture publishes a fresh intent (R2).

Expired path: same subscriber; when `Trial::trialExpired`, the box offers
Activate (`showLicenseDialog` via the existing `license.activate` command) / Upgrade
(`LemonSqueezy::buy()`) / Cancel — no trial offer (R5).

Expiry detection: `Trial` gains a coarse (60 s) timer that watches for the cached
calendar-date rollover; on an enabled→expired transition it emits `enabledChanged`, which
re-publishes license state — the whole degrade fan-out (including the new symmetric
re-derives) runs mid-session. The one-time notice posts via `postMessageBox` (K13:
never a blocking box from a non-gesture stack).

Project-load trigger: wired in `ModuleManager` (composition root — it can see both
`ProjectModel` and the licensing block) after `setupExternalConnections`, reacting to
project-load with `containsCommercialFeatures && Trial::firstRun`. No retry closure
needed: on accept, the `LicenseStateChanged` fan-out upgrades the loaded widgets in
place (AC3/AC6).

Boot: `continueBoot()` goes straight to `showMainWindow()` in every build.
`Trial::readSettings()` is unchanged — on a fresh install `m_deviceRegistered` is false,
so no network request happens (R1); registered devices keep their silent re-check (R10).

## Hotpath & threading impact

- **Touches the hotpath?** No. Every new or changed site is command-rate (user gestures,
  project load, a 60 s timer). The one per-message gate (`MQTT.cpp` receive drop guard)
  and the per-publish `Publisher::licenseValid()` stay pure samples — they never publish
  intents and never prompt. The bus stays off the per-frame path (`bus-on-hotpath` lint
  unaffected); `requestProFeature` calls live only in refusal branches of
  command-rate GUI-thread functions.
- **New cross-thread signal/slot?** No new cross-thread edges. `TrialGate`,
  `Trial`, the message boxes, and all gate refusal sites named above run on the GUI
  thread. The retry closure is invoked via a queued single-shot on the GUI thread to
  stay out of the `enabledChanged` emission stack.
- **New input to a cached hotpath flag?** None. `Core::License` stays an atomic read;
  adding `trialDaysRemaining` adds no new consumer on the frame path. No change to
  `m_anyAsyncSink` wiring — the export replay pattern re-uses the existing
  `setExportEnabled` path, which already drives `sinkActivityChanged`.
- **Timestamp ownership** — untouched; no driver or export worker changes stamping.

## Data model & persistence

- No project-JSON change (deciding constraint: schema untouched). `commercialCfg()`
  detection is reused as-is.
- QSettings: `trial/` group gains the one-time expiry-notice latch (plain bool — UX
  state, not entitlement; the three SimpleCrypt values are unchanged, R10). The orphaned
  `App/hideWelcomeDialog` key is no longer read or written; stale copies are harmless.
- `Core::License` atomic state widens by one int (`trialDaysRemaining`); the
  `LicenseStateChanged` bus payload carries it so the banner line regenerates on every
  transition without polling.

## API / SDK surface

None. No new commands; `LicensingHandler` status queries are unchanged. `TrialGate`
suppresses prompts when `app.runtimeMode` / headless — API- or CLI-originated refusals
keep today's silent behavior, and `SERIAL_STUDIO_API_AUTO_CONSENT` semantics are
untouched (it consents to API gates, not licensing). All new C++ is inside the
commercial build (`app/src/Licensing/` is compiled only under `BUILD_COMMERCIAL`; the
core-side `requestProFeature` calls are no-ops when no handler is installed, which is also the GPL3
behavior).

## QML / UI

- `Welcome.qml` deleted; no replacement dialog. The two prompt shapes are native
  message boxes from `TrialGate` (convention: `Misc::Utilities::showMessageBox` from a
  gesture, `postMessageBox` otherwise), so no new QML component, no motion-contract
  surface, no registry entry.
- Toolbar: Activate button removed; Connect visible in all licensing states (R7).
  `license.activate` stays a registered command (palette, About, LicenseManagement all
  keep working); `registry-verify.py` passes because the layout reference is removed
  with it. Orphaned-binding cleanup (`Toolbar.qml` lookup) is manual — the script does
  not catch it.
- `ProNotice.qml` becomes the commercial degraded notice (`!app.proVersion` in place of
  `!Cpp_CommercialBuild`); its dormant Buy/Activate branches activate as designed.
- `AddWidgetDialog.qml` re-runs `refresh()` on license change while open.
- Console status line: appended in `Terminal::loadWelcomeGuide()` after the Tip line —
  regenerates automatically on `LicenseStateChanged` and language change. GPL3 build
  prints no line.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Intent channel | Root-installed handler on `Core::License` (spec-0040 static pattern); new bus message; QML-side prompts | **`Core::License` seam** — the bus message was rejected by the Messages.h contract itself (value-type fields only, one publisher per topic; gate sites span four libraries), and `Core::License` is already the sanctioned any-layer licensing surface. |
| "Proceeds immediately" (R2) | Retry closure carried in the intent; rely on reactive re-derive only; nested-event-loop blocking wait | **Closure, with re-derive as the floor** — connect/export gestures need an explicit resume; project load and widget upgrades come free from the fan-out. Nested loops rejected (reentrancy class, common-mistakes). |
| Mid-session expiry | 60 s date-rollover timer; check only on `daysRemaining()` reads; keep launch-only | **Timer** — R8 requires live expiry; read-triggered checks fire from paint-rate bindings (wrong place to emit from); 60 s is day-granularity state, cost is nil. |
| Console line data | Extend `Core::License` with days; QML-composed line via `Cpp_Licensing_Trial` | **Extend `Core::License`** — the banner is core-side C++ (`Terminal.cpp`) and must not reach app/src licensing; one int in the existing atomic state. |
| Welcome dialog fate | Delete; keep dormant behind a flag | **Delete** — dead UI invites drift; activation/purchase paths all survive via `LicenseManagement.qml` and `About.qml`. |
| Expired-state UX at gates | Reuse `TrialGate` box with Activate/Upgrade; silently route to `showLicenseDialog` | **Box with explicit choices** — R5 wants "tell, then offer"; a silent dialog jump surprises. |
| Decline memory | Ask again on next gesture; once per session; persistent opt-out | **Ask again per gesture** (spec R2 as approved) — a repeated explicit attempt is renewed intent; no new state. |

## Risks & mitigations

- **Baked-state consumer missed → stale gating after a mid-session transition** (the
  2026-07-09 class). Mitigation: the spec-0042 inventory is the checklist; this plan
  explicitly closes the six known gaps (sweep state, GPS map type, in-progress
  recordings, three one-way export flags, script-API stubs, AddWidgetDialog) and
  refreshes `consumers.md`; AC6 is the end-to-end probe.
- **Prompt from a non-gesture stack** (network reply, bus handler) → reentrancy /
  blocked event loop (macOS dialog-reentrancy class, common-mistakes; K13 note at
  Trial.cpp:297). Mitigation: blocking boxes only inside `TrialGate`'s gesture-driven
  handler; everything asynchronous (`onServerReply`, expiry notice) uses
  `postMessageBox`; retry closures run via queued single-shot.
- **Intent spam** — a gate hit in a loop (e.g. output widget send) could stack boxes.
  Mitigation: `TrialGate` is modal-once — while a prompt is up or a registration is in
  flight, further intents are coalesced (latest retry wins, no second box).
- **`enabledChanged` storms re-triggering live-device rebuilds** (pre-2026-08-04
  class). Mitigation: no new emission paths; the rollover timer emits only on a real
  enabled→expired transition, and `publishLicenseState` stays the single funnel.
- **Removing the welcome flow breaks trial-parity/registry/claim lint baselines.**
  Mitigation: `_TRIAL_PARITY_ALLOWED` edit travels in the same change; `claim-verify.py`
  and `registry-verify.py` run before handoff (Welcome.qml is referenced in
  doc-anchors only via spec archives, which are exempt).
- **Fresh-install regression: an unexpected licensing network call** would violate R1's
  promise. Mitigation: AC1 includes a network observation; `readSettings()` fetch stays
  conditional on `m_deviceRegistered`.

## Test & verification plan

- **Unit (ctest, maintainer builds; I run against the build dir):**
  - New `app/tests/tst_trial_rollover.cpp`: date-rollover detector transitions exactly
    once per expiry and the notice latch fires once (logic factored to be constructible
    without the server) (AC4's once-only, AC8 predicates).
  - Existing `tst_*` unaffected; no hotpath unit changes.
- **Integration (`pytest`, app up with API server):**
  - Extend `tests/integration/test_project_save.py` or a new
    `test_licensing_degradation.py`: free-feature session end-to-end while unlicensed
    (connect loopback, dashboard, CSV export — no licensing dialog blocks the API
    flow) (AC5); project with Pro widgets loads degraded and `containsCommercialFeatures`
    reads true over the API (AC3's automatable half).
  - AC6's automatable half: with the app activated mid-test (maintainer gesture),
    widget types re-read over the API flip from fallback to Pro.
- **Maintainer observation:** AC1 (fresh profile + network capture), AC2 (driver prompt
  → same-session proceed), AC3/AC6 (prompt on load, in-place upgrade), AC4 (console
  line, once-only expiry notice across two launches), AC7 (offline trial accept
  fails gracefully, later attempt gets full 14 days).
- **Hotpath:** `--benchmark-hotpath` unchanged gates (AC9) — run against the next
  user-built binary; no hotpath TU is touched, so a delta would itself be a finding.
- **Static:** `python scripts/code-verify.py --check` on every touched file (including
  the `_TRIAL_PARITY_ALLOWED` edit), `scripts/registry-verify.py`,
  `scripts/claim-verify.py`, `scripts/layer-verify.py` (new bus message is
  core-internal; no new upward includes); `qt-cpp-review` + `qt-qml-review` before
  handoff; `python scripts/sanitize-commit.py` before commit.
