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

#include "IO/Drivers/SerialPortIdentity.h"

#include <utility>

static const auto kSeparator = QStringLiteral("  ");

static const auto kVid    = QStringLiteral("vid");
static const auto kPid    = QStringLiteral("pid");
static const auto kSerial = QStringLiteral("serial");
static const auto kName   = QStringLiteral("portName");
static const auto kDesc   = QStringLiteral("description");

static constexpr int kVidPidScore   = 100;
static constexpr int kSerialScore   = 50;
static constexpr int kDescScore     = 10;
static constexpr int kPortNameScore = 5;

/**
 * @brief Formats a USB identifier the way both the saved identity and the matcher expect.
 */
[[nodiscard]] static QString usbId(const quint16 value)
{
  return QString::number(value, 16).rightJustified(4, '0').toUpper();
}

/**
 * @brief Returns the serial ports the pickers show. Callout devices are hidden on macOS: every
 *        port appears twice there (cu.* and tty.*) and only the callout node is usable.
 */
QVector<QSerialPortInfo> IO::Drivers::SerialPorts::visiblePorts()
{
  QVector<QSerialPortInfo> filtered;

  const auto ports = QSerialPortInfo::availablePorts();
  for (const auto& info : ports) {
    if (info.isNull())
      continue;

#ifdef Q_OS_MACOS
    if (info.portName().toLower().startsWith("tty."))
      continue;
#endif

    filtered.append(info);
  }

  return filtered;
}

/**
 * @brief Returns the string a picker shows for one port. The description is what tells two
 *        identical-looking nodes apart, so it is appended whenever the device reports one.
 */
QString IO::Drivers::SerialPorts::portLabel(const QSerialPortInfo& info)
{
  const auto description = info.description().simplified();
  if (description.isEmpty())
    return info.portName();

  return info.portName() + kSeparator + description;
}

/**
 * @brief Returns the port name a label was built from. Every consumer of an enumerated string
 *        that must open, compare or persist a port goes through here: the label is also the
 *        identifier the CAN backend registry hands to create().
 */
QString IO::Drivers::SerialPorts::portNameFromLabel(const QString& label)
{
  const int separator = label.indexOf(kSeparator);
  if (separator < 0)
    return label.trimmed();

  return label.left(separator).trimmed();
}

/**
 * @brief Enumerates once and returns the ports with their labels and system locations, so a
 *        picker rebuilds its combo without a second scan of the bus.
 */
IO::Drivers::SerialPorts::PortListing IO::Drivers::SerialPorts::listPorts()
{
  PortListing listing;

  listing.ports = visiblePorts();
  listing.labels.reserve(listing.ports.count());
  listing.locations.reserve(listing.ports.count());

  for (const auto& info : std::as_const(listing.ports)) {
    listing.labels.append(portLabel(info));
    listing.locations.append(info.systemLocation());
  }

  return listing;
}

/**
 * @brief Resolves a label, port name or device path to the canonical port name, so a UART source
 *        and a Modbus or CAN source pointing at one port produce the same string.
 */
QString IO::Drivers::SerialPorts::resourceName(const QString& labelOrPath)
{
  const auto name = portNameFromLabel(labelOrPath);
  if (name.isEmpty())
    return {};

  const auto ports = visiblePorts();
  for (const auto& info : ports)
    if (info.portName() == name || info.systemLocation() == name)
      return info.portName();

  return name;
}

/**
 * @brief Finds a previously saved selection among the enumerated labels. The fallback matches on
 *        the port-name token alone: a selection saved before the labels carried the device
 *        description, or saved while the device reported a different one, is still the same port.
 */
int IO::Drivers::SerialPorts::indexOfPort(const QStringList& labels, const QString& saved)
{
  if (saved.isEmpty())
    return -1;

  const int exact = labels.indexOf(saved);
  if (exact >= 0)
    return exact;

  const auto name = portNameFromLabel(saved);
  for (int i = 0; i < labels.count(); ++i)
    if (portNameFromLabel(labels.at(i)) == name)
      return i;

  return -1;
}

/**
 * @brief Returns the identifier a project stores for one port. Only the fields the device really
 *        reports are written, so a match never scores on a fabricated zero.
 */
QJsonObject IO::Drivers::SerialPorts::identity(const QSerialPortInfo& info)
{
  QJsonObject id;

  if (info.hasVendorIdentifier())
    id.insert(kVid, usbId(info.vendorIdentifier()));

  if (info.hasProductIdentifier())
    id.insert(kPid, usbId(info.productIdentifier()));

  const auto serial = info.serialNumber();
  if (!serial.isEmpty())
    id.insert(kSerial, serial);

  id.insert(kName, info.portName());

  const auto description = info.description();
  if (!description.isEmpty())
    id.insert(kDesc, description);

  return id;
}

/**
 * @brief Scores @p candidate against @p saved. The VID/PID pair identifies the hardware model and
 *        the serial the individual unit, so those outrank a description or a port name, which the
 *        operating system reassigns between sessions.
 */
int IO::Drivers::SerialPorts::scoreIdentityMatch(const QJsonObject& candidate,
                                                 const QJsonObject& saved)
{
  const auto savedVid = saved.value(kVid).toString();
  const auto candVid  = candidate.value(kVid).toString();

  int score = 0;

  if (!savedVid.isEmpty() && !candVid.isEmpty() && candVid == savedVid
      && candidate.value(kPid).toString() == saved.value(kPid).toString()) {
    score += kVidPidScore;

    const auto savedSerial = saved.value(kSerial).toString();
    if (!savedSerial.isEmpty() && candidate.value(kSerial).toString() == savedSerial)
      score += kSerialScore;
  }

  const auto savedDesc = saved.value(kDesc).toString();
  if (!savedDesc.isEmpty() && candidate.value(kDesc).toString() == savedDesc)
    score += kDescScore;

  const auto savedName = saved.value(kName).toString();
  if (!savedName.isEmpty() && candidate.value(kName).toString() == savedName)
    score += kPortNameScore;

  return score;
}
