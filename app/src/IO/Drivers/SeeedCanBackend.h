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

#include <QByteArray>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QString>
#include <QStringList>

#include "IO/Drivers/CanBackends.h"

class QSerialPort;

namespace IO {
namespace Drivers {

/**
 * @brief QCanBusDevice backend for the Seeed/Waveshare USB-CAN Analyzer (CH340 serial).
 */
class SeeedCanBackend : public QCanBusDevice {
  Q_OBJECT

public:
  explicit SeeedCanBackend(const QString& portName, QObject* parent = nullptr);
  ~SeeedCanBackend() override;

  SeeedCanBackend(SeeedCanBackend&&)                 = delete;
  SeeedCanBackend(const SeeedCanBackend&)            = delete;
  SeeedCanBackend& operator=(SeeedCanBackend&&)      = delete;
  SeeedCanBackend& operator=(const SeeedCanBackend&) = delete;

  [[nodiscard]] static CanBackends::Entry registration();
  [[nodiscard]] static const QString& pluginKey();
  [[nodiscard]] static QStringList availableInterfaces();
  [[nodiscard]] static QCanBusDevice* create(const QString& portName);

protected:
  [[nodiscard]] bool open() override;
  void close() override;
  [[nodiscard]] bool writeFrame(const QCanBusFrame& frame) override;
  [[nodiscard]] QString interpretErrorFrame(const QCanBusFrame& frame) override;

private slots:
  void onReadyRead();

private:
  [[nodiscard]] bool sendInitFrame(quint32 bitrate);

private:
  QSerialPort* m_port;
  QString m_portName;
  QByteArray m_rxBuffer;
};
}  // namespace Drivers
}  // namespace IO
