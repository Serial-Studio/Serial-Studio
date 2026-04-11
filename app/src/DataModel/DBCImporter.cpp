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

/**
 * @brief Private constructor for the singleton pattern.
 */
DataModel::DBCImporter::DBCImporter() {}

/**
 * @brief Returns the singleton instance of the DBC importer.
 * @return Reference to the DBCImporter singleton.
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
 * @brief Returns the total number of signals across all messages.
 * @return Total signal count from the parsed DBC file.
 */
int DataModel::DBCImporter::signalCount() const
{
  return countTotalSignals(m_messages);
}

/**
 * @brief Returns the number of messages in the parsed DBC file.
 * @return Message count.
 */
int DataModel::DBCImporter::messageCount() const
{
  return m_messages.count();
}

/**
 * @brief Returns the filename of the currently loaded DBC file.
 * @return DBC filename without path.
 */
QString DataModel::DBCImporter::dbcFileName() const
{
  return QFileInfo(m_dbcFilePath).fileName();
}

/**
 * @brief Returns formatted information string for a specific message.
 *
 * The returned string contains the message index, name, CAN ID (in hex),
 * and signal count in the format: "N: MessageName @ 0xID (X signals)"
 *
 * @param index Index of the message in the parsed list.
 * @return Formatted message information string, or empty if index is
 *         invalid.
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
 * @brief Opens a file dialog to select and import a DBC file.
 *
 * This function presents the user with a non-native file selection dialog
 * filtered for DBC files. Upon selection, it calls showPreview() to parse
 * and preview the file contents.
 */
void DataModel::DBCImporter::importDBC()
{
  const auto p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  auto* dialog =
    new QFileDialog(nullptr, tr("Import DBC File"), p, tr("DBC Files (*.dbc);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);
  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      showPreview(path);

    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Cancels the current import operation and clears loaded data.
 *
 * This function clears the message list and file path, then emits signals
 * to update the UI. It's called when the user cancels the preview dialog.
 */
void DataModel::DBCImporter::cancelImport()
{
  m_messages.clear();
  m_dbcFilePath.clear();
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
}

/**
 * @brief Parses a DBC file and emits a preview signal for UI display.
 *
 * This function uses Qt's QCanDbcFileParser to parse the DBC file at the
 * specified path. If parsing succeeds and messages are found, it stores
 * the message list and emits previewReady() to trigger the preview dialog.
 *
 * On failure, displays an error message box to the user.
 *
 * @param filePath Absolute path to the DBC file to parse.
 */
void DataModel::DBCImporter::showPreview(const QString& filePath)
{
  QCanDbcFileParser parser;
  if (!parser.parse(filePath)) {
    Misc::Utilities::showMessageBox(tr("Failed to parse DBC file: %1").arg(parser.errorString()),
                                    tr("Please verify the file format and try again."),
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
 * @brief Confirms the import and generates a Serial Studio project.
 *
 * This function is called when the user confirms the import from the
 * preview dialog. It generates a complete project from the parsed DBC
 * messages, including:
 * - Groups for each CAN message
 * - Datasets for each signal with appropriate widgets
 * - JavaScript frame parser for extracting CAN signals
 *
 * The project is saved to a temporary file, loaded into the ProjectModel,
 * and the user is prompted to save it to a permanent location. On success,
 * the project editor opens automatically.
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
 * @brief Generates a complete Serial Studio project from DBC messages.
 *
 * This function creates a QJsonObject representing a complete Serial Studio
 * project file (.ssproj) from the provided CAN message descriptions.
 *
 * The generated project includes:
 * - Project title: Based on DBC filename
 * - Decoder method: 3 (custom JavaScript parser)
 * - Frame detection: 2 (manual mode)
 * - Frame parser: JavaScript code for extracting CAN signals
 * - Groups: One group per CAN message with datasets for each signal
 * - Actions: Empty array (no actions defined)
 *
 * @param messages List of CAN message descriptions from the DBC file.
 * @return QJsonObject containing the complete project structure.
 */
QJsonObject DataModel::DBCImporter::generateProject(const QList<QCanMessageDescription>& messages)
{
  QJsonObject project;

  const auto dbcInfo      = QFileInfo(m_dbcFilePath);
  const auto projectTitle = dbcInfo.baseName();

  project["title"]   = projectTitle;
  project["actions"] = QJsonArray();

  QJsonObject source;
  source["sourceId"]              = 0;
  source["title"]                 = tr("CAN Bus");
  source["busType"]               = static_cast<int>(SerialStudio::BusType::CanBus);
  source["frameStart"]            = QString("");
  source["frameEnd"]              = QString("");
  source["checksum"]              = QString("");
  source["frameDetection"]        = static_cast<int>(SerialStudio::NoDelimiters);
  source["decoder"]               = static_cast<int>(SerialStudio::Binary);
  source["hexadecimalDelimiters"] = false;
  source["frameParserCode"]       = generateFrameParser(messages);
  source["frameParserLanguage"]   = static_cast<int>(SerialStudio::Lua);

  project["sources"] = QJsonArray{source};

  const auto groups = generateGroups(messages);
  QJsonArray groupArray;
  for (const auto& group : groups)
    groupArray.append(serialize(group));

  project["groups"] = groupArray;

  return project;
}

/**
 * @brief Generates group structures for all CAN messages.
 *
 * This function creates a Group object for each CAN message in the DBC
 * file. Each group contains:
 * - Group ID: Sequential starting from 0
 * - Title: CAN message name
 * - Widget: Auto-selected based on signal family detection
 * - Datasets: One dataset per signal in the message
 *
 * Each dataset is configured with:
 * - Index: Sequential dataset index across all groups
 * - Title: Signal name
 * - Units: Physical unit from DBC (e.g., "rpm", "°C")
 * - Widget: Conditionally assigned based on group widget type
 * - Min/Max: Signal range from DBC definition
 * - Flags: graph=true for numeric signals, led=true for boolean signals
 *
 * Widget Assignment Optimization:
 * Individual widgets are only assigned when the group widget cannot
 * adequately visualize the data. For example:
 * - MultiPlot groups: No individual widgets (plot shows all signals)
 * - DataGrid groups: Individual widgets assigned (grid is just a table)
 * - Boolean signals: Always get LED widgets (group can't show these)
 *
 * This significantly reduces widget count and improves dashboard clarity.
 *
 * @param messages List of CAN message descriptions from the DBC file.
 * @return Vector of Group structures ready for serialization.
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
 * @brief Generates a JavaScript frame parser for extracting CAN signals.
 *
 * This function generates a complete JavaScript frame parser that:
 * 1. Extracts the CAN ID and DLC from the frame header
 * 2. Routes frames to message-specific decoders via switch statement
 * 3. Provides helper functions for signal extraction supporting:
 *    - Arbitrary bit positions and lengths
 *    - Both Intel (little-endian) and Motorola (big-endian) byte order
 *    - Signed and unsigned integers
 *    - Scaling and offset transformations
 *
 * Frame format expected:
 * - Bytes 0-1: CAN ID (big-endian, 16-bit)
 * - Byte 2: Data Length Code (DLC)
 * - Bytes 3+: CAN data payload
 *
 * The generated parser includes:
 * - Main parse() function that routes by CAN ID
 * - decode_XXXX() functions for each message
 * - extractSignal() helper for bit-level signal extraction
 *
 * @param messages List of CAN message descriptions from the DBC file.
 * @return JavaScript code as a QString ready for embedding in the project.
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
 * @brief Generates a decoder function for a specific CAN message.
 *
 * This function generates JavaScript code for decoding all signals within
 * a CAN message. The generated function:
 * - Is named decode_XXXX where XXXX is the hex CAN ID
 * - Accepts the CAN data payload as a byte array
 * - Extracts each signal using the extractSignal() helper
 * - Applies scaling and offset transformations
 * - Updates the global values array at the appropriate indices
 *
 * @param message CAN message description from the DBC file.
 * @param datasetIndex Reference to the current dataset index, incremented
 *                     for each signal to maintain proper global array
 *                     indexing.
 * @return JavaScript function code as a QString.
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
 * @brief Generates JavaScript code to extract a single CAN signal.
 *
 * This function generates two JavaScript statements:
 * 1. Extraction of the raw signal value using extractSignal()
 * 2. Application of scaling and offset: physical = (raw * factor) + offset
 *
 * The extraction accounts for:
 * - Signal bit position and length from the DBC definition
 * - Byte order (big-endian/little-endian)
 * - Signed vs unsigned interpretation
 *
 * @param signal CAN signal description from the DBC file.
 * @return JavaScript code for extracting and scaling the signal.
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
 * @brief Selects an appropriate group widget type for a CAN message.
 *
 * This function analyzes the signals in a CAN message to automatically
 * assign a suitable group widget type for dashboard visualization.
 *
 * The selection uses signal family detection to identify related signals
 * and choose the most appropriate visualization:
 *
 * - **NoGroupWidget**: Single signal or all boolean (1-bit) signals
 * - **GPS**: Messages with latitude and longitude signals
 * - **Accelerometer**: Exactly 3 signals with acceleration characteristics
 * - **Gyroscope**: Exactly 3 signals with gyroscope/rotation characteristics
 * - **MultiPlot**: Related signals that benefit from time-series visualization
 *   (wheel speeds, tire pressures, temperature series, voltage series, etc.)
 * - **DataGrid**: Mixed signal types (battery clusters) or non-plottable data
 *
 * Key improvements:
 * - Detects positional patterns (FL/FR/RL/RR)
 * - Detects numbered series (Temp_1, Temp_2, etc.)
 * - Groups related signals with similar units
 * - Reduces redundant widgets by smart group selection
 *
 * @param message CAN message description from the DBC file.
 * @return Group widget type string.
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
 * @brief Sanitizes a string for use as a JavaScript identifier.
 *
 * This function replaces all non-alphanumeric characters (except
 * underscores) with underscores, ensuring the resulting string is a valid
 * JavaScript variable name.
 *
 * @param str Input string (e.g., signal name from DBC).
 * @return Sanitized string safe for use in JavaScript code.
 */
QString DataModel::DBCImporter::sanitizeJavaScriptString(const QString& str)
{
  QString result = str;
  result.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
  return result;
}

/**
 * @brief Selects an appropriate widget type for visualizing a signal.
 *
 * This function analyzes signal properties to automatically assign a
 * suitable widget type for dashboard visualization:
 *
 * - **Bar widget**: Percentage signals (0-100 range or "%" unit)
 * - **Gauge widget**: Automotive/industrial signals with units like rpm,
 *   km/h, mph, V, A, °C, °F, PSI, bar, Nm, kPa
 * - **Empty (no widget)**: Counters (odometer, trip, distance), status
 *   values, or signals without meaningful ranges
 *
 * The selection is based on:
 * 1. Signal name (to detect counters/status)
 * 2. Physical unit string (case-insensitive)
 * 3. Value range (min/max)
 *
 * @param signal CAN signal description from the DBC file.
 * @return Widget type string ("bar", "gauge", or empty).
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
 * @brief Counts the total number of signals across all messages.
 *
 * This function iterates through all CAN messages and sums up the number
 * of signals in each message.
 *
 * @param messages List of CAN message descriptions.
 * @return Total number of signals across all messages.
 */
int DataModel::DBCImporter::countTotalSignals(const QList<QCanMessageDescription>& messages) const
{
  int count = 0;
  for (const auto& message : messages)
    count += message.signalDescriptions().count();

  return count;
}

/**
 * @brief Checks if signals follow a positional naming pattern.
 *
 * Detects patterns like FL/FR/RL/RR (front-left, front-right, etc.) in
 * signal names, which typically indicate related sensors at different
 * positions.
 *
 * @param signalList List of signal descriptions to check.
 * @param positions List of position identifiers to look for.
 * @return true if at least 2 signals match the positional pattern.
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
 * @brief Checks if signals follow a numbered naming pattern.
 *
 * Detects patterns like Temp_1, Temp_2, Temp_3 or Cell1, Cell2, Cell3
 * which indicate a series of similar sensors.
 *
 * @param signalList List of signal descriptions to check.
 * @return true if at least 2 signals have numbered suffixes.
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
 * @brief Checks if all signals have similar or identical physical units.
 *
 * This helps identify related signals that should be visualized together,
 * such as multiple temperature sensors or multiple voltage readings.
 *
 * @param signalList List of signal descriptions to check.
 * @return true if all non-empty units are similar.
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
 * @brief Detects if signals represent a battery monitoring cluster.
 *
 * Battery messages typically contain a mix of voltage, current, temperature,
 * and state of charge signals. These are best shown in a data grid rather
 * than a plot.
 *
 * @param signalList List of signal descriptions to check.
 * @return true if signals match battery cluster pattern.
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
 * @brief Checks if all signals are status/boolean flags.
 *
 * Messages with only boolean signals or status indicators don't need a
 * group widget since individual LEDs will handle visualization.
 *
 * @param signalList List of signal descriptions to check.
 * @return true if all signals are 1-bit or status indicators.
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
 * @brief Counts the number of plottable signals.
 *
 * Excludes counters, timestamps, and status fields that shouldn't be
 * plotted on time-series graphs.
 *
 * @param signalList List of signal descriptions to check.
 * @return Number of signals suitable for plotting.
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
 * @brief Detects the family/category of related signals in a message.
 *
 * Analyzes signal names, units, and patterns to categorize the message,
 * which helps select appropriate group widgets and reduce redundant
 * individual widgets.
 *
 * @param signalList List of signal descriptions from a CAN message.
 * @return SignalFamily enum indicating the detected signal category.
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
 * @brief Determines if a signal is critical and should always have a widget.
 *
 * Critical signals are important values that users need to see at a glance
 * in the dashboard overview, such as RPM, speed, temperature, voltage, etc.
 * These should have individual gauges/bars even when a group-level MultiPlot
 * is showing trends.
 *
 * @param signal The signal description to check.
 * @return true if the signal is critical and should always get a widget.
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
 * @brief Determines if a signal should get an individual widget.
 *
 * This function implements smart widget assignment that balances overview
 * visibility with dashboard clarity:
 *
 * Rules:
 * - Single-bit signals always get LEDs (group widgets don't show these)
 * - Critical signals (RPM, speed, temp, voltage, etc.) always get individual
 *   widgets for overview visibility, even with MultiPlot groups
 * - Non-critical signals in MultiPlot groups skip individual widgets (reduces
 *   clutter, group shows trends)
 * - Signals in DataGrid/NoGroupWidget always get individual widgets
 *
 * This ensures important values are visible in overview while reducing
 * redundant widgets for less critical signals.
 *
 * @param groupWidget The widget type assigned to the parent group.
 * @param signal The signal description.
 * @param isSingleBit Whether the signal is a 1-bit boolean.
 * @return true if an individual widget should be assigned.
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
