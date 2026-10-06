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

#include "Core/License.h"

// The lazy-trial seam contract (spec 0092): the atomic license state carries the trial day
// count next to the three original facts, and the Pro-intent request is a silent no-op until
// the composition root installs a handler, is delivered verbatim while one is installed, and
// goes silent again once it is cleared. Gate sites rely on exactly these properties to stay
// safe in GPL and headless roots, where nothing ever installs a handler.

/**
 * @brief Pins the Core::License state fields and the requestProFeature handler lifecycle.
 */
class TestLicenseSeam : public QObject {
  Q_OBJECT

private slots:
  void init();
  void stateDefaultsToNoTrial();
  void stateRoundTripsTrialDays();
  void requestWithoutHandlerIsSilent();
  void requestDeliversFeatureAndRetry();
  void clearedHandlerStopsDelivery();
};

/**
 * @brief Resets the module-static state before every case; the seam has no other reset path.
 */
void TestLicenseSeam::init()
{
  Core::License::setProFeatureHandler({});
  Core::License::set(false, 0, false, -1);
}

/**
 * @brief The reset state reports no trial registered.
 */
void TestLicenseSeam::stateDefaultsToNoTrial()
{
  QCOMPARE(Core::License::trialDaysRemaining(), -1);
  QCOMPARE(Core::License::activated(), false);
  QCOMPARE(Core::License::trialExpired(), false);
}

/**
 * @brief Every field written through set() reads back, days included.
 */
void TestLicenseSeam::stateRoundTripsTrialDays()
{
  Core::License::set(true, 2, false, 9);
  QCOMPARE(Core::License::activated(), true);
  QCOMPARE(Core::License::tier(), quint8(2));
  QCOMPARE(Core::License::trialExpired(), false);
  QCOMPARE(Core::License::trialDaysRemaining(), 9);

  Core::License::set(false, 0, true, 0);
  QCOMPARE(Core::License::activated(), false);
  QCOMPARE(Core::License::trialExpired(), true);
  QCOMPARE(Core::License::trialDaysRemaining(), 0);
}

/**
 * @brief A gate site's request is a no-op with no handler installed (GPL/headless roots).
 */
void TestLicenseSeam::requestWithoutHandlerIsSilent()
{
  bool ran = false;
  Core::License::requestProFeature(QStringLiteral("driver.connect"), [&ran] { ran = true; });
  QCOMPARE(ran, false);
}

/**
 * @brief An installed handler receives the feature id and a runnable retry closure.
 */
void TestLicenseSeam::requestDeliversFeatureAndRetry()
{
  QString seen;
  bool retried = false;
  Core::License::setProFeatureHandler(
    [&seen](const QString& id, Core::License::ProFeatureRetry retry) {
      seen = id;
      if (retry)
        retry();
    });

  Core::License::requestProFeature(QStringLiteral("mdf4.playback"), [&retried] { retried = true; });
  QCOMPARE(seen, QStringLiteral("mdf4.playback"));
  QCOMPARE(retried, true);

  seen.clear();
  Core::License::requestProFeature(QStringLiteral("dashboard.freeze"));
  QCOMPARE(seen, QStringLiteral("dashboard.freeze"));
}

/**
 * @brief Clearing the handler (TrialGate teardown) stops delivery without touching callers.
 */
void TestLicenseSeam::clearedHandlerStopsDelivery()
{
  int calls = 0;
  Core::License::setProFeatureHandler(
    [&calls](const QString&, Core::License::ProFeatureRetry) { ++calls; });
  Core::License::requestProFeature(QStringLiteral("output.send"));
  QCOMPARE(calls, 1);

  Core::License::setProFeatureHandler({});
  Core::License::requestProFeature(QStringLiteral("output.send"));
  QCOMPARE(calls, 1);
}

QTEST_GUILESS_MAIN(TestLicenseSeam)
#include "tst_license_seam.moc"
