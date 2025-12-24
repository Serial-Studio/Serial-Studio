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

#include <atomic>

#include <QFile>
#include <QObject>
#include <QThread>
#include <QSettings>
#include <QTextStream>

#include "ThirdParty/readerwriterqueue.h"

namespace Console
{
static constexpr size_t kExportQueueCapacity = 8192;

/**
 * @brief Represents a single console data item for export
 */
struct ExportData
{
  QString data;

  ExportData() = default;

  ExportData(QString &&d)
    : data(std::move(d))
  {
  }

  ExportData(ExportData &&) = default;
  ExportData(const ExportData &) = delete;
  ExportData &operator=(ExportData &&) = default;
  ExportData &operator=(const ExportData &) = delete;
};

class Export;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Worker that handles console export file I/O on background thread
 */
class ExportWorker : public QObject
{
  Q_OBJECT

public:
  explicit ExportWorker(moodycamel::ReaderWriterQueue<ExportData> *queue,
                        std::atomic<bool> *exportEnabled,
                        std::atomic<size_t> *queueSize);
  ~ExportWorker();

  [[nodiscard]] bool isOpen() const;

signals:
  void openChanged();

public slots:
  void writeValues();
  void closeFile();
  void createFile();

private:
  QFile m_file;
  QTextStream m_textStream;
  std::vector<ExportData> m_writeBuffer;
  moodycamel::ReaderWriterQueue<ExportData> *m_pendingData;
  std::atomic<bool> *m_exportEnabled;
  std::atomic<size_t> *m_queueSize;
};
#endif

/**
 * @class Export
 * @brief Manages automatic export of console data to log files.
 *
 * The Export class is a singleton that provides functionality to capture
 * and export console output data to persistent log files. This is particularly
 * useful for debugging, data logging, and post-analysis of serial communication
 * sessions.
 *
 * Key Features:
 * - **Automatic File Creation**: Automatically creates dated log files in the
 *   workspace directory
 * - **Buffered Writing**: Buffers console data and writes periodically to
 *   reduce disk I/O
 * - **Pro Feature**: Available only in commercial builds with valid license
 * - **Singleton Pattern**: Single instance ensures consistent file handling
 *   across the application
 *
 * @note This feature is only available in commercial builds (BUILD_COMMERCIAL).
 *       In GPL builds, all methods return false/empty values and no export
 *       occurs.
 *
 * @warning Export functionality requires an active Serial Studio Pro license.
 *          The export will be automatically disabled if the license becomes
 *          invalid.
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

  void registerData(QStringView data);

private slots:
#ifdef BUILD_COMMERCIAL
  void onWorkerOpenChanged();
#endif

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;

#ifdef BUILD_COMMERCIAL
  QThread m_workerThread;
  ExportWorker *m_worker;
  std::atomic<size_t> m_queueSize;
  moodycamel::ReaderWriterQueue<ExportData> m_pendingData{kExportQueueCapacity};
#endif
};
} // namespace Console
