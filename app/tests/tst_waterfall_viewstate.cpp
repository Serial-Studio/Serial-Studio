/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <cmath>
#include <limits>
#include <QRectF>
#include <QTest>

#include "UI/Widgets/Waterfall/WaterfallViewState.h"

// Every test function here is self-contained: each builds its own view state, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Pan/zoom clamping, dB window invariants and window mapping of WaterfallViewState.
 */
class TstWaterfallViewState : public QObject {
  Q_OBJECT

private:
  static constexpr int kMapCount = 8;
  static constexpr int kTurbo    = 4;

  [[nodiscard]] static Widgets::WaterfallViewState makeState();

private slots:
  void defaults();

  void colorMapClamping_data();
  void colorMapClamping();

  void dbWindowChangeDetection();
  void dbWindowInvariants_data();
  void dbWindowInvariants();

  void zoomClamping_data();
  void zoomClamping();
  void zoomRejectsInvalidFactors_data();
  void zoomRejectsInvalidFactors();
  void zoomAnchorPan();

  void panClampedToZoom();
  void panMathKat_data();
  void panMathKat();
  void panRejectsNonFinite();

  void resetRoundTrip();

  void axisWindowKat_data();
  void axisWindowKat();
  void sourceRectFollowsView();

  void visibilityFlags();
};

//--------------------------------------------------------------------------------------------------
// Fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the state the widget builds: eight color maps, Turbo selected.
 */
Widgets::WaterfallViewState TstWaterfallViewState::makeState()
{
  return Widgets::WaterfallViewState(kMapCount, kTurbo);
}

//--------------------------------------------------------------------------------------------------
// Defaults
//--------------------------------------------------------------------------------------------------

/**
 * @brief A freshly built state is the default view: unity zoom, no pan, full -100..0 dB window.
 */
void TstWaterfallViewState::defaults()
{
  const auto state = makeState();

  QCOMPARE(state.colorMap(), kTurbo);
  QCOMPARE(state.xZoom(), 1.0);
  QCOMPARE(state.yZoom(), 1.0);
  QCOMPARE(state.xPan(), 0.0);
  QCOMPARE(state.yPan(), 0.0);
  QCOMPARE(state.minDb(), -100.0);
  QCOMPARE(state.maxDb(), 0.0);
  QVERIFY(state.atDefaultView());
  QVERIFY(state.axisVisible());
  QVERIFY(state.markersVisible());
  QVERIFY(state.colorbarVisible());
  QVERIFY(!state.cursorEnabled());
}

//--------------------------------------------------------------------------------------------------
// Color map
//--------------------------------------------------------------------------------------------------

void TstWaterfallViewState::colorMapClamping_data()
{
  QTest::addColumn<int>("requested");
  QTest::addColumn<int>("expected");
  QTest::addColumn<bool>("changed");

  QTest::newRow("in range") << 2 << 2 << true;
  QTest::newRow("first map") << 0 << 0 << true;
  QTest::newRow("last map") << kMapCount - 1 << kMapCount - 1 << true;
  QTest::newRow("negative clamps to first") << -5 << 0 << true;
  QTest::newRow("past the table clamps to last") << 99 << kMapCount - 1 << true;
  QTest::newRow("already selected") << kTurbo << kTurbo << false;
}

/**
 * @brief The map index is clamped to the table and only a real change is reported, which is what
 *        keeps the facade from emitting colorMapChanged() on a no-op write from QML.
 */
void TstWaterfallViewState::colorMapClamping()
{
  QFETCH(int, requested);
  QFETCH(int, expected);
  QFETCH(bool, changed);

  auto state = makeState();
  QCOMPARE(state.setColorMap(requested), changed);
  QCOMPARE(state.colorMap(), expected);
}

//--------------------------------------------------------------------------------------------------
// dB window
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writing the value already stored is not a change; writing a different one is.
 */
void TstWaterfallViewState::dbWindowChangeDetection()
{
  auto state = makeState();

  QVERIFY(!state.setMinDb(-100.0));
  QVERIFY(!state.setMaxDb(0.0));

  QVERIFY(state.setMinDb(-60.0));
  QCOMPARE(state.minDb(), -60.0);
  QVERIFY(!state.setMinDb(-60.0));

  QVERIFY(state.setMaxDb(-10.0));
  QCOMPARE(state.maxDb(), -10.0);
  QVERIFY(!state.setMaxDb(-10.0));
}

void TstWaterfallViewState::dbWindowInvariants_data()
{
  QTest::addColumn<double>("minDb");
  QTest::addColumn<double>("maxDb");
  QTest::addColumn<float>("expected");

  QTest::newRow("default window") << -100.0 << 0.0 << 0.01f;
  QTest::newRow("narrow window") << -50.0 << -40.0 << 0.1f;
  QTest::newRow("collapsed window") << -20.0 << -20.0 << 1.0e6f;
  QTest::newRow("inverted window") << 0.0 << -50.0 << 1.0e6f;
}

/**
 * @brief The colorization reciprocal is finite and strictly positive for every window the UI can
 *        produce, including the collapsed and inverted ones the two independent sliders allow.
 */
void TstWaterfallViewState::dbWindowInvariants()
{
  QFETCH(double, minDb);
  QFETCH(double, maxDb);
  QFETCH(float, expected);

  auto state = makeState();
  QVERIFY(state.setMinDb(minDb));
  QVERIFY(state.setMaxDb(maxDb));

  const float inv = state.invDbRange();
  QVERIFY(std::isfinite(inv));
  QVERIFY(inv > 0.0f);
  QVERIFY(qFuzzyCompare(inv, expected));
}

//--------------------------------------------------------------------------------------------------
// Zoom
//--------------------------------------------------------------------------------------------------

void TstWaterfallViewState::zoomClamping_data()
{
  QTest::addColumn<double>("factor");
  QTest::addColumn<double>("expected");
  QTest::addColumn<bool>("changed");

  QTest::newRow("double") << 2.0 << 2.0 << true;
  QTest::newRow("far past the ceiling") << 1000.0 << Widgets::WaterfallViewState::kMaxZoom << true;
  QTest::newRow("exactly the ceiling")
    << Widgets::WaterfallViewState::kMaxZoom << Widgets::WaterfallViewState::kMaxZoom << true;
  QTest::newRow("zoom out below unity is a no-op") << 0.5 << 1.0 << false;
  QTest::newRow("unity factor") << 1.0 << 1.0 << false;
}

/**
 * @brief Zoom is bounded to [1, kMaxZoom] on both axes; a factor that cannot move it reports no
 *        change, so the widget never repaints for a wheel tick at the stop.
 */
void TstWaterfallViewState::zoomClamping()
{
  QFETCH(double, factor);
  QFETCH(double, expected);
  QFETCH(bool, changed);

  auto state = makeState();
  QCOMPARE(state.zoomBy(factor, 0.5, 0.5), changed);
  QCOMPARE(state.xZoom(), expected);
  QCOMPARE(state.yZoom(), expected);
}

void TstWaterfallViewState::zoomRejectsInvalidFactors_data()
{
  QTest::addColumn<double>("factor");

  QTest::newRow("zero") << 0.0;
  QTest::newRow("negative") << -2.0;
  QTest::newRow("infinite") << std::numeric_limits<double>::infinity();
  QTest::newRow("not a number") << std::numeric_limits<double>::quiet_NaN();
}

/**
 * @brief A non-finite or non-positive factor leaves the view untouched instead of poisoning the
 *        zoom with a NaN that every later window computation would inherit.
 */
void TstWaterfallViewState::zoomRejectsInvalidFactors()
{
  QFETCH(double, factor);

  auto state = makeState();
  QVERIFY(!state.zoomBy(factor, 0.5, 0.5));
  QVERIFY(state.atDefaultView());
}

/**
 * @brief Zooming toward the right edge pans toward it, and the pan lands on the clamp the new
 *        zoom allows: (1 - 1/2) / 2 = 0.25.
 */
void TstWaterfallViewState::zoomAnchorPan()
{
  auto state = makeState();

  QVERIFY(state.zoomBy(2.0, 1.0, 0.5));
  QCOMPARE(state.xZoom(), 2.0);
  QCOMPARE(state.xPan(), 0.25);
  QCOMPARE(state.yPan(), 0.0);
}

//--------------------------------------------------------------------------------------------------
// Pan
//--------------------------------------------------------------------------------------------------

/**
 * @brief At unity zoom the whole image is on screen, so there is nothing to pan and the drag is
 *        reported as a no-op.
 */
void TstWaterfallViewState::panClampedToZoom()
{
  auto state = makeState();

  QVERIFY(!state.panBy(0.25, 0.25));
  QCOMPARE(state.xPan(), 0.0);
  QCOMPARE(state.yPan(), 0.0);
  QVERIFY(state.atDefaultView());
}

void TstWaterfallViewState::panMathKat_data()
{
  QTest::addColumn<double>("normDx");
  QTest::addColumn<double>("normDy");
  QTest::addColumn<double>("expectedXPan");
  QTest::addColumn<double>("expectedYPan");

  QTest::newRow("drag right") << 0.1 << 0.0 << -0.05 << 0.0;
  QTest::newRow("drag left") << -0.1 << 0.0 << 0.05 << 0.0;
  QTest::newRow("drag down") << 0.0 << 0.2 << 0.0 << -0.1;
  QTest::newRow("diagonal") << 0.1 << 0.1 << -0.05 << -0.05;
  QTest::newRow("past the left clamp") << -1.0 << 0.0 << 0.25 << 0.0;
  QTest::newRow("past the right clamp") << 1.0 << 0.0 << -0.25 << 0.0;
}

/**
 * @brief Drag deltas divide by the zoom before they move the pan, and the result is clamped to
 *        +/- (1 - 1/zoom) / 2 -- 0.25 at 2x.
 */
void TstWaterfallViewState::panMathKat()
{
  QFETCH(double, normDx);
  QFETCH(double, normDy);
  QFETCH(double, expectedXPan);
  QFETCH(double, expectedYPan);

  auto state = makeState();
  QVERIFY(state.zoomBy(2.0, 0.5, 0.5));
  QVERIFY(state.panBy(normDx, normDy));

  QCOMPARE(state.xPan(), expectedXPan);
  QCOMPARE(state.yPan(), expectedYPan);
}

/**
 * @brief A non-finite drag delta is dropped rather than written into the pan.
 */
void TstWaterfallViewState::panRejectsNonFinite()
{
  auto state = makeState();
  QVERIFY(state.zoomBy(4.0, 0.5, 0.5));

  QVERIFY(!state.panBy(std::numeric_limits<double>::quiet_NaN(), 0.0));
  QVERIFY(!state.panBy(0.0, std::numeric_limits<double>::infinity()));
  QCOMPARE(state.xPan(), 0.0);
  QCOMPARE(state.yPan(), 0.0);
}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reset restores the exact default view once, and reports no change when already there.
 */
void TstWaterfallViewState::resetRoundTrip()
{
  auto state = makeState();
  QVERIFY(!state.resetView());

  QVERIFY(state.zoomBy(8.0, 0.0, 1.0));
  QVERIFY(state.panBy(-0.05, 0.05));
  QVERIFY(!state.atDefaultView());

  QVERIFY(state.resetView());
  QCOMPARE(state.xZoom(), 1.0);
  QCOMPARE(state.yZoom(), 1.0);
  QCOMPARE(state.xPan(), 0.0);
  QCOMPARE(state.yPan(), 0.0);
  QVERIFY(state.atDefaultView());
  QVERIFY(!state.resetView());
}

//--------------------------------------------------------------------------------------------------
// Window mapping
//--------------------------------------------------------------------------------------------------

void TstWaterfallViewState::axisWindowKat_data()
{
  QTest::addColumn<double>("zoom");
  QTest::addColumn<double>("pan");
  QTest::addColumn<double>("expectedMin");
  QTest::addColumn<double>("expectedMax");

  QTest::newRow("unity shows everything") << 1.0 << 0.0 << 0.0 << 100.0;
  QTest::newRow("2x centered") << 2.0 << 0.0 << 25.0 << 75.0;
  QTest::newRow("2x panned right") << 2.0 << 0.25 << 50.0 << 100.0;
  QTest::newRow("2x pan past the edge clamps") << 2.0 << 1.0 << 50.0 << 100.0;
  QTest::newRow("2x panned left") << 2.0 << -0.25 << 0.0 << 50.0;
  QTest::newRow("4x centered") << 4.0 << 0.0 << 37.5 << 62.5;
}

/**
 * @brief axisWindow is the single mapping behind the axes, the cursor readout, the marker
 *        placement and the texture source rect; these rows pin it on a 0..100 domain.
 */
void TstWaterfallViewState::axisWindowKat()
{
  QFETCH(double, zoom);
  QFETCH(double, pan);
  QFETCH(double, expectedMin);
  QFETCH(double, expectedMax);

  double lo = 0.0;
  double hi = 0.0;
  Widgets::WaterfallViewState::axisWindow(0.0, 100.0, zoom, pan, lo, hi);

  QCOMPARE(lo, expectedMin);
  QCOMPARE(hi, expectedMax);
}

/**
 * @brief The image source rect is the same window math applied to the image size, so a zoomed
 *        view samples the middle half of the history in both axes.
 */
void TstWaterfallViewState::sourceRectFollowsView()
{
  auto state = makeState();
  QCOMPARE(state.sourceRect(200.0, 100.0), QRectF(0.0, 0.0, 200.0, 100.0));

  QVERIFY(state.zoomBy(2.0, 0.5, 0.5));
  QCOMPARE(state.sourceRect(200.0, 100.0), QRectF(50.0, 25.0, 100.0, 50.0));

  double wMin = 0.0;
  double wMax = 0.0;
  state.freqWindow(0.0, 200.0, wMin, wMax);
  QCOMPARE(wMin, 50.0);
  QCOMPARE(wMax, 150.0);

  double yMin = 0.0;
  double yMax = 0.0;
  state.timeWindow(0.0, 100.0, yMin, yMax);
  QCOMPARE(yMin, 25.0);
  QCOMPARE(yMax, 75.0);
}

//--------------------------------------------------------------------------------------------------
// Visibility flags
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each overlay toggle reports a change exactly once per real transition.
 */
void TstWaterfallViewState::visibilityFlags()
{
  auto state = makeState();

  QVERIFY(state.setAxisVisible(false));
  QVERIFY(!state.setAxisVisible(false));
  QVERIFY(!state.axisVisible());

  QVERIFY(state.setCursorEnabled(true));
  QVERIFY(!state.setCursorEnabled(true));
  QVERIFY(state.cursorEnabled());

  QVERIFY(state.setMarkersVisible(false));
  QVERIFY(!state.setMarkersVisible(false));
  QVERIFY(!state.markersVisible());

  QVERIFY(state.setColorbarVisible(false));
  QVERIFY(!state.setColorbarVisible(false));
  QVERIFY(!state.colorbarVisible());
}

QTEST_APPLESS_MAIN(TstWaterfallViewState)

#include "tst_waterfall_viewstate.moc"
