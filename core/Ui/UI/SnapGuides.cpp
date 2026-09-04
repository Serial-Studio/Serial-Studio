/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "SnapGuides.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <QtGlobal>

constexpr int kRankEdge     = 0;
constexpr int kRankCenter   = 1;
constexpr int kRankSize     = 1;
constexpr int kRankSpacing  = 2;
constexpr int kRankFraction = 3;

// Imperial wrench ladder; a rung closer-spaced than kMinFractionSpacing is skipped.
constexpr int kFractionDenominators[] = {2, 4, 8, 16};
constexpr int kMinFractionSpacing     = 40;

namespace detail {

/**
 * @brief One snap candidate on a single axis: the shift to apply, the guide
 *        line it aligns to, and the metadata needed to render its visuals.
 */
struct Candidate {
  int delta         = 0;
  int line          = 0;
  int rank          = kRankEdge;
  int gap           = 0;
  bool canvasTarget = false;
  bool afterPair    = false;
  QRect pairA       = {};
  QRect pairB       = {};
};

}  // namespace detail

using detail::Candidate;

/**
 * @brief Returns the rect's start coordinate on the given axis.
 */
static int rectLo(const QRect& r, const bool horiz)
{
  return horiz ? r.x() : r.y();
}

/**
 * @brief Returns the rect's end coordinate on the given axis.
 */
static int rectHi(const QRect& r, const bool horiz)
{
  return horiz ? r.x() + r.width() : r.y() + r.height();
}

/**
 * @brief Returns the rect's center coordinate on the given axis.
 */
static int rectMid(const QRect& r, const bool horiz)
{
  return rectLo(r, horiz) + (horiz ? r.width() : r.height()) / 2;
}

/**
 * @brief Returns the rect's size on the given axis.
 */
static int rectSize(const QRect& r, const bool horiz)
{
  return horiz ? r.width() : r.height();
}

/**
 * @brief Returns whether two rects overlap on the axis perpendicular to horiz.
 */
static bool crossOverlaps(const QRect& a, const QRect& b, const bool horiz)
{
  return rectLo(a, !horiz) < rectHi(b, !horiz) && rectLo(b, !horiz) < rectHi(a, !horiz);
}

/**
 * @brief Returns whether a window spanning [lo, lo+size) stays inside the canvas extent.
 */
static bool fitsCanvas(const int lo, const int size, const int extent)
{
  return lo >= 0 && lo + size <= extent;
}

/**
 * @brief Appends the candidate when it is within threshold and keeps the moved
 *        geometry inside the canvas.
 */
static void appendIfViable(const Candidate& candidate,
                           const UI::Snap::SnapInput& input,
                           const bool horiz,
                           QVector<Candidate>& out)
{
  if (qAbs(candidate.delta) > input.threshold)
    return;

  const int extent = horiz ? input.canvas.width() : input.canvas.height();
  const int newLo  = rectLo(input.rect, horiz) + candidate.delta;
  if (!fitsCanvas(newLo, rectSize(input.rect, horiz), extent))
    return;

  out.append(candidate);
}

/**
 * @brief Collects edge/center alignment candidates against every sibling, the
 *        canvas edges/centerline and the wrench-ladder canvas fractions for one axis.
 *        The two abutting sibling edges carry siblingSpacing so a flush snap
 *        shares a border instead of doubling it.
 */
static void appendAlignCandidates(const UI::Snap::SnapInput& input,
                                  const bool horiz,
                                  QVector<Candidate>& out)
{
  const int lo      = rectLo(input.rect, horiz);
  const int hi      = rectHi(input.rect, horiz);
  const int mid     = rectMid(input.rect, horiz);
  const int extent  = horiz ? input.canvas.width() : input.canvas.height();
  const int spacing = input.siblingSpacing;

  for (const auto& sibling : input.siblings) {
    const int sLo  = rectLo(sibling, horiz);
    const int sHi  = rectHi(sibling, horiz);
    const int sMid = rectMid(sibling, horiz);

    appendIfViable({sLo - lo, sLo, kRankEdge}, input, horiz, out);
    appendIfViable({sHi + spacing - lo, sHi, kRankEdge}, input, horiz, out);
    appendIfViable({sLo - spacing - hi, sLo, kRankEdge}, input, horiz, out);
    appendIfViable({sHi - hi, sHi, kRankEdge}, input, horiz, out);
    appendIfViable({sMid - mid, sMid, kRankCenter}, input, horiz, out);
  }

  appendIfViable({0 - lo, 0, kRankEdge, 0, true}, input, horiz, out);
  appendIfViable({extent - hi, extent, kRankEdge, 0, true}, input, horiz, out);
  appendIfViable({extent / 2 - mid, extent / 2, kRankCenter, 0, true}, input, horiz, out);

  for (const int den : kFractionDenominators) {
    if (extent / den < kMinFractionSpacing)
      continue;

    for (int num = 1; num < den; ++num) {
      const int frac = extent * num / den;
      appendIfViable({frac - lo, frac, kRankFraction, 0, true}, input, horiz, out);
      appendIfViable({frac - hi, frac, kRankFraction, 0, true}, input, horiz, out);
      appendIfViable({frac - mid, frac, kRankFraction, 0, true}, input, horiz, out);
    }
  }
}

/**
 * @brief Collects equal-spacing candidates for one axis: reproducing an existing
 *        gap between two row/column neighbors, before or after the pair.
 */
static void appendSpacingCandidates(const UI::Snap::SnapInput& input,
                                    const bool horiz,
                                    QVector<Candidate>& out)
{
  QVector<QRect> row;
  row.reserve(input.siblings.size());
  for (const auto& sibling : input.siblings)
    if (crossOverlaps(sibling, input.rect, horiz))
      row.append(sibling);

  std::sort(row.begin(), row.end(), [horiz](const QRect& a, const QRect& b) {
    return rectLo(a, horiz) < rectLo(b, horiz);
  });

  const int lo   = rectLo(input.rect, horiz);
  const int size = rectSize(input.rect, horiz);

  for (int i = 0; i + 1 < row.size(); ++i) {
    const QRect& a = row[i];
    const QRect& b = row[i + 1];
    const int gap  = rectLo(b, horiz) - rectHi(a, horiz);
    if (gap <= 0)
      continue;

    Candidate after{rectHi(b, horiz) + gap - lo, 0, kRankSpacing, gap, false, true, a, b};
    appendIfViable(after, input, horiz, out);

    Candidate before{rectLo(a, horiz) - gap - size - lo, 0, kRankSpacing, gap, false, false, a, b};
    appendIfViable(before, input, horiz, out);
  }
}

/**
 * @brief Picks the winning candidate: nearest within threshold; ties prefer
 *        edges over centers, either over spacing, and all over canvas fractions.
 */
static std::optional<Candidate> pickCandidate(const QVector<Candidate>& candidates)
{
  std::optional<Candidate> best;
  for (const auto& candidate : candidates) {
    if (!best.has_value()) {
      best = candidate;
      continue;
    }

    const int bestDist = qAbs(best->delta);
    const int dist     = qAbs(candidate.delta);
    if (dist < bestDist || (dist == bestDist && candidate.rank < best->rank))
      best = candidate;
  }

  return best;
}

/**
 * @brief Builds the guide line for an alignment pick: a 1-px rect at the snap
 *        line, spanning the snapped window and every sibling aligned to it
 *        (full canvas span for canvas targets).
 */
static UI::Snap::GuideLine buildGuide(const UI::Snap::SnapInput& input,
                                      const QRect& snapped,
                                      const bool horiz,
                                      const Candidate& pick)
{
  int spanLo = rectLo(snapped, !horiz);
  int spanHi = rectHi(snapped, !horiz);

  if (pick.canvasTarget) {
    spanLo = 0;
    spanHi = horiz ? input.canvas.height() : input.canvas.width();
  }

  else {
    for (const auto& sibling : input.siblings) {
      const bool matches = rectLo(sibling, horiz) == pick.line
                        || rectHi(sibling, horiz) == pick.line
                        || rectMid(sibling, horiz) == pick.line;
      if (!matches)
        continue;

      spanLo = qMin(spanLo, rectLo(sibling, !horiz));
      spanHi = qMax(spanHi, rectHi(sibling, !horiz));
    }
  }

  const QRect rect = horiz ? QRect(pick.line, spanLo, 1, spanHi - spanLo)
                           : QRect(spanLo, pick.line, spanHi - spanLo, 1);
  const auto kind =
    pick.rank == kRankCenter ? UI::Snap::GuideKind::Center : UI::Snap::GuideKind::Edge;
  return {rect, kind};
}

/**
 * @brief Returns the axis-aligned gap rect between two rects, using the pair's
 *        cross-axis overlap (falling back to the moved window's when disjoint).
 */
static QRect gapRect(const QRect& first,
                     const QRect& second,
                     const QRect& fallback,
                     const bool horiz)
{
  int crossLo = qMax(rectLo(first, !horiz), rectLo(second, !horiz));
  int crossHi = qMin(rectHi(first, !horiz), rectHi(second, !horiz));
  if (crossLo >= crossHi) {
    crossLo = rectLo(fallback, !horiz);
    crossHi = rectHi(fallback, !horiz);
  }

  const int gapLo  = rectHi(first, horiz);
  const int gapLen = rectLo(second, horiz) - gapLo;
  return horiz ? QRect(gapLo, crossLo, gapLen, crossHi - crossLo)
               : QRect(crossLo, gapLo, crossHi - crossLo, gapLen);
}

/**
 * @brief Builds the two spacing indicators for a spacing pick: the matched
 *        existing gap and the equal gap the snapped window now forms.
 */
static void appendSpacingIndicators(const QRect& snapped,
                                    const bool horiz,
                                    const Candidate& pick,
                                    QVector<UI::Snap::SpacingIndicator>& out)
{
  out.append({gapRect(pick.pairA, pick.pairB, snapped, horiz), pick.gap});

  if (pick.afterPair)
    out.append({gapRect(pick.pairB, snapped, snapped, horiz), pick.gap});
  else
    out.append({gapRect(snapped, pick.pairA, snapped, horiz), pick.gap});
}

/**
 * @brief Appends a resize candidate when it keeps the moving edge inside the
 *        canvas and the resulting size at or above the input's minimum.
 */
static void appendIfViableResize(const Candidate& candidate,
                                 const UI::Snap::SnapInput& input,
                                 const bool horiz,
                                 const bool movingLo,
                                 QVector<Candidate>& out)
{
  if (qAbs(candidate.delta) > input.threshold)
    return;

  const int extent  = horiz ? input.canvas.width() : input.canvas.height();
  const int lo      = rectLo(input.rect, horiz);
  const int hi      = rectHi(input.rect, horiz);
  const int newLo   = movingLo ? lo + candidate.delta : lo;
  const int newHi   = movingLo ? hi : hi + candidate.delta;
  const int minSize = qMax(1, input.minSize);
  if (newLo < 0 || newHi > extent || newHi - newLo < minSize)
    return;

  out.append(candidate);
}

/**
 * @brief Collects alignment, size-match and canvas n/2..n/8 fraction size
 *        candidates for the moving edge of a resize gesture on one axis. The
 *        sibling edge the moving edge abuts carries siblingSpacing so a flush
 *        resize shares a border; the aligned sibling edge stays exact.
 */
static void appendResizeCandidates(const UI::Snap::SnapInput& input,
                                   const bool horiz,
                                   const bool movingLo,
                                   QVector<Candidate>& out)
{
  const int lo      = rectLo(input.rect, horiz);
  const int hi      = rectHi(input.rect, horiz);
  const int edge    = movingLo ? lo : hi;
  const int extent  = horiz ? input.canvas.width() : input.canvas.height();
  const int spacing = input.siblingSpacing;

  for (const auto& sibling : input.siblings) {
    const int sLo    = rectLo(sibling, horiz);
    const int sHi    = rectHi(sibling, horiz);
    const int loEdge = movingLo ? sLo : sLo - spacing;
    const int hiEdge = movingLo ? sHi + spacing : sHi;

    appendIfViableResize({loEdge - edge, sLo, kRankEdge}, input, horiz, movingLo, out);
    appendIfViableResize({hiEdge - edge, sHi, kRankEdge}, input, horiz, movingLo, out);

    const int sSize  = rectSize(sibling, horiz);
    const int target = movingLo ? hi - sSize : lo + sSize;
    Candidate size{target - edge, 0, kRankSize, 0, false, false, sibling};
    appendIfViableResize(size, input, horiz, movingLo, out);
  }

  const int canvasEdge = movingLo ? 0 : extent;
  appendIfViableResize(
    {canvasEdge - edge, canvasEdge, kRankEdge, 0, true}, input, horiz, movingLo, out);

  for (const int den : kFractionDenominators) {
    if (extent / den < kMinFractionSpacing)
      continue;

    for (int num = 1; num < den; ++num) {
      const int fracSize = extent * num / den;
      const int target   = movingLo ? hi - fracSize : lo + fracSize;
      appendIfViableResize(
        {target - edge, target, kRankFraction, 0, true}, input, horiz, movingLo, out);
    }
  }
}

/**
 * @brief Applies a resize pick (or grid quantization) to the moving edge of one
 *        axis, keeping the opposite edge fixed.
 */
static void applyResizePick(const std::optional<Candidate>& pick,
                            const UI::Snap::SnapInput& input,
                            const bool horiz,
                            const bool movingLo,
                            QRect& rect)
{
  const int lo = rectLo(rect, horiz);
  const int hi = rectHi(rect, horiz);

  int edge = movingLo ? lo : hi;
  if (pick.has_value())
    edge += pick->delta;
  else if (input.gridEnabled)
    edge = UI::Snap::snapToGrid(edge, input.gridSize);
  else
    return;

  const int newLo   = movingLo ? edge : lo;
  const int newHi   = movingLo ? hi : edge;
  const int minSize = qMax(1, input.minSize);
  if (newHi - newLo < minSize)
    return;

  if (horiz) {
    rect.moveLeft(newLo);
    rect.setWidth(newHi - newLo);
  } else {
    rect.moveTop(newLo);
    rect.setHeight(newHi - newLo);
  }
}

/**
 * @brief Quantizes a coordinate to the nearest grid line.
 */
int UI::Snap::snapToGrid(const int value, const int gridSize)
{
  if (gridSize <= 1)
    return value;

  const int offset = value >= 0 ? gridSize / 2 : -(gridSize / 2);
  return ((value + offset) / gridSize) * gridSize;
}

/**
 * @brief Snaps a coordinate onto the nearest wrench-ladder stop of @a extent when one sits
 *        within @a tolerance, else returns it untouched. Canvas bounds count as stops, so a
 *        layout rescale lands shared edges on one coordinate instead of drifting apart.
 */
int UI::Snap::snapToFraction(const int value, const int extent, const int tolerance)
{
  if (extent <= 0)
    return value;

  int best     = value;
  int bestDist = tolerance + 1;
  for (const int den : kFractionDenominators) {
    if (extent / den < kMinFractionSpacing)
      continue;

    for (int num = 0; num <= den; ++num) {
      const int stop = extent * num / den;
      const int dist = qAbs(stop - value);
      if (dist < bestDist) {
        bestDist = dist;
        best     = stop;
      }
    }
  }

  return bestDist <= tolerance ? best : value;
}

/**
 * @brief Returns @a size as a reduced wrench fraction of @a extent ("1/2", "3/16"), or empty
 *        when it is not ON a live stop -- it confirms a snap landed and must never name a
 *        fraction the size merely resembles. Coarsest rung first, so shared stops reduce.
 */
QString UI::Snap::fractionLabel(const int size, const int extent)
{
  if (size <= 0 || extent <= 0)
    return {};

  for (const int den : kFractionDenominators) {
    if (extent / den < kMinFractionSpacing)
      continue;

    for (int num = 1; num <= den; ++num) {
      if (qAbs(size - extent * num / den) > 1)
        continue;

      const int divisor = std::gcd(num, den);
      return QStringLiteral("%1/%2").arg(QString::number(num / divisor),
                                         QString::number(den / divisor));
    }
  }

  return {};
}

/**
 * @brief Resolves a move gesture: per-axis edge/center/spacing snap against
 *        siblings and canvas, grid quantization when no smart guide holds, and
 *        the guide/spacing visuals for the snapped position.
 */
UI::Snap::SnapResult UI::Snap::resolveMoveSnap(const SnapInput& input)
{
  SnapResult result;
  result.rect = input.rect;
  if (input.canvas.isEmpty())
    return result;

  QVector<Candidate> horizontal;
  QVector<Candidate> vertical;
  horizontal.reserve(input.siblings.size() * 5 + 88);
  vertical.reserve(input.siblings.size() * 5 + 88);

  if (input.smartGuidesEnabled) {
    appendAlignCandidates(input, true, horizontal);
    appendAlignCandidates(input, false, vertical);
    appendSpacingCandidates(input, true, horizontal);
    appendSpacingCandidates(input, false, vertical);
  }

  const auto pickX = pickCandidate(horizontal);
  const auto pickY = pickCandidate(vertical);

  if (pickX.has_value())
    result.rect.moveLeft(result.rect.x() + pickX->delta);
  else if (input.gridEnabled)
    result.rect.moveLeft(snapToGrid(result.rect.x(), input.gridSize));

  if (pickY.has_value())
    result.rect.moveTop(result.rect.y() + pickY->delta);
  else if (input.gridEnabled)
    result.rect.moveTop(snapToGrid(result.rect.y(), input.gridSize));

  result.guides.reserve(2);
  result.spacings.reserve(4);

  if (pickX.has_value()) {
    if (pickX->rank == kRankSpacing)
      appendSpacingIndicators(result.rect, true, *pickX, result.spacings);
    else
      result.guides.append(buildGuide(input, result.rect, true, *pickX));
  }

  if (pickY.has_value()) {
    if (pickY->rank == kRankSpacing)
      appendSpacingIndicators(result.rect, false, *pickY, result.spacings);
    else
      result.guides.append(buildGuide(input, result.rect, false, *pickY));
  }

  return result;
}

/**
 * @brief Resolves a resize gesture: the moving edge(s) snap to sibling edges,
 *        canvas edges, matching sibling sizes, or the grid; fixed edges are
 *        never emitted. Reports the guide lines and the size-matched sibling.
 */
UI::Snap::SnapResult UI::Snap::resolveResizeSnap(const SnapInput& input, const MovingEdges& edges)
{
  SnapResult result;
  result.rect = input.rect;
  if (input.canvas.isEmpty() || (edges.left && edges.right) || (edges.top && edges.bottom))
    return result;

  std::optional<Candidate> pickX;
  std::optional<Candidate> pickY;
  const bool horizMoving = edges.left || edges.right;
  const bool vertMoving  = edges.top || edges.bottom;

  QVector<Candidate> candidates;
  candidates.reserve(input.siblings.size() * 3 + 32);

  if (horizMoving) {
    if (input.smartGuidesEnabled)
      appendResizeCandidates(input, true, edges.left, candidates);

    pickX = pickCandidate(candidates);
    applyResizePick(pickX, input, true, edges.left, result.rect);
  }

  if (vertMoving) {
    candidates.clear();
    if (input.smartGuidesEnabled)
      appendResizeCandidates(input, false, edges.top, candidates);

    pickY = pickCandidate(candidates);
    applyResizePick(pickY, input, false, edges.top, result.rect);
  }

  result.guides.reserve(2);

  if (pickX.has_value()) {
    if (pickX->rank == kRankSize)
      result.sizeMatch = pickX->pairA;
    else
      result.guides.append(buildGuide(input, result.rect, true, *pickX));
  }

  if (pickY.has_value()) {
    if (pickY->rank == kRankSize) {
      if (!result.sizeMatch.isValid())
        result.sizeMatch = pickY->pairA;
    } else
      result.guides.append(buildGuide(input, result.rect, false, *pickY));
  }

  return result;
}
