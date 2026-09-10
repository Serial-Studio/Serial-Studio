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

#include <QJsonArray>
#include <QSettings>
#include <QVector>

namespace IO {
namespace Drivers {
/**
 * @brief A contiguous block of Modbus registers to poll, optionally from a slave of its own so one
 *        connection can read several devices sharing a bus. Zero means "use the driver's address":
 *        unit 0 is the Modbus broadcast address and never a legal read target, so it is a free
 *        sentinel that leaves groups stored before this field existed behaving as they did.
 */
struct ModbusRegisterGroup {
  quint8 registerType;
  quint16 startAddress;
  quint16 count;
  quint8 slaveAddress;

  ModbusRegisterGroup() : registerType(0), startAddress(0), count(0), slaveAddress(0) {}

  ModbusRegisterGroup(quint8 type, quint16 start, quint16 cnt, quint8 slave = 0)
    : registerType(type), startAddress(start), count(cnt), slaveAddress(slave)
  {}
};

/**
 * @brief The register groups of one Modbus driver, with their persistence: the group list, the
 *        count validation (1..125 registers per read) and the de-duplication that keeps a project
 *        from polling the same block twice. The QSettings object is injected rather than created
 *        here, so a unit test points the same constructor at a temporary file.
 */
class ModbusRegisterGroups {
public:
  explicit ModbusRegisterGroups(QSettings& settings);

  ModbusRegisterGroups(ModbusRegisterGroups&&)                 = delete;
  ModbusRegisterGroups(const ModbusRegisterGroups&)            = delete;
  ModbusRegisterGroups& operator=(ModbusRegisterGroups&&)      = delete;
  ModbusRegisterGroups& operator=(const ModbusRegisterGroups&) = delete;

  void clear();
  void restore();

  [[nodiscard]] int count() const;
  [[nodiscard]] bool isEmpty() const;
  [[nodiscard]] int totalDatasets() const;
  [[nodiscard]] bool remove(const int index);
  [[nodiscard]] QJsonArray toJson() const;
  [[nodiscard]] const ModbusRegisterGroup& at(const int index) const;
  [[nodiscard]] const QVector<ModbusRegisterGroup>& groups() const;
  [[nodiscard]] bool add(const quint8 type,
                         const quint16 start,
                         const quint16 count,
                         const quint8 slave = 0);

private:
  void persist();

  QSettings& m_settings;
  QVector<ModbusRegisterGroup> m_groups;
};
}  // namespace Drivers
}  // namespace IO
