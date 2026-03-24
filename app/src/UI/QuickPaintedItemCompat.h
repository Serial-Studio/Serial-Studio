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

//--------------------------------------------------------------------------------------------------
// Qt Canvas Painter availability check
//--------------------------------------------------------------------------------------------------

#if QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
#  define SS_HAS_CANVAS_PAINTER 1
#else
#  define SS_HAS_CANVAS_PAINTER 0
#endif

//--------------------------------------------------------------------------------------------------
// Image-bridge base class
//--------------------------------------------------------------------------------------------------
//
// On Qt >= 6.11, inherits QCanvasPainterItem and provides a renderer that
// calls the subclass paint(QPainter*) onto an offscreen QImage, then blits
// it via QCanvasPainter::drawImage(). Existing QPainter code works unchanged
// while compositing is GPU-accelerated.
//
// On older Qt, this is just QQuickPaintedItem.
//
//--------------------------------------------------------------------------------------------------

#if SS_HAS_CANVAS_PAINTER

#  include <QCanvasPainterItem>
#  include <QCanvasPainterItemRenderer>
#  include <QImage>
#  include <QPainter>

/**
 * @class QuickPaintedItemCompat
 * @brief GPU-accelerated drop-in replacement for QQuickPaintedItem.
 *
 * Subclasses override paint(QPainter*) exactly as they would with
 * QQuickPaintedItem. The image-bridge renderer takes care of routing
 * the QPainter output through QCanvasPainter for GPU compositing.
 */
class QuickPaintedItemCompat : public QCanvasPainterItem {
  Q_OBJECT

public:
  explicit QuickPaintedItemCompat(QQuickItem* parent = nullptr);

  /**
   * @brief Subclasses implement this to paint with QPainter.
   *
   * Called from the render thread via the image-bridge renderer.
   * The painter targets an offscreen QImage whose size matches the item.
   */
  virtual void paint(QPainter* painter) = 0;

  // Stubs for QQuickPaintedItem API used by existing constructors
  void setMipmap(bool) {}

  void setOpaquePainting(bool) {}

  void setRenderTarget(int) {}

  // Schedule a repaint and mark the item as needing a CPU re-render
  void update()
  {
    m_needsRepaint = true;
    QCanvasPainterItem::update();
  }

  void update(const QRect&) { update(); }

protected:
  [[nodiscard]] QCanvasPainterItemRenderer* createItemRenderer() const override;

private:
  friend class QuickPaintedItemCompatRenderer;
  bool m_needsRepaint = true;
};

/**
 * @class QuickPaintedItemCompatRenderer
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
 * @brief On older Qt versions, QuickPaintedItemCompat is QQuickPaintedItem.
 */
using QuickPaintedItemCompat = QQuickPaintedItem;

#endif  // SS_HAS_CANVAS_PAINTER
