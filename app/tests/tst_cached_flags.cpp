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

#include "DataModel/CachedFlag.h"

/**
 * @file tst_cached_flags.cpp
 * @brief The cached-flag cross-check (spec 0095 M1): a stale cache is repaired on the second
 *        consecutive mismatch, never the first (one queued refresh may be in flight), an unsettled
 *        flag is never judged, and a mirror without a Cached<T> is audited through its reader.
 */

namespace {

/**
 * @brief A fake owner: the truth the flag derives from, a repair counter and a settled switch.
 */
struct Owner {
  Owner();

  DataModel::CachedFlagChecker checker;
  int truth;
  int mirror;
  int repairs;
  bool settled;
  DataModel::Cached<int> flag;
};

/**
 * @brief The owner behind a checker callback.
 */
Owner& ownerOf(const void* owner)
{
  return *static_cast<Owner*>(const_cast<void*>(owner));
}

const DataModel::CachedFlagSpec kFlagSpec{
  .name  = "Owner::flag",
  .fresh = [](const void* owner) { return ownerOf(owner).truth; },
  .repair =
    [](void* owner) {
      auto& o = ownerOf(owner);
      o.flag  = o.truth;
      ++o.repairs;
    },
  .settled = [](const void* owner) { return ownerOf(owner).settled; },
  .cached  = nullptr,
};

const DataModel::CachedFlagSpec kMirrorSpec{
  .name  = "Owner::mirror",
  .fresh = [](const void* owner) { return ownerOf(owner).truth; },
  .repair =
    [](void* owner) {
      auto& o  = ownerOf(owner);
      o.mirror = o.truth;
      ++o.repairs;
    },
  .settled = nullptr,
  .cached  = [](const void* owner) { return ownerOf(owner).mirror; },
};

/**
 * @brief Registers the one Cached<int> flag; the mirror entry is added by the test that wants it.
 */
Owner::Owner()
  : checker(this), truth(0), mirror(0), repairs(0), settled(true), flag(checker, kFlagSpec, 0)
{}

}  // namespace

class CachedFlagsTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void inSyncFlagIsLeftAlone();
  void staleFlagIsRepairedOnTheSecondCheck();
  void transientMismatchIsForgiven();
  void unsettledFlagIsNeverJudged();
  void mirrorIsAuditedThroughItsReader();
  void registrationIsBounded();
};

/**
 * @brief Makes a debug build take the recovery branch instead of aborting.
 */
void CachedFlagsTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
}

/**
 * @brief A flag that matches its derivation is never repaired.
 */
void CachedFlagsTest::inSyncFlagIsLeftAlone()
{
  Owner owner;
  owner.checker.check();
  owner.checker.check();
  QCOMPARE(owner.repairs, 0);
}

/**
 * @brief The first mismatch is tolerated; the second consecutive one repairs.
 */
void CachedFlagsTest::staleFlagIsRepairedOnTheSecondCheck()
{
  Owner owner;
  owner.truth = 1;

  owner.checker.check();
  QCOMPARE(owner.repairs, 0);
  QCOMPARE(static_cast<int>(owner.flag), 0);

  owner.checker.check();
  QCOMPARE(owner.repairs, 1);
  QCOMPARE(static_cast<int>(owner.flag), 1);
}

/**
 * @brief A mismatch that clears before the next check (a refresh that was in flight) is forgiven.
 */
void CachedFlagsTest::transientMismatchIsForgiven()
{
  Owner owner;
  owner.truth = 1;
  owner.checker.check();

  owner.flag = 1;
  owner.checker.check();
  owner.truth = 2;
  owner.checker.check();
  QCOMPARE(owner.repairs, 0);
}

/**
 * @brief A flag whose owner says it is legitimately stale is skipped, however long it stays so.
 */
void CachedFlagsTest::unsettledFlagIsNeverJudged()
{
  Owner owner;
  owner.truth   = 1;
  owner.settled = false;

  for (int i = 0; i < 5; ++i)
    owner.checker.check();

  QCOMPARE(owner.repairs, 0);
}

/**
 * @brief A mirror held elsewhere (no Cached<T>) is compared through the spec's reader.
 */
void CachedFlagsTest::mirrorIsAuditedThroughItsReader()
{
  Owner owner;
  owner.checker.add(kMirrorSpec, nullptr, nullptr);
  owner.truth  = 3;
  owner.flag   = 3;
  owner.mirror = 0;

  owner.checker.check();
  owner.checker.check();
  QCOMPARE(owner.repairs, 1);
  QCOMPARE(owner.mirror, 3);
}

/**
 * @brief Registration past the fixed capacity is refused, never written out of bounds.
 */
void CachedFlagsTest::registrationIsBounded()
{
  Owner owner;
  for (std::size_t i = 0; i < DataModel::CachedFlagChecker::kMaxEntries + 4; ++i)
    owner.checker.add(kMirrorSpec, nullptr, nullptr);

  QCOMPARE(owner.checker.size(), DataModel::CachedFlagChecker::kMaxEntries);
}

QTEST_APPLESS_MAIN(CachedFlagsTest)

#include "tst_cached_flags.moc"
