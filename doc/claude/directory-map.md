# Directory Map

```
app/src/
├── IO/              ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig,
│   │                PipelineHost (frame-processing thread), StreamWorker (typed stream lane),
│   │                AsyncTcpDial (resolve, probe, one connect, one verdict)
│   ├── ConnectionManager/  the facade's sub-objects: ConnectFanOut, DeviceIoRouter (the byte
│   │                path + framing), DeviceTableQuery (every read, incl. IO::LinkStats),
│   │                DriverFactory, DriverUiRegistry, ReplyCapture, StreamConfigBuilder,
│   │                StreamWorkerPool, UiDriverSync
│   ├── Drivers/     UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, MQTT, Process, USB,
│   │                OpcUa (+ OpcUaTagModel browse tree, OpcUaWire.h delta-frame vocabulary),
│   │                S7, EthernetIp, Iec104 + PolledPlcWorkerBase (the shared polled-PLC worker);
│   │                CANBus/SerialCanBackendBase for the two serial CAN adapters
│   └── FileTransmission/  Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── Async/           TaskTree, RetryPolicy, AsyncClock — task-tree engine used by MQTT::Publisher
│                    and the spec-0035 diagnostics probes; boundaries only, nothing per frame
├── DataModel/       Frame, FrameKeys.h (namespace Keys), FrameBuilder, FrameConsumer,
│   │                DataTable(Store), ExportSchema, ExportStructure (the schema half every
│   │                export worker owns), ReplayPlaybackEngine (the players' shared mechanics),
│   │                ProjectModel, ProjectEditor, NotificationCenter, HotpathOptimization.h
│   ├── FrameBuilder/  the facade's sub-objects: BlockStager (staging + the block pool),
│   │                BlockPublisher (the sink fan-out), ReplayIngest, TransformCompiler,
│   │                LatestFrameTap, QuickPlotBuilder, TableScriptBridge, TableSnapshotChannel
│   ├── Project/     ProjectModel's owned sub-objects (spec 0070): ProjectPersistence,
│   │                ProjectPresentation, ProjectLoader, ProjectFolders, ProjectWorkspaces,
│   │                ProjectTables, ProjectSources, ProjectEntities, ProjectOutputWidgets,
│   │                ProjectBulkOps, ProjectNavHistory (+ ProjectNaming.h,
│   │                ProjectEditorIcons.h, ProjectEditorItemIds.h); the ProjectEditor*
│   │                per-concern TUs remain (follow-up: re-form into classes).
│   │                Plus ProjectHistory (undo/redo mementos, spec 0031) and
│   │                PropertyHooks/PropertyValidators (registry hooks, spec 0036)
│   ├── Generated/   DatasetForm, DatasetRegistry.h, DatasetSerialization — emitted by
│   │                generate-property-registry.py from app/rcc/properties/dataset.json;
│   │                never hand-edit
│   ├── Scripting/   IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │                LuaScriptEngine, LuaCompat, LuaDeadlineHook (the one Lua count hook),
│   │                ScriptDryRun (the one GUI-thread throwaway evaluation), ScriptTemplates
│   ├── Editors/     EmbeddedCodeEditorItem (the base all five hosts derive from) +
│   │                ControlScriptEditor, JsCodeEditor, MacroEditor, OutputCodeEditor,
│   │                PainterCodeEditor, DatasetTransformEditor, CodeFormatter
│   ├── Importers/   DBCImporter, ModbusMapImporter, ProtoImporter
│   └── Dialogs/     TransmitTestDialog
├── UI/              Dashboard, Taskbar (workspaces), visualization + output widget types,
│   │                WidgetExtensions + WidgetExtensionManifest (installable widgets, spec 0038)
│   ├── Dashboard/   DashboardIngest (the block-ingest sub-object + every push table),
│   │                DashboardTools
│   ├── Widgets/Waterfall/  ColorMap (+LUT), Overlay, Tiles, RingTexture (QRhi scanline
│   │                uploads), SpectrogramNodes (both draw paths)
│   ├── Widgets/     PlotBase (state Plot/MultiPlot/FFTPlot share, composed not inherited),
│   │                GpuStroke (grow-only geometry + degenerate padding), WidgetBands.h
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── AI/       (Pro) The in-app assistant: Assistant, Conversation, ToolDispatcher,
│   │                CommandRegistry (safety tiers), FileSandbox, KeyVault, Redactor,
│   │                SentinelProbe, SseEventReader — see architecture/ai.md
│   ├── Conversation/  the turn's sub-objects: ToolTurnRunner, AsyncToolRunner, MetaToolCatalog,
│   │                MetaToolRunner, TokenBudget, HistorySurgery, ReplyAssembly, AutoVerifier
│   ├── Providers/   Provider/Reply base + Anthropic, OpenAI, Gemini, Local, and
│   │                OpenAICompatibleProvider (one adapter, four vendor tables)
│   └── Tools/       ToolCatalog/Schemas/Dispatch and the fs, script, tile and bulk tools
├── API/             TCP server port 7777 (MCP + legacy JSON-RPC), 30+ handlers,
│   │                PathPolicy (the one path-parameter gate)
│   ├── Handlers/    per-command handlers; ProjectHandler is a facade owning 13 real
│   │                Project*Commands classes + the ProjectApiSupport namespace (spec 0070);
│   │                Diagnostics/Problems/Mirror handlers
│   ├── Mirror/      MirrorProtocol/Session/Publisher/Client — remote dashboard mirroring
│   │                (spec 0040)
│   └── Generated/   DatasetApiFields.cpp — property-registry emitter output; never hand-edit
├── Console/         Terminal + export
├── CSV/ MDF4/       File playback & export
├── Sessions/  (Pro) DatabaseManager + Sessions::Export + Sessions::Player
├── MQTT/            Publisher (FrameConsumer-based, threaded, rate-limited 1-30 Hz)
├── Licensing/       LemonSqueezy, Trial, MachineID, CommercialToken (FeatureTier)
├── Platform/        CSD, NativeWindow (true-size CSD windows: no painted shadow; Win10 gets a
│                    DWM-drawn shadow via WM_NCCALCSIZE filter, Linux the 1px border);
│                    FileOpenEventFilter (queued QFileOpenEvent .ssproj open, unit-test-linkable)
├── Misc/            JsonValidator, ThemeManager, ModuleManager, ContextRegistry (the one
│   │                QML-globals table), ProblemCenter (pull-only problem aggregation,
│   │                spec 0033), ConnectionDiagnostics (spec 0035)
│   ├── Extensions/  ExtensionCatalog (catalog v2 + per-file digests), ExtensionInstaller
│   │                (staged atomic install), PluginRunner
│   ├── Problems/    Project/Script/Link/Extension checkers (synchronous, polled)
│   └── Diagnostics/ Serial/Network/Bluetooth/Audio checks + DeviceAccess probes
├── SelfTest/        In-process --selftest suites (SS_INAPP_TESTS); run before the
│                    composition root — never touch a singleton
├── AppState.h       Singleton: OperationMode, projectFilePath, FrameConfig
├── SessionContext.h Session-scoped service context (spec 0039)
├── SSAssert.h       Soft-assert layer: SS_ASSERT(cond, action) — release-safe recovery,
│                    debug abort; always-evaluated condition
├── SerialStudio.h   Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h       C++20 concepts
app/qml/             Commands/, DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/,
                     Dialogs/ (incl. ProblemCenter, RemoteAttach, ExtensionConsent)
app/rcc/             icons/ + commands/ (spec 0028), properties/dataset.json (spec 0036),
                     extensions/ (bundled widget-extension packages, spec 0038), ai/ corpus
app/tests/           C++ unit-test tier (spec 0032): Qt Test suites via ss_add_unit_test,
│                    SS_BUILD_TESTS=ON (no CMake presets; configure by hand)
    ├── support/     shared doubles: FakeDriver, FakeProvider, FakeTransport
    └── fuzz/        libFuzzer entry points + corpus/, also runnable as ctest corpus replay
examples/            example projects; widget-extension/ = spec-0038 package template
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, luajit, hidapi, QSimpleUpdater,
                     open62541, mbedtls, libplctag; VERSIONS.json pins every vendored tree
```
