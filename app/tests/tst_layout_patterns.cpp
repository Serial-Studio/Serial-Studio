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
  void tilingIsDeterministic();
  void degenerateInputsYieldNothing();

private:
  [[nodiscard]] static QVector<LayoutEnv> canvases();
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
}

/**
 * @brief The tiler is pure: repeating a call with the same inputs yields the same rects.
 *        Every GUI-free acceptance check in this suite rests on that property.
 */
void TstLayoutPatterns::tilingIsDeterministic()
{
  const LayoutEnv env = canvases().first();
  for (int n = 1; n <= 12; ++n)
    QCOMPARE(UI::Layouts::tile(n, Pattern::Grid, env), UI::Layouts::tile(n, Pattern::Grid, env));
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

QTEST_GUILESS_MAIN(TstLayoutPatterns)
#include "tst_layout_patterns.moc"
