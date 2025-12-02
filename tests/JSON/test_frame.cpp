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

class TestFrame : public QObject
{
  Q_OBJECT

private slots:
  void testDatasetConstruction();
  void testDatasetNumericParsing();
  void testDatasetAlignment();
  void testGroupConstruction();
  void testActionConstruction();
  void testFrameConstruction();
  void testClearFrame();
  void testCompareFrames();
  void testFrameAlignment();
};

void TestFrame::testDatasetConstruction()
{
  JSON::Dataset dataset;

  QCOMPARE(dataset.index, 0);
  QCOMPARE(dataset.xAxisId, -1);
  QCOMPARE(dataset.groupId, 0);
  QCOMPARE(dataset.uniqueId, 0);
  QCOMPARE(dataset.datasetId, 0);
  QCOMPARE(dataset.fftSamples, 256);
  QCOMPARE(dataset.fftSamplingRate, 100);
  QVERIFY(!dataset.fft);
  QVERIFY(!dataset.led);
  QVERIFY(!dataset.log);
  QVERIFY(!dataset.plt);
  QVERIFY(!dataset.alarmEnabled);
  QVERIFY(!dataset.overviewDisplay);
  QVERIFY(!dataset.isNumeric);
  QCOMPARE(dataset.wgtMax, 100.0);
}

void TestFrame::testDatasetNumericParsing()
{
  JSON::Dataset dataset;

  dataset.value = "25.5";
  dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
  QVERIFY(dataset.isNumeric);
  QCOMPARE(dataset.numericValue, 25.5);

  dataset.value = "not a number";
  dataset.isNumeric = false;
  dataset.numericValue = dataset.value.toDouble(&dataset.isNumeric);
  QVERIFY(!dataset.isNumeric);
}

void TestFrame::testDatasetAlignment()
{
  QCOMPARE(sizeof(JSON::Dataset) % alignof(JSON::Dataset), size_t(0));
}

void TestFrame::testGroupConstruction()
{
  JSON::Group group;

  QCOMPARE(group.groupId, -1);
  QVERIFY(group.title.isEmpty());
  QVERIFY(group.widget.isEmpty());
  QVERIFY(group.datasets.empty());
}

void TestFrame::testActionConstruction()
{
  JSON::Action action;

  QCOMPARE(action.actionId, -1);
  QCOMPARE(action.timerIntervalMs, 100);
  QCOMPARE(action.timerMode, JSON::TimerMode::Off);
  QVERIFY(!action.binaryData);
  QVERIFY(!action.autoExecuteOnConnect);
  QCOMPARE(action.icon, QStringLiteral("Play Property"));
}

void TestFrame::testFrameConstruction()
{
  JSON::Frame frame;

  QVERIFY(frame.title.isEmpty());
  QVERIFY(frame.groups.empty());
  QVERIFY(frame.actions.empty());
  QVERIFY(!frame.containsCommercialFeatures);
}

void TestFrame::testClearFrame()
{
  JSON::Frame frame;
  frame.title = "Test Frame";

  JSON::Group group;
  group.groupId = 0;
  group.title = "Test Group";
  frame.groups.push_back(group);

  JSON::Action action;
  action.actionId = 0;
  action.title = "Test Action";
  frame.actions.push_back(action);

  frame.containsCommercialFeatures = true;

  QVERIFY(!frame.title.isEmpty());
  QCOMPARE(frame.groups.size(), size_t(1));
  QCOMPARE(frame.actions.size(), size_t(1));
  QVERIFY(frame.containsCommercialFeatures);

  JSON::clear_frame(frame);

  QVERIFY(frame.title.isEmpty());
  QVERIFY(frame.groups.empty());
  QVERIFY(frame.actions.empty());
  QVERIFY(!frame.containsCommercialFeatures);
}

void TestFrame::testCompareFrames()
{
  JSON::Frame frame1, frame2;

  QVERIFY(JSON::compare_frames(frame1, frame2));

  JSON::Group group1;
  group1.groupId = 0;
  group1.title = "Group 1";

  JSON::Dataset dataset1;
  dataset1.index = 0;
  dataset1.title = "Dataset 1";
  group1.datasets.push_back(dataset1);

  frame1.groups.push_back(group1);
  frame2.groups.push_back(group1);

  QVERIFY(JSON::compare_frames(frame1, frame2));

  JSON::Group group2;
  group2.groupId = 1;
  frame2.groups.push_back(group2);

  QVERIFY(!JSON::compare_frames(frame1, frame2));
}

void TestFrame::testFrameAlignment()
{
  QCOMPARE(sizeof(JSON::Frame) % alignof(JSON::Frame), size_t(0));
  QCOMPARE(sizeof(JSON::Group) % alignof(JSON::Group), size_t(0));
  QCOMPARE(sizeof(JSON::Action) % alignof(JSON::Action), size_t(0));
}

QTEST_MAIN(TestFrame)
#include "test_frame.moc"
