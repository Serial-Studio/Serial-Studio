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
#include "JSON/Frame.h"

class TestFrameUtilities : public QObject
{
  Q_OBJECT

private slots:
  void testClearFrame();
  void testCompareFrames();
  void testFinalizeFrame();
};

void TestFrameUtilities::testClearFrame()
{
  JSON::Frame frame;
  frame.title = "Test";
  JSON::clear_frame(frame);
  QVERIFY(frame.title.isEmpty());
}

void TestFrameUtilities::testCompareFrames()
{
  JSON::Frame frame1, frame2;
  QVERIFY(JSON::compare_frames(frame1, frame2));
}

void TestFrameUtilities::testFinalizeFrame()
{
  JSON::Frame frame;
  JSON::finalize_frame(frame);
  QVERIFY(true);
}

QTEST_MAIN(TestFrameUtilities)
#include "test_frame_utilities.moc"
