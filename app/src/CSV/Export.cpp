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
#include <QStandardPaths>

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
    m_textStream.setDevice(nullptr);

    Q_EMIT openChanged();
  }
}

/**
 * Creates a CSV file based on the JSON frames contained in the JSON list.
 * @note This function is called periodically every 1 second.
 */
void CSV::Export::writeValues()
{
  // Initialize a list of pairs between indexes & headers
  static QVector<QPair<int, QString>> indexHeaderPairs;

  // Write each frame
  for (auto i = m_frames.begin(); i != m_frames.end(); ++i)
  {
    // File not open, create it & add cell titles
    if (!isOpen() && exportEnabled())
    {
      indexHeaderPairs.clear();
      indexHeaderPairs = createCsvFile(*i);
    }

    // Continue if index/header pairs is not empty
    if (indexHeaderPairs.isEmpty())
      return;

    // Obtain frame data
    const auto &data = i->data;
    const auto &rxTime = i->rxDateTime;

    // Write RX date/time
    const auto format = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");
    m_textStream << rxTime.toString(format) << QStringLiteral(",");

    // Write frame data in the order of sorted fields
    const auto &groups = data.groups();
    QMap<int, QString> fieldValues;

    // Iterate through groups and datasets to collect field values
    for (auto g = groups.constBegin(); g != groups.constEnd(); ++g)
    {
      const auto &datasets = g->datasets();
      for (auto d = datasets.constBegin(); d != datasets.constEnd(); ++d)
        fieldValues[d->index()] = d->value();
    }

    // Write data according to the sorted field order
    for (int i = 0; i < indexHeaderPairs.count(); ++i)
    {
      // Print value for current pair
      const auto fieldIndex = indexHeaderPairs[i].first;
      m_textStream << fieldValues.value(fieldIndex, QStringLiteral(""));

      // Add comma or newline based on the position in the row
      if (i < indexHeaderPairs.count() - 1)
        m_textStream << QStringLiteral(",");
      else
        m_textStream << QStringLiteral("\n");
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
QVector<QPair<int, QString>>
CSV::Export::createCsvFile(const CSV::TimestampFrame &frame)
{
  // Obtain frame data
  const auto &data = frame.data;
  const auto &rxTime = frame.rxDateTime;

  // Get file name
  const auto fileName
      = rxTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss")) + ".csv";

  // Get path
  const QString path = QStringLiteral("%1/%2/CSV/%3/")
                           .arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                qApp->applicationDisplayName(), data.title());

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
    return QVector<QPair<int, QString>>();
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
  QVector<QString> headers;
  QVector<int> datasetIndexes;
  const auto &groups = data.groups();
  for (auto g = groups.constBegin(); g != groups.constEnd(); ++g)
  {
    const auto &datasets = g->datasets();
    for (auto d = datasets.constBegin(); d != datasets.constEnd(); ++d)
    {
      if (!datasetIndexes.contains(d->index()))
      {
        datasetIndexes.append(d->index());
        headers.append(QString("%1/%2").arg(g->title(), d->title()));
      }
    }
  }

  // Combine fields and headers into pairs for sorting
  QVector<QPair<int, QString>> fieldHeaderPairs;
  for (int i = 0; i < datasetIndexes.count(); ++i)
    fieldHeaderPairs.append(qMakePair(datasetIndexes[i], headers[i]));

  // Sort the pairs based on the field values (first element of the pair)
  std::sort(fieldHeaderPairs.begin(), fieldHeaderPairs.end(),
            [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
              return a.first < b.first;
            });

  // Add CSV header directly from sorted pairs
  m_textStream << QStringLiteral("RX Date/Time,");
  for (int i = 0; i < fieldHeaderPairs.count(); ++i)
  {
    m_textStream << fieldHeaderPairs[i].second;
    if (i < fieldHeaderPairs.count() - 1)
      m_textStream << QStringLiteral(",");
    else
      m_textStream << QStringLiteral("\n");
  }

  // Update UI
  Q_EMIT openChanged();
  return fieldHeaderPairs;
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
