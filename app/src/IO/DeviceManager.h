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
#include <QObject>
#include <QPointer>
#include <QThread>

#include "IO/FrameConfig.h"
#include "IO/FrameReader.h"
#include "IO/HAL_Driver.h"

namespace IO {

/**
 * @brief Non-singleton owner of one HAL driver, one FrameReader, and one
 *        worker QThread.
 *
 * DeviceManager encapsulates the full lifecycle of a single device connection:
 * it owns the driver, configures and runs the FrameReader in an optional
 * worker thread, and emits frameReady() / rawDataReceived() for consumers.
 *
 * Drivers must NEVER be singletons for connection purposes — each DeviceManager
 * holds an independent driver instance. Driver singletons (e.g. UART::instance())
 * exist only for device enumeration (port lists, discovery state, etc.).
 *
 * ConnectionManager owns all DeviceManager instances and wires their outputs to
 * FrameBuilder and Console.
 */
class DeviceManager : public QObject {
  Q_OBJECT

signals:
  void frameReady(int deviceId, const QByteArray& frame);
  void rawDataReceived(int deviceId, const IO::ByteArrayPtr& data);

public:
  explicit DeviceManager(int deviceId,
                         std::unique_ptr<HAL_Driver> driver,
                         const FrameConfig& config,
                         bool threadedExtraction = false,
                         QObject* parent         = nullptr);
  ~DeviceManager();

  [[nodiscard]] int deviceId() const noexcept;
  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] bool isWritable() const;
  [[nodiscard]] HAL_Driver* driver() const noexcept;

  [[nodiscard]] qint64 write(const QByteArray& data);

  void reconfigure(const FrameConfig& config);

public slots:
  void open(QIODevice::OpenMode mode = QIODevice::ReadWrite);
  void close();

private slots:
  void onReadyRead();
  void onRawDataReceived(const IO::ByteArrayPtr& data);

private:
  void startFrameReader(const FrameConfig& config);
  void killFrameReader();

private:
  int m_deviceId;
  bool m_threadedExtraction;
  FrameConfig m_frameConfig;
  std::unique_ptr<HAL_Driver> m_driver;
  QThread m_workerThread;
  QPointer<FrameReader> m_frameReader;
  QByteArray m_frameScratch;
};

}  // namespace IO
