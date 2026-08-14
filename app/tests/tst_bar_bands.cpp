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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include <QtTest>
#include <vector>

#include "UI/WidgetBands.h"

/**
 * @brief Minimal band shape matching what Widgets::Bar / Widgets::BarPanel store.
 */
struct TestBand {
  double min   = 0;
  double max   = 0;
  int severity = 2;
};

/**
 * @brief Exercises the shared alarm-band lookup (spec 0052): containment, hint fast path,
 *        gap-to-nearest clamping, overrange-to-outermost clamping, and the empty-list case.
 */
class TstBarBands : public QObject {
  Q_OBJECT

private slots:
  void containmentFindsTheEnclosingBand();
  void hintShortCircuitsWithoutChangingTheAnswer();
  void boundariesAreInclusive();
  void gapValuesClampToTheNearestBand();
  void overrangeValuesClampToTheOutermostBand();
  void emptyListsStayUnclassified();
  void activeIndexPrefersContainmentOverDistance();

private:
  [[nodiscard]] static std::vector<TestBand> apuOilBands();
};

/**
 * @brief The APS500 oil-pressure ladder: critical / warning / ok / warning / critical.
 */
std::vector<TestBand> TstBarBands::apuOilBands()
{
  return {
    { 0,  25, 3},
    {25,  55, 2},
    {55,  75, 1},
    {75,  80, 2},
    {80, 150, 3},
  };
}

void TstBarBands::containmentFindsTheEnclosingBand()
{
  const auto bands = apuOilBands();
  QCOMPARE(Widgets::Bands::indexFor(bands, 10.0), 0);
  QCOMPARE(Widgets::Bands::indexFor(bands, 40.0), 1);
  QCOMPARE(Widgets::Bands::indexFor(bands, 60.0), 2);
  QCOMPARE(Widgets::Bands::indexFor(bands, 78.0), 3);
  QCOMPARE(Widgets::Bands::indexFor(bands, 120.0), 4);
}

void TstBarBands::hintShortCircuitsWithoutChangingTheAnswer()
{
  const auto bands = apuOilBands();
  QCOMPARE(Widgets::Bands::indexFor(bands, 60.0, 2), 2);
  QCOMPARE(Widgets::Bands::indexFor(bands, 60.0, 0), 2);
  QCOMPARE(Widgets::Bands::indexFor(bands, 60.0, 99), 2);
  QCOMPARE(Widgets::Bands::indexFor(bands, 60.0, -5), 2);
}

void TstBarBands::boundariesAreInclusive()
{
  const auto bands = apuOilBands();
  QCOMPARE(Widgets::Bands::indexFor(bands, 0.0), 0);
  QCOMPARE(Widgets::Bands::indexFor(bands, 25.0), 0);
  QCOMPARE(Widgets::Bands::indexFor(bands, 150.0), 4);
}

void TstBarBands::gapValuesClampToTheNearestBand()
{
  const std::vector<TestBand> gapped = {
    { 0, 10, 1},
    {20, 30, 3},
  };

  QCOMPARE(Widgets::Bands::indexFor(gapped, 12.0), -1);
  QCOMPARE(Widgets::Bands::nearestIndex(gapped, 12.0), 0);
  QCOMPARE(Widgets::Bands::nearestIndex(gapped, 18.0), 1);
  QCOMPARE(Widgets::Bands::activeIndex(gapped, 12.0), 0);
  QCOMPARE(Widgets::Bands::activeIndex(gapped, 18.0), 1);
}

void TstBarBands::overrangeValuesClampToTheOutermostBand()
{
  const auto bands = apuOilBands();
  QCOMPARE(Widgets::Bands::activeIndex(bands, -40.0), 0);
  QCOMPARE(Widgets::Bands::activeIndex(bands, 1472.0), 4);
}

void TstBarBands::emptyListsStayUnclassified()
{
  const std::vector<TestBand> none;
  QCOMPARE(Widgets::Bands::indexFor(none, 1.0), -1);
  QCOMPARE(Widgets::Bands::nearestIndex(none, 1.0), -1);
  QCOMPARE(Widgets::Bands::activeIndex(none, 1.0), -1);
}

void TstBarBands::activeIndexPrefersContainmentOverDistance()
{
  const std::vector<TestBand> overlapping = {
    { 0, 100, 1},
    {49,  51, 3},
  };

  QCOMPARE(Widgets::Bands::activeIndex(overlapping, 50.0), 0);
  QCOMPARE(Widgets::Bands::activeIndex(overlapping, 50.0, 1), 1);
}

QTEST_GUILESS_MAIN(TstBarBands)
#include "tst_bar_bands.moc"
