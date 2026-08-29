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

#include "Misc/CLI/CliSpecParsers.h"

#include <QDebug>
#include <QHash>
#include <QStringList>

#include "SSAssert.h"

namespace Misc {
namespace CliSpecParsers {

//---------------------------------------------------------------------------------------------------
// Generic option validation
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses a numeric CLI option into @p out, rejecting a non-numeric or out-of-range token
 *        with a warning that names it. Returns true only when @p out was assigned, so the caller
 *        touches the driver only for a value that actually validated (a mistyped 0 is never
 *        persisted the way a bare qBound would). @p lo must not exceed @p hi.
 */
bool parseIntOption(const QCommandLineParser& parser,
                    const QCommandLineOption& opt,
                    const int lo,
                    const int hi,
                    const QString& label,
                    int& out)
{
  SS_ASSERT(lo <= hi, return false);
  if (!parser.isSet(opt))
    return false;

  bool ok       = false;
  const int val = parser.value(opt).toInt(&ok);
  if (!ok || val < lo || val > hi) {
    qWarning().noquote()
      << QStringLiteral("Invalid %1 (%2-%3):").arg(label, QString::number(lo), QString::number(hi))
      << parser.value(opt);
    return false;
  }

  out = val;
  return true;
}

//---------------------------------------------------------------------------------------------------
// Modbus spec strings
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses a Modbus TCP host[:port] string.
 */
bool parseModbusTcpAddress(const QString& tcpAddress, QString& host, quint16& port)
{
  const QStringList parts = tcpAddress.split(':');
  if (parts.size() != 1 && parts.size() != 2)
    return false;

  host = parts[0];
  port = 502;
  if (parts.size() != 2)
    return true;

  bool ok         = false;
  const quint16 p = parts[1].toUInt(&ok);
  if (!ok || p == 0) {
    qWarning() << "Invalid ModBus TCP port:" << parts[1];
    return true;
  }

  port = p;
  return true;
}

/**
 * @brief Parses a `type:start:count` Modbus register spec into its three validated fields.
 */
bool parseModbusRegisterSpec(const QString& spec, quint8& type, quint16& start, quint16& count)
{
  const QStringList parts = spec.split(':');
  if (parts.size() != 3) {
    qWarning() << "Invalid register format. Expected: type:start:count";
    return false;
  }

  static const QHash<QString, quint8> kRegisterTypes = {
    { QStringLiteral("holding"), 0},
    {   QStringLiteral("input"), 1},
    {   QStringLiteral("coils"), 2},
    {QStringLiteral("discrete"), 3},
  };

  const QString typeStr = parts[0].toLower();
  const auto it         = kRegisterTypes.constFind(typeStr);
  if (it == kRegisterTypes.cend()) {
    qWarning() << "Invalid register type (holding/input/coils/discrete):" << typeStr;
    return false;
  }

  bool startOk             = false;
  bool countOk             = false;
  const quint16 startValue = parts[1].toUInt(&startOk);
  const quint16 countValue = parts[2].toUInt(&countOk);
  if (!startOk || !countOk || countValue < 1 || countValue > 125) {
    qWarning() << "Invalid register specification (start:0-65535, count:1-125):" << spec;
    return false;
  }

  type  = it.value();
  start = startValue;
  count = countValue;
  return true;
}

/**
 * @brief Maps a --modbus-parity token to its driver combo index, or -1 when unknown.
 */
int modbusParityIndex(const QString& parity)
{
  static const QHash<QString, quint8> kParity = {
    { QStringLiteral("none"), 0},
    { QStringLiteral("even"), 1},
    {  QStringLiteral("odd"), 2},
    {QStringLiteral("space"), 3},
    { QStringLiteral("mark"), 4},
  };

  const auto it = kParity.constFind(parity);
  return it == kParity.cend() ? -1 : static_cast<int>(it.value());
}

/**
 * @brief Maps a --modbus-databits token to its driver combo index, or -1 when unknown.
 */
int modbusDataBitsIndex(const QString& dataBits)
{
  static const QHash<QString, quint8> kDataBits = {
    {QStringLiteral("5"), 0},
    {QStringLiteral("6"), 1},
    {QStringLiteral("7"), 2},
    {QStringLiteral("8"), 3},
  };

  const auto it = kDataBits.constFind(dataBits);
  return it == kDataBits.cend() ? -1 : static_cast<int>(it.value());
}

/**
 * @brief Maps a --modbus-stopbits token to its driver combo index, or -1 when unknown.
 */
int modbusStopBitsIndex(const QString& stopBits)
{
  static const QHash<QString, quint8> kStopBits = {
    {  QStringLiteral("1"), 0},
    {QStringLiteral("1.5"), 1},
    {  QStringLiteral("2"), 2},
  };

  const auto it = kStopBits.constFind(stopBits);
  return it == kStopBits.cend() ? -1 : static_cast<int>(it.value());
}

}  // namespace CliSpecParsers
}  // namespace Misc
