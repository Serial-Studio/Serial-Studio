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

#include <QJsonObject>
#include <QString>
#include <QVector>

#include "IO/Drivers/Modbus/ModbusRegisterGroups.h"

namespace IO {
namespace Drivers {
/**
 * @brief Builds a Serial Studio project from a set of Modbus register groups. A pure translation
 *        from configuration to project JSON: it reads the groups it was constructed with and
 *        nothing else, so the connection settings are handed in by the driver. Holding no driver
 *        reference is what makes the generated parser and group layout assertable from a test.
 */
class ModbusProjectGenerator {
public:
  explicit ModbusProjectGenerator(const QVector<ModbusRegisterGroup>& groups);

  [[nodiscard]] int totalDatasets() const;
  [[nodiscard]] QString buildFrameParser() const;
  [[nodiscard]] QJsonObject buildProject(const QJsonObject& connectionSettings) const;

private:
  QVector<ModbusRegisterGroup> m_groups;
};
}  // namespace Drivers
}  // namespace IO
