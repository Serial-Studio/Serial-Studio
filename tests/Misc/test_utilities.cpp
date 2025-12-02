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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include <QtTest>
#include "Misc/Utilities.h"

class TestUtilities : public QObject
{
  Q_OBJECT

private slots:
  void testSingleton();
  void testHdpiImagePath();
};

void TestUtilities::testSingleton()
{
  Misc::Utilities &instance1 = Misc::Utilities::instance();
  Misc::Utilities &instance2 = Misc::Utilities::instance();
  QCOMPARE(&instance1, &instance2);
}

void TestUtilities::testHdpiImagePath()
{
  const QString path = Misc::Utilities::hdpiImagePath("test.png");
  QVERIFY(!path.isEmpty());
}

QTEST_MAIN(TestUtilities)
#include "test_utilities.moc"
