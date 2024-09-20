/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#include "IO/Console.h"
#include "IO/Manager.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/Terminal.h"

//------------------------------------------------------------------------------
// QML PlainTextEdit implementation
//------------------------------------------------------------------------------

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

  // Setup default options
  setScrollbarWidth(14);
  m_textEdit.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_textEdit.setSizeAdjustPolicy(QPlainTextEdit::AdjustToContents);

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Terminal::onThemeChanged);

  // Connect signals/slots
  connect(&IO::Console::instance(), &IO::Console::stringReceived, this,
          &Widgets::Terminal::insertText);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout10Hz, this,
          &Widgets::Terminal::repaint);

  // React to widget events
  connect(&m_textEdit, SIGNAL(copyAvailable(bool)), this,
          SLOT(setCopyAvailable(bool)));
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
 * Returns @c true if the widget shall scroll automatically to the bottom when
 * new text is appended to the widget.
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
 * Returns the wrap mode of the QPlainTextEdit casted to an integer type (so
 * that it can be used from the QML interface).
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
 * Returns @c true if the user is able to copy any text from the document. This
 * value is updated through the copyAvailable() signal sent by the
 * QPlainTextEdit.
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
 * If set to true, the plain text edit scrolls the document vertically to make
 * the cursor visible at the center of the viewport. This also allows the text
 * edit to scroll below the end of the document. Otherwise, if set to false, the
 * plain text edit scrolls the smallest amount possible to ensure the cursor is
 * visible.
 */
bool Widgets::Terminal::centerOnScroll() const
{
  return m_textEdit.centerOnScroll();
}

/**
 * Returns true if the control shall parse basic VT-100 escape secuences. This
 * can be useful if you need to interface with a shell/CLI from Serial Studio.
 */
bool Widgets::Terminal::vt100emulation() const
{
  return m_emulateVt100;
}

/**
 * This property holds whether undo and redo are enabled.
 * Users are only able to undo or redo actions if this property is true, and if
 * there is an action that can be undone (or redone).
 */
bool Widgets::Terminal::undoRedoEnabled() const
{
  return m_textEdit.isUndoRedoEnabled();
}

/**
 * This property holds the limit for blocks in the document.
 *
 * Specifies the maximum number of blocks the document may have. If there are
 * more blocks in the document that specified with this property blocks are
 * removed from the beginning of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited
 * amount of blocks.
 */
int Widgets::Terminal::maximumBlockCount() const
{
  return m_textEdit.maximumBlockCount();
}

/**
 * This property holds the editor placeholder text.
 *
 * Setting this property makes the editor display a grayed-out placeholder text
 * as long as the document is empty.
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
 * In a read-only text edit the user can only navigate through the text and
 * select text; modifying the text is not possible.
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
 * If @c autoscroll() is enabled, this function shall also update the scrollbar
 * position to scroll to the bottom of the text.
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
 * If @c autoscroll() is enabled, this function shall also update the scrollbar
 * position to scroll to the bottom of the text.
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
 * Enables/disable automatic scrolling. If automatic scrolling is enabled, then
 * the vertical scrollbar shall automatically scroll to the end of the document
 * when the text of the text editor is changed.
 */
void Widgets::Terminal::setAutoscroll(const bool enabled)
{
  // Change internal variables
  m_autoscroll = enabled;
  updateScrollbarVisibility();

  // Scroll to bottom if autoscroll is enabled
  if (enabled)
    scrollToBottom();

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
 * This property holds the mode QPlainTextEdit will use when wrapping text by
 * words.
 */
void Widgets::Terminal::setWordWrapMode(const int mode)
{
  m_textEdit.setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
  updateScrollbarVisibility();
  requestRepaint();

  Q_EMIT wordWrapModeChanged();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make
 * the cursor visible at the center of the viewport. This also allows the text
 * edit to scroll below the end of the document. Otherwise, if set to false, the
 * plain text edit scrolls the smallest amount possible to ensure the cursor is
 * visible.
 */
void Widgets::Terminal::setCenterOnScroll(const bool enabled)
{
  m_textEdit.setCenterOnScroll(enabled);
  requestRepaint();

  Q_EMIT centerOnScrollChanged();
}

/**
 * Enables/disables interpretation of VT-100 escape secuences. This can be
 * useful when interfacing through network ports or interfacing with a MCU that
 * implements some kind of shell.
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
 * Changes the placeholder text of the text editor. The placeholder text is only
 * displayed when the document is empty.
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

  // Abort operation no text is visible
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

  // Trigger UI repaint if requested
  if (repaint)
    requestRepaint();
}

/**
 * Specifies the maximum number of blocks the document may have. If there are
 * more blocks in the document that specified with this property blocks are
 * removed from the beginning of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited
 * amount of blocks.
 */
void Widgets::Terminal::setMaximumBlockCount(const int maxBlockCount)
{
  m_textEdit.setMaximumBlockCount(maxBlockCount);
  requestRepaint();

  Q_EMIT maximumBlockCountChanged();
}

/**
 * Redraws the widget. This function is called by a timer to reduce the number
 * of paint requests (and thus avoid considerable slow-downs).
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
 * Updates the widget's visual style and color palette to match the colors
 * defined by the application theme file.
 */
void Widgets::Terminal::onThemeChanged()
{
  // clang-format off
  QPalette palette;
  auto theme = &Misc::ThemeManager::instance();
  palette.setColor(QPalette::Text, theme->getColor("console_text"));
  palette.setColor(QPalette::Base, theme->getColor("console_base"));
  palette.setColor(QPalette::Button, theme->getColor("console_button"));
  palette.setColor(QPalette::Window, theme->getColor("console_window"));
  palette.setColor(QPalette::ButtonText, theme->getColor("console_button"));
  palette.setColor(QPalette::Highlight, theme->getColor("console_highlight"));
  palette.setColor(QPalette::HighlightedText, theme->getColor("console_highlighted_text"));
  palette.setColor(QPalette::PlaceholderText, theme->getColor("console_placeholder_text"));
  m_textEdit.setPalette(palette);
  update();
  // clang-format on
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
 * Updates the value of copy-available. This function is automatically called by
 * the text editor widget when the user makes any text selection/deselection.
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

  // Ensure that at most we show 1000 lines
  if (m_textEdit.blockCount() > 1000)
    m_textEdit.clear();

  // Add text at the current cursor position
  QTextCursor cursor(m_textEdit.document());
  cursor.beginEditBlock();
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(textToInsert);
  cursor.endEditBlock();

  // Autoscroll to bottom (if needed)
  if (autoscroll())
    scrollToBottom();

  // Repaint the widget
  requestRepaint(true);
}

/**
 * Enables the re-paint flag, which is later used by the @c repaint() function
 * to know if the widget shall be repainted.
 */
void Widgets::Terminal::requestRepaint(const bool textChanged)
{
  m_repaint = true;
  m_textChanged = textChanged;
}

//------------------------------------------------------------------------------
// VT-100 emulation
//------------------------------------------------------------------------------

QString Widgets::Terminal::vt100Processing(const QString &data)
{
  QString result;
  int i = 0;

  while (i < data.length())
  {
    if (data[i] == '\xFF') // Telnet IAC (Interpret As Command) - 0xFF
    {
      // Handle Telnet commands
      if (i + 1 < data.length())
      {
        char command = data[i + 1].toLatin1();
        if (command == '\xF0') // Telnet SE (End of subnegotiation)
        {
          // Handle subnegotiation end
          i += 2;
          continue;
        }
        else if (command == '\xFB' || command == '\xFD') // WILL or DO
        {
          // Handle Telnet WILL or DO (option negotiation)
          // We can inspect the following byte for the option code.
          i += 3; // Skip the IAC, command, and option byte
          continue;
        }
        else if (command == '\xFA') // Telnet SB (subnegotiation start)
        {
          // Handle subnegotiation start (for things like NAWS)
          // Typically you would capture the subnegotiation data here
          i += 2;
          while (i < data.length() && data[i] != '\xFF')
            i++; // Move through subnegotiation data
          continue;
        }
        else
        {
          // Skip over any unhandled Telnet commands
          i += 2;
          continue;
        }
      }
    }
    else if (data[i] == '\x1b') // ANSI/VT-100 Escape character
    {
      if (data.mid(i, 2) == "\x1b[")
      {
        i += 2; // Skip the escape and [

        // Initialize parameters string for the escape sequence
        QString params;

        // Extract parameters until we find a non-digit or semicolon (indicating
        // we are in the CSI sequence)
        while (i < data.length() && (data[i].isDigit() || data[i] == ';'))
        {
          params += data[i];
          ++i;
        }

        // Text formatting (color, bold, etc.)
        if (data[i] == 'm')
        {
          applyTextFormatting(params);
          ++i;
          continue;
        }

        // Clear screen command
        else if (data[i] == 'J')
        {
          if (params.isEmpty() || params == "2")
            m_textEdit.clear();
          else if (params == "0")
            clearFromCursorToEnd();
          else if (params == "1")
            clearFromStartToCursor();

          ++i;
          continue;
        }

        // Clear line command
        else if (data[i] == 'K')
        {
          if (params.isEmpty() || params == "0")
            clearFromCursorToEndOfLine();
          else if (params == "1")
            clearFromStartOfLineToCursor();
          else if (params == "2")
            clearEntireLine();

          ++i;
          continue;
        }

        // Cursor positioning command \x1b[<row>;<col>H or \x1b[<row>;<col>f
        else if (data[i] == 'H' || data[i] == 'f')
        {
          // Default row and column to (1, 1) for \x1b[H (move to home position)
          int row = 1, col = 1;

          if (!params.isEmpty())
          {
            QStringList coordinates = params.split(';');
            if (coordinates.size() > 0)
              row = coordinates[0].toInt(); // Row number
            if (coordinates.size() > 1)
              col = coordinates[1].toInt(); // Column number
          }

          // Call moveCursorTo with the parsed row and column
          moveCursorTo(row, col);

          ++i;
          continue;
        }

        // Add more VT-100 sequences as necessary (cursor movement, etc.)
      }
    }

    // Handle normal text data by appending to the result
    result += data[i];
    ++i;
  }

  return result;
}

void Widgets::Terminal::clearEntireLine()
{
  QTextCursor cursor = m_textEdit.textCursor();
  cursor.select(QTextCursor::BlockUnderCursor);
  cursor.removeSelectedText();
}

void Widgets::Terminal::clearFromCursorToEnd()
{
  QTextCursor cursor = m_textEdit.textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
}

void Widgets::Terminal::clearFromStartToCursor()
{
  QTextCursor cursor = m_textEdit.textCursor();
  cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
}

void Widgets::Terminal::clearFromCursorToEndOfLine()
{
  QTextCursor cursor = m_textEdit.textCursor();
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
}

void Widgets::Terminal::clearFromStartOfLineToCursor()
{
  QTextCursor cursor = m_textEdit.textCursor();
  cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
}

void Widgets::Terminal::moveCursorTo(int row, int col)
{
  if (autoscroll())
    scrollToBottom();

  // Get the current cursor position and visible area
  QTextCursor cursor = m_textEdit.textCursor();

  // Get the block count (lines) in the document
  int totalBlockCount = m_textEdit.document()->blockCount();

  // Get the first visible block index (starting at the top of the visible area)
  int firstBlock = m_textEdit.cursorForPosition(QPoint(0, 0)).blockNumber();

  // Move cursor to the visible area (relative to the viewport)
  int targetBlock = firstBlock + row - 1;

  // Ensure target block is within valid range
  if (targetBlock < 0)
    targetBlock = 0;
  if (targetBlock >= totalBlockCount)
    targetBlock = totalBlockCount - 1;

  // Move to the target block (row)
  cursor.movePosition(QTextCursor::Start);
  for (int i = 0; i < targetBlock; ++i)
    cursor.movePosition(QTextCursor::Down);

  // Move to the start of the block and then move to the desired column
  cursor.movePosition(QTextCursor::StartOfBlock);
  for (int i = 1; i < col; ++i)
    cursor.movePosition(QTextCursor::Right);

  // Set the new cursor position in the text edit
  m_textEdit.setTextCursor(cursor);
}

void Widgets::Terminal::applyTextFormatting(const QString &params)
{
  QTextCharFormat format = m_textEdit.currentCharFormat();
  QStringList codes = params.split(';');

  for (const QString &codeStr : codes)
  {
    int code = codeStr.toInt();
    switch (code)
    {
      case 0: // Reset
        format.setForeground(Qt::black);
        format.setBackground(Qt::white);
        format.setFontWeight(QFont::Normal);
        break;
      case 1: // Bold
        format.setFontWeight(QFont::Bold);
        break;
      case 30: // Black text
        format.setForeground(Qt::black);
        break;
      case 31: // Red text
        format.setForeground(Qt::red);
        break;
      case 32: // Green text
        format.setForeground(Qt::green);
        break;
      case 33: // Yellow text
        format.setForeground(Qt::yellow);
        break;
      case 34: // Blue text
        format.setForeground(Qt::blue);
        break;
      case 35: // Magenta text
        format.setForeground(Qt::magenta);
        break;
      case 36: // Cyan text
        format.setForeground(Qt::cyan);
        break;
      case 37: // White text
        format.setForeground(Qt::white);
        break;
      default:
        break;
        // Add more color codes and background settings if necessary
    }
  }

  m_textEdit.setCurrentCharFormat(format);
}
