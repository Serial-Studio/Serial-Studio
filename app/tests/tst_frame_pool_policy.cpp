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

#include "DataModel/FramePoolPolicy.h"

using DataModel::FramePoolPolicy;
using DataModel::SlotPick;

/**
 * @brief Models the caller's liveness rule: a slot is free when nothing still references the
 *        frame published from it. The tests drive this by hand so a "consumer" holding a frame
 *        for one tick is expressible without a dashboard.
 */
class Liveness {
public:
  explicit Liveness(std::size_t capacity) : m_pinned(capacity, false) {}

  void pin(std::size_t idx) { m_pinned[idx] = true; }

  void releaseAll() { std::fill(m_pinned.begin(), m_pinned.end(), false); }

  [[nodiscard]] auto fn() const
  {
    return [this](std::size_t idx) {
      return !m_pinned[idx];
    };
  }

private:
  std::vector<bool> m_pinned;
};

class FramePoolPolicyTest : public QObject {
  Q_OBJECT

private slots:
  void steadyStateNeverMaterialisesNewSlots();
  void pinnedSlotWalksToWarmSlotNotVirgin();
  void hintedOnlyRefusesRatherThanMaterialise();
  void memoryBudgetCapsMaterialisedSlots();
  void ownershipReleaseKeepsMaterialisedCount();
};

/**
 * @brief The steady state: one source publishing and consuming in lockstep must settle on a
 *        single slot. Regression guard for the pool that faulted in a fresh slot per publish and
 *        held a full frame copy in each, forever.
 */
void FramePoolPolicyTest::steadyStateNeverMaterialisesNewSlots()
{
  FramePoolPolicy policy(64);
  Liveness live(64);

  for (int i = 0; i < 500; ++i) {
    const auto [idx, pick] = policy.claim(0, false, live.fn());
    QVERIFY(idx != FramePoolPolicy::kInvalidSlot);
    Q_UNUSED(pick)
  }

  QCOMPARE(policy.materialised(), std::size_t(1));
  QCOMPARE(policy.stats().virginWalk, quint64(1));
  QCOMPARE(policy.stats().hintReuse, quint64(499));
}

/**
 * @brief When the previous frame is still pinned, the walk must reuse another slot this source
 *        already owns rather than a virgin one. Taking a virgin slot is what cost a full frame
 *        deep copy per publish; the warm slot only pays a structural rebind.
 */
void FramePoolPolicyTest::pinnedSlotWalksToWarmSlotNotVirgin()
{
  FramePoolPolicy policy(64);
  Liveness live(64);

  const auto first = policy.claim(0, false, live.fn());
  live.pin(first.first);

  const auto second = policy.claim(0, false, live.fn());
  QCOMPARE(second.second, SlotPick::VirginWalk);
  live.pin(second.first);

  live.releaseAll();
  live.pin(second.first);

  const auto third = policy.claim(0, false, live.fn());
  QCOMPARE(third.second, SlotPick::WarmWalk);
  QCOMPARE(third.first, first.first);
  QCOMPARE(policy.materialised(), std::size_t(2));
}

/**
 * @brief A synthetic refresh asks with hintedOnly and must be told "no slot" rather than be
 *        handed a virgin one: it carries no new data, so skipping is free.
 */
void FramePoolPolicyTest::hintedOnlyRefusesRatherThanMaterialise()
{
  FramePoolPolicy policy(64);
  Liveness live(64);

  const auto first = policy.claim(0, false, live.fn());
  live.pin(first.first);

  const auto refused = policy.claim(0, true, live.fn());
  QCOMPARE(refused.first, FramePoolPolicy::kInvalidSlot);
  QCOMPARE(refused.second, SlotPick::None);
  QCOMPARE(policy.materialised(), std::size_t(1));

  live.releaseAll();
  const auto reused = policy.claim(0, true, live.fn());
  QCOMPARE(reused.first, first.first);
  QCOMPARE(reused.second, SlotPick::HintReuse);
}

/**
 * @brief The budget bounds the pool by the memory it will occupy, not by a slot count tuned for
 *        small frames: once the ceiling is reached the policy steals an existing slot instead of
 *        materialising another. A 512 KB frame against a 4 MB budget allows eight slots.
 */
void FramePoolPolicyTest::memoryBudgetCapsMaterialisedSlots()
{
  FramePoolPolicy policy(8192);
  Liveness live(8192);

  policy.applyMemoryBudget(512 * 1024, 4 * 1024 * 1024);
  QCOMPARE(policy.slotBudget(), std::size_t(8));

  for (int source = 0; source < 40; ++source) {
    const auto [idx, pick] = policy.claim(source, false, live.fn());
    QVERIFY(idx != FramePoolPolicy::kInvalidSlot);
    live.pin(idx);
    Q_UNUSED(pick)
    live.releaseAll();
  }

  QCOMPARE(policy.materialised(), std::size_t(8));
  QVERIFY(policy.stats().steal > 0);
}

/**
 * @brief A structural change drops ownership so every source re-affines, but the slots already
 *        written still hold their frames: the materialised count must not reset, or the budget
 *        would let the pool grow again on every reconfigure.
 */
void FramePoolPolicyTest::ownershipReleaseKeepsMaterialisedCount()
{
  FramePoolPolicy policy(64);
  Liveness live(64);

  const auto first = policy.claim(0, false, live.fn());
  live.pin(first.first);
  const auto second = policy.claim(0, false, live.fn());
  Q_UNUSED(second)
  QCOMPARE(policy.materialised(), std::size_t(2));

  live.releaseAll();
  policy.releaseOwnership();

  const auto after = policy.claim(0, false, live.fn());
  QVERIFY(after.first != FramePoolPolicy::kInvalidSlot);
  QCOMPARE(policy.materialised(), std::size_t(2));
}

QTEST_APPLESS_MAIN(FramePoolPolicyTest)

#include "tst_frame_pool_policy.moc"
