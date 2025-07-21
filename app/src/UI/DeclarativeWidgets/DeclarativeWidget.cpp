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

/**
 * Configures item flags, sets fill color and connects signals/slots to
 * automatically resize the contained widget to the QML item's size.
 */
DeclarativeWidget::DeclarativeWidget(QQuickItem *parent)
  : QQuickPaintedItem(parent)
  , m_widget(nullptr)
  , m_contentWidth(0)
  , m_contentHeight(0)
  , m_updateRequested(true)
{
  // Set QML flags
  setMipmap(false);
  setOpaquePainting(true);
  setAcceptTouchEvents(false);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, false);
  setFlag(ItemAcceptsInputMethod, false);
  setAcceptedMouseButtons(Qt::NoButton);

  // Paint the widget
  requestUpdate();
  redraw();

  // Resize widget when the item's size is changed
  connect(this, &QQuickPaintedItem::widthChanged, this,
          &DeclarativeWidget::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this,
          &DeclarativeWidget::resizeWidget);

  // Redraw widget when visibility changes
  connect(this, &DeclarativeWidget::visibleChanged, [=, this]() {
    requestUpdate();
    redraw();
  });

  // Redraw widget when widget changes
  connect(this, &DeclarativeWidget::widgetChanged, [=, this]() {
    requestUpdate();
    redraw();
  });
}

/**
 * Returns a pointer to the contained widget
 */
QWidget *DeclarativeWidget::widget() const
{
  return m_widget;
}

/**
 * Returns the palette currently used by the widget.
 */
QPalette DeclarativeWidget::palette() const
{
  if (m_widget)
    return m_widget->palette();

  return {};
}

/**
 * Allows the sub-class widget to define its own content width hint for the
 * QML user interface. This is useful to add scrolling support using native
 * QML elements.
 */
int DeclarativeWidget::contentWidth() const
{
  return m_contentWidth;
}

/**
 * Allows the sub-class widget to define its own content height hint for the
 * QML user interface. This is useful to add scrolling support using native
 * QML elements.
 */
int DeclarativeWidget::contentHeight() const
{
  return m_contentHeight;
}

/**
 * Grabs an image/pixmap of the contained widget. The pixmap is later
 * used to render the widget in the QML interface without causing signal/slot
 * interferences with the scenegraph render thread.
 */
void DeclarativeWidget::redraw(const QRect &rect)
{
  if (widget() != nullptr)
  {
    if (qobject_cast<QTableView *>(widget()) != nullptr)
    {
      class PwnedWidget : public QTableView
      {
      public:
        using QTableView::updateGeometries;
      };
      reinterpret_cast<PwnedWidget *>(m_widget)->updateGeometries();
    }

    if (isVisible() && m_updateRequested)
    {
      m_updateRequested = false;
      m_image = m_widget->grab().toImage();
      QQuickPaintedItem::update(rect);
    }
  }
}

/**
 * Displays the pixmap generated in the @c update() function in the QML
 * interface through the given @a painter pointer.
 */
void DeclarativeWidget::paint(QPainter *painter)
{
  if (painter != nullptr)
    painter->drawImage(0, 0, m_image);
}

/**
 * Resizes the widget to fit inside the QML painted item.
 */
void DeclarativeWidget::resizeWidget()
{
  if (widget() != nullptr)
  {
    if (width() > 0 && height() > 0)
    {
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

/**
 * Changes the @param widget to be rendered in the QML interface.
 */
void DeclarativeWidget::setWidget(QWidget *widget)
{
  if (widget != nullptr)
  {
    m_widget = widget;
    Q_EMIT widgetChanged();
  }
}

/**
 * Allows the sub-class widget to define its own content width hint for the
 * QML user interface. This is useful to add scrolling support using native
 * QML elements.
 */
void DeclarativeWidget::setContentWidth(const int width)
{
  m_contentWidth = width;
  Q_EMIT geometryChanged();
}

/**
 * Allows the sub-class widget to define its own content height hint for the
 * QML user interface. This is useful to add scrolling support using native
 * QML elements.
 */
void DeclarativeWidget::setContentHeight(const int height)
{
  m_contentHeight = height;
  Q_EMIT geometryChanged();
}

/**
 * Modifies the palette used by the widget.
 */
void DeclarativeWidget::setPalette(const QPalette &palette)
{
  if (m_widget != nullptr)
  {
    m_widget->setPalette(palette);
    requestUpdate();

    Q_EMIT paletteChanged();
  }
}
