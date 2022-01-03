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

#include <Misc/ThemeManager.h>
#include <UI/DeclarativeWidget.h>

/**
 * Creates a subclass of @c QWidget that allows us to call the given protected/private
 * @a function and pass the given @a event as a parameter to the @a function.
 */
#define DW_EXEC_EVENT(pointer, function, event)                                          \
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

/**
 * Configures item flags, sets fill color and connects signals/slots to automatically
 * resize the contained widget to the QML item's size.
 */
UI::DeclarativeWidget::DeclarativeWidget(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setMipmap(true);
    setAntialiasing(true);
    setOpaquePainting(true);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents, true);
    setFlag(ItemIsFocusScope, true);
    setFlag(ItemAcceptsInputMethod, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFillColor(Misc::ThemeManager::instance().base());

    // clang-format off
    connect(this, &QQuickPaintedItem::widthChanged,
            this, &UI::DeclarativeWidget::resizeWidget);
    connect(this, &QQuickPaintedItem::heightChanged,
            this, &UI::DeclarativeWidget::resizeWidget);
    connect(this, &UI::DeclarativeWidget::widgetChanged, [=](){update();});
    // clang-format on
}

/**
 * Returns a pointer to the contained widget
 */
QWidget *UI::DeclarativeWidget::widget()
{
    return m_widget;
}

/**
 * Grabs an image/pixmap of the contained widget. The pixmap is later
 * used to render the widget in the QML interface without causing signal/slot
 * interferences with the scenegraph render thread.
 */
void UI::DeclarativeWidget::update(const QRect &rect)
{
    if (widget())
    {
        m_pixmap = m_widget->grab();
        QQuickPaintedItem::update(rect);
    }
}

/**
 * Displays the pixmap generated in the @c update() function in the QML
 * interface through the given @a painter pointer.
 */
void UI::DeclarativeWidget::paint(QPainter *painter)
{
    if (painter)
        painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::keyPressEvent(QKeyEvent *event)
{
    DW_EXEC_EVENT(m_widget, keyPressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::keyReleaseEvent(QKeyEvent *event)
{
    DW_EXEC_EVENT(m_widget, keyReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::inputMethodEvent(QInputMethodEvent *event)
{
    DW_EXEC_EVENT(m_widget, inputMethodEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::focusInEvent(QFocusEvent *event)
{
    DW_EXEC_EVENT(m_widget, focusInEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::focusOutEvent(QFocusEvent *event)
{
    DW_EXEC_EVENT(m_widget, focusOutEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::mousePressEvent(QMouseEvent *event)
{
    DW_EXEC_EVENT(m_widget, mousePressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::mouseMoveEvent(QMouseEvent *event)
{
    DW_EXEC_EVENT(m_widget, mouseMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    DW_EXEC_EVENT(m_widget, mouseReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    DW_EXEC_EVENT(m_widget, mouseDoubleClickEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::wheelEvent(QWheelEvent *event)
{
    DW_EXEC_EVENT(m_widget, wheelEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    DW_EXEC_EVENT(m_widget, dragEnterEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    DW_EXEC_EVENT(m_widget, dragMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    DW_EXEC_EVENT(m_widget, dragLeaveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void UI::DeclarativeWidget::dropEvent(QDropEvent *event)
{
    DW_EXEC_EVENT(m_widget, dropEvent, event);
}

/**
 * Resizes the widget to fit inside the QML painted item.
 */
void UI::DeclarativeWidget::resizeWidget()
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

/**
 * Changes the @param widget to be rendered in the QML interface.
 */
void UI::DeclarativeWidget::setWidget(QWidget *widget)
{
    if (widget)
    {
        if (m_widget)
            delete m_widget;

        m_widget = widget;
        Q_EMIT widgetChanged();
    }
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_DeclarativeWidget.cpp"
#endif
