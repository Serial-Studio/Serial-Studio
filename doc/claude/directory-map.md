# Directory Map

## Product Overview

Data sources: UART,
TCP/UDP/WebSocket/HTTP (one Network driver, spec 0068), BLE, Audio, Modbus, CAN Bus (Qt
plugins plus direct gs_usb/SLCAN/Seeed backends; J1939 TP + ISO-TP reassembly, spec 0073),
MQTT (Sparkplug B both directions, specs 0073/0074: host-application subscribe with project
generation, edge-node publish with stable aliases), OPC UA (embedded open62541 + mbedTLS,
six security policies None through Aes256_Sha256_RsaPss, spec 0067), Siemens S7comm
(S7-300/400/1200/1500, in-house stack), EtherNet/IP (libplctag), IEC 60870-5-104 (in-house
stack; all three PLC/telecontrol drivers are read-only clients, spec 0073), USB (libusb),
HID (hidapi), Process I/O.

Per-dataset value transforms in JS or Lua, with one shared library per
language per project and per-dataset `params` (spec 0083). Pro features: Output
widgets, Modbus, CAN Bus, OPC UA, S7comm, EtherNet/IP, IEC 60870-5-104, Sparkplug B, MDF4,
3D, ImageView, Waterfall, file-transfer protocols (X/Y/ZMODEM), Modbus map importer,
Historian (per-session SQLite recording), InfluxDB 2.x sink (`core/Storage/InfluxDB/`, line
protocol per block).

Aural alerts (spec 0087, every build): an ISA-18.1 annunciator (`UI::Alarms::AlarmAnnunciator`,
root-owned, `Cpp_UI_Alarms`, `alarms.*` API) with IEC 60601-1-8 WAV signatures played by
`IO::Audio::SoundPlayer` on a raw miniaudio device.

### Names, parsers and layers

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.2 + C++20, 15+ visualization
widgets, 5 output (control) widgets, 256 kHz+ data rate (CI-gated; see below). Data sources,
per-dataset transforms, the Pro feature list and the aural alerts live in
[directory-map.md](directory-map.md) "Product Overview".
Frame parsers in JavaScript (`QJSEngine`), Lua (embedded LuaJIT 2.1, 5.1 + shims; per-project
Safe/Fast execution mode — spec 0051), or Built-In ("Native"
in all internal identifiers — `SerialStudio::Native`, `CFrameParser`, `NativeTemplate`; only
user-facing strings/docs say Built-In. Parametrized C++ templates configured via a JSON
descriptor, no user code).
User-facing renames of 2026-08-19, internal identifiers unchanged: Historian (was Session
Database; `Sessions::`, `sessions.*`, the "Session Databases" folder stay), Variables (was Shared
Memory; `RegisterDef`/`registers` stay), Computed Dataset (was Virtual Dataset; `virtual_` and
`"Virtual"` stay), Canvas Widget (was Painter; `"painter"`, `project.painter.*`, Painter* stay).
In doc/help the hotpath is called the "acquisition pipeline" — code, benchmarks, and internal
docs keep "hotpath". The code base is seven static libraries under `core/`, every one a
STRICT layer since spec 0077 (`SerialStudioCore` and `SerialStudioProtocols` with prefixed
includes; `SerialStudioPipeline`, `SerialStudioDevices`, `SerialStudioStorage`, `SerialStudioApi`
and `SerialStudioUi` keeping their relative include roots) plus the composition root in
`app/src`; a library includes and links only the layers below it. Cross-library talk goes over
`Core::Bus::MessageBus` (typed topics, shared-pointer messages, retained state), through the
interfaces the root binds, or through the three root-bound module sets (`Core::services()`,
`DataModel::pipelineModules()`, `API::handlerContext()`) — never an `instance()` reach into
another library; `scripts/layer-verify.py` fails on any include, include root or link outside
the graph, and `code-verify.py --singleton-census` fails on any cross-library reach.

## `core/`

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
│   │                CircularBuffer, Checksum, JsonValidator.h, SerialStudio.h/.cpp (central
│   │                enums), AppInfo.h
│   ├── DataModel/   Frame, FrameKeys.h (namespace Keys), DataBlock, ExportSchema, FrameConsumer,
│   │                FrameSupport, IBlockSink (the sink seam), PropertyValidators (the colour
│   │                validators), Generated/DatasetSerialization.cpp (never hand-edit)
│   ├── IO/          FrameConfig, HAL_Driver, StreamConfig, LinkStats and the seam interfaces the
│   │                root binds: IIngestBinder, IRawByteTap, IRawFrameTap, IMqttPublisher,
│   │                IDeviceWriter, IPayloadInjector; FrameConfigBuilder derives a reader's
│   │                framing from the retained project snapshot
│   ├── Api/         CommandProtocol (the request/response value types) + ICommandExecutor,
│   │                the seam the scripting layer drives commands through (spec 0077 T60)
│   ├── Licensing/   CommercialToken (Pro) — the HMAC-sealed capability token every guard site
│   │                in the libraries above reads; QtCore and the generated guards header only
│   ├── Crypto/      CredentialVault, MachineKey (the root-published vault key), SimpleCrypt
│   ├── Deploy/      PackageCodec, PackageFormat, ResourceTree — the deployable package format
│   │                and its in-memory resource tree (spec 0096)
│   ├── Prompt/      IUserPrompter + UserPrompt — the user-prompt seam libraries below Ui speak
│   │                through (spec 0077)
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
│   ├── OpcUa/       OpcUaWire.h — the OPC UA delta-frame wire vocabulary (spec 0066)
│   ├── S7/          IsoTsap, S7Pdu, S7Address (Pro)
│   ├── Iec104/      Apci, Asdu (Pro)
│   ├── Sparkplug/   SparkplugPayload (Pro)
│   ├── Modbus/      ModbusRtuCodec (Pro)
│   ├── Tls/         TlsIdentity, TlsConfig (Pro) — the QSslConfiguration both MQTT ends build
│   └── FileTransfer/  CRC, Protocol base, XMODEM, YMODEM, ZMODEM (GPL base)
├── Pipeline/        (target SerialStudioPipeline, alias SerialStudio::Pipeline; links Core and
│   │                Protocols only) the frame/parse/project engine
│   ├── DataModel/   FrameBuilder, DataTable, ExportStructure, ReplayPlaybackEngine,
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
│   │   │            PropertyHooks (registry hooks, spec 0036)
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
│   ├── Misc/        PasswordHash (PBKDF2-SHA256)
│   ├── DSP.h DSPDownsample.h, DSP/FixedQueue.h (the ring container, spec 0097)
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
│   └── ThirdParty/  the vendored miniaudio TU (the Audio device's backend)
├── Storage/         (target SerialStudioStorage, alias SerialStudio::Storage)
│   ├── Replay/      LinkGate, PlayerState — the "is a replay player open / release the live link
│   │                first" predicates the players share
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
│       ├── Handlers/  per-command handlers; ProjectHandler is a facade owning 14 real
│       │            Project*Commands classes + the ProjectApiSupport namespace (spec 0070);
│       │            Mirror handler. ICheckpointStore is the pre-mutation snapshot seam; the
│       │            handlers that reach the UI live in Ui/ApiHandlers/ (spec 0077 T62)
│       ├── Server/  the API server's parts: ServerWorker (socket I/O thread), ServerAuth +
│       │            AuthPrimitives (token and one-time consents), ConsentGate, ClientReception
│       ├── Mirror/  MirrorProtocol/Session/Publisher/Client — remote dashboard mirroring
│       │            (spec 0040)
│       └── Generated/  DatasetApiFields.cpp — property-registry emitter output; never
│                    hand-edit
└── Ui/              (target SerialStudioUi, alias SerialStudio::Ui) dashboard, widgets, the
    │                AI assistant, platform chrome
    ├── UI/          Dashboard, Taskbar (workspaces), visualization + output widget types,
    │   │            WidgetExtensions (installable widgets, spec 0038), CommandStrings.cpp
    │   │            (generated, never hand-edit), SnapGuides
    │   ├── Automation/  the driving surface (spec 0101): ElementCatalog, InputSynthesizer,
    │   │            WindowCapture, WindowInventory
    │   ├── WindowManager/  WindowLayoutStore (manual-layout persistence), SnapOverlay (gesture
    │   │            guides), WindowGeometry
    │   ├── Dashboard/  DashboardIngest (the block-ingest sub-object + every push table),
    │   │            DashboardTools, PlotMemoryLedger (plot-history budget + report, spec 0097)
    │   ├── Alarms/    AlarmAnnunciator (root-owned facade, Cpp_UI_Alarms, alarms.* API) +
    │   │            AnnunciatorSequence (ISA-18.1 point table), SoundTheme, AppEventSounds
    │   │            (spec 0087)
    │   ├── Widgets/Waterfall/  ColorMap (+LUT), Overlay, Tiles, RingTexture (QRhi scanline
    │   │            uploads), SpectrogramNodes (both draw paths)
    │   ├── Widgets/  PlotBase (state Plot/MultiPlot/FFTPlot share, composed not inherited),
    │   │            GpuStroke (grow-only geometry + degenerate padding), SdfStroke +
    │   │            SdfStrokeMaterial (signed-distance plot stroke, spec 0097), WidgetBands.h
    │   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
    ├── ProjectEditor/  ProjectEditor (the QML facade: every Q_PROPERTY/Q_INVOKABLE/slot/signal
    │   │            plus the shared state) owning by-value sub-objects in the spec 0070
    │   │            shape, one class per .h/.cpp pair: EditorWiring, EditorSelection,
    │   │            EditorTree, EditorForms, EditorCommit, EditorSummaries, EditorMultiSelect,
    │   │            EditorMqtt, EditorInflux, EditorVocabulary (fills the combo lists the facade
    │   │            keeps); ProjectNavHistory, ProjectEditorIcons.h, ProjectEditorItemIds.h
    │   ├── EditorForms/  row builders split out of EditorForms.cpp: OutputStateRows,
    │   │            OutputValueRows, SourceFramingRows (frame detection, Connection Sequence)
    │   ├── Editors/  EmbeddedCodeEditorItem (the base six hosts derive from:
    │   │            ControlScriptEditor, JsCodeEditor, MacroEditor, OutputCodeEditor,
    │   │            PainterCodeEditor, TransformLibraryEditor) + PublisherScriptEditor (Pro) and
    │   │            DatasetTransformEditor (both QDialogs), FrameParserModel
    │   └── Generated/  DatasetForm.cpp — property-registry emitter output; never hand-edit
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
    └── Misc/        ThemeManager, ContextRegistry (the one QML-globals
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
app/src/           Composition root only (spec 0076): everything else moved to core/ (SerialStudio.h
│                  and AppInfo.h are in core/Core/, AppState.h in core/Pipeline/).
├── main.cpp
├── SessionContext.h/.cpp  Session-scoped service context (spec 0039), owns the nine core modules
│                    plus the message bus
├── Misc/            ModuleManager (composition root) + CLI/ (CLI entry point, argument
│                    parsing) — the rest of Misc/ lives in core/Ui/Misc/
├── Licensing/       LemonSqueezy, Trial, MachineID (CommercialToken lives in core/Core/Licensing/)
├── SelfTest/        In-process --selftest suites (SS_INAPP_TESTS); run before the
│                    composition root — never touch a singleton
├── Benchmark/       --benchmark-hotpath driver
├── ThirdParty/       Vendored single-file libraries built directly into the executable
└── API/GRPC/        gRPC service glue (protoc custom command stays with the executable)
app/qml/             AI/ (assistant panel, chat, key and memory dialogs), Commands/, DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/,
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
