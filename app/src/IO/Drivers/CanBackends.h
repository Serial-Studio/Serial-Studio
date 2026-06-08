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

#include <QCanBusDevice>
#include <QList>
#include <QString>
#include <QStringList>

namespace IO {
namespace Drivers {

// Shared listen-only config key (no standard Qt key); CANBus and the gs_usb backend must agree
inline constexpr auto kListenOnlyConfigKey =
  static_cast<QCanBusDevice::ConfigurationKey>(QCanBusDevice::UserKey);

/**
 * @brief Registry of libusb/serial CAN backends surfaced as synthetic CANBus plugins.
 */
class CanBackends {
public:
  /**
   * @brief One synthetic CAN backend, identified by a plugin key.
   */
  struct Entry {
    QString key;
    QString displayName;
    bool supported;
    QStringList (*availableInterfaces)();
    QCanBusDevice* (*create)(const QString& interfaceName);
  };

  [[nodiscard]] static const QList<Entry>& all();
  [[nodiscard]] static const Entry* find(const QString& key);
};
}  // namespace Drivers
}  // namespace IO
