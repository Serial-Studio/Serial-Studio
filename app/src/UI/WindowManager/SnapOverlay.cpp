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

#include "UI/WindowManager/SnapOverlay.h"

#include <QCoreApplication>
#include <QQuickItem>
#include <QVariantMap>

#include "SSAssert.h"
#include "UI/WindowManager/WindowGeometry.h"

//--------------------------------------------------------------------------------------------------
// Local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Serializes guide lines and spacing gaps into the QVariantList shape the QML overlay
 *        consumes.
 */
static void snapVisualsToVariants(const UI::Snap::SnapResult& result,
                                  QVariantList& guides,
                                  QVariantList& spacings)
{
  guides.reserve(result.guides.size());
  for (const auto& guide : result.guides) {
    QVariantMap entry;
    entry["x"]      = guide.rect.x();
    entry["y"]      = guide.rect.y();
    entry["width"]  = guide.rect.width();
    entry["height"] = guide.rect.height();
    entry["center"] = guide.kind == UI::Snap::GuideKind::Center;
    guides.append(entry);
  }

  spacings.reserve(result.spacings.size());
  for (const auto& spacing : result.spacings) {
    QVariantMap entry;
    entry["x"]      = spacing.rect.x();
    entry["y"]      = spacing.rect.y();
    entry["width"]  = spacing.rect.width();
    entry["height"] = spacing.rect.height();
    entry["gap"]    = spacing.gap;
    spacings.append(entry);
  }
}

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle overlay: no gesture, no guides, no cached siblings.
 */
UI::SnapOverlay::SnapOverlay(QObject* parent) : QObject(parent), m_manualGestureActive(false) {}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a size-matched sibling should be highlighted.
 */
bool UI::SnapOverlay::sizeMatchVisible() const
{
  return m_sizeMatchRect.isValid();
}

/**
 * @brief Returns whether a manual move/resize gesture is currently in progress.
 */
bool UI::SnapOverlay::manualGestureActive() const
{
  return m_manualGestureActive;
}

/**
 * @brief Returns the geometry of the sibling whose size the resize gesture matched.
 */
const QRect& UI::SnapOverlay::sizeMatchRect() const
{
  return m_sizeMatchRect;
}

/**
 * @brief Returns the footprint of the wrench-fraction resize preview; invalid when hidden.
 */
const QRect& UI::SnapOverlay::fractionPreviewRect() const
{
  return m_fractionPreviewRect;
}

/**
 * @brief Returns the fraction preview's label ("1/2 x 1/4"); empty when hidden.
 */
const QString& UI::SnapOverlay::fractionPreviewLabel() const
{
  return m_fractionPreviewLabel;
}

/**
 * @brief Returns the live geometry of the window being moved or resized.
 */
const QRect& UI::SnapOverlay::manualGestureGeometry() const
{
  return m_manualGestureGeometry;
}

/**
 * @brief Returns the alignment guide lines for the active gesture.
 */
const QVariantList& UI::SnapOverlay::alignmentGuides() const
{
  return m_alignmentGuides;
}

/**
 * @brief Returns the equal-spacing indicators for the active gesture.
 */
const QVariantList& UI::SnapOverlay::spacingIndicators() const
{
  return m_spacingIndicators;
}

/**
 * @brief Returns the sibling rectangles the snap resolver measures the gesture against.
 */
const QVector<QRect>& UI::SnapOverlay::snapSiblings() const
{
  return m_snapSiblings;
}

//--------------------------------------------------------------------------------------------------
// Snap sibling cache
//--------------------------------------------------------------------------------------------------

/**
 * @brief Caches the geometry of every visible normal window except the gesture target, so
 *        per-move snap resolution never re-walks the window map.
 */
void UI::SnapOverlay::cacheSnapSiblings(const QMap<int, QQuickItem*>& windows,
                                        const QQuickItem* target)
{
  m_snapSiblings.clear();
  m_snapSiblings.reserve(windows.size());
  for (auto it = windows.constBegin(); it != windows.constEnd(); ++it) {
    auto* win = it.value();
    if (!win || win == target || !win->isVisible() || win->state() != QStringLiteral("normal"))
      continue;

    m_snapSiblings.append(WindowGeometry::extractGeometry(win));
  }
}

/**
 * @brief Drops the cached sibling geometry.
 */
void UI::SnapOverlay::clearSnapSiblings()
{
  m_snapSiblings.clear();
}

//--------------------------------------------------------------------------------------------------
// Publication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes the guide/spacing/size-match visuals of a snap resolution, emitting only for
 *        the properties that actually changed.
 */
void UI::SnapOverlay::publishSnapGuides(const Snap::SnapResult& result)
{
  QVariantList guides;
  QVariantList spacings;
  snapVisualsToVariants(result, guides, spacings);

  if (m_alignmentGuides != guides) {
    m_alignmentGuides = std::move(guides);
    Q_EMIT alignmentGuidesChanged();
  }

  if (m_spacingIndicators != spacings) {
    m_spacingIndicators = std::move(spacings);
    Q_EMIT spacingIndicatorsChanged();
  }

  if (m_sizeMatchRect != result.sizeMatch) {
    m_sizeMatchRect = result.sizeMatch;
    Q_EMIT sizeMatchRectChanged();
  }
}

/**
 * @brief Publishes the wrench-fraction preview for a resize in progress: the footprint the window
 *        occupies plus its width/height as canvas fractions. Only axes actually snapped onto a
 *        stop are named, so the panel confirms a real snap instead of approximating one. The label
 *        strings keep the UI::WindowManager translation context they shipped under.
 */
void UI::SnapOverlay::publishFractionPreview(const QRect& geometry,
                                             const QSize& canvas,
                                             const bool guidesEnabled)
{
  SS_ASSERT(canvas.width() >= 0, return);
  SS_ASSERT(canvas.height() >= 0, return);

  const QString widthLabel =
    guidesEnabled ? Snap::fractionLabel(geometry.width(), canvas.width()) : QString();
  const QString heightLabel =
    guidesEnabled ? Snap::fractionLabel(geometry.height(), canvas.height()) : QString();

  QRect rect;
  QString label;
  if (!widthLabel.isEmpty() && !heightLabel.isEmpty())
    label = QCoreApplication::translate("UI::WindowManager", "Width: %1    Height: %2")
              .arg(widthLabel, heightLabel);
  else if (!widthLabel.isEmpty())
    label = QCoreApplication::translate("UI::WindowManager", "Width: %1").arg(widthLabel);
  else if (!heightLabel.isEmpty())
    label = QCoreApplication::translate("UI::WindowManager", "Height: %1").arg(heightLabel);

  if (!label.isEmpty())
    rect = geometry;

  if (m_fractionPreviewRect == rect && m_fractionPreviewLabel == label)
    return;

  m_fractionPreviewRect  = rect;
  m_fractionPreviewLabel = label;
  Q_EMIT fractionPreviewChanged();
}

/**
 * @brief Publishes the live geometry of the active manual gesture for the badge.
 */
void UI::SnapOverlay::publishManualGesture(const QRect& geometry)
{
  if (m_manualGestureActive && m_manualGestureGeometry == geometry)
    return;

  m_manualGestureActive   = true;
  m_manualGestureGeometry = geometry;
  Q_EMIT manualGestureChanged();
}

/**
 * @brief Clears the guide/spacing/size-match visuals, emitting only when needed.
 */
void UI::SnapOverlay::clearSnapGuides()
{
  if (m_fractionPreviewRect.isValid() || !m_fractionPreviewLabel.isEmpty()) {
    m_fractionPreviewRect = QRect();
    m_fractionPreviewLabel.clear();
    Q_EMIT fractionPreviewChanged();
  }

  if (!m_alignmentGuides.isEmpty()) {
    m_alignmentGuides.clear();
    Q_EMIT alignmentGuidesChanged();
  }

  if (!m_spacingIndicators.isEmpty()) {
    m_spacingIndicators.clear();
    Q_EMIT spacingIndicatorsChanged();
  }

  if (m_sizeMatchRect.isValid()) {
    m_sizeMatchRect = QRect();
    Q_EMIT sizeMatchRectChanged();
  }
}

/**
 * @brief Ends the manual gesture: hides the badge and clears every snap visual.
 */
void UI::SnapOverlay::clearManualGesture()
{
  clearSnapGuides();
  if (!m_manualGestureActive)
    return;

  m_manualGestureActive = false;
  Q_EMIT manualGestureChanged();
}
