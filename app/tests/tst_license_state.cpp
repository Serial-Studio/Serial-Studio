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

#include <QTest>

#include "Core/License.h"

/**
 * @brief Contract of the Core licensing flag (spec 0077): closed until the root publishes, every
 *        field readable after one set(), and a later set() replaces all three at once.
 */
class TstLicenseState : public QObject {
  Q_OBJECT

private slots:
  void defaultsToNotActivated();
  void setPublishesEveryField();
  void laterSetReplacesEarlierFacts();
};

/**
 * @brief A process that never installed a token reads a closed gate and tier zero.
 */
void TstLicenseState::defaultsToNotActivated()
{
  QVERIFY(!Core::License::activated());
  QCOMPARE(Core::License::tier(), quint8(0));
  QVERIFY(!Core::License::trialExpired());
}

/**
 * @brief One set() exposes activation, tier and trial state together.
 */
void TstLicenseState::setPublishesEveryField()
{
  Core::License::set(true, 3, false, -1);
  QVERIFY(Core::License::activated());
  QCOMPARE(Core::License::tier(), quint8(3));
  QVERIFY(!Core::License::trialExpired());
}

/**
 * @brief A deactivation replaces the earlier facts rather than merging with them.
 */
void TstLicenseState::laterSetReplacesEarlierFacts()
{
  Core::License::set(true, 2, false, -1);
  Core::License::set(false, 0, true, 0);
  QVERIFY(!Core::License::activated());
  QCOMPARE(Core::License::tier(), quint8(0));
  QVERIFY(Core::License::trialExpired());
}

QTEST_GUILESS_MAIN(TstLicenseState)
#include "tst_license_state.moc"
