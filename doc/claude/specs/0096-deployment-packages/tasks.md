---
spec: 0096-deployment-packages
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement) (approved 2026-10-13)
updated: 2026-10-13
---

# Tasks 0096 — Deployment packages

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). Milestone order:
> M1 (format) → M2 (sealed exits) → M3 (opening) → M4 (export) → M5 (file association) →
> M6 (tests and docs). Each milestone leaves the tree coherent. Every C++ task's Verify
> includes `python3 scripts/syntax-check.py <files>` and
> `python3 scripts/code-verify.py --check <files>`, which are not repeated below. A new TU is
> missing from the compile database until the maintainer reconfigures, so its syntax-check
> SKIPs, and the handoff says so.

## Conventions

- One task = one focused, reviewable change. If a task touches more than 3 files or needs a
  paragraph to describe, split it.
- **Deps** lists task IDs that must land first.
- **Main session** marks a protected surface or security judgment that is never delegated
  (CLAUDE.md "Delegation & Cost"). Unmarked tasks may run in a Sonnet subagent from a
  half-page brief.
- Commercial-only code sits behind `BUILD_COMMERCIAL` (CMake lists, `#ifdef` in shared TUs).
  The GPL build must compile unchanged after every task.

## M1 — Format and codec

### T1 — Seal latch

- **Files:** `core/Core/Runtime.h`, `core/Core/Runtime.cpp`
- **Does:** Adds `Core::Runtime::sealed()` and `seal()`: a file-static `std::atomic<bool>`,
  with a release store and relaxed loads, beside `benchmarkActive()`. *Invariant:* the seal is
  one-way. There is no unseal and no setter that takes a bool. A repeat `seal()` is a no-op. It
  constructs nothing (a plain flag, so the census is unchanged), and the GPL build compiles it
  but never calls `seal()`.
- **Verify:** `code-verify.py --singleton-census --check` shows no growth; grep shows no
  `seal(false)`-style API.
- **Deps:** none · **Main session**
- [x] done

### T2 — Package format and codec

- **Files:** `core/Core/Deploy/PackageFormat.h`, `core/Core/Deploy/PackageCodec.{h,cpp}`,
  `core/Core/CMakeLists.txt`
- **Does:** Implements v1 exactly as `plan.md` "Data model & persistence" defines it: header
  CBOR, scramble-v1 (xoshiro256\*\* seeded by SHA-256 of label and nonce), no compression
  (amended 2026-10-13), and a SHA-256 digest. `decode()` checks, in order: size cap, magic,
  bounded header parse, `minApp`, `fmt` and `enc`, payload length equal to `plainSize`, the
  digest, then the CBOR payload parse with entry, path and length caps. It returns a typed refusal.
  *Invariant:* every loop and length has a fixed bound (Power of Ten), nothing is allocated
  from an unchecked declared length, and nothing touches a singleton, because `CLI::process`
  calls it before the composition root.
- **Verify:** T4 tests; read-back that the `Deploy/*` files sit only in the commercial list.
- **Deps:** none
- [x] done

### T3 — In-memory resource tree (spike first)

- **Files:** `core/Core/Deploy/ResourceTree.{h,cpp}`, `core/Core/CMakeLists.txt`
- **Does:** Writes an RCC v2 tree (with Qt's resource name hash) from `{path, bytes}` entries
  into an owned `QByteArray`, then registers it once under `:/ssdeploy-<id>`. Spike this
  before the rest of M2: if Qt rejects the tree or `qrc` QML imports fail, stop and switch to
  the plan's fallback (b) before building on it. *Invariant:* the owned bytes stay alive for
  the life of the process (never unregistered), and the tree is built only from entries the
  codec already validated, never from raw file bytes.
- **Verify:** T5 test.
- **Deps:** none · **Main session** (spike decision)
- **Spike result (2026-10-13):** a line-for-line Python port of `build()`, registered through
  PySide6 6.11.1, read 4096 entries back byte-identical (nested and Unicode paths), listed
  folders through `QDir`, and loaded a two-file QML component over `qrc` (sibling type
  resolved). Option (a) stands; the fallback is not needed.
- [x] done

### T4 — Codec unit test and fixture hook

- **Files:** `app/tests/tst_deploy_package_codec.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Covers the cases below, and writes `tests/fixtures/deploy/marker.ssdeploy` from
  `marker.ssproj` when `SS_DEPLOY_WRITE_FIXTURE` is set; otherwise it checks that the
  committed fixture decodes.
  - **AC1:** round-trip of the project and the files.
  - **AC2:** the marker never appears in the encoded bytes, and an included credential
    round-trips.
  - **AC3:** truncation at every offset of a 4 KiB package, a single-bit flip at every byte,
    and oversized header, plain-size and entry lengths are all refused under the test timeout.
  - **AC4:** `minApp` above the current version and an unknown `enc` are refused with the
    version named; an unknown header key is accepted; 64 MiB + 1 is refused on encode and on
    decode.
- **Verify:** `ctest -R tst_deploy_package_codec` against an existing build dir, once the
  maintainer has built it.
- **Deps:** T2
- [x] done

### T5 — Resource tree unit test

- **Files:** `app/tests/tst_deploy_resource_tree.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Covers nested folders, Unicode names and 4096 entries. Reads back through `QFile`
  and a `QDir` listing, and loads a two-file QML component (one imports the other) from the
  `qrc` URL.
- **Verify:** `ctest -R tst_deploy_resource_tree`.
- **Deps:** T3
- [x] done

## M2 — Sealed exits

### T6 — Sealed strip function

- **Files:** `core/Pipeline/DataModel/Project/ProjectSealing.{h,cpp}`,
  `core/Pipeline/CMakeLists.txt`
- **Does:** Adds `DataModel::stripSealedContent(QJsonObject&)`, the one home for what a sealed
  copy omits:
  - every parser, transform, transform library, Canvas, control-script and publisher-script
    field;
  - transform params;
  - Password-typed connection properties;
  - certificate and key paths.

  Keys come from the existing `Keys::` names, never from new literals. Compiled in both builds,
  so its callers need no `#ifdef`.
- **Verify:** T7 test.
- **Deps:** none
- [x] done

### T7 — Strip unit test

- **Files:** `app/tests/tst_sealed_strip.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Puts a unique marker in every code and credential key of a fixture project, then
  asserts that none survive the strip and that the group, dataset and widget structure is
  byte-identical otherwise.
- **Verify:** `ctest -R tst_sealed_strip`.
- **Deps:** T6
- [x] done

### T8 — Sealed command policy tables and registry sweep

- **Files:** `core/Api/API/SealedPolicy.{h,cpp}`, `core/Api/CMakeLists.txt`,
  `app/tests/tst_sealed_policy_registry.cpp` (+ `app/tests/CMakeLists.txt`)
- **Does:** Two sorted, disjoint `constexpr` name tables plus `allowedWhenSealed(name)`.
  Unknown names are denied. All 446 commands in `app/rcc/api/api-schema.json` are classified
  using the plan's rules. Each denied `io.*` getter carries a reason comment; the
  `// code-verify off` fence goes around the table only if the linter flags it. The ctest
  (AC6) follows `tst_path_policy_registry`: every schema command appears in exactly one table,
  and the tables are sorted and disjoint.
- **Verify:** `ctest -R tst_sealed_policy_registry`; main-session review of every allowed
  `io.*`, `dashboard.*` and `licensing.*` entry for anything returning project content.
- **Deps:** none · **Main session** for the classification review (a subagent may draft it)
- [x] done

### T9 — Gate in `CommandRegistry::execute` (protected surface)

- **Files:** `core/Api/API/CommandRegistry.cpp`, `core/Api/API/CommandHandler.cpp` (error-code
  map only, if `attachErrorMetadata` needs the new code registered)
- **Does:** The first statement after `hasCommand` refuses with `SEALED_SESSION` when
  `Core::Runtime::sealed()` holds and the name is not allowed.
  *Invariants:*
  - it runs before `RemoteDispatchScope`, so a Trusted origin (scripts, terminal, assistant)
    cannot bypass it;
  - nested `project.batch` re-dispatch passes through it again;
  - unsealed behavior is byte-identical.

  Its failure modes go to the maintainer before landing: too broad breaks integrations, too
  narrow leaks. It lands as its own commit.
- **Verify:** read-back of unsealed `execute()` behavior; AC5 (T30) once the app runs a
  package.
- **Deps:** T1, T8 · **Main session**
- [x] done

### T10 — Path policy refuses the mount root

- **Files:** `core/Api/API/PathPolicy.cpp`
- **Does:** When sealed, a declared path parameter that resolves under any `:/ssdeploy-`
  root is rejected with the existing disallowed-path error.
- **Verify:** `ctest -R tst_path_policy` (existing suites stay green).
- **Deps:** T1
- [x] done

### T11 — Mirror sends the stripped structure

- **Files:** `core/Api/API/Mirror/MirrorPublisher.cpp`
- **Does:** When sealed, the structure payload (`:396`) embeds a stripped copy of
  `serializeToJson()`. *Invariant:* this is a structure-change-rate path, not per frame, so
  `kWireVersion` and the payload shape are unchanged (only field values go missing) and no
  fixture regeneration is needed.
- **Verify:** read-back; mirror-structure assertion in T30.
- **Deps:** T1, T6
- [x] done

### T11b — Live frame exits strip script fields (added 2026-10-13)

- **Files:** `core/Pipeline/DataModel/Project/ProjectSealing.{h,cpp}` (struct overload),
  `core/Api/API/Server/ServerWorker.cpp`, `app/src/API/GRPC/GRPCServer.cpp`,
  `core/Ui/ApiHandlers/DashboardHandler.cpp`, `core/Api/API/MCPHandler.cpp`,
  `core/Storage/MQTT/PublisherWorker.cpp`
- **Does:** Adds `stripSealedContent(DataModel::Frame&)`. When sealed:
  - both `setTemplateFrame` setters store a stripped copy (structure rate);
  - `dashboard.getData`, the MCP frame resources and the MQTT publisher strip the serialized JSON.

  *Invariants:*
  - nothing is stripped per sample on the broadcast path;
  - unsealed output is byte-identical;
  - the dashboard's own frame keeps its Canvas code, so local rendering is untouched.
- **Verify:** read-back; AC5 asserts no marker reaches a raw TCP client or `dashboard.getData`.
- **Deps:** T1, T6
- [x] done

### T12 — Historian stores the stripped project and the sealed marker

- **Files:** `core/Storage/Sessions/Export.cpp`, `core/Storage/Sessions/DatabaseManager.cpp`
- **Does:** At `Export.cpp:1192`, `Export.cpp:586` (the groups array) and
  `DatabaseManager.cpp:1133`, a sealed session stores `DataModel::sealedCopy()`: the stripped copy, marked with a top-level
  `sealedRecording: true` key (amended 2026-10-13: a marker inside the stored JSON instead of a
  `project_metadata` row, because `Export.cpp` has 4 lines left under the cap). *Invariant:* both TUs sit
  near the 1500-line cap, so the change is guard lines only, and `--tu-census --check` shows
  no growth past the baseline.
- **Verify:** `code-verify.py --tu-census --check`; AC7 in T30.
- **Deps:** T1, T6
- [x] done

### T13 — Verify and regress refuse sealed recordings

- **Files:** `core/Storage/Sessions/Verifier.cpp`, `core/Storage/Sessions/VerifierRegression.cpp`
- **Does:** A recording with `sealed=1` is refused with "sealed recording: verify/regress needs
  the project's scripts" and a non-zero exit, before any temp DB is written (R17).
- **Verify:** read-back; maintainer runs `--verify-session` on the T30 recording.
- **Deps:** T12
- [x] done

### T14 — One project per sealed process; no saves

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- **Does:** Once a document has loaded under the seal, further `openJsonFile` /
  `loadFromJsonDocument` calls are refused (logged, return false), and save, Save-As and
  autosave refuse while sealed. *Invariants:*
  - `ProjectLoader.cpp` is at 1414 lines, so this is guard lines only;
  - it never touches undo-scope or `setModified` semantics;
  - unsealed loads are unchanged.
- **Verify:** `--tu-census --check`; read-back.
- **Deps:** T1
- [x] done

### T15 — No last-project restore when sealed

- **Files:** `core/Pipeline/AppState.cpp`
- **Does:** `restoreLastProject()` returns early when sealed. *Invariant:* the composition-root
  order is untouched (no `ModuleManager` edit), and `restoreLastProject` still runs after every
  `setupExternalConnections()`.
- **Verify:** read-back.
- **Deps:** T1
- [x] done

### T16 — Plugins and assistant refuse when sealed

- **Files:** `core/Ui/Misc/PluginRunner.cpp`, `core/Ui/AI/Assistant.cpp`
- **Does:** When sealed, plugin launch and running-plugin restore return early (also without
  writing `RunningPlugins`), and the assistant refuses to start a turn or dispatch a tool,
  which covers its `fs.*` tools that bypass the registry.
- **Verify:** read-back; `ExtensionManager.cpp` is not touched (`--tu-census --check`).
- **Deps:** T1
- [x] done

### T17 — Widget extensions load only from the package

- **Files:** `core/Ui/UI/WidgetExtensions.{h,cpp}`
- **Does:** When sealed, skip the user extension folder and scan `<mount>/extensions/`. Those
  descriptors get `fromPackage`: they require consent and hide host context names (like
  user-installed packages), but load through the `qrc` URL branch. *Invariant:* `qmlUrl()`
  stays the single consent choke point, and bundled app extensions behave exactly as today.
- **Verify:** read-back; AC11 (maintainer).
- **Deps:** T1, T3
- [x] done

### T18 — Canvas images resolve to the mount

- **Files:** `core/Ui/UI/Widgets/PainterContext.cpp`
- **Does:** When sealed, `resolveImagePath` maps a relative path, or an absolute path under the
  package's recorded original project folder, to the mount's `project/` tree. The mount root and
  the original folder come from `Core::Runtime::seal(mountRoot, projectFolder)` (amended
  2026-10-13: the seal records both), never a new singleton reach.
  Unsealed resolution is unchanged.
- **Verify:** read-back; AC1/AC8 observation with the fixture's Canvas image.
- **Deps:** T1, T3
- [x] done

### T19 — Credential vault transient scope

- **Files:** `core/Core/Crypto/CredentialVault.{h,cpp}`
- **Does:** `CredentialVault::TransientScope` (RAII, GUI thread, sealed only):
  - a store inside it writes to an in-memory overlay and marks the key package-owned;
  - a later store to a package-owned key stays in memory;
  - lookups check the overlay first.

  *Invariant:* outside a sealed process the vault's behavior and QSettings layout are
  unchanged. An operator-entered credential for a key the package did not include still
  persists (R4).
- **Verify:** read-back; AC7 search finds no package credential in settings.
- **Deps:** T1 · **Main session** (credential handling)
- [x] done

## M3 — Opening a package

### T20 — CLI deployment option and options struct

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:**
  - Adds `--deployment <file>`, and treats a positional `*.ssdeploy` the same way.
  - `runtimeMode()` is true for a deployment.
  - `applyOperatorRuntimeSettings` takes one `RuntimeOptions` struct, filled from the parser
    or the package.
  - Conflicting non-bus options (`--project`, display or export flags) log a notice and are
    ignored; bus flags and `--api-*` still apply; `--headless` with a deployment is refused.

  *Invariant:* every existing runtime-shortcut invocation produces identical behavior (R19).
- **Verify:** read-back against the generated argument lists in `ShortcutGenerator.cpp`;
  AC13 (maintainer).
- **Deps:** none
- [x] done

### T21 — DeploymentSession

- **Files:** `app/src/Misc/DeploymentSession.{h,cpp}`, `app/CMakeLists.txt`
- **Does:** Owned by the root; lives in `runApplication`'s frame.
  - **`open(path)`:** decodes the package, or shows the refusal dialog. Then it seals, mounts,
    calls `setWindowIcon` and `setApplicationDisplayName`, unsets `QML_FORCE_DISK_CACHE`, and
    sets `QML_DISK_CACHE=aot,qmlc-read`.
  - **`loadProject()`:** `parseAndValidate`, then `loadFromJsonDocument(doc, "")`, then
    (amended 2026-10-13: `BackupManager::snapshot` now checks the seal itself, so the root
    reaches no extra singleton). It applies included credentials inside the vault's
    transient scope through the UI driver's `setDriverProperty`, and rebinds `:/ssdeploy/`
    placeholders to the mount root.
  - **`runtimeOptions()`:** hands the options to `CLI`.

  *Invariants:*
  - `open()` runs before `bootstrapModuleManager` and reaches no singleton; `loadProject()`
    runs at the existing CLI project step, which is after relocation, so the
    `FrameParser`/`FrameBuilder` mutators self-marshal;
  - no new function-static singleton;
  - the window icon is set before any window exists.
- **Verify:** `--singleton-census --check`; read-back of the call order.
- **Deps:** T1–T3, T14, T19, T20 · **Main session** (startup)
- [x] done

### T22 — main.cpp wiring

- **Files:** `app/src/main.cpp`
- **Does:**
  - Constructs `DeploymentSession` after `CLI::process`. A refusal exits non-zero.
  - Sets the `CLI_DEPLOYMENT_TITLE` context property beside `CLI_RUNTIME_MODE`.
  - At the project step, calls `loadProject()` instead of opening a file.

  *Invariant:* `setEphemeralSession(cli.runtimeMode())` already covers sealed sessions, the
  composition order is unchanged, and the session object outlives `SessionContext::shutdown()`.
- **Verify:** read-back; AC8 (maintainer).
- **Deps:** T21 · **Main session** (composition root)
- [x] done

### T23 — QML guards and title

- **Files:** `app/qml/main.qml`, `app/qml/MainWindow/MainWindow.qml`
- **Does:** Runtime-mode guards on `showMacros()` and `showProblemCenter()`, the two missing
  ones; this also closes the gap for ordinary runtime sessions, as named in the plan.
  `documentTitle` uses `CLI_DEPLOYMENT_TITLE` when it is non-empty.
- **Verify:** `code-verify.py --check` on the QML; AC10 (maintainer).
- **Deps:** T22
- [x] done

### T24 — macOS open event (spike, then implement)

- **Files:** `core/Ui/Platform/FileOpenEventFilter.cpp`
- **Does:** **Spike:** can a pre-composition event pump catch the cold-launch open event (plan
  option b)? Record the answer in this task. **Then:** a `.ssdeploy` event starts
  `<own exe> --deployment <file>` detached. If this process is unsealed and was launched only
  for that file, it quits; otherwise it stays. A `.ssdeploy` event in a sealed process is
  ignored.
- **Verify:** AC8 on macOS (maintainer).
- **Deps:** T22
- **Spike result (2026-10-13):** not runnable here. LaunchServices delivers open events only to a
  registered app bundle, and launching the GUI is the maintainer's. Implemented option (a): a
  `.ssdeploy` starts `--deployment` in a new process; the receiving process quits only when it was
  started with no arguments and the event arrived within 5 s (cold double-click). A sealed process
  forwards `.ssproj` to a new process. AC8 on macOS verifies, including whether a normal window
  flashes on a cold launch.
- [x] done

## M4 — Export

### T25 — DeploymentPackager

- **Files:** `core/Ui/Misc/DeploymentPackager.{h,cpp}`, `core/Ui/CMakeLists.txt`
- **Does:** A member sub-object, not a singleton. It collects:
  - `serializeToJson()`;
  - sound paths (`AlarmBand::sound` and the project's `sounds` map);
  - Canvas literal image paths (string literals with an image extension that resolve under
    the project folder) plus author-added files;
  - used `ext:` package folders;
  - certificate and key files;
  - credential candidates (Password-typed properties).

  It rewrites the carried paths to `:/ssdeploy/` placeholders and strips Password-typed
  properties from `connectionSettings`. It validates: R2 (a missing file names it), R3 (the
  external dependency list), and the 64 MiB cap. It writes through `PackageCodec`. It exposes
  list models (carried files, external dependencies, credentials) and an `exportPackage(path,
  options)` slot. Export requires `Core::License::activated()`, as the dialog does today
  (the trial counts).
- **Verify:** read-back; AC1/AC2 through T4's fixture path; AC4 size refusal.
- **Deps:** T2, T6
- [x] done

### T26 — ShortcutGenerator owns the packager and makes package shortcuts

- **Files:** `core/Ui/Misc/ShortcutGenerator.{h,cpp}`
- **Does:** Owns `DeploymentPackager` as a member and exposes it as a `CONSTANT` pointer
  property. `buildArguments` gains a deployment mode (`--deployment <abs .ssdeploy>`, keeping
  `--shortcut-path`). The per-OS writers are unchanged. *Invariant:* project-shortcut mode
  produces byte-identical arguments (R19).
- **Verify:** read-back of both argument lists.
- **Deps:** T25
- [x] done

### T27 — Deploy dialog

- **Files:** `app/qml/Dialogs/ShortcutGenerator.qml`
- **Does:** Adds an output selector (Package by default, or Project shortcut) and a Contents tab:
  - carried files, with "Add files…";
  - external dependencies;
  - credential checkboxes, unchecked by default, with the plain warning shown while any is
    checked.

  Save writes `.ssdeploy`, with an optional desktop shortcut. *Invariant:* file-dialog
  completion stays deferred through `Qt.callLater` (macOS `fileSelected` reentrancy); a `%n` is
  never combined with `.arg()`.
- **Verify:** `code-verify.py --check`; AC2 warning observation (maintainer).
- **Deps:** T26
- [x] done

## M5 — File association

### T28 — Windows: HKCU and MSIX

- **Files:** `core/Pipeline/Platform/AppPlatform.cpp`, `.github/workflows/ci.yml`
- **Does:** Commercial builds register `.ssdeploy` → `SerialStudio.ssdeploy` →
  `"exe" --deployment "%1"` next to the `.ssproj` writer. The MSIX manifest gains
  `uap:FileTypeAssociation` for `.ssdeploy` only; the pre-existing `.ssproj` gap is named, not
  fixed.
- **Verify:** read-back; AC8 on Windows (maintainer).
- **Deps:** T20
- [x] done

### T29 — macOS and Linux declarations

- **Files:** `app/deploy/macOS/info-pro.plist`, `app/deploy/linux/serial-studio-pro.desktop`,
  `app/deploy/linux/serial-studio-ssdeploy.xml` (new), `app/CMakeLists.txt`
- **Does:**
  - **plist:** `CFBundleDocumentTypes` (role Viewer) and `UTExportedTypeDeclarations` for
    `com.serial-studio.ssdeploy`.
  - **Linux:** the desktop file's `MimeType` gains `application/x-serial-studio-deployment`;
    the new XML is installed on commercial builds only. The AppImage mime gap is named, not
    fixed.
- **Verify:** `plutil -lint` on the plist; `xmllint` on the XML; AC8 on macOS and Linux
  (maintainer).
- **Deps:** T20
- [x] done

## M6 — Fixture, integration test, docs

### T30 — Marker fixture and sealed-session integration test

- **Files:** `tests/fixtures/deploy/marker.ssproj`, `tests/fixtures/deploy/marker.ssdeploy`
  (generated by T4), `tests/integration/test_sealed_session.py`
- **Does:** In the fixture, every code, credential and title field carries a marker. Its parser
  calls `apiCall("project.exportJson")` and publishes the success flag as a dataset. The test,
  run against an app launched with `--deployment … --api-server`:
  - **AC5:**
    - every denied command returns `SEALED_SESSION`;
    - the flag dataset reads false;
    - `dashboard.getData` and an `io.*` write succeed;
    - a mirror subscription's structure carries no marker.
  - **AC7:** after a recording, the backups, settings, historian and temp folders (resolved per
    OS) contain no marker.
- **Verify:** `pytest tests/integration/test_sealed_session.py -v` (maintainer launches the
  app). It skips cleanly when the API port is closed.
- **Deps:** T4, T9–T22
- **Note (2026-10-13):** `marker.ssdeploy` was produced by a throwaway Python port of the v1 encoder
  in the session scratchpad (never committed), because the codec ctest is not built yet;
  `committedFixtureDecodes` cross-checks it against the C++ decoder on the first build.
- [x] done

### T31 — User docs

- **Files:** `doc/help/Operator-Deployments.md`, `doc/help/Command-Line-Interface.md`
- **Does:** Documents package export and opening, `--deployment`, the credential warning and
  the bus overrides. Wording is "encoded" and "sealed", never "encrypted" or "secure". Pro
  content says "license or trial" (trial parity).
- **Verify:** `python3 scripts/documentation-verify.py --no-report <files>`; the `trial-parity`
  lint stays clean.
- **Deps:** T27
- [x] done

### T32 — Contributor contract

- **Files:** `CLAUDE.md`, `doc/claude/architecture/io.md`
- **Does:** Adds a Subsystem Contracts row and an io.md "Sealed sessions" section covering:
  - a new API command needs a `SealedPolicy` classification;
  - a new exit for project content consults `Core::Runtime::sealed()`;
  - sealed copies go only through `stripSealedContent`;
  - a sealed process holds exactly one project.

  Named enforcement: `ctest:tst_sealed_policy_registry` and `ctest:tst_sealed_strip`.
- **Verify:** `python3 scripts/claim-verify.py` resolves the new markers.
- **Deps:** T7, T8
- [x] done

## Review round (2026-10-13)

`qt-cpp-review` ran six read-only lenses over the diff. The fixes that landed:

- **Decoder:**
  - The digest is verified before any version claim is believed.
  - An unknown format that claims this app's version is reported as Corrupt.
  - Only the parsed version digits reach the dialog, which renders plain text.
  - Duplicate map keys are refused, and so are oversized unknown text values.
- **Strip:** its fallbacks fail closed.
- **Session:**
  - Credential targets and property names from the package are validated (source id >= 0, Password-typed only, the publisher's three secrets).
  - Credentials are applied one loop pass after the connection manager's queued sync and rebuild.
  - Decoded files, the project and the credentials are freed once used.
  - `UiDriverSync::syncFromSource0` now mirrors a sealed session's source 0 (plan amendment).
- **CLI:** the deployment members sit under `BUILD_COMMERCIAL` (the GPL build no longer warns), and the deployment path is resolved once in `process()`.
- **`seal()`:** no default arguments, an assertion that the root is non-empty, and `noexcept`.
- **Packager:**
  - Stale author-added files are dropped.
  - Include choices survive a rescan of the same project.
  - Carried names are validated at scan time.
  - The 256-file extension cap, short reads and an unreadable custom icon are named errors.
  - The estimated size is cached, and the credential list has its own NOTIFY signals.
  - `Secret` uses designated initializers, and the credential targets are shared constants.

Files touched outside the plan's list:

- `core/Core/Api/CommandProtocol.h`: the `SEALED_SESSION` code.
- `core/Ui/Misc/BackupManager.cpp`: the seal check.
- `core/Devices/IO/ConnectionManager/UiDriverSync.cpp`: the source-0 sync.
- `app/tests/CMakeLists.txt`: link sets for `tst_path_policy_registry` and `tst_server_worker_caps`.

## Definition of Done

- [ ] AC1–AC13 in `spec.md` are met and checked off there (AC8–AC11 and AC13 by maintainer
      observation; AC5 and AC7 by the maintainer's pytest run).
- [x] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `scripts/syntax-check.py` run on every edited C++ file; any SKIP (new TUs before a
      reconfigure) is reported as unverified. (New TUs were checked with a borrowed compile
      entry. Still unverified: `GRPCServer.cpp`, because gRPC is off in this build, and the
      Windows-only registry block in `AppPlatform.cpp`.)
- [ ] `layer-verify.py` passes with no new baseline entries; `--tu-census --check` and
      `--singleton-census --check` show no growth; `registry-verify.py` is clean. (All pass
      except `registry-verify`'s mirror-wire digest: the seal check changed `rebuildStructure`'s
      text, not the wire. Re-seeding without a version bump is the maintainer's call.)
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` gates unchanged (AC12). The hotpath is not touched; this confirms
      it.
- [x] Maintainer pytest targets identified: `tests/integration/test_sealed_session.py`.
- [x] The T9 gate was named to the maintainer with its failure modes before it landed.
- [ ] `python3 scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that*: no scope creep, no foreign files touched. The
      pre-existing gaps (MSIX/AppImage `.ssproj` association, Setup-pane password capture) are
      named, not fixed.
- [ ] `spec.md` status set to `done`.
