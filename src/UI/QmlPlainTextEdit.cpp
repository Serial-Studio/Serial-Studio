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

#include "QmlPlainTextEdit.h"

using namespace UI;

/*
 * NOTE: most of the Doxygen documentation comments where heavily based from the following
 *       URL https://doc.qt.io/qt-5/qplaintextedit.html. In some cases the comments are a
 *       simple copy-paste job. I am lazy and the Qt documentation is very good IMO.
 *
 *       This class works by initializing a QPlainTextEdit widget, rendering it into the
 *       the painter of a QML item and handling keyboard/mouse events.
 *
 *       The rest of the functions are just a wrapper around the functions of the
 *       QPlainTextEdit widget for increased QML-friendliness.
 */

/**
 * Constructor function
 */
QmlPlainTextEdit::QmlPlainTextEdit(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_copyAvailable(false)
{
    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Initialize the text edit widget
    m_textEdit = new QPlainTextEdit();
    m_textEdit->installEventFilter(this);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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

    // React to widget events
    connect(m_textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(setCopyAvailable(bool)));
}

/**
 * Destructor function
 */
QmlPlainTextEdit::~QmlPlainTextEdit()
{
    m_textEdit->deleteLater();
}

/**
 * Handle application events manually
 */
bool QmlPlainTextEdit::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FocusIn:
            forceActiveFocus();
            return QQuickPaintedItem::event(event);
            break;
        case QEvent::Wheel:
            processWheelEvents(static_cast<QWheelEvent *>(event));
            return true;
            break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
            processMouseEvents(static_cast<QMouseEvent *>(event));
            return true;
            break;
        default:
            break;
    }

    return QApplication::sendEvent(m_textEdit, event);
}

/**
 * Render the text edit on the given @a painter
 */
void QmlPlainTextEdit::paint(QPainter *painter)
{
    if (m_textEdit && painter)
        m_textEdit->render(painter);
}

/**
 * Custom event filter to manage redraw requests
 */
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

/**
 * Returns the font used by the QPlainTextEdit widget
 */
QFont QmlPlainTextEdit::font() const
{
    return m_textEdit->font();
}

/**
 * Returns the text color used by the QPlainTextEdit widget
 */
QColor QmlPlainTextEdit::color() const
{
    return m_color;
}

/**
 * Returns the plain text of the QPlainTextEdit widget
 */
QString QmlPlainTextEdit::text() const
{
    return m_textEdit->toPlainText();
}

/**
 * Returns @c true if the widget is set to read-only
 */
bool QmlPlainTextEdit::readOnly() const
{
    return m_textEdit->isReadOnly();
}

/**
 * Returns @c true if the widget shall scroll automatically to the bottom when new
 * text is appended to the widget.
 */
bool QmlPlainTextEdit::autoscroll() const
{
    return m_autoscroll;
}

/**
 * Returns the palette used by the QPlainTextEdit widget
 */
QPalette QmlPlainTextEdit::palette() const
{
    return m_textEdit->palette();
}

/**
 * Returns the wrap mode of the QPlainTextEdit casted to an integer type (so that it
 * can be used from the QML interface).
 */
int QmlPlainTextEdit::wordWrapMode() const
{
    return static_cast<int>(m_textEdit->wordWrapMode());
}

/**
 * Returns @c true if the user is able to copy any text from the document. This value is
 * updated through the copyAvailable() signal sent by the QPlainTextEdit.
 */
bool QmlPlainTextEdit::copyAvailable() const
{
    return m_copyAvailable;
}

/**
 * Returns @c true if the QPlainTextEdit widget is enabled
 */
bool QmlPlainTextEdit::widgetEnabled() const
{
    return m_textEdit->isEnabled();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
bool QmlPlainTextEdit::centerOnScroll() const
{
    return m_textEdit->centerOnScroll();
}

/**
 * This property holds whether undo and redo are enabled.
 * Users are only able to undo or redo actions if this property is true, and if there is
 * an action that can be undone (or redone).
 */
bool QmlPlainTextEdit::undoRedoEnabled() const
{
    return m_textEdit->isUndoRedoEnabled();
}

/**
 * This property holds the limit for blocks in the document.
 *
 * Specifies the maximum number of blocks the document may have. If there are more blocks
 * in the document that specified with this property blocks are removed from the beginning
 * of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited amount
 * of blocks.
 */
int QmlPlainTextEdit::maximumBlockCount() const
{
    return m_textEdit->maximumBlockCount();
}

/**
 * This property holds the editor placeholder text.
 *
 * Setting this property makes the editor display a grayed-out placeholder text as long as
 * the document is empty.
 */
QString QmlPlainTextEdit::placeholderText() const
{
    return m_textEdit->placeholderText();
}

/**
 * Copies any selected text to the clipboard.
 */
void QmlPlainTextEdit::copy()
{
    m_textEdit->copy();
}

/**
 * Deletes all the text in the text edit.
 */
void QmlPlainTextEdit::clear()
{
    m_textEdit->clear();
    update();

    emit textChanged();
}

/**
 * Selects all the text of the text edit.
 */
void QmlPlainTextEdit::selectAll()
{
    m_textEdit->selectAll();
    update();
}

/**
 * Clears the text selection
 */
void QmlPlainTextEdit::clearSelection()
{
    auto cursor = QTextCursor(m_textEdit->document());
    cursor.clearSelection();
    m_textEdit->setTextCursor(cursor);
    update();
}

/**
 * Changes the scrollbar size to fit the document window
 */
void QmlPlainTextEdit::resetScrollbarSize()
{
    auto *bar = m_textEdit->verticalScrollBar();

    auto textHeight = height() / m_textEdit->fontMetrics().height();
    auto scrollIndex = bar->maximum() - textHeight;
    if (scrollIndex < 0)
        scrollIndex = 0;

    bar->setMaximum(scrollIndex);
    bar->setValue(scrollIndex);
    update();
}

/**
 * Changes the read-only state of the text edit.
 *
 * In a read-only text edit the user can only navigate through the text and select text;
 * modifying the text is not possible.
 */
void QmlPlainTextEdit::setReadOnly(const bool ro)
{
    m_textEdit->setReadOnly(ro);
    update();

    emit readOnlyChanged();
}

/**
 * Changes the font used to display the text of the text edit.
 */
void QmlPlainTextEdit::setFont(const QFont &font)
{
    m_textEdit->setFont(font);
    update();

    emit fontChanged();
}

/**
 * Appends a new paragraph with text to the end of the text edit.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void QmlPlainTextEdit::append(const QString &text)
{
    m_textEdit->appendPlainText(text);

    if (autoscroll())
        scrollToBottom(false);

    update();
    emit textChanged();
}

/**
 * Replaces the text of the text editor with @c text.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void QmlPlainTextEdit::setText(const QString &text)
{
    m_textEdit->setPlainText(text);

    if (autoscroll())
        scrollToBottom(false);

    update();
    emit textChanged();
}

/**
 * Changes the text color of the text editor.
 */
void QmlPlainTextEdit::setColor(const QColor &color)
{
    m_color = color;
    auto qss = QString("QPlainTextEdit{color: %1;}").arg(color.name());
    m_textEdit->setStyleSheet(qss);
    update();

    emit colorChanged();
}

/**
 * Changes the @c QPalette of the text editor widget and its children.
 */
void QmlPlainTextEdit::setPalette(const QPalette &palette)
{
    m_textEdit->setPalette(palette);
    update();

    emit paletteChanged();
}

/**
 * Enables or disables the text editor widget.
 */
void QmlPlainTextEdit::setWidgetEnabled(const bool enabled)
{
    m_textEdit->setEnabled(enabled);
    update();

    emit widgetEnabledChanged();
}

/**
 * Enables/disable automatic scrolling. If automatic scrolling is enabled, then the
 * vertical scrollbar shall automatically scroll to the end of the document when the
 * text of the text editor is changed.
 */
void QmlPlainTextEdit::setAutoscroll(const bool enabled)
{
    m_autoscroll = enabled;
    emit autoscrollChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void QmlPlainTextEdit::insertText(const QString &text)
{
    // Move the cursor to the end of the document
    auto cursor = QTextCursor(m_textEdit->document());
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);

    // Insert text at the end of the document
    m_textEdit->insertPlainText(text);

    // Auto-scroll if needed
    if (autoscroll())
        scrollToBottom(false);

    // Redraw UI
    update();
    emit textChanged();
}

/**
 * Changes the word wrap mode of the text editor.
 *
 * This property holds the mode QPlainTextEdit will use when wrapping text by words.
 */
void QmlPlainTextEdit::setWordWrapMode(const int mode)
{
    m_textEdit->setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
    update();

    emit wordWrapModeChanged();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
void QmlPlainTextEdit::setCenterOnScroll(const bool enabled)
{
    m_textEdit->setCenterOnScroll(enabled);
    update();

    emit centerOnScrollChanged();
}

/**
 * Enables/disables undo/redo history support.
 */
void QmlPlainTextEdit::setUndoRedoEnabled(const bool enabled)
{
    m_textEdit->setUndoRedoEnabled(enabled);
    update();

    emit undoRedoEnabledChanged();
}

/**
 * Changes the placeholder text of the text editor. The placeholder text is only displayed
 * when the document is empty.
 */
void QmlPlainTextEdit::setPlaceholderText(const QString &text)
{
    m_textEdit->setPlaceholderText(text);
    update();

    emit placeholderTextChanged();
}

/**
 * Moves the position of the vertical scrollbar to the end of the document. However, this
 * function also ensures that the last line of the document is shown at the bottom of
 * the widget to mimic a terminal.
 */
void QmlPlainTextEdit::scrollToBottom(const bool repaint)
{
    auto *bar = m_textEdit->verticalScrollBar();

    auto textHeight = height() / m_textEdit->fontMetrics().height();
    auto scrollIndex = bar->minimum() + bar->maximum() - textHeight;

    if (scrollIndex >= 0)
    {
        bar->setMaximum(scrollIndex);
        bar->setValue(scrollIndex);
    }

    if (repaint)
        update();
}

/**
 * Specifies the maximum number of blocks the document may have. If there are more blocks
 * in the document that specified with this property blocks are removed from the beginning
 * of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited amount of
 * blocks.
 */
void QmlPlainTextEdit::setMaximumBlockCount(const int maxBlockCount)
{
    m_textEdit->setMaximumBlockCount(maxBlockCount);
    update();

    emit maximumBlockCountChanged();
}

/**
 * Resizes the text editor widget to fit inside the QML item.
 */
void QmlPlainTextEdit::updateWidgetSize()
{
    m_textEdit->setGeometry(0, 0, static_cast<int>(width()), static_cast<int>(height()));
    update();
}

/**
 * Updates the value of copy-available. This function is automatically called by the text
 * editor widget when the user makes any text selection/deselection.
 */
void QmlPlainTextEdit::setCopyAvailable(const bool yes)
{
    m_copyAvailable = yes;
    emit copyAvailableChanged();
}

/**
 * Hack: call the appropiate protected mouse event handler function of the QPlainTextEdit
 *       item depending on event type
 */
void QmlPlainTextEdit::processMouseEvents(QMouseEvent *event)
{
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::mouseDoubleClickEvent;
        using QPlainTextEdit::mouseMoveEvent;
        using QPlainTextEdit::mousePressEvent;
        using QPlainTextEdit::mouseReleaseEvent;
    };

    auto hack = static_cast<Hack *>(m_textEdit);
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
            hack->mousePressEvent(event);
            break;
        case QEvent::MouseMove:
            hack->mouseMoveEvent(event);
            break;
        case QEvent::MouseButtonRelease:
            hack->mouseReleaseEvent(event);
            break;
        case QEvent::MouseButtonDblClick:
            hack->mouseDoubleClickEvent(event);
            break;
        default:
            break;
    }
}

/**
 * Hack: call the protected wheel event handler function of the QPlainTextEdit item
 */
void QmlPlainTextEdit::processWheelEvents(QWheelEvent *event)
{
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::wheelEvent;
    };

    static_cast<Hack *>(m_textEdit)->wheelEvent(event);
}
