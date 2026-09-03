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
#include <vector>

#include "UI/WidgetBands.h"

// The no-data severity gate on the instrument widgets (spec 0075, N3). Bar, Gauge and Meter show
// 0.0 before their first sample, and the band lookup clamps an out-of-band value to the NEAREST
// band by design -- which is right for overrange data and wrong for a placeholder. On a project
// whose bands sit above zero, 0.0 therefore resolved to the lowest band and, on most real
// projects, to a critical one: twenty widgets blinking forever on an occluded window for datasets
// that had never received a byte. Bands::reportedSeverity is the gate that fixes it; what is
// pinned here is that the clamp still happens (it is load-bearing for real data) and that the gate
// overrides it while the widget has nothing to show.

namespace {
/**
 * @brief The two fields the band lookup reads, plus the severity it reports.
 */
struct Band {
  double min;
  double max;
  int severity;
};
}  // namespace

class ValueWidgetHasDataTest : public QObject {
  Q_OBJECT

private slots:
  void placeholderZeroStillClampsToTheNearestBand();
  void noDataOverridesTheClampedSeverity();
  void firstSampleReportsTheResolvedSeverity();
  void anUnresolvedBandIsUnclassifiedEvenWithData();
  void anEmptyBandListIsAlwaysUnclassified();
};

//--------------------------------------------------------------------------------------------------
// Cases
//--------------------------------------------------------------------------------------------------

/**
 * @brief The nearest-band clamp is unchanged: a value below every band still resolves to the
 *        lowest one, which is what keeps overrange data classified.
 */
void ValueWidgetHasDataTest::placeholderZeroStillClampsToTheNearestBand()
{
  const std::vector<Band> bands = {
    { 800.0, 1200.0, 3},
    {1200.0, 1500.0, 2}
  };

  QCOMPARE(Widgets::Bands::activeIndex(bands, 0.0), 0);
  QCOMPARE(Widgets::Bands::activeIndex(bands, 1300.0), 1);
}

/**
 * @brief With no sample yet the reported severity is unclassified, even though the lookup
 *        resolved a critical band for the placeholder value. This is the defect.
 */
void ValueWidgetHasDataTest::noDataOverridesTheClampedSeverity()
{
  const std::vector<Band> bands = {
    { 800.0, 1200.0, 3},
    {1200.0, 1500.0, 2}
  };
  const int resolved = Widgets::Bands::activeIndex(bands, 0.0);

  QCOMPARE(resolved, 0);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, resolved, false), -1);
}

/**
 * @brief Once a sample arrives the widget reports exactly what the lookup resolved, so the gate
 *        cannot suppress a real alarm.
 */
void ValueWidgetHasDataTest::firstSampleReportsTheResolvedSeverity()
{
  const std::vector<Band> bands = {
    { 800.0, 1200.0, 3},
    {1200.0, 1500.0, 2}
  };

  QCOMPARE(Widgets::Bands::reportedSeverity(bands, Widgets::Bands::activeIndex(bands, 900.0), true),
           3);
  QCOMPARE(
    Widgets::Bands::reportedSeverity(bands, Widgets::Bands::activeIndex(bands, 1300.0), true), 2);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, Widgets::Bands::activeIndex(bands, 0.0), true),
           3);
}

/**
 * @brief An index outside the band list stays unclassified rather than reading past the end --
 *        the state a widget sits in between a reset and the next sample.
 */
void ValueWidgetHasDataTest::anUnresolvedBandIsUnclassifiedEvenWithData()
{
  const std::vector<Band> bands = {
    {800.0, 1200.0, 3}
  };

  QCOMPARE(Widgets::Bands::reportedSeverity(bands, -1, true), -1);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, 1, true), -1);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, 4096, true), -1);
}

/**
 * @brief A dataset with no bands never reports a severity, with or without data.
 */
void ValueWidgetHasDataTest::anEmptyBandListIsAlwaysUnclassified()
{
  const std::vector<Band> bands;

  QCOMPARE(Widgets::Bands::activeIndex(bands, 42.0), -1);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, 0, true), -1);
  QCOMPARE(Widgets::Bands::reportedSeverity(bands, 0, false), -1);
}

QTEST_APPLESS_MAIN(ValueWidgetHasDataTest)

#include "tst_value_widget_hasdata.moc"
