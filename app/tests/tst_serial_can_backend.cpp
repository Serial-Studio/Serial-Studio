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

#include <QTest>

#include "IO/Drivers/CANBus/SeeedCanBackend.h"
#include "IO/Drivers/CANBus/SerialCanBackendBase.h"
#include "IO/Drivers/CANBus/SlcanBackend.h"

// Everything under test here is pure: the two wire decoders and the two policies the shared base
// applies to bytes it has already read. A real QSerialPort is deliberately absent -- an adapter
// cannot be plugged into a unit suite -- so the unplug path is pinned through isFatalSerialError(),
// which is the whole decision the error handler makes.

/**
 * @brief Wire decoders and buffer policy of the two serial CAN backends and their shared base.
 */
class TstSerialCanBackend : public QObject {
  Q_OBJECT

private slots:
  void slcanDecodesAStandardFrame();
  void slcanDecodesAnExtendedFrame();
  void slcanRejectsANonHexIdentifier();
  void slcanRejectsATruncatedPayload();
  void slcanMapsKnownBitratesOnly();

  void seeedDecodesAStandardFrame();
  void seeedResyncsOnAMisalignedByte();
  void seeedNeedsMoreOnAPartialPacket();
  void seeedMapsKnownBitratesOnly();

  void fatalSerialErrorsFlipTheState();
  void bufferIsBoundedAndCountsDrops();
  void bufferKeepsContentUnderTheCeiling();
};

//--------------------------------------------------------------------------------------------------
// slcan / LAWICEL decoder
//--------------------------------------------------------------------------------------------------

/**
 * @brief A standard-format data token decodes into its identifier and payload.
 */
void TstSerialCanBackend::slcanDecodesAStandardFrame()
{
  QCanBusFrame frame;
  QVERIFY(IO::Drivers::SlcanBackend::parseToken(QByteArrayLiteral("t1232DEAD"), frame));

  QCOMPARE(frame.frameId(), 0x123u);
  QVERIFY(!frame.hasExtendedFrameFormat());
  QCOMPARE(frame.payload(), QByteArray::fromHex("dead"));
}

/**
 * @brief An extended-format token carries a 29-bit identifier and sets the extended flag.
 */
void TstSerialCanBackend::slcanDecodesAnExtendedFrame()
{
  QCanBusFrame frame;
  QVERIFY(IO::Drivers::SlcanBackend::parseToken(QByteArrayLiteral("T12345678111"), frame));

  QCOMPARE(frame.frameId(), 0x12345678u);
  QVERIFY(frame.hasExtendedFrameFormat());
  QCOMPARE(static_cast<int>(frame.payload().size()), 1);
}

/**
 * @brief A non-hex identifier is refused instead of publishing a fabricated frame on id 0x000:
 *        the identifier and the DLC used to share one conversion flag, and the DLC won.
 */
void TstSerialCanBackend::slcanRejectsANonHexIdentifier()
{
  QCanBusFrame frame;
  QVERIFY(!IO::Drivers::SlcanBackend::parseToken(QByteArrayLiteral("tZZZ0"), frame));
  QVERIFY(!IO::Drivers::SlcanBackend::parseToken(QByteArrayLiteral("tG1F1AA"), frame));
}

/**
 * @brief A token whose payload is shorter than its DLC is refused rather than zero-padded.
 */
void TstSerialCanBackend::slcanRejectsATruncatedPayload()
{
  QCanBusFrame frame;
  QVERIFY(!IO::Drivers::SlcanBackend::parseToken(QByteArrayLiteral("t1234AABB"), frame));
}

/**
 * @brief Only the LAWICEL rate table maps to a command index; anything else is refused, which is
 *        what stops the driver from opening a channel the adapter will never put on the bus.
 */
void TstSerialCanBackend::slcanMapsKnownBitratesOnly()
{
  QCOMPARE(IO::Drivers::SlcanBackend::bitrateIndex(500000), 6);
  QCOMPARE(IO::Drivers::SlcanBackend::bitrateIndex(1000000), 8);
  QCOMPARE(IO::Drivers::SlcanBackend::bitrateIndex(33333), -1);
}

//--------------------------------------------------------------------------------------------------
// Seeed / Waveshare decoder
//--------------------------------------------------------------------------------------------------

/**
 * @brief A well-formed variable-length packet decodes and reports how many bytes it consumed.
 */
void TstSerialCanBackend::seeedDecodesAStandardFrame()
{
  QByteArray packet;
  packet.append(static_cast<char>(0xaa));
  packet.append(static_cast<char>(0xc2));
  packet.append(static_cast<char>(0x34));
  packet.append(static_cast<char>(0x01));
  packet.append(static_cast<char>(0xde));
  packet.append(static_cast<char>(0xad));
  packet.append(static_cast<char>(0x55));

  int consumed = 0;
  QCanBusFrame frame;
  const auto result = IO::Drivers::SeeedCanBackend::decodePacket(packet, frame, consumed);

  QCOMPARE(result, IO::Drivers::SeeedCanBackend::Parse::Frame);
  QCOMPARE(consumed, static_cast<int>(packet.size()));
  QCOMPARE(frame.frameId(), 0x134u);
  QCOMPARE(frame.payload(), QByteArray::fromHex("dead"));
}

/**
 * @brief A type byte that is not a data packet consumes one byte and asks the caller to resync,
 *        which is what keeps the drain loop bounded on a noisy line.
 */
void TstSerialCanBackend::seeedResyncsOnAMisalignedByte()
{
  QByteArray packet;
  packet.append(static_cast<char>(0xaa));
  packet.append(static_cast<char>(0x11));
  packet.append(static_cast<char>(0x22));

  int consumed = 0;
  QCanBusFrame frame;
  const auto result = IO::Drivers::SeeedCanBackend::decodePacket(packet, frame, consumed);

  QCOMPARE(result, IO::Drivers::SeeedCanBackend::Parse::Resync);
  QCOMPARE(consumed, 1);
}

/**
 * @brief A packet that has not fully arrived leaves the buffer untouched.
 */
void TstSerialCanBackend::seeedNeedsMoreOnAPartialPacket()
{
  QByteArray packet;
  packet.append(static_cast<char>(0xaa));
  packet.append(static_cast<char>(0xc2));
  packet.append(static_cast<char>(0x34));

  int consumed = 0;
  QCanBusFrame frame;
  const auto result = IO::Drivers::SeeedCanBackend::decodePacket(packet, frame, consumed);

  QCOMPARE(result, IO::Drivers::SeeedCanBackend::Parse::NeedMore);
  QCOMPARE(consumed, 0);
}

/**
 * @brief The analyzer's rate table is closed too: an unsupported rate maps to code 0, which the
 *        backend turns into a refused open.
 */
void TstSerialCanBackend::seeedMapsKnownBitratesOnly()
{
  QCOMPARE(IO::Drivers::SeeedCanBackend::bitrateCode(500000), std::uint8_t(0x03));
  QCOMPARE(IO::Drivers::SeeedCanBackend::bitrateCode(10000), std::uint8_t(0x0b));
  QCOMPARE(IO::Drivers::SeeedCanBackend::bitrateCode(33333), std::uint8_t(0x00));
}

//--------------------------------------------------------------------------------------------------
// Shared base policies
//--------------------------------------------------------------------------------------------------

/**
 * @brief An unplugged adapter raises ResourceError, and that is what has to end the session: the
 *        backends never wired errorOccurred at all, so the state stayed Connected on a dead bus.
 */
void TstSerialCanBackend::fatalSerialErrorsFlipTheState()
{
  using Base = IO::Drivers::SerialCanBackendBase;

  QVERIFY(Base::isFatalSerialError(QSerialPort::ResourceError));
  QVERIFY(Base::isFatalSerialError(QSerialPort::DeviceNotFoundError));
  QVERIFY(Base::isFatalSerialError(QSerialPort::PermissionError));

  QVERIFY(!Base::isFatalSerialError(QSerialPort::NoError));
  QVERIFY(!Base::isFatalSerialError(QSerialPort::TimeoutError));
}

/**
 * @brief An adapter that never emits a terminator cannot grow the buffer without bound: the whole
 *        buffer is dropped and counted, because half a packet decodes into noise.
 */
void TstSerialCanBackend::bufferIsBoundedAndCountsDrops()
{
  QByteArray buffer;
  quint64 drops = 0;

  for (int i = 0; i < 32; ++i)
    IO::Drivers::SerialCanBackendBase::appendBounded(buffer, QByteArray(8192, 'x'), drops);

  QVERIFY(buffer.size() <= 65536);
  QVERIFY(drops > 0);

  IO::Drivers::SerialCanBackendBase::appendBounded(buffer, QByteArray(200000, 'y'), drops);
  QVERIFY(buffer.isEmpty());
}

/**
 * @brief Ordinary traffic accumulates untouched, so the bound costs the decoder nothing.
 */
void TstSerialCanBackend::bufferKeepsContentUnderTheCeiling()
{
  QByteArray buffer;
  quint64 drops = 0;

  IO::Drivers::SerialCanBackendBase::appendBounded(buffer, QByteArrayLiteral("t1230"), drops);
  IO::Drivers::SerialCanBackendBase::appendBounded(buffer, QByteArrayLiteral("\r"), drops);

  QCOMPARE(buffer, QByteArrayLiteral("t1230\r"));
  QCOMPARE(drops, quint64(0));
}

QTEST_GUILESS_MAIN(TstSerialCanBackend)

#include "tst_serial_can_backend.moc"
