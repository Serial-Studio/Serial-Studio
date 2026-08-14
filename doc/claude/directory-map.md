# Directory Map

```
app/src/
├── IO/              ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig,
│                    PipelineHost (frame-processing thread), StreamWorker (typed stream lane)
│   ├── Drivers/     UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, MQTT, Process, USB
│   └── FileTransmission/  Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── Async/           TaskTree, RetryPolicy, AsyncClock — task-tree engine used by MQTT::Publisher
│                    and the spec-0035 diagnostics probes; boundaries only, nothing per frame
├── DataModel/       Frame, FrameBuilder, FrameConsumer, DataTable(Store), ExportSchema,
│   │                ProjectModel, ProjectEditor, NotificationCenter, HotpathOptimization.h
│   ├── Project/     ProjectModel TU split (Crud, Folders, Loading, Persistence, Sources,
│   │                Tables, Workspaces + ProjectModelShared.h) and ProjectEditor TU split
│   │                (Commit, Forms, Mqtt, MultiSelect, Selection, Summaries, Tree, Wiring +
│   │                ProjectEditorItemIds.h, ProjectEditorShared.h); facades stay in DataModel/.
│   │                Plus ProjectHistory (undo/redo mementos, spec 0031) and
│   │                PropertyHooks/PropertyValidators (registry hooks, spec 0036)
│   ├── Generated/   DatasetForm, DatasetRegistry.h, DatasetSerialization — emitted by
│   │                generate-property-registry.py from app/rcc/properties/dataset.json;
│   │                never hand-edit
│   ├── Scripting/   IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │                LuaScriptEngine, LuaCompat, ScriptTemplates
│   ├── Editors/     JsCodeEditor, OutputCodeEditor, PainterCodeEditor,
│   │                DatasetTransformEditor, CodeFormatter
│   ├── Importers/   DBCImporter, ModbusMapImporter, ProtoImporter
│   └── Dialogs/     TransmitTestDialog
├── UI/              Dashboard, Taskbar (workspaces), visualization + output widget types,
│   │                WidgetExtensions + WidgetExtensionManifest (installable widgets, spec 0038)
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── API/             TCP server port 7777 (MCP + legacy JSON-RPC), 30+ handlers
│   ├── Handlers/    per-command handlers; ProjectHandler split into ProjectHandler{File,
│   │                Entities,Parser,Batch}.cpp + ProjectApiSupport.h (registration stays
│   │                in ProjectHandler.cpp); Diagnostics/Problems/Mirror handlers
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
├── Misc/            JsonValidator, ThemeManager, ModuleManager, ProblemCenter (pull-only
│   │                problem aggregation, spec 0033), ConnectionDiagnostics (spec 0035)
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
                     ctest preset dev/unit-ci, SS_BUILD_TESTS=ON
examples/            example projects; widget-extension/ = spec-0038 package template
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, lua54, QuaZip, hidapi, QSimpleUpdater
```
