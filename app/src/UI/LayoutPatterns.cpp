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

#include "LayoutPatterns.h"

#include <algorithm>
#include <limits>
#include <QHash>
#include <QSet>
#include <QtGlobal>
#include <QtMath>
#include <utility>

//--------------------------------------------------------------------------------------------------
// Grid: the historical auto-layout, moved here verbatim
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks the auto-layout column count for a window count: fixed choices
 *        up to six windows, aspect-aware square-root fit beyond that.
 */
[[nodiscard]] static int autoLayoutColumnCount(const int n, const UI::Layouts::LayoutEnv& env)
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

/**
 * @brief Lays out windows in a uniform cols x rows grid in row-major order;
 *        requires the window count to be an exact multiple of cols.
 */
[[nodiscard]] static QVector<QRect> tileExactGrid(const int n,
                                                  const UI::Layouts::LayoutEnv& env,
                                                  const int cols)
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

/**
 * @brief Lays out windows in equal-width columns, column-major, when the count
 *        does not fill a grid evenly: every widget keeps the same width, the
 *        extra windows stack in the trailing columns, and each column's windows
 *        split the full canvas height evenly.
 */
[[nodiscard]] static QVector<QRect> tileUnevenColumns(const int n,
                                                      const UI::Layouts::LayoutEnv& env,
                                                      const int cols)
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

/**
 * @brief Tiles into equal-width columns so every widget shares the same width regardless of
 *        count; exact grids keep row-major order.
 */
[[nodiscard]] static QVector<QRect> tileGrid(const int n, const UI::Layouts::LayoutEnv& env)
{
  const int cols = autoLayoutColumnCount(n, env);
  if (n % cols == 0)
    return tileExactGrid(n, env, cols);

  return tileUnevenColumns(n, env, cols);
}

//--------------------------------------------------------------------------------------------------
// The remaining patterns
//--------------------------------------------------------------------------------------------------

/**
 * @brief The full usable area as a rectangle, in the same origin convention the grid tilers use.
 */
[[nodiscard]] static QRect wholeCanvas(const UI::Layouts::LayoutEnv& env)
{
  return QRect(env.margin, env.margin, env.availW, env.availH);
}

/**
 * @brief Returns @a env narrowed to a sub-region, for a pattern that tiles part of the canvas
 *        with another pattern. The caller translates the result into place.
 */
[[nodiscard]] static UI::Layouts::LayoutEnv subEnv(const UI::Layouts::LayoutEnv& env,
                                                   const int w,
                                                   const int h)
{
  UI::Layouts::LayoutEnv sub = env;
  sub.margin                 = 0;
  sub.availW                 = w;
  sub.availH                 = h;
  sub.isLandscape            = w >= h;
  return sub;
}

/**
 * @brief Moves a sub-layout into its place on the canvas.
 */
static void translateAll(QVector<QRect>& rects, const int dx, const int dy)
{
  for (auto& rect : rects)
    rect.translate(dx, dy);
}

/**
 * @brief Splits @a n widgets into one strip: equal shares along one axis, full extent across
 *        the other. Row and Column are this, and the master patterns stack their remainder
 *        with it.
 */
[[nodiscard]] static QVector<QRect> tileStrip(const int n,
                                              const UI::Layouts::LayoutEnv& env,
                                              const bool horizontal)
{
  const int spacingForSizing = qMax(env.spacing, 0);
  const int axis             = horizontal ? env.availW : env.availH;
  const int total            = qMax(1, axis - (n - 1) * spacingForSizing);
  const int base             = total / n;
  const int extra            = total % n;

  QVector<QRect> out;
  out.reserve(n);

  int running = env.margin;
  for (int i = 0; i < n; ++i) {
    const int size = base + (i < extra ? 1 : 0);
    out.append(horizontal ? QRect(running, env.margin, size, env.availH)
                          : QRect(env.margin, running, env.availW, size));
    running += size + env.spacing;
  }

  return out;
}

/**
 * @brief Master plus remainder: the primary widget takes the split ratio of the canvas along
 *        its long axis, and the rest share what is left - stacked in a strip, or gridded when
 *        @a gridRemainder is set.
 */
[[nodiscard]] static QVector<QRect> tileMaster(const int n,
                                               const UI::Layouts::LayoutEnv& env,
                                               const bool gridRemainder)
{
  if (n == 1)
    return {wholeCanvas(env)};

  const bool vertical = env.isLandscape;
  const int axis      = vertical ? env.availW : env.availH;
  const int cross     = vertical ? env.availH : env.availW;
  const int primary =
    qBound(1, axis * env.ratio / UI::Layouts::kRatioDenominator, qMax(1, axis - 1));
  const int rest = axis - primary - env.spacing;
  if (rest < 1)
    return {};

  QVector<QRect> out;
  out.reserve(n);
  out.append(vertical ? QRect(env.margin, env.margin, primary, cross)
                      : QRect(env.margin, env.margin, cross, primary));

  const auto region = vertical ? subEnv(env, rest, cross) : subEnv(env, cross, rest);
  auto remainder    = gridRemainder ? tileGrid(n - 1, region) : tileStrip(n - 1, region, !vertical);
  translateAll(remainder,
               vertical ? env.margin + primary + env.spacing : env.margin,
               vertical ? env.margin : env.margin + primary + env.spacing);

  out.append(remainder);
  return out;
}

/**
 * @brief Spiral: each widget takes the split ratio of what is left, along whichever axis of the
 *        remaining rectangle is longer, so the cut alternates on its own. The last widget takes
 *        the remainder whole.
 */
[[nodiscard]] static QVector<QRect> tileSpiral(const int n, const UI::Layouts::LayoutEnv& env)
{
  QVector<QRect> out;
  out.reserve(n);

  QRect remaining = wholeCanvas(env);
  for (int i = 0; i < n - 1; ++i) {
    const bool vertical = remaining.width() >= remaining.height();
    const int axis      = vertical ? remaining.width() : remaining.height();
    const int take =
      qBound(1, axis * env.ratio / UI::Layouts::kRatioDenominator, qMax(1, axis - 1));
    const int rest = axis - take - env.spacing;
    if (rest < 1)
      return {};

    if (vertical) {
      out.append(QRect(remaining.x(), remaining.y(), take, remaining.height()));
      remaining.setX(remaining.x() + take + env.spacing);
      remaining.setWidth(rest);
    }

    else {
      out.append(QRect(remaining.x(), remaining.y(), remaining.width(), take));
      remaining.setY(remaining.y() + take + env.spacing);
      remaining.setHeight(rest);
    }
  }

  out.append(remaining);
  return out;
}

/**
 * @brief Whether every rectangle clears the size floor. A pattern that cannot is dropped in
 *        favour of Grid: the spec would rather show fewer subdivisions than unusable widgets.
 */
[[nodiscard]] static bool meetsFloor(const QVector<QRect>& rects, const UI::Layouts::LayoutEnv& env)
{
  for (const auto& rect : rects)
    if (rect.width() < env.minWidth || rect.height() < env.minHeight)
      return false;

  return true;
}

/**
 * @brief Routes a pattern to its tiler; Grid and anything unrecognized fall through to the
 *        historical layout.
 */
[[nodiscard]] static QVector<QRect> dispatchPattern(const int n,
                                                    const UI::Layouts::Pattern pattern,
                                                    const UI::Layouts::LayoutEnv& env)
{
  switch (pattern) {
    case UI::Layouts::Pattern::MasterStack:
      return tileMaster(n, env, false);
    case UI::Layouts::Pattern::MasterGrid:
      return tileMaster(n, env, true);
    case UI::Layouts::Pattern::Row:
      return tileStrip(n, env, true);
    case UI::Layouts::Pattern::Column:
      return tileStrip(n, env, false);
    case UI::Layouts::Pattern::Spiral:
      return tileSpiral(n, env);
    case UI::Layouts::Pattern::Grid:
      return tileGrid(n, env);
  }

  return tileGrid(n, env);
}

//--------------------------------------------------------------------------------------------------
// Manual rescale: the seam model
//--------------------------------------------------------------------------------------------------

namespace detail {

/**
 * @brief One widget's extent along a single axis, as a half-open [lo, hi) edge pair.
 */
struct Span {
  int lo;
  int hi;
};

}  // namespace detail

using detail::Span;

/**
 * @brief Maps every edge coordinate onto a shared representative, chaining values that sit
 *        within the seam tolerance of each other. A cluster holding a canvas bound adopts that
 *        bound, so outer edges stay flush instead of drifting toward the cluster average.
 */
[[nodiscard]] static QHash<int, int> clusterEdges(const QVector<int>& values, const int canvasEnd)
{
  QVector<int> edges = values;
  edges.append(0);
  edges.append(canvasEnd);
  std::sort(edges.begin(), edges.end());
  edges.erase(std::unique(edges.begin(), edges.end()), edges.end());

  QHash<int, int> mapping;
  int index = 0;
  while (index < edges.size()) {
    int last      = index;
    long long sum = edges[index];
    while (last + 1 < edges.size()
           && edges[last + 1] - edges[last] <= UI::Layouts::kSeamTolerance) {
      ++last;
      sum += edges[last];
    }

    const int count    = last - index + 1;
    int representative = static_cast<int>(sum / count);
    if (edges[index] <= 0 && edges[last] >= 0)
      representative = 0;

    else if (edges[index] <= canvasEnd && edges[last] >= canvasEnd)
      representative = canvasEnd;

    for (int k = index; k <= last; ++k)
      mapping.insert(edges[k], representative);

    index = last + 1;
  }

  return mapping;
}

/**
 * @brief Returns the seam lines a join actually consumes spacing at: interior lines where one
 *        widget ends and another begins. A lone floating edge sits on a seam of its own and
 *        consumes nothing, which is why it cannot be counted here.
 */
[[nodiscard]] static QSet<int> gapConsumingSeams(const QHash<int, int>& mapping,
                                                 const QVector<int>& leads,
                                                 const QVector<int>& trails,
                                                 const int canvasEnd)
{
  QSet<int> leading;
  QSet<int> trailing;
  for (const int edge : leads)
    leading.insert(mapping.value(edge));

  for (const int edge : trails)
    trailing.insert(mapping.value(edge));

  QSet<int> seams;
  for (const int seam : std::as_const(leading))
    if (seam > 0 && seam < canvasEnd && trailing.contains(seam))
      seams.insert(seam);

  return seams;
}

/**
 * @brief Shrinks the spacing when the new canvas cannot host every gap at full size, so a
 *        cramped canvas loses its gaps before it loses widgets. A negative spacing (overlapped
 *        borders) always fits and is returned untouched.
 */
[[nodiscard]] static int effectiveSpacing(const int spacing, const int seams, const int extent)
{
  if (seams <= 0 || extent - seams * spacing > 0)
    return spacing;

  return qBound(0, (extent - 1) / seams, spacing);
}

/**
 * @brief Places every seam line on the new canvas: deflate it by the gaps that precede it,
 *        scale the resulting gap-free span, then re-inflate. The outermost lines land exactly
 *        on 0 and the canvas extent, which is what keeps outer edges flush at any size.
 */
[[nodiscard]] static QHash<int, int> seamPositions(const QVector<int>& seams,
                                                   const QSet<int>& gapSeams,
                                                   const int refExtent,
                                                   const int newExtent,
                                                   const int spacing)
{
  const int count    = static_cast<int>(gapSeams.size());
  const int refSpan  = refExtent - count * spacing;
  const int newSpan  = newExtent - count * spacing;
  const bool spanned = refSpan > 0 && newSpan > 0;
  const double scale =
    spanned ? double(newSpan) / double(refSpan) : double(newExtent) / double(refExtent);
  const int gap = spanned ? spacing : 0;

  QHash<int, int> positions;
  int preceding = 0;
  int previous  = std::numeric_limits<int>::min();
  for (const int seam : seams) {
    const int deflated = seam - preceding * gap;
    const int scaled   = seam >= refExtent ? newExtent : qRound(deflated * scale) + preceding * gap;
    const int placed   = qMax(scaled, previous);
    positions.insert(seam, placed);
    previous = placed;
    if (gapSeams.contains(seam))
      ++preceding;
  }

  return positions;
}

/**
 * @brief Rescales one axis. Normalizing each leading edge by the spacing before clustering is
 *        the whole trick: it collapses both sides of a join onto one value, so the join is a
 *        seam in the model rather than a gap the tolerance has to recognize afterwards.
 */
[[nodiscard]] static QVector<Span> rescaleAxis(const QVector<Span>& spans,
                                               const int refExtent,
                                               const int newExtent,
                                               const int spacing)
{
  QVector<int> leads;
  QVector<int> trails;
  leads.reserve(spans.size());
  trails.reserve(spans.size());
  for (const auto& span : spans) {
    leads.append(span.lo > 0 ? span.lo - spacing : 0);
    trails.append(span.hi);
  }

  const auto mapping  = clusterEdges(leads + trails, refExtent);
  const auto gapSeams = gapConsumingSeams(mapping, leads, trails, refExtent);
  const int gap       = effectiveSpacing(spacing, static_cast<int>(gapSeams.size()), newExtent);

  QVector<int> seams;
  seams.reserve(mapping.size());
  for (auto it = mapping.cbegin(); it != mapping.cend(); ++it)
    seams.append(it.value());

  std::sort(seams.begin(), seams.end());
  seams.erase(std::unique(seams.begin(), seams.end()), seams.end());
  const auto positions = seamPositions(seams, gapSeams, refExtent, newExtent, gap);

  QVector<Span> out;
  out.reserve(spans.size());
  for (int i = 0; i < spans.size(); ++i) {
    const int seam  = mapping.value(leads[i]);
    const int trail = positions.value(mapping.value(trails[i]));
    const int lo    = positions.value(seam) + (seam > 0 ? gap : 0);
    out.append(Span{lo, qMax(trail, lo + 1)});
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the geometry for @a count widgets under @a pattern, in widget order.
 */
QVector<QRect> UI::Layouts::tile(const int count, const Pattern pattern, const LayoutEnv& env)
{
  if (count <= 0 || env.availW <= 0 || env.availH <= 0)
    return {};

  if (pattern == Pattern::Grid)
    return tileGrid(count, env);

  const auto candidate = dispatchPattern(count, pattern, env);
  if (candidate.size() != count || !meetsFloor(candidate, env))
    return tileGrid(count, env);

  return candidate;
}

/**
 * @brief Maps a manual layout built on @a refCanvas onto @a newCanvas, holding every join at
 *        exactly @a spacing pixels and every outer edge flush with the canvas.
 */
QVector<QRect> UI::Layouts::rescaleManual(const QVector<QRect>& rects,
                                          const QSize& refCanvas,
                                          const QSize& newCanvas,
                                          const int spacing)
{
  const bool sized = refCanvas.width() > 0 && refCanvas.height() > 0 && newCanvas.width() > 0
                  && newCanvas.height() > 0;
  if (rects.isEmpty() || !sized)
    return rects;

  QVector<Span> horizontal;
  QVector<Span> vertical;
  horizontal.reserve(rects.size());
  vertical.reserve(rects.size());
  for (const auto& rect : rects) {
    horizontal.append(Span{rect.x(), rect.x() + rect.width()});
    vertical.append(Span{rect.y(), rect.y() + rect.height()});
  }

  const auto x = rescaleAxis(horizontal, refCanvas.width(), newCanvas.width(), spacing);
  const auto y = rescaleAxis(vertical, refCanvas.height(), newCanvas.height(), spacing);

  QVector<QRect> out;
  out.reserve(rects.size());
  for (int i = 0; i < rects.size(); ++i)
    out.append(QRect(x[i].lo, y[i].lo, x[i].hi - x[i].lo, y[i].hi - y[i].lo));

  return out;
}

/**
 * @brief Returns the offered split ratios, in sixteenths (1/4, 3/8, 1/2, 5/8, 3/4).
 */
QVector<int> UI::Layouts::ratioStops()
{
  return {4, 6, kDefaultRatio, 10, 12};
}

/**
 * @brief Returns the persisted identifier for a pattern; Grid is the empty default.
 */
QString UI::Layouts::patternId(const Pattern pattern)
{
  switch (pattern) {
    case Pattern::MasterStack:
      return QStringLiteral("master-stack");
    case Pattern::MasterGrid:
      return QStringLiteral("master-grid");
    case Pattern::Row:
      return QStringLiteral("row");
    case Pattern::Column:
      return QStringLiteral("column");
    case Pattern::Spiral:
      return QStringLiteral("spiral");
    case Pattern::Grid:
      return {};
  }

  return {};
}

/**
 * @brief Resolves a persisted identifier; anything unknown degrades to Grid rather than
 *        failing the load, so a project written by a newer build still opens.
 */
UI::Layouts::Pattern UI::Layouts::patternFromId(const QString& id)
{
  const auto key = id.trimmed().toLower();
  if (key == QLatin1String("master-stack"))
    return Pattern::MasterStack;

  if (key == QLatin1String("master-grid"))
    return Pattern::MasterGrid;

  if (key == QLatin1String("row"))
    return Pattern::Row;

  if (key == QLatin1String("column"))
    return Pattern::Column;

  if (key == QLatin1String("spiral"))
    return Pattern::Spiral;

  return Pattern::Grid;
}

/**
 * @brief Returns whether the pattern has a primary region, i.e. whether its split ratio is
 *        meaningful and the picker should offer the ratio control.
 */
bool UI::Layouts::patternHasPrimary(const Pattern pattern)
{
  return pattern == Pattern::MasterStack || pattern == Pattern::MasterGrid
      || pattern == Pattern::Spiral;
}
