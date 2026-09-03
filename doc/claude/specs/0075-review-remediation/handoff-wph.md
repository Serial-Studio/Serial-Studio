# Handoff — WP-H (assistant, extensions, CLI, licensing, startup)

Tasks WPH-T1 .. WPH-T12: all twelve ticked in `tasks.md`. Findings closed: J1-J8, K1-K6, K9,
K12-K14, plus the M10/M11 coverage that fits a unit tier.

Gates run in the worktree: `code-verify.py --check` on all 154 changed/new C++/QML files
(**0 errors**, one pre-existing advisory), `registry-verify.py` (CLEAN, now including the new
catalog gate), `pytest tests/scripts/` (**309 passed**, 7 of them new). ctest, cmake and the app
were not run, per the brief.

---

## Files changed

### Assistant (J1-J8)

| File | Change |
|------|--------|
| `app/src/AI/Conversation.cpp/.h` | Debounced timer takes a project **checkpoint** (`assistant.checkpoint` through the dispatcher) instead of saving the `.ssproj` (J2). Approve/deny/help-fetch/async-tool all resume through one `maybeResumeAfterToolBatch()` tri-condition guard, so a click mid-stream cannot start a second live reply (J4). `runToolCall` split into `runToolCallAsync` + `finishToolCall`; `onAsyncToolFinished` drops results of a cancelled or superseded turn. `budgetedHistory` derives its window through the new capability caps (J1). |
| `app/src/AI/Conversation/AsyncToolRunner.{h,cpp}` (new) | `fs.read`/`fs.search` run on a one-thread pool owned by the conversation and report back queued with the turn generation (J3). The pool is a member so its destructor waits for a running scan. |
| `app/src/AI/Providers/Provider.h` | `ProviderCapabilities::budgetedOutputTokens()/budgetedSystemReserve()` cap both reservations at a quarter of the window (J1); `Reply` gains the finish latch, the stream budget and the transport-policy declarations. |
| `app/src/AI/Providers/Provider.cpp` (new) | `finishOk`/`finishWithError`/`streamBudgetBreached` lifted out of the three backends (J5); `isTransportAllowed` (https anywhere, http loopback only), `applyStreamPolicy` (ManualRedirectPolicy), `endsTurnOnParseError` (the one parse-error rule, J6). |
| `app/src/AI/Providers/{Anthropic,OpenAI,Gemini}Reply.{h,cpp}` | Duplicated finalization removed; all three use the base. Anthropic now skips a recoverable parse error instead of ending the turn (J6). Every request applies the redirect policy; `OpenAIReply` refuses a non-loopback `http://` endpoint before the key is attached (queued so the caller has connected). |
| `app/src/AI/Providers/LocalProvider.{h,cpp}` | `ai/localContextWindow` (default 8192, clamped 2048..1e6) feeds `capabilities().contextWindowTokens` (J1). |
| `app/src/AI/Assistant.{h,cpp}` | `localContextWindow()` / `setLocalContextWindow()` QML surface beside the existing local base-URL pair. |
| `app/src/AI/KeyVault.cpp` | `redact()` returns `"***"`; no character of a key reaches a log line (J8). |
| `app/src/AI/Tools/ToolFilesystemTools.cpp` | Doc contract: the fs primitives may run on a worker, so nothing added there may touch a GUI-owned object. |
| `app/qml/AI/AssistantPanel.qml` | Auto-approve tooltip states checkpoint semantics ("the project file on disk changes only when you save"); "Keys are encrypted" -> "stored obfuscated in this machine's settings file" (K6). |

### Extensions (K3, K5, K12)

| File | Change |
|------|--------|
| `app/src/Misc/Extensions/ExtensionCatalog.{h,cpp}` | Catalog v2: `CatalogFile{path,sha256,size}`, `parseFileList` (refuses v1/bad digests with a reason), `digestMatches`, `isTrustedRepoUrl`, `compareVersions` (numeric). |
| `app/src/Misc/Extensions/ExtensionInstaller.{h,cpp}` | Staged, atomic install: download/copy into `<id>.staging`, verify each file, swap through `<id>.previous`, restore it if the second rename fails, delete staging on any failure. `installed.json` records per-file digests and is written last. New `installFailed(id, reason)` + `lastError()`. |
| `app/src/Misc/ExtensionManager.{h,cpp}` | `addRepository` refuses non-https/non-local (queued message box); `hasUpdate` numeric; an update keeps the locally recorded install folder; a `extension.catalog` Problem Center checker names repositories whose entries carry no digests; `restoreRunningPlugins` resumes queued so a launch modal never opens under a reply's stack (K13). |
| `app/rcc/extensions/schema/catalog.json` (new) + `app/rcc/rcc.qrc` | Published catalog v2 schema (`schemaVersion: 2`, 64-hex `sha256` per file). |
| `scripts/registry-verify.py` | `check_extension_catalog()`: schema shape, jsonschema seeds (valid v2 accepted, v1 rejected), and the C++ gate still calling the parser/staged swap. |

### CLI, licensing, startup (K1, K2, K4, K9, K13, K14)

| File | Change |
|------|--------|
| `app/src/Licensing/LemonSqueezy.{h,cpp}` | New `requestFinished(ok, reason)` emitted exactly once per activate/deactivate on every path (pre-flight refusals, empty/malformed responses, each rule-chain refusal, success). A refused deactivation no longer clears the local cache — only `deactivated == true` does (K1). Every message box is now posted, not shown (K13). |
| `app/src/Licensing/OfflineLicense.cpp` | `activatedChanged` forwards through `notifyEntitlementMaybeChanged`, restoring the latch it bypassed (K9). |
| `app/src/Licensing/Trial.cpp` | The three reply-handler message boxes are posted (K13). |
| `app/src/Misc/CrashTracker.{h,cpp}` | Preserve-list reset factored into `resetSettingsPreservingLicense(QSettings&)`, shared with the CLI. |
| `app/src/Misc/CLI.{h,cpp}` | `--reset` clears the **default** `QSettings` via that shared function (K2); `--activate`/`--deactivate` wait on `requestFinished` and print the server's reason (K1); `--api-token-file` and `SS_API_TOKEN` resolve ahead of argv (K14); `registerCommercialOptions()` split out to stay under the function-length cap. |
| `app/src/Misc/Utilities.{h,cpp}` | New `postMessageBox(...)`: the queued-modal seam the three K13 sites share. **Out-of-list file — see Patches.** |
| `app/src/main.cpp` | `shutdownSession()` ladder (workers joined, drivers stopped, handler removed, context shut down) runs on both exits; the failed-UI path returns a status from `runConfiguredSession` instead of escaping the scope (K4). |
| `app/CMakeLists.txt` | Two new sources appended contiguously (`AsyncToolRunner.cpp`, `Provider.cpp`) plus their headers. |

## Tests added

| Test | Tier | Pins |
|------|------|------|
| `app/tests/tst_sse_event_reader.cpp` | ctest | Frame splitting, CRLF carry-over, `[DONE]`, multi-line data, recoverable vs fatal parse errors (M10) |
| `app/tests/tst_redactor.cpp` | ctest | Tool-result scrubbing of key/bearer/PEM shapes; ordinary telemetry untouched (M10) |
| `app/tests/tst_sentinel_probe.cpp` | ctest | Classification, display strip, compliance state machine, latch restore (M10) |
| `app/tests/tst_file_sandbox.cpp` | ctest | Read/write roots, traversal, dropped-path allow-list, search — **plus the J3 worker lane** (generation echo, queued result) |
| `app/tests/tst_reply_state_machine.cpp` | ctest | Three backends against `FakeTransport`: one `finished` per reply, 401 vs 429 classification, unified parse policy, transport policy, redaction (J5/J6/J8) |
| `app/tests/tst_conversation_turn.cpp` | ctest | The J1 window arithmetic: an 8k window budgets negative uncapped and trims when capped; `FakeProvider` event ordering |
| `app/tests/tst_extension_installer.cpp` | ctest | v1 refused, bad digest refused, verified install, **corrupt update leaves 1.0.0 intact**, digests recorded, numeric compare, repo scheme (K3/K5/K12) |
| `app/tests/tst_simplecrypt.cpp` | ctest | Round trip, wrong key, tamper, no-key refusal, non-deterministic ciphertext (M11) |
| `app/tests/tst_monotonic_clock.cpp` | ctest | Floor semantics **and the once-a-minute write rate WPE-T10 must implement** (K10) |
| `app/tests/tst_commercial_token.cpp` | ctest | Seal integrity, post-seal edits invalidate, current-slot transitions (M11) |
| `app/tests/tst_machine_id.cpp` | ctest | Fingerprint stability, digest shape, non-zero cipher key (M11, persisted-id path only) |
| `app/tests/fuzz/fuzz_sse_reader.cpp` + 6 corpus seeds | fuzz | Provider stream bytes, whole and split (R14.3) |
| `tests/scripts/test_cpp_regressions.py` (+7 cases) | pytest, **runnable now, green** | K4 ladder + order, K2 store, K1 verdict wait + deactivation cache, K14 token order, K13 no inline modals, J2 checkpoint-not-save, K3/K5 digest+staging |
| `tests/integration/test_assistant_autosave.py` | pytest (maintainer) | File hash unchanged across edits; only `project.save` writes; checkpoints listed |
| `tests/integration/test_extension_install.py` | pytest (maintainer) | v1 never installs; corrupt update keeps the installed version; http repo refused |
| `tests/integration/test_cli_licensing.py` | pytest (maintainer, `SS_BINARY`) | Fast non-zero exit on a bad key, clean no-op deactivate, `--reset` |

Two suites (`tst_reply_state_machine`, `tst_conversation_turn`, `tst_extension_installer`) link
WP0's `support/FakeTransport.cpp` / `support/FakeProvider.cpp`; `ss_add_unit_test` skips a suite
whose sources are absent, so they configure quietly until WP0 lands. The fuzz call uses WP0's
`ss_add_fuzz_target` at the end of `app/tests/CMakeLists.txt` under `# spec 0075 fuzz targets`
and **requires WP0 to merge first** (an unknown command fails the configure).

## Tasks not done as specified, and why

1. **`tst_lemonsqueezy_rules` (named in WPH-T8's Verify) is not added.** `LemonSqueezy.cpp` pulls
   `OfflineLicense` -> `OfflineCertificate` -> `ThirdParty/ed25519_verify`, `Trial`, `MachineID`,
   `CommercialToken` (whose header hard-`static_assert`s on `COMMERCIAL_BUILD_SALT`) and
   `Misc::Utilities` (QtWidgets/QtSvg) — the link wall `tst_proto_importer` documents in
   `app/tests/CMakeLists.txt`, and `Trial`'s constructor issues a live network request. The K1
   rules are pinned instead by `test_cpp_regressions.py::test_cli_license_commands_wait_on_the_request_verdict`
   (source-level, runs today) and end-to-end by `test_cli_licensing.py`.
2. **`tst_session_context_lifecycle` (WPH-T10's Verify) is not added**, for the same reason:
   `SessionContext.cpp`'s destructor closure drags all nine modules, and the adopt/create surface
   is private with no test double. The K4 contract is pinned by
   `test_cpp_regressions.py::test_startup_failure_runs_the_same_teardown_ladder`, which asserts the
   single ladder, its order, and that no `EXIT_FAILURE` escapes the session scope.
3. **`tst_trial_state` (WPH-T11) is not added** — same link web plus the constructor's network
   fetch. `tst_commercial_token` covers the token slot the trial shares.
4. **`tst_think_tag_splitter.cpp`** already exists and is already registered; left untouched.

## Patches for the coordinator

### P1 — REQUIRED with this branch: the model-facing autosave promise is now false

`project.save`'s own description still tells the model the runtime auto-saves to disk after every
mutating call. With J2 in, nothing saves the document but the user or `project.save`, so a model
reading this will never save the user's work. Files outside WP-H (WP-F / WP-J own them):

`app/src/API/Handlers/ProjectFileCommands.cpp` (~L209), replace the description of
`project.save`:

```cpp
    QStringLiteral("Write the current project to disk. The AI runtime does NOT write the "
                   "project file: a successful mutating tool call takes a checkpoint, not a "
                   "save. Call this when the user asks to save, or pass {filePath: \"/abs/path\"} "
                   "for a headless save-as."),
```

Same file (~L183), `project.setTitle` description: replace
`"the .ssproj file on disk; auto-save still writes to the existing file path."` with
`"the .ssproj file on disk; the title reaches disk on the next save."`

`app/rcc/ai/skills/project_basics.md` (~L123-134), the "## The auto-save loop" section: it must
say that a successful mutating call schedules a debounced **checkpoint** (recoverable through
`assistant.restore`), that the project file changes only on `project.save` or a user save, and
that the assistant should call `project.save` when the user asks for their work to be saved.
The "Don't call project.save after every edit" line stays true for the batch case only.

Also imprecise but not behaviour-changing (wording only): "suspended-autosave window" in
`ProjectBatchCommands.cpp`, `WorkspacesHandler.cpp`, `AssistantHandler.cpp`,
`ProjectListCommands.cpp`, `app/rcc/ai/skills/tool_discovery.md` — the window now suspends the
checkpoint debounce.

### P2 — `tests/README.md` rows (WP-J owns the file)

Add to the C++ unit-test table: `tst_sse_event_reader`, `tst_redactor`, `tst_sentinel_probe`,
`tst_file_sandbox`, `tst_reply_state_machine`, `tst_conversation_turn`, `tst_extension_installer`,
`tst_simplecrypt`, `tst_monotonic_clock`, `tst_commercial_token`, `tst_machine_id`; and to the
integration table: `test_assistant_autosave.py`, `test_extension_install.py`,
`test_cli_licensing.py` (needs `SS_BINARY`).

### P3 — Census re-seeds this branch needs

* `python scripts/code-verify.py --tu-census --accept` — `app/src/AI/Conversation.cpp`
  1513 -> 1569. The async-tool lane was extracted into `AsyncToolRunner` to hold this down;
  the rest is the checkpoint change, the shared resume guard and the split `finishToolCall`.
  WPI-T6 splits this facade and supersedes the re-seed. `ExtensionManager.cpp` was brought back
  to exactly 1500 rather than re-seeded.
* `python scripts/code-verify.py --singleton-census --accept` — **+1 total**
  (`ExtensionManager.cpp` 4 -> 5): `ProblemCenter::instance()` for the catalog checker, at
  wiring time. The other new reach (BackupManager) was avoided by routing the checkpoint through
  the dispatcher instead. If the freeze is strict, the alternative is dropping the Problem Center
  finding and reporting only through `installFailed` + `lastError()`.

### P4 — Files I edited that are shared or outside my list

* `app/src/Misc/CLI.cpp` **and `CLI.h`** — CLI.h was unavoidable (`--api-token-file` is an option
  in `CliOptions`, and `resolveApiToken`/`registerCommercialOptions` are members). WP0's QML
  selftest hook and WP-G's `--api-port` land in the same two files: my edits are confined to the
  option struct, `registerOptions` (which now ends with `registerCommercialOptions();`), the
  `--reset` branch, `applyApiServerOptions`, and the two license functions.
* `app/src/Misc/Utilities.{h,cpp}` — new `postMessageBox`. Not on any package's file list; it is
  the seam the three K13 sites (LemonSqueezy, Trial, ExtensionManager) share, and WP-C/WP-D have
  the same hazard in their drivers, so it is worth keeping shared rather than three local statics.
* `app/src/Misc/CrashTracker.h`, `app/src/AI/{Conversation,Assistant}.h`,
  `app/src/AI/Providers/{Provider,LocalProvider,*Reply}.h`, `app/src/Misc/ExtensionManager.h`,
  `app/src/Misc/Extensions/Extension{Catalog,Installer}.h` — headers of .cpp files my tasks list.
* `app/CMakeLists.txt`, `app/rcc/rcc.qrc` — two source rows and one resource row appended.
* `tests/scripts/test_cpp_regressions.py` — seven appended cases (the file is the sanctioned home
  for logic-only regressions and is the only tier I can run).
* **Not touched:** `app/src/Licensing/MachineID.cpp` (WP-C's), `MonotonicClock.cpp` (WP-E's).

## Invariants found that the plan did not state

1. **A `Reply` must not emit before its caller connects.** `Provider::sendMessage` returns the
   reply and `Conversation::issueRequest` connects afterwards, so the new transport refusal in
   `OpenAIReply::issueRequest` had to be posted with `QTimer::singleShot(0, ...)` — the
   `ImmediateErrorReply` idiom. Emitting inline would have hung the turn with `busy` latched true.
2. **`LemonSqueezy::busy` had no owner on a failed request.** The old code cleared it only inside
   `clearLicenseCache()`. Since a refused deactivation must now keep the cache, `finishRequest()`
   clears `m_busy` itself; otherwise the licensing UI stays busy forever.
3. **`ExtensionManager::addRepository` is API-reachable** (`extensions.addRepository`), so its
   refusal message must be queued, not modal: an API client would otherwise block until a human
   clicked. Same class as R5.5, one package over.
4. **`restoreRunningPlugins` reaches a modal from a QNetworkReply stack** through
   `loadingChanged` (emitted inside `onManifestReply`) -> `launchPlugin` -> "API Server Required".
   The plan listed ExtensionManager under K13 without naming this path.
5. **The AI corpus and the API command descriptions carry the autosave promise** (P1). A
   behaviour change to the assistant's disk contract is incomplete until the strings the model
   reads change with it.
6. **`command_safety.json` already had `assistant.checkpoint` as Safe and `project.save` as
   Confirm**, so R9.3 needed no tier change — only the timer's target.

## Counterfactual self-check

**Which rule does this diff most risk violating?** The startup contract: `SessionContext::shutdown()`
must run with `qApp` alive, after the QML engine dies, and never from a destructor (INV-6) — and
`main.cpp` is the only place that holds it.

**Evidence it does not.** `shutdownSession()` is called from `runApplication` **after** the block
that owns `Misc::ModuleManager` (which owns the `QQmlApplicationEngine`) has closed, and **before**
`QApplication app` leaves scope — the same two boundaries the previous code sat between; the diff
moved the ladder into a function and added one caller, it did not move it relative to either
object. The failed-bootstrap path now returns a status out of `runConfiguredSession` instead of
returning out of `runApplication`, so it reaches the same ladder. `stopFrameConsumerWorkers()` was
added ahead of the context release because `aboutToQuit` never fires when `exec()` never ran; it is
documented idempotent and is called in the same position as in `CLI::teardownHeadlessSession`.
`test_cpp_regressions.py::test_startup_failure_runs_the_same_teardown_ladder` asserts one
`shutdown()` call, one `shutdownSession()` call, the four steps in order, and that no
`"Critical QML error"` return path remains inside `runApplication` — and it passes.

**Runner-up risk:** the new worker lane (J3) touching GUI-owned state. `AsyncToolRunner` calls only
`ToolDetail::executeFsTool`, whose reachable state is `FileSandbox` (own mutex) and
`WorkspaceManager::path()` (a `QString` read on an object constructed long before, in the pinned
order); every result crosses back with `Qt::QueuedConnection` and is dropped unless the turn
generation still matches. Nothing on the frame hotpath is touched by this package at all.
