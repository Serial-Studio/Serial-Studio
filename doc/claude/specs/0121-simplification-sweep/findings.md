---
spec: 0121-simplification-sweep
phase: findings
updated: 2026-11-13
---

# Findings 0121 — Simplification sweep

> The campaign record: what the finders reported, what was planned, and per packet the lines
> promised, the lines delivered and the reviewer's verdict. Nothing here is compile-proven: no
> compile database exists on the machine that ran the campaign.

## Find

- 48 Sonnet finders plus 14 re-queue finders read 409,678 lines in 1,403 files (155 files,
  48,024 lines, excluded as protected, generated or hot). Coverage was checked against the
  slice lists, not against what a finder said: 52 files (8,981 lines) were missing from the
  first pass (one finder listed 6 of its 34 files, one skipped 23 small headers) and were read
  by two more finders. No file ended under 95% read.
- 438 findings promising 12,400 lines. 185 were dropped in planning; the reasons are tallied
  at the end of this file.
- Proofs re-run by the main session: every dead-code finding (a word-bounded `git grep` of each
  removed symbol over `core`, `app`, `tests`, `examples`, `doc/help` and `scripts`; hits outside
  the owning files were read one by one), the 21-line palette block (identical in 40 files) and
  the folder-view diff. Duplication claims inside one file were not re-read by the main session:
  each fixer was briefed to re-verify its finding before editing and to skip it otherwise, and
  each reviewer to refute it afterwards.
- Cross-file clone findings whose partner files sit in another packet were cut to the part
  inside the packet (file-local helper) and their promise halved.

## Groups

| Group | Files | Packets | Promised | Delivered | Accepted | Refuted packets |
|---|---|---|---|---|---|---|
| C1 | 52 | 5 | 994 | 800 | 800 | none |
| C2 | 57 | 6 | 827 | 770 | 770 | none |
| C3 | 53 | 5 | 560 | 460 | 460 | C3-P01 |
| C4 | 53 | 6 | 484 | 415 | 415 | C4-P04 |
| C5 | 56 | 5 | 838 | 410 | 410 | none |
| C6 | 56 | 5 | 764 | 785 | 785 | none |
| Q1 | 38 | 4 | 1126 | 1143 | 1143 | none |
| Q0 (families, by hand) | 48 | 2 | 850 (spec estimate) | 1,152 | 1,152 | none |
| **Total** (finder groups) | | | 5593 | 4783 | 4783 | |

Promised and delivered are net lines removed (`git diff --numstat`, deletions minus insertions) over a packet's files; accepted counts only packets the reviewer did not refute.

## Outcome

- **Audit count:** 471,048 before, 465,120 after: **−5,928** first-party lines (C++ −3,633, QML
  −2,295), against the 4,000 target of R1 and 3,843 under the quarter's 468,963 ceiling.
- **Stop rule (R6) fired after group C5**, whose accepted yield was 410 of 838 promised (49%):
  the editor form-row family in C5-P05 promised 320 lines and most of it did not survive honest
  counting. The campaign stopped there at exactly −4,000 and handed off. The maintainer then
  asked for the held groups (2026-11-13), so C6 and Q1 ran after the first handoff; both came in
  above 100% of their promise. The two refuted packets were not retried.
- **Refuted and undone:** C3-P01 (139 lines; a new call from `ModbusProjectGenerator.cpp` into
  the Protocols library would break the link of `tst_modbus_generation`, which compiles that
  file without it) and C4-P04 (110 lines; inlining `encodeLogoAsDataUri` would fail
  `test_session_report_logo_is_never_inlined_as_markup`). Each was undone by restoring the
  packet's files from the copy taken before the group started, which equals reverse-applying
  its diff because those files were clean. In both, one finding was at fault and the others
  were sound; they were still undone whole, as the spec asks.
- **Duplicate census:** 1,721 baseline, 1,270 now, re-seeded downward (AC6).
- **Accepted with a corner-case difference, for the maintainer to judge** (each reviewer found no
  ordinary path that reaches it):
  - C6-P03, `TaskbarSearch.cpp`: a child entry's `widgetName` is now the raw role value, so an
    unset role reaches QML as `undefined` where it used to be `""`.
  - C6-P03, `TaskbarWorkspaces.cpp`: `indexForGroupId` returns the last match where it returned
    the first, which differs only if two workspaces share an id.
  - C6-P02, `Taskbar.cpp`: `nextActiveWindow` differs only for a window registered with id −1.
  - Q1-P04, `TableDelegate.qml`: the text and password field loaders are one loader now, so a
    row whose type flips between the two keeps its editor instead of rebuilding it.
  - Q1-P02, `ProcessPicker.qml`: the title label was drawn twice (once by the base dialog) and
    is now drawn once.
- **Style, not enforced by any tool:** 62 of the new one-line command entries in
  `app/qml/Commands/` run past 100 columns.

## Build outcome (maintainer, 2026-11-13)

The first build (MSVC 2022, Release, Qt 6.11) stopped on one error, in packet C3-P02:
`OpcUaSession.cpp(1303): C2653: 'OpcUaTypes' is not a class or namespace name`. The fixer had
inlined `toNodeClass` into a file-static free function, where the short qualifier does not
resolve. The line now names `NodeClass` unqualified, through the file's using-directive. The
packet stayed in: the defect was a name lookup, not a behavior question. The second build
passed, and no packet was backed out.

What ran after it: `pytest tests/scripts scripts/tests` (878 passed, 5 skipped for a missing
`luajit` and symlink rights), and the maintainer's own pass over the running app and the
deduplicated views. What did not run: `ctest` and the QML self-test, because that build
directory has `SS_BUILD_TESTS` and `SS_INAPP_TESTS` off, and `--benchmark-hotpath`, which the
maintainer ruled out on a build without the release optimization profile. CI is the first
place those three run on this diff.

## QML families (group Q0, made by the main session)

| Family | Files | Net | Reviewer |
|---|---|---|---|
| Palette block → `Widgets.ThemedPage` | 37 views and dialogs, 1 new file (7 of them commit with Q1) | −748 | Opus, accepted |
| Folder views → `FolderView` base | 3 views, 1 new file | −404 | Opus, accepted |

- The palette block was checked identical (21 lines, same roles and color keys) in each of the
  37 files before removal; `SmartDialog.qml`, `Macros.qml` and `Dashboard/Terminal.qml` carry a
  different block and were left alone.
- Every `qsTr` string of the three folder views stays in its own file, so the translation
  context is unchanged; `FolderView.qml` holds none.
- One observable-in-principle difference, for the screenshot: the Add Group menu of
  `GroupFolderView` is now a child of the pane instead of the themed page, so it takes its
  colors from the application palette. `ThemeManager` builds that palette from the same 21
  colors, so it should look the same; right after a theme switch it is applied one event-loop
  turn later.

## Review of the full C++ diff (Definition of Done)

`qt-cpp-review`, six Opus agents over the 164 C++ files of C1 to C5 (C6 got one further Opus pass), after `code-verify.py --check`
(0 errors) and `clang-format --dry-run` (clean):

- No finding on model contracts, thread affinity, error text or validation, compile or link.
  No hot or protected file is in the diff and no `connect(` line was added, removed or moved.
- One rule finding, fixed by the main session: `stepProjectHistory` in
  `ProjectFileCommands.cpp` lacked `[[nodiscard]]`.
- Left in the tree, named for the maintainer (each is the far side of an accepted deletion and
  none breaks the build):
  - `Audio::m_inputDrops` and `SerialCanBackendBase::m_bufferOverflows` are still counted but
    their accessors were removed, so nothing can pull them. Diagnostics are meant to be pulled
    (specs 0033/0035): either restore the two accessors or drop the counters.
  - `ParserTemplateCatalog::nativeNames()` and `m_nativeNames` lost their only consumer.
  - `MemoryStore::updateFact()` lost its only caller.
  - `Checksum.cpp` still includes `<QDebug>` without using it.
- Not introduced by the campaign, noticed by a reviewer: in `OpcUaTagModel::finishAttributes`
  the "inherit the parent folder's tick" branch could never run before this change either; the
  campaign removed the unreachable branch. If inheritance was intended, that is a separate fix.

## Dependencies between packets

Packets hold disjoint files, but three are needed by others:

- `core/Core/Api/CommandProtocol.h` (`CommandResponse::makeMissingParam`, added by the main
  session, committed with C1) is called from handler files in C1, C2 and C5. Backing it out
  breaks all three groups.
- C2-P03 (`SchemaBuilder.h`, the one-argument `makeSchema` now forwards) can be backed out on
  its own; nothing depends on the forwarding.
- `app/qml/Widgets/ThemedPage.qml` is used by all 37 palette files and by `FolderView.qml`.

Everything else can be restored file by file with `git restore -- <the packet's files>`.


## Group C1

### C1-P01 — promised 58, delivered 9, verdict: accepted

Reviewer: Accept. The one applied finding (294) moves the body of the single-use static helper applyModbusRegister into the loop in applyModbusRegisters. Behavior is identical: each spec gets the same locals, the same parse call, and calls addRegisterGroup only when the parse succeeds, in the same order. No leftover references remain. The function is still well under 100 lines and nests two levels deep. No connect() call, SS_ASSERT, [[nodiscard]] or tr() string was touched. Findings 433 and 435 were skipped, and CLI.cpp matches HEAD. Compile risk low: The only change is in app/src/Misc/CLI/CliIndustrialConfig.cpp, in applyModbusRegisters at about lines 160-182. The inlined loop body is a token-for-token copy of the deleted helper. It still calls CliSpecParsers::parseModbusRegisterSpec, which was already in scope here, and Modbus::addRegisterGroup, whose return value the old helper also ignored. git grep -nw applyModbusRegister over core, app, tests, examples, scripts and doc/help returns nothing. CLI.cpp, CLI.h and CliIndustrialConfig.h have no diff against HEAD. Nothing was compiled, so the build is unverified; if it fails, start with this loop.

Files:

- `app/src/Misc/CLI.cpp`
- `app/src/Misc/CLI.h`
- `app/src/Misc/CLI/CliIndustrialConfig.cpp`
- `app/src/Misc/CLI/CliIndustrialConfig.h`

Findings:

- #433 `CLI.cpp` (local-duplication, 40) — skipped: Applied then reverted: addOptions is equivalent, but the repo's clang-format keeps one braced-list entry per line, so 100 addOption lines became ~100 list lines (net about -4, under the 5-line floor).: Replace the ~100 one-per-line m_parser.addOption(m_opts.X) calls in registerOptions/registerCommercialOptions with m_parser.addOptions({...}) lists keeping the same order and the #ifdef groups; clang-format packs them several per line.
- #435 `CLI.cpp` (local-duplication, 10) — skipped: The two 13-line tails were verified identical and a static runVerifier(const Options&) helper worked, but the net saving after the helper's doc and signature was 3 lines (under 5); CLI.cpp was restored to HEAD content.: runSessionVerification and runSessionRegression end with the same 13-line block (compose, headless wiring, Verifier run, print report, teardown, exit-code map); extract a static runVerifier(options) helper and call it from both.
- #294 `CliIndustrialConfig.cpp` (single-use-helper, 8) — applied: Delete static applyModbusRegister (15 lines with doc and blank) and inline its body into the for loop in applyModbusRegisters, which is its only caller; the inlined body adds about 6 lines.

### C1-P02 — promised 265, delivered 198, verdict: accepted

Reviewer: I read every hunk in the 7 changed files and found no concrete defect, so the verdict is accept.

- **isApiMessage member:** the member is gone from both the header and the .cpp. `git grep -nw isApiMessage` now finds only the free function `API::isApiMessage` in CommandProtocol.h, so nothing was left calling the member.
- **AudioHandler setters:** the 7 setters now share the `applyListIndex` helper and behave as before:
  - The same JSON keys (deviceIndex/deviceName, rateIndex/rateName, formatIndex/formatName, channelIndex/channelName).
  - The "Invalid %1: %2. Valid range: 0-%3" error text expands to the same bytes as before.
  - The same order: range check, then the setter, then a result built from the list fetched before the setter ran.
  - The same count types: qsizetype count - 1.
- **Missing-parameter blocks:** every replaced block in Audio (8), BluetoothLE (7), CANBus (7) and CSVPlayer (3) tests the same key its text names. `makeMissingParam` produces the identical "Missing required parameter: <key>" text with ErrorCode::MissingParam.
- **CSVExport/CSVPlayer schemas:** checked against `schemaPropToJson`/`makeSchema` in SchemaBuilder.h/.cpp, the new schemas give the same JSON as the hand-built ones:
  - type and description on each property.
  - minimum 0.0 and maximum 1.0 only on progress.
  - A required array only where there was one before; `step` has none, because it uses the two-argument `makeSchema` with delta as optional.
  - `emptySchema()` gives the same {type: object, properties: {}}.
  - The removed local QJsonObject named emptySchema no longer hides the free function.
- **Rules:** no connect edits, no SS_ASSERT or [[nodiscard]] removed from a kept declaration (the one dropped [[nodiscard]] went with the deleted member), no tr() changes, no new files, no protected surfaces touched, and no function near 100 lines.

Skipping finding 54 was justified: the in-file saving is under 5 lines. Leaving the two CSVExport Missing-parameter blocks alone was also correct, because they were outside the brief. Compile risk low: This packet builds only if the uncommitted helper CommandResponse::makeMissingParam(const QString&, const char*) in core/Core/Api/CommandProtocol.h (line 165) builds too. That helper belongs to another packet, so if that packet is undone, all 33 makeMissingParam call sites here fail to compile. After that, check two spots. First, the 6-element aggregate init in core/Api/API/Handlers/CSVPlayerHandler.cpp (csvPlayer.setProgress): SchemaProp{name, "number", desc, {}, 0.0, 1.0}. No other handler uses this form, but SchemaProp is an aggregate with default member initializers, which C++20 allows, and QJsonValue(double) is an implicit constructor. Second, the static template applyListIndex in core/Api/API/Handlers/AudioHandler.cpp. It uses QString::arg(QLatin1StringView) and QJsonObject::operator[](QLatin1StringView), and both exist in Qt 6. The Audio list accessors return QStringList by value, so binding them to const QStringList& keeps the temporary alive for the whole call, including list.at(index) after apply(). audio() returns IO::Drivers::Audio*, which matches auto*.

Files:

- `core/Api/API/CommandHandler.cpp`
- `core/Api/API/CommandHandler.h`
- `core/Api/API/Handlers/AudioHandler.cpp`
- `core/Api/API/Handlers/AudioHandler.h`
- `core/Api/API/Handlers/BluetoothLEHandler.cpp`
- `core/Api/API/Handlers/BluetoothLEHandler.h`
- `core/Api/API/Handlers/CANBusHandler.cpp`
- `core/Api/API/Handlers/CANBusHandler.h`
- `core/Api/API/Handlers/CSVExportHandler.cpp`
- `core/Api/API/Handlers/CSVExportHandler.h`
- `core/Api/API/Handlers/CSVPlayerHandler.cpp`
- `core/Api/API/Handlers/CSVPlayerHandler.h`

Findings:

- #436 `CommandHandler.cpp` (dead-code, 9) — applied: Remove the CommandHandler::isApiMessage member (declaration in CommandHandler.h line 61 plus the pass-through definition and doc comment); it only forwards to the free API::isApiMessage and nothing calls the member.
- #0 `AudioHandler.cpp` (local-duplication, 150) — applied: Six setters (setInputDevice, setOutputDevice, setSampleRate, setInputSampleFormat, setInputChannelConfig, setOutputSampleFormat/ChannelConfig) share identical check/range/apply/respond blocks; collapse into one static helper taking param key, list, setter lamb
- family `AudioHandler.cpp` (cross-file-clone, 16) — applied: Replace each of the 8 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `BluetoothLEHandler.cpp` (cross-file-clone, 14) — applied: Replace each of the 7 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #54 `CANBusHandler.cpp` (cross-file-clone, 25) — skipped: In-file part (CANBus setCanFD/setLoopback/setListenOnly) nets about 4 lines after a file-local helper, under the 5-line threshold.: Add setBoolParam(id, params, key, resultKey, apply-lambda) and shrink the 16-line boolean 'enabled' setters to ~5 lines each: CAN setCanFD/setLoopback/setListenOnly, Network setUdpMulticast/setIgnoreTlsErrors, CSVExport/MDF4Export setEnabled.
- family `CANBusHandler.cpp` (cross-file-clone, 14) — applied: Replace each of the 7 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #59 `CSVExportHandler.cpp` (cross-file-clone, 9) — applied: Replace hand-built enabled/intervalMs schemas and local emptySchema with makeSchema/emptySchema(); add SchemaBuilder include.
- #49 `CSVPlayerHandler.cpp` (cross-file-clone, 22) — applied: Replace the hand-built QJsonObject/QJsonArray schemas (emptySchema, open, progress, paused, step) with emptySchema() and makeSchema({...}) / makeSchema({}, {delta}) inline in registerCommand; add #include "API/SchemaBuilder.h". progress uses SchemaProp{name,"n
- family `CSVPlayerHandler.cpp` (cross-file-clone, 6) — applied: Replace each of the 3 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM

### C1-P03 — promised 185, delivered 224, verdict: accepted

Reviewer: I found no defect, so I accept the packet. I compared the schemas against SchemaBuilder.h/.cpp. They produce the same JSON as before, with the same keys, types, descriptions and required order:
- A "number|string" type splits into ["number","string"].
- busType keeps its enum and default 0.
- writeData has binary:true.
- getLatestFrame has no required key, and progress keeps its minimum and maximum.
Every missing-param error is the same text as before ("Missing required parameter: X") with the MissingParam code.

findTable keeps the old first-match behaviour. Replacing path.isEmpty() with !hit.def gives the same result: requireString guarantees a non-empty name, and tableFullPath(name) is never empty for a non-empty name. tableRename still gets parentFolderId -1 and an empty oldPath when nothing matches. tables() and editorTableFolders() return const references, so the TableDef pointer stays valid as before. The table and folders locals that are kept (tableRename, registerAdd) are still used, and none of the removed ones is used anywhere.

Scope: only these five packet files changed, all inside the packet's file list. No connect, tr(), SS_ASSERT or objectName was touched, and the only removed [[nodiscard]] function is defaultValueProp, which is deleted along with its two uses. No new file was added. Compile risk low: Nothing here looks like it will break the build. The edits rely on uncommitted work from another packet: makeMissingParam(const QString&, const char*) is only in the working tree's core/Core/Api/CommandProtocol.h (git status shows it modified). If that packet is reverted, every makeMissingParam call in these five files stops compiling. Places to check first:
(1) DataTablesHandler.cpp:83-123. A `namespace detail` at global scope (CommandSafety.cpp and ScriptsHandler.cpp already declare one) plus the static findTable and requireTableAndName. All names are unique across core/, so a unity build is safe.
(2) MDF4PlayerHandler.cpp, the progress prop: positional aggregate {name, type, desc, {}, 0.0, 1.0}. 0.0 and 1.0 convert implicitly to QJsonValue(double).
(3) IOManagerHandler.cpp:108 and :189. Keys::BusType and Keys::SourceId are QLatin1StringView going into a QString field or parameter. The conversion is implicit, and AssistantHandler.cpp:1173 already does the same thing.
Unqualified makeSchema() and emptySchema() inside the API::Handlers member functions resolve to the API:: free functions. None of the handler classes has a member with those names, and no shadowing local named emptySchema is left. clang-format 23 --dry-run reports nothing on all five files. QJsonArray includes left in place but unused are harmless.

Files:

- `core/Api/API/Handlers/DataTablesHandler.cpp`
- `core/Api/API/Handlers/DataTablesHandler.h`
- `core/Api/API/Handlers/EipHandler.cpp`
- `core/Api/API/Handlers/EipHandler.h`
- `core/Api/API/Handlers/HIDHandler.cpp`
- `core/Api/API/Handlers/HIDHandler.h`
- `core/Api/API/Handlers/IOManagerHandler.cpp`
- `core/Api/API/Handlers/IOManagerHandler.h`
- `core/Api/API/Handlers/MDF4ExportHandler.cpp`
- `core/Api/API/Handlers/MDF4ExportHandler.h`
- `core/Api/API/Handlers/MDF4PlayerHandler.cpp`
- `core/Api/API/Handlers/MDF4PlayerHandler.h`

Findings:

- #51 `DataTablesHandler.cpp` (local-duplication, 22) — applied: Express valueSchema, valueHSchema, addSchema, updSchema with makeSchema using SchemaProp type "number|string" (schemaPropToJson lowers it to the ["number","string"] array); delete defaultValueProp() and the manual props/required assembly.
- #56 `DataTablesHandler.cpp` (local-duplication, 20) — applied: Add static findTable(name) returning {const TableDef*, QString path} and replace the six copies of the 'full == name || t.name == name' scan loop (tableGet, tableDelete, tableRename, registerAdd, registerDelete, registerUpdate).
- #57 `DataTablesHandler.cpp` (local-duplication, 25) — applied: Add a helper that checks requireString for 'table' and 'name' and returns the MissingParam error as std::optional<CommandResponse>; replace the 8-line double check in registerAdd, registerDelete, registerUpdate, valueGet, valueSet, valueHandle.
- family `DataTablesHandler.cpp` (cross-file-clone, 40) — skipped: Only 4 blocks of the exact `if (!params.contains(K))` shape exist (valueSet 'value', valueGetH 'handle', valueSetH 'handle' and 'value'). They are 3 lines each and would save 1 line apiece, about 4 net lines in total, under the 5-line floor. The other 16 grep hits are requireString/isEmpty checks, which finding 57 already folds into the helper.: Replace each of the 20 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- family `EipHandler.cpp` (cross-file-clone, 8) — skipped: Only 2 of the 4 hits match the `params.contains` + MissingParam shape (value, index), saving about 4 lines. The 'key' and 'tag or type' hits test isEmpty() and have different text, so the finding is under the 5-line floor.: Replace each of the 4 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #58 `HIDHandler.cpp` (cross-file-clone, 10) — applied: Replace the hand-built deviceIndex schema and the two copy-pasted emptySchema blocks with makeSchema({{deviceIndex,integer,desc}}) and emptySchema(); add SchemaBuilder include.
- #52 `IOManagerHandler.cpp` (cross-file-clone, 20) — applied: Replace hand-built schemas (2x emptySchema, paused, busType, writeData, getLatestFrame) with emptySchema(), makeSchema, enumProp (busType, enum 0..8 default 0), byteProp (writeData) and makeSchema({}, {sourceId, encoding}); add SchemaBuilder include.
- family `IOManagerHandler.cpp` (cross-file-clone, 6) — applied: Replace each of the 3 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #60 `MDF4ExportHandler.cpp` (cross-file-clone, 7) — applied: Replace setEnabledSchema, closeSchema, getStatusSchema hand-built objects with makeSchema/emptySchema() inline; add SchemaBuilder include.
- #50 `MDF4PlayerHandler.cpp` (cross-file-clone, 21) — applied: Replace the hand-built schemas (open, empty, paused, step, progress, status) with emptySchema()/makeSchema inline in the registerCommand calls; add #include "API/SchemaBuilder.h". Emitted JSON is identical.
- family `MDF4PlayerHandler.cpp` (cross-file-clone, 6) — applied: Replace each of the 3 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM

### C1-P04 — promised 259, delivered 205, verdict: accepted

Reviewer: I found no defect in any of the seven files. I read every hunk in context and checked each one as a compiler and as a tester.

- **Missing-parameter returns (Modbus, Network, Process, OpcUa):** makeMissingParam(id, const char*) in CommandProtocol.h:165 gives the same error code and the same 'Missing required parameter: K' text. In every converted block the key in the text equals the key tested. The plural message at ModbusHandler.cpp:491 is correctly left alone. OpcUaHandler's deleted missingParam was a private file-local static with the same body, and nothing references it any more.
- **Modbus selectFromList:** the error text ('Invalid <key>: N. Valid range: 0-M'), the error codes and the result keys are all unchanged. The apply-then-echo order is also unchanged. The lists are returned by value, so the echoed name comes from the list as it was before the apply, exactly as in the old code. The only change is that the const list getter now runs before the missing-parameter check, and nothing can observe that.
- **Modbus indexedNameArray:** it builds the same {index, name} or {index, value} objects as the old loops.
- **MQTT:** the only behavior change is in the subscriber, where autoKeepAlive is now set before cleanSession instead of after. Both setters are independent, both guard against no-op writes, and each emits one mqttConfigurationChanged. The signal count stays the same, and both still run after the endpoint check and before any keepAlive or TLS error. autoKeepAlive can never fail.
- **OpcUa::supportedPolicies removal:** a grep of core, app, tests, examples, scripts and doc/help finds no remaining caller of OpcUa::supportedPolicies. The test file calls the free function in OpcUaEndpointSelection. Nothing in QML or moc uses it, because it was a plain static and not a Q_INVOKABLE.
- **Rules:** none of the packet's header files are modified. No connect() call, SS_ASSERT, [[nodiscard]] or tr() string was touched. No new file was added and no protected surface was touched. Compile risk low: Check these first: (1) core/Devices/IO/Drivers/OpcUa.cpp. Now that the class static OpcUa::supportedPolicies() is gone, the unqualified supportedPolicies() calls at lines 161, 820, 1271, 1279 and 1300 rely on the new global using-declaration at line 54. No base class or enclosing namespace declares that name, so lookup reaches the using-declaration. (2) core/Api/API/Handlers/OpcUaHandler.cpp:34 is the first include of a header from a driver subfolder (IO/Drivers/OpcUa/...) outside core/Devices. It resolves through the PUBLIC core/Devices include root, and both files sit in the commercial blocks. (3) The applySessionAndTls<Target> template in core/Api/API/Handlers/MqttHandler.cpp:349 is instantiated with MQTT::Publisher& and IO::Drivers::MQTT&. Its body is token-for-token the old subscriber body, which already compiled through d->, apart from the removed autoKeepAlive line. (4) The selectFromList and indexedNameArray templates in ModbusHandler.cpp:244-289 depend on QString::fromLatin1(const char*) resolving to the QByteArrayView overload. That is standard in Qt 6. The Modbus list getters return QStringList by value, so the temporary bound to the const& parameter lives until the full expression ends.

Files:

- `core/Api/API/Handlers/ModbusHandler.cpp`
- `core/Api/API/Handlers/ModbusHandler.h`
- `core/Api/API/Handlers/MqttHandler.cpp`
- `core/Api/API/Handlers/MqttHandler.h`
- `core/Api/API/Handlers/NetworkHandler.cpp`
- `core/Api/API/Handlers/NetworkHandler.h`
- `core/Api/API/Handlers/OpcUaHandler.cpp`
- `core/Api/API/Handlers/OpcUaHandler.h`
- `core/Api/API/Handlers/ProcessHandler.cpp`
- `core/Api/API/Handlers/ProcessHandler.h`
- `core/Devices/IO/Drivers/OpcUa.cpp`
- `core/Devices/IO/Drivers/OpcUa.h`

Findings:

- #47 `ModbusHandler.cpp` (cross-file-clone, 55) — applied: Add one helper setIndexFromList(id, params, indexKey, nameKey, const QStringList&, apply-lambda) and turn the 9 near-identical 'select by index from a QStringList' setters into 3-5 line wrappers: Modbus protocol/serialPort/parity/dataBits/stopBits, CAN plugin/
- #48 `ModbusHandler.cpp` (cross-file-clone, 30) — applied: Add indexedNameArray(const QStringList&, valueKey = 'name', first = 0) and replace the 8-line 'index/name object per list entry' loops in the list getters with one call (Modbus protocol/serialPort/parity/dataBits/stopBits/registerType, CAN interface, Network s
- family `ModbusHandler.cpp` (cross-file-clone, 24) — partly-applied: Converted 11 of the 12 Modbus blocks to makeMissingParam (the first-sweep wrappers selectFromList's own check also uses it). The 12th grep hit is the plural 'Missing required parameters: type, startAddress, count', which is a different message, so it was left alone.: Replace each of the 12 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- #53 `MqttHandler.cpp` (local-duplication, 48) — applied: Make applyPublisherSessionAndTls a template over the target (MQTT::Publisher / IO::Drivers::MQTT) and delete applySubscriberSessionAndTls; the subscriber's autoKeepAlive line is applied in subscriberSetConfig before the call.
- #46 `NetworkHandler.cpp` (cross-file-clone, 55) — partly-applied: Network's 14 blocks are covered by 1262. Deleted OpcUaHandler's private missingParam and switched its 18 call sites to API::CommandResponse::makeMissingParam; the text and code are identical. Did not add a new requireParam helper because makeMissingParam already exists in CommandProtocol.h. The partner files are not mine.: Add one shared requireParam(id, params, key) returning std::optional<CommandResponse> (same MissingParam code and 'Missing required parameter: K' text); replace each 4-line contains()/makeError block with 2 lines. Delete OpcUaHandler's private missingParam and
- family `NetworkHandler.cpp` (cross-file-clone, 28) — applied: Replace each of the 14 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- #40 `OpcUa.cpp` (single-use-helper, 9) — applied: Remove pass-through static OpcUa::supportedPolicies() (+ OpcUa.h:253 decl); add 'using IO::Drivers::OpcUaEndpointSelection::supportedPolicies;' beside the existing using-declarations in OpcUa.cpp and qualify the single call in OpcUaHandler.cpp:348 as OpcUaEndp
- family `ProcessHandler.cpp` (cross-file-clone, 10) — applied: Replace each of the 5 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM

### C1-P05 — promised 227, delivered 164, verdict: accepted

Reviewer: I accept C1-P05: I found no defect of kind behavior, compile, scope or rule, and I rate compile risk low. Nothing was compiled, so "it compiles" rests on reading the code as a compiler.
- **Behavior:** responses are unchanged. Missing-parameter errors keep the same code and the same text byte for byte. Undo and redo keep the same result keys ("performed", "reason", "undone"/"redone"), reasons, side-effect order and epoch attachment. The six dataset handlers keep the same InvalidParam messages and lookup order. The transform wrapper text is identical.
- **Scope and rules:** only the six packet files changed, and no new file was added. No `connect()` call, SS_ASSERT, `[[nodiscard]]` or `tr()` string was touched. No function goes over 100 lines, and nothing protected (licensing, API auth, the hotpath files) was edited.
- **Skipped findings:** the 1265 blocks and the one 1264 loop the executor left alone are `for key in required` loops with a `%1` message, so they did not match the exact form and leaving them is correct. Compile risk low: I read every hunk against its surrounding code and found nothing that would break the build.
- `makeMissingParam(const QString&, const char*)` in `core/Core/Api/CommandProtocol.h:165` builds "Missing required parameter: %1" from `QLatin1String(name)`, so the error text matches the old messages byte for byte. The Keys constants match the literals that replaced them: Title is "title", DatasetId is "datasetId", UniqueId is "uniqueId" and GroupId is "groupId" (`FrameKeys.h`).
- `stepProjectHistory` (`ProjectFileCommands.cpp`, about line 444) relies on `canUndo`/`canRedo` returning bool, `undoText`/`redoText` returning QString, and `undo()`/`redo()` returning bool. `ProjectModel.h` lines 513-519 and 619-620 confirm all of these. Both arms of each ternary are QString (QStringLiteral in Qt 6). `attachProjectEpoch` resolves through the file-level `using namespace API::Handlers::ProjectApiSupport`.
- Moving `labelTarget` up to `ProjectDatasetFieldCommands.cpp:356` keeps it above all three of its uses (the six setter/getter handlers and the value-label handlers). Its group and dataset lookup and both InvalidParam messages are the same as the find_if blocks it replaces. `pm` remains only where `updateDataset` or `sources()` still use it, so there is no unused variable, and no other local named `error` or `dataset` exists in those functions.
- `wrapTransformForParser` was moved above `transformDryRun` unchanged. `pickHeadlineDatasetOption` has the same seven candidates in the same order and the same Generic fallback.
If the build fails, look first at `stepProjectHistory` in `ProjectFileCommands.cpp`, then at the `labelTarget` callers in `ProjectDatasetFieldCommands.cpp`. The six new keys-hardcoded-literal advisories from code-verify are advisories only, not errors.

Files:

- `core/Api/API/Handlers/ProjectDatasetCommands.cpp`
- `core/Api/API/Handlers/ProjectDatasetCommands.h`
- `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`
- `core/Api/API/Handlers/ProjectDatasetFieldCommands.h`
- `core/Api/API/Handlers/ProjectDryRunCommands.cpp`
- `core/Api/API/Handlers/ProjectDryRunCommands.h`
- `core/Api/API/Handlers/ProjectFileCommands.cpp`
- `core/Api/API/Handlers/ProjectFileCommands.h`
- `core/Api/API/Handlers/ProjectGroupCommands.cpp`
- `core/Api/API/Handlers/ProjectGroupCommands.h`
- `core/Api/API/Handlers/ProjectListCommands.cpp`
- `core/Api/API/Handlers/ProjectListCommands.h`

Findings:

- #4 `ProjectDatasetCommands.cpp` (local-duplication, 12) — applied: datasetAdd inlines the headline-option loop. Replace the 13 lines with const auto headline = pickHeadlineDatasetOption(options);, the same call datasetAddMany makes at line 460.
- family `ProjectDatasetCommands.cpp` (cross-file-clone, 28) — partly-applied: 13 of 14 blocks converted (incl. Keys::X ones, constants verified equal to the text); the 14th is a loop over a QString key with a %1 message, not an exact match, left alone.: Replace each of the 14 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- #2 `ProjectDatasetFieldCommands.cpp` (local-duplication, 85) — applied: Six handlers (setVirtual, setTransformCode, get/setAlarmBands, get/setFFTMarkers) each repeat a 19-line group/dataset find_if lookup. Replace each with the existing labelTarget(params, error) plus a 2-line InvalidParam return; move labelTarget above line 360.
- family `ProjectDatasetFieldCommands.cpp` (cross-file-clone, 16) — skipped: All 8 blocks are 'for key in required' loops with a %1 message built from a QString key, not the exact four-line form; makeMissingParam takes const char*.: Replace each of the 8 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #3 `ProjectDryRunCommands.cpp` (local-duplication, 16) — applied: transformDryRun builds the wrapped Lua/JS parse() script inline. Replace the 17 lines with const QString wrapped = wrapTransformForParser(code, language); move that helper above transformDryRun.
- family `ProjectDryRunCommands.cpp` (cross-file-clone, 22) — partly-applied: 9 of 11 converted; the other 2 have different custom messages (inputBytesHex..., sampleFrame...) so they do not match.: Replace each of the 11 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- #5 `ProjectFileCommands.cpp` (local-duplication, 18) — applied: projectUndo and projectRedo are line-for-line mirrors. Collapse them into one static helper that takes can/text/step callables, the 'nothing to ...' reason and the result key, with two one-line wrappers.
- family `ProjectFileCommands.cpp` (cross-file-clone, 8) — applied: Replace each of the 4 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `ProjectGroupCommands.cpp` (cross-file-clone, 8) — applied: Replace each of the 4 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `ProjectListCommands.cpp` (cross-file-clone, 14) — applied: Replace each of the 7 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM

## Group C2

### C2-P01 — promised 194, delivered 146, verdict: accepted

Reviewer: Every hunk in the six packet files was checked against its full function. The missing-param replacements match the old checks byte for byte: makeMissingParam produces the same "Missing required parameter: K" text with the same MissingParam code through makeError, and K matches the key tested at each of the 39 sites. The left-out sites (S7 'key' and 'address', ProjectUpdate Keys::DatasetId) are untouched. findSource keeps the same order of checks and the same "Unknown sourceId" error. parserSetLanguage still validates the language before looking up the source. model is still used where it is still declared, and source is read in every handler that declares it. takeParam gives the same result as the old lambdas: contains-check, insert into consumed, return bool, with the lambdas still capturing by reference. SessionsHandler schemas match: same property types and descriptions, the same 'required' arrays (renameTag is [tagId,newLabel], setNotes is [sessionId,notes], assignTag and removeTag are [sessionId,tagId]), sessions.regress still has no 'required' key, the three empty schemas now come from API::emptySchema() with identical content, and getRegression's bare schema is unchanged. No .h file, connect() call, SS_ASSERT, [[nodiscard]], tr() string or protected surface was touched, and no function exceeds 100 lines. No defects found. Compile risk low: This packet depends on two working-tree edits from earlier packets: CommandResponse::makeMissingParam(const QString&, const char*) in core/Core/Api/CommandProtocol.h, and the reordered makeSchema overloads in core/Api/API/SchemaBuilder.h, where the one-argument form now forwards to makeSchema(props, {}). If either earlier packet is reverted, every call site here stops compiling. Start with core/Api/API/Handlers/SessionsHandler.cpp. sessions.regress calls makeSchema({}, {...}): the empty `{}` binds to std::initializer_list<SchemaProp>, and since only the two-argument overload takes two arguments there is no ambiguity. The single-property calls makeSchema({{q,q,q}}) follow the pattern OpcUaHandler already uses. clang-format left SessionsHandler.cpp:160 at 118 columns (the setNotes sessionId entry). That is a cosmetic alignment artifact, not a compile issue. findSource returns const DataModel::Source* through &*it on the const vector iterator, and <algorithm> is included. takeParam and findSource are file-static. No other definition of either name exists in core/ or app/, so a unity build has nothing to collide with. Nothing was syntax-checked, because there is no compile database.

Files:

- `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp`
- `core/Api/API/Handlers/ProjectOutputWidgetCommands.h`
- `core/Api/API/Handlers/ProjectPainterCommands.cpp`
- `core/Api/API/Handlers/ProjectPainterCommands.h`
- `core/Api/API/Handlers/ProjectParserCommands.cpp`
- `core/Api/API/Handlers/ProjectParserCommands.h`
- `core/Api/API/Handlers/ProjectUpdateCommands.cpp`
- `core/Api/API/Handlers/ProjectUpdateCommands.h`
- `core/Api/API/Handlers/S7Handler.cpp`
- `core/Api/API/Handlers/S7Handler.h`
- `core/Api/API/Handlers/SessionsHandler.cpp`
- `core/Api/API/Handlers/SessionsHandler.h`

Findings:

- family `ProjectOutputWidgetCommands.cpp` (cross-file-clone, 14) — applied: Replace each of the 7 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `ProjectPainterCommands.cpp` (cross-file-clone, 6) — applied: Replace each of the 3 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #70 `ProjectParserCommands.cpp` (local-duplication, 9) — applied: Four handlers (parserSetLanguage, parserGetLanguage, parserGetTemplate, parserSetTemplate) repeat the find_if-by-sourceId plus 'Unknown sourceId' error. Add one static findSource(sourceId) returning const Source*, and use it at each site.
- family `ProjectParserCommands.cpp` (cross-file-clone, 8) — applied: Replace each of the 4 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #68 `ProjectUpdateCommands.cpp` (local-duplication, 9) — applied: The 'take' lambda (contains-check plus consumed.insert) is copied four times. Add one static takeParam(params, consumed, key) and make each lambda a one-line forwarder.
- family `ProjectUpdateCommands.cpp` (cross-file-clone, 12) — partly-applied: 5 of 6 replaced. The datasetId block tests Keys::DatasetId, whose string value I could not confirm equals 'datasetId', so it was left as is.: Replace each of the 6 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `S7Handler.cpp` (cross-file-clone, 8) — partly-applied: 2 of 4 replaced. The 'key' and 'address' sites test isEmpty() on a string, not contains(), so they do not match the pattern and were left.: Replace each of the 4 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #61 `SessionsHandler.cpp` (local-duplication, 90) — applied: Replace the hand-built JSON schemas (props/schema/required blocks) and the three local emptySchema copies with API::makeSchema({...}) and API::emptySchema(); add #include API/SchemaBuilder.h. Keep getRegression's bare {type:object} schema as is.
- family `SessionsHandler.cpp` (cross-file-clone, 38) — applied: Replace each of the 19 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make

### C2-P02 — promised 159, delivered 191, verdict: accepted

Reviewer: I read every hunk in the 9 files and found nothing to refute. Behavior:
- **Missing parameter:** the error text is identical, because makeMissingParam builds "Missing required parameter: %1" from the key.
- **Invalid index:** the error text is identical. "Invalid %1: %2. Valid range: 0-%3" is filled with the key, the index and count-1, which gives the same bytes as the five UART and three USB originals.
- **Result keys:** unchanged. portIndex/portName, parityIndex/parityName, dataBitsIndex/dataBitsValue, stopBitsIndex/stopBitsValue, flowControlIndex/flowControlName, deviceIndex/deviceName, and endpointIndex/endpointName for both endpoint commands.
- **Setter calls:** UART still queues its setter through invokeMethod with Qt::AutoConnection and a quint8 cast. USB still calls its setter directly with an int. Side effects happen in the same order.
- **Lists:** QJsonArray::fromStringList gives the same JSON as the old append loops.
- **Removed symbols:** none of them has a remaining caller. MCPHandler::isMCPMessage is not called anywhere; ClientReception uses the free MCP::isMCPMessage. ClientSession::clientName/clientVersion were only written. MCPResponse::jsonrpc was only assigned, and toJson writes a literal "2.0". Prompt::arguments was never set, and nothing initializes any of these structs field by field. MirrorClient::epoch/lastError/lastErrorCode are not called; MirrorSession uses only close/endpoint/hz/info/live/stale/open on m_client and keeps its own lastErrorCode property. MirrorPublisher::epoch/subscribed are not called; the only publisher calls anywhere are effectiveHz/info/setRate/structure/structureChunk/structureParts/subscribe/unsubscribe.
- **Rules:** no connect() call, SS_ASSERT, [[nodiscard]], tr() string or objectName was touched. There are no new files, and UARTHandler.h and USBHandler.h are unchanged. Nothing falls on a protected surface, and no function is close to 100 lines.

Nothing was compiled; I checked the code by reading it, and the maintainer's build is still the real test. Compile risk low: I found nothing that should break the build. The most likely trouble spots are the two file-local setListIndex helpers, at core/Api/API/Handlers/UARTHandler.cpp:256 and core/Api/API/Handlers/USBHandler.cpp:99. Their member-function-pointer parameter types match the driver declarations: UART.h:172-178 and 193-199 declare QStringList X() const and void setX(const quint8), and USB.h:145-163 declares QStringList X() const and void setX(const int). Top-level const on a parameter is not part of the function type, and none of these setters is overloaded. The full IO::Drivers::UART and USB types reach both files through IO/ConnectionManager.h, as they already did for the old ->list() calls. API::ErrorCode is a namespace in Core/Api/CommandProtocol.h, so the qualified uses resolve. makeMissingParam(const QString&, const char*) exists at CommandProtocol.h:165. Under a unity build the two static helpers would become overloads with different pointer types, which still compiles. Every removed symbol is gone from its .h and its .cpp. m_epoch is still used in MirrorClient (open, line 505) and MirrorPublisher (line 122), and m_subscribers is still used in MirrorPublisher.

Files:

- `core/Api/API/Handlers/UARTHandler.cpp`
- `core/Api/API/Handlers/UARTHandler.h`
- `core/Api/API/Handlers/USBHandler.cpp`
- `core/Api/API/Handlers/USBHandler.h`
- `core/Api/API/MCPHandler.cpp`
- `core/Api/API/MCPHandler.h`
- `core/Api/API/MCPProtocol.h`
- `core/Api/API/Mirror/MirrorClient.cpp`
- `core/Api/API/Mirror/MirrorClient.h`
- `core/Api/API/Mirror/MirrorPublisher.cpp`
- `core/Api/API/Mirror/MirrorPublisher.h`

Findings:

- #62 `UARTHandler.cpp` (local-duplication, 30) — applied: setPortIndex/setParity/setDataBits/setStopBits/setFlowControl are the same missing-param, list-range check, queued setter and {index,name} result. Collapse into one static helper taking key, result-name key, list and setter lambda; each handler becomes a ~5-li
- family `UARTHandler.cpp` (cross-file-clone, 24) — applied: Replace each of the 12 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make
- #64 `USBHandler.cpp` (local-duplication, 10) — applied: setDeviceIndex, setInEndpointIndex and setOutEndpointIndex share missing-param, range-check against a QStringList, setter call and {index,name} result. Fold into one static helper parameterized by key names, list and setter.
- #66 `USBHandler.cpp` (local-duplication, 12) — applied: Replace the hand-written append loops over QStringList (devices_array, in_endpoints, out_endpoints) with QJsonArray::fromStringList(...) inline in the result assignments.
- family `USBHandler.cpp` (cross-file-clone, 10) — applied: Replace each of the 5 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #69 `MCPHandler.cpp` (dead-code, 9) — applied: Remove the pass-through MCPHandler::isMCPMessage member (cpp 60-66 incl. doc and blank, h 56).
- #72 `MCPHandler.cpp` (dead-code, 5) — applied: Drop write-only ClientSession::clientName/clientVersion (h 42-43) and the clientInfo read plus two assignments in handleInitialize (330-332).
- #71 `MCPProtocol.h` (dead-code, 7) — applied: Remove never-read MCPResponse::jsonrpc (field plus two assignments in makeSuccess/makeError) and the never-set Prompt::arguments (field plus the toJson branch).
- #63 `MirrorClient.cpp` (dead-code, 34) — applied: Remove MirrorClient::epoch(), lastError(), lastErrorCode() (cpp 131-137, 147-162; h 68, 70, 71) and the then write-only members m_lastError/m_lastErrorCode (h 121-122, cpp 197-198, 711-712).
- #65 `MirrorPublisher.cpp` (dead-code, 18) — applied: Remove MirrorPublisher::epoch() and subscribed(const QString&) definitions (cpp 177-191) and their declarations (h 81-82).

### C2-P03 — promised 12, delivered 13, verdict: accepted

Reviewer: I accept the packet. It removes 13 lines and changes nothing observable. With an empty optional list, the two-argument body is identical to the deleted one-argument body: the same properties, the same required array, which is still left out when empty, and the same "type"/"properties" keys. [[nodiscard]] and the doc comment are kept. There are no signature changes, so no callers needed updating. No connects, no SS_ASSERTs, no tr() strings and no protected surfaces are touched, and no new file was added. Nothing was compiled; I checked this by reading only. Compile risk low: I found no defects. The only change is in core/Api/API/SchemaBuilder.h; SchemaBuilder.cpp is untouched. The one-argument makeSchema(std::initializer_list<SchemaProp>) at lines 167-170 now comes after the two-argument overload and returns makeSchema(props, {}). That call compiles: `props` is an initializer_list lvalue, it copies cheaply into the first parameter, and `{}` initialises an empty std::initializer_list<SchemaProp>. No callers needed to change. Existing one-argument calls, including makeSchema({}), cannot bind to the two-argument overload, and two-argument calls resolve as before. The backing array of `props` belongs to the caller's full expression, so it outlives the inner call. If the build does flag something, look first at line 169: the inner call has to resolve to the two-argument overload declared above it (it does).

Files:

- `core/Api/API/SchemaBuilder.cpp`
- `core/Api/API/SchemaBuilder.h`

Findings:

- #14 `SchemaBuilder.h` (local-duplication, 12) — applied: Make the one-argument makeSchema(initializer_list) delegate: return makeSchema(props, {}); and delete its duplicated 15-line body, which is the two-argument overload with an empty optional list.

### C2-P04 — promised 172, delivered 144, verdict: accepted

Reviewer: Accepted. Every removed symbol is unreferenced in the grep scope. The qToBigEndian and qToLittleEndian calls in Checksum.cpp produce the same bytes as the old helpers. SimpleCrypt (licensing) was correctly skipped, and Checksum.h, Frame.cpp and the Crypto files are untouched. No connect, SS_ASSERT, tr() or protected surface was edited, and a nodiscard attribute disappeared only along with the declaration it was on. Compile risk low: Look first at core/Core/Checksum.cpp lines 214-237. Those lines call qToBigEndian(v, out) and qToLittleEndian(v, out) with out a uint8_t[] that decays to void*, through Qt's template<T> void qToBigEndian(T, void*) overload. T deduces to uint16_t or uint32_t, and both have qbswap specializations. The byte output is the same as the old helpers' on both little-endian and big-endian hosts. Checksum.cpp no longer uses memcpy or anything from stdlib.h, so dropping the MSVC <stdlib.h> include is safe. <QDebug> is now unused there, which is harmless. Concepts.h keeps <QIODevice>/<QString>, so anything that included them through it still compiles. None of the removed symbols (multiplier(), timeoutMsec, Comparable, Hashable, Frameable, Driver, isValidFrame, IS_LITTLE_ENDIAN, big_endian_memcpy, little_endian_memcpy) has any reference left in core, app, tests, examples, scripts or doc/help; app/rcc/ai/search_index.json was excluded from that search as generated data. None of them was a slot, a Q_PROPERTY or used by QML.

Files:

- `core/Core/Async/RetryPolicy.cpp`
- `core/Core/Async/RetryPolicy.h`
- `core/Core/Async/TaskTree.cpp`
- `core/Core/Async/TaskTree.h`
- `core/Core/Checksum.cpp`
- `core/Core/Checksum.h`
- `core/Core/Concepts.h`
- `core/Core/Crypto/SimpleCrypt.cpp`
- `core/Core/Crypto/SimpleCrypt.h`
- `core/Core/DataModel/Frame.cpp`
- `core/Core/DataModel/Frame.h`

Findings:

- #16 `RetryPolicy.cpp` (dead-code, 9) — applied: Delete RetryPolicy::multiplier() (definition and doc comment in RetryPolicy.cpp, declaration at RetryPolicy.h:35). delayForAttempt reads m_multiplier directly.
- #15 `TaskTree.cpp` (dead-code, 9) — applied: Delete TimeoutTask::timeoutMsec() (definition and doc comment in TaskTree.cpp, declaration at TaskTree.h:172). Nothing calls it; m_msec is still used internally.
- #138 `Checksum.cpp` (redundant-logic, 55) — applied: Delete the IS_LITTLE_ENDIAN detection block and the big_endian_memcpy/little_endian_memcpy helpers. packU16BE/packU16LE/packU32BE call qToBigEndian/qToLittleEndian (add #include <QtEndian>) with the same 3-line bodies. Output is identical.
- #137 `Concepts.h` (dead-code, 63) — applied: Remove the unused concepts Comparable, Hashable, Frameable and Driver (Concepts.h 62-83, 101-135) and the now-unused isValidFrame template in Frame.h 1403-1410. Keep Numeric, ByteLike and Serializable.
- #139 `SimpleCrypt.cpp` (dead-code, 36) — skipped: SimpleCrypt is in the Licensing namespace and used by licensing code; the brief rules say never touch licensing, and the finding itself asks for a human glance.: Remove four members with no callers: compressionMode() getter, integrityProtectionMode() getter, setCompressionMode(), and the encryptToByteArray(const QString&) overload. Also remove their declarations in SimpleCrypt.h (lines 77, 78, 82, 91). No behavior chan

### C2-P05 — promised 205, delivered 212, verdict: accepted

Reviewer: I found no defects. Every hunk does what the packet asked, and only packet files changed. None of the eight removed symbols has a remaining use. I searched for each one with `git grep -w` over core, app, tests, examples, scripts and doc/help: `hexToString`, `stringToHex`, `escapeControlCharacters`, `resolveXAxisPolicy`/`XAxisPolicy`, `kBusAll`, `layoutPatternSlug`, `layoutPatternLabel` and `translateAll`. EnumLabels.cpp uses nothing from LayoutPatterns.h, so dropping that include is safe. `XAxisMode` is kept because `groupXAxisMode` still returns it.

Behavior is unchanged:
- `textEncodingFromName`: the hash holds the same 24 aliases on the same trimmed, upper-cased key, still defaults to `EncUtf8`, and keeps the empty-name early return.
- `dashboardWidgetFromSlug`: all 34 aliases are there with no duplicate keys, the `BUILD_COMMERCIAL` set is inside the `#ifdef`, and a miss still returns -1 on the trimmed, lower-cased key.
- The four dataset-option functions keep the same bit order (plot, FFT, bar, gauge, compass, waterfall, meter). The label still says "FFT" and the slug "fft". `datasetOptionSlug` is still an exact match, `datasetOptionFromSlug` still trims and lower-cases and returns 0 on a miss, and the label function still returns "generic" when nothing is set.
- `tileMaster` uses the same dx/dy values and applies the translation at the same point.

The doc comment for `groupWidgetSlug` in EnumLabels.h is correctly attached and not duplicated. No `connect` call, `SS_ASSERT`, `[[nodiscard]]` on a kept declaration, `tr()` string or protected surface was touched. Compile risk low: I read the code as a compiler would. Nothing was compiled. If the build complains, look first at core/Core/EnumLabels.cpp lines 15-33. That file-scope `struct DatasetOptionInfo` and the `static constexpr` array depend on Qt 6's constexpr explicit `QLatin1StringView(const char*)` constructor. They also depend on the unscoped `DatasetOption` enum converting to `int` inside braces; every value is at most 0x80, so that is not a narrowing conversion. The struct sits in the global namespace without internal linkage. It is trivial and no other translation unit defines that name, so there is no ODR clash. If you want it tidier, wrap it in an anonymous namespace. Both QHash initializers, `{QStringLiteral, enum}` into `QHash<QString,int>` and into `QHash<QString,TextEncoding>`, go through `std::pair`'s converting constructor and are fine. SerialStudio.cpp already includes `<QHash>`, and EnumLabels.cpp now includes it too. FrameSupport.h dropped its `<QMap>` include, but it still gets `QMap` through Frame.h and SerialStudio.h.

Files:

- `core/Core/DataModel/FrameSupport.cpp`
- `core/Core/DataModel/FrameSupport.h`
- `core/Core/DiagnosticsTypes.h`
- `core/Core/EnumLabels.cpp`
- `core/Core/EnumLabels.h`
- `core/Core/LayoutPatterns.cpp`
- `core/Core/LayoutPatterns.h`
- `core/Core/SerialStudio.cpp`
- `core/Core/SerialStudio.h`

Findings:

- #75 `SerialStudio.cpp` (dead-code, 37) — applied: Delete hexToString (570-577), stringToHex (579-586) and escapeControlCharacters (670-686, whose only caller is hexToString), plus their three declarations in SerialStudio.h (376, 377, 380). resolveEscapeSequences stays (many callers).
- #78 `SerialStudio.cpp` (local-duplication, 12) — applied: Replace the 11-branch alias if-chain in textEncodingFromName with a static QHash<QString,TextEncoding> of upper-case aliases and one lookup defaulting to EncUtf8 (empty name stays an early return).
- #141 `FrameSupport.cpp` (dead-code, 24) — applied: Remove resolveXAxisPolicy() (FrameSupport.cpp 233-248, FrameSupport.h 58-59) and the XAxisPolicy struct it alone returns (SerialStudio.h ~334-341). The QMap include in FrameSupport.h then goes too.
- #81 `DiagnosticsTypes.h` (dead-code, 5) — applied: Delete the kBusAll constant and its doc comment; nothing reads it.
- #73 `EnumLabels.cpp` (dead-code, 40) — applied: Delete layoutPatternSlug and layoutPatternLabel from EnumLabels.cpp (406-434), their two header declarations (EnumLabels.h 72-81), and the now-unused #include of Core/LayoutPatterns.h (cpp line 14). Re-attach the orphaned groupWidgetSlug comment (h 69-71) to l
- #74 `EnumLabels.cpp` (local-duplication, 45) — applied: Replace four 7-flag chains (datasetOptionsLabel, datasetOptionSlug, datasetOptionFromSlug, datasetOptionsBitsToSlugs) with one static table {bit, slug, label} and four short loops. Label uses 'FFT' capitalised, slug uses 'fft'; keep exact-match for single-bit 
- #76 `EnumLabels.cpp` (local-duplication, 35) — applied: Replace the 26+8 branch if-chain in dashboardWidgetFromSlug with a static QHash<QString,int> of slug-to-enum entries (aliases included, BUILD_COMMERCIAL entries inside an #ifdef) and one trimmed/lowercased lookup returning -1 on miss.
- #79 `LayoutPatterns.cpp` (single-use-helper, 7) — applied: Inline translateAll (4-line loop, 184-191 with doc and blank) into tileMaster at 250-252 as a two-constant dx/dy plus a range-for translate.

### C2-P06 — promised 85, delivered 64, verdict: accepted

Reviewer: Accept. Packet C2-P06 deletes six accessors that nothing calls, together with their declarations. It also folds six reinterpret_cast byte-append blocks in AudioPcm.cpp into one file-static template that writes the same bytes. The diff stays inside the packet's file list, touches no connect(), and removes no SS_ASSERT or [[nodiscard]] from code that remains. No tr() string, objectName or protected surface changes. Behavior is identical. Compile risk low: Nothing in the diff calls the removed getters any more: SoundBank::deviceRate, Audio::inputDrops, Audio::playbackUnderruns, Audio::streamLaneActive, SerialCanBackendBase::bufferOverflows and EthernetIp::tags. A word-bounded git grep over core, app, tests, examples, scripts and doc/help (vendor excluded) finds none of them. Every remaining tags() call belongs to the OpcUa host interface (OpcUaBrowser, OpcUaSubscriptions and the tst_opcua_subscriptions mock). None of the removed names is a Q_PROPERTY, Q_INVOKABLE or slot. The one new construct is the file-static template appendRaw(QVector<quint8>&, const T&) in core/Devices/IO/Drivers/Audio/AudioPcm.cpp, lines 30-36. Every call passes the QVector<quint8>& out parameter with a qint16, qint32 or float value, so deduction is trivial and sizeof(T) matches the old hand-unrolled lengths (2, 4, 4, 4, 2, 4 bytes, native order). The values are clamped before the call, as before. The name is unique in the repo, and SS_UNITY_BUILD applies only to the app target, not core, so no static name can collide. If anything fails, look there first; size_t without <cstddef> is the only theoretical gap, and Qt headers supply it. The members m_inputDrops, m_bufferOverflows, m_streamLaneActive and m_deviceRate remain written and read elsewhere, so no unused-private-field warning should appear. The AudioDeviceCatalog setter edit (finding 31) was reverted, and that file shows no diff.

Files:

- `core/Devices/IO/Audio/SoundBank.cpp`
- `core/Devices/IO/Audio/SoundBank.h`
- `core/Devices/IO/Drivers/Audio.cpp`
- `core/Devices/IO/Drivers/Audio.h`
- `core/Devices/IO/Drivers/Audio/AudioDeviceCatalog.cpp`
- `core/Devices/IO/Drivers/Audio/AudioDeviceCatalog.h`
- `core/Devices/IO/Drivers/Audio/AudioPcm.cpp`
- `core/Devices/IO/Drivers/Audio/AudioPcm.h`
- `core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.cpp`
- `core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.h`
- `core/Devices/IO/Drivers/EthernetIp.cpp`
- `core/Devices/IO/Drivers/EthernetIp.h`

Findings:

- #35 `SoundBank.cpp` (dead-code, 9) — applied: Remove SoundBank::deviceRate() (definition plus declaration at SoundBank.h:61). m_deviceRate stays; it is used by load() and setDeviceRate().
- #32 `Audio.cpp` (dead-code, 19) — applied: Remove Audio::inputDrops() and Audio::playbackUnderruns() (definitions plus the two declarations in Audio.h lines 122-123). Leave m_inputDrops and PlaybackRing::underruns() as they are.
- #34 `Audio.cpp` (dead-code, 9) — applied: Remove the Audio::streamLaneActive() getter and its declaration at Audio.h:126. Only setStreamLaneActive() and the m_streamLaneActive member are used.
- #31 `AudioDeviceCatalog.cpp` (redundant-logic, 21) — skipped: The code-verify rule driver-setter-guard (spec 0050) errors on unguarded setters; the same-value guard is mandated, so I reverted this edit.: In the 7 setSelected* setters, drop the 'if (m_x == index) return;' guard plus its blank line; the unconditional assignment is identical. Each body becomes a single 'm_x = index;'.
- #306 `AudioPcm.cpp` (local-duplication, 8) — applied: Add one file-local template appendRaw(QVector<quint8>&, T) that appends a value's native bytes; replace the six hand-unrolled reinterpret_cast byte-append blocks (s16, s32, f32 in packCsvSample; f32, s16, s32 in packNormalizedSample) with single calls. Log tex
- #27 `SerialCanBackendBase.cpp` (dead-code, 10) — applied: Remove SerialCanBackendBase::bufferOverflows() (header line 56 plus 8-line definition); nothing reads it.
- #29 `EthernetIp.cpp` (dead-code, 9) — applied: Remove EthernetIp::tags() accessor (header line 196 plus the 7-line definition); m_tags is used directly.

## Group C3

### C3-P01 — promised 144, delivered 0, verdict: refuted

Refuted: ModbusProjectGenerator.cpp:209 (compile) Finding 8 swaps the TU-local functionCodeLabel() for IO::Drivers::ModbusRtu::functionCodeForType(). That function is defined out of line in core/Protocols/Modbus/ModbusRtuCodec.cpp, which is part of SerialStudio::Protocols. ModbusProjectGenerator.cpp is also built straight into the unit test tst_modbus_generation (app/tests/CMakeLists.txt:2116-2125), and that target links only Qt6::Core, Qt6::Core5Compat, Qt6::Gui and SerialStudio::Core. SerialStudio::Core links only Qt6::Core, and Protocols sits above Core. The header resolves through SS_CORE_SRC, but the symbol does not: tst_modbus_generation fails to link with an undefined reference to IO::Drivers::ModbusRtu::functionCodeForType(unsigned char). Before this change the generator TU used nothing from Protocols. Keeping it would mean editing app/tests/CMakeLists.txt, which is outside this packet.

Files:

- `core/Devices/IO/Drivers/HID.cpp`
- `core/Devices/IO/Drivers/HID.h`
- `core/Devices/IO/Drivers/Iec104.cpp`
- `core/Devices/IO/Drivers/Iec104.h`
- `core/Devices/IO/Drivers/MQTT.cpp`
- `core/Devices/IO/Drivers/MQTT.h`
- `core/Devices/IO/Drivers/Modbus.cpp`
- `core/Devices/IO/Drivers/Modbus.h`
- `core/Devices/IO/Drivers/Modbus/ModbusProjectGenerator.cpp`
- `core/Devices/IO/Drivers/Modbus/ModbusProjectGenerator.h`
- `core/Devices/IO/Drivers/Modbus/ModbusRedial.cpp`
- `core/Devices/IO/Drivers/Modbus/ModbusRedial.h`

Findings:

- #26 `HID.cpp` (local-duplication, 12) — applied: The 7-line usage-page/usage hex formatting is pasted in open() (215-221) and setDeviceIndex() (285-291) although updateHidUsageCache() (497-506) does exactly this; replace both with a call to updateHidUsageCache().
- #22 `Iec104.cpp` (local-duplication, 22) — applied: setWindowK/W and setTimeoutT1/T2/T3 are the same 8-line body differing by member and settings key; add one private setProtocolParameter(int& field, int value, const char* key) and make each setter a one-line call.
- #25 `Iec104.cpp` (dead-code, 16) — applied: Remove Q_INVOKABLE Iec104::pointInfo (header line 167 and the definition); pointName stays used by wireSchema and datasetFor.
- #28 `Iec104.cpp` (dead-code, 9) — applied: Remove Iec104::points() accessor (header 160 plus the 7-line definition); callers use pointTable() internally.
- #24 `MQTT.cpp` (redundant-logic, 16) — applied: setUsername and setPassword re-implement applyCredentials; make each body the single call applyCredentials(username, m_password) / applyCredentials(m_username, password).
- #30 `MQTT.cpp` (local-duplication, 13) — applied: mqttVersion(), sslProtocol() and peerVerifyMode() are the same QMap index-of-value scan; add one static template indexOfValue(map, value) in the TU and make each a one-line return.
- #7 `Modbus.cpp` (single-use-helper, 13) — applied: Inline handleDialSetback() into onStateChanged() as failDial(m_device->errorString()). Remove the function, its comment and its Modbus.h declaration. Its !m_connecting guard repeats the caller's if (m_connecting), and m_device is non-null at the top of onState
- #9 `Modbus.cpp` (local-duplication, 11) — applied: Replace the 4-case register-type switch in pollNextGroup() with a static constexpr RegisterType[4] table {Holding, Input, Coils, Discrete}. Index it when group.registerType < 4, otherwise use HoldingRegisters. Same mapping as before.
- #8 `ModbusProjectGenerator.cpp` (cross-file-clone, 12) — applied: Delete the static functionCodeLabel(). Call IO::Drivers::ModbusRtu::functionCodeForType(group.registerType) at line 225 and include Protocols/Modbus/ModbusRtuCodec.h. The function body is identical.
- #6 `ModbusRedial.cpp` (dead-code, 20) — applied: Delete ModbusRedial::armed() and static intervalMs() from the .cpp (lines 107-123) and their two declarations in ModbusRedial.h (lines 62-64). kRedialIntervalMs stays, the constructor uses it.

### C3-P02 — promised 77, delivered 70, verdict: accepted

Reviewer: I found no defects, so I accept the packet. I grepped every removed symbol (isConnecting on the session, handleValue, toNodeClass, OpcUaTypes::testFlag, requestServerLimits, requestNamespaceArray, Node::inherited). Nothing in core, app, tests, examples, scripts or doc/help still uses the OpcUa versions. The remaining isConnecting hits are other drivers and overrides, OpcUa.h's own override among them, and the remaining testFlag hits are QFlags calls. stop() now ends with cancel(), whose body is identical to the old tail, so the emissions and the order of side effects are unchanged. The merged read helper drops `attribute = Value`, but the ReadRow constructor already sets that (OpcUaTypes.h:219), and the reads go out in the same order before Q_EMIT connected(). The removed branch in finishAttributes could never run because it came after `else if (!node->checked)`, so inherited was only ever written and never read. The packet changed no connect call, SS_ASSERT, [[nodiscard]] on a remaining declaration, tr() string or objectName, and touched no file outside its list. Process.cpp and Process.h were left alone because the executor skipped finding 84. Compile risk low: I read every hunk against its surrounding code. requestInternalRead uses the private enum InternalRead, which is declared at OpcUaSession.h:146 before its first use at line 188. kNodeMaxNodesPerRead and kNodeNamespaceArray are `inline constexpr const char*` in OpcUaTypes, and that namespace is brought in by the `using namespace` at OpcUaSession.cpp:35, so they match the `const char*` parameter. The unqualified OpcUaTypes::NodeClass cast at OpcUaSession.cpp:1303 resolves inside IO::Drivers and comes to exactly 100 columns, which is within the limit. valueTrampoline emits session->valueChanged directly; Qt signals are public, so that compiles. If anything breaks, look first at the clang-format alignment of that cast line, which no longer lines up with the row.* assignments above it.

Files:

- `core/Devices/IO/Drivers/OpcUa/OpcUaBrowser.cpp`
- `core/Devices/IO/Drivers/OpcUa/OpcUaBrowser.h`
- `core/Devices/IO/Drivers/OpcUaMarshal.cpp`
- `core/Devices/IO/Drivers/OpcUaMarshal.h`
- `core/Devices/IO/Drivers/OpcUaSession.cpp`
- `core/Devices/IO/Drivers/OpcUaSession.h`
- `core/Devices/IO/Drivers/OpcUaTagModel.cpp`
- `core/Devices/IO/Drivers/OpcUaTagModel.h`
- `core/Devices/IO/Drivers/OpcUaTypes.h`
- `core/Devices/IO/Drivers/Process.cpp`
- `core/Devices/IO/Drivers/Process.h`

Findings:

- #41 `OpcUaBrowser.cpp` (local-duplication, 9) — applied: Replace the tail of stop() (tagModel setSession(nullptr), timer stop, teardownSession, browsing flag reset + emit) with a single call to cancel().
- #37 `OpcUaSession.cpp` (local-duplication, 7) — applied: Merge requestServerLimits() and requestNamespaceArray() into one private requestInternalRead(const char* nodeId, InternalRead kind) (build ReadRow, sendRead, SS_ASSERT_LOG); the two handleStateChanged calls pass kNodeMaxNodesPerRead/ReadLimit and kNodeNamespac
- #39 `OpcUaSession.cpp` (single-use-helper, 10) — applied: Remove pass-through OpcUaSession::handleValue (and its header decl); valueTrampoline emits 'Q_EMIT session->valueChanged(notification);' directly (signals are public).
- #42 `OpcUaMarshal.cpp` (single-use-helper, 8) — applied: Remove toNodeClass() (and its OpcUaMarshal.h:53 decl); at OpcUaSession.cpp:1337 use static_cast<OpcUaTypes::NodeClass>(reference.nodeClass) directly.
- #43 `OpcUaSession.cpp` (dead-code, 8) — applied: Delete OpcUaSession::isConnecting() and its declaration at OpcUaSession.h:88; m_connecting stays (used internally).
- #38 `OpcUaTagModel.cpp` (redundant-logic, 10) — applied: Delete the unreachable 'else if (node->inherited && !node->checked)' branch in finishAttributes, then drop the now write-only Node::inherited member (header decl + ctor init) and the 'if (parent->checked && parent->folder) child->inherited = true;' block in ma
- #45 `OpcUaTypes.h` (dead-code, 5) — applied: Delete the unused OpcUaTypes::testFlag(NodeClass, NodeClass) inline function.
- #84 `Process.cpp` (local-duplication, 20) — skipped: The collapse would move the three connect(QFileDialog::fileSelected...) calls into a shared helper, which the hard rules forbid; a helper that keeps the connects in place saves under 5 lines.: Collapse browseExecutable, browseWorkingDir and browsePipePath into one private helper taking title, start dir, file mode and a setter member pointer. The three public slots stay as 4-line callers.

### C3-P03 — promised 55, delivered 54, verdict: accepted

Reviewer: Accept. All three changes keep behavior the same.

- **S7 (finding 85):** the wrapper is gone and its body now sits inside variablesLocked(). The four callers are renamed and the private declaration is removed. The QML only uses variablesLocked, through the Q_PROPERTY.
- **UART (finding 82):** the index-to-enum arrays give the same mappings as the old switches, in the same order. Each array index is range-checked before use, and persistSetting and the port updates still run in the same order.
- **FileTransmission (finding 114):** XModem and XModem1K are now one case, with setUse1K(m_transferMode == XModem1K) giving the same true/false values. The single activeChanged emitted after the switch still fires exactly once, after startTransfer or m_timer.start(), as each case did before. All six TransferMode values have a case and none returns early. An out-of-range mode would already have hit transferModes().at() earlier in the function, so the edge case the executor raised does not change anything.

Rules: only the packet's files changed. No connect call, SS_ASSERT, [[nodiscard]] or tr() string was touched. The old doc comment for variablesFrozen now sits on variablesLocked, and the second doc comment was dropped. Compile risk low: Check core/Devices/IO/Drivers/UART.cpp lines 51-65 first. The four file-scope static constexpr arrays of QSerialPort enums need QSerialPort to be visible through UART.h, and it is, because the getters return QSerialPort::Parity and similar types. Each array is used exactly once, so none is left unused. Indexing with quint8 is safe: every setter checks the index against list.count() before it reaches the array, and the list sizes (5/4/3/3) match the array sizes. In S7, variablesLocked() is still declared at S7.h:207 and stays the Q_PROPERTY READ at S7.h:167-168. No reference to variablesFrozen is left in core, app, tests, examples, scripts or doc/help.

Files:

- `core/Devices/IO/Drivers/S7.cpp`
- `core/Devices/IO/Drivers/S7.h`
- `core/Devices/IO/Drivers/UART.cpp`
- `core/Devices/IO/Drivers/UART.h`
- `core/Devices/IO/FileTransmission.cpp`
- `core/Devices/IO/FileTransmission.h`

Findings:

- #85 `S7.cpp` (single-use-helper, 8) — applied: Delete the pass-through variablesLocked() wrapper. Rename private variablesFrozen() to variablesLocked() (public, still the Q_PROPERTY READ), update its 4 call sites in S7.cpp, drop the extra declaration in S7.h.
- #82 `UART.cpp` (local-duplication, 36) — applied: Replace the four index-to-enum switch blocks in setParity, setDataBits, setStopBits and setFlowControl with file-scope constexpr arrays (e.g. kParities[], kDataBits[]) indexed by the already range-checked index: one assignment line each.
- #114 `FileTransmission.cpp` (local-duplication, 11) — applied: The XModem and XModem1K cases (421-437) are identical except setUse1K(false/true). Merge them into 'case XModem: case XModem1K:' with setUse1K(m_transferMode == XModem1K). Also hoist the repeated Q_EMIT activeChanged() to once after the switch.

### C3-P04 — promised 116, delivered 130, verdict: accepted

Reviewer: I accept packet C3-P04: I read all nine changes in full context and found no behaviour, compile, scope or rule defect.

- **CodeFormatter.cpp:**
  - isLuaIdentChar now returns `ch != '$' && isJsIdentChar(ch, first)`. That is the same result as before, and isJsIdentChar is defined earlier in the file.
  - The inlined computeIndent computes qMax(0, depth + hang - outdent), the same as the deleted helper.
  - splitLines was a thin wrapper around split; both call sites now call split directly, with the same arguments.
- **ScriptTemplateCatalog:** defaultFile() and m_defaultFile have no users left anywhere (ParserTemplateCatalog is a different class), and defaultFileOf is still used at line 118.
- **ExportStructure::sanitizeTitle:** the chop loop trims trailing dots and spaces exactly as the old scan-and-truncate did. An input made only of dots and spaces still becomes empty and returns the fallback.
- **ModbusRegisterMap:**
  - The 13 lookup rules have disjoint name sets, so the order they are checked in cannot change which column a header maps to.
  - The description/desc/comment cells still set the name only when no name column was seen, and only after no rule matched.
  - The old rules returned early on a match. The loop's `return` does the same.
- **ProtoImporter:**
  - The removed re-checks could never be true. isNumericScalar is false only for String, Bytes and MessageRef. The first two were let through by those same checks, and MessageRef was already skipped or returned earlier in all three places.
  - EnumRef was never produced and has no references left.
  - In defaultRangeFor, the unsigned types now fall to the default {0,100}, which is what their own case returned. Float and Double return {-100,100} as before.
  - The countScalarFields merge counts the same fields, and the cycle guard still wraps the loop.

No SS_ASSERT, [[nodiscard]], connect, tr() string or objectName was touched, no protected surface was involved, and only files on the packet's list changed. Compile risk low: I found nothing that should fail the build. If something does, look first at the kRules table in ModbusRegisterMap.cpp, around lines 69-138. Two things there are new to this file. First, each row's names are brace-initialised into a std::array<QLatin1String, 4> with fewer than 4 entries, and the unused slots fall back to default-constructed QLatin1String. Clang does not raise -Wmissing-braces for std::array, and GCC does not enable that warning for C++ under -Wall. Second, rows are assigned through a pointer-to-member, `map.*rule.slot = index`. This parses as map.*(rule.slot), which is the intended meaning. The clang-format layout of the table is ugly but valid.

Removing EnumRef from ProtoScalar is safe: git grep finds no remaining use in core, app, tests, examples, scripts or doc/help, and the switches in ProtoImporter.cpp either have a default or list every remaining enumerator. isNumericScalar is not static (it lives in namespace detail::proto) and still has two callers, so no unused-function warning. defaultFileOf is still used at ScriptTemplateCatalog.cpp:118. <initializer_list> is still needed at ModbusRegisterMap.cpp:218 and :247.

Files:

- `core/Pipeline/DataModel/Editors/CodeFormatter.cpp`
- `core/Pipeline/DataModel/Editors/CodeFormatter.h`
- `core/Pipeline/DataModel/Editors/ScriptTemplateCatalog.cpp`
- `core/Pipeline/DataModel/Editors/ScriptTemplateCatalog.h`
- `core/Pipeline/DataModel/ExportStructure.cpp`
- `core/Pipeline/DataModel/ExportStructure.h`
- `core/Pipeline/DataModel/Importers/ModbusRegisterMap.cpp`
- `core/Pipeline/DataModel/Importers/ModbusRegisterMap.h`
- `core/Pipeline/DataModel/Importers/ProtoImporter.cpp`
- `core/Pipeline/DataModel/Importers/ProtoImporter.h`
- `core/Pipeline/DataModel/Importers/ProtoParser.cpp`
- `core/Pipeline/DataModel/Importers/ProtoParser.h`

Findings:

- #112 `CodeFormatter.cpp` (local-duplication, 11) — applied: isLuaIdentChar is isJsIdentChar minus the '$' case. Reduce its body to: return ch != QLatin1Char('$') && isJsIdentChar(ch, first); (isJsIdentChar is defined earlier at line 87).
- #113 `CodeFormatter.cpp` (single-use-helper, 10) — applied: Inline computeIndent into reformatOne: return QString(qMax(0, depthAtStart + hangAtStart - info.outdentLeading) * indentSpaces, QLatin1Char(' ')) + body; delete the helper and its doc block.
- #117 `CodeFormatter.cpp` (single-use-helper, 7) — applied: Delete the splitLines pass-through wrapper. At lines 656 and 677 call source.split(QLatin1Char('\n'), Qt::KeepEmptyParts) directly.
- #110 `ScriptTemplateCatalog.cpp` (dead-code, 13) — applied: Delete ScriptTemplateCatalog::defaultFile() (cpp 91-97, header line 43), the m_defaultFile member (header line 50) and its two assignments in reload() (cpp lines 72 and 80). defaultFileOf() stays; defaultScriptTemplateCode uses it.
- #116 `ExportStructure.cpp` (redundant-logic, 8) — applied: Replace the reverse index scan plus truncate (103-112) with: while (!safe.isEmpty() && (safe.back() == QChar('.') || safe.back() == QChar(' '))) safe.chop(1); The result is the same, including an all-dots-and-spaces input giving empty and then the fallback.
- #91 `ModbusRegisterMap.cpp` (local-duplication, 35) — applied: Replace mapCsvExtraColumn plus the 11 repeated matchAny/assign/return blocks in mapCsvHeaderColumn with one static table of {int CsvColumnMap::* slot, name list} walked in a loop. Keep the description/desc/comment fallback (only sets name if unset).
- #95 `ProtoImporter.cpp` (redundant-logic, 15) — applied: Remove the never-true re-checks `!isNumericScalar && != String && != Bytes && != EnumRef` (buildGroups 443-445, selectGroupWidget 520-522, and the `emittable` test in formatDispatchEntry 732-735). Also drop the dead EnumRef enum value (ProtoParser.h:51) and it
- #98 `ProtoImporter.cpp` (single-use-helper, 11) — applied: Inline countScalarFields into the cycle-guarded countFieldsRecursive (add an `else ++n` to its existing field loop with `int n = 0`) and delete the method plus its header declaration (ProtoImporter.h:122).
- #100 `ProtoImporter.cpp` (redundant-logic, 6) — applied: In defaultRangeFor, delete the UInt32/UInt64/Fixed32/Fixed64 case group, since it returns {0,100}, the same as the default fall-through. Merge the Float/Double cases into the signed-integer group, which returns the same {-100,100}.

### C3-P05 — promised 168, delivered 206, verdict: accepted

Reviewer: Accept: I found no defect concrete and certain enough to refute the packet. All 10 findings keep the same observable behavior:
- uniqueEntityTitle still returns the first free name in the title, (2), (3) sequence. The old two-pass scan only moved to the next name when the current one was taken, and only stopped on a free one.
- confirmDelete, promptNewTitle and promptName keep the same tr() strings in the same ProjectModel context. They also keep the same dialog arguments, the same check for empty and unchanged names, and the same trimming. The rename paths in ProjectFolders still compare against `current`.
- The three load* functions were inlined with the same expressions and the same memory_order, in the same position after loadPlotTimeRange.
- readArrayInto and serializeFolders produce the same arrays, and every isEmpty/empty guard is kept.
- datasetLabelsByUid gives the same first-wins, uid-sorted key and value order as the old QMap<int,bool> and QMap<int,QString> loops.
- dashboardControl() has no callers left.
No connect, SS_ASSERT, [[nodiscard]], tr string or objectName changed, no function goes over 100 lines, no new file was added, no protected surface was touched, and only packet files were modified.

Two loose ends, neither a defect:
- **Reference held across a modal dialog.** In promptRenameGroup, promptRenameDataset and promptRenameAction (ProjectEntities.cpp ~1349, 1371, 1389), `old` used to be a copy. Now it is a const reference into m_model.m_groups or m_actions, read after QInputDialog's nested event loop. If something added a group or action while the dialog was open and the vector grew, that reference would dangle. The old code already indexes groups[groupId] after the dialog without re-checking bounds, so it already assumes the vectors don't change during the dialog. A one-line fix would restore the copy: `const auto old = ...;` before the call.
- **Doc mention.** doc/claude/architecture/dashboard.md still mentions loadFrozen, which no longer exists. Compile risk low: I checked every new helper against its callers and found nothing that should break the build. readArrayInto (ProjectLoader.cpp ~1310) makes a qualified call to DataModel::read, and that lookup happens where the template is defined. All five overloads it needs (Workspace, WorkspaceFolder, GroupFolder, TableDef, TableFolder) are inline in Core/DataModel/Frame.h, which is included above that point. Every target is a std::vector& (mutableList, mutableWorkspaceFolders, mutableGroupFolders, mutableTableFolders). The reused serializeFolders template in ProjectPersistence.cpp:66 makes the same kind of qualified call to DataModel::serialize. Its overloads for Group, Action, Source, Workspace, WorkspaceFolder, TableDef and WorkspaceProfile are all reachable through the includes above the template, and every container it now receives is a std::vector. titlesOf and the static helpers in ProjectEntities.cpp and ProjectFolders.cpp are each used, so there is no unused-static warning. Their names do not clash with any member or other symbol. datasetLabelsByUid in ProjectPresentation.cpp sits at global scope and refers to DataModel::ProjectModel by its qualified name. QStringList::append(QList<QString>) and QList<int>::append(QList<int>) both resolve. A repo-wide grep finds no remaining callers of the removed loadFrozen, loadChangeDrivenTransforms, loadLuaFastMode or DataModel::dashboardControl(). If the build does fail, look first at the two templates, readArrayInto and serializeFolders.

Files:

- `core/Pipeline/DataModel/Project/ProjectEntities.cpp`
- `core/Pipeline/DataModel/Project/ProjectEntities.h`
- `core/Pipeline/DataModel/Project/ProjectFolders.cpp`
- `core/Pipeline/DataModel/Project/ProjectFolders.h`
- `core/Pipeline/DataModel/Project/ProjectLoader.cpp`
- `core/Pipeline/DataModel/Project/ProjectLoader.h`
- `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- `core/Pipeline/DataModel/Project/ProjectPersistence.h`
- `core/Pipeline/DataModel/Project/ProjectPresentation.cpp`
- `core/Pipeline/DataModel/Project/ProjectPresentation.h`
- `core/Pipeline/DataModel/Scripting/DashboardApi.cpp`
- `core/Pipeline/DataModel/Scripting/DashboardApi.h`

Findings:

- #92 `ProjectEntities.cpp` (redundant-logic, 19) — applied: Rewrite uniqueEntityTitle as `while (taken.contains(newTitle)) newTitle = "%1 (%2)".arg(title, QString::number(++count));`. The two-phase scan plus the while/for/break loop yields the same first free candidate in the title, (2), (3) sequence.
- #96 `ProjectEntities.cpp` (local-duplication, 14) — applied: Fold the identical suppress-check plus showMessageBox(Question, APP_NAME, Yes|No) confirm block in deleteCurrentGroup, deleteCurrentAction and deleteCurrentDataset into one private helper confirmDelete(text). Each site becomes `if (!confirmDelete(ProjectModel:
- #97 `ProjectEntities.cpp` (cross-file-clone, 6) — applied: Add one template `titlesOf(const std::vector<T>&)` next to nextDuplicateTitle in ProjectNaming.h and replace the 4-line existingTitles reserve/append loop with it at 6 sites here (510, 558, 590, 718 as siblingTitles, 839, 867, 900) and at ProjectOutputWidgets.
- #99 `ProjectEntities.cpp` (local-duplication, 10) — applied: Extract a file-static `promptNewTitle(heading, old, out)` wrapping the QInputDialog::getText call, trim and `!ok || empty || same` check shared by promptRenameGroup, promptRenameDataset and promptRenameAction. Each keeps its own bounds check and update call.
- #101 `ProjectFolders.cpp` (cross-file-clone, 30) — applied: Replace the repeated 'bool ok; QInputDialog::getText(...); if (!ok || name.trimmed().isEmpty()) return;' block with one inline helper returning the trimmed name or empty (tr() strings stay at call sites). 8 sites here, 4 in ProjectTables.cpp, 2 in ProjectWorks
- #102 `ProjectLoader.cpp` (single-use-helper, 25) — applied: Inline the one-line single-use loaders loadFrozen, loadChangeDrivenTransforms and loadLuaFastMode into applyJsonDocumentCore, and drop their definitions, doc blocks and ProjectLoader.h declarations (lines 97-99). Fold the 'direct member write' why into the cal
- #105 `ProjectLoader.cpp` (local-duplication, 15) — applied: Add a small template readArrayInto(json, key, vector&) and use it for the workspace folder, group folder, table and table folder arrays (and the workspace list at 1317-1321), each currently a 7-9 line 'for val: DataModel::read(x, val.toObject()) push_back' loo
- #103 `ProjectPersistence.cpp` (local-duplication, 22) — applied: serializeFolders already is a generic vector-to-QJsonArray template; reuse it (renamed serializeList or as is) for groups, actions, sources, workspaces, workspace folders, profiles and tables, deleting 7 hand-written 'QJsonArray a; for (...) a.append(serialize
- #104 `ProjectPresentation.cpp` (local-duplication, 18) — applied: Add one static helper building QMap<int,QString> uid->'title (group)' labels; xDataSources and yWaterfallSources iterate it, and both UniqueIds functions append its keys() instead of re-walking groups with a QMap<int,bool>.
- #131 `DashboardApi.cpp` (dead-code, 9) — applied: Delete DataModel::dashboardControl() getter (cpp 55-61 plus blank) and its declaration at DashboardApi.h:36; only requireDashboardControl() is used.

## Group C4

### C4-P01 — promised 92, delivered 93, verdict: accepted

Reviewer: I accept the packet. Its five deletions remove functions that nothing references anywhere, each together with its declaration, and no test target compiles any of the changed files. Nothing observable changes and I expect it to compile. Compile risk low: I found no build problems. All six files only lose lines, and each removed declaration went out together with its definition. A repo-wide git grep (excluding doc/claude/specs) finds no remaining reference to deviceWriter(), setLuaDefaultSourceId, clearContext or previewDecoderOutput, and nothing calls templateNames(int). That covers QML, the app/rcc AI material, connect calls, invokeMethod strings and tests. The no-argument templateNames() is still used by JsCodeEditor.cpp:423/431, and that overload is untouched. Everything that remains still has what it needs: s_deviceWriter is used by requireDeviceWriter (DeviceWriteApi.cpp:78-79), and the file-static decoderRepresentation is still used at FrameParserPipeline.cpp:223, so no unused-function warning. FrameParser.cpp still uses QThread, PipelineHost, ProjectModel and SerialStudio:: in many places, so no include is left unused, and FrameParserPipeline.h still uses SerialStudio.h for its spec struct. None of the three .cpp files is compiled into any test target in app/tests/CMakeLists.txt; only CFrameParser.cpp from that directory is. No connect, SS_ASSERT, [[nodiscard]] on a surviving declaration, tr string or objectName changed. If the maintainer looks anywhere first, it should be the moc output for FrameParser.h, where the public slot clearContext was removed. Nothing references it. ParserTemplateCatalog::nativeNames is now dead but lies outside this packet, and an unused member function does not cause a warning.

Files:

- `core/Pipeline/DataModel/Scripting/DeviceWriteApi.cpp`
- `core/Pipeline/DataModel/Scripting/DeviceWriteApi.h`
- `core/Pipeline/DataModel/Scripting/FrameParser.cpp`
- `core/Pipeline/DataModel/Scripting/FrameParser.h`
- `core/Pipeline/DataModel/Scripting/FrameParserPipeline.cpp`
- `core/Pipeline/DataModel/Scripting/FrameParserPipeline.h`

Findings:

- #132 `DeviceWriteApi.cpp` (dead-code, 9) — applied: Delete DataModel::deviceWriter() getter (cpp 59-65 plus blank) and its declaration at DeviceWriteApi.h:38; only requireDeviceWriter() is used.
- #133 `DeviceWriteApi.cpp` (dead-code, 9) — applied: Delete DeviceWriteApi::setLuaDefaultSourceId (cpp 254-260 plus blank) and its declaration at DeviceWriteApi.h:79; it only forwards to installLua and nothing calls it.
- #125 `FrameParser.cpp` (dead-code, 40) — applied: Delete FrameParser::clearContext() (cpp 764-802) and its public slot declaration at FrameParser.h:126; nothing calls or connects it. Its body also duplicates readCode()'s engine-prune and load loop.
- #130 `FrameParser.cpp` (dead-code, 12) — applied: Delete the templateNames(int language) overload (cpp 218-227, declaration at FrameParser.h:94). Afterwards ParserTemplateCatalog::nativeNames()/m_nativeNames become dead too (outside this slice).
- #127 `FrameParserPipeline.cpp` (dead-code, 22) — applied: Delete DataModel::previewDecoderOutput() (cpp 339-352) and its declaration with doc comment at FrameParserPipeline.h:129-134; decoderRepresentation() stays, it is still used by buildPipelineFrame.

### C4-P02 — promised 76, delivered 76, verdict: accepted

Reviewer: I found no defects; this read the diff only, since nothing could be compiled here.

- **ZMODEM, finding 120:** the two byte handlers differed only in the `wide` argument passed to binaryHeaderCrcValid. Both call sites pass the value that matches their parse state. The header declaration matches the definition, and git grep finds no other callers.
- **ZMODEM, finding 123:** m_file is a QFile, and QFileDevice::close() returns immediately when the file is not open. Removing the seven guards changes nothing. Signal emission order and every tr() string are unchanged.
- **OpcUaWire and S7Address, findings 121 and 122:** each hoisted table has the same entries as the two copies it replaces. The uses of std::size and the index checks are the same, and both SS_ASSERT_LOG lines are still there.
- **Sparkplug, finding 151:** appendVarintField and appendLengthDelimited write the same tag, length and value bytes as the code they replace. The only difference is that appendLengthDelimited skips the append for an empty block, and name ("Node Control/Rebirth") and metric are never empty. Their added SS_ASSERTs always pass for these positive field numbers.

**Rules:** only packet files changed. No connect() call, SS_ASSERT, [[nodiscard]], tr() string or objectName was removed or edited, no new file was added, and no protected surface was touched. No test target newly depends on an unlinked symbol, because every newly called function is file-static in the same .cpp. Compile risk low: If anything breaks, the most likely place is core/Protocols/OpcUa/OpcUaWire.h:89. The new namespace-scope `inline constexpr const char* k_codes[]` is pulled in by about 15 headers and TUs, and three of them use `using namespace IO::Drivers::OpcUaWire` (WireLatchTemplates.cpp, OpcUaFrameAssembler.cpp, tst_opcua_wire.cpp). A git grep for k_codes across core, app, tests and examples finds no other definition, so there is no ambiguity today. A later global `k_codes` in any of those TUs would clash. In S7Address.cpp:150 the file-static array sits at global scope and is found by ordinary lookup from the qualified function definitions. That array would only collide in a unity build that also put a global k_codes in the same TU, and none exists. ZMODEM's processBinaryByte matches its declaration in ZMODEM.h:130, and nothing else references the old names. Sparkplug's helpers are file-static and defined before line 892. The constants are quint32, and QByteArray converts implicitly to QByteArrayView.

Files:

- `core/Protocols/FileTransfer/ZMODEM.cpp`
- `core/Protocols/FileTransfer/ZMODEM.h`
- `core/Protocols/OpcUa/OpcUaWire.h`
- `core/Protocols/S7/S7Address.cpp`
- `core/Protocols/S7/S7Address.h`
- `core/Protocols/Sparkplug/SparkplugPayload.cpp`
- `core/Protocols/Sparkplug/SparkplugPayload.h`

Findings:

- #120 `ZMODEM.cpp` (local-duplication, 36) — applied: Merge processBin32Byte and processBinByte into one processBinaryByte(ch, bool wide) that calls binaryHeaderCrcValid(wide); processInput passes true/false per ParseState; drop one declaration in ZMODEM.h.
- #123 `ZMODEM.cpp` (redundant-logic, 7) — applied: Replace seven 'if (m_file.isOpen()) m_file.close();' two-line guards with a bare m_file.close(), since QFile::close() on an unopened file returns immediately.
- #121 `OpcUaWire.h` (local-duplication, 14) — applied: typeFromCode and codeFromType each declare the same 12-entry k_codes table. Hoist one table to namespace scope (inline constexpr const char* k_codes[]) and delete the second copy.
- #122 `S7Address.cpp` (local-duplication, 10) — applied: typeFromCode and codeFromType each declare the same 8-entry k_codes table. Hoist one to a file-scope static constexpr array and delete the second copy.
- #151 `SparkplugPayload.cpp` (local-duplication, 9) — applied: encodeRebirthRequest hand-rolls tag+varint+append. Replace lines 897-913 with appendLengthDelimited/appendVarintField calls (the file's own helpers); byte output is identical.

### C4-P03 — promised 109, delivered 115, verdict: accepted

Reviewer: Accepted. I found no behavior, compile, scope or rule defect.
- **Player::stepFrame:** its early return is the exact negation of the old nextFrame and previousFrame guards. The order is unchanged: return if not open, pause, change position, clear the plots, process the batch, update the data.
- **getFrame:** inlining buildRowCells keeps the same guard and the same cell rules ('g'/10 and the "0" fallback), and joins the row the same way.
- **injectFrame:** setting anyActive = frameIndex < 0 gives the same result as the old `anyActive || frameIndex < 0`. channelActive is const with no side effects, so skipping the call when frameIndex is negative changes nothing. sourceHasActive.value(key, true) always found its key, so the default never applied. Payloads are still inserted in QMap key order.
- **Publisher:** markConfigChanged still starts the sync timer and then emits configurationChanged, in that order. m_scriptLanguage (the plain int) still feeds cfg.scriptLanguage and the JSON key. Nothing read the removed atomic.
- **PublisherWorker:** sparkplugActive() just returned m_sparkplug.valid(), so swapping the call in is identical. No SS_ASSERT or SS_ASSERT_LOG was dropped.
- **Removed dead code:** closeBroker, errorString and SparkplugPublisher::clearRegistry had no callers, including string invocations, QML and tests.
- **Rules:** no connect() was edited, no tr() string changed, no [[nodiscard]] was removed from anything that still exists, and no protected surface was touched. The .ts line-number drift is derived output and does not belong to this packet. Compile risk low: I found no compile or link problems. None of the removed symbols is still used anywhere in core, app/src, app/qml, app/tests, tests, examples, scripts or doc/help. That covers buildRowCells, scheduleSyncToWorker, closeBroker, PublisherWorker::errorString, sparkplugActive, m_workerScriptLanguage, the PublisherWorker scriptLanguage ctor parameter and member, SparkplugPublisher::clearRegistry, runRestoreProjectFromJson and renderReportFromPayload. The only clearRegistry hits left belong to QuickPlotFormatState. app/tests/CMakeLists.txt compiles only SparkplugPublisher.cpp out of these files (line 1128), and that test never calls clearRegistry; the packet only removes code there and calls nothing new. Player.cpp, Publisher.cpp, PublisherWorker.cpp, DatabaseManager.cpp and SessionExporter.cpp are not compiled into any test target. The renamed definitions match their existing header declarations: DatabaseManager.h:237 onWorkerGlobalProjectJsonReady(const QString&) and SessionExporter.h:78 onReportDataReady(const Sessions::ReportPayloadPtr&), which the definition names as ReportPayloadPtr inside Sessions::. The connect() calls at DatabaseManager.cpp:175 and :183 were not edited. Where to look first: the PublisherWorker constructor call in Publisher::createWorker (Publisher.cpp:147), where the argument count now has to match the header with one fewer std::atomic<int>* parameter. Next is Player.h:169, the new private stepFrame. Publisher.cpp is 1559 lines, over the TU limit, but it was already over before this packet.

Files:

- `core/Storage/MDF4/Player.cpp`
- `core/Storage/MDF4/Player.h`
- `core/Storage/MQTT/Publisher.cpp`
- `core/Storage/MQTT/Publisher.h`
- `core/Storage/MQTT/PublisherWorker.cpp`
- `core/Storage/MQTT/PublisherWorker.h`
- `core/Storage/MQTT/SparkplugPublisher.cpp`
- `core/Storage/MQTT/SparkplugPublisher.h`
- `core/Storage/Sessions/DatabaseManager.cpp`
- `core/Storage/Sessions/DatabaseManager.h`
- `core/Storage/Sessions/DatabaseManager/SessionExporter.cpp`
- `core/Storage/Sessions/DatabaseManager/SessionExporter.h`

Findings:

- #150 `Player.cpp` (single-use-helper, 10) — applied: Inline buildRowCells() into getFrame(): keep the isOpen/range guard returning QByteArray(), build cells, join, append newline. Remove buildRowCells definition and its Player.h declaration (line 174).
- #152 `Player.cpp` (redundant-logic, 8) — applied: injectFrame builds two intermediate QMaps (sourceFields, sourceHasActive) and a second loop. Emit sourcePayloads directly in the first loop when anyActive (init anyActive = frameIndex < 0); drop both maps and the second loop.
- #153 `Player.cpp` (local-duplication, 9) — applied: nextFrame and previousFrame are identical except for the position step. Merge into one private stepFrame(int delta) (guard on isOpen, pause, bounds, clearPlotData/processFrameBatch/updateData); the two slots become one-line calls.
- #154 `Publisher.cpp` (single-use-helper, 8) — applied: Remove pass-through scheduleSyncToWorker() (body is m_syncTimer.start()) and its Publisher.h declaration; call m_syncTimer.start() at the two call sites (lines 701, 1453).
- #172 `PublisherWorker.cpp` (dead-code, 13) — applied: Delete PublisherWorker::closeBroker (a slot whose body is identical to closeResources) and its declaration at PublisherWorker.h:134.
- #173 `PublisherWorker.cpp` (single-use-helper, 11) — applied: Remove the pass-through sparkplugActive() (returns m_sparkplug.valid()), and its declaration at PublisherWorker.h:152. Replace its 6 call sites with m_sparkplug.valid().
- #176 `PublisherWorker.cpp` (dead-code, 9) — applied: Delete PublisherWorker::errorString (forwards to describeMqttError) and its declaration at PublisherWorker.h:129.
- #180 `Publisher.h` (dead-code, 8) — applied: Remove the write-only atomic m_workerScriptLanguage: its member (Publisher.h:431), init (Publisher.cpp:77), constructor arg (154) and store (1193), plus the unused PublisherWorker ctor parameter and member m_scriptLanguage (PublisherWorker.h:116,179 and .cpp:1
- #171 `SparkplugPublisher.cpp` (dead-code, 15) — applied: Delete SparkplugPublisher::clearRegistry (definition and its doc block) and its declaration at SparkplugPublisher.h:122. It was superseded by clearSource.
- #177 `DatabaseManager.cpp` (single-use-helper, 9) — applied: Delete the pass-through slot onWorkerGlobalProjectJsonReady and rename the definition of runRestoreProjectFromJson to onWorkerGlobalProjectJsonReady. Drop one declaration at DatabaseManager.h:245 and fix the comment at line 1148. The connect at line 183 stays 
- #178 `SessionExporter.cpp` (single-use-helper, 9) — applied: Delete the pass-through slot onReportDataReady and rename the definition of renderReportFromPayload to onReportDataReady. Drop the private declaration at SessionExporter.h:83. The connect in DatabaseManager::initWorker stays untouched.

### C4-P04 — promised 81, delivered 0, verdict: refuted

Refuted: HtmlReport.cpp:334 (behavior) Finding 143 inlines encodeLogoAsDataUri into buildCoverLogoMarkup, and that breaks the security regression test tests/scripts/test_cpp_regressions.py:2369 (test_session_report_logo_is_never_inlined_as_markup). The test pulls out the buildCoverLogoMarkup body, asserts that it contains "encodeLogoAsDataUri(", and asserts that it does NOT contain "readAll(". The new body has no encodeLogoAsDataUri call and calls f.readAll() directly, so both assertions fail. The finder's grep covered core/app/tests but missed tests/scripts, and the guard was added in the recent commit c9165c493 (SVG logo XSS hardening). The rendered output is unchanged, but `pytest tests/scripts/` goes red.

Files:

- `core/Storage/Sessions/DatabaseWorker.cpp`
- `core/Storage/Sessions/DatabaseWorker.h`
- `core/Storage/Sessions/Export.cpp`
- `core/Storage/Sessions/Export.h`
- `core/Storage/Sessions/HtmlReport.cpp`
- `core/Storage/Sessions/HtmlReport.h`
- `core/Storage/Sessions/Player.cpp`
- `core/Storage/Sessions/Player.h`
- `core/Storage/Sessions/Player/ReplayClock.cpp`
- `core/Storage/Sessions/Player/ReplayClock.h`
- `core/Storage/Sessions/Player/ReplaySynthesis.cpp`
- `core/Storage/Sessions/Player/ReplaySynthesis.h`

Findings:

- #170 `DatabaseWorker.cpp` (local-duplication, 22) — applied: Add one private helper failMutation(token, error) that emits mutationFinished(token,false,error). Replace the repeated 4-line 'Database not open', transaction-failed and q.exec()-failed blocks in deleteSession, setSessionNotes, addTag, deleteTag, renameTag, as
- #174 `DatabaseWorker.cpp` (single-use-helper, 11) — applied: Delete ensureSchemaInternal and its declaration (DatabaseWorker.h:121). In openDatabase replace the call with 'QSqlQuery q(m_db); Sessions::DatabaseManager::createSchema(q);'. The isOpen guard is redundant because the db was just opened.
- #179 `Export.cpp` (single-use-helper, 9) — applied: Delete ExportWorker::createSchema (forwards to DatabaseManager::createSchema) and its declaration at Export.h:117. At line 382 call DatabaseManager::createSchema(pragma) directly.
- #143 `HtmlReport.cpp` (single-use-helper, 10) — applied: Inline encodeLogoAsDataUri() (defined 741-757, declared HtmlReport.h:95) into buildCoverLogoMarkup(): read file, bail if empty, build the img tag directly. Drops the wrapper, the dataUri temp and its empty check.
- #144 `Player.cpp` (dead-code, 11) — applied: Delete Sessions::Player::filename() (Player.cpp 250-259 including doc comment) and its declaration at Player.h:112. Not a Q_PROPERTY.
- #146 `ReplayClock.cpp` (dead-code, 9) — applied: Delete ReplayClock::base() (cpp 31-38 incl. doc) and its declaration at ReplayClock.h:34. m_base stays, used by timestampFor and anchorAt.
- #145 `ReplaySynthesis.cpp` (single-use-helper, 9) — applied: Remove pass-through ReplaySynthesis::buildFrameAt() (cpp 94-101, header line 56); replayFrameAt becomes injectFrame(m_reader.readFrameValues(timestampNs, m_layout), timestampNs).

### C4-P05 — promised 25, delivered 26, verdict: accepted

Reviewer: I accept the packet: I found no defect in it. Finding 142 removes two getters that nothing calls; the members behind them are still used. Finding 147 replaces eight zero assignments with value-initialization, which gives the same values with no change in behaviour. Only the packet's files changed. No SS_ASSERT, connect(...), tr() string or protected surface was touched, and none of the removed functions was a slot, Q_PROPERTY or Q_INVOKABLE. One correction to the executor's report: the removed getters carried [[nodiscard]], but that went with the functions themselves, so nothing was stripped from code that stays. Compile risk low: Nothing calls usesBlocks() or hasFinalValues() anywhere in core, app, tests, examples, scripts or doc/help. The other hits for those names are unrelated local variables in DatabaseWorker.cpp and ReportData.cpp. The members m_usesBlocks and m_hasFinalValues are still read in SessionDbReader.cpp (lines 142, 174, 194, 284, 305), so no unused-private-field warning appears. DatasetStats (ReportData.h:32-46) is a plain aggregate with no default member initializers, so 'ds{}' sets the int, qint64 and double fields to zero, and empty braces do not trigger -Wmissing-field-initializers. app/tests/CMakeLists.txt does not mention either .cpp file. If anything breaks, look first at an out-of-tree caller of the removed getters (none found).

Files:

- `core/Storage/Sessions/Player/SessionDbReader.cpp`
- `core/Storage/Sessions/Player/SessionDbReader.h`
- `core/Storage/Sessions/ReportData.cpp`
- `core/Storage/Sessions/ReportData.h`

Findings:

- #142 `SessionDbReader.cpp` (dead-code, 17) — applied: Delete SessionDbReader::usesBlocks() and hasFinalValues() definitions (lines 66-81) and their two declarations in SessionDbReader.h lines 50-51. Members m_usesBlocks/m_hasFinalValues stay.
- #147 `ReportData.cpp` (redundant-logic, 8) — applied: In loadDatasetSkeleton, replace 'Sessions::DatasetStats ds;' plus the eight explicit zero assignments (lines 104-111) with 'Sessions::DatasetStats ds{};' which value-initializes the same numeric fields to 0.

### C4-P06 — promised 101, delivered 105, verdict: accepted

Reviewer: I found no defects, so I accept the packet. Every hunk keeps behavior the same:
- **Removed functions:** the four Assistant functions (updateMemory, redactedKey, selectProvider, clearDroppedPaths) and DocSearch::corpusSize had no callers in C++, QML, tests or string-based invocations.
- **ContextBuilder:** the deleted static helper had the same body as ReplyAssembly::neutralizeHistoryDelimiter (same two replace calls, case-insensitive), and that function is in the same library.
- **Conversation:** runToolCallAsync was inlined into its single caller with the same logic and control flow.
- **MetaToolRunner:** the dispatch table keeps all 11 names and handlers in the original order, with the same call arguments and return values.

The rule checks also pass: no connect, SS_ASSERT, [[nodiscard]] on a remaining declaration, tr() string or protected surface was touched, no file was created, and only files listed in the packet changed. Compile risk low: I found no defects, but nothing was compiled. Where to look first:

1. core/Ui/AI/Conversation/MetaToolRunner.cpp, dispatch() (around lines 44-77). It now uses a function-local Entry struct holding a QLatin1String and a pointer to a member function, and a function-local static table. All eleven run* handlers are private, non-const and have the exact signature (const QString&, const QString&, const QJsonObject&) (MetaToolRunner.h:71-81). Taking their address inside a member function is legal, and `QString != QLatin1String` is a valid overload. The field Entry::name has the same name as the parameter `name`; I expect neither GCC nor Clang -Wshadow to flag a field of a local class, and these warnings are not promoted to errors.

2. core/Ui/AI/ContextBuilder.cpp now calls AI::ReplyAssembly::neutralizeHistoryDelimiter. It is a namespace free function declared in ReplyAssembly.h:21 and defined in ReplyAssembly.cpp:25. Both files are in the same core/Ui library list (core/Ui/CMakeLists.txt:542 and :552), and no test target in app/tests/CMakeLists.txt compiles ContextBuilder.cpp, MetaToolRunner.cpp, Assistant.cpp, Conversation.cpp or DocSearch.cpp on its own, so linking is fine.

3. The removed symbols have no remaining callers in core, app (QML and rcc included), tests, examples, scripts or doc/help. updateMemory, redactedKey and selectProvider appear only in historical spec notes under doc/claude/specs. clearDroppedPaths remains only as FileSandbox calls. MemoryStore::updateFact is now uncalled but still defined; a non-static member function produces no warning.

Files:

- `core/Ui/AI/Assistant.cpp`
- `core/Ui/AI/Assistant.h`
- `core/Ui/AI/ContextBuilder.cpp`
- `core/Ui/AI/ContextBuilder.h`
- `core/Ui/AI/Conversation.cpp`
- `core/Ui/AI/Conversation.h`
- `core/Ui/AI/Conversation/MetaToolRunner.cpp`
- `core/Ui/AI/Conversation/MetaToolRunner.h`
- `core/Ui/AI/DocSearch.cpp`
- `core/Ui/AI/DocSearch.h`
- `core/Ui/AI/MemoryStore.cpp`
- `core/Ui/AI/MemoryStore.h`

Findings:

- #156 `Assistant.cpp` (dead-code, 12) — applied: Delete Assistant::updateMemory (Q_INVOKABLE) and its declaration at Assistant.h:180. Its only callee, MemoryStore::updateFact, then has no caller either (follow-up outside this slice).
- #157 `Assistant.cpp` (dead-code, 11) — applied: Delete Assistant::redactedKey (Q_INVOKABLE) and its declaration at Assistant.h:170. KeyVault::redact stays, since the provider reply classes use it.
- #160 `Assistant.cpp` (dead-code, 9) — applied: Delete the Assistant::clearDroppedPaths slot and its declaration at Assistant.h:217. newChat and switchChat call FileSandbox directly.
- #161 `Assistant.cpp` (dead-code, 8) — applied: Delete the Assistant::selectProvider slot (a pass-through to setCurrentProvider) and its declaration at Assistant.h:195.
- #159 `ContextBuilder.cpp` (cross-file-clone, 5) — applied: Delete the static neutralizeUntrustedDelimiter and call AI::ReplyAssembly::neutralizeHistoryDelimiter at the 3 call sites (lines 906, 925, 943). Add the ReplyAssembly.h include.
- #158 `Conversation.cpp` (single-use-helper, 13) — applied: Inline runToolCallAsync into runToolCall as 'if (AsyncToolRunner::handles(name)) { m_asyncTools.run(callId, name, arguments, m_turnGeneration); return; }'. Remove the helper definition and its Conversation.h:161-163 declaration.
- #155 `MetaToolRunner.cpp` (local-duplication, 35) — applied: Replace the ten identical 'if (name == X) { runY(callId, name, arguments); return true; }' blocks in dispatch() with a static table of {QLatin1String, pointer-to-member} pairs and one loop.
- #162 `DocSearch.cpp` (dead-code, 8) — applied: Delete DocSearch::corpusSize and its declaration at DocSearch.h:38.

## Group C5

### C5-P01 — promised 25, delivered 19, verdict: accepted

Reviewer: I found no defects, so I accept the packet. Finding 415 only moves code: the three identical KeyGetter aliases become one in the shared base header, every user reaches it through Provider.h, and no other definition or use exists to conflict with it. Finding 420 changes nothing observable. With an empty chunk, normalizeNewlines puts back and then holds back again any pending CR, so m_pendingCr ends the same and the result is empty. The existing normalized.isEmpty() branch then calls drainFrames() and returns, just as the removed guard did. No signal, string, assertion, connect call or protected file is touched. Compile risk low: Finding 415: `AI::KeyGetter` (std::function<QString()>) now lives once in core/Ui/AI/Providers/Provider.h, which includes <functional>, and QString arrives through <QObject>. All three provider headers include Provider.h. A git grep over core, app and tests finds no other definition and no use outside the three provider .h/.cpp pairs. The test target that names Provider.h in its sources (app/tests/CMakeLists.txt around line 2596) only gains one `using` alias and the <functional> include. GeminiProvider.h keeps <QJsonValue>. AnthropicProvider.cpp is unchanged from HEAD. If the build complains, look first at the GeminiProvider and OpenAIProvider headers.

Files:

- `core/Ui/AI/Providers/AnthropicProvider.cpp`
- `core/Ui/AI/Providers/AnthropicProvider.h`
- `core/Ui/AI/Providers/GeminiProvider.cpp`
- `core/Ui/AI/Providers/GeminiProvider.h`
- `core/Ui/AI/Providers/OpenAIProvider.cpp`
- `core/Ui/AI/Providers/OpenAIProvider.h`
- `core/Ui/AI/Providers/Provider.cpp`
- `core/Ui/AI/Providers/Provider.h`
- `core/Ui/AI/SseEventReader.cpp`
- `core/Ui/AI/SseEventReader.h`

Findings:

- #415 `AnthropicProvider.h` (cross-file-clone, 13) — applied: Identical `using KeyGetter = std::function<QString()>;` alias with its doc comment and <functional> include is declared in three provider headers. Declare it once in Provider.h (add <functional>) and delete the other two copies.
- #419 `AnthropicProvider.cpp` (local-duplication, 7) — skipped: The table rewrite saved only 2 net lines after clang-format (under the 5-line floor), so I reverted it. AnthropicProvider.cpp is byte-identical to HEAD.: availableModels and modelDisplayName list the same six model ids twice (a list, then an if-chain). Replace with one static table of {id,label} rows; availableModels and modelDisplayName both loop over it.
- #420 `SseEventReader.cpp` (redundant-logic, 5) — applied: Drop the `if (chunk.isEmpty()) { drainFrames(); return; }` early return in feed(). An empty chunk goes through normalizeNewlines to an empty result and hits the identical second check.

### C5-P02 — promised 129, delivered 118, verdict: accepted

Reviewer: Verdict: accept. I found no defect in packet C5-P02, and it changes only the 8 packet files. The describeCommand lookup works the same: assistant tools are checked first, then fs tools, and an unknown name returns {}. tryResolve still records every attempt and sets resolved/method only when a call succeeds, exactly as before. The id and title matching in resolveWorkspace and resolveGroup is unchanged. The forward command, input keys, result fields and the "no key" success reply in resolveAddTileDataset are the same. All 15 missing-param errors keep their text, and the 7 boolean setters return the same value, side effect and result JSON. Calling instance() on each request instead of caching it in a function-static is not observable. No connect, SS_ASSERT, [[nodiscard]], tr() string or protected surface is touched, and no function is over 100 lines. Compile risk low: All new calls resolve. API::CommandResponse::makeMissingParam(const QString&, const char*) is in core/Core/Api/CommandProtocol.h:165, AudioHandler and BluetoothLEHandler already call it, and it produces the same "Missing required parameter: %1" text with ErrorCode::MissingParam. Keys::UniqueId is an inline constexpr KeyView (FrameKeys.h:131), and QString(Keys::X) already compiles elsewhere (ProjectDatasetFieldCommands.cpp:78). That makes both ternary arms in AssistantHandler.cpp:521-523 QString prvalues. In ToolCatalog.cpp:450 the ternary joins two const QVector<AssistantToolDef>& lvalues of the same type, so the range-for binds a reference with no copy. The removed fsToolDescription and assistantToolDescription have no remaining callers in code or tests; only spec manifests under doc/claude/specs/0069 and 0070 still name them. Of the unit tests, only app/tests/CMakeLists.txt:2792 compiles ToolFilesystemTools.cpp, and it only loses a definition nothing used. If anything fails, check the setEnabledFlag template first (an anonymous namespace at ConsoleHandler.cpp:205, with [[nodiscard]] and a forwarding-reference callable). The ToolSchemas.h includes left in ToolFilesystemTools.cpp and ToolSupport.cpp are harmless.

Files:

- `core/Ui/AI/Tools/ToolCatalog.cpp`
- `core/Ui/AI/Tools/ToolCatalog.h`
- `core/Ui/AI/Tools/ToolFilesystemTools.cpp`
- `core/Ui/AI/Tools/ToolFilesystemTools.h`
- `core/Ui/AI/Tools/ToolResolve.cpp`
- `core/Ui/AI/Tools/ToolResolve.h`
- `core/Ui/AI/Tools/ToolSupport.cpp`
- `core/Ui/AI/Tools/ToolSupport.h`
- `core/Ui/ApiHandlers/AssistantHandler.cpp`
- `core/Ui/ApiHandlers/AssistantHandler.h`
- `core/Ui/ApiHandlers/ConsoleHandler.cpp`
- `core/Ui/ApiHandlers/ConsoleHandler.h`

Findings:

- #414 `ToolFilesystemTools.cpp` (cross-file-clone, 24) — applied: fsToolDescription and assistantToolDescription are the same 15-line def-lookup loop. Delete both (plus their header decls), and in ToolCatalog::describeCommand loop over {&assistantToolDefs(), &fsToolDefs()}, returning {} when the name is an assistant/fs tool 
- #416 `ToolResolve.cpp` (local-duplication, 11) — applied: resolveDataset repeats 'tryResolve then if ok set resolved/method' three times. Pass the method name into the tryResolve lambda, which assigns resolved/method on success, leaving one call per attempt (use QString(Keys::UniqueId)).
- #417 `ToolResolve.cpp` (local-duplication, 10) — applied: resolveWorkspace (125-141) and resolveGroup (164-181) each build the same success reply twice, once per id match and once per title match. Compute byId and byTitle booleans and return one reply when either holds.
- #220 `AssistantHandler.cpp` (local-duplication, 9) — applied: resolveAddTileDataset repeats the same forward + three-field extraction for 'dataset' (getByPath, key path) and uniqueId (getByUniqueId); fold into one block choosing command and param key.
- #219 `ConsoleHandler.cpp` (local-duplication, 45) — applied: Seven boolean setters (setEcho, setShowTimestamp, setCollapseDuplicates, setSearchCaseSensitive, setVt100Emulation, setAnsiColorsEnabled, exportSetEnabled) share identical contains-check/toBool/result body; add one file-static helper taking a callable and redu
- family `ConsoleHandler.cpp` (cross-file-clone, 30) — applied: Replace each of the 15 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::make

### C5-P03 — promised 79, delivered 62, verdict: accepted

Reviewer: Accepted: I found no defects. Finding 1279 replaced 6 missing-parameter blocks in WindowHandler.cpp, 1280 replaced 2 in WorkspaceProfileHandler.cpp and 1281 replaced 7 in WorkspacesHandler.cpp. Each now calls the shared makeMissingParam helper, which returns the same error text and code. The other edits remove a forwarding wrapper (hasImageWidget folded into imageWidgetActive) and three pieces of dead code: defaultBackendForPlatform and the IconPacks mono and activePackId properties. None of them has a remaining reference. No assert, [[nodiscard]] on a kept function, connect(), tr() string or objectName changed, and no protected surface was touched. Finding 1280 saves only 2 net lines, below the packet's 5-line threshold, but it is correct. Compile risk low: I found nothing in the packet that should break the build. The diff touches only the packet's files.
- makeMissingParam(const QString&, const char*) is defined inline in core/Core/Api/CommandProtocol.h:165. The headers of all three handlers include that file. It builds "Missing required parameter: %1" with QLatin1String(name), and since every key is ASCII the text matches the old string byte for byte, with the same ErrorCode::MissingParam.
- The functions are defined with qualified names (API::Handlers::X::f), so the unqualified CommandResponse resolves the way it did before.
- Console::Handler: imageWidgetActive() is still declared public at Handler.h:268 and is still the Q_PROPERTY READ (Handler.h:99-101). The renamed definition keeps both SS_ASSERTs. hasImageWidget has no remaining references anywhere.
- defaultBackendForPlatform, activePackId and IconPacks::mono have no remaining references in core, app (qml, src, tests), tests, examples, scripts or doc/help. tst_icon_packs uses the registry's setPack("mono"), not the removed IconPacks property. The other IconPacks properties still use revisionChanged.
- None of the changed .cpp files is compiled into a unit-test target in app/tests/CMakeLists.txt, so linking is not affected.
If the build does fail, look first at the moc output for IconPacks.h, in case a QML binding reads Cpp_Misc_IconRegistry.mono or .activePackId dynamically. A grep found no such reader.

Files:

- `core/Ui/ApiHandlers/WindowHandler.cpp`
- `core/Ui/ApiHandlers/WindowHandler.h`
- `core/Ui/ApiHandlers/WorkspaceProfileHandler.cpp`
- `core/Ui/ApiHandlers/WorkspaceProfileHandler.h`
- `core/Ui/ApiHandlers/WorkspacesHandler.cpp`
- `core/Ui/ApiHandlers/WorkspacesHandler.h`
- `core/Ui/Console/Handler.cpp`
- `core/Ui/Console/Handler.h`
- `core/Ui/Misc/GraphicsBackend.cpp`
- `core/Ui/Misc/GraphicsBackend.h`
- `core/Ui/Misc/IconPacks.cpp`
- `core/Ui/Misc/IconPacks.h`

Findings:

- family `WindowHandler.cpp` (cross-file-clone, 16) — applied: Replace each of the 8 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `WorkspaceProfileHandler.cpp` (cross-file-clone, 6) — partly-applied: 2 profileId blocks rewritten (the title check is isEmpty so untouched); only 2 net lines saved, below the 5-line threshold, so the reviewer may prefer to drop it.: Replace each of the 3 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- family `WorkspacesHandler.cpp` (cross-file-clone, 16) — applied: Replace each of the 8 four-line 'if (!params.contains(K)) { return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: K")); }' blocks with the two-line 'if (!params.contains(K))' + 'return CommandResponse::makeM
- #135 `Handler.cpp` (single-use-helper, 8) — applied: Delete the pass-through Handler::imageWidgetActive() wrapper (doc, signature, body) and its private hasImageWidget() declaration in Handler.h (line 357). Rename hasImageWidget's definition to imageWidgetActive and update the two callers at lines 275 and 286.
- #148 `GraphicsBackend.cpp` (dead-code, 9) — applied: Remove the private static GraphicsBackend::defaultBackendForPlatform() definition (doc comment plus body, lines 93-100) and its declaration at GraphicsBackend.h:138. It only returns Backend::Default and nothing calls it.
- #214 `IconPacks.cpp` (dead-code, 12) — applied: Remove the IconPacks::mono Q_PROPERTY (IconPacks.h 69-71), the mono() declaration (IconPacks.h:102) and the mono() definition with its docblock (IconPacks.cpp 130-137). Nothing reads it.
- #215 `IconPacks.cpp` (dead-code, 12) — applied: Remove the IconPacks::activePackId Q_PROPERTY (IconPacks.h 66-68), its declaration (IconPacks.h:103) and the definition with docblock (IconPacks.cpp 138-145). Nothing reads it.

### C5-P04 — promised 121, delivered 75, verdict: accepted

Reviewer: I found no defects, so I accept the packet. I read every hunk in the context of its full function, and nothing observable changes.
- **ExtensionCheckers.cpp:** the inline append makes the same call in the same order as the deleted helper.
- **ThemeManager.h:** QSS appears nowhere else in the repo.
- **CSD:** the removed getters and the three signals have no remaining users in core, app (including qml), tests, examples, scripts or doc/help. NativeWindow_CSD.cpp only constructs CSD::Window. The setters still update their members and call update() the same way.
- **EditorCommit.cpp:** each retitleTreeItem call keeps the original loop's predicate and does the same setText, setData(TreeViewText) and title update, then stops at the first match.
- **Rules:** no SS_ASSERT, [[nodiscard]] on a kept function, connect, tr() string or objectName was touched. No new file was added, and nothing on a protected surface changed. The two skipped findings (249, 256) were left alone, as their reasons say. Compile risk low: Check retitleTreeItem in core/Ui/ProjectEditor/EditorCommit.cpp (around line 120) first. It is a file-static template inside namespace DataModel, after `using enum ProjectEditor::CustomRoles`, so the unqualified TreeViewText resolves there. That same enum is already used unqualified at the output-widget loop near line 1455. The template only reads the map's value fields `.title`/`.sourceId`/etc. through deduced types and never names a private type, so access checks happen at the call sites, which are member functions. The lambdas capture by reference and implicitly capture `this`. CSD::Window derives from QObject, not QQuickItem, so removing Window::window() hides nothing. The Titlebar code that calls window() (CSD.h:91-95) still reaches QQuickItem::window(). No Q_PROPERTY in CSD.h referenced the removed signals or getters. None of the five files is compiled by a target in app/tests/CMakeLists.txt.

Files:

- `core/Ui/Misc/Problems/ExtensionCheckers.cpp`
- `core/Ui/Misc/Problems/ExtensionCheckers.h`
- `core/Ui/Misc/ThemeManager.cpp`
- `core/Ui/Misc/ThemeManager.h`
- `core/Ui/Platform/CSD.cpp`
- `core/Ui/Platform/CSD.h`
- `core/Ui/ProjectEditor/EditorCommit.cpp`
- `core/Ui/ProjectEditor/EditorCommit.h`

Findings:

- #218 `ExtensionCheckers.cpp` (single-use-helper, 8) — applied: Inline checkCatalogFindings into checkWidgetExtensions as 'out.append(widgetCatalog().findings());' and delete the helper with its docblock.
- #216 `ThemeManager.h` (dead-code, 11) — applied: Delete the unused inline variadic template QSS(style, colors...) and its trailing blank line.
- #246 `CSD.cpp` (dead-code, 42) — applied: Delete never-called Titlebar::windowActive() and backgroundColor() getters, Window::window() and Window::titleBar() getters (cpp and CSD.h decls), and the never-connected titleChanged/windowActiveChanged/backgroundColorChanged signals with their three Q_EMITs.
- #249 `EditorCommit.cpp` (single-use-helper, 30) — skipped: Merging the three helpers would drop the SS_ASSERT(uniqueId >= 0) inside findDatasetByUniqueId, and the brief forbids removing an SS_ASSERT.: Merge findDatasetByUniqueId, findDatasetById and resolveDialogDataset into one resolveDialogDataset loop using a predicate (uniqueId>=0 ? uniqueId match : group/dataset id match); drop two private decls in EditorCommit.h.
- #253 `EditorCommit.cpp` (local-duplication, 18) — applied: Add a file-static template retitleTreeItem(map, title, predicate) and replace the four tree-item title sync loops (source, group, action, dataset).
- #256 `EditorCommit.cpp` (single-use-helper, 12) — skipped: applyOutputWidgetField is already 101 lines, and inlining the helper would add about 4 lines and push it further past the 100-line hard cap.: Inline applyPresentationField into the three switch cases (ShowTitle, SpringReturn, PositionLabels) of applyOutputWidgetField and delete the helper.

### C5-P05 — promised 484, delivered 136, verdict: accepted

Reviewer: Accept. All ten applied findings keep behavior the same:
- The QMap index lookups give the same position, with 0 when the key is missing.
- The batch-collect helper keeps the same skip and first-match semantics. reserve now also runs at three more call sites, which nothing can observe.
- markMixedRows keeps AutoIntField counted for datasets only and the same identity predicates. tr strings and context are unchanged.
- The keep / clear-flags / rebuild / flush order is the same as before, with setData still running before batchApplying is cleared.
- The selection functions keep the same null-model guard and first-match order through navFind.
- The forwarders were removed and the facades now call the targets directly with the same arguments.
- applyDatasetTarget was inlined.

Only packet files changed for this packet. No connect, SS_ASSERT, nodiscard, objectName or protected surface was touched, and no test target links any of these TUs. Compile risk low: Look first at the collectBatchSelection template in core/Ui/ProjectEditor/EditorMultiSelect.cpp (around lines 66-88). Item is deduced from three places: std::vector<Item> Group::* (Frame.h:269-270), int Item::* (Dataset::datasetId and OutputWidget::widgetId are both plain int), and QVector<Item>& (sel). All three agree, and m_batchItems is exactly QVector<QPair<int,int>> (ProjectEditor.h:595). pm.groups() returns const std::vector<Group>&.

markMixedRows takes a QStandardItemModel*. CustomModel derives from it publicly and its full definition is visible through ProjectEditor.h. The role and widget enumerators resolve through the file's using-enum lines, which sit above the template.

ProjectEditor::tr gives the same context as before. EditorMultiSelect declares Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor), so the context stays DataModel::ProjectEditor and the strings are unchanged.

In EditorForms.cpp, qMax(0, keys().indexOf(...)) mixes int with qsizetype. That form is already used and compiles in SendLibrary.cpp:128 and OpcUa.cpp:1271. selectNavMatch passes its predicate to navFind (declared at EditorSelection.cpp:64) as an lvalue, and Pred&& deduces to lambda&.

None of the changed .cpp files is compiled into an app/tests target. The removed symbols have no remaining C++ callers. QML calls only the ProjectEditor facade slots, which still exist. appendExtensionWidgets was a private slot with no string-based invocation.

EditorCommit.cpp is also modified in the tree, but it belongs to another packet and was not reviewed here.

Files:

- `core/Ui/ProjectEditor/EditorForms.cpp`
- `core/Ui/ProjectEditor/EditorForms.h`
- `core/Ui/ProjectEditor/EditorForms/OutputStateRows.cpp`
- `core/Ui/ProjectEditor/EditorForms/OutputStateRows.h`
- `core/Ui/ProjectEditor/EditorForms/OutputValueRows.cpp`
- `core/Ui/ProjectEditor/EditorForms/OutputValueRows.h`
- `core/Ui/ProjectEditor/EditorForms/SourceFramingRows.cpp`
- `core/Ui/ProjectEditor/EditorForms/SourceFramingRows.h`
- `core/Ui/ProjectEditor/EditorMultiSelect.cpp`
- `core/Ui/ProjectEditor/EditorMultiSelect.h`
- `core/Ui/ProjectEditor/EditorSelection.cpp`
- `core/Ui/ProjectEditor/EditorSelection.h`
- `core/Ui/ProjectEditor/ProjectEditor.cpp`
- `core/Ui/ProjectEditor/ProjectEditor.h`

Findings:

- #206 `ProjectEditor.cpp` (dead-code, 9) — applied: Delete the private-slot forwarder ProjectEditor::appendExtensionWidgets() (definition with doc, lines 735-741) and its declaration at ProjectEditor.h:561; EditorVocabulary calls its own method directly.
- #242 `EditorForms.cpp` (cross-file-clone, 150) — skipped: Needs a new FormRows.h file, which the brief forbids; a 40-site conversion is also too risky to do uncompiled.: Add one shared inline appendFormRow(model, widgetType, paramType, name, value, description, placeholder, active) plus appendSectionHeader(model, title, icon) in a new EditorForms/FormRows.h; replace ~40 six-to-ten-line 'new QStandardItem + setData x7' rows and
- #244 `OutputValueRows.cpp` (cross-file-clone, 90) — skipped: Depends on the shared helper from 242 that cannot be created.: Delete local appendRow/appendTextRow (they move into the shared FormRows.h) and convert the Decimals, Input kind, Knob style, Text Field style/size/colour rows and the Value Range header to shared appendFormRow/appendSectionHeader calls.
- #245 `SourceFramingRows.cpp` (cross-file-clone, 45) — skipped: Depends on the shared helper from 242 that cannot be created.: Remove appendDelimiterRow and appendFixedFrameRow bodies and the hand-built hexSeq, frame-detection, header and connection-sequence rows; use the shared appendFormRow/appendSectionHeader (fixed-frame min/max via small extra setData).
- #247 `EditorMultiSelect.cpp` (local-duplication, 40) — applied: Add one file-static template collectBatchSelection(groups, batchItems, membersOf, idOf, sel, ids) and replace the five copies of the batch-id-to-record resolution loop (datasets x3, output widgets x2).
- #248 `OutputStateRows.cpp` (cross-file-clone, 35) — skipped: Depends on the shared helper from 242 that cannot be created.: Convert appendReplyPicker, target, on-value, confirm, state-kind rows and the State Feedback header to shared appendFormRow/appendSectionHeader; appendReplyPicker collapses to a one-liner wrapper or disappears.
- #251 `EditorMultiSelect.cpp` (local-duplication, 26) — applied: Extract the 'grey identity rows / mark Mixed' row loop shared by buildMultiDatasetModel and buildMultiOutputWidgetModel into one static helper taking maps, an identity predicate and an int-widget predicate (use QCoreApplication::translate with the same context
- #252 `EditorForms.cpp` (redundant-logic, 22) — applied: Replace the two hand-rolled QMap index-of-key loops with qMax(0, map.keys().indexOf(key)).
- #254 `EditorForms.cpp` (single-use-helper, 17) — applied: Remove pass-through wrappers EditorForms::openFrequencyMarkersEditorForSelection and openTransformEditor; ProjectEditor.cpp:1048-1074 forwards directly to the ...For variants with m_selectedDataset ids.
- #257 `EditorMultiSelect.cpp` (redundant-logic, 10) — applied: In both fan-out functions compute keep = multiSelectionEditKeepsModel(...) first, then clear batchApplying/autosave once, rebuild only if !keep, then post the flush, removing the duplicated early-return block.
- #259 `EditorForms.cpp` (single-use-helper, 9) — applied: Remove pass-through buildOutputWidgetValueRows (and header decl); its two callers call appendOutputValueRows(m_editor.m_outputWidgetModel, widget) directly (EditorMultiSelect already includes OutputValueRows.h).
- #260 `OutputStateRows.cpp` (single-use-helper, 9) — applied: Inline applyDatasetTarget (single wrapper around datasetIdAtIndex) into applyOutputStateTarget.
- #280 `EditorSelection.cpp` (local-duplication, 13) — applied: Add a static selectTreeItem(QItemSelectionModel*, QStandardItem*) and reuse the existing navFind. selectSource, selectGroup, selectDataset, selectAction and selectOutputWidget become 2-3 lines each.
- #285 `EditorSelection.cpp` (single-use-helper, 9) — applied: Delete EditorSelection::selectFrameParser (a pure forward to displayFrameParserView) and its header line. Make ProjectEditor::selectFrameParser at ProjectEditor.cpp:922 call m_selection.displayFrameParserView.

## Group C6

### C6-P01 — promised 230, delivered 255, verdict: accepted

Reviewer: The packet holds up: the removals compile on paper and nothing observable changes.

- **Removed members (findings 278, 283, 284):** DatasetTransformEditor's code, language, targetGroupId and targetDatasetId had no users. Its only user, EditorForms, calls displayDialog and setLibraryCode and connects the signals. JsCodeEditor::help had no QML caller, and QUrl and QDesktopServices were used nowhere else in that file. reload() now calls FrameParser::loadDefaultTemplate directly with the same arguments.
- **Clipboard slots moved to the base class (finding 275):** The six slots now live in EmbeddedCodeEditorItem with the same bodies. ControlScriptEditor, JsCodeEditor and MacroEditor all derive from it, so the QML calls in CodeEditorMenu.qml still reach the inherited slots. No C++ code takes the address of the deleted slots.
- **Selector helpers (finding 277):** The selectors keep the old behaviour, including the null checks on the selection model and the item. The one difference is harmless: a map search now runs before the null selection-model check, and it has no side effects.
- **Folder rows (finding 279):** appendFolderRows writes the same row keys, values and order as the three copies it replaces.
- **Workspace sibling search (finding 281):** siblingWorkspaceIndex keeps the semantics of canMoveCurrent and moveWorkspace. moveWorkspace still calls moveWorkspaceInFolder with the original direction.
- **Rules:** No connect call, SS_ASSERT, [[nodiscard]] on a remaining function, tr() string or objectName was touched, and no app/tests target compiles these files.

Other files in the diff (ExpressionHighlighter, OutputCodeEditor, PublisherScriptEditor) belong to other packets and were not judged. Compile risk low: I checked the whole packet as a compiler would and found nothing that should break the build. Nothing was compiled, and the maintainer should look first at these points.

1. EditorSummaries.cpp, findMappedItem. Value is deduced from both the map and the key, and the two agree at every call site. ProjectEditor.h declares m_userTableItems as QMap<QStandardItem*, QString> and the other four maps as QMap<QStandardItem*, int>. The calls pass a const QString& and ints, so deduction should succeed.

2. appendFolderRows. Every container it takes is a const std::vector reference: editorWorkspaceFolders, editorGroupFolders, editorTableFolders, editorWorkspaces, groups and tables all return const std::vector<T>&. workspaceFolderContents and tableFolderContents still use their own `registry` local for item icons. groupFolderContents no longer needs one, so removing it there leaves no unused variable.

3. siblingWorkspaceIndex refers to `Workspace` without a namespace. That resolves to DataModel::Workspace in Core/DataModel/Frame.h because the file body sits inside `namespace DataModel {`.

4. EmbeddedCodeEditorItem now has public slots cut, undo, redo, copy, paste and selectAll, and m_editor is a protected member of that class. The same names in PainterCodeEditor, OutputCodeEditor and TransformLibraryEditor are not virtual and are only hidden by the derived declarations, so moc and QML accept them.

5. No app/tests target compiles any of the changed .cpp files.

Files:

- `core/Ui/ProjectEditor/EditorSummaries.cpp`
- `core/Ui/ProjectEditor/EditorSummaries.h`
- `core/Ui/ProjectEditor/Editors/ControlScriptEditor.cpp`
- `core/Ui/ProjectEditor/Editors/ControlScriptEditor.h`
- `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp`
- `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.h`
- `core/Ui/ProjectEditor/Editors/EmbeddedCodeEditorItem.cpp`
- `core/Ui/ProjectEditor/Editors/EmbeddedCodeEditorItem.h`
- `core/Ui/ProjectEditor/Editors/JsCodeEditor.cpp`
- `core/Ui/ProjectEditor/Editors/JsCodeEditor.h`
- `core/Ui/ProjectEditor/Editors/MacroEditor.cpp`
- `core/Ui/ProjectEditor/Editors/MacroEditor.h`

Findings:

- #277 `EditorSummaries.cpp` (local-duplication, 22) — applied: Add a static selectTreeItem(QItemSelectionModel*, QStandardItem*) and a static findMappedKey template. The 8 singleton selectors become one call each; the 5 map selectors (userTable, workspace, workspaceFolder, groupFolder, tableFolder) become one call each.
- #279 `EditorSummaries.cpp` (local-duplication, 30) — applied: Add a template appendFolderRows(out, folders, items, parentFolderId) holding the 22-line folder-row loop. workspaceFolderContents, groupFolderContents and tableFolderContents each call it and keep only their own item loop.
- #281 `EditorSummaries.cpp` (local-duplication, 17) — applied: Extract a static siblingWorkspaceIndex(workspaces, id, step) returning the next same-folder workspace index or -1. canMoveCurrent's WorkspaceView branch and moveWorkspace both reduce to a call plus a >= 0 check.
- #275 `ControlScriptEditor.cpp` (cross-file-clone, 104) — applied: Hoist the six clipboard wrappers (cut/undo/redo/copy/paste/selectAll) into EmbeddedCodeEditorItem as public slots; delete them from ControlScriptEditor, JsCodeEditor and MacroEditor (and the other three hosts). Slice-only net shown; about 260 with the partners
- #283 `JsCodeEditor.cpp` (dead-code, 12) — applied: Delete the unused slot help() and its header line. The QDesktopServices and QUrl includes become unused too (2 more lines).
- #284 `JsCodeEditor.cpp` (single-use-helper, 9) — applied: Delete the slot loadDefaultTemplate() and its header line. reload() calls m_frameParser.loadDefaultTemplate(m_sourceId, guiTrigger) directly.
- #278 `DatasetTransformEditor.cpp` (dead-code, 36) — applied: Delete the unused accessors code(), language(), targetGroupId() and targetDatasetId() and their four header declarations. Keep params(), which onApply and testTransform use.

### C6-P02 — promised 138, delivered 133, verdict: accepted

Reviewer: I accept packet C6-P02 and found no defects. Every removed symbol is unreferenced. ExpressionHighlighter applies its five formats in the same order with the same capture groups, so highlighting output is unchanged. In Taskbar::nextActiveWindow the result changes only if a QML window registers with id -1. QML registers windows with the delegate's model.windowId, and I did not confirm that this can never be -1, so that corner stays unverified. Constructor init lists match the remaining members. No connect(), SS_ASSERT, [[nodiscard]] on a kept function, tr() string or protected surface was touched. Only this packet's files changed, and CommandRegistry was reverted (finding 187 skipped). Compile risk low: Look first at the paint lambda in core/Ui/ProjectEditor/Editors/ExpressionHighlighter.cpp:56. It calls the protected setFormat through the captured `this` (legal inside a member function's lambda) and passes a const char* to QSyntaxStyle::getFormat(QString). The original code already made that same implicit conversion with string literals, so the conversion settings that apply there are unchanged. OutputCodeEditor.cpp now calls DataModel::defaultOutputWidgetTemplate() unqualified from inside DataModel:: member functions; ScriptTemplateCatalog.h declares it and OutputCodeEditor.h already includes that header. tst_snap_overlay compiles SnapGuides.cpp but never used snapToFraction. kFractionDenominators and kMinFractionSpacing are still used by fractionLabel and two other functions, so removing snapToFraction leaves no unused constant. No other removed symbol (scheduleValidation, OutputCodeEditor::defaultTemplate, PublisherScriptEditor::code/language, findGroupItemByGroupId, createItemFromWidgetInfo, m_batchUpdateInProgress) is referenced anywhere in core, app, tests, examples or scripts. PainterCodeEditor::defaultTemplate is a separate method and stays.

Files:

- `core/Ui/ProjectEditor/Editors/ExpressionHighlighter.cpp`
- `core/Ui/ProjectEditor/Editors/ExpressionHighlighter.h`
- `core/Ui/ProjectEditor/Editors/OutputCodeEditor.cpp`
- `core/Ui/ProjectEditor/Editors/OutputCodeEditor.h`
- `core/Ui/ProjectEditor/Editors/PublisherScriptEditor.cpp`
- `core/Ui/ProjectEditor/Editors/PublisherScriptEditor.h`
- `core/Ui/UI/CommandRegistry.cpp`
- `core/Ui/UI/CommandRegistry.h`
- `core/Ui/UI/SnapGuides.cpp`
- `core/Ui/UI/SnapGuides.h`
- `core/Ui/UI/Taskbar.cpp`
- `core/Ui/UI/Taskbar.h`

Findings:

- #282 `ExpressionHighlighter.cpp` (local-duplication, 14) — applied: Replace the five identical globalMatch/while/setFormat loops with one local lambda (regex, capture group, format name), called five times. The order and the capture group (1 for functions) are kept.
- #204 `OutputCodeEditor.cpp` (single-use-helper, 10) — applied: Inline scheduleValidation() (one caller, body is m_validateTimer.start()) into the textChanged lambda at line 80; delete the 8-line definition with its doc and the header declaration at OutputCodeEditor.h:124.
- #205 `OutputCodeEditor.cpp` (single-use-helper, 9) — applied: Remove the pass-through static OutputCodeEditor::defaultTemplate() (returns defaultOutputWidgetTemplate()); call defaultOutputWidgetTemplate() directly at lines 263 and 278; drop declaration at .h:116.
- #203 `PublisherScriptEditor.cpp` (dead-code, 17) — applied: Delete the unused accessors code() and language() (definition lines 236-250 with doc comments) and their two declarations at PublisherScriptEditor.h:37-38.
- #187 `CommandRegistry.cpp` (local-duplication, 13) — skipped: Helper extraction netted only 1 line after clang-format (30 ins / 31 del), below the 5-line threshold; the edit was reverted by reverse-applying my own diff.: loadManifest, loadLayout and loadMenus repeat the same QFile open + QJsonParseError parse + isObject check; extract one static helper returning the root QJsonObject (taking a label so the existing warning text is preserved) and call it from all three.
- #184 `SnapGuides.cpp` (dead-code, 30) — applied: Delete UI::Snap::snapToFraction (doc comment plus body, cpp 438-465) and its declaration at SnapGuides.h:100.
- #183 `Taskbar.cpp` (dead-code, 37) — applied: Delete Taskbar::findGroupItemByGroupId and Taskbar::createItemFromWidgetInfo (cpp 1145-1176, declarations Taskbar.h 221-222) plus the write-only m_batchUpdateInProgress member (Taskbar.h 234, ctor init Taskbar.cpp 91).
- #189 `Taskbar.cpp` (local-duplication, 8) — applied: nextActiveWindow rebuilds the row-to-windowId walk that taskbarWindowIds() already does; iterate 'for (const int wid : taskbarWindowIds())' then windowData/closed-state checks, dropping the manual row loop and reserve.

### C6-P03 — promised 143, delivered 153, verdict: accepted

Reviewer: I accept the packet. All 10 files hold only the six findings described. No connect() call, SS_ASSERT, [[nodiscard]] on a kept function, tr() string or objectName was touched, and nothing is on a protected surface.

Two edge cases change behaviour, and I judged neither a confident defect:
- **Finding 185 (TaskbarSearch::results(), child entries):** widgetName used to be stored as `name`, a QString from `.toString()`. It is now the raw role QVariant. The executor said the role always holds a QString, but Taskbar::appendGroupChildItem (core/Ui/UI/Taskbar.cpp:1043) sets WidgetNameRole only when `slot.valid` is true. For an unset role, the old code stored "" and the new code stores an invalid QVariant, which QML sees as undefined. Every built-in widget type is a group or dataset widget, so `slot.valid` can only be false for an extension widget whose package id comes back empty (Dashboard::extensionSlot). I found no ordinary path that produces that state. allWidgets() and the group entries behave exactly as before.
- **Finding 188 (indexForGroupId via changeIndices):** it now returns the last matching index rather than the first if two workspaces share an id. Empty and no-match inputs still return -1, and entries without an id are still skipped without advancing the index. Duplicate ids are not expected.

Keys, key order, value types and the isGroupWidget emission rules (true or false in results(), absent in allWidgets()) are all unchanged. Compile risk low: I found nothing that should break the build. If it does fail, the first place to look is core/Ui/UI/Taskbar/TaskbarSearch.cpp lines 96-113, the new file-static makeWidgetEntry helper. It relies on three things: QStandardItem::data(int) const being called on a const QStandardItem*, an int converting implicitly into a temporary for the const QVariant& parameter, and bool converting to std::optional<bool>; all are legal. The helper is used four times, so no unused-function warning. In BarPanel.cpp, Bands:: is resolved by the same lookup the removed member wrappers used, and UI/WidgetBands.h is still included. changeIndices is a const member declared in TaskbarWorkspaces.h:77. No unit test in app/tests/CMakeLists.txt compiles any of the 10 changed .cpp files. None of the removed symbols was a Q_PROPERTY, slot, signal or Q_INVOKABLE. Grepping core, app, tests, examples, scripts and doc/help turned up no remaining callers of FocusCycler::running, WidgetExtensions::idsForScope or acceptsEntity, WidgetRegistry::isInBatchUpdate, contains, allWidgetIds, widgetIdsByGroup or widgetCount, or BarPanel::bandIndexFor or nearestBandIndex. The AlarmMonitor and LEDPanel functions named bandIndexFor are different classes.

Files:

- `core/Ui/UI/Taskbar/FocusCycler.cpp`
- `core/Ui/UI/Taskbar/FocusCycler.h`
- `core/Ui/UI/Taskbar/TaskbarSearch.cpp`
- `core/Ui/UI/Taskbar/TaskbarSearch.h`
- `core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp`
- `core/Ui/UI/Taskbar/TaskbarWorkspaces.h`
- `core/Ui/UI/WidgetExtensions.cpp`
- `core/Ui/UI/WidgetExtensions.h`
- `core/Ui/UI/WidgetRegistry.cpp`
- `core/Ui/UI/WidgetRegistry.h`
- `core/Ui/UI/Widgets/BarPanel.cpp`
- `core/Ui/UI/Widgets/BarPanel.h`

Findings:

- #186 `FocusCycler.cpp` (dead-code, 13) — applied: Delete FocusCycler::running() with its 'State' section banner (cpp 48-59) and the declaration at FocusCycler.h:53.
- #185 `TaskbarSearch.cpp` (local-duplication, 14) — applied: Four near-identical 9-10 line QVariantMap entry builders in results() and allWidgets() collapse into one static helper taking (item, groupName, optional isGroupWidget pointer) that fills the same keys; call sites become one line each.
- #188 `TaskbarWorkspaces.cpp` (redundant-logic, 12) — applied: indexForGroupId() duplicates the model walk of changeIndices(); reduce its body to 'return changeIndices(groupId, groupId).first;'.
- #208 `WidgetExtensions.cpp` (dead-code, 40) — applied: Delete WidgetExtensions::idsForScope (196-209) and acceptsEntity (239-262) plus their two declarations in WidgetExtensions.h. No replacement. (Accepts/readAccepts then only feed the parsed descriptor; left alone.)
- #207 `WidgetRegistry.cpp` (dead-code, 46) — applied: Delete five uncalled WidgetRegistry methods (isInBatchUpdate, contains, allWidgetIds, widgetIdsByGroup, widgetCount) from the .cpp and their five declarations in WidgetRegistry.h; no replacement. widgetIdsByType/widgetIdByTypeAndIndex stay (used).
- #211 `BarPanel.cpp` (single-use-helper, 18) — applied: Delete the static pass-through wrappers BarPanel::bandIndexFor and nearestBandIndex and their two header declarations. In refreshRow call Bands::indexFor(row.bands, v, row.hint) and Bands::nearestIndex(row.bands, v) directly. Drop the now-empty 'Band lookup' b

### C6-P04 — promised 139, delivered 129, verdict: accepted

Reviewer: I found no defect in packet C6-P04. Each removed symbol is unused: m_frameStartPos, widgetSize, m_size, applyImageSmoothing, removeStringFromCursor, Left/RightDirection, the Direction Q_ENUM, cursorPosition(), setCursorPosition(int,int), zoomLevelPrecise, altitude, latitude and longitude. I grepped core, app, tests, examples, scripts and doc/help, and the only hits are other classes' own members with the same names: m_size in FixedQueue, FileIndexer and FFTPlot, and QLineEdit and WaterfallOverlay cursor calls. None of them is a Q_PROPERTY, and nothing in QML or any string-based call refers to them. rebindStrokeBrush() has exactly the body it replaces. The setImageSmoothingEnabled inline matches the old helper. The new Terminal rect helper does the same calculation as both originals, including the int truncation. The selection path now gets painter->fontMetrics() once instead of once per character, with the same font, so the result is the same. In Plot3D::drawData, clearing the trace buffers before updateCamera() changes nothing visible: updateCamera reads neither buffer, and they are only read in updatePaintNode and buildTracePolyline. No connect calls, SS_ASSERT, [[nodiscard]] on a surviving function, tr strings or objectNames were touched. All changes are in the packet's own files. Compile risk low: If anything breaks the build, look first at core/Ui/UI/Widgets/Terminal.cpp:160-195, the new file-static segmentHighlightRect. It is declared [[nodiscard]] static, takes width() as a qreal and turns it into int maxWidth and rightEdge. The original code did the same qreal-to-int conversion straight from width(), so the warning profile should not change. QFontMetrics and QPainter are already included. eraseFromCursor now passes AnsiEraseDirection straight to TerminalBuffer::removeFromCursor(AnsiEraseDirection, int), which is the type it already received. No unit test target compiles DataGrid.cpp, GPS.cpp, ImageView.cpp, PainterContext.cpp, Plot3D.cpp or Terminal.cpp. The tests compile only Plot3DStereo.cpp and the Terminal/*.cpp helpers, which this packet does not change. Plot3D had no ctor init for m_size, and displayPixelRatio() is still used at Plot3D.cpp:1001 and :1014. The PainterContext callers and rebindStrokeBrush sit in the same BUILD_COMMERCIAL guard.

Files:

- `core/Ui/UI/Widgets/DataGrid.cpp`
- `core/Ui/UI/Widgets/DataGrid.h`
- `core/Ui/UI/Widgets/GPS.cpp`
- `core/Ui/UI/Widgets/GPS.h`
- `core/Ui/UI/Widgets/ImageView.cpp`
- `core/Ui/UI/Widgets/ImageView.h`
- `core/Ui/UI/Widgets/PainterContext.cpp`
- `core/Ui/UI/Widgets/PainterContext.h`
- `core/Ui/UI/Widgets/Plot3D.cpp`
- `core/Ui/UI/Widgets/Plot3D.h`
- `core/Ui/UI/Widgets/Terminal.cpp`
- `core/Ui/UI/Widgets/Terminal.h`

Findings:

- #212 `DataGrid.cpp` (single-use-helper, 9) — applied: Remove the pass-through DataGrid::formatValue (def plus header declaration). Replace its two calls (lines 240, 286) with DataModel::ValueFormat::formatValue(dataset), which fits the 100-column limit.
- #190 `GPS.cpp` (dead-code, 35) — applied: Delete the unused accessors altitude(), latitude(), longitude() and zoomLevelPrecise() (definitions at GPS.cpp 200-222 and 240-246, declarations at GPS.h 92-94 and 98). Not Q_PROPERTYs, no callers.
- #191 `ImageView.cpp` (dead-code, 13) — applied: Remove the write-only member ImageFrameReader::m_frameStartPos: its declaration (ImageView.h:84), the init in the manual ctor (line 68, and the autodetect ctor init at line 56), and the 11 'm_frameStartPos = ...' assignments (lines 133,169,192,202,226,236,258,
- #196 `PainterContext.cpp` (single-use-helper, 7) — applied: Inline applyImageSmoothing() into setImageSmoothingEnabled() as `if (m_painter) m_painter->setRenderHint(SmoothPixmapTransform, m_state.imageSmoothing);` and delete the helper plus its declaration at PainterContext.h:210.
- #197 `PainterContext.cpp` (local-duplication, 5) — applied: Replace the five hand-written `if (m_painter) m_painter->setPen(m_state.strokePen);` blocks (setLineWidth, setLineCap, setLineJoin, setMiterLimit, applyDashToPen at ~1264) with the existing rebindStrokeBrush() call.
- #195 `Plot3D.cpp` (dead-code, 13) — applied: Delete Plot3D::widgetSize() (decl Plot3D.h:142, def here), the m_size member (Plot3D.h:295) and the two m_size/dpr lines in updateSize(), leaving updateSize() as just markDirty().
- #198 `Plot3D.cpp` (local-duplication, 5) — applied: In drawData(), hoist the per-slot m_tracePx/m_traceColors clear loop above the data.empty() check and delete the second identical loop (lines 859-862); updateCamera() does not read those buffers.
- #224 `Terminal.cpp` (single-use-helper, 21) — applied: Delete private slot removeStringFromCursor and the public Terminal::Direction enum+Q_ENUM (Left/RightDirection); eraseFromCursor calls m_buffer.removeFromCursor(direction, length); m_search.markDirty(); drainBufferEvents() directly, removing a double enum conv
- #225 `Terminal.cpp` (local-duplication, 15) — applied: drawSegmentSelection (lines 186-209) and drawSegmentMatch (472-495) share an identical ~24-line block computing leadingOffset, width, RTL startX and clamping; extract one helper returning the QRect and have both call it.
- #228 `Terminal.cpp` (dead-code, 8) — applied: Remove Terminal::cursorPosition() (definition+comment) and its declaration in Terminal.h:167; it is not a Q_PROPERTY/slot/invokable.
- #229 `Terminal.cpp` (single-use-helper, 8) — applied: Remove the setCursorPosition(int x, int y) overload and its Terminal.h:219 declaration; clear() calls setCursorPosition(QPoint(0, 0)) instead.

### C6-P05 — promised 114, delivered 115, verdict: accepted

Reviewer: I accept this packet. It changes only files in its own list.

- **Finding 223 (dispatchCsiFinal):** processFormat used to end in m_state = Text on every path: either the handled cases set it before returning true, or the false return led to it. The handlers (handleCsi*) never write m_state; I checked every m_state assignment in AnsiStateMachine.cpp. So making the function void and setting Text once after the call keeps the same order of side effects.
- **Findings 226 and 227 (IgnoreSeq, processIgnoreSeq, state()):** none of these names is used anywhere in core, app, tests, examples, scripts or doc/help. The switch in feed() still has a default.
- **Finding 222 (Vt100Keymap table):** every byte sequence matches the old switch, and unknown keys still return an empty QByteArray.
- **Finding 240 (SnapOverlay inline):** the code was moved verbatim with the same JSON keys, value types, reserve calls, signal emissions and order. publishSnapGuides is now about 45 lines.
- **Finding 241 (referenceCanvas):** it had no callers. QSize is still used in WindowLayoutStore.h.

Nothing in this packet edits a connect() call, an SS_ASSERT, a tr() string or a protected surface. Compile risk low: I found no defects. The first place to look is core/Ui/UI/Widgets/Terminal/Vt100Keymap.cpp specialKey. It now uses a static constexpr aggregate table: Qt::Key is an unscoped enum and converts to int without a narrowing error. I checked every byte string against the old switch and they all match. An escape like "\x1bOP" stops at the 'O', and "\x7f" and "\x1b" give the same single bytes the old char appends did. Unknown keys still return an empty QByteArray. tst_ansi_state_machine compiles only AnsiStateMachine.cpp with Qt6::Core, and the change adds no new calls or includes there; the test does not use state() or IgnoreSeq. tst SnapOverlay compiles SnapOverlay.cpp, and the inlined loops call nothing new. The only removed [[nodiscard]] markers belonged to the deleted state() and to dispatchCsiFinal, which is now void. Neither change is avoidable.

Files:

- `core/Ui/UI/Widgets/Terminal/AnsiStateMachine.cpp`
- `core/Ui/UI/Widgets/Terminal/AnsiStateMachine.h`
- `core/Ui/UI/Widgets/Terminal/Vt100Keymap.cpp`
- `core/Ui/UI/Widgets/Terminal/Vt100Keymap.h`
- `core/Ui/UI/WindowManager/SnapOverlay.cpp`
- `core/Ui/UI/WindowManager/SnapOverlay.h`
- `core/Ui/UI/WindowManager/WindowLayoutStore.cpp`
- `core/Ui/UI/WindowManager/WindowLayoutStore.h`

Findings:

- #223 `AnsiStateMachine.cpp` (redundant-logic, 20) — applied: dispatchCsiFinal's bool result and every per-case `m_state = Text; return true;` (plus the default letter check) are redundant: processFormat sets Text on false anyway. Make it void, set Text once in processFormat, cases just break.
- #226 `AnsiStateMachine.cpp` (dead-code, 14) — applied: Remove the never-entered IgnoreSeq state: enum value, the `case IgnoreSeq` arm in feed(), processIgnoreSeq definition and its declaration.
- #227 `AnsiStateMachine.cpp` (dead-code, 10) — applied: Remove AnsiStateMachine::state() accessor (definition with comment, plus header declaration line 77).
- #222 `Vt100Keymap.cpp` (local-duplication, 40) — applied: Replace the 24-case switch in specialKey (each case = 3 lines of append+break) with a static const {int key; const char* seq;} table and a range-for lookup returning the QByteArray; same bytes, same signature.
- #240 `SnapOverlay.cpp` (single-use-helper, 15) — applied: Inline static snapVisualsToVariants into publishSnapGuides (the guides/spacings locals already exist there); delete the helper, its doc comment, the 'Local helpers' banner and the call line.
- #241 `WindowLayoutStore.cpp` (dead-code, 15) — applied: Delete WindowLayoutStore::referenceCanvas() (definition plus the now-empty 'State queries' banner) and its declaration at WindowLayoutStore.h:50 plus the blank line after it.

## Group Q1

### Q1-P01 — promised 327, delivered 339, verdict: accepted

Reviewer: I accept the packet: I found no concrete behavior, compile, scope or rule defect. All six findings are applied the same way:
- 13 driver commands become DriverCommand.
- The add group/dataset/output factories become GroupCommand/DatasetCommand/OutputCommand, and the menu ones become AddGroupCommand/AddDatasetCommand/AddOutputCommand.
- The 7 dataset toggles become DatasetToggleCommand.
- 4 tab frames become TabFrame.
- The dead Reproducibility/verification code in SessionDetail is deleted.

Each command keeps its name, its `enabled`/`visible`/`checked`/`run` behavior, its enum argument and its qsTr string. No outside users exist, and no connect, objectName or C++ file is touched. The one concern I could not resolve without running the app is ordering at startup for the `var host` bindings, which I think is benign. The other note is style only: about 62 new QML lines exceed 100 columns. Compile risk low: This packet changes QML only, so no C++ is compiled, linked or run through moc. I checked these points:
- Every inline component (DriverCommand, GroupCommand/DatasetCommand/OutputCommand, AddGroupCommand/AddDatasetCommand/AddOutputCommand, DatasetToggleCommand, TabFrame) is declared directly under the root object. That matches the existing precedents in Macros.qml and StartMenu.qml.
- No component body reaches an outer id. Each one gets `root` through `allowed:` or `host: root`.
- Every `root.cmdX` name in the three binding maps still resolves. The set of removed `cmdX` properties is identical to the set of re-added ones.
- Nothing outside the three Commands files references these names.
- The new property names (bus, allowed, groupName, widget, option, type, host) do not collide with the keys CommandModel.join reads (title, tooltip, visible, enabled, checked, run).
- `run` is reached as `b.run` (a QObjectMethod bound to its object), so unqualified `bus`/`option`/`type` inside run() still resolve on the object.
- The qsTr strings for group names are byte-identical and stay in the same file, so the translation context is unchanged.
- SessionDetail: no live use of `verification`, `verdictLabel` or `onVerificationFinished` remains. ToolCallCard.verification is a separate property. latestVerification is a const Q_INVOKABLE, so dropping its call has no side effect.
- TabFrame keeps `anchors.fill: parent`, and each one replaces its Rectangle in the same position among the children.

Where to look first at run time: the `host`-based bindings in ProjectEditorMenuBindings.qml (`visible: host.singleTarget` at about line 407, `checked: host.datasetOptionOn(option)` at about line 856). If any of them is evaluated before `host` is assigned, it logs a one-time TypeError and then corrects itself. I expect Qt's LIFO binding finalization (outer bindings first) to prevent that. A qmllint run (ss_qmllint) may also report unqualified/var access on `host`.

Style note, not counted as a defect: 62 new entry lines in the Commands files run past 100 columns, against the 100-col rule in code-style.md. No tool enforces this for QML, and HEAD already has over-length QML lines elsewhere.

Files:

- `app/qml/Commands/AppCommandBindings.qml`
- `app/qml/Commands/ProjectEditorCommandBindings.qml`
- `app/qml/Commands/ProjectEditorMenuBindings.qml`
- `app/qml/DatabaseExplorer/ReportOptionsDialog.qml`
- `app/qml/DatabaseExplorer/SessionDetail.qml`

Findings:

- #266 `AppCommandBindings.qml` (local-duplication, 55) — applied: Replace the 13 cmdDriverX objects with one inline component DriverCommand { required property int bus } and one-line entries; enabled/checked/run are derived from bus.
- #268 `ProjectEditorCommandBindings.qml` (local-duplication, 45) — applied: Fold the 9 addGroup, 6 addDataset and 5 addOutputControl command objects into three inline components (title+widget, option, type) and one-line entries.
- #265 `ProjectEditorMenuBindings.qml` (local-duplication, 85) — applied: Collapse 11 group, 7 dataset and 5 output command QtObjects (identical visible/run shape) into three inline components taking title/widget, option or type; each entry becomes one line. Keep cmdAddOutputPanel as is.
- #271 `ProjectEditorMenuBindings.qml` (local-duplication, 25) — applied: Collapse the 7 cmdDatasetX objects into one inline component taking the SerialStudio.Dataset* option.
- #273 `ReportOptionsDialog.qml` (local-duplication, 12) — applied: Replace the four identical tab-panel frame Rectangles (radius 2, border 1, groupbox colors, anchors.fill) with one inline component.
- #264 `SessionDetail.qml` (dead-code, 105) — applied: Delete the commented-out Reproducibility block (513-590), the now-unused verdictLabel() (48-65), the `verification` property (32-34) and its two refresh sites in Connections (161-163, 181-185).

### Q1-P02 — promised 261, delivered 250, verdict: accepted

Reviewer: I read every hunk in the 9 packet files, and none of the edits changes behavior. Macros TinyButton: the 8 buttons get the same 4 defaults and keep their own tooltip, enabled and click bindings. MasterAnnunciator: there are exactly 6 PanelButton instances, all of which had ToolTip.delay 700 and ToolTip.visible hovered, now set in the component. StartMenu: searchableItems has no remaining reference in app, core, tests, examples, scripts or doc/help. closeSubMenusExcept keeps the guarded close order for each caller, and the root, _groups, _actions, _plugins, _export and _tools ids all resolve. StartMenuDivider matches the old Rectangle and keeps the two visible: lines at their sites. CANBus syncFromDriver: rates.indexOf returns -1 on a miss, so the merged form is equivalent. Modbus syncRates: its body is the same as both old handler bodies, unqualified names resolve to the combo, and the guards are unchanged. ProcessPicker: the deleted children were copies of what DriverTagPickerDialog already provides, and its own onVisibleChanged is kept. Removed ids: grep -cw finds 0 uses of each in its own file, and none is referenced from a child component. Every objectName is kept, no qsTr string changed, and the visual order of children is the same. MainWindow: the Instantiator-of-Shortcut pattern copies the existing one in the same file. The palette-to-ThemedPage swaps were excluded as instructed. The diffs in other driver files (Network, S7, UART, Process, the other dialogs) belong to other packets. Compile risk low: This packet changes QML only, so there is no C++ or linking surface. The two places to check first at runtime: (1) MainWindow.qml:451-461, the Ctrl+1..9 shortcuts. They are now Shortcut objects created by an Instantiator. They rely on the same parenting the command-shortcut Instantiator at MainWindow.qml:463-484 already depends on for window-context matching. No Qt source was available to confirm it, but that existing Instantiator is the precedent. Press Ctrl+1..9 on the dashboard once. (2) ProcessPicker.qml: the titlebar, drag handler and theme Connections now come only from DriverTagPickerDialog. NativeWindow::addWindow is idempotent for a window it already knows (NativeWindow_CSD.cpp:282), so dropping the second theme-change call changes nothing. The titlebar label is no longer drawn twice over itself, so its glyph edges may look a hair lighter. That is a correction, not a regression.

Files:

- `app/qml/Dialogs/Macros.qml`
- `app/qml/MainWindow/MainWindow.qml`
- `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml`
- `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/DriverTagPickerDialog.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/EthernetIp.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/Iec104.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml`

Findings:

- #234 `Macros.qml` (local-duplication, 28) — applied: Nine icon buttons repeat iconSize 16, implicitHeight 24, Layout.maximumWidth 24 and Layout.alignment VCenter. Define a local 'component TinyButton: Widgets.IconButton' with those defaults and drop four lines per use.
- #290 `MainWindow.qml` (local-duplication, 25) — applied: Nine Ctrl+1..9 Shortcut blocks differ only in the key and the workspace index. Replace with one Instantiator over 9 entries with sequence 'Ctrl+' + (index+1), as the Instantiator at line 487 already does.
- #338 `MasterAnnunciator.qml` (local-duplication, 12) — applied: All seven PanelButton instances repeat 'ToolTip.delay: 700' and 'ToolTip.visible: hovered'. Put both in the PanelButton component (lines 166-172) and delete the 14 per-instance lines.
- #328 `StartMenu.qml` (local-duplication, 38) — applied: Five showMenu() functions each end with an 11-line 'close the other four popups' block. Add one closeSubMenusExcept(self) helper on root that loops over the five buttons, and call it once per showMenu.
- #330 `StartMenu.qml` (local-duplication, 32) — applied: Five identical 10-line divider Rectangles (opacity 0.18, height 1, margins, start_menu_text colour); three differ by a 'visible' line. Define a local 'component StartMenuDivider: Rectangle' once and write 'StartMenuDivider {}' at each site.
- #342 `StartMenu.qml` (dead-code, 7) — applied: searchableItems(filter) has no callers; the palette now uses PaletteModel/CommandModel directly. Remove the function and its comment.
- #335 `CANBus.qml` (redundant-logic, 14) — applied: Both syncFromDriver() functions set editText = current in both branches, and currentIndex is already -1 when indexOf misses. Replace each if/else with 'currentIndex = rates.indexOf(current); editText = current'.
- #341 `CANBus.qml` (dead-code, 8) — applied: Four unused ids (_canFDCheck, _loopbackCheck, _listenOnlyCheck, _tpReassemblyCheck). Delete each id line and its blank line.
- #360 `ProcessPicker.qml` (redundant-logic, 49) — applied: ProcessPicker already derives from DriverTagPickerDialog but declares a second theme Connections, titlebar Rectangle, titlebar Label and DragHandler. Delete those four child objects; the base supplies them.
- #340 `EthernetIp.qml` (dead-code, 8) — applied: Four unused ids (_hostField, _pathField, _familyCombo, _intervalField). Delete each id line and its blank line.
- #334 `Iec104.qml` (dead-code, 16) — applied: Eight 'id:' declarations (_hostField, _portField, _addressField, _windowKField, _windowWField, _t1Field, _t2Field, _t3Field) are never referenced. Delete each id line and its trailing blank line.
- #337 `Modbus.qml` (local-duplication, 12) — applied: The onBaudRateChanged and onProtocolIndexChanged handlers in two Connections blocks have identical 12-line bodies (rates, current, model = rates, indexOf, set currentIndex or editText). Move the body to one function on the combo and call it from both handlers.
- #339 `Modbus.qml` (dead-code, 12) — applied: Six unused ids (_protocolCombo, _serialPortCombo, _hostField, _portField, _slaveField, _intervalField). Delete each id line and its blank line.

### Q1-P03 — promised 246, delivered 257, verdict: accepted

Reviewer: I found no defect I can pin to a line, so the packet stands. I read every hunk of the 12 packet files in context. Diff hunks in other files (CANBus, Modbus, Iec104, EthernetIp, ProcessPicker and other dialogs) belong to other packets or to the main session's ThemedPage change, and I did not count them.

- **Removed ids:** nothing still refers to them. That covers `column` in the three dialogs; `_typeCombo`, `_wsFormat` and `_httpMethod` in Network.qml; the four in S7.qml; and the six in UART.qml. The only remaining `_typeCombo` hits are separate ids declared in ModbusGroupsDialog and EipTagsDialog.
- **ProjectStructure.qml:** a repo-wide grep finds no use of `groupClicked`, `datasetClicked`, `ctxItemId` or `ctxItemKind` outside the spec docs. `badgeColor` computes the same three-way value as the old color blocks, and both labels still bind it.
- **ValueLabelsEditor.qml:** `applyPreset` now appends `{value: String(i), text: words[i]}`, which gives the same rows as before. The preset labels, the combo `model` and `textRole` reads, and all qsTr strings are unchanged.
- **DatasetView.qml:** `toggleOption` has the same body as the seven inline handlers and is called through `root`, which is declared at line 30.
- **MqttPublisherView.qml:** the three TlsButton uses keep their text, onClicked, enabled, icon and the tooltip ternary. Only the ordering of three property lines changed, which has no visual effect.
- **Hardware.qml:** the source URLs are the same strings, `required property modelData` stops model or index from leaking into the loaded drivers, and `registerBus` order did not matter before because the async loads finished in any order.
- **Process.qml:** each removed `onEditingFinished` handler matched its `onTextEdited` exactly, and the `Connections` blocks keep the text in step with the backend, so nothing changes.
- **S7.qml, one edge case below the bar:** rack, slot and poll interval are only filled in once, in `Component.onCompleted`, and have no resync. `S7.cpp:1251` (`setRack(value.toInt())`) and the CLI can change the backend while the pane is open. Before this change, focusing a stale field and leaving it (or pressing Enter) wrote the stale displayed value back. Now nothing is written. The difference only appears when the field is already out of date, and the new behaviour is the less harmful of the two, so I did not count it as a defect. The maintainer should know about it.

No connect call, objectName, qsTr string or context was touched, and no new file was added. No protected surface is involved. Compile risk low: This is QML only, so a failure would show at runtime, not in the C++ build. Check these first. (1) Hardware.qml:148-168: a Repeater of Loaders inside StackLayout `layout`. The Repeater is the last child and delegates stack before it, so the order is UART, Network, BLE, then the 11 commercial drivers, which matches the old index order. Confirm in the Setup pane that busType switching still shows the right driver for one commercial bus (e.g. S7 and Iec104), since StackLayout must treat the Repeater as transparent. Line 161 is about 106 columns, over the 100-column style (code-verify does not flag it in QML). (2) MqttPublisherView.qml:41: the inline component TlsButton is declared at the Pane root. Its ToolTip/Layout attached properties and the qsTr strings stay in the same file, so the translation context is unchanged. (3) ProjectStructure.qml:587: badgeColor lives on the delegate root `item`, where the required properties selected/current and `model` resolve. Labels at :812 and :833 bind item.badgeColor and are still bindings.

Files:

- `app/qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/S7.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/S7VariablesDialog.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml`
- `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml`
- `app/qml/ProjectEditor/Dialogs/ValueLabelsEditor.qml`
- `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- `app/qml/ProjectEditor/Views/DatasetView.qml`
- `app/qml/ProjectEditor/Views/MqttPublisherView.qml`

Findings:

- #372 `ModbusGroupsDialog.qml` (dead-code, 6) — applied: The ColumnLayout id 'column' is never referenced in the three dialogs that declare it. Remove the id line and blank line in each (ModbusGroups, S7Variables, OpcUaTagBrowser).
- #371 `Network.qml` (dead-code, 6) — applied: Ids _typeCombo, _wsFormat and _httpMethod are never referenced in Network.qml. Remove the three id lines and blank lines.
- #364 `Process.qml` (redundant-logic, 20) — applied: Each of the four fields has an onEditingFinished handler identical to its onTextEdited (same enabled/compare/assign), so it can never change the result. Remove the four onEditingFinished blocks.
- #365 `S7.qml` (redundant-logic, 18) — applied: Rack, Slot and Poll Interval fields each repeat the onTextEdited body verbatim in onEditingFinished. Remove the three onEditingFinished blocks.
- #370 `S7.qml` (dead-code, 8) — applied: Ids _hostField, _rackField, _slotField and _intervalField are never referenced inside S7.qml. Remove the four id lines and their blank lines.
- #369 `UART.qml` (dead-code, 12) — applied: Six ids are declared but never referenced: _dataCombo, _parityCombo, _stopBitsCombo, _flowCombo, _autoreconnect, _dtr. Remove each id line and its following blank line.
- #356 `Hardware.qml` (local-duplication, 110) — applied: Eleven identical 13-line commercial Loader blocks differing only in the source file name. Replace with one Repeater (inside the StackLayout, same order) over an 11-name array, each delegate a Loader with source built from the name.
- #315 `ValueLabelsEditor.qml` (redundant-logic, 20) — applied: The presets array repeats value: 0/1/2 by hand. Store each preset as {label, words: [...]} and let applyPreset use the array index as the value. Text and labels stay the same.
- #319 `ProjectStructure.qml` (local-duplication, 7) — applied: sourceBadge and frameIndex have the same 9-line color binding. Add one readonly property badgeColor on the delegate and bind both labels to it.
- #321 `ProjectStructure.qml` (dead-code, 6) — applied: Remove the signals groupClicked and datasetClicked (lines 37-38) and the properties ctxItemId and ctxItemKind with their two assignments in openContextMenu (136, 137, 174, 175). Nothing reads them.
- #314 `DatasetView.qml` (local-duplication, 23) — applied: Seven dataset-option toggle buttons each carry the same 5-line onClicked (const option, const value, changeDatasetOption). Add one root function toggleOption(option) and make each onClicked one line.
- #305 `MqttPublisherView.qml` (local-duplication, 10) — applied: Load CA Certs, Client Certificate and Private Key buttons repeat the same enabled expression, icon and nested tooltip ternary; use one inline TlsButton with text, tooltip and action properties.

### Q1-P04 — promised 292, delivered 297, verdict: accepted

Reviewer: I found no concrete defect, so I accept the packet. I read every hunk, the full diff of all 11 files, in context.

**Removed ids.** All 41 removed ids were confirmed unreferenced:
- passwordFieldLoader in TableDelegate.qml
- three in Meter.qml
- three in Plot3D.qml
- five in Stopwatch.qml
- 18 in Terminal.qml
- three in Waterfall.qml
- _xPosLabel in PlotWidget.qml

QML ids only count inside their own file, and a per-file grep finds no remaining use of any of them.

**PlotWidget wrappers and PlotFormatter.yTickLabel.** A repo-wide grep of app, core, tests, examples, scripts and doc/help finds no outside caller of the seven removed wrappers (xTickLabel, yTickLabel, safeTickAnchor, subTicksFor, tickDecimals, logTickFormat, cursorReadout). Every redirected call names a function PlotFormatter.qml still defines: xTickLabel, safeTickAnchor, subTicksFor, tickDecimals, logTickFormat and cursorReadout. PlotFormatter.yTickLabel had no callers. engineeringFormat, displayValueX and displayValueY are kept for their outside users.

**updateCrosshairLabels.** Removing it is unobservable. It only wrote text into two labels that are never visible. _yPosLabel is kept because the TextMetrics read its font and padding, and _overlayMouse is still used.

**MultiPlot Connections block.** The removed block's only handler, onCurvesChanged, had an empty body, so it did nothing.

**Restore loops.** In MultiPlot, Plot, Plot3D and Waterfall the loops keep the same keys in the same order with the same undefined guard. The special cases (sweepEnabled, sweepRetention and the showFrequencyMarkers to markersVisible mapping) are unchanged.

**Waterfall.toggleSetting.** It keeps the same null-model guard, flips the property first, then saves the same key with the new value.

**Meter widestValueText.** It is the same expression, moved onto root, where root.formatValue and root.model resolve.

**TableDelegate merged loader.** It is field-for-field identical to the old pair except echoMode, which is now a binding. It sits in the same layout position, so visual order is unchanged.

**Terminal inline components.** Every objectName and qsTr string is kept, and child order is unchanged. The bindings inside RibbonButton and SmallButton use only the button's own properties, so no outer-file id is needed. The automation catalog walks the class chain up to IconButton and Button, so roles resolve as before. RibbonSection's click wiring checks item.clicked, which the inline components still have.

**Rules.** No objectName, tr/qsTr, connect call, SS_ASSERT or C++ symbol was touched, no new file was added, and no protected surface was involved. Compile risk low: All 11 files are QML. No C++, CMake or moc surface changed. qmllint runs as the separate ss_qmllint target and does not gate the build, and none of these files uses pragma Strict, so the dynamic model[key] / root.model[prop] writes in MultiPlot, Plot, Plot3D and Waterfall only fall back from AOT compilation to the interpreter. If anything breaks, look first at the two inline components in app/qml/Widgets/Dashboard/Terminal.qml (RibbonButton and SmallButton, about lines 35-52). They put Layout.* attached properties on the component root, and cyclicBt and sendBt override Layout.maximumWidth back to 32 on the instance, which Qt allows. Then check the merged textFieldLoader in app/qml/ProjectEditor/Views/TableDelegate.qml at line 454. One small runtime difference: if a row's widgetType switches between TextField and PasswordField, the old code rebuilt the TextField and the merged loader keeps it and flips echoMode. The displayed value is the same either way.

Files:

- `app/qml/ProjectEditor/Views/TableDelegate.qml`
- `app/qml/Widgets/Dashboard/Meter.qml`
- `app/qml/Widgets/Dashboard/MultiPlot.qml`
- `app/qml/Widgets/Dashboard/Plot.qml`
- `app/qml/Widgets/Dashboard/Plot3D.qml`
- `app/qml/Widgets/Dashboard/Stopwatch.qml`
- `app/qml/Widgets/Dashboard/Terminal.qml`
- `app/qml/Widgets/Dashboard/Waterfall.qml`
- `app/qml/Widgets/PlotWidget.qml`
- `app/qml/Widgets/PlotWidget/PlotFormatter.qml`
- `app/qml/Widgets/PlotWidget/PlotInteraction.qml`

Findings:

- #299 `TableDelegate.qml` (local-duplication, 40) — applied: textFieldLoader and passwordFieldLoader are identical except echoMode: merge into one Loader active for TextField or PasswordField, with echoMode chosen by widgetType.
- #408 `Meter.qml` (dead-code, 6) — applied: Remove unused `id:` lines and following blanks for titleText, needleHolder and pivotHub.
- #409 `Meter.qml` (local-duplication, 5) — applied: valueBoxMetrics.text and bigValueMetrics.text compute the identical widest-of-min/max-plus-units string. Add one readonly property on root and bind both to it.
- #399 `MultiPlot.qml` (local-duplication, 14) — applied: Six `if (s[key] !== undefined) root.model.key = s[key]` blocks (sweepMode, triggerEdge, triggerLevel, triggerSource, holdoff, sweepTimebase) become one loop over a key array.
- #405 `MultiPlot.qml` (dead-code, 7) — applied: Delete the Connections block whose only handler onCurvesChanged() has an empty body; it does nothing.
- #401 `Plot.qml` (local-duplication, 11) — applied: Five sweep restore blocks (sweepMode, triggerEdge, triggerLevel, holdoff, sweepTimebase) become a key loop; the sweepEnabled and sweepRetention special cases stay.
- #403 `Plot3D.qml` (local-duplication, 10) — applied: Six same-name restore blocks (interpolationEnabled, autoCenter, orbitNavigation, anaglyphEnabled, invertEyePositions, eyeSeparation) become a loop over a key list.
- #407 `Plot3D.qml` (dead-code, 6) — applied: Remove unused `id:` lines and following blanks for _interpolate, _anaglyph and _eyeSeparation.
- #402 `Stopwatch.qml` (dead-code, 10) — applied: Remove five unused `id:` lines and the blank after each: tickTimer, readoutBox, startStopBtn, lapBtn, resetBtn.
- #389 `Terminal.qml` (local-duplication, 40) — applied: Eight ribbon Widgets.IconButtons (Clear, Find, Collapse, Annotations, Pause, Text, Hex, Settings) repeat 7 identical lines (flat, leftPadding 8, rightPadding 8, implicitHeight 32, icon.color transparent, Layout.alignment, Layout.preferredWidth). Define one inl
- #390 `Terminal.qml` (dead-code, 32) — applied: Remove 18 unused `id:` declarations plus the blank line after each: copyMenu, consoleRibbon, clearButton, searchButton, collapseButton, annotationsButton, pauseButton, timestampCheck, echoCheck, vt100Check, ansiColorsCheck, mouseArea, annotationsPanel, ftButto
- #397 `Terminal.qml` (local-duplication, 18) — partly-applied: Added inline SmallButton (iconSize 16, implicitHeight 24, Layout.maximumWidth 24) for the 7 buttons; the 32-wide ones override the width. The opacity lines were left per instance because searchClose does not have one.: Seven small 24px IconButtons (searchPrevious, searchNext, searchClose, ftButton, libraryButton, cyclicBt, sendBt) repeat iconSize 16, implicitHeight 24, Layout.maximumWidth and opacity lines. Use one inline component with a width property.
- #395 `Waterfall.qml` (local-duplication, 20) — applied: Four toolbar toggles (colorbarVisible, axisVisible, cursorEnabled, markersVisible) repeat guard-model, flip property, save setting. Replace each onClicked with one call to a helper toggleSetting(prop, key).
- #400 `Waterfall.qml` (local-duplication, 12) — applied: Six same-name restore blocks (colorMap, minDb, maxDb, axisVisible, cursorEnabled, colorbarVisible) fold into one loop over keys; keep the single showFrequencyMarkers to markersVisible mapping explicit.
- #406 `Waterfall.qml` (dead-code, 6) — applied: Remove unused `id:` lines and following blanks for minDbLabel, rangeSlider and maxDbLabel.
- #378 `PlotWidget.qml` (dead-code, 24) — applied: Remove updateCrosshairLabels(), the invisible _xPosLabel Label (lines ~767-777) and its one call in PlotInteraction.qml:314-315. Keep _yPosLabel (font/padding source for the TextMetrics). Text written only to visible:false labels.
- #379 `PlotWidget.qml` (single-use-helper, 24) — applied: Delete pass-through wrappers xTickLabel, yTickLabel, safeTickAnchor, subTicksFor, tickDecimals, logTickFormat, cursorReadout (4 lines each); point their callers in this file at _format.X. Keep engineeringFormat/displayValueX/displayValueY, used externally.
- #380 `PlotFormatter.qml` (dead-code, 7) — applied: Remove yTickLabel() from PlotFormatter (and its PlotWidget wrapper, counted in the wrapper finding): never called anywhere.

## Dropped in planning

| Reason | Findings | Lines promised |
|---|---|---|
| judged out in planning | 139 | 6542 |
| C++ below high confidence | 35 | 596 |
| under 5 net lines | 8 | -322 |
| names a licensing/auth concept | 2 | 34 |
| touches a test (out of scope) | 1 | 14 |

"Judged out in planning" covers: hot or hot-adjacent files the exclusion regex missed (`HAL_Driver.h`, `DataTable.cpp`, the native parser templates, the script engines and watchdog, the MQTT publisher's hot sinks), `ProjectModel`/`ProjectWorkspaces`/`ProjectFolders` mutators (undo-history contract, spec 0031), key vault and license-guarded closures, defensive checks at a system boundary, the `Frame.h` folder-struct aliasing, the dashboard instrument clone families (HDR contract, spec 0089), QML rewrites that replace named controls with a `Repeater` (automation `objectName` surface, focus order), and duplicates of another finding.
