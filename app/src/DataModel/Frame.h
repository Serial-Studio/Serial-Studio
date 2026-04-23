/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
#include <memory>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>
#include <vector>

#include "Concepts.h"

//--------------------------------------------------------------------------------------------------
// Standard keys for loading/offloading frame structures using JSON files
//--------------------------------------------------------------------------------------------------

namespace Keys {
inline constexpr auto EOL           = "eol";
inline constexpr auto Icon          = "icon";
inline constexpr auto Title         = "title";
inline constexpr auto TxData        = "txData";
inline constexpr auto Binary        = "binary";
inline constexpr auto TxEncoding    = "txEncoding";
inline constexpr auto TimerMode     = "timerMode";
inline constexpr auto RepeatCount   = "repeatCount";
inline constexpr auto TimerInterval = "timerIntervalMs";
inline constexpr auto AutoExecute   = "autoExecuteOnConnect";

inline constexpr auto Sources         = "sources";
inline constexpr auto SourceId        = "sourceId";
inline constexpr auto SourceConn      = "connection";
inline constexpr auto DatasetSourceId = "datasetSourceId";

inline constexpr auto FFT             = "fft";
inline constexpr auto LED             = "led";
inline constexpr auto Log             = "log";
inline constexpr auto Min             = "min";
inline constexpr auto Max             = "max";
inline constexpr auto Graph           = "graph";
inline constexpr auto Index           = "index";
inline constexpr auto XAxis           = "xAxis";
inline constexpr auto Alarm           = "alarm";
inline constexpr auto Units           = "units";
inline constexpr auto Value           = "value";
inline constexpr auto Widget          = "widget";
inline constexpr auto FFTMin          = "fftMin";
inline constexpr auto FFTMax          = "fftMax";
inline constexpr auto PltMin          = "plotMin";
inline constexpr auto PltMax          = "plotMax";
inline constexpr auto LedHigh         = "ledHigh";
inline constexpr auto WgtMin          = "widgetMin";
inline constexpr auto WgtMax          = "widgetMax";
inline constexpr auto AlarmLow        = "alarmLow";
inline constexpr auto AlarmHigh       = "alarmHigh";
inline constexpr auto FFTSamples      = "fftSamples";
inline constexpr auto Overview        = "overviewDisplay";
inline constexpr auto AlarmEnabled    = "alarmEnabled";
inline constexpr auto FFTSamplingRate = "fftSamplingRate";
inline constexpr auto TransformCode   = "transformCode";

inline constexpr auto Groups        = "groups";
inline constexpr auto Actions       = "actions";
inline constexpr auto Datasets      = "datasets";
inline constexpr auto OutputWidgets = "outputWidgets";

inline constexpr auto OutputType         = "outputType";
inline constexpr auto OutputMinValue     = "outputMin";
inline constexpr auto OutputMaxValue     = "outputMax";
inline constexpr auto OutputStepSize     = "outputStep";
inline constexpr auto OutputInitialValue = "initialValue";
inline constexpr auto OutputOnLabel      = "onLabel";
inline constexpr auto OutputOffLabel     = "offLabel";
inline constexpr auto OutputMonoIcon     = "monoIcon";
inline constexpr auto OutputColumns      = "outputColumns";
inline constexpr auto TransmitFunction   = "transmitFunction";
inline constexpr auto OutputTxEncoding   = "outputTxEncoding";

inline constexpr auto GroupId   = "groupId";
inline constexpr auto GroupType = "groupType";

inline constexpr auto ImgMode  = "imgDetectionMode";
inline constexpr auto ImgStart = "imgStartSequence";
inline constexpr auto ImgEnd   = "imgEndSequence";

inline constexpr auto DashboardLayout = "dashboardLayout";
inline constexpr auto ActiveGroupId   = "activeGroupId";
inline constexpr auto WidgetSettings  = "widgetSettings";

inline constexpr auto PointCount         = "pointCount";
inline constexpr auto kActiveGroupSubKey = "activeGroup";
inline constexpr auto HiddenGroups       = "hiddenGroups";

inline constexpr auto Workspaces    = "workspaces";
inline constexpr auto WorkspaceId   = "workspaceId";
inline constexpr auto WidgetRefs    = "widgetRefs";
inline constexpr auto WidgetType    = "widgetType";
inline constexpr auto RelativeIndex = "relativeIndex";

inline constexpr auto Virtual = "virtual";

inline constexpr auto Tables           = "tables";
inline constexpr auto Registers        = "registers";
inline constexpr auto RegisterTypeName = "type";
inline constexpr auto Name             = "name";

inline QString layoutKey(int groupId)
{
  return QStringLiteral("layout:") + QString::number(groupId);
}
}  // namespace Keys

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Utility functions for data deserialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads a value from a QJsonObject based on a key, returning a default
 *        value if the key does not exist.
 *
 * This function checks if the given key exists in the provided QJsonObject.
 * If the key is found, it returns the associated value. Otherwise, it returns
 * the specified default value.
 *
 * @param object The QJsonObject to read the data from.
 * @param key The key to look for in the QJsonObject.
 * @param defaultValue The value to return if the key is not found in JSON.
 *
 * @return The value associated with the key, or the defaultValue if the key is
 *         not present.
 */
[[nodiscard]] inline QVariant ss_jsr(const QJsonObject& object,
                                     const QString& key,
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
 *
 * This function resolves the action's `txData`, `eolSequence`, and encoding
 * settings into a final `QByteArray` suitable for transmission.
 *
 * This function exists outside the Frame definition to avoid circular
 * dependencies between `Frame.h` and `SerialStudio.h`.
 *
 * @param action The Action to generate the transmission bytes from.
 * @return QByteArray containing the resolved byte stream to transmit.
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
  RampGenerator,
};

/**
 * @brief Represents an interactive output widget that sends data to the device.
 *
 * Each output widget contains a JavaScript `transmit(value)` function that
 * converts the widget's current value into bytes to send over the connection.
 * This allows users to format output data for any protocol or requirement.
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
 * @param w The OutputWidget to serialize.
 * @return QJsonObject representing the OutputWidget.
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
 * @param w Output OutputWidget object to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
 */
[[nodiscard]] inline bool read(OutputWidget& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.icon             = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  w.title            = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.sourceId         = ss_jsr(obj, Keys::SourceId, 0).toInt();
  w.type             = static_cast<OutputWidgetType>(qBound(0,
                                                ss_jsr(obj, Keys::OutputType, 0).toInt(),
                                                static_cast<int>(OutputWidgetType::RampGenerator)));
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
 * @brief Represents a single unit of sensor data with optional metadata and
 * graphing. Fully aligned and stack-optimized.
 */
struct alignas(8) Dataset {
  int index              = 0;      ///< Frame offset index
  int xAxisId            = -1;     ///< Optional reference to x-axis dataset
  int groupId            = 0;      ///< Owning group ID
  int sourceId           = 0;      ///< Source this dataset belongs to
  int uniqueId           = 0;      ///< Unique ID within frame
  int datasetId          = 0;      ///< Unique ID within group
  int fftSamples         = 256;    ///< Number of samples for FFT
  int fftSamplingRate    = 100;    ///< Sampling rate for FFT
  bool fft               = false;  ///< Enables FFT processing
  bool led               = false;  ///< Enables LED widget
  bool log               = false;  ///< Enables logging
  bool plt               = false;  ///< Enables plotting
  bool alarmEnabled      = false;  ///< Enable/disable alarm values
  bool overviewDisplay   = false;  ///< Show in overview
  bool isNumeric         = false;  ///< True if value was parsed as numeric
  bool virtual_          = false;  ///< True if dataset is generated rather than parsed directly
  double fftMin          = 0;      ///< Minimum value (for FFT)
  double fftMax          = 0;      ///< Maximum value (for FFT)
  double pltMin          = 0;      ///< Minimum value (for plots)
  double pltMax          = 0;      ///< Maximum value (for plots)
  double wgtMin          = 0;      ///< Minimum value (for widgets)
  double wgtMax          = 0;      ///< Maximum value (for widgets)
  double ledHigh         = 80;     ///< LED activation threshold
  double alarmLow        = 20;     ///< Low alarm threshold
  double alarmHigh       = 80;     ///< High alarm threshold
  double numericValue    = 0;      ///< Parsed numeric value after transforms
  double rawNumericValue = 0;      ///< Parsed numeric value before transforms
  QString value;                   ///< Raw string value after transforms
  QString rawValue;                ///< Raw string value before transforms
  QString title;                   ///< Human-readable title
  QString units;                   ///< Measurement units (e.g., °C)
  QString widget;                  ///< Widget type (bar, gauge, etc.)
  QString transformCode;           ///< Optional per-dataset transform script
};

static_assert(sizeof(Dataset) % alignof(Dataset) == 0, "Unaligned Dataset struct");

/**
 * @brief Distinguishes input (visualization) groups from output (control) groups.
 *
 * Legacy projects that lack this field default to Input, preserving backward
 * compatibility.
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
  int groupId         = -1;                 ///< Unique group identifier
  int sourceId        = 0;                  ///< Source this group reads from (0 = default)
  int columns         = 2;                  ///< Number of columns for output panel grid layout
  GroupType groupType = GroupType::Input;   ///< Input (visualization) or Output (controls)
  QString title;                            ///< Group display name
  QString widget;                           ///< Group widget type
  std::vector<Dataset> datasets;            ///< Datasets contained in this group
  std::vector<OutputWidget> outputWidgets;  ///< Interactive output widgets
  QString imgDetectionMode;                 ///< "autodetect" | "manual" (default: "autodetect")
  QString imgStartSequence;                 ///< Hex start delimiter (manual mode only)
  QString imgEndSequence;                   ///< Hex end delimiter (manual mode only)
};

static_assert(sizeof(Group) % alignof(Group) == 0, "Unaligned Group struct");

/**
 * @brief Describes a single data source (device + connection settings) within a
 * project. Each source owns its own frame detection configuration and opaque
 * connection settings.
 *
 * busType is stored as int to avoid a circular dependency with SerialStudio.h
 * (which already includes Frame.h). Cast to SerialStudio::BusType at call sites.
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
 * @param s The Source to serialize.
 * @return QJsonObject representing the Source.
 */
[[nodiscard]] inline QJsonObject serialize(const Source& s)
{
  QJsonObject obj;
  obj.insert(Keys::SourceId, s.sourceId);
  obj.insert(Keys::Title, s.title.simplified());
  obj.insert("busType", s.busType);
  obj.insert("frameStart", s.frameStart);
  obj.insert("frameEnd", s.frameEnd);
  obj.insert("checksum", s.checksumAlgorithm);
  obj.insert("checksumAlgorithm", s.checksumAlgorithm);
  obj.insert("frameDetection", s.frameDetection);
  obj.insert("decoder", s.decoderMethod);
  obj.insert("decoderMethod", s.decoderMethod);
  obj.insert("hexadecimalDelimiters", s.hexadecimalDelimiters);
  if (!s.connectionSettings.isEmpty())
    obj.insert(Keys::SourceConn, s.connectionSettings);

  if (!s.frameParserCode.isEmpty())
    obj.insert("frameParserCode", s.frameParserCode);

  if (s.frameParserLanguage != 0)
    obj.insert("frameParserLanguage", s.frameParserLanguage);

  return obj;
}

/**
 * @brief Serializes a RegisterDef to a QJsonObject.
 * @param r The RegisterDef to serialize.
 * @return QJsonObject representing the register definition.
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
 * @param r Output RegisterDef to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
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
 * @param t The TableDef to serialize.
 * @return QJsonObject representing the table definition.
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
 * @param t Output TableDef to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
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
 * @brief Represents a reference to a specific widget in the dashboard.
 *
 * Each WidgetRef identifies a widget by its type, the group it belongs to,
 * and its relative index within that widget type.
 */
struct WidgetRef {
  int widgetType    = 0;
  int groupId       = -1;
  int relativeIndex = -1;
};

/**
 * @brief Represents a user-defined dashboard workspace.
 *
 * A workspace is a named collection of widget references that the user can
 * create, modify, and switch between in the dashboard. Each workspace
 * maintains its own layout persistence via the layout key system.
 */
struct Workspace {
  int workspaceId = -1;
  QString title;
  QString icon;
  std::vector<WidgetRef> widgetRefs;
};

/**
 * @brief Serializes a Workspace to a QJsonObject.
 * @param w The Workspace to serialize.
 * @return QJsonObject representing the Workspace.
 */
[[nodiscard]] inline QJsonObject serialize(const Workspace& w)
{
  QJsonObject obj;
  obj.insert(Keys::WorkspaceId, w.workspaceId);
  obj.insert(Keys::Title, w.title.simplified());
  if (!w.icon.isEmpty())
    obj.insert(Keys::Icon, w.icon);

  QJsonArray refs;
  for (const auto& ref : w.widgetRefs) {
    QJsonObject r;
    r.insert(Keys::WidgetType, ref.widgetType);
    r.insert(Keys::GroupId, ref.groupId);
    r.insert(Keys::RelativeIndex, ref.relativeIndex);
    refs.append(r);
  }

  obj.insert(Keys::WidgetRefs, refs);
  return obj;
}

/**
 * @brief Deserializes a Workspace from a QJsonObject.
 * @param w Output Workspace object to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
 */
[[nodiscard]] inline bool read(Workspace& w, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  w.workspaceId = ss_jsr(obj, Keys::WorkspaceId, -1).toInt();
  w.title       = ss_jsr(obj, Keys::Title, "").toString().simplified();
  w.icon        = ss_jsr(obj, Keys::Icon, "").toString().simplified();

  w.widgetRefs.clear();
  const auto refs = obj.value(Keys::WidgetRefs).toArray();
  for (const auto& val : refs) {
    const auto r = val.toObject();
    WidgetRef ref;
    ref.widgetType    = ss_jsr(r, Keys::WidgetType, 0).toInt();
    ref.groupId       = ss_jsr(r, Keys::GroupId, -1).toInt();
    ref.relativeIndex = ss_jsr(r, Keys::RelativeIndex, -1).toInt();
    w.widgetRefs.push_back(ref);
  }

  return w.workspaceId >= 0 && !w.title.isEmpty();
}

/**
 * @brief Represents a full data frame, including groups and actions.
 *        This is the root structure for each UI update.
 */
struct alignas(8) Frame {
  int sourceId = 0;                         ///< Source that produced this frame
  QString title;                            ///< Frame title
  std::vector<Group> groups;                ///< Sensor groups in this frame
  std::vector<Action> actions;              ///< Triggerable actions
  bool containsCommercialFeatures = false;  ///< Feature gating flag
};

static_assert(sizeof(Frame) % alignof(Frame) == 0, "Unaligned Frame struct");

/**
 * @brief Clears and resets a Frame object to its default state.
 *
 * This utility function performs a full reset of a Frame by:
 * - Clearing the frame title string
 * - Clearing all groups and actions vectors
 * - Releasing memory held by groups and actions via `shrink_to_fit()`
 * - Resetting `containsCommercialFeatures` to `false`
 *
 * **Performance:**
 * - Time: O(n) where n = total datasets across all groups
 * - Space: Releases all heap-allocated memory for groups/actions
 *
 * **Use Cases:**
 * - Recycling Frame objects in object pools
 * - Resetting state when switching data sources
 * - Preparing for new frame deserialization
 *
 * **Thread Safety:** Not thread-safe - caller must synchronize access
 *
 * @param frame The Frame object to be cleared and reset
 *
 * @note Prefer this over destroying and recreating Frame objects to reduce
 *       allocator pressure in high-frequency scenarios
 */
inline void clear_frame(Frame& frame) noexcept
{
  frame.title.clear();
  frame.groups.clear();
  frame.actions.clear();
  frame.groups.shrink_to_fit();
  frame.actions.shrink_to_fit();
  frame.containsCommercialFeatures = false;
}

/**
 * @brief Copies only dataset values from source to destination frame.
 *
 * This function performs a fast value-only copy between two structurally
 * equivalent frames. It assumes both frames have identical structure (same
 * groups and datasets) and only copies the mutable value fields.
 *
 * **Copied Fields per Dataset:**
 * - value/rawValue (QString)
 * - numericValue/rawNumericValue (double)
 * - isNumeric (bool)
 *
 * **Performance:**
 * - Time: O(d) where d = total datasets across all groups
 * - Space: O(1) - no allocations, reuses existing vectors
 * - Compared to full copy: substantially faster for typical frames
 *
 * **Precondition:** Both frames must have identical structure (same number
 * of groups and datasets per group). Use compare_frames() to verify.
 *
 * **Thread Safety:** Not thread-safe - caller must synchronize access
 *
 * @param dst Destination frame (must have same structure as src)
 * @param src Source frame to copy values from
 *
 * @warning Undefined behavior if frames have different structures
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
 *
 * This function checks whether two Frame instances have the same group count,
 * matching group IDs, and identical dataset `index` values within each group.
 *
 * **Comparison Scope:**
 * - Checked: Group count, group IDs, dataset count, dataset indices
 * - Ignored: Titles, actions, values, units, widget types
 *
 * **Use Cases:**
 * - Detecting frame structure changes during live updates
 * - Validating compatibility between recorded and live data
 * - Determining if dashboard layout needs regeneration
 *
 * **Performance:**
 * - Best case: O(1) when group counts differ
 * - Average case: O(g) where g = number of groups (when early mismatch)
 * - Worst case: O(g × d) where d = average datasets per group
 *
 * **Thread Safety:** Safe if both frames are immutable during comparison
 *
 * @param a First frame to compare
 * @param b Second frame to compare
 * @return true if both frames are structurally equivalent; false otherwise
 *
 * @note This is used in hot path for frame change detection - keep optimized
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
 * @brief Finalizes a Frame after deserialization.
 *
 * This function performs post-processing on a Frame object after it has been
 * read from JSON.
 *
 * This function exists outside the Frame definition to avoid circular
 * dependencies between `Frame.h` and `SerialStudio.h`.
 *
 * @param frame The Frame object to finalize.
 */
void finalize_frame(Frame& frame);

/**
 * @brief Reads and parses I/O frame settings from a JSON object.
 *
 * This function extracts serial frame delimiters and checksum settings
 * from the given JSON configuration. It supports both hexadecimal and
 * escaped string formats.
 *
 * This function exists outside the Frame definition to avoid circular
 * dependencies between `Frame.h` and `SerialStudio.h`.
 *
 * @param frameStart Output byte array for the start-of-frame marker.
 * @param frameEnd Output byte array for the end-of-frame marker.
 * @param checksum Output string indicating the checksum method to use.
 * @param obj Input JSON object containing the I/O settings.
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
 *
 * Converts an Action object into a JSON representation containing:
 * - `icon`: The icon associated with the action.
 * - `title`: The display title of the action.
 * - `txData`: The data to transmit.
 * - `eol`: The end-of-line sequence (e.g., "\\r\\n").
 * - `binary`: Whether the data should be interpreted as binary.
 * - `sourceId`: Target source/device ID for transmission.
 * - `txEncoding`: Encoding mode for payload generation.
 * - `timerIntervalMs`: Timer interval in milliseconds.
 * - `timerMode`: Integer value representing the timer mode enum.
 * - `autoExecuteOnConnect`: Whether to auto-execute this action on device
 *                           connection.
 *
 * @param a The Action object to serialize.
 * @return QJsonObject representing the Action.
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
 * @brief Serializes a Dataset to a QJsonObject.
 *
 * Converts a Dataset object into a JSON structure including:
 * - Flags: `fft`, `led`, `log`, `graph`, `overviewDisplay`, `virtual`
 * - Indices: `index`, `xAxis`, `groupId`, `datasetId`
 * - Thresholds: `ledHigh`, `alarmLow`, `alarmHigh`
 * - Limits: `fftMin/fftMax`, `plotMin/plotMax`, `widgetMin/widgetMax`
 * - Metadata: `title`, `value`, `units`, `widget`, `numericValue`
 * - FFT settings: `fftSamples`, `fftSamplingRate`
 * - Optional transform code
 *
 * All QString fields are simplified where appropriate.
 *
 * @param d The Dataset object to serialize.
 * @return QJsonObject representing the Dataset.
 */
[[nodiscard]] inline QJsonObject serialize(const Dataset& d)
{
  QJsonObject obj;
  obj.insert(Keys::FFT, d.fft);
  obj.insert(Keys::LED, d.led);
  obj.insert(Keys::Log, d.log);
  obj.insert(Keys::Graph, d.plt);
  obj.insert(Keys::Index, d.index);
  obj.insert(Keys::XAxis, d.xAxisId);
  obj.insert(Keys::LedHigh, d.ledHigh);
  obj.insert(Keys::FFTSamples, d.fftSamples);
  obj.insert(Keys::Title, d.title.simplified());
  obj.insert(Keys::Value, d.value.simplified());
  obj.insert(Keys::Units, d.units.simplified());
  obj.insert(Keys::AlarmEnabled, d.alarmEnabled);
  obj.insert(Keys::Widget, d.widget.simplified());
  obj.insert(Keys::FFTMin, qMin(d.fftMin, d.fftMax));
  obj.insert(Keys::FFTMax, qMax(d.fftMin, d.fftMax));
  obj.insert(Keys::PltMin, qMin(d.pltMin, d.pltMax));
  obj.insert(Keys::PltMax, qMax(d.pltMin, d.pltMax));
  obj.insert(Keys::WgtMin, qMin(d.wgtMin, d.wgtMax));
  obj.insert(Keys::WgtMax, qMax(d.wgtMin, d.wgtMax));
  obj.insert(Keys::FFTSamplingRate, d.fftSamplingRate);
  obj.insert(Keys::AlarmLow, qMin(d.alarmLow, d.alarmHigh));
  obj.insert(Keys::AlarmHigh, qMax(d.alarmLow, d.alarmHigh));

  obj.insert(Keys::GroupId, d.groupId);
  obj.insert(QStringLiteral("datasetId"), d.datasetId);
  obj.insert(QStringLiteral("numericValue"), d.numericValue);

  if (!d.transformCode.isEmpty())
    obj.insert(Keys::TransformCode, d.transformCode);

  if (d.virtual_)
    obj.insert(Keys::Virtual, true);

  return obj;
}

/**
 * @brief Serializes a Group to a QJsonObject.
 *
 * Converts a Group and all of its datasets/output widgets into a JSON
 * structure.
 *
 * @param g The Group object to serialize.
 * @return QJsonObject representing the Group.
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
 *
 * Converts a Frame and its contents into a full JSON object including:
 * - `title`: Frame title
 * - `groups`: Array of serialized Group objects
 * - `actions`: Array of serialized Action objects
 *
 * This function is used to export the entire runtime model to JSON.
 *
 * @param f The Frame object to serialize.
 * @return QJsonObject representing the complete Frame.
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
  return obj;
}

//--------------------------------------------------------------------------------------------------
// Data deserialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Deserializes a Source from a QJsonObject.
 * @param s Output Source to populate.
 * @param obj JSON object to read from.
 * @return true if successfully parsed.
 */
[[nodiscard]] inline bool read(Source& s, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  s.sourceId              = ss_jsr(obj, Keys::SourceId, 0).toInt();
  s.title                 = ss_jsr(obj, Keys::Title, "").toString().simplified();
  s.busType               = ss_jsr(obj, "busType", 0).toInt();
  s.frameStart            = ss_jsr(obj, "frameStart", "").toString();
  s.frameEnd              = ss_jsr(obj, "frameEnd", "\\n").toString();
  s.checksumAlgorithm     = ss_jsr(obj, "checksum", "").toString();
  s.frameDetection        = ss_jsr(obj, "frameDetection", 0).toInt();
  s.decoderMethod         = ss_jsr(obj, "decoder", 0).toInt();
  s.hexadecimalDelimiters = ss_jsr(obj, "hexadecimalDelimiters", false).toBool();
  s.connectionSettings    = ss_jsr(obj, Keys::SourceConn, QJsonObject()).toJsonObject();
  s.frameParserCode       = ss_jsr(obj, "frameParserCode", "").toString();
  s.frameParserLanguage   = ss_jsr(obj, "frameParserLanguage", 0).toInt();
  return true;
}

/**
 * @brief Deserializes an Action from a QJsonObject.
 *
 * Parses fields from JSON into an Action structure, including:
 * - `txData`: Transmit data
 * - `eol`: End-of-line sequence
 * - `binary`: Binary mode flag
 * - `sourceId`: Target source/device ID for transmission
 * - `icon`: Icon path or name
 * - `title`: Display name
 * - `txEncoding`: Payload encoding mode
 * - `timerIntervalMs`: Timer interval in milliseconds
 * - `autoExecuteOnConnect`: Whether to trigger action on connection
 * - `timerMode`: TimerMode enum (validated and cast safely)
 *
 * @param a Output Action object to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
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
 * @brief Deserializes a Dataset from a QJsonObject.
 *
 * Parses all dataset configuration fields, including:
 * - Structural fields: `index`, `groupId`, `datasetId`, `xAxis`
 * - Visualization flags: `fft`, `led`, `log`, `plt`, `overviewDisplay`
 * - Thresholds and limits: `min`, `max`, `ledHigh`, `alarmLow`, `alarmHigh`
 * - FFT settings: `fftSamples`, `fftSamplingRate`
 * - Display info: `title`, `value`, `units`, `widget`
 * - Optional transform code and `virtual` flag
 *
 * If a numeric value is detected in `value`, it's parsed and stored in
 * `numericValue` with the `isNumeric` flag set.
 *
 * Handles legacy single `alarm` field if both high/low are unset.
 * Applies auto-normalization for min/max order.
 *
 * @param d Output Dataset object to populate.
 * @param obj JSON object to parse.
 * @return true if successfully parsed, false if input is malformed.
 */
[[nodiscard]] inline bool read(Dataset& d, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  d.index           = ss_jsr(obj, Keys::Index, -1).toInt();
  d.fft             = ss_jsr(obj, Keys::FFT, false).toBool();
  d.led             = ss_jsr(obj, Keys::LED, false).toBool();
  d.log             = ss_jsr(obj, Keys::Log, false).toBool();
  d.plt             = ss_jsr(obj, Keys::Graph, false).toBool();
  d.xAxisId         = ss_jsr(obj, Keys::XAxis, -1).toInt();
  d.fftMin          = ss_jsr(obj, Keys::FFTMin, 0).toDouble();
  d.fftMax          = ss_jsr(obj, Keys::FFTMax, 0).toDouble();
  d.pltMin          = ss_jsr(obj, Keys::PltMin, 0).toDouble();
  d.pltMax          = ss_jsr(obj, Keys::PltMax, 0).toDouble();
  d.wgtMin          = ss_jsr(obj, Keys::WgtMin, 0).toDouble();
  d.wgtMax          = ss_jsr(obj, Keys::WgtMax, 0).toDouble();
  d.fftSamples      = ss_jsr(obj, Keys::FFTSamples, -1).toInt();
  d.title           = ss_jsr(obj, Keys::Title, "").toString().simplified();
  d.value           = ss_jsr(obj, Keys::Value, "").toString().simplified();
  d.units           = ss_jsr(obj, Keys::Units, "").toString().simplified();
  d.overviewDisplay = ss_jsr(obj, Keys::Overview, false).toBool();
  d.alarmEnabled    = ss_jsr(obj, Keys::AlarmEnabled, false).toBool();
  d.ledHigh         = ss_jsr(obj, Keys::LedHigh, 0).toDouble();
  d.widget          = ss_jsr(obj, Keys::Widget, "").toString().simplified();
  d.alarmLow        = ss_jsr(obj, Keys::AlarmLow, 0).toDouble();
  d.fftSamplingRate = ss_jsr(obj, Keys::FFTSamplingRate, -1).toInt();
  d.alarmHigh       = ss_jsr(obj, Keys::AlarmHigh, 0).toDouble();
  d.sourceId        = ss_jsr(obj, Keys::DatasetSourceId, 0).toInt();
  d.transformCode   = obj.value(Keys::TransformCode).toString();
  d.virtual_        = ss_jsr(obj, Keys::Virtual, false).toBool();

  if (d.value.isEmpty())
    d.value = QStringLiteral("--.--");
  else
    d.numericValue = d.value.toDouble(&d.isNumeric);

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

  if (obj.contains(Keys::Alarm)) {
    if (std::isnan(d.alarmHigh) && std::isnan(d.alarmLow)) {
      auto alarm  = ss_jsr(obj, Keys::Alarm, 0).toDouble();
      d.alarmHigh = alarm;
    }
  }

  if (!std::isnan(d.fftMin) && !std::isnan(d.fftMax)) {
    d.fftMin = qMin(d.fftMin, d.fftMax);
    d.fftMax = qMax(d.fftMin, d.fftMax);
  }

  if (!std::isnan(d.pltMin) && !std::isnan(d.pltMax)) {
    d.pltMin = qMin(d.pltMin, d.pltMax);
    d.pltMax = qMax(d.pltMin, d.pltMax);
  }

  if (!std::isnan(d.wgtMin) && !std::isnan(d.wgtMax)) {
    d.wgtMin = qMin(d.wgtMin, d.wgtMax);
    d.wgtMax = qMax(d.wgtMin, d.wgtMax);
  }

  return true;
}

/**
 * @brief Deserializes a Group from a QJsonObject.
 *
 * Reads a group's metadata and its dataset array. Each dataset is deserialized
 * and automatically assigned:
 * - `datasetId` based on array index
 * - `groupId` inherited from parent group
 *
 * Image and output groups may be valid even when they do not contain telemetry
 * datasets.
 *
 * @param g Output Group object to populate.
 * @param obj JSON object representing a group.
 * @return true if group and datasets were valid and parsed correctly.
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

  const bool isImageGroup  = (widget == QLatin1String("image"));
  const bool isOutputGroup = (groupType == GroupType::Output);

  if (!title.isEmpty() && (!array.isEmpty() || isImageGroup || isOutputGroup)) {
    g.title     = title;
    g.widget    = widget;
    g.groupType = groupType;
    g.columns   = qBound(1, ss_jsr(obj, Keys::OutputColumns, 2).toInt(), 10);
    g.sourceId  = ss_jsr(obj, Keys::SourceId, 0).toInt();

    if (isImageGroup) {
      g.imgDetectionMode = ss_jsr(obj, Keys::ImgMode, "autodetect").toString();
      g.imgStartSequence = ss_jsr(obj, Keys::ImgStart, "").toString();
      g.imgEndSequence   = ss_jsr(obj, Keys::ImgEnd, "").toString();
      return true;
    }

    g.datasets.clear();
    g.datasets.reserve(array.count());

    bool ok = true;
    for (qsizetype i = 0; i < array.count(); ++i) {
      const auto dObj = array[i].toObject();
      if (!dObj.isEmpty()) {
        Dataset dataset;
        ok &= read(dataset, dObj);

        if (ok) {
          dataset.datasetId = i;
          dataset.groupId   = g.groupId;
          dataset.sourceId  = g.sourceId;
          g.datasets.push_back(dataset);
        }

        else
          break;
      }
    }

    const auto owArray = obj.value(Keys::OutputWidgets).toArray();
    if (!owArray.isEmpty()) {
      g.outputWidgets.clear();
      g.outputWidgets.reserve(owArray.count());

      for (qsizetype i = 0; i < owArray.count(); ++i) {
        OutputWidget ow;
        if (read(ow, owArray[i].toObject())) {
          ow.widgetId = static_cast<int>(i);
          ow.groupId  = g.groupId;
          g.outputWidgets.push_back(ow);
        }
      }
    }

    return ok;
  }

  return false;
}

/**
 * @brief Deserializes a Frame from a QJsonObject.
 *
 * Parses the entire frame hierarchy, including:
 * - `title`: Frame title
 * - `groups`: Array of Group objects
 * - `actions`: Array of Action objects
 *
 * Each group and dataset is assigned unique IDs based on position.
 * Automatically calls `finalize_frame()` after successful parse to compute
 * `containsCommercialFeatures` and assign dataset unique IDs.
 *
 * @param f Output Frame object to populate.
 * @param obj JSON object representing the frame.
 * @return true if the frame is valid and successfully parsed.
 */
[[nodiscard]] inline bool read(Frame& f, const QJsonObject& obj)
{
  if (obj.isEmpty())
    return false;

  const auto groups  = obj.value(Keys::Groups).toArray();
  const auto actions = obj.value(Keys::Actions).toArray();
  const auto title   = ss_jsr(obj, Keys::Title, "").toString().simplified();

  if (!title.isEmpty() && !groups.isEmpty()) {
    f.title = title;
    f.groups.clear();
    f.actions.clear();
    f.groups.reserve(groups.count());
    f.actions.reserve(actions.count());

    bool ok = true;
    for (qsizetype i = 0; i < groups.count(); ++i) {
      Group group;
      group.groupId = i;
      ok &= read(group, groups[i].toObject());
      if (ok)
        f.groups.push_back(group);
      else
        break;
    }

    if (ok) {
      for (qsizetype i = 0; i < actions.count(); ++i) {
        Action action;
        ok &= read(action, actions[i].toObject());
        if (ok)
          f.actions.push_back(action);
        else
          break;
      }
    }

    if (ok)
      finalize_frame(f);

    return ok;
  }

  return false;
}

/**
 * @brief Represents a single timestamped frame for data export.
 *
 * Stores a Frame and the associated reception timestamp using steady_clock.
 * This is optimized for high-frequency data acquisition where timestamp
 * precision and low overhead are both important.
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
   *
   * @param f Frame data to copy into this timestamped frame
   */
  explicit TimestampedFrame(const DataModel::Frame& f) : data(f), timestamp(SteadyClock::now()) {}

  /**
   * @brief Constructs a timestamped frame by copying frame data and using an
   * explicit timestamp.
   *
   * @param f Frame data to copy into this timestamped frame
   * @param ts Timestamp to store with the frame
   */
  explicit TimestampedFrame(const DataModel::Frame& f, SteadyTimePoint ts) : data(f), timestamp(ts)
  {}

  /**
   * @brief Constructs a timestamped frame by moving frame data.
   *
   * @param f Frame data to move into this timestamped frame
   */
  explicit TimestampedFrame(DataModel::Frame&& f) noexcept
    : data(std::move(f)), timestamp(SteadyClock::now())
  {}

  /**
   * @brief Constructs a timestamped frame by moving frame data and using an
   * explicit timestamp.
   *
   * @param f Frame data to move into this timestamped frame
   * @param ts Timestamp to store with the frame
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
 * @typedef TimestampedFramePtr
 * @brief Shared pointer to a TimestampedFrame for efficient multi-consumer
 *        distribution.
 */
typedef std::shared_ptr<DataModel::TimestampedFrame> TimestampedFramePtr;

//--------------------------------------------------------------------------------------------------
// Generic utilities using C++20 concepts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generic JSON serialization for any Serializable type.
 *
 * Provides compile-time guarantee that T has a serialize() function.
 *
 * @tparam T Type satisfying the Serializable concept
 * @param obj Object to serialize
 * @return JSON representation of the object
 */
template<Concepts::Serializable T>
[[nodiscard]] inline QJsonObject toJson(const T& obj) noexcept
{
  return serialize(obj);
}

/**
 * @brief Generic JSON deserialization with validation for Serializable types.
 *
 * Provides compile-time guarantee that T has a read() function.
 * Returns std::optional for safer error handling.
 *
 * @tparam T Type satisfying the Serializable concept
 * @param json JSON object to deserialize
 * @return Optional containing the deserialized object if successful
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
 *
 * Checks that a Frame-like object has valid groups and actions.
 *
 * @tparam T Type satisfying the Frameable concept
 * @param frame Frame-like object to validate
 * @return true if frame has at least one group
 */
template<Concepts::Frameable T>
[[nodiscard]] constexpr bool isValidFrame(const T& frame) noexcept
{
  return !frame.groups.empty();
}

}  // namespace DataModel
