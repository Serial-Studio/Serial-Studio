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

#include <chrono>
#include <memory>
#include <QTest>
#include <type_traits>

#include "DataModel/Frame.h"
#include "DataModel/FrameConsumer.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing. Every clock value is an explicit
// std::chrono::steady_clock::time_point built from a fixed baseline -- the source-owns-time rule
// means the test supplies "now", never the wall clock, except where the case is specifically about
// TimestampedFrame's own SteadyClock::now() stamping.

using namespace DataModel;

namespace {

/**
 * @brief Minimal concrete worker: the pure virtuals are unreachable from this suite, so each is a
 *        no-op that exists only to make the base class instantiable.
 */
class ConcreteFrameConsumerWorker : public FrameConsumerWorkerBase {
public:
  explicit ConcreteFrameConsumerWorker(QObject* parent = nullptr) : FrameConsumerWorkerBase(parent)
  {}

  void processData() override {}

  void close() override {}

  void flush() override {}
};

}  // namespace

static_assert(!std::is_copy_constructible_v<TimestampedFrame>);
static_assert(!std::is_copy_assignable_v<TimestampedFrame>);
static_assert(std::is_move_constructible_v<TimestampedFrame>);
static_assert(std::is_move_assignable_v<TimestampedFrame>);

/**
 * @brief Contract of FrameConsumerWorkerBase's monotonic clock and DataModel::TimestampedFrame.
 */
class TstFrameConsumer : public QObject {
  Q_OBJECT

private slots:
  void firstCallAtBaselineReturnsZero();
  void strictlyIncreasingNowValuesMatchInputDeltas();
  void identicalNowCollisionBumpsByOne();
  void nowBeforeBaselineNeverRegresses();
  void burstAtSameInstantIsStrictlyIncreasing();
  void resetMonotonicClockRestoresInitialBehavior();

  void timestampedFrameFromFrameStampsNearNow();
  void timestampedFrameMoveCtorPreservesExplicitTimestamp();
  void timestampedFramePtrRoundTripsThroughMakeShared();

  void frameConsumerConfigDefaultsMatchDocumentedValues();
};

//--------------------------------------------------------------------------------------------------
// FrameConsumerWorkerBase::monotonicFrameNs / resetMonotonicClock
//--------------------------------------------------------------------------------------------------

/**
 * @brief A freshly-constructed worker has no prior frame, so "now == baseline" reports offset zero.
 */
void TstFrameConsumer::firstCallAtBaselineReturnsZero()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();

  QCOMPARE(worker.monotonicFrameNs(baseline, baseline), qint64(0));
}

/**
 * @brief Strictly increasing "now" values with no collision pass their exact delta through.
 */
void TstFrameConsumer::strictlyIncreasingNowValuesMatchInputDeltas()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();

  QCOMPARE(worker.monotonicFrameNs(baseline + std::chrono::nanoseconds(100), baseline),
           qint64(100));
  QCOMPARE(worker.monotonicFrameNs(baseline + std::chrono::nanoseconds(250), baseline),
           qint64(250));
  QCOMPARE(worker.monotonicFrameNs(baseline + std::chrono::microseconds(1), baseline),
           qint64(1000));
}

/**
 * @brief Two frames stamped at the exact same instant must not report the exact same offset --
 *        the second collides with the first and bumps forward by one nanosecond.
 */
void TstFrameConsumer::identicalNowCollisionBumpsByOne()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();
  const auto now      = baseline + std::chrono::nanoseconds(500);

  QCOMPARE(worker.monotonicFrameNs(now, baseline), qint64(500));
  QCOMPARE(worker.monotonicFrameNs(now, baseline), qint64(501));
  QCOMPARE(worker.monotonicFrameNs(now, baseline), qint64(502));
}

/**
 * @brief A "now" that lands before the baseline (negative delta) never regresses the sequence: it
 *        bumps forward from the last reported value instead of returning the negative offset.
 */
void TstFrameConsumer::nowBeforeBaselineNeverRegresses()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();

  QCOMPARE(worker.monotonicFrameNs(baseline + std::chrono::nanoseconds(1000), baseline),
           qint64(1000));

  const auto earlyNow = baseline - std::chrono::nanoseconds(50);
  QCOMPARE(worker.monotonicFrameNs(earlyNow, baseline), qint64(1001));
  QCOMPARE(worker.monotonicFrameNs(earlyNow, baseline), qint64(1002));
}

/**
 * @brief A burst of calls sharing one instant produces a strictly increasing run: every value is
 *        exactly one more than the last, with no gaps and no repeats.
 */
void TstFrameConsumer::burstAtSameInstantIsStrictlyIncreasing()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();
  const auto now      = baseline;

  constexpr int kBurstSize = 50;
  qint64 previous          = worker.monotonicFrameNs(now, baseline);
  for (int i = 1; i < kBurstSize; ++i) {
    const qint64 current = worker.monotonicFrameNs(now, baseline);
    QCOMPARE(current, previous + 1);
    previous = current;
  }
}

/**
 * @brief resetMonotonicClock() drops all history, so the next call behaves like a first call again
 *        regardless of how far the sequence had advanced.
 */
void TstFrameConsumer::resetMonotonicClockRestoresInitialBehavior()
{
  ConcreteFrameConsumerWorker worker;
  const auto baseline = std::chrono::steady_clock::now();

  worker.monotonicFrameNs(baseline + std::chrono::seconds(1), baseline);
  worker.monotonicFrameNs(baseline + std::chrono::seconds(2), baseline);

  worker.resetMonotonicClock();

  QCOMPARE(worker.monotonicFrameNs(baseline, baseline), qint64(0));
  QCOMPARE(worker.monotonicFrameNs(baseline + std::chrono::nanoseconds(10), baseline), qint64(10));
}

//--------------------------------------------------------------------------------------------------
// DataModel::TimestampedFrame
//--------------------------------------------------------------------------------------------------

/**
 * @brief The implicit-timestamp constructor stamps with SteadyClock::now(), so the result must fall
 *        between a "before" and "after" reading taken around the call.
 */
void TstFrameConsumer::timestampedFrameFromFrameStampsNearNow()
{
  const Frame source;

  const auto before = std::chrono::steady_clock::now();
  const TimestampedFrame stamped(source);
  const auto after = std::chrono::steady_clock::now();

  QVERIFY(stamped.timestamp >= before);
  QVERIFY(stamped.timestamp <= after);
}

/**
 * @brief The move constructor carries the explicit timestamp through unchanged -- it must never be
 *        replaced by SteadyClock::now() at the point of the move.
 */
void TstFrameConsumer::timestampedFrameMoveCtorPreservesExplicitTimestamp()
{
  Frame source;
  source.title = QStringLiteral("move-ctor-fixture");

  const auto explicitTimestamp = std::chrono::steady_clock::now() - std::chrono::hours(1);

  TimestampedFrame original(std::move(source), explicitTimestamp);
  QCOMPARE(original.timestamp, explicitTimestamp);

  const TimestampedFrame moved(std::move(original));
  QCOMPARE(moved.timestamp, explicitTimestamp);
  QCOMPARE(moved.data.title, QStringLiteral("move-ctor-fixture"));
}

/**
 * @brief TimestampedFramePtr is a plain std::shared_ptr<TimestampedFrame>; std::make_shared must
 *        round-trip both the frame payload and the explicit timestamp.
 */
void TstFrameConsumer::timestampedFramePtrRoundTripsThroughMakeShared()
{
  Frame source;
  source.sourceId = 7;

  const auto explicitTimestamp  = std::chrono::steady_clock::now();
  const TimestampedFramePtr ptr = std::make_shared<TimestampedFrame>(source, explicitTimestamp);

  QVERIFY(static_cast<bool>(ptr));
  QCOMPARE(ptr->data.sourceId, 7);
  QCOMPARE(ptr->timestamp, explicitTimestamp);
  QCOMPARE(ptr.use_count(), long(1));
}

//--------------------------------------------------------------------------------------------------
// FrameConsumerConfig
//--------------------------------------------------------------------------------------------------

/**
 * @brief The default configuration is a deterministic contract every consumer relies on when it
 *        omits an explicit FrameConsumerConfig.
 */
void TstFrameConsumer::frameConsumerConfigDefaultsMatchDocumentedValues()
{
  const FrameConsumerConfig config;

  QCOMPARE(config.queueCapacity, size_t(8192));
  QCOMPARE(config.flushThreshold, size_t(1024));
  QCOMPARE(config.timerIntervalMs, 1000);
}

QTEST_APPLESS_MAIN(TstFrameConsumer)

#include "tst_frame_consumer.moc"
