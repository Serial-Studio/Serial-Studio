/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <QString>
#include <QVector>

namespace DataModel::ModbusMap {

/**
 * @brief One Modbus register parsed from a CSV/XML/JSON map.
 */
struct RegisterEntry {
  quint16 address;
  QString name;
  quint8 registerType;
  QString dataType;
  QString units;
  double min;
  double max;
  double scale;
  double offset;
};

[[nodiscard]] quint8 parseRegisterType(const QString& str);

[[nodiscard]] int registersForDataType(const QString& dataType);

[[nodiscard]] bool parseCsv(const QString& path, QVector<RegisterEntry>& out);

[[nodiscard]] bool parseXml(const QString& path, QVector<RegisterEntry>& out);

[[nodiscard]] bool parseJson(const QString& path, QVector<RegisterEntry>& out);

}  // namespace DataModel::ModbusMap
