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
#include <QJsonObject>
#include <QTest>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/DataModel/FrameSupport.h"
#include "Core/SerialStudio.h"
#include "DataModel/Project/QuickPlotProject.h"

// Save as Project (spec 0100 R8/R9): a live Quick Plot structure becomes a project that reads the
// same stream the same way. The device must keep Quick Plot's framing and use the free Built-In
// comma parser; every channel keeps its title, frame index and plot; nothing in it needs Pro.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief A Quick Plot-shaped structure: a data grid and a multi-plot over the same @p titles,
 *        carrying runtime ids and live values the project must not inherit.
 */
static std::vector<DataModel::Group> quickPlotGroups(const QStringList& titles)
{
  std::vector<DataModel::Group> groups;
  const QStringList widgets{QStringLiteral("datagrid"), QStringLiteral("multiplot")};
  for (int g = 0; g < widgets.size(); ++g) {
    DataModel::Group group;
    group.groupId  = g;
    group.uniqueId = 900000 + g;
    group.widget   = widgets[g];
    group.title    = g == 0 ? QStringLiteral("Quick Plot Data") : QStringLiteral("Multi-Plot");
    for (int i = 0; i < titles.size(); ++i) {
      DataModel::Dataset dataset;
      dataset.groupId  = g;
      dataset.uniqueId = 800000 + i;
      dataset.index    = i + 1;
      dataset.plt      = true;
      dataset.title    = titles[i];
      dataset.value    = QStringLiteral("42.0");
      group.datasets.push_back(dataset);
    }

    groups.push_back(group);
  }

  return groups;
}

/**
 * @brief Reads the generated groups back through the project readers.
 */
static std::vector<DataModel::Group> readGroups(const QJsonObject& project)
{
  std::vector<DataModel::Group> groups;
  const auto array = project.value(Keys::Groups).toArray();
  for (const auto& value : array) {
    DataModel::Group group;
    if (DataModel::read(group, value.toObject()))
      groups.push_back(group);
  }

  return groups;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pins the Quick Plot to project conversion.
 */
class QuickPlotProjectTest : public QObject {
  Q_OBJECT

private slots:
  void sourceKeepsQuickPlotFraming();
  void sourceUsesTheFreeCommaParser();
  void sourceCarriesTheConnectionSettings();
  void groupsKeepTitlesIndicesAndPlots();
  void liveValuesAndRuntimeIdsAreDropped();
  void projectNeedsNoPro();
  void audioFftSettingsCarryOver();
  void emptyStructureYieldsNothing();
};

/**
 * @brief One device, newline-terminated plain text, end delimiter only.
 */
void QuickPlotProjectTest::sourceKeepsQuickPlotFraming()
{
  const auto project = DataModel::QuickPlotProject::build(
    quickPlotGroups({"a", "b"}), static_cast<int>(SerialStudio::BusType::UART), QJsonObject());

  const auto sources = project.value(Keys::Sources).toArray();
  QCOMPARE(sources.size(), 1);

  DataModel::Source source;
  QVERIFY(DataModel::read(source, sources.first().toObject()));
  QCOMPARE(source.sourceId, 0);
  QCOMPARE(source.frameEnd, QStringLiteral("\\n"));
  QVERIFY(source.frameStart.isEmpty());
  QCOMPARE(source.frameDetection, static_cast<int>(SerialStudio::EndDelimiterOnly));
  QCOMPARE(source.decoderMethod, static_cast<int>(SerialStudio::PlainText));
  QCOMPARE(source.busType, static_cast<int>(SerialStudio::BusType::UART));
}

/**
 * @brief The Built-In delimited template splits on commas and trims the CR of CRLF lines.
 */
void QuickPlotProjectTest::sourceUsesTheFreeCommaParser()
{
  const auto project = DataModel::QuickPlotProject::build(
    quickPlotGroups({"a"}), static_cast<int>(SerialStudio::BusType::UART), QJsonObject());

  DataModel::Source source;
  QVERIFY(DataModel::read(source, project.value(Keys::Sources).toArray().first().toObject()));
  QCOMPARE(source.frameParserLanguage, static_cast<int>(SerialStudio::Native));
  QCOMPARE(source.frameParserTemplate, QStringLiteral("delimited"));
  QCOMPARE(source.frameParserParams.value(QStringLiteral("separator")).toString(),
           QStringLiteral(","));
  QCOMPARE(source.frameParserParams.value(QStringLiteral("trimFields")).toBool(), true);
}

/**
 * @brief The connection settings given are stored so the project reconnects to the same device.
 */
void QuickPlotProjectTest::sourceCarriesTheConnectionSettings()
{
  const QJsonObject settings{
    {QStringLiteral("baudRate"), 115200}
  };
  const auto project = DataModel::QuickPlotProject::build(
    quickPlotGroups({"a"}), static_cast<int>(SerialStudio::BusType::UART), settings);

  DataModel::Source source;
  QVERIFY(DataModel::read(source, project.value(Keys::Sources).toArray().first().toObject()));
  QCOMPARE(source.connectionSettings.value(QStringLiteral("baudRate")).toInt(), 115200);
}

/**
 * @brief Both groups survive with their widget, and every channel keeps title, index and plot.
 */
void QuickPlotProjectTest::groupsKeepTitlesIndicesAndPlots()
{
  const QStringList titles{"Temperature", "Humidity", "Pressure"};
  const auto groups = readGroups(DataModel::QuickPlotProject::build(
    quickPlotGroups(titles), static_cast<int>(SerialStudio::BusType::UART), QJsonObject()));

  QCOMPARE(groups.size(), size_t(2));
  QCOMPARE(groups[0].widget, QStringLiteral("datagrid"));
  QCOMPARE(groups[1].widget, QStringLiteral("multiplot"));
  for (const auto& group : groups) {
    QCOMPARE(group.datasets.size(), size_t(titles.size()));
    for (int i = 0; i < titles.size(); ++i) {
      QCOMPARE(group.datasets[i].title, titles[i]);
      QCOMPARE(group.datasets[i].index, i + 1);
      QVERIFY(group.datasets[i].plt);
    }
  }
}

/**
 * @brief Live values and Quick Plot's runtime unique ids do not leak into the document.
 */
void QuickPlotProjectTest::liveValuesAndRuntimeIdsAreDropped()
{
  const auto groups = readGroups(DataModel::QuickPlotProject::build(
    quickPlotGroups({"a"}), static_cast<int>(SerialStudio::BusType::UART), QJsonObject()));

  QVERIFY(!groups.empty());
  QVERIFY(groups[0].uniqueId != 900000);
  QVERIFY(groups[0].datasets[0].uniqueId != 800000);
  QVERIFY(groups[0].datasets[0].value != QStringLiteral("42.0"));
}

/**
 * @brief No transforms, no tables, no commercial widget: the project connects in every edition.
 */
void QuickPlotProjectTest::projectNeedsNoPro()
{
  const auto groups = readGroups(DataModel::QuickPlotProject::build(
    quickPlotGroups({"a", "b"}), static_cast<int>(SerialStudio::BusType::UART), QJsonObject()));

  const auto summary = SerialStudio::proContentSummary(groups, 0);
  QCOMPARE(summary.transforms, 0);
  QCOMPARE(summary.tables, 0);
  QVERIFY(!SerialStudio::commercialCfg(groups));
}

/**
 * @brief An audio Quick Plot structure keeps the FFT rate, size and ranges Quick Plot derived from
 *        the capture format, so the saved project plots the same spectrum.
 */
void QuickPlotProjectTest::audioFftSettingsCarryOver()
{
  auto live                           = quickPlotGroups({"Left"});
  live[0].datasets[0].fft             = true;
  live[0].datasets[0].fftSamplingRate = 48000;
  live[0].datasets[0].fftSamples      = 4096;
  live[0].datasets[0].fftMin          = -1.0;
  live[0].datasets[0].fftMax          = 1.0;
  live[0].datasets[0].pltMin          = -1.0;
  live[0].datasets[0].pltMax          = 1.0;

  const auto groups = readGroups(DataModel::QuickPlotProject::build(
    live, static_cast<int>(SerialStudio::BusType::UART), QJsonObject()));

  QVERIFY(!groups.empty());
  const auto& dataset = groups[0].datasets[0];
  QVERIFY(dataset.fft);
  QCOMPARE(dataset.fftSamplingRate, 48000);
  QCOMPARE(dataset.fftSamples, 4096);
  QCOMPARE(dataset.fftMin, -1.0);
  QCOMPARE(dataset.pltMax, 1.0);
}

/**
 * @brief Without a Quick Plot structure there is nothing to save.
 */
void QuickPlotProjectTest::emptyStructureYieldsNothing()
{
  const auto project = DataModel::QuickPlotProject::build(
    {}, static_cast<int>(SerialStudio::BusType::UART), QJsonObject());
  QVERIFY(project.isEmpty());
}

QTEST_APPLESS_MAIN(QuickPlotProjectTest)

#include "tst_quickplot_project.moc"
