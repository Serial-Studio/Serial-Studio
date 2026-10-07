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

#include <QTest>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/WidgetResolution.h"

// Alarm-band applicability (spec 0093): a dataset's bands notify and alarm only when a
// band-drawing widget is configured for it and Suppress Alarms is off. Every row uses a built-in
// widget string or none, so the resolver never reaches the extension catalog and the test needs
// no bound services.

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class AlarmBandApplicabilityTest : public QObject {
  Q_OBJECT

private slots:
  void rule_data();
  void rule();
  void raises_data();
  void raises();
};

/**
 * @brief One row per dataset shape: group widget, dataset widget, LED, plot, hidden, expectation.
 */
void AlarmBandApplicabilityTest::rule_data()
{
  QTest::addColumn<QString>("groupWidget");
  QTest::addColumn<QString>("datasetWidget");
  QTest::addColumn<bool>("led");
  QTest::addColumn<bool>("plot");
  QTest::addColumn<bool>("hidden");
  QTest::addColumn<bool>("applicable");

  const QString none;
  const auto bar      = QStringLiteral("bar");
  const auto gauge    = QStringLiteral("gauge");
  const auto meter    = QStringLiteral("meter");
  const auto compass  = QStringLiteral("compass");
  const auto painter  = QStringLiteral("painter");
  const auto barPanel = QStringLiteral("barpanel");
  const auto dataGrid = QStringLiteral("datagrid");

  QTest::newRow("bar") << none << bar << false << false << false << true;
  QTest::newRow("gauge") << none << gauge << false << false << false << true;
  QTest::newRow("meter") << none << meter << false << false << false << true;
  QTest::newRow("led") << none << none << true << false << false << true;
  QTest::newRow("gauge in a datagrid group")
    << dataGrid << gauge << false << false << false << true;

  QTest::newRow("no widget") << none << none << false << false << false << false;
  QTest::newRow("compass") << none << compass << false << false << false << false;
  QTest::newRow("plot only") << none << none << false << true << false << false;
  QTest::newRow("hidden gauge") << none << gauge << false << false << true << false;
  QTest::newRow("hidden led") << none << none << true << false << true << false;
  QTest::newRow("canvas member") << painter << none << false << false << false << false;
  QTest::newRow("datagrid member") << dataGrid << none << false << true << false << false;

  QTest::newRow("bar panel member") << barPanel << none << false << false << false << true;
  QTest::newRow("hidden bar panel member") << barPanel << none << false << false << true << true;
}

/**
 * @brief The rule answers each shape as the dashboard builds it.
 */
void AlarmBandApplicabilityTest::rule()
{
  QFETCH(QString, groupWidget);
  QFETCH(QString, datasetWidget);
  QFETCH(bool, led);
  QFETCH(bool, plot);
  QFETCH(bool, hidden);
  QFETCH(bool, applicable);

  DataModel::Group group;
  group.widget = groupWidget;

  DataModel::Dataset dataset;
  dataset.widget          = datasetWidget;
  dataset.led             = led;
  dataset.plt             = plot;
  dataset.hideOnDashboard = hidden;

  QCOMPARE(SerialStudio::datasetRendersAlarmBands(dataset, group), applicable);
}

/**
 * @brief One row per alarm-side shape: group widget, dataset widget, LED, plot, suppressed, has
 *        bands, whether the bands are drawn, and whether they raise alarms.
 */
void AlarmBandApplicabilityTest::raises_data()
{
  QTest::addColumn<QString>("groupWidget");
  QTest::addColumn<QString>("datasetWidget");
  QTest::addColumn<bool>("led");
  QTest::addColumn<bool>("plot");
  QTest::addColumn<bool>("suppressed");
  QTest::addColumn<bool>("banded");
  QTest::addColumn<bool>("drawn");
  QTest::addColumn<bool>("alarms");

  const QString none;
  const auto gauge    = QStringLiteral("gauge");
  const auto barPanel = QStringLiteral("barpanel");

  QTest::newRow("gauge with bands")
    << none << gauge << false << false << false << true << true << true;
  QTest::newRow("bar panel member with bands")
    << barPanel << none << false << false << false << true << true << true;

  QTest::newRow("suppressed gauge")
    << none << gauge << false << false << true << true << true << false;
  QTest::newRow("suppressed led") << none << none << true << false << true << true << true << false;
  QTest::newRow("suppressed bar panel member")
    << barPanel << none << false << false << true << true << true << false;

  QTest::newRow("gauge without bands")
    << none << gauge << false << false << false << false << true << false;
  QTest::newRow("plot only with bands")
    << none << none << false << true << false << true << false << false;
}

/**
 * @brief Suppression silences the alarm side only: a suppressed dataset's bands stay drawn.
 */
void AlarmBandApplicabilityTest::raises()
{
  QFETCH(QString, groupWidget);
  QFETCH(QString, datasetWidget);
  QFETCH(bool, led);
  QFETCH(bool, plot);
  QFETCH(bool, suppressed);
  QFETCH(bool, banded);
  QFETCH(bool, drawn);
  QFETCH(bool, alarms);

  DataModel::Group group;
  group.widget = groupWidget;

  DataModel::Dataset dataset;
  dataset.widget         = datasetWidget;
  dataset.led            = led;
  dataset.plt            = plot;
  dataset.suppressAlarms = suppressed;
  if (banded) {
    DataModel::AlarmBand band;
    band.min      = 800;
    band.max      = 1000;
    band.severity = DataModel::AlarmSeverity::Critical;
    dataset.alarmBands.push_back(band);
  }

  QCOMPARE(SerialStudio::datasetRendersAlarmBands(dataset, group), drawn);
  QCOMPARE(SerialStudio::datasetRaisesBandAlarms(dataset, group), alarms);
}

QTEST_APPLESS_MAIN(AlarmBandApplicabilityTest)

#include "tst_alarm_band_applicability.moc"
