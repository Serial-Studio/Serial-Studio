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

#include <QObject>
#include <QString>
#include <QTimer>

namespace Core {
namespace Bus {
class MessageBus;
}  // namespace Bus
}  // namespace Core

namespace IO {
namespace Drivers {
/**
 * @brief The slow re-dial a Modbus TCP source gets after a refused dial or a dropped link: one
 *        single-shot timer the driver arms and close() cancels, so a manual disconnect is final
 *        and a healthy link is never bounced. It only asks: due() and dropped() are forwarded to
 *        the driver's HAL_Driver::reopenRequested and linkDropped, which the manager honors.
 */
class ModbusRedial : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void due();
  void dropped();

public:
  explicit ModbusRedial(QObject* parent = nullptr);

  void arm();
  void cancel();
  void reportDrop(Core::Bus::MessageBus* bus,
                  const QString& target,
                  const QString& error,
                  bool redial);
  void reportRecovered(Core::Bus::MessageBus* bus);

  [[nodiscard]] bool armed() const;
  [[nodiscard]] static int intervalMs() noexcept;

private:
  QString m_raisedTitle;
  QTimer m_timer;
};
}  // namespace Drivers
}  // namespace IO
