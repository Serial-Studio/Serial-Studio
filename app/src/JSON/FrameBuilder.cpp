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

#include <QFileInfo>

#include "IO/Manager.h"
#include "Misc/Utilities.h"

#include "CSV/Player.h"
#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/LemonSqueezy.h"
#endif

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
JSON::FrameBuilder::FrameBuilder()
  : m_frameParser(nullptr)
  , m_opMode(SerialStudio::ProjectFile)
{
  // Read JSON map location
  auto path = m_settings.value("json_map_location", "").toString();
  if (!path.isEmpty())
    loadJsonMap(path);

  // Obtain operation mode from settings
  auto m = m_settings.value("operation_mode", SerialStudio::QuickPlot).toInt();
  setOperationMode(static_cast<SerialStudio::OperationMode>(m));

  // Reload JSON map file when license is activated
#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this, [=] {
            if (!jsonMapFilepath().isEmpty())
              loadJsonMap(jsonMapFilepath());
          });
#endif
}

/**
 * Returns the only instance of the class
 */
JSON::FrameBuilder &JSON::FrameBuilder::instance()
{
  static FrameBuilder singleton;
  return singleton;
}

/**
 * Returns the file path of the loaded JSON map file
 */
QString JSON::FrameBuilder::jsonMapFilepath() const
{
  if (m_jsonMap.isOpen())
  {
    auto fileInfo = QFileInfo(m_jsonMap.fileName());
    return fileInfo.filePath();
  }

  return "";
}

/**
 * Returns the file name of the loaded JSON map file
 */
QString JSON::FrameBuilder::jsonMapFilename() const
{
  if (m_jsonMap.isOpen())
  {
    auto fileInfo = QFileInfo(m_jsonMap.fileName());
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the currently loaded JSON frame.
 *
 * The frame contains the structure of all groups, datasets, and actions
 * parsed from the active JSON project file. It represents the complete
 * configuration used to build the dashboard and manage data parsing.
 *
 * @return A constant reference to the current JSON::Frame.
 */
const JSON::Frame &JSON::FrameBuilder::frame() const
{
  return m_frame;
}

/**
 * Returns a pointer to the currently loaded frame parser editor.
 */
JSON::FrameParser *JSON::FrameBuilder::frameParser() const
{
  return m_frameParser;
}

/**
 * Returns the operation mode
 */
SerialStudio::OperationMode JSON::FrameBuilder::operationMode() const
{
  return m_opMode;
}

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void JSON::FrameBuilder::setupExternalConnections()
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &JSON::FrameBuilder::onConnectedChanged);
  connect(&IO::Manager::instance(), &IO::Manager::frameReceived, this,
          &JSON::FrameBuilder::readData);
}

/**
 * Opens, validates & loads into memory the JSON file in the given @a path.
 */
void JSON::FrameBuilder::loadJsonMap(const QString &path)
{
  // Validate path
  if (path.isEmpty())
    return;

  // Close previous file (if open)
  if (m_jsonMap.isOpen())
  {
    m_frame.clear();
    m_jsonMap.close();
    Q_EMIT jsonFileMapChanged();
  }

  // Try to open the file (read only mode)
  m_jsonMap.setFileName(path);
  if (m_jsonMap.open(QFile::ReadOnly))
  {
    // Read data & validate JSON from file
    QJsonParseError error;
    auto data = m_jsonMap.readAll();
    auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
      m_frame.clear();
      m_jsonMap.close();
      setJsonPathSetting("");
      Misc::Utilities::showMessageBox(
          tr("JSON parse error"), error.errorString(), QMessageBox::Critical);
    }

    // JSON contains no errors, load compacted JSON document & save settings
    else
    {
      // Save settings
      setJsonPathSetting(path);

      // Load frame from data
      m_frame.clear();
      const bool ok = m_frame.read(document.object());

      // Update I/O manager settings
      if (ok && m_frame.isValid())
      {
        if (operationMode() == SerialStudio::ProjectFile)
        {
          IO::Manager::instance().setFinishSequence(m_frame.frameEnd());
          IO::Manager::instance().setStartSequence(m_frame.frameStart());
          IO::Manager::instance().setChecksumAlgorithm(m_frame.checksum());

          IO::Manager::instance().resetFrameReader();
        }
      }

      // Invalid frame data
      else
      {
        m_frame.clear();
        m_jsonMap.close();
        setJsonPathSetting("");
        Misc::Utilities::showMessageBox(
            tr("This file isn’t a valid project file"),
            tr("Make sure it’s a properly formatted JSON project."),
            QMessageBox::Warning);
      }
    }

    // Get rid of warnings
    Q_UNUSED(document);
  }

  // Open error
  else
  {
    setJsonPathSetting("");
    Misc::Utilities::showMessageBox(
        tr("Cannot read JSON file"),
        tr("Please check file permissions & location"), QMessageBox::Critical);
    m_jsonMap.close();
  }

  // Update UI
  Q_EMIT jsonFileMapChanged();
}

/**
 * @brief Assigns an instance to the frame parser to be used to split frame
 *        data/elements into individual parts.
 */
void JSON::FrameBuilder::setFrameParser(JSON::FrameParser *parser)
{
  m_frameParser = parser;
}

/**
 * Changes the operation mode of the JSON parser. There are two possible op.
 * modes:
 *
 * @c kManual serial data only contains the comma-separated values, and we need
 *            to use a JSON map file (given by the user) to know what each value
 *            means. This method is recommended when we need to transfer &
 *            display a large amount of information from the microcontroller
 *            unit to the computer.
 *
 * @c kAutomatic serial data contains the JSON data frame, good for simple
 *               applications or for prototyping.
 */
void JSON::FrameBuilder::setOperationMode(
    const SerialStudio::OperationMode mode)
{
  m_opMode = mode;

  switch (mode)
  {
    case SerialStudio::DeviceSendsJSON:
      IO::Manager::instance().setStartSequence("");
      IO::Manager::instance().setFinishSequence("");
      IO::Manager::instance().setChecksumAlgorithm("");
      break;
    case SerialStudio::ProjectFile:
      IO::Manager::instance().setFinishSequence(m_frame.frameEnd());
      IO::Manager::instance().setStartSequence(m_frame.frameStart());
      IO::Manager::instance().setChecksumAlgorithm(m_frame.checksum());
      break;
    case SerialStudio::QuickPlot:
      IO::Manager::instance().setStartSequence("");
      IO::Manager::instance().setFinishSequence("");
      IO::Manager::instance().setChecksumAlgorithm("");
      break;
    default:
      qWarning() << "Invalid operation mode selected" << mode;
      break;
  }

  m_settings.setValue("operation_mode", mode);
  Q_EMIT operationModeChanged();
}

/**
 * Saves the location of the last valid JSON map file that was opened (if any)
 */
void JSON::FrameBuilder::setJsonPathSetting(const QString &path)
{
  m_settings.setValue(QStringLiteral("json_map_location"), path);
}

/**
 * @brief Handles device connection events and triggers auto-execute actions.
 *
 * This slot is called when the connection state of the serial device changes.
 * If the device has just connected and the application is in project mode
 * (SerialStudio::ProjectFile), this method scans all defined actions in the
 * loaded JSON frame and immediately transmits those marked with the
 * `autoExecuteOnConnect` flag.
 *
 * This is useful for scenarios where a device must receive a command
 * (e.g. "start streaming") before it begins sending data frames.
 *
 * Binary and text-based actions are handled accordingly based on the
 * `binaryData()` flag, and the data is sent via IO::Manager.
 */
void JSON::FrameBuilder::onConnectedChanged()
{
  // Validate that the device is connected
  if (!IO::Manager::instance().isConnected())
    return;

  // Validate that we are in project mode
  if (m_opMode != SerialStudio::ProjectFile)
    return;

  // Auto-execute actions if required
  const auto actions = m_frame.actions();
  for (const auto &action : actions)
  {
    if (action.autoExecuteOnConnect())
      IO::Manager::instance().writeData(action.txByteArray());
  }
}

/**
 * Tries to parse the given data as a JSON document according to the selected
 * operation mode.
 *
 * Possible operation modes:
 * - Auto:   serial data contains the JSON data frame
 * - Manual: serial data only contains the comma-separated values, and we need
 *           to use a JSON map file (given by the user) to know what each value
 *           means
 *
 * If JSON parsing is successfull, then the class shall notify the rest of the
 * application in order to process packet data.
 */
void JSON::FrameBuilder::readData(const QByteArray &data)
{
  // Data empty, abort
  if (data.isEmpty())
    return;

  // Get operation mode
  const auto opMode = operationMode();

  // Serial device sends JSON (auto mode)
  if (opMode == SerialStudio::DeviceSendsJSON)
  {
    auto jsonData = QJsonDocument::fromJson(data).object();
    if (m_frame.read(jsonData))
      Q_EMIT frameChanged(m_frame);
  }

  // Data is separated and parsed by Serial Studio project
  else if (opMode == SerialStudio::ProjectFile && m_frameParser)
  {
    // Obtain state of the app
    const bool csvPlaying = CSV::Player::instance().isOpen();

    // Real-time data, parse data & perform conversion
    QStringList fields;
    if (!csvPlaying)
    {
      switch (JSON::ProjectModel::instance().decoderMethod())
      {
        case SerialStudio::PlainText:
          fields = m_frameParser->parse(QString::fromUtf8(data));
          break;
        case SerialStudio::Hexadecimal:
          fields = m_frameParser->parse(QString::fromUtf8(data.toHex()));
          break;
        case SerialStudio::Base64:
          fields = m_frameParser->parse(QString::fromUtf8(data.toBase64()));
          break;
        case SerialStudio::Binary:
          fields = m_frameParser->parse(data);
          break;
        default:
          fields = m_frameParser->parse(QString::fromUtf8(data));
          break;
      }
    }

    // CSV data, no need to perform conversions or use frame parser
    else
      fields = QString::fromUtf8(data.simplified()).split(',');

    // Replace data in frame
    for (int g = 0; g < m_frame.groupCount(); ++g)
    {
      auto &group = m_frame.m_groups[g];
      for (int d = 0; d < group.datasetCount(); ++d)
      {
        auto &dataset = group.m_datasets[d];
        const auto index = dataset.index();
        if (index <= fields.count() && index > 0)
          dataset.m_value = fields.at(index - 1);
      }
    }

    // Update user interface
    Q_EMIT frameChanged(m_frame);
  }

  // Data is separated by comma separated values
  else if (opMode == SerialStudio::QuickPlot)
  {
    // Parse audio data
#ifdef BUILD_COMMERCIAL
    const auto busType = IO::Manager::instance().busType();
    if (busType == SerialStudio::BusType::Audio)
    {
      // Get reference to Audio driver
      const auto &audio = IO::Drivers::Audio::instance();
      const auto format = audio.config().capture.format;
      const auto sampleRate = audio.config().sampleRate;

      // Compute audio parameters
      double maxValue = 1.0;
      double minValue = 0.0;
      switch (format)
      {
        case ma_format_u8:
          maxValue = 255;
          minValue = 0;
          break;
        case ma_format_s16:
          maxValue = 32767;
          minValue = -32768;
          break;
        case ma_format_s32:
          maxValue = 2147483647;
          minValue = -2147483648;
          break;
        case ma_format_f32:
          maxValue = 1.0;
          minValue = -1.0;
          break;
        default:
          break;
      }

      // Obtain microphone values for each channel
      int index = 1;
      auto channels = data.split(',');
      QVector<JSON::Dataset> datasets;
      for (const auto &channel : std::as_const(channels))
      {
        JSON::Dataset dataset;
        dataset.m_fft = true;
        dataset.m_groupId = 0;
        dataset.m_graph = true;
        dataset.m_index = index;
        dataset.m_max = maxValue;
        dataset.m_min = minValue;
        dataset.m_fftSamples = 2048;
        dataset.m_fftWindowFn = "Hann";
        dataset.m_fftSamplingRate = sampleRate;
        dataset.m_title = tr("Channel %1").arg(index);
        dataset.m_value = QString::fromUtf8(channel);
        datasets.append(dataset);

        ++index;
      }

      // Create the holder group
      JSON::Group group(0);
      group.m_datasets = datasets;
      group.m_title = tr("Multiple Plots");
      if (index > 2)
        group.m_widget = QStringLiteral("multiplot");

      // Create a project frame object
      JSON::Frame frame;
      frame.m_title = tr("Quick Plot");
      frame.m_groups.append(group);

      // Update user interface
      frame.buildUniqueIds();
      Q_EMIT frameChanged(frame);
      return;
    }
#endif

    // Obtain fields from data frame
    auto fields = data.split(',');

    // Create datasets from the data
    int channel = 1;
    QVector<JSON::Dataset> datasets;
    for (const auto &field : std::as_const(fields))
    {
      JSON::Dataset dataset;
      dataset.m_groupId = 0;
      dataset.m_index = channel;
      dataset.m_title = tr("Channel %1").arg(channel);
      dataset.m_value = QString::fromUtf8(field);
      dataset.m_graph = false;
      datasets.append(dataset);

      ++channel;
    }

    // Create a project frame from the groups
    JSON::Frame frame;
    frame.m_title = tr("Quick Plot");

    // Create a datagrid group from the dataset array
    JSON::Group datagrid(0);
    datagrid.m_datasets = datasets;
    datagrid.m_title = tr("Quick Plot Data");
    datagrid.m_widget = QStringLiteral("datagrid");
    for (int i = 0; i < datagrid.m_datasets.count(); ++i)
      datagrid.m_datasets[i].m_graph = true;

    // Append datagrid to frame
    frame.m_groups.append(datagrid);

    // Create a multiplot group when multiple datasets are found
    if (datasets.count() > 1)
    {
      JSON::Group multiplot(1);
      multiplot.m_datasets = datasets;
      multiplot.m_title = tr("Multiple Plots");
      multiplot.m_widget = QStringLiteral("multiplot");
      for (int i = 0; i < multiplot.m_datasets.count(); ++i)
        multiplot.m_datasets[i].m_groupId = 1;

      frame.m_groups.append(multiplot);
    }

    // Update user interface
    frame.buildUniqueIds();
    Q_EMIT frameChanged(frame);
  }
}
