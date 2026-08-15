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

#include <QtMath>
#include <QtTest>

#include "UI/LayoutPatterns.h"

using UI::Layouts::LayoutEnv;
using UI::Layouts::Pattern;

//--------------------------------------------------------------------------------------------------
// Frozen reference implementation
//--------------------------------------------------------------------------------------------------
//
// A verbatim copy of the auto-layout that shipped BEFORE spec 0053 extracted the tiler. It
// exists so the Grid pattern can be proven byte-identical to the historical behaviour
// (AC4) instead of that being asserted by inspection. Do not "fix" anything here: if this
// drifts from the original, the guard silently stops guarding.
//
//--------------------------------------------------------------------------------------------------

namespace reference {

[[nodiscard]] static int columnCount(const int n, const LayoutEnv& env)
{
  static constexpr int kLandscapeCols[] = {1, 1, 2, 2, 2, 3, 3};
  static constexpr int kPortraitCols[]  = {1, 1, 1, 1, 2, 2, 2};
  if (n <= 6)
    return env.isLandscape ? kLandscapeCols[n] : kPortraitCols[n];

  const double safeW = qMax(1, env.availW);
  const double safeH = qMax(1, env.availH);

  int cols;
  if (env.isLandscape) {
    cols = qCeil(qSqrt(static_cast<double>(n) * safeW / safeH));
  } else {
    const int rows = qBound(1, qCeil(qSqrt(static_cast<double>(n) * safeH / safeW)), n);
    cols           = qCeil(static_cast<double>(n) / rows);
  }

  return qBound(1, cols, n);
}

[[nodiscard]] static QVector<QRect> exactGrid(const int n, const LayoutEnv& env, const int cols)
{
  const int rows             = n / cols;
  const int spacingForSizing = qMax(env.spacing, 0);
  const int totalCellsW      = qMax(1, env.availW - (cols - 1) * spacingForSizing);
  const int totalCellsH      = qMax(1, env.availH - (rows - 1) * spacingForSizing);
  const int baseCellW        = totalCellsW / cols;
  const int baseCellH        = totalCellsH / rows;
  const int extraW           = totalCellsW % cols;
  const int extraH           = totalCellsH % rows;

  QVector<int> colWidths(cols), colXs(cols);
  QVector<int> rowHeights(rows), rowYs(rows);

  int runningX = env.margin;
  for (int c = 0; c < cols; ++c) {
    colWidths[c]  = baseCellW + (c < extraW ? 1 : 0);
    colXs[c]      = runningX;
    runningX     += colWidths[c] + env.spacing;
  }

  int runningY = env.margin;
  for (int r = 0; r < rows; ++r) {
    rowHeights[r]  = baseCellH + (r < extraH ? 1 : 0);
    rowYs[r]       = runningY;
    runningY      += rowHeights[r] + env.spacing;
  }

  QVector<QRect> out;
  out.reserve(n);
  for (int i = 0; i < n; ++i)
    out.append(QRect(colXs[i % cols], rowYs[i / cols], colWidths[i % cols], rowHeights[i / cols]));

  return out;
}

[[nodiscard]] static QVector<QRect> unevenColumns(const int n, const LayoutEnv& env, const int cols)
{
  const int spacingForSizing = qMax(env.spacing, 0);
  const int totalCellsW      = qMax(1, env.availW - (cols - 1) * spacingForSizing);
  const int baseCellW        = totalCellsW / cols;
  const int extraW           = totalCellsW % cols;
  const int baseCount        = n / cols;
  const int extraCount       = n % cols;

  QVector<QRect> out;
  out.reserve(n);

  int runningX = env.margin;
  for (int c = 0; c < cols; ++c) {
    const int colW        = baseCellW + (c < extraW ? 1 : 0);
    const int count       = baseCount + (c >= cols - extraCount ? 1 : 0);
    const int totalCellsH = qMax(1, env.availH - (count - 1) * spacingForSizing);
    const int baseCellH   = totalCellsH / count;
    const int extraH      = totalCellsH % count;

    int runningY = env.margin;
    for (int r = 0; r < count; ++r) {
      const int cellH = baseCellH + (r < extraH ? 1 : 0);
      out.append(QRect(runningX, runningY, colW, cellH));
      runningY += cellH + env.spacing;
    }

    runningX += colW + env.spacing;
  }

  return out;
}

[[nodiscard]] static QVector<QRect> grid(const int n, const LayoutEnv& env)
{
  const int cols = columnCount(n, env);
  if (n % cols == 0)
    return exactGrid(n, env, cols);

  return unevenColumns(n, env, cols);
}

}  // namespace reference

//--------------------------------------------------------------------------------------------------
// Manual-layout fixtures
//--------------------------------------------------------------------------------------------------
//
// Hand-built manual layouts, expressed the way a user would leave them: joins exactly one
// spacing apart and outer edges flush with the canvas. The rescale cases below assert those two
// properties survive any canvas change, which is what R11 promises.
//
//--------------------------------------------------------------------------------------------------

namespace fixtures {

[[nodiscard]] static QVector<QRect> grid2x2(const QSize& canvas, const int spacing)
{
  const int cw = (canvas.width() - spacing) / 2;
  const int ch = (canvas.height() - spacing) / 2;
  const int rw = canvas.width() - cw - spacing;
  const int rh = canvas.height() - ch - spacing;

  return {QRect(0, 0, cw, ch),
          QRect(cw + spacing, 0, rw, ch),
          QRect(0, ch + spacing, cw, rh),
          QRect(cw + spacing, ch + spacing, rw, rh)};
}

[[nodiscard]] static QVector<QRect> masterStack(const QSize& canvas, const int spacing)
{
  const int mw = (canvas.width() - spacing) / 2;
  const int sw = canvas.width() - mw - spacing;
  const int sh = (canvas.height() - 2 * spacing) / 3;

  return {QRect(0, 0, mw, canvas.height()),
          QRect(mw + spacing, 0, sw, sh),
          QRect(mw + spacing, sh + spacing, sw, sh),
          QRect(mw + spacing, 2 * (sh + spacing), sw, canvas.height() - 2 * (sh + spacing))};
}

[[nodiscard]] static QVector<QRect> joinedWithFloater(const QSize& canvas, const int spacing)
{
  const int lw = (canvas.width() - spacing) / 2;

  return {QRect(0, 0, lw, canvas.height()),
          QRect(lw + spacing, 0, canvas.width() - lw - spacing, canvas.height()),
          QRect(200, 150, 260, 180)};
}

[[nodiscard]] static QVector<QRect> overlappingPair(const QSize& canvas, const int spacing)
{
  Q_UNUSED(canvas)
  Q_UNUSED(spacing)

  return {QRect(120, 90, 400, 300), QRect(300, 200, 400, 300)};
}

[[nodiscard]] static QVector<QRect> build(const int layout, const QSize& canvas, const int spacing)
{
  switch (layout) {
    case 1:
      return masterStack(canvas, spacing);
    case 2:
      return joinedWithFloater(canvas, spacing);
    case 3:
      return overlappingPair(canvas, spacing);
    default:
      return grid2x2(canvas, spacing);
  }
}

[[nodiscard]] static bool spansTheCanvas(const int layout)
{
  return layout != 3;
}

[[nodiscard]] static QHash<int, int> joins(const QVector<QRect>& rects, const bool horizontal)
{
  QHash<int, int> out;
  for (int i = 0; i < rects.size(); ++i) {
    for (int j = 0; j < rects.size(); ++j) {
      if (i == j)
        continue;

      const QRect a     = rects[i];
      const QRect b     = rects[j];
      const int overlap = horizontal
                          ? qMin(a.y() + a.height(), b.y() + b.height()) - qMax(a.y(), b.y())
                          : qMin(a.x() + a.width(), b.x() + b.width()) - qMax(a.x(), b.x());
      if (overlap <= 0)
        continue;

      const int distance = horizontal ? b.x() - (a.x() + a.width()) : b.y() - (a.y() + a.height());
      out.insert(static_cast<int>(i * rects.size() + j), distance);
    }
  }

  return out;
}

[[nodiscard]] static bool joinsHold(const QVector<QRect>& before,
                                    const QVector<QRect>& after,
                                    const int spacing)
{
  for (const bool horizontal : {true, false}) {
    const auto was = joins(before, horizontal);
    const auto now = joins(after, horizontal);
    for (auto it = was.cbegin(); it != was.cend(); ++it)
      if (it.value() == spacing && now.value(it.key(), spacing + 1) != spacing)
        return false;
  }

  return true;
}

[[nodiscard]] static bool edgesAreFlush(const QVector<QRect>& rects, const QSize& canvas)
{
  int left   = rects.first().x();
  int top    = rects.first().y();
  int right  = 0;
  int bottom = 0;
  for (const auto& rect : rects) {
    left   = qMin(left, rect.x());
    top    = qMin(top, rect.y());
    right  = qMax(right, rect.x() + rect.width());
    bottom = qMax(bottom, rect.y() + rect.height());
  }

  return left == 0 && top == 0 && right == canvas.width() && bottom == canvas.height();
}

[[nodiscard]] static int worstEdgeShift(const QVector<QRect>& a, const QVector<QRect>& b)
{
  int worst = 0;
  for (int i = 0; i < qMin(a.size(), b.size()); ++i) {
    worst = qMax(worst, qAbs(a[i].x() - b[i].x()));
    worst = qMax(worst, qAbs(a[i].y() - b[i].y()));
    worst = qMax(worst, qAbs(a[i].width() - b[i].width()));
    worst = qMax(worst, qAbs(a[i].height() - b[i].height()));
  }

  return worst;
}

}  // namespace fixtures

/**
 * @brief Verifies the extracted tiler (spec 0053): Grid reproduces the pre-refactor layout
 *        exactly, and every pattern tiles any widget count without gaps or overlaps.
 */
class TstLayoutPatterns : public QObject {
  Q_OBJECT

private slots:
  void gridMatchesPreRefactorBaseline_data();
  void gridMatchesPreRefactorBaseline();
  void everyPatternCoversTheCanvas_data();
  void everyPatternCoversTheCanvas();
  void patternsHonorTheRatioStops();
  void tilingIsDeterministic();
  void degenerateInputsYieldNothing();
  void rescaleHoldsJoinsAndOuterEdges_data();
  void rescaleHoldsJoinsAndOuterEdges();
  void rescaleFromTheReferenceIsLossless();
  void rescaleSettlesWhenReDerived();
  void rescaleLeavesUntouchedWidgetsAlone();
  void rescaleDegradesOnACrampedCanvas();

private:
  [[nodiscard]] static QVector<LayoutEnv> canvases();
  [[nodiscard]] static QVector<int> spacings();
};

/**
 * @brief The canvas shapes every case is exercised against: landscape, portrait, square,
 *        with and without margins/spacing, plus a deliberately cramped one.
 */
QVector<LayoutEnv> TstLayoutPatterns::canvases()
{
  QVector<LayoutEnv> out;

  const auto make = [](int w, int h, int margin, int spacing) {
    LayoutEnv env;
    env.availW      = w;
    env.availH      = h;
    env.margin      = margin;
    env.spacing     = spacing;
    env.isLandscape = w >= h;
    env.minWidth    = 100;
    env.minHeight   = 80;
    return env;
  };

  out.append(make(1920, 1080, 0, -1));
  out.append(make(1920, 1080, 8, 4));
  out.append(make(1024, 1400, 0, -1));
  out.append(make(1024, 1400, 12, 0));
  out.append(make(900, 900, 0, 2));
  out.append(make(420, 320, 0, -1));

  return out;
}

void TstLayoutPatterns::gridMatchesPreRefactorBaseline_data()
{
  QTest::addColumn<int>("count");
  QTest::addColumn<int>("canvas");

  const auto envs = canvases();
  for (int n = 1; n <= 12; ++n)
    for (int c = 0; c < envs.size(); ++c)
      QTest::newRow(qPrintable(QStringLiteral("n=%1 canvas=%2").arg(n).arg(c))) << n << c;
}

/**
 * @brief AC4: the Grid pattern is byte-identical to the layout that shipped before the
 *        tiler was extracted, so existing dashboards do not move.
 */
void TstLayoutPatterns::gridMatchesPreRefactorBaseline()
{
  QFETCH(int, count);
  QFETCH(int, canvas);

  const LayoutEnv env = canvases().at(canvas);
  QCOMPARE(UI::Layouts::tile(count, Pattern::Grid, env), reference::grid(count, env));
}

void TstLayoutPatterns::everyPatternCoversTheCanvas_data()
{
  QTest::addColumn<int>("pattern");
  QTest::addColumn<int>("count");
  QTest::addColumn<int>("canvas");

  const QVector<Pattern> patterns = {Pattern::Grid,
                                     Pattern::MasterStack,
                                     Pattern::MasterGrid,
                                     Pattern::Row,
                                     Pattern::Column,
                                     Pattern::Spiral};

  const auto envs = canvases();
  for (int p = 0; p < patterns.size(); ++p)
    for (int n = 1; n <= 12; ++n)
      for (int c = 0; c < envs.size(); ++c)
        QTest::newRow(qPrintable(QStringLiteral("p=%1 n=%2 canvas=%3").arg(p).arg(n).arg(c)))
          << p << n << c;
}

/**
 * @brief AC2: every pattern emits exactly one rect per widget, all inside the canvas, none
 *        empty, and no two overlapping beyond the (possibly negative) shared-border spacing.
 */
void TstLayoutPatterns::everyPatternCoversTheCanvas()
{
  QFETCH(int, pattern);
  QFETCH(int, count);
  QFETCH(int, canvas);

  const LayoutEnv env  = canvases().at(canvas);
  const auto rects     = UI::Layouts::tile(count, static_cast<Pattern>(pattern), env);
  const int fullWidth  = env.availW + 2 * env.margin;
  const int fullHeight = env.availH + 2 * env.margin;

  QCOMPARE(rects.size(), count);

  for (const auto& rect : rects) {
    QVERIFY2(rect.width() > 0 && rect.height() > 0, "pattern emitted an empty rect");
    QVERIFY2(rect.left() >= 0 && rect.top() >= 0, "pattern emitted a rect outside the canvas");
    QVERIFY2(rect.left() + rect.width() <= fullWidth, "pattern overflowed the canvas width");
    QVERIFY2(rect.top() + rect.height() <= fullHeight, "pattern overflowed the canvas height");
  }

  const int overlapSlack = qMax(0, -env.spacing);
  for (int i = 0; i < rects.size(); ++i) {
    for (int j = i + 1; j < rects.size(); ++j) {
      const QRect a = rects[i].adjusted(overlapSlack, overlapSlack, -overlapSlack, -overlapSlack);
      QVERIFY2(!a.intersects(rects[j]), "two widgets overlap");
    }
  }

  if (env.spacing >= 0) {
    int left   = rects.first().left();
    int top    = rects.first().top();
    int right  = 0;
    int bottom = 0;
    for (const auto& rect : rects) {
      left   = qMin(left, rect.left());
      top    = qMin(top, rect.top());
      right  = qMax(right, rect.left() + rect.width());
      bottom = qMax(bottom, rect.top() + rect.height());
    }

    QCOMPARE(QRect(left, top, right - left, bottom - top),
             QRect(env.margin, env.margin, env.availW, env.availH));
  }

  const auto baseline  = UI::Layouts::tile(count, Pattern::Grid, env);
  bool gridClearsFloor = true;
  for (const auto& rect : baseline)
    gridClearsFloor &= rect.width() >= env.minWidth && rect.height() >= env.minHeight;

  if (gridClearsFloor)
    for (const auto& rect : rects)
      QVERIFY2(rect.width() >= env.minWidth && rect.height() >= env.minHeight,
               "a pattern emitted a widget under the size floor instead of degrading to Grid");
}

/**
 * @brief Every ladder stop is a usable split: the patterns with a primary region keep emitting
 *        one rect per widget across the whole 1..15 sixteenths range, and out-of-range values
 *        degrade instead of collapsing a region.
 */
void TstLayoutPatterns::patternsHonorTheRatioStops()
{
  const QVector<Pattern> primary = {Pattern::MasterStack, Pattern::MasterGrid, Pattern::Spiral};

  LayoutEnv env = canvases().at(1);
  for (const auto pattern : primary) {
    for (int ratio = 1; ratio < UI::Layouts::kRatioDenominator; ++ratio) {
      env.ratio = ratio;
      QCOMPARE(UI::Layouts::tile(6, pattern, env).size(), 6);
    }

    for (const int ratio : {-4, 0, UI::Layouts::kRatioDenominator, 99}) {
      env.ratio = ratio;
      QCOMPARE(UI::Layouts::tile(6, pattern, env).size(), 6);
    }
  }
}

/**
 * @brief The tiler is pure: repeating a call with the same inputs yields the same rects.
 *        Every GUI-free acceptance check in this suite rests on that property.
 */
void TstLayoutPatterns::tilingIsDeterministic()
{
  const QVector<Pattern> patterns = {Pattern::Grid,
                                     Pattern::MasterStack,
                                     Pattern::MasterGrid,
                                     Pattern::Row,
                                     Pattern::Column,
                                     Pattern::Spiral};

  const LayoutEnv env = canvases().first();
  for (const auto pattern : patterns)
    for (int n = 1; n <= 12; ++n)
      QCOMPARE(UI::Layouts::tile(n, pattern, env), UI::Layouts::tile(n, pattern, env));
}

/**
 * @brief A zero widget count or a collapsed canvas tiles nothing instead of dividing by zero.
 */
void TstLayoutPatterns::degenerateInputsYieldNothing()
{
  LayoutEnv env = canvases().first();
  QVERIFY(UI::Layouts::tile(0, Pattern::Grid, env).isEmpty());

  env.availW = 0;
  QVERIFY(UI::Layouts::tile(4, Pattern::Grid, env).isEmpty());

  env.availW = 800;
  env.availH = 0;
  QVERIFY(UI::Layouts::tile(4, Pattern::Grid, env).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Manual rescale (spec 0053 amendment, R11/R12)
//--------------------------------------------------------------------------------------------------

// Canvas the manual fixtures are authored on, plus the lopsided one they are rescaled onto.
static const QSize kReference(1000, 700);
static const QSize kSquashed(1600, 400);

// Largest edge shift integer rounding may introduce when a layout is re-derived from an
// intermediate canvas instead of from its reference (AC10).
constexpr int kSettleTolerance = 2;

/**
 * @brief The spacing values every rescale case runs against: shared borders, flush, and two
 *        gaps either side of the seam tolerance.
 */
QVector<int> TstLayoutPatterns::spacings()
{
  return {-1, 0, 4, 16};
}

void TstLayoutPatterns::rescaleHoldsJoinsAndOuterEdges_data()
{
  QTest::addColumn<int>("layout");
  QTest::addColumn<int>("spacing");
  QTest::addColumn<QSize>("target");

  const QVector<QSize> targets = {
    QSize(2000, 1400), QSize(500, 350), kSquashed, QSize(317, 911), kReference};

  for (int layout = 0; layout < 4; ++layout)
    for (const int spacing : spacings())
      for (const auto& target : targets)
        QTest::newRow(qPrintable(QStringLiteral("layout=%1 spacing=%2 %3x%4")
                                   .arg(layout)
                                   .arg(spacing)
                                   .arg(target.width())
                                   .arg(target.height())))
          << layout << spacing << target;
}

/**
 * @brief AC9: whatever the canvas change, every join still measures exactly the configured
 *        spacing and every outer edge is still flush. The widgets absorb the resize; the gaps
 *        do not scale with it.
 */
void TstLayoutPatterns::rescaleHoldsJoinsAndOuterEdges()
{
  QFETCH(int, layout);
  QFETCH(int, spacing);
  QFETCH(QSize, target);

  const auto base = fixtures::build(layout, kReference, spacing);
  const auto out  = UI::Layouts::rescaleManual(base, kReference, target, spacing);

  QCOMPARE(out.size(), base.size());
  QVERIFY2(fixtures::joinsHold(base, out, spacing), "a join stopped measuring the spacing");

  if (fixtures::spansTheCanvas(layout))
    QVERIFY2(fixtures::edgesAreFlush(out, target), "an outer edge left the canvas bound");

  for (const auto& rect : out)
    QVERIFY2(rect.width() >= 1 && rect.height() >= 1, "rescale emitted an empty rect");
}

/**
 * @brief AC10, the path the application actually takes: geometry is always re-derived from the
 *        stored reference, so returning to an earlier canvas size reproduces it exactly, ten
 *        times over. Idempotence is the property that makes repeated resizes drift-free.
 */
void TstLayoutPatterns::rescaleFromTheReferenceIsLossless()
{
  for (int layout = 0; layout < 4; ++layout) {
    for (const int spacing : spacings()) {
      const auto base = fixtures::build(layout, kReference, spacing);
      for (int i = 0; i < 10; ++i) {
        const auto away = UI::Layouts::rescaleManual(base, kReference, kSquashed, spacing);
        QCOMPARE(away.size(), base.size());
        QCOMPARE(UI::Layouts::rescaleManual(base, kReference, kReference, spacing), base);
      }
    }
  }
}

/**
 * @brief AC10, the re-derived path: a save taken at another canvas size, or an edit committed
 *        there, feeds a rescale's own output back in. Integer rounding shifts an edge by at
 *        most kSettleTolerance and then stops - the shift must not compound the way the old
 *        seam weld's half-spacing walk did.
 */
void TstLayoutPatterns::rescaleSettlesWhenReDerived()
{
  for (int layout = 0; layout < 4; ++layout) {
    for (const int spacing : spacings()) {
      const auto base = fixtures::build(layout, kReference, spacing);
      auto current    = base;
      for (int i = 0; i < 50; ++i) {
        const auto away = UI::Layouts::rescaleManual(current, kReference, kSquashed, spacing);
        current         = UI::Layouts::rescaleManual(away, kSquashed, kReference, spacing);

        QVERIFY2(fixtures::joinsHold(base, current, spacing), "a join drifted while cycling");
        QVERIFY2(fixtures::worstEdgeShift(current, base) <= kSettleTolerance,
                 "geometry kept drifting instead of settling");
      }
    }
  }
}

/**
 * @brief AC11, the math half: editing one widget at another canvas size and coming back leaves
 *        its neighbours where they were, within the settling bound.
 */
void TstLayoutPatterns::rescaleLeavesUntouchedWidgetsAlone()
{
  for (int layout = 0; layout < 4; ++layout) {
    for (const int spacing : spacings()) {
      const auto base = fixtures::build(layout, kReference, spacing);
      auto edited     = UI::Layouts::rescaleManual(base, kReference, kSquashed, spacing);
      edited[0].setWidth(qMax(20, edited[0].width() - 30));

      const auto back = UI::Layouts::rescaleManual(edited, kSquashed, kReference, spacing);
      QCOMPARE(back.size(), base.size());

      for (int i = 1; i < back.size(); ++i)
        QVERIFY2(fixtures::worstEdgeShift({back[i]}, {base[i]}) <= kSettleTolerance,
                 "an untouched widget moved when a neighbour was edited");
    }
  }
}

/**
 * @brief A canvas too small to host the gaps loses the gaps, not the widgets: spacing shrinks
 *        first, and no widget ends up empty or stacked on top of another.
 */
void TstLayoutPatterns::rescaleDegradesOnACrampedCanvas()
{
  for (const int spacing : spacings()) {
    const auto base = fixtures::grid2x2(kReference, spacing);
    const auto out  = UI::Layouts::rescaleManual(base, kReference, QSize(12, 10), spacing);

    QCOMPARE(out.size(), base.size());
    for (const auto& rect : out)
      QVERIFY2(rect.width() >= 1 && rect.height() >= 1, "degrade emitted an empty rect");

    const int slack = qMax(0, -spacing);
    for (int i = 0; i < out.size(); ++i)
      for (int j = i + 1; j < out.size(); ++j)
        QVERIFY2(!out[i].adjusted(slack, slack, -slack, -slack).intersects(out[j]),
                 "degrade overlapped two widgets");
  }
}

QTEST_GUILESS_MAIN(TstLayoutPatterns)
#include "tst_layout_patterns.moc"
