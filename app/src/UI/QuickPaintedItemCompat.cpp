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

#include "UI/QuickPaintedItemCompat.h"

#if SS_HAS_CANVAS_PAINTER

#include <QQuickWindow>

//--------------------------------------------------------------------------------------------------
// QuickPaintedItemCompat
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the compatibility item with GPU-accelerated compositing.
 */
QuickPaintedItemCompat::QuickPaintedItemCompat(QQuickItem *parent)
  : QCanvasPainterItem(parent)
{
  setFlag(ItemHasContents, true);
  setFillColor(Qt::transparent);
}

/**
 * @brief Creates the image-bridge renderer.
 */
QCanvasPainterItemRenderer *QuickPaintedItemCompat::createItemRenderer() const
{
  return new QuickPaintedItemCompatRenderer;
}

//--------------------------------------------------------------------------------------------------
// QuickPaintedItemCompatRenderer
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the renderer.
 */
QuickPaintedItemCompatRenderer::QuickPaintedItemCompatRenderer()
  : m_item(nullptr)
{
}

/**
 * @brief Copies the item pointer for use during paint().
 *
 * This is called on the render thread while the GUI thread is blocked, so
 * it is safe to read item state and call paint(QPainter*).
 */
void QuickPaintedItemCompatRenderer::synchronize(QCanvasPainterItem *item)
{
  m_item = static_cast<QuickPaintedItemCompat *>(item);

  // Render at physical pixel resolution for crisp HiDPI output
  const qreal dpr = m_item->window() ? m_item->window()->devicePixelRatio() : 1.0;
  const QSize needed(static_cast<int>(width() * dpr),
                     static_cast<int>(height() * dpr));
  if (needed.isEmpty())
    return;

  // Resize the CPU buffer if the item size changed
  if (m_buffer.size() != needed)
  {
    m_buffer = QImage(needed, QImage::Format_ARGB32_Premultiplied);
    m_lastSize = QSize();
  }

  // Set DPR so QPainter works in logical coordinates while rendering at
  // physical resolution — text and lines are crisp on Retina displays
  m_buffer.setDevicePixelRatio(dpr);
  m_buffer.fill(Qt::transparent);
  QPainter p(&m_buffer);
  p.setRenderHint(QPainter::Antialiasing);
  m_item->paint(&p);
  p.end();
}

/**
 * @brief Uploads the CPU buffer as a QCanvasImage on first use or size change.
 */
void QuickPaintedItemCompatRenderer::initializeResources(QCanvasPainter *painter)
{
  Q_UNUSED(painter)
}

/**
 * @brief Draws the offscreen image onto the canvas painter.
 */
void QuickPaintedItemCompatRenderer::paint(QCanvasPainter *painter)
{
  if (m_buffer.isNull())
    return;

  const QSize current = m_buffer.size();

  // Re-upload the image every frame (the buffer changes each synchronize)
  if (!m_canvasImage.isNull())
    painter->removeImage(m_canvasImage);

  m_canvasImage = painter->addImage(m_buffer);
  m_lastSize = current;

  // Draw the high-res buffer into the logical-size rect so QCanvasPainter
  // downsamples it crisply on HiDPI instead of upscaling a blurry image
  painter->drawImage(m_canvasImage, QRectF(0, 0, width(), height()));
}

#endif // SS_HAS_CANVAS_PAINTER
