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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
#  define SS_HAS_CANVAS_PAINTER 1
#else
#  define SS_HAS_CANVAS_PAINTER 0
#endif

#if SS_HAS_CANVAS_PAINTER

#  include <QCanvasPainterItem>
#  include <QCanvasPainterItemRenderer>
#  include <QImage>
#  include <QPainter>

/**
 * @brief GPU-accelerated drop-in replacement for QQuickPaintedItem.
 */
class QuickPaintedItemCompat : public QCanvasPainterItem {
  Q_OBJECT

public:
  explicit QuickPaintedItemCompat(QQuickItem* parent = nullptr);
  virtual void paint(QPainter* painter) = 0;

  void setMipmap(bool enabled);
  void setOpaquePainting(bool enabled);
  void setRenderTarget(int target);

  void update();
  void update(const QRect& rect);

protected:
  [[nodiscard]] QCanvasPainterItemRenderer* createItemRenderer() const override;
  void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
  friend class QuickPaintedItemCompatRenderer;
  bool m_needsRepaint;
};

/**
 * @brief Renders QPainter output through QCanvasPainter via an offscreen image.
 */
class QuickPaintedItemCompatRenderer : public QCanvasPainterItemRenderer {
public:
  QuickPaintedItemCompatRenderer();

  void synchronize(QCanvasPainterItem* item) override;
  void initializeResources(QCanvasPainter* painter) override;
  void paint(QCanvasPainter* painter) override;

private:
  bool m_bufferDirty;
  QuickPaintedItemCompat* m_item;
  QImage m_buffer;
  QCanvasImage m_canvasImage;
};

#else  // Qt < 6.11

#  include <QQuickPaintedItem>

/**
 * @brief Pre-Qt-6.11 fallback alias when the canvas-painter shim is not built.
 */
using QuickPaintedItemCompat = QQuickPaintedItem;

#endif  // SS_HAS_CANVAS_PAINTER
