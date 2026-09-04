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

#include <cstdint>
#include <QByteArray>
#include <QCanBusFrame>
#include <QString>
#include <QStringList>

#include "IO/Drivers/CANBus/CanBackends.h"
#include "IO/Drivers/CANBus/SerialCanBackendBase.h"

namespace IO {
namespace Drivers {

/**
 * @brief Backend for the Seeed/Waveshare USB-CAN Analyzer (CH340 serial). The port, the buffer
 *        and the unplug handling live in SerialCanBackendBase; what is here is the analyzer's
 *        variable-length packet protocol.
 */
class SeeedCanBackend : public SerialCanBackendBase {
  Q_OBJECT

public:
  explicit SeeedCanBackend(const QString& portName, QObject* parent = nullptr);

  SeeedCanBackend(SeeedCanBackend&&)                 = delete;
  SeeedCanBackend(const SeeedCanBackend&)            = delete;
  SeeedCanBackend& operator=(SeeedCanBackend&&)      = delete;
  SeeedCanBackend& operator=(const SeeedCanBackend&) = delete;

  ~SeeedCanBackend() override;

  /**
   * @brief Outcome of decoding one packet from the receive buffer.
   */
  enum class Parse {
    Frame,
    Resync,
    NeedMore,
  };

  [[nodiscard]] static CanBackends::Entry registration();
  [[nodiscard]] static const QString& pluginKey();
  [[nodiscard]] static QStringList availableInterfaces();
  [[nodiscard]] static QCanBusDevice* create(const QString& portName);
  [[nodiscard]] static std::uint8_t bitrateCode(quint32 bitrate);
  [[nodiscard]] static Parse decodePacket(const QByteArray& buffer,
                                          QCanBusFrame& out,
                                          int& consumed);

protected:
  [[nodiscard]] bool writeFrame(const QCanBusFrame& frame) override;
  [[nodiscard]] QString interpretErrorFrame(const QCanBusFrame& frame) override;

  [[nodiscard]] bool validateBitrate(quint32 bitrate, QString& reason) const override;
  [[nodiscard]] bool sendInit(quint32 bitrate) override;
  void drainBuffer(QByteArray& buffer, QList<QCanBusFrame>& frames) override;
};
}  // namespace Drivers
}  // namespace IO
