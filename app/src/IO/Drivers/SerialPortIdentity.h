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

#include <QJsonObject>
#include <QSerialPortInfo>
#include <QVector>

namespace IO::Drivers::SerialPorts {

/**
 * @brief Returns the serial ports the pickers show, with the platform filter applied.
 */
[[nodiscard]] QVector<QSerialPortInfo> visiblePorts();

/**
 * @brief Returns the persisted hardware identifier of one port (VID, PID, serial, name, label).
 */
[[nodiscard]] QJsonObject identity(const QSerialPortInfo& info);

/**
 * @brief Scores how strongly a port identity matches a previously saved one.
 */
[[nodiscard]] int scoreIdentityMatch(const QJsonObject& candidate, const QJsonObject& saved);

}  // namespace IO::Drivers::SerialPorts
