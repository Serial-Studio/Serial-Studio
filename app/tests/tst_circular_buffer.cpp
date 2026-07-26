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
#include <QList>
#include <QTest>

#include "IO/CircularBuffer.h"

// Capacity used by the pattern-scan tests: large enough for the fixtures, small enough that a
// modest starting offset forces the payload across the wrap point.
static constexpr qsizetype kScanCapacity = 32;

/**
 * @brief Writes @p content into @p buffer starting at ring offset @p offset, so a test can pick
 *        between the linear fast lane (offset 0) and the wrap-straddling lane by data alone.
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
 * @brief Ring-buffer semantics for IO::CircularBuffer, the SPSC store the frame reader
 *        parses out of. Qt Test runs private slots in declaration order, so every test
 *        function here builds its own buffer and carries no state to the next one.
 */
class TstCircularBuffer : public QObject {
  Q_OBJECT

private slots:
  void roundUpToPowerOfTwo_data();
  void roundUpToPowerOfTwo();
  void emptyBufferAccounting();
  void appendUpdatesSizeAndFreeSpace();
  void readConsumesBytes();
  void peekLeavesBytesInPlace();
  void discardAdvancesTheReadHead();
  void clearResetsAccounting();
  void wrappedContentRoundTrips();

  void setCapacityRoundsAndClears();
  void overflowingAppendCountsTheLostBytes();
  void oversizedAppendKeepsTheTailOfTheChunk();

  void peekRangeIntoReusesTheCallerBuffer();
  void peekRangeIntoReplacesASharedBuffer();
  void peekRangeIntoClampsOutOfRangeRequests();

  void buildKMPTable_data();
  void buildKMPTable();

  void findPattern_data();
  void findPattern();
  void findPatternRejectsAnEmptyPattern();

  void findFirstOfPatternsReturnsTheEarliestMatch();
  void findFirstOfPatternsPrefersTheLowerIndexAtEqualPositions();
  void findFirstOfPatternsHandlesEightPatterns();
  void findFirstOfPatternsAcrossTheWrap();
  void findFirstOfPatternsReportsNoMatch();
};

/**
 * @brief Capacity is rounded so the ring can mask the wrap with `& (cap - 1)`.
 */
void TstCircularBuffer::roundUpToPowerOfTwo_data()
{
  QTest::addColumn<qsizetype>("input");
  QTest::addColumn<qsizetype>("expected");

  QTest::newRow("zero clamps to two") << qsizetype(0) << qsizetype(2);
  QTest::newRow("one clamps to two") << qsizetype(1) << qsizetype(2);
  QTest::newRow("two is already exact") << qsizetype(2) << qsizetype(2);
  QTest::newRow("three rounds to four") << qsizetype(3) << qsizetype(4);
  QTest::newRow("five rounds to eight") << qsizetype(5) << qsizetype(8);
  QTest::newRow("eight is already exact") << qsizetype(8) << qsizetype(8);
  QTest::newRow("nine rounds to sixteen") << qsizetype(9) << qsizetype(16);
  QTest::newRow("1000 rounds to 1024") << qsizetype(1000) << qsizetype(1024);
  QTest::newRow("1024 is already exact") << qsizetype(1024) << qsizetype(1024);
  QTest::newRow("1025 rounds to 2048") << qsizetype(1025) << qsizetype(2048);
  QTest::newRow("64k is already exact") << qsizetype(65536) << qsizetype(65536);
}

void TstCircularBuffer::roundUpToPowerOfTwo()
{
  QFETCH(qsizetype, input);
  QFETCH(qsizetype, expected);

  QCOMPARE(IO::roundUpToPowerOfTwo(input), expected);
}

/**
 * @brief A fresh buffer reports the rounded capacity, no content, and all of it free.
 */
void TstCircularBuffer::emptyBufferAccounting()
{
  IO::CircularBuffer<QByteArray, char> buffer(24);

  QCOMPARE(buffer.capacity(), qsizetype(32));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(32));
  QCOMPARE(buffer.overflowCount(), qsizetype(0));
}

/**
 * @brief append() moves bytes from free space into size, one for one.
 */
void TstCircularBuffer::appendUpdatesSizeAndFreeSpace()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);

  buffer.append(QByteArray("abcd"));
  QCOMPARE(buffer.size(), qsizetype(4));
  QCOMPARE(buffer.freeSpace(), qsizetype(12));

  buffer.append(QByteArray("efg"));
  QCOMPARE(buffer.size(), qsizetype(7));
  QCOMPARE(buffer.freeSpace(), qsizetype(9));

  buffer.append(QByteArray());
  QCOMPARE(buffer.size(), qsizetype(7));
  QCOMPARE(buffer.overflowCount(), qsizetype(0));
}

/**
 * @brief read() returns the bytes in FIFO order and releases them.
 */
void TstCircularBuffer::readConsumesBytes()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray("hello world"));

  QCOMPARE(buffer.read(5), QByteArray("hello"));
  QCOMPARE(buffer.size(), qsizetype(6));

  QCOMPARE(buffer.read(6), QByteArray(" world"));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(16));
}

/**
 * @brief peek()/peekRange() are non-destructive views at a logical offset.
 */
void TstCircularBuffer::peekLeavesBytesInPlace()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray("hello"));

  QCOMPARE(buffer.peek(3), QByteArray("hel"));
  QCOMPARE(buffer.peek(5), QByteArray("hello"));
  QCOMPARE(buffer.peekRange(2, 3), QByteArray("llo"));
  QCOMPARE(buffer.size(), qsizetype(5));

  QCOMPARE(buffer.peek(99), QByteArray("hello"));
  QCOMPARE(buffer.peekRange(5, 1), QByteArray());
  QCOMPARE(buffer.size(), qsizetype(5));
}

/**
 * @brief discard() drops bytes without copying them out.
 */
void TstCircularBuffer::discardAdvancesTheReadHead()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray("hello"));

  buffer.discard(2);
  QCOMPARE(buffer.size(), qsizetype(3));
  QCOMPARE(buffer.peek(3), QByteArray("llo"));

  buffer.discard(99);
  QCOMPARE(buffer.size(), qsizetype(0));
}

/**
 * @brief clear() returns the buffer to its constructed accounting without resizing it.
 */
void TstCircularBuffer::clearResetsAccounting()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray("hello"));
  buffer.clear();

  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(16));
  QCOMPARE(buffer.capacity(), qsizetype(16));
}

/**
 * @brief Content written across the wrap point reads back contiguous: the write head is
 *        driven near the end of the ring first, so the payload lands in two segments.
 */
void TstCircularBuffer::wrappedContentRoundTrips()
{
  IO::CircularBuffer<QByteArray, char> buffer(8);

  buffer.append(QByteArray("123456"));
  buffer.discard(6);
  QCOMPARE(buffer.size(), qsizetype(0));

  buffer.append(QByteArray("wxyz"));
  QCOMPARE(buffer.size(), qsizetype(4));
  QCOMPARE(buffer.peek(4), QByteArray("wxyz"));
  QCOMPARE(buffer.peekRange(1, 2), QByteArray("xy"));
  QCOMPARE(buffer.read(4), QByteArray("wxyz"));
  QCOMPARE(buffer.size(), qsizetype(0));
}

//--------------------------------------------------------------------------------------------------
// Reconfiguration and overflow accounting
//--------------------------------------------------------------------------------------------------

/**
 * @brief setCapacity() rounds to the next power of two and drops whatever was buffered, in both
 *        the grow and the shrink direction.
 */
void TstCircularBuffer::setCapacityRoundsAndClears()
{
  IO::CircularBuffer<QByteArray, char> buffer(16);
  buffer.append(QByteArray("payload"));
  QCOMPARE(buffer.size(), qsizetype(7));

  buffer.setCapacity(100);

  QCOMPARE(buffer.capacity(), qsizetype(128));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(128));

  buffer.append(QByteArray("again"));
  QCOMPARE(buffer.peek(5), QByteArray("again"));

  buffer.setCapacity(4);

  QCOMPARE(buffer.capacity(), qsizetype(4));
  QCOMPARE(buffer.size(), qsizetype(0));
  QCOMPARE(buffer.freeSpace(), qsizetype(4));
}

/**
 * @brief An append that outruns the free space counts every overwritten byte. The ring encodes
 *        "full" and "empty" identically (head == tail), so an overflowing append also leaves
 *        size() reading zero -- the reader's overflow branch exists because of exactly that.
 */
void TstCircularBuffer::overflowingAppendCountsTheLostBytes()
{
  IO::CircularBuffer<QByteArray, char> buffer(8);

  buffer.append(QByteArray("1234567"));
  QCOMPARE(buffer.size(), qsizetype(7));
  QCOMPARE(buffer.overflowCount(), qsizetype(0));

  buffer.append(QByteArray("ab"));
  QCOMPARE(buffer.overflowCount(), qsizetype(1));
  QCOMPARE(buffer.size(), qsizetype(0));

  buffer.resetOverflowCount();
  QCOMPARE(buffer.overflowCount(), qsizetype(0));
}

/**
 * @brief A chunk larger than the whole ring keeps its tail and counts the discarded head bytes.
 */
void TstCircularBuffer::oversizedAppendKeepsTheTailOfTheChunk()
{
  IO::CircularBuffer<QByteArray, char> buffer(8);

  buffer.append(QByteArray(20, 'z'));

  QCOMPARE(buffer.capacity(), qsizetype(8));
  QCOMPARE(buffer.overflowCount(), qsizetype(12));

  buffer.clear();
  QCOMPARE(buffer.overflowCount(), qsizetype(0));
}

//--------------------------------------------------------------------------------------------------
// peekRangeInto
//--------------------------------------------------------------------------------------------------

/**
 * @brief The pooled-frame steady state depends on peekRangeInto() writing through a caller buffer
 *        that is unique and large enough, without reallocating it.
 */
void TstCircularBuffer::peekRangeIntoReusesTheCallerBuffer()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  buffer.append(QByteArray("hello world"));

  QByteArray out;
  out.reserve(64);
  const char* const allocation = out.constData();

  buffer.peekRangeInto(0, 5, out);
  QCOMPARE(out, QByteArray("hello"));
  QVERIFY(out.constData() == allocation);

  buffer.peekRangeInto(6, 5, out);
  QCOMPARE(out, QByteArray("world"));
  QVERIFY(out.constData() == allocation);

  buffer.peekRangeInto(0, 11, out);
  QCOMPARE(out, QByteArray("hello world"));
  QVERIFY(out.constData() == allocation);
  QCOMPARE(buffer.size(), qsizetype(11));
}

/**
 * @brief A destination still shared with another holder must be replaced, never written through:
 *        the other holder is reading a frame that was already handed to a consumer.
 */
void TstCircularBuffer::peekRangeIntoReplacesASharedBuffer()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  buffer.append(QByteArray("hello world"));

  QByteArray out;
  out.reserve(64);
  buffer.peekRangeInto(0, 5, out);

  const QByteArray shared = out;
  QVERIFY(!out.isDetached());

  buffer.peekRangeInto(6, 5, out);

  QCOMPARE(out, QByteArray("world"));
  QCOMPARE(shared, QByteArray("hello"));
}

/**
 * @brief An offset past the content empties the destination; a size past the content is clamped.
 */
void TstCircularBuffer::peekRangeIntoClampsOutOfRangeRequests()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  buffer.append(QByteArray("hello"));

  QByteArray out;
  buffer.peekRangeInto(2, 999, out);
  QCOMPARE(out, QByteArray("llo"));

  buffer.peekRangeInto(5, 1, out);
  QVERIFY(out.isEmpty());

  buffer.peekRangeInto(0, 0, out);
  QVERIFY(out.isEmpty());
  QCOMPARE(buffer.size(), qsizetype(5));
}

//--------------------------------------------------------------------------------------------------
// KMP table
//--------------------------------------------------------------------------------------------------

void TstCircularBuffer::buildKMPTable_data()
{
  QTest::addColumn<QByteArray>("pattern");
  QTest::addColumn<QList<int>>("expected");

  QTest::newRow("single byte") << QByteArray("a") << QList<int>{0};
  QTest::newRow("crlf") << QByteArray("\r\n") << QList<int>{0, 0};
  QTest::newRow("no self overlap") << QByteArray("abc") << QList<int>{0, 0, 0};
  QTest::newRow("prefix repeat") << QByteArray("aab") << QList<int>{0, 1, 0};
  QTest::newRow("all identical") << QByteArray("aaaa") << QList<int>{0, 1, 2, 3};
  QTest::newRow("alternating") << QByteArray("abab") << QList<int>{0, 0, 1, 2};
  QTest::newRow("classic") << QByteArray("abcabcd") << QList<int>{0, 0, 0, 1, 2, 3, 0};
}

/**
 * @brief The longest-proper-prefix table is precomputed once per delimiter by the frame reader,
 *        so a wrong entry mis-splits every frame on the connection.
 */
void TstCircularBuffer::buildKMPTable()
{
  QFETCH(QByteArray, pattern);
  QFETCH(QList<int>, expected);

  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  const auto lps = buffer.buildKMPTable(pattern);

  QCOMPARE(qsizetype(lps.size()), expected.size());
  for (qsizetype i = 0; i < expected.size(); ++i)
    QCOMPARE(lps[static_cast<size_t>(i)], expected.at(i));
}

//--------------------------------------------------------------------------------------------------
// Pattern search
//--------------------------------------------------------------------------------------------------

/**
 * @brief The same fixtures are searched twice: once laid out linearly and once written across the
 *        wrap, because the linear and wrapping scans are separate implementations.
 */
void TstCircularBuffer::findPattern_data()
{
  QTest::addColumn<qsizetype>("offset");
  QTest::addColumn<QByteArray>("content");
  QTest::addColumn<QByteArray>("pattern");
  QTest::addColumn<int>("pos");
  QTest::addColumn<int>("expected");

  struct Case {
    const char* name;
    QByteArray content;
    QByteArray pattern;
    int pos;
    int expected;
  };

  const QByteArray text("hello world abcdef");
  const QList<Case> cases = {
    {"single byte", text, QByteArray("w"), 0, 6},
    {"single byte at the final index", text, QByteArray("f"), 0, 17},
    {"single byte absent", text, QByteArray("z"), 0, -1},
    {"single byte from a start position", text, QByteArray("l"), 4, 9},
    {"two byte pattern", text, QByteArray("wo"), 0, 6},
    {"two byte pattern at the final index", text, QByteArray("ef"), 0, 16},
    {"eight byte pattern (memcmp lane)", text, QByteArray("d abcdef"), 0, 10},
    {"nine byte pattern (kmp lane)", text, QByteArray("ld abcdef"), 0, 9},
    {"pattern absent", text, QByteArray("world!"), 0, -1},
    {"pattern longer than the content", text, QByteArray(24, 'q'), 0, -1},
    {"kmp backtracking", QByteArray("aaaaaaaaab"), QByteArray("aaaaaaaab"), 0, 1},
  };

  const QList<qsizetype> offsets = {qsizetype(0), qsizetype(28)};
  for (const auto offset : offsets)
    for (const auto& row : cases) {
      const auto lane = (offset == 0) ? QStringLiteral("linear") : QStringLiteral("wrapped");
      const auto tag  = QStringLiteral("%1, %2").arg(QString::fromUtf8(row.name), lane);
      QTest::newRow(qPrintable(tag))
        << offset << row.content << row.pattern << row.pos << row.expected;
    }
}

void TstCircularBuffer::findPattern()
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
 * @brief An empty delimiter would otherwise match everywhere; the scan refuses it outright.
 */
void TstCircularBuffer::findPatternRejectsAnEmptyPattern()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  buffer.append(QByteArray("hello"));

  QCOMPARE(buffer.findPatternKMP(QByteArray()), -1);
}

//--------------------------------------------------------------------------------------------------
// Multi-pattern search
//--------------------------------------------------------------------------------------------------

/**
 * @brief The single-pass scan reports the earliest position, not the first pattern that happens
 *        to occur anywhere.
 */
void TstCircularBuffer::findFirstOfPatternsReturnsTheEarliestMatch()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 0, QByteArray("abc<END>xyz"));

  const QVector<QByteArray> patterns = {QByteArray("xyz"), QByteArray("<END>")};
  const auto match                   = buffer.findFirstOfPatterns(patterns);

  QCOMPARE(match.position, 3);
  QCOMPARE(match.patternIndex, 1);
}

/**
 * @brief Two delimiters matching at the same offset resolve to the lower list index, whichever
 *        of the two is longer.
 */
void TstCircularBuffer::findFirstOfPatternsPrefersTheLowerIndexAtEqualPositions()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 0, QByteArray("0123456789"));

  const auto longFirst = buffer.findFirstOfPatterns({QByteArray("45"), QByteArray("4")});
  QCOMPARE(longFirst.position, 4);
  QCOMPARE(longFirst.patternIndex, 0);

  const auto shortFirst = buffer.findFirstOfPatterns({QByteArray("4"), QByteArray("45")});
  QCOMPARE(shortFirst.position, 4);
  QCOMPARE(shortFirst.patternIndex, 0);
}

/**
 * @brief Eight simultaneous patterns is the documented maximum the scan is sized for.
 */
void TstCircularBuffer::findFirstOfPatternsHandlesEightPatterns()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 0, QByteArray("0123456789"));

  const QVector<QByteArray> patterns = {QByteArray("9"),
                                        QByteArray("8"),
                                        QByteArray("7"),
                                        QByteArray("6"),
                                        QByteArray("5"),
                                        QByteArray("4"),
                                        QByteArray("3"),
                                        QByteArray("2")};

  QCOMPARE(patterns.size(), qsizetype(8));

  const auto match = buffer.findFirstOfPatterns(patterns);
  QCOMPARE(match.position, 2);
  QCOMPARE(match.patternIndex, 7);
}

/**
 * @brief The wrapping lane of the multi-pattern scan indexes through the mask, so a match that
 *        straddles the ring boundary must still report its logical offset.
 */
void TstCircularBuffer::findFirstOfPatternsAcrossTheWrap()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 28, QByteArray("abcdefghij"));
  QCOMPARE(buffer.size(), qsizetype(10));

  const auto match = buffer.findFirstOfPatterns({QByteArray("hij"), QByteArray("cd")});

  QCOMPARE(match.position, 2);
  QCOMPARE(match.patternIndex, 1);
}

/**
 * @brief No match, and patterns longer than the content, both report the default result.
 */
void TstCircularBuffer::findFirstOfPatternsReportsNoMatch()
{
  IO::CircularBuffer<QByteArray, char> buffer(kScanCapacity);
  primeAtOffset(buffer, 0, QByteArray("0123456789"));

  const auto missing = buffer.findFirstOfPatterns({QByteArray("ab"), QByteArray("cd")});
  QCOMPARE(missing.position, -1);
  QCOMPARE(missing.patternIndex, -1);

  const auto tooLong = buffer.findFirstOfPatterns({QByteArray(24, 'q')});
  QCOMPARE(tooLong.position, -1);
  QCOMPARE(tooLong.patternIndex, -1);
}

QTEST_APPLESS_MAIN(TstCircularBuffer)

#include "tst_circular_buffer.moc"
