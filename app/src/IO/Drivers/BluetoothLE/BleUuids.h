/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <QBluetoothUuid>
#include <QLowEnergyCharacteristic>
#include <QString>

namespace IO::Drivers::BleDetail {

/**
 * @brief Returns the friendly name of a BLE service UUID, or its own string when unknown.
 */
[[nodiscard]] QString bleServiceName(const QBluetoothUuid& uuid);

/**
 * @brief Parses a UUID in any of the forms a project or a device may spell it in.
 */
[[nodiscard]] QBluetoothUuid bleUuidFromString(const QString& uuid);

/**
 * @brief Returns the friendly name of a characteristic, falling back to its UUID.
 */
[[nodiscard]] QString bleCharacteristicName(const QLowEnergyCharacteristic& characteristic);

}  // namespace IO::Drivers::BleDetail
