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
#include <vector>

#include "IO/Drivers/Audio/PlaybackRing.h"

// The ring is the seam between the GUI thread and the real-time audio callback, so the properties
// that matter are the ones the callback depends on: it always fills the buffer it was handed, it
// never blocks, and what it plays is exactly what was written, in order, across the wrap point.

/**
 * @brief SPSC byte ring behind audio playback: ordering, wrap, underrun and overflow accounting.
 */
class TstPlaybackRing : public QObject {
  Q_OBJECT

private slots:
  void startsEmpty();
  void roundTripsBytesInOrder();
  void readZeroFillsAndCountsUnderruns();
  void refusesAWriteThatDoesNotFit();
  void wrapsAroundTheEnd();
  void resetDropsBufferedAudio();
};

/**
 * @brief A fresh ring holds nothing and has counted nothing.
 */
void TstPlaybackRing::startsEmpty()
{
  IO::Drivers::PlaybackRing ring(64);

  QCOMPARE(ring.capacity(), qsizetype(64));
  QCOMPARE(ring.available(), qsizetype(0));
  QCOMPARE(ring.freeSpace(), qsizetype(64));
  QCOMPARE(ring.underruns(), quint64(0));
  QCOMPARE(ring.overflows(), quint64(0));
}

/**
 * @brief What the producer wrote is what the consumer reads, byte for byte and in order.
 */
void TstPlaybackRing::roundTripsBytesInOrder()
{
  IO::Drivers::PlaybackRing ring(64);

  const QByteArray payload = QByteArrayLiteral("0123456789");
  QVERIFY(ring.write(payload.constData(), payload.size()));
  QCOMPARE(ring.available(), payload.size());

  std::vector<char> out(static_cast<std::size_t>(payload.size()));
  QCOMPARE(ring.read(out.data(), payload.size()), payload.size());
  QCOMPARE(QByteArray(out.data(), payload.size()), payload);
  QCOMPARE(ring.available(), qsizetype(0));
  QCOMPARE(ring.underruns(), quint64(0));
}

/**
 * @brief A callback asking for more than was buffered still gets a full buffer: the rest is
 *        silence, and the shortfall is counted rather than reported per sample.
 */
void TstPlaybackRing::readZeroFillsAndCountsUnderruns()
{
  IO::Drivers::PlaybackRing ring(64);

  const QByteArray payload = QByteArrayLiteral("ab");
  QVERIFY(ring.write(payload.constData(), payload.size()));

  std::vector<char> out(8, 'x');
  QCOMPARE(ring.read(out.data(), 8), qsizetype(2));
  QCOMPARE(QByteArray(out.data(), 8), QByteArrayLiteral("ab\0\0\0\0\0\0"));
  QCOMPARE(ring.underruns(), quint64(1));
}

/**
 * @brief A write larger than the free space is refused WHOLE and counted: half a sample frame
 *        played out is noise, so a partial write is never taken.
 */
void TstPlaybackRing::refusesAWriteThatDoesNotFit()
{
  IO::Drivers::PlaybackRing ring(8);

  const QByteArray fits = QByteArrayLiteral("12345678");
  QVERIFY(ring.write(fits.constData(), fits.size()));
  QCOMPARE(ring.freeSpace(), qsizetype(0));

  QVERIFY(!ring.write(fits.constData(), 1));
  QCOMPARE(ring.overflows(), quint64(1));
  QCOMPARE(ring.available(), qsizetype(8));
}

/**
 * @brief Content that straddles the end of the buffer is read back intact.
 */
void TstPlaybackRing::wrapsAroundTheEnd()
{
  IO::Drivers::PlaybackRing ring(8);

  const QByteArray first = QByteArrayLiteral("123456");
  QVERIFY(ring.write(first.constData(), first.size()));

  std::vector<char> drain(6);
  QCOMPARE(ring.read(drain.data(), 6), qsizetype(6));

  const QByteArray second = QByteArrayLiteral("ABCDE");
  QVERIFY(ring.write(second.constData(), second.size()));

  std::vector<char> out(5);
  QCOMPARE(ring.read(out.data(), 5), qsizetype(5));
  QCOMPARE(QByteArray(out.data(), 5), second);
}

/**
 * @brief reset() drops what was buffered, which is what open() and close() need so a new session
 *        never plays the previous one's tail.
 */
void TstPlaybackRing::resetDropsBufferedAudio()
{
  IO::Drivers::PlaybackRing ring(16);

  const QByteArray payload = QByteArrayLiteral("abcd");
  QVERIFY(ring.write(payload.constData(), payload.size()));
  ring.reset();

  QCOMPARE(ring.available(), qsizetype(0));
  QCOMPARE(ring.freeSpace(), qsizetype(16));
}

QTEST_APPLESS_MAIN(TstPlaybackRing)

#include "tst_playback_ring.moc"
