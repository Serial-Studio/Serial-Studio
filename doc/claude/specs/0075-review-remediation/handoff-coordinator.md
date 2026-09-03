# Coordinator notes — integration of WP0 and WP-A..H (2026-09-01)

Nine package worktrees merged into one tree (this worktree). Every static gate is green:
`code-verify --check` 0 errors (26 advisories, listed below), tu/singleton/dup censuses
re-seeded after the merge, `claim-verify` 9 pre-existing errors (all the startup.md ordered
anchor: WP-J fixes the doc), `registry-verify` clean, `generate-property-registry --check`
clean, `pytest tests/scripts scripts/tests` 525 passed, `pytest tests --collect-only` 1762.

## Patches the coordinator applied (from the package handoffs)
- `ErrorCode::ScriptTimeout` + `ErrorCode::SessionLive` in `API/CommandProtocol.h`; dry runs use it.
- `CLI`: `--api-port`, post-root selftest dispatch (`postRootSelfTestRequested` /
  `runPostRootSelfTests`), `x-pathParams` projected in `dumpApiSchema`; `main.cpp` runs the
  post-root suite after bootstrap and exits through the same ladder.
- `REUSE.toml` + `.gitattributes` cover `app/tests/fuzz/corpus/**` (binary); `code-verify`
  skips the corpus in its walk; `_HOTPATH_ASSERT_ALLOWED` gained `BlockStager.cpp`;
  `doc-anchors.json` `frame-block-sample-cap` now points at `BlockStager.h`.
- `CommandHandler`/`MCPHandler`: pending consent reported as `CONSENT_REQUIRED` / retry text.
- `ModbusHandler`: per-type count cap (2000 bits for FC01/02).
- `OpcUa.qml`: checkbox for `allowPlaintextPassword` beside the plaintext warning.
- `SessionsHandler`: `sessions.delete` refuses the live session with `SESSION_LIVE`;
  `sessions.getStatus` exposes `writeFailed`, `rawOverruns`, `droppedBlocks`, `currentSessionId`.
- `DashboardTools`: `refreshActionsFromProject()` on `ProjectModel::actionDataChanged`
  (`ProjectModel&` injected through the ctor); `configureActions` shares `rebuildActions`.
- `ProjectPresentation::savePluginState` gated on ProjectFile; `ProjectEditorCommit` title
  branch returns instead of an unconditional `setModified`.
- `MonotonicClock::nowFloored` skips the write inside `kPersistIntervalMs` (the injected seam
  WP-H's test drives); `now()` keeps its cache.
- `MQTT::applyCredentials` writes the vault once on restore (E13).
- `project.save` / `project.setTitle` descriptions and `ai/skills/project_basics.md` describe
  the checkpoint semantics.
- `tst_machine_id.cpp` (WP-C, persisted-id path) kept; WP-H's identity suite renamed
  `tst_machine_id_identity`.
- `CLAUDE.md`: tu-cutter mention removed.
- `test_cpp_regressions.py`: two pins re-targeted (`rebuildLineSeriesPreservingState`,
  `ScriptDryRun`).

## Open for WP-I (beyond its task block)
1. `tst_ethernetip_worker` goes with WPI-T1 against `PolledPlcWorkerBase` (the `kEipBackend`
   label is not a seam).
2. Wildcard `disconnect(x, nullptr, y, nullptr)`: 19 sites in 16 files, baselined by WP0-T13.
   R12.3 wants them all closed and the baseline emptied; `OpcUaSubscriptions::unbindSession`
   is one of them.
3. Four Lua hook implementations remain (`LuaScriptEngine`, `MacroRunner`,
   `MQTT/PublisherScript`, `StreamWorker`): consolidate onto `LuaDeadlineHook`.
4. `ExtensionData`: `tst_extension_data_rows` and the shared `datasetWidgetsFor()` helper need
   the `ExtensionRowsModel` file-pair split (F14, WPI-T7 territory).
5. `FrameBuilder::wireAsyncSinkHooks` carries 11 `arch-singleton-instance` advisories (moved
   code): `BlockPublisher` (WPI-T5) takes the sinks by injected reference instead.
6. `DashboardIngest` binds facade state by reference (`IngestBindings`); finishing the move
   into owned state is optional, the facade is at 1856 lines.
7. `Terminal.cpp` is 1988 lines (grew 50 in WP-E); not in the R12.8 four, a follow-up.
8. `code-verify`'s `id-placement` rule is dead (`_check_shallow_id` breaks at the first content
   line); fix with a fixture or delete it.
9. The dup-window rule catches no C++ pair at the agreed threshold (S7/EIP share 24 windows,
   identifiers differ); WPI-T1 removes that pair by construction, the rule stays as seeded.
10. Not done by WP-C, hardware-blind, stays open in plan.md: USB `write()` off the GUI thread
    and the shared libusb/miniaudio contexts + `setPersistent(false)` rollout (WPC-T12/T14).
11. Re-seed `--tu-census --accept` and `--singleton-census --accept` at the end of WP-I.

## Open for WP-J
- startup.md pinned order (9 claim errors), `Keys::` lives in `DataModel/FrameKeys.h` not
  `Frame.h` (CLAUDE.md + tasks.md say Frame.h), io.md dial doctrine (TCP/S7/EIP/IEC 104 now
  async, `dialTcpBlocking` gone, `isConnecting()` list), project.md pointsChanged sentence,
  scripting.md (LuaDeadlineHook, ScriptDryRun, stream JS watchdog), dashboard.md ingest section,
  export.md session boundary, code-style.md QML (theme map, canvas repaint), tests/README rows
  from every handoff, `doc/claude/architecture/ai.md`, spec/plan amendments for the tests moved
  to pytest (WP-A T13/T14/T15/T17/T18/T19, WP-F five suites, WP-H three suites, WP-C four).
