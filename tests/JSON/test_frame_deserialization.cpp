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
#include <QFile>
#include <QJsonDocument>
#include "JSON/Frame.h"

class TestFrameDeserialization : public QObject
{
  Q_OBJECT

private slots:
  void initTestCase();
  void testSimpleFrameDeserialization();
  void testComplexFrameDeserialization();
  void testEmptyFrameDeserialization();
  void testMalformedJSON();
  void testMissingFields();
  void cleanupTestCase();

private:
  QJsonObject m_fixtures;
};

void TestFrameDeserialization::initTestCase()
{
  QFile file("fixtures/sample_frames.json");
  QVERIFY2(file.open(QIODevice::ReadOnly),
           "Failed to open sample_frames.json");

  const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QVERIFY2(!doc.isNull(), "Failed to parse sample_frames.json");

  const QJsonObject root = doc.object();
  QVERIFY2(root.contains("frames"),
           "sample_frames.json missing 'frames' key");

  m_fixtures = root["frames"].toObject();
  QVERIFY2(!m_fixtures.isEmpty(), "No fixtures loaded");
}

void TestFrameDeserialization::testSimpleFrameDeserialization()
{
  const QJsonObject frameObj = m_fixtures["simple_frame"].toObject();
  QVERIFY(!frameObj.isEmpty());

  JSON::Frame frame;
  const bool success = JSON::read(frame, frameObj);

  QVERIFY(success);
  QCOMPARE(frame.title, QStringLiteral("Simple Test Frame"));
  QCOMPARE(frame.groups.size(), size_t(1));
  QCOMPARE(frame.actions.size(), size_t(0));

  const JSON::Group &group = frame.groups[0];
  QCOMPARE(group.title, QStringLiteral("Temperature Sensor"));
  QCOMPARE(group.datasets.size(), size_t(1));

  const JSON::Dataset &dataset = group.datasets[0];
  QCOMPARE(dataset.title, QStringLiteral("Temperature"));
  QCOMPARE(dataset.value, QStringLiteral("25.5"));
  QCOMPARE(dataset.units, QStringLiteral("°C"));
  QCOMPARE(dataset.index, 0);
  QVERIFY(dataset.plt);
  QCOMPARE(dataset.widget, QStringLiteral("gauge"));
}

void TestFrameDeserialization::testComplexFrameDeserialization()
{
  const QJsonObject frameObj = m_fixtures["complex_frame"].toObject();
  QVERIFY(!frameObj.isEmpty());

  JSON::Frame frame;
  const bool success = JSON::read(frame, frameObj);

  QVERIFY(success);
  QCOMPARE(frame.title, QStringLiteral("Complex Test Frame"));
  QCOMPARE(frame.groups.size(), size_t(2));
  QCOMPARE(frame.actions.size(), size_t(1));

  const JSON::Group &group1 = frame.groups[0];
  QCOMPARE(group1.title, QStringLiteral("Environmental Sensors"));
  QCOMPARE(group1.datasets.size(), size_t(2));

  const JSON::Dataset &dataset1 = group1.datasets[0];
  QCOMPARE(dataset1.title, QStringLiteral("Temperature"));
  QVERIFY(dataset1.plt);
  QVERIFY(dataset1.log);
  QVERIFY(dataset1.overviewDisplay);

  const JSON::Dataset &dataset2 = group1.datasets[1];
  QCOMPARE(dataset2.title, QStringLiteral("Humidity"));
  QVERIFY(dataset2.led);
  QCOMPARE(dataset2.ledHigh, 80.0);

  const JSON::Action &action = frame.actions[0];
  QCOMPARE(action.title, QStringLiteral("Start Recording"));
  QCOMPARE(action.txData, QStringLiteral("START"));
  QCOMPARE(action.eolSequence, QStringLiteral("\r\n"));
}

void TestFrameDeserialization::testEmptyFrameDeserialization()
{
  const QJsonObject frameObj = m_fixtures["empty_frame"].toObject();
  QVERIFY(!frameObj.isEmpty());

  JSON::Frame frame;
  const bool success = JSON::read(frame, frameObj);

  QVERIFY(!success);
}

void TestFrameDeserialization::testMalformedJSON()
{
  QJsonObject emptyObj;
  JSON::Frame frame;

  const bool success = JSON::read(frame, emptyObj);
  QVERIFY(!success);
}

void TestFrameDeserialization::testMissingFields()
{
  QJsonObject incompleteFrame;
  incompleteFrame["title"] = "Incomplete";

  JSON::Frame frame;
  const bool success = JSON::read(frame, incompleteFrame);

  QVERIFY(!success);
}

void TestFrameDeserialization::cleanupTestCase()
{
  m_fixtures = QJsonObject();
}

QTEST_MAIN(TestFrameDeserialization)
#include "test_frame_deserialization.moc"
