/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include "DataModel/DBCImporter.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QStandardPaths>

#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/** @brief Constructs the DBCImporter singleton. */
DataModel::DBCImporter::DBCImporter() {}

/** @brief Returns the singleton DBCImporter instance. */
DataModel::DBCImporter& DataModel::DBCImporter::instance()
{
  static DBCImporter instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/** @brief Returns the total number of signals across all loaded messages. */
int DataModel::DBCImporter::signalCount() const
{
  return countTotalSignals(m_messages);
}

/** @brief Returns the number of messages loaded from the DBC file. */
int DataModel::DBCImporter::messageCount() const
{
  return m_messages.count();
}

/** @brief Returns the file name of the loaded DBC file. */
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

  const auto& message    = m_messages.at(index);
  const auto msgId       = static_cast<quint32>(message.uniqueId());
  const auto signalCount = message.signalDescriptions().count();

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
  auto* dialog =
    new QFileDialog(nullptr, tr("Import DBC File"), p, tr("DBC Files (*.dbc);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      showPreview(path);

    dialog->deleteLater();
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

  // Sort by CAN ID so dataset indices stay consistent
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

  const auto project     = generateProject(m_messages);
  const auto dbcInfo     = QFileInfo(m_dbcFilePath);
  const auto projectName = dbcInfo.baseName();
  const auto tempDir     = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  const auto tempPath    = tempDir + "/" + projectName + "_temp.ssproj";

  QFile file(tempPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    Misc::Utilities::showMessageBox(
      tr("Failed to create temporary project file"),
      tr("Check if the application has write permissions to the temporary directory."),
      QMessageBox::Critical,
      tr("DBC Import Error"));
    return;
  }

  QJsonDocument doc(project);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  ProjectModel::instance().openJsonFile(tempPath);

  if (ProjectModel::instance().saveJsonFile(true)) {
    QFile::remove(tempPath);

    // clang-format off
    Misc::Utilities::showMessageBox(
        tr("Successfully imported DBC file with %1 messages and %2 signals.")
            .arg(m_messages.count())
            .arg(countTotalSignals(m_messages)),
        tr("The project editor is now open for customization."),
        QMessageBox::Information,
        tr("DBC Import Complete"));
    // clang-format on
  } else {
    QFile::remove(tempPath);
  }
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
  source[Keys::FrameParserCode]       = generateFrameParser(messages);
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);

  project[Keys::Sources] = QJsonArray{source};

  const auto groups = generateGroups(messages);
  QJsonArray groupArray;
  for (const auto& group : groups)
    groupArray.append(serialize(group));

  project[Keys::Groups] = groupArray;

  return project;
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
    const auto signalList = message.signalDescriptions();
    if (signalList.isEmpty())
      continue;

    DataModel::Group group;
    group.groupId = groupId;
    group.title   = message.name();
    group.widget  = selectGroupWidget(message);

    for (const auto& signal : signalList) {
      DataModel::Dataset dataset;
      dataset.index = datasetIndex++;
      dataset.title = signal.name();
      dataset.units = signal.physicalUnit();

      double minVal = signal.minimum();
      double maxVal = signal.maximum();

      if (std::isnan(minVal) || std::isnan(maxVal) || minVal >= maxVal) {
        minVal = 0.0;
        maxVal = 100.0;
      }

      dataset.wgtMin = minVal;
      dataset.wgtMax = maxVal;
      dataset.pltMin = minVal;
      dataset.pltMax = maxVal;
      dataset.fftMin = minVal;
      dataset.fftMax = maxVal;

      const auto isSingleBit = (signal.bitLength() == 1);

      if (isSingleBit) {
        dataset.widget  = QString("");
        dataset.plt     = false;
        dataset.fft     = false;
        dataset.led     = true;
        dataset.ledHigh = 1;
      } else {
        if (shouldAssignIndividualWidget(group.widget, signal, isSingleBit))
          dataset.widget = selectWidgetForSignal(signal);
        else
          dataset.widget = QString("");

        dataset.plt     = false;
        dataset.fft     = false;
        dataset.led     = false;
        dataset.ledHigh = 0;
      }

      dataset.log = true;

      group.datasets.push_back(dataset);
    }

    groups.push_back(group);
    ++groupId;
  }

  return groups;
}

/**
 * @brief Generates the Lua frame parser script for the DBC messages.
 */
QString DataModel::DBCImporter::generateFrameParser(const QList<QCanMessageDescription>& messages)
{
  QString code;

  const auto totalSignals = countTotalSignals(messages);

  // clang-format off
  code += "--\n";
  code += "-- CAN DBC Frame Parser\n";
  code += "--\n";
  code += QString("-- Auto-generated from: %1\n")
              .arg(QFileInfo(m_dbcFilePath).fileName());
  code += "--\n";
  code += QString("-- Total signals: %1\n").arg(totalSignals);
  code += QString("-- Total messages: %1\n").arg(messageCount());
  code += "--\n";
  code += "-- Frame format:\n";
  code += "--   Byte 1-2: CAN ID (big-endian, 16-bit)\n";
  code += "--   Byte 3:   Data Length Code (DLC)\n";
  code += "--   Byte 4+:  CAN data payload\n";
  code += "--\n\n";

  code += QString("local values = {}\nfor i = 1, %1 do values[i] = 0 end\n\n").arg(totalSignals);
  // clang-format on

  // Emit extractSignal helper for bit-level extraction
  // clang-format off
  code += "local function extractSignal(data, startBit, length, isBigEndian, isSigned)\n";
  code += "  local value = 0\n\n";
  code += "  if isBigEndian then\n";
  code += "    local startByte = startBit // 8\n";
  code += "    local numBytes = (length + 7) // 8\n\n";
  code += "    -- Byte-aligned fast path\n";
  code += "    if startBit % 8 == 0 and length % 8 == 0 then\n";
  code += "      for i = 0, numBytes - 1 do\n";
  code += "        if startByte + i + 1 <= #data then\n";
  code += "          value = (value << 8) | data[startByte + i + 1]\n";
  code += "        end\n";
  code += "      end\n";
  code += "    else\n";
  code += "      -- Non-byte-aligned: bit-by-bit extraction\n";
  code += "      for bit = 0, length - 1 do\n";
  code += "        local bitPos = startBit + bit\n";
  code += "        local byteIdx = bitPos // 8 + 1\n";
  code += "        local bitIdx = 7 - (bitPos % 8)\n";
  code += "        if byteIdx <= #data then\n";
  code += "          value = (value << 1) | ((data[byteIdx] >> bitIdx) & 1)\n";
  code += "        end\n";
  code += "      end\n";
  code += "    end\n";
  code += "  else\n";
  code += "    -- Intel (little-endian)\n";
  code += "    local startByte = startBit // 8\n";
  code += "    local startBitInByte = startBit % 8\n";
  code += "    local bitsRead = 0\n\n";
  code += "    local i = startByte\n";
  code += "    while i + 1 <= #data and bitsRead < length do\n";
  code += "      local bitsInThisByte\n";
  code += "      if i == startByte then\n";
  code += "        bitsInThisByte = math.min(8 - startBitInByte, length - bitsRead)\n";
  code += "      else\n";
  code += "        bitsInThisByte = math.min(8, length - bitsRead)\n";
  code += "      end\n";
  code += "      local shift = (i == startByte) and startBitInByte or 0\n";
  code += "      local mask = ((1 << bitsInThisByte) - 1) << shift\n";
  code += "      local bits = (data[i + 1] & mask) >> shift\n";
  code += "      value = value | (bits << bitsRead)\n";
  code += "      bitsRead = bitsRead + bitsInThisByte\n";
  code += "      i = i + 1\n";
  code += "    end\n";
  code += "  end\n\n";
  code += "  -- Apply signed conversion\n";
  code += "  if isSigned and (value & (1 << (length - 1))) ~= 0 then\n";
  code += "    value = value - (1 << length)\n";
  code += "  end\n\n";
  code += "  return value\n";
  code += "end\n\n";
  // clang-format on

  // Emit per-message decoder functions
  int datasetIndex = 1;
  for (const auto& message : messages) {
    if (message.signalDescriptions().isEmpty())
      continue;

    code += generateMessageDecoder(message, datasetIndex);
    code += "\n";
  }

  // Emit main parse function with CAN ID routing
  // clang-format off
  code += "function parse(frame)\n";
  code += "  if #frame < 3 then return values end\n\n";
  code += "  local canId = (frame[1] << 8) | frame[2]\n";
  code += "  local dlc = frame[3]\n\n";
  code += "  -- Extract data payload (1-indexed)\n";
  code += "  local data = {}\n";
  code += "  for i = 1, dlc do\n";
  code += "    if 3 + i <= #frame then data[i] = frame[3 + i] end\n";
  code += "  end\n\n";
  // clang-format on

  code += "  -- Route CAN ID to the matching message decoder\n";
  bool first = true;
  for (const auto& message : messages) {
    if (message.signalDescriptions().isEmpty())
      continue;

    const auto msgId = static_cast<quint32>(message.uniqueId());
    const auto hex   = QString::number(msgId, 16).toUpper();

    if (first) {
      code += QString("  if canId == 0x%1 then\n").arg(hex);
      first = false;
    } else {
      code += QString("  elseif canId == 0x%1 then\n").arg(hex);
    }

    code += QString("    -- %1\n").arg(message.name());
    code += QString("    decode_%1(data)\n").arg(QString::number(msgId, 16));
  }

  if (!first)
    code += "  end\n\n";

  code += "  return values\n";
  code += "end\n";

  return code;
}

/**
 * @brief Generates the decode_XXXX() Lua function for one CAN message.
 */
QString DataModel::DBCImporter::generateMessageDecoder(const QCanMessageDescription& message,
                                                       int& datasetIndex)
{
  QString code;
  const auto msgId      = static_cast<quint32>(message.uniqueId());
  const auto signalList = message.signalDescriptions();
  const auto startIndex = datasetIndex;

  code += "--\n";
  code += QString("-- Decodes CAN message: %1 (ID: 0x%2)\n")
            .arg(message.name())
            .arg(QString::number(msgId, 16).toUpper());
  code += "--\n";
  code += QString("-- Signals (%1):\n").arg(signalList.count());

  int idx = startIndex;
  for (const auto& signal : signalList) {
    code += QString("--   [%1] %2 (%3)\n")
              .arg(idx)
              .arg(signal.name())
              .arg(signal.physicalUnit().isEmpty() ? "no unit" : signal.physicalUnit());
    ++idx;
  }

  code += "--\n";
  code += QString("local function decode_%1(data)\n").arg(QString::number(msgId, 16));

  bool firstSignal = true;
  for (const auto& signal : signalList) {
    if (!firstSignal)
      code += "\n";
    firstSignal = false;

    const auto unit = signal.physicalUnit();
    if (unit.isEmpty())
      code += QString("  -- %1\n").arg(signal.name());
    else
      code += QString("  -- %1 [%2]\n").arg(signal.name(), unit);

    code += generateSignalExtraction(signal);
    code += QString("  values[%1] = value_%2\n")
              .arg(datasetIndex)
              .arg(sanitizeJavaScriptString(signal.name()));
    ++datasetIndex;
  }

  code += "end\n";

  return code;
}

//--------------------------------------------------------------------------------------------------
// Signal processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates the Lua code to extract and scale one CAN signal.
 */
QString DataModel::DBCImporter::generateSignalExtraction(const QCanSignalDescription& signal)
{
  // clang-format off
  QString code;

  // Qt's DBC parser inverts byte order, flip it back
  const auto isBigEndian = (signal.dataEndian() == QSysInfo::LittleEndian);
  const auto isSigned = (signal.dataFormat() == QtCanBus::DataFormat::SignedInteger);
  // clang-format on

  // clang-format off
  code += QString("  local raw_%1 = extractSignal(data, %2, %3, %4, %5)\n")
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(signal.startBit())
              .arg(signal.bitLength())
              .arg(isBigEndian ? "true" : "false")
              .arg(isSigned ? "true" : "false");

  code += QString("  local value_%1 = (raw_%2 * %3) + %4\n")
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(signal.factor(), 0, 'g', 10)
              .arg(signal.offset(), 0, 'g', 10);
  // clang-format on

  return code;
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

  const auto family = detectSignalFamily(signalDescriptions);

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

  if (signalCount >= 2) {
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
  }

  if (signalCount == 3) {
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
  }

  if (signalCount >= 2 && signalCount <= 8) {
    const int plottableCount = countPlottable(signalDescriptions);
    if (plottableCount >= 2)
      return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);
  }

  return SerialStudio::groupWidgetId(SerialStudio::DataGrid);
}

//--------------------------------------------------------------------------------------------------
// Text utilities
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces non-alphanumeric characters so the string is a valid ident.
 */
QString DataModel::DBCImporter::sanitizeJavaScriptString(const QString& str)
{
  QString result = str;
  result.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
  return result;
}

/**
 * @brief Picks a dataset widget (bar/gauge/empty) for one CAN signal.
 */
QString DataModel::DBCImporter::selectWidgetForSignal(const QCanSignalDescription& signal)
{
  const auto name = signal.name().toLower();
  const auto unit = signal.physicalUnit().toLower();

  // Skip counters and status fields
  if (name.contains("odometer") || name.contains("trip") || name.contains("counter")
      || name.contains("timestamp") || name.contains("status"))
    return QString("");

  if (unit == "%" || (signal.minimum() == 0 && signal.maximum() == 100))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc") || unit.contains("degf")
      || name.contains("temp") || name.contains("temperature"))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  if (unit.contains("rpm") || unit.contains("km/h") || unit.contains("mph") || unit.contains("v")
      || unit.contains("a") || unit.contains("psi") || unit.contains("bar") || unit.contains("nm")
      || unit.contains("kpa") || unit.contains("mbar") || unit.contains("kg")
      || unit.contains("deg"))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  const auto range = signal.maximum() - signal.minimum();
  if (range > 0 && range <= 360)
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  return QString("");
}

//--------------------------------------------------------------------------------------------------
// Pattern detection helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of signals across all messages.
 */
int DataModel::DBCImporter::countTotalSignals(const QList<QCanMessageDescription>& messages) const
{
  int count = 0;
  for (const auto& message : messages)
    count += message.signalDescriptions().count();

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
      if (unit.contains("°") || unit.contains("deg"))
        return Temperatures;
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

  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc") || unit.contains("degf"))
    return true;

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
