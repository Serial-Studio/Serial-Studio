/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "Misc/Diagnostics/BluetoothChecks.h"

#include <QCoreApplication>

#if QT_CONFIG(permissions)
#  include <QPermissions>
#endif

#include "IO/ConnectionManager.h"
#include "IO/Drivers/BluetoothLE.h"

//--------------------------------------------------------------------------------------------------
// Local aliases
//--------------------------------------------------------------------------------------------------

using Misc::Diagnostics::Bus;
using Misc::Diagnostics::makeResult;
using Misc::Diagnostics::Result;
using Misc::Diagnostics::trDiag;
using Misc::Diagnostics::Verdict;

//--------------------------------------------------------------------------------------------------
// Individual checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the platform's advice for switching the Bluetooth radio back on.
 */
[[nodiscard]] static QString powerOnRemedy()
{
#if defined(Q_OS_WINDOWS)
  return trDiag("Turn Bluetooth on in Settings > Bluetooth & devices.");
#elif defined(Q_OS_MACOS)
  return trDiag("Turn Bluetooth on in System Settings > Bluetooth.");
#else
  return trDiag("Turn Bluetooth on in the desktop's Bluetooth settings, or run %1.")
    .arg(QStringLiteral("rfkill unblock bluetooth"));
#endif
}

/**
 * @brief Returns the platform's advice for granting the Bluetooth permission by hand.
 */
[[nodiscard]] static QString permissionRemedy()
{
#if defined(Q_OS_MACOS)
  return trDiag("Allow Serial Studio under System Settings > Privacy & Security > Bluetooth, "
                "then restart the application.");
#else
  return trDiag("Grant Serial Studio the Bluetooth permission in the system's privacy settings, "
                "then restart the application.");
#endif
}

/**
 * @brief Reports a Bluetooth permission the system has refused. The status is read, never
 *        requested, so a run cannot raise a permission dialog the user did not ask for.
 */
static void reportBluetoothPermission(QList<Result>& out)
{
#if QT_CONFIG(permissions)
  auto* app = QCoreApplication::instance();
  if (app == nullptr)
    return;

  if (app->checkPermission(QBluetoothPermission{}) != Qt::PermissionStatus::Denied)
    return;

  out.append(makeResult(Bus::Bluetooth,
                        Verdict::Failure,
                        "ble-permission-denied",
                        trDiag("Bluetooth access is not permitted"),
                        trDiag("The system has refused Serial Studio permission to use "
                               "Bluetooth, so no device can be discovered."),
                        permissionRemedy()));
#else
  Q_UNUSED(out)
#endif
}

/**
 * @brief Reports an absent or powered-off adapter, read from the shared state the driver already
 *        tracks rather than from a second local-device object.
 */
static void reportAdapter(QList<Result>& out)
{
  if (IO::Drivers::BluetoothLE::adapterPoweredOn())
    return;

  out.append(makeResult(Bus::Bluetooth,
                        Verdict::Failure,
                        "ble-adapter-off",
                        trDiag("Bluetooth is turned off"),
                        trDiag("No powered-on Bluetooth adapter was found, so no device can be "
                               "discovered or connected."),
                        powerOnRemedy()));
}

/**
 * @brief Reports a platform with no Bluetooth Low Energy support at all, returning false when
 *        the remaining checks would be meaningless.
 */
[[nodiscard]] static bool reportPlatformSupport(QList<Result>& out)
{
  static auto& manager = IO::ConnectionManager::instance();

  auto* ble = manager.bluetoothLE();
  if (ble != nullptr && ble->operatingSystemSupported())
    return true;

  out.append(makeResult(Bus::Bluetooth,
                        Verdict::Warning,
                        "ble-unsupported",
                        trDiag("Bluetooth Low Energy is not available"),
                        trDiag("This build of the operating system does not provide the "
                               "Bluetooth Low Energy support Serial Studio needs."),
                        trDiag("Use another bus, or run Serial Studio on a system with "
                               "Bluetooth Low Energy support.")));
  return false;
}

//--------------------------------------------------------------------------------------------------
// Collection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs every instant Bluetooth check in declaration order.
 */
void Misc::Diagnostics::BluetoothChecks::collect(QList<Result>& out)
{
  if (!reportPlatformSupport(out))
    return;

  reportBluetoothPermission(out);
  reportAdapter(out);
}
