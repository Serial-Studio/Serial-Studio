/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>

namespace Licensing {
/**
 * @brief Provides a consistent, hashed machine identifier for licensing
 *        and data encryption.
 */
class MachineID : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QString machineId
             READ machineId
             CONSTANT)
  Q_PROPERTY(quint64 machineSpecificKey
             READ machineSpecificKey
             CONSTANT)
  // clang-format on

private:
  explicit MachineID();
  MachineID(MachineID&&)                 = delete;
  MachineID(const MachineID&)            = delete;
  MachineID& operator=(MachineID&&)      = delete;
  MachineID& operator=(const MachineID&) = delete;

public:
  [[nodiscard]] static MachineID& instance();

  [[nodiscard]] const QString& machineId() const noexcept;
  [[nodiscard]] const QString& appVerMachineId() const noexcept;
  [[nodiscard]] quint64 machineSpecificKey() const noexcept;

private:
  void readInformation();

private:
  QString m_machineId;
  QString m_appVerMachineId;
  quint64 m_machineSpecificKey;
};
}  // namespace Licensing
