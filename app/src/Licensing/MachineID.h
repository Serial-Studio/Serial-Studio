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
  [[nodiscard]] const QString &appVerMachineId() const;

  [[nodiscard]] quint64 machineSpecificKey() const;

private:
  void readInformation();

private:
  QString m_machineId;
  QString m_appVerMachineId;
  quint64 m_machineSpecificKey;
};
} // namespace Licensing
