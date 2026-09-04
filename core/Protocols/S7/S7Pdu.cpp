/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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

#include "Protocols/S7/S7Pdu.h"

#include <cstring>
#include <QCoreApplication>

#include "Core/SSAssert.h"

static constexpr int kS7BitsPerByte = 8;

/**
 * @brief Returns the translated string for the S7comm protocol context.
 */
[[nodiscard]] static QString trPdu(const char* text)
{
  return QCoreApplication::translate("S7Comm", text);
}

/**
 * @brief Reads one octet as an unsigned byte; the caller has already bounds-checked the position.
 */
[[nodiscard]] static inline std::uint8_t s7PduOctet(QByteArrayView view, qsizetype pos) noexcept
{
  SS_ASSERT(pos >= 0 && pos < view.size(), return 0);

  return static_cast<std::uint8_t>(view[pos]);
}

/**
 * @brief Reads a big-endian 16-bit word, the byte order every S7 length and address field uses.
 */
[[nodiscard]] static quint16 readBe16(QByteArrayView view, qsizetype pos) noexcept
{
  SS_ASSERT_LOG(pos >= 0);

  return static_cast<quint16>((static_cast<int>(s7PduOctet(view, pos)) << 8)
                              | s7PduOctet(view, pos + 1));
}

/**
 * @brief Appends a big-endian 16-bit word.
 */
static void appendBe16(QByteArray& out, quint16 value)
{
  SS_ASSERT_LOG(out.size() < IO::Drivers::S7Comm::kMaxPduBytes);

  out.append(static_cast<char>((value >> 8) & 0xFF));
  out.append(static_cast<char>(value & 0xFF));
}

/**
 * @brief Appends the ten-octet job header. Jobs carry no error pair; only an acknowledgement does,
 *        which is the one asymmetry a reader has to honour before it can find the parameters.
 */
static void appendJobHeader(QByteArray& out, quint16 reference, int parameterBytes, int dataBytes)
{
  using namespace IO::Drivers::S7Comm;
  SS_ASSERT_LOG(parameterBytes >= 0 && parameterBytes <= kMaxPduBytes);
  SS_ASSERT_LOG(dataBytes >= 0 && dataBytes <= kMaxPduBytes);

  out.append(static_cast<char>(kProtocolId));
  out.append(static_cast<char>(kRosctrJob));
  appendBe16(out, 0);
  appendBe16(out, reference);
  appendBe16(out, static_cast<quint16>(parameterBytes));
  appendBe16(out, static_cast<quint16>(dataBytes));
}

/**
 * @brief Appends one twelve-octet request item. The start address is a BIT address, which is why a
 *        byte-oriented read still multiplies its offset by eight before it goes on the wire.
 */
static void appendRequestItem(QByteArray& out, const IO::Drivers::S7Comm::ReadItem& item)
{
  using namespace IO::Drivers::S7Comm;
  SS_ASSERT_LOG(item.elementCount > 0);
  SS_ASSERT_LOG(item.area != 0);

  out.append(static_cast<char>(kSpecificationAny));
  out.append(static_cast<char>(kAnyLengthBytes));
  out.append(static_cast<char>(kSyntaxAny));
  out.append(static_cast<char>(item.bitAccess ? kTransportBit : kTransportByte));
  appendBe16(out, item.elementCount);
  appendBe16(out, item.dbNumber);
  out.append(static_cast<char>(item.area));
  out.append(static_cast<char>((item.startAddress >> 16) & 0xFF));
  out.append(static_cast<char>((item.startAddress >> 8) & 0xFF));
  out.append(static_cast<char>(item.startAddress & 0xFF));
}

/**
 * @brief Walks one response item, advancing @p pos past its header, its payload and the fill octet
 *        an odd payload carries when another item follows. Returns false the moment the declared
 *        length would address bytes the controller did not send.
 */
[[nodiscard]] static bool readResultItem(QByteArrayView pdu,
                                         qsizetype& pos,
                                         bool more,
                                         IO::Drivers::S7Comm::ReadResult& out)
{
  using namespace IO::Drivers::S7Comm;
  SS_ASSERT_LOG(pos >= 0);

  if (pos < 0 || pos + kResultItemBytes > pdu.size())
    return false;

  const auto transport = s7PduOctet(pdu, pos + 1);
  const int declared   = readBe16(pdu, pos + 2);
  const bool counted   = transport == kResultBit || transport == kResultBits;
  const int bytes      = counted ? (declared + kS7BitsPerByte - 1) / kS7BitsPerByte : declared;

  out             = ReadResult{};
  out.returnCode  = s7PduOctet(pdu, pos);
  pos            += kResultItemBytes;
  if (pos + bytes > pdu.size())
    return false;

  out.offset  = pos;
  out.size    = bytes;
  pos        += bytes;
  if (more && (bytes % 2) != 0 && pos < pdu.size())
    ++pos;

  SS_ASSERT_LOG(out.size >= 0);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Vocabulary
//--------------------------------------------------------------------------------------------------

/**
 * @brief Turns a parsed absolute address into the item the read service carries. A bit address
 *        reads exactly one bit, so the containing byte is never fetched and never masked here.
 */
IO::Drivers::S7Comm::ReadItem IO::Drivers::S7Comm::itemForAddress(
  const S7Address::Address& address) noexcept
{
  SS_ASSERT_LOG(address.size > 0);
  SS_ASSERT_LOG(address.byteOffset >= 0);

  ReadItem item;
  item.area         = static_cast<std::uint8_t>(address.area);
  item.bitAccess    = address.type == S7Address::Type::Bool;
  item.dbNumber     = static_cast<quint16>(qBound(0, address.dbNumber, S7Address::kMaxDbNumber));
  item.elementCount = item.bitAccess ? 1 : static_cast<quint16>(qMax(1, address.size));
  item.startAddress = static_cast<quint32>(qMax(0, address.byteOffset)) * kS7BitsPerByte
                    + static_cast<quint32>(qMax(0, address.bitOffset));
  return item;
}

/**
 * @brief Returns how many payload octets one item's answer occupies, which is what the response
 *        half of the message budget is spent on.
 */
int IO::Drivers::S7Comm::resultBytes(const ReadItem& item) noexcept
{
  SS_ASSERT_LOG(item.elementCount > 0);
  SS_ASSERT_LOG(item.area != 0);

  return item.bitAccess ? 1 : static_cast<int>(item.elementCount);
}

/**
 * @brief Renders the header error pair an acknowledgement carries when the whole job failed.
 */
QString IO::Drivers::S7Comm::errorText(std::uint8_t errorClass, std::uint8_t errorCode)
{
  SS_ASSERT_LOG(errorClass != 0 || errorCode == 0);

  switch (errorClass) {
    case 0x81:
      return trPdu("The controller refused the application relationship");
    case 0x82:
      return trPdu("The controller rejected the object definition");
    case 0x83:
      return trPdu("The controller has no resources left for this session");
    case 0x84:
      return trPdu("The controller failed while processing the service");
    case 0x85:
      return trPdu("The controller reported an error on supplies");
    case 0x87:
      return trPdu("The controller denied access; check PUT/GET and optimized block access");
    default:
      break;
  }

  return trPdu("The controller reported error %1/%2")
    .arg(QString::number(errorClass, 16), QString::number(errorCode, 16));
}

/**
 * @brief Renders one item's return code, which is what tells an engineer whether the address is
 *        wrong, the block is missing or the CPU is refusing access to it.
 */
QString IO::Drivers::S7Comm::returnCodeText(std::uint8_t returnCode)
{
  SS_ASSERT_LOG(returnCode != kReturnSuccess);

  switch (returnCode) {
    case 0x01:
      return trPdu("hardware fault");
    case 0x03:
      return trPdu("access denied");
    case 0x05:
      return trPdu("address out of range");
    case 0x06:
      return trPdu("data type not supported");
    case 0x07:
      return trPdu("data type inconsistent");
    case 0x0A:
      return trPdu("object does not exist");
    default:
      break;
  }

  return trPdu("refused (code %1)").arg(QString::number(returnCode, 16));
}

/**
 * @brief Renders the big-endian octets an area returns as the value the address declares. A bit
 *        read answers one octet whose low bit is the bit; an S7 STRING answers its two-octet
 *        maximum/current header ahead of the characters. An EMPTY payload is a controller-chosen
 *        length, so it returns invalid for the caller to count rather than asserting on wire input.
 */
QVariant IO::Drivers::S7Comm::decodeValue(const S7Address::Address& address, QByteArrayView raw)
{
  SS_ASSERT(address.size > 0, return {});

  if (raw.isEmpty())
    return {};

  if (address.type == S7Address::Type::Bool)
    return QVariant((s7PduOctet(raw, 0) & 0x01U) != 0);

  if (address.type == S7Address::Type::Str) {
    if (raw.size() < 2)
      return QVariant(QString());

    const int declared = s7PduOctet(raw, 1);
    const int length   = qBound(0, declared, static_cast<int>(raw.size()) - 2);
    return QVariant(QString::fromLatin1(raw.data() + 2, length));
  }

  quint64 bits    = 0;
  const int width = qMin(static_cast<int>(raw.size()), address.size);
  for (int i = 0; i < width; ++i)
    bits = (bits << 8) | s7PduOctet(raw, i);

  switch (address.type) {
    case S7Address::Type::Byte:
    case S7Address::Type::Word:
      return QVariant(static_cast<uint>(bits));
    case S7Address::Type::DWord:
      return QVariant(static_cast<qulonglong>(bits));
    case S7Address::Type::Int:
      return QVariant(static_cast<int>(static_cast<qint16>(bits)));
    case S7Address::Type::DInt:
      return QVariant(static_cast<int>(static_cast<qint32>(bits)));
    case S7Address::Type::Real:
      break;
    case S7Address::Type::Bool:
    case S7Address::Type::Str:
    case S7Address::Type::Invalid:
      return {};
  }

  const auto word = static_cast<quint32>(bits);
  float value     = 0.0F;
  std::memcpy(&value, &word, sizeof(value));
  return QVariant(static_cast<double>(value));
}

//--------------------------------------------------------------------------------------------------
// Header decoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes the protocol header and locates the parameter and data sections. An
 *        acknowledgement carries two extra error octets ahead of its parameters, so the section
 *        offsets depend on the message class and cannot be a constant.
 */
IO::Drivers::S7Comm::PduResult IO::Drivers::S7Comm::parseHeader(QByteArrayView pdu,
                                                                PduHeader& out) noexcept
{
  out = PduHeader{};
  if (pdu.size() < kJobHeaderBytes)
    return PduResult::Truncated;

  if (s7PduOctet(pdu, 0) != kProtocolId)
    return PduResult::Malformed;

  out.rosctr          = s7PduOctet(pdu, 1);
  out.reference       = readBe16(pdu, 4);
  out.parameterBytes  = readBe16(pdu, 6);
  out.dataBytes       = readBe16(pdu, 8);
  out.parameterOffset = kJobHeaderBytes;

  if (out.rosctr == kRosctrAckData) {
    if (pdu.size() < kAckHeaderBytes)
      return PduResult::Truncated;

    out.errorClass      = s7PduOctet(pdu, 10);
    out.errorCode       = s7PduOctet(pdu, 11);
    out.parameterOffset = kAckHeaderBytes;
  }

  out.dataOffset = out.parameterOffset + out.parameterBytes;
  if (out.dataOffset + out.dataBytes > pdu.size())
    return PduResult::Truncated;

  SS_ASSERT_LOG(out.dataOffset >= kJobHeaderBytes);
  return PduResult::Ok;
}

//--------------------------------------------------------------------------------------------------
// Codec construction and state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a codec holding the smallest message budget the protocol defines, so a caller that
 *        plans chunks before a negotiation plans them against the pessimistic size rather than an
 *        optimistic one no controller may honour.
 */
IO::Drivers::S7Comm::PduCodec::PduCodec()
  : m_pduBytes(kMinPduBytes), m_reference(0), m_malformed(0), m_refusedItems(0), m_oversized(0)
{}

/**
 * @brief Returns the codec to its opening condition; the counters survive as pulled diagnostics.
 */
void IO::Drivers::S7Comm::PduCodec::reset() noexcept
{
  m_pduBytes = kMinPduBytes;
  m_lastError.clear();
}

/**
 * @brief Returns the negotiated message budget in bytes.
 */
int IO::Drivers::S7Comm::PduCodec::pduBytes() const noexcept
{
  return m_pduBytes;
}

/**
 * @brief Returns the next request reference; the controller echoes it, which is what tells a late
 *        answer from the one this request is waiting for.
 */
quint16 IO::Drivers::S7Comm::PduCodec::nextReference() noexcept
{
  ++m_reference;
  if (m_reference == 0)
    ++m_reference;

  SS_ASSERT_LOG(m_reference != 0);
  return m_reference;
}

/**
 * @brief Why the controller refused the last job, empty when it refused nothing.
 */
QString IO::Drivers::S7Comm::PduCodec::lastError() const
{
  return m_lastError;
}

/**
 * @brief Protocol data units refused for a bad identifier, a contradictory length or an answer to
 *        a service this codec never asked for.
 */
quint64 IO::Drivers::S7Comm::PduCodec::malformedPdus() const noexcept
{
  return m_malformed;
}

/**
 * @brief Items the controller answered with a non-success return code.
 */
quint64 IO::Drivers::S7Comm::PduCodec::refusedItems() const noexcept
{
  return m_refusedItems;
}

/**
 * @brief Items whose own answer cannot fit the negotiated budget. They are still requested, alone
 *        in their chunk: refusing to send them would stall the poll on one bad configuration line.
 */
quint64 IO::Drivers::S7Comm::PduCodec::oversizedItems() const noexcept
{
  return m_oversized;
}

//--------------------------------------------------------------------------------------------------
// Setup negotiation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the setup-communication job. One outstanding message in each direction is all a
 *        polling client needs, and the requested length is the value the controller answers with
 *        its own ceiling to.
 */
QByteArray IO::Drivers::S7Comm::PduCodec::buildSetupRequest(quint16 reference) const
{
  SS_ASSERT_LOG(reference != 0);

  QByteArray pdu;
  pdu.reserve(kJobHeaderBytes + kSetupParamBytes);
  appendJobHeader(pdu, reference, kSetupParamBytes, 0);
  pdu.append(static_cast<char>(kFunctionSetup));
  pdu.append(static_cast<char>(0));
  appendBe16(pdu, kMaxAmqCount);
  appendBe16(pdu, kMaxAmqCount);
  appendBe16(pdu, static_cast<quint16>(kRequestedPduBytes));

  SS_ASSERT_LOG(pdu.size() == kJobHeaderBytes + kSetupParamBytes);
  return pdu;
}

/**
 * @brief Accepts the setup acknowledgement and stores the length the controller settled on. That
 *        length caps every later request AND its answer, so a codec that missed it would plan
 *        chunks the controller silently truncates.
 */
IO::Drivers::S7Comm::PduResult IO::Drivers::S7Comm::PduCodec::parseSetupResponse(QByteArrayView pdu)
{
  PduHeader header;
  const auto parsed = parseHeader(pdu, header);
  if (parsed != PduResult::Ok) {
    ++m_malformed;
    return parsed;
  }

  if (header.rosctr != kRosctrAckData || header.parameterBytes < kSetupParamBytes) {
    ++m_malformed;
    return PduResult::Malformed;
  }

  if (header.errorClass != 0) {
    m_lastError = errorText(header.errorClass, header.errorCode);
    return PduResult::Refused;
  }

  if (s7PduOctet(pdu, header.parameterOffset) != kFunctionSetup) {
    ++m_malformed;
    return PduResult::Malformed;
  }

  m_lastError.clear();
  m_pduBytes =
    qBound(kMinPduBytes, static_cast<int>(readBe16(pdu, header.parameterOffset + 6)), kMaxPduBytes);

  SS_ASSERT_LOG(m_pduBytes >= kMinPduBytes);
  return PduResult::Ok;
}

//--------------------------------------------------------------------------------------------------
// Read service
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the longest run of items starting at @p first that one exchange can carry. Both
 *        halves are budgeted: twelve octets per item on the way out, four plus the payload and its
 *        fill octet on the way back. The run is never empty, so a list always makes progress.
 */
IO::Drivers::S7Comm::Chunk IO::Drivers::S7Comm::PduCodec::nextChunk(const QList<ReadItem>& items,
                                                                    int first)
{
  SS_ASSERT(first >= 0 && first < items.size(), return Chunk{});
  SS_ASSERT_LOG(m_pduBytes >= kMinPduBytes);

  const int requestBudget  = m_pduBytes - kJobHeaderBytes - kReadParamBytes;
  const int responseBudget = m_pduBytes - kAckHeaderBytes - kReadParamBytes;

  Chunk chunk{first, 0};
  int answered = 0;
  for (int i = first; i < items.size() && chunk.count < kMaxItemsPerRequest; ++i) {
    const int payload = resultBytes(items.at(i));
    const int cost    = kResultItemBytes + payload + (payload % 2);
    const int asked   = (chunk.count + 1) * kRequestItemBytes;
    if (chunk.count > 0 && (answered + cost > responseBudget || asked > requestBudget))
      break;

    if (chunk.count == 0 && cost > responseBudget)
      ++m_oversized;

    answered += cost;
    ++chunk.count;
  }

  SS_ASSERT_LOG(chunk.count > 0);
  return chunk;
}

/**
 * @brief Splits the whole variable list into the exchanges one poll tick performs.
 */
QList<IO::Drivers::S7Comm::Chunk> IO::Drivers::S7Comm::PduCodec::planChunks(
  const QList<ReadItem>& items)
{
  SS_ASSERT(!items.isEmpty(), return {});
  SS_ASSERT_LOG(m_pduBytes >= kMinPduBytes);

  QList<Chunk> chunks;
  int index = 0;
  while (index < items.size()) {
    const auto chunk = nextChunk(items, index);
    if (chunk.count <= 0)
      break;

    chunks.append(chunk);
    index += chunk.count;
  }

  SS_ASSERT_LOG(!chunks.isEmpty());
  return chunks;
}

/**
 * @brief Builds one read-variable job carrying the items @p chunk names.
 */
QByteArray IO::Drivers::S7Comm::PduCodec::buildReadRequest(quint16 reference,
                                                           const QList<ReadItem>& items,
                                                           const Chunk& chunk) const
{
  SS_ASSERT(chunk.count > 0 && chunk.count <= kMaxItemsPerRequest, return {});
  SS_ASSERT(chunk.first >= 0 && chunk.first + chunk.count <= items.size(), return {});

  const int parameterBytes = kReadParamBytes + chunk.count * kRequestItemBytes;

  QByteArray pdu;
  pdu.reserve(kJobHeaderBytes + parameterBytes);
  appendJobHeader(pdu, reference, parameterBytes, 0);
  pdu.append(static_cast<char>(kFunctionReadVar));
  pdu.append(static_cast<char>(chunk.count));
  for (int i = 0; i < chunk.count; ++i)
    appendRequestItem(pdu, items.at(chunk.first + i));

  SS_ASSERT_LOG(pdu.size() == kJobHeaderBytes + parameterBytes);
  return pdu;
}

/**
 * @brief Walks a read acknowledgement into one result per requested item. A response whose item
 *        list runs short appends NOTHING and reports Truncated: an item assembled out of the next
 *        item's octets carries no marker downstream and reads as a controller value.
 */
IO::Drivers::S7Comm::PduResult IO::Drivers::S7Comm::PduCodec::parseReadResponse(
  QByteArrayView pdu, int expected, QList<ReadResult>& out)
{
  SS_ASSERT(expected > 0 && expected <= kMaxItemsPerRequest, return PduResult::Malformed);

  PduHeader header;
  const qsizetype mark = out.size();
  const auto parsed    = parseHeader(pdu, header);
  if (parsed != PduResult::Ok) {
    ++m_malformed;
    return parsed;
  }

  if (header.rosctr != kRosctrAckData || header.parameterBytes < kReadParamBytes) {
    ++m_malformed;
    return PduResult::Malformed;
  }

  if (header.errorClass != 0) {
    m_lastError = errorText(header.errorClass, header.errorCode);
    return PduResult::Refused;
  }

  if (s7PduOctet(pdu, header.parameterOffset) != kFunctionReadVar
      || s7PduOctet(pdu, header.parameterOffset + 1) != expected) {
    ++m_malformed;
    return PduResult::Malformed;
  }

  qsizetype pos = header.dataOffset;
  for (int i = 0; i < expected; ++i) {
    ReadResult item;
    if (!readResultItem(pdu, pos, i + 1 < expected, item)) {
      out.resize(mark);
      ++m_malformed;
      return PduResult::Truncated;
    }

    if (item.returnCode != kReturnSuccess)
      ++m_refusedItems;

    out.append(item);
  }

  SS_ASSERT_LOG(out.size() - mark == expected);
  return PduResult::Ok;
}
