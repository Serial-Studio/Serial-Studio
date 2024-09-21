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

#include "Export.h"

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopServices>

#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "Misc/Utilities.h"
#include "Misc/TimerEvents.h"

/**
 * Connect JSON Parser & Serial Manager signals to begin registering JSON
 * dataframes into JSON list.
 */
CSV::Export::Export()
  : m_exportEnabled(true)
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &Export::closeFile);
  connect(&UI::Dashboard::instance(), &UI::Dashboard::frameReceived, this,
          &Export::registerFrame);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Export::writeValues);
}

/**
 * Close file & finnish write-operations before destroying the class
 */
CSV::Export::~Export()
{
  closeFile();
}

/**
 * Returns a pointer to the only instance of this class
 */
CSV::Export &CSV::Export::instance()
{
  static Export singleton;
  return singleton;
}

/**
 * Returns @c true if the CSV output file is open
 */
bool CSV::Export::isOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * Returns @c true if CSV export is enabled
 */
bool CSV::Export::exportEnabled() const
{
  return m_exportEnabled;
}

/**
 * Open the current CSV file in the Explorer/Finder window
 */
void CSV::Export::openCurrentCsv()
{
  if (isOpen())
    Misc::Utilities::revealFile(m_csvFile.fileName());
  else
    Misc::Utilities::showMessageBox(tr("CSV file not open"),
                                    tr("Cannot find CSV export file!"));
}

/**
 * Enables or disables data export
 */
void CSV::Export::setExportEnabled(const bool enabled)
{
  m_exportEnabled = enabled;
  Q_EMIT enabledChanged();

  if (!exportEnabled() && isOpen())
  {
    m_frames.clear();
    closeFile();
  }
}

/**
 * Write all remaining JSON frames & close the CSV file
 */
void CSV::Export::closeFile()
{
  if (isOpen())
  {
    while (!m_frames.isEmpty())
      writeValues();

    m_csvFile.close();
    m_textStream.setDevice(Q_NULLPTR);

    Q_EMIT openChanged();
  }
}

/**
 * Creates a CSV file based on the JSON frames contained in the JSON list.
 * @note This function is called periodically every 1 second.
 */
void CSV::Export::writeValues()
{
  // Write each frame
  for (auto i = m_frames.begin(); i != m_frames.end(); ++i)
  {
    // File not open, create it & add cell titles
    if (!isOpen() && exportEnabled())
      createCsvFile(*i);

    // Obtain frame data
    const auto &data = i->data;
    const auto &rxTime = i->rxDateTime;

    // Write RX date/time
    const auto format = QStringLiteral("yyyy/MM/dd/ HH:mm:ss::zzz");
    m_textStream << rxTime.toString(format) << QStringLiteral(",");

    // Write frame data
    const auto &groups = data.groups();
    for (auto g = groups.begin(); g != groups.end(); ++g)
    {
      const auto &datasets = g->datasets();
      for (auto d = datasets.begin(); d != datasets.end(); ++d)
      {
        m_textStream << d->value();
        if (d->datasetId() < datasets.count() - 1)
          m_textStream << QStringLiteral(",");
        else
          m_textStream << QStringLiteral("\n");
      }
    }
  }

  // Flush the stream to writte it to the hard disk
  m_textStream.flush();

  // Clear frames
  m_frames.clear();
}

/**
 * Creates a new CSV file corresponding to the current project title & field
 * count
 */
void CSV::Export::createCsvFile(const CSV::TimestampFrame &frame)
{
  // Obtain frame data
  const auto &data = frame.data;
  const auto &rxTime = frame.rxDateTime;

  // Get file name
  const auto fileName = rxTime.toString(QStringLiteral("HH-mm-ss")) + ".csv";

  // Get path
  const auto format = rxTime.toString("yyyy/MMM/dd/");
  const QString path = QStringLiteral("%1/Documents/%2/CSV/%3/%4")
                           .arg(QDir::homePath(), qApp->applicationName(),
                                data.title(), format);

  // Generate file path if required
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(".");

  // Open file
  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    Misc::Utilities::showMessageBox(tr("CSV File Error"),
                                    tr("Cannot open CSV file for writing!"));
    closeFile();
    return;
  }

  // Add cell titles & force UTF-8 codec
  m_textStream.setDevice(&m_csvFile);
  m_textStream.setGenerateByteOrderMark(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  m_textStream.setCodec("UTF-8");
#else
  m_textStream.setEncoding(QStringConverter::Utf8);
#endif

  // Get number of fields by counting datasets with non-duplicated indexes
  QVector<int> fields;
  QVector<QString> headers;
  const auto &groups = data.groups();
  for (auto g = groups.constBegin(); g != groups.constEnd(); ++g)
  {
    const auto &datasets = g->datasets();
    for (auto d = datasets.constBegin(); d != datasets.constEnd(); ++d)
    {
      if (!fields.contains(d->index()))
      {
        fields.append(d->index());
        headers.append(QString("%1/%2").arg(g->title(), d->title()));
      }
    }
  }

  // Add CSV header
  m_textStream << QStringLiteral("RX Date/Time,");
  for (auto i = 0; i < fields.count(); ++i)
  {
    m_textStream << headers.at(i);
    if (i < fields.count() - 1)
      m_textStream << QStringLiteral(",");
    else
      m_textStream << QStringLiteral("\n");
  }

  // Update UI
  Q_EMIT openChanged();
}

/**
 * Appends the latest frame from the device to the output buffer
 */
void CSV::Export::registerFrame(const JSON::Frame &frame)
{
  // Ignore if device is not connected (we don't want to generate a CSV file
  // when we are reading another CSV file don't we?)
  if (!IO::Manager::instance().connected())
    return;

  // Ignore if frame is invalid
  if (!frame.isValid())
    return;

  // Ignore if CSV export is disabled
  if (!exportEnabled())
    return;

  // Register raw frame to list
  TimestampFrame tframe;
  tframe.data = frame;
  tframe.rxDateTime = QDateTime::currentDateTime();
  m_frames.append(tframe);
}
