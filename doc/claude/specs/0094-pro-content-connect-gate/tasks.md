---
spec: 0094-pro-content-connect-gate
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-06
---

# Tasks 0094 — Pro-Content Connect Gate

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

Approved together with the plan on 2026-10-06: the maintainer's instruction was to start
work at once, without waiting for spec 0093 to be committed or for a release tag.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- **Standing caveats for every C++ task.** No compile database exists under `build/`, so
  `scripts/syntax-check.py` cannot run: verification is lint plus read-back, and compilation
  stays unverified until the maintainer builds. Spec 0093 is uncommitted in the working
  tree; a file it also modified takes targeted edits only, never a rewrite.

## Tasks

### T1 — Core rule

- **Files:** `core/Core/DataModel/FrameSupport.h`, `core/Core/DataModel/FrameSupport.cpp`
- **Does:** Adds `ProContentSummary { transforms, tables }` and
  `SerialStudio::proContentSummary(groups, tableCount)`: counts datasets whose
  `transformCode` is non-blank after trimming. `commercialCfg` is not touched.
- **Verify:** `code-verify.py --check` on both files; read-back.
- **Deps:** none
- [x] done

### T2 — Snapshot carries the table count

- **Files:** `core/Core/Bus/Messages.h`, `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:** `ProjectStructureSnapshot` gains `userTableCount`; `publishStructureSnapshot`
  fills it and a table-definition change republishes. Invariant: the trigger is
  definition-rate, never the per-frame table value writes, and every construction site of
  the snapshot value-initializes the new field.
- **Verify:** grep every construction site of the snapshot; `code-verify.py --check`.
- **Deps:** T1
- [x] done — no new trigger needed: every edit already republishes through setModified; tst_message_bus.cpp gained the new positional argument (wire-break rule, outside the plan list)

### T3 — Unit test for the rule

- **Files:** `app/tests/tst_pro_content.cpp` (new), `app/tests/CMakeLists.txt` (0093-shared:
  one appended registration)
- **Does:** Pins AC1: blank, whitespace-only and non-blank code; zero and one table; mixed
  groups.
- **Verify:** `code-verify.py --check`; `ctest` on the maintainer's build.
- **Deps:** T1
- [x] done

### T4 — Connect gate

- **Files:** `core/Devices/IO/ConnectionManager/DeviceTableQuery.h`, `.cpp`,
  `core/Devices/IO/ConnectionManager.h`, `.cpp`
- **Does:** `connectRequiresEntitlement` returns a reason; `connectDevice` raises
  `project.pro-content` for the content reason in both build flavors, logs the refusal and
  exposes the reason. Invariants: invoke `ss-hotpath` first; the refusal stays before
  `beginRequest()`; no per-frame work; the empty-snapshot fallback to the bus check stays.
- **Verify:** `code-verify.py --check`; `layer-verify.py`; read-back of every early return.
- **Deps:** T2
- [x] done — gate logic lives in a new sub-object, IO::EntitlementGate (two files outside the plan list): the facade was 12 lines under the 1500 cap

### T5 — Running sessions survive a license transition

- **Files:** `core/Devices/IO/ConnectionManager/BusBridge.cpp` (and `ConnectionManager.cpp`
  if the guard belongs in `rebuildDevices`)
- **Does:** Skips the device rebuild on `LicenseStateChanged` when connected, single-source
  and every live device is on a free bus. Invariant: read `rebuildDevices` in full first;
  the guard may not change behavior for Pro buses or multi-source projects.
- **Verify:** `code-verify.py --check`; read-back against the plan's fallback design.
- **Deps:** T4
- [x] done — deferral, not skip: the rebuild a licence transition asks for runs at the user's disconnect, so its side effects still happen

### T6 — `io.connect` names the refusal

- **Files:** `core/Api/API/Handlers/IOManagerHandler.cpp`
- **Does:** Returns `ErrorCode::OperationFailed` with the reason when the gate would refuse,
  for all three reasons.
- **Verify:** `code-verify.py --check`; covered by T11.
- **Deps:** T4
- [x] done

### T7 — API setters refuse

- **Files:** `core/Api/API/Handlers/ProjectUpdateCommands.cpp`, `DataTablesHandler.cpp`,
  `ProjectDatasetFieldCommands.cpp` (0093-shared: targeted edit)
- **Does:** Non-blank transform code and table or variable authoring return the
  notification-handler error shape when unlicensed; clearing and deleting stay allowed.
- **Verify:** `code-verify.py --check`; covered by T11.
- **Deps:** none
- [x] done

### T8 — Remove Transform

- **Files:** `core/Pipeline/DataModel/Project/ProjectEntities.h`, `.cpp`,
  `core/Pipeline/DataModel/ProjectModel.h`, `app/rcc/commands/projecteditor.json`,
  `app/rcc/commands/layouts/editor-menus.json`,
  `app/qml/Commands/ProjectEditorMenuBindings.qml`
- **Does:** Ungated `clearDatasetTransform(groupId, datasetId)` plus its context-menu
  command. Invariant: the mutator opens a `ProjectUndoScope` and reaches
  `setModified(true)`, or nothing is recorded.
- **Verify:** `code-verify.py --check` (`undo-scope-missing`); `registry-verify.py`.
- **Deps:** none
- [x] done

### T9 — The prompt explains itself

- **Files:** `app/src/Misc/ProFeatureNotice.h`, `.cpp` (new), `app/CMakeLists.txt`,
  `app/src/Misc/ModuleManager.cpp`, `app/src/Licensing/TrialGate.h`, `.cpp`
- **Does:** Moves the GPL notice out of `ModuleManager.cpp`, adds the shared paragraph
  builder, and has both handlers append it for `project.pro-content`. Invariants: licensing
  surface, so the change is named in chat before the edit; no entitlement decision changes;
  the handler still installs after `restoreLastProject()`; no new singleton reach.
- **Verify:** `code-verify.py --check`; singleton and size censuses; read-back.
- **Deps:** T1
- [x] done — no provider injection: the paragraph reads the root-bound pipeline module set directly, so nothing new is wired and no singleton reach is added; ModuleManager.cpp ends at 1457 lines

### T10 — Workspace fallback (cuttable)

- **Files:** `core/Pipeline/DataModel/Project/ProjectWorkspaces.h`, `.cpp`,
  `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:** `activeList()` returns a derived automatic list while the project customizes
  workspaces and no entitlement is active; refreshed from the existing license hook.
  Invariant: the save path keeps reading `list()`, so the file is never rewritten from the
  fallback; state re-derives on every license transition.
- **Verify:** `code-verify.py --check`; AC7 on the maintainer's build. Cut and report if a
  full read of the workspace code shows the fallback cannot be made view-only.
- **Deps:** none
- [ ] done — CUT on 2026-10-06, as the plan allowed. A non-persisted session list exists and could back a fallback, but keeping it fresh touches several refresh paths whose signals cannot be tested without a build, and it is unconfirmed that dashboard layout autosave stays out of the project file. Custom workspaces keep the editor-only gate; spec R6 and AC7 stay open

### T11 — Integration test

- **Files:** `tests/integration/test_pro_content_gate.py` (new)
- **Does:** AC2, AC3, AC10, branching on `licensing.getStatus` the way
  `test_licensing_degradation.py` does.
- **Verify:** `pytest --collect-only`; full run needs the app up.
- **Deps:** T6, T7
- [x] done — five tests collected; not run (a running app would be the pre-change build)

### T12 — Published rule

- **Files:** `README.md`, `doc/help/Pro-vs-Free.md`, `Dataset-Transforms.md`,
  `Data-Tables.md`, `FAQ.md`, `Troubleshooting.md`, `API-Reference.md` (0093-shared),
  `app/rcc/ai/skills/transforms.md`, `project_basics.md`,
  `doc/claude/specs/0042-license-token-hardening/consumers.md`,
  `doc/claude/architecture/io.md`, `startup.md`
- **Does:** States the connect gate, the free removal path and the API errors; records the
  superseded ruling and the new refusal reason.
- **Verify:** `documentation-verify.py`, `claim-verify.py`.
- **Deps:** T4 through T10
- [x] done — also added a Subsystem Contracts row to CLAUDE.md (outside the plan list); FAQ needed no change; app/rcc/ai/search_index.json is left for the pre-commit sanitize to regenerate

### T13 — Gate battery and self-review

- **Files:** none new
- **Does:** `code-verify.py --check` on every touched file, `registry-verify.py`,
  `layer-verify.py`, the singleton and size censuses, then a read of the whole diff against
  "what was asked, and only that".
- **Verify:** all listed gates clean; findings fixed or recorded.
- **Deps:** all previous
- [x] done — code-verify, clang-format, registry-verify, layer-verify, claim-verify, documentation-verify and both censuses clean; sanitize-commit.py NOT run (it rewrites files across a tree that holds the uncommitted spec 0093 work); syntax-check could not run (no compile database)

### T14 — Remediation after the six-reviewer pass (2026-10-06, maintainer: "fix all issues")

- **Files:** `core/Core/License.{h,cpp}`, `core/Core/DataModel/FrameSupport.{h,cpp}`,
  `core/Devices/IO/ConnectionManager/{EntitlementGate,DeviceTableQuery,BusBridge}.{h,cpp}`,
  `core/Devices/IO/ConnectionManager.{h,cpp}`, `core/Api/API/Handlers/{IOManagerHandler,
  ProjectUpdateCommands,ProjectDatasetFieldCommands,DataTablesHandler,ProjectApiSupport}`,
  `core/Pipeline/DataModel/Project/ProjectEntities.{h,cpp}`, `ProjectModel.h`,
  `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp`, `app/src/Misc/ProFeatureNotice.cpp`,
  `app/qml/Commands/ProjectEditorMenuBindings.qml`, `app/tests/tst_pro_content.cpp`,
  `tests/integration/test_pro_content_gate.py`, docs.
- **Does:**
  - Live sessions: content growing past a live unentitled session's admission ends it a turn
    later (`observeContent`), and a driver reopen is refused on the same terms
    (`refuseRecovery`).
  - Remove Transform resyncs the runtime and shows only on a single dataset with code.
  - The rebuild reconnect captures remote origin at queue time.
  - The deferred rebuild: in-flight dials count as live, a repeated publication is skipped,
    any rebuild clears the hold, and its release re-checks before rebuilding.
  - API and log refusals share `Core::License::requiresProMessage`, the API branches on the
    enum, and both transform setters use `SerialStudio::authorsTransform`.
  - The editor's Apply re-checks the licence, and the GPL root binds the remote-dispatch
    probe.
  - Dead `connectRequiresEntitlement` and the QVector overload are removed.
- **Verify:** gate battery; `pytest --collect-only`; syntax-check still unavailable.
- **Deps:** T13
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (if hotpath touched).
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
