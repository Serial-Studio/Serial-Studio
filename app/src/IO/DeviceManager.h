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
#include <QPointer>

#include "IO/FrameConfig.h"
#include "IO/FrameReader.h"
#include "IO/HAL_Driver.h"

namespace IO {

class PipelineHost;

/**
 * @brief Non-singleton owner of one HAL driver and one FrameReader.
 */
class DeviceManager : public QObject {
  Q_OBJECT

signals:
  void rawDataReceived(int deviceId, const IO::CapturedDataPtr& data);
  void consoleDataReceived(int deviceId, const IO::CapturedDataPtr& data);

public:
  explicit DeviceManager(int deviceId,
                         std::unique_ptr<HAL_Driver> driver,
                         const FrameConfig& config,
                         QObject* parent = nullptr);
  ~DeviceManager();

  [[nodiscard]] int deviceId() const noexcept;
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isWritable() const;
  [[nodiscard]] HAL_Driver* driver() const noexcept;

  [[nodiscard]] inline FrameReader* frameReader() const noexcept { return m_frameReader.data(); }

  [[nodiscard]] qint64 write(const QByteArray& data);

  void reconfigure(const FrameConfig& config);

public slots:
  bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);
  void close();

private slots:
  void onRawDataReceived(const IO::CapturedDataPtr& data);
  void onConsoleDataReceived(const IO::CapturedDataPtr& data);

private:
  void startFrameReader(const FrameConfig& config);
  void killFrameReader();

private:
  int m_deviceId;
  PipelineHost& m_pipeline;
  FrameConfig m_frameConfig;
  std::unique_ptr<HAL_Driver> m_driver;
  QPointer<FrameReader> m_frameReader;
};

}  // namespace IO
