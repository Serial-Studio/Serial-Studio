---
spec: 0070-concern-classes
phase: tasks
status: in-progress
created: 2026-08-28
author: Claude (Fable 5)
---

# Tasks — App-Wide Concern-Class Campaign (overnight 2026-08-28)

Status legend: `[ ]` pending · `[~]` agent running · `[R]` under manager review ·
`[x]` done (reviewed + integrated) · `[!]` bounced/needs rework · `[-]` deferred with note.

## Wave 1 — UI leaves + AI
- [x] P01 Terminal decomposition + ANSI/SGR tests — AnsiPalette, AnsiStateMachine (AnsiSink
      interface), TerminalSearch; Terminal.cpp 2849→2364. Reviewed + CMake-registered.
      Integrator TODO: write app/tests/tst_terminal_search.cpp (suite pre-registered, skips
      until the file lands). Risk notes in package report: private AnsiSink multiple
      inheritance is the likeliest compile surprise; two qBound hardenings in indexedColor/
      truecolor are intentional behavior changes.
- [x] P02 Waterfall view-state + widget sweep + tests — WaterfallViewState (Qt-value-type),
      WaterfallOverlay re-formed from multi-TU split into real class, WaterfallTicks namespace;
      Waterfall.cpp 1697→1500 (at cap, zero headroom). Painter.cpp GPL #endif fix + Bar.h
      nodiscard applied. Rich follow-up list in report (PainterContext, PlotSweepController,
      WavWriter, ExtensionData caching). Reviewed + registered + 2 suites.
- [x] P03 WindowManager + Taskbar — WindowGeometry namespace (20 pure fns), SnapOverlay
      (no Dashboard dep — testable), WindowLayoutStore (Dashboard& injected, instance() −2),
      TaskbarWindowMap, FocusCycler (inverted control), TaskbarSearch, TaskbarModel own header;
      WindowManager.cpp 2495→1498 (under cap), Taskbar.cpp 1939→1703 (workspace cluster =
      named next cut). Reviewed + registered + 2 suites.
- [x] P04 ToolDispatcher concern classes + schema/resolver tests — 10 concern TUs under
      AI/Tools/ (AI::ToolDetail), ToolDispatcher.cpp 2561→179, instance() parity 22→22,
      line-multiset clean. Reviewed + CMake-registered (commercial block) + 2 suites.
      Risk: Tools files must stay inside BUILD_COMMERCIAL block (done).
- [x] P05 Conversation history/budget units + AI sweep + tests — HistorySurgery, TokenBudget,
      MetaToolCatalog, ReplyAssembly (free fns), HelpFetcher class; Conversation.cpp 3285→2089
      (still over TU cap; named next cut: MetaToolRunner). Commercial SPDX deviation correct.
      HelpFetcher epoch = only behavior change. Redactor/DocSearch small fixes. MemoryStore
      QSaveFile defect REPORTED NOT FIXED (maintainer call). Reviewed + registered + 3 suites.
- [x] P06 NativeTemplates one-class-per-file split — 26 file pairs (25 protocols + support),
      BinaryTemplates 2474→63, TextTemplates 1827→55, registry order/ids byte-identical,
      string multisets verified. WireLatch left whole (deliberate: shared inheritance chain).
      tst_opcua_wire + tst_cframe_parser link sets updated by integrator. No new tests needed
      (tst_cframe_parser covers all 25 ids).

## Wave 2 — Drivers + comms
- [x] P07 USB + Audio — UsbTransferPump (both threads + drain cv moved as ONE unit; dtor
      only joins, never frees — teardown order sacred), UsbHex (libusb-free mirror pinned by
      static_asserts), AudioDeviceCatalog, AudioPcm (per-sample decode header-inline);
      USB.cpp 1711→1328, Audio.cpp 2125→1462 (under cap). rt path + m_csv* scratch stayed.
      instance() flat, zero in new classes. Reviewed + registered + 1 suite.
- [x] P08 OpcUa — CertificateStore, FrameAssembler (source-owns-time verbatim), Browser +
      OpcUaBrowseHost interface, EndpointSelection free fns, OpcUaTag value type; OpcUa.cpp
      2833→2343 (still over cap; named next cuts: OpcUaProjectBuilder, poll-worker). Morning
      items: ~10 moved strings need .ts regen (untranslated until then); multiple inheritance
      OpcUa : HAL_Driver + OpcUaBrowseHost. Reviewed + registered + 1 suite.
- [x] P09 Driver sweep — ModbusRegisterGroups + ModbusProjectGenerator (Modbus 1774→1459),
      SerialPortIdentity dedup (UART+Modbus), BleUuids header narrowed. THREE DEFECT FIXES:
      generated Lua parser %% bug, EthernetIp peer-blind tagsFrozen (QML greying NOT wired —
      morning item), EipPollWorker readsOk miscount + CIP path injection guard. Follow-ups:
      Network 5-TU re-form, MQTT driver split, CanReassembly 2-class file, S7/EIP poll-worker
      splits. Reviewed + registered + 2 suites.
- [x] P10 MQTT — PublisherWorker re-formed to own pair (PublisherSparkplug.cpp DELETED),
      TlsConfig (passphrase stays vault-side), BrokerOptions (QMap label-order = persisted
      index — do not reorder), CsvExpansion free fns; Publisher.cpp 2333→1529. Hotpath queues
      untouched. Reviewed + registered (worker .h in AUTOMOC list) + 1 suite.
- [x] P11 API core — ServerAuth + AuthPrimitives (constantTimeEquals verbatim), ClientReception
      + ReceptionHost interface, MirrorCommands free fns, ServerWorker re-formed to own .h/.cpp,
      ConnectionState shared header; Server.cpp 1756→993, instance() net −1. Stream lane left
      in facade (deliberate). Mirror/GRPC untouched-clean. New SS_ASSERT_LOG guards = debug
      aborts on missed paths (accepted). Reviewed + registered + 1 suite.
- [x] P12 Providers + Console + InfluxDB sweep — AI::ProviderJson (8-provider dedup),
      AI::ThinkTagSplitter, Console::TextFormat; Handler.cpp 1394→1183; 34 files audited,
      Annotations/Export/InfluxDB clean. Reviewed + CMake-registered + 3 suites.
      Noted behavior deltas (report): ansiColorsEnabled now always evaluated (pure getter),
      passthrough short-circuit folded into splitter drain — accepted.

## Wave 3 — Data + sessions + misc
- [x] P13 Sessions — ReplayClock, SessionDbReader (owns SQLite whole), ReplaySynthesis
      (injection: reader+layout+AppState+FrameBuilder+ConnectionManager; frame-injection
      shape verbatim), ReplayAlignment + ReplayFrameValues free fns, SessionExporter,
      DatabaseSchema re-formed from TU-split statics; Player.cpp 1738→1098, DatabaseManager
      1614→1344, instance() −2. Morning items: ~12 tr() strings moved context (lupdate);
      exercise block-format + multi-source + stream-only sessions + scrub-then-close.
      Follow-ups: Verifier 2-TU re-form (needs spec-0044/0047 design pass), Export pairing.
      Reviewed + registered + 2 suites.
- [x] P14 CSV + MDF4 — RowSyntax (pure), RowCodec (owns timestamp mode — single owner now),
      MultiSourceMap, FileIndexer (owns loader thread; topology verbatim incl. join-timeout
      QFile handoff); Player.cpp 1959→1586 (prompts stay: tr() context). MDF4 all clean.
      Preserved pre-existing hazard: CSV injectRow iterates live map while event loop pumps
      (MDF4 copies — crashed in wild there); follow-up decision. Cross-package dedup noted:
      MDF4 backfill duplicates CSV MultiSourceMap. Reviewed + registered + 3 suites.
- [x] P15 Misc A — ExtensionCatalog (pure fns incl. isPathSafe verbatim, traversal KATs),
      ExtensionInstaller (owns installed.json + download queue; NAM + WorkspaceManager
      injected), ExtensionAutoUpdater; ExtensionManager.cpp 1817→1449. Adjacent bug reported
      NOT fixed: m_pluginMetadataCache not cleared on workspace change (maintainer call).
      Named next cut: plugin-launch half (~350 lines, moves tr() contexts). Reviewed +
      registered + 1 suite.
- [x] P16 Misc B + Licensing + Platform — LanguageTable (4 parallel switches → 1 table,
      Translator.cpp 420→165), ThemeCatalog (ThemeManager.cpp 640→454), Examples.h slot
      hygiene. CLI kept as one dispatch surface (correct). Licensing/Platform all clean —
      nothing moved in security code. Deltas: Fallback theme label, out-of-range language
      guard (plain guard not assert — deliberate). Follow-ups: CSD 2-classes-1-TU,
      HelpVersionCatalog. Reviewed + registered + 2 suites.
- [x] P17 Importers + Dialogs — ProtoLexer + ProtoParser classes (ProtoImporter.cpp
      1876→1147; parser now QtCore-only, which unblocked tst_proto_parser — the coverage the
      2026-07-25 analysis said a TU split couldn't buy), DBCMux free fns (DBC 1168→957),
      ModbusMap readers (1257→768). Commercial gating respected (DBC/ModbusMap units in Pro
      block). Follow-up: ProtoLuaGenerator (~440 lines codegen). Reviewed + registered +
      1 suite.
- [x] P18 Scripting core — ScriptResult/ScriptDeviceWait/ReplayRowCodec/ParserTemplateCatalog/
      ScriptFrameShaping (header-inline unzip); 12 files deduped, guardedCall/watchdog/routed-
      lambda contracts untouched. Naming collision with Editors' ScriptTemplateCatalog resolved
      by rename — future merge candidate. Follow-ups: ControlApiMarshaller own pair,
      LuaStateSupport dedup, ExpressionTransform split (benchmark-gated). Reviewed +
      registered + 2 suites.

## Wave 4 — Multi-TU class re-forms + cores
- [x] P19 ProjectModel family re-form — 11 real classes (Persistence, Presentation, Loader,
      Folders, Workspaces, Tables, Sources, Entities, OutputWidgets, BulkOps, NavHistory) +
      2 narrow headers; BOTH Shared.h coupling headers DELETED; 7 multi-TU ProjectModel TUs
      absorbed. Undo contract: scope+setModified pairs verbatim, whitelist preserved;
      undo-scope-missing linter glob widened by integrator to follow mutators (verified 0
      violations). Ctor closure unchanged. instance() 83→83. Facade ProjectModel.cpp
      2299→2735 (forwarders — honest report; named next cut ProjectWorkspaceRefs).
      Integrator applied the 2 ProjectEditor hunks (NavEntry alias, m_nav member) + CMake
      swap + 2 suites. tests/scripts save-pin updated; tier 302 passed.
- [x] P20 Editors + DataModel root — EmbeddedCodeEditor bridge (5x duplicated offscreen
      plumbing → 1; RenderGate preserves per-editor visibility gating verbatim),
      EditorFormatting (12 copies → 2 fns), ScriptTemplateCatalog; instance() net −6.
      DataModel root verdicted clean, routing rule intact. THREE DEFECTS REPORTED (morning
      list): post-shutdown dangling function-local static refs to SessionContext singletons
      (NotificationCenter/DataTable/ProjectEditor sites), copy_frame_values() unasserted OOB
      write precondition, FrameConsumer flush-before-init null worker window. MacroEditor
      weaker render gate preserved (maintainer decision pending). ProjectEditor god-class
      5-cluster plan recorded. Reviewed + registered + 1 suite.
- [x] P21 API Handlers — ProjectHandler re-formed: 13 real command classes + ProjectApiSupport
      namespace pair; facade 1815→77; registerCommand count 70→70, names/order byte-identical;
      instance() 115→90 (registry injected). applyDatasetUpdateParams stays on facade
      (generated DatasetApiFields.cpp defines it — untouchable seam). 39 handlers verdicted
      clean. Integrator: CMake swap done; tests/scripts split-maps expanded to all new
      component dirs + 5 shape-pins updated — full tier 302 passed. Follow-up: requireParams()
      dedup (~200 sites), pure-helper unit for tst_project_api_support.
- [x] P22 IO core — ReplyCapture (mutex + buffers one unit; armed() inline atomic on chunk
      path), ConnectFanOut (dial-verdict bookkeeping; emits + doctrine stay on facade);
      ConnectionManager.cpp 2264→2208. Stream wiring NOT moved (receiver-context argument —
      correct). FrameReader/CircularBuffer/PipelineHost/StreamWorker verdicted clean, no
      edits. Morning watch: async dialers (BLE/ModbusTCP/WS/OPC UA) for stuck connect button.
      Follow-ups: StreamConfigBuilder, DriverFactory, FileTransmission SessionContext slot.
      Reviewed + registered + 1 suite.
- [x] P23 UI core + root sweep — WidgetManifestParser re-formed from decl-in-facade-header
      TU (WidgetExtensionManifest.cpp deleted; CMake swapped); spec-0038 trust semantics
      byte-identical, all 7 rejection codes now test-pinned. 30+ files verdicted clean.
      Follow-ups: SerialStudio 2-TU split (text-encoding class), HotpathBenchmark clusters,
      CommandManifestLoader. Reviewed + registered + 1 suite.

## Wave 5 — Hotpath (serial, manager line review)
- [ ] P24 FrameBuilder (per-frame lanes stay in facade TU / header-inline)
- [ ] P25 Dashboard (drain/apply/push tables stay in facade TU)

## Wave 6 — Integration (manager)
- [x] T1 CMake registration — all 25 packages registered; deleted TUs de-registered
- [x] T2 Composition root — untouched by design; no facade ctor signature changed
- [x] T3 code-verify — 0 errors, 28 advisories (pre-campaign baseline was 34); cleanup agent removed 65 new-code advisories; tree-sitter misparse fixed at source
- [x] T4 Singleton census SHRANK: 1762→1718 occurrences, static-cache 1279→1225; re-seeded
- [x] T5 pytest tests/scripts — 302 passed (split-maps widened, 6 shape-pins updated)
- [x] T6 Docs — directory-map, io/dashboard/scripting architecture docs, CLAUDE.md god-file row, ss-new-driver skill, generator comment, spec Progress; claim-verify 0 errors 0 advisories
- [x] T7 sanitize-commit.py ran clean (formatting + .code-report regenerated); NO commit made
- [x] T8 TU census: 32 files/23,643 excess/worst 4612 → 19 files/11,581/worst 3544; critical 2→0; re-seeded
- [x] T9 Morning report delivered in chat 2026-08-28

## Test exemptions log
(recorded as packages report; unit must be singleton-bound or device-library-bound)

## Deferral log
(anything `[-]`, with reason and follow-up shape)
