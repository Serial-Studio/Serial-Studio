/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <QByteArrayView>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <utility>
#include <vector>

#include "Concepts.h"
#include "HotpathOptimization.h"

//--------------------------------------------------------------------------------------------------
// Standard keys for loading/offloading frame structures using JSON files
//--------------------------------------------------------------------------------------------------

namespace Keys {
/**
 * @brief Non-allocating view alias used for inline-constexpr JSON key constants.
 */
using KeyView = QLatin1StringView;

// Action keys
inline constexpr KeyView ActionId("actionId");
inline constexpr KeyView EOL("eol");
inline constexpr KeyView Icon("icon");
inline constexpr KeyView Title("title");
inline constexpr KeyView TxData("txData");
inline constexpr KeyView Binary("binary");
inline constexpr KeyView TxEncoding("txEncoding");
inline constexpr KeyView TimerMode("timerMode");
inline constexpr KeyView RepeatCount("repeatCount");
inline constexpr KeyView TimerInterval("timerIntervalMs");
inline constexpr KeyView AutoExecute("autoExecuteOnConnect");

// Source / connection keys
inline constexpr KeyView Sources("sources");
inline constexpr KeyView SourceId("sourceId");
inline constexpr KeyView SourceConn("connection");
inline constexpr KeyView DatasetSourceId("datasetSourceId");
inline constexpr KeyView BusType("busType");
inline constexpr KeyView FrameStart("frameStart");
inline constexpr KeyView FrameEnd("frameEnd");
inline constexpr KeyView Checksum("checksum");
inline constexpr KeyView ChecksumAlgorithm("checksumAlgorithm");
inline constexpr KeyView FrameDetection("frameDetection");
inline constexpr KeyView Decoder("decoder");
inline constexpr KeyView DecoderMethod("decoderMethod");
inline constexpr KeyView HexadecimalDelimiters("hexadecimalDelimiters");
inline constexpr KeyView FrameParserCode("frameParserCode");
inline constexpr KeyView FrameParserLanguage("frameParserLanguage");
inline constexpr KeyView FrameParserTemplate("frameParserTemplate");
inline constexpr KeyView FrameParserParams("frameParserParams");

// Dataset keys
inline constexpr KeyView FFT("fft");
inline constexpr KeyView LED("led");
inline constexpr KeyView Log("log");
inline constexpr KeyView Waterfall("waterfall");
inline constexpr KeyView WaterfallYAxis("waterfallYAxis");
inline constexpr KeyView Min("min");
inline constexpr KeyView Max("max");
inline constexpr KeyView Graph("graph");
inline constexpr KeyView Index("index");
inline constexpr KeyView XAxis("xAxis");
inline constexpr KeyView Alarm("alarm");
inline constexpr KeyView Units("units");
inline constexpr KeyView Value("value");
inline constexpr KeyView Widget("widget");
inline constexpr KeyView FFTMin("fftMin");
inline constexpr KeyView FFTMax("fftMax");
inline constexpr KeyView PltMin("plotMin");
inline constexpr KeyView PltMax("plotMax");
inline constexpr KeyView LedHigh("ledHigh");
inline constexpr KeyView WgtMin("widgetMin");
inline constexpr KeyView WgtMax("widgetMax");
inline constexpr KeyView AlarmLow("alarmLow");
inline constexpr KeyView AlarmHigh("alarmHigh");
inline constexpr KeyView DisplayTickCount("displayTickCount");
inline constexpr KeyView DisplayFormat("displayFormat");
inline constexpr KeyView DecimalPoints("decimalPoints");
inline constexpr KeyView FFTSamples("fftSamples");
inline constexpr KeyView Overview("overviewDisplay");
inline constexpr KeyView AlarmEnabled("alarmEnabled");
inline constexpr KeyView AlarmBands("alarmBands");
inline constexpr KeyView Color("color");
inline constexpr KeyView Label("label");
inline constexpr KeyView Blink("blink");
inline constexpr KeyView Severity("severity");
inline constexpr KeyView FFTSamplingRate("fftSamplingRate");
inline constexpr KeyView TransformCode("transformCode");
inline constexpr KeyView TransformLanguage("transformLanguage");
inline constexpr KeyView DatasetId("datasetId");
inline constexpr KeyView UniqueId("uniqueId");
inline constexpr KeyView NumericValue("numericValue");

// Frame container keys
inline constexpr KeyView Groups("groups");
inline constexpr KeyView Actions("actions");
inline constexpr KeyView Datasets("datasets");
inline constexpr KeyView OutputWidgets("outputWidgets");
inline constexpr KeyView ControlScriptCode("controlScriptCode");

// Output widget keys
inline constexpr KeyView OutputType("outputType");
inline constexpr KeyView OutputMinValue("outputMin");
inline constexpr KeyView OutputMaxValue("outputMax");
inline constexpr KeyView OutputStepSize("outputStep");
inline constexpr KeyView OutputInitialValue("initialValue");
inline constexpr KeyView OutputOnLabel("onLabel");
inline constexpr KeyView OutputOffLabel("offLabel");
inline constexpr KeyView OutputMonoIcon("monoIcon");
inline constexpr KeyView OutputColumns("outputColumns");
inline constexpr KeyView TransmitFunction("transmitFunction");
inline constexpr KeyView OutputTxEncoding("outputTxEncoding");

// Group keys
inline constexpr KeyView GroupId("groupId");
inline constexpr KeyView GroupType("groupType");
inline constexpr KeyView GroupFolders("groupFolders");

// Image-group keys
inline constexpr KeyView ImgMode("imgDetectionMode");
inline constexpr KeyView ImgStart("imgStartSequence");
inline constexpr KeyView ImgEnd("imgEndSequence");

// Painter-group keys
inline constexpr KeyView PainterCode("painterCode");
inline constexpr KeyView HideOnDashboard("hideOnDashboard");

// Web-view-group keys
inline constexpr KeyView WebViewUrl("webViewUrl");

// Dashboard layout keys
inline constexpr KeyView DashboardLayout("dashboardLayout");
inline constexpr KeyView ActiveGroupId("activeGroupId");
inline constexpr KeyView WidgetSettings("widgetSettings");

// Project-editor tree state (path-keyed node expansion map)
inline constexpr KeyView TreeExpansion("treeExpansion");

// Project-overview diagram state (stable-id keyed node collapse map)
inline constexpr KeyView DiagramCollapse("diagramCollapse");

// Plot history keys
inline constexpr KeyView PointCount("pointCount");
inline constexpr KeyView PlotTimeRange("plotTimeRange");

// Transform execution
inline constexpr KeyView ChangeDrivenTransforms("changeDrivenTransforms");
inline constexpr KeyView kActiveGroupSubKey("activeGroup");
inline constexpr KeyView kDashboardWindowsSubKey("dashboardWindows");
inline constexpr KeyView HiddenGroups("hiddenGroups");

// Workspace keys
inline constexpr KeyView Workspaces("workspaces");
inline constexpr KeyView WorkspaceId("workspaceId");
inline constexpr KeyView WidgetRefs("widgetRefs");
inline constexpr KeyView WidgetType("widgetType");
inline constexpr KeyView RelativeIndex("relativeIndex");
inline constexpr KeyView CustomizeWorkspaces("customizeWorkspaces");
inline constexpr KeyView WorkspaceDescription("description");
inline constexpr KeyView WorkspaceFolders("workspaceFolders");
inline constexpr KeyView FolderId("folderId");
inline constexpr KeyView ParentFolderId("parentFolderId");

inline constexpr KeyView Virtual("virtual");

// Group/dataset enablement: written only when off, so absence means enabled.
inline constexpr KeyView Disabled("disabled");

// Data table keys
inline constexpr KeyView Tables("tables");
inline constexpr KeyView TableFolders("tableFolders");
inline constexpr KeyView Registers("registers");
inline constexpr KeyView RegisterTypeName("type");
inline constexpr KeyView Name("name");

// Project metadata keys (file format / app version stamps)
inline constexpr KeyView SchemaVersion("schemaVersion");
inline constexpr KeyView WriterVersion("writerVersion");
inline constexpr KeyView WriterVersionAtCreation("writerVersionAtCreation");
inline constexpr KeyView NextUniqueId("nextUniqueId");

// Project lock: PBKDF2 hash (legacy MD5 still accepted on load).
inline constexpr KeyView PasswordHash("passwordHash");

// Per-project MQTT publisher configuration (Pro).
inline constexpr KeyView MqttPublisher("mqttPublisher");

// Full MCP command surface opt-in for user-script apiCall()
inline constexpr KeyView ApiCallAllowFullSurface("apiCallAllowFullSurface");

inline QString layoutKey(int groupId)
{
  return QStringLiteral("layout:") + QString::number(groupId);
}

inline QString layoutKey(const QString& scope, int groupId)
{
  if (scope.isEmpty())
    return layoutKey(groupId);

  return QStringLiteral("layout:") + scope + QStringLiteral(":") + QString::number(groupId);
}
}  // namespace Keys

namespace WorkspaceIds {
/**
 * @brief Reserved workspace ID slots and the auto/user range boundary.
 */
inline constexpr int AutoStart     = 1000;
inline constexpr int Overview      = 1000;
inline constexpr int AllData       = 1001;
inline constexpr int PerGroupStart = 1002;
inline constexpr int PerFolderStart =
  3000;  ///< Auto workspace synthesised from a leaf group folder
inline constexpr int UserStart = 5000;
}  // namespace WorkspaceIds

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Utility functions for data deserialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a value from a QJsonObject based on a key, returning a default value if the key does
 * not exist.
 */
[[nodiscard]] inline QVariant ss_jsr(const QJsonObject& object,
                                     QLatin1StringView key,
                                     const QVariant& defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

//--------------------------------------------------------------------------------------------------
// Action structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Timer mode for an Action.
 */
enum class TimerMode {
  Off,              ///< No timer
  AutoStart,        ///< Starts timer automatically (e.g. on connection)
  StartOnTrigger,   ///< Starts timer when the action is triggered
  ToggleOnTrigger,  ///< Toggles timer state with each trigger
  RepeatNTimes      ///< Sends command N times with configured interval between each
};

/**
 * @brief Represents a user-defined action or command to send over serial.
 */
struct alignas(8) Action {
  int actionId              = -1;               ///< Unique action ID
  int sourceId              = 0;                ///< Target source/device ID
  int repeatCount           = 3;                ///< Times to send in RepeatNTimes mode
  int timerIntervalMs       = 100;              ///< Timer interval in ms
  int txEncoding            = 0;                ///< Output encoding mode for txData
  TimerMode timerMode       = TimerMode::Off;   ///< Timer behavior mode
  bool binaryData           = false;            ///< If true, txData is binary
  bool autoExecuteOnConnect = false;            ///< Auto execute on connect
  QString icon              = "Play Property";  ///< Action icon name or path
  QString title;                                ///< Display title
  QString txData;                               ///< Data to transmit
  QString eolSequence;                          ///< End-of-line sequence (e.g. "\r\n")
};

static_assert(sizeof(Action) % alignof(Action) == 0, "Unaligned Action struct");

/**
 * @brief Generates the raw byte sequence to transmit for a given Action.
 */
QByteArray get_tx_bytes(const Action& action);

//--------------------------------------------------------------------------------------------------
// OutputWidget structure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Type of interactive output widget for bidirectional communication.
 */
enum class OutputWidgetType : quint8 {
  Button,
  Slider,
  Toggle,
  TextField,
  Knob,
};

/**
 * @brief Represents an interactive output widget that sends data to the device.
 */
struct alignas(8) OutputWidget {
  int widgetId          = -1;                        ///< Unique widget ID within the group
  int groupId           = -1;                        ///< Owning group ID
  int sourceId          = 0;                         ///< Target source/device ID
  int txEncoding        = 0;                         ///< Encoding used for transmit payload
  OutputWidgetType type = OutputWidgetType::Button;  ///< Widget presentation/behavior type
  bool monoIcon         = false;                     ///< Use monochrome icon styling
  double minValue       = 0;                         ///< Minimum allowed value
  double maxValue       = 100;                       ///< Maximum allowed value
  double stepSize       = 1;                         ///< Value increment step
  double initialValue   = 0;                         ///< Initial widget value
  QString icon;
  QString title;
  QString transmitFunction;
};

static_assert(sizeof(OutputWidget) % alignof(OutputWidget) == 0, "Unaligned OutputWidget struct");

/**
 * @brief Serializes an OutputWidget to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const OutputWidget& w)
{
  QJsonObject obj;
  obj.insert(Keys::Icon, w.icon);
  obj.insert(Keys::Title, w.title);
  obj.insert(Keys::SourceId, w.sourceId);
  obj.insert(Keys::OutputType, static_cast<int>(w.type));
  obj.insert(Keys::OutputMinValue, w.minValue);
  obj.insert(Keys::OutputMaxValue, w.maxValue);
  obj.insert(Keys::OutputStepSize, w.stepSize);
  obj.insert(Keys::OutputInitialValue, w.initialValue);
  if (w.monoIcon)
    obj.insert(Keys::OutputMonoIcon, true);

  obj.insert(Keys::TransmitFunction, w.transmitFunction);
  obj.insert(Keys::OutputTxEncoding, w.txEncoding);
  return obj;
}

/**
 * @brief Deserializes an OutputWidget from a QJsonObject (Frame.cpp: uses SerialStudio::toDouble).
 */
[[nodiscard]] bool read(OutputWidget& w, const QJsonObject& obj);

/**
 * @brief Severity tier for an alarm band; drives default colour and notification policy.
 */
enum class AlarmSeverity : quint8 {
  Info     = 0,  ///< Informational; never raises a notification
  Ok       = 1,  ///< Healthy / operating range; never raises a notification
  Warning  = 2,  ///< Out-of-normal; raises a Warning notification on entry
  Critical = 3   ///< Out-of-safe; raises a Critical notification on entry
};

/**
 * @brief Coloured value band attached to a Dataset for alarm visualisation.
 */
struct alignas(8) AlarmBand {
  double min             = 0;                       ///< Lower bound (inclusive)
  double max             = 0;                       ///< Upper bound (exclusive at top of range)
  AlarmSeverity severity = AlarmSeverity::Warning;  ///< Default colour + notification policy
  bool blink             = false;  ///< Flash the indicator while the band is active
  QString color;                   ///< Optional hex override; empty -> severity default
  QString label;                   ///< Optional human label (used in notifications)
};

static_assert(sizeof(AlarmBand) % alignof(AlarmBand) == 0, "Unaligned AlarmBand struct");

inline constexpr int kXAxisSamples = -1;
inline constexpr int kXAxisTime    = -2;

/**
 * @brief Represents a single unit of sensor data with optional metadata and
 * graphing. Fully aligned and stack-optimized.
 */
struct alignas(8) Dataset {
  int index             = 0;           ///< Frame offset index
  int xAxisId           = kXAxisTime;  ///< X-axis: -2 = time (default), else dataset uniqueId
  int waterfallYAxis    = 0;      ///< Y source for the waterfall -- 0 = Time, else dataset uniqueId
  int groupId           = 0;      ///< Owning group ID
  int sourceId          = 0;      ///< Source this dataset belongs to
  int uniqueId          = -1;     ///< Persisted stable identity (-1 = unassigned, compute legacy)
  int datasetId         = 0;      ///< Unique ID within group
  int fftSamples        = 256;    ///< Number of samples for FFT
  int fftSamplingRate   = 100;    ///< Sampling rate for FFT
  int transformLanguage = -1;     ///< Transform script language (-1 = unset, 0 = JS, 1 = Lua)
  bool fft              = false;  ///< Enables FFT processing
  bool led              = false;  ///< Enables LED widget
  bool log              = false;  ///< Enables logging
  bool plt              = false;  ///< Enables plotting
  bool waterfall        = false;  ///< Enables waterfall (spectrogram) widget -- Pro
  bool overviewDisplay  = false;  ///< Show in overview
  bool isNumeric        = false;  ///< True if value was parsed as numeric
  bool virtual_         = false;  ///< True if dataset is generated rather than parsed directly
  bool hideOnDashboard  = false;  ///< Suppress dataset-level dashboard tile (painter still sees it)
  bool enabled          = true;   ///< False excludes the dataset from frame building (editor-only)
  double fftMin         = 0;      ///< Minimum value (for FFT)
  double fftMax         = 0;      ///< Maximum value (for FFT)
  double pltMin         = 0;      ///< Minimum value (for plots)
  double pltMax         = 0;      ///< Maximum value (for plots)
  double wgtMin         = 0;      ///< Minimum value (for widgets)
  double wgtMax         = 0;      ///< Maximum value (for widgets)
  double ledHigh        = 80;     ///< LED activation threshold
  double numericValue   = 0;      ///< Parsed numeric value after transforms
  double rawNumericValue = 0;     ///< Parsed numeric value before transforms
  int displayTickCount   = 5;     ///< Preferred major-tick count on analog widgets (0 = auto)
  int decimalPoints = -1;  ///< Fixed value-display decimals; overrides displayFormat (-1 = auto)
  QString value;           ///< Raw string value after transforms
  QString rawValue;        ///< Raw string value before transforms
  QString title;           ///< Human-readable title
  QString units;           ///< Measurement units (e.g., degC)
  QString widget;          ///< Widget type (bar, gauge, etc.)
  QString transformCode;   ///< Optional per-dataset transform script
  QString displayFormat =
    QStringLiteral("0d");  ///< Tick/value label format on analog widgets ("0d" = integer)
  std::vector<AlarmBand> alarmBands;  ///< Colour-banded alarm zones (empty = no alarms)
};

static_assert(sizeof(Dataset) % alignof(Dataset) == 0, "Unaligned Dataset struct");

/**
 * @brief Distinguishes input (visualization) groups from output (control) groups.
 */
enum class GroupType : quint8 {
  Input  = 0,  ///< Visualization group (datasets, plots, etc.)
  Output = 1,  ///< Control group (output widgets only)
};

/**
 * @brief Represents a collection of datasets that are related (e.g., for a
 * specific sensor).
 */
struct alignas(8) Group {
  int groupId         = -1;  ///< Positional id within the project (changes on reorder)
  int uniqueId        = -1;  ///< Persisted stable identity (-1 = unassigned)
  int parentFolderId  = -1;  ///< Editor folder this group is filed under (-1 = top level)
  int sourceId        = 0;   ///< Source this group reads from (0 = default)
  int columns         = 2;   ///< Number of columns for output panel grid layout
  GroupType groupType = GroupType::Input;   ///< Input (visualization) or Output (controls)
  bool enabled        = true;               ///< False excludes the group from frame building
  QString title;                            ///< Group display name
  QString widget;                           ///< Group widget type
  std::vector<Dataset> datasets;            ///< Datasets contained in this group
  std::vector<OutputWidget> outputWidgets;  ///< Interactive output widgets
  QString imgDetectionMode;                 ///< "autodetect" | "manual" (default: "autodetect")
  QString imgStartSequence;                 ///< Hex start delimiter (manual mode only)
  QString imgEndSequence;                   ///< Hex end delimiter (manual mode only)
  QString painterCode;                      ///< User JS for painter widget (paint/onFrame)
  QString webViewUrl;                       ///< URL loaded by the web-view widget
};

static_assert(sizeof(Group) % alignof(Group) == 0, "Unaligned Group struct");

/**
 * @brief Describes a single data source (device + connection settings) within a project. Each
 * source owns its own frame detection configuration and opaque connection settings.
 */
struct alignas(8) Source {
  int sourceId = 0;                    ///< Unique source identifier (0 = default/backward-compat)
  int busType  = 0;                    ///< SerialStudio::BusType cast to int
  QString title;                       ///< Human-readable source name
  QString frameStart;                  ///< Frame start delimiter sequence
  QString frameEnd;                    ///< Frame end delimiter sequence
  QString checksumAlgorithm;           ///< Checksum algorithm name
  int frameDetection         = 0;      ///< SerialStudio::FrameDetection cast to int
  int decoderMethod          = 0;      ///< SerialStudio::DecoderMethod cast to int
  bool hexadecimalDelimiters = false;  ///< True if delimiters are hex-encoded
  int frameParserLanguage    = 0;      ///< Frame parser language/runtime identifier
  QJsonObject connectionSettings;      ///< Opaque bus-specific connection params
  QString frameParserCode;             ///< Per-source parser code
  QString frameParserTemplate;         ///< Native parser template id (Native language only)
  QJsonObject frameParserParams;       ///< Native parser template params (Native language only)
};

static_assert(sizeof(Source) % alignof(Source) == 0, "Unaligned Source struct");

/**
 * @brief Register storage mode inside a user-defined data table.
 */
enum class RegisterType : quint8 {
  Constant = 0,
  Computed = 1,
  System   = 2,
};

/**
 * @brief Represents a single register definition inside a user-defined table.
 */
struct RegisterDef {
  QString name;
  RegisterType type = RegisterType::Constant;
  QVariant defaultValue;
};

/**
 * @brief Represents a user-defined data table.
 */
struct TableDef {
  QString name;
  int parentFolderId = -1;  ///< Editor folder this table is filed under (-1 = top level)
  std::vector<RegisterDef> registers;
};

/**
 * @brief Serializes a Source to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Source& s)
{
  QJsonObject obj;
  obj.insert(Keys::SourceId, s.sourceId);
  obj.insert(Keys::Title, s.title.simplified());
  obj.insert(Keys::BusType, s.busType);
  obj.insert(Keys::FrameStart, s.frameStart);
  obj.insert(Keys::FrameEnd, s.frameEnd);

  obj.insert(Keys::Checksum, s.checksumAlgorithm);
  obj.insert(Keys::ChecksumAlgorithm, s.checksumAlgorithm);
  obj.insert(Keys::FrameDetection, s.frameDetection);
  obj.insert(Keys::Decoder, s.decoderMethod);
  obj.insert(Keys::DecoderMethod, s.decoderMethod);

  obj.insert(Keys::HexadecimalDelimiters, s.hexadecimalDelimiters);
  if (!s.connectionSettings.isEmpty())
    obj.insert(Keys::SourceConn, s.connectionSettings);

  if (!s.frameParserCode.isEmpty())
    obj.insert(Keys::FrameParserCode, s.frameParserCode);

  if (s.frameParserLanguage != 0)
    obj.insert(Keys::FrameParserLanguage, s.frameParserLanguage);

  if (!s.frameParserTemplate.isEmpty()) {
    obj.insert(Keys::FrameParserTemplate, s.frameParserTemplate);
    if (!s.frameParserParams.isEmpty())
      obj.insert(Keys::FrameParserParams, s.frameParserParams);
  }

  return obj;
}

/**
 * @brief Serializes a RegisterDef to a QJsonObject (Frame.cpp: uses SerialStudio::toDouble).
 */
[[nodiscard]] QJsonObject serialize(const RegisterDef& r);

/**
 * @brief Deserializes a RegisterDef from a QJsonObject (Frame.cpp: uses SerialStudio::toDouble).
 */
[[nodiscard]] bool read(RegisterDef& r, const QJsonObject& obj);

/**
 * @brief Serializes a TableDef to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const TableDef& t)
{
  QJsonArray regs;
  for (const auto& r : t.registers)
    regs.append(serialize(r));

  QJsonObject obj;
  obj.insert(Keys::Name, t.name);
  if (t.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, t.parentFolderId);

  obj.insert(Keys::Registers, regs);
  return obj;
}

/**
 * @brief Deserializes a TableDef from a QJsonObject.
 */
[[nodiscard]] inline bool read(TableDef& t, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  t.name = ss_jsr(obj, Keys::Name, "").toString().simplified();
  if (t.name.isEmpty())
    return false;

  t.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();

  t.registers.clear();
  const auto regs = obj.value(Keys::Registers).toArray();
  t.registers.reserve(regs.count());
  for (const auto& val : regs) {
    RegisterDef r;
    if (read(r, val.toObject()))
      t.registers.push_back(r);
  }

  return true;
}

/**
 * @brief Reference to a specific widget in the dashboard, stable across group reorders.
 */
struct WidgetRef {
  int widgetType    = 0;
  int groupUniqueId = -1;  ///< Group.uniqueId (stable identity, not the positional groupId)
  int relativeIndex = -1;
};

/**
 * @brief Represents a user-defined dashboard workspace.
 */
struct Workspace {
  int workspaceId    = -1;
  int parentFolderId = -1;
  QString title;
  QString icon;
  QString description;
  std::vector<WidgetRef> widgetRefs;
};

/**
 * @brief Serializes a Workspace to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Workspace& w)
{
  QJsonObject obj;
  obj.insert(Keys::WorkspaceId, w.workspaceId);
  obj.insert(Keys::Title, w.title.simplified());
  if (!w.icon.isEmpty())
    obj.insert(Keys::Icon, w.icon);

  if (!w.description.isEmpty())
    obj.insert(Keys::WorkspaceDescription, w.description);

  if (w.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, w.parentFolderId);

  QJsonArray refs;
  for (const auto& ref : w.widgetRefs) {
    QJsonObject r;
    r.insert(Keys::WidgetType, ref.widgetType);
    r.insert(Keys::GroupId, ref.groupUniqueId);
    r.insert(Keys::RelativeIndex, ref.relativeIndex);
    refs.append(r);
  }

  obj.insert(Keys::WidgetRefs, refs);
  return obj;
}

/**
 * @brief Deserializes a Workspace from a QJsonObject.
 */
[[nodiscard]] inline bool read(Workspace& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.workspaceId    = ss_jsr(obj, Keys::WorkspaceId, -1).toInt();
  w.title          = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.icon           = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.description    = ss_jsr(obj, Keys::WorkspaceDescription, "").toString();
  w.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();

  w.widgetRefs.clear();
  const auto refs = obj.value(Keys::WidgetRefs).toArray();
  for (const auto& val : refs) {
    const auto r = val.toObject();
    WidgetRef ref;
    ref.widgetType    = ss_jsr(r, Keys::WidgetType, 0).toInt();
    ref.groupUniqueId = ss_jsr(r, Keys::GroupId, -1).toInt();
    ref.relativeIndex = ss_jsr(r, Keys::RelativeIndex, -1).toInt();
    w.widgetRefs.push_back(ref);
  }

  return w.workspaceId >= 0 && !w.title.isEmpty();
}

/**
 * @brief Represents a folder that groups workspaces in the project editor.
 */
struct WorkspaceFolder {
  int folderId       = -1;
  int parentFolderId = -1;
  QString title;
};

/**
 * @brief Serializes a WorkspaceFolder to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const WorkspaceFolder& f)
{
  QJsonObject obj;
  obj.insert(Keys::FolderId, f.folderId);
  obj.insert(Keys::Title, f.title.simplified());
  if (f.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, f.parentFolderId);

  return obj;
}

/**
 * @brief Deserializes a WorkspaceFolder from a QJsonObject.
 */
[[nodiscard]] inline bool read(WorkspaceFolder& f, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  f.folderId       = ss_jsr(obj, Keys::FolderId, -1).toInt();
  f.title          = ss_jsr(obj, Keys::Title, "").toString().simplified();
  f.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();
  return f.folderId >= 0 && !f.title.isEmpty();
}

/**
 * @brief Represents a folder that organizes groups in the project editor.
 */
struct GroupFolder {
  int folderId       = -1;
  int parentFolderId = -1;
  QString title;
};

/**
 * @brief Serializes a GroupFolder to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const GroupFolder& f)
{
  QJsonObject obj;
  obj.insert(Keys::FolderId, f.folderId);
  obj.insert(Keys::Title, f.title.simplified());
  if (f.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, f.parentFolderId);

  return obj;
}

/**
 * @brief Deserializes a GroupFolder from a QJsonObject.
 */
[[nodiscard]] inline bool read(GroupFolder& f, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  f.folderId       = ss_jsr(obj, Keys::FolderId, -1).toInt();
  f.title          = ss_jsr(obj, Keys::Title, "").toString().simplified();
  f.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();
  return f.folderId >= 0 && !f.title.isEmpty();
}

/**
 * @brief Represents a folder that organizes user-defined tables in the project editor.
 */
struct TableFolder {
  int folderId       = -1;
  int parentFolderId = -1;
  QString title;
};

/**
 * @brief Serializes a TableFolder to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const TableFolder& f)
{
  QJsonObject obj;
  obj.insert(Keys::FolderId, f.folderId);
  obj.insert(Keys::Title, f.title.simplified());
  if (f.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, f.parentFolderId);

  return obj;
}

/**
 * @brief Deserializes a TableFolder from a QJsonObject.
 */
[[nodiscard]] inline bool read(TableFolder& f, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  f.folderId       = ss_jsr(obj, Keys::FolderId, -1).toInt();
  f.title          = ss_jsr(obj, Keys::Title, "").toString().simplified();
  f.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();
  return f.folderId >= 0 && !f.title.isEmpty();
}

/**
 * @brief Builds the "/"-joined editor folder path for a table-folder chain (root -> leaf);
 *        empty for a top-level table (parentFolderId == -1).
 */
[[nodiscard]] QString tableFolderPath(const std::vector<TableFolder>& folders, int parentFolderId);

/**
 * @brief Builds a table's full accessor/store path: the folder path joined with the leaf name.
 */
[[nodiscard]] QString tableFullPath(const std::vector<TableFolder>& folders,
                                    int parentFolderId,
                                    const QString& name);

/**
 * @brief Current Serial Studio project schema version.
 */
inline constexpr int kSchemaVersion = 3;

/**
 * @brief Represents a full data frame, including groups and actions.
 *        This is the root structure for each UI update.
 */
struct alignas(8) Frame {
  int sourceId      = 0;                    ///< Source that produced this frame
  int schemaVersion = 0;                    ///< File-format schema version (0 = unstamped/live)
  QString title;                            ///< Frame title
  QString writerVersion;                    ///< App version that last wrote this file
  QString writerVersionAtCreation;          ///< App version that first created this file
  std::vector<Group> groups;                ///< Sensor groups in this frame
  std::vector<Action> actions;              ///< Triggerable actions
  std::vector<Source> sources;              ///< Sources/devices the frame was assembled from
  bool containsCommercialFeatures = false;  ///< Feature gating flag
  QString controlScriptCode;                ///< Project setup()/loop() control script
};

static_assert(sizeof(Frame) % alignof(Frame) == 0, "Unaligned Frame struct");

/**
 * @brief Returns the current application version string for project stamps.
 */
[[nodiscard]] QString current_writer_version();

/**
 * @brief Clears and resets a Frame object to its default state.
 */
inline void clear_frame(Frame& frame) noexcept
{
  frame.title.clear();
  frame.writerVersion.clear();
  frame.writerVersionAtCreation.clear();
  frame.groups.clear();
  frame.actions.clear();
  frame.sources.clear();
  frame.groups.shrink_to_fit();
  frame.actions.shrink_to_fit();
  frame.sources.shrink_to_fit();
  frame.containsCommercialFeatures = false;
  frame.schemaVersion              = 0;
  frame.controlScriptCode.clear();
}

/**
 * @brief Allocation-free QString copy: reuses the destination buffer when it is unique and
 *        large enough, falling back to an implicit-share assignment otherwise.
 */
SS_FORCE_INLINE void assign_string_in_place(QString& dst, const QString& src) noexcept
{
  const qsizetype n = src.size();
  if (dst.isDetached() && dst.capacity() >= n) {
    dst.resize(n);
    if (n > 0)
      memcpy(dst.data(), src.constData(), static_cast<size_t>(n) * sizeof(QChar));
  } else
    dst = src;
}

/**
 * @brief Writes UTF-8 bytes into a QString, reusing the destination buffer for ASCII payloads. The
 *        widen runs branchless (OR-accumulating the high bit instead of an in-loop early return) so
 *        it auto-vectorizes; a single non-ASCII byte then redoes the rare frame via
 * QString::fromUtf8.
 */
SS_FORCE_INLINE void assign_utf8_in_place(QString& dst, QByteArrayView src)
{
  const char* p     = src.data();
  const qsizetype n = src.size();

  Q_ASSERT(n >= 0);

  if (!(dst.isDetached() && dst.capacity() >= n)) {
    dst = QString();
    dst.reserve(qMax<qsizetype>(n, 8));
  }

  dst.resize(n);
  QChar* out = dst.data();

  uchar nonAscii = 0;
  for (qsizetype i = 0; i < n; ++i) {
    const uchar c  = static_cast<uchar>(p[i]);
    nonAscii      |= c;
    out[i]         = QChar(static_cast<char16_t>(c));
  }

  if (nonAscii & 0x80) [[unlikely]]
    dst = QString::fromUtf8(p, n);
}

/**
 * @brief Copies per-frame volatile state (source id + dataset values) into a structure-matched dst.
 */
inline void copy_frame_values(Frame& dst, const Frame& src) noexcept
{
  dst.sourceId = src.sourceId;

  const size_t groupCount = src.groups.size();
  for (size_t g = 0; g < groupCount; ++g) {
    const auto& srcGroup      = src.groups[g];
    auto& dstGroup            = dst.groups[g];
    const size_t datasetCount = srcGroup.datasets.size();
    SS_NO_UNROLL
    for (size_t d = 0; d < datasetCount; ++d) {
      const auto& srcDataset = srcGroup.datasets[d];
      auto& dstDataset       = dstGroup.datasets[d];

      assign_string_in_place(dstDataset.value, srcDataset.value);
      assign_string_in_place(dstDataset.rawValue, srcDataset.rawValue);
      dstDataset.numericValue    = srcDataset.numericValue;
      dstDataset.isNumeric       = srcDataset.isNumeric;
      dstDataset.rawNumericValue = srcDataset.rawNumericValue;
    }
  }
}

/**
 * @brief Compares two frames for structural equivalence.
 */
[[nodiscard]] inline bool compare_frames(const Frame& a, const Frame& b) noexcept
{
  if (a.groups.size() != b.groups.size())
    return false;

  const auto& groupsA = a.groups;
  const auto& groupsB = b.groups;

  for (size_t i = 0, gc = groupsA.size(); i < gc; ++i) {
    const auto& g1 = groupsA[i];
    const auto& g2 = groupsB[i];

    if (g1.groupId != g2.groupId) [[unlikely]]
      return false;

    const auto& datasetsA = g1.datasets;
    const auto& datasetsB = g2.datasets;

    const size_t dc = datasetsA.size();
    if (dc != datasetsB.size()) [[unlikely]]
      return false;

    for (size_t j = 0; j < dc; ++j)
      if (datasetsA[j].index != datasetsB[j].index) [[unlikely]]
        return false;
  }

  return true;
}

/**
 * @brief Stride applied to a dataset's source id when computing its uniqueId.
 */
inline constexpr int kSourceIdStride = 1000000;

/**
 * @brief Stride applied to a dataset's group id when computing its uniqueId.
 */
inline constexpr int kGroupIdStride = 10000;

/**
 * @brief High-offset base for uniqueIds assigned to runtime-synthesised groups
 *        (QuickPlot, Dashboard's built-in Terminal / Clock / Stopwatch / LED panel).
 *        Sits well above any value `dataset_unique_id()` can produce and well above
 *        any plausible value of ProjectModel's nextUniqueId counter.
 */
inline constexpr int kRuntimeGroupUidBase = 0x40000000;

/**
 * @brief Deterministic uniqueId for a runtime-synthesised group.
 */
[[nodiscard]] inline constexpr int runtime_group_unique_id(int groupId) noexcept
{
  return kRuntimeGroupUidBase + groupId;
}

/**
 * @brief Single source of truth for the (source, group, dataset) -> uniqueId mapping.
 */
[[nodiscard]] inline constexpr int dataset_unique_id(int sourceId,
                                                     int groupId,
                                                     int datasetId) noexcept
{
  return sourceId * kSourceIdStride + groupId * kGroupIdStride + datasetId;
}

/**
 * @brief Convenience overload taking a Group + Dataset directly.
 */
[[nodiscard]] inline constexpr int dataset_unique_id(const Group& g, const Dataset& d) noexcept
{
  return dataset_unique_id(g.sourceId, g.groupId, d.datasetId);
}

/**
 * @brief Finalizes a Frame after deserialization.
 */
void finalize_frame(Frame& frame);

/**
 * @brief Reads and parses I/O frame settings from a JSON object.
 */
void read_io_settings(QByteArray& frameStart,
                      QByteArray& frameEnd,
                      QString& checksum,
                      const QJsonObject& obj);

//--------------------------------------------------------------------------------------------------
// Data -> JSON serialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes an Action to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Action& a)
{
  QJsonObject obj;
  obj.insert(Keys::Icon, a.icon);
  obj.insert(Keys::Title, a.title);
  obj.insert(Keys::TxData, a.txData);
  obj.insert(Keys::EOL, a.eolSequence);
  obj.insert(Keys::Binary, a.binaryData);
  obj.insert(Keys::SourceId, a.sourceId);
  obj.insert(Keys::TxEncoding, a.txEncoding);
  obj.insert(Keys::RepeatCount, a.repeatCount);
  obj.insert(Keys::TimerInterval, a.timerIntervalMs);
  obj.insert(Keys::AutoExecute, a.autoExecuteOnConnect);
  obj.insert(Keys::TimerMode, static_cast<int>(a.timerMode));
  return obj;
}

/**
 * @brief Serializes an AlarmBand to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const AlarmBand& b)
{
  QJsonObject obj;
  obj.insert(Keys::Min, qMin(b.min, b.max));
  obj.insert(Keys::Max, qMax(b.min, b.max));
  obj.insert(Keys::Severity, static_cast<int>(b.severity));
  if (b.blink)
    obj.insert(Keys::Blink, b.blink);

  if (!b.color.isEmpty())
    obj.insert(Keys::Color, b.color);

  if (!b.label.isEmpty())
    obj.insert(Keys::Label, b.label);

  return obj;
}

/**
 * @brief Serializes a Dataset to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Dataset& d)
{
  QJsonObject obj;
  obj.insert(Keys::FFT, d.fft);
  obj.insert(Keys::LED, d.led);
  obj.insert(Keys::Log, d.log);
  obj.insert(Keys::Graph, d.plt);
  if (d.waterfall)
    obj.insert(Keys::Waterfall, true);

  if (d.waterfallYAxis != 0)
    obj.insert(Keys::WaterfallYAxis, d.waterfallYAxis);

  obj.insert(Keys::Index, d.index);
  obj.insert(Keys::XAxis, d.xAxisId);
  obj.insert(Keys::LedHigh, d.ledHigh);
  obj.insert(Keys::FFTSamples, d.fftSamples);
  obj.insert(Keys::Title, d.title.simplified());
  obj.insert(Keys::Value, d.value.simplified());
  obj.insert(Keys::Units, d.units.simplified());
  obj.insert(Keys::Widget, d.widget.simplified());
  obj.insert(Keys::FFTMin, qMin(d.fftMin, d.fftMax));
  obj.insert(Keys::FFTMax, qMax(d.fftMin, d.fftMax));
  obj.insert(Keys::PltMin, qMin(d.pltMin, d.pltMax));
  obj.insert(Keys::PltMax, qMax(d.pltMin, d.pltMax));
  obj.insert(Keys::WgtMin, qMin(d.wgtMin, d.wgtMax));
  obj.insert(Keys::WgtMax, qMax(d.wgtMin, d.wgtMax));
  obj.insert(Keys::FFTSamplingRate, d.fftSamplingRate);

  if (!d.alarmBands.empty()) {
    QJsonArray bands;
    for (const auto& band : d.alarmBands)
      bands.append(serialize(band));

    obj.insert(Keys::AlarmBands, bands);
  }

  if (d.displayTickCount > 0)
    obj.insert(Keys::DisplayTickCount, d.displayTickCount);

  if (!d.displayFormat.isEmpty())
    obj.insert(Keys::DisplayFormat, d.displayFormat);

  if (d.decimalPoints >= 0)
    obj.insert(Keys::DecimalPoints, d.decimalPoints);

  obj.insert(Keys::GroupId, d.groupId);
  obj.insert(Keys::DatasetId, d.datasetId);
  if (d.uniqueId >= 0)
    obj.insert(Keys::UniqueId, d.uniqueId);

  obj.insert(Keys::NumericValue, d.numericValue);

  if (!d.transformCode.isEmpty()) {
    obj.insert(Keys::TransformCode, d.transformCode);
    obj.insert(Keys::TransformLanguage, d.transformLanguage);
  }

  if (d.virtual_)
    obj.insert(Keys::Virtual, true);

  if (d.hideOnDashboard)
    obj.insert(Keys::HideOnDashboard, true);

  if (!d.enabled)
    obj.insert(Keys::Disabled, true);

  return obj;
}

/**
 * @brief Serializes a Group to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Group& g)
{
  QJsonArray datasetArray;
  for (const auto& dataset : g.datasets)
    datasetArray.append(serialize(dataset));

  QJsonObject obj;
  obj.insert(Keys::Datasets, datasetArray);
  obj.insert(Keys::Title, g.title.simplified());
  obj.insert(Keys::Widget, g.widget.simplified());

  if (g.uniqueId >= 0)
    obj.insert(Keys::UniqueId, g.uniqueId);

  if (g.parentFolderId != -1)
    obj.insert(Keys::ParentFolderId, g.parentFolderId);

  if (g.groupType != GroupType::Input)
    obj.insert(Keys::GroupType, static_cast<int>(g.groupType));

  if (g.columns != 2)
    obj.insert(Keys::OutputColumns, g.columns);

  if (g.sourceId != 0)
    obj.insert(Keys::SourceId, g.sourceId);

  if (!g.enabled)
    obj.insert(Keys::Disabled, true);

  if (g.widget.simplified() == QLatin1String("image")) {
    obj.insert(Keys::ImgMode, g.imgDetectionMode);
    obj.insert(Keys::ImgStart, g.imgStartSequence);
    obj.insert(Keys::ImgEnd, g.imgEndSequence);
  }

  if (g.widget.simplified() == QLatin1String("painter") && !g.painterCode.isEmpty())
    obj.insert(Keys::PainterCode, g.painterCode);

  if (g.widget.simplified() == QLatin1String("webview") && !g.webViewUrl.isEmpty())
    obj.insert(Keys::WebViewUrl, g.webViewUrl);

  if (!g.outputWidgets.empty()) {
    QJsonArray owArray;
    for (const auto& ow : g.outputWidgets)
      owArray.append(serialize(ow));

    obj.insert(Keys::OutputWidgets, owArray);
  }

  return obj;
}

/**
 * @brief Serializes a Frame to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const Frame& f)
{
  QJsonArray groupArray;
  QJsonArray actionArray;

  for (const auto& group : f.groups)
    groupArray.append(serialize(group));

  for (const auto& action : f.actions)
    actionArray.append(serialize(action));

  QJsonObject obj;
  obj.insert(Keys::Title, f.title);
  obj.insert(Keys::Groups, groupArray);
  obj.insert(Keys::Actions, actionArray);

  if (!f.controlScriptCode.isEmpty())
    obj.insert(Keys::ControlScriptCode, f.controlScriptCode);

  if (!f.writerVersion.isEmpty() || !f.writerVersionAtCreation.isEmpty() || f.schemaVersion > 0) {
    obj.insert(Keys::SchemaVersion, f.schemaVersion > 0 ? f.schemaVersion : kSchemaVersion);
    if (!f.writerVersion.isEmpty())
      obj.insert(Keys::WriterVersion, f.writerVersion);

    if (!f.writerVersionAtCreation.isEmpty())
      obj.insert(Keys::WriterVersionAtCreation, f.writerVersionAtCreation);
  }

  return obj;
}

//--------------------------------------------------------------------------------------------------
// Data deserialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deserializes a Source from a QJsonObject.
 */
[[nodiscard]] inline bool read(Source& s, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  s.sourceId              = ss_jsr(obj, Keys::SourceId, 0).toInt();
  s.title                 = ss_jsr(obj, Keys::Title, "").toString().simplified();
  s.busType               = ss_jsr(obj, Keys::BusType, 0).toInt();
  s.frameStart            = ss_jsr(obj, Keys::FrameStart, "").toString();
  s.frameEnd              = ss_jsr(obj, Keys::FrameEnd, "\\n").toString();
  s.frameDetection        = ss_jsr(obj, Keys::FrameDetection, 0).toInt();
  s.hexadecimalDelimiters = ss_jsr(obj, Keys::HexadecimalDelimiters, false).toBool();
  s.connectionSettings    = ss_jsr(obj, Keys::SourceConn, QJsonObject()).toJsonObject();
  s.frameParserCode       = ss_jsr(obj, Keys::FrameParserCode, "").toString();
  s.frameParserLanguage   = ss_jsr(obj, Keys::FrameParserLanguage, 0).toInt();
  s.frameParserTemplate   = ss_jsr(obj, Keys::FrameParserTemplate, "").toString();
  s.frameParserParams     = ss_jsr(obj, Keys::FrameParserParams, QJsonObject()).toJsonObject();

  if (obj.contains(Keys::ChecksumAlgorithm))
    s.checksumAlgorithm = obj.value(Keys::ChecksumAlgorithm).toString();
  else
    s.checksumAlgorithm = ss_jsr(obj, Keys::Checksum, "").toString();

  if (obj.contains(Keys::DecoderMethod))
    s.decoderMethod = obj.value(Keys::DecoderMethod).toInt();
  else
    s.decoderMethod = ss_jsr(obj, Keys::Decoder, 0).toInt();

  return true;
}

/**
 * @brief Deserializes an Action from a QJsonObject.
 */
[[nodiscard]] inline bool read(Action& a, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  a.txData               = ss_jsr(obj, Keys::TxData, "").toString();
  a.eolSequence          = ss_jsr(obj, Keys::EOL, "").toString();
  a.binaryData           = ss_jsr(obj, Keys::Binary, false).toBool();
  a.sourceId             = ss_jsr(obj, Keys::SourceId, 0).toInt();
  a.icon                 = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  a.title                = ss_jsr(obj, Keys::Title, "").toString().simplified();
  a.txEncoding           = ss_jsr(obj, Keys::TxEncoding, 0).toInt();
  a.repeatCount          = ss_jsr(obj, Keys::RepeatCount, 3).toInt();
  a.timerIntervalMs      = ss_jsr(obj, Keys::TimerInterval, 100).toInt();
  a.autoExecuteOnConnect = ss_jsr(obj, Keys::AutoExecute, false).toBool();

  const int mode = ss_jsr(obj, Keys::TimerMode, 0).toInt();
  if (mode >= 0 && mode <= 4)
    a.timerMode = static_cast<TimerMode>(mode);
  else
    a.timerMode = TimerMode::Off;

  return true;
}

/**
 * @brief Deserializes an AlarmBand from a QJsonObject (Frame.cpp: uses SerialStudio::toDouble).
 */
[[nodiscard]] bool read(AlarmBand& b, const QJsonObject& obj);

/**
 * @brief Populates @p d.alarmBands from @p obj, accepting both canonical and v3.3 legacy fields.
 */
void readDatasetAlarmBands(Dataset& d, const QJsonObject& obj);

/**
 * @brief Swaps inverted (min > max) FFT / plot / widget range pairs from legacy projects.
 */
inline void normalizeDatasetRanges(Dataset& d)
{
  if (d.fftMin > d.fftMax)
    std::swap(d.fftMin, d.fftMax);

  if (d.pltMin > d.pltMax)
    std::swap(d.pltMin, d.pltMax);

  if (d.wgtMin > d.wgtMax)
    std::swap(d.wgtMin, d.wgtMax);
}

/**
 * @brief Deserializes a Dataset from a QJsonObject.
 *        Out-of-line (Frame.cpp): needs SerialStudio::toDouble, which this header cannot include.
 */
[[nodiscard]] bool read(Dataset& d, const QJsonObject& obj);

/**
 * @brief Deserializes a Group from a QJsonObject.
 */
[[nodiscard]] inline bool read(Group& g, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  const auto array  = obj.value(Keys::Datasets).toArray();
  const auto title  = ss_jsr(obj, Keys::Title, "").toString().simplified();
  const auto widget = ss_jsr(obj, Keys::Widget, "").toString().simplified();

  const auto gtVal     = ss_jsr(obj, Keys::GroupType, 0).toInt();
  const auto groupType = static_cast<GroupType>(qBound(0, gtVal, 1));

  const bool isImageGroup   = (widget == QLatin1String("image"));
  const bool isOutputGroup  = (groupType == GroupType::Output);
  const bool isPainterGroup = (widget == QLatin1String("painter"));
  const bool isWebViewGroup = (widget == QLatin1String("webview"));

  if (title.isEmpty())
    return false;

  if (array.isEmpty() && !isImageGroup && !isOutputGroup && !isPainterGroup && !isWebViewGroup)
    return false;

  g.title          = title;
  g.widget         = widget;
  g.groupType      = groupType;
  g.columns        = qBound(1, ss_jsr(obj, Keys::OutputColumns, 2).toInt(), 10);
  g.sourceId       = ss_jsr(obj, Keys::SourceId, 0).toInt();
  g.uniqueId       = ss_jsr(obj, Keys::UniqueId, -1).toInt();
  g.parentFolderId = ss_jsr(obj, Keys::ParentFolderId, -1).toInt();
  g.enabled        = !ss_jsr(obj, Keys::Disabled, false).toBool();

  if (isImageGroup) {
    g.imgDetectionMode = ss_jsr(obj, Keys::ImgMode, "autodetect").toString();
    g.imgStartSequence = ss_jsr(obj, Keys::ImgStart, "").toString();
    g.imgEndSequence   = ss_jsr(obj, Keys::ImgEnd, "").toString();
    return true;
  }

  if (widget == QLatin1String("painter"))
    g.painterCode = ss_jsr(obj, Keys::PainterCode, "").toString();

  if (widget == QLatin1String("webview"))
    g.webViewUrl = ss_jsr(obj, Keys::WebViewUrl, "").toString();

  g.datasets.clear();
  g.datasets.reserve(array.count());

  bool ok = true;
  for (qsizetype i = 0; i < array.count(); ++i) {
    const auto dObj = array[i].toObject();
    if (dObj.isEmpty())
      continue;

    Dataset dataset;
    ok &= read(dataset, dObj);
    if (!ok)
      break;

    dataset.datasetId = i;
    dataset.groupId   = g.groupId;
    dataset.sourceId  = g.sourceId;
    g.datasets.push_back(dataset);
  }

  const auto owArray = obj.value(Keys::OutputWidgets).toArray();
  if (!owArray.isEmpty()) {
    g.outputWidgets.clear();
    g.outputWidgets.reserve(owArray.count());

    for (qsizetype i = 0; i < owArray.count(); ++i) {
      OutputWidget ow;
      if (!read(ow, owArray[i].toObject()))
        continue;

      ow.widgetId = static_cast<int>(i);
      ow.groupId  = g.groupId;
      g.outputWidgets.push_back(ow);
    }
  }

  return ok;
}

/**
 * @brief Deserializes a Frame from a QJsonObject.
 */
[[nodiscard]] inline bool read(Frame& f, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  const auto groups  = obj.value(Keys::Groups).toArray();
  const auto actions = obj.value(Keys::Actions).toArray();
  const auto title   = ss_jsr(obj, Keys::Title, "").toString().simplified();

  if (title.isEmpty() || groups.isEmpty())
    return false;

  f.title = title;
  f.groups.clear();
  f.actions.clear();
  f.groups.reserve(groups.count());
  f.actions.reserve(actions.count());

  f.schemaVersion           = ss_jsr(obj, Keys::SchemaVersion, 0).toInt();
  f.writerVersion           = ss_jsr(obj, Keys::WriterVersion, "").toString();
  f.writerVersionAtCreation = ss_jsr(obj, Keys::WriterVersionAtCreation, "").toString();
  f.controlScriptCode       = ss_jsr(obj, Keys::ControlScriptCode, "").toString();

  bool ok = true;
  for (qsizetype i = 0; i < groups.count(); ++i) {
    Group group;
    group.groupId  = i;
    ok            &= read(group, groups[i].toObject());
    if (!ok)
      break;

    f.groups.push_back(group);
  }

  if (ok) {
    for (qsizetype i = 0; i < actions.count(); ++i) {
      Action action;
      ok &= read(action, actions[i].toObject());
      if (!ok)
        break;

      f.actions.push_back(action);
    }
  }

  if (ok) {
    const auto sources = obj.value(Keys::Sources).toArray();
    f.sources.clear();
    f.sources.reserve(sources.count());
    for (qsizetype i = 0; i < sources.count(); ++i) {
      Source src;
      if (read(src, sources[i].toObject()))
        f.sources.push_back(src);
    }
  }

  if (ok)
    finalize_frame(f);

  return ok;
}

/**
 * @brief Represents a single timestamped frame for data export.
 */
struct TimestampedFrame {
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  DataModel::Frame data;
  SteadyTimePoint timestamp;
  quint64 structureGeneration = 0;  ///< Publisher pool generation; bumps when the layout changes

  /**
   * @brief Default constructor - creates empty timestamped frame.
   */
  TimestampedFrame() = default;

  /**
   * @brief Constructs a timestamped frame by copying frame data.
   */
  explicit TimestampedFrame(const DataModel::Frame& f) : data(f), timestamp(SteadyClock::now()) {}

  /**
   * @brief Constructs a timestamped frame by copying frame data and using an explicit timestamp.
   */
  explicit TimestampedFrame(const DataModel::Frame& f, SteadyTimePoint ts) : data(f), timestamp(ts)
  {}

  /**
   * @brief Constructs a timestamped frame by moving frame data.
   */
  explicit TimestampedFrame(DataModel::Frame&& f) noexcept
    : data(std::move(f)), timestamp(SteadyClock::now())
  {}

  /**
   * @brief Constructs a timestamped frame by moving frame data and using an explicit timestamp.
   */
  explicit TimestampedFrame(DataModel::Frame&& f, SteadyTimePoint ts) noexcept
    : data(std::move(f)), timestamp(ts)
  {}

  TimestampedFrame(TimestampedFrame&&) noexcept            = default;
  TimestampedFrame(const TimestampedFrame&)                = delete;
  TimestampedFrame& operator=(TimestampedFrame&&) noexcept = default;
  TimestampedFrame& operator=(const TimestampedFrame&)     = delete;
};

/**
 * @brief Shared pointer to a TimestampedFrame for efficient multi-consumer distribution.
 */
typedef std::shared_ptr<DataModel::TimestampedFrame> TimestampedFramePtr;

//--------------------------------------------------------------------------------------------------
// Generic utilities using C++20 concepts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generic JSON serialization for any Serializable type.
 */
template<Concepts::Serializable T>
[[nodiscard]] inline QJsonObject toJson(const T& obj) noexcept
{
  return serialize(obj);
}

/**
 * @brief Generic JSON deserialization with validation for Serializable types.
 */
template<Concepts::Serializable T>
[[nodiscard]] inline std::optional<T> fromJson(const QJsonObject& json) noexcept
{
  T obj;
  if (read(obj, json))
    return obj;

  return std::nullopt;
}

/**
 * @brief Validates a Frameable object's structure.
 */
template<Concepts::Frameable T>
[[nodiscard]] constexpr bool isValidFrame(const T& frame) noexcept
{
  return !frame.groups.empty();
}

}  // namespace DataModel
