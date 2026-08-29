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

#include "IO/Drivers/Modbus/ModbusProjectGenerator.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <utility>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Shared helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Translates a generated-project string in the Modbus driver's context, which is where the
 *        catalogs already carry these entries.
 */
[[nodiscard]] static QString trModbus(const char* text)
{
  return QCoreApplication::translate("IO::Drivers::Modbus", text);
}

/**
 * @brief Returns the display name of a register type, or "Unknown" for a type this build does not
 *        know about.
 */
[[nodiscard]] static QString registerTypeName(const quint8 type)
{
  const QStringList names = {
    trModbus("Holding Registers"),
    trModbus("Input Registers"),
    trModbus("Coils"),
    trModbus("Discrete Inputs"),
  };

  return (type < names.count()) ? names[type] : trModbus("Unknown");
}

/**
 * @brief Returns the register-type name written into the generated parser. The parser is source
 *        code, so its comments stay in English regardless of the UI language.
 */
[[nodiscard]] static QString registerTypeLabel(const quint8 type)
{
  const QStringList names = {
    QStringLiteral("Holding Registers"),
    QStringLiteral("Input Registers"),
    QStringLiteral("Coils"),
    QStringLiteral("Discrete Inputs"),
  };

  return (type < names.count()) ? names[type] : QStringLiteral("Unknown");
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the groups the project is generated from; the driver may edit its own list while
 *        the save dialog is up.
 */
IO::Drivers::ModbusProjectGenerator::ModbusProjectGenerator(
  const QVector<ModbusRegisterGroup>& groups)
  : m_groups(groups)
{}

//--------------------------------------------------------------------------------------------------
// Project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns how many datasets the generated project will contain.
 */
int IO::Drivers::ModbusProjectGenerator::totalDatasets() const
{
  int total = 0;
  for (const auto& group : std::as_const(m_groups))
    total += group.count;

  return total;
}

/**
 * @brief Assembles the complete project JSON object, storing @p connectionSettings as the source's
 *        connection block so the generated project reconnects to the device it was polled from.
 */
QJsonObject IO::Drivers::ModbusProjectGenerator::buildProject(
  const QJsonObject& connectionSettings) const
{
  QJsonObject project;
  project[Keys::Title]   = trModbus("Modbus Project");
  project[Keys::Actions] = QJsonArray();

  QJsonObject source;
  source[Keys::SourceId]              = 0;
  source[Keys::Title]                 = trModbus("Modbus");
  source[Keys::BusType]               = static_cast<int>(SerialStudio::BusType::ModBus);
  source[Keys::FrameStart]            = QString();
  source[Keys::FrameEnd]              = QString();
  source[Keys::Checksum]              = QString();
  source[Keys::FrameDetection]        = static_cast<int>(SerialStudio::NoDelimiters);
  source[Keys::Decoder]               = static_cast<int>(SerialStudio::Binary);
  source[Keys::HexadecimalDelimiters] = false;
  source[Keys::FrameParserCode]       = buildFrameParser();
  source[Keys::FrameParserLanguage]   = static_cast<int>(SerialStudio::Lua);
  source[Keys::SourceConn]            = connectionSettings;

  project[Keys::Sources] = QJsonArray{source};

  QJsonArray group_array;
  int group_id      = 0;
  int dataset_index = 1;

  for (const auto& reg_group : std::as_const(m_groups)) {
    DataModel::Group group;
    group.groupId = group_id;
    group.widget  = QStringLiteral("datagrid");
    group.title   = QStringLiteral("%1 @ %2").arg(registerTypeName(reg_group.registerType),
                                                QString::number(reg_group.startAddress));

    const bool is_reg = (reg_group.registerType <= 1);

    for (quint16 i = 0; i < reg_group.count; ++i) {
      DataModel::Dataset dataset;
      dataset.index = dataset_index++;
      dataset.log   = true;

      const quint16 addr = reg_group.startAddress + i;

      if (is_reg) {
        dataset.title  = trModbus("Register %1").arg(addr);
        dataset.plt    = true;
        dataset.wgtMin = 0;
        dataset.wgtMax = 65535;
        dataset.pltMin = 0;
        dataset.pltMax = 65535;
      } else {
        dataset.title   = (reg_group.registerType == 2) ? trModbus("Coil %1").arg(addr)
                                                        : trModbus("Discrete %1").arg(addr);
        dataset.led     = true;
        dataset.ledHigh = 1;
        dataset.wgtMin  = 0;
        dataset.wgtMax  = 1;
      }

      group.datasets.push_back(dataset);
    }

    group_array.append(DataModel::serialize(group));
    ++group_id;
  }

  project[QStringLiteral("groups")] = group_array;
  return project;
}

/**
 * @brief Generates the Lua frame parser for the configured groups. The groups are polled in turn
 *        and every reply carries the same header, so the parser tracks the poll cycle itself. The
 *        cycle's modulo is appended rather than formatted: QString::arg() does not collapse "%%",
 *        so the escape the old builder used reached the generated source verbatim.
 */
QString IO::Drivers::ModbusProjectGenerator::buildFrameParser() const
{
  const int group_count = m_groups.count();

  QString code;

  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Modbus Register Frame Parser\n");
  code += QStringLiteral("-- Auto-generated by Serial Studio\n");
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Total groups: %1\n").arg(group_count);
  code += QStringLiteral("-- Total datasets: %1\n").arg(totalDatasets());
  code += QStringLiteral("--\n");
  code += QStringLiteral("-- Frame format: {slaveAddr, funcCode, byteCount, ...data}\n");
  code += QStringLiteral("-- Groups are polled sequentially; this parser tracks the cycle.\n");
  code += QStringLiteral("--\n\n");

  code += QStringLiteral("local values = {}\n");
  code += QStringLiteral("for i = 1, %1 do values[i] = 0 end\n").arg(totalDatasets());
  code += QStringLiteral("local currentGroup = 0\n\n");

  code += QStringLiteral("function parse(frame)\n");
  code += QStringLiteral("  if #frame < 3 then return values end\n\n");
  code += QStringLiteral("  -- Extract data payload (skip slave addr, func code, byte count)\n");
  code += QStringLiteral("  local data = {}\n");
  code += QStringLiteral("  for i = 4, #frame do data[#data + 1] = frame[i] end\n\n");

  int dataset_offset = 0;
  for (int g = 0; g < group_count; ++g) {
    const auto& reg_group = m_groups[g];
    const bool is_reg     = (reg_group.registerType <= 1);

    const QString keyword  = (g == 0) ? QStringLiteral("if") : QStringLiteral("elseif");
    code                  += QStringLiteral("  %1 currentGroup == %2 then -- %3 @ %4, count=%5\n")
              .arg(keyword,
                   QString::number(g),
                   registerTypeLabel(reg_group.registerType),
                   QString::number(reg_group.startAddress),
                   QString::number(reg_group.count));

    for (quint16 i = 0; i < reg_group.count; ++i) {
      if (is_reg) {
        const int byte_off  = i * 2 + 1;
        code               += QStringLiteral("    values[%1] = (data[%2] << 8) | data[%3]\n")
                  .arg(dataset_offset + i + 1)
                  .arg(byte_off)
                  .arg(byte_off + 1);
      }

      else {
        const int byte_idx  = i / 8 + 1;
        const int bit_idx   = i % 8;
        code               += QStringLiteral("    values[%1] = (data[%2] >> %3) & 1\n")
                  .arg(dataset_offset + i + 1)
                  .arg(byte_idx)
                  .arg(bit_idx);
      }
    }

    dataset_offset += reg_group.count;
  }

  if (group_count > 0)
    code += QStringLiteral("  end\n\n");

  code += QStringLiteral("  currentGroup = (currentGroup + 1) % ");
  code += QString::number(group_count);
  code += QStringLiteral("\n  return values\n");
  code += QStringLiteral("end\n");

  return code;
}
