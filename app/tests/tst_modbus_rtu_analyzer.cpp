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

#include <cmath>
#include <QByteArray>
#include <QList>
#include <QTest>
#include <vector>

#include "Console/ModbusRtuAnalyzer.h"

// The bus monitor's Modbus RTU analysis (spec 0127 R12-R14, AC5-AC7): framing by function code
// and CRC under arbitrary chunking, master/slave roles, request/response pairing and one case per
// finding. The last case replays the spec's premise trace: 20,000 exchanges with dropped,
// corrupt and exception replies, fed in random chunks. Its generator is a port of the throwaway
// simulation the spec measured with, random numbers included, so the expected counts below are
// the ones that simulation printed for this exact algorithm on 2026-11-21.

using Console::BusFrame;
using Console::BusRole;
using Console::BusUpdate;

//--------------------------------------------------------------------------------------------------
// Frame builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief The Modbus CRC-16 of @p data, written here independently of the application's checksum
 *        table so the test is not checking that table against itself.
 */
[[nodiscard]] static quint16 crc16(const QByteArray& data)
{
  quint16 crc = 0xFFFF;
  for (const char c : data) {
    crc ^= static_cast<quint8>(c);
    for (int bit = 0; bit < 8; ++bit)
      crc =
        (crc & 1) != 0 ? static_cast<quint16>((crc >> 1) ^ 0xA001) : static_cast<quint16>(crc >> 1);
  }

  return crc;
}

/**
 * @brief @p body with its CRC appended, low byte first.
 */
[[nodiscard]] static QByteArray seal(QByteArray body)
{
  const quint16 crc = crc16(body);
  body.append(static_cast<char>(crc & 0xFF));
  body.append(static_cast<char>(crc >> 8));
  return body;
}

/**
 * @brief A sealed frame from a list of byte values.
 */
[[nodiscard]] static QByteArray frame(std::initializer_list<int> bytes)
{
  QByteArray body;
  for (const int value : bytes)
    body.append(static_cast<char>(value));

  return seal(body);
}

//--------------------------------------------------------------------------------------------------
// Harness
//--------------------------------------------------------------------------------------------------

/**
 * @brief An analyzer plus everything it reported.
 */
struct Bus {
  Console::ModbusRtuAnalyzer analyzer;
  std::vector<BusFrame> frames;
  std::vector<BusUpdate> updates;

  void feed(const QByteArray& bytes, const qint64 nowMs)
  {
    analyzer.feed(bytes, nowMs, frames, updates);
  }

  void tick(const qint64 nowMs) { analyzer.tick(nowMs, frames, updates); }

  [[nodiscard]] quint16 findings(const size_t index) const
  {
    quint16 all = frames[index].findings;
    for (const auto& update : updates)
      if (update.sequence == frames[index].sequence)
        all |= update.findings;

    return all;
  }
};

//--------------------------------------------------------------------------------------------------
// Premise trace
//--------------------------------------------------------------------------------------------------

/**
 * @brief The simulation's random source, reproduced operation for operation: it multiplied in
 *        double precision, so this does too, in separate statements so the compiler cannot fuse
 *        the multiply and the add into one rounding.
 */
struct TraceRandom {
  quint32 seed = 12345;

  [[nodiscard]] double next()
  {
    double m         = static_cast<double>(seed) * 1103515245.0;
    m                = m + 12345.0;
    const double low = std::floor(std::fmod(m, 4294967296.0));
    seed             = static_cast<quint32>(low) & 0x7FFFFFFFU;
    return static_cast<double>(seed) / 2147483647.0;
  }

  [[nodiscard]] int range(const int a, const int b)
  {
    return a + static_cast<int>(std::floor(next() * static_cast<double>(b - a + 1)));
  }
};

/**
 * @brief One frame the trace put on the wire and what happened to it.
 */
struct TraceFrame {
  QByteArray bytes;
  BusRole role     = BusRole::Master;
  bool corrupt     = false;
  bool unanswered  = false;
  bool isException = false;
};

/**
 * @brief The wire of the premise trace with a time per byte, and the frames it is made of.
 */
struct Trace {
  QByteArray wire;
  QList<qint64> times;
  QList<TraceFrame> frames;
  double clockMs = 0.0;

  void put(const QByteArray& bytes, const BusRole role, const bool corrupt, const bool exception)
  {
    TraceFrame entry;
    entry.bytes       = bytes;
    entry.role        = role;
    entry.corrupt     = corrupt;
    entry.isException = exception;
    frames.append(entry);
    for (const char c : bytes) {
      wire.append(c);
      times.append(static_cast<qint64>(std::floor(clockMs)));
      clockMs += 11.0 / 9.6;
    }
  }
};

/**
 * @brief Builds @p exchanges request/response pairs at 9600 baud across eight function codes:
 *        3% of responses never come, 3% are exceptions and 2% arrive with a damaged CRC.
 */
static void buildTrace(Trace& trace, TraceRandom& random, const int exchanges)
{
  static constexpr int kFunctions[] = {1, 2, 3, 4, 5, 6, 15, 16};
  for (int n = 0; n < exchanges; ++n) {
    const int slave    = random.range(1, 20);
    const int function = kFunctions[random.range(0, 7)];
    const int address  = random.range(0, 500);
    const int quantity = function <= 2 ? random.range(1, 40) : random.range(1, 20);

    QByteArray head;
    for (const int value : {slave, function, address >> 8, address & 255})
      head.append(static_cast<char>(value));

    QByteArray request;
    QByteArray response;
    if (function <= 4) {
      const int counted = function <= 2 ? (quantity + 7) / 8 : quantity * 2;
      QByteArray data;
      for (int i = 0; i < counted; ++i)
        data.append(static_cast<char>(random.range(0, 255)));

      request  = seal(head + QByteArray(1, static_cast<char>(quantity >> 8))
                      + QByteArray(1, static_cast<char>(quantity & 255)));
      response = seal(head.first(2) + QByteArray(1, static_cast<char>(counted)) + data);
    } else if (function <= 6) {
      const int value = function == 5 ? (random.next() < 0.5 ? 0xFF00 : 0) : random.range(0, 65535);
      request         = seal(head + QByteArray(1, static_cast<char>(value >> 8))
                             + QByteArray(1, static_cast<char>(value & 255)));
      response        = request;
    } else {
      const int counted = function == 15 ? (quantity + 7) / 8 : quantity * 2;
      QByteArray data;
      for (int i = 0; i < counted; ++i)
        data.append(static_cast<char>(random.range(0, 255)));

      const auto span = QByteArray(1, static_cast<char>(quantity >> 8))
                      + QByteArray(1, static_cast<char>(quantity & 255));
      request         = seal(head + span + QByteArray(1, static_cast<char>(counted)) + data);
      response        = seal(head + span);
    }

    trace.put(request, BusRole::Master, false, false);
    trace.clockMs     += random.range(5, 40);
    const double fate  = random.next();
    if (fate < 0.03) {
      trace.frames.last().unanswered  = true;
      trace.clockMs                  += 600.0;
    } else if (fate < 0.06) {
      const int code = random.range(1, 4);
      trace.put(frame({slave, function | 0x80, code}), BusRole::Slave, false, true);
    } else if (fate < 0.08) {
      QByteArray damaged          = response;
      damaged[damaged.size() - 1] = static_cast<char>(damaged.back() ^ 0x55);
      trace.put(damaged, BusRole::Slave, true, false);
    } else {
      trace.put(response, BusRole::Slave, false, false);
    }

    trace.clockMs += random.range(20, 120);
  }
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class ModbusRtuAnalyzerTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanExchangeHasNoFindings();
  void chunkingDoesNotChangeTheFrames();
  void rolesWhereRequestAndResponseLookAlike();
  void silenceTimesOutWithoutMoreBytes();
  void nextRequestClosesTheUnansweredOne();
  void lateAndOrphanResponses();
  void mismatchedResponse();
  void corruptResponseIsOneFailedRow();
  void changedEchoAndWrongByteCount();
  void exceptions();
  void invalidRequests_data();
  void invalidRequests();
  void broadcastWriteExpectsNoResponse();
  void unknownFunctionIsFramedWhenItArrivesWhole();
  void idleBytesAreReportedIncomplete();
  void severityAndTexts();
  void dataSummaries();
  void resetDropsBuffersButKeepsCounting();
  void premiseTrace();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void ModbusRtuAnalyzerTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief A read request and its response are a master and a slave row with nothing to report.
 */
void ModbusRtuAnalyzerTest::cleanExchangeHasNoFindings()
{
  Bus bus;
  bus.feed(frame({1, 3, 0, 16, 0, 2}), 10);
  QVERIFY(bus.analyzer.awaitingResponse());
  bus.feed(frame({1, 3, 4, 0, 10, 0, 20}), 30);

  QCOMPARE(bus.frames.size(), size_t(2));
  QCOMPARE(bus.frames[0].role, BusRole::Master);
  QCOMPARE(bus.frames[1].role, BusRole::Slave);
  QCOMPARE(bus.frames[0].timeMs, qint64(10));
  QVERIFY(bus.frames[0].crcOk && bus.frames[1].crcOk);
  QCOMPARE(bus.findings(0), quint16(Console::FindingNone));
  QCOMPARE(bus.findings(1), quint16(Console::FindingNone));
  QVERIFY(!bus.analyzer.awaitingResponse());
  QVERIFY(bus.updates.empty());
  QCOMPARE(bus.analyzer.bufferedBytes(), qsizetype(0));
}

/**
 * @brief Fed one byte at a time, or as one block holding three exchanges, the same frames come
 *        out.
 */
void ModbusRtuAnalyzerTest::chunkingDoesNotChangeTheFrames()
{
  const QByteArray stream = frame({1, 3, 0, 16, 0, 2}) + frame({1, 3, 4, 0, 10, 0, 20})
                          + frame({2, 16, 0, 1, 0, 2, 4, 0, 10, 1, 2}) + frame({2, 16, 0, 1, 0, 2})
                          + frame({3, 1, 0, 0, 0, 20}) + frame({3, 1, 3, 0xCD, 0x6B, 0x05});

  Bus whole;
  whole.feed(stream, 5);

  Bus bytes;
  for (qsizetype i = 0; i < stream.size(); ++i)
    bytes.feed(stream.sliced(i, 1), 5 + i);

  QCOMPARE(whole.frames.size(), size_t(6));
  QCOMPARE(bytes.frames.size(), size_t(6));
  for (size_t i = 0; i < 6; ++i) {
    QCOMPARE(bytes.frames[i].bytes, whole.frames[i].bytes);
    QCOMPARE(bytes.frames[i].role, whole.frames[i].role);
    QCOMPARE(bytes.findings(i), quint16(Console::FindingNone));
    QCOMPARE(whole.findings(i), quint16(Console::FindingNone));
  }
}

/**
 * @brief A coil response with three data bytes is eight bytes, like its request, and a write
 *        response echoes its request: the waiting request tells them apart.
 */
void ModbusRtuAnalyzerTest::rolesWhereRequestAndResponseLookAlike()
{
  Bus coils;
  coils.feed(frame({3, 1, 0, 0, 0, 20}), 0);
  coils.feed(frame({3, 1, 3, 0xCD, 0x6B, 0x05}), 20);
  QCOMPARE(coils.frames[0].role, BusRole::Master);
  QCOMPARE(coils.frames[1].role, BusRole::Slave);
  QCOMPARE(coils.findings(1), quint16(Console::FindingNone));

  Bus write;
  write.feed(frame({4, 6, 0, 1, 0, 3}), 0);
  write.feed(frame({4, 6, 0, 1, 0, 3}), 20);
  write.feed(frame({4, 6, 0, 1, 0, 3}), 300);
  QCOMPARE(write.frames[0].role, BusRole::Master);
  QCOMPARE(write.frames[1].role, BusRole::Slave);
  QCOMPARE(write.frames[2].role, BusRole::Master);

  Bus multi;
  multi.feed(frame({2, 15, 0, 19, 0, 10, 2, 0xCD, 0x01}), 0);
  multi.feed(frame({2, 15, 0, 19, 0, 10}), 20);
  QCOMPARE(multi.frames[0].role, BusRole::Master);
  QCOMPARE(multi.frames[1].role, BusRole::Slave);
  QCOMPARE(multi.findings(1), quint16(Console::FindingNone));
}

/**
 * @brief A request followed by silence gets its verdict from tick() alone, at the timeout and
 *        not before, on its own row (AC7).
 */
void ModbusRtuAnalyzerTest::silenceTimesOutWithoutMoreBytes()
{
  Bus bus;
  bus.feed(frame({9, 4, 0, 0, 0, 1}), 100);
  bus.tick(100 + Console::kModbusTimeoutDefaultMs - 1);
  QVERIFY(bus.updates.empty());
  QVERIFY(bus.analyzer.awaitingResponse());

  bus.tick(100 + Console::kModbusTimeoutDefaultMs);
  QCOMPARE(bus.frames.size(), size_t(1));
  QCOMPARE(bus.updates.size(), size_t(1));
  QCOMPARE(bus.updates[0].sequence, bus.frames[0].sequence);
  QCOMPARE(bus.findings(0), quint16(Console::FindingNoResponse));
  QVERIFY(!bus.analyzer.awaitingResponse());

  bus.tick(5000);
  QCOMPARE(bus.updates.size(), size_t(1));

  Bus fast;
  fast.analyzer.setTimeoutMs(50);
  QCOMPARE(fast.analyzer.timeoutMs(), 50);
  fast.feed(frame({9, 4, 0, 0, 0, 1}), 0);
  fast.tick(50);
  QCOMPARE(fast.findings(0), quint16(Console::FindingNoResponse));
  fast.analyzer.setTimeoutMs(1);
  QCOMPARE(fast.analyzer.timeoutMs(), Console::kModbusTimeoutMinMs);
}

/**
 * @brief A second request before any response marks the first one unanswered.
 */
void ModbusRtuAnalyzerTest::nextRequestClosesTheUnansweredOne()
{
  Bus bus;
  bus.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  bus.feed(frame({2, 3, 0, 0, 0, 1}), 100);
  bus.feed(frame({2, 3, 2, 0, 7}), 120);

  QCOMPARE(bus.frames.size(), size_t(3));
  QCOMPARE(bus.findings(0), quint16(Console::FindingNoResponse));
  QCOMPARE(bus.findings(1), quint16(Console::FindingNone));
  QCOMPARE(bus.findings(2), quint16(Console::FindingNone));
}

/**
 * @brief A response that turns up after its request timed out is late; one with no request at
 *        all is an orphan.
 */
void ModbusRtuAnalyzerTest::lateAndOrphanResponses()
{
  Bus late;
  late.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  late.tick(600);
  late.feed(frame({1, 3, 2, 0, 7}), 650);
  QCOMPARE(late.findings(0), quint16(Console::FindingNoResponse));
  QCOMPARE(late.findings(1), quint16(Console::FindingLateResponse));

  Bus orphan;
  orphan.feed(frame({1, 3, 2, 0, 7}), 0);
  QCOMPARE(orphan.frames[0].role, BusRole::Slave);
  QCOMPARE(orphan.findings(0), quint16(Console::FindingOrphanResponse));

  Bus stale;
  stale.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  stale.tick(600);
  stale.feed(frame({1, 3, 2, 0, 7}), 600 + Console::kModbusLateWindowMs + 1);
  QCOMPARE(stale.findings(1), quint16(Console::FindingOrphanResponse));
}

/**
 * @brief A response from another slave, or to another function, does not match its request.
 */
void ModbusRtuAnalyzerTest::mismatchedResponse()
{
  Bus slave;
  slave.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  slave.feed(frame({2, 3, 2, 0, 7}), 20);
  QCOMPARE(slave.findings(1), quint16(Console::FindingMismatch));
  QVERIFY(!slave.analyzer.awaitingResponse());

  Bus function;
  function.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  function.feed(frame({1, 4, 2, 0, 7}), 20);
  QCOMPARE(function.findings(1), quint16(Console::FindingMismatch));
}

/**
 * @brief A response with a damaged CRC becomes one failed row, the request is not also reported
 *        as unanswered, and the exchange after it is intact.
 */
void ModbusRtuAnalyzerTest::corruptResponseIsOneFailedRow()
{
  QByteArray damaged          = frame({1, 3, 4, 0, 10, 0, 20});
  damaged[damaged.size() - 1] = static_cast<char>(damaged.back() ^ 0x55);

  Bus bus;
  bus.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  bus.feed(damaged, 20);
  bus.feed(frame({2, 3, 0, 0, 0, 1}), 200);
  bus.feed(frame({2, 3, 2, 0, 7}), 220);

  QCOMPARE(bus.frames.size(), size_t(4));
  QVERIFY(!bus.frames[1].crcOk);
  QCOMPARE(bus.frames[1].role, BusRole::Unknown);
  QCOMPARE(bus.frames[1].bytes, damaged);
  QVERIFY((bus.findings(1) & Console::FindingCrcFailed) != 0);
  QCOMPARE(bus.findings(0), quint16(Console::FindingNone));
  QCOMPARE(bus.frames[2].role, BusRole::Master);
  QCOMPARE(bus.findings(2), quint16(Console::FindingNone));
  QCOMPARE(bus.findings(3), quint16(Console::FindingNone));
  QVERIFY(bus.updates.empty());
}

/**
 * @brief A write response must echo what was written, and a read response must carry the bytes
 *        the quantity asked for.
 */
void ModbusRtuAnalyzerTest::changedEchoAndWrongByteCount()
{
  Bus single;
  single.feed(frame({4, 6, 0, 1, 0, 3}), 0);
  single.feed(frame({4, 6, 0, 1, 0, 9}), 20);
  QCOMPARE(single.frames[1].role, BusRole::Slave);
  QCOMPARE(single.findings(1), quint16(Console::FindingEchoDiffers));

  Bus multi;
  multi.feed(frame({2, 16, 0, 1, 0, 2, 4, 0, 10, 1, 2}), 0);
  multi.feed(frame({2, 16, 0, 1, 0, 3}), 20);
  QCOMPARE(multi.findings(1), quint16(Console::FindingEchoDiffers));

  Bus count;
  count.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  count.feed(frame({1, 3, 2, 0, 10}), 20);
  QCOMPARE(count.frames.size(), size_t(2));
  QCOMPARE(count.findings(1), quint16(Console::FindingByteCountWrong));
}

/**
 * @brief An exception with a defined code is a warning; an undefined code is an error.
 */
void ModbusRtuAnalyzerTest::exceptions()
{
  Bus defined;
  defined.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  defined.feed(frame({1, 0x83, 2}), 20);
  QCOMPARE(defined.frames[1].role, BusRole::Slave);
  QCOMPARE(defined.findings(1), quint16(Console::FindingException));
  QCOMPARE(Console::busSeverity(defined.findings(1)), Console::BusSeverity::Warning);

  Bus undefined;
  undefined.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  undefined.feed(frame({1, 0x83, 0x7F}), 20);
  QCOMPARE(undefined.findings(1), quint16(Console::FindingUnknownException));
  QCOMPARE(Console::busSeverity(undefined.findings(1)), Console::BusSeverity::Error);
}

/**
 * @brief One row per request the protocol does not allow.
 */
void ModbusRtuAnalyzerTest::invalidRequests_data()
{
  QTest::addColumn<QByteArray>("request");

  QTest::newRow("slave above 247") << frame({248, 3, 0, 0, 0, 1});
  QTest::newRow("read to broadcast") << frame({0, 3, 0, 0, 0, 1});
  QTest::newRow("zero registers") << frame({1, 3, 0, 0, 0, 0});
  QTest::newRow("126 registers") << frame({1, 4, 0, 0, 0, 126});
  QTest::newRow("2001 coils") << frame({1, 1, 0, 0, 0x07, 0xD1});
  QTest::newRow("coil value") << frame({1, 5, 0, 0, 0x12, 0x34});
  QTest::newRow("register byte count") << frame({1, 16, 0, 1, 0, 2, 2, 0, 10});
  QTest::newRow("coil byte count") << frame({1, 15, 0, 19, 0, 10, 1, 0xCD});
}

/**
 * @brief Each is framed as a master request and flagged as out of range.
 */
void ModbusRtuAnalyzerTest::invalidRequests()
{
  QFETCH(QByteArray, request);

  Bus bus;
  bus.feed(request, 0);
  QCOMPARE(bus.frames.size(), size_t(1));
  QCOMPARE(bus.frames[0].role, BusRole::Master);
  QVERIFY((bus.findings(0) & Console::FindingInvalidRequest) != 0);
  QCOMPARE(Console::busSeverity(bus.findings(0)), Console::BusSeverity::Error);
}

/**
 * @brief A write to address 0 is information, and nothing waits for a response to it.
 */
void ModbusRtuAnalyzerTest::broadcastWriteExpectsNoResponse()
{
  Bus bus;
  bus.feed(frame({0, 6, 0, 1, 0, 3}), 0);
  QCOMPARE(bus.findings(0), quint16(Console::FindingBroadcastWrite));
  QCOMPARE(Console::busSeverity(bus.findings(0)), Console::BusSeverity::Info);
  QVERIFY(!bus.analyzer.awaitingResponse());

  bus.tick(5000);
  QVERIFY(bus.updates.empty());
}

/**
 * @brief A custom function code has no known length, so it is accepted when the whole chunk
 *        verifies as one frame, and flagged as unknown.
 */
void ModbusRtuAnalyzerTest::unknownFunctionIsFramedWhenItArrivesWhole()
{
  Bus bus;
  bus.feed(frame({5, 0x41, 1, 2, 3, 4, 5, 6, 7}), 0);
  QCOMPARE(bus.frames.size(), size_t(1));
  QVERIFY(bus.frames[0].crcOk);
  QVERIFY((bus.findings(0) & Console::FindingUnknownFunction) != 0);
  QCOMPARE(Console::busSeverity(Console::FindingUnknownFunction), Console::BusSeverity::Warning);
}

/**
 * @brief Half a frame followed by silence is reported once the line has been idle for the flush
 *        limit, and not before.
 */
void ModbusRtuAnalyzerTest::idleBytesAreReportedIncomplete()
{
  Bus bus;
  bus.feed(frame({1, 3, 4, 0, 10, 0, 20}).first(5), 0);
  bus.tick(Console::kModbusIdleFlushMs - 1);
  QVERIFY(bus.frames.empty());
  QCOMPARE(bus.analyzer.bufferedBytes(), qsizetype(5));

  bus.tick(Console::kModbusIdleFlushMs);
  QCOMPARE(bus.frames.size(), size_t(1));
  QVERIFY(!bus.frames[0].crcOk);
  QCOMPARE(bus.frames[0].bytes.size(), qsizetype(5));
  QCOMPARE(bus.findings(0), quint16(Console::FindingCrcFailed | Console::FindingIncomplete));
  QCOMPARE(bus.analyzer.bufferedBytes(), qsizetype(0));
}

/**
 * @brief Every finding has a level and a sentence, and several findings join with semicolons.
 */
void ModbusRtuAnalyzerTest::severityAndTexts()
{
  BusFrame row;
  row.bytes = frame({7, 3, 0, 0, 0, 1});

  QCOMPARE(Console::busSeverity(Console::FindingNone), Console::BusSeverity::None);
  QCOMPARE(Console::busSeverity(Console::FindingLateResponse), Console::BusSeverity::Warning);
  QCOMPARE(Console::busSeverity(Console::FindingNoResponse | Console::FindingBroadcastWrite),
           Console::BusSeverity::Error);
  QVERIFY(Console::busFindingsText(Console::FindingNone, row).isEmpty());
  QVERIFY(Console::busFindingsText(Console::FindingNoResponse, row).contains(QLatin1Char('7')));

  for (int bit = 0; bit < 13; ++bit)
    QVERIFY(!Console::busFindingsText(static_cast<quint16>(1 << bit), row).isEmpty());

  const auto both =
    Console::busFindingsText(Console::FindingCrcFailed | Console::FindingIncomplete, row);
  QVERIFY(both.contains(QStringLiteral("; ")));
  QVERIFY(!Console::modbusFunctionName(3).isEmpty());
  QVERIFY(Console::modbusFunctionName(0x83).contains(Console::modbusFunctionName(3)));
  QVERIFY(Console::modbusFunctionName(0x41).contains(QStringLiteral("65")));
}

/**
 * @brief The summary names what a request asks for and renders a response in the ticked formats.
 */
void ModbusRtuAnalyzerTest::dataSummaries()
{
  Bus bus;
  bus.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  bus.feed(frame({1, 3, 4, 0xFF, 0xFE, 0x00, 0x14}), 20);

  const auto request = Console::modbusDataSummary(bus.frames[0], Console::FormatU16);
  QVERIFY(request.contains(QStringLiteral("0x0010")));
  QVERIFY(request.contains(QLatin1Char('2')));

  QCOMPARE(Console::modbusDataSummary(bus.frames[1], Console::FormatU16),
           QStringLiteral("U16: [65534, 20]"));
  QCOMPARE(Console::modbusDataSummary(bus.frames[1], Console::FormatS16),
           QStringLiteral("S16: [-2, 20]"));
  QCOMPARE(Console::modbusDataSummary(bus.frames[1], Console::FormatU32),
           QStringLiteral("U32: [4294836244]"));
  QCOMPARE(Console::modbusDataSummary(bus.frames[1], Console::FormatU16 | Console::FormatS16),
           QStringLiteral("U16: [65534, 20]; S16: [-2, 20]"));
  QCOMPARE(Console::modbusDataSummary(bus.frames[1], 0), QStringLiteral("FF FE 00 14"));

  Bus coils;
  coils.feed(frame({3, 1, 0, 0, 0, 8}), 0);
  coils.feed(frame({3, 1, 1, 0x0D}), 20);
  QVERIFY(Console::modbusDataSummary(coils.frames[1], Console::FormatU16)
            .contains(QStringLiteral("[1, 0, 1, 1, 0, 0, 0, 0]")));

  Bus refused;
  refused.feed(frame({1, 3, 0, 16, 0, 2}), 0);
  refused.feed(frame({1, 0x83, 2}), 20);
  QVERIFY(Console::modbusDataSummary(refused.frames[1], Console::FormatU16)
            .contains(QStringLiteral("0x02")));

  BusFrame garbage;
  garbage.bytes = QByteArray::fromHex("010203");
  QVERIFY(Console::modbusDataSummary(garbage, Console::FormatU16).contains(QLatin1Char('3')));
}

/**
 * @brief reset() forgets buffered bytes and the waiting request; sequence numbers keep rising so
 *        a later update can never point at a row from before the reset.
 */
void ModbusRtuAnalyzerTest::resetDropsBuffersButKeepsCounting()
{
  Bus bus;
  bus.feed(frame({1, 3, 0, 0, 0, 1}), 0);
  bus.feed(QByteArray::fromHex("0103"), 10);
  const quint64 before = bus.frames[0].sequence;
  QVERIFY(bus.analyzer.awaitingResponse());
  QVERIFY(bus.analyzer.bufferedBytes() > 0);

  bus.analyzer.reset();
  QVERIFY(!bus.analyzer.awaitingResponse());
  QCOMPARE(bus.analyzer.bufferedBytes(), qsizetype(0));

  bus.feed(frame({2, 3, 0, 0, 0, 1}), 20);
  QVERIFY(bus.frames[1].sequence > before);
  bus.tick(5000);
  QCOMPARE(bus.updates.size(), size_t(1));
  QCOMPARE(bus.updates[0].sequence, bus.frames[1].sequence);
}

/**
 * @brief The spec's premise trace (AC5): every valid frame is framed in order with the right
 *        role, every dropped response is flagged on its request and nowhere else, every
 *        exception is flagged, every corrupt response is one failed row, and nothing else on
 *        the bus carries a finding.
 */
void ModbusRtuAnalyzerTest::premiseTrace()
{
  Trace trace;
  TraceRandom random;
  buildTrace(trace, random, 20000);
  QCOMPARE(trace.wire.size(), qsizetype(459457));
  QCOMPARE(trace.frames.size(), qsizetype(39160));

  Bus bus;
  qsizetype position = 0;
  while (position < trace.wire.size()) {
    const qsizetype count = qMin<qsizetype>(random.range(1, 64), trace.wire.size() - position);
    bus.tick(trace.times.at(position));
    bus.feed(trace.wire.sliced(position, count), trace.times.at(position + count - 1));
    position += count;
  }

  bus.tick(trace.times.last() + 1000);
  QCOMPARE(random.seed, quint32(1624741376));

  qsizetype expected = 0;
  int failedRows     = 0;
  int unanswered     = 0;
  int exceptions     = 0;
  int clean          = 0;
  for (size_t i = 0; i < bus.frames.size(); ++i) {
    if (!bus.frames[i].crcOk) {
      ++failedRows;
      continue;
    }

    while (expected < trace.frames.size() && trace.frames.at(expected).corrupt)
      ++expected;

    QVERIFY(expected < trace.frames.size());
    const auto& truth = trace.frames.at(expected++);
    QCOMPARE(bus.frames[i].bytes, truth.bytes);
    QCOMPARE(bus.frames[i].role, truth.role);

    const quint16 found = bus.findings(i);
    if (truth.unanswered) {
      QCOMPARE(found, quint16(Console::FindingNoResponse));
      ++unanswered;
    } else if (truth.isException) {
      QCOMPARE(found, quint16(Console::FindingException));
      ++exceptions;
    } else {
      QCOMPARE(found, quint16(Console::FindingNone));
      ++clean;
    }
  }

  QCOMPARE(failedRows, 402);
  QCOMPARE(unanswered, 840);
  QCOMPARE(exceptions, 677);
  QCOMPARE(clean, 37241);
  QCOMPARE(bus.updates.size(), size_t(840));
}

QTEST_APPLESS_MAIN(ModbusRtuAnalyzerTest)

#include "tst_modbus_rtu_analyzer.moc"
