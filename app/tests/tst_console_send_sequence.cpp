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
#include <QList>
#include <QTest>
#include <QVariantList>
#include <QVariantMap>

#include "Console/SendSequence.h"
#include "Core/Runtime.h"

// The console command list (spec 0127 R1-R3, AC1): the enabled pins go out in order, each with
// its own framing; a loop starts over; a closed link ends the run and a paused one holds it; and
// a sealed session never starts one. The runner sends through callbacks, so a struct of flags
// stands in for the device. The sealing case runs last because a seal cannot be undone.

//--------------------------------------------------------------------------------------------------
// Fake link
//--------------------------------------------------------------------------------------------------

struct FakeLink {
  bool connected = true;
  bool paused    = false;
  QList<QByteArray> sent;

  [[nodiscard]] Console::SequenceLink callbacks()
  {
    Console::SequenceLink link;
    link.connected = [this] {
      return connected;
    };
    link.paused = [this] {
      return paused;
    };
    link.send = [this](const QByteArray& bytes) {
      sent.append(bytes);
    };
    return link;
  }
};

/**
 * @brief One stored pin; the keys are the ones the send library persists.
 */
[[nodiscard]] static QVariantMap pin(const QString& payload,
                                     const bool enabled,
                                     const int delayMs,
                                     const bool hex       = false,
                                     const int lineEnding = 0,
                                     const QString& crc   = QString())
{
  QVariantMap map;
  map.insert(QStringLiteral("title"), payload);
  map.insert(QStringLiteral("payload"), payload);
  map.insert(QStringLiteral("hex"), hex);
  map.insert(QStringLiteral("lineEnding"), lineEnding);
  map.insert(QStringLiteral("checksumName"), crc);
  map.insert(QStringLiteral("encoding"), 0);
  map.insert(QStringLiteral("enabled"), enabled);
  map.insert(QStringLiteral("delayMs"), delayMs);
  return map;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class ConsoleSendSequenceTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void legacyPinsAreEnabledWithTheDefaultDelay();
  void nextEnabledPinSkipsDisabledAndEmpty();
  void presetsEncodeWithTheirOwnFraming();
  void runSendsEnabledPinsInOrder();
  void loopStartsOverUntilStopped();
  void closedLinkEndsTheRun();
  void pausedLinkHoldsItsPosition();
  void startIsRefusedWhenItCannotRun();
  void listIsCopiedWhenTheRunStarts();
  void sealedSessionNeverStarts();
};

/**
 * @brief Takes the SS_ASSERT recovery branch instead of aborting.
 */
void ConsoleSendSequenceTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief A pin saved before lists existed has neither key: it is enabled and waits one second.
 */
void ConsoleSendSequenceTest::legacyPinsAreEnabledWithTheDefaultDelay()
{
  QVariantMap legacy;
  legacy.insert(QStringLiteral("payload"), QStringLiteral("AT"));
  QVERIFY(Console::pinEnabled(legacy));
  QCOMPARE(Console::pinDelayMs(legacy), Console::kSequenceDelayDefault);

  QCOMPARE(Console::pinDelayMs(pin(QStringLiteral("AT"), true, -5)), 0);
  QCOMPARE(Console::pinDelayMs(pin(QStringLiteral("AT"), true, 99999999)),
           Console::kSequenceDelayMax);
}

/**
 * @brief The walk skips disabled pins and pins without a payload, and ends with -1.
 */
void ConsoleSendSequenceTest::nextEnabledPinSkipsDisabledAndEmpty()
{
  const QVariantList pins = {pin(QStringLiteral("A"), false, 0),
                             pin(QString(), true, 0),
                             pin(QStringLiteral("C"), true, 0),
                             pin(QStringLiteral("D"), true, 0)};

  QCOMPARE(Console::nextEnabledPin(pins, -1), 2);
  QCOMPARE(Console::nextEnabledPin(pins, 2), 3);
  QCOMPARE(Console::nextEnabledPin(pins, 3), -1);
  QCOMPARE(Console::nextEnabledPin(QVariantList(), -1), -1);
}

/**
 * @brief Text takes its line ending, hexadecimal becomes bytes, and a checksum closes either.
 */
void ConsoleSendSequenceTest::presetsEncodeWithTheirOwnFraming()
{
  const auto text = Console::txPresetFromMap(pin(QStringLiteral("AT"), true, 0, false, 3));
  QCOMPARE(Console::encodeTxPreset(text), QByteArray("AT\r\n"));

  const auto hex = Console::txPresetFromMap(pin(QStringLiteral("0102"), true, 0, true));
  QCOMPARE(Console::encodeTxPreset(hex), QByteArray::fromHex("0102"));

  const auto summed = Console::txPresetFromMap(
    pin(QStringLiteral("0102"), true, 0, true, 0, QStringLiteral("XOR-8")));
  QCOMPARE(Console::encodeTxPreset(summed), QByteArray::fromHex("010203"));

  QVariantMap stored;
  Console::writeTxPreset(text, stored);
  QCOMPARE(Console::encodeTxPreset(Console::txPresetFromMap(stored)), QByteArray("AT\r\n"));
  QCOMPARE(Console::lineEndingBytes(1), QByteArray("\n"));
  QCOMPARE(Console::lineEndingBytes(2), QByteArray("\r"));
  QVERIFY(Console::lineEndingBytes(0).isEmpty());
}

/**
 * @brief Three enabled pins and one disabled: exactly three payloads, in list order, each with
 *        its own framing, and the run ends by itself.
 */
void ConsoleSendSequenceTest::runSendsEnabledPinsInOrder()
{
  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  const QVariantList pins = {pin(QStringLiteral("ONE"), true, 0, false, 1),
                             pin(QStringLiteral("SKIP"), false, 0),
                             pin(QStringLiteral("0A0B"), true, 20, true),
                             pin(QStringLiteral("THREE"), true, 30, false, 3)};

  QVERIFY(runner.start(pins, false));
  QVERIFY(runner.running());
  QVERIFY(!runner.looping());
  QCOMPARE(runner.currentIndex(), 0);

  QTRY_VERIFY_WITH_TIMEOUT(!runner.running(), 3000);
  QCOMPARE(link.sent.size(), 3);
  QCOMPARE(link.sent.at(0), QByteArray("ONE\n"));
  QCOMPARE(link.sent.at(1), QByteArray::fromHex("0a0b"));
  QCOMPARE(link.sent.at(2), QByteArray("THREE\r\n"));
  QCOMPARE(runner.currentIndex(), -1);
}

/**
 * @brief In a loop the payload after the last pin is the first one again, until stopped.
 */
void ConsoleSendSequenceTest::loopStartsOverUntilStopped()
{
  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  const QVariantList pins = {pin(QStringLiteral("A"), true, 2),
                             pin(QStringLiteral("B"), true, 2),
                             pin(QStringLiteral("C"), true, 2)};

  QVERIFY(runner.start(pins, true));
  QVERIFY(runner.looping());
  QTRY_VERIFY_WITH_TIMEOUT(link.sent.size() >= 7, 3000);
  runner.stop();
  QVERIFY(!runner.running());

  const QList<QByteArray> cycle = {"A", "B", "C"};
  for (qsizetype i = 0; i < 7; ++i)
    QCOMPARE(link.sent.at(i), cycle.at(i % 3));

  const auto count = link.sent.size();
  QTest::qWait(40);
  QCOMPARE(link.sent.size(), count);
}

/**
 * @brief A link that closes under the timer ends the run without another send.
 */
void ConsoleSendSequenceTest::closedLinkEndsTheRun()
{
  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  const QVariantList pins = {pin(QStringLiteral("A"), true, 0), pin(QStringLiteral("B"), true, 60)};

  QVERIFY(runner.start(pins, true));
  QTRY_COMPARE_WITH_TIMEOUT(link.sent.size(), 1, 3000);
  link.connected = false;
  QTRY_VERIFY_WITH_TIMEOUT(!runner.running(), 3000);
  QCOMPARE(link.sent.size(), 1);
}

/**
 * @brief A paused link sends nothing and keeps the position; resuming sends from there.
 */
void ConsoleSendSequenceTest::pausedLinkHoldsItsPosition()
{
  FakeLink link;
  link.paused = true;
  Console::SendSequencer runner(link.callbacks());
  const QVariantList pins = {pin(QStringLiteral("A"), true, 0), pin(QStringLiteral("B"), true, 0)};

  QVERIFY(runner.start(pins, false));
  QTest::qWait(3 * Console::kSequencePausePollMs);
  QVERIFY(runner.running());
  QVERIFY(link.sent.isEmpty());
  QCOMPARE(runner.currentIndex(), 0);

  link.paused = false;
  QTRY_VERIFY_WITH_TIMEOUT(!runner.running(), 3000);
  QCOMPARE(link.sent, QList<QByteArray>({"A", "B"}));
}

/**
 * @brief No run starts on a closed link, with nothing to send, or on top of another run.
 */
void ConsoleSendSequenceTest::startIsRefusedWhenItCannotRun()
{
  FakeLink closed;
  closed.connected = false;
  Console::SendSequencer offline(closed.callbacks());
  QVERIFY(!offline.start({pin(QStringLiteral("A"), true, 0)}, false));
  QVERIFY(!offline.running());

  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  QVERIFY(!runner.start({}, false));
  QVERIFY(!runner.start({pin(QStringLiteral("A"), false, 0)}, false));
  QVERIFY(runner.start({pin(QStringLiteral("A"), true, 500)}, false));
  QVERIFY(!runner.start({pin(QStringLiteral("B"), true, 0)}, false));
  runner.stop();
  QVERIFY(link.sent.isEmpty());
}

/**
 * @brief Editing the caller's list after the run started changes nothing that is sent.
 */
void ConsoleSendSequenceTest::listIsCopiedWhenTheRunStarts()
{
  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  QVariantList pins = {pin(QStringLiteral("A"), true, 10), pin(QStringLiteral("B"), true, 10)};

  QVERIFY(runner.start(pins, false));
  pins.clear();
  QTRY_VERIFY_WITH_TIMEOUT(!runner.running(), 3000);
  QCOMPARE(link.sent, QList<QByteArray>({"A", "B"}));
}

/**
 * @brief Once the process is sealed no run starts, whatever the list or the link.
 */
void ConsoleSendSequenceTest::sealedSessionNeverStarts()
{
  FakeLink link;
  Console::SendSequencer runner(link.callbacks());
  Core::Runtime::seal(QStringLiteral(":/ssdeploy-test"), QString());
  QVERIFY(Core::Runtime::sealed());

  QVERIFY(!runner.start({pin(QStringLiteral("A"), true, 0)}, false));
  QVERIFY(!runner.running());
  QTest::qWait(20);
  QVERIFY(link.sent.isEmpty());
}

QTEST_GUILESS_MAIN(ConsoleSendSequenceTest)

#include "tst_console_send_sequence.moc"
