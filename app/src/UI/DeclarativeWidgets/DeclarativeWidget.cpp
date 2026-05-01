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

#include "DeclarativeWidget.h"

#include <QTableView>

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures item flags and wires geometry/visibility signals so the embedded QWidget tracks
 * the QML item.
 */
DeclarativeWidget::DeclarativeWidget(QQuickItem* parent)
  : QuickPaintedItemCompat(parent)
  , m_widget(nullptr)
  , m_contentWidth(0)
  , m_contentHeight(0)
  , m_updateRequested(true)
{
  setMipmap(false);
  setOpaquePainting(true);
  setAcceptTouchEvents(false);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, false);
  setFlag(ItemAcceptsInputMethod, false);
  setAcceptedMouseButtons(Qt::NoButton);
  requestUpdate();
  redraw();

  // Resize and redraw on geometry/visibility changes
  connect(this, &QuickPaintedItemCompat::widthChanged, this, &DeclarativeWidget::resizeWidget);
  connect(this, &QuickPaintedItemCompat::heightChanged, this, &DeclarativeWidget::resizeWidget);
  connect(this, &DeclarativeWidget::visibleChanged, [=, this]() {
    requestUpdate();
    redraw();
  });
  connect(this, &DeclarativeWidget::widgetChanged, [=, this]() {
    requestUpdate();
    redraw();
  });
}

//--------------------------------------------------------------------------------------------------
// Widget & palette access
//--------------------------------------------------------------------------------------------------

/**
 * Returns a pointer to the contained widget
 */
QWidget* DeclarativeWidget::widget() const
{
  return m_widget;
}

/**
 * @brief Returns the palette currently used by the embedded widget, or an empty palette when
 * absent.
 */
QPalette DeclarativeWidget::palette() const
{
  if (m_widget)
    return m_widget->palette();

  return {};
}

//--------------------------------------------------------------------------------------------------
// Content sizing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the sub-class widget's content-width hint used by the QML scroller.
 */
int DeclarativeWidget::contentWidth() const
{
  return m_contentWidth;
}

/**
 * @brief Returns the sub-class widget's content-height hint used by the QML scroller.
 */
int DeclarativeWidget::contentHeight() const
{
  return m_contentHeight;
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Grabs the embedded widget into an image so the scenegraph can render it without thread
 * crosstalk.
 */
void DeclarativeWidget::redraw(const QRect& rect)
{
  if (widget() != nullptr) {
    if (auto* tableView = qobject_cast<QTableView*>(widget()))
      tableView->doItemsLayout();

    if (isVisible() && m_updateRequested) {
      m_updateRequested = false;
      m_image           = m_widget->grab().toImage();
      Q_UNUSED(rect)
      update();
    }
  }
}

/**
 * @brief Paints the cached widget image onto the QML scenegraph through the supplied painter.
 */
void DeclarativeWidget::paint(QPainter* painter)
{
  if (painter != nullptr)
    painter->drawImage(0, 0, m_image);
}

//--------------------------------------------------------------------------------------------------
// Layout management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resizes the embedded widget to match the current QML painted-item geometry.
 */
void DeclarativeWidget::resizeWidget()
{
  if (widget() != nullptr) {
    if (width() > 0 && height() > 0) {
      widget()->setFixedSize(width(), height());

      requestUpdate();
      redraw();
    }
  }
}

/**
 * @brief Instructs the widget to redraw the pixmap on the next event loop
 */
void DeclarativeWidget::requestUpdate()
{
  m_updateRequested = true;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the embedded QWidget rendered into the QML item, ignoring null or unchanged values.
 */
void DeclarativeWidget::setWidget(QWidget* widget)
{
  if (widget == nullptr || m_widget == widget)
    return;

  m_widget = widget;
  Q_EMIT widgetChanged();
}

/**
 * @brief Updates the content-width hint reported to QML and signals geometryChanged on change.
 */
void DeclarativeWidget::setContentWidth(const int width)
{
  if (m_contentWidth == width)
    return;

  m_contentWidth = width;
  Q_EMIT geometryChanged();
}

/**
 * @brief Updates the content-height hint reported to QML and signals geometryChanged on change.
 */
void DeclarativeWidget::setContentHeight(const int height)
{
  if (m_contentHeight == height)
    return;

  m_contentHeight = height;
  Q_EMIT geometryChanged();
}

/**
 * @brief Applies the given palette to the embedded widget and triggers a redraw.
 */
void DeclarativeWidget::setPalette(const QPalette& palette)
{
  if (m_widget != nullptr) {
    m_widget->setPalette(palette);
    requestUpdate();
    redraw();

    Q_EMIT paletteChanged();
  }
}
