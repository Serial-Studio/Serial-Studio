/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "DeclarativeWidget.h"

namespace UI
{
#define EXEC_EVENT(pointer, function, event)                                             \
    if (!pointer.isNull())                                                               \
    {                                                                                    \
        class PwnedWidget : public QWidget                                               \
        {                                                                                \
        public:                                                                          \
            using QWidget::function;                                                     \
        };                                                                               \
        static_cast<PwnedWidget *>(pointer.data())->function(event);                     \
        update();                                                                        \
    }

DeclarativeWidget::DeclarativeWidget(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setMipmap(true);
    setAntialiasing(true);
    setOpaquePainting(true);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents, true);
    setFlag(ItemIsFocusScope, true);
    setFillColor(QColor(255, 0, 255));
    setFlag(ItemAcceptsInputMethod, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // clang-format off
    connect(this, &QQuickPaintedItem::widthChanged,
            this, &UI::DeclarativeWidget::resizeWidget);
    connect(this, &QQuickPaintedItem::heightChanged,
            this, &UI::DeclarativeWidget::resizeWidget);
    connect(this, &UI::DeclarativeWidget::widgetChanged, [=](){update();});
    // clang-format on
}

QWidget *DeclarativeWidget::widget()
{
    return m_widget;
}

void DeclarativeWidget::update(const QRect &rect)
{
    if (widget())
    {
        m_pixmap = m_widget->grab();
        QQuickPaintedItem::update(rect);
    }
}

void DeclarativeWidget::paint(QPainter *painter)
{
    if (painter)
        painter->drawPixmap(0, 0, m_pixmap);
}

void DeclarativeWidget::keyPressEvent(QKeyEvent *event)
{
    EXEC_EVENT(m_widget, keyPressEvent, event);
}

void DeclarativeWidget::keyReleaseEvent(QKeyEvent *event)
{
    EXEC_EVENT(m_widget, keyReleaseEvent, event);
}

void DeclarativeWidget::inputMethodEvent(QInputMethodEvent *event)
{
    EXEC_EVENT(m_widget, inputMethodEvent, event);
}

void DeclarativeWidget::focusInEvent(QFocusEvent *event)
{
    EXEC_EVENT(m_widget, focusInEvent, event);
}

void DeclarativeWidget::focusOutEvent(QFocusEvent *event)
{
    EXEC_EVENT(m_widget, focusOutEvent, event);
}

void DeclarativeWidget::mousePressEvent(QMouseEvent *event)
{
    EXEC_EVENT(m_widget, mousePressEvent, event);
}

void DeclarativeWidget::mouseMoveEvent(QMouseEvent *event)
{
    EXEC_EVENT(m_widget, mouseMoveEvent, event);
}

void DeclarativeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    EXEC_EVENT(m_widget, mouseReleaseEvent, event);
}

void DeclarativeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    EXEC_EVENT(m_widget, mouseDoubleClickEvent, event);
}

void DeclarativeWidget::wheelEvent(QWheelEvent *event)
{
    EXEC_EVENT(m_widget, wheelEvent, event);
}

void DeclarativeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    EXEC_EVENT(m_widget, dragEnterEvent, event);
}

void DeclarativeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    EXEC_EVENT(m_widget, dragMoveEvent, event);
}

void DeclarativeWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    EXEC_EVENT(m_widget, dragLeaveEvent, event);
}

void DeclarativeWidget::dropEvent(QDropEvent *event)
{
    EXEC_EVENT(m_widget, dropEvent, event);
}

void DeclarativeWidget::resizeWidget()
{
    if (widget())
    {
        if (width() > 0 && height() > 0)
        {
            widget()->setFixedSize(width(), height());
            update();
        }
    }
}

void DeclarativeWidget::setWidget(QWidget *widget)
{
    if (widget)
    {
        if (m_widget)
            delete m_widget;

        m_widget = widget;
        Q_EMIT widgetChanged();
    }
}
}
