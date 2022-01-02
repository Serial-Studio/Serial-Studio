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

#include <IO/Console.h>
#include <Misc/TimerEvents.h>
#include <Misc/ThemeManager.h>
#include <UI/Widgets/Terminal.h>

//----------------------------------------------------------------------------------------
// QML PlainTextEdit implementation
//----------------------------------------------------------------------------------------

/**
 * Constructor function
 */
Widgets::Terminal::Terminal(QQuickItem *parent)
    : UI::DeclarativeWidget(parent)
    , m_repaint(false)
    , m_autoscroll(true)
    , m_textChanged(false)
    , m_emulateVt100(false)
    , m_copyAvailable(false)
{
    // Set widget & configure VT-100 emulator
    setWidget(&m_textEdit);
    m_escapeCodeHandler.setTextEdit(&m_textEdit);

    // Setup default options
    setScrollbarWidth(14);
    m_textEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit.setSizeAdjustPolicy(QPlainTextEdit::AdjustToContents);

    // Set widget palette
    QPalette palette;
    auto theme = &Misc::ThemeManager::instance();
    palette.setColor(QPalette::Text, theme->consoleText());
    palette.setColor(QPalette::Base, theme->consoleBase());
    palette.setColor(QPalette::Button, theme->consoleButton());
    palette.setColor(QPalette::Window, theme->consoleWindow());
    palette.setColor(QPalette::Highlight, theme->consoleHighlight());
    palette.setColor(QPalette::HighlightedText, theme->consoleHighlightedText());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    palette.setColor(QPalette::PlaceholderText, theme->consolePlaceholderText());
#endif
    m_textEdit.setPalette(palette);

    // Connect signals/slots
    // clang-format off
    connect(&IO::Console::instance(), &IO::Console::stringReceived,
            this, &Widgets::Terminal::insertText);
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout10Hz,
            this, &Widgets::Terminal::repaint);
    // clang-format on

    // React to widget events
    connect(&m_textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(setCopyAvailable(bool)));
}

/**
 * Returns the font used by the QPlainTextEdit widget
 */
QFont Widgets::Terminal::font() const
{
    return m_textEdit.font();
}

/**
 * Returns the plain text of the QPlainTextEdit widget
 */
QString Widgets::Terminal::text() const
{
    return m_textEdit.toPlainText();
}

/**
 * Returns @c true if the text document is empty
 */
bool Widgets::Terminal::empty() const
{
    return m_textEdit.document()->isEmpty();
}

/**
 * Returns @c true if the widget is set to read-only
 */
bool Widgets::Terminal::readOnly() const
{
    return m_textEdit.isReadOnly();
}

/**
 * Returns @c true if the widget shall scroll automatically to the bottom when new
 * text is appended to the widget.
 */
bool Widgets::Terminal::autoscroll() const
{
    return m_autoscroll;
}

/**
 * Returns the palette used by the QPlainTextEdit widget
 */
QPalette Widgets::Terminal::palette() const
{
    return m_textEdit.palette();
}

/**
 * Returns the wrap mode of the QPlainTextEdit casted to an integer type (so that it
 * can be used from the QML interface).
 */
int Widgets::Terminal::wordWrapMode() const
{
    return static_cast<int>(m_textEdit.wordWrapMode());
}

/**
 * Returns the width of the vertical scrollbar
 */
int Widgets::Terminal::scrollbarWidth() const
{
    return m_textEdit.verticalScrollBar()->width();
}

/**
 * Returns @c true if the user is able to copy any text from the document. This value is
 * updated through the copyAvailable() signal sent by the QPlainTextEdit.
 */
bool Widgets::Terminal::copyAvailable() const
{
    return m_copyAvailable;
}

/**
 * Returns @c true if the QPlainTextEdit widget is enabled
 */
bool Widgets::Terminal::widgetEnabled() const
{
    return m_textEdit.isEnabled();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
bool Widgets::Terminal::centerOnScroll() const
{
    return m_textEdit.centerOnScroll();
}

/**
 * Returns true if the control shall parse basic VT-100 escape secuences. This can be
 * useful if you need to interface with a shell/CLI from Serial Studio.
 */
bool Widgets::Terminal::vt100emulation() const
{
    return m_emulateVt100;
}

/**
 * This property holds whether undo and redo are enabled.
 * Users are only able to undo or redo actions if this property is true, and if there is
 * an action that can be undone (or redone).
 */
bool Widgets::Terminal::undoRedoEnabled() const
{
    return m_textEdit.isUndoRedoEnabled();
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
int Widgets::Terminal::maximumBlockCount() const
{
    return m_textEdit.maximumBlockCount();
}

/**
 * This property holds the editor placeholder text.
 *
 * Setting this property makes the editor display a grayed-out placeholder text as long as
 * the document is empty.
 */
QString Widgets::Terminal::placeholderText() const
{
    return m_textEdit.placeholderText();
}

/**
 * Returns the pointer to the text document
 */
QTextDocument *Widgets::Terminal::document() const
{
    return m_textEdit.document();
}

/**
 * Copies any selected text to the clipboard.
 */
void Widgets::Terminal::copy()
{
    m_textEdit.copy();
}

/**
 * Deletes all the text in the text edit.
 */
void Widgets::Terminal::clear()
{
    m_textEdit.clear();
    updateScrollbarVisibility();
    requestRepaint(true);
}

/**
 * Selects all the text of the text edit.
 */
void Widgets::Terminal::selectAll()
{
    m_textEdit.selectAll();
    requestRepaint();
}

/**
 * Clears the text selection
 */
void Widgets::Terminal::clearSelection()
{
    auto cursor = QTextCursor(m_textEdit.document());
    cursor.clearSelection();
    m_textEdit.setTextCursor(cursor);
    updateScrollbarVisibility();
    requestRepaint();
}

/**
 * Changes the read-only state of the text edit.
 *
 * In a read-only text edit the user can only navigate through the text and select text;
 * modifying the text is not possible.
 */
void Widgets::Terminal::setReadOnly(const bool ro)
{
    m_textEdit.setReadOnly(ro);
    requestRepaint();

    Q_EMIT readOnlyChanged();
}

/**
 * Changes the font used to display the text of the text edit.
 */
void Widgets::Terminal::setFont(const QFont &font)
{
    m_textEdit.setFont(font);
    updateScrollbarVisibility();
    requestRepaint();

    Q_EMIT fontChanged();
}

/**
 * Appends a new paragraph with text to the end of the text edit.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void Widgets::Terminal::append(const QString &text)
{
    m_textEdit.appendPlainText(text);
    updateScrollbarVisibility();

    if (autoscroll())
        scrollToBottom();

    requestRepaint(true);
}

/**
 * Replaces the text of the text editor with @c text.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void Widgets::Terminal::setText(const QString &text)
{
    m_textEdit.setPlainText(text);
    updateScrollbarVisibility();

    if (autoscroll())
        scrollToBottom();

    requestRepaint(true);
}

/**
 * Changes the width of the vertical scrollbar
 */
void Widgets::Terminal::setScrollbarWidth(const int width)
{
    auto bar = m_textEdit.verticalScrollBar();
    bar->setFixedWidth(width);
    requestRepaint();

    Q_EMIT scrollbarWidthChanged();
}

/**
 * Changes the @c QPalette of the text editor widget and its children.
 */
void Widgets::Terminal::setPalette(const QPalette &palette)
{
    m_textEdit.setPalette(palette);
    requestRepaint();

    Q_EMIT colorPaletteChanged();
}

/**
 * Enables or disables the text editor widget.
 */
void Widgets::Terminal::setWidgetEnabled(const bool enabled)
{
    m_textEdit.setEnabled(enabled);
    requestRepaint();

    Q_EMIT widgetEnabledChanged();
}

/**
 * Enables/disable automatic scrolling. If automatic scrolling is enabled, then the
 * vertical scrollbar shall automatically scroll to the end of the document when the
 * text of the text editor is changed.
 */
void Widgets::Terminal::setAutoscroll(const bool enabled)
{
    // Change internal variables
    m_autoscroll = enabled;
    updateScrollbarVisibility();

    // Scroll to bottom if autoscroll is enabled
    if (enabled)
        scrollToBottom(true);

    // Update console configuration
    IO::Console::instance().setAutoscroll(enabled);

    // Update UI
    requestRepaint();
    Q_EMIT autoscrollChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void Widgets::Terminal::insertText(const QString &text)
{
    if (widgetEnabled())
        addText(text, vt100emulation());
}

/**
 * Changes the word wrap mode of the text editor.
 *
 * This property holds the mode QPlainTextEdit will use when wrapping text by words.
 */
void Widgets::Terminal::setWordWrapMode(const int mode)
{
    m_textEdit.setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
    updateScrollbarVisibility();
    requestRepaint();

    Q_EMIT wordWrapModeChanged();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
void Widgets::Terminal::setCenterOnScroll(const bool enabled)
{
    m_textEdit.setCenterOnScroll(enabled);
    requestRepaint();

    Q_EMIT centerOnScrollChanged();
}

/**
 * Enables/disables interpretation of VT-100 escape secuences. This can be useful when
 * interfacing through network ports or interfacing with a MCU that implements some
 * kind of shell.
 */
void Widgets::Terminal::setVt100Emulation(const bool enabled)
{
    m_emulateVt100 = enabled;
    Q_EMIT vt100EmulationChanged();
}

/**
 * Enables/disables undo/redo history support.
 */
void Widgets::Terminal::setUndoRedoEnabled(const bool enabled)
{
    m_textEdit.setUndoRedoEnabled(enabled);
    requestRepaint();

    Q_EMIT undoRedoEnabledChanged();
}

/**
 * Changes the placeholder text of the text editor. The placeholder text is only displayed
 * when the document is empty.
 */
void Widgets::Terminal::setPlaceholderText(const QString &text)
{
    m_textEdit.setPlaceholderText(text);
    requestRepaint();

    Q_EMIT placeholderTextChanged();
}

/**
 * Moves the position of the vertical scrollbar to the end of the document.
 */
void Widgets::Terminal::scrollToBottom(const bool repaint)
{
    // Get scrollbar pointer, calculate line count & visible text lines
    auto *bar = m_textEdit.verticalScrollBar();
    auto lineCount = m_textEdit.document()->blockCount();
    auto visibleLines = qRound(height() / m_textEdit.fontMetrics().height());

    // Abort operation if control is not visible
    if (visibleLines <= 0)
        return;

    // Update scrolling range
    bar->setMinimum(0);
    bar->setMaximum(lineCount);

    // Do not scroll to bottom if all text fits in current window
    if (lineCount > visibleLines)
        bar->setValue(lineCount - visibleLines + 2);
    else
        bar->setValue(0);

    // Trigger UI repaint
    if (repaint)
        requestRepaint();
}

/**
 * Specifies the maximum number of blocks the document may have. If there are more blocks
 * in the document that specified with this property blocks are removed from the beginning
 * of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited amount of
 * blocks.
 */
void Widgets::Terminal::setMaximumBlockCount(const int maxBlockCount)
{
    m_textEdit.setMaximumBlockCount(maxBlockCount);
    requestRepaint();

    Q_EMIT maximumBlockCountChanged();
}

/**
 * Redraws the widget. This function is called by a timer to reduce the number of paint
 * requests (and thus avoid considerable slow-downs).
 */
void Widgets::Terminal::repaint()
{
    if (m_repaint)
    {
        m_repaint = false;
        update();

        if (m_textChanged)
            Q_EMIT textChanged();
    }
}

/**
 * Hides or shows the scrollbar
 */
void Widgets::Terminal::updateScrollbarVisibility()
{
    // Get current line count & visible lines
    auto lineCount = m_textEdit.document()->blockCount();
    auto visibleLines = qCeil(height() / m_textEdit.fontMetrics().height());

    // Autoscroll enabled or text content is less than widget height
    if (autoscroll() || visibleLines >= lineCount)
        m_textEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Show the scrollbar if the text content is greater than the widget height
    else
        m_textEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

/**
 * Updates the value of copy-available. This function is automatically called by the text
 * editor widget when the user makes any text selection/deselection.
 */
void Widgets::Terminal::setCopyAvailable(const bool yes)
{
    m_copyAvailable = yes;
    Q_EMIT copyAvailableChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void Widgets::Terminal::addText(const QString &text, const bool enableVt100)
{
    // Get text to insert
    QString textToInsert = text;
    if (enableVt100)
        textToInsert = vt100Processing(text);

    // Clear terminal scrollback after 10000 lines
    if (m_textEdit.blockCount() >= 10000)
        m_textEdit.clear();

    // Add text at the end of the text document
    QTextCursor cursor(m_textEdit.document());
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(textToInsert);
    cursor.endEditBlock();

    // Autoscroll to bottom (if needed)
    updateScrollbarVisibility();
    if (autoscroll())
        scrollToBottom();

    // Repaint the widget
    requestRepaint(true);
}

/**
 * Enables the re-paint flag, which is later used by the @c repaint() function to know
 * if the widget shall be repainted.
 */
void Widgets::Terminal::requestRepaint(const bool textChanged)
{
    m_repaint = true;
    m_textChanged = textChanged;
}

/**
 * Processes the given @a data to remove the escape sequences from the text using code
 * from Qt Creator output terminal. Check the next code block for more info.
 */
QString Widgets::Terminal::vt100Processing(const QString &data)
{
    auto formattedText = m_escapeCodeHandler.parseText(FormattedText(data));
    const QString cleanLine = std::accumulate(
        formattedText.begin(), formattedText.end(), QString(),
        [](const FormattedText &t1, const FormattedText &t2) -> QString {
            return t1.text + t2.text;
        });
    m_escapeCodeHandler.endFormatScope();

    return cleanLine;
}

//----------------------------------------------------------------------------------------
// VT-100 / ANSI terminal hacks
// https://code.qt.io/cgit/qt-creator/qt-creator.git/tree/src/libs/utils/ansiescapecodehandler.cpp
//---------------------------------------------------------º-----------------------------------------

#define QTC_ASSERT(cond, action)                                                         \
    if (Q_LIKELY(cond)) { }                                                              \
    else                                                                                 \
    {                                                                                    \
        action;                                                                          \
    }                                                                                    \
    do                                                                                   \
    {                                                                                    \
    } while (0)

static QColor ansiColor(uint code)
{
    QTC_ASSERT(code < 8, return QColor());

    const int red = code & 1 ? 170 : 0;
    const int green = code & 2 ? 170 : 0;
    const int blue = code & 4 ? 170 : 0;
    return QColor(red, green, blue);
}

QVector<Widgets::FormattedText>
Widgets::AnsiEscapeCodeHandler::parseText(const FormattedText &input)
{
    enum AnsiEscapeCodes
    {
        ResetFormat = 0,
        BoldText = 1,
        TextColorStart = 30,
        TextColorEnd = 37,
        RgbTextColor = 38,
        DefaultTextColor = 39,
        BackgroundColorStart = 40,
        BackgroundColorEnd = 47,
        RgbBackgroundColor = 48,
        DefaultBackgroundColor = 49
    };

    const QString escape = "\x1b[";
    const QChar semicolon = ';';
    const QChar colorTerminator = 'm';
    const QChar eraseToEol = 'K';

    QVector<FormattedText> outputData;
    QTextCharFormat charFormat = m_previousFormatClosed ? input.format : m_previousFormat;
    QString strippedText;
    if (m_pendingText.isEmpty())
        strippedText = input.text;

    else
    {
        strippedText = m_pendingText.append(input.text);
        m_pendingText.clear();
    }

    while (!strippedText.isEmpty())
    {
        QTC_ASSERT(m_pendingText.isEmpty(), break);
        if (m_waitingForTerminator)
        {
            // We ignore all escape codes taking string arguments.
            QString terminator = "\x1b\\";
            int terminatorPos = strippedText.indexOf(terminator);
            if (terminatorPos == -1 && !m_alternateTerminator.isEmpty())
            {
                terminator = m_alternateTerminator;
                terminatorPos = strippedText.indexOf(terminator);
            }
            if (terminatorPos == -1)
            {
                m_pendingText = strippedText;
                break;
            }
            m_waitingForTerminator = false;
            m_alternateTerminator.clear();
            strippedText.remove(0, terminatorPos + terminator.length());
            if (strippedText.isEmpty())
                break;
        }
        const int escapePos = strippedText.indexOf(escape.at(0));
        if (escapePos < 0)
        {
            outputData << FormattedText(strippedText, charFormat);
            break;
        }
        else if (escapePos != 0)
        {
            outputData << FormattedText(strippedText.left(escapePos), charFormat);
            strippedText.remove(0, escapePos);
        }
        QTC_ASSERT(strippedText.at(0) == escape.at(0), break);

        while (!strippedText.isEmpty() && escape.at(0) == strippedText.at(0))
        {
            if (escape.startsWith(strippedText))
            {
                // control secquence is not complete
                m_pendingText += strippedText;
                strippedText.clear();
                break;
            }
            if (!strippedText.startsWith(escape))
            {
                switch (strippedText.at(1).toLatin1())
                {
                    case '\\': // Unexpected terminator sequence.
                        Q_FALLTHROUGH();
                    case 'N':
                    case 'O': // Ignore unsupported single-character sequences.
                        strippedText.remove(0, 2);
                        break;
                    case ']':
                        m_alternateTerminator = QChar(7);
                        Q_FALLTHROUGH();
                    case 'P':
                    case 'X':
                    case '^':
                    case '_':
                        strippedText.remove(0, 2);
                        m_waitingForTerminator = true;
                        break;
                    default:
                        // not a control sequence
                        m_pendingText.clear();
                        outputData << FormattedText(strippedText.left(1), charFormat);
                        strippedText.remove(0, 1);
                        continue;
                }
                break;
            }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_pendingText += strippedText.midRef(0, escape.length());
#else
            m_pendingText
                += QStringView { strippedText }.mid(0, escape.length()).toString();
#endif

            strippedText.remove(0, escape.length());

            // Get stripped text in uppercase
            auto upperCase = strippedText.toUpper();

            // Clear line
            if (upperCase.contains("2K"))
            {
                textEdit->setFocus();
                auto storedCursor = textEdit->textCursor();
                textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                textEdit->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                textEdit->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
                textEdit->textCursor().removeSelectedText();
                textEdit->textCursor().deletePreviousChar();
                textEdit->setTextCursor(storedCursor);
                return outputData;
            }

            // Clear screen
            if (upperCase.contains("2J"))
            {
                textEdit->clear();
                return QVector<FormattedText>();
            }

            // \e[K is not supported. Just strip it.
            if (strippedText.startsWith(eraseToEol))
            {
                m_pendingText.clear();
                strippedText.remove(0, 1);
                continue;
            }
            // get the number
            QString strNumber;
            StringList numbers;
            while (!strippedText.isEmpty())
            {
                if (strippedText.at(0).isDigit())
                {
                    strNumber += strippedText.at(0);
                }
                else
                {
                    if (!strNumber.isEmpty())
                        numbers << strNumber;
                    if (strNumber.isEmpty() || strippedText.at(0) != semicolon)
                        break;
                    strNumber.clear();
                }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                m_pendingText += strippedText.midRef(0, 1);
#else
                m_pendingText += QStringView { strippedText }.mid(0, 1).toString();
#endif
                strippedText.remove(0, 1);
            }
            if (strippedText.isEmpty())
                break;

            // remove terminating char
            if (!strippedText.startsWith(colorTerminator))
            {
                m_pendingText.clear();
                strippedText.remove(0, 1);
                break;
            }
            // got consistent control sequence, ok to clear pending text
            m_pendingText.clear();
            strippedText.remove(0, 1);

            if (numbers.isEmpty())
            {
                charFormat = input.format;
                endFormatScope();
            }

            for (int i = 0; i < numbers.size(); ++i)
            {
                const uint code = numbers.at(i).toUInt();

                if (code >= TextColorStart && code <= TextColorEnd)
                {
                    charFormat.setForeground(ansiColor(code - TextColorStart));
                    setFormatScope(charFormat);
                }
                else if (code >= BackgroundColorStart && code <= BackgroundColorEnd)
                {
                    charFormat.setBackground(ansiColor(code - BackgroundColorStart));
                    setFormatScope(charFormat);
                }
                else
                {
                    switch (code)
                    {
                        case ResetFormat:
                            charFormat = input.format;
                            endFormatScope();
                            break;
                        case BoldText:
                            charFormat.setFontWeight(QFont::Bold);
                            setFormatScope(charFormat);
                            break;
                        case DefaultTextColor:
                            charFormat.setForeground(input.format.foreground());
                            setFormatScope(charFormat);
                            break;
                        case DefaultBackgroundColor:
                            charFormat.setBackground(input.format.background());
                            setFormatScope(charFormat);
                            break;
                        case RgbTextColor:
                        case RgbBackgroundColor:
                            // See http://en.wikipedia.org/wiki/ANSI_escape_code#Colors
                            if (++i >= numbers.size())
                                break;
                            switch (numbers.at(i).toInt())
                            {
                                case 2:
                                    // RGB set with format: 38;2;<r>;<g>;<b>
                                    if ((i + 3) < numbers.size())
                                    {
                                        (code == RgbTextColor)
                                            ? charFormat.setForeground(
                                                QColor(numbers.at(i + 1).toInt(),
                                                       numbers.at(i + 2).toInt(),
                                                       numbers.at(i + 3).toInt()))
                                            : charFormat.setBackground(
                                                QColor(numbers.at(i + 1).toInt(),
                                                       numbers.at(i + 2).toInt(),
                                                       numbers.at(i + 3).toInt()));
                                        setFormatScope(charFormat);
                                    }
                                    i += 3;
                                    break;
                                case 5:
                                    // 256 color mode with format: 38;5;<i>
                                    uint index = numbers.at(i + 1).toUInt();

                                    QColor color;
                                    if (index < 8)
                                    {
                                        // The first 8 colors are standard low-intensity
                                        // ANSI colors.
                                        color = ansiColor(index);
                                    }
                                    else if (index < 16)
                                    {
                                        // The next 8 colors are standard high-intensity
                                        // ANSI colors.
                                        color = ansiColor(index - 8).lighter(150);
                                    }
                                    else if (index < 232)
                                    {
                                        // The next 216 colors are a 6x6x6 RGB cube.
                                        uint o = index - 16;
                                        color = QColor((o / 36) * 51, ((o / 6) % 6) * 51,
                                                       (o % 6) * 51);
                                    }
                                    else
                                    {
                                        // The last 24 colors are a greyscale gradient.
                                        int grey = int((index - 232) * 11);
                                        color = QColor(grey, grey, grey);
                                    }

                                    if (code == RgbTextColor)
                                        charFormat.setForeground(color);
                                    else
                                        charFormat.setBackground(color);

                                    setFormatScope(charFormat);
                                    ++i;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return outputData;
}

void Widgets::AnsiEscapeCodeHandler::setTextEdit(QPlainTextEdit *widget)
{
    textEdit = widget;
}

void Widgets::AnsiEscapeCodeHandler::endFormatScope()
{
    m_previousFormatClosed = true;
}

void Widgets::AnsiEscapeCodeHandler::setFormatScope(const QTextCharFormat &charFormat)
{
    m_previousFormat = charFormat;
    m_previousFormatClosed = false;
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Terminal.cpp"
#endif
