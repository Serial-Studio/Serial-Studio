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

#include "IO/Drivers/Modbus/ModbusRegisterGroups.h"

#include <QJsonObject>
#include <utility>

#include "Core/SSAssert.h"

static constexpr quint16 kMaxRegisterCount = 125;
static constexpr quint16 kMaxBitCount      = 2000;

static const auto kSettingsArray = QStringLiteral("ModbusDriver/registerGroups");

static const IO::Drivers::ModbusRegisterGroup kInvalidGroup;

/**
 * @brief Largest count one read of @p type can carry. FC03/FC04 answer at most 125 sixteen-bit
 *        registers, FC01/FC02 at most 2000 bits: both ceilings are what a single-octet byte count
 *        leaves room for, and sharing the register ceiling refused four fifths of a legal coil
 * read.
 */
[[nodiscard]] static quint16 maxCountForType(const quint8 type) noexcept
{
  return (type == 2 || type == 3) ? kMaxBitCount : kMaxRegisterCount;
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the group list to the settings scope it persists into.
 */
IO::Drivers::ModbusRegisterGroups::ModbusRegisterGroups(QSettings& settings) : m_settings(settings)
{}

//--------------------------------------------------------------------------------------------------
// Persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reloads the groups from the settings array, dropping any entry whose count is outside the
 *        range a single Modbus read of that register type can carry.
 */
void IO::Drivers::ModbusRegisterGroups::restore()
{
  m_groups.clear();

  const int stored = m_settings.beginReadArray(kSettingsArray);
  for (int i = 0; i < stored; ++i) {
    m_settings.setArrayIndex(i);

    ModbusRegisterGroup group;
    group.registerType = static_cast<quint8>(m_settings.value("type", 0).toUInt());
    group.startAddress = static_cast<quint16>(m_settings.value("start", 0).toUInt());
    group.count        = static_cast<quint16>(m_settings.value("count", 0).toUInt());

    if (group.count > 0 && group.count <= maxCountForType(group.registerType))
      m_groups.append(group);
  }

  m_settings.endArray();
}

/**
 * @brief Writes the current groups back to the settings array. The array size endArray() stamps
 *        is what a later restore() reads, so a shrunk list never resurrects a removed group.
 */
void IO::Drivers::ModbusRegisterGroups::persist()
{
  m_settings.beginWriteArray(kSettingsArray);
  for (int i = 0; i < m_groups.size(); ++i) {
    m_settings.setArrayIndex(i);
    m_settings.setValue("type", m_groups[i].registerType);
    m_settings.setValue("start", m_groups[i].startAddress);
    m_settings.setValue("count", m_groups[i].count);
  }

  m_settings.endArray();
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a group, returning whether the list actually changed: an out-of-range count and
 *        an exact duplicate are both refused, so the caller only announces a real edit. The count
 *        bound is validation, not an invariant: project files and the JSON-RPC API both reach
 *        here with whatever the caller wrote.
 */
bool IO::Drivers::ModbusRegisterGroups::add(const quint8 type,
                                            const quint16 start,
                                            const quint16 count)
{
  if (count == 0 || count > maxCountForType(type))
    return false;

  for (const auto& group : std::as_const(m_groups))
    if (group.registerType == type && group.startAddress == start && group.count == count)
      return false;

  m_groups.append(ModbusRegisterGroup(type, start, count));
  persist();

  return true;
}

/**
 * @brief Removes the group at @p index, returning whether the index named one.
 */
bool IO::Drivers::ModbusRegisterGroups::remove(const int index)
{
  if (index < 0 || index >= m_groups.count())
    return false;

  m_groups.removeAt(index);
  persist();

  return true;
}

/**
 * @brief Drops every group and truncates the persisted array.
 */
void IO::Drivers::ModbusRegisterGroups::clear()
{
  m_groups.clear();
  persist();
}

//--------------------------------------------------------------------------------------------------
// Accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of configured groups.
 */
int IO::Drivers::ModbusRegisterGroups::count() const
{
  return m_groups.count();
}

/**
 * @brief Returns true when no group is configured.
 */
bool IO::Drivers::ModbusRegisterGroups::isEmpty() const
{
  return m_groups.isEmpty();
}

/**
 * @brief Returns how many datasets the groups describe in total, one per register or bit.
 */
int IO::Drivers::ModbusRegisterGroups::totalDatasets() const
{
  int total = 0;
  for (const auto& group : std::as_const(m_groups))
    total += group.count;

  return total;
}

/**
 * @brief Returns the group at @p index; an out-of-range index yields an empty group.
 */
const IO::Drivers::ModbusRegisterGroup& IO::Drivers::ModbusRegisterGroups::at(const int index) const
{
  SS_ASSERT(index >= 0 && index < m_groups.count(), return kInvalidGroup);
  return m_groups.at(index);
}

/**
 * @brief Returns the groups in poll order.
 */
const QVector<IO::Drivers::ModbusRegisterGroup>& IO::Drivers::ModbusRegisterGroups::groups() const
{
  return m_groups;
}

/**
 * @brief Returns the groups as the JSON array the driver property model publishes.
 */
QJsonArray IO::Drivers::ModbusRegisterGroups::toJson() const
{
  QJsonArray array;
  for (const auto& group : std::as_const(m_groups)) {
    QJsonObject object;
    object[QStringLiteral("type")]  = group.registerType;
    object[QStringLiteral("start")] = group.startAddress;
    object[QStringLiteral("count")] = group.count;
    array.append(object);
  }

  return array;
}
