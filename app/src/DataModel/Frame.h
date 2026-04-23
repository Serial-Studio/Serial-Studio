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

[[nodiscard]] inline QVariant ss_jsr(const QJsonObject& object,
                                     const QString& key,
                                     const QVariant& defaultValue)
{
  if (object.contains(key))
    return object.value(key);

  return defaultValue;
}

enum class TimerMode {
  Off,
  AutoStart,
  StartOnTrigger,
  ToggleOnTrigger,
  RepeatNTimes
};

/**
 * @brief User-configured transmit action (fixed payload + optional timer).
 */
struct alignas(8) Action {
  int actionId              = -1;
  int sourceId              = 0;
  int repeatCount           = 3;
  int timerIntervalMs       = 100;
  int txEncoding            = 0;
  TimerMode timerMode       = TimerMode::Off;
  bool binaryData           = false;
  bool autoExecuteOnConnect = false;
  QString icon              = "Play Property";
  QString title;
  QString txData;
  QString eolSequence;
};

static_assert(sizeof(Action) % alignof(Action) == 0, "Unaligned Action struct");

QByteArray get_tx_bytes(const Action& action);

enum class OutputWidgetType : quint8 {
  Button,
  Slider,
  Toggle,
  TextField,
  Knob,
  RampGenerator,
};

/**
 * @brief Dashboard output widget definition driving transmit scripts.
 */
struct alignas(8) OutputWidget {
  int widgetId   = -1;
  int groupId    = -1;
  int sourceId   = 0;
  int txEncoding = 0;
  OutputWidgetType type = OutputWidgetType::Button;
  bool monoIcon         = false;
  double minValue       = 0;
  double maxValue       = 100;
  double stepSize       = 1;
  double initialValue   = 0;
  QString icon;
  QString title;
  QString transmitFunction;
};

static_assert(sizeof(OutputWidget) % alignof(OutputWidget) == 0, "Unaligned OutputWidget struct");

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
 * @brief Single numeric or string channel inside a group.
 */
struct alignas(8) Dataset {
  int index              = 0;
  int xAxisId            = -1;
  int groupId            = 0;
  int sourceId           = 0;
  int uniqueId           = 0;
  int datasetId          = 0;
  int fftSamples         = 256;
  int fftSamplingRate    = 100;
  bool fft               = false;
  bool led               = false;
  bool log               = false;
  bool plt               = false;
  bool alarmEnabled      = false;
  bool overviewDisplay   = false;
  bool isNumeric         = false;
  bool virtual_          = false;
  double fftMin          = 0;
  double fftMax          = 0;
  double pltMin          = 0;
  double pltMax          = 0;
  double wgtMin          = 0;
  double wgtMax          = 0;
  double ledHigh         = 80;
  double alarmLow        = 20;
  double alarmHigh       = 80;
  double numericValue    = 0;
  double rawNumericValue = 0;
  QString value;
  QString rawValue;
  QString title;
  QString units;
  QString widget;
  QString transformCode;
};

static_assert(sizeof(Dataset) % alignof(Dataset) == 0, "Unaligned Dataset struct");

enum class GroupType : quint8 {
  Input  = 0,
  Output = 1,
};

/**
 * @brief Collection of datasets and output widgets sharing a dashboard card.
 */
struct alignas(8) Group {
  int groupId         = -1;
  int sourceId        = 0;
  int columns         = 2;
  GroupType groupType = GroupType::Input;
  QString title;
  QString widget;
  std::vector<Dataset> datasets;
  std::vector<OutputWidget> outputWidgets;
  QString imgDetectionMode;
  QString imgStartSequence;
  QString imgEndSequence;
};

static_assert(sizeof(Group) % alignof(Group) == 0, "Unaligned Group struct");

/**
 * @brief Input source configuration (bus, delimiters, parser script).
 */
struct alignas(8) Source {
  int sourceId = 0;
  int busType  = 0;
  QString title;
  QString frameStart;
  QString frameEnd;
  QString checksumAlgorithm;
  int frameDetection         = 0;
  int decoderMethod          = 0;
  bool hexadecimalDelimiters = false;
  int frameParserLanguage    = 0;
  QJsonObject connectionSettings;
  QString frameParserCode;
};

static_assert(sizeof(Source) % alignof(Source) == 0, "Unaligned Source struct");

enum class RegisterType : quint8 {
  Constant = 0,
  Computed = 1,
  System   = 2,
};

/**
 * @brief Single register definition inside a user-defined data table.
 */
struct RegisterDef {
  QString name;
  RegisterType type = RegisterType::Constant;
  QVariant defaultValue;
};

/**
 * @brief User-defined data table (named collection of registers).
 */
struct TableDef {
  QString name;
  std::vector<RegisterDef> registers;
};

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
 * @brief Reference to a single dashboard widget inside a workspace.
 */
struct WidgetRef {
  int widgetType    = 0;
  int groupId       = -1;
  int relativeIndex = -1;
};

/**
 * @brief User-defined dashboard tab that groups selected widgets.
 */
struct Workspace {
  int workspaceId = -1;
  QString title;
  QString icon;
  std::vector<WidgetRef> widgetRefs;
};

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
 * @brief Top-level parsed frame: title, groups, and user actions.
 */
struct alignas(8) Frame {
  int sourceId = 0;
  QString title;
  std::vector<Group> groups;
  std::vector<Action> actions;
  bool containsCommercialFeatures = false;
};

static_assert(sizeof(Frame) % alignof(Frame) == 0, "Unaligned Frame struct");

inline void clear_frame(Frame& frame) noexcept
{
  frame.title.clear();
  frame.groups.clear();
  frame.actions.clear();
  frame.groups.shrink_to_fit();
  frame.actions.shrink_to_fit();
  frame.containsCommercialFeatures = false;
}

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

void finalize_frame(Frame& frame);

void read_io_settings(QByteArray& frameStart,
                      QByteArray& frameEnd,
                      QString& checksum,
                      const QJsonObject& obj);

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
 * @brief Frame paired with the steady-clock timestamp of the source chunk.
 */
struct TimestampedFrame {
  using SteadyClock     = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  DataModel::Frame data;
  SteadyTimePoint timestamp;

  TimestampedFrame() = default;

  explicit TimestampedFrame(const DataModel::Frame& f) : data(f), timestamp(SteadyClock::now()) {}

  explicit TimestampedFrame(const DataModel::Frame& f, SteadyTimePoint ts)
    : data(f), timestamp(ts)
  {}

  explicit TimestampedFrame(DataModel::Frame&& f) noexcept
    : data(std::move(f)), timestamp(SteadyClock::now())
  {}

  explicit TimestampedFrame(DataModel::Frame&& f, SteadyTimePoint ts) noexcept
    : data(std::move(f)), timestamp(ts)
  {}

  TimestampedFrame(TimestampedFrame&&) noexcept            = default;
  TimestampedFrame(const TimestampedFrame&)                = delete;
  TimestampedFrame& operator=(TimestampedFrame&&) noexcept = default;
  TimestampedFrame& operator=(const TimestampedFrame&)     = delete;
};

typedef std::shared_ptr<DataModel::TimestampedFrame> TimestampedFramePtr;

template<Concepts::Serializable T>
[[nodiscard]] inline QJsonObject toJson(const T& obj) noexcept
{
  return serialize(obj);
}

template<Concepts::Serializable T>
[[nodiscard]] inline std::optional<T> fromJson(const QJsonObject& json) noexcept
{
  T obj;
  if (read(obj, json))
    return obj;

  return std::nullopt;
}

template<Concepts::Frameable T>
[[nodiscard]] constexpr bool isValidFrame(const T& frame) noexcept
{
  return !frame.groups.empty();
}

}  // namespace DataModel
