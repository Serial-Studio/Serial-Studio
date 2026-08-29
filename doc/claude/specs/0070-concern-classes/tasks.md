---
spec: 0070-concern-classes
phase: tasks
status: done         # closed 2026-08-29: Wave 5 P24/P25 superseded by Wave 7 Q1/Q2; all waves landed
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
- [x] P24 FrameBuilder — superseded by Wave 7 Q2 (TableSnapshotChannel + LatestFrameTap,
      3534→3348; remainder proven per-frame core, Q19). Closed 2026-08-29.
- [x] P25 Dashboard — superseded by Wave 7 Q1 (ReplaySeekEngine + PlotControlBank + ctor
      split, 3544→2724; push tables stayed facade). Closed 2026-08-29.

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


## Wave 7 — Round 2 (2026-08-28 night 2): census reduction + singleton captures
Directive: build is green and pushed; drive TU census (19 files / 11,924 excess) and
singleton census (1198 static-cache sites) down hard. Same model: manager briefs, opus
executors, disjoint ownership, integrator handles CMake/docs/gates. After the main task:
teach the translation pipeline to reuse identical strings whose context/location moved.

- [x] Q1 Dashboard 3544→2724 — ctor split into 4 helpers; ReplaySeekEngine (bindings bundle
      WITHOUT clocks — deliberately stronger than briefed: engine cannot desync the pair;
      resetPlotClocks stays facade); PlotControlBank (one owner per state: rings→engine,
      sweeps→bank, clocks→facade; license gate diversity preserved); 40/43 pinned bodies
      byte-identical, 3 diffs = call-site reroutes only; captures 40→26, static-caches 34→13
      (13 survivors = byte-identity-frozen sites). Benchmark watch: lua+dashboard floor;
      designated-init-with-ifdef = likeliest compile complaint. Reviewed + registered +
      1 suite.
- [x] Q2 FrameBuilder 3534→3348 — TableSnapshotChannel + LatestFrameTap (SPSC lanes moved
      whole; drain/publish handshake split so marshal stays facade-owned; per-frame capture
      trio stayed). HONEST SHORTFALL vs ≤2900 accepted: full audit shows remaining lines are
      forbidden per-frame core; only relaxation candidate = TransformRunner (~210, per-frame
      Lua/JS — benchmark-gated follow-up). instance() 62→62 (all remaining in forbidden lanes
      or ctor-reachable — proven). Ctor-order sensitivities documented. Reviewed + registered
      + 2 suites (channel suite's PipelineHost stubs = only unproven link).
- [x] Q3 ProjectModel complex — ProjectModel.cpp 3095→1149 (227 one-line forwarders inlined;
      header 1224, 276 under cap; scope-bearing setters deliberately held back), Workspaces
      1683→1295 via WorkspaceRefs free-fn unit (zero setModified/UndoScope — contract clean),
      Entities 1501→1362 via FixedLayouts unit (ProjectModel::tr preserved), both long-fn
      advisories fixed. instance() 120→120 (documented). Tests exempt (SerialStudio.cpp link
      set; suite unlocks if eligibility predicates get a leaf TU — recorded). Follow-up:
      naming rules consolidation into ProjectNaming.h. Reviewed + registered.
- [x] Q4 Terminal 2364→1938 — TerminalBuffer value class (event-drain design: buffer records,
      facade publishes once per chunk — cursorMoved/scrollOffsetY consumers verified absent/
      unbound) + Vt100Keymap free fns; paint path proven substitution-only via diff range
      audit; captures 7→6. Honest gap to 1600: TerminalRenderer (~590 lines) = named
      follow-up, outside mandate. KATs validated against a Python model pre-write (caught 2
      wrong assumptions). Reviewed + registered + 1 suite (21 slots).
- [x] Q5 OpcUa 2338→1624 — OpcUaProjectBuilder (pure, tested, ModbusProjectGenerator shape),
      OpcUaSubscriptions + host interface (P08's 7 'HAL-shared' members DISPROVEN by
      inspection — 3 were already assembler-private, 4 engine-exclusive; dial-verdict
      isolation proven by grep), endpoint presentation into existing pure unit. Captures
      4→4 with live-vs-UI-instance reasoning (setupExternalConnections never runs on live
      drivers — correct refusal). Morning: real-PLC watchdog/poll re-arm watch; lupdate.
      Reviewed + registered + 1 suite.
- [x] Q6 ConnectionManager 2208→1702 — StreamConfigBuilder, DriverFactory (guard diversity
      INCREASED 8→11 dispatch sites — clever licensing preservation), UiDriverSync (echo
      fence intact), StreamWorkerPool (queued blockReady verbatim); captures 65→18 (−47;
      ctor refs justified by headless-verifier path). Integrator RE-VERIFIED ctor-edge proof:
      PM 700 → AppState 701 → FB 702 → CM 704, sole construction site = root. Morning watch
      #1: onDriverOpenFinished's deviceIdForDriver (hung-connect failure mode) — manual async
      dial test. 4 units test-exempt (session-bound/device-bound — recorded). Reviewed +
      registered.
- [x] Q7 Conversation 2086→1513 — MetaToolRunner behind MetaToolSink (5 of 7 methods =
      existing privates promoted to overrides — no forwarders), AutoVerifier, ChatDigest
      (tested), ToolCallStatus namespace enum (Q_ENUM dropped — QML compares raw ints,
      verified). Captures 13→4 with the load-bearing finding: Conversation is built INSIDE
      Assistant's ctor — Assistant capture would re-enter the Meyers guard (2026-07-07
      class); lazy accessor correct. DocSearch stays lazy (BM25 parse cost — considered
      deviation, right call). Morning: lupdate (4 AutoVerifier strings). Reviewed +
      registered + 1 suite.
- [x] Q8 CLI 1943→995 — honest seam found: all 12 setup*Connection members read ONLY
      m_parser/m_opts → free functions in CliBusConfig / CliIndustrialConfig (license-boundary
      split: GPL builds stop compiling 6 Pro driver headers) / CliSpecParsers (pure, tested);
      instance() 49→29, all conversions provably post-root; selftest paths byte-identical;
      dead applyModbusCommonOptions decl removed; option-table + selftest moves declined with
      numbers. Reviewed + registered + 1 suite.
- [x] Q9 PainterContext 1666→1321 — multi-TU split ENDED (Gradient/Pattern own pairs +
      factories back home), PainterEnums + PainterGeometry header-inline (per-frame inlining
      kept), PainterBlur kernel verbatim (magic 32 → named constant), PainterFont takes
      CommonFonts& (instance() −1). Style state + shadow pipeline stayed (honest). Review
      watch: normalizedSweep fold (only genuine rewrite; heavily tested), M_PI include
      provenance. Reviewed + registered + 2 suites (BUILD_COMMERCIAL).
- [x] Q10 Taskbar 1700→1286 — TaskbarWorkspaces (Taskbar& back-ref documented as the
      deliberate exception; signal-relay pattern; 5 deps by ctor ref, zero instance());
      instance() 10→8. Review watch: selectGroupAfterRebuild optional return + deleteWorkspace
      control-flow rewrite (semantics argued equivalent). Carried-over hazards flagged:
      clearWorkspaceWidgets iterator invalidation (→P19 follow-up), tree recursion uncapped.
      Reviewed + registered. Tests exempt (singleton-bound; data-seam extraction = follow-up).
- [x] Q11 DSP.h 1934→1353 (DSPDownsample.h 628, include-through, MD5-verified byte-identical
      region) + Frame.h 1606→1388 (FrameKeys.h 243; serializer set verdicted irreducible —
      name-lookup-coupled + generator-derived). 540 excess lines retired, zero consumer
      edits, spec-0021 contract untouched by construction. Reviewed + headers registered.
- [x] Q12 Settings.qml 2243→247 — 8 zero-parameter pages under Dialogs/Settings/ (grep
      proved zero outer-scope reaches; only coupling is root's implicitHeight Math.max over
      instance ids — unchanged). Line-diff verified: only id renames. GPL empty-Notifications
      StackLayout-index trick preserved. Reviewed + 8 QML files registered.
- [x] Q13 PlotWidget.qml 2161→908 — 7 components (formatter, interaction, ruler overlay/menu,
      marker popup, cursor ×2 instances, trigger line); core GraphsView block PROVEN
      byte-identical by diff; all 4 consumers' symbols preserved by name; data-rate binding
      changes limited to 4 one-hop formatter calls. Morning: visual smoke test (cursors,
      ruler, hover marker, sweep trigger). Reviewed + 7 QML registered.
- [x] Q14 FlowDiagram.qml 1703→357 — 9 components (layout engine QtObject with verbatim
      build(), icons, arrows canvas, node delegate + 4 visuals, menu controller preserving
      CommandMenu wiring); node records now carry stamped key (id-reach eliminated).
      Integrator did the proper _FONT_PIXEL_OK_FILES fix + stripped fences. Morning items:
      lupdate (14 strings → DiagramLayout context); dir/file name coexistence = loud-fail
      risk at qmlcachegen. tablefolder dual-instantiation quirk preserved deliberately.
      Reviewed + 9 QML registered.
- [x] Q15 IO sweep — 50→47 total, static-cache 33→23 (−10): 5 drivers capture ProjectModel/
      AppState by ctor ref (order proof: adopted at ModuleManager 700/701, drivers built at
      704); 15 driver→ConnectionManager sites PROVEN unconvertible (ctor runs inside CM
      construction — capture = startup qFatal). QCanBus = Qt's accessor, not ours. TWO
      INTEGRATOR FOLLOW-UPS RECORDED: (a) FileTransmission → SessionContext 10th slot (fixes
      2 real teardown defects — dangling CM pointer + post-QApplication dtor; full edit list
      in report; maintainer call on core-module status), (b) PipelineHost::routeFrames could
      use its own m_frameBuilder member instead of static cache (hotpath owner change,
      benchmark-gated). Reviewed; no CMake.
- [x] Q16 Misc/Licensing/Platform sweep — 133→106 sites (−27): checker free functions get
      file-local accessors, Handler/CSD/HelpCenter ctor captures with order proofs, Licensing
      untouched (security surface; ~−9 available if maintainer sanctions). FINDING recorded in
      startup.md: NativeWindow by-value member constructs ThemeManager→WorkspaceManager→
      Translator BEFORE the pinned order (pre-existing, benign, but exempts those files from
      order-based captures). InfluxDB lambda raw-pointer tradeoff noted. Reviewed; no CMake.
- [x] Q17 Sessions/CSV/MDF4/Importers sweep — Player synthesis() 3 sites → sanctioned cache
      form (Player has NO setupExternalConnections; ctor capture ruled out by composition
      order — correct deviation); ProtoParser loops given visible Eof sentinel (no numeric
      cap — preserves valid pathological inputs); 74 files audited, all 148 remaining sites
      already sanctioned forms; refused Dashboard ctor-captures (built last) and Export
      wiring captures (null on headless path) — right calls. Owned advisories 6→1
      (pre-existing TU length). Note: ProjectLoader/Persistence long-fn fixes are Q3's.
- [x] Q18 API/AI/MQTT sweep — ProjectHandler::registerCommands(CommandRegistry&) threaded
      from sole caller (zero new resolutions); real captures: ProcessLauncher 6→3, Publisher
      7→5, Assistant + GRPCServer ctor-init refs (order proofs given); totals 599→589.
      484 remaining = handler statics by design; load-bearing (void) registration forcer,
      security surfaces, gRPC worker threads, ctor-edge guards all correctly left.
      Reviewed; no CMake needed.
- [x] Q19 Integration — all 18 packages registered; advisory cleanup 40→9 (9 = accepted TU
      floor); doc drift fixed (TableSnapshotChannel/MetaToolRunner/DriverFactory renames);
      claim-verify 0 errors; censuses re-seeded: TU 19 files/11,924 → 9/3,967 excess
      (worst 3348, both remaining 'gods' = proven per-frame cores); singleton 1718→1584,
      static-cache 1225→1075; two ring shape-pins updated to follow ReplaySeekEngine;
      pytest 302 green; sanitize clean; NO commit.
- [x] Q20 Translation reuse — reuse_existing_translations() pre-pass + --reuse-only mode in
      llm_translate.py; keyed (source, comment), conflict-safe (all-donors-agree), numerus/
      comment paths synthetic-fixture tested, PINNED_TRANSLATIONS priority kept, byte-identical
      round-trip, idempotent. Dry run: 3,472 recoverable across 20 languages (141-196 each),
      43 conflicts left for LLM, 3,471 of the recoveries are flag-only (lupdate had already
      copied the text). NO .ts modified — maintainer runs `python3 llm_translate.py
      --reuse-only` then the normal LLM pass for the 43. Known tradeoff: silently overwrites
      a human's deliberately-unfinished draft (1 entry today).
