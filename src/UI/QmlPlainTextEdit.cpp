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

#include <QScrollBar>
#include <QApplication>
#include <IO/Console.h>

#include "QmlPlainTextEdit.h"

using namespace UI;

QmlPlainTextEdit::QmlPlainTextEdit(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Initialize the text edit widget
    m_textEdit = new QPlainTextEdit();
    m_textEdit->installEventFilter(this);

    // Set focus to the text edit
    m_textEdit->setContextMenuPolicy(Qt::DefaultContextMenu);

    // Set the QML item's implicit size
    auto hint = m_textEdit->sizeHint();
    setImplicitSize(hint.width(), hint.height());

    // Get default text color
    m_color = qApp->palette().color(QPalette::Text);

    // Resize QPlainTextEdit to fit QML item
    connect(this, &QQuickPaintedItem::widthChanged, this,
            &QmlPlainTextEdit::updateWidgetSize);
    connect(this, &QQuickPaintedItem::heightChanged, this,
            &QmlPlainTextEdit::updateWidgetSize);
}

QmlPlainTextEdit::~QmlPlainTextEdit()
{
    m_textEdit->deleteLater();
}

bool QmlPlainTextEdit::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FocusIn:
            forceActiveFocus();
            return QQuickPaintedItem::event(event);
            break;
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove:
            routeMouseEvents(static_cast<QMouseEvent *>(event));
            return true;
            break;
        case QEvent::Wheel:
            routeWheelEvents(static_cast<QWheelEvent *>(event));
            return true;
            break;
        default:
            break;
    }

    return QApplication::sendEvent(m_textEdit, event);
}

void QmlPlainTextEdit::paint(QPainter *painter)
{
    if (m_textEdit && painter)
        m_textEdit->render(painter);
}

bool QmlPlainTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(m_textEdit);

    if (watched == m_textEdit)
    {
        switch (event->type())
        {
            case QEvent::Paint:
            case QEvent::UpdateRequest:
                update();
                break;
            default:
                break;
        }
    }

    return QQuickPaintedItem::eventFilter(watched, event);
}

QFont QmlPlainTextEdit::font() const
{
    return m_textEdit->font();
}

QColor QmlPlainTextEdit::color() const
{
    return m_color;
}

QString QmlPlainTextEdit::text() const
{
    return m_textEdit->toPlainText();
}

bool QmlPlainTextEdit::readOnly() const
{
    return m_textEdit->isReadOnly();
}

bool QmlPlainTextEdit::autoscroll() const
{
    return m_autoscroll;
}

QPalette QmlPlainTextEdit::palette() const
{
    return m_textEdit->palette();
}

int QmlPlainTextEdit::wordWrapMode() const
{
    return static_cast<int>(m_textEdit->wordWrapMode());
}

bool QmlPlainTextEdit::widgetEnabled() const
{
    return m_textEdit->isEnabled();
}

bool QmlPlainTextEdit::centerOnScroll() const
{
    return m_textEdit->centerOnScroll();
}

bool QmlPlainTextEdit::undoRedoEnabled() const
{
    return m_textEdit->isUndoRedoEnabled();
}

int QmlPlainTextEdit::maximumBlockCount() const
{
    return m_textEdit->maximumBlockCount();
}

QString QmlPlainTextEdit::placeholderText() const
{
    return m_textEdit->placeholderText();
}

bool QmlPlainTextEdit::copyAvailable() const
{
    return text().length() > 0;
}

void QmlPlainTextEdit::routeMouseEvents(QMouseEvent *event)
{
    QMouseEvent *newEvent
        = new QMouseEvent(event->type(), event->localPos(), event->button(),
                          event->buttons(), event->modifiers());
    m_textEdit->setFocus(Qt::ActiveWindowFocusReason);
    QApplication::postEvent(m_textEdit, newEvent);
}

void QmlPlainTextEdit::routeWheelEvents(QWheelEvent *event)
{
    QWheelEvent *newEvent
        = new QWheelEvent(event->position(), event->delta(), event->buttons(),
                          event->modifiers(), event->orientation());
    QApplication::postEvent(m_textEdit, newEvent);
}

void QmlPlainTextEdit::copy()
{
    m_textEdit->copy();
}

void QmlPlainTextEdit::clear()
{
    m_textEdit->clear();
    update();

    emit textChanged();
}

void QmlPlainTextEdit::selectAll()
{
    m_textEdit->selectAll();
    update();

    emit textChanged();
}

void QmlPlainTextEdit::setReadOnly(const bool ro)
{
    m_textEdit->setReadOnly(ro);
    update();

    emit readOnlyChanged();
}

void QmlPlainTextEdit::setFont(const QFont &font)
{
    m_textEdit->setFont(font);
    update();

    emit fontChanged();
}

void QmlPlainTextEdit::append(const QString &text)
{
    m_textEdit->appendPlainText(text);

    if (autoscroll())
    {
        auto *bar = m_textEdit->verticalScrollBar();
        bar->setValue(bar->maximum());
    }

    update();
    emit textChanged();
}

void QmlPlainTextEdit::setText(const QString &text)
{
    m_textEdit->setPlainText(text);

    if (autoscroll())
    {
        auto *bar = m_textEdit->verticalScrollBar();
        bar->setValue(bar->maximum());
    }

    update();
    emit textChanged();
}

void QmlPlainTextEdit::setColor(const QColor &color)
{
    m_color = color;
    auto qss = QString("QPlainTextEdit{color: %1;}").arg(color.name());
    m_textEdit->setStyleSheet(qss);
    update();

    emit colorChanged();
}

void QmlPlainTextEdit::setPalette(const QPalette &palette)
{
    m_textEdit->setPalette(palette);
    update();

    emit paletteChanged();
}

void QmlPlainTextEdit::setWidgetEnabled(const bool enabled)
{
    m_textEdit->setEnabled(enabled);
    update();

    emit widgetEnabledChanged();
}

void QmlPlainTextEdit::setAutoscroll(const bool enabled)
{
    m_autoscroll = enabled;
    emit autoscrollChanged();
}

void QmlPlainTextEdit::setWordWrapMode(const int mode)
{
    m_textEdit->setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
    update();

    emit wordWrapModeChanged();
}

void QmlPlainTextEdit::setCenterOnScroll(const bool enabled)
{
    m_textEdit->setCenterOnScroll(enabled);
    update();

    emit centerOnScrollChanged();
}

void QmlPlainTextEdit::setUndoRedoEnabled(const bool enabled)
{
    m_textEdit->setUndoRedoEnabled(enabled);
    update();

    emit undoRedoEnabledChanged();
}

void QmlPlainTextEdit::setPlaceholderText(const QString &text)
{
    m_textEdit->setPlaceholderText(text);
    update();

    emit placeholderTextChanged();
}

void QmlPlainTextEdit::setMaximumBlockCount(const int maxBlockCount)
{
    m_textEdit->setMaximumBlockCount(maxBlockCount);
    update();

    emit maximumBlockCountChanged();
}

void QmlPlainTextEdit::updateWidgetSize()
{
    m_textEdit->setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    update();
}
