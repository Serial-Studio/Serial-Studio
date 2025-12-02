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
#include "JSON/FrameBuilder.h"

class TestFrameBuilder : public QObject
{
  Q_OBJECT

private slots:
  void testSingleton();
  void testOperationMode();
};

void TestFrameBuilder::testSingleton()
{
  JSON::FrameBuilder &instance1 = JSON::FrameBuilder::instance();
  JSON::FrameBuilder &instance2 = JSON::FrameBuilder::instance();
  QCOMPARE(&instance1, &instance2);
}

void TestFrameBuilder::testOperationMode()
{
  QSKIP("FrameBuilder requires full application context");
}

QTEST_MAIN(TestFrameBuilder)
#include "test_frame_builder.moc"
