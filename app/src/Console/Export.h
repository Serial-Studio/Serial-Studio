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

#include <memory>
#include <QFile>
#include <QObject>
#include <QSettings>
#include <QTextStream>
#include <unordered_map>

#include "DataModel/FrameConsumer.h"

namespace Console {

/**
 * @brief Per-device console payload queued for asynchronous export.
 */
struct ExportData {
  int deviceId = -1;
  QString data;

  ExportData() = default;

  ExportData(int id, QString&& d) : deviceId(id), data(std::move(d)) {}

  ExportData(ExportData&&)                 = default;
  ExportData(const ExportData&)            = delete;
  ExportData& operator=(ExportData&&)      = default;
  ExportData& operator=(const ExportData&) = delete;
};

/**
 * @brief Shared-ownership pointer used to ferry ExportData onto worker threads.
 */
typedef std::shared_ptr<ExportData> ExportDataPtr;

class Export;

#ifdef BUILD_COMMERCIAL
/**
 * @brief Worker that handles console export file I/O on a background thread.
 */
class ExportWorker : public DataModel::FrameConsumerWorker<ExportDataPtr> {
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<ExportDataPtr>::FrameConsumerWorker;
  ~ExportWorker() override;

  void closeResources() override;
  bool isResourceOpen() const override;

protected:
  void processItems(const std::vector<ExportDataPtr>& items) override;

private:
  struct DeviceExportState {
    std::unique_ptr<QFile> file;
    std::unique_ptr<QTextStream> stream;
  };

  void createFile(int deviceId);

private:
  std::unordered_map<int, DeviceExportState> m_deviceFiles;
};
#endif

/**
 * @brief Manages automatic export of console data to log files.
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
  Export(Export&&)                 = delete;
  Export(const Export&)            = delete;
  Export& operator=(Export&&)      = delete;
  Export& operator=(const Export&) = delete;

  ~Export();

public:
  [[nodiscard]] static Export& instance();

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool exportEnabled() const;

public slots:
  void closeFile();
  void setupExternalConnections();
  void setExportEnabled(const bool enabled);
  void setSettingsPersistent(const bool persistent);

  void registerData(int deviceId, QStringView data);

protected:
#ifdef BUILD_COMMERCIAL
  DataModel::FrameConsumerWorkerBase* createWorker() override;
#endif

private slots:
#ifdef BUILD_COMMERCIAL
  void onWorkerOpenChanged();
#endif

private:
  QSettings m_settings;
  std::atomic<bool> m_isOpen;
  std::atomic<bool> m_exportEnabled;
  bool m_persistSettings;
};
}  // namespace Console
