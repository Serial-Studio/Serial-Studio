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

#include "Misc/Diagnostics/SerialChecks.h"

#include <QFile>
#include <QMap>
#include <QSerialPortInfo>
#include <QSettings>
#include <QStringList>

#include "Misc/Diagnostics/DeviceAccess.h"

//--------------------------------------------------------------------------------------------------
// Constants, local aliases & grouped state
//--------------------------------------------------------------------------------------------------

using Misc::Diagnostics::Bus;
using Misc::Diagnostics::makeResult;
using Misc::Diagnostics::Result;
using Misc::Diagnostics::trDiag;
using Misc::Diagnostics::Verdict;

static constexpr int kMaxProbedPorts = 64;

namespace Misc::Diagnostics::detail {
/**
 * @brief The ports one owning group blocks, plus that group's membership facts, so N ports in
 *        one group produce one finding instead of N.
 */
struct GroupBlock {
  bool accountInGroup  = false;
  bool sessionHasGroup = false;
  QStringList ports;
};
}  // namespace Misc::Diagnostics::detail

using Misc::Diagnostics::detail::GroupBlock;

//--------------------------------------------------------------------------------------------------
// Remedy composition
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the platform's advice for a machine that enumerates no serial port at all.
 */
[[nodiscard]] static QString driverFamilyRemedy()
{
#if defined(Q_OS_WINDOWS)
  return trDiag("Install the USB-serial driver for the adapter's chip (%1), then unplug and "
                "reconnect the adapter.")
    .arg(QStringLiteral("CH340/CH341, CP210x, FTDI FT232"));
#elif defined(Q_OS_MACOS)
  return trDiag("Install the USB-serial driver for the adapter's chip (%1), allow it in System "
                "Settings > Privacy & Security, then reconnect the adapter.")
    .arg(QStringLiteral("CH340/CH341, CP210x, FTDI FT232"));
#else
  return trDiag("Confirm the adapter is detected with %1 and that its kernel module (%2) is "
                "loaded, then reconnect the adapter.")
    .arg(QStringLiteral("lsusb"), QStringLiteral("ch341, cp210x, ftdi_sio"));
#endif
}

/**
 * @brief Builds the literal command that adds this account to @p group, assembled from
 *        untranslated literals so a translator can never localize it into something that fails.
 */
[[nodiscard]] static QString usermodCommand(const QString& group)
{
  const auto account = Misc::Diagnostics::currentAccountName();
  if (account.isEmpty())
    return QStringLiteral("sudo usermod -aG %1 $USER").arg(group);

  return QStringLiteral("sudo usermod -aG %1 %2").arg(group, account);
}

/**
 * @brief Picks the remedy for a blocked group from the four states of (member on paper, member
 *        in this session), so the command is printed only when running it is actually the fix.
 */
[[nodiscard]] static QString describeGroupRemedy(const QString& group, const GroupBlock& block)
{
  if (group.isEmpty())
    return trDiag("Check the owner and mode of the device node, and add a udev rule that grants "
                  "this account access to it.");

  if (block.accountInGroup && block.sessionHasGroup)
    return trDiag("Group membership is not the cause: this session already belongs to %1. Check "
                  "the owner and mode of the device node, or add a udev rule for it.")
      .arg(group);

  if (block.accountInGroup)
    return trDiag("This account already belongs to %1, but the running session does not. Log out "
                  "and back in for the membership to take effect.")
      .arg(group);

  return trDiag("Run %1, then log out and back in.").arg(usermodCommand(group));
}

/**
 * @brief Assembles the finding for one owning group that denies access to its ports.
 */
[[nodiscard]] static Result describeBlockedGroup(const QString& group, const GroupBlock& block)
{
  const auto names = block.ports.join(QStringLiteral(", "));

  QString explanation = trDiag("This account cannot read and write %1.").arg(names);
  if (!group.isEmpty())
    explanation = trDiag("This account cannot read and write %1, whose device node belongs to "
                         "group %2.")
                    .arg(names, group);

  return makeResult(Bus::Serial,
                    Verdict::Failure,
                    "port-access-denied",
                    trDiag("Serial port access is denied"),
                    explanation,
                    describeGroupRemedy(group, block));
}

//--------------------------------------------------------------------------------------------------
// Individual checks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reports a machine with no serial ports at all, which on Windows and macOS is almost
 *        always a missing USB-serial driver rather than a missing adapter.
 */
static void reportNoPorts(QList<Result>& out)
{
  if (!QSerialPortInfo::availablePorts().isEmpty())
    return;

  out.append(makeResult(Bus::Serial,
                        Verdict::Warning,
                        "no-serial-ports",
                        trDiag("No serial ports were found"),
                        trDiag("The system reports no serial ports, so there is nothing for this "
                               "bus to connect to."),
                        driverFamilyRemedy()));
}

/**
 * @brief Reports a previously selected port that the system no longer enumerates.
 */
static void reportMissingSelection(QList<Result>& out)
{
  QSettings settings;
  const auto selected = settings.value(QStringLiteral("IO_Serial_SelectedDevice")).toString();
  if (selected.isEmpty())
    return;

  if (selected.startsWith(QLatin1Char('/')) && QFile::exists(selected))
    return;

  const auto port_name = selected.section(QStringLiteral("  "), 0, 0).trimmed();
  const auto ports     = QSerialPortInfo::availablePorts();
  for (const auto& info : ports)
    if (info.portName() == port_name || info.systemLocation() == selected)
      return;

  out.append(makeResult(Bus::Serial,
                        Verdict::Warning,
                        "selected-port-missing",
                        trDiag("The selected serial port is gone"),
                        trDiag("Port %1 was selected previously and the system does not report "
                               "it any more.")
                          .arg(selected),
                        trDiag("Reconnect the adapter, or select another port in the device "
                               "setup pane.")));
}

/**
 * @brief Reports enumerated ports this process cannot read and write, grouped by the group that
 *        owns their device nodes.
 */
static void reportInaccessiblePorts(QList<Result>& out)
{
  QMap<QString, GroupBlock> blocked;
  const auto ports = QSerialPortInfo::availablePorts();

  for (int i = 0; i < ports.size() && i < kMaxProbedPorts; ++i) {
    const auto probe = Misc::Diagnostics::probeDeviceNode(ports.at(i).systemLocation());
    if (!probe.accessKnown || !probe.exists || (probe.readable && probe.writable))
      continue;

    auto& block           = blocked[probe.ownerGroup];
    block.accountInGroup  = probe.accountInGroup;
    block.sessionHasGroup = probe.sessionHasGroup;
    block.ports.append(ports.at(i).portName());
  }

  for (auto it = blocked.constBegin(); it != blocked.constEnd(); ++it)
    out.append(describeBlockedGroup(it.key(), it.value()));
}

//--------------------------------------------------------------------------------------------------
// Collection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs every instant serial check in declaration order.
 */
void Misc::Diagnostics::SerialChecks::collect(QList<Result>& out)
{
  reportNoPorts(out);
  reportMissingSelection(out);
  reportInaccessiblePorts(out);
}
