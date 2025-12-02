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

class TestFrameSerialization : public QObject
{
  Q_OBJECT

private slots:
  void testDatasetSerialization();
  void testGroupSerialization();
  void testActionSerialization();
  void testFrameSerialization();
  void testMinMaxOrdering();
};

void TestFrameSerialization::testDatasetSerialization()
{
  JSON::Dataset dataset;
  dataset.title = "Temperature";
  dataset.value = "25.5";
  dataset.units = "°C";
  dataset.index = 0;
  dataset.plt = true;
  dataset.pltMin = 0.0;
  dataset.pltMax = 100.0;
  dataset.widget = "gauge";

  const QJsonObject obj = JSON::serialize(dataset);

  QCOMPARE(obj[Keys::Title].toString(), dataset.title);
  QCOMPARE(obj[Keys::Value].toString(), dataset.value);
  QCOMPARE(obj[Keys::Units].toString(), dataset.units);
  QCOMPARE(obj[Keys::Index].toInt(), dataset.index);
  QCOMPARE(obj[Keys::Graph].toBool(), dataset.plt);
  QCOMPARE(obj[Keys::Widget].toString(), dataset.widget);
  QCOMPARE(obj[Keys::PltMin].toDouble(), 0.0);
  QCOMPARE(obj[Keys::PltMax].toDouble(), 100.0);
}

void TestFrameSerialization::testGroupSerialization()
{
  JSON::Group group;
  group.title = "Sensors";
  group.widget = "container";

  JSON::Dataset dataset;
  dataset.title = "Temperature";
  dataset.value = "25.5";
  group.datasets.push_back(dataset);

  const QJsonObject obj = JSON::serialize(group);

  QCOMPARE(obj[Keys::Title].toString(), group.title);
  QCOMPARE(obj[Keys::Widget].toString(), group.widget);
  QVERIFY(obj.contains(Keys::Datasets));

  const QJsonArray datasets = obj[Keys::Datasets].toArray();
  QCOMPARE(datasets.size(), 1);
}

void TestFrameSerialization::testActionSerialization()
{
  JSON::Action action;
  action.icon = "Play";
  action.title = "Start";
  action.txData = "START";
  action.eolSequence = "\r\n";
  action.binaryData = false;
  action.timerIntervalMs = 1000;
  action.timerMode = JSON::TimerMode::AutoStart;
  action.autoExecuteOnConnect = true;

  const QJsonObject obj = JSON::serialize(action);

  QCOMPARE(obj[Keys::Icon].toString(), action.icon);
  QCOMPARE(obj[Keys::Title].toString(), action.title);
  QCOMPARE(obj[Keys::TxData].toString(), action.txData);
  QCOMPARE(obj[Keys::EOL].toString(), action.eolSequence);
  QCOMPARE(obj[Keys::Binary].toBool(), action.binaryData);
  QCOMPARE(obj[Keys::TimerInterval].toInt(), action.timerIntervalMs);
  QCOMPARE(obj[Keys::AutoExecute].toBool(), action.autoExecuteOnConnect);
}

void TestFrameSerialization::testFrameSerialization()
{
  JSON::Frame frame;
  frame.title = "Test Frame";

  JSON::Group group;
  group.groupId = 0;
  group.title = "Group 1";

  JSON::Dataset dataset;
  dataset.title = "Data 1";
  dataset.value = "10";
  group.datasets.push_back(dataset);

  frame.groups.push_back(group);

  JSON::Action action;
  action.title = "Action 1";
  frame.actions.push_back(action);

  const QJsonObject obj = JSON::serialize(frame);

  QCOMPARE(obj[Keys::Title].toString(), frame.title);
  QVERIFY(obj.contains(Keys::Groups));
  QVERIFY(obj.contains(Keys::Actions));

  const QJsonArray groups = obj[Keys::Groups].toArray();
  const QJsonArray actions = obj[Keys::Actions].toArray();

  QCOMPARE(groups.size(), 1);
  QCOMPARE(actions.size(), 1);
}

void TestFrameSerialization::testMinMaxOrdering()
{
  JSON::Dataset dataset;
  dataset.pltMin = 100.0;
  dataset.pltMax = 0.0;
  dataset.fftMin = 50.0;
  dataset.fftMax = 10.0;
  dataset.wgtMin = 200.0;
  dataset.wgtMax = 100.0;

  const QJsonObject obj = JSON::serialize(dataset);

  QVERIFY(obj[Keys::PltMin].toDouble() <= obj[Keys::PltMax].toDouble());
  QVERIFY(obj[Keys::FFTMin].toDouble() <= obj[Keys::FFTMax].toDouble());
  QVERIFY(obj[Keys::WgtMin].toDouble() <= obj[Keys::WgtMax].toDouble());

  QCOMPARE(obj[Keys::PltMin].toDouble(), 0.0);
  QCOMPARE(obj[Keys::PltMax].toDouble(), 100.0);
}

QTEST_MAIN(TestFrameSerialization)
#include "test_frame_serialization.moc"
