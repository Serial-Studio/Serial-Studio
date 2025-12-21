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

#include "Frame.h"
#include "DBCImporter.h"
#include "FrameBuilder.h"
#include "ProjectModel.h"
#include "Misc/Utilities.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QFileDialog>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QRegularExpression>

/**
 * @brief Private constructor for the singleton pattern.
 */
JSON::DBCImporter::DBCImporter() {}

/**
 * @brief Returns the singleton instance of the DBC importer.
 * @return Reference to the DBCImporter singleton.
 */
JSON::DBCImporter &JSON::DBCImporter::instance()
{
  static DBCImporter instance;
  return instance;
}

/**
 * @brief Returns the total number of signals across all messages.
 * @return Total signal count from the parsed DBC file.
 */
int JSON::DBCImporter::signalCount() const
{
  return countTotalSignals(m_messages);
}

/**
 * @brief Returns the number of messages in the parsed DBC file.
 * @return Message count.
 */
int JSON::DBCImporter::messageCount() const
{
  return m_messages.count();
}

/**
 * @brief Returns the filename of the currently loaded DBC file.
 * @return DBC filename without path.
 */
QString JSON::DBCImporter::dbcFileName() const
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
QString JSON::DBCImporter::messageInfo(int index) const
{
  if (index < 0 || index >= m_messages.count())
    return QString();

  const auto &message = m_messages.at(index);
  const auto msgId = static_cast<quint32>(message.uniqueId());
  const auto signalCount = message.signalDescriptions().count();

  return QString("%1: %2 @ 0x%3 (%4 signals)")
      .arg(index + 1)
      .arg(message.name())
      .arg(QString::number(msgId, 16).toUpper())
      .arg(signalCount);
}

/**
 * @brief Opens a file dialog to select and import a DBC file.
 *
 * This function presents the user with a non-native file selection dialog
 * filtered for DBC files. Upon selection, it calls showPreview() to parse
 * and preview the file contents.
 */
void JSON::DBCImporter::importDBC()
{
  const auto p = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  auto *dialog = new QFileDialog(nullptr, tr("Import DBC File"), p,
                                 tr("DBC Files (*.dbc);;All Files (*)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);
  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
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
void JSON::DBCImporter::cancelImport()
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
void JSON::DBCImporter::showPreview(const QString &filePath)
{
  QCanDbcFileParser parser;
  if (!parser.parse(filePath))
  {
    Misc::Utilities::showMessageBox(
        tr("Failed to parse DBC file: %1").arg(parser.errorString()),
        tr("Please verify the file format and try again."),
        QMessageBox::Critical, tr("DBC Import Error"));
    return;
  }

  m_messages = parser.messageDescriptions();
  if (m_messages.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("DBC file contains no messages"),
        tr("The selected file does not contain any CAN message definitions."),
        QMessageBox::Warning, tr("DBC Import Warning"));
    return;
  }

  m_dbcFilePath = filePath;
  Q_EMIT messagesChanged();
  Q_EMIT dbcFileNameChanged();
  Q_EMIT previewReady();
}

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
void JSON::DBCImporter::confirmImport()
{
  if (m_messages.isEmpty())
    return;

  const auto project = generateProject(m_messages);
  const auto dbcInfo = QFileInfo(m_dbcFilePath);
  const auto projectName = dbcInfo.baseName();
  const auto tempDir
      = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
  const auto tempPath = tempDir + "/" + projectName + "_temp.ssproj";

  QFile file(tempPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    Misc::Utilities::showMessageBox(
        tr("Failed to create temporary project file"),
        tr("Check if the application has write permissions to the "
           "temporary directory."),
        QMessageBox::Critical, tr("DBC Import Error"));
    return;
  }

  QJsonDocument doc(project);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  ProjectModel::instance().openJsonFile(tempPath);

  if (ProjectModel::instance().saveJsonFile(true))
  {
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
  }
  else
  {
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
QJsonObject JSON::DBCImporter::generateProject(
    const QList<QCanMessageDescription> &messages)
{
  QJsonObject project;

  const auto dbcInfo = QFileInfo(m_dbcFilePath);
  const auto projectTitle = dbcInfo.baseName();

  project["title"] = projectTitle;
  project["frameStart"] = QString("");
  project["frameEnd"] = QString("");
  project["frameParser"] = generateFrameParser(messages);
  project["decoder"] = static_cast<int>(SerialStudio::Binary);
  project["frameDetection"] = static_cast<int>(SerialStudio::NoDelimiters);
  project["actions"] = QJsonArray();

  const auto groups = generateGroups(messages);
  QJsonArray groupArray;
  for (const auto &group : groups)
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
 * - Widget: "multiplot" for visualization
 * - Datasets: One dataset per signal in the message
 *
 * Each dataset is configured with:
 * - Index: Sequential dataset index across all groups
 * - Title: Signal name
 * - Units: Physical unit from DBC (e.g., "rpm", "°C")
 * - Widget: Auto-selected based on signal properties
 * - Min/Max: Signal range from DBC definition
 * - Flags: graph=true for numeric signals, led=true for boolean signals
 *
 * @param messages List of CAN message descriptions from the DBC file.
 * @return Vector of Group structures ready for serialization.
 */
std::vector<JSON::Group>
JSON::DBCImporter::generateGroups(const QList<QCanMessageDescription> &messages)
{
  std::vector<JSON::Group> groups;
  int groupId = 0;
  int datasetIndex = 0;

  for (const auto &message : messages)
  {
    const auto signalList = message.signalDescriptions();
    if (signalList.isEmpty())
      continue;

    JSON::Group group;
    group.groupId = groupId;
    group.title = message.name();
    group.widget = selectGroupWidget(message);

    for (const auto &signal : signalList)
    {
      JSON::Dataset dataset;
      dataset.index = datasetIndex++;
      dataset.title = signal.name();
      dataset.units = signal.physicalUnit();

      double minVal = signal.minimum();
      double maxVal = signal.maximum();

      if (std::isnan(minVal) || std::isnan(maxVal) || minVal >= maxVal)
      {
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

      if (isSingleBit)
      {
        dataset.widget = QString("");
        dataset.plt = false;
        dataset.fft = false;
        dataset.led = true;
        dataset.ledHigh = 1;
      }

      else
      {
        dataset.widget = selectWidgetForSignal(signal);
        dataset.plt = false;
        dataset.fft = false;
        dataset.led = false;
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
QString JSON::DBCImporter::generateFrameParser(
    const QList<QCanMessageDescription> &messages)
{
  QString code;

  const auto totalSignals = countTotalSignals(messages);

  // clang-format off
  code += "/**\n";
  code += " * CAN DBC Frame Parser\n";
  code += " *\n";
  code += QString(" * Auto-generated from: %1\n")
              .arg(QFileInfo(m_dbcFilePath).fileName());
  code += " *\n";
  code += " * This parser decodes CAN frames based on DBC message definitions.\n";
  code += " * It maintains a global array of all signal values across different\n";
  code += " * CAN message types, allowing multiple messages to coexist in a\n";
  code += " * single project.\n";
  code += " *\n";
  code += QString(" * Total signals: %1\n").arg(totalSignals);
  code += QString(" * Total messages: %1\n").arg(messageCount());
  code += " */\n\n";
  // clang-format on

  // clang-format off
  code += "/**\n";
  code += " * Global array holding all CAN signal values.\n";
  code += " *\n";
  code += " * This array persists between parse() calls, maintaining the most\n";
  code += " * recent value for each signal across all CAN messages.\n";
  code += " *\n";
  code += QString(" * @type {Array<number>} Array of %1 signal values\n").arg(totalSignals);
  code += " */\n";
  code += QString("const values = new Array(%1).fill(0);\n\n").arg(totalSignals);
  code += "/**\n";
  code += " * Parses a CAN frame and extracts signal values.\n";
  code += " *\n";
  code += " * Frame format:\n";
  code += " *   Byte 0-1: CAN ID (big-endian, 16-bit)\n";
  code += " *   Byte 2:   Data Length Code (DLC)\n";
  code += " *   Byte 3+:  CAN data payload\n";
  code += " *\n";
  code += " * @param {Uint8Array} frame - Raw CAN frame data\n";
  code += " * @returns {Array<number>} Global values array with updated signals\n";
  code += " */\n";
  code += "function parse(frame) {\n";
  code += "  if (frame.length < 3) return values;\n\n";
  code += "  const canId = (frame[0] << 8) | frame[1];\n";
  code += "  const dlc = frame[2];\n";
  code += "  const data = frame.slice(3, 3 + dlc);\n\n";
  code += "  switch(canId) {\n";
  // clang-format on

  int datasetIndex = 0;
  for (const auto &message : messages)
  {
    if (message.signalDescriptions().isEmpty())
      continue;

    const auto msgId = static_cast<quint32>(message.uniqueId());
    code += QString("    case 0x%1:\n")
                .arg(QString::number(msgId, 16).toUpper());
    code += QString("      decode_%1(data);\n").arg(QString::number(msgId, 16));
    code += "      break;\n";
  }

  code += "  }\n\n";
  code += "  return values;\n";
  code += "}\n\n";

  datasetIndex = 0;
  for (const auto &message : messages)
  {
    if (message.signalDescriptions().isEmpty())
      continue;

    code += generateMessageDecoder(message, datasetIndex);
    code += "\n";
  }

  // clang-format off
  code += "/**\n";
  code += " * Extracts a signal from CAN data with arbitrary bit positioning.\n";
  code += " *\n";
  code += " * Supports:\n";
  code += " * - Intel (little-endian) and Motorola (big-endian) byte order\n";
  code += " * - Signed and unsigned integers\n";
  code += " * - Signals spanning multiple bytes\n";
  code += " * - Non-byte-aligned signals\n";
  code += " *\n";
  code += " * @param {Uint8Array} data - CAN message data payload\n";
  code += " * @param {number} startBit - Signal start bit position\n";
  code += " * @param {number} length - Signal length in bits\n";
  code += " * @param {boolean} isBigEndian - true for Motorola, false for Intel\n";
  code += " * @param {boolean} isSigned - true for signed integer\n";
  code += " * @returns {number} Extracted raw signal value\n";
  code += " */\n";
  code += "function extractSignal(data, startBit, length, isBigEndian, isSigned) {\n";
  code += "  let value = 0;\n\n";
  code += "  if (isBigEndian) {\n";
  code += "    const startByte = Math.floor(startBit / 8);\n";
  code += "    const startBitInByte = startBit % 8;\n";
  code += "    let bitsRead = 0;\n\n";
  code += "    for (let i = startByte; i < data.length && bitsRead < length; i++) {\n";
  code += "      const bitsInThisByte = Math.min(8 - (i === startByte ? startBitInByte : 0), length - bitsRead);\n";
  code += "      const shift = (i === startByte) ? startBitInByte : 0;\n";
  code += "      const mask = ((1 << bitsInThisByte) - 1) << shift;\n";
  code += "      const bits = (data[i] & mask) >> shift;\n";
  code += "      value = (value << bitsInThisByte) | bits;\n";
  code += "      bitsRead += bitsInThisByte;\n";
  code += "    }\n";
  code += "  } else {\n";
  code += "    const startByte = Math.floor(startBit / 8);\n";
  code += "    const startBitInByte = startBit % 8;\n";
  code += "    let bitsRead = 0;\n\n";
  code += "    for (let i = startByte; i < data.length && bitsRead < length; i++) {\n";
  code += "      const bitsInThisByte = Math.min(8 - (i === startByte ? startBitInByte : 0), length - bitsRead);\n";
  code += "      const shift = (i === startByte) ? startBitInByte : 0;\n";
  code += "      const mask = ((1 << bitsInThisByte) - 1) << shift;\n";
  code += "      const bits = (data[i] & mask) >> shift;\n";
  code += "      value |= bits << bitsRead;\n";
  code += "      bitsRead += bitsInThisByte;\n";
  code += "    }\n";
  code += "  }\n\n";
  code += "  if (isSigned && (value & (1 << (length - 1)))) {\n";
  code += "    value -= (1 << length);\n";
  code += "  }\n\n";
  code += "  return value;\n";
  code += "}\n";
  // clang-format on

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
QString
JSON::DBCImporter::generateMessageDecoder(const QCanMessageDescription &message,
                                          int &datasetIndex)
{
  QString code;
  const auto msgId = static_cast<quint32>(message.uniqueId());
  const auto signalList = message.signalDescriptions();
  const auto startIndex = datasetIndex;

  code += "/**\n";
  code += QString(" * Decodes CAN message: %1 (ID: 0x%2)\n")
              .arg(message.name())
              .arg(QString::number(msgId, 16).toUpper());
  code += " *\n";
  code += QString(" * Signals (%1):\n").arg(signalList.count());

  int idx = startIndex;
  for (const auto &signal : signalList)
  {
    code += QString(" *   [%1] %2 (%3)\n")
                .arg(idx)
                .arg(signal.name())
                .arg(signal.physicalUnit().isEmpty() ? "no unit"
                                                     : signal.physicalUnit());
    ++idx;
  }

  code += " *\n";
  code += " * @param {Uint8Array} data - CAN message data payload\n";
  code += " */\n";
  code += QString("function decode_%1(data) {\n")
              .arg(QString::number(msgId, 16));

  for (const auto &signal : signalList)
  {
    code += generateSignalExtraction(signal);
    code += QString("  values[%1] = value_%2;\n")
                .arg(datasetIndex)
                .arg(sanitizeJavaScriptString(signal.name()));
    ++datasetIndex;
  }

  code += "}\n";

  return code;
}

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
QString
JSON::DBCImporter::generateSignalExtraction(const QCanSignalDescription &signal)
{
  // clang-format off
  QString code;
  const auto isBigEndian = (signal.dataEndian() == QSysInfo::BigEndian);
  const auto isSigned = (signal.dataFormat() == QtCanBus::DataFormat::SignedInteger);
  // clang-format on

  // clang-format off
  code += QString("  const raw_%1 = extractSignal(data, %2, %3, %4, %5);\n")
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(signal.startBit())
              .arg(signal.bitLength())
              .arg(isBigEndian ? "true" : "false")
              .arg(isSigned ? "true" : "false");

  code += QString("  const value_%1 = (raw_%2 * %3) + %4;\n")
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(sanitizeJavaScriptString(signal.name()))
              .arg(signal.factor(), 0, 'g', 10)
              .arg(signal.offset(), 0, 'g', 10);
  // clang-format on

  return code;
}

/**
 * @brief Selects an appropriate group widget type for a CAN message.
 *
 * This function analyzes the signals in a CAN message to automatically
 * assign a suitable group widget type for dashboard visualization:
 *
 * - **NoGroupWidget**: Single signal or all boolean (1-bit) signals
 * - **GPS**: Messages with latitude and longitude signals
 * - **Accelerometer**: Exactly 3 signals with acceleration characteristics
 * - **Gyroscope**: Exactly 3 signals with gyroscope/rotation characteristics
 * - **MultiPlot**: 2-8 plottable numeric signals (excludes counters/status)
 * - **DataGrid**: Messages with counters, status, or non-plottable data
 *
 * The selection is based on:
 * 1. Number of signals in the message
 * 2. Signal names and units (to detect counters vs plottable values)
 * 3. Signal bit lengths (numeric vs boolean)
 *
 * @param message CAN message description from the DBC file.
 * @return Group widget type string.
 */
QString
JSON::DBCImporter::selectGroupWidget(const QCanMessageDescription &message)
{
  const auto signalDescriptions = message.signalDescriptions();
  const auto signalCount = signalDescriptions.count();

  if (signalCount == 1)
    return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

  bool allBoolean = true;
  for (const auto &signal : signalDescriptions)
  {
    if (signal.bitLength() != 1)
    {
      allBoolean = false;
      break;
    }
  }

  if (allBoolean)
    return SerialStudio::groupWidgetId(SerialStudio::NoGroupWidget);

  if (signalCount >= 2)
  {
    bool hasLat = false;
    bool hasLon = false;

    for (const auto &signal : signalDescriptions)
    {
      const auto name = signal.name().toLower();
      if (name.contains("lat"))
        hasLat = true;
      if (name.contains("lon") || name.contains("lng"))
        hasLon = true;
    }

    if (hasLat && hasLon)
      return SerialStudio::groupWidgetId(SerialStudio::GPS);
  }

  if (signalCount == 3)
  {
    int accelCount = 0;
    for (const auto &signal : signalDescriptions)
    {
      const auto name = signal.name().toLower();
      const auto unit = signal.physicalUnit().toLower();
      if (name.contains("accel") || unit.contains("m/s") || unit.contains("g"))
        accelCount++;
    }

    if (accelCount == 3)
      return SerialStudio::groupWidgetId(SerialStudio::Accelerometer);

    int gyroCount = 0;
    for (const auto &signal : signalDescriptions)
    {
      const auto name = signal.name().toLower();
      const auto unit = signal.physicalUnit().toLower();
      if (name.contains("gyro") || name.contains("roll")
          || name.contains("pitch") || name.contains("yaw")
          || unit.contains("deg/s") || unit.contains("rad/s"))
        gyroCount++;
    }

    if (gyroCount == 3)
      return SerialStudio::groupWidgetId(SerialStudio::Gyroscope);
  }

  if (signalCount >= 2 && signalCount <= 8)
  {
    int plottableCount = 0;
    int counterCount = 0;

    for (const auto &signal : signalDescriptions)
    {
      if (signal.bitLength() <= 1)
        continue;

      const auto name = signal.name().toLower();

      if (name.contains("odometer") || name.contains("trip")
          || name.contains("counter") || name.contains("timestamp")
          || name.contains("status"))
      {
        counterCount++;
      }
      else
      {
        plottableCount++;
      }
    }

    if (plottableCount >= 2 && counterCount == 0)
      return SerialStudio::groupWidgetId(SerialStudio::MultiPlot);
  }

  return SerialStudio::groupWidgetId(SerialStudio::DataGrid);
}

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
QString JSON::DBCImporter::sanitizeJavaScriptString(const QString &str)
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
QString
JSON::DBCImporter::selectWidgetForSignal(const QCanSignalDescription &signal)
{
  const auto name = signal.name().toLower();
  const auto unit = signal.physicalUnit().toLower();

  if (name.contains("odometer") || name.contains("trip")
      || name.contains("counter") || name.contains("timestamp")
      || name.contains("status"))
    return QString("");

  if (unit == "%" || (signal.minimum() == 0 && signal.maximum() == 100))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  if (unit.contains("°c") || unit.contains("°f") || unit.contains("degc")
      || unit.contains("degf") || name.contains("temp")
      || name.contains("temperature"))
    return SerialStudio::datasetWidgetId(SerialStudio::Bar);

  if (unit.contains("rpm") || unit.contains("km/h") || unit.contains("mph")
      || unit.contains("v") || unit.contains("a") || unit.contains("psi")
      || unit.contains("bar") || unit.contains("nm") || unit.contains("kpa")
      || unit.contains("mbar") || unit.contains("kg") || unit.contains("deg"))
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  const auto range = signal.maximum() - signal.minimum();
  if (range > 0 && range <= 360)
    return SerialStudio::datasetWidgetId(SerialStudio::Gauge);

  return QString("");
}

/**
 * @brief Counts the total number of signals across all messages.
 *
 * This function iterates through all CAN messages and sums up the number
 * of signals in each message.
 *
 * @param messages List of CAN message descriptions.
 * @return Total number of signals across all messages.
 */
int JSON::DBCImporter::countTotalSignals(
    const QList<QCanMessageDescription> &messages) const
{
  int count = 0;
  for (const auto &message : messages)
    count += message.signalDescriptions().count();

  return count;
}
