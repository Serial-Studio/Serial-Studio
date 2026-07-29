/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>

#include "IO/FileTransmission/ZMODEM.h"

// This suite plays the receiver. Every byte the sender emits is decoded by the local helpers below,
// and every reply is rebuilt from the wire format rather than from the production builders, so a
// change to either side of the encoding shows up as a failure instead of cancelling out. The CRC-16
// and CRC-32 helpers are bitwise reimplementations for the same reason: CRC.h's table-driven
// versions are the code under test.
//
// Every test function is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

//--------------------------------------------------------------------------------------------------
// Wire constants
//--------------------------------------------------------------------------------------------------

static constexpr quint8 kZDLE = 0x18;
static constexpr char kZBIN   = 'A';
static constexpr char kZBIN32 = 'C';
static constexpr char kZHEX   = 'B';

static constexpr quint8 kZRQINIT = 0;
static constexpr quint8 kZRINIT  = 1;
static constexpr quint8 kZACK    = 3;
static constexpr quint8 kZFILE   = 4;
static constexpr quint8 kZSKIP   = 5;
static constexpr quint8 kZNAK    = 6;
static constexpr quint8 kZABORT  = 7;
static constexpr quint8 kZFIN    = 8;
static constexpr quint8 kZRPOS   = 9;
static constexpr quint8 kZDATA   = 10;
static constexpr quint8 kZEOF    = 11;
static constexpr quint8 kZFERR   = 12;
static constexpr quint8 kZCAN    = 16;

static constexpr quint8 kZCRCE = 'h';
static constexpr quint8 kZCRCG = 'i';
static constexpr quint8 kZCRCW = 'k';

//--------------------------------------------------------------------------------------------------
// Independent checksum references
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bitwise CRC-16/XMODEM (init 0x0000, poly 0x1021, no reflection), the algorithm ZMODEM hex
 *        and binary headers carry.
 */
[[nodiscard]] static quint16 crc16Xmodem(const QByteArray& data)
{
  quint16 crc = 0x0000;
  for (const char raw : data) {
    crc ^= static_cast<quint16>(static_cast<quint16>(static_cast<quint8>(raw)) << 8);
    for (int bit = 0; bit < 8; ++bit)
      crc =
        (crc & 0x8000) ? static_cast<quint16>((crc << 1) ^ 0x1021) : static_cast<quint16>(crc << 1);
  }

  return crc;
}

/**
 * @brief Bitwise CRC-32/ISO-HDLC (init 0xFFFFFFFF, reflected poly 0xEDB88320, final complement),
 *        the algorithm ZMODEM binary-32 headers and data subpackets carry.
 */
[[nodiscard]] static quint32 crc32Hdlc(const QByteArray& data)
{
  quint32 crc = 0xFFFFFFFFu;
  for (const char raw : data) {
    crc ^= static_cast<quint32>(static_cast<quint8>(raw));
    for (int bit = 0; bit < 8; ++bit)
      crc = (crc & 1u) ? ((crc >> 1) ^ 0xEDB88320u) : (crc >> 1);
  }

  return ~crc;
}

/**
 * @brief Reads a little-endian 32-bit field out of a four-byte slice.
 */
[[nodiscard]] static quint32 leToU32(const QByteArray& bytes)
{
  if (bytes.size() != 4)
    return 0;

  return static_cast<quint32>(static_cast<quint8>(bytes.at(0)))
       | (static_cast<quint32>(static_cast<quint8>(bytes.at(1))) << 8)
       | (static_cast<quint32>(static_cast<quint8>(bytes.at(2))) << 16)
       | (static_cast<quint32>(static_cast<quint8>(bytes.at(3))) << 24);
}

/**
 * @brief Appends a little-endian 32-bit field, the byte order every ZMODEM header argument uses.
 */
static void appendU32(QByteArray& target, quint32 value)
{
  for (int shift = 0; shift < 32; shift += 8)
    target.append(static_cast<char>((value >> shift) & 0xFF));
}

//--------------------------------------------------------------------------------------------------
// ZDLE transcoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief The seven byte values ZMODEM.cpp escapes: ZDLE itself, both flow-control pairs and DLE,
 *        in their plain and high-bit forms.
 */
[[nodiscard]] static bool needsZdleEscape(quint8 value)
{
  return value == 0x18 || value == 0x11 || value == 0x13 || value == 0x10 || value == 0x90
      || value == 0x91 || value == 0x93;
}

/**
 * @brief Escapes a byte sequence the way the receiver's own transmitter would.
 */
[[nodiscard]] static QByteArray zdleEncode(const QByteArray& data)
{
  QByteArray encoded;
  encoded.reserve(data.size() * 2);

  for (const char raw : data) {
    const quint8 value = static_cast<quint8>(raw);
    if (!needsZdleEscape(value)) {
      encoded.append(raw);
      continue;
    }

    encoded.append(static_cast<char>(kZDLE));
    encoded.append(static_cast<char>(value ^ 0x40));
  }

  return encoded;
}

/**
 * @brief Reverses zdleEncode(); @p ok reports whether the sequence ended on a complete escape.
 */
[[nodiscard]] static QByteArray zdleDecode(const QByteArray& data, bool* ok)
{
  QByteArray decoded;
  *ok = true;

  for (qsizetype pos = 0; pos < data.size(); ++pos) {
    if (static_cast<quint8>(data.at(pos)) != kZDLE) {
      decoded.append(data.at(pos));
      continue;
    }

    ++pos;
    if (pos >= data.size()) {
      *ok = false;
      return decoded;
    }

    decoded.append(static_cast<char>(static_cast<quint8>(data.at(pos)) ^ 0x40));
  }

  return decoded;
}

//--------------------------------------------------------------------------------------------------
// Receiver-side frame builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a hex header carrying an explicit CRC, so a corrupted frame can be fabricated.
 */
[[nodiscard]] static QByteArray makeHexHeaderWithCrc(quint8 type, quint32 arg, quint16 crc)
{
  QByteArray payload;
  payload.append(static_cast<char>(type));
  appendU32(payload, arg);

  QByteArray crcBytes;
  crcBytes.append(static_cast<char>((crc >> 8) & 0xFF));
  crcBytes.append(static_cast<char>(crc & 0xFF));

  QByteArray frame("**");
  frame.append(static_cast<char>(kZDLE));
  frame.append(kZHEX);
  frame.append(payload.toHex());
  frame.append(crcBytes.toHex());
  frame.append("\r\n");

  return frame;
}

/**
 * @brief Builds a well-formed ZMODEM hex header (ZPAD ZPAD ZDLE ZHEX type arg crc CR LF).
 */
[[nodiscard]] static QByteArray makeHexHeader(quint8 type, quint32 arg)
{
  QByteArray payload;
  payload.append(static_cast<char>(type));
  appendU32(payload, arg);

  return makeHexHeaderWithCrc(type, arg, crc16Xmodem(payload));
}

/**
 * @brief Builds a ZDLE-escaped binary-32 header (ZPAD ZPAD ZDLE ZBIN32 type arg crc32).
 */
[[nodiscard]] static QByteArray makeBin32Header(quint8 type, quint32 arg)
{
  QByteArray payload;
  payload.append(static_cast<char>(type));
  appendU32(payload, arg);

  QByteArray body = payload;
  appendU32(body, crc32Hdlc(payload));

  QByteArray frame("**");
  frame.append(static_cast<char>(kZDLE));
  frame.append(kZBIN32);
  frame.append(zdleEncode(body));

  return frame;
}

/**
 * @brief Builds a ZDLE-escaped binary header (ZPAD ZPAD ZDLE ZBIN type arg crc16).
 */
[[nodiscard]] static QByteArray makeBinHeader(quint8 type, quint32 arg)
{
  QByteArray payload;
  payload.append(static_cast<char>(type));
  appendU32(payload, arg);

  const quint16 crc = crc16Xmodem(payload);
  QByteArray body   = payload;
  body.append(static_cast<char>((crc >> 8) & 0xFF));
  body.append(static_cast<char>(crc & 0xFF));

  QByteArray frame("**");
  frame.append(static_cast<char>(kZDLE));
  frame.append(kZBIN);
  frame.append(zdleEncode(body));

  return frame;
}

//--------------------------------------------------------------------------------------------------
// Sender-side frame decoders
//--------------------------------------------------------------------------------------------------

/**
 * @brief A decoded ZMODEM header: valid says the framing parsed, crc_ok says the checksum matched.
 */
struct DecodedHeader {
  bool valid;
  bool crc_ok;
  quint8 type;
  quint32 arg;
};

/**
 * @brief A decoded data subpacket: the unescaped payload, its frame-end byte and its CRC verdict.
 */
struct DecodedSubpacket {
  bool valid;
  bool crc_ok;
  quint8 frame_end;
  QByteArray data;
};

/**
 * @brief Decodes one hex header emission and verifies its CRC-16 over type plus argument.
 */
[[nodiscard]] static DecodedHeader decodeHexHeader(const QByteArray& frame)
{
  DecodedHeader result{false, false, 0, 0};
  if (frame.size() != 20 || !frame.startsWith("**") || !frame.endsWith("\r\n"))
    return result;

  if (static_cast<quint8>(frame.at(2)) != kZDLE || frame.at(3) != kZHEX)
    return result;

  const QByteArray body = QByteArray::fromHex(frame.mid(4, 14));
  if (body.size() != 7)
    return result;

  const quint16 rx_crc =
    static_cast<quint16>((static_cast<quint8>(body.at(5)) << 8) | static_cast<quint8>(body.at(6)));

  result.valid  = true;
  result.type   = static_cast<quint8>(body.at(0));
  result.arg    = leToU32(body.mid(1, 4));
  result.crc_ok = (crc16Xmodem(body.left(5)) == rx_crc);

  return result;
}

/**
 * @brief Decodes one binary-32 header emission and verifies its CRC-32 over type plus argument.
 */
[[nodiscard]] static DecodedHeader decodeBin32Header(const QByteArray& frame)
{
  DecodedHeader result{false, false, 0, 0};
  if (frame.size() < 5 || !frame.startsWith("**"))
    return result;

  if (static_cast<quint8>(frame.at(2)) != kZDLE || frame.at(3) != kZBIN32)
    return result;

  bool ok               = false;
  const QByteArray body = zdleDecode(frame.mid(4), &ok);
  if (!ok || body.size() != 9)
    return result;

  result.valid  = true;
  result.type   = static_cast<quint8>(body.at(0));
  result.arg    = leToU32(body.mid(1, 4));
  result.crc_ok = (crc32Hdlc(body.left(5)) == leToU32(body.mid(5, 4)));

  return result;
}

/**
 * @brief Decodes one data subpacket emission: unescapes until ZDLE meets a frame-end byte, then
 *        checks the trailing CRC-32, which ZMODEM computes over the data plus that frame-end byte.
 */
[[nodiscard]] static DecodedSubpacket parseSubpacket(const QByteArray& frame)
{
  DecodedSubpacket result{false, false, 0, QByteArray()};
  qsizetype pos   = 0;
  bool terminated = false;

  for (; pos < frame.size() && !terminated; ++pos) {
    const quint8 value = static_cast<quint8>(frame.at(pos));
    if (value != kZDLE) {
      result.data.append(static_cast<char>(value));
      continue;
    }

    if (pos + 1 >= frame.size())
      return result;

    const quint8 next = static_cast<quint8>(frame.at(++pos));
    if (next >= kZCRCE && next <= kZCRCW) {
      result.frame_end = next;
      terminated       = true;
      continue;
    }

    result.data.append(static_cast<char>(next ^ 0x40));
  }

  if (!terminated)
    return result;

  bool ok                   = false;
  const QByteArray crcBytes = zdleDecode(frame.mid(pos), &ok);
  if (!ok || crcBytes.size() != 4)
    return result;

  QByteArray crcInput = result.data;
  crcInput.append(static_cast<char>(result.frame_end));

  result.valid  = true;
  result.crc_ok = (crc32Hdlc(crcInput) == leToU32(crcBytes));

  return result;
}

//--------------------------------------------------------------------------------------------------
// Spy helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the byte array carried by the writeRequested() emission at @p index.
 */
[[nodiscard]] static QByteArray writeAt(const QSignalSpy& spy, int index)
{
  return spy.at(index).at(0).toByteArray();
}

/**
 * @brief Collects the run of consecutive subpacket emissions that starts at @p from, stopping at
 *        the first emission that is a header, an over-and-out or a cancel sequence.
 */
[[nodiscard]] static QList<DecodedSubpacket> collectSubpackets(const QSignalSpy& spy, int from)
{
  QList<DecodedSubpacket> packets;
  for (int i = from; i < spy.size(); ++i) {
    const DecodedSubpacket packet = parseSubpacket(writeAt(spy, i));
    if (!packet.valid)
      return packets;

    packets.append(packet);
  }

  return packets;
}

/**
 * @brief Concatenates the payload of a subpacket run into the file image the receiver would write.
 */
[[nodiscard]] static QByteArray joinSubpackets(const QList<DecodedSubpacket>& packets)
{
  QByteArray payload;
  for (const auto& packet : packets)
    payload.append(packet.data);

  return payload;
}

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

/**
 * @brief A payload that spans several subpackets and repeats every byte ZDLE must escape, so the
 *        escaping and the chunking are exercised by one transfer.
 */
[[nodiscard]] static QByteArray escapedPayload(int length)
{
  static const QByteArray kSeed = QByteArray::fromHex("181113109091930d0aff2a41005a");
  const int repeats             = static_cast<int>(length / kSeed.size()) + 1;

  QByteArray payload;
  payload.reserve(length + kSeed.size());
  for (int i = 0; i < repeats; ++i)
    payload.append(kSeed);

  payload.truncate(length);
  return payload;
}

/**
 * @brief Materializes @p payload into @p file and closes it; QTemporaryFile keeps the path alive
 *        until the object goes out of scope, which is what bounds the fixture's lifetime.
 */
static void writeTempPayload(QTemporaryFile& file, const QByteArray& payload)
{
  QVERIFY(file.open());
  QCOMPARE(file.write(payload), qint64(payload.size()));
  file.close();
}

/**
 * @brief Advances a freshly started sender through ZRINIT and ZRPOS(0) into the data phase.
 */
static void handshakeToData(IO::Protocols::ZMODEM& zmodem)
{
  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  zmodem.processInput(makeHexHeader(kZRPOS, 0));
}

/**
 * @brief The ZMODEM sender state machine driven from the receiver's side of the link.
 */
class TstZmodem : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void protocolNameAndIdleState();
  void settersClampToTheDocumentedRange();
  void unreadableFileFinishesWithFailure();

  void startTransferEmitsAutoStartAndZrqinit();
  void zrqinitHexHeaderMatchesTheWireFormat();

  void zrinitProducesZfileHeaderAndMetadata();

  void zrposZeroOpensTheDataPhase();
  void dataSubpacketsCarryTheWholePayload();
  void zdleEscapingRemovesEveryControlByte();
  void subpacketCrcCoversDataAndFrameEnd();
  void blockSizeDeterminesSubpacketBoundaries();
  void largePayloadYieldsToTheEventLoop();

  void zeofCarriesFileSizeAndZfinCompletesTheSession();

  void zrposMidFileResendsFromTheRequestedOffset();
  void zrposBeyondEndOfFileIsClamped();
  void emptyFileCompletesWithoutADataSubpacket();
  void startTransferWhileActiveCancelsThePrevious();

  void zskipEndsTheSessionWithZfin();
  void receiverAbortFinishesWithFailure_data();
  void receiverAbortFinishesWithFailure();
  void cancelTransferSendsCanAndFinishes();
  void cancelTransferWhileIdleIsANoOp();

  void garbageBetweenHeadersIsIgnored();
  void corruptedHexHeaderCrcIsDropped();
  void binaryHeadersFromTheReceiverAreAccepted();
  void byteAtATimeDeliveryMatchesBulkDelivery();
  void znakRetriesUntilTheBudgetIsExhausted();
  void zackClearsTheRetryBudget();
  void processInputWhileInactiveIsIgnored();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, which is what makes the guarded
 *        paths (processInput() on an inactive sender, empty input) observable from a debug build.
 */
void TstZmodem::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

//--------------------------------------------------------------------------------------------------
// Configuration and lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief A fresh sender is inactive and names itself; isActive() is what ConnectionManager polls
 *        before routing device bytes into processInput().
 */
void TstZmodem::protocolNameAndIdleState()
{
  IO::Protocols::ZMODEM zmodem;

  QCOMPARE(zmodem.protocolName(), QStringLiteral("ZMODEM"));
  QVERIFY(!zmodem.isActive());
  QCOMPARE(zmodem.blockSize(), 1024);
  QCOMPARE(zmodem.timeoutMs(), 15000);
  QCOMPARE(zmodem.maxRetries(), 10);
}

/**
 * @brief Every setter clamps rather than rejects, so a project file carrying nonsense still yields
 *        a transmittable subpacket size and a timer that can fire.
 */
void TstZmodem::settersClampToTheDocumentedRange()
{
  IO::Protocols::ZMODEM zmodem;

  zmodem.setBlockSize(1);
  QCOMPARE(zmodem.blockSize(), 64);
  zmodem.setBlockSize(999999);
  QCOMPARE(zmodem.blockSize(), 8192);
  zmodem.setBlockSize(256);
  QCOMPARE(zmodem.blockSize(), 256);

  zmodem.setTimeoutMs(0);
  QCOMPARE(zmodem.timeoutMs(), 1000);
  zmodem.setTimeoutMs(30000);
  QCOMPARE(zmodem.timeoutMs(), 30000);

  zmodem.setMaxRetries(0);
  QCOMPARE(zmodem.maxRetries(), 1);
  zmodem.setMaxRetries(4);
  QCOMPARE(zmodem.maxRetries(), 4);
}

/**
 * @brief A path that cannot be opened fails before a single byte reaches the link.
 */
void TstZmodem::unreadableFileFinishesWithFailure()
{
  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(QDir::tempPath() + QStringLiteral("/tst_zmodem_absent_file.bin"));

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), false);
  QVERIFY(finished.at(0).at(1).toString().contains(QStringLiteral("Cannot open file")));
  QVERIFY(!zmodem.isActive());
}

//--------------------------------------------------------------------------------------------------
// Opening sequence
//--------------------------------------------------------------------------------------------------

/**
 * @brief startTransfer() emits one write holding the "rz" auto-start string and the ZRQINIT hex
 *        header, and reports zero progress against the real file size.
 */
void TstZmodem::startTransferEmitsAutoStartAndZrqinit()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(300));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy progress(&zmodem, &IO::Protocols::Protocol::progressChanged);

  zmodem.startTransfer(file.fileName());

  QVERIFY(zmodem.isActive());
  QCOMPARE(writes.size(), qsizetype(1));
  QVERIFY(writeAt(writes, 0).startsWith("rz\r"));

  const DecodedHeader header = decodeHexHeader(writeAt(writes, 0).mid(3));
  QVERIFY(header.valid);
  QVERIFY(header.crc_ok);
  QCOMPARE(header.type, kZRQINIT);
  QCOMPARE(header.arg, quint32(0));

  QCOMPARE(progress.size(), qsizetype(1));
  QCOMPARE(progress.at(0).at(0).toLongLong(), qint64(0));
  QCOMPARE(progress.at(0).at(1).toLongLong(), qint64(300));
}

/**
 * @brief The hex header is byte-exact: ZPAD ZPAD ZDLE 'B', fourteen lowercase hex digits and CRLF.
 *        ZRQINIT with a zero argument checksums to zero, so the whole field is ASCII zeroes.
 */
void TstZmodem::zrqinitHexHeaderMatchesTheWireFormat()
{
  QTemporaryFile file;
  writeTempPayload(file, QByteArray("payload"));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  QCOMPARE(writes.size(), qsizetype(1));

  QByteArray expected("**");
  expected.append(static_cast<char>(kZDLE));
  expected.append(kZHEX);
  expected.append(QByteArray(14, '0'));
  expected.append("\r\n");

  QCOMPARE(writeAt(writes, 0).mid(3), expected);
  QCOMPARE(writeAt(writes, 0), QByteArray("rz\r") + expected);
}

//--------------------------------------------------------------------------------------------------
// File announcement
//--------------------------------------------------------------------------------------------------

/**
 * @brief ZRINIT moves the sender to ZFILE: a binary-32 header followed by the ZCRCW metadata
 *        subpacket whose two NUL-terminated fields are the base name and the size/mtime record.
 */
void TstZmodem::zrinitProducesZfileHeaderAndMetadata()
{
  const QByteArray payload = escapedPayload(700);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  zmodem.processInput(makeHexHeader(kZRINIT, 0));

  QCOMPARE(writes.size(), qsizetype(3));

  const DecodedHeader header = decodeBin32Header(writeAt(writes, 1));
  QVERIFY(header.valid);
  QVERIFY(header.crc_ok);
  QCOMPARE(header.type, kZFILE);
  QCOMPARE(header.arg, quint32(0));

  const DecodedSubpacket meta = parseSubpacket(writeAt(writes, 2));
  QVERIFY(meta.valid);
  QVERIFY(meta.crc_ok);
  QCOMPARE(meta.frame_end, kZCRCW);

  const QFileInfo info(file.fileName());
  const QList<QByteArray> fields = meta.data.split('\0');
  QCOMPARE(fields.size(), qsizetype(3));
  QCOMPARE(fields.at(0), info.fileName().toUtf8());
  QVERIFY(fields.at(1).startsWith(QByteArray::number(payload.size()) + ' '));
  QVERIFY(fields.at(1).endsWith(" 0 0 0 0"));
  QVERIFY(fields.at(1).contains(QByteArray::number(info.lastModified().toSecsSinceEpoch())));
  QVERIFY(fields.at(2).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Data phase
//--------------------------------------------------------------------------------------------------

/**
 * @brief ZRPOS(0) opens the data phase with a binary-32 ZDATA header carrying that same offset.
 */
void TstZmodem::zrposZeroOpensTheDataPhase()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(500));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const DecodedHeader header = decodeBin32Header(writeAt(writes, 3));
  QVERIFY(header.valid);
  QVERIFY(header.crc_ok);
  QCOMPARE(header.type, kZDATA);
  QCOMPARE(header.arg, quint32(0));
}

/**
 * @brief The full happy path: every subpacket between ZDATA and ZEOF concatenates back to the file
 *        byte for byte, all but the last close with ZCRCG and the last closes with ZCRCE.
 */
void TstZmodem::dataSubpacketsCarryTheWholePayload()
{
  const QByteArray payload = escapedPayload(2600);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy progress(&zmodem, &IO::Protocols::Protocol::progressChanged);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, 4);
  QCOMPARE(packets.size(), qsizetype(3));
  QCOMPARE(joinSubpackets(packets), payload);

  for (int i = 0; i < packets.size(); ++i) {
    QVERIFY(packets.at(i).crc_ok);
    QCOMPARE(packets.at(i).frame_end, (i == packets.size() - 1) ? kZCRCE : kZCRCG);
  }

  QCOMPARE(progress.size(), qsizetype(4));
  QCOMPARE(progress.at(3).at(0).toLongLong(), qint64(payload.size()));
  QCOMPARE(progress.at(3).at(1).toLongLong(), qint64(payload.size()));
}

/**
 * @brief None of the seven escaped byte values ever reaches the link literally, and ZDLE only ever
 *        appears as an escape introducer -- otherwise a hardware flow-control pair or a spurious
 *        frame terminator would ride inside the file image.
 */
void TstZmodem::zdleEscapingRemovesEveryControlByte()
{
  const QByteArray payload = escapedPayload(400);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, 4);
  QCOMPARE(packets.size(), qsizetype(1));
  QCOMPARE(packets.at(0).data, payload);
  QVERIFY(packets.at(0).crc_ok);

  const QByteArray wire = writeAt(writes, 4);
  QVERIFY(wire.size() > payload.size());
  for (const char raw : wire) {
    const quint8 value = static_cast<quint8>(raw);
    QVERIFY(value == kZDLE || !needsZdleEscape(value));
  }
}

/**
 * @brief The subpacket CRC-32 spans the unescaped data plus the frame-end byte. Recomputing over
 *        the data alone must disagree, or the frame-end byte would be unprotected.
 */
void TstZmodem::subpacketCrcCoversDataAndFrameEnd()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(120));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, 4);
  QCOMPARE(packets.size(), qsizetype(1));

  const DecodedSubpacket packet = packets.at(0);
  QVERIFY(packet.crc_ok);

  QByteArray withFrameEnd = packet.data;
  withFrameEnd.append(static_cast<char>(packet.frame_end));
  QVERIFY(crc32Hdlc(withFrameEnd) != crc32Hdlc(packet.data));

  const DecodedSubpacket meta = parseSubpacket(writeAt(writes, 2));
  QVERIFY(meta.crc_ok);
}

/**
 * @brief blockSize() is the subpacket boundary, not a hint: a payload of 200 bytes at 64 splits
 *        into 64/64/64/8 and nothing else.
 */
void TstZmodem::blockSizeDeterminesSubpacketBoundaries()
{
  const QByteArray payload = escapedPayload(200);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  zmodem.setBlockSize(64);
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, 4);
  QCOMPARE(packets.size(), qsizetype(4));
  QCOMPARE(packets.at(0).data.size(), qsizetype(64));
  QCOMPARE(packets.at(1).data.size(), qsizetype(64));
  QCOMPARE(packets.at(2).data.size(), qsizetype(64));
  QCOMPARE(packets.at(3).data.size(), qsizetype(8));
  QCOMPARE(joinSubpackets(packets), payload);
}

/**
 * @brief Past 64 chunks the sender yields to the event loop and resumes on a zero-timer, so a long
 *        transfer only completes once the loop runs; the reassembled image must be identical.
 */
void TstZmodem::largePayloadYieldsToTheEventLoop()
{
  const QByteArray payload = escapedPayload(64 * 70);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  zmodem.setBlockSize(64);
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  QCOMPARE(writes.size(), qsizetype(4 + 64));
  QTRY_COMPARE(writes.size(), qsizetype(4 + 70 + 1));

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, 4);
  QCOMPARE(packets.size(), qsizetype(70));
  QCOMPARE(joinSubpackets(packets), payload);

  const DecodedHeader eof = decodeBin32Header(writeAt(writes, 74));
  QVERIFY(eof.valid);
  QCOMPARE(eof.type, kZEOF);
  QCOMPARE(eof.arg, quint32(payload.size()));
}

//--------------------------------------------------------------------------------------------------
// Session teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief ZEOF carries the total file size; the closing ZRINIT draws a hex ZFIN, and the receiver's
 *        ZFIN draws the "OO" over-and-out and a successful finished().
 */
void TstZmodem::zeofCarriesFileSizeAndZfinCompletesTheSession()
{
  const QByteArray payload = escapedPayload(1500);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);
  QSignalSpy progress(&zmodem, &IO::Protocols::Protocol::progressChanged);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const DecodedHeader eof = decodeBin32Header(writeAt(writes, 6));
  QVERIFY(eof.valid);
  QVERIFY(eof.crc_ok);
  QCOMPARE(eof.type, kZEOF);
  QCOMPARE(eof.arg, quint32(payload.size()));

  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  const DecodedHeader fin = decodeHexHeader(writeAt(writes, 7));
  QVERIFY(fin.valid);
  QVERIFY(fin.crc_ok);
  QCOMPARE(fin.type, kZFIN);
  QCOMPARE(progress.last().at(0).toLongLong(), qint64(payload.size()));

  zmodem.processInput(makeHexHeader(kZFIN, 0));
  QCOMPARE(writeAt(writes, 8), QByteArray("OO"));
  QCOMPARE(writes.size(), qsizetype(9));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), true);
  QVERIFY(finished.at(0).at(1).toString().isEmpty());
  QVERIFY(!zmodem.isActive());
}

//--------------------------------------------------------------------------------------------------
// Reposition
//--------------------------------------------------------------------------------------------------

/**
 * @brief A ZRPOS after ZEOF is a retransmission request: the sender seeks to that offset and the
 *        second data run reproduces exactly the tail of the file, while ZEOF still reports the
 *        total size rather than the resumed byte count.
 */
void TstZmodem::zrposMidFileResendsFromTheRequestedOffset()
{
  const QByteArray payload = escapedPayload(600);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  zmodem.setBlockSize(128);
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  const int firstRun = writes.size();
  QCOMPARE(joinSubpackets(collectSubpackets(writes, 4)), payload);

  zmodem.processInput(makeHexHeader(kZRPOS, 250));

  const DecodedHeader resumed = decodeBin32Header(writeAt(writes, firstRun));
  QVERIFY(resumed.valid);
  QVERIFY(resumed.crc_ok);
  QCOMPARE(resumed.type, kZDATA);
  QCOMPARE(resumed.arg, quint32(250));

  const QList<DecodedSubpacket> packets = collectSubpackets(writes, firstRun + 1);
  QCOMPARE(joinSubpackets(packets), payload.mid(250));
  QCOMPARE(packets.last().frame_end, kZCRCE);

  const DecodedHeader eof = decodeBin32Header(writeAt(writes, writes.size() - 1));
  QVERIFY(eof.valid);
  QCOMPARE(eof.type, kZEOF);
  QCOMPARE(eof.arg, quint32(payload.size()));
}

/**
 * @brief A ZRPOS past the end is clamped to the file size instead of failing the seek: the sender
 *        opens an empty data run and goes straight back to ZEOF.
 */
void TstZmodem::zrposBeyondEndOfFileIsClamped()
{
  const QByteArray payload = escapedPayload(300);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  zmodem.processInput(makeHexHeader(kZRPOS, 999999));

  const DecodedHeader data = decodeBin32Header(writeAt(writes, 3));
  QVERIFY(data.valid);
  QCOMPARE(data.type, kZDATA);
  QCOMPARE(data.arg, quint32(payload.size()));

  QCOMPARE(collectSubpackets(writes, 4).size(), qsizetype(0));

  const DecodedHeader eof = decodeBin32Header(writeAt(writes, 4));
  QVERIFY(eof.valid);
  QCOMPARE(eof.type, kZEOF);
  QCOMPARE(eof.arg, quint32(payload.size()));
  QCOMPARE(finished.size(), qsizetype(0));
  QVERIFY(zmodem.isActive());
}

/**
 * @brief A zero-byte file is still announced and still handshaken to completion: ZDATA is followed
 *        immediately by a ZEOF of zero, with no data subpacket in between. sendDataSubpackets()
 *        asserts m_fileSize > 0 on the way through, so this path logs a soft assertion.
 */
void TstZmodem::emptyFileCompletesWithoutADataSubpacket()
{
  QTemporaryFile file;
  writeTempPayload(file, QByteArray());

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  handshakeToData(zmodem);

  QCOMPARE(collectSubpackets(writes, 4).size(), qsizetype(0));

  const DecodedHeader eof = decodeBin32Header(writeAt(writes, 4));
  QVERIFY(eof.valid);
  QVERIFY(eof.crc_ok);
  QCOMPARE(eof.type, kZEOF);
  QCOMPARE(eof.arg, quint32(0));

  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  zmodem.processInput(makeHexHeader(kZFIN, 0));

  QCOMPARE(writeAt(writes, 6), QByteArray("OO"));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), true);
}

/**
 * @brief Starting a second transfer over a live one cancels the first: the link sees the cancel
 *        sequence, the caller sees one failed finished(), and only then does the new ZRQINIT go
 * out.
 */
void TstZmodem::startTransferWhileActiveCancelsThePrevious()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  zmodem.startTransfer(file.fileName());

  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(writeAt(writes, 1), QByteArray(5, static_cast<char>(0x18)) + QByteArray(5, '\b'));
  QCOMPARE(writeAt(writes, 2), writeAt(writes, 0));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), false);
  QVERIFY(zmodem.isActive());
}

//--------------------------------------------------------------------------------------------------
// Termination paths
//--------------------------------------------------------------------------------------------------

/**
 * @brief ZSKIP ends the session politely with ZFIN. The sender reports success once the receiver
 *        answers, so a skipped file is not distinguishable from a delivered one at this layer.
 */
void TstZmodem::zskipEndsTheSessionWithZfin()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  zmodem.processInput(makeHexHeader(kZSKIP, 0));

  const DecodedHeader fin = decodeHexHeader(writeAt(writes, 3));
  QVERIFY(fin.valid);
  QVERIFY(fin.crc_ok);
  QCOMPARE(fin.type, kZFIN);
  QCOMPARE(finished.size(), qsizetype(0));

  zmodem.processInput(makeHexHeader(kZFIN, 0));
  QCOMPARE(writeAt(writes, 4), QByteArray("OO"));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), true);
  QVERIFY(!zmodem.isActive());
}

void TstZmodem::receiverAbortFinishesWithFailure_data()
{
  QTest::addColumn<int>("type");
  QTest::addColumn<QString>("reason");

  QTest::newRow("ZABORT") << int(kZABORT) << QStringLiteral("Receiver cancelled the transfer");
  QTest::newRow("ZCAN") << int(kZCAN) << QStringLiteral("Receiver cancelled the transfer");
  QTest::newRow("ZFERR") << int(kZFERR) << QStringLiteral("Receiver reported a file error");
}

/**
 * @brief Every receiver-initiated failure header stops the sender without a reply frame and
 *        reports the matching reason.
 */
void TstZmodem::receiverAbortFinishesWithFailure()
{
  QFETCH(int, type);
  QFETCH(QString, reason);

  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  zmodem.processInput(makeHexHeader(static_cast<quint8>(type), 0));

  QCOMPARE(writes.size(), qsizetype(1));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), false);
  QCOMPARE(finished.at(0).at(1).toString(), reason);
  QVERIFY(!zmodem.isActive());
}

/**
 * @brief A user cancel on an active transfer writes the canonical five CAN plus five backspace
 *        sequence before reporting failure.
 */
void TstZmodem::cancelTransferSendsCanAndFinishes()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());
  zmodem.cancelTransfer();

  QCOMPARE(writes.size(), qsizetype(2));
  QCOMPARE(writeAt(writes, 1), QByteArray(5, static_cast<char>(0x18)) + QByteArray(5, '\b'));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), false);
  QCOMPARE(finished.at(0).at(1).toString(), QStringLiteral("Transfer cancelled by user"));
  QVERIFY(!zmodem.isActive());
}

/**
 * @brief Cancelling an idle sender is silent: no cancel sequence, no finished(), nothing a device
 *        that is not in a transfer would have to interpret.
 */
void TstZmodem::cancelTransferWhileIdleIsANoOp()
{
  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.cancelTransfer();
  zmodem.cancelTransfer();

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(finished.size(), qsizetype(0));
  QVERIFY(!zmodem.isActive());
}

//--------------------------------------------------------------------------------------------------
// Robustness
//--------------------------------------------------------------------------------------------------

/**
 * @brief Console noise, partial ZPAD runs, an unknown header flavour and an over-long hex header
 *        are all discarded while waiting for ZRINIT, and none of them poisons the parser for the
 *        real header that follows.
 */
void TstZmodem::garbageBetweenHeadersIsIgnored()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());

  QByteArray junk("login: root\r\npassword?\r\n");
  junk.append("*");
  junk.append("*x");
  junk.append("**");
  junk.append(static_cast<char>(kZDLE));
  junk.append('Z');
  junk.append("**");
  junk.append(static_cast<char>(kZDLE));
  junk.append(kZHEX);
  junk.append("0102\r\n");
  junk.append("**");
  junk.append(static_cast<char>(kZDLE));
  junk.append(kZHEX);
  junk.append(QByteArray(40, '0'));
  junk.append("\r\n");

  zmodem.processInput(junk);

  QCOMPARE(writes.size(), qsizetype(1));
  QVERIFY(zmodem.isActive());

  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(decodeBin32Header(writeAt(writes, 1)).type, kZFILE);
}

/**
 * @brief A hex header whose CRC does not match is dropped with a status message rather than acted
 *        on, and the sender still accepts the corrected retransmission.
 */
void TstZmodem::corruptedHexHeaderCrcIsDropped()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy status(&zmodem, &IO::Protocols::Protocol::statusMessage);

  zmodem.startTransfer(file.fileName());

  QByteArray payload;
  payload.append(static_cast<char>(kZRINIT));
  appendU32(payload, 0);
  const quint16 badCrc = static_cast<quint16>(crc16Xmodem(payload) ^ 0xFFFF);

  status.clear();
  zmodem.processInput(makeHexHeaderWithCrc(kZRINIT, 0, badCrc));

  QCOMPARE(writes.size(), qsizetype(1));
  QCOMPARE(status.size(), qsizetype(1));
  QVERIFY(status.at(0).at(0).toString().contains(QStringLiteral("CRC mismatch")));

  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  QCOMPARE(writes.size(), qsizetype(3));
}

/**
 * @brief The sender accepts the two binary header flavours as well as hex, including the escaped
 *        argument bytes: a ZRPOS of 0x18 is ZDLE-escaped on the wire and must still decode to 24.
 */
void TstZmodem::binaryHeadersFromTheReceiverAreAccepted()
{
  const QByteArray payload = escapedPayload(300);

  QTemporaryFile file;
  writeTempPayload(file, payload);

  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);

  zmodem.startTransfer(file.fileName());
  zmodem.processInput(makeBin32Header(kZRINIT, 0));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(decodeBin32Header(writeAt(writes, 1)).type, kZFILE);

  zmodem.processInput(makeBinHeader(kZRPOS, 0x18));

  const DecodedHeader data = decodeBin32Header(writeAt(writes, 3));
  QVERIFY(data.valid);
  QCOMPARE(data.type, kZDATA);
  QCOMPARE(data.arg, quint32(0x18));
  QCOMPARE(joinSubpackets(collectSubpackets(writes, 4)), payload.mid(0x18));
}

/**
 * @brief Feeding the receiver's replies one byte at a time produces the identical write stream:
 *        the header parser carries its state across processInput() calls. Both senders read the
 *        same file so the ZFILE metadata subpacket is comparable.
 */
void TstZmodem::byteAtATimeDeliveryMatchesBulkDelivery()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(900));

  QByteArray replies;
  replies.append(makeHexHeader(kZRINIT, 0));
  replies.append(makeHexHeader(kZRPOS, 0));
  replies.append(makeHexHeader(kZRINIT, 0));
  replies.append(makeHexHeader(kZFIN, 0));

  IO::Protocols::ZMODEM bulk;
  QSignalSpy bulkWrites(&bulk, &IO::Protocols::Protocol::writeRequested);
  bulk.startTransfer(file.fileName());
  bulk.processInput(replies);

  IO::Protocols::ZMODEM drip;
  QSignalSpy dripWrites(&drip, &IO::Protocols::Protocol::writeRequested);
  drip.startTransfer(file.fileName());
  for (const char raw : replies) {
    if (!drip.isActive())
      break;

    drip.processInput(QByteArray(1, raw));
  }

  QVERIFY(!bulk.isActive());
  QVERIFY(!drip.isActive());
  QCOMPARE(dripWrites.size(), bulkWrites.size());

  for (int i = 0; i < bulkWrites.size(); ++i)
    QCOMPARE(writeAt(dripWrites, i), writeAt(bulkWrites, i));
}

/**
 * @brief ZNAK retries the step the sender is waiting on and gives up once the budget is spent,
 *        cancelling the link before reporting failure.
 */
void TstZmodem::znakRetriesUntilTheBudgetIsExhausted()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  zmodem.setMaxRetries(2);
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());

  zmodem.processInput(makeHexHeader(kZNAK, 0));
  QCOMPARE(writes.size(), qsizetype(2));
  QCOMPARE(writeAt(writes, 1), writeAt(writes, 0));
  QCOMPARE(finished.size(), qsizetype(0));

  zmodem.processInput(makeHexHeader(kZNAK, 0));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(writeAt(writes, 2), QByteArray(5, static_cast<char>(0x18)) + QByteArray(5, '\b'));
  QCOMPARE(finished.size(), qsizetype(1));
  QCOMPARE(finished.at(0).at(0).toBool(), false);
  QCOMPARE(finished.at(0).at(1).toString(), QStringLiteral("Maximum retries exceeded"));
  QVERIFY(!zmodem.isActive());
}

/**
 * @brief ZACK is a bare acknowledgment: it advances no state but clears the retry budget, so the
 *        cap counts the errors of one stalled step rather than of the whole session.
 */
void TstZmodem::zackClearsTheRetryBudget()
{
  QTemporaryFile file;
  writeTempPayload(file, escapedPayload(200));

  IO::Protocols::ZMODEM zmodem;
  zmodem.setMaxRetries(2);
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.startTransfer(file.fileName());

  zmodem.processInput(makeHexHeader(kZNAK, 0));
  zmodem.processInput(makeHexHeader(kZACK, 0));
  QCOMPARE(writes.size(), qsizetype(2));

  zmodem.processInput(makeHexHeader(kZNAK, 0));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(finished.size(), qsizetype(0));
  QVERIFY(zmodem.isActive());
}

/**
 * @brief Bytes that arrive when no transfer is running take the SS_ASSERT recovery branch: they
 *        are dropped without a reply and without touching the parser state.
 */
void TstZmodem::processInputWhileInactiveIsIgnored()
{
  IO::Protocols::ZMODEM zmodem;
  QSignalSpy writes(&zmodem, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy finished(&zmodem, &IO::Protocols::Protocol::finished);

  zmodem.processInput(QByteArray());
  zmodem.processInput(makeHexHeader(kZRINIT, 0));
  zmodem.processInput(makeHexHeader(kZFIN, 0));

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(finished.size(), qsizetype(0));
  QVERIFY(!zmodem.isActive());
}

QTEST_GUILESS_MAIN(TstZmodem)

#include "tst_zmodem.moc"
