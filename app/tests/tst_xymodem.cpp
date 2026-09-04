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

#include <QByteArray>
#include <QFile>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QTest>

#include "Protocols/FileTransfer/CRC.h"
#include "Protocols/FileTransfer/XMODEM.h"
#include "Protocols/FileTransfer/YMODEM.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. The senders are driven synchronously through
// processInput() so no slot depends on a real timer firing, except the one timeout test that says
// so in its name.

//--------------------------------------------------------------------------------------------------
// Wire constants
//--------------------------------------------------------------------------------------------------

// Restated here rather than reused from XMODEM's protected section: a test that imports the
// constants it verifies cannot catch a change to them.
static constexpr quint8 kSoh      = 0x01;
static constexpr quint8 kStx      = 0x02;
static constexpr quint8 kEot      = 0x04;
static constexpr quint8 kAck      = 0x06;
static constexpr quint8 kNak      = 0x15;
static constexpr quint8 kCan      = 0x18;
static constexpr quint8 kCrcStart = 0x43;

static const QByteArray kCheckInput("123456789");

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Test-only view of XMODEM's protected block builder.
 */
class BlockBuilder : public IO::Protocols::XMODEM {
public:
  using XMODEM::buildBlock;
};

/**
 * @brief Deterministic non-repeating payload, so a block boundary error shows up as a content
 *        mismatch instead of silently comparing equal runs of the same byte.
 */
static QByteArray patternPayload(int size)
{
  QByteArray out(size, '\0');
  for (int i = 0; i < size; ++i)
    out[i] = static_cast<char>((i * 7 + 13) & 0xFF);

  return out;
}

/**
 * @brief Writes a payload into the scratch directory and returns its path.
 */
static QString writePayload(const QTemporaryDir& dir, const QString& name, const QByteArray& bytes)
{
  const QString path = dir.filePath(name);
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly))
    return QString();

  const qint64 written = file.write(bytes);
  file.close();
  return written == bytes.size() ? path : QString();
}

/**
 * @brief One-byte receiver response.
 */
static QByteArray byteOf(quint8 value)
{
  return QByteArray(1, static_cast<char>(value));
}

/**
 * @brief The QByteArray argument of the n-th writeRequested() emission.
 */
static QByteArray writeAt(const QSignalSpy& spy, qsizetype index)
{
  return spy.at(index).at(0).toByteArray();
}

/**
 * @brief CRC-16/XMODEM over a QByteArray, computed through the production header.
 */
static quint16 crc16Of(const QByteArray& data)
{
  return IO::Protocols::CRC::crc16(reinterpret_cast<const quint8*>(data.constData()),
                                   static_cast<int>(data.size()));
}

/**
 * @brief CRC-32/ISO-HDLC over a QByteArray, computed through the production header.
 */
static quint32 crc32Of(const QByteArray& data)
{
  return IO::Protocols::CRC::crc32(reinterpret_cast<const quint8*>(data.constData()),
                                   static_cast<int>(data.size()));
}

/**
 * @brief Byte-level contract of the XMODEM / XMODEM-1K / YMODEM senders and their CRC helpers.
 */
class TstXyModem : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void crc16MatchesTheXmodemCheckValue();
  void crc32MatchesTheIsoHdlcCheckValue();
  void emptyInputReturnsTheSeedValues();

  void buildBlockFrames128ByteDataWithSoh();
  void buildBlockFrames1024ByteDataWithStx();
  void buildBlockCrcMatchesAnIndependentComputation();

  void startTransferOnMissingFileFails();
  void startTransferAnnouncesZeroProgressAndWaits();
  void crcStartByteTriggersTheFirstBlock();

  void ackAdvancesToTheNextBlock();
  void ackOnTheLastBlockSendsEot();
  void eotAckFinishesSuccessfully();
  void eotNakResendsTheEot();
  void blockNumberWrapsAtTheByteBoundary();

  void nakResendsTheSameBlock();
  void nakBeyondMaxRetriesCancelsTheTransfer();
  void ackResetsTheRetryCounter();
  void recoverableErrorsAreReportedTyped();

  void receiverCanAbortsTheTransfer();
  void cancelTransferOnIdleProtocolIsSilent();
  void cancelTransferSendsFiveCanBytes();

  void shortFinalBlockIsPaddedWithSubBytes();
  void use1KSwitchesNameAndBlockSize();
  void retryAndTimeoutSettersClampToTheirFloors();

  void unexpectedBytesAreIgnoredWhileWaitingForStart();
  void inputOnIdleProtocolIsIgnored();
  void bytewiseDeliveryMatchesBulkDelivery();

  void timeoutWithoutRetriesLeftAbortsTheTransfer();
  void timeoutMidTransferRearmsOrReports();
  void fullTransferReproducesTheSourceFile();

  void ymodemDefaultsToOneKilobyteBlocks();
  void ymodemHeaderBlockCarriesNameAndSize();
  void ymodemRunsTheFullBatchHandshake();
  void ymodemEndOfBatchBlockIsAllZeroes();
  void ymodemNakResendsTheSameDataBlock();
  void ymodemCancelResetsTheBatchState();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting, so the guard clauses that reject
 *        input on an idle protocol are exercised here rather than shipping unrun.
 */
void TstXyModem::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
  QVERIFY(qEnvironmentVariableIsSet("SS_ASSERT_NONFATAL"));
}

//--------------------------------------------------------------------------------------------------
// CRC helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief CRC.h implements CRC-16/XMODEM: seed 0x0000, polynomial 0x1021, no final xor. The
 *        catalogue check value is 0x31C3 and it must agree with IO::checksum("CRC-16-CCITT").
 */
void TstXyModem::crc16MatchesTheXmodemCheckValue()
{
  QCOMPARE(crc16Of(kCheckInput), quint16(0x31C3));
  QCOMPARE(crc16Of(QByteArray("a")), quint16(0x7C87));
  QCOMPARE(crc16Of(QByteArray(128, '\0')), quint16(0x0000));
  QCOMPARE(crc16Of(QByteArray(1, static_cast<char>(0x01))), quint16(0x1021));
}

/**
 * @brief CRC.h implements CRC-32/ISO-HDLC: reflected 0xEDB88320, seed 0xFFFFFFFF, final inversion.
 */
void TstXyModem::crc32MatchesTheIsoHdlcCheckValue()
{
  QCOMPARE(crc32Of(kCheckInput), quint32(0xCBF43926));
  QCOMPARE(crc32Of(QByteArray("a")), quint32(0xE8B7BE43));
  QCOMPARE(crc32Of(QByteArray("abc")), quint32(0x352441C2));
}

/**
 * @brief Zero-length input returns each algorithm's post-processed seed. CRC-16 has no final xor so
 *        it yields 0x0000; CRC-32 inverts 0xFFFFFFFF and yields 0x00000000.
 */
void TstXyModem::emptyInputReturnsTheSeedValues()
{
  QCOMPARE(crc16Of(QByteArray()), quint16(0x0000));
  QCOMPARE(crc32Of(QByteArray()), quint32(0x00000000));
}

//--------------------------------------------------------------------------------------------------
// Block framing
//--------------------------------------------------------------------------------------------------

/**
 * @brief A 128-byte payload frames as SOH | block | ~block | data | CRC16-BE, 133 bytes total.
 */
void TstXyModem::buildBlockFrames128ByteDataWithSoh()
{
  BlockBuilder builder;
  const QByteArray data   = patternPayload(128);
  const QByteArray packet = builder.buildBlock(data, 7);

  QCOMPARE(packet.size(), qsizetype(133));
  QCOMPARE(static_cast<quint8>(packet.at(0)), kSoh);
  QCOMPARE(static_cast<quint8>(packet.at(1)), quint8(0x07));
  QCOMPARE(static_cast<quint8>(packet.at(2)), quint8(0xF8));
  QCOMPARE(packet.mid(3, 128), data);
}

/**
 * @brief A 1024-byte payload switches the header byte to STX. The selection is driven by the
 *        payload size, not by the use1K() flag, so a 1K sender still frames a short block as SOH.
 */
void TstXyModem::buildBlockFrames1024ByteDataWithStx()
{
  BlockBuilder builder;
  const QByteArray data   = patternPayload(1024);
  const QByteArray packet = builder.buildBlock(data, 0);

  QCOMPARE(packet.size(), qsizetype(1029));
  QCOMPARE(static_cast<quint8>(packet.at(0)), kStx);
  QCOMPARE(static_cast<quint8>(packet.at(1)), quint8(0x00));
  QCOMPARE(static_cast<quint8>(packet.at(2)), quint8(0xFF));
  QCOMPARE(packet.mid(3, 1024), data);
}

/**
 * @brief The trailing CRC covers the padded data field only, never the three header bytes.
 */
void TstXyModem::buildBlockCrcMatchesAnIndependentComputation()
{
  BlockBuilder builder;
  const QByteArray small = patternPayload(128);
  const QByteArray large = patternPayload(1024);

  const QByteArray shortPacket = builder.buildBlock(small, 3);
  const quint16 shortCrc       = crc16Of(small);
  QCOMPARE(static_cast<quint8>(shortPacket.at(131)), quint8((shortCrc >> 8) & 0xFF));
  QCOMPARE(static_cast<quint8>(shortPacket.at(132)), quint8(shortCrc & 0xFF));

  const QByteArray longPacket = builder.buildBlock(large, 3);
  const quint16 longCrc       = crc16Of(large);
  QCOMPARE(static_cast<quint8>(longPacket.at(1027)), quint8((longCrc >> 8) & 0xFF));
  QCOMPARE(static_cast<quint8>(longPacket.at(1028)), quint8(longCrc & 0xFF));
}

//--------------------------------------------------------------------------------------------------
// Transfer start
//--------------------------------------------------------------------------------------------------

/**
 * @brief An unopenable path fails immediately and never enters a transfer state, so the device is
 *        left alone instead of receiving a header for a file that does not exist.
 */
void TstXyModem::startTransferOnMissingFileFails()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(dir.filePath(QStringLiteral("no-such-file.bin")));

  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), false);
  QVERIFY(done.at(0).at(1).toString().contains(QStringLiteral("Cannot open")));
  QCOMPARE(writes.size(), qsizetype(0));
  QVERIFY(!sender.isActive());
}

/**
 * @brief A started transfer publishes the total size up front and then stays silent: XMODEM is
 *        receiver-driven, so nothing goes on the wire until the receiver asks for CRC mode.
 */
void TstXyModem::startTransferAnnouncesZeroProgressAndWaits()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(300));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy progress(&sender, &IO::Protocols::Protocol::progressChanged);

  sender.startTransfer(path);

  QVERIFY(sender.isActive());
  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(progress.size(), qsizetype(1));
  QCOMPARE(progress.at(0).at(0).toLongLong(), qint64(0));
  QCOMPARE(progress.at(0).at(1).toLongLong(), qint64(300));

  sender.cancelTransfer();
}

/**
 * @brief The receiver's 'C' selects CRC mode and releases the first data block.
 */
void TstXyModem::crcStartByteTriggersTheFirstBlock()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(300);
  const QString path       = writePayload(dir, QStringLiteral("payload.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));

  QCOMPARE(writes.size(), qsizetype(1));
  const QByteArray packet = writeAt(writes, 0);
  QCOMPARE(packet.size(), qsizetype(133));
  QCOMPARE(static_cast<quint8>(packet.at(0)), kSoh);
  QCOMPARE(static_cast<quint8>(packet.at(1)), quint8(0x01));
  QCOMPARE(packet.mid(3, 128), payload.mid(0, 128));

  sender.cancelTransfer();
}

//--------------------------------------------------------------------------------------------------
// Acknowledgement handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief ACK advances the block counter and immediately sends the next slice of the file.
 */
void TstXyModem::ackAdvancesToTheNextBlock()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(400);
  const QString path       = writePayload(dir, QStringLiteral("payload.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));

  QCOMPARE(writes.size(), qsizetype(2));
  const QByteArray second = writeAt(writes, 1);
  QCOMPARE(static_cast<quint8>(second.at(1)), quint8(0x02));
  QCOMPARE(static_cast<quint8>(second.at(2)), quint8(0xFD));
  QCOMPARE(second.mid(3, 128), payload.mid(128, 128));

  sender.cancelTransfer();
}

/**
 * @brief Once the file is exhausted the ACK for the final block produces a bare EOT.
 */
void TstXyModem::ackOnTheLastBlockSendsEot()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(200));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kAck));

  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(writeAt(writes, 2), byteOf(kEot));
  QVERIFY(sender.isActive());

  sender.cancelTransfer();
}

/**
 * @brief The ACK for the EOT is the only success path: it reports finished(true) with no message.
 */
void TstXyModem::eotAckFinishesSuccessfully()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(200));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kAck));

  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), true);
  QVERIFY(done.at(0).at(1).toString().isEmpty());
  QVERIFY(!sender.isActive());
}

/**
 * @brief A NAK for the EOT resends the EOT rather than restarting the file.
 */
void TstXyModem::eotNakResendsTheEot()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(100));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  QCOMPARE(writeAt(writes, 1), byteOf(kEot));

  sender.processInput(byteOf(kNak));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(writeAt(writes, 2), byteOf(kEot));
  QCOMPARE(done.size(), qsizetype(0));

  sender.processInput(byteOf(kAck));
  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), true);
}

/**
 * @brief The block counter is a single byte: block 256 goes on the wire numbered 0, and its
 *        complement follows it back to 0xFF.
 */
void TstXyModem::blockNumberWrapsAtTheByteBoundary()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(260 * 128));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(QByteArray(259, static_cast<char>(kAck)));

  QCOMPARE(writes.size(), qsizetype(260));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 254).at(1)), quint8(0xFF));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 254).at(2)), quint8(0x00));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 255).at(1)), quint8(0x00));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 255).at(2)), quint8(0xFF));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 256).at(1)), quint8(0x01));

  sender.cancelTransfer();
}

//--------------------------------------------------------------------------------------------------
// Retries
//--------------------------------------------------------------------------------------------------

/**
 * @brief A NAK seeks back and resends the identical packet: same block number, same payload, same
 *        CRC. The NAK branch returns the state to SendingBlocks first, which is what sendBlock()
 *        asserts on: resending from WaitingForAck aborted debug builds and dropped the block in
 *        release (spec 0075, C1).
 */
void TstXyModem::nakResendsTheSameBlock()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(5);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy progress(&sender, &IO::Protocols::Protocol::progressChanged);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kNak));

  QCOMPARE(progress.size(), qsizetype(3));
  QCOMPARE(progress.at(1).at(0).toLongLong(), qint64(128));
  QCOMPARE(progress.at(2).at(0).toLongLong(), qint64(128));
  QVERIFY(sender.isActive());

  QCOMPARE(writes.size(), qsizetype(2));
  QCOMPARE(writeAt(writes, 1), writeAt(writes, 0));
}

/**
 * @brief The retry budget is inclusive: with maxRetries 2 the second NAK aborts, cancelling the
 *        receiver with the five-CAN sequence before reporting the failure. The first NAK's arm
 *        works today -- the resend the first NAK should have produced is the dropped one above.
 */
void TstXyModem::nakBeyondMaxRetriesCancelsTheTransfer()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(2);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kNak));
  sender.processInput(byteOf(kNak));

  QCOMPARE(writeAt(writes, writes.size() - 1), QByteArray(5, static_cast<char>(kCan)));
  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), false);
  QVERIFY(done.at(0).at(1).toString().contains(QStringLiteral("Maximum retries")));
  QVERIFY(!sender.isActive());

  QCOMPARE(writes.size(), qsizetype(3));
}

/**
 * @brief A successful ACK clears the retry counter, so a noisy link that recovers between blocks
 *        never accumulates its way into a spurious abort: a second NAK at budget 2 resends rather
 *        than aborting the transfer. The four writes are block 1, its resend, block 2 and its
 *        resend, so the ACK's advance to block 2 shows at index 2.
 */
void TstXyModem::ackResetsTheRetryCounter()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(2);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kNak));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kNak));

  QCOMPARE(done.size(), qsizetype(0));
  QVERIFY(sender.isActive());
  QCOMPARE(writes.size(), qsizetype(4));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 2).at(1)), quint8(0x02));
  QCOMPARE(writeAt(writes, 3), writeAt(writes, 2));

  sender.cancelTransfer();
}

/**
 * @brief Every recoverable error reports itself through protocolError(), which is what the
 *        controller counts. Matching the English words of the status text counted nothing in any
 *        other locale, and missed the errors whose text never said "NAK" (spec 0075, C3).
 */
void TstXyModem::recoverableErrorsAreReportedTyped()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(5);
  QSignalSpy errors(&sender, &IO::Protocols::Protocol::protocolError);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  QCOMPARE(errors.size(), qsizetype(0));

  sender.processInput(byteOf(kNak));
  QCOMPARE(errors.size(), qsizetype(1));

  sender.processInput(byteOf(kNak));
  QCOMPARE(errors.size(), qsizetype(2));

  sender.processInput(byteOf(kAck));
  QCOMPARE(errors.size(), qsizetype(2));

  sender.cancelTransfer();
}

//--------------------------------------------------------------------------------------------------
// Cancellation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A CAN from the receiver tears the transfer down without echoing a cancel sequence back.
 */
void TstXyModem::receiverCanAbortsTheTransfer()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kCan));

  QCOMPARE(writes.size(), qsizetype(1));
  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), false);
  QVERIFY(done.at(0).at(1).toString().contains(QStringLiteral("Receiver cancelled")));
  QVERIFY(!sender.isActive());
}

/**
 * @brief Cancelling an idle protocol is a no-op: no CAN bytes reach a device that is not in a
 *        transfer, and no spurious failure lands in the UI.
 */
void TstXyModem::cancelTransferOnIdleProtocolIsSilent()
{
  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);
  QSignalSpy status(&sender, &IO::Protocols::Protocol::statusMessage);

  sender.cancelTransfer();

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(done.size(), qsizetype(0));
  QCOMPARE(status.size(), qsizetype(0));
  QVERIFY(!sender.isActive());
}

/**
 * @brief A user cancel emits the five-CAN sequence in one write and reports the failure reason.
 */
void TstXyModem::cancelTransferSendsFiveCanBytes()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.cancelTransfer();

  QCOMPARE(writes.size(), qsizetype(2));
  QCOMPARE(writeAt(writes, 1), QByteArray(5, static_cast<char>(kCan)));
  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), false);
  QVERIFY(done.at(0).at(1).toString().contains(QStringLiteral("cancelled by user")));
  QVERIFY(!sender.isActive());
}

//--------------------------------------------------------------------------------------------------
// Padding and configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief XMODEM has no length field, so the final short block is padded to the block size with
 *        0x1A before the CRC is taken over the padded field.
 */
void TstXyModem::shortFinalBlockIsPaddedWithSubBytes()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(200);
  const QString path       = writePayload(dir, QStringLiteral("payload.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));

  const QByteArray packet = writeAt(writes, 1);
  QCOMPARE(packet.size(), qsizetype(133));
  QCOMPARE(packet.mid(3, 72), payload.mid(128, 72));
  QCOMPARE(packet.mid(75, 56), QByteArray(56, static_cast<char>(0x1A)));
  QCOMPARE(
    crc16Of(packet.mid(3, 128)),
    quint16((static_cast<quint8>(packet.at(131)) << 8) | static_cast<quint8>(packet.at(132))));

  sender.cancelTransfer();
}

/**
 * @brief use1K() renames the protocol and widens every block to 1024 bytes behind an STX header.
 */
void TstXyModem::use1KSwitchesNameAndBlockSize()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(1500);
  const QString path       = writePayload(dir, QStringLiteral("payload.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QCOMPARE(sender.protocolName(), QStringLiteral("XMODEM"));
  QVERIFY(!sender.use1K());

  sender.setUse1K(true);
  QCOMPARE(sender.protocolName(), QStringLiteral("XMODEM-1K"));
  QVERIFY(sender.use1K());

  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));

  const QByteArray packet = writeAt(writes, 0);
  QCOMPARE(packet.size(), qsizetype(1029));
  QCOMPARE(static_cast<quint8>(packet.at(0)), kStx);
  QCOMPARE(packet.mid(3, 1024), payload.mid(0, 1024));

  sender.cancelTransfer();
}

/**
 * @brief Both setters clamp: one retry and one second are the floors a caller cannot go below, so
 *        a zero from a settings file can never disable retrying or spin the timeout timer.
 */
void TstXyModem::retryAndTimeoutSettersClampToTheirFloors()
{
  IO::Protocols::XMODEM sender;
  QCOMPARE(sender.maxRetries(), 10);
  QCOMPARE(sender.timeoutMs(), 10000);

  sender.setMaxRetries(0);
  QCOMPARE(sender.maxRetries(), 1);
  sender.setMaxRetries(-7);
  QCOMPARE(sender.maxRetries(), 1);
  sender.setMaxRetries(4);
  QCOMPARE(sender.maxRetries(), 4);

  sender.setTimeoutMs(0);
  QCOMPARE(sender.timeoutMs(), 1000);
  sender.setTimeoutMs(999);
  QCOMPARE(sender.timeoutMs(), 1000);
  sender.setTimeoutMs(2500);
  QCOMPARE(sender.timeoutMs(), 2500);
}

//--------------------------------------------------------------------------------------------------
// Input handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Line noise and premature ACK/NAK bytes are dropped while the sender waits for the start
 *        byte, and the transfer still opens on the 'C' that follows them.
 */
void TstXyModem::unexpectedBytesAreIgnoredWhileWaitingForStart()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(300));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(QByteArray("noise"));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kNak));
  sender.processInput(byteOf(kEot));

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(done.size(), qsizetype(0));
  QVERIFY(sender.isActive());

  sender.processInput(byteOf(kCrcStart));
  QCOMPARE(writes.size(), qsizetype(1));

  sender.cancelTransfer();
}

/**
 * @brief Bytes that arrive on an idle protocol take the SS_ASSERT recovery branch and are dropped:
 *        the trailing bytes of a finished transfer must not restart the state machine.
 */
void TstXyModem::inputOnIdleProtocolIsIgnored()
{
  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  sender.processInput(QByteArray());

  QCOMPARE(writes.size(), qsizetype(0));
  QCOMPARE(done.size(), qsizetype(0));
  QVERIFY(!sender.isActive());
}

/**
 * @brief The state machine is byte-driven, so a fragmented serial read and one coalesced buffer
 *        have to produce the same bytes on the wire.
 */
void TstXyModem::bytewiseDeliveryMatchesBulkDelivery()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(300));
  QVERIFY(!path.isEmpty());

  QByteArray script;
  script.append(static_cast<char>(kCrcStart));
  script.append(4, static_cast<char>(kAck));

  IO::Protocols::XMODEM bulk;
  QSignalSpy bulkWrites(&bulk, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy bulkDone(&bulk, &IO::Protocols::Protocol::finished);
  bulk.startTransfer(path);
  bulk.processInput(script);

  IO::Protocols::XMODEM drip;
  QSignalSpy dripWrites(&drip, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy dripDone(&drip, &IO::Protocols::Protocol::finished);
  drip.startTransfer(path);
  for (qsizetype i = 0; i < script.size(); ++i)
    drip.processInput(script.mid(i, 1));

  QCOMPARE(bulkWrites.size(), dripWrites.size());
  QCOMPARE(bulkWrites.size(), qsizetype(4));
  for (qsizetype i = 0; i < bulkWrites.size(); ++i)
    QCOMPARE(writeAt(dripWrites, i), writeAt(bulkWrites, i));

  QCOMPARE(bulkDone.size(), qsizetype(1));
  QCOMPARE(dripDone.size(), qsizetype(1));
  QVERIFY(bulkDone.at(0).at(0).toBool());
  QVERIFY(dripDone.at(0).at(0).toBool());
}

//--------------------------------------------------------------------------------------------------
// Timeout
//--------------------------------------------------------------------------------------------------

/**
 * @brief The one test that waits on a real timer. With the retry budget at its floor the first
 *        expiry is terminal, so the wait costs a single timeout period.
 */
void TstXyModem::timeoutWithoutRetriesLeftAbortsTheTransfer()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(300));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(1);
  sender.setTimeoutMs(1000);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  QVERIFY(done.wait(10000));

  QCOMPARE(done.size(), qsizetype(1));
  QCOMPARE(done.at(0).at(0).toBool(), false);
  QVERIFY(done.at(0).at(1).toString().contains(QStringLiteral("Timeout")));
  QCOMPARE(writes.size(), qsizetype(1));
  QCOMPARE(writeAt(writes, 0), QByteArray(5, static_cast<char>(kCan)));
  QVERIFY(!sender.isActive());
}

/**
 * @brief With retries left, a timeout mid-transfer puts the block back on the wire, and sendBlock()
 *        rearms the one-shot timer as it goes. Resending from WaitingForAck used to be rejected,
 *        which left the transfer stalled with no finished() and a UI still showing progress.
 */
void TstXyModem::timeoutMidTransferRearmsOrReports()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("payload.bin"), patternPayload(400));
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  sender.setMaxRetries(3);
  sender.setTimeoutMs(1000);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  QCOMPARE(writes.size(), qsizetype(1));
  QTest::qWait(2500);

  QCOMPARE(done.size(), qsizetype(0));
  QVERIFY(sender.isActive());

  QVERIFY(writes.size() > 1);

  sender.cancelTransfer();
}

//--------------------------------------------------------------------------------------------------
// End-to-end
//--------------------------------------------------------------------------------------------------

/**
 * @brief The payload carried by the spied packets, stripped of framing and trailing pad, has to be
 *        the source file byte for byte. Every packet's CRC is re-derived from its own data field.
 */
void TstXyModem::fullTransferReproducesTheSourceFile()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(700);
  const QString path       = writePayload(dir, QStringLiteral("payload.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::XMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy progress(&sender, &IO::Protocols::Protocol::progressChanged);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  for (int i = 0; i < 32 && done.isEmpty(); ++i)
    sender.processInput(byteOf(kAck));

  QCOMPARE(done.size(), qsizetype(1));
  QVERIFY(done.at(0).at(0).toBool());
  QCOMPARE(writes.size(), qsizetype(7));
  QCOMPARE(writeAt(writes, 6), byteOf(kEot));

  QByteArray received;
  for (qsizetype i = 0; i < writes.size() - 1; ++i) {
    const QByteArray packet   = writeAt(writes, i);
    const QByteArray dataPart = packet.mid(3, packet.size() - 5);
    const quint16 crc         = crc16Of(dataPart);
    QCOMPARE(static_cast<quint8>(packet.at(1)), quint8((i + 1) & 0xFF));
    QCOMPARE(static_cast<quint8>(packet.at(packet.size() - 2)), quint8((crc >> 8) & 0xFF));
    QCOMPARE(static_cast<quint8>(packet.at(packet.size() - 1)), quint8(crc & 0xFF));
    received.append(dataPart);
  }

  QCOMPARE(received.size(), qsizetype(768));
  received.truncate(payload.size());
  QCOMPARE(received, payload);
  QCOMPARE(progress.last().at(0).toLongLong(), qint64(700));
}

//--------------------------------------------------------------------------------------------------
// YMODEM
//--------------------------------------------------------------------------------------------------

/**
 * @brief YMODEM is XMODEM-1K plus a batch header, so the 1K flag is on from construction.
 */
void TstXyModem::ymodemDefaultsToOneKilobyteBlocks()
{
  IO::Protocols::YMODEM sender;

  QCOMPARE(sender.protocolName(), QStringLiteral("YMODEM"));
  QVERIFY(sender.use1K());
  QVERIFY(!sender.isActive());
}

/**
 * @brief Block 0 is the batch header: a 128-byte SOH block numbered 0 holding the NUL-terminated
 *        base name followed by the decimal file size, zero-filled to the end.
 */
void TstXyModem::ymodemHeaderBlockCarriesNameAndSize()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("telemetry.bin"), patternPayload(1500));
  QVERIFY(!path.isEmpty());

  IO::Protocols::YMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  QVERIFY(sender.isActive());
  sender.processInput(byteOf(kCrcStart));

  QByteArray expected;
  expected.append(QByteArray("telemetry.bin"));
  expected.append('\0');
  expected.append(QByteArray::number(1500));
  expected.append('\0');
  expected.append(QByteArray(128 - expected.size(), '\0'));

  QCOMPARE(writes.size(), qsizetype(1));
  const QByteArray header = writeAt(writes, 0);
  QCOMPARE(header.size(), qsizetype(133));
  QCOMPARE(static_cast<quint8>(header.at(0)), kSoh);
  QCOMPARE(static_cast<quint8>(header.at(1)), quint8(0x00));
  QCOMPARE(static_cast<quint8>(header.at(2)), quint8(0xFF));
  QCOMPARE(header.mid(3, 128), expected);

  sender.cancelTransfer();
}

/**
 * @brief The full batch handshake: header, its ACK, a second 'C' for the data phase, the 1K data
 *        blocks, two EOTs, and the end-of-batch header the receiver asks for with a third 'C'.
 */
void TstXyModem::ymodemRunsTheFullBatchHandshake()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QByteArray payload = patternPayload(1500);
  const QString path       = writePayload(dir, QStringLiteral("telemetry.bin"), payload);
  QVERIFY(!path.isEmpty());

  IO::Protocols::YMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  QCOMPARE(writes.size(), qsizetype(1));

  sender.processInput(byteOf(kCrcStart));
  QCOMPARE(writes.size(), qsizetype(2));
  QCOMPARE(writeAt(writes, 1).size(), qsizetype(1029));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 1).at(0)), kStx);
  QCOMPARE(static_cast<quint8>(writeAt(writes, 1).at(1)), quint8(0x01));

  sender.processInput(byteOf(kAck));
  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 2).at(1)), quint8(0x02));

  sender.processInput(byteOf(kAck));
  QCOMPARE(writeAt(writes, 3), byteOf(kEot));
  sender.processInput(byteOf(kNak));
  QCOMPARE(writeAt(writes, 4), byteOf(kEot));
  sender.processInput(byteOf(kAck));
  QCOMPARE(writes.size(), qsizetype(5));

  sender.processInput(byteOf(kCrcStart));
  QCOMPARE(writes.size(), qsizetype(6));
  sender.processInput(byteOf(kAck));

  QByteArray received = writeAt(writes, 1).mid(3, 1024) + writeAt(writes, 2).mid(3, 1024);
  received.truncate(payload.size());
  QCOMPARE(received, payload);
  QCOMPARE(done.size(), qsizetype(1));
  QVERIFY(done.at(0).at(0).toBool());
  QVERIFY(!sender.isActive());
}

/**
 * @brief The end-of-batch marker is a block 0 whose 128-byte payload is entirely zero, which makes
 *        its CRC zero too. It is what tells the receiver no further file follows.
 */
void TstXyModem::ymodemEndOfBatchBlockIsAllZeroes()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("telemetry.bin"), patternPayload(1500));
  QVERIFY(!path.isEmpty());

  QByteArray script;
  script.append(static_cast<char>(kCrcStart));
  script.append(static_cast<char>(kAck));
  script.append(static_cast<char>(kCrcStart));
  script.append(2, static_cast<char>(kAck));
  script.append(static_cast<char>(kNak));
  script.append(static_cast<char>(kAck));
  script.append(static_cast<char>(kCrcStart));
  script.append(static_cast<char>(kAck));

  IO::Protocols::YMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);
  QSignalSpy done(&sender, &IO::Protocols::Protocol::finished);

  sender.startTransfer(path);
  sender.processInput(script);

  QCOMPARE(writes.size(), qsizetype(6));
  const QByteArray marker = writeAt(writes, 5);
  QCOMPARE(marker.size(), qsizetype(133));
  QCOMPARE(static_cast<quint8>(marker.at(0)), kSoh);
  QCOMPARE(static_cast<quint8>(marker.at(1)), quint8(0x00));
  QCOMPARE(marker.mid(3, 128), QByteArray(128, '\0'));
  QCOMPARE(static_cast<quint8>(marker.at(131)), quint8(0x00));
  QCOMPARE(static_cast<quint8>(marker.at(132)), quint8(0x00));
  QCOMPARE(done.size(), qsizetype(1));
  QVERIFY(done.at(0).at(0).toBool());
}

/**
 * @brief YMODEM keeps XMODEM's resend contract in its own data-phase handler: a NAK reseeks and
 *        repeats the identical 1029-byte packet.
 */
void TstXyModem::ymodemNakResendsTheSameDataBlock()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("telemetry.bin"), patternPayload(3000));
  QVERIFY(!path.isEmpty());

  IO::Protocols::YMODEM sender;
  sender.setMaxRetries(4);
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kNak));

  QCOMPARE(writes.size(), qsizetype(3));
  QCOMPARE(writeAt(writes, 2), writeAt(writes, 1));
  QCOMPARE(static_cast<quint8>(writeAt(writes, 2).at(1)), quint8(0x01));

  sender.cancelTransfer();
}

/**
 * @brief cancelTransfer() resets the batch state as well as the base state, so bytes arriving
 *        after a cancel cannot resume the batch machine on a closed file.
 */
void TstXyModem::ymodemCancelResetsTheBatchState()
{
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writePayload(dir, QStringLiteral("telemetry.bin"), patternPayload(3000));
  QVERIFY(!path.isEmpty());

  IO::Protocols::YMODEM sender;
  QSignalSpy writes(&sender, &IO::Protocols::Protocol::writeRequested);

  sender.startTransfer(path);
  sender.processInput(byteOf(kCrcStart));
  sender.processInput(byteOf(kAck));
  sender.processInput(byteOf(kCrcStart));
  sender.cancelTransfer();
  QVERIFY(!sender.isActive());

  const qsizetype settled = writes.size();
  sender.processInput(byteOf(kAck));

  QCOMPARE(writes.size(), settled);
}

QTEST_GUILESS_MAIN(TstXyModem)

#include "tst_xymodem.moc"
