# CommercialToken consumer inventory (R3 / AC3) — 2026-07-28

Generated from `grep -rn "CommercialToken::current()" app/src --include="*.cpp"` (licensing
module itself excluded). Classification: **sample** = token checked inside the operation on
every call (safe by construction — a later entitlement change is picked up on the next
call); **bakes** = token value derived into longer-lived state (must be wired to the
entitlement funnel `Licensing::LemonSqueezy::activatedChanged`, into which Trial and
OfflineLicense transitions are ctor-forwarded).

| TU : line(s) | Use | Class | Wire (if bakes) |
|---|---|---|---|
| `SerialStudio.cpp:49` | `SerialStudio::activated()` wrapper | sample | callers own their class; wrapper itself stateless |
| `Misc/Translator.cpp:234` | welcome-text variant per load | sample | — |
| `DataModel/NotificationCenter.cpp:462` | `isProTierActive()` per post | sample | — |
| `UI/Widgets/GPS.cpp:371` | map-type guard per set | sample | — |
| `UI/Widgets/Output/Base.cpp:147` | `sendValue` guard per send | sample | — |
| `MDF4/Player.cpp:271` | `openFile` guard per open | sample | — |
| `UI/Dashboard.cpp:1721,1753` | plot-sweep setter guards | sample | Dashboard's baked state (frozen) separately wired: `Dashboard.cpp:266` |
| `IO/ConnectionManager.cpp:741-742` | `connectDevice` guard per attempt (now also consults `trial.trialExpired()`) | sample | — |
| `IO/ConnectionManager.cpp:1780-1822` | `createDriver` commercial-bus gates | **bakes** (device existence) | `ConnectionManager.cpp:951` `activatedChanged -> rebuildDevices` |
| `IO/Drivers/MQTT.cpp:174-175,1049-1050` | open request (box now queued) / message drop guards | sample | — |
| `MDF4/Export.cpp:407,617` | export enable per operation | sample | — |
| `MDF4/Export.cpp:484` | re-derive on activation | **bakes** (export enable) | wired in place (lambda on `activatedChanged`) |
| `Console/Export.cpp:122,312` | per-operation guards | sample | — |
| `Console/Export.cpp:214` | re-derive on activation | **bakes** | wired in place |
| `Sessions/Export.cpp:785` | per-operation guard | sample | — |
| `Sessions/Export.cpp:608` | re-derive on activation | **bakes** | wired in place |
| `MQTT/Publisher.cpp:2106` | `licenseValid()` per publish path call | sample | — |
| `InfluxDB/Export.cpp:940` | `licenseValid()` per enable | **bakes** (sink enable) | wired in place; the hook REPLAYS the recorded request, it does not only disable — the sink's enable comes from the project, so a trial token installed after `restoreLastProject()` must still switch it on |
| `API/Handlers/LicensingHandler.cpp:265` | status query per call | sample | — |

Indirect consumers reaching the token through `SerialStudio::activated()` /
`commercialCfg()` that bake derived state carry their own wires (verified via
`grep -rn "activatedChanged" app/src`): `UI/Dashboard.cpp:266`,
`UI/Widgets/AudioExport.cpp:615`, `UI/Widgets/Terminal.cpp:145`,
`DataModel/ProjectModel.cpp:1446`, `DataModel/FrameBuilder.cpp:168`, `Misc/CLI.cpp:878,919`.

**Result: no unwired bakes-state consumer found (T5 = no-op).** New-consumer rule going
forward: sampling per operation needs nothing; deriving stored state from the token
requires a `LemonSqueezy::activatedChanged` connection, recorded here.

## Notes

- `AI/Assistant.cpp` and `AI/Conversation.cpp` gate on `SS_LICENSE_GUARD()` alone by design
  (build integrity, deliberately tier-free) and are excluded from the token-consumer table.
- Table refreshed 2026-08-04 after the connection-flow removal (38c9ef66) and the 0043
  reliability sweep.
- `LemonSqueezy::activatedChanged` now fires only on real CommercialToken-validity
  transitions via `notifyEntitlementMaybeChanged()` — consumers listed as re-deriving on
  `activatedChanged` no longer see redundant emissions.

## Spec 0092 refresh (2026-10-03) — lazy trial & graceful degradation

New funnel: gate sites raise Pro intent through `Core::License::requestProFeature(featureId,
retry)` (root-installed handler, `Licensing::TrialGate`, GUI builds only; no-op in GPL/headless
and under an active `API::RemoteDispatchScope`). Prompts fire from explicit gestures only; the
`LicenseStateChanged` fan-out does the unlocking, so retry closures exist only on session-long
objects (ConnectionManager, MDF4 Player), never widgets.

| Consumer | Change | Class now |
|---|---|---|
| `Devices/IO/ConnectionManager.cpp` connectDevice | Expired trial no longer blocks free buses (R6); Pro-bus connect raises intent with reconnect retry (`DeviceTableQuery::connectRequiresEntitlement`) | sample + intent |
| `Ui/Console/Export.cpp`, `Storage/MDF4/Export.cpp`, `Storage/Sessions/Export.cpp` | Adopted the InfluxDB replay: `m_exportRequested` + license watch re-applies BOTH directions under a `m_licenseReplay` guard; refused branches stop clobbering persisted intent; gesture refusals raise intent | bakes, wired both ways |
| `Storage/InfluxDB/Export.cpp` | Gesture refusal raises intent (`m_inApply`/`m_licenseReplay` keep config loads and replays silent) | unchanged wiring + intent |
| `Ui/UI/Dashboard/PlotControlBank.cpp` | `disableSweeps()` on entitlement-OFF via DashboardWiring license watch (saved-config restore can no longer resurrect an unentitled sweep) | bakes, wired |
| `Ui/UI/Widgets/GPS.cpp` | Restore path clamps silently (`m_restoringMapType`); gesture refusal raises intent; degrade via widget rebuild | sample |
| `Ui/UI/Widgets/FFTPlot.cpp` / `Waterfall.cpp` | Enable toggles raise intent; in-progress recordings were already closed by `AudioExport`'s license watch | sample |
| `Pipeline/DataModel/NotificationCenter.cpp` (Lua `notify*`) | Real closures always installed, gated per call on `Core::License::activated()` (stub + `luaL_error` removed) — engine age no longer matters | **sample** (was bakes) |
| JS `__nc` bridge | UNCHANGED, known residual: an engine created entitled keeps the live bridge until its next recompile after expiry; Painter rebuilds on reconfigure, the no-`__nc` fallback goes through the license-checked `apiCall` | bakes (documented) |
| `app/qml/ProjectEditor/Views/AddWidgetDialog.qml` | `refresh()` on `activatedChanged` while open (stale snapshot fixed) | wired |
| `Pipeline/DataModel/ProjectModel.cpp` setFrozen, `Storage/MDF4/Player.cpp` openFile, `Ui/Misc/ShortcutGenerator.cpp`, `Ui/UI/Widgets/Output/Base.cpp` sendValue (gesture entry, never the timer-reachable deliverValue) | Refusals raise intent | sample + intent |

`Trial` gained a 60 s date-rollover check (emits `enabledChanged` once per real
enabled->expired transition; `m_lastEffectiveEnabled` resyncs on licenseDataChanged to prevent
phantom re-publishes) and a once-per-machine expiry notice (`trial/notified`, posted via
`postMessageBox`). `Core::License` carries `trialDaysRemaining`; the console status line in
`WelcomeText.cpp` reads it and regenerates on every `LicenseStateChanged` reprint.

### 0092 remediation (2026-10-04) — maintainer first-run test findings

| Surface | Was | Now |
|---|---|---|
| `ConnectionManager::dropUnavailablePrimaryDevice` (Pro bus selected in Setup) | Pitch box "Activate ... or start a trial" with no offer | `requestProFeature("driver.select")` with a setBusType retry; boot-time restore silent (runs before the handler installs) |
| AI assistant (`main.qml showAIAssistant`) | Opened ungated (build-integrity only); dead `ProUpgradeNotice.qml` | Gated on `app.proVersion`, prompts via `Cpp_Licensing_TrialGate`; dead dialog + loader deleted |
| Historian window (`showDatabaseExplorer`) | Opened ungated in author mode | Gated on `app.proVersion`, prompts centrally (recording toggle was already gated) |
| File transmission (`showFileTransmission`) | Ungated in author mode | Gated on `app.proVersion`, prompts centrally |
| Shortcut generator Save button | `enabled: app.proVersion` made the C++ prompt unreachable | Click always reaches `ShortcutGenerator` (prompts); dim kept as affordance |
| `ProjectSources::addSource` (commercial) | No gate at all (multi-source is Pro) | Second source refused + `requestProFeature("project.multi-source")`; GPL truncation box unchanged |
| Project Pro-content hook (ModuleManager) | `jsonFileChanged` (loads only) | RISING edge of `containsCommercialFeatures` on `groupsChanged`: loads AND first editor-added Pro widget prompt; baseline seeded post-restore |
| Console welcome text | `trial` variant shown for every non-paid state | New `messages/unlicensed/` variant (21 languages) for first-run/declined/expired; `trial` only during an active trial |

### 0092 feature-list audit (2026-10-04, second pass) — enumerated from Welcome_EN.txt and the
### BUILD_COMMERCIAL CMake lists, per the new ledger rule

| Feature | Was | Now |
|---|---|---|
| Variables (shared tables/registers) | UNGATED | `ProjectTables::promptAddTable/promptAddRegister` + `ProjectFolders::promptAddTableInFolder` prompt (`project.variables`); `addTable()` itself stays open for API/importers |
| Value transforms | UNGATED (only notify*() was gated) | `DatasetTransformEditor::displayDialog` prompts (`dataset.transforms`) — covers every open path; library editor left free (library code is inert without an attached transform) |
| Custom workspaces | UNGATED | `setCustomizeWorkspaces` off-to-on prompts (`project.workspaces`); `addWorkspace` halts when refused; file-load flag path untouched; loaded projects with existing custom workspaces stay editable (degraded-load courtesy) |
| Notification log | UNGATED widget (feed was gated) | `DashboardTools::setNotificationLogEnabled(true)` prompts (`dashboard.notification-log`); script/API callers silent via dispatch scope + GUI-thread guard |
| XY plots | Advertised Pro, deliberately free (gate withdrawn in 9d9c6837f) | Kept free; "XY" removed from pro/trial/unlicensed welcome texts (21 languages each); gpl3 texts untouched (XY is present there) |
| Transmit scripts | — | Audited: correctly gated at `Output::Base::sendValue`; editor open is free by design (ProNotice covers it) |

### 0092 six-agent hole hunt (2026-10-04, pre-commit) — fixes applied

| Hole (agent-confirmed) | Fix |
|---|---|
| Workspace CRUD mutated the auto list after a refused customize enable | Every call site of the enable idiom (9 in ProjectWorkspaces, 6 in ProjectFolders) halts when the gate refuses; reorder hoists the guard pre-mutation |
| `.db` drop / `sessions.*` played Historian recordings ungated | `Sessions::Player::openFile(path,id)` + `DatabaseManager::openDatabase(path)` gate with `sessions.playback` / `sessions.historian` (retry-capable; remote-silent) |
| Duplicate Device bypassed the multi-source gate | `duplicateSource` mirrors `addSource`'s check |
| Multi-source with free buses connected end to end | `connectRequiresEntitlement` returns true for `sources.size() > 1` |
| `project.workspace.autoGenerate` set the customize flag directly | Gated on `activated()` before materializing |
| `project.workspace.customize` reported success after refusal | Handler returns an error when the setter refused |
| `notifications.post`/`resolve` ran unlicensed (docs promise an error) | Both handlers error on `!activated()` |
| MQTT publisher dialed brokers / sent Sparkplug births unlicensed | `snapshotConfig` folds `licenseValid()` into `cfg.enabled` (license watch re-syncs) |
| ImageView/Waterfall/NotificationLog rendered fully unlicensed (only Plot3D/Painter degraded) | `WidgetMapBuilder`: ImageView falls back to DataGrid (mirrored in the three workspace resolvers, which also gained the missing Painter mapping), NotificationLog bucket removed. Waterfall deliberately still renders unlicensed: degrading it needs a dataset-level mirror across six workspace sites (OPEN) |
| Remote `io.setBusType` popped a modal after the dispatch scope unwound | New `Core::License::remoteDispatchActive()` probe (root-bound to `RemoteDispatchScope`) consulted at queue time |
| Trial accept during an in-flight silent fetch suppressed errors | `enableTrial()` clears `m_silentFetch` |
| Day cache / reply math used wall clock (rewind window) | Keyed to `MonotonicClock` |
| Empty project snapshot made the connect gate return false | Falls back to the bus-type check |
| Pro-vs-Free.md sold XY as Pro | Marked free (matches code + FAQ + 2026-10-04 decision) |

Open decisions from the hunt are tracked in chat with the maintainer (per-version trial
renewal policy, settings-clone/forgery server-side hardening, export-replay consent for
file-originated sinks, runtime gating of loaded transforms/tables/workspaces, action
autoExecuteOnConnect consent, consent-grant scoping, Lua notify silent no-op).

### Maintainer rulings on the hole-hunt decision list (2026-10-04)

- **Per-version trial renewal is INTENDED policy** (commit a99bd8c77: a fresh 14-day trial per
  app version, via appVerMachineId). The expiry-notice latch is now version-scoped
  (`trial/notified-<version>`) so each version's trial lifecycle shows its one notice.
- **Loaded projects keep RUNNING transforms, tables and file-made custom workspaces unlicensed**
  (accepted: silently changing displayed data is worse than the leak; creation/editing gestures
  and multi-source CONNECT are gated).
- Fixed on ruling: action add/update/duplicate with TX-bearing params now clear the device-write
  consent gate; API consent grants are session-scoped (empty ConsentGate key; the old
  API/DeviceWriteConsent and API/ScriptInstallConsent keys are stale) and prompts name the
  command; unlicensed Lua notify*() logs one warning per session; OPC UA endpoint discovery
  gates (`driver.opcua`), blocking the browse flow's entry; table duplicate + CSV import +
  the constants-library register insert gate as Variables gestures.
- **Open (needs a careful slot, not a one-liner):** gating bare `--runtime` — CLI::runtimeMode()
  is consulted before the licensing block exists, so a naive activated() check would break
  licensed operator deployments; needs a post-root re-derive. **RESOLVED 2026-10-04:** the MachineID
  stored-first fingerprint order (spec 0075) is reverted — fresh hardware read every launch,
  stored id only as the degraded-read fallback (maintainer: the crash the cache guarded against
  had a different cause). `tst_machine_id` now pins fresh-read-wins. **Open:** export-replay first-use
  consent for file-originated sinks to unknown hosts (small spec).
