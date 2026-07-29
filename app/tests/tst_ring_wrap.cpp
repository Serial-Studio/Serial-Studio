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
#include <QTest>

#include "IO/CircularBuffer.h"

// Capacity shared by the wrap-adjacent fixtures below: large enough to host the pattern-scan
// fixtures, small enough that a modest starting offset forces the payload across the wrap point.
static constexpr qsizetype kScanCapacity = 32;

/**
 * @brief Writes @p content into @p buffer starting at ring offset @p offset, so a test can pick
 *        between the linear fast lane (offset 0) and the wrap-straddling lane by data alone. Same
 *        idiom as tst_circular_buffer.cpp: prime with filler, discard it, then append the payload.
 */
static void primeAtOffset(IO::CircularBuffer<QByteArray, char>& buffer,
                          qsizetype offset,
                          const QByteArray& content)
{
  if (offset > 0) {
    buffer.append(QByteArray(offset, 'x'));
    buffer.discard(offset);
  }

  buffer.append(content);
}

/**
 * @brief Wrap-adjacent coverage for IO::CircularBuffer that tst_circular_buffer.cpp does not
 *        exercise: operator[], pos-mid-scan resume on wrapped content, setCapacity() while
 *        wrapped, and peekRangeInto() at a nonzero offset that itself straddles the wrap point.
 *        Qt Test runs private slots in declaration order; every slot below builds its own buffer.
 */
class TstRingWrap : public QObject {
  Q_OBJECT

private slots:
  void indexOperatorOverLinearContent();

  void indexOperatorMatchesPeekRangeAcrossTheWrap_data();
  void indexOperatorMatchesPeekRangeAcrossTheWrap();

  void indexOperatorOutOfRangeReturnsTheStorageOrigin();

  void findPatternKMPResumesFromPosAcrossTheWrap_data();
  void findPatternKMPResumesFromPosAcrossTheWrap();

  void findFirstOfPatternsMatchStraddlingTheWrapBoundary();

  void setCapacityDropsWrappedContentThenPostReconfigureWrapScansCorrectly();

  void peekRangeIntoSpansTheWrapAtNonzeroOffset_data();
  void peekRangeIntoSpansTheWrapAtNonzeroOffset();

  void peekRangeIntoAcrossTheWrapReusesTheCallerBuffer();
};

//--------------------------------------------------------------------------------------------------
// operator[]
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every index of a purely linear buffer must read back the byte at that logical offset.
 */
void TstRingWrap::indexOperatorOverLinearContent()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  const QByteArray content("abcdef");
  buffer.append(content);

  for (qsizetype i = 0; i < content.size(); ++i)
    QCOMPARE(buffer[i], content.at(i));
}

/**
 * @brief Fixtures with the wrap point landing at different logical indices, so the boundary itself
 *        (the last index of the first segment and the first index of the second) is exercised as
 *        well as the interior of both segments.
 */
void TstRingWrap::indexOperatorMatchesPeekRangeAcrossTheWrap_data()
{
  QTest::addColumn<qsizetype>("offset");
  QTest::addColumn<QByteArray>("content");

  const QByteArray content("0123456789AB");

  QTest::newRow("wrap after eleven of twelve bytes") << qsizetype(21) << content;
  QTest::newRow("wrap after seven of twelve bytes") << qsizetype(25) << content;
  QTest::newRow("wrap after four of twelve bytes") << qsizetype(28) << content;
  QTest::newRow("wrap after a single byte") << qsizetype(31) << content;
}

void TstRingWrap::indexOperatorMatchesPeekRangeAcrossTheWrap()
{
  QFETCH(qsizetype, offset);
  QFETCH(QByteArray, content);

  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, offset, content);
  QCOMPARE(buffer.size(), content.size());

  for (qsizetype i = 0; i < content.size(); ++i) {
    QCOMPARE(buffer[i], content.at(i));
    QCOMPARE(buffer.peekRange(i, 1), QByteArray(1, content.at(i)));
  }
}

/**
 * @brief An out-of-range index (negative or >= size) does not assert or throw: it silently returns
 *        the byte at physical storage index 0, whatever that happens to hold. The buffer is primed
 *        with a discard so its logical index 0 and physical index 0 differ, proving the sentinel is
 *        genuinely the storage origin and not a coincidental match with a fresh, unrotated buffer.
 */
void TstRingWrap::indexOperatorOutOfRangeReturnsTheStorageOrigin()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray(5, 'x'));
  buffer.discard(5);
  buffer.append(QByteArray("hello"));

  QCOMPARE(buffer[0], 'h');
  QCOMPARE(buffer[-1], 'x');
  QCOMPARE(buffer[100], 'x');
  QCOMPARE(&buffer[-1], &buffer[100]);
}

//--------------------------------------------------------------------------------------------------
// pos-mid-scan resume on wrapped buffers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Two fixtures, each holding the same pattern twice (once in the first ring segment, once
 *        in the second), scanned with pos landing before the first occurrence, exactly on an
 *        occurrence, before/at/after the wrap boundary, and past the last occurrence. "short"
 *        stays under the 8-byte memcmp-lane threshold, "long" is above it -- on a wrapped buffer
 *        both funnel into kmpScanWrap (the memchr-anchored short lane only runs when the storage
 *        is linear), so this also documents that the short/long split is a linear-only fast path.
 */
void TstRingWrap::findPatternKMPResumesFromPosAcrossTheWrap_data()
{
  QTest::addColumn<qsizetype>("offset");
  QTest::addColumn<QByteArray>("content");
  QTest::addColumn<QByteArray>("pattern");
  QTest::addColumn<int>("pos");
  QTest::addColumn<int>("expected");

  const QByteArray shortContent = QByteArray("xxxMARKxxx") + QByteArray("xxxxMARKxx");
  const QByteArray shortPattern("MARK");

  QTest::newRow("short: pos 0 finds the first occurrence")
    << qsizetype(22) << shortContent << shortPattern << 0 << 3;
  QTest::newRow("short: pos on the first occurrence's start")
    << qsizetype(22) << shortContent << shortPattern << 3 << 3;
  QTest::newRow("short: pos inside the first occurrence skips it")
    << qsizetype(22) << shortContent << shortPattern << 4 << 14;
  QTest::newRow("short: pos on the last index before the wrap")
    << qsizetype(22) << shortContent << shortPattern << 9 << 14;
  QTest::newRow("short: pos exactly on the wrap boundary")
    << qsizetype(22) << shortContent << shortPattern << 10 << 14;
  QTest::newRow("short: pos inside the second segment before the second occurrence")
    << qsizetype(22) << shortContent << shortPattern << 13 << 14;
  QTest::newRow("short: pos on the second occurrence's start")
    << qsizetype(22) << shortContent << shortPattern << 14 << 14;
  QTest::newRow("short: pos inside the second occurrence finds nothing")
    << qsizetype(22) << shortContent << shortPattern << 15 << -1;
  QTest::newRow("short: pos at the end of the content finds nothing")
    << qsizetype(22) << shortContent << shortPattern << 20 << -1;

  const QByteArray longContent = QByteArray("xxxx") + QByteArray("NEEDLE123") + QByteArray("x")
                               + QByteArray("xxxxxx") + QByteArray("NEEDLE123") + QByteArray("xx");
  const QByteArray longPattern("NEEDLE123");

  QTest::newRow("long: pos 0 finds the first occurrence")
    << qsizetype(18) << longContent << longPattern << 0 << 4;
  QTest::newRow("long: pos on the first occurrence's start")
    << qsizetype(18) << longContent << longPattern << 4 << 4;
  QTest::newRow("long: pos inside the first occurrence skips it")
    << qsizetype(18) << longContent << longPattern << 5 << 20;
  QTest::newRow("long: pos on the last byte of the first occurrence")
    << qsizetype(18) << longContent << longPattern << 12 << 20;
  QTest::newRow("long: pos on the last index before the wrap")
    << qsizetype(18) << longContent << longPattern << 13 << 20;
  QTest::newRow("long: pos exactly on the wrap boundary")
    << qsizetype(18) << longContent << longPattern << 14 << 20;
  QTest::newRow("long: pos inside the second segment before the second occurrence")
    << qsizetype(18) << longContent << longPattern << 19 << 20;
  QTest::newRow("long: pos on the second occurrence's start")
    << qsizetype(18) << longContent << longPattern << 20 << 20;
  QTest::newRow("long: pos inside the second occurrence finds nothing")
    << qsizetype(18) << longContent << longPattern << 21 << -1;
  QTest::newRow("long: pos at the end of the content finds nothing")
    << qsizetype(18) << longContent << longPattern << 31 << -1;
}

void TstRingWrap::findPatternKMPResumesFromPosAcrossTheWrap()
{
  QFETCH(qsizetype, offset);
  QFETCH(QByteArray, content);
  QFETCH(QByteArray, pattern);
  QFETCH(int, pos);
  QFETCH(int, expected);

  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, offset, content);
  QCOMPARE(buffer.size(), content.size());

  QCOMPARE(buffer.findPatternKMP(pattern, pos), expected);
  QCOMPARE(buffer.findPatternKMP(pattern, buffer.buildKMPTable(pattern), pos), expected);
}

/**
 * @brief findFirstOfPatterns() has no pos parameter, so the resume semantics above do not apply to
 *        it; what is untested is a match whose two bytes straddle the physical wrap boundary
 *        itself. The earliest position must still win regardless of which pattern in the list
 *        produced it, and the reported patternIndex must track the list order, not which side of
 *        the wrap the match sits on.
 */
void TstRingWrap::findFirstOfPatternsMatchStraddlingTheWrapBoundary()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 30, QByteArray("pqrstuvwxy"));
  QCOMPARE(buffer.size(), qsizetype(10));

  const auto straddling = buffer.findFirstOfPatterns({QByteArray("qr"), QByteArray("tu")});
  QCOMPARE(straddling.position, 1);
  QCOMPARE(straddling.patternIndex, 0);

  const auto reordered = buffer.findFirstOfPatterns({QByteArray("tu"), QByteArray("qr")});
  QCOMPARE(reordered.position, 1);
  QCOMPARE(reordered.patternIndex, 1);
}

//--------------------------------------------------------------------------------------------------
// setCapacity() while wrapped content is buffered
//--------------------------------------------------------------------------------------------------

/**
 * @brief setCapacity() clears before resizing (per tst_circular_buffer.cpp), but that contract is
 *        only proven there from a linear starting state. This drives the ring into a wrapped state
 *        first, grows through setCapacity(), confirms the old wrapped bytes are unreachable (not
 *        just that size() reads zero), then shrinks back down and forces a brand-new wrap in the
 *        reconfigured buffer to prove the scan paths are not left confused by the old physical
 *        layout.
 */
void TstRingWrap::setCapacityDropsWrappedContentThenPostReconfigureWrapScansCorrectly()
{
  IO::CircularBuffer<QByteArray, char> buffer(8);

  buffer.append(QByteArray("123456"));
  buffer.discard(6);
  buffer.append(QByteArray("wxyzAB"));
  QCOMPARE(buffer.size(), qsizetype(6));
  QCOMPARE(buffer.peek(6), QByteArray("wxyzAB"));
  QCOMPARE(buffer.peekRange(1, 3), QByteArray("xyz"));

  buffer.setCapacity(64);
  QCOMPARE(buffer.capacity(), qsizetype(64));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(64));

  buffer.append(QByteArray("fresh"));
  QCOMPARE(buffer.findPatternKMP(QByteArray("wx")), -1);
  QCOMPARE(buffer.findPatternKMP(QByteArray("esh")), 2);

  buffer.setCapacity(8);
  QCOMPARE(buffer.capacity(), qsizetype(8));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(8));

  buffer.append(QByteArray("123456"));
  buffer.discard(6);
  buffer.append(QByteArray("PQRSTU"));
  QCOMPARE(buffer.size(), qsizetype(6));
  QCOMPARE(buffer.peek(6), QByteArray("PQRSTU"));
  QCOMPARE(buffer.findPatternKMP(QByteArray("QR")), 1);
}

//--------------------------------------------------------------------------------------------------
// peekRangeInto at a nonzero offset that straddles the wrap
//--------------------------------------------------------------------------------------------------

/**
 * @brief peekRangeInto() is exercised elsewhere only against linear storage. This primes a wrapped
 *        buffer once (firstLen 4 of 10 logical bytes) and reads windows at several nonzero offsets:
 *        one entirely inside the first segment, one that straddles the boundary, one that starts
 *        exactly on the second segment, one that straddles and runs to the last logical byte, and
 *        one whose offset is past the content (clamped to empty).
 */
void TstRingWrap::peekRangeIntoSpansTheWrapAtNonzeroOffset_data()
{
  QTest::addColumn<qsizetype>("readOffset");
  QTest::addColumn<qsizetype>("readSize");
  QTest::addColumn<QByteArray>("expected");

  QTest::newRow("inside the first segment only") << qsizetype(3) << qsizetype(1) << QByteArray("d");
  QTest::newRow("straddles the boundary") << qsizetype(2) << qsizetype(6) << QByteArray("cdefgh");
  QTest::newRow("starts exactly on the second segment")
    << qsizetype(4) << qsizetype(4) << QByteArray("efgh");
  QTest::newRow("straddles and runs to the last logical byte")
    << qsizetype(1) << qsizetype(9) << QByteArray("bcdefghij");
  QTest::newRow("offset past the content clamps to empty")
    << qsizetype(10) << qsizetype(5) << QByteArray();
}

void TstRingWrap::peekRangeIntoSpansTheWrapAtNonzeroOffset()
{
  QFETCH(qsizetype, readOffset);
  QFETCH(qsizetype, readSize);
  QFETCH(QByteArray, expected);

  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 28, QByteArray("abcdefghij"));
  QCOMPARE(buffer.size(), qsizetype(10));

  QByteArray out;
  buffer.peekRangeInto(readOffset, readSize, out);
  QCOMPARE(out, expected);
  QCOMPARE(buffer.peekRange(readOffset, readSize), expected);
}

/**
 * @brief The pooled-frame reuse contract (tst_circular_buffer.cpp's
 *        peekRangeIntoReusesTheCallerBuffer) must keep holding once the read window itself crosses
 *        the physical wrap point, not just when the buffer's own content happens to.
 */
void TstRingWrap::peekRangeIntoAcrossTheWrapReusesTheCallerBuffer()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 28, QByteArray("abcdefghij"));

  QByteArray out;
  out.reserve(64);
  const char* const allocation = out.constData();

  buffer.peekRangeInto(2, 6, out);
  QCOMPARE(out, QByteArray("cdefgh"));
  QVERIFY(out.constData() == allocation);

  buffer.peekRangeInto(1, 9, out);
  QCOMPARE(out, QByteArray("bcdefghij"));
  QVERIFY(out.constData() == allocation);
}

QTEST_APPLESS_MAIN(TstRingWrap)

#include "tst_ring_wrap.moc"
