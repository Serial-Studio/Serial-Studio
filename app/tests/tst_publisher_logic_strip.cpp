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
#include "DataModel/Project/ProjectSealing.h"

static const QString kLogic  = QStringLiteral("PUBLISHER_LOGIC_7e21");
static const QString kCanvas = QStringLiteral("CANVAS_DRAW_44b9");

/**
 * @brief A project document from a publisher, every logic field carrying kLogic and the Canvas
 *        code carrying kCanvas, including the legacy root parser key the loader falls back to.
 */
static QJsonObject publisherProject()
{
  QJsonObject dataset;
  dataset.insert(Keys::Title, QStringLiteral("Pressure"));
  dataset.insert(Keys::Units, QStringLiteral("kPa"));
  dataset.insert(QStringLiteral("virtual"), true);
  dataset.insert(Keys::TransformCode, kLogic);
  dataset.insert(Keys::TransformParams,
                 QJsonObject{
                   {QStringLiteral("gain"), kLogic}
  });

  QJsonObject output;
  output.insert(Keys::Title, QStringLiteral("Valve"));
  output.insert(Keys::TransmitFunction, kLogic);

  QJsonObject group;
  group.insert(Keys::Title, QStringLiteral("Canvas"));
  group.insert(Keys::Widget, QStringLiteral("painter"));
  group.insert(Keys::PainterCode, kCanvas);
  group.insert(Keys::Datasets, QJsonArray{dataset});
  group.insert(Keys::OutputWidgets, QJsonArray{output});

  QJsonObject source;
  source.insert(Keys::SourceId, 0);
  source.insert(Keys::FrameParserCode, kLogic);
  source.insert(Keys::FrameParserParams,
                QJsonObject{
                  {QStringLiteral("delimiter"), kLogic}
  });
  source.insert(Keys::SourceConn,
                QJsonObject{
                  {QStringLiteral("executable"), kLogic}
  });

  QJsonObject project;
  project.insert(Keys::Title, QStringLiteral("Publisher"));
  project.insert(Keys::Groups, QJsonArray{group});
  project.insert(Keys::Sources, QJsonArray{source});
  project.insert(Keys::ControlScriptCode, kLogic);
  project.insert(Keys::TransformLibrary, kLogic);
  project.insert(Keys::TransformLibraryJs, kLogic);
  project.insert(QStringLiteral("frameParser"), kLogic);
  project.insert(Keys::MqttPublisher,
                 QJsonObject{
                   {QStringLiteral("scriptCode"), kLogic}
  });
  project.insert(Keys::InfluxSink,
                 QJsonObject{
                   {QStringLiteral("url"), kLogic}
  });
  return project;
}

/**
 * @brief The viewer-side strip (spec 0098 R10, AC6): no publisher logic survives, including the
 *        legacy root parser key; Canvas code and every display field do.
 */
class TstPublisherLogicStrip : public QObject {
  Q_OBJECT

private slots:
  void removesEveryLogicField();
  void keepsCanvasCodeAndStructure();
  void leavesAbsentKeysAbsent();
};

/**
 * @brief The serialized stripped document no longer contains the logic marker anywhere.
 */
void TstPublisherLogicStrip::removesEveryLogicField()
{
  auto project = publisherProject();
  QVERIFY(QJsonDocument(project).toJson().contains(kLogic.toUtf8()));

  DataModel::stripPublisherLogic(project);
  QVERIFY(!QJsonDocument(project).toJson().contains(kLogic.toUtf8()));
  QVERIFY(!project.contains(QStringLiteral("frameParser")));
}

/**
 * @brief Canvas code, titles, units, widget kinds and the virtual flag are what they were.
 */
void TstPublisherLogicStrip::keepsCanvasCodeAndStructure()
{
  auto project = publisherProject();
  DataModel::stripPublisherLogic(project);

  QCOMPARE(project.value(Keys::Title).toString(), QStringLiteral("Publisher"));

  const auto group = project.value(Keys::Groups).toArray().first().toObject();
  QCOMPARE(group.value(Keys::PainterCode).toString(), kCanvas);
  QCOMPARE(group.value(Keys::Widget).toString(), QStringLiteral("painter"));

  const auto dataset = group.value(Keys::Datasets).toArray().first().toObject();
  QCOMPARE(dataset.value(Keys::Title).toString(), QStringLiteral("Pressure"));
  QCOMPARE(dataset.value(Keys::Units).toString(), QStringLiteral("kPa"));
  QCOMPARE(dataset.value(QStringLiteral("virtual")).toBool(), true);

  const auto output = group.value(Keys::OutputWidgets).toArray().first().toObject();
  QCOMPARE(output.value(Keys::Title).toString(), QStringLiteral("Valve"));

  const auto source = project.value(Keys::Sources).toArray().first().toObject();
  QCOMPARE(source.value(Keys::SourceId).toInt(), 0);
}

/**
 * @brief A document with none of the logic keys comes out unchanged.
 */
void TstPublisherLogicStrip::leavesAbsentKeysAbsent()
{
  QJsonObject project;
  project.insert(Keys::Title, QStringLiteral("Plain"));
  const auto before = project;

  DataModel::stripPublisherLogic(project);
  QCOMPARE(project, before);
}

QTEST_GUILESS_MAIN(TstPublisherLogicStrip)

#include "tst_publisher_logic_strip.moc"
