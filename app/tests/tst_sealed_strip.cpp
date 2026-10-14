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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "Core/DataModel/FrameKeys.h"
#include "Core/Runtime.h"
#include "DataModel/Project/ProjectSealing.h"

static const QString kMarker = QStringLiteral("SEALED_MARKER_51c0");

/**
 * @brief A project with the marker in every field a sealed copy must omit.
 */
static QJsonObject markedProject()
{
  QJsonObject dataset;
  dataset.insert(Keys::Title, QStringLiteral("Temperature"));
  dataset.insert(Keys::Units, QStringLiteral("C"));
  dataset.insert(Keys::TransformCode, kMarker);
  dataset.insert(Keys::TransformParams,
                 QJsonObject{
                   {QStringLiteral("gain"), kMarker}
  });

  QJsonObject output;
  output.insert(Keys::Title, QStringLiteral("Setpoint"));
  output.insert(Keys::TransmitFunction, kMarker);

  QJsonObject group;
  group.insert(Keys::Title, QStringLiteral("Sensors"));
  group.insert(Keys::Widget, QStringLiteral("painter"));
  group.insert(Keys::PainterCode, kMarker);
  group.insert(Keys::Datasets, QJsonArray{dataset});
  group.insert(Keys::OutputWidgets, QJsonArray{output});

  QJsonObject source;
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::BusType, 3);
  source.insert(Keys::FrameParserCode, kMarker);
  source.insert(Keys::FrameParserParams,
                QJsonObject{
                  {QStringLiteral("delimiter"), kMarker}
  });
  source.insert(
    Keys::SourceConn,
    QJsonObject{
      {             QStringLiteral("password"), kMarker},
      {QStringLiteral("clientCertificatePath"), kMarker}
  });

  QJsonObject project;
  project.insert(Keys::Title, QStringLiteral("Demo"));
  project.insert(Keys::Groups, QJsonArray{group});
  project.insert(Keys::Sources, QJsonArray{source});
  project.insert(Keys::ControlScriptCode, kMarker);
  project.insert(Keys::TransformLibrary, kMarker);
  project.insert(Keys::TransformLibraryJs, kMarker);
  project.insert(Keys::PasswordHash, kMarker);
  project.insert(Keys::MqttPublisher,
                 QJsonObject{
                   {QStringLiteral("script"), kMarker}
  });
  project.insert(Keys::InfluxSink,
                 QJsonObject{
                   {QStringLiteral("url"), kMarker}
  });
  return project;
}

/**
 * @brief The sealed strip (spec 0096 T6): no marked field survives, and nothing else changes.
 */
class TstSealedStrip : public QObject {
  Q_OBJECT

private slots:
  void removesEveryScriptAndCredential();
  void keepsStructureUnchanged();
  void leavesAbsentKeysAbsent();
  void frameOverloadClearsTemplates();
  void sealedCopyMarksOnlyWhenSealed();
};

/**
 * @brief The serialized stripped document no longer contains the marker anywhere.
 */
void TstSealedStrip::removesEveryScriptAndCredential()
{
  auto project = markedProject();
  QVERIFY(QJsonDocument(project).toJson().contains(kMarker.toUtf8()));

  DataModel::stripSealedContent(project);
  QVERIFY(!QJsonDocument(project).toJson().contains(kMarker.toUtf8()));
}

/**
 * @brief Titles, units, widgets and ids are exactly what they were before the strip.
 */
void TstSealedStrip::keepsStructureUnchanged()
{
  auto project = markedProject();
  DataModel::stripSealedContent(project);

  QCOMPARE(project.value(Keys::Title).toString(), QStringLiteral("Demo"));

  const auto group = project.value(Keys::Groups).toArray().first().toObject();
  QCOMPARE(group.value(Keys::Title).toString(), QStringLiteral("Sensors"));
  QCOMPARE(group.value(Keys::Widget).toString(), QStringLiteral("painter"));

  const auto dataset = group.value(Keys::Datasets).toArray().first().toObject();
  QCOMPARE(dataset.value(Keys::Title).toString(), QStringLiteral("Temperature"));
  QCOMPARE(dataset.value(Keys::Units).toString(), QStringLiteral("C"));
  QCOMPARE(dataset.keys().size(), 2);

  const auto output = group.value(Keys::OutputWidgets).toArray().first().toObject();
  QCOMPARE(output.value(Keys::Title).toString(), QStringLiteral("Setpoint"));

  const auto source = project.value(Keys::Sources).toArray().first().toObject();
  QCOMPARE(source.value(Keys::SourceId).toInt(), 0);
  QCOMPARE(source.value(Keys::BusType).toInt(), 3);
  QCOMPARE(source.keys().size(), 2);
}

/**
 * @brief A document without the stripped sections gains no empty placeholders.
 */
void TstSealedStrip::leavesAbsentKeysAbsent()
{
  QJsonObject project{
    {Keys::Title, QStringLiteral("Bare")}
  };
  const auto before = project;
  DataModel::stripSealedContent(project);
  QCOMPARE(project, before);
}

/**
 * @brief The struct overload clears the same fields on a frame template, so the serialized frame
 *        an API client receives carries no marker.
 */
void TstSealedStrip::frameOverloadClearsTemplates()
{
  DataModel::Dataset dataset;
  dataset.title         = QStringLiteral("Temperature");
  dataset.transformCode = kMarker;
  dataset.transformParams.insert(QStringLiteral("gain"), kMarker);

  DataModel::OutputWidget output;
  output.transmitFunction = kMarker;

  DataModel::Group group;
  group.title       = QStringLiteral("Sensors");
  group.widget      = QStringLiteral("painter");
  group.painterCode = kMarker;
  group.datasets.push_back(dataset);
  group.outputWidgets.push_back(output);

  DataModel::Source source;
  source.frameParserCode    = kMarker;
  source.connectionSettings = QJsonObject{
    {QStringLiteral("password"), kMarker}
  };

  DataModel::Frame frame;
  frame.title              = QStringLiteral("Demo");
  frame.controlScriptCode  = kMarker;
  frame.transformLibrary   = kMarker;
  frame.transformLibraryJs = kMarker;
  frame.groups.push_back(group);
  frame.sources.push_back(source);
  QVERIFY(QJsonDocument(DataModel::serialize(frame)).toJson().contains(kMarker.toUtf8()));

  DataModel::stripSealedContent(frame);
  QVERIFY(!QJsonDocument(DataModel::serialize(frame)).toJson().contains(kMarker.toUtf8()));
  QCOMPARE(frame.groups.front().title, QStringLiteral("Sensors"));
  QCOMPARE(frame.groups.front().datasets.front().title, QStringLiteral("Temperature"));
  QVERIFY(frame.sources.front().frameParserCode.isEmpty());
}

/**
 * @brief Unsealed, a recording keeps the project as-is; sealed (one-way, so this slot runs last),
 *        it stores the stripped copy marked as a sealed recording.
 */
void TstSealedStrip::sealedCopyMarksOnlyWhenSealed()
{
  const auto project = markedProject();
  QCOMPARE(DataModel::sealedCopy(project), project);
  QVERIFY(!DataModel::isSealedRecording(QString::fromUtf8(QJsonDocument(project).toJson())));

  Core::Runtime::seal(QStringLiteral(":/ssdeploy-test"), QString());
  const auto sealed = DataModel::sealedCopy(project);
  const auto json   = QJsonDocument(sealed).toJson();
  QVERIFY(!json.contains(kMarker.toUtf8()));
  QVERIFY(DataModel::isSealedRecording(QString::fromUtf8(json)));
  QVERIFY(!DataModel::isSealedRecording(QStringLiteral("not json")));
}

QTEST_GUILESS_MAIN(TstSealedStrip)

#include "tst_sealed_strip.moc"
