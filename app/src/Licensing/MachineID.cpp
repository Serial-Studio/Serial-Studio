/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "MachineID.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QProcess>
#include <QtCore/qendian.h>

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a MachineID instance and initializes system information.
 */
Licensing::MachineID::MachineID()
{
  readInformation();
}

/**
 * @brief Retrieves the singleton instance of MachineID.
 */
Licensing::MachineID& Licensing::MachineID::instance()
{
  static MachineID instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Member access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the hashed, base64-encoded machine identifier.
 */
const QString& Licensing::MachineID::machineId() const noexcept
{
  return m_machineId;
}

/**
 * @brief Returns the hashed, base64-encoded application version machine identifier.
 */
const QString& Licensing::MachineID::appVerMachineId() const noexcept
{
  return m_appVerMachineId;
}

/**
 * @brief Returns the machine-specific encryption key.
 */
quint64 Licensing::MachineID::machineSpecificKey() const noexcept
{
  return m_machineSpecificKey;
}

//--------------------------------------------------------------------------------------------------
// Information gathering and processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Collects system-specific data to generate a unique machine identifier and encryption key.
 */
void Licensing::MachineID::readInformation()
{
  QString id;
  QString os;
  QProcess process;

// Obtain machine ID in GNU/Linux
#if defined(Q_OS_LINUX)
  os = QStringLiteral("Linux");
  process.start("cat", {"/var/lib/dbus/machine-id"});
  process.waitForFinished();
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty()) {
    process.start("cat", {"/etc/machine-id"});
    process.waitForFinished();
    id = process.readAllStandardOutput().trimmed();
  }
#endif

// Obtain machine ID in macOS
#if defined(Q_OS_MAC)
  os = QStringLiteral("macOS");
  process.start("ioreg", {"-rd1", "-c", "IOPlatformExpertDevice"});
  process.waitForFinished();
  QString output = process.readAllStandardOutput();

  QStringList lines = output.split("\n");
  for (const QString& line : std::as_const(lines)) {
    if (line.contains("IOPlatformUUID")) {
      id = line.split("=").last().trimmed();
      id.remove("\"");
      break;
    }
  }
#endif

// Obtain machine ID in Windows
#if defined(Q_OS_WIN)
  os = QStringLiteral("Windows");
  QString machineGuid, uuid;

  // Read MachineGuid from the registry
  process.start(
    "reg", {"query", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", "/v", "MachineGuid"});
  process.waitForFinished();
  QString output    = process.readAllStandardOutput();
  QStringList lines = output.split("\n");
  for (const QString& line : std::as_const(lines)) {
    if (line.contains("MachineGuid")) {
      machineGuid = line.split(" ").last().trimmed();
      break;
    }
  }

  // Read system UUID via PowerShell
  process.start("powershell",
                {"-ExecutionPolicy",
                 "Bypass",
                 "-command",
                 "(Get-CimInstance -Class Win32_ComputerSystemProduct).UUID"});
  process.waitForFinished();
  uuid = process.readAllStandardOutput().trimmed();

  id = machineGuid + uuid;
#endif

// Obtain machine ID in OpenBSD
#if defined(Q_OS_BSD)
  os = QStringLiteral("BSD");
  process.start("cat", {"/etc/hostid"});
  process.waitForFinished();
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty()) {
    process.start("kenv", {"-q", "smbios.system.uuid"});
    process.waitForFinished();
    id = process.readAllStandardOutput().trimmed();
  }
#endif

  // Warn when every platform fallback returned empty; derivation still proceeds
  if (id.isEmpty()) [[unlikely]]
    qWarning() << "[MachineID] fallback produced empty fingerprint";

  // Hash the composite identifier with BLAKE2s-128
  auto data = QString("%1@%2:%3").arg(qApp->applicationName(), id, os);
  auto hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Blake2s_128);

  // Derive machine ID and encryption key
  m_machineId          = QString::fromUtf8(hash.toBase64());
  m_machineSpecificKey = qFromBigEndian<quint64>(hash.left(8));

  // Derive version-specific machine ID
  auto appVerData =
    QString("%1_%2@%3:%4").arg(qApp->applicationName(), qApp->applicationVersion(), id, os);
  auto appVerHash = QCryptographicHash::hash(appVerData.toUtf8(), QCryptographicHash::Blake2s_128);
  m_appVerMachineId = QString::fromUtf8(appVerHash.toBase64());
}
