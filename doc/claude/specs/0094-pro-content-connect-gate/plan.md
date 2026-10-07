---
spec: 0094-pro-content-connect-gate
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-06
---

# Plan 0094 — Pro-Content Connect Gate

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The connect path already refuses a Pro driver or a multi-source project when unlicensed
(`IO::DeviceTableQuery::connectRequiresEntitlement`, read by
`IO::ConnectionManager::connectDevice`). This plan teaches that same predicate one more
reason: the project snapshot the Devices library already holds contains Pro content. The
rule is one pure function in Core over types Core already owns, so every layer answers the
question the same way and a unit test pins it. The refusal reuses the central Pro-intent
seam with a new feature id; the handlers (commercial `Licensing::TrialGate`, GPL notice)
add one explanatory paragraph built from the same rule. The API setters refuse with the
error shape the notification and workspace handlers already use. Nothing touches the frame
pipeline. This shape was chosen because it fails closed: the gate reads data that is always
present, not a binding a composition root could forget.

## Affected subsystems & files

Touch-points confirmed by grep on 2026-10-06 (line numbers are today's and will drift).

| File | Change |
|------|--------|
| `core/Core/DataModel/FrameSupport.h`, `FrameSupport.cpp` | New `ProContentSummary { transforms, tables }` and `SerialStudio::proContentSummary(groups, tableCount)` beside `commercialCfg`. `commercialCfg` itself is untouched. |
| `core/Core/Bus/Messages.h` | `ProjectStructureSnapshot` gains `userTableCount` (the snapshot carries `groups` with `transformCode` already, but no tables). |
| `core/Pipeline/DataModel/ProjectModel.cpp` | `publishStructureSnapshot` fills the count; a table-definition change republishes the snapshot; the existing `LicenseStateChanged` hook also refreshes the workspace fallback. |
| `core/Devices/IO/ConnectionManager/DeviceTableQuery.h`, `.cpp` | `connectRequiresEntitlement` returns a reason (`None`, `ProBus`, `MultiSource`, `ProContent`) instead of a bool. |
| `core/Devices/IO/ConnectionManager.h`, `.cpp` | `connectDevice` raises `project.pro-content` for the content reason, in both build flavors (today's check sits inside `BUILD_COMMERCIAL`); logs the refusal; exposes the reason for the API. |
| `core/Devices/IO/ConnectionManager/BusBridge.cpp` | The `LicenseStateChanged` subscription skips the device rebuild when the live devices cannot be affected by entitlement (R8). |
| `core/Api/API/Handlers/IOManagerHandler.cpp` | `io.connect` returns an error naming the reason on an entitlement refusal (today: success with `connected: false`). |
| `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`, `ProjectUpdateCommands.cpp` | `project.dataset.setTransformCode` and `project.dataset.update` refuse non-blank transform code when unlicensed; clearing stays allowed. |
| `core/Api/API/Handlers/DataTablesHandler.cpp` | `project.dataTable.add`, `rename`, `addRegister`, `updateRegister` refuse when unlicensed; `delete` and `deleteRegister` stay open. |
| `core/Pipeline/DataModel/Project/ProjectEntities.h`, `.cpp`, `core/Pipeline/DataModel/ProjectModel.h` | New `clearDatasetTransform(groupId, datasetId)` mutator: undo scope, ungated (R7). |
| `core/Pipeline/DataModel/Project/ProjectWorkspaces.h`, `.cpp` | Unlicensed fallback list returned by `activeList()` while the project customizes workspaces; workspace CRUD gates no longer exempt an already-customized project. |
| `app/src/Licensing/TrialGate.h`, `.cpp` | The trial and activation prompts add the content paragraph for `project.pro-content`. **Protected licensing surface.** |
| `app/src/Misc/ProFeatureNotice.h`, `.cpp` (new), `app/CMakeLists.txt` | The GPL notice moves out of `ModuleManager.cpp` (1496 of 1500 lines) and gains the same paragraph; also hosts the shared text builder. |
| `app/src/Misc/ModuleManager.cpp` | Loses the static notice function; passes a summary provider to both handlers. |
| `app/rcc/commands/projecteditor.json`, `app/rcc/commands/layouts/editor-menus.json`, `app/qml/Commands/ProjectEditorMenuBindings.qml` | New "Remove Transform" context-menu command. |
| `app/tests/tst_pro_content.cpp` (new), `app/tests/CMakeLists.txt` | Unit test for the rule (AC1). |
| `tests/integration/test_pro_content_gate.py` (new) | API-driven checks (AC2, AC3, AC10). |
| `doc/help/*.md`, `README.md`, `app/rcc/ai/skills/transforms.md`, `project_basics.md` | The published rule (R12). |
| `doc/claude/specs/0042-license-token-hardening/consumers.md`, `doc/claude/architecture/io.md` | Supersede the 2026-10-04 ruling; record the new refusal reason under "Opening a Link". |

## Architecture & data flow

Everything below runs on the GUI thread at command rate.

1. **The rule.** `proContentSummary` counts datasets whose `transformCode` is non-blank
   after trimming, and takes the user-table count as given. User tables are the saved
   `tables` list only: the runtime `__datasets__` system table never appears there, and a
   computed dataset with no transform is a constant zero, so neither needs its own case.
2. **Snapshot.** `ProjectModel::publishStructureSnapshot` already publishes a retained
   `ProjectStructureSnapshot` on content changes; `BusBridge` stores it in the Devices
   library through a direct connection on the same thread. It gains the table count and a
   republish on table-definition changes.
3. **Gate.** `connectDevice` asks `DeviceTableQuery` for the refusal reason before
   `beginRequest()`, which `io.md` requires of any pre-start refusal. For `ProContent` it
   calls `Core::License::requestProFeature("project.pro-content", retry)` and returns; the
   retry is the existing `connectDevice` closure, so accepting the trial connects with no
   second click (R4, R11).
4. **Explanation.** The seam's signature does not change. The handlers recognize the
   feature id and ask a root-injected provider for the summary; a helper in
   `ProFeatureNotice` turns it into one translated paragraph with plural counts. A missing
   provider degrades to today's generic prompt, never to a missing gate.
5. **Running sessions.** A license transition reaches the Devices library as
   `LicenseStateChanged` and calls `rebuildDevices`, which today can close, rebuild and
   reconnect a live Project File session. The subscription gains an early return: when
   connected, single-source, and every live device is on a free bus, entitlement cannot
   change anything about those devices, so nothing is rebuilt and the session continues.
6. **Workspaces.** `activeList()` returns a derived automatic list while the project
   customizes workspaces and no entitlement is active. The editor list and the save path
   read `list()`, so the file keeps its custom workspaces. The fallback refreshes from the
   existing `LicenseStateChanged` hook in `ProjectModel`.
7. **API.** Each handler checks `Core::License::activated()` first and returns
   `ErrorCode::OperationFailed` with a message, exactly as `notifications.post` does.
   Remote dispatch already keeps the prompt silent.

## Hotpath & threading impact

- **Touches the hotpath?** No. `FrameReader`, `CircularBuffer`, `FrameBuilder`
  (`TransformDispatch` included) and the dashboard draw path are not edited, and no
  per-frame read of licensing state is added. `ConnectionManager` is edited only in the
  connect gesture and the license-transition handler; `ss-hotpath` is invoked before that
  edit as the skill requires.
- **New cross-thread signal/slot?** No. The snapshot, the gate, the prompt and the API
  checks all run on the GUI thread.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unaffected.
- **Benchmark:** `--benchmark-hotpath` runs unchanged as a regression guard (AC13).

## Data model & persistence

No schema change, no new project key, no writer-version bump. `userTableCount` lives only in
the in-memory bus snapshot. The workspace fallback is a view: nothing derived from it is
written to the project file.

## API / SDK surface

- `io.connect`: an entitlement refusal becomes an error that names the reason. This changes
  today's reply for a Pro driver too (success with `connected: false`); the plan unifies all
  three reasons rather than leaving two reply shapes.
- `project.dataset.setTransformCode`, `project.dataset.update`: error when unlicensed and the
  code is non-blank.
- `project.dataTable.add`, `rename`, `addRegister`, `updateRegister`: error when unlicensed.
- Unchanged on purpose: `project.loadJson`, `project.open`, `project.template.apply` and
  `project.dataset.duplicate` may bring Pro content in; the connect gate is what stops it
  from running. Dry-run commands write nothing.
- `doc/help/API-Reference.md` gains the new errors. No gRPC field moves.

## QML / UI

One new context-menu command, "Remove Transform", shown for a dataset that has transform
code. It is registry-driven (manifest entry, menu node, bindings entry), so no menu QML is
written by hand and `DatasetView.qml` is not touched. The prompts are the existing message
boxes with one more paragraph.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the gate reads from | Project snapshot already in Devices; a root-bound probe function; a Pipeline-owned flag | Snapshot. It fails closed: no binding exists for a root to forget. |
| How the prompt gets its counts | Feature id plus root-injected provider; widen the seam with a detail string; generic text | Feature id plus provider. The seam signature and its 30-odd call sites stay as they are, and one helper owns the wording for both build flavors. |
| Live session at expiry (R8) | Skip the device rebuild when it cannot matter; exempt only the automatic reconnect from the content check | Skip the rebuild. The second option keeps today's close-and-reopen at expiry, which splits a recording in two, and the spec promises the session is not disconnected. Falls back to the second if reading `rebuildDevices` in full shows a dependency on that rebuild. |
| Fold transforms into `commercialCfg` | Yes; no | No. That flag also drives editor notices and the dashboard's commercial-frame flag; changing its meaning has a wide, unrelated blast radius. |
| Prompt when a Pro-content project is opened | Prompt on load; first refusal at Connect | At Connect. Opening must stay free so the content can be removed (R7). |
| API refusal site | One check in `ProjectModel::updateDataset`; per-handler checks | Per handler. The mutator returns void and also serves the editor form and undo, and clearing must stay allowed. |
| `io.connect` reply | Error only for the new reason; error for every entitlement refusal | Every refusal. One reply shape per outcome. |
| Workspace fallback | View-level fallback list; clear the customize flag on load; defer to its own spec | View-level. Clearing the flag would rewrite the file on the next save (breaks R6). It is the last milestone and can be cut without affecting the gate. |
| GPL notice location | Keep in `ModuleManager.cpp`; extract | Extract. The file is four lines under the cap and the notice is about to grow. |

## Risks & mitigations

- **Licensing surface.** `TrialGate.cpp` is under `app/src/Licensing/`. The only change is
  prompt text selected by feature id; failure mode is a wrong or missing paragraph, never a
  changed entitlement decision. Named here and again in chat before the edit; it does not
  ride with the other milestones' commits.
- **Republishing the snapshot on table edits.** Subscribers react to every snapshot. If a
  `Content` republish on a table-definition change has side effects (transform recompiles,
  dashboard resets), carry the count in a small dedicated retained topic instead. The
  signal used must be definition-rate, never the per-frame table value writes.
- **`rebuildDevices` early return.** The rebuild exists so Pro drivers appear and disappear
  with entitlement. The guard is limited to connected, single-source, free-bus sessions;
  `rebuildDevices` is read in full first, and the fallback design is recorded above.
- **Workspace machinery.** Fifteen gate sites, merge logic and per-workspace layout state
  live in `ProjectWorkspaces`. The fallback must not let dashboard layout autosave write
  derived workspace ids into the project; AC7's byte-identical check exists to catch that.
- **No grandfathering.** Free users who have used transforms since before they were gated
  meet the refusal on upgrade. The prompt's own paragraph (R5) is the notice; the release
  notes carry the migration note.
- **Concurrent work in the tree.** Uncommitted spec 0093 edits
  `ProjectDatasetFieldCommands.cpp`, `app/tests/CMakeLists.txt` and `API-Reference.md`.
  Implementation starts after 0093 is committed and after the 4.2 tag.
- **No compiler in the loop today.** There is no compile database under `build/`, so
  `scripts/syntax-check.py` cannot run. Implementation needs one first; otherwise every
  C++ claim is unverified until the maintainer's build.
- **Silent-breakage classes in play:** license-derived state that does not re-derive on a
  transition (the workspace fallback and the gate both read live state, never a baked
  value); a refusal placed after `beginRequest()` (stranded wait cursor).

## Review remediation (2026-10-06)

A six-reviewer pass found that a connect-time gate alone lets content into a session already
running, and that the deferral could be cut short. The design now adds four things, all in
`IO::EntitlementGate`, so the facade stays under its size cap:

- **Live sessions.** `observeContent()` runs after every project snapshot and mode change.
  Without an entitlement, a live session whose transforms or tables exceed what its connect
  was admitted with ends one event-loop turn later. The intent is silent when the change came
  from a remote request. A driver's own reopen (`refuseRecovery()`) follows the same rule.
- **Deferral.** `deferRebuild()` skips a publication the devices were already rebuilt for (a
  trial start publishes twice) and counts in-flight dials as live. Any `rebuildDevices()`
  clears the hold. The release re-checks before rebuilding, so a reconnect landing in the same
  event-loop turn is not closed.
- **Remote origin.** `queueReconnect()` captures `remoteDispatchActive()` at queue time and
  refuses silently.
- **One rule, one wording.** `SerialStudio::authorsTransform()` defines what counts as writing
  a transform for both API setters. `Core::License::requiresProMessage()` words every textual
  refusal, per build flavor.

## Test & verification plan

| AC | Check |
|----|-------|
| AC1 | `ctest` `tst_pro_content`: blank, whitespace, non-blank code; zero and one table; mixed groups. |
| AC2, AC3, AC10 | `pytest tests/integration/test_pro_content_gate.py`, branching on `licensing.getStatus` as `test_licensing_degradation.py` does: unentitled asserts the errors and an empty dashboard; entitled asserts the same calls succeed. |
| AC4 | Amended, see spec: CSV playback of a Pro-content project works unlicensed and shows recorded values. Maintainer observation. |
| AC5, AC6, AC8, AC9, AC11, AC12 | Maintainer observations in a built app: first-run trial path, prompt wording in English and Arabic, remove-then-connect with undo, seeded expiry during a live session, operator deployment, GPL build. |
| AC7 | Maintainer observation plus a file hash before and after the session. |
| AC13 | `--benchmark-hotpath` on the maintainer's build. |
| AC14 | `documentation-verify.py`, `claim-verify.py`, `registry-verify.py` (new command), `code-verify.py --check`, `layer-verify.py`, the singleton and size censuses. |

Before handoff: `qt-cpp-review` on the C++ diff, `qt-qml-review` on the bindings change,
`scripts/sanitize-commit.py`.
