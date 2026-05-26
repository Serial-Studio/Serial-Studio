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
#include <limits>
#include <memory>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <utility>
#include <vector>

#include "Concepts.h"

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
inline constexpr KeyView FFTSamples("fftSamples");
inline constexpr KeyView Overview("overviewDisplay");
inline constexpr KeyView AlarmEnabled("alarmEnabled");
inline constexpr KeyView AlarmBands("alarmBands");
inline constexpr KeyView Color("color");
inline constexpr KeyView Label("label");
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

// Image-group keys
inline constexpr KeyView ImgMode("imgDetectionMode");
inline constexpr KeyView ImgStart("imgStartSequence");
inline constexpr KeyView ImgEnd("imgEndSequence");

// Painter-group keys
inline constexpr KeyView PainterCode("painterCode");
inline constexpr KeyView HideOnDashboard("hideOnDashboard");

// Dashboard layout keys
inline constexpr KeyView DashboardLayout("dashboardLayout");
inline constexpr KeyView ActiveGroupId("activeGroupId");
inline constexpr KeyView WidgetSettings("widgetSettings");

// Plot history keys
inline constexpr KeyView PointCount("pointCount");
inline constexpr KeyView kActiveGroupSubKey("activeGroup");
inline constexpr KeyView HiddenGroups("hiddenGroups");

// Workspace keys
inline constexpr KeyView Workspaces("workspaces");
inline constexpr KeyView WorkspaceId("workspaceId");
inline constexpr KeyView WidgetRefs("widgetRefs");
inline constexpr KeyView WidgetType("widgetType");
inline constexpr KeyView RelativeIndex("relativeIndex");
inline constexpr KeyView CustomizeWorkspaces("customizeWorkspaces");
inline constexpr KeyView WorkspaceDescription("description");

inline constexpr KeyView Virtual("virtual");

// Data table keys
inline constexpr KeyView Tables("tables");
inline constexpr KeyView Registers("registers");
inline constexpr KeyView RegisterTypeName("type");
inline constexpr KeyView Name("name");

// Project metadata keys (file format / app version stamps)
inline constexpr KeyView SchemaVersion("schemaVersion");
inline constexpr KeyView WriterVersion("writerVersion");
inline constexpr KeyView WriterVersionAtCreation("writerVersionAtCreation");
inline constexpr KeyView NextUniqueId("nextUniqueId");

// Project lock -- PBKDF2 hash (legacy MD5 still accepted on load).
inline constexpr KeyView PasswordHash("passwordHash");

// Per-project MQTT publisher configuration (Pro).
inline constexpr KeyView MqttPublisher("mqttPublisher");

// Full MCP command surface opt-in for user-script apiCall()
inline constexpr KeyView ApiCallAllowFullSurface("apiCallAllowFullSurface");

inline QString layoutKey(int groupId)
{
  return QStringLiteral("layout:") + QString::number(groupId);
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
inline constexpr int UserStart     = 5000;
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
 * @brief Deserializes an OutputWidget from a QJsonObject.
 */
[[nodiscard]] inline bool read(OutputWidget& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.icon     = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.title    = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.sourceId = ss_jsr(obj, Keys::SourceId, 0).toInt();
  w.type     = static_cast<OutputWidgetType>(
    qBound(0, ss_jsr(obj, Keys::OutputType, 0).toInt(), static_cast<int>(OutputWidgetType::Knob)));
  w.minValue         = ss_jsr(obj, Keys::OutputMinValue, 0).toDouble();
  w.maxValue         = ss_jsr(obj, Keys::OutputMaxValue, 100).toDouble();
  w.stepSize         = ss_jsr(obj, Keys::OutputStepSize, 1).toDouble();
  w.initialValue     = ss_jsr(obj, Keys::OutputInitialValue, 0).toDouble();
  w.monoIcon         = ss_jsr(obj, Keys::OutputMonoIcon, false).toBool();
  w.transmitFunction = obj.value(Keys::TransmitFunction).toString();
  w.txEncoding       = ss_jsr(obj, Keys::OutputTxEncoding, 0).toInt();

  return !w.title.isEmpty();
}

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
  QString color;  ///< Optional hex override; empty -> severity default
  QString label;  ///< Optional human label (used in notifications)
};

static_assert(sizeof(AlarmBand) % alignof(AlarmBand) == 0, "Unaligned AlarmBand struct");

/**
 * @brief Represents a single unit of sensor data with optional metadata and
 * graphing. Fully aligned and stack-optimized.
 */
struct alignas(8) Dataset {
  int index             = 0;      ///< Frame offset index
  int xAxisId           = -1;     ///< X-axis source: -1 = sample index, else dataset uniqueId
  int waterfallYAxis    = 0;      ///< Y source for the waterfall -- 0 = Time, N = dataset.index
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
  QString value;                  ///< Raw string value after transforms
  QString rawValue;               ///< Raw string value before transforms
  QString title;                  ///< Human-readable title
  QString units;                  ///< Measurement units (e.g., degC)
  QString widget;                 ///< Widget type (bar, gauge, etc.)
  QString transformCode;          ///< Optional per-dataset transform script
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
  int sourceId        = 0;   ///< Source this group reads from (0 = default)
  int columns         = 2;   ///< Number of columns for output panel grid layout
  GroupType groupType = GroupType::Input;   ///< Input (visualization) or Output (controls)
  QString title;                            ///< Group display name
  QString widget;                           ///< Group widget type
  std::vector<Dataset> datasets;            ///< Datasets contained in this group
  std::vector<OutputWidget> outputWidgets;  ///< Interactive output widgets
  QString imgDetectionMode;                 ///< "autodetect" | "manual" (default: "autodetect")
  QString imgStartSequence;                 ///< Hex start delimiter (manual mode only)
  QString imgEndSequence;                   ///< Hex end delimiter (manual mode only)
  QString painterCode;                      ///< User JS for painter widget (paint/onFrame)
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

  // Emit both legacy and canonical aliases for back-compat with older versions.
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

  return obj;
}

/**
 * @brief Serializes a RegisterDef to a QJsonObject.
 */
[[nodiscard]] inline QJsonObject serialize(const RegisterDef& r)
{
  QJsonObject obj;
  obj.insert(Keys::Name, r.name);

  obj.insert(Keys::RegisterTypeName,
             r.type == RegisterType::Computed ? QStringLiteral("computed")
                                              : QStringLiteral("constant"));

  if (r.defaultValue.typeId() == QMetaType::Double)
    obj.insert(Keys::Value, r.defaultValue.toDouble());
  else if (!r.defaultValue.toString().isEmpty())
    obj.insert(Keys::Value, r.defaultValue.toString());

  return obj;
}

/**
 * @brief Deserializes a RegisterDef from a QJsonObject.
 */
[[nodiscard]] inline bool read(RegisterDef& r, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  r.name = ss_jsr(obj, Keys::Name, "").toString().simplified();
  if (r.name.isEmpty())
    return false;

  const auto typeStr = ss_jsr(obj, Keys::RegisterTypeName, "constant").toString();
  r.type = (typeStr == QLatin1String("computed")) ? RegisterType::Computed : RegisterType::Constant;

  const auto val = obj.value(Keys::Value);
  if (val.isDouble())
    r.defaultValue = val.toDouble();
  else if (val.isString())
    r.defaultValue = val.toString();
  else
    r.defaultValue = QVariant(0.0);

  return true;
}

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
  int workspaceId = -1;
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

  w.workspaceId = ss_jsr(obj, Keys::WorkspaceId, -1).toInt();
  w.title       = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.icon        = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.description = ss_jsr(obj, Keys::WorkspaceDescription, "").toString();

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
}

/**
 * @brief Copies only dataset values from source to destination frame.
 */
inline void copy_frame_values(Frame& dst, const Frame& src) noexcept
{
  const size_t groupCount = src.groups.size();
  for (size_t g = 0; g < groupCount; ++g) {
    const auto& srcGroup      = src.groups[g];
    auto& dstGroup            = dst.groups[g];
    const size_t datasetCount = srcGroup.datasets.size();
    for (size_t d = 0; d < datasetCount; ++d) {
      const auto& srcDataset     = srcGroup.datasets[d];
      auto& dstDataset           = dstGroup.datasets[d];
      dstDataset.value           = srcDataset.value;
      dstDataset.numericValue    = srcDataset.numericValue;
      dstDataset.isNumeric       = srcDataset.isNumeric;
      dstDataset.rawValue        = srcDataset.rawValue;
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

  if (g.groupType != GroupType::Input)
    obj.insert(Keys::GroupType, static_cast<int>(g.groupType));

  if (g.columns != 2)
    obj.insert(Keys::OutputColumns, g.columns);

  if (g.sourceId != 0)
    obj.insert(Keys::SourceId, g.sourceId);

  if (g.widget.simplified() == QLatin1String("image")) {
    obj.insert(Keys::ImgMode, g.imgDetectionMode);
    obj.insert(Keys::ImgStart, g.imgStartSequence);
    obj.insert(Keys::ImgEnd, g.imgEndSequence);
  }

  if (g.widget.simplified() == QLatin1String("painter") && !g.painterCode.isEmpty())
    obj.insert(Keys::PainterCode, g.painterCode);

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

  // Round-trip project-version metadata only when the Frame carries a writer stamp.
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

  // Prefer canonical keys; fall back to legacy aliases from older versions.
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
 * @brief Deserializes an AlarmBand from a QJsonObject.
 */
[[nodiscard]] inline bool read(AlarmBand& b, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  b.min = ss_jsr(obj, Keys::Min, 0).toDouble();
  b.max = ss_jsr(obj, Keys::Max, 0).toDouble();
  if (b.min > b.max)
    std::swap(b.min, b.max);

  const int sev = ss_jsr(obj, Keys::Severity, static_cast<int>(AlarmSeverity::Warning)).toInt();
  b.severity    = static_cast<AlarmSeverity>(qBound(0, sev, 3));
  b.color       = ss_jsr(obj, Keys::Color, "").toString().simplified();
  b.label       = ss_jsr(obj, Keys::Label, "").toString().simplified();
  return b.max > b.min;
}

/**
 * @brief Populates @p d.alarmBands from @p obj, accepting both canonical and v3.3 legacy fields.
 */
inline void readDatasetAlarmBands(Dataset& d, const QJsonObject& obj)
{
  d.alarmBands.clear();
  if (obj.contains(Keys::AlarmBands)) {
    const auto arr = obj.value(Keys::AlarmBands).toArray();
    d.alarmBands.reserve(arr.size());
    for (const auto& v : arr) {
      AlarmBand b;
      if (read(b, v.toObject()))
        d.alarmBands.push_back(std::move(b));
    }
    return;
  }

  if (!ss_jsr(obj, Keys::AlarmEnabled, false).toBool())
    return;

  double lo = ss_jsr(obj, Keys::AlarmLow, std::numeric_limits<double>::quiet_NaN()).toDouble();
  double hi = ss_jsr(obj, Keys::AlarmHigh, std::numeric_limits<double>::quiet_NaN()).toDouble();
  if (std::isnan(hi) && obj.contains(Keys::Alarm))
    hi = ss_jsr(obj, Keys::Alarm, 0).toDouble();

  const double rangeMin = qMin(d.wgtMin, d.wgtMax);
  const double rangeMax = qMax(d.wgtMin, d.wgtMax);
  if (!std::isnan(lo) && lo > rangeMin && lo < rangeMax) {
    AlarmBand low;
    low.min      = rangeMin;
    low.max      = lo;
    low.severity = AlarmSeverity::Warning;
    d.alarmBands.push_back(std::move(low));
  }

  if (!std::isnan(hi) && hi > rangeMin && hi < rangeMax) {
    AlarmBand high;
    high.min      = hi;
    high.max      = rangeMax;
    high.severity = AlarmSeverity::Warning;
    d.alarmBands.push_back(std::move(high));
  }
}

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
 */
[[nodiscard]] inline bool read(Dataset& d, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  d.index             = ss_jsr(obj, Keys::Index, -1).toInt();
  d.fft               = ss_jsr(obj, Keys::FFT, false).toBool();
  d.led               = ss_jsr(obj, Keys::LED, false).toBool();
  d.log               = ss_jsr(obj, Keys::Log, false).toBool();
  d.plt               = ss_jsr(obj, Keys::Graph, false).toBool();
  d.waterfall         = ss_jsr(obj, Keys::Waterfall, false).toBool();
  d.waterfallYAxis    = ss_jsr(obj, Keys::WaterfallYAxis, 0).toInt();
  d.xAxisId           = ss_jsr(obj, Keys::XAxis, -1).toInt();
  d.fftMin            = ss_jsr(obj, Keys::FFTMin, 0).toDouble();
  d.fftMax            = ss_jsr(obj, Keys::FFTMax, 0).toDouble();
  d.pltMin            = ss_jsr(obj, Keys::PltMin, 0).toDouble();
  d.pltMax            = ss_jsr(obj, Keys::PltMax, 0).toDouble();
  d.wgtMin            = ss_jsr(obj, Keys::WgtMin, 0).toDouble();
  d.wgtMax            = ss_jsr(obj, Keys::WgtMax, 0).toDouble();
  d.fftSamples        = ss_jsr(obj, Keys::FFTSamples, -1).toInt();
  d.title             = ss_jsr(obj, Keys::Title, "").toString().simplified();
  d.value             = ss_jsr(obj, Keys::Value, "").toString().simplified();
  d.units             = ss_jsr(obj, Keys::Units, "").toString().simplified();
  d.overviewDisplay   = ss_jsr(obj, Keys::Overview, false).toBool();
  d.ledHigh           = ss_jsr(obj, Keys::LedHigh, 0).toDouble();
  d.widget            = ss_jsr(obj, Keys::Widget, "").toString().simplified();
  d.fftSamplingRate   = ss_jsr(obj, Keys::FFTSamplingRate, -1).toInt();
  d.displayTickCount  = ss_jsr(obj, Keys::DisplayTickCount, 5).toInt();
  d.displayFormat     = ss_jsr(obj, Keys::DisplayFormat, "0d").toString();
  d.sourceId          = ss_jsr(obj, Keys::DatasetSourceId, 0).toInt();
  d.transformCode     = obj.value(Keys::TransformCode).toString();
  d.transformLanguage = ss_jsr(obj, Keys::TransformLanguage, -1).toInt();
  d.virtual_          = ss_jsr(obj, Keys::Virtual, false).toBool();
  d.hideOnDashboard   = ss_jsr(obj, Keys::HideOnDashboard, false).toBool();
  d.uniqueId          = ss_jsr(obj, Keys::UniqueId, -1).toInt();

  if (!d.value.isEmpty())
    d.numericValue = d.value.toDouble(&d.isNumeric);

  // Legacy fallback: pre-3.x projects used a shared min/max for FFT/plot/widget.
  if (!obj.contains(Keys::FFTMin) || !obj.contains(Keys::FFTMax)) {
    d.fftMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.fftMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  if (!obj.contains(Keys::PltMin) || !obj.contains(Keys::PltMax)) {
    d.pltMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.pltMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  if (!obj.contains(Keys::WgtMin) || !obj.contains(Keys::WgtMax)) {
    d.wgtMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.wgtMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  readDatasetAlarmBands(d, obj);
  normalizeDatasetRanges(d);
  return true;
}

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

  if (title.isEmpty())
    return false;

  if (array.isEmpty() && !isImageGroup && !isOutputGroup && !isPainterGroup)
    return false;

  g.title     = title;
  g.widget    = widget;
  g.groupType = groupType;
  g.columns   = qBound(1, ss_jsr(obj, Keys::OutputColumns, 2).toInt(), 10);
  g.sourceId  = ss_jsr(obj, Keys::SourceId, 0).toInt();
  g.uniqueId  = ss_jsr(obj, Keys::UniqueId, -1).toInt();

  if (isImageGroup) {
    g.imgDetectionMode = ss_jsr(obj, Keys::ImgMode, "autodetect").toString();
    g.imgStartSequence = ss_jsr(obj, Keys::ImgStart, "").toString();
    g.imgEndSequence   = ss_jsr(obj, Keys::ImgEnd, "").toString();
    return true;
  }

  // Painter groups carry user JS plus optional datasets; don't early-return
  if (widget == QLatin1String("painter"))
    g.painterCode = ss_jsr(obj, Keys::PainterCode, "").toString();

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

  // Read project-version metadata; pre-stamp files default to 0 / empty.
  f.schemaVersion           = ss_jsr(obj, Keys::SchemaVersion, 0).toInt();
  f.writerVersion           = ss_jsr(obj, Keys::WriterVersion, "").toString();
  f.writerVersionAtCreation = ss_jsr(obj, Keys::WriterVersionAtCreation, "").toString();

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

  // Sources are optional
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
