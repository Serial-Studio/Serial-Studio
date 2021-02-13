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

#include <QtMath>
#include <QScrollBar>
#include <QApplication>
#include <IO/Console.h>

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
    , m_textEdit(new QPlainTextEdit)
{
    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Configure text edit widget
    setScrollbarWidth(14);
    textEdit()->installEventFilter(this);
    textEdit()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Set the QML item's implicit size
    auto hint = textEdit()->sizeHint();
    setImplicitSize(hint.width(), hint.height());

    // Get default text color
    m_color = qApp->palette().color(QPalette::Text);

    // Resize QPlainTextEdit to fit QML item
    connect(this, &QQuickPaintedItem::widthChanged, this,
            &QmlPlainTextEdit::updateWidgetSize);
    connect(this, &QQuickPaintedItem::heightChanged, this,
            &QmlPlainTextEdit::updateWidgetSize);

    // Connect console signals (doing this on QML uses about 50% of UI thread time)
    auto console = IO::Console::getInstance();
    connect(console, &IO::Console::stringReceived, this, &QmlPlainTextEdit::insertText);

    // React to widget events
    connect(textEdit(), SIGNAL(copyAvailable(bool)), this, SLOT(setCopyAvailable(bool)));
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

    return QApplication::sendEvent(textEdit(), event);
}

/**
 * Render the text edit on the given @a painter
 */
void QmlPlainTextEdit::paint(QPainter *painter)
{
    if (m_textEdit && painter)
        textEdit()->render(painter);
}

/**
 * Custom event filter to manage redraw requests
 */
bool QmlPlainTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(m_textEdit);

    if (watched == textEdit())
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
    return textEdit()->font();
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
    return textEdit()->toPlainText();
}

/**
 * Returns @c true if the text document is empty
 */
bool QmlPlainTextEdit::empty() const
{
    return textEdit()->document()->isEmpty();
}

/**
 * Returns @c true if the widget is set to read-only
 */
bool QmlPlainTextEdit::readOnly() const
{
    return textEdit()->isReadOnly();
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
    return textEdit()->palette();
}

/**
 * Returns the wrap mode of the QPlainTextEdit casted to an integer type (so that it
 * can be used from the QML interface).
 */
int QmlPlainTextEdit::wordWrapMode() const
{
    return static_cast<int>(textEdit()->wordWrapMode());
}

/**
 * Returns the width of the vertical scrollbar
 */
int QmlPlainTextEdit::scrollbarWidth() const
{
    return textEdit()->verticalScrollBar()->width();
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
    return textEdit()->isEnabled();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
bool QmlPlainTextEdit::centerOnScroll() const
{
    return textEdit()->centerOnScroll();
}

/**
 * This property holds whether undo and redo are enabled.
 * Users are only able to undo or redo actions if this property is true, and if there is
 * an action that can be undone (or redone).
 */
bool QmlPlainTextEdit::undoRedoEnabled() const
{
    return textEdit()->isUndoRedoEnabled();
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
    return textEdit()->maximumBlockCount();
}

/**
 * This property holds the editor placeholder text.
 *
 * Setting this property makes the editor display a grayed-out placeholder text as long as
 * the document is empty.
 */
QString QmlPlainTextEdit::placeholderText() const
{
    return textEdit()->placeholderText();
}

/**
 * Returns the pointer to the text document
 */
QTextDocument *QmlPlainTextEdit::document() const
{
    return textEdit()->document();
}

/**
 * Returns the pointer to the text edit object
 */
QPlainTextEdit *QmlPlainTextEdit::textEdit() const
{
    return m_textEdit;
}

/**
 * Copies any selected text to the clipboard.
 */
void QmlPlainTextEdit::copy()
{
    textEdit()->copy();
}

/**
 * Deletes all the text in the text edit.
 */
void QmlPlainTextEdit::clear()
{
    textEdit()->clear();
    update();

    emit textChanged();
}

/**
 * Selects all the text of the text edit.
 */
void QmlPlainTextEdit::selectAll()
{
    textEdit()->selectAll();
    update();
}

/**
 * Clears the text selection
 */
void QmlPlainTextEdit::clearSelection()
{
    auto cursor = QTextCursor(textEdit()->document());
    cursor.clearSelection();
    textEdit()->setTextCursor(cursor);
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
    textEdit()->setReadOnly(ro);
    update();

    emit readOnlyChanged();
}

/**
 * Changes the font used to display the text of the text edit.
 */
void QmlPlainTextEdit::setFont(const QFont &font)
{
    textEdit()->setFont(font);
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
    textEdit()->appendPlainText(text);

    if (autoscroll())
        scrollToBottom();

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
    textEdit()->setPlainText(text);

    if (autoscroll())
        scrollToBottom();

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
    textEdit()->setStyleSheet(qss);
    update();

    emit colorChanged();
}

/**
 * Changes the width of the vertical scrollbar
 */
void QmlPlainTextEdit::setScrollbarWidth(const int width)
{
    auto bar = textEdit()->verticalScrollBar();
    bar->setFixedWidth(width);
    update();

    emit scrollbarWidthChanged();
}

/**
 * Changes the @c QPalette of the text editor widget and its children.
 */
void QmlPlainTextEdit::setPalette(const QPalette &palette)
{
    textEdit()->setPalette(palette);
    update();

    emit paletteChanged();
}

/**
 * Enables or disables the text editor widget.
 */
void QmlPlainTextEdit::setWidgetEnabled(const bool enabled)
{
    textEdit()->setEnabled(enabled);
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
    // Change internal variables
    m_autoscroll = enabled;

    // Scroll to bottom if autoscroll is enabled
    if (enabled)
        scrollToBottom(true);

    // Update console configuration
    IO::Console::getInstance()->setAutoscroll(enabled);

    // Hide vertical scrollbar if autoscroll is enabled
    auto bar = textEdit()->verticalScrollBar();
    bar->setVisible(!enabled);
    update();

    // Update UI
    emit autoscrollChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void QmlPlainTextEdit::insertText(const QString &text)
{
    // Add text at the end of the text document
    QTextCursor cursor(textEdit()->document());
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
    cursor.endEditBlock();

    // Autoscroll to bottom (if needed)
    if (autoscroll())
        scrollToBottom();

    // Redraw the control
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
    textEdit()->setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
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
    textEdit()->setCenterOnScroll(enabled);
    update();

    emit centerOnScrollChanged();
}

/**
 * Enables/disables undo/redo history support.
 */
void QmlPlainTextEdit::setUndoRedoEnabled(const bool enabled)
{
    textEdit()->setUndoRedoEnabled(enabled);
    update();

    emit undoRedoEnabledChanged();
}

/**
 * Changes the placeholder text of the text editor. The placeholder text is only displayed
 * when the document is empty.
 */
void QmlPlainTextEdit::setPlaceholderText(const QString &text)
{
    textEdit()->setPlaceholderText(text);
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
    auto *bar = textEdit()->verticalScrollBar();
    auto lineCount = textEdit()->document()->blockCount();
    auto visibleLines = qCeil(height() / textEdit()->fontMetrics().height());

    bar->setMinimum(0);
    bar->setMaximum(lineCount);

    if (lineCount > visibleLines)
        bar->setValue(lineCount - visibleLines);
    else
        bar->setValue(0);

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
    textEdit()->setMaximumBlockCount(maxBlockCount);
    update();

    emit maximumBlockCountChanged();
}

/**
 * Resizes the text editor widget to fit inside the QML item.
 */
void QmlPlainTextEdit::updateWidgetSize()
{
    textEdit()->setFixedSize(width(), height());
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
    // Subclass QPlainTextEdit so that we can call protected functions
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::mouseDoubleClickEvent;
        using QPlainTextEdit::mouseMoveEvent;
        using QPlainTextEdit::mousePressEvent;
        using QPlainTextEdit::mouseReleaseEvent;
    };

    // Call appropiate function
    auto hack = static_cast<Hack *>(textEdit());
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
    // Subclass QPlainTextEdit so that we can call protected functions
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::wheelEvent;
    };

    // Call wheel event function
    static_cast<Hack *>(textEdit())->wheelEvent(event);

    // Disable autoscroll if we are scrolling upwards
    if (autoscroll())
    {
        auto delta = event->angleDelta();
        auto deltaY = delta.y();

        if (deltaY > 0)
        {
            setAutoscroll(false);
            update();
        }
    }

    // Enable autoscroll if scrolling to bottom
    else
    {
        auto bar = textEdit()->verticalScrollBar();
        if (bar->value() >= bar->maximum())
        {
            setAutoscroll(true);
            update();
        }
    }
}
