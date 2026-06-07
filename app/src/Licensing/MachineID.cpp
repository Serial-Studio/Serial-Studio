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
#include <QFileInfo>
#include <QProcess>
#include <QtCore/qendian.h>

//--------------------------------------------------------------------------------------------------
// Executable path resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves a system tool to an absolute path, falling back to its bare name.
 */
static QString resolveSystemTool(const QStringList& candidates, const QString& fallbackName)
{
  for (const auto& path : candidates) {
    QFileInfo info(path);
    if (info.exists() && info.isExecutable())
      return path;
  }

  return fallbackName;
}

/**
 * @brief Waits for a fingerprint helper with a hard timeout, killing it if it wedges.
 */
static void awaitTool(QProcess& process, int timeoutMs = 3000)
{
  // The timeout must clear the tool's normal worst case, else a slow start changes the fingerprint.
  if (!process.waitForFinished(timeoutMs)) {
    process.kill();
    process.waitForFinished(500);
  }
}

/**
 * @brief Gathers the raw, platform-specific machine identifier and OS label.
 */
static QString readPlatformId(QString& os)
{
  QString id;
  QProcess process;

// Obtain machine ID in GNU/Linux
#if defined(Q_OS_LINUX)
  os                 = QStringLiteral("Linux");
  const auto catTool = resolveSystemTool({"/bin/cat", "/usr/bin/cat"}, "cat");
  process.start(catTool, {"/var/lib/dbus/machine-id"});
  awaitTool(process);
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty()) {
    process.start(catTool, {"/etc/machine-id"});
    awaitTool(process);
    id = process.readAllStandardOutput().trimmed();
  }
#endif

// Obtain machine ID in macOS
#if defined(Q_OS_MAC)
  os                   = QStringLiteral("macOS");
  const auto ioregTool = resolveSystemTool({"/usr/sbin/ioreg"}, "ioreg");
  process.start(ioregTool, {"-rd1", "-c", "IOPlatformExpertDevice"});
  awaitTool(process);
  QString output = process.readAllStandardOutput();

  QStringList lines = output.split("\n");
  for (const QString& line : std::as_const(lines)) {
    if (line.contains("IOPlatformUUID")) {
      // Only accept the `key = "value"` shape; never build an ID from a line lacking '='.
      const auto parts = line.split("=");
      if (parts.size() >= 2) {
        id = parts.last().trimmed();
        id.remove("\"");
      }
      break;
    }
  }
#endif

// Obtain machine ID in Windows
#if defined(Q_OS_WIN)
  os = QStringLiteral("Windows");
  QString machineGuid, uuid;

  // Resolve system tools under the real Windows directory
  auto systemRoot = qEnvironmentVariable("SystemRoot");
  if (systemRoot.isEmpty())
    systemRoot = QStringLiteral("C:\\Windows");

  const auto system32 = systemRoot + QStringLiteral("\\System32\\");
  const auto regTool  = resolveSystemTool({system32 + "reg.exe"}, "reg");

  // Read MachineGuid from the registry
  process.start(
    regTool,
    {"query", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", "/v", "MachineGuid"});
  awaitTool(process);
  QString output    = process.readAllStandardOutput();
  QStringList lines = output.split("\n");
  for (const QString& line : std::as_const(lines)) {
    if (line.contains("MachineGuid")) {
      machineGuid = line.split(" ").last().trimmed();
      break;
    }
  }

  // Read the system UUID via PowerShell; allow a long timeout so cold-start cannot drop it.
  const auto psTool =
    resolveSystemTool({system32 + "WindowsPowerShell\\v1.0\\powershell.exe"}, "powershell");
  process.start(psTool,
                {"-ExecutionPolicy",
                 "Bypass",
                 "-command",
                 "(Get-CimInstance -Class Win32_ComputerSystemProduct).UUID"});
  awaitTool(process, 30000);
  uuid = process.readAllStandardOutput().trimmed();

  id = machineGuid + uuid;
#endif

// Obtain machine ID in OpenBSD
#if defined(Q_OS_BSD)
  os                  = QStringLiteral("BSD");
  const auto catTool  = resolveSystemTool({"/bin/cat", "/usr/bin/cat"}, "cat");
  const auto kenvTool = resolveSystemTool({"/sbin/kenv", "/usr/sbin/kenv"}, "kenv");
  process.start(catTool, {"/etc/hostid"});
  awaitTool(process);
  id = process.readAllStandardOutput().trimmed();

  if (id.isEmpty()) {
    process.start(kenvTool, {"-q", "smbios.system.uuid"});
    awaitTool(process);
    id = process.readAllStandardOutput().trimmed();
  }
#endif

  return id;
}

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
  QString os;
  const QString id = readPlatformId(os);

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
