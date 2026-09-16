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
#include <QStringList>
#include <QVector>

namespace IO::Drivers::SerialPorts {

/**
 * @brief One enumeration pass: the ports themselves plus the parallel label and system-location
 *        lists every serial picker builds its combo from.
 */
struct PortListing {
  QVector<QSerialPortInfo> ports;
  QStringList labels;
  QStringList locations;
};

/**
 * @brief Returns the serial ports the pickers show, with the platform filter applied.
 */
[[nodiscard]] QVector<QSerialPortInfo> visiblePorts();

/**
 * @brief Enumerates the visible ports once and returns them with their labels and locations.
 */
[[nodiscard]] PortListing listPorts();

/**
 * @brief Returns the string a picker shows for one port: its name, plus the device description
 *        when the port reports one.
 */
[[nodiscard]] QString portLabel(const QSerialPortInfo& info);

/**
 * @brief Returns the port name a label was built from; a string that is not a label is its own.
 */
[[nodiscard]] QString portNameFromLabel(const QString& label);

/**
 * @brief Returns the canonical name of the port a label, port name or device path refers to, so
 *        two drivers naming one port compare equal.
 */
[[nodiscard]] QString resourceName(const QString& labelOrPath);

/**
 * @brief Finds @p saved among @p labels, falling back to the port-name token: a selection saved
 *        before the labels carried descriptions holds a bare port name. Returns -1 on no match.
 */
[[nodiscard]] int indexOfPort(const QStringList& labels, const QString& saved);

/**
 * @brief Returns the persisted hardware identifier of one port (VID, PID, serial, name, label).
 */
[[nodiscard]] QJsonObject identity(const QSerialPortInfo& info);

/**
 * @brief Scores how strongly a port identity matches a previously saved one.
 */
[[nodiscard]] int scoreIdentityMatch(const QJsonObject& candidate, const QJsonObject& saved);

}  // namespace IO::Drivers::SerialPorts
