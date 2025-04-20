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

#include "MachineID.h"

#include <QProcess>
#include <QApplication>
#include <QtCore/qendian.h>
#include <QCryptographicHash>

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs a MachineID instance and initializes system information.
 *
 * The constructor calls readInformation() once to gather all relevant
 * hardware and system details. This ensures that machine identification
 * remains consistent throughout the program's execution.
 */
Licensing::MachineID::MachineID()
{
  readInformation();
}

/**
 * @brief Retrieves the singleton instance of MachineID.
 *
 * This function follows the Singleton pattern to ensure that only one
 * instance of MachineID exists during runtime.
 *
 * @return Reference to the singleton instance of MachineID.
 */
Licensing::MachineID &Licensing::MachineID::instance()
{
  static MachineID instance;
  return instance;
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns the hashed, base64-encoded machine identifier.
 *
 * This value is generated based on platform-specific identifiers and the
 * application name, then hashed and encoded to avoid leaking identifiable
 * information. It provides a consistent machine ID across sessions for
 * licensing, caching, or other per-device logic.
 *
 * @return A reference to the machine-specific ID string (Base64-encoded).
 */
const QString &Licensing::MachineID::machineId() const
{
  return m_machineId;
}

/**
 * @brief Returns the machine-specific encryption key.
 *
 * This 64-bit key is derived from the same input used to generate the machine
 * ID. It is intended for local data encryption (e.g., cached license info),
 * ensuring that encrypted content cannot be reused or decrypted on other
 * machines.
 *
 * @return A 64-bit unsigned integer representing the machine-specific
 *         encryption key.
 */
quint64 Licensing::MachineID::machineSpecificKey() const
{
  return m_machineSpecificKey;
}

//------------------------------------------------------------------------------
// Information gathering and processing
//------------------------------------------------------------------------------

/**
 * @brief Collects system-specific data to generate a unique machine identifier
 *        and encryption key.
 *
 * This method gathers platform-specific machine information depending on the
 * operating system.
 *
 * The resulting machine-specific ID is then combined with the application name
 * and operating system name. This string is hashed using the BLAKE2s-128
 * algorithm to create a non-reversible, privacy-preserving identifier that
 * remains consistent on the same machine.
 *
 * Two values are derived:
 * - Machine ID: A base64-encoded hash string used for machine identification.
 * - Machine-specific key: A 64-bit key extracted from the hash, used to encrypt
 *   or decrypt locally cached sensitive data (e.g., license information),
 *   ensuring it cannot be reused across different machines.
 *
 * This method is called internally by the `MachineID` singleton and does not
 * need to be invoked directly.
 */
void Licensing::MachineID::readInformation()
{
  // Initialize common platform variables
  QString id;
  QString os;
  QProcess process;

// Obtain machine ID in GNU/Linux
#if defined(Q_OS_LINUX)
  os = QStringLiteral("Linux");
  process.start("cat", {"/var/lib/dbus/machine-id"});
  process.waitForFinished();
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty())
  {
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
  for (const QString &line : std::as_const(lines))
  {
    if (line.contains("IOPlatformUUID"))
    {
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

  // Get MachineGuid from Registry
  process.start("reg", {"query",
                        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography",
                        "/v", "MachineGuid"});
  process.waitForFinished();
  QString output = process.readAllStandardOutput();
  QStringList lines = output.split("\n");
  for (const QString &line : std::as_const(lines))
  {
    if (line.contains("MachineGuid"))
    {
      machineGuid = line.split(" ").last().trimmed();
      break;
    }
  }

  // Get UUID using PowerShell
  process.start("powershell",
                {"-ExecutionPolicy", "Bypass", "-command",
                 "(Get-CimInstance -Class Win32_ComputerSystemProduct).UUID"});
  process.waitForFinished();
  uuid = process.readAllStandardOutput().trimmed();

  // Combine MachineGuid and UUID
  id = machineGuid + uuid;
#endif

// Obtain machine ID in OpenBSD
#if defined(Q_OS_BSD)
  os = QStringLiteral("BSD");
  process.start("cat", {"/etc/hostid"});
  process.waitForFinished();
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty())
  {
    process.start("kenv", {"-q", "smbios.system.uuid"});
    process.waitForFinished();
    id = process.readAllStandardOutput().trimmed();
  }
#endif

  // Generate a hash based on the machine ID, application name and OS
  auto data = QString("%1@%2:%3").arg(qApp->applicationName(), id, os);
  auto hash = QCryptographicHash::hash(data.toUtf8(),
                                       QCryptographicHash::Blake2s_128);

  // Obtain the machine ID and encryption key as a base64 string
  m_machineId = QString::fromUtf8(hash.toBase64());
  m_machineSpecificKey = qFromBigEndian<quint64>(hash.left(8));
}
