/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Importers/DBCImporter.h"

#include <cmath>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include "DataModel/Frame.h"
#include "DataModel/Importers/AxisTicks.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the DBCImporter singleton.
 */
DataModel::DBCImporter::DBCImporter() : m_skippedExtendedMuxSignals(0) {}

/**
 * @brief Returns the singleton DBCImporter instance.
 */
DataModel::DBCImporter& DataModel::DBCImporter::instance()
{
  static DBCImporter instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of signals across all loaded messages.
 */
int DataModel::DBCImporter::signalCount() const
{
  return countTotalSignals(m_messages);
}

/**
 * @brief Returns the number of messages loaded from the DBC file.
 */
int DataModel::DBCImporter::messageCount() const
{
  return m_messages.count();
}

/**
 * @brief Returns the file name of the loaded DBC file.
 */
QString DataModel::DBCImporter::dbcFileName() const
{
  return QFileInfo(m_dbcFilePath).fileName();
}

/**
 * @brief Returns a formatted "N: Name @ 0xID (X signals)" string.
 */
QString DataModel::DBCImporter::messageInfo(int index) const
{
  if (index < 0 || index >= m_messages.count())
    return QString();

  const auto& message = m_messages.at(index);
  const auto msgId    = static_cast<quint32>(message.uniqueId());

  int signalCount = 0;
  for (const auto& signal : message.signalDescriptions()) {
    qint64 mv       = 0;
    const auto role = classifyMux(signal, message, mv);
    if (role != MuxRole::ExtendedMuxed)
      ++signalCount;
  }

  return QString("%1: %2 @ 0x%3 (%4 signals)")
    .arg(index + 1)
    .arg(message.name())
    .arg(QString::number(msgId, 16).toUpper())
    .arg(signalCount);
}

//--------------------------------------------------------------------------------------------------
// User interface operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a file dialog to select a DBC file and preview it.
 */
void DataModel::DBCImporter::importDBC()
{
  const auto p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  auto* dialog = new QFileDialog(
    qApp->activeWindow(), tr("Import DBC File"), p, tr("DBC Files (*.dbc);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { showPreview(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Cancels the current import and clears loaded data.
 */
void DataModel::DBCImporter::cancelImport()
{
  m_messages.clear();
  m_dbcFilePath.clear();
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
}

/**
 * @brief Parses the DBC file at filePath and emits previewReady on success.
 */
void DataModel::DBCImporter::showPreview(const QString& filePath)
{
  QCanDbcFileParser parser;
  if (!parser.parse(filePath)) {
    Misc::Utilities::showMessageBox(tr("Failed to parse DBC file: %1").arg(parser.errorString()),
                                    tr("Verify the file format and try again."),
                                    QMessageBox::Critical,
                                    tr("DBC Import Error"));
    return;
  }

  m_messages = parser.messageDescriptions();
  if (m_messages.isEmpty()) {
    Misc::Utilities::showMessageBox(
      tr("DBC file contains no messages"),
      tr("The selected file does not contain any CAN message definitions."),
      QMessageBox::Warning,
      tr("DBC Import Warning"));
    return;
  }

  std::sort(m_messages.begin(),
            m_messages.end(),
            [](const QCanMessageDescription& a, const QCanMessageDescription& b) {
              return a.uniqueId() < b.uniqueId();
            });

  m_dbcFilePath = filePath;
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
  Q_EMIT previewReady();
}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a Serial Studio project from the parsed DBC messages.
 */
void DataModel::DBCImporter::confirmImport()
{
  if (m_messages.isEmpty())
    return;

  const auto project       = generateProject(m_messages);
  const QString suggestion = QFileInfo(m_dbcFilePath).baseName();

  const int messageCount  = m_messages.count();
  const int signalCount   = countTotalSignals(m_messages);
  const int skippedExtMux = m_skippedExtendedMuxSignals;

  auto& pm = ProjectModel::instance();
  QObject::connect(
    &pm,
    &ProjectModel::importCompleted,
    this,
    [messageCount, signalCount, skippedExtMux](bool accepted, const QString&) {
      if (!accepted)
        return;

      QString detail = tr("The project editor is now open for customization.");
      if (skippedExtMux > 0)
        detail += tr(" Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); "
                     "only simple multiplexing is supported.")
                    .arg(skippedExtMux);

      Misc::Utilities::showMessageBox(
        tr("Successfully imported DBC file with %1 messages and %2 signals.")
          .arg(messageCount)
          .arg(signalCount),
        detail,
        QMessageBox::Information,
        tr("DBC Import Complete"));
    },
    Qt::SingleShotConnection);

  pm.importProjectFromJson(project, suggestion);
}

/**
 * @brief Builds a complete .ssproj QJsonObject from the DBC messages.
 */
QJsonObject DataModel::DBCImporter::generateProject(const QList<QCanMessageDescription>& messages)
{
  QJsonObject project;

  const auto dbcInfo      = QFileInfo(m_dbcFilePath);
  const auto projectTitle = dbcInfo.baseName();

  project[Keys::Title]   = projectTitle;
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = tr("CAN Bus");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::CanBus);
  source[Keys::FrameStart]            = QString("");
  source[Keys::FrameEnd]              = QString("");
  source[Keys::Checksum]              = QString("");
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = QString("");
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Native);
  source[Keys::FrameParserTemplate]   = QStringLiteral("can_signal_map");
  source[Keys::FrameParserParams]     = generateNativeParserParams(messages);

  project[Keys::Sources] = QJsonArray{source};

  const auto groups = generateGroups(messages);
  QJsonArray groupArray;
  for (const auto& group : groups)
    groupArray.append(serialize(group));

  project[Keys::Groups] = groupArray;

  return project;
}

/**
 * @brief Builds a Dataset for a CAN signal, choosing widget and ranges sensibly.
 */
DataModel::Dataset DataModel::DBCImporter::buildDatasetFromSignal(
  const QCanSignalDescription& signal,
  const QString& groupWidget,
  int datasetIndex,
  MuxRole role,
  qint64 muxValue)
{
  DataModel::Dataset dataset;
  dataset.index = datasetIndex;
  dataset.units = signal.physicalUnit();

  if (role == MuxRole::SimpleMuxed)
    dataset.title = QString("%1 (mux %2)").arg(signal.name()).arg(muxValue);
  else if (role == MuxRole::Selector)
    dataset.title = QString("%1 (selector)").arg(signal.name());
  else
    dataset.title = signal.name();

  double minVal = signal.minimum();
  double maxVal = signal.maximum();
  if (std::isnan(minVal) || std::isnan(maxVal) || minVal >= maxVal) {
    minVal = 0.0;
    maxVal = 100.0;
  }

  dataset.pltMin = minVal;
  dataset.pltMax = maxVal;
  dataset.fftMin = minVal;
  dataset.fftMax = maxVal;

  const auto nice          = niceAxisTicks(minVal, maxVal);
  dataset.wgtMin           = nice.min;
  dataset.wgtMax           = nice.max;
  dataset.displayTickCount = nice.tickCount;
  dataset.displayFormat    = isIntegerSignal(signal) ? QStringLiteral("0d") : QStringLiteral("1d");

  const auto isSingleBit = (signal.bitLength() == 1);
  if (isSingleBit) {
    dataset.widget  = QString("");
    dataset.plt     = false;
    dataset.fft     = false;
    dataset.led     = true;
    dataset.ledHigh = 1;
    dataset.log     = true;
    return dataset;
  }

  if (shouldAssignIndividualWidget(groupWidget, signal, isSingleBit))
    dataset.widget = selectWidgetForSignal(signal);
  else
    dataset.widget = QString("");

  dataset.plt     = false;
  dataset.fft     = false;
  dataset.led     = false;
  dataset.ledHigh = 0;
  dataset.log     = true;
  return dataset;
}

/**
 * @brief Builds Group/Dataset structures from the DBC messages.
 */
std::vector<DataModel::Group> DataModel::DBCImporter::generateGroups(
  const QList<QCanMessageDescription>& messages)
{
  std::vector<DataModel::Group> groups;
  int groupId      = 0;
  int datasetIndex = 1;

  for (const auto& message : messages) {
    if (!hasImportableSignals(message))
      continue;

    DataModel::Group group;
    group.groupId = groupId;
    group.title   = message.name();
    group.widget  = selectGroupWidget(message);

    const auto signalList    = message.signalDescriptions();
    const auto selectorIndex = findSelectorIndex(message);
    if (selectorIndex >= 0)
      group.datasets.push_back(buildDatasetFromSignal(
        signalList.at(selectorIndex), group.widget, datasetIndex++, MuxRole::Selector, 0));

    for (const auto& signal : message.signalDescriptions()) {
      qint64 muxValue = 0;
      const auto role = classifyMux(signal, message, muxValue);
      if (role != MuxRole::Plain)
        continue;

      group.datasets.push_back(
        buildDatasetFromSignal(signal, group.widget, datasetIndex++, role, muxValue));
    }

    for (const auto& signal : message.signalDescriptions()) {
      qint64 muxValue = 0;
      const auto role = classifyMux(signal, message, muxValue);
      if (role != MuxRole::SimpleMuxed)
        continue;

      group.datasets.push_back(
        buildDatasetFromSignal(signal, group.widget, datasetIndex++, role, muxValue));
    }

    groups.push_back(group);
    ++groupId;
  }

  return groups;
}

/**
 * @brief Builds the native can_signal_map template params from the DBC messages.
 */
QJsonObject DataModel::DBCImporter::generateNativeParserParams(
  const QList<QCanMessageDescription>& messages)
{
  m_skippedExtendedMuxSignals = 0;
  for (const auto& message : messages) {
    for (const auto& signal : message.signalDescriptions()) {
      qint64 mv       = 0;
      const auto role = classifyMux(signal, message, mv);
      if (role == MuxRole::ExtendedMuxed)
        ++m_skippedExtendedMuxSignals;
    }
  }

  QJsonArray messages_json;
  for (const auto& message : messages) {
    if (!hasImportableSignals(message))
      continue;

    const auto signalList = message.signalDescriptions();
    QJsonArray signals_json;

    const auto selectorIndex = findSelectorIndex(message);
    if (selectorIndex >= 0)
      signals_json.append(signalToJson(signalList.at(selectorIndex), MuxRole::Selector, 0));

    for (const auto& signal : signalList) {
      qint64 mv       = 0;
      const auto role = classifyMux(signal, message, mv);
      if (role == MuxRole::Plain)
        signals_json.append(signalToJson(signal, role, mv));
    }

    for (const auto& signal : signalList) {
      qint64 mv       = 0;
      const auto role = classifyMux(signal, message, mv);
      if (role == MuxRole::SimpleMuxed)
        signals_json.append(signalToJson(signal, role, mv));
    }

    QJsonObject message_json;
    message_json.insert(QStringLiteral("id"),
                        static_cast<qint64>(static_cast<quint32>(message.uniqueId())));
    message_json.insert(QStringLiteral("signals"), signals_json);
    messages_json.append(message_json);
  }

  QJsonObject params;
  params.insert(QStringLiteral("messages"), messages_json);
  return params;
}

/**
 * @brief Serializes one CAN signal's bit layout, scaling and multiplex role.
 */
QJsonObject DataModel::DBCImporter::signalToJson(const QCanSignalDescription& signal,
                                                 MuxRole role,
                                                 qint64 muxValue)
{
  QJsonObject json;
  json.insert(QStringLiteral("startBit"), signal.startBit());
  json.insert(QStringLiteral("length"), signal.bitLength());
  json.insert(QStringLiteral("bigEndian"), signal.dataEndian() == QSysInfo::BigEndian);
  json.insert(QStringLiteral("signed"), signal.dataFormat() == QtCanBus::DataFormat::SignedInteger);
  json.insert(QStringLiteral("factor"), signal.factor());
  json.insert(QStringLiteral("offset"), signal.offset());

  if (role == MuxRole::Selector)
    json.insert(QStringLiteral("selector"), true);
  else if (role == MuxRole::SimpleMuxed)
    json.insert(QStringLiteral("mux"), muxValue);

  return json;
}

//--------------------------------------------------------------------------------------------------
// Signal processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the message contributes at least one non-extended-mux signal.
 */
bool DataModel::DBCImporter::hasImportableSignals(const QCanMessageDescription& message) const
{
  for (const auto& signal : message.signalDescriptions()) {
    qint64 mv       = 0;
    const auto role = classifyMux(signal, message, mv);
    if (role != MuxRole::ExtendedMuxed)
      return true;
  }

  return false;
}

/**
 * @brief Returns the index of the message's top-level MultiplexorSwitch signal, or -1.
 */
int DataModel::DBCImporter::findSelectorIndex(const QCanMessageDescription& message) const
{
  const auto signalList = message.signalDescriptions();
  for (qsizetype i = 0; i < signalList.size(); ++i)
    if (signalList.at(i).multiplexState() == QtCanBus::MultiplexState::MultiplexorSwitch)
      return static_cast<int>(i);

  return -1;
}

/**
 * @brief Classifies a signal's multiplex role and extracts the mux value when simple.
 */
DataModel::DBCImporter::MuxRole DataModel::DBCImporter::classifyMux(
  const QCanSignalDescription& signal,
  const QCanMessageDescription& message,
  qint64& outMuxValue) const
{
  outMuxValue = 0;

  const auto state = signal.multiplexState();
  if (state == QtCanBus::MultiplexState::None)
    return MuxRole::Plain;

  if (state == QtCanBus::MultiplexState::MultiplexorSwitch)
    return MuxRole::Selector;

  if (state == QtCanBus::MultiplexState::SwitchAndSignal)
    return MuxRole::ExtendedMuxed;

  const auto parents = signal.multiplexSignals();
  if (parents.isEmpty())
    return MuxRole::Plain;

  if (parents.size() > 1)
    return MuxRole::ExtendedMuxed;

  const auto parentName    = parents.keys().first();
  const auto selectorIndex = findSelectorIndex(message);
  if (selectorIndex < 0)
    return MuxRole::ExtendedMuxed;

  const auto signalList = message.signalDescriptions();
  if (signalList.at(selectorIndex).name() != parentName)
    return MuxRole::ExtendedMuxed;

  const auto ranges = parents.value(parentName);
  if (ranges.size() != 1)
    return MuxRole::ExtendedMuxed;

  const auto& range = ranges.first();
  if (range.minimum != range.maximum)
    return MuxRole::ExtendedMuxed;

  bool ok      = false;
  const auto v = range.minimum.toLongLong(&ok);
  if (!ok)
    return MuxRole::ExtendedMuxed;

  outMuxValue = v;
  return MuxRole::SimpleMuxed;
}

//--------------------------------------------------------------------------------------------------
// Widget selection helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a group widget type based on the message's signal family.
 */
QString DataModel::DBCImporter::selectGroupWidget(const QCanMessageDescription& message)
{
  const auto signalDescriptions = message.signalDescriptions();
  const auto signalCount        = signalDescriptions.count();

  if (signalCount == 1)
    return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

  bool allBoolean = true;
  for (const auto& signal : signalDescriptions) {
    if (signal.bitLength() != 1) {
      allBoolean = false;
      break;
    }
  }

  if (allBoolean)
    return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

  const auto family   = detectSignalFamily(signalDescriptions);
  const auto familyId = resolveSignalFamilyWidget(family, signalCount, signalDescriptions);
  if (!familyId.isEmpty())
    return familyId;

  const auto gpsId = detectGpsWidget(signalDescriptions);
  if (!gpsId.isEmpty())
    return gpsId;

  const auto motionId = detectMotionWidget(signalDescriptions);
  if (!motionId.isEmpty())
    return motionId;

  if (signalCount >= 2 && signalCount <= 8) {
    const int plottableCount = countPlottable(signalDescriptions);
    if (plottableCount >= 2)
      return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);
  }

  return SerialStudio::groupWidgetId(SerialStudio::DataGrid);
}

/**
 * @brief Returns a widget id for an explicitly recognised signal family, or empty if unmatched.
 */
QString DataModel::DBCImporter::resolveSignalFamilyWidget(
  SignalFamily family,
  int signalCount,
  const QList<QCanSignalDescription>& signalDescriptions) const
{
  switch (family) {
    case WheelSpeeds:
    case TirePressures:
      return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);

    case Temperatures:
    case Voltages:
      if (signalCount <= 8)
        return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);

      return SerialStudio::groupWidgetId(SerialStudio::DataGrid);

    case BatteryCluster:
      return SerialStudio::groupWidgetId(SerialStudio::DataGrid);

    case StatusFlags:
      return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

    case GenericRelated:
      if (signalCount >= 2 && signalCount <= 8) {
        const int plottableCount = countPlottable(signalDescriptions);
        if (plottableCount >= 2)
          return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);
      }
      break;

    case None:
    default:
      break;
  }

  return QString();
}

/**
 * @brief Returns the GPS widget id when the message carries lat/lon-style signals.
 */
QString DataModel::DBCImporter::detectGpsWidget(
  const QList<QCanSignalDescription>& signalDescriptions) const
{
  if (signalDescriptions.size() < 2)
    return QString();

  bool hasLat = false;
  bool hasLon = false;

  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    if (name.contains("lat"))
      hasLat = true;

    if (name.contains("lon") || name.contains("lng"))
      hasLon = true;
  }

  if (hasLat && hasLon)
    return SerialStudio::groupWidgetId(SerialStudio::GPS);

  return QString();
}

/**
 * @brief Returns Accelerometer/Gyroscope when the 3-signal message names match an IMU pattern.
 */
QString DataModel::DBCImporter::detectMotionWidget(
  const QList<QCanSignalDescription>& signalDescriptions) const
{
  if (signalDescriptions.size() != 3)
    return QString();

  int accelCount = 0;
  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    const auto unit = signal.physicalUnit().toLower();
    if (name.contains("accel") || unit.contains("m/s") || unit.contains("g"))
      accelCount++;
  }

  if (accelCount == 3)
    return SerialStudio::groupWidgetId(SerialStudio::Accelerometer);

  int gyroCount = 0;
  for (const auto& signal : signalDescriptions) {
    const auto name = signal.name().toLower();
    const auto unit = signal.physicalUnit().toLower();
    if (name.contains("gyro") || name.contains("roll") || name.contains("pitch")
        || name.contains("yaw") || unit.contains("deg/s") || unit.contains("rad/s"))
      gyroCount++;
  }

  if (gyroCount == 3)
    return SerialStudio::groupWidgetId(SerialStudio::Gyroscope);

  return QString();
}

//--------------------------------------------------------------------------------------------------
// Text utilities
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a dataset widget (bar/gauge/meter/empty) for one CAN signal.
 */
QString DataModel::DBCImporter::selectWidgetForSignal(const QCanSignalDescription& signal)
{
  const auto name = signal.name().toLower();
  const auto unit = signal.physicalUnit().toLower().trimmed();

  if (name.contains("odometer") || name.contains("trip") || name.contains("counter")
      || name.contains("timestamp") || name.contains("status"))
    return QString("");

  if (unit == "%" || (signal.minimum() == 0 && signal.maximum() == 100))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  // code-verify off
  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc") || unit.contains("degf")
      || name.contains("temp") || name.contains("temperature"))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);
  // code-verify on

  static const QSet<QString> kMeterUnits = {
    "rpm", "r/min", "1/min", "km/h", "kmh", "kph", "mph", "knot", "knots", "kn", "m/s", "kw", "hp"};
  if (kMeterUnits.contains(unit) || name.contains("rpm") || name.contains("speed")
      || name.contains("velocity") || name.contains("power"))
    return SerialStudio::datasetWidgetId(SerialStudio::Meter);

  static const QSet<QString> kGaugeUnits = {"v",
                                            "mv",
                                            "kv",
                                            "a",
                                            "ma",
                                            "ka",
                                            "psi",
                                            "bar",
                                            "nm",
                                            "kpa",
                                            "pa",
                                            "hpa",
                                            "mbar",
                                            "kg",
                                            "g",
                                            "n",
                                            "deg",
                                            "rad"};
  if (kGaugeUnits.contains(unit))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  if (unit.contains("psi") || unit.contains("kpa") || unit.contains("mbar"))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  const auto range = signal.maximum() - signal.minimum();
  if (range > 0 && range <= 360)
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  return QString("");
}

/**
 * @brief Returns true when the signal can only take integer physical values.
 */
bool DataModel::DBCImporter::isIntegerSignal(const QCanSignalDescription& signal)
{
  const double factor = signal.factor();
  const double offset = signal.offset();
  const auto isWhole  = [](double v) {
    return std::isfinite(v) && std::fabs(v - std::round(v)) < 1e-9;
  };

  return isWhole(factor) && isWhole(offset);
}

//--------------------------------------------------------------------------------------------------
// Pattern detection helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of importable signals (excludes extended-mux signals).
 */
int DataModel::DBCImporter::countTotalSignals(const QList<QCanMessageDescription>& messages) const
{
  int count = 0;
  for (const auto& message : messages) {
    for (const auto& signal : message.signalDescriptions()) {
      qint64 mv       = 0;
      const auto role = classifyMux(signal, message, mv);
      if (role != MuxRole::ExtendedMuxed)
        ++count;
    }
  }

  return count;
}

/**
 * @brief Returns true if at least 2 signal names match a positional marker.
 */
bool DataModel::DBCImporter::hasPositionalPattern(const QList<QCanSignalDescription>& signalList,
                                                  const QStringList& positions) const
{
  int matchCount = 0;
  for (const auto& signal : signalList) {
    const auto name = signal.name().toLower();
    for (const auto& pos : positions) {
      if (name.contains(pos)) {
        matchCount++;
        break;
      }
    }
  }

  return matchCount >= 2;
}

/**
 * @brief Returns true if at least 2 signal names use a numbered suffix.
 */
bool DataModel::DBCImporter::hasNumberedPattern(
  const QList<QCanSignalDescription>& signalList) const
{
  if (signalList.count() < 2)
    return false;

  QRegularExpression numberPattern("[_\\s]?\\d+$|\\d+[_\\s]?");
  int numberedCount = 0;

  for (const auto& signal : signalList) {
    const auto name = signal.name();
    if (numberPattern.match(name).hasMatch())
      numberedCount++;
  }

  return numberedCount >= 2;
}

/**
 * @brief Returns true if all non-empty physical units are identical.
 */
bool DataModel::DBCImporter::allSimilarUnits(const QList<QCanSignalDescription>& signalList) const
{
  if (signalList.isEmpty())
    return false;

  QStringList units;
  for (const auto& signal : signalList) {
    const auto unit = signal.physicalUnit().toLower().trimmed();
    if (!unit.isEmpty() && !units.contains(unit))
      units.append(unit);
  }

  return units.count() <= 1;
}

/**
 * @brief Returns true if the signals look like a battery monitoring cluster.
 */
bool DataModel::DBCImporter::hasBatterySignals(const QList<QCanSignalDescription>& signalList) const
{
  if (signalList.count() < 2)
    return false;

  bool hasVoltage = false;
  bool hasCurrent = false;
  bool hasSoC     = false;

  for (const auto& signal : signalList) {
    const auto name = signal.name().toLower();
    const auto unit = signal.physicalUnit().toLower();

    if (name.contains("volt") || unit.contains("v"))
      hasVoltage = true;

    if (name.contains("current") || name.contains("amp") || unit.contains("a"))
      hasCurrent = true;

    if (name.contains("soc") || name.contains("charge") || unit.contains("%"))
      hasSoC = true;
  }

  return (hasVoltage && hasCurrent) || (hasVoltage && hasSoC) || (hasCurrent && hasSoC);
}

/**
 * @brief Returns true if all signals are 1-bit flags or status-named.
 */
bool DataModel::DBCImporter::allStatusSignals(const QList<QCanSignalDescription>& signalList) const
{
  for (const auto& signal : signalList) {
    if (signal.bitLength() > 1) {
      const auto name = signal.name().toLower();
      if (!name.contains("status") && !name.contains("state") && !name.contains("mode")
          && !name.contains("flag"))
        return false;
    }
  }

  return true;
}

/**
 * @brief Counts signals suitable for time-series plotting.
 */
int DataModel::DBCImporter::countPlottable(const QList<QCanSignalDescription>& signalList) const
{
  int count = 0;
  for (const auto& signal : signalList) {
    if (signal.bitLength() <= 1)
      continue;

    const auto name = signal.name().toLower();
    if (name.contains("odometer") || name.contains("trip") || name.contains("counter")
        || name.contains("timestamp") || name.contains("status") || name.contains("state"))
      continue;

    count++;
  }

  return count;
}

/**
 * @brief Classifies the message's signals into a SignalFamily category.
 */
DataModel::DBCImporter::SignalFamily DataModel::DBCImporter::detectSignalFamily(
  const QList<QCanSignalDescription>& signalList) const
{
  if (signalList.isEmpty())
    return None;

  if (hasPositionalPattern(signalList, {"fl", "fr", "rl", "rr"})) {
    for (const auto& signal : signalList) {
      const auto unit = signal.physicalUnit().toLower();
      if (unit.contains("km/h") || unit.contains("mph") || unit.contains("m/s"))
        return WheelSpeeds;

      if (unit.contains("psi") || unit.contains("bar") || unit.contains("kpa"))
        return TirePressures;
    }
  }

  if (hasNumberedPattern(signalList) && allSimilarUnits(signalList)) {
    if (!signalList.isEmpty()) {
      const auto unit = signalList.first().physicalUnit().toLower();
      // code-verify off
      if (unit.contains("°") || unit.contains("deg"))
        return Temperatures;
      // code-verify on

      if (unit.contains("v") && !unit.contains("rev"))
        return Voltages;
    }
  }

  if (hasBatterySignals(signalList))
    return BatteryCluster;

  if (allStatusSignals(signalList))
    return StatusFlags;

  if (signalList.count() >= 2 && allSimilarUnits(signalList))
    return GenericRelated;

  return None;
}

/**
 * @brief Returns true for critical signals that always warrant an own widget.
 */
bool DataModel::DBCImporter::isCriticalSignal(const QCanSignalDescription& signal) const
{
  const auto name = signal.name().toLower();
  const auto unit = signal.physicalUnit().toLower();

  if (name.contains("rpm") || unit.contains("rpm"))
    return true;

  if (name.contains("speed") || name.contains("velocity"))
    return true;

  if (unit.contains("km/h") || unit.contains("mph") || unit.contains("m/s"))
    return true;

  if (name.contains("temp") || name.contains("temperature"))
    return true;

  // code-verify off
  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc") || unit.contains("degf"))
    return true;
  // code-verify on

  if ((name.contains("volt") || unit.contains("v")) && !unit.contains("rev"))
    return true;

  if (name.contains("current") || name.contains("amp") || unit.contains("a"))
    return true;

  if (name.contains("pressure") || unit.contains("psi") || unit.contains("bar")
      || unit.contains("kpa") || unit.contains("pa"))
    return true;

  if (name.contains("throttle") || name.contains("brake") || name.contains("accelerator"))
    return true;

  if (name.contains("fuel") || name.contains("battery") || name.contains("soc")
      || name.contains("charge"))
    return true;

  if (name.contains("torque") || unit.contains("nm") || unit.contains("n.m"))
    return true;

  if (name.contains("power") || unit.contains("kw") || unit.contains("hp") || unit.contains("w"))
    return true;

  return false;
}

/**
 * @brief Returns true if the signal should receive its own dataset widget.
 */
bool DataModel::DBCImporter::shouldAssignIndividualWidget(const QString& groupWidget,
                                                          const QCanSignalDescription& signal,
                                                          bool isSingleBit) const
{
  if (isSingleBit)
    return true;

  if (groupWidget.isEmpty() || groupWidget == SerialStudio::groupWidgetId(SerialStudio::DataGrid))
    return true;

  if (isCriticalSignal(signal))
    return true;

  return false;
}
