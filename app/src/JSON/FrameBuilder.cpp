/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QFileInfo>
#include <QFileDialog>
#include <QRegularExpression>

#include "IO/Manager.h"
#include "Misc/Utilities.h"

#include "JSON/ProjectModel.h"
#include "JSON/FrameBuilder.h"

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
JSON::FrameBuilder::FrameBuilder()
  : m_opMode(ProjectFile)
{
  connect(&IO::Manager::instance(), &IO::Manager::frameReceived, this,
          &JSON::FrameBuilder::readData);

  // Read JSON map location
  auto path = m_settings.value("json_map_location", "").toString();
  if (!path.isEmpty())
    loadJsonMap(path);

  // Obtain operation mode from settings
  auto mode = m_settings.value("operation_mode", CommaSeparatedValues).toInt();
  setOperationMode(static_cast<OperationMode>(mode));
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
 * Returns the operation mode
 */
JSON::FrameBuilder::OperationMode JSON::FrameBuilder::operationMode() const
{
  return m_opMode;
}

/**
 * Creates a file dialog & lets the user select the JSON file map
 */
void JSON::FrameBuilder::loadJsonMap()
{
  const auto file = QFileDialog::getOpenFileName(
      nullptr, tr("Select JSON map file"),
      JSON::ProjectModel::instance().jsonProjectsPath(),
      tr("JSON files") + QStringLiteral(" (*.json)"));

  if (!file.isEmpty())
    loadJsonMap(file);
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
      Misc::Utilities::showMessageBox(tr("JSON parse error"),
                                      error.errorString());
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
        if (operationMode() == ProjectFile)
        {
          IO::Manager::instance().setFinishSequence(m_frame.frameEnd());
          IO::Manager::instance().setStartSequence(m_frame.frameStart());
          IO::Manager::instance().setSeparatorSequence(m_frame.separator());
        }
      }

      // Invalid frame data
      else
      {
        m_frame.clear();
        m_jsonMap.close();
        setJsonPathSetting("");
        Misc::Utilities::showMessageBox(tr("Invalid JSON project format"));
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
        tr("Please check file permissions & location"));
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
void JSON::FrameBuilder::setOperationMode(const OperationMode mode)
{
  m_opMode = mode;

  switch (mode)
  {
    case DeviceSendsJSON:
      IO::Manager::instance().setStartSequence("");
      IO::Manager::instance().setFinishSequence("");
      IO::Manager::instance().setSeparatorSequence("");
      break;
    case ProjectFile:
      IO::Manager::instance().setFinishSequence(m_frame.frameEnd());
      IO::Manager::instance().setStartSequence(m_frame.frameStart());
      IO::Manager::instance().setSeparatorSequence(m_frame.separator());
      break;
    case CommaSeparatedValues:
      IO::Manager::instance().setStartSequence("");
      IO::Manager::instance().setFinishSequence("");
      IO::Manager::instance().setSeparatorSequence("");
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

  // Serial device sends JSON (auto mode)
  if (operationMode() == JSON::FrameBuilder::DeviceSendsJSON)
  {
    auto jsonData = QJsonDocument::fromJson(data).object();
    if (m_frame.read(jsonData))
      Q_EMIT frameChanged(m_frame);
  }

  // Data is separated and parsed by Serial Studio project
  else if (operationMode() == JSON::FrameBuilder::ProjectFile && m_frameParser)
  {
    // Convert binary frame data to a string
    QString frameData;
    QString separator;
    switch (JSON::ProjectModel::instance().decoderMethod())
    {
      case WC::PlainText:
        frameData = QString::fromUtf8(data);
        separator = IO::Manager::instance().separatorSequence();
        break;
      case WC::Hexadecimal:
        frameData = QString::fromUtf8(data.toHex());
        separator = "";
        break;
      case WC::Base64:
        frameData = QString::fromUtf8(data.toBase64());
        separator = "";
        break;
      default:
        frameData = QString::fromUtf8(data);
        separator = IO::Manager::instance().separatorSequence();
        break;
    }

    // Get fields from frame parser function
    auto fields = m_frameParser->parse(frameData, separator);

    // Replace data in frame
    for (auto g = m_frame.m_groups.begin(); g != m_frame.m_groups.end(); ++g)
    {
      for (auto d = g->m_datasets.begin(); d != g->m_datasets.end(); ++d)
      {
        const auto index = d->index();
        if (index <= fields.count())
          d->m_value = fields.at(index - 1);
      }
    }

    // Update user interface
    Q_EMIT frameChanged(m_frame);
  }

  // Data is separated by comma separated values
  else if (operationMode() == CommaSeparatedValues)
  {
    // Obtain fields from data frame
    auto fields = data.split(',');

    // Create datasets from the data
    int channel = 1;
    QVector<JSON::Dataset> datasets;
    for (const auto &field : fields)
    {
      JSON::Dataset dataset;
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
    JSON::Group datagrid;
    datagrid.m_datasets = datasets;
    datagrid.m_title = tr("Data Grid");
    datagrid.m_widget = QStringLiteral("datagrid");
    for (int i = 0; i < datagrid.m_datasets.count(); ++i)
      datagrid.m_datasets[i].m_graph = true;

    // Append datagrid to frame
    frame.m_groups.append(datagrid);

    // Create a multiplot group when multiple datasets are found
    if (datasets.count() > 1)
    {
      JSON::Group plots;
      plots.m_datasets = datasets;
      plots.m_title = tr("Multiple Plots");
      plots.m_widget = QStringLiteral("multiplot");
      frame.m_groups.append(plots);
    }

    Q_EMIT frameChanged(frame);
  }
}
