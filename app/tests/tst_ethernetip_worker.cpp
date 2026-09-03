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

#include <QSignalSpy>
#include <QTest>

#include "IO/Drivers/PolledPlcWorkerBase.h"

using IO::Drivers::PolledPlcWorkerBase;
using IO::Drivers::OpcUaWire::Entry;
using IO::Drivers::OpcUaWire::Type;

/**
 * @brief A concrete worker whose protocol half is a script: connectToPlc() answers what the test
 *        told it to answer and pollTick() records that it ran. Everything else under test is the
 *        base's, which is exactly the point -- S7 and EtherNet/IP share this half (spec 0075 E8).
 */
class StubPlcWorker : public PolledPlcWorkerBase {
  Q_OBJECT

public:
  explicit StubPlcWorker(bool dialSucceeds);

  StubPlcWorker(StubPlcWorker&&)                 = delete;
  StubPlcWorker(const StubPlcWorker&)            = delete;
  StubPlcWorker& operator=(StubPlcWorker&&)      = delete;
  StubPlcWorker& operator=(const StubPlcWorker&) = delete;

  using PolledPlcWorkerBase::aborted;
  using PolledPlcWorkerBase::channelCount;
  using PolledPlcWorkerBase::clearAbort;
  using PolledPlcWorkerBase::configureChannels;
  using PolledPlcWorkerBase::countReadsFailed;
  using PolledPlcWorkerBase::countReadsOk;
  using PolledPlcWorkerBase::latchChannel;
  using PolledPlcWorkerBase::noteDialError;
  using PolledPlcWorkerBase::pollIntervalMs;
  using PolledPlcWorkerBase::publishDirtySlots;
  using PolledPlcWorkerBase::reportFailure;
  using PolledPlcWorkerBase::sessionOpen;

  [[nodiscard]] int releaseCount() const noexcept;

private:
  [[nodiscard]] bool connectToPlc() override;
  void pollTick() override;
  void releaseResources() override;

  bool m_dialSucceeds;
  int m_releases;
};

/**
 * @brief Builds a stub whose dial answers @p dialSucceeds and which has released nothing yet.
 */
StubPlcWorker::StubPlcWorker(bool dialSucceeds) : m_dialSucceeds(dialSucceeds), m_releases(0) {}

/**
 * @brief How often the base asked the derived half to release its protocol resources.
 */
int StubPlcWorker::releaseCount() const noexcept
{
  return m_releases;
}

/**
 * @brief Answers the scripted verdict, recording a reason on the failure path exactly as the two
 *        real workers do.
 */
bool StubPlcWorker::connectToPlc()
{
  clearAbort();
  if (!m_dialSucceeds) {
    noteDialError(QStringLiteral("no route to controller"));
    shutdown();
    return false;
  }

  noteDialError(QString());
  return true;
}

/**
 * @brief The poll body is not what this suite pins; the timer path is driven directly.
 */
void StubPlcWorker::pollTick() {}

/**
 * @brief Counts how often the base asked the derived half to let go of its protocol resources.
 */
void StubPlcWorker::releaseResources()
{
  ++m_releases;
}

/**
 * @brief The half of a polled-PLC worker S7 and EtherNet/IP now share: the delta encoder that only
 *        puts CHANGED channels on the wire, the counters the pane pulls at 1 Hz, the report-once
 *        link-loss edge and the dial verdict the spec-0050 latch depends on being emitted exactly
 *        once per attempt.
 */
class TstEthernetIpWorker : public QObject {
  Q_OBJECT

private slots:
  void anUnchangedValueCostsNoWireEntry();
  void onlyDirtyChannelsReachTheFrame();
  void aFrameIsPublishedOncePerChangeSet();
  void countersAccumulateWhatThePollReported();
  void theLinkIsLostExactlyOnce();
  void theDialVerdictIsReportedOncePerAttempt();
  void abortLatchesUntilTheNextAttemptClearsIt();
};

/**
 * @brief Decodes an encoded delta frame into its entries; an unparsable frame yields none, which
 *        is the failure the assertions below would catch.
 */
[[nodiscard]] static QList<Entry> decodeFrame(const QByteArray& frame)
{
  QList<Entry> entries;
  if (!IO::Drivers::OpcUaWire::checkHeader(frame))
    return entries;

  qsizetype pos = IO::Drivers::OpcUaWire::kHeaderBytes;
  Entry entry;
  while (IO::Drivers::OpcUaWire::readEntry(frame, pos, entry))
    entries.append(entry);

  return entries;
}

/**
 * @brief A delta frame carries what moved. Latching the same value twice must not mark the channel
 *        dirty again, or a controller answering with a constant would cost a wire entry per tick
 *        for the life of the session.
 */
void TstEthernetIpWorker::anUnchangedValueCostsNoWireEntry()
{
  StubPlcWorker worker(true);
  worker.configureChannels(100, {Type::I32, Type::F64});

  QCOMPARE(worker.channelCount(), 2);
  QCOMPARE(worker.pollIntervalMs(), 100);
  QVERIFY(worker.latchChannel(0, QVariant(7)));
  QVERIFY(!worker.latchChannel(0, QVariant(7)));
  QVERIFY(worker.latchChannel(0, QVariant(8)));
}

/**
 * @brief Only the channels that moved are encoded, and each one appears at its own wire index; a
 *        frame that carried every channel would defeat the latch the templates rely on.
 */
void TstEthernetIpWorker::onlyDirtyChannelsReachTheFrame()
{
  StubPlcWorker worker(true);
  worker.configureChannels(50, {Type::I32, Type::I32, Type::Bool});

  QSignalSpy frames(&worker, &PolledPlcWorkerBase::frameReady);
  QVERIFY(worker.latchChannel(2, QVariant(true)));
  worker.publishDirtySlots(1000);

  QCOMPARE(frames.count(), 1);
  const auto entries = decodeFrame(frames.at(0).at(0).toByteArray());
  QCOMPARE(entries.size(), 1);
  QCOMPARE(entries.at(0).index, 2);
  QCOMPARE(entries.at(0).type, Type::Bool);
  QCOMPARE(entries.at(0).text, QStringLiteral("1"));
  QCOMPARE(frames.at(0).at(1).toLongLong(), qint64(1000));
}

/**
 * @brief The dirty marks are consumed by the publish, so a second publish with nothing new emits
 *        no frame at all and the counter does not move.
 */
void TstEthernetIpWorker::aFrameIsPublishedOncePerChangeSet()
{
  StubPlcWorker worker(true);
  worker.configureChannels(50, {Type::U16});

  QSignalSpy frames(&worker, &PolledPlcWorkerBase::frameReady);
  QVERIFY(worker.latchChannel(0, QVariant(42u)));
  worker.publishDirtySlots(10);
  worker.publishDirtySlots(20);

  QCOMPARE(frames.count(), 1);
  QCOMPARE(worker.framesPublished(), quint64(1));

  QVERIFY(worker.latchChannel(0, QVariant(43u)));
  worker.publishDirtySlots(30);
  QCOMPARE(frames.count(), 2);
  QCOMPARE(worker.framesPublished(), quint64(2));
}

/**
 * @brief The three counters are pulled, never pushed (spec 0033): they only ever accumulate what
 *        the poll reported, and the pane reads them without a signal.
 */
void TstEthernetIpWorker::countersAccumulateWhatThePollReported()
{
  StubPlcWorker worker(true);
  worker.configureChannels(50, {Type::I8});

  QCOMPARE(worker.readsOk(), quint64(0));
  QCOMPARE(worker.readsFailed(), quint64(0));
  QCOMPARE(worker.framesPublished(), quint64(0));

  worker.countReadsOk(3);
  worker.countReadsFailed(2);
  worker.countReadsFailed(5);

  QCOMPARE(worker.readsOk(), quint64(3));
  QCOMPARE(worker.readsFailed(), quint64(7));
}

/**
 * @brief A dropped link is reported ONCE: the driver turns the signal into a queued disconnect, so
 *        a second report would tear down a device the first one already closed.
 */
void TstEthernetIpWorker::theLinkIsLostExactlyOnce()
{
  StubPlcWorker worker(true);
  worker.configureChannels(50, {Type::I8});

  QSignalSpy lost(&worker, &PolledPlcWorkerBase::linkLost);
  worker.reportFailure(QStringLiteral("the controller stopped answering"));
  worker.reportFailure(QStringLiteral("the controller stopped answering"));

  QCOMPARE(lost.count(), 1);
  QCOMPARE(lost.at(0).at(0).toString(), QStringLiteral("the controller stopped answering"));
  QVERIFY(!worker.sessionOpen());
}

/**
 * @brief beginDial() emits dialFinished exactly once per attempt, carrying the reason the derived
 *        half recorded. The spec-0050 latch wedges the connect button on a driver that reports
 *        only success, so the failure path is pinned here too.
 */
void TstEthernetIpWorker::theDialVerdictIsReportedOncePerAttempt()
{
  StubPlcWorker good(true);
  good.configureChannels(50, {Type::I8});
  QSignalSpy okVerdicts(&good, &PolledPlcWorkerBase::dialFinished);
  good.beginDial();
  QCOMPARE(okVerdicts.count(), 1);
  QCOMPARE(okVerdicts.at(0).at(0).toBool(), true);
  QCOMPARE(okVerdicts.at(0).at(1).toString(), QString());

  StubPlcWorker bad(false);
  bad.configureChannels(50, {Type::I8});
  QSignalSpy badVerdicts(&bad, &PolledPlcWorkerBase::dialFinished);
  bad.beginDial();
  QCOMPARE(badVerdicts.count(), 1);
  QCOMPARE(badVerdicts.at(0).at(0).toBool(), false);
  QCOMPARE(badVerdicts.at(0).at(1).toString(), QStringLiteral("no route to controller"));
  QCOMPARE(bad.releaseCount(), 1);
}

/**
 * @brief requestAbort() is set from the GUI thread before the blocking teardown invoke, so it has
 *        to stay latched until the next attempt clears it; a worker that forgot to clear would
 *        return at its first abort check and never poll again.
 */
void TstEthernetIpWorker::abortLatchesUntilTheNextAttemptClearsIt()
{
  StubPlcWorker worker(true);
  worker.configureChannels(50, {Type::I8});

  QVERIFY(!worker.aborted());
  worker.requestAbort();
  QVERIFY(worker.aborted());
  worker.clearAbort();
  QVERIFY(!worker.aborted());
}

QTEST_GUILESS_MAIN(TstEthernetIpWorker)

#include "tst_ethernetip_worker.moc"
