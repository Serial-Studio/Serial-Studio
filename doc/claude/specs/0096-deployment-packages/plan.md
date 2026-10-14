---
spec: 0096-deployment-packages
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks) (approved 2026-10-13)
updated: 2026-10-13
---

# Plan 0096 — Deployment packages

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A `.ssdeploy` file is a small binary container: a CBOR header (format, minimum app version,
writer version, digest, and reserved phase-2 slots), followed by a payload that is
CBOR-encoded, then XOR-scrambled with a non-secret keystream. v1 does not compress (amended
2026-10-13: `qUncompress` treats the stored size as a hint and can grow without bound, which
R10 forbids). The payload holds the
project as compact JSON, the files it carries, the runtime options, and the credentials the
author included. The codec is pure QtCore and lives in `core/Core`. At startup, before the
composition root runs, a package argument makes the root decode the package and close a
process-wide **one-way seal** (`Core::Runtime::seal()`). It then mounts the carried files as
an in-memory Qt resource tree, built by our own writer, so alarm sounds, Canvas images,
certificates and bundled widget extensions resolve through the `:/` paths they already
accept. Finally it loads the project with the existing path-less
`ProjectModel::loadFromJsonDocument`. Every exit for project content consults the seal at its
single choke point:
- `CommandRegistry::execute`, which applies a default-deny table,
- the mirror's structure push,
- backups,
- the historian's stored project,
- project load and save,
- extension and plugin loading,
- the assistant.

## Affected subsystems & files

Grep-confirmed touch points. Commercial-only unless marked.

| File | Change |
|------|--------|
| `core/Core/Runtime.{h,cpp}` (both builds) | `sealed()` / `seal()`: an atomic one-way latch beside `benchmarkActive()`. Never unsealed, and always false in the GPL build. |
| `core/Core/Deploy/PackageFormat.h` (new) | Magic, format version, header keys, and the fixed caps (64 MiB file, 64 KiB header, 128 MiB decoded, 4096 entries, 10 MiB project JSON). |
| `core/Core/Deploy/PackageCodec.{h,cpp}` (new) | `Deploy::Package` struct plus encode/decode. Decoding is bounded and verifies the digest before any CBOR parse; refusals carry typed reasons (corrupt, too large, needs app X.Y.Z, unknown encoding). |
| `core/Core/Deploy/ResourceTree.{h,cpp}` (new) | Builds an RCC-format tree in memory from the decoded entries, owns the bytes, and registers them once under a per-session mount root. |
| `core/Core/CMakeLists.txt` | Commercial list gains `Deploy/*`. |
| `core/Core/Crypto/CredentialVault.{h,cpp}` | Adds a transient scope (RAII, GUI thread). Stores made inside it go to an in-memory overlay and mark the key package-owned; later stores to a package-owned key stay in memory; lookups check the overlay first. |
| `core/Pipeline/DataModel/Project/ProjectSealing.{h,cpp}` (new, both builds) | `stripSealedContent(QJsonObject&)` plus a `DataModel::Frame&` overload for frame templates (amended 2026-10-13), the one home for "what a sealed copy omits": every parser, transform, Canvas, control-script and publisher-script field, transform params and libraries, Password-typed connection properties, and certificate/key paths. Used by the mirror and the historian. |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | Once a sealed document has loaded, any further `openJsonFile` / `loadFromJsonDocument` is refused (a sealed process holds exactly one project). |
| `core/Pipeline/DataModel/Project/ProjectPersistence.cpp` | Save, Save-As and autosave refuse when sealed. |
| `core/Pipeline/AppState.cpp` | `restoreLastProject()` returns early when sealed. Kept out of `ModuleManager.cpp`, which is at 1471 of 1500 lines. |
| `core/Pipeline/Platform/AppPlatform.cpp` | Windows HKCU association: `.ssdeploy` → `SerialStudio.ssdeploy` → `"exe" --deployment "%1"`, next to the existing `.ssproj` writer. |
| `core/Api/API/SealedPolicy.{h,cpp}` (new, both builds) | Two sorted, compiled name tables (allowed, denied) and `allowedWhenSealed(name)`. A name in neither table is denied. |
| `core/Api/API/CommandRegistry.cpp` | First statement of `execute()`, ahead of the scope gate: when sealed and not allowed, return a stable `SEALED_SESSION` error. **Protected surface** (API dispatch). |
| `core/Api/API/PathPolicy.cpp` | When sealed, refuses any path parameter under the mount root, so an allowed file-taking command cannot read a carried file back out. |
| `core/Api/API/Mirror/MirrorPublisher.cpp` | When sealed, the structure payload (`:396`) embeds the stripped project. |
| `core/Storage/Sessions/Export.cpp`, `DatabaseManager.cpp` | When sealed, store the stripped project (`Export.cpp:1192`, `DatabaseManager.cpp:1133`) and write `sealed=1` into `project_metadata`. Two or three lines each, because both files are near the TU cap. |
| `core/Api/API/Server/ServerWorker.cpp`, `app/src/API/GRPC/GRPCServer.cpp` | When sealed, `setTemplateFrame` stores a struct-stripped copy of the frame template. Structure rate, never per sample. These templates feed the per-frame TCP broadcast and gRPC `StreamFrames`, which would otherwise carry every script field to any connected client (amended 2026-10-13). |
| `core/Ui/ApiHandlers/DashboardHandler.cpp`, `core/Api/API/MCPHandler.cpp` | When sealed, `dashboard.getData` and the MCP `serialstudio://frame/*` resources (which bypass the registry) strip their serialized frame (amended 2026-10-13). |
| `core/Storage/MQTT/PublisherWorker.cpp` | When sealed, the published frame JSON is stripped (amended 2026-10-13). |
| `core/Storage/Sessions/Verifier.cpp`, `VerifierRegression.cpp` | Refuse a `sealed=1` recording with "sealed recording: verify/regress needs the project's scripts". |
| `core/Ui/UI/WidgetExtensions.cpp` | When sealed, skip the user extension folder and scan only the mount's `extensions/`. Package extensions get a new `fromPackage` flag: they need consent and have host context names hidden, like user-installed packages, but load through the `qrc` URL branch. |
| `core/Ui/Misc/PluginRunner.cpp` | Refuse to launch, and to restore running plugins, when sealed. This is the launch site, chosen to stay out of `ExtensionManager.cpp`, which is already over the cap. |
| `core/Ui/AI/Assistant.cpp` | Refuse to start a turn or dispatch a tool when sealed. This covers its `fs.*` tools, which bypass the registry. |
| `core/Ui/UI/Widgets/PainterContext.cpp` | When sealed, `resolveImagePath` maps a relative path, or an absolute path under the recorded original project folder, to the mount. |
| `core/Ui/Misc/DeploymentPackager.{h,cpp}` (new) | A member sub-object of `ShortcutGenerator`, not a singleton. It collects the project, sounds, Canvas literal image paths plus author-added files, used `ext:` packages, certificate/key files and credential candidates, then validates (R2, R3) and writes through `PackageCodec`. |
| `core/Ui/Misc/ShortcutGenerator.{h,cpp}` | Owns the packager and exposes its slots and models. Its shortcut writer gains a `--deployment <file>` argument mode. The per-OS writers are unchanged. |
| `app/src/Misc/DeploymentSession.{h,cpp}` (new) | Root-owned; lives in `runApplication`'s frame. It reads the package, refuses with a dialog on failure, seals, mounts, sets the window icon and display name, and sets `QML_DISK_CACHE=aot,qmlc-read` (unsetting `QML_FORCE_DISK_CACHE`). After composition it loads the project, applies the included credentials inside the vault's transient scope, and hands the runtime options to `CLI`. |
| `app/src/Misc/CLI.{h,cpp}` | Adds `--deployment <file>` and treats a positional `*.ssdeploy` the same way. `runtimeMode()` is true for a deployment. `applyOperatorRuntimeSettings` reads one options struct filled from the parser or the package. A conflicting non-bus flag logs a notice and is ignored. `--headless` combined with a deployment is refused. |
| `app/src/main.cpp` | Calls `DeploymentSession` between `CLI::process` and `bootstrapModuleManager`. Loads the package's project at the existing CLI project step (`:163`). |
| `core/Ui/Platform/FileOpenEventFilter.cpp` | macOS: a `.ssdeploy` open event starts `<own exe> --deployment <file>` detached. If this process is unsealed and was launched only to open that file, it then quits. |
| `app/qml/Dialogs/ShortcutGenerator.qml` | Output selector (Package or Project shortcut). In Package mode, a Contents tab lists the carried files, external dependencies, "Add files…", and the credential picker with the plain warning, plus an "Also create a desktop shortcut" option. |
| `app/qml/main.qml`, `app/qml/MainWindow/MainWindow.qml` | Runtime-mode guards on `showMacros()` and `showProblemCenter()`, the two missing ones, which also closes those gaps for ordinary runtime sessions. Window title comes from `CLI_DEPLOYMENT_TITLE` when set. |
| `app/deploy/macOS/info-pro.plist` | `CFBundleDocumentTypes` (role Viewer) plus `UTExportedTypeDeclarations` for `com.serial-studio.ssdeploy`. |
| `app/deploy/linux/serial-studio-pro.desktop`, `serial-studio-ssdeploy.xml` (new), `app/CMakeLists.txt` | Adds the MIME type and installs the XML on commercial builds. |
| `.github/workflows/ci.yml` | MSIX manifest: `uap:FileTypeAssociation` for `.ssdeploy`, since HKCU writes are virtualized there. |
| `app/tests/tst_deploy_package_codec.cpp`, `tst_deploy_resource_tree.cpp`, `tst_sealed_policy_registry.cpp`, `tst_sealed_strip.cpp`, `app/tests/CMakeLists.txt` | Unit tier (see the test plan). |
| `tests/fixtures/deploy/marker.ssproj`, `marker.ssdeploy`; `tests/integration/test_sealed_session.py` | Integration tier. The fixture is regenerated by the codec ctest with `SS_DEPLOY_WRITE_FIXTURE=<path>`. |
| `doc/help/Operator-Deployments.md`, `doc/help/Command-Line-Interface.md` | User docs: wording is "encoded" and "sealed", and Pro content says "license or trial". |
| `CLAUDE.md`, `doc/claude/architecture/io.md` | A Subsystem Contracts row stating that a new API command needs a `SealedPolicy` classification, a new exit for project content consults the seal, and sealed copies go through `stripSealedContent`. |

## Architecture & data flow

**Startup with a package (GUI thread, before the pipeline exists):**
1. `main` → `CLI::process`. It sees `--deployment X`, or a positional `X.ssdeploy`.
2. `DeploymentSession::open(X)` decodes the package in this order:
   - size and magic checks;
   - bounded header parse;
   - `minApp` and encoding checks;
   - descramble (the decoded payload is exactly as long as the stored one);
   - SHA-256 compare;
   - CBOR parse.

   Any refusal shows a dialog and exits non-zero (R10, R11).
3. On success:
   - `Core::Runtime::seal()`
   - `ResourceTree` registers under `:/ssdeploy-<16 hex of digest>`
   - `setWindowIcon` / `setApplicationDisplayName` (before any window, because the custom title bar reads the icon once)
   - the QML cache variables are set
4. `bootstrapModuleManager` runs. Ephemeral is on because `runtimeMode()` is true, and `restoreLastProject()` returns early because the session is sealed.
5. The API server may start from the user's settings. From its first command, `execute()` already sees the seal.
6. At the CLI project step:
   - `DeploymentSession::loadProject()`: `JsonValidator::parseAndValidate`, then `loadFromJsonDocument(doc, "")`. The empty path disables autosave, the file watcher and the migration write. Old writer versions migrate as usual.
   - `CredentialVault::TransientScope` is opened, and each included credential goes through the UI driver's `setDriverProperty`.
   - `applyOperatorRuntimeSettings(packageOptions)` → `applyBusConfiguration()` (bus overrides, R7) → the existing queued auto-connect. Its license and trial gate is unchanged (R12).

**Mount and package identity.** A sealed process holds one package, so the mount root is the
fixed `:/ssdeploy-package` (amended 2026-10-13: no per-package id and no placeholder rebinding).
Files the project references by project-relative path live under `project/…`; sounds outside the
project folder under `external/…`, certificates and keys under `certs/…`, extensions under
`extensions/<id>/…`. The packager writes the final `:/ssdeploy-package/…` paths into the stored
project. Canvas code is never rewritten; `PainterContext` maps literal paths onto `project/` at
draw time instead.

**Sealed exits, one choke point each:**
- **API, gRPC, MCP, scripts, assistant forwards, `project.batch`:** `CommandRegistry::execute`. The seal check runs before the remote/Trusted scope logic, so a Trusted script origin cannot pass it.
- **Mirror:** subscription bypasses the registry, so `MirrorPublisher` strips the structure itself.
- **Live frames** (amended 2026-10-13): the TCP per-frame broadcast and gRPC `StreamFrames` templates, `dashboard.getData`, the MCP frame resources, and the MQTT publisher's frame JSON each strip at their own site.
- **Backups:** `DeploymentSession` calls the existing `BackupManager::setEnabled(false)`.
- **Historian:** strip at its two writers.
- **Files read back out:** `PathPolicy`.
- **Disk caches:** the QML environment variables.
- **Code from outside the package:** `WidgetExtensions` (extensions) and `PluginRunner` (plugins).
- **Assistant:** `Assistant`.
- **A second project or a save:** `ProjectLoader` and `ProjectPersistence`.

**Packages from a running app.** Opening a package from a running normal app (macOS open event)
always starts a new process, because a live session is never sealed after the fact.

## Hotpath & threading impact

- **Touches the hotpath?** No. No `FrameReader`, `CircularBuffer`, `FrameBuilder`, `Dashboard`
  draw or span-lane code changes. Decoding and mounting happen once at startup on the GUI thread.
  The project load is the existing CLI-path load.
- **New cross-thread signal/slot?** No. Every new check is a GUI-thread read of one relaxed
  atomic at command, structure-change or session-start rate.
- **New input to a cached hotpath flag?** No. The seal never changes after startup, and nothing
  on the frame path reads it.
- **Timestamp ownership:** unchanged.
- **Benchmark:** `--benchmark-hotpath` is unaffected (AC12). Its bootstrap never seals.

## Data model & persistence

- **`.ssproj` is unchanged.** No `Frame.h` keys, no schema or writer bump.
- **New on-disk format: `.ssdeploy` v1.**
  - Layout: `"SSDEPLOY"` magic (8 bytes) | `u16` container version | `u16` reserved | `u32` header length | CBOR header | payload.
  - Header keys:
    - `fmt` = 1
    - `minApp` (string)
    - `writer` (string)
    - `enc`: 1 means scramble-v1, uncompressed
    - `nonce` (16 bytes)
    - `plainSize`
    - `digest` (SHA-256 over every other header field and the decoded payload, each
      length-prefixed, so a flipped bit anywhere the decoder trusts is refused; amended
      2026-10-13 for AC3). The `u16` reserved field must be zero.
    - `grant`: reserved, absent in v1
  - Unknown header keys are ignored. An unknown `enc` value, or `fmt` above 1, refuses with the `minApp` message. Phase 2 adds `enc` = 2 (AEAD) and `grant` without changing v1 parsing (R6).
  - Payload, after decoding: a CBOR map holding `manifest` (title, PNG icon, runtime options, original project folder, external dependencies), `project` (compact JSON bytes), `files` (array of {path, bytes}) and `credentials` (array of {source index, property, value}).
- **Scramble-v1** is not a cipher. The keystream is xoshiro256\*\* seeded from SHA-256 of a
  fixed label and the nonce. It exists only so that no string appears verbatim (R5).
  Compression can come back with phase 2's AEAD encoding, through a bounded inflate.
- **Historian:** `project_metadata` gains `sealed` = 1 for sealed recordings. There is no schema
  migration, because the table is key/value.

## API / SDK surface

- **No new command.** Behavior change: in a sealed process every command that is not in
  `SealedPolicy`'s allowed table returns the `SEALED_SESSION` error code. That code goes into
  the error-metadata map so SDKs see it as stable.
- **Classification rules for the 446 commands, reviewed by hand in the tasks phase:**
  - **Denied:** all of `project.*`, `scripts.*`, `controlScript.*`, `assistant.*`, `extensions.*`, `system.*`, `sessions.*`, `csvPlayer.*`, `mdf4Player.*`, `mirror.getStructure`, `influx.*`, and every `io.*` getter that returns a credential or certificate path.
  - **Allowed:** live-data and device commands (`dashboard.*` reads, the remaining `io.*`, console, alarms, notifications, exports per runtime flag, `licensing.*`, diagnostics).
- **Scripts:** a project script's `apiCall` reaches the same gate, and a refused call returns the same error to the script.

## QML / UI

- **Deploy dialog:**
  - A two-way output selector is added to the existing General tab, defaulting to Package.
  - Package mode adds a **Contents** tab, fed by the packager's list models:
    - carried files (path, size), with "Add files…" for Canvas images the scan cannot see;
    - external dependencies (read-only);
    - credentials and keys (checkbox per item, unchecked by default), with a warning shown while anything is checked: "Packages are encoded, not encrypted. Anyone with the file can recover the items you include."
  - Save writes `.ssdeploy`, and optionally a shortcut through the existing writers with `--deployment`.
  - File-dialog completion is deferred with `Qt.callLater` (as today), because of the macOS `fileSelected` reentrancy rule.
- **Runtime:** no new component. Two `show*()` guards and the title binding are added.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How carried files are served | (a) In-memory RCC tree. (b) A byte API per consumer (WAV, image, PEM, QML `setData`). (c) Extract to a private temp folder. | **(a).** Every consumer already accepts `:/` paths, so this means zero consumer changes. Multi-file QML extensions resolve relative imports. (b) cannot load multi-file QML synchronously. (c) breaks R16. Qt's provenance warning does not apply, because the tree is written by our code from validated entries, never mapped from file bytes. |
| Container | (a) CBOR header + payload. (b) Zip. (c) `QDataStream`. | **(a).** QtCore only, and the header is extensible by key, which is what R6 needs. Zip is a recognizable format that invites "rename and unzip". Untrusted `QDataStream` input has weaker length guarantees. |
| Scramble strength | (a) A non-secret keystream. (b) A real cipher with a key built into the app. | **(a).** The maintainer chose "encoded, encryption in phase 2". A real cipher with a built-in key would invite calling packages encrypted, which the honesty constraint forbids. |
| Where command classification lives | (a) Compiled tables + a ctest sweep. (b) A JSON manifest. (c) A required argument on all 448 `registerCommand` calls. | **(a).** Follows the `PathPolicy` / `tst_path_policy_registry` precedent. It cannot fail to load, and the review happens in one file. (c) is the most forget-proof but is a 448-site mechanical diff. Default-deny makes a missed classification fail closed. |
| Mirror in a sealed session | (a) Stripped structure. (b) Refuse subscribe. | **(a).** Viewers keep working and the strip function already exists for the historian. Cost: Canvas widgets render as placeholders on viewers. |
| UI gating for R13 | (a) Guards per surface in `show*()` + refusals in the model. (b) A `sealed` field on about 40 UI command manifest entries + registry filtering. | **(a).** Every R13 surface opens through one `show*()` function, and save/open are refused in the model whatever the caller. (b) duplicates that, and only matters for palette cosmetics that runtime bindings already hide. |
| Included credentials | (a) A transient vault scope (memory-only for keys from the package). (b) Write them to the vault like operator-entered ones. | **(a).** R16 forbids settings entries holding package content. Operator-entered credentials still persist as today (R4). |
| Where the seal lives | (a) `Core::Runtime`. (b) An `AppState` flag. | **(a).** It is visible to every layer, including Devices, which cannot see Pipeline. `AppState`'s ephemeral flag is mutable and the benchmark flips it. |
| macOS double-click | (a) Spawn from the open event, quitting if launched for it. (b) Pump events before composition to catch the event early. | **(a)**, with (b) as an early spike. Qt delivers the open event after the root is composed, so a cold launch would otherwise compose a normal session first. Detecting "launched only to open the file" is the spike's question. |
| Integration fixture | (a) Generated by the codec ctest and committed. (b) A Python reference codec (the mirror precedent). (c) A new `--build-deployment` CLI flag. | **(a).** (b) would publish a ready-made decoder for phase-1 packages. (c) adds a user-facing flag the spec never asked for. |

**Assumption checked:** "sealing is worth doing before encryption". A phase-1 file can be
decoded by anyone who reads the format, so the seal adds little confidentiality until phase 2.
It stays in phase 1 because the spec requires it (R13–R18), because it gives demo integrity
(the operator cannot wander into editing), and because phase 2's grant needs the same seal: a
grant without it could be turned into Pro for any project by editing the live session.

## Risks & mitigations

- **A project-content exit is missed (leak).** Mitigations:
  - default-deny table plus CI sweep (AC6);
  - one home for the strip function;
  - the marker fixture puts a marker in every code and credential field, and AC7 searches every app data folder for it.
  - **Silent class:** a future feature adds a new exit. The CLAUDE.md row and the `SealedPolicy` sweep catch API additions; a new non-API exit is caught only by AC7 being re-run. Named debt.
- **Sealing too broadly breaks integrations.** AC5 asserts live reads and device commands still work. Every denied `io.*` getter is named in the table with its reason.
- **RCC writer correctness.** The format is Qt-internal but stable (Qt still loads Qt 4-era `.rcc` files). Mitigations:
  - `tst_deploy_resource_tree` reads back through `QFile`, `QDir` listing and a `QQmlComponent` over a `qrc` URL;
  - the writer is spiked first in the tasks;
  - fallback if it proves fragile: option (b) for sounds, images and certificates, with extensions limited to a single file.
- **`QML_FORCE_DISK_CACHE` in the operator's environment would force cache writes.** It is unset in sealed processes.
- **TU caps.** `ModuleManager` (1471), `ExtensionManager` (1505), `Export` (1496), `DatabaseManager` (1419) and `ProjectLoader` (1414) take guard lines only. All logic goes into the new classes.
- **API dispatch is a protected surface.** The `execute()` gate lands in its own task and commit, named before landing, with its two failure modes (too broad, too narrow) tested by AC5 and AC6.
- **macOS cold launch** may briefly show a normal window before the sealed process starts. This is acceptable for (a). The spike decides whether (b) can remove it.
- **Credentials are not prompted on authentication failure** (MQTT and OPC UA just fail). The operator reaches the password through the existing runtime reconfigure prompt. Verified manually under AC9.
- **Canvas literal scan** misses computed image paths. Mitigation: "Add files…", plus the Problems view, which already reports `drawImage` failures in the author's own test run.
- **Spec deltas, needing your confirmation:**
  - **R17:** a sealed recording replays its data widgets, but Canvas widgets and anything that needs script source render as placeholders.
  - **R14:** mirror viewers get the same placeholders.
- **Pre-existing gaps, named but not fixed here:**
  - MSIX and AppImage lack the `.ssproj` association too;
  - the Setup-pane plaintext password capture (`UiDriverSync.cpp:253`).

## Test & verification plan

- **Unit (ctest; you can run these against an existing build):**
  - `tst_deploy_package_codec`:
    - AC1: round-trip of project and files;
    - AC2: the marker never appears in the bytes, and an included credential round-trips;
    - AC3: truncation at every offset of a small package, single-bit flips, and oversized header, plain size and entry lengths are all refused, within a test timeout, in the ASan tier;
    - AC4: `minApp` above the current version and an unknown `enc` are refused; an unknown header key is accepted; 64 MiB plus 1 is refused on encode and on decode;
    - the committed fixture decodes.
  - `tst_deploy_resource_tree`: nested folders, Unicode names, 4096 entries, reading back through `QFile`, `QDir` and a `qrc` QML load.
  - `tst_sealed_policy_registry` (AC6): every command in `app/rcc/api/api-schema.json` sits in exactly one table, the tables are sorted and disjoint, and an unknown name is denied.
  - `tst_sealed_strip`: a marker in every code and credential key is absent after stripping, and the group, dataset and widget structure is unchanged.
- **Integration (pytest, maintainer launches `--deployment tests/fixtures/deploy/marker.ssdeploy --api-server`):** `tests/integration/test_sealed_session.py`:
  - AC5:
    - every denied command returns `SEALED_SESSION`;
    - the fixture's parser calls `apiCall("project.exportJson")` and publishes the success flag as a dataset, which reads back false;
    - `dashboard.getData` and an `io.*` write succeed;
    - a mirror subscription's structure has no marker.
  - AC7: after a session with session export on, the backups, settings, historian and temp folders (resolved per OS) contain no marker, and the recording replays.
- **Maintainer observation:**
  - AC8: double-click on Windows, macOS and Linux shows the title and icon, including the macOS Dock icon through `setWindowIcon`; then copy the package alone to a second machine.
  - AC9: unlicensed build: a non-Pro package connects, and a Pro package prompts for activation or the trial. Includes re-entering an excluded MQTT password through the reconfigure prompt.
  - AC10: palette and every shortcut.
  - AC11: user-installed vs bundled extensions.
  - AC13: port override, and project-shortcut mode unchanged.
- **Hotpath:** `--benchmark-hotpath` gates unchanged (AC12).
- **Static:**
  - `scripts/syntax-check.py` on every edited C++ file;
  - `python3 scripts/code-verify.py --check` on touched files;
  - `layer-verify.py` with no new baseline entries;
  - `--tu-census --check` and `--singleton-census --check`, with no growth;
  - `registry-verify.py`;
  - `documentation-verify.py` on the help pages;
  - `qt-cpp-review` before handoff;
  - `sanitize-commit.py` before commit.
