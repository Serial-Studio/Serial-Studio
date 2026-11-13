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

#include "ApiHandlers/MessageBoxGate.h"

// The message-box rules for API and script callers (spec 0120). The 2026-11-12 rescan found a
// headless session answering "ok" to a box nobody saw, and boxes stacking without limit when a
// script called the command from its loop.

namespace Gate = API::MessageBoxGate;

class MessageBoxGateTest : public QObject {
  Q_OBJECT

private slots:
  void postWithNothingOpenProceeds();
  void anyRequestWhileABoxIsOpenIsBusy();
  void waitWithoutTheGrantIsRefusedFirst();
  void headlessWaitIsRefused();
  void grantedWaitWithADisplayProceeds();
  void headlessPostGoesToTheLog();
};

/**
 * @brief A plain post with no box open goes ahead.
 */
void MessageBoxGateTest::postWithNothingOpenProceeds()
{
  QCOMPARE(Gate::refusal(false, false, false, false), Gate::Refusal::None);
}

/**
 * @brief One API box at a time: a post or a granted wait is refused while another is open (R1).
 */
void MessageBoxGateTest::anyRequestWhileABoxIsOpenIsBusy()
{
  QCOMPARE(Gate::refusal(false, false, false, true), Gate::Refusal::Busy);
  QCOMPARE(Gate::refusal(true, true, false, true), Gate::Refusal::Busy);
}

/**
 * @brief A wait outside the script marshaller keeps its existing refusal, ahead of the others.
 */
void MessageBoxGateTest::waitWithoutTheGrantIsRefusedFirst()
{
  QCOMPARE(Gate::refusal(true, false, true, true), Gate::Refusal::WaitNotAllowed);
}

/**
 * @brief Nobody can answer a box in a headless session, so a wait is refused instead of answered
 *        "ok" (R4).
 */
void MessageBoxGateTest::headlessWaitIsRefused()
{
  QCOMPARE(Gate::refusal(true, true, true, false), Gate::Refusal::HeadlessWait);
}

/**
 * @brief The control script or a macro may wait when a display exists and nothing is open (R7).
 */
void MessageBoxGateTest::grantedWaitWithADisplayProceeds()
{
  QCOMPARE(Gate::refusal(true, true, false, false), Gate::Refusal::None);
}

/**
 * @brief A headless post is logged rather than dropped silently; with a display it is shown.
 */
void MessageBoxGateTest::headlessPostGoesToTheLog()
{
  QVERIFY(Gate::logInsteadOfShow(true));
  QVERIFY(!Gate::logInsteadOfShow(false));
}

QTEST_APPLESS_MAIN(MessageBoxGateTest)

#include "tst_message_box_gate.moc"
