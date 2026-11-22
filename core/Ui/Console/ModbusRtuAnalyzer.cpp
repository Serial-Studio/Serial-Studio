/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Console/ModbusRtuAnalyzer.h"

#include <array>
#include <bit>
#include <QStringList>

#include "Core/Checksum.h"
#include "Core/SSAssert.h"

using Console::BusFrame;
using Console::BusRole;

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr quint8 kExceptionBit     = 0x80;
static constexpr int kMinFrameBytes       = 4;
static constexpr int kMaxSummaryRegisters = 32;
static constexpr int kMaxSummaryBits      = 64;
static constexpr quint16 kErrorFindings =
  Console::FindingCrcFailed | Console::FindingIncomplete | Console::FindingNoResponse
  | Console::FindingMismatch | Console::FindingEchoDiffers | Console::FindingByteCountWrong
  | Console::FindingUnknownException | Console::FindingInvalidRequest;
static constexpr quint16 kWarningFindings =
  Console::FindingLateResponse | Console::FindingOrphanResponse | Console::FindingException
  | Console::FindingUnknownFunction;

//--------------------------------------------------------------------------------------------------
// Byte helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The unsigned byte at @p i, or 0 past the end of @p data.
 */
[[nodiscard]] static quint8 byteAt(const QByteArrayView data, const qsizetype i) noexcept
{
  if (i < 0 || i >= data.size())
    return 0;

  return static_cast<quint8>(data.at(i));
}

/**
 * @brief The big-endian 16-bit value at @p i, the order every Modbus address and quantity uses.
 */
[[nodiscard]] static quint16 be16(const QByteArrayView data, const qsizetype i) noexcept
{
  return static_cast<quint16>((byteAt(data, i) << 8) | byteAt(data, i + 1));
}

/**
 * @brief Whether the last two bytes of @p frame are the Modbus CRC of the bytes before them, low
 *        byte first. The registered checksum is the one implementation of the algorithm.
 */
[[nodiscard]] static bool crcValid(const QByteArrayView frame)
{
  static const IO::ChecksumFunc s_crc =
    IO::checksumFunctionMap().value(QStringLiteral("CRC-16-MODBUS"));

  SS_ASSERT(s_crc != nullptr, return false);
  if (frame.size() < kMinFrameBytes || frame.size() > Console::kModbusMaxFrameBytes)
    return false;

  const auto crc = s_crc(frame.data(), static_cast<int>(frame.size() - 2));
  return crc.size() == 2 && crc.at(0) == frame.at(frame.size() - 2)
      && crc.at(1) == frame.at(frame.size() - 1);
}

/**
 * @brief Whether @p function (exception bit cleared) is one whose frame lengths are known.
 */
[[nodiscard]] static bool knownFunction(const quint8 function) noexcept
{
  static constexpr std::array<quint8, 14> kKnown = {1, 2, 3, 4, 5, 6, 7, 8, 11, 15, 16, 17, 22, 23};
  for (const quint8 known : kKnown)
    if (known == function)
      return true;

  return false;
}

/**
 * @brief The bytes of @p frame from @p from up to its CRC; empty when the frame is too short to
 *        hold any, so a summary never slices past a short frame.
 */
[[nodiscard]] static QByteArrayView dataArea(const QByteArrayView frame,
                                             const qsizetype from) noexcept
{
  const qsizetype end = frame.size() - 2;
  if (from < 0 || from >= end)
    return {};

  return frame.sliced(from, end - from);
}

/**
 * @brief Bytes a coil or discrete-input quantity packs into.
 */
[[nodiscard]] static int coilBytes(const int quantity) noexcept
{
  return (quantity + 7) / 8;
}

//--------------------------------------------------------------------------------------------------
// Severity and text
//--------------------------------------------------------------------------------------------------

/**
 * @brief The worst level among @p findings.
 */
Console::BusSeverity Console::busSeverity(const quint16 findings) noexcept
{
  if ((findings & kErrorFindings) != 0)
    return BusSeverity::Error;

  if ((findings & kWarningFindings) != 0)
    return BusSeverity::Warning;

  return findings != 0 ? BusSeverity::Info : BusSeverity::None;
}

/**
 * @brief Every finding on @p frame as one line, most serious first, separated by semicolons.
 */
QString Console::busFindingsText(const quint16 findings, const BusFrame& frame)
{
  if (findings == 0)
    return {};

  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  const struct {
    quint16 flag;
    QString text;
  } kTexts[] = {
    {FindingCrcFailed, tr("CRC check failed")},
    {FindingIncomplete, tr("Incomplete frame")},
    {FindingNoResponse,
     tr("Slave %1 did not respond").arg(static_cast<int>(byteAt(frame.bytes, 0)))},
    {FindingMismatch, tr("Response does not match the request")},
    {FindingEchoDiffers, tr("Write response differs from the request")},
    {FindingByteCountWrong, tr("Byte count does not match the quantity")},
    {FindingUnknownException, tr("Undefined exception code")},
    {FindingInvalidRequest, tr("Request parameters are out of range")},
    {FindingLateResponse, tr("Late response")},
    {FindingOrphanResponse, tr("Response without a request")},
    {FindingException, tr("Slave reported an exception")},
    {FindingUnknownFunction, tr("Unknown or custom function code")},
    {FindingBroadcastWrite, tr("Broadcast write, no response expected")},
  };

  QStringList parts;
  for (const auto& entry : kTexts)
    if ((findings & entry.flag) != 0)
      parts.append(entry.text);

  return parts.join(QStringLiteral("; "));
}

/**
 * @brief The standard name of a function code; an exception reply names the function it refuses.
 */
QString Console::modbusFunctionName(const quint8 function)
{
  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  if ((function & kExceptionBit) != 0)
    return tr("%1 (exception)").arg(modbusFunctionName(function & 0x7F));

  switch (function) {
    case 1:
      return tr("Read Coils");
    case 2:
      return tr("Read Discrete Inputs");
    case 3:
      return tr("Read Holding Registers");
    case 4:
      return tr("Read Input Registers");
    case 5:
      return tr("Write Single Coil");
    case 6:
      return tr("Write Single Register");
    case 7:
      return tr("Read Exception Status");
    case 8:
      return tr("Diagnostics");
    case 11:
      return tr("Get Comm Event Counter");
    case 15:
      return tr("Write Multiple Coils");
    case 16:
      return tr("Write Multiple Registers");
    case 17:
      return tr("Report Server ID");
    case 22:
      return tr("Mask Write Register");
    case 23:
      return tr("Read/Write Multiple Registers");
    default:
      return tr("Function %1").arg(static_cast<int>(function));
  }
}

//--------------------------------------------------------------------------------------------------
// Data summary
//--------------------------------------------------------------------------------------------------

/**
 * @brief The standard name of an exception code, empty for an undefined one.
 */
[[nodiscard]] static QString exceptionName(const quint8 code)
{
  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  switch (code) {
    case 1:
      return tr("Illegal Function");
    case 2:
      return tr("Illegal Data Address");
    case 3:
      return tr("Illegal Data Value");
    case 4:
      return tr("Server Device Failure");
    case 5:
      return tr("Acknowledge");
    case 6:
      return tr("Server Device Busy");
    case 8:
      return tr("Memory Parity Error");
    case 10:
      return tr("Gateway Path Unavailable");
    case 11:
      return tr("Gateway Target Device Failed to Respond");
    default:
      return {};
  }
}

/**
 * @brief Hexadecimal bytes separated by spaces.
 */
[[nodiscard]] static QString hexText(const QByteArrayView data)
{
  return QString::fromLatin1(data.toByteArray().toHex(' ')).toUpper();
}

/**
 * @brief The first kMaxSummaryBits bits of @p data, lowest bit of each byte first.
 */
[[nodiscard]] static QString bitsText(const QByteArrayView data)
{
  QStringList bits;
  const int count = qMin(static_cast<int>(data.size()) * 8, kMaxSummaryBits);
  for (int i = 0; i < count; ++i)
    bits.append(((byteAt(data, i / 8) >> (i % 8)) & 1) != 0 ? QStringLiteral("1")
                                                            : QStringLiteral("0"));

  return QStringLiteral("[%1]").arg(bits.join(QStringLiteral(", ")));
}

/**
 * @brief One format's rendering of the registers in @p data: single registers for the 16-bit
 *        formats, pairs (high word first) for the 32-bit ones, so an odd last register is unused.
 */
[[nodiscard]] static QString registersAs(const QByteArrayView data, const quint8 format)
{
  const int registers = qMin(static_cast<int>(data.size() / 2), kMaxSummaryRegisters);
  const bool wide =
    format == Console::FormatU32 || format == Console::FormatS32 || format == Console::FormatF32;

  QStringList values;
  for (int i = 0; i + (wide ? 1 : 0) < registers; i += wide ? 2 : 1) {
    const quint16 high = be16(data, i * 2);
    const quint32 pair = (static_cast<quint32>(high) << 16) | be16(data, i * 2 + 2);
    switch (format) {
      case Console::FormatS16:
        values.append(QString::number(static_cast<qint16>(high)));
        break;
      case Console::FormatU32:
        values.append(QString::number(pair));
        break;
      case Console::FormatS32:
        values.append(QString::number(static_cast<qint32>(pair)));
        break;
      case Console::FormatF32:
        values.append(QString::number(static_cast<double>(std::bit_cast<float>(pair)), 'g', 6));
        break;
      case Console::FormatBin:
        values.append(QString::number(high, 2).rightJustified(16, QLatin1Char('0')));
        break;
      default:
        values.append(QString::number(high));
        break;
    }
  }

  return QStringLiteral("[%1]").arg(values.join(QStringLiteral(", ")));
}

/**
 * @brief The registers in @p data in every ticked format; hexadecimal when none is ticked. A
 *        single register has no 32-bit reading, so those formats are skipped for it.
 */
[[nodiscard]] static QString registersText(const QByteArrayView data, const quint8 formats)
{
  const struct {
    quint8 flag;
    const char* name;
  } kFormats[] = {
    {Console::FormatU16, "U16"},
    {Console::FormatS16, "S16"},
    {Console::FormatU32, "U32"},
    {Console::FormatS32, "S32"},
    {Console::FormatF32, "F32"},
    {Console::FormatBin, "BIN"},
  };

  const quint8 wide   = Console::FormatU32 | Console::FormatS32 | Console::FormatF32;
  const quint8 usable = data.size() < 4 ? formats & static_cast<quint8>(~wide) : formats;

  QStringList parts;
  for (const auto& entry : kFormats)
    if ((usable & entry.flag) != 0)
      parts.append(
        QStringLiteral("%1: %2").arg(QLatin1String(entry.name), registersAs(data, entry.flag)));

  return parts.isEmpty() ? hexText(data) : parts.join(QStringLiteral("; "));
}

/**
 * @brief The summary of a master request: what it reads or writes.
 */
[[nodiscard]] static QString requestSummary(const QByteArrayView frame, const quint8 formats)
{
  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  const quint8 function = byteAt(frame, 1);
  const auto address    = QStringLiteral("0x%1").arg(be16(frame, 2), 4, 16, QLatin1Char('0'));
  const auto payload    = dataArea(frame, 7);
  switch (function) {
    case 1:
    case 2:
    case 3:
    case 4:
      return tr("Start address %1, Quantity %2").arg(address).arg(be16(frame, 4));
    case 5:
      if (be16(frame, 4) == 0xFF00)
        return tr("Address %1, ON(0xFF00)").arg(address);

      if (be16(frame, 4) == 0x0000)
        return tr("Address %1, OFF(0x0000)").arg(address);

      return tr("Address %1, 0x%2").arg(address).arg(be16(frame, 4), 4, 16, QLatin1Char('0'));
    case 6:
      return tr("Address %1, %2").arg(address, registersText(dataArea(frame, 4), formats));
    case 15:
      return tr("Start address %1, Quantity %2, Bits: %3")
        .arg(address)
        .arg(be16(frame, 4))
        .arg(bitsText(payload));
    case 16:
      return tr("Start address %1, Quantity %2, %3")
        .arg(address)
        .arg(be16(frame, 4))
        .arg(registersText(payload, formats));
    default:
      return hexText(dataArea(frame, 2));
  }
}

/**
 * @brief The summary of a slave response: the values it carries, or what it confirms.
 */
[[nodiscard]] static QString responseSummary(const QByteArrayView frame, const quint8 formats)
{
  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  const quint8 function = byteAt(frame, 1);
  const auto counted    = dataArea(frame, 3);
  switch (function) {
    case 1:
    case 2:
      return tr("Bits: %1").arg(bitsText(counted));
    case 3:
    case 4:
      return registersText(counted, formats);
    case 5:
    case 6:
      return requestSummary(frame, formats);
    case 15:
    case 16:
      return tr("Start address 0x%1, Quantity %2")
        .arg(be16(frame, 2), 4, 16, QLatin1Char('0'))
        .arg(be16(frame, 4));
    default:
      return hexText(dataArea(frame, 2));
  }
}

/**
 * @brief What @p frame says, for the monitor's Data Summary column. Bytes that never passed the
 *        CRC are only counted; an exception names its code.
 */
QString Console::modbusDataSummary(const BusFrame& frame, const quint8 formats)
{
  const auto tr = [](const char* text) {
    return QCoreApplication::translate("Console::ModbusRtuAnalyzer", text);
  };

  const QByteArrayView bytes(frame.bytes);
  if (!frame.crcOk || bytes.size() < kMinFrameBytes)
    return tr("Unparsed data (%1 bytes)").arg(bytes.size());

  if ((byteAt(bytes, 1) & kExceptionBit) != 0) {
    const quint8 code = byteAt(bytes, 2);
    const auto name   = exceptionName(code);
    const auto hex =
      QStringLiteral("%1").arg(static_cast<int>(code), 2, 16, QLatin1Char('0')).toUpper();
    return name.isEmpty() ? tr("Exception code 0x%1").arg(hex)
                          : tr("Exception code 0x%1 (%2)").arg(hex, name);
  }

  return frame.role == BusRole::Slave ? responseSummary(bytes, formats)
                                      : requestSummary(bytes, formats);
}

//--------------------------------------------------------------------------------------------------
// Analyzer state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts with nothing buffered and the default response timeout.
 */
Console::ModbusRtuAnalyzer::ModbusRtuAnalyzer()
  : m_nextSequence(1)
  , m_lastRxMs(0)
  , m_expiredAtMs(0)
  , m_timeoutMs(kModbusTimeoutDefaultMs)
  , m_expiredSlave(0)
  , m_expiredFunction(0)
  , m_hasExpired(false)
{
  m_carry.reserve(kModbusMaxCarryBytes);
  m_garbage.reserve(kModbusMaxFrameBytes);
}

/**
 * @brief How long a request waits for its response before it is marked unanswered.
 */
int Console::ModbusRtuAnalyzer::timeoutMs() const noexcept
{
  return m_timeoutMs;
}

/**
 * @brief Whether a request is waiting to see its response.
 */
bool Console::ModbusRtuAnalyzer::awaitingResponse() const noexcept
{
  return m_pending.active;
}

/**
 * @brief Bytes received that are not part of a reported frame yet.
 */
qsizetype Console::ModbusRtuAnalyzer::bufferedBytes() const noexcept
{
  return m_carry.size() + m_garbage.size();
}

/**
 * @brief Drops buffered bytes and the pairing state; sequence numbers keep counting so rows
 *        already shown are never referred to by a later update.
 */
void Console::ModbusRtuAnalyzer::reset()
{
  m_carry.clear();
  m_garbage.clear();
  m_pending    = Pending();
  m_hasExpired = false;
}

/**
 * @brief Sets the response timeout, clamped; a request already waiting uses the new value too.
 */
void Console::ModbusRtuAnalyzer::setTimeoutMs(const int timeoutMs)
{
  m_timeoutMs = qBound(kModbusTimeoutMinMs, timeoutMs, kModbusTimeoutMaxMs);
}

//--------------------------------------------------------------------------------------------------
// Framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a byte count is one a response from @p slave to @p function could carry. With a
 *        request waiting, only its own slave, function and the count it asked for qualify. This
 *        is what keeps the bytes of a corrupt frame from being taken for the start of a long
 *        response: without it the scan waits for a length that never arrives while good frames
 *        queue up behind (measured on the premise trace: 72 of 38,758 frames lost).
 */
bool Console::ModbusRtuAnalyzer::countPlausible(const quint8 slave,
                                                const quint8 function,
                                                const int counted) const noexcept
{
  SS_ASSERT(counted >= 0, return false);
  if (counted < 1 || counted > 250)
    return false;

  if (!m_pending.active)
    return (function == 3 || function == 4 || function == 23) ? (counted & 1) == 0 : true;

  if (slave != m_pending.slave || function != m_pending.function)
    return false;

  const int quantity = be16(m_pending.bytes, 4);
  if (function == 1 || function == 2)
    return counted == coilBytes(quantity);

  if (function == 3 || function == 4)
    return counted == quantity * 2;

  return true;
}

/**
 * @brief Whether the quantity and byte count of a write-multiple request agree, which they must
 *        before its declared length is worth waiting for.
 */
[[nodiscard]] static bool writePlausible(const quint8 function,
                                         const int quantity,
                                         const int counted) noexcept
{
  if (function == 15)
    return quantity >= 1 && quantity <= 1968 && counted == coilBytes(quantity);

  return quantity >= 1 && quantity <= 123 && counted == quantity * 2;
}

/**
 * @brief One length a frame could have, and whether it is worth waiting for when the bytes are
 *        not all there yet. Plausibility only decides the waiting: a candidate whose bytes have
 *        all arrived is always put to the CRC, so an unexpected but intact frame is still
 *        reported (and then judged as a mismatch), never shredded.
 */
struct Candidate {
  qsizetype length;
  bool waitable;
};

/**
 * @brief The length of the CRC-verified frame that starts at @p at in the carry, or -1. A
 *        function code allows a request length and a response length; each is tried, so the CRC
 *        decides which it is. @p needMore reports that a candidate worth waiting for did not fit
 *        the bytes received so far; in a @p final pass no more bytes are coming.
 */
qsizetype Console::ModbusRtuAnalyzer::frameLengthAt(const qsizetype at,
                                                    const bool final,
                                                    bool& needMore) const
{
  SS_ASSERT(at >= 0 && at < m_carry.size(), return -1);

  needMore = false;
  const QByteArrayView view(m_carry);
  const qsizetype avail = view.size() - at;
  const quint8 slave    = byteAt(view, at);
  const quint8 function = byteAt(view, at + 1);
  const quint8 counted  = byteAt(view, at + 2);
  const quint8 written  = byteAt(view, at + 6);
  const quint8 both     = byteAt(view, at + 10);

  Candidate byCount = {0, true};
  if (avail >= 3)
    byCount = {5 + counted, countPlausible(slave, function, counted)};

  Candidate byWrite = {0, true};
  if (avail >= 7)
    byWrite = {9 + written, writePlausible(function, be16(view, at + 4), written)};

  Candidate byBoth = {0, true};
  if (avail >= 11)
    byBoth = {13 + both, both >= 2 && both == be16(view, at + 8) * 2};

  const Candidate none             = {-1, false};
  std::array<Candidate, 2> lengths = {none, none};
  switch ((function & kExceptionBit) != 0 ? kExceptionBit : function) {
    case kExceptionBit:
      lengths = {
        Candidate{5, true},
        none
      };
      break;
    case 1:
    case 2:
    case 3:
    case 4:
      lengths = {
        Candidate{8, true},
        byCount
      };
      break;
    case 5:
    case 6:
    case 8:
      lengths = {
        Candidate{8, true},
        none
      };
      break;
    case 7:
      lengths = {
        Candidate{4, true},
        Candidate{5, true}
      };
      break;
    case 11:
      lengths = {
        Candidate{4, true},
        Candidate{8, true}
      };
      break;
    case 15:
    case 16:
      lengths = {
        Candidate{8, true},
        byWrite
      };
      break;
    case 17:
      lengths = {
        Candidate{4, true},
        byCount
      };
      break;
    case 22:
      lengths = {
        Candidate{10, true},
        none
      };
      break;
    case 23:
      lengths = {byBoth, byCount};
      break;
    default:
      break;
  }

  for (const auto& candidate : lengths) {
    if (candidate.length < 0)
      continue;

    if (candidate.length == 0 || candidate.length > avail) {
      needMore = needMore || (candidate.waitable && !final);
      continue;
    }

    if (crcValid(view.sliced(at, candidate.length)))
      return candidate.length;
  }

  return -1;
}

/**
 * @brief Walks the carry and reports every frame it holds. Bytes that start no verified frame
 *        are skipped one at a time and collected, to be reported as one failed row when a good
 *        frame follows them. An unknown function code has no known length, so its frame is
 *        accepted only when everything from its first byte on verifies as one frame. A normal
 *        pass stops where a frame may still be arriving; a @p final pass runs to the end.
 */
void Console::ModbusRtuAnalyzer::scan(const qint64 nowMs,
                                      const bool final,
                                      std::vector<BusFrame>& frames,
                                      std::vector<BusUpdate>& updates)
{
  SS_ASSERT(m_carry.size() <= kModbusMaxCarryBytes, m_carry.clear());

  qsizetype at = 0;
  for (qsizetype step = 0; step <= kModbusMaxCarryBytes; ++step) {
    if (m_carry.size() - at < kMinFrameBytes)
      break;

    bool needMore          = false;
    const qsizetype avail  = m_carry.size() - at;
    const qsizetype length = frameLengthAt(at, final, needMore);
    const bool known       = knownFunction(byteAt(m_carry, at + 1) & 0x7F);
    const auto rest        = QByteArrayView(m_carry).sliced(at);
    const bool whole       = length < 0 && !known && crcValid(rest);
    if (length > 0 || whole) {
      const qsizetype taken = whole ? avail : length;
      emitGarbage(nowMs, false, frames);
      emitFrame(rest.first(taken), nowMs, frames, updates);
      at += taken;
      continue;
    }

    const bool wait = (needMore && avail < kModbusMaxFrameBytes) || (!known && avail < 8);
    if (wait && !final)
      break;

    m_garbage.append(m_carry.at(at));
    ++at;
    if (m_garbage.size() >= kModbusMaxFrameBytes)
      emitGarbage(nowMs, false, frames);
  }

  m_carry.remove(0, at);
}

/**
 * @brief Takes received bytes at @p nowMs and reports every frame they complete. A chunk larger
 *        than the carry is taken in slices with a scan between them, so the cap bounds memory
 *        without dropping input: a scan always leaves less than one frame behind.
 */
void Console::ModbusRtuAnalyzer::feed(const QByteArrayView bytes,
                                      const qint64 nowMs,
                                      std::vector<BusFrame>& frames,
                                      std::vector<BusUpdate>& updates)
{
  SS_ASSERT(m_carry.size() <= kModbusMaxCarryBytes, m_carry.clear());
  if (bytes.isEmpty())
    return;

  m_lastRxMs = nowMs;
  for (qsizetype at = 0; at < bytes.size();) {
    const qsizetype room = kModbusMaxCarryBytes - m_carry.size();
    const qsizetype take = qMin(room, bytes.size() - at);
    if (take <= 0)
      break;

    m_carry.append(bytes.data() + at, take);
    at += take;
    scan(nowMs, false, frames, updates);
  }
}

/**
 * @brief Lets time pass without bytes: a request older than the timeout is marked unanswered on
 *        its own row. Once the line has been idle for the flush limit nothing more will join
 *        what is buffered, so a final pass takes out every frame still in it and whatever is
 *        left is reported as one failed row.
 */
void Console::ModbusRtuAnalyzer::tick(const qint64 nowMs,
                                      std::vector<BusFrame>& frames,
                                      std::vector<BusUpdate>& updates)
{
  SS_ASSERT(m_timeoutMs >= kModbusTimeoutMinMs, m_timeoutMs = kModbusTimeoutDefaultMs);
  if (m_pending.active && nowMs - m_pending.timeMs >= m_timeoutMs) {
    m_expiredAtMs = nowMs;
    expirePending(updates, FindingNoResponse);
  }

  if (bufferedBytes() == 0 || nowMs - m_lastRxMs < kModbusIdleFlushMs)
    return;

  scan(nowMs, true, frames, updates);
  const bool incomplete = !m_carry.isEmpty();
  m_garbage.append(m_carry.constData(),
                   qMin<qsizetype>(m_carry.size(), kModbusMaxCarryBytes - m_garbage.size()));
  m_carry.clear();
  emitGarbage(nowMs, incomplete, frames);
}

//--------------------------------------------------------------------------------------------------
// Roles and pairing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether @p frame is a master request or a slave response. Most functions give it away by
 *        length; where a request and a response are the same size, a waiting request for the
 *        same slave and function makes the frame its response.
 */
BusRole Console::ModbusRtuAnalyzer::roleOf(const QByteArrayView frame) const noexcept
{
  SS_ASSERT(frame.size() >= kMinFrameBytes, return BusRole::Unknown);

  const quint8 raw      = byteAt(frame, 1);
  const quint8 function = raw & 0x7F;
  const bool expected =
    m_pending.active && m_pending.slave == byteAt(frame, 0) && m_pending.function == function;
  if ((raw & kExceptionBit) != 0)
    return BusRole::Slave;

  switch (function) {
    case 1:
    case 2:
      if (frame.size() != 8)
        return BusRole::Slave;

      return expected && byteAt(frame, 2) == 3 ? BusRole::Slave : BusRole::Master;
    case 3:
    case 4:
      return frame.size() == 8 ? BusRole::Master : BusRole::Slave;
    case 15:
    case 16:
      return frame.size() == 8 ? BusRole::Slave : BusRole::Master;
    case 7:
    case 11:
    case 17:
      return frame.size() == 4 ? BusRole::Master : BusRole::Slave;
    default:
      return expected ? BusRole::Slave : BusRole::Master;
  }
}

/**
 * @brief Findings on a master request taken alone: an address no slave can have, a read sent to
 *        the broadcast address, or a quantity, value or byte count outside the protocol's limits.
 */
quint16 Console::ModbusRtuAnalyzer::judgeRequest(const QByteArrayView frame) const noexcept
{
  SS_ASSERT(frame.size() >= kMinFrameBytes, return FindingInvalidRequest);

  const quint8 slave    = byteAt(frame, 0);
  const quint8 function = byteAt(frame, 1);
  const int quantity    = be16(frame, 4);
  const bool write      = function == 5 || function == 6 || function == 15 || function == 16;
  if (slave > 247 || (slave == 0 && !write))
    return FindingInvalidRequest;

  bool valid = true;
  switch (function) {
    case 1:
    case 2:
      valid = quantity >= 1 && quantity <= 2000;
      break;
    case 3:
    case 4:
      valid = quantity >= 1 && quantity <= 125;
      break;
    case 5:
      valid = quantity == 0xFF00 || quantity == 0x0000;
      break;
    case 15:
      valid = quantity >= 1 && quantity <= 1968 && byteAt(frame, 6) == coilBytes(quantity);
      break;
    case 16:
      valid = quantity >= 1 && quantity <= 123 && byteAt(frame, 6) == quantity * 2;
      break;
    default:
      break;
  }

  const quint16 broadcast = slave == 0 ? FindingBroadcastWrite : FindingNone;
  return valid ? broadcast : static_cast<quint16>(broadcast | FindingInvalidRequest);
}

/**
 * @brief Findings on a slave response against the request that is waiting for it: the wrong
 *        slave or function, an exception, an echo that changed, or a byte count that disagrees
 *        with what was asked.
 */
quint16 Console::ModbusRtuAnalyzer::judgeResponse(const QByteArrayView frame) const noexcept
{
  SS_ASSERT(m_pending.active, return FindingOrphanResponse);
  SS_ASSERT(frame.size() >= kMinFrameBytes, return FindingMismatch);

  const quint8 raw      = byteAt(frame, 1);
  const quint8 function = raw & 0x7F;
  const QByteArrayView request(m_pending.bytes);
  if (byteAt(frame, 0) == 0 || byteAt(frame, 0) != m_pending.slave
      || function != m_pending.function)
    return FindingMismatch;

  if ((raw & kExceptionBit) != 0) {
    const quint8 code  = byteAt(frame, 2);
    const bool defined = (code >= 1 && code <= 6) || code == 8 || code == 10 || code == 11;
    return defined ? FindingException : FindingUnknownException;
  }

  const int quantity = be16(request, 4);
  const int counted  = byteAt(frame, 2);
  switch (function) {
    case 1:
    case 2:
      return counted == coilBytes(quantity) ? FindingNone : FindingByteCountWrong;
    case 3:
    case 4:
      return counted == quantity * 2 ? FindingNone : FindingByteCountWrong;
    case 5:
    case 6:
      return frame == request ? FindingNone : FindingEchoDiffers;
    case 15:
    case 16:
      return frame.sliced(2, 4) == request.sliced(2, 4) ? FindingNone : FindingEchoDiffers;
    default:
      return FindingNone;
  }
}

/**
 * @brief Reports one verified frame with its role and findings and updates the pairing: a new
 *        request closes the one still waiting as unanswered and waits in its place, unless it is
 *        a broadcast; a response closes the waiting request, or is late or orphaned without one.
 */
void Console::ModbusRtuAnalyzer::emitFrame(const QByteArrayView frame,
                                           const qint64 nowMs,
                                           std::vector<BusFrame>& frames,
                                           std::vector<BusUpdate>& updates)
{
  SS_ASSERT(frame.size() >= kMinFrameBytes, return);
  SS_ASSERT(frame.size() <= kModbusMaxFrameBytes, return);

  const quint8 slave    = byteAt(frame, 0);
  const quint8 function = byteAt(frame, 1) & 0x7F;
  const BusRole role    = roleOf(frame);
  quint16 findings      = knownFunction(function) ? FindingNone : FindingUnknownFunction;
  if (role == BusRole::Master) {
    if (m_pending.active) {
      m_expiredAtMs = nowMs;
      expirePending(updates, FindingNoResponse);
    }

    findings |= judgeRequest(frame);
  } else if (m_pending.active) {
    findings         |= judgeResponse(frame);
    m_pending.active  = false;
  } else {
    const bool late  = m_hasExpired && m_expiredSlave == slave && m_expiredFunction == function
                    && nowMs - m_expiredAtMs <= kModbusLateWindowMs;
    findings        |= late ? FindingLateResponse : FindingOrphanResponse;
    m_hasExpired     = false;
  }

  BusFrame out;
  out.sequence = m_nextSequence++;
  out.timeMs   = nowMs;
  out.findings = findings;
  out.role     = role;
  out.crcOk    = true;
  out.bytes    = frame.toByteArray();
  frames.push_back(out);
  if (role != BusRole::Master || slave == 0)
    return;

  m_pending.sequence = out.sequence;
  m_pending.timeMs   = nowMs;
  m_pending.slave    = slave;
  m_pending.function = function;
  m_pending.bytes    = out.bytes;
  m_pending.active   = true;
}

/**
 * @brief Reports the skipped bytes as one failed row and forgets them. When they begin with the
 *        address of the slave a request is waiting on, they were its response arriving damaged:
 *        the wait ends here instead of being reported a second time as a timeout.
 */
void Console::ModbusRtuAnalyzer::emitGarbage(const qint64 nowMs,
                                             const bool incomplete,
                                             std::vector<BusFrame>& frames)
{
  SS_ASSERT(m_garbage.size() <= kModbusMaxCarryBytes + kModbusMaxFrameBytes, m_garbage.clear());
  if (m_garbage.isEmpty())
    return;

  const bool damagedReply = m_garbage.size() >= kMinFrameBytes;
  if (m_pending.active && damagedReply && byteAt(m_garbage, 0) == m_pending.slave)
    m_pending.active = false;

  BusFrame out;
  out.sequence = m_nextSequence++;
  out.timeMs   = nowMs;
  out.findings = incomplete ? FindingCrcFailed | FindingIncomplete : FindingCrcFailed;
  out.role     = BusRole::Unknown;
  out.crcOk    = false;
  out.bytes    = QByteArray(m_garbage.constData(), m_garbage.size());
  frames.push_back(out);
  m_garbage.resize(0);
}

/**
 * @brief Closes the waiting request with @p finding on its own row, and remembers which slave
 *        and function it was so a response that still turns up reads as late, not as orphaned.
 */
void Console::ModbusRtuAnalyzer::expirePending(std::vector<BusUpdate>& updates,
                                               const quint16 finding)
{
  SS_ASSERT(m_pending.active, return);

  updates.push_back({m_pending.sequence, finding});
  m_expiredSlave    = m_pending.slave;
  m_expiredFunction = m_pending.function;
  m_hasExpired      = true;
  m_pending.active  = false;
}
