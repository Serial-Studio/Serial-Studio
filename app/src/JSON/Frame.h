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

#include <cmath>
#include <vector>

#include <QString>
#include <QJsonArray>
#include <QJsonObject>

//------------------------------------------------------------------------------
// Standard keys for loading/offloading frame structures using JSON files
//------------------------------------------------------------------------------

namespace Keys
{
inline constexpr auto EOL = "eol";
inline constexpr auto Icon = "icon";
inline constexpr auto Title = "title";
inline constexpr auto TxData = "txData";
inline constexpr auto Binary = "binary";
inline constexpr auto TimerMode = "timerMode";
inline constexpr auto TimerInterval = "timerIntervalMs";
inline constexpr auto AutoExecute = "autoExecuteOnConnect";

inline constexpr auto FFT = "fft";
inline constexpr auto LED = "led";
inline constexpr auto Log = "log";
inline constexpr auto Min = "min";
inline constexpr auto Max = "max";
inline constexpr auto Graph = "graph";
inline constexpr auto Index = "index";
inline constexpr auto XAxis = "xAxis";
inline constexpr auto Alarm = "alarm";
inline constexpr auto Units = "units";
inline constexpr auto Value = "value";
inline constexpr auto Widget = "widget";
inline constexpr auto FFTMin = "fftMin";
inline constexpr auto FFTMax = "fftMax";
inline constexpr auto PltMin = "plotMin";
inline constexpr auto PltMax = "plotMax";
inline constexpr auto LedHigh = "ledHigh";
inline constexpr auto WgtMin = "widgetMin";
inline constexpr auto WgtMax = "widgetMax";
inline constexpr auto AlarmLow = "alarmLow";
inline constexpr auto AlarmHigh = "alarmHigh";
inline constexpr auto FFTSamples = "fftSamples";
inline constexpr auto Overview = "overviewDisplay";
inline constexpr auto AlarmEnabled = "alarmEnabled";
inline constexpr auto FFTSamplingRate = "fftSamplingRate";

inline constexpr auto Groups = "groups";
inline constexpr auto Actions = "actions";
inline constexpr auto Datasets = "datasets";
} // namespace Keys

namespace JSON
{

//------------------------------------------------------------------------------
// Action structure
//------------------------------------------------------------------------------

/**
 * @brief Timer mode for an Action.
 */
enum class TimerMode
{
  Off,            ///< No timer
  AutoStart,      ///< Starts timer automatically (e.g. on connection)
  StartOnTrigger, ///< Starts timer when the action is triggered
  ToggleOnTrigger ///< Toggles timer state with each trigger
};

/**
 * @brief Represents a user-defined action or command to send over serial.
 */
struct alignas(8) Action
{
  int actionId = -1;                    ///< Unique action ID
  int timerIntervalMs = 100;            ///< Timer interval in ms
  TimerMode timerMode = TimerMode::Off; ///< Timer behavior mode
  bool binaryData = false;              ///< If true, txData is binary
  bool autoExecuteOnConnect = false;    ///< Auto execute on connect
  QString icon = "Play Property";       ///< Action icon name or path
  QString title;                        ///< Display title
  QString txData;                       ///< Data to transmit
  QString eolSequence;                  ///< End-of-line sequence (e.g. "\r\n")
};
static_assert(sizeof(Action) % alignof(Action) == 0, "Unaligned Action struct");

/**
 * @brief Generates the raw byte sequence to transmit for a given Action.
 *
 * This function resolves the action’s `txData` and `eolSequence` fields
 * into a final `QByteArray` suitable for serial transmission.
 *
 * This function exists outside the Frame definition to avoid circular
 * dependencies between `Frame.h` and `SerialStudio.h`.
 *
 * @param action The Action to generate the transmission bytes from.
 * @return QByteArray containing the resolved byte stream to transmit.
 */
QByteArray get_tx_bytes(const Action &action);

//------------------------------------------------------------------------------
// Dataset structure
//------------------------------------------------------------------------------

/**
 * @brief Represents a single unit of sensor data with optional metadata and
 * graphing. Fully aligned and stack-optimized.
 */
struct alignas(8) Dataset
{
  int index = 0;                ///< Frame offset index
  int xAxisId = -1;             ///< Optional reference to x-axis dataset
  int groupId = 0;              ///< Owning group ID
  int uniqueId = 0;             ///< Unique ID within frame
  int datasetId = 0;            ///< Unique ID within group
  int fftSamples = 256;         ///< Number of samples for FFT
  int fftSamplingRate = 100;    ///< Sampling rate for FFT
  bool fft = false;             ///< Enables FFT processing
  bool led = false;             ///< Enables LED widget
  bool log = false;             ///< Enables logging
  bool plt = false;             ///< Enables plotting
  bool alarmEnabled = false;    ///< Enable/disable alarm values
  bool overviewDisplay = false; ///< Show in overview
  bool isNumeric = false;       ///< True if value was parsed as numeric
  double fftMin = 0;            ///< Minimum value (for FFT)
  double fftMax = 0;            ///< Maximum value (for FFT)
  double pltMin = 0;            ///< Minimum value (for plots)
  double pltMax = 0;            ///< Maximum value (for plots)
  double wgtMin = 0;            ///< Minimum value (for widgets)
  double wgtMax = 100;          ///< Maximum value (for widgets)
  double ledHigh = 80;          ///< LED activation threshold
  double alarmLow = 20;         ///< Low alarm threshold
  double alarmHigh = 80;        ///< High alarm threshold
  double numericValue = 0;      ///< Parsed numeric value
  QString value;                ///< Raw string value
  QString title;                ///< Human-readable title
  QString units;                ///< Measurement units (e.g., °C)
  QString widget;               ///< Widget type (bar, gauge, etc.)
};
static_assert(sizeof(Dataset) % alignof(Dataset) == 0,
              "Unaligned Dataset struct");

//------------------------------------------------------------------------------
// Group structure
//------------------------------------------------------------------------------

/**
 * @brief Represents a collection of datasets that are related (e.g., for a
 * specific sensor).
 */
struct alignas(8) Group
{
  int groupId = -1;              ///< Unique group identifier
  QString title;                 ///< Group display name
  QString widget;                ///< Group widget type
  std::vector<Dataset> datasets; ///< Datasets contained in this group
};
static_assert(sizeof(Group) % alignof(Group) == 0, "Unaligned Group struct");

//------------------------------------------------------------------------------
// Frame structure
//------------------------------------------------------------------------------

/**
 * @brief Represents a full data frame, including groups and actions.
 *        This is the root structure for each UI update.
 */
struct alignas(8) Frame
{
  QString title;                           ///< Frame title
  std::vector<Group> groups;               ///< Sensor groups in this frame
  std::vector<Action> actions;             ///< Triggerable actions
  bool containsCommercialFeatures = false; ///< Feature gating flag
};
static_assert(sizeof(Frame) % alignof(Frame) == 0, "Unaligned Frame struct");

//------------------------------------------------------------------------------
// Frame utilities and post-processing
//------------------------------------------------------------------------------

/**
 * @brief Clears and resets a Frame object to its default state.
 *
 * This utility function performs a full reset of a Frame by:
 * - Clearing the frame title string.
 * - Clearing all groups and actions.
 * - Releasing memory held by groups and actions via `shrink_to_fit()`.
 * - Resetting `containsCommercialFeatures` to `false`.
 *
 * Use this to recycle or reuse an existing Frame instance without reallocating.
 *
 * @param frame The Frame object to be cleared and reset.
 */
inline void clear_frame(Frame &frame)
{
  frame.title.clear();
  frame.groups.clear();
  frame.actions.clear();
  frame.groups.shrink_to_fit();
  frame.actions.shrink_to_fit();
  frame.containsCommercialFeatures = false;
}

/**
 * @brief Compares two frames for structural equivalence.
 *
 * This function checks whether two Frame instances have the same group count,
 * matching group IDs, and identical dataset `index` values within each group.
 *
 * It does not compare actions, titles, or any other dataset fields beyond
 * `index`.
 *
 * @param a First frame to compare.
 * @param b Second frame to compare.
 * @return true if both frames are structurally equivalent; false otherwise.
 */
inline bool compare_frames(const Frame &a, const Frame &b)
{
  if (a.groups.size() != b.groups.size())
    return false;

  const auto &groupsA = a.groups;
  const auto &groupsB = b.groups;

  for (size_t i = 0, gc = groupsA.size(); i < gc; ++i)
  {
    const auto &g1 = groupsA[i];
    const auto &g2 = groupsB[i];

    if (g1.groupId != g2.groupId)
      return false;

    const auto &datasetsA = g1.datasets;
    const auto &datasetsB = g2.datasets;

    const size_t dc = datasetsA.size();
    if (dc != datasetsB.size())
      return false;

    for (size_t j = 0; j < dc; ++j)
    {
      if (datasetsA[j].index != datasetsB[j].index)
        return false;
    }
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
void finalize_frame(Frame &frame);

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
void read_io_settings(QByteArray &frameStart, QByteArray &frameEnd,
                      QString &checksum, const QJsonObject &obj);

//------------------------------------------------------------------------------
// Data -> JSON serialization
//------------------------------------------------------------------------------

/**
 * @brief Serializes an Action to a QJsonObject.
 *
 * Converts an Action object into a JSON representation containing:
 * - `icon`: The icon associated with the action.
 * - `title`: The display title of the action.
 * - `txData`: The data to transmit.
 * - `eol`: The end-of-line sequence (e.g., "\\r\\n").
 * - `binary`: Whether the data should be interpreted as binary.
 * - `timerIntervalMs`: Timer interval in milliseconds.
 * - `timerMode`: Integer value representing the timer mode enum.
 * - `autoExecuteOnConnect`: Whether to auto-execute this action on device
 *                           connection.
 *
 * @param a The Action object to serialize.
 * @return QJsonObject representing the Action.
 */
[[nodiscard]] inline QJsonObject serialize(const Action &a)
{
  QJsonObject obj;
  obj.insert(Keys::Icon, a.icon);
  obj.insert(Keys::Title, a.title);
  obj.insert(Keys::TxData, a.txData);
  obj.insert(Keys::EOL, a.eolSequence);
  obj.insert(Keys::Binary, a.binaryData);
  obj.insert(Keys::TimerInterval, a.timerIntervalMs);
  obj.insert(Keys::AutoExecute, a.autoExecuteOnConnect);
  obj.insert(Keys::TimerMode, static_cast<int>(a.timerMode));
  return obj;
}

/**
 * @brief Serializes a Dataset to a QJsonObject.
 *
 * Converts a Dataset object into a JSON structure including:
 * - Flags: `fft`, `led`, `log`, `graph`, `overviewDisplay`
 * - Indices: `index`, `xAxis`
 * - Thresholds: `ledHigh`, `alarmLow`, `alarmHigh`
 * - Limits: `min`, `max` (automatically ordered via qMin/qMax)
 * - Metadata: `title`, `value`, `units`, `widget`, `fftWindow`
 * - FFT settings: `fftSamples`, `fftSamplingRate`
 *
 * All QString fields are simplified (trimmed and collapsed whitespace).
 *
 * @param d The Dataset object to serialize.
 * @return QJsonObject representing the Dataset.
 */
[[nodiscard]] inline QJsonObject serialize(const Dataset &d)
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
  obj.insert(Keys::Overview, d.overviewDisplay);
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
  return obj;
}

/**
 * @brief Serializes a Group to a QJsonObject.
 *
 * Converts a Group and all of its Datasets into a JSON structure:
 * - `title`: Group display name (simplified)
 * - `widget`: Widget type for visual display (simplified)
 * - `datasets`: Array of serialized Datasets
 *
 * @param g The Group object to serialize.
 * @return QJsonObject representing the Group.
 */
[[nodiscard]] inline QJsonObject serialize(const Group &g)
{
  QJsonArray datasetArray;
  for (const auto &dataset : g.datasets)
    datasetArray.append(serialize(dataset));

  QJsonObject obj;
  obj.insert(Keys::Datasets, datasetArray);
  obj.insert(Keys::Title, g.title.simplified());
  obj.insert(Keys::Widget, g.widget.simplified());
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
[[nodiscard]] inline QJsonObject serialize(const Frame &f)
{
  QJsonArray groupArray;
  for (const auto &group : f.groups)
    groupArray.append(serialize(group));

  QJsonArray actionArray;
  for (const auto &action : f.actions)
    actionArray.append(serialize(action));

  QJsonObject obj;
  obj.insert(Keys::Title, f.title);
  obj.insert(Keys::Groups, groupArray);
  obj.insert(Keys::Actions, actionArray);
  return obj;
}

//------------------------------------------------------------------------------
// Utility functions for data deserialization
//------------------------------------------------------------------------------

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
[[nodiscard]] inline QVariant ss_jsr(const QJsonObject &object,
                                     const QString &key,
                                     const QVariant &defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

//------------------------------------------------------------------------------
// Data deserialization
//------------------------------------------------------------------------------

/**
 * @brief Deserializes an Action from a QJsonObject.
 *
 * Parses fields from JSON into an Action structure, including:
 * - `txData`: Transmit data
 * - `eol`: End-of-line sequence
 * - `binary`: Binary mode flag
 * - `icon`: Icon path or name
 * - `title`: Display name
 * - `timerIntervalMs`: Timer interval in milliseconds
 * - `autoExecuteOnConnect`: Whether to trigger action on connection
 * - `timerMode`: TimerMode enum (validated and cast safely)
 *
 * @param a Output Action object to populate.
 * @param obj JSON object to read from.
 * @return true if valid and successfully parsed, false otherwise.
 */
[[nodiscard]] inline bool read(Action &a, const QJsonObject &obj)
{
  if (obj.isEmpty())
    return false;

  a.txData = ss_jsr(obj, Keys::TxData, "").toString();
  a.eolSequence = ss_jsr(obj, Keys::EOL, "").toString();
  a.binaryData = ss_jsr(obj, Keys::Binary, false).toBool();
  a.icon = ss_jsr(obj, Keys::Icon, "").toString().simplified();
  a.title = ss_jsr(obj, Keys::Title, "").toString().simplified();
  a.timerIntervalMs = ss_jsr(obj, Keys::TimerInterval, 100).toInt();
  a.autoExecuteOnConnect = ss_jsr(obj, Keys::AutoExecute, false).toBool();

  const int mode = ss_jsr(obj, Keys::TimerMode, 0).toInt();
  if (mode >= 0 && mode <= 3)
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
 * - FFT settings: `fftSamples`, `fftSamplingRate`, `fftWindow`
 * - Display info: `title`, `value`, `units`, `widget`
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
[[nodiscard]] inline bool read(Dataset &d, const QJsonObject &obj)
{
  if (obj.isEmpty())
    return false;

  d.index = ss_jsr(obj, Keys::Index, -1).toInt();
  d.fft = ss_jsr(obj, Keys::FFT, false).toBool();
  d.led = ss_jsr(obj, Keys::LED, false).toBool();
  d.log = ss_jsr(obj, Keys::Log, false).toBool();
  d.plt = ss_jsr(obj, Keys::Graph, false).toBool();
  d.xAxisId = ss_jsr(obj, Keys::XAxis, -1).toInt();
  d.fftMin = ss_jsr(obj, Keys::FFTMin, 0).toDouble();
  d.fftMax = ss_jsr(obj, Keys::FFTMax, 0).toDouble();
  d.pltMin = ss_jsr(obj, Keys::PltMin, 0).toDouble();
  d.pltMax = ss_jsr(obj, Keys::PltMax, 0).toDouble();
  d.wgtMin = ss_jsr(obj, Keys::WgtMin, 0).toDouble();
  d.wgtMax = ss_jsr(obj, Keys::WgtMax, 0).toDouble();
  d.fftSamples = ss_jsr(obj, Keys::FFTSamples, -1).toInt();
  d.title = ss_jsr(obj, Keys::Title, "").toString().simplified();
  d.value = ss_jsr(obj, Keys::Value, "").toString().simplified();
  d.units = ss_jsr(obj, Keys::Units, "").toString().simplified();
  d.overviewDisplay = ss_jsr(obj, Keys::Overview, false).toBool();
  d.alarmEnabled = ss_jsr(obj, Keys::AlarmEnabled, false).toBool();
  d.ledHigh = ss_jsr(obj, Keys::LedHigh, 0).toDouble();
  d.widget = ss_jsr(obj, Keys::Widget, "").toString().simplified();
  d.alarmLow = ss_jsr(obj, Keys::AlarmLow, 0).toDouble();
  d.fftSamplingRate = ss_jsr(obj, Keys::FFTSamplingRate, -1).toInt();
  d.alarmHigh = ss_jsr(obj, Keys::AlarmHigh, 0).toDouble();
  if (d.value.isEmpty())
    d.value = QStringLiteral("--.--");
  else
    d.numericValue = d.value.toDouble(&d.isNumeric);

  if (!obj.contains(Keys::FFTMin) || !obj.contains(Keys::FFTMax))
  {
    d.fftMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.fftMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  if (!obj.contains(Keys::PltMin) || !obj.contains(Keys::PltMax))
  {
    d.pltMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.pltMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  if (!obj.contains(Keys::WgtMin) || !obj.contains(Keys::WgtMax))
  {
    d.wgtMin = ss_jsr(obj, Keys::Min, 0).toDouble();
    d.wgtMax = ss_jsr(obj, Keys::Max, 0).toDouble();
  }

  if (obj.contains(Keys::Alarm))
  {
    if (std::isnan(d.alarmHigh) && std::isnan(d.alarmLow))
    {
      auto alarm = ss_jsr(obj, Keys::Alarm, 0).toDouble();
      d.alarmHigh = alarm;
    }
  }

  if (!std::isnan(d.fftMin) && !std::isnan(d.fftMax))
  {
    d.fftMin = qMin(d.fftMin, d.fftMax);
    d.fftMax = qMax(d.fftMin, d.fftMax);
  }

  if (!std::isnan(d.pltMin) && !std::isnan(d.pltMax))
  {
    d.pltMin = qMin(d.pltMin, d.pltMax);
    d.pltMax = qMax(d.pltMin, d.pltMax);
  }

  if (!std::isnan(d.wgtMin) && !std::isnan(d.wgtMax))
  {
    d.wgtMin = qMin(d.wgtMin, d.wgtMax);
    d.wgtMax = qMax(d.wgtMin, d.wgtMax);
  }

  return true;
}

/**
 * @brief Deserializes a Group from a QJsonObject.
 *
 * Reads a group’s metadata and its dataset array. Each dataset is deserialized
 * and automatically assigned:
 * - `datasetId` based on array index
 * - `groupId` inherited from parent group
 *
 * Expects:
 * - `title`: Group title
 * - `widget`: Widget type
 * - `datasets`: Array of Dataset objects
 *
 * @param g Output Group object to populate.
 * @param obj JSON object representing a group.
 * @return true if group and datasets were valid and parsed correctly.
 */
[[nodiscard]] inline bool read(Group &g, const QJsonObject &obj)
{
  if (obj.isEmpty())
    return false;

  const auto array = obj.value(Keys::Datasets).toArray();
  const auto title = ss_jsr(obj, Keys::Title, "").toString().simplified();
  const auto widget = ss_jsr(obj, Keys::Widget, "").toString().simplified();

  if (!title.isEmpty() && !array.isEmpty())
  {
    g.title = title;
    g.widget = widget;
    g.datasets.clear();
    g.datasets.reserve(array.count());

    bool ok = true;
    for (qsizetype i = 0; i < array.count(); ++i)
    {
      const auto dObj = array[i].toObject();
      if (!dObj.isEmpty())
      {
        Dataset dataset;
        ok &= read(dataset, dObj);

        if (ok)
        {
          dataset.datasetId = i;
          dataset.groupId = g.groupId;
          g.datasets.push_back(dataset);
        }

        else
          break;
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
 * `containsCommercialFeatures` and assign Dataset unique IDs.
 *
 * @param f Output Frame object to populate.
 * @param obj JSON object representing the frame.
 * @return true if the frame is valid and successfully parsed.
 */
[[nodiscard]] inline bool read(Frame &f, const QJsonObject &obj)
{
  if (obj.isEmpty())
    return false;

  const auto groups = obj.value(Keys::Groups).toArray();
  const auto actions = obj.value(Keys::Actions).toArray();
  const auto title = ss_jsr(obj, Keys::Title, "").toString().simplified();

  if (!title.isEmpty() && !groups.isEmpty())
  {
    f.title = title;
    f.groups.clear();
    f.actions.clear();
    f.groups.reserve(groups.count());
    f.actions.reserve(actions.count());

    bool ok = true;
    for (qsizetype i = 0; i < groups.count(); ++i)
    {
      Group group;
      group.groupId = i;
      ok &= read(group, groups[i].toObject());
      if (ok)
        f.groups.push_back(group);
      else
        break;
    }

    if (ok)
    {
      for (qsizetype i = 0; i < actions.count(); ++i)
      {
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

} // namespace JSON
