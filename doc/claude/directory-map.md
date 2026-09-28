# Directory Map

Seven statically linked CamelCase libraries live under `core/`; `app/src` holds only the
composition root (spec 0076). Two include conventions: `Core`/`Protocols` are reached by
layer name (`#include "Core/…"`, `#include "Protocols/…"`, root = `core/`) and are STRICT —
`layer-verify.py` errors on any include outside their allowed lower layers. The five
partition libraries carved out of `app/src` (`Pipeline`, `Devices`, `Storage`, `Api`, `Ui`)
keep their ORIGINAL relative include paths verbatim, so `#include "DataModel/Frame.h"` still
resolves — each of `core/Pipeline`, `core/Devices`, `core/Storage`, `core/Api`, `core/Ui`,
is its own include root, and each sees only the roots of the layers below it that it may
include. Since spec 0077 they are STRICT like Core and Protocols: an include, an include root or
a link outside the graph fails `scripts/layer-verify.py` (`layer-upward`,
`cmake-root-violation`), `scripts/layer-baseline.json` holds no ratcheted edge, and no library
includes `app/src`. The downward-only graph below is the tree:

```
Ui -> Api -> Storage -\
Ui -> Devices --------> Pipeline -> Protocols -> Core -> Qt6::Core
Ui -> Pipeline --------/
```

(`Api`, `Storage`, `Devices` also depend on `Pipeline`/`Core`/`Protocols`; the full
adjacency is `LAYERS` in `scripts/layer-verify.py`.) State crosses a library boundary only as a
`Core::Bus::MessageBus` topic (see "Message bus" below), through an interface the composition
root binds (`IIngestBinder`, `IBlockSink`, `IRawByteTap`, `IRawFrameTap`, `IMqttPublisher`,
`ICommandExecutor`, `ICheckpointStore`, `IReplayPlotSink`, `IDashboardControl`,
`IDashboardFrames`, `IDeviceWriter`, `IPayloadInjector`), or through one of the three
root-bound module sets a library reads instead of a foreign `instance()`: `Core::services()`
(bus, Translator, TimerEvents, WorkspaceManager, IconRegistry), `DataModel::pipelineModules()`
(AppState, ProjectModel, FrameBuilder, FrameParser, ControlScript, PipelineHost,
NotificationCenter) and `API::handlerContext()` (ConnectionManager, the dashboard's frame
surface, the command registry and server, the players, exports and the historian).
`code-verify.py --singleton-census` fails on any cross-library reach left inside `core/`.

`core/CMakeLists.txt` adds all seven targets, called from `app/CMakeLists.txt` before
`include_directories(src)`, so no library translation unit can see `app/src/`. Every library
is `STATIC`, named `SerialStudio<Layer>` with alias `SerialStudio::<Layer>`, one owning
`CMakeLists.txt` per directory.

```
core/
├── Core/            (target SerialStudioCore, alias SerialStudio::Core; links only Qt6::Core)
│   │                SSAssert, Concepts, DSPSimd, HotpathOptimization.h, ParseBudget.h,
│   │                CircularBuffer, Checksum
│   ├── DataModel/   Frame, FrameKeys.h (namespace Keys), DataBlock, ExportSchema, FrameConsumer,
│   │                FrameSupport, IBlockSink (the sink seam), PropertyValidators (the colour
│   │                hook), Generated/DatasetSerialization.cpp (never hand-edit)
│   ├── IO/          FrameConfig, HAL_Driver, StreamConfig, LinkStats and the seam interfaces the
│   │                root binds: IIngestBinder, IRawByteTap, IRawFrameTap, IMqttPublisher,
│   │                IDeviceWriter, IPayloadInjector; FrameConfigBuilder derives a reader's
│   │                framing from the retained project snapshot
│   ├── Api/         CommandProtocol (the request/response value types) + ICommandExecutor,
│   │                the seam the scripting layer drives commands through (spec 0077 T60)
│   ├── Licensing/   CommercialToken (Pro) — the HMAC-sealed capability token every guard site
│   │                in the libraries above reads; QtCore and the generated guards header only
│   ├── Services.h   the root-bound Core service set: bus, Translator, TimerEvents,
│   │                WorkspaceManager, IconRegistry (spec 0077 T71)
│   ├── Async/       TaskTree, RetryPolicy, AsyncClock — used by MQTT::Publisher and the
│   │                spec-0035 diagnostics probes
│   └── Bus/         MessageBus, Subscription (RAII), Messages.h — the in-process, typed
│                    (topics are C++ types), memory-based bus every layer talks over instead
│                    of a singleton reach; never on the per-frame path (spec 0076 stage 2)
├── Protocols/       (target SerialStudioProtocols, alias SerialStudio::Protocols; links
│   │                SerialStudio::Core PUBLIC, plus Qt6::Network for Tls/) pure wire-format
│   │                codecs — no sockets, dialogs, settings or singletons
│   ├── CAN/         CanReassembly, GsUsbProtocol (Pro; gated BUILD_COMMERCIAL OR
│   │                SS_BUILD_TESTS)
│   ├── S7/          IsoTsap, S7Pdu, S7Address (Pro)
│   ├── Iec104/      Apci, Asdu (Pro)
│   ├── Sparkplug/   SparkplugPayload (Pro)
│   ├── Modbus/      ModbusRtuCodec (Pro)
│   ├── Tls/         TlsIdentity, TlsConfig (Pro) — the QSslConfiguration both MQTT ends build
│   └── FileTransfer/  CRC, Protocol base, XMODEM, YMODEM, ZMODEM (GPL base)
├── Pipeline/        (target SerialStudioPipeline, alias SerialStudio::Pipeline; links Core and
│   │                Protocols only) the frame/parse/project engine
│   ├── DataModel/   FrameBuilder, DataTable(Store), ExportStructure, ReplayPlaybackEngine,
│   │   │            ProjectModel, NotificationCenter, ActionBytes, TextCodec (the frame value
│   │   │            types themselves live in core/Core/DataModel/); PipelineModules (the
│   │   │            root-bound Pipeline module set) and the dashboard seams IReplayPlotSink,
│   │   │            IDashboardControl, IDashboardFrames the players and the API drive
│   │   ├── FrameBuilder/  the facade's sub-objects: BlockStager (staging + the block pool),
│   │   │            BlockPublisher (the sink fan-out), ReplayIngest, TransformCompiler,
│   │   │            LatestFrameTap, QuickPlotBuilder, TableScriptBridge, TableSnapshotChannel
│   │   ├── Project/  ProjectModel's owned sub-objects (spec 0070): ProjectPersistence,
│   │   │            ProjectPresentation, ProjectLoader, ProjectFolders, ProjectWorkspaces,
│   │   │            ProjectTables, ProjectSources, ProjectEntities, ProjectOutputWidgets,
│   │   │            ProjectBulkOps (+ ProjectNaming.h, WorkspaceKeys — the widget-ref
│   │   │            key + lookup the editor and the workspaces API share). Plus
│   │   │            ProjectHistory (undo/redo mementos, spec 0031) and
│   │   │            PropertyHooks/PropertyValidators (registry hooks, spec 0036)
│   │   ├── Generated/  DatasetRegistry.h, DatasetSerialization — emitted by
│   │   │            generate-property-registry.py from app/rcc/properties/dataset.json;
│   │   │            never hand-edit
│   │   ├── Scripting/  IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │   │            LuaScriptEngine, LuaCompat, LuaDeadlineHook (the one Lua count hook),
│   │   │            ScriptDryRun (the one GUI-thread throwaway evaluation), ScriptTemplates
│   │   ├── Editors/  CodeFormatter, EditorFormatting, ScriptTemplateCatalog (the
│   │   │            model-side script helpers; the editor hosts live in core/Ui/ProjectEditor/)
│   │   └── Importers/  DBCImporter, ModbusMapImporter, ProtoImporter
│   ├── IO/          FrameReader, PipelineHost (frame-processing thread; owns the readers and
│   │                implements IIngestBinder), StreamWorker (typed stream lane),
│   │                StreamWorkerPool (the per-source worker lifecycle)
│   ├── DSP.h DSPDownsample.h
│   └── Platform/    AppPlatform.*
├── Devices/         (target SerialStudioDevices, alias SerialStudio::Devices) drivers +
│   │                connection plumbing; reaches nothing above Core/Protocols (spec 0077)
│   ├── IO/          ConnectionManager, DeviceManager, AsyncTcpDial (resolve, probe, one
│   │   │            connect, one verdict), FileTransmission.h/.cpp (the transfer facade;
│   │   │            X/Y/ZMODEM codecs live in core/Protocols/FileTransfer/)
│   │   ├── ConnectionManager/  the facade's sub-objects: ConnectFanOut, DeviceIoRouter (the
│   │   │            byte path + framing, fanning chunks to the root-bound raw taps),
│   │   │            DeviceTableQuery (every read), DriverFactory, DriverUiRegistry,
│   │   │            ReplyCapture, StreamConfigBuilder, UiDriverSync, BusBridge
│   │   ├── Audio/    WavDecoder, SoundBank, SoundPlayer: the aural alert playback device
│   │   │            (spec 0087; every build, own ma_context, two-lane SPSC mixer, no Qt in
│   │   │            the callback); the vendored miniaudio TU compiles here in every build
│   │   └── Drivers/  UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, MQTT, Process,
│   │                USB, OpcUa (+ OpcUaTagModel browse tree, OpcUaWire.h delta-frame
│   │                vocabulary), S7, EthernetIp, Iec104 + PolledPlcWorkerBase (the shared
│   │                polled-PLC worker); CANBus/SerialCanBackendBase for the two serial CAN
│   │                adapters; GeneratedProjectRequest (the drivers' project-generation ask)
├── Storage/         (target SerialStudioStorage, alias SerialStudio::Storage)
│   ├── CSV/         File playback & export
│   ├── MDF4/        File playback & export
│   ├── MQTT/        (Pro) Publisher (FrameConsumer-based, threaded, rate-limited 1-30 Hz) +
│   │                PublisherWorker, CsvExpansion, SparkplugPublisher, PublisherScript,
│   │                BrokerOptions, NotificationPayload
│   ├── Sessions/    (Pro) DatabaseManager + Sessions::Export + Sessions::Player
│   └── InfluxDB/    LineProtocol.h + Export — InfluxDB 2.x sink, line protocol per
│                    published block
├── Api/             (target SerialStudioApi, alias SerialStudio::Api) TCP server port 7777
│   │                (MCP + legacy JSON-RPC), 30+ handlers, PathPolicy (the one
│   │                path-parameter gate) — `API/GRPC/` stays in app/src (its protoc custom
│   │                command)
│   └── API/
│       ├── HandlerContext.h  the root-bound Devices/Storage/Api module set the handlers and
│       │            the Ui read (ConnectionManager, dashboard frames, registry, server,
│       │            players, exports, historian) — spec 0077 T71
│       ├── Handlers/  per-command handlers; ProjectHandler is a facade owning 13 real
│       │            Project*Commands classes + the ProjectApiSupport namespace (spec 0070);
│       │            Mirror handler. ICheckpointStore is the pre-mutation snapshot seam; the
│       │            handlers that reach the UI live in Ui/ApiHandlers/ (spec 0077 T62)
│       ├── Mirror/  MirrorProtocol/Session/Publisher/Client — remote dashboard mirroring
│       │            (spec 0040)
│       └── Generated/  DatasetApiFields.cpp — property-registry emitter output; never
│                    hand-edit
└── Ui/              (target SerialStudioUi, alias SerialStudio::Ui) dashboard, widgets, the
    │                AI assistant, platform chrome
    ├── UI/          Dashboard, Taskbar (workspaces), visualization + output widget types,
    │   │            WidgetExtensions (installable widgets, spec 0038), CommandStrings.cpp
    │   │            (generated, never hand-edit), SnapGuides
    │   ├── Dashboard/  DashboardIngest (the block-ingest sub-object + every push table),
    │   │            DashboardTools
    │   ├── Alarms/    AlarmAnnunciator (root-owned facade, Cpp_UI_Alarms, alarms.* API) +
    │   │            AnnunciatorSequence (ISA-18.1 point table), SoundTheme, AppEventSounds
    │   │            (spec 0087)
    │   ├── Widgets/Waterfall/  ColorMap (+LUT), Overlay, Tiles, RingTexture (QRhi scanline
    │   │            uploads), SpectrogramNodes (both draw paths)
    │   ├── Widgets/  PlotBase (state Plot/MultiPlot/FFTPlot share, composed not inherited),
    │   │            GpuStroke (grow-only geometry + degenerate padding), WidgetBands.h
    │   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
    ├── ProjectEditor/  ProjectEditor (the QML facade: every Q_PROPERTY/Q_INVOKABLE/slot/signal
    │   │            plus the shared state) owning eight by-value sub-objects in the spec 0070
    │   │            shape, one class per .h/.cpp pair: EditorWiring, EditorSelection,
    │   │            EditorTree, EditorForms, EditorCommit, EditorSummaries, EditorMultiSelect,
    │   │            EditorMqtt; ProjectNavHistory, ProjectEditorIcons.h, ProjectEditorItemIds.h
    │   ├── Editors/  EmbeddedCodeEditorItem (the base all five hosts derive from) +
    │   │            ControlScriptEditor, JsCodeEditor, MacroEditor, OutputCodeEditor,
    │   │            PainterCodeEditor, PublisherScriptEditor (Pro), DatasetTransformEditor,
    │   │            FrameParserModel
    │   ├── Generated/  DatasetForm.cpp — property-registry emitter output; never hand-edit
    │   └── Dialogs/  TransmitTestDialog
    ├── Console/     Terminal + export, Annotations.* (frame annotation layer, spec 0059)
    ├── Platform/    CSD, NativeWindow (true-size CSD windows: no painted shadow; Win10 gets
    │                a DWM-drawn shadow via WM_NCCALCSIZE filter, Linux the 1px border);
    │                FileOpenEventFilter (queued QFileOpenEvent .ssproj open,
    │                unit-test-linkable)
    ├── AI/  (Pro)   The in-app assistant: Assistant, Conversation, ToolDispatcher,
    │   │            CommandRegistry (safety tiers), FileSandbox, KeyVault, Redactor,
    │   │            SentinelProbe, SseEventReader — see architecture/ai.md
    │   ├── Conversation/  the turn's sub-objects: ToolTurnRunner, AsyncToolRunner,
    │   │            MetaToolCatalog, MetaToolRunner, TokenBudget, HistorySurgery,
    │   │            ReplyAssembly, AutoVerifier
    │   ├── Providers/  Provider/Reply base + Anthropic, OpenAI, Gemini, Local, and
    │   │            OpenAICompatibleProvider (one adapter, four vendor tables)
    │   └── Tools/   ToolCatalog/Schemas/Dispatch and the fs, script, tile and bulk tools
    ├── ApiHandlers/  Assistant/Console/Dashboard/Diagnostics/Extension/Problems/Window/
    │            Workspaces/Notifications API handlers + UiHandlers::registerAll(), the set
    │            the composition root registers after the core handlers (spec 0077 T62)
    └── Misc/        JsonValidator, ThemeManager, ContextRegistry (the one QML-globals
        │            table), ProblemCenter (pull-only problem aggregation, spec 0033),
        │            ConnectionDiagnostics (spec 0035) — NOT ModuleManager or CLI/, which
        │            stay in app/src/Misc/ (composition-root code)
        ├── Extensions/  ExtensionCatalog (catalog v2 + per-file digests), ExtensionInstaller
        │            (staged atomic install), PluginRunner
        ├── Problems/  Project/Script/Link/Extension checkers (synchronous, polled)
        └── Diagnostics/  Serial/Network/Bluetooth/Audio checks + DeviceAccess probes
```

## Message bus

`core/Core/Bus/` is the "virtual CAN bus" the libraries talk over instead of reaching a
sibling's singleton: `MessageBus` (subscribe/publish/publishState), `Subscription` (an RAII
handle that unsubscribes on destruction or when its receiver is destroyed), and `Messages.h`
(the shared vocabulary — plain structs of Core/Qt-Core types only, so speaking to the bus
needs nothing but `Core`). Topics are C++ types (`std::type_index`), never strings;
`publish<T>` allocates one `std::shared_ptr<const T>` and hands every subscriber the same
object; delivery is direct on the receiver's thread and queued across threads;
`publishState<T>` retains the latest message per type for `latest<T>()` to read by pointer.
Constructed by the composition root (`ModuleManager`), adopted as a `SessionContext` slot,
handed to every module constructor and carried in `Core::Services` (spec 0077 T73 removed the
transitional global accessor): singleton reaches migrated to bus topics and root-bound reference
sets one edge at a time (see the migration table in
`doc/claude/specs/0076-modular-core/plan.md` "Stage 2"), never to a second `instance()`.
Never on the per-frame path — `DataBlockReady`/structure fan-out stays on the pooled-block
path (spec 0055); the bus is command/state/notification rate, and `code-verify.py` keeps
`MessageBus` out of the hotpath allowlist files.

## `app/`

```
app/src/           Composition root only (spec 0076): everything else moved to core/.
├── main.cpp
├── SerialStudio.h/.cpp  Central enums (BusType, OperationMode, FrameDetection) +
│                    SerialStudioFrameSupport.cpp
├── AppState.h       Singleton: OperationMode, projectFilePath, FrameConfig
├── AppInfo.h
├── SessionContext.h  Session-scoped service context (spec 0039), owns the nine core modules
├── Misc/            ModuleManager (composition root) + CLI/ (CLI entry point, argument
│                    parsing) — the rest of Misc/ lives in core/Ui/Misc/
├── Licensing/       LemonSqueezy, Trial, MachineID (CommercialToken lives in core/Core/Licensing/)
├── SelfTest/        In-process --selftest suites (SS_INAPP_TESTS); run before the
│                    composition root — never touch a singleton
├── Benchmark/       --benchmark-hotpath driver
├── ThirdParty/       Vendored single-file libraries built directly into the executable
└── API/GRPC/        gRPC service glue (protoc custom command stays with the executable)
app/qml/             Commands/, DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/,
                     Dialogs/ (incl. ProblemCenter, RemoteAttach, ExtensionConsent)
app/rcc/             icons/ + commands/ (spec 0028), properties/dataset.json (spec 0036),
                     extensions/ (bundled widget-extension packages, spec 0038), ai/ corpus
app/tests/           C++ unit-test tier (spec 0032): Qt Test suites via ss_add_unit_test,
│                    SS_BUILD_TESTS=ON (no CMake presets; configure by hand); suites for a
│                    moved unit link SerialStudio::Core / SerialStudio::Protocols instead of
│                    recompiling the .cpp; partition-layer suites still recompile their
│                    `.cpp` lists (via the `core/<Layer>` include roots) rather than link the
│                    partition archive, to avoid dragging in the singleton closure
    ├── support/     shared doubles: FakeDriver, FakeProvider, FakeTransport
    └── fuzz/        libFuzzer entry points + corpus/, also runnable as ctest corpus replay
examples/            example projects; widget-extension/ = spec-0038 package template
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, luajit, hidapi, QSimpleUpdater,
                     open62541, mbedtls, libplctag; VERSIONS.json pins every vendored tree
```
