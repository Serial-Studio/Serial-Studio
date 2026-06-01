# Directory Map

```
app/src/
├── IO/              ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig
│   ├── Drivers/     UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
│   └── FileTransmission/  Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── DataModel/       Frame, FrameBuilder, FrameConsumer, DataTable(Store), ExportSchema,
│   │                ProjectModel, ProjectEditor, NotificationCenter
│   ├── Scripting/   IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │                LuaScriptEngine, LuaCompat, ScriptTemplates
│   ├── Editors/     JsCodeEditor, OutputCodeEditor, PainterCodeEditor,
│   │                DatasetTransformEditor, CodeFormatter
│   ├── Importers/   DBCImporter, ModbusMapImporter, ProtoImporter
│   └── Dialogs/     FrameParserTestDialog, TransmitTestDialog
├── UI/              Dashboard, Taskbar (workspaces), visualization + output widget types
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── API/             TCP server port 7777 (MCP + legacy JSON-RPC), 25 handlers
├── Console/         Terminal + export
├── CSV/ MDF4/       File playback & export
├── Sessions/  (Pro) DatabaseManager + SQLite::Export + SQLite::Player
├── MQTT/            Publisher (FrameConsumer-based, threaded, rate-limited 1-30 Hz)
├── Licensing/       LemonSqueezy, Trial, MachineID, CommercialToken (FeatureTier)
├── Platform/        CSD, NativeWindow
├── Misc/            JsonValidator, ThemeManager, ModuleManager
├── AppState.h       Singleton: OperationMode, projectFilePath, FrameConfig
├── SerialStudio.h   Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h       C++20 concepts
app/qml/             DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/, Dialogs/
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, lua54, QuaZip, hidapi, QSimpleUpdater
```
