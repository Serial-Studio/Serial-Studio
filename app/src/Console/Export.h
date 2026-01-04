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
#include <QObject>
#include <QSettings>
#include <QTextStream>

#include "DataModel/FrameConsumer.h"

namespace Console
{

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

/**
 * @brief Shared pointer to ExportData for efficient queuing
 */
typedef std::shared_ptr<ExportData> ExportDataPtr;

class Export;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Worker that handles console export file I/O on background thread
 */
class ExportWorker : public DataModel::FrameConsumerWorker<ExportDataPtr>
{
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<ExportDataPtr>::FrameConsumerWorker;
  ~ExportWorker() override;

  void closeResources() override;
  bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<ExportDataPtr> &items) override;

private:
  void createFile();

private:
  QFile m_file;
  QTextStream m_textStream;
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
class Export
#ifdef BUILD_COMMERCIAL
  : public DataModel::FrameConsumer<ExportDataPtr>
#else
  : public QObject
#endif
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

protected:
#ifdef BUILD_COMMERCIAL
  DataModel::FrameConsumerWorkerBase *createWorker() override;
#endif

private slots:
#ifdef BUILD_COMMERCIAL
  void onWorkerOpenChanged();
#endif

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;
};
} // namespace Console
