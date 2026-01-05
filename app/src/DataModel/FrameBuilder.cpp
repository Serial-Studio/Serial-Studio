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
#include "CSV/Player.h"
#include "CSV/Export.h"
#include "MDF4/Export.h"
#include "MDF4/Player.h"
#include "UI/Dashboard.h"
#include "Plugins/Server.h"
#include "Misc/Utilities.h"
#include "Misc/JsonValidator.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/FrameBuilder.h"

#ifdef BUILD_COMMERCIAL
#  include "IO/Drivers/Audio.h"
#  include "Licensing/LemonSqueezy.h"
#endif

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

/**
 * Initializes the DataModel Parser class and connects appropiate SIGNALS/SLOTS
 */
DataModel::FrameBuilder::FrameBuilder()
  : m_quickPlotChannels(-1)
  , m_quickPlotHasHeader(false)
  , m_frameParser(nullptr)
  , m_opMode(SerialStudio::ProjectFile)
{
  // Read JSON map location
  auto path = m_settings.value("json_map_location", "").toString();
  if (!path.isEmpty())
  {
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && fileInfo.isFile())
      loadJsonMap(path);
    else
    {
      qWarning() << "Saved JSON map path does not exist:" << path;
      m_settings.setValue("json_map_location", "");
    }
  }

  // Obtain operation mode from settings
  auto m = m_settings.value("operation_mode", SerialStudio::QuickPlot).toInt();
  setOperationMode(static_cast<SerialStudio::OperationMode>(m));

  // Reload JSON map file when license is activated
#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this, [=, this] {
            if (!jsonMapFilepath().isEmpty())
              loadJsonMap(jsonMapFilepath());
          });
#endif
}

/**
 * Returns the only instance of the class
 */
DataModel::FrameBuilder &DataModel::FrameBuilder::instance()
{
  static FrameBuilder singleton;
  return singleton;
}

/**
 * Returns the file path of the loaded DataModel map file
 */
QString DataModel::FrameBuilder::jsonMapFilepath() const
{
  if (m_jsonMap.isOpen())
  {
    auto fileInfo = QFileInfo(m_jsonMap.fileName());
    return fileInfo.filePath();
  }

  return "";
}

/**
 * Returns the file name of the loaded DataModel map file
 */
QString DataModel::FrameBuilder::jsonMapFilename() const
{
  if (m_jsonMap.isOpen())
  {
    auto fileInfo = QFileInfo(m_jsonMap.fileName());
    return fileInfo.fileName();
  }

  return "";
}

/**
 * @brief Returns the currently loaded DataModel frame.
 *
 * The frame contains the structure of all groups, datasets, and actions
 * parsed from the active DataModel project file. It represents the complete
 * configuration used to build the dashboard and manage data parsing.
 *
 * @return A constant reference to the current DataModel::Frame.
 */
const DataModel::Frame &DataModel::FrameBuilder::frame() const
{
  return m_frame;
}

/**
 * Returns a pointer to the currently loaded frame parser editor.
 */
DataModel::FrameParser *DataModel::FrameBuilder::frameParser() const
{
  return m_frameParser;
}

/**
 * Returns the operation mode
 */
SerialStudio::OperationMode DataModel::FrameBuilder::operationMode() const
{
  return m_opMode;
}

/**
 * @brief Registers explicit channel headers for Quick Plot mode
 *
 * Allows file players (CSV/MDF4) to explicitly set channel names from
 * their header rows. For live streaming data, automatic header detection
 * in parseQuickPlotFrame() will still work.
 *
 * @param headers List of channel names to use
 */
void DataModel::FrameBuilder::registerQuickPlotHeaders(
    const QStringList &headers)
{
  if (!headers.isEmpty())
  {
    m_quickPlotHasHeader = true;
    m_quickPlotChannelNames = headers;
  }

  else
  {
    m_quickPlotHasHeader = false;
    m_quickPlotChannelNames.clear();
  }
}

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void DataModel::FrameBuilder::setupExternalConnections()
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &DataModel::FrameBuilder::onConnectedChanged);
}

/**
 * @brief Loads and validates a JSON project file for frame mapping.
 *
 * This function handles the complete lifecycle of loading a JSON project file:
 * 1. Validates the file path
 * 2. Closes any previously open file
 * 3. Opens and reads the new file
 * 4. Validates JSON structure with security checks
 * 5. Parses frame structure
 * 6. Updates I/O manager settings
 *
 * **Security:** Uses JsonValidator to prevent malicious JSON attacks.
 * **Thread Safety:** Must be called from main thread (updates UI state).
 *
 * @param path Absolute path to JSON project file
 */
void DataModel::FrameBuilder::loadJsonMap(const QString &path)
{
  if (path.isEmpty())
    return;

  // Close and cleanup any previously loaded JSON file
  if (m_jsonMap.isOpen())
  {
    clear_frame(m_frame);
    m_jsonMap.close();
    Q_EMIT jsonFileMapChanged();
  }

  // Attempt to open the new file
  m_jsonMap.setFileName(path);
  if (!m_jsonMap.open(QFile::ReadOnly)) [[unlikely]]
  {
    setJsonPathSetting("");
    Misc::Utilities::showMessageBox(
        tr("Cannot read JSON file"),
        tr("Please check file permissions & location"), QMessageBox::Critical);
    m_jsonMap.close();
    Q_EMIT jsonFileMapChanged();
    return;
  }

  // Read and validate JSON with security bounds checking
  const auto data = m_jsonMap.readAll();
  const auto result = Misc::JsonValidator::parseAndValidate(data);

  if (!result.valid) [[unlikely]]
  {
    clear_frame(m_frame);
    m_jsonMap.close();
    setJsonPathSetting("");
    Misc::Utilities::showMessageBox(tr("JSON validation error"),
                                    result.errorMessage, QMessageBox::Critical);
    Q_EMIT jsonFileMapChanged();
    return;
  }

  // Parse the validated JSON document into frame structure
  setJsonPathSetting(path);
  clear_frame(m_frame);

  const auto document = result.document;
  const bool ok = read(m_frame, document.object());

  if (!ok) [[unlikely]]
  {
    clear_frame(m_frame);
    m_jsonMap.close();
    setJsonPathSetting("");
    Misc::Utilities::showMessageBox(
        tr("This file isn't a valid project file"),
        tr("Make sure it's a properly formatted JSON project."),
        QMessageBox::Warning);
    Q_EMIT jsonFileMapChanged();
    return;
  }

  // Successfully parsed - update I/O manager with frame delimiters and checksum
  if (operationMode() == SerialStudio::ProjectFile)
  {
    read_io_settings(m_frameStart, m_frameFinish, m_checksum,
                     document.object());

    IO::Manager::instance().setStartSequence(m_frameStart);
    IO::Manager::instance().setFinishSequence(m_frameFinish);
    IO::Manager::instance().setChecksumAlgorithm(m_checksum);
    IO::Manager::instance().resetFrameReader();
  }

  // Notify UI that JSON file has been successfully loaded
  Q_EMIT jsonFileMapChanged();
}

/**
 * @brief Assigns an instance to the frame parser to be used to split frame
 *        data/elements into individual parts.
 */
void DataModel::FrameBuilder::setFrameParser(DataModel::FrameParser *parser)
{
  m_frameParser = parser;
}

/**
 * Changes the operation mode of the DataModel parser. There are two possible
 * op. modes:
 *
 * @c kManual serial data only contains the comma-separated values, and we need
 *            to use a DataModel map file (given by the user) to know what each
 * value means. This method is recommended when we need to transfer & display a
 * large amount of information from the microcontroller unit to the computer.
 *
 * @c kAutomatic serial data contains the DataModel data frame, good for simple
 *               applications or for prototyping.
 */
void DataModel::FrameBuilder::setOperationMode(
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
      IO::Manager::instance().setStartSequence(m_frameStart);
      IO::Manager::instance().setFinishSequence(m_frameFinish);
      IO::Manager::instance().setChecksumAlgorithm(m_checksum);
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

//------------------------------------------------------------------------------
// Hotpath data processing functions
//------------------------------------------------------------------------------

/**
 * @brief Dispatches raw data to the appropriate frame parser based on the
 * current operation mode.
 *
 * This is a hotpath function executed at high frequency.
 *
 * It routes the incoming binary data to the correct parsing strategy:
 * - If the device sends DataModel directly, parses it using QJsonDocument.
 * - If using a project file, delegates parsing to the configured frame parser.
 * - If in Quick Plot mode, parses CSV-like data for plotting.
 *
 * @param data Raw binary input data to be processed.
 */
void DataModel::FrameBuilder::hotpathRxFrame(const QByteArray &data)
{
  switch (operationMode())
  {
    case SerialStudio::QuickPlot:
      parseQuickPlotFrame(data);
      break;
    case SerialStudio::ProjectFile:
      parseProjectFrame(data);
      break;
    case SerialStudio::DeviceSendsJSON:
      auto result = Misc::JsonValidator::parseAndValidate(data);
      if (result.valid && read(m_rawFrame, result.document.object())) [[likely]]
        hotpathTxFrame(m_rawFrame);
      break;
  }
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

/**
 * @brief Handles device connection events and triggers auto-execute actions.
 *
 * This slot is called when the connection state of the serial device changes.
 * If the device has just connected and the application is in project mode
 * (SerialStudio::ProjectFile), this method scans all defined actions in the
 * loaded DataModel frame and immediately transmits those marked with the
 * `autoExecuteOnConnect` flag.
 *
 * This is useful for scenarios where a device must receive a command
 * (e.g. "start streaming") before it begins sending data frames.
 *
 * Binary and text-based actions are handled accordingly based on the
 * `binaryData()` flag, and the data is sent via IO::Manager.
 */
void DataModel::FrameBuilder::onConnectedChanged()
{
  // Reset quick plot field count (headers are preserved and reset explicitly by
  // players)
  m_quickPlotChannels = -1;

  // Validate that the device is connected
  if (!IO::Manager::instance().isConnected())
    return;

  // Validate that we are in project mode
  if (m_opMode != SerialStudio::ProjectFile)
    return;

  // Reset the frame parser execution context
  if (m_frameParser)
    m_frameParser->readCode();

  // Auto-execute actions if required
  const auto &actions = m_frame.actions;
  for (const auto &action : actions)
  {
    if (action.autoExecuteOnConnect)
      IO::Manager::instance().writeData(get_tx_bytes(action));
  }
}

/**
 * Saves the location of the last valid DataModel map file that was opened (if
 * any)
 */
void DataModel::FrameBuilder::setJsonPathSetting(const QString &path)
{
  m_settings.setValue(QStringLiteral("json_map_location"), path);
}

//------------------------------------------------------------------------------
// Frame parsing
//------------------------------------------------------------------------------

/**
 * @brief Parses a project frame using the configured decoding method.
 *
 * Converts incoming binary data into structured field values based on the
 * decoder method defined in the project model (e.g., plain text, hex, base64,
 * binary). If CSV playback is active, skips decoding and parses the simplified
 * string directly. Updates all frame datasets with the parsed values and
 * triggers a UI update.
 *
 * @param data Raw binary input to be decoded and assigned to frame datasets.
 *
 * @note This function is part of the high-frequency data path. Optimize later.
 */
void DataModel::FrameBuilder::parseProjectFrame(const QByteArray &data)
{
  // Real-time data, parse data & perform conversion
  QStringList channels;
  channels.reserve(64);
  if (!SerialStudio::isAnyPlayerOpen() && m_frameParser) [[likely]]
  {
    const auto decoderMethod
        = DataModel::ProjectModel::instance().decoderMethod();
    switch (decoderMethod)
    {
      case SerialStudio::Hexadecimal:
        channels = m_frameParser->parse(QString::fromUtf8(data.toHex()));
        break;
      case SerialStudio::Base64:
        channels = m_frameParser->parse(QString::fromUtf8(data.toBase64()));
        break;
      case SerialStudio::Binary:
        channels = m_frameParser->parse(data);
        break;
      case SerialStudio::PlainText:
      default:
        channels = m_frameParser->parse(QString::fromUtf8(data));
        break;
    }
  }

  // CSV data, no need to perform conversions or use frame parser
  else
    channels = QString::fromUtf8(data).split(',', Qt::SkipEmptyParts);

  // Process data
  if (!channels.isEmpty())
  {
    // Replace data in frame
    auto *channelData = channels.data();
    const int channelCount = channels.size();
    const size_t groupCount = m_frame.groups.size();
    for (size_t g = 0; g < groupCount; ++g)
    {
      auto &group = m_frame.groups[g];
      const size_t datasetCount = group.datasets.size();
      for (size_t d = 0; d < datasetCount; ++d)
      {
        auto &dataset = group.datasets[d];
        const int idx = dataset.index;
        if (idx > 0 && idx <= channelCount) [[likely]]
        {
          QString &value = channelData[idx - 1];
          dataset.value = std::move(value);
          dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
        }
      }
    }

    // Update user interface
    hotpathTxFrame(m_frame);
  }
}

/**
 * @brief Parses and updates the Quick Plot frame with incoming comma-separated
 *       values.
 *
 * Converts UTF-8 encoded CSV data into channel values. If the number of
 * channels has changed since the last call, it rebuilds the internal frame
 * layout via buildQuickPlotFrame(). Then updates the dataset values in the
 * existing frame and publishes it to the UI.
 *
 * @param data UTF-8 encoded, comma-separated channel values for plotting.
 *
 * @note This function is part of the high-frequency data path. Optimize later.
 */
void DataModel::FrameBuilder::parseQuickPlotFrame(const QByteArray &data)
{
  // Parse comma-separated values
  int start = 0;
  QStringList channels;
  const auto str = QString::fromUtf8(data);
  const int dataLength = str.size();

  if (m_quickPlotChannels > 0) [[likely]]
    channels.reserve(m_quickPlotChannels);
  else
    channels.reserve(64);

  for (int i = 0; i <= dataLength; ++i)
  {
    if (i == dataLength || str[i] == ',')
    {
      const auto channel = str.mid(start, i - start).trimmed();
      if (!channel.isEmpty())
        channels.append(channel);

      start = i + 1;
    }
  }

  // Process data
  const int channelCount = channels.size();
  if (channelCount > 0)
  {
    if (m_quickPlotChannels == -1)
    {
      bool allNonNumeric = true;
      for (const auto &channel : std::as_const(channels))
      {
        bool isNumeric = false;
        channel.toDouble(&isNumeric);
        if (isNumeric)
        {
          allNonNumeric = false;
          break;
        }
      }

      if (allNonNumeric)
      {
        m_quickPlotHasHeader = true;
        m_quickPlotChannelNames = channels;
        return;
      }
    }

    // Rebuild frame if channel count changed
    if (channelCount != m_quickPlotChannels) [[unlikely]]
    {
      buildQuickPlotFrame(channels);
      m_quickPlotChannels = channelCount;
    }

    // Replace data in frame
    auto *channelData = channels.data();
    const size_t groupCount = m_quickPlotFrame.groups.size();
    for (size_t g = 0; g < groupCount; ++g)
    {
      auto &group = m_quickPlotFrame.groups[g];
      const size_t datasetCount = group.datasets.size();
      for (size_t d = 0; d < datasetCount; ++d)
      {
        auto &dataset = group.datasets[d];
        const int idx = dataset.index;
        if (idx > 0 && idx <= channelCount) [[likely]]
        {
          QString &value = channelData[idx - 1];
          dataset.value = std::move(value);
          dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
        }
      }
    }

    // Process the frame
    hotpathTxFrame(m_quickPlotFrame);
  }
}

//------------------------------------------------------------------------------
// Quick-plot project generation functions
//------------------------------------------------------------------------------

/**
 * @brief Rebuilds the internal frame structure for Quick Plot mode based on
 *        current channel count.
 *
 * Constructs a new `DataModel::Frame` and associated
 * `DataModel::Group`/`Dataset` layout using the provided channel values. If the
 * build is configured for commercial use and the audio bus is active, the
 * function includes additional metadata required for FFT plotting (e.g., sample
 * rate, min/max). Otherwise, it builds a generic datagrid and multiplot view
 * for standard Quick Plot channels.
 *
 * This function is only called when the number of input channels changes, not
 * on every data frame.
 *
 * @param channels List of channel values received in the most recent data
 *                 frame.
 *
 * @note This function allocates and initializes all datasets and groups from
 *       scratch, which is expensive. It should be called only when the number
 *       of channels changes. Avoid calling this in the real-time path unless
 *       necessary.
 */
void DataModel::FrameBuilder::buildQuickPlotFrame(const QStringList &channels)
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
      case ma_format_s24:
        maxValue = 8388607;
        minValue = -8388608;
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
    std::vector<DataModel::Dataset> datasets;
    datasets.reserve(channels.count());
    for (const auto &channel : std::as_const(channels))
    {
      DataModel::Dataset dataset;
      dataset.fft = true;
      dataset.plt = true;
      dataset.groupId = 0;
      dataset.index = index;
      dataset.value = channel;
      dataset.pltMax = maxValue;
      dataset.pltMin = minValue;
      dataset.fftMax = maxValue;
      dataset.fftMin = minValue;
      dataset.fftSamples = 2048;
      dataset.fftSamplingRate = sampleRate;

      if (m_quickPlotHasHeader && index - 1 < m_quickPlotChannelNames.size())
        dataset.title = m_quickPlotChannelNames[index - 1];
      else
        dataset.title = tr("Channel %1").arg(index);

      dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
      datasets.push_back(dataset);

      ++index;
    }

    // Create the holder group
    DataModel::Group group;
    group.groupId = 0;
    group.datasets = datasets;
    group.title = tr("Audio Input");
    if (index > 2)
      group.widget = QStringLiteral("multiplot");

    // Create a project frame object
    clear_frame(m_quickPlotFrame);
    m_quickPlotFrame.title = tr("Quick Plot");
    m_quickPlotFrame.groups.push_back(group);

    // Build dataset UIDs
    finalize_frame(m_quickPlotFrame);
    return;
  }
#endif

  // Create datasets from the data
  int idx = 1;
  std::vector<DataModel::Dataset> datasets;
  datasets.reserve(channels.count());
  for (const auto &channel : std::as_const(channels))
  {
    DataModel::Dataset dataset;
    dataset.groupId = 0;
    dataset.index = idx;
    dataset.plt = false;
    dataset.value = channel;

    if (m_quickPlotHasHeader && idx - 1 < m_quickPlotChannelNames.size())
      dataset.title = m_quickPlotChannelNames[idx - 1];
    else
      dataset.title = tr("Channel %1").arg(idx);

    dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
    datasets.push_back(dataset);

    ++idx;
  }

  // Create a project frame from the groups
  clear_frame(m_quickPlotFrame);
  m_quickPlotFrame.title = tr("Quick Plot");

  // Create a datagrid group from the dataset array
  DataModel::Group datagrid;
  datagrid.groupId = 0;
  datagrid.datasets = datasets;
  datagrid.title = tr("Quick Plot Data");
  datagrid.widget = QStringLiteral("datagrid");
  for (size_t i = 0; i < datagrid.datasets.size(); ++i)
    datagrid.datasets[i].plt = true;

  // Append datagrid to frame
  m_quickPlotFrame.groups.push_back(datagrid);

  // Create a multiplot group when multiple datasets are found
  if (datasets.size() > 1)
  {
    DataModel::Group multiplot;
    multiplot.groupId = 1;
    multiplot.datasets = datasets;
    multiplot.title = tr("Multiple Plots");
    multiplot.widget = QStringLiteral("multiplot");
    for (size_t i = 0; i < multiplot.datasets.size(); ++i)
      multiplot.datasets[i].groupId = 1;

    m_quickPlotFrame.groups.push_back(multiplot);
  }

  // Build dataset UIDs
  finalize_frame(m_quickPlotFrame);
}

//------------------------------------------------------------------------------
// Hotpath data publishing functions
//------------------------------------------------------------------------------

/**
 * @brief Publishes a fully constructed DataModel frame to all registered output
 *        modules.
 *
 * Dispatches the provided frame to:
 * - The dashboard UI for real-time visualization.
 * - The CSV export system for logging.
 * - The plugin server for external data consumption.
 *
 * @param frame The fully populated frame to distribute.
 *
 * @note This function touches multiple subsystems, including I/O and UI.
 *       Do not call it in tight loops unless you're sure the frame is
 *       finalized.
 */
void DataModel::FrameBuilder::hotpathTxFrame(const DataModel::Frame &frame)
{
  static auto &csvExport = CSV::Export::instance();
  static auto &mdf4Export = MDF4::Export::instance();
  static auto &dashboard = UI::Dashboard::instance();
  static auto &pluginsServer = Plugins::Server::instance();

  auto shared_frame = std::make_shared<const DataModel::Frame>(frame);
  auto timestamped_frame
      = std::make_shared<DataModel::TimestampedFrame>(shared_frame);

  dashboard.hotpathRxFrame(shared_frame);
  csvExport.hotpathTxFrame(timestamped_frame);
  mdf4Export.hotpathTxFrame(timestamped_frame);
  pluginsServer.hotpathTxFrame(timestamped_frame);
}
