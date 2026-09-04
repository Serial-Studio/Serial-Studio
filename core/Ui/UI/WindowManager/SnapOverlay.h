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

#pragma once

#include <QMap>
#include <QObject>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariantList>
#include <QVector>

#include "UI/SnapGuides.h"

class QQuickItem;

namespace UI {

/**
 * @brief Owns the visual feedback of a manual move/resize gesture: alignment guides, equal-spacing
 *        indicators, the size-matched sibling, the wrench-fraction preview and the live gesture
 *        badge. Every publisher emits only for the values that actually changed, so a gesture that
 *        resolves to the same visuals costs no QML re-evaluation.
 */
class SnapOverlay : public QObject {
  Q_OBJECT

signals:
  void sizeMatchRectChanged();
  void manualGestureChanged();
  void fractionPreviewChanged();
  void alignmentGuidesChanged();
  void spacingIndicatorsChanged();

public:
  explicit SnapOverlay(QObject* parent = nullptr);
  SnapOverlay(SnapOverlay&&)                 = delete;
  SnapOverlay(const SnapOverlay&)            = delete;
  SnapOverlay& operator=(SnapOverlay&&)      = delete;
  SnapOverlay& operator=(const SnapOverlay&) = delete;

  [[nodiscard]] bool sizeMatchVisible() const;
  [[nodiscard]] bool manualGestureActive() const;
  [[nodiscard]] const QRect& sizeMatchRect() const;
  [[nodiscard]] const QRect& fractionPreviewRect() const;
  [[nodiscard]] const QString& fractionPreviewLabel() const;
  [[nodiscard]] const QRect& manualGestureGeometry() const;
  [[nodiscard]] const QVariantList& alignmentGuides() const;
  [[nodiscard]] const QVariantList& spacingIndicators() const;
  [[nodiscard]] const QVector<QRect>& snapSiblings() const;

  void clearSnapGuides();
  void clearManualGesture();
  void clearSnapSiblings();
  void publishManualGesture(const QRect& geometry);
  void publishSnapGuides(const Snap::SnapResult& result);
  void cacheSnapSiblings(const QMap<int, QQuickItem*>& windows, const QQuickItem* target);
  void publishFractionPreview(const QRect& geometry, const QSize& canvas, bool guidesEnabled);

private:
  bool m_manualGestureActive;
  QRect m_manualGestureGeometry;
  QRect m_sizeMatchRect;
  QRect m_fractionPreviewRect;
  QString m_fractionPreviewLabel;
  QVariantList m_alignmentGuides;
  QVariantList m_spacingIndicators;
  QVector<QRect> m_snapSiblings;
};

}  // namespace UI
