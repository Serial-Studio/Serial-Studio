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
#include <QScopeGuard>
#include <QtCore/qendian.h>

#if defined(Q_OS_WIN)
#  include <comdef.h>
#  include <wbemidl.h>
#  include <windows.h>
#endif

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
static void awaitTool(QProcess& process)
{
  // Runs before the event loop, so a hung tool must not block launch for Qt's 30s default.
  static constexpr int kToolTimeoutMs = 3000;
  if (!process.waitForFinished(kToolTimeoutMs)) {
    process.kill();
    process.waitForFinished(500);
  }
}

#if defined(Q_OS_WIN)
/**
 * @brief Reads Win32_ComputerSystemProduct.UUID via WMI/COM without spawning a process.
 */
static QString readWmiComputerSystemProductUuid()
{
  // Initialize COM; comOwned records whether this call owns the de-init.
  const HRESULT initHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  const bool comOwned  = SUCCEEDED(initHr);
  if (initHr == RPC_E_CHANGED_MODE)
    return QString();

  // Cleanup runs on every exit path; release order is reverse of acquisition.
  IWbemLocator* locator     = nullptr;
  IWbemServices* services   = nullptr;
  IEnumWbemClassObject* itr = nullptr;
  IWbemClassObject* obj     = nullptr;
  QString uuid;
  auto cleanup = qScopeGuard([&] {
    if (obj)
      obj->Release();

    if (itr)
      itr->Release();

    if (services)
      services->Release();

    if (locator)
      locator->Release();

    if (comOwned)
      CoUninitialize();
  });

  // Process-wide security is a one-shot call; a benign failure here is non-fatal.
  CoInitializeSecurity(nullptr,
                       -1,
                       nullptr,
                       nullptr,
                       RPC_C_AUTHN_LEVEL_DEFAULT,
                       RPC_C_IMP_LEVEL_IMPERSONATE,
                       nullptr,
                       EOAC_NONE,
                       nullptr);

  // Connect to the WMI locator and the CIMV2 namespace
  if (FAILED(CoCreateInstance(CLSID_WbemLocator,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_IWbemLocator,
                              reinterpret_cast<void**>(&locator))))
    return QString();
  if (FAILED(locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services)))
    return QString();

  // Query the single product instance for its UUID property
  CoSetProxyBlanket(services,
                    RPC_C_AUTHN_WINNT,
                    RPC_C_AUTHZ_NONE,
                    nullptr,
                    RPC_C_AUTHN_LEVEL_CALL,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr,
                    EOAC_NONE);
  if (FAILED(services->ExecQuery(_bstr_t(L"WQL"),
                                 _bstr_t(L"SELECT UUID FROM Win32_ComputerSystemProduct"),
                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 nullptr,
                                 &itr)))
    return QString();

  // Read the UUID string from the first row
  ULONG returned = 0;
  if (itr->Next(WBEM_INFINITE, 1, &obj, &returned) == WBEM_S_NO_ERROR && obj) {
    VARIANT value;
    VariantInit(&value);
    if (SUCCEEDED(obj->Get(L"UUID", 0, &value, nullptr, nullptr)) && value.vt == VT_BSTR)
      uuid = QString::fromWCharArray(value.bstrVal).trimmed();

    VariantClear(&value);
  }

  // Treat the documented "no UUID" sentinels as empty so the fingerprint stays stable.
  if (uuid.compare("00000000-0000-0000-0000-000000000000", Qt::CaseInsensitive) == 0
      || uuid.compare("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF", Qt::CaseInsensitive) == 0)
    return QString();

  return uuid;
}
#endif

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

  // Read the system UUID via WMI; native call avoids PowerShell cold-start racing the timeout.
  uuid = readWmiComputerSystemProductUuid();

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
