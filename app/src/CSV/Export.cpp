/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Export.h"

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopServices>

#include "IO/Manager.h"
#include "CSV/Player.h"
#include "Misc/Utilities.h"
#include "Misc/TimerEvents.h"
#include "JSON/FrameBuilder.h"
#include "Misc/WorkspaceManager.h"

#ifdef USE_QT_COMMERCIAL
#  include "MQTT/Client.h"
#endif

/**
 * Constructor function, configures the path in which Serial Studio shall
 * automatically write generated CSV files.
 */
CSV::Export::Export()
  : m_exportEnabled(true)
{
}

/**
 * Close file & finnish write-operations before destroying the class.
 */
CSV::Export::~Export()
{
  closeFile();
}

/**
 * Returns a pointer to the only instance of this class.
 */
CSV::Export &CSV::Export::instance()
{
  static Export singleton;
  return singleton;
}

/**
 * Returns @c true if the CSV output file is open.
 */
bool CSV::Export::isOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * Returns @c true if CSV export is enabled.
 */
bool CSV::Export::exportEnabled() const
{
  return m_exportEnabled;
}

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void CSV::Export::setupExternalConnections()
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this,
          &Export::closeFile);
  connect(&JSON::FrameBuilder::instance(), &JSON::FrameBuilder::frameChanged,
          this, &Export::registerFrame, Qt::QueuedConnection);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Export::writeValues);
  connect(&IO::Manager::instance(), &IO::Manager::pausedChanged, this, [=] {
    if (IO::Manager::instance().paused())
      closeFile();
  });
}

/**
 * Enables or disables data export.
 */
void CSV::Export::setExportEnabled(const bool enabled)
{
  m_exportEnabled = enabled;
  Q_EMIT enabledChanged();

  if (!exportEnabled() && isOpen())
  {
    m_frames.clear();
    m_frames.squeeze();
    closeFile();
  }
}

/**
 * Write all remaining JSON frames & close the CSV file.
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
 * @brief Writes frame data to the current CSV file.
 *
 * This function writes the data from all stored frames (`m_frames`) to the
 * currently open CSV file. It ensures that values in each row are written
 * in the same order as the headers, based on dataset indexes.
 *
 * If the file is not open, it creates the CSV file using the first frame and
 * sets up the headers before writing data. Missing dataset values are replaced
 * with empty strings.
 *
 * After writing, the stream is flushed to ensure the data is saved, and
 * the frame buffer (`m_frames`) is cleared.
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
      indexHeaderPairs.squeeze();
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
        fieldValues[d->index()] = d->value().simplified();
    }

    // Write data according to the sorted field order
    for (int j = 0; j < indexHeaderPairs.count(); ++j)
    {
      // Print value for current pair
      const auto fieldIndex = indexHeaderPairs[j].first;
      m_textStream << fieldValues.value(fieldIndex, QLatin1String(""));

      // Add comma or newline based on the position in the row
      if (j < indexHeaderPairs.count() - 1)
        m_textStream << QStringLiteral(",");
      else
        m_textStream << QStringLiteral("\n");
    }
  }

  // Flush the stream to writte it to the hard disk
  m_textStream.flush();

  // Clear frames
  m_frames.clear();
  m_frames.squeeze();
}

/**
 * @brief Creates and initializes a new CSV file for exporting frame data.
 *
 * This function generates a CSV file in a project-specific directory using the
 * frame's data and timestamps. The dataset headers are added to the CSV file,
 * sorted by their indexes, ensuring ordered column headers. If the file cannot
 * be created or opened, an error message is displayed, and the function returns
 * an empty vector.
 *
 * @param frame The frame containing data and timestamp information.
 * @return A vector of pairs, each containing a dataset index and its
 * corresponding header string, sorted by the dataset index.
 */
QVector<QPair<int, QString>>
CSV::Export::createCsvFile(const CSV::TimestampFrame &frame)
{
  // Obtain frame data
  const auto &data = frame.data;
  const auto &rxTime = frame.rxDateTime;

  // Get file name
  const auto fileName = rxTime.toString(QStringLiteral("yyyy_MMM_dd HH_mm_ss"))
                        + QStringLiteral(".csv");

  // Get path
  const QString path = QStringLiteral("%1/%2/").arg(
      Misc::WorkspaceManager::instance().path("CSV"), data.title());

  // Generate file path if required
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(QStringLiteral("."));

  // Open file
  m_csvFile.setFileName(dir.filePath(fileName));
  if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    Misc::Utilities::showMessageBox(tr("CSV File Error"),
                                    tr("Cannot open CSV file for writing!"),
                                    QMessageBox::Critical);
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
        auto header = QString("%1/%2").arg(g->title(), d->title()).simplified();
        datasetIndexes.append(d->index());
        headers.append(header);
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
 * Appends the latest frame from the device to the output buffer.
 */
void CSV::Export::registerFrame(const JSON::Frame &frame)
{
  // Ignore if CSV export is disabled
  if (!exportEnabled())
    return;

  // Don't generate a CSV file when we are playing a CSV file
  if (CSV::Player::instance().isOpen())
    return;

  // Ignore if frame is invalid
  if (!frame.isValid())
    return;

  // Don't save CSV data when the device/service is not connected
#ifdef USE_QT_COMMERCIAL
  if (!IO::Manager::instance().isConnected()
      && !(MQTT::Client::instance().isConnected()
           && MQTT::Client::instance().isSubscriber()))
    return;
#else
  if (!IO::Manager::instance().isConnected())
    return;
#endif

  // Register raw frame to list
  TimestampFrame tframe;
  tframe.data = frame;
  tframe.rxDateTime = QDateTime::currentDateTime();
  m_frames.append(tframe);
}
