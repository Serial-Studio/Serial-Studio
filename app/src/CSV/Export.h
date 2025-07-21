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

#pragma once

#include <QFile>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QVector>
#include <QObject>
#include <QTextStream>

#include "JSON/Frame.h"
#include "ThirdParty/readerwriterqueue.h"

namespace CSV
{
/**
 * @brief Represents a single timestamped frame for CSV export.
 *
 * Stores a JSON frame and the associated reception timestamp (in local time).
 * Designed to be move-only for performance reasons, especially when using
 * concurrent queues.
 */
struct TimestampFrame
{
  JSON::Frame data;     ///< The actual data frame.
  QDateTime rxDateTime; ///< Time at which the frame was received.

  /**
   * @brief Default constructor.
   */
  TimestampFrame() {}

  /**
   * @brief Constructs a timestamped frame with current time.
   *
   * @param d The data frame (moved).
   */
  TimestampFrame(JSON::Frame &&d)
    : data(std::move(d))
    , rxDateTime(QDateTime::currentDateTime())
  {
  }

  // Disable copy constructs and use standard move assignments
  TimestampFrame(TimestampFrame &&) = default;
  TimestampFrame(const TimestampFrame &) = delete;
  TimestampFrame &operator=(TimestampFrame &&) = default;
  TimestampFrame &operator=(const TimestampFrame &) = delete;
};

/**
 * @brief Handles CSV export of incoming data frames.
 *
 * The Export class collects incoming JSON frames tagged with a timestamp,
 * writes them asynchronously to a CSV file, and manages output buffering
 * and formatting.
 *
 * This class is implemented as a singleton and runs a background thread
 * to offload file I/O operations. It supports enabling/disabling export
 * dynamically and integrates with external modules (IO manager, MQTT, etc.).
 */
class Export : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isOpen
             READ isOpen
             NOTIFY openChanged)
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  // clang-format on

signals:
  void openChanged();
  void enabledChanged();

private:
  explicit Export();
  Export(Export &&) = delete;
  Export(const Export &) = delete;
  Export &operator=(Export &&) = delete;
  Export &operator=(const Export &) = delete;

  ~Export();

public:
  static Export &instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);

  void hotpathTxFrame(const JSON::Frame &frame);

private slots:
  void writeValues();

private:
  QVector<QPair<int, QString>> createCsvFile(const JSON::Frame &frame);

private:
  QFile m_csvFile;
  QMutex m_queueLock;
  bool m_exportEnabled;
  QTimer *m_workerTimer;
  QThread m_workerThread;
  QTextStream m_textStream;
  std::vector<TimestampFrame> m_writeBuffer;
  QVector<QPair<int, QString>> m_indexHeaderPairs;
  moodycamel::ReaderWriterQueue<TimestampFrame> m_pendingFrames{8128};
};
} // namespace CSV
