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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <memory>
#include <QObject>

#include "Core/IO/FrameConfig.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/IO/IIngestBinder.h"

namespace IO {

/**
 * @brief Non-singleton owner of one HAL driver, attached to the acquisition pipeline through the
 *        ingest binder (spec 0077): the reader that frames this driver's bytes is created,
 *        recreated and retired by the binder, never named here.
 */
class DeviceManager : public QObject {
  Q_OBJECT

signals:
  void rawDataSent(int deviceId, const QByteArray& data);
  void rawDataReceived(int deviceId, const IO::CapturedDataPtr& data);
  void consoleDataReceived(int deviceId, const IO::CapturedDataPtr& data);

public:
  explicit DeviceManager(int deviceId,
                         std::unique_ptr<HAL_Driver> driver,
                         const FrameConfig& config,
                         IIngestBinder& binder,
                         QObject* parent = nullptr);
  ~DeviceManager();

  [[nodiscard]] int deviceId() const noexcept;
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isWritable() const;
  [[nodiscard]] bool isAttached() const noexcept;
  [[nodiscard]] HAL_Driver* driver() const noexcept;

  [[nodiscard]] qint64 write(const QByteArray& data);

  void reconfigure(const FrameConfig& config);

public slots:
  bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);
  void close();

private slots:
  void onRawDataSent(const QByteArray& data);
  void onRawDataReceived(const IO::CapturedDataPtr& data);
  void onConsoleDataReceived(const IO::CapturedDataPtr& data);

private:
  void attachToPipeline(const FrameConfig& config);
  void detachFromPipeline();

private:
  int m_deviceId;
  bool m_attached;
  IIngestBinder& m_binder;
  FrameConfig m_frameConfig;
  std::unique_ptr<HAL_Driver> m_driver;
};

}  // namespace IO
