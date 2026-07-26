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

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include "DataModel/Frame.h"
#include "DataModel/Importers/ProtoImporter.h"
#include "SerialStudio.h"
#include "SessionContext.h"

// The point of this suite is the seam, not the parser: a SessionContext and a ProtoImporter are
// built on the stack, so the generation path runs with no composition root, no QML engine and no
// project model. Every assertion below reads only the returned JSON.

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

// Two top-level messages plus one nested reference: Attitude emits its own group, Telemetry emits a
// group whose message-typed field is skipped as a dataset, and the reference emits a third group
// titled after the field rather than the message.
static const char* kFixtureProto = R"proto(
syntax = "proto3";
package telemetry;

message Attitude {
  float roll = 1;
  float pitch = 2;
  float yaw = 3;
}

message Telemetry {
  uint32 timestamp = 1;
  double altitude = 2;
  bool armed = 3;
  string status = 4;
  Attitude attitude = 5;
}
)proto";

/**
 * @brief Generation contract of DataModel::ProtoImporter under constructor injection (spec 0039).
 */
class TstProtoImporter : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  void contextIsTheProcessDefaultShape();
  void groupsFollowTopLevelAndNestedMessages();
  void datasetsCoverEveryScalarField();
  void datasetIndicesAreDenseAndOrdered();
  void booleanFieldsBecomeLeds();
  void sourceCarriesGeneratedLuaParser();
  void unreadableFileYieldsEmptyProject();

private:
  QString m_fixture;
  QTemporaryDir m_dir;
};

//--------------------------------------------------------------------------------------------------
// Setup
//--------------------------------------------------------------------------------------------------

void TstProtoImporter::initTestCase()
{
  QVERIFY(m_dir.isValid());

  m_fixture = m_dir.filePath(QStringLiteral("telemetry.proto"));
  QFile file(m_fixture);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QVERIFY(file.write(kFixtureProto) > 0);
  file.close();
}

void TstProtoImporter::cleanupTestCase()
{
  m_fixture.clear();
}

//--------------------------------------------------------------------------------------------------
// The seam
//--------------------------------------------------------------------------------------------------

/**
 * @brief A stack context is a complete dependency for the importer: it identifies itself and
 *        constructs nothing, which is what lets this suite skip the composition root.
 */
void TstProtoImporter::contextIsTheProcessDefaultShape()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  QCOMPARE(ctx.sessionId(), 0);
  QCOMPARE(SessionContext::current().sessionId(), 0);
  QCOMPARE(importer.messageCount(), 0);
  QCOMPARE(importer.fieldCount(), 0);
}

//--------------------------------------------------------------------------------------------------
// Generated project shape
//--------------------------------------------------------------------------------------------------

/**
 * @brief One group per top-level message plus one per resolved message-typed field; the nested
 *        group takes the field name, not the message name.
 */
void TstProtoImporter::groupsFollowTopLevelAndNestedMessages()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  const auto project = importer.projectFromProtoFile(m_fixture);
  const auto groups  = project[Keys::Groups].toArray();

  QCOMPARE(groups.size(), 3);
  QCOMPARE(groups.at(0).toObject()[Keys::Title].toString(), QStringLiteral("Attitude"));
  QCOMPARE(groups.at(1).toObject()[Keys::Title].toString(), QStringLiteral("Telemetry"));
  QCOMPARE(groups.at(2).toObject()[Keys::Title].toString(), QStringLiteral("attitude"));
  QCOMPARE(project[Keys::Title].toString(), QStringLiteral("telemetry"));
}

/**
 * @brief Scalar fields become datasets titled after the field; message-typed fields do not.
 */
void TstProtoImporter::datasetsCoverEveryScalarField()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  const auto project = importer.projectFromProtoFile(m_fixture);
  const auto groups  = project[Keys::Groups].toArray();
  QCOMPARE(groups.size(), 3);

  const auto attitude = groups.at(0).toObject()[Keys::Datasets].toArray();
  QCOMPARE(attitude.size(), 3);
  QCOMPARE(attitude.at(0).toObject()[Keys::Title].toString(), QStringLiteral("roll"));
  QCOMPARE(attitude.at(1).toObject()[Keys::Title].toString(), QStringLiteral("pitch"));
  QCOMPARE(attitude.at(2).toObject()[Keys::Title].toString(), QStringLiteral("yaw"));

  const auto telemetry = groups.at(1).toObject()[Keys::Datasets].toArray();
  QCOMPARE(telemetry.size(), 4);
  QCOMPARE(telemetry.at(0).toObject()[Keys::Title].toString(), QStringLiteral("timestamp"));
  QCOMPARE(telemetry.at(3).toObject()[Keys::Title].toString(), QStringLiteral("status"));

  QCOMPARE(groups.at(2).toObject()[Keys::Datasets].toArray().size(), 3);
  QCOMPARE(importer.messageCount(), 2);
  QCOMPARE(importer.fieldCount(), 10);
}

/**
 * @brief Dataset indices are the parser's dispatch slots: they run 1..N across every group in
 *        emission order, so a gap or a repeat silently misroutes a field at runtime.
 */
void TstProtoImporter::datasetIndicesAreDenseAndOrdered()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  const auto project = importer.projectFromProtoFile(m_fixture);
  const auto groups  = project[Keys::Groups].toArray();

  int expected = 1;
  for (const auto& group : groups) {
    const auto datasets = group.toObject()[Keys::Datasets].toArray();
    for (const auto& dataset : datasets) {
      QCOMPARE(dataset.toObject()[Keys::Index].toInt(), expected);
      ++expected;
    }
  }

  QCOMPARE(expected, 11);
}

/**
 * @brief A proto3 bool maps to an LED dataset, a string to a plain readout; neither is plotted.
 */
void TstProtoImporter::booleanFieldsBecomeLeds()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  const auto project   = importer.projectFromProtoFile(m_fixture);
  const auto telemetry = project[Keys::Groups].toArray().at(1).toObject();
  const auto datasets  = telemetry[Keys::Datasets].toArray();
  QCOMPARE(datasets.size(), 4);

  const auto armed  = datasets.at(2).toObject();
  const auto status = datasets.at(3).toObject();

  QCOMPARE(armed[Keys::Title].toString(), QStringLiteral("armed"));
  QVERIFY(armed[Keys::LED].toBool());
  QVERIFY(!armed[Keys::Graph].toBool());

  QCOMPARE(status[Keys::Title].toString(), QStringLiteral("status"));
  QVERIFY(!status[Keys::LED].toBool());
  QVERIFY(!status[Keys::Graph].toBool());

  QVERIFY(datasets.at(1).toObject()[Keys::Graph].toBool());
}

/**
 * @brief The single generated source carries the Lua decoder; without it the datasets never fill.
 */
void TstProtoImporter::sourceCarriesGeneratedLuaParser()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  const auto project = importer.projectFromProtoFile(m_fixture);
  const auto sources = project[Keys::Sources].toArray();
  QCOMPARE(sources.size(), 1);

  const auto source = sources.at(0).toObject();
  QCOMPARE(source[Keys::SourceId].toInt(), 0);
  QCOMPARE(source[Keys::FrameParserLanguage].toInt(), static_cast<int>(SerialStudio::Lua));
  QVERIFY(source[Keys::FrameParserCode].toString().contains(QStringLiteral("function parse")));
}

/**
 * @brief The pure path reports failure by returning nothing: no dialog, no message box, no
 *        session state reached, which is what makes it callable from here.
 */
void TstProtoImporter::unreadableFileYieldsEmptyProject()
{
  SessionContext ctx;
  DataModel::ProtoImporter importer(ctx);

  QVERIFY(importer.projectFromProtoFile(m_dir.filePath(QStringLiteral("absent.proto"))).isEmpty());
  QVERIFY(importer.projectFromProtoFile(QString()).isEmpty());
  QCOMPARE(importer.messageCount(), 0);
}

QTEST_APPLESS_MAIN(TstProtoImporter)

#include "tst_proto_importer.moc"
