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
#include <QCanBusFrame>
#include <QString>
#include <QStringList>

#include "IO/Drivers/CANBus/CanBackends.h"
#include "IO/Drivers/CANBus/SerialCanBackendBase.h"

namespace IO {
namespace Drivers {

/**
 * @brief QCanBusDevice backend for slcan / LAWICEL ASCII adapters over a serial port. The port,
 *        the buffer and the unplug handling live in SerialCanBackendBase; what is here is the
 *        LAWICEL protocol.
 */
class SlcanBackend : public SerialCanBackendBase {
  Q_OBJECT

public:
  explicit SlcanBackend(const QString& portName, QObject* parent = nullptr);

  SlcanBackend(SlcanBackend&&)                 = delete;
  SlcanBackend(const SlcanBackend&)            = delete;
  SlcanBackend& operator=(SlcanBackend&&)      = delete;
  SlcanBackend& operator=(const SlcanBackend&) = delete;

  ~SlcanBackend() override;

  [[nodiscard]] static CanBackends::Entry registration();
  [[nodiscard]] static const QString& pluginKey();
  [[nodiscard]] static QStringList availableInterfaces();
  [[nodiscard]] static QCanBusDevice* create(const QString& portName);
  [[nodiscard]] static int bitrateIndex(quint32 bitrate);
  [[nodiscard]] static bool parseToken(const QByteArray& token, QCanBusFrame& out);

protected:
  [[nodiscard]] bool writeFrame(const QCanBusFrame& frame) override;
  [[nodiscard]] QString interpretErrorFrame(const QCanBusFrame& frame) override;

  [[nodiscard]] bool validateBitrate(quint32 bitrate, QString& reason) const override;
  [[nodiscard]] bool sendInit(quint32 bitrate) override;
  [[nodiscard]] bool openReplyIsError(const QByteArray& reply) const override;
  void drainBuffer(QByteArray& buffer, QList<QCanBusFrame>& frames) override;
  void sendShutdown() override;
};
}  // namespace Drivers
}  // namespace IO
