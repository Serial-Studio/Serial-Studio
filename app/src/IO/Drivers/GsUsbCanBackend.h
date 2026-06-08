/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <QByteArray>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>

#include "IO/Drivers/CanBackends.h"

struct libusb_context;
struct libusb_device_handle;

namespace IO {
namespace Drivers {

/**
 * @brief QCanBusDevice backend for gs_usb / candleLight USB-CAN adapters via libusb.
 */
class GsUsbCanBackend : public QCanBusDevice {
  Q_OBJECT

public:
  explicit GsUsbCanBackend(const QString& interfaceName, QObject* parent = nullptr);
  ~GsUsbCanBackend() override;

  GsUsbCanBackend(GsUsbCanBackend&&)                 = delete;
  GsUsbCanBackend(const GsUsbCanBackend&)            = delete;
  GsUsbCanBackend& operator=(GsUsbCanBackend&&)      = delete;
  GsUsbCanBackend& operator=(const GsUsbCanBackend&) = delete;

  [[nodiscard]] static CanBackends::Entry registration();
  [[nodiscard]] static const QString& pluginKey();
  [[nodiscard]] static bool supported();
  [[nodiscard]] static QStringList availableInterfaces();
  [[nodiscard]] static QCanBusDevice* create(const QString& interfaceName);

protected:
  [[nodiscard]] bool open() override;
  void close() override;
  [[nodiscard]] bool writeFrame(const QCanBusFrame& frame) override;
  [[nodiscard]] QString interpretErrorFrame(const QCanBusFrame& frame) override;

private slots:
  void checkTxTimeouts();
  void handleReadError(const QString& reason);
  void completeTransmits(const QList<quint32>& echoIds);

private:
  void readLoop();
  void stopReadThread();
  [[nodiscard]] bool configureDevice(quint32 bitrate);
  [[nodiscard]] bool claimGsUsbInterface();
  [[nodiscard]] int controlOut(std::uint8_t request,
                               std::uint16_t value,
                               void* data,
                               std::uint16_t length);
  [[nodiscard]] int controlIn(std::uint8_t request,
                              std::uint16_t value,
                              void* data,
                              std::uint16_t length);

private:
  libusb_context* m_ctx;
  libusb_device_handle* m_handle;

  QString m_interfaceName;
  int m_interfaceNumber;
  std::uint8_t m_inEndpoint;
  std::uint8_t m_outEndpoint;
  std::uint32_t m_echoCounter;

  QByteArray m_rxCarry;

  QTimer m_txTimeoutTimer;
  QElapsedTimer m_txClock;
  QHash<quint32, qint64> m_pendingTx;

  static constexpr std::size_t kCacheLine = 64;
  alignas(kCacheLine) std::atomic<bool> m_running;

  QThread m_readThread;
};
}  // namespace Drivers
}  // namespace IO
