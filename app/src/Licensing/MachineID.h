/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>

namespace Licensing
{
/**
 * @class MachineID
 * @brief Provides a consistent, hashed machine identifier for licensing
 *        and data encryption.
 *
 * The `MachineID` singleton class generates a unique, platform-agnostic machine
 * identifier based on system-specific properties.
 *
 * This ID is used to:
 * - Bind license keys and activations to a specific machine.
 * - Derive a stable encryption key for securing sensitive data such as license
 *   keys, passwords, and configuration values.
 *
 * The generated identifier does not contain personal or hardware-identifiable
 * information. It is a one-way hash, ensuring privacy while enforcing
 * per-device restrictions (e.g., seat limits).
 *
 * If encrypted data (e.g., license info) is copied from one machine to another,
 * the app will detect the mismatch and invalidate the cache automatically.
 *
 * This class guarantees that each machine returns the same ID across sessions,
 * while remaining secure and non-reversible.
 */
class MachineID : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString machineId READ machineId CONSTANT)
  Q_PROPERTY(quint64 machineSpecificKey READ machineSpecificKey CONSTANT)

private:
  explicit MachineID();
  MachineID(MachineID &&) = delete;
  MachineID(const MachineID &) = delete;
  MachineID &operator=(MachineID &&) = delete;
  MachineID &operator=(const MachineID &) = delete;

public:
  [[nodiscard]] static MachineID &instance();
  [[nodiscard]] const QString &machineId() const;
  [[nodiscard]] quint64 machineSpecificKey() const;

private:
  void readInformation();

private:
  QString m_machineId;
  quint64 m_machineSpecificKey;
};
} // namespace Licensing
