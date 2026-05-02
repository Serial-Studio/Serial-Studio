/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "UI/Widgets/Terminal.h"

#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>

#include "Console/Handler.h"
#include "IO/ConnectionManager.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#endif

/**
 * @brief Define the number of max lines supported
 */
constexpr int MAX_LINES = 1000;

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Terminal object with the given parent item.
 *
 * Initializes the terminal widget, setting up various configurations, including
 * font, palette, and buffer. It also establishes multiple signal-slot
 * connections to handle theme changes, data input, device connections, and
 * cursor blinking.
 *
 * @param parent Pointer to the parent QQuickItem.
 *
 * The constructor performs the following key actions:
 * - Initializes internal buffers using `initBuffer()`.
 * - Configures item flags to allow the terminal to accept mouse input, have
 *   content, and manage focus appropriately.
 * - Sets a monospaced font for text rendering using
 *   `Misc::CommonFonts::instance().monoFont()`.
 * - Configures the initial color palette by calling `onThemeChanged()`.
 * - Connects to the theme manager to update the palette whenever the theme
 *   changes.
 * - Connects to the IO Handler handler to receive and append new data for
 *   display.
 * - Clears the screen when a device connection status changes.
 * - Sets up a cursor blink timer and connects it to toggle the cursor
 *   visibility.
 * - Establishes a connection to redraw the terminal at a rate of 24 Hz for
 *   smooth updates.
 *
 * @note The cursor flash time is retrieved from
 * `QGuiApplication::styleHints()`, and the blink interval is adjusted
 * accordingly.
 */
Widgets::Terminal::Terminal(QQuickItem* parent)
  : QuickPaintedItemCompat(parent)
  , m_cWidth(0)
  , m_cHeight(0)
  , m_borderX(0)
  , m_borderY(0)
  , m_scrollOffsetY(0)
  , m_state(Text)
  , m_autoscroll(true)
  , m_ansiColors(false)
  , m_emulateVt100(false)
  , m_cursorVisible(true)
  , m_mouseTracking(false)
  , m_currentFormatValue(0)
  , m_privateMode(false)
  , m_stateChanged(false)
  , m_cursorHidden(false)
{
  initBuffer();

  // Configure QML item flags
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setActiveFocusOnTab(true);
  setMipmap(true);
  setOpaquePainting(true);

  // Initialize font and welcome screen
  loadWelcomeGuide();
  setFont(Console::Handler::instance().font());
  connect(&Console::Handler::instance(), &Console::Handler::fontChanged, this, [this] {
    setFont(Console::Handler::instance().font());
  });

  // Connect theme, data, and connection signals
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &Widgets::Terminal::onThemeChanged);
  connect(&Console::Handler::instance(),
          &Console::Handler::displayString,
          this,
          &Widgets::Terminal::append);
  connect(
    &Console::Handler::instance(), &Console::Handler::cleared, this, &Widgets::Terminal::clear);
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [=, this] {
      if (IO::ConnectionManager::instance().isConnected())
        clear();
      else if (m_data.isEmpty())
        loadWelcomeGuide();
    });

  // Scroll to cursor when becoming visible
  connect(this, &Widgets::Terminal::visibleChanged, this, [=, this] {
    if (isVisible()) {
      if (autoscroll() && linesPerPage() > 0) {
        int cursorLine   = m_cursorPosition.y();
        int wrappedLines = 1;
        if (cursorLine < m_data.size()) {
          int lineLength = m_data[cursorLine].length();
          wrappedLines   = (lineLength + maxCharsPerLine() - 1) / maxCharsPerLine();
        }

        int visualBottom = cursorLine + wrappedLines - 1;
        setScrollOffsetY(qMax(0, visualBottom - linesPerPage() + 1));
      }

      update();
    }
  });

  // Update welcome guide when Serial Studio changes its activation status
#ifdef BUILD_COMMERCIAL
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          &Widgets::Terminal::loadWelcomeGuide);
#endif

  // Reload welcome guide when changing language
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged, this, [this] {
    loadWelcomeGuide();
  });

  // Cursor blink timer
  m_cursorTimer.start(200);
  m_cursorTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_cursorTimer, &QTimer::timeout, this, &Widgets::Terminal::toggleCursor);

  // Redraw at UI refresh rate when state changes
  m_stateChanged = true;
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [=, this] {
    if (isVisible() && m_stateChanged) {
      m_stateChanged = false;
      update();
    }
  });
}

//--------------------------------------------------------------------------------------------------
// Rendering pipeline
//--------------------------------------------------------------------------------------------------

/**
 * @brief Draws the selection highlight for one word-wrapped segment of a line.
 *
 * Determines which characters within [segStart, segEnd) are covered by the
 * current selection and fills the corresponding pixel rectangle. Does nothing
 * if the segment is outside the selection range.
 *
 * @param painter   Active QPainter.
 * @param line      Full text of the logical line.
 * @param lineIndex Logical line index (row).
 * @param segStart  First character index of this wrapped segment.
 * @param segEnd    One-past-last character index of this wrapped segment.
 * @param y         Top pixel Y coordinate of the segment row.
 */
void Widgets::Terminal::drawSegmentSelection(
  QPainter* painter, const QString& line, int lineIndex, int segStart, int segEnd, int y)
{
  // Skip if no selection or this line is out of range
  if (m_selectionEnd.isNull())
    return;

  if (lineIndex < m_selectionStart.y() || lineIndex > m_selectionEnd.y())
    return;

  // Determine which character columns are selected in this segment
  int selStartX, selEndX;
  if (lineIndex == m_selectionStart.y() && lineIndex == m_selectionEnd.y()) {
    selStartX = qMax(m_selectionStart.x(), segStart);
    selEndX   = qMin(m_selectionEnd.x(), segEnd);
  } else if (lineIndex == m_selectionStart.y()) {
    selStartX = qMax(m_selectionStart.x(), segStart);
    selEndX   = segEnd;
  } else if (lineIndex == m_selectionEnd.y()) {
    selStartX = segStart;
    selEndX   = qMin(m_selectionEnd.x(), segEnd);
  } else {
    selStartX = segStart;
    selEndX   = segEnd;
  }

  if (selStartX >= selEndX)
    return;

  int startX         = m_borderX;
  int selectionWidth = 0;
  const int maxWidth = width() - 2 * m_borderX;

  for (int j = segStart; j < selEndX; ++j) {
    const int charWidth = painter->fontMetrics().horizontalAdvance(line[j]);
    if (j < selStartX)
      startX += charWidth;
    else
      selectionWidth += charWidth;
  }

  selectionWidth = qMin(selectionWidth, maxWidth - (startX - m_borderX));
  painter->fillRect(QRect(startX, y, selectionWidth, m_cHeight),
                    m_palette.color(QPalette::Highlight));
}

/**
 * @brief Renders a word-wrapped segment with a single uniform text color.
 *
 * Fast path used when ANSI color data is not available for this line.
 * Characters are drawn one at a time with variable advance widths.
 *
 * @param painter   Active QPainter.
 * @param segment   The substring to render.
 * @param textColor Color to use for all characters.
 * @param x         Left pixel X to start drawing at.
 * @param y         Top pixel Y of the row.
 */
void Widgets::Terminal::renderFastSegment(
  QPainter* painter, const QString& segment, const QColor& textColor, int x, int y)
{
  painter->setPen(textColor);
  const int segWidth = painter->fontMetrics().horizontalAdvance(segment);
  painter->drawText(x, y, segWidth, m_cHeight, Qt::AlignLeft | Qt::AlignVCenter, segment);
}

/**
 * @brief Renders one word-wrapped segment using per-character ANSI colors.
 *
 * Builds a vector of character info (position, width, fg/bg color) from
 * @p colorLine, then draws all background fills in a first pass and all
 * text glyphs in a second pass.
 *
 * @param painter        Active QPainter.
 * @param segment        The substring to render.
 * @param segStart       Start index of @p segment within the logical line
 *                       (used to look up ANSI colors).
 * @param colorLine      Per-character color data for the line, or nullptr.
 * @param defaultFg      Default foreground color when no ANSI override.
 * @param x              Left pixel X to start drawing at.
 * @param y              Top pixel Y of the row.
 */
void Widgets::Terminal::renderAnsiSegment(QPainter* painter,
                                          const QString& segment,
                                          int segStart,
                                          const QList<CharColor>* colorLine,
                                          const QColor& defaultFg,
                                          int x,
                                          int y)
{
  // Coalesce same-color runs into a single fillRect + drawText instead of per-char
  const auto& fm = painter->fontMetrics();
  int xPos       = x;
  int j          = 0;

  while (j < segment.length()) {
    // Determine color for current character
    const int charIndex = segStart + j;
    QColor runFg        = defaultFg;
    QColor runBg;

    if (colorLine && charIndex < colorLine->size()) {
      const CharColor& cc = (*colorLine)[charIndex];
      runFg               = cc.foreground.isValid() ? cc.foreground : defaultFg;
      runBg               = cc.background;
    }

    // Accumulate a run of characters with the same colors
    const int runStart = j;
    ++j;
    while (j < segment.length()) {
      const int ci = segStart + j;
      QColor fg    = defaultFg;
      QColor bg;

      if (colorLine && ci < colorLine->size()) {
        const CharColor& cc = (*colorLine)[ci];
        fg                  = cc.foreground.isValid() ? cc.foreground : defaultFg;
        bg                  = cc.background;
      }

      if (fg != runFg || bg != runBg)
        break;

      ++j;
    }

    // Draw the coalesced run
    const auto runText = QStringView(segment).mid(runStart, j - runStart);
    const int runWidth = fm.horizontalAdvance(runText.toString());

    if (runBg.isValid())
      painter->fillRect(xPos, y, runWidth, m_cHeight, runBg);

    painter->setPen(runFg);
    painter->drawText(xPos, y, runWidth, m_cHeight, Qt::AlignVCenter, runText.toString());
    xPos += runWidth;
  }
}

/**
 * @brief Draws the blinking cursor at the current cursor position.
 *
 * Iterates visible lines from @p firstLine to @p lastVLine, accounting for
 * word-wrapped segments, to find the visual row that contains the cursor.
 * Renders a block glyph at the correct pixel position.
 *
 * @param painter    Active QPainter.
 * @param firstLine  First logical line index in the visible viewport.
 * @param lastVLine  Last logical line index in the visible viewport.
 * @param lineHeight Pixel height of one visual row.
 */
void Widgets::Terminal::drawCursor(QPainter* painter, int firstLine, int lastVLine, int lineHeight)
{
  // Walk visible lines to find the cursor's visual row and draw it
  const int cursorLine = m_cursorPosition.y();
  const int cursorCol  = m_cursorPosition.x();

  int visualLineY  = m_borderY;
  bool cursorDrawn = false;

  for (int i = firstLine; i <= lastVLine && i < m_data.size(); ++i) {
    const QString& line = m_data[i];

    if (line.isEmpty()) {
      if (i == cursorLine) {
        painter->setPen(m_palette.color(QPalette::Text));
        // code-verify off
        painter->drawText(m_borderX, visualLineY + m_cHeight, QStringLiteral("█"));
        // code-verify on
        cursorDrawn = true;
        break;
      }
      visualLineY += lineHeight;
      continue;
    }

    int start = 0;
    while (start < line.length()) {
      const int end = qMin<int>(start + maxCharsPerLine(), line.length());

      if (i == cursorLine && cursorCol >= start && cursorCol <= end) {
        const int cursorX = calcCursorPixelX(painter, line, start, cursorCol, end);
        painter->setPen(m_palette.color(QPalette::Text));
        // code-verify off
        painter->drawText(cursorX, visualLineY + m_cHeight, QStringLiteral("█"));
        // code-verify on
        cursorDrawn = true;
        break;
      }

      visualLineY += lineHeight;
      start = end;
    }

    if (cursorDrawn)
      break;
  }

  if (!cursorDrawn && cursorLine >= m_data.size()) {
    painter->setPen(m_palette.color(QPalette::Text));
    // code-verify off
    painter->drawText(m_borderX, visualLineY + m_cHeight, QStringLiteral("█"));
    // code-verify on
  }
}

/**
 * @brief Paints the terminal widget content.
 *
 * This method overrides the QQuickPaintedItem::paint() to render the terminal's
 * content, including the text, cursor, and optional scrollbar.
 *
 * @param painter A QPainter object used to draw the terminal's content.
 *
 * The paint method performs the following key tasks:
 * - Skips rendering if the terminal is not visible.
 * - Prepares the painter by setting the current font and fills the terminal
 *   background.
 * - Draws each visible line of terminal data, using the current palette.
 * - Draws the cursor if it is currently visible and within the visible range of
 *   lines.
 * - Draws a vertical scrollbar if autoscroll is disabled and not all lines are
 *   visible.
 *
 * @note This function handles rendering for different terminal states,
 * including cursor visibility and scrolling requirements.
 */
void Widgets::Terminal::paint(QPainter* painter)
{
  if (!isVisible() || !painter)
    return;

  // Prepare font and compute visible line range
  painter->setFont(m_font);
  const int lineHeight = m_cHeight;
  const int firstLine  = m_scrollOffsetY;
  const int lastVLine  = qMin(firstLine + linesPerPage(), lineCount() - 1);

  paintSelectionHighlights(painter, firstLine, lastVLine, lineHeight);
  paintTextContent(painter, firstLine, lastVLine, lineHeight);

  // Draw cursor if visible and not hidden by escape sequence
  if (m_cursorVisible && !m_cursorHidden)
    drawCursor(painter, firstLine, lastVLine, lineHeight);

  paintScrollbar(painter);
}

/**
 * @brief Fills the selection-highlight rectangles for the visible line range.
 */
void Widgets::Terminal::paintSelectionHighlights(QPainter* painter,
                                                 int firstLine,
                                                 int lastVLine,
                                                 int lineHeight)
{
  int y = m_borderY;
  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i) {
    const QString& line = m_data[i];
    bool lineFullySelected =
      !m_selectionEnd.isNull() && i >= m_selectionStart.y() && i < m_selectionEnd.y();

    if (line.isEmpty()) {
      if (lineFullySelected) {
        QRect selectionRect(m_borderX, y, width() - 2 * m_borderX, m_cHeight);
        painter->fillRect(selectionRect, m_palette.color(QPalette::Highlight));
      }

      y += lineHeight;
      continue;
    }

    if (lineFullySelected) {
      const int wrappedLines = qMax(1, (line.length() + maxCharsPerLine() - 1) / maxCharsPerLine());
      for (int wrapIndex = 0; wrapIndex < wrappedLines && y < height() - m_borderY; ++wrapIndex) {
        QRect selectionRect(m_borderX, y, width() - 2 * m_borderX, m_cHeight);
        painter->fillRect(selectionRect, m_palette.color(QPalette::Highlight));
        y += lineHeight;
      }

      continue;
    }

    int start = 0;
    while (start < line.length()) {
      const int lineEnd = qMin<int>(start + maxCharsPerLine(), line.length());
      drawSegmentSelection(painter, line, i, start, lineEnd, y);
      y += lineHeight;
      start = lineEnd;
    }
  }
}

/**
 * @brief Renders the text content for the visible line range, ANSI-colored or not.
 */
void Widgets::Terminal::paintTextContent(QPainter* painter,
                                         int firstLine,
                                         int lastVLine,
                                         int lineHeight)
{
  int y                         = m_borderY;
  const QColor defaultTextColor = m_palette.color(QPalette::Text);
  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i) {
    const QString& line = m_data[i];

    if (line.isEmpty()) {
      y += lineHeight;
      continue;
    }

    const QList<CharColor>* colorLine = nullptr;
    if (ansiColors() && i < m_colorData.size())
      colorLine = &m_colorData[i];

    int start = 0;
    while (start < line.length()) {
      const int end         = qMin<int>(start + maxCharsPerLine(), line.length());
      const QString segment = line.mid(start, end - start);
      int x                 = m_borderX;

      if (!colorLine)
        renderFastSegment(painter, segment, defaultTextColor, x, y);
      else
        renderAnsiSegment(painter, segment, start, colorLine, defaultTextColor, x, y);

      y += lineHeight;
      start = end;
    }
  }
}

/**
 * @brief Draws the rounded scrollbar thumb when manual scrolling is active.
 */
void Widgets::Terminal::paintScrollbar(QPainter* painter)
{
  if (autoscroll() || lineCount() <= linesPerPage())
    return;

  const int availableHeight = height() - 2 * m_borderY;
  const int scrollbarWidth  = 6;
  int scrollbarHeight       = qMax(20.0, qPow(availableHeight, 2) / lineCount());
  if (scrollbarHeight > availableHeight / 2)
    scrollbarHeight = availableHeight / 2;

  const int x = width() - scrollbarWidth - m_borderX;
  int y       = (m_scrollOffsetY / static_cast<float>(lineCount() - linesPerPage()))
          * (availableHeight - scrollbarHeight)
        - m_borderY;
  y = qMax(m_borderY, y);

  QRect scrollbarRect(x, y, scrollbarWidth, scrollbarHeight);
  QBrush scrollbarBrush(m_palette.color(QPalette::Window));
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setBrush(scrollbarBrush);
  painter->setPen(Qt::NoPen);
  painter->drawRoundedRect(scrollbarRect, scrollbarWidth / 2, scrollbarWidth / 2);
}

//--------------------------------------------------------------------------------------------------
// Character metrics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the width of a single terminal character.
 * @return
 */
int Widgets::Terminal::charWidth() const
{
  return m_cWidth;
}

/**
 * @brief Returns the height of a single terminal character.
 * @return
 */
int Widgets::Terminal::charHeight() const
{
  return m_cHeight;
}

//--------------------------------------------------------------------------------------------------
// Style & display getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the current font used by the terminal.
 *
 * @return The QFont object representing the terminal's current font.
 */
const QFont& Widgets::Terminal::font() const
{
  return m_font;
}

/**
 * @brief Gets the current color palette used by the terminal.
 *
 * @return The QPalette object representing the terminal's color palette.
 */
const QPalette& Widgets::Terminal::colorPalette() const
{
  return m_palette;
}

/**
 * @brief Checks if autoscroll is enabled.
 *
 * @return True if autoscroll is enabled, false otherwise.
 */
bool Widgets::Terminal::autoscroll() const
{
  return m_autoscroll;
}

/**
 * @brief Checks if there is a valid text selection available for copying.
 *
 * This function determines whether a selection is available in the terminal
 * for copying. A selection is considered invalid if:
 *
 * - The selection start or selection end is null.
 * - The terminal's data buffer is empty.
 *
 * @return True if no valid selection is available, otherwise false.
 *
 * @note This function effectively checks if it is possible to perform a copy
 * operation.
 */
bool Widgets::Terminal::copyAvailable() const
{
  return (!m_selectionEnd.isNull() || !m_selectionStart.isNull()) && !m_data.isEmpty();
}

/**
 * @brief Checks if VT-100 emulation mode is enabled.
 *
 * @return True if VT-100 emulation is enabled, false otherwise.
 */
bool Widgets::Terminal::vt100emulation() const
{
  return m_emulateVt100;
}

/**
 * @brief Checks if ANSI color support is enabled.
 *
 * @return True if ANSI colors are enabled, false otherwise.
 */
bool Widgets::Terminal::ansiColors() const
{
  return m_ansiColors && m_emulateVt100;
}

//--------------------------------------------------------------------------------------------------
// Buffer & cursor info
//--------------------------------------------------------------------------------------------------

/**
 * @brief Gets the total number of lines in the terminal's data buffer.
 *
 * @return The number of lines currently stored in the terminal's data buffer.
 */
int Widgets::Terminal::lineCount() const
{
  return m_data.size();
}

/**
 * @brief Gets the number of lines that can be displayed per page.
 *
 * @return The number of lines that fit within the current terminal height.
 */
int Widgets::Terminal::linesPerPage() const
{
  // Avoid divide-by-zero when font metrics are not yet available
  if (m_cHeight <= 0)
    return 0;

  return static_cast<int>(qFloor((height() - 2 * m_borderY) / m_cHeight));
}

/**
 * @brief Gets the current vertical scroll offset.
 *
 * @return The vertical scroll offset in lines.
 */
int Widgets::Terminal::scrollOffsetY() const
{
  return m_scrollOffsetY;
}

/**
 * @brief Calculates the maximum number of characters that can fit on a single
 *        line of the terminal.
 *
 * This function determines the maximum number of characters that can be
 * displayed on a single line of the terminal, based on the current width of the
 * widget and the width of individual characters. It ensures a minimum value to
 * maintain consistency, especially during UI loading or resizing.
 *
 * The calculation takes into account:
 * - The current width of the terminal widget
 * - The border width on both sides
 * - The width of individual characters
 *
 * To prevent inconsistencies during UI initialization or extreme resizing,
 * the function enforces a minimum return value of 84 characters per line.
 *
 * @return The maximum number of characters that can fit on a single line,
 *         with a minimum value of 84.
 */
int Widgets::Terminal::maxCharsPerLine() const
{
  // Fall back to 84 columns until font metrics are available
  if (m_cWidth <= 0)
    return 84;

  const auto realValue = (width() - 2 * m_borderX) / m_cWidth;
  return qMax<int>(84, realValue);
}

/**
 * @brief Returns the number of character columns currently visible.
 *
 * Delegates to maxCharsPerLine() and is exposed as a reactive Q_PROPERTY so
 * QML bindings update automatically when the widget is resized.
 */
int Widgets::Terminal::terminalColumns() const
{
  return maxCharsPerLine();
}

/**
 * @brief Returns the number of character rows currently visible.
 *
 * Delegates to linesPerPage() and is exposed as a reactive Q_PROPERTY so QML
 * bindings update automatically when the widget is resized.
 */
int Widgets::Terminal::terminalRows() const
{
  return linesPerPage();
}

/**
 * @brief Handles key press events and forwards them to the active driver as
 *        VT-100 byte sequences.
 *
 * When VT-100 emulation is active and the IO manager is connected in
 * read-write mode, each key press is translated to the canonical VT-100 /
 * xterm sequence and written directly to the driver.  This makes interactive
 * CLI programs (shells, editors, REPLs) work correctly inside the widget
 * without any QML-level workarounds.
 *
 * Keys handled:
 * - Printable characters -> UTF-8 bytes
 * - Return / Enter      -> CR (\r)
 * - Backspace           -> BS (\x7f) per modern xterm default
 * - Delete              -> ESC[3~
 * - Tab                 -> HT (\t)
 * - Escape              -> ESC (\x1b)
 * - Arrow keys          -> ESC[A ... ESC[D
 * - Home / End          -> ESC[H / ESC[F
 * - Page Up / Down      -> ESC[5~ / ESC[6~
 * - Insert              -> ESC[2~
 * - F1-F12              -> standard xterm sequences
 * - Ctrl+letter         -> control codes 0x01-0x1a
 *
 * @param event The key press event.
 */
void Widgets::Terminal::keyPressEvent(QKeyEvent* event)
{
  // Delegate to base class when VT-100 emulation is inactive
  if (!vt100emulation() || !IO::ConnectionManager::instance().isConnected()
      || !IO::ConnectionManager::instance().readWrite()) {
    QuickPaintedItemCompat::keyPressEvent(event);
    return;
  }

  // Translate the key event to a VT-100 byte sequence
  const QByteArray seq = translateKeyToVt100(event);
  if (!seq.isEmpty()) {
    (void)IO::ConnectionManager::instance().writeData(seq);
    event->accept();
    return;
  }

  QuickPaintedItemCompat::keyPressEvent(event);
}

/**
 * @brief Maps a Qt key event to its VT-100 byte sequence; empty if unmapped.
 */
QByteArray Widgets::Terminal::translateKeyToVt100(const QKeyEvent* event)
{
  QByteArray seq;
  const Qt::KeyboardModifiers mods = event->modifiers();
  const int key                    = event->key();

  // Ctrl+letter -> control code
  if ((mods & Qt::ControlModifier) && key >= Qt::Key_A && key <= Qt::Key_Z) {
    seq.append(char(key - Qt::Key_A + 1));
    return seq;
  }

  // Ctrl+[ -> ESC
  if ((mods & Qt::ControlModifier) && key == Qt::Key_BracketLeft) {
    seq.append('\x1b');
    return seq;
  }

  // Return/Enter respects the configured line-ending preference
  if (key == Qt::Key_Return || key == Qt::Key_Enter)
    return translateEnterKey();

  // Editing/navigation/function keys
  seq = translateSpecialKey(key);
  if (!seq.isEmpty())
    return seq;

  // Fall back to raw text input for printable keys
  if (!event->text().isEmpty())
    seq = event->text().toUtf8();

  return seq;
}

/**
 * @brief Returns the byte sequence for Return/Enter under the active line ending.
 */
QByteArray Widgets::Terminal::translateEnterKey()
{
  QByteArray seq;
  switch (Console::Handler::instance().lineEnding()) {
    case Console::Handler::LineEnding::NoLineEnding:
      break;
    case Console::Handler::LineEnding::NewLine:
      seq.append('\n');
      break;
    case Console::Handler::LineEnding::CarriageReturn:
      seq.append('\r');
      break;
    case Console::Handler::LineEnding::BothNewLineAndCarriageReturn:
      seq.append('\r');
      seq.append('\n');
      break;
  }

  return seq;
}

/**
 * @brief Maps editing, navigation, and F-keys to their VT-100 byte sequences.
 */
QByteArray Widgets::Terminal::translateSpecialKey(int key)
{
  QByteArray seq;
  switch (key) {
    case Qt::Key_Backspace:
      seq.append('\x7f');
      break;
    case Qt::Key_Delete:
      seq.append("\x1b[3~");
      break;
    case Qt::Key_Tab:
      seq.append('\t');
      break;
    case Qt::Key_Backtab:
      seq.append("\x1b[Z");
      break;
    case Qt::Key_Escape:
      seq.append('\x1b');
      break;
    case Qt::Key_Up:
      seq.append("\x1b[A");
      break;
    case Qt::Key_Down:
      seq.append("\x1b[B");
      break;
    case Qt::Key_Right:
      seq.append("\x1b[C");
      break;
    case Qt::Key_Left:
      seq.append("\x1b[D");
      break;
    case Qt::Key_Home:
      seq.append("\x1b[H");
      break;
    case Qt::Key_End:
      seq.append("\x1b[F");
      break;
    case Qt::Key_PageUp:
      seq.append("\x1b[5~");
      break;
    case Qt::Key_PageDown:
      seq.append("\x1b[6~");
      break;
    case Qt::Key_Insert:
      seq.append("\x1b[2~");
      break;
    case Qt::Key_F1:
      seq.append("\x1bOP");
      break;
    case Qt::Key_F2:
      seq.append("\x1bOQ");
      break;
    case Qt::Key_F3:
      seq.append("\x1bOR");
      break;
    case Qt::Key_F4:
      seq.append("\x1bOS");
      break;
    case Qt::Key_F5:
      seq.append("\x1b[15~");
      break;
    case Qt::Key_F6:
      seq.append("\x1b[17~");
      break;
    case Qt::Key_F7:
      seq.append("\x1b[18~");
      break;
    case Qt::Key_F8:
      seq.append("\x1b[19~");
      break;
    case Qt::Key_F9:
      seq.append("\x1b[20~");
      break;
    case Qt::Key_F10:
      seq.append("\x1b[21~");
      break;
    case Qt::Key_F11:
      seq.append("\x1b[23~");
      break;
    case Qt::Key_F12:
      seq.append("\x1b[24~");
      break;
    default:
      break;
  }

  return seq;
}

/**
 * @brief Emits terminalSizeChanged() whenever the widget is resized.
 */
void Widgets::Terminal::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
  // Forward to base class and notify QML when the size changes
  QuickPaintedItemCompat::geometryChange(newGeometry, oldGeometry);

  if (newGeometry.size() != oldGeometry.size())
    Q_EMIT terminalSizeChanged();
}

/**
 * @brief Gets the current cursor position within the terminal.
 *
 * @return A QPoint representing the current cursor position, in character
 *         coordinates.
 */
const QPoint& Widgets::Terminal::cursorPosition() const
{
  return m_cursorPosition;
}

//--------------------------------------------------------------------------------------------------
// Clipboard & selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Finds the character index within a line segment closest to a pixel X position.
 *
 * Iterates through the segment accumulating character widths. Returns the index
 * of the first character whose right edge exceeds @p pixelX, or the segment end
 * if @p pixelX is beyond all characters.
 *
 * @param line     The full text line.
 * @param segStart Start index of the segment within @p line.
 * @param segEnd   End index (exclusive) of the segment within @p line.
 * @param pixelX   The pixel X offset from the left border.
 * @return Character index within @p line, in [segStart, segEnd].
 */
int Widgets::Terminal::findCharAtPixelX(const QString& line,
                                        int segStart,
                                        int segEnd,
                                        int pixelX) const
{
  // Accumulate character widths until the target pixel offset is reached
  const QFontMetrics fm(m_font);
  int widthSum = 0;
  for (int j = segStart; j < segEnd; ++j) {
    const int charWidth = fm.horizontalAdvance(line[j]);
    if (widthSum + charWidth > pixelX)
      return j;

    widthSum += charWidth;
  }
  return segEnd;
}

/**
 * @brief Computes the pixel X position of a cursor column within a segment.
 *
 * Sums character widths from @p segStart up to (but not including) @p cursorCol,
 * capped at @p segEnd. The returned position is relative to the left border.
 *
 * @param painter   Active QPainter (used for font metrics).
 * @param line      Full text of the logical line.
 * @param segStart  Start character index of the current wrapped segment.
 * @param cursorCol Target cursor column (character index within the logical line).
 * @param segEnd    End character index of the current wrapped segment.
 * @return Pixel X position (including border) for the cursor.
 */
int Widgets::Terminal::calcCursorPixelX(
  QPainter* painter, const QString& line, int segStart, int cursorCol, int segEnd) const
{
  // Sum character widths from segment start up to the cursor column
  int cursorX     = m_borderX;
  const int limit = qMin(cursorCol, segEnd);
  for (int j = segStart; j < limit; ++j)
    cursorX += painter->fontMetrics().horizontalAdvance(line[j]);

  return cursorX;
}

/**
 * @brief Converts a pixel position to a cursor position.
 *
 * @param pos The pixel position within the terminal.
 * @return A QPoint representing the cursor position corresponding to the given
 *         pixel position.
 */
QPoint Widgets::Terminal::positionToCursor(const QPoint& pos) const
{
  // Map pixel coordinates to a logical (column, line) cursor position
  int localY     = (pos.y() - m_borderY) / m_cHeight;
  int remainingY = localY;

  if (localY < 0) {
    if (m_scrollOffsetY < m_data.size())
      return QPoint(0, m_scrollOffsetY);

    return QPoint(0, 0);
  }

  for (int i = m_scrollOffsetY; i < m_data.size(); ++i) {
    const QString& line = m_data[i];

    if (line.isEmpty()) {
      if (remainingY == 0)
        return QPoint(0, i);

      remainingY--;
    }

    else {
      int lines = (line.length() + maxCharsPerLine() - 1) / maxCharsPerLine();
      if (remainingY < lines) {
        int segmentStart = qMax(0, remainingY) * maxCharsPerLine();
        int segmentEnd   = qMin(segmentStart + maxCharsPerLine(), line.length());
        return QPoint(findCharAtPixelX(line, segmentStart, segmentEnd, pos.x() - m_borderX), i);
      }

      remainingY -= lines;
    }
  }

  if (!m_data.isEmpty()) {
    int lastLine = m_data.size() - 1;
    int lastChar = m_data.last().length();
    return QPoint(lastChar, lastLine);
  }

  return QPoint(0, 0);
}

/**
 * @brief Copies the currently selected text to the system clipboard.
 *
 * This function copies the currently selected text from the terminal buffer
 * to the system clipboard, ensuring the correct order of the start and end
 * points of the selection:
 * - If no valid selection is available (`copyAvailable()` returns true), the
 *   function returns without action.
 * - The correct selection boundaries are determined and adjusted if needed.
 * - Iterates over the selected lines and extracts the corresponding text,
 *   preserving line breaks.
 * - Copies the extracted text to the system clipboard for use in other
 *   applications.
 *
 * @note The copied text includes line breaks between lines to preserve
 * formatting.
 *
 * @see copyAvailable(), QClipboard, QGuiApplication::clipboard()
 */
void Widgets::Terminal::copy()
{
  if (!copyAvailable())
    return;

  // Normalize selection direction and extract text
  QString copiedText;
  QPoint start = m_selectionStart;
  QPoint end   = m_selectionEnd;

  if (start.y() > end.y() || (start.y() == end.y() && start.x() > end.x()))
    std::swap(start, end);

  for (int lineIndex = start.y(); lineIndex <= end.y(); ++lineIndex) {
    const QString& line = m_data[lineIndex];

    int startX = (lineIndex == start.y()) ? start.x() : 0;
    int endX   = (lineIndex == end.y()) ? end.x() : line.size();

    if (start.y() == end.y() && start.x() > end.x())
      std::swap(startX, endX);

    if (lineIndex != start.y() && lineIndex != end.y()) {
      startX = 0;
      endX   = line.size();
    }

    if (startX < endX)
      copiedText.append(line.mid(startX, endX - startX));

    if (lineIndex != end.y() || (startX == 0 && endX == line.size()))
      copiedText.append('\n');
  }

  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(copiedText);
}

/**
 * @brief Clears the terminal's content.
 *
 * Resets the data buffer, moves the cursor to the top-left position,
 * and enables autoscroll. This effectively clears the terminal's display.
 */
void Widgets::Terminal::clear()
{
  initBuffer();
  setCursorPosition(0, 0);
  setAutoscroll(true);
  m_stateChanged = true;
}

/**
 * @brief Selects all the text currently present in the terminal.
 *
 * Updates the selection state to encompass all text in the terminal, if
 * available. Emits the selectionChanged() signal to notify that the selection
 * has been updated.
 */
void Widgets::Terminal::selectAll()
{
  if (m_data.isEmpty())
    return;

  // Select entire buffer
  m_selectionStart       = QPoint(0, 0);
  int lastLineIndex      = m_data.size() - 1;
  int lastCharIndex      = m_data[lastLineIndex].size();
  m_selectionEnd         = QPoint(lastCharIndex, lastLineIndex);
  m_selectionStartCursor = m_selectionStart;

  m_stateChanged = true;
  Q_EMIT selectionChanged();
}

//--------------------------------------------------------------------------------------------------
// Style setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the font used for rendering the terminal text.
 *
 * @param font The QFont object to be used for terminal text.
 *
 * Updates the internal font, recalculates character dimensions, and adjusts the
 * terminal border accordingly. Emits the fontChanged() signal to notify about
 * the change.
 */
void Widgets::Terminal::setFont(const QFont& font)
{
  // Apply the font and recalculate character metrics
  m_font = font;
  m_font.setStyleStrategy(QFont::PreferAntialias);

  // Recalculate character metrics and border padding
  auto metrics = QFontMetrics(m_font);
  m_cHeight    = metrics.height();
  m_cWidth     = metrics.horizontalAdvance("M");
  m_borderX    = qMax(m_cWidth, m_cHeight) / 2;
  m_borderY    = qMax(m_cWidth, m_cHeight) / 2;

  Q_EMIT fontChanged();
}

/**
 * @brief Enables or disables autoscroll.
 *
 * @param enabled If true, autoscroll is enabled; otherwise, it is disabled.
 *
 * Changes the autoscroll behavior of the terminal and emits the
 * autoscrollChanged() signal.
 */
void Widgets::Terminal::setAutoscroll(const bool enabled)
{
  m_autoscroll = enabled;
  Q_EMIT autoscrollChanged();
}

/**
 * @brief Sets the vertical scroll offset for the terminal.
 *
 * @param offset The new vertical scroll offset value.
 *
 * If the scroll offset is changed, it updates the internal offset, emits the
 * scrollOffsetYChanged() signal, and triggers a redraw of the terminal.
 */
void Widgets::Terminal::setScrollOffsetY(const int offset)
{
  if (m_scrollOffsetY != offset) {
    m_scrollOffsetY = offset;
    Q_EMIT scrollOffsetYChanged();

    update();
  }
}

/**
 * @brief Sets the color palette used by the terminal.
 *
 * @param palette The QPalette object representing the new color palette.
 *
 * Updates the terminal's color palette and emits the colorPaletteChanged()
 * signal to indicate that the palette has been updated.
 */
void Widgets::Terminal::setColorPalette(const QPalette& palette)
{
  m_palette = palette;
  Q_EMIT colorPaletteChanged();
}

//--------------------------------------------------------------------------------------------------
// Emulation setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables VT-100 emulation.
 *
 * @param enabled If true, VT-100 emulation is enabled; otherwise, it is
 * disabled.
 *
 * Controls whether the terminal interprets VT-100 escape sequences for
 * additional terminal functionality. Emits the vt100EmulationChanged() signal
 * on change.
 */
void Widgets::Terminal::setVt100Emulation(const bool enabled)
{
  m_emulateVt100 = enabled;
  Q_EMIT vt100EmulationChanged();
}

/**
 * @brief Enables or disables ANSI color support.
 *
 * @param enabled If true, ANSI colors are enabled; otherwise, text uses the
 * default palette color.
 *
 * When enabled, the terminal interprets ANSI SGR escape sequences for text
 * coloring (codes 30-37 for foreground colors, 0 for reset, 1 for bold).
 * Emits the ansiColorsChanged() signal on change.
 */
void Widgets::Terminal::setAnsiColors(const bool enabled)
{
  // Toggle ANSI color mode and allocate color data when enabling
  m_ansiColors = enabled;

  if (enabled) {
    m_currentColor = QColor();
    m_colorData.reserve(MAX_LINES);
  }

  Q_EMIT ansiColorsChanged();
}

//--------------------------------------------------------------------------------------------------
// Cursor management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles the visibility of the cursor.
 *
 * Flips the visibility state of the cursor, which is typically used to create
 * a blinking cursor effect.
 */
void Widgets::Terminal::toggleCursor()
{
  m_stateChanged  = true;
  m_cursorVisible = !m_cursorVisible;
}

//--------------------------------------------------------------------------------------------------
// Theme management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the terminal's color palette when the theme changes.
 *
 * Retrieves the current theme colors from the ThemeManager and updates the
 * terminal's color palette accordingly. The new palette is used for rendering
 * various elements, such as text, background, buttons, and highlights.
 *
 * @note After updating the palette, the terminal is redrawn to reflect the
 *       changes.
 */
void Widgets::Terminal::onThemeChanged()
{
  // clang-format off
  m_stateChanged = true;
  const auto theme = &Misc::ThemeManager::instance();
  m_palette.setColor(QPalette::Text, theme->getColor("console_text"));
  m_palette.setColor(QPalette::Base, theme->getColor("console_base"));
  m_palette.setColor(QPalette::Window, theme->getColor("console_border"));
  m_palette.setColor(QPalette::Highlight, theme->getColor("console_highlight"));
  setFillColor(m_palette.color(QPalette::Base));
  updateAnsiColorPalette();
  // clang-format on

  // Reset current color so new unformatted text uses the new theme
  if (ansiColors())
    m_currentColor = QColor();

  update();
}

//--------------------------------------------------------------------------------------------------
// Welcome & help
//--------------------------------------------------------------------------------------------------

/**
 * @brief Displays the localized welcome guide in the terminal widget, without
 *        modifying the terminal output.
 */
void Widgets::Terminal::loadWelcomeGuide()
{
  // Define logo
  // clang-format off
  // code-verify off
  static const QString logo = \
    "▒█▀▀▀█ ▒█▀▀▀ ▒█▀▀█ ▀█▀ ▒█▀▀█ ▒█░░░   ▒█▀▀▀█ ▀▀█▀▀ ▒█░▒█ ▒█▀▀▄ ▀█▀ ▒█▀▀▀█\n" \
    "░▀▀▀▄▄ ▒█▀▀▀ ▒█▄▄▀ ▒█░ ▒█▄▄█ ▒█░░░   ░▀▀▀▄▄ ░▒█░░ ▒█░▒█ ▒█░▒█ ▒█░ ▒█░░▒█\n" \
    "▒█▄▄▄█ ▒█▄▄▄ ▒█░▒█ ▄█▄ ▒█░▒█ ▒█▄▄█   ▒█▄▄▄█ ░▒█░░ ░▀▄▄▀ ▒█▄▄▀ ▄█▄ ▒█▄▄▄█\n\n";
  // code-verify on
  // clang-format on

  clear();
  setAutoscroll(false);
  append(logo);
  append(Misc::Translator::instance().welcomeConsoleText());
  setAutoscroll(true);

  // Scroll so the welcome text is fully visible
  const int lines = linesPerPage();
  if (lines > 0 && height() > 0) {
    int cursorLine   = m_cursorPosition.y();
    int wrappedLines = 1;
    if (cursorLine < m_data.size()) {
      int lineLength = m_data[cursorLine].length();
      wrappedLines   = (lineLength + maxCharsPerLine() - 1) / maxCharsPerLine();
    }

    int visualBottom = cursorLine + wrappedLines - 1;
    setScrollOffsetY(qMax(0, visualBottom - lines + 1));
  }

  m_stateChanged = true;
}

//--------------------------------------------------------------------------------------------------
// Data append
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a string of data to the terminal, processing each character
 *        accordingly.
 *
 * @param data The string of data to be appended to the terminal.
 *
 * This method processes each character in the provided data string,
 * interpreting escape sequences, formatting commands, and resetting font styles
 * based on the terminal's current state (Text, Escape, Format, ResetFont).
 *
 * The processed text is accumulated and then appended to the terminal's buffer.
 *
 * @see processText(), processEscape(), processFormat(), processResetFont(),
 * appendString()
 */
/** @brief Scans a printable-character run starting at @p pos in @p data; returns end offset. */
int Widgets::Terminal::scanPrintableRun(const QString& data, int pos)
{
  const int len = data.size();
  while (pos < len) {
    const auto ch = data[pos].unicode();

    // Break on control chars that processText() handles specially
    if (ch == 0x1b || ch == '\n' || ch == '\r' || ch == '\b' || ch == 0x7F || ch == '\t')
      break;

    // Non-printable -> replaced with space by replaceData anyway
    if (ch < 0x20)
      break;

    ++pos;
  }

  return pos;
}

void Widgets::Terminal::append(const QString& data)
{
  // Process each character through the VT-100 state machine
  QString text;
  text.reserve(data.size());

  int pos       = 0;
  const int len = data.size();

  while (pos < len) {
    // Fast path: scan runs of printable chars without per-char state dispatch
    if (m_state == Text) [[likely]] {
      const int runStart = pos;
      pos                = scanPrintableRun(data, pos);

      // Bulk-append the printable run
      if (pos > runStart)
        text.append(QStringView(data).mid(runStart, pos - runStart));

      // Process the control character that ended the run
      if (pos < len && m_state == Text) {
        processText(data[pos], text);
        ++pos;
      }

      continue;
    }

    // Slow path: non-Text states (Escape, Format, etc.)
    const auto byte = data[pos];
    switch (m_state) {
      case Escape:
        processEscape(byte, text);
        break;
      case Format:
        processFormat(byte, text);
        break;
      case ResetFont:
        processResetFont(byte, text);
        break;
      case OSC:
        processOsc(byte);
        break;
      case IgnoreSeq:
        processIgnoreSeq(byte);
        break;
      default:
        break;
    }

    ++pos;
  }

  appendString(text);
  m_stateChanged = true;
}

//--------------------------------------------------------------------------------------------------
// Buffer management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a string to the terminal's data buffer, updating the cursor
 *        position.
 *
 * @param string The QString to be appended to the terminal.
 *
 * This method processes each character in the given string by:
 * - Registering it in the terminal's internal buffer at the current cursor
 *   position.
 * - Moving the cursor to the right after each character is placed.
 *
 * If autoscroll is enabled, the vertical scroll offset (`scrollOffsetY`) is
 * adjusted to ensure that the cursor remains visible, and
 * `scrollOffsetYChanged()` is emitted to notify of any changes.
 *
 * @see replaceData(), setCursorPosition(), autoscroll()
 */
void Widgets::Terminal::appendString(QStringView string)
{
  // Ensure buffer memory does not exceed MAX_LINES
  const int linesToDrop = m_data.size() - MAX_LINES + 1;
  if (m_data.size() >= MAX_LINES && linesToDrop > 0) {
    m_data.erase(m_data.begin(), m_data.begin() + linesToDrop);
    if (ansiColors() && m_colorData.size() >= linesToDrop)
      m_colorData.erase(m_colorData.begin(), m_colorData.begin() + linesToDrop);

    if (m_cursorPosition.y() >= linesToDrop)
      m_cursorPosition.setY(m_cursorPosition.y() - linesToDrop);
    else
      m_cursorPosition.setY(0);

    if (m_scrollOffsetY >= linesToDrop)
      m_scrollOffsetY -= linesToDrop;
    else
      m_scrollOffsetY = 0;
  }

  // Register each character in the provided string
  for (const auto& character : string) {
    int cursorX = m_cursorPosition.x();
    int cursorY = m_cursorPosition.y();
    replaceData(cursorX, cursorY, character);
    setCursorPosition(cursorX + 1, cursorY);

    if (m_cursorPosition.x() >= maxCharsPerLine())
      setCursorPosition(0, m_cursorPosition.y() + 1);
  }

  // Adjust the scroll offset if autoscroll is enabled
  if (autoscroll()) {
    // Calculate the total number of wrapped lines for the current line
    int cursorLine   = m_cursorPosition.y();
    int wrappedLines = 1;
    if (cursorLine < m_data.size()) {
      int lineLength = m_data[cursorLine].length();
      wrappedLines   = (lineLength + maxCharsPerLine() - 1) / maxCharsPerLine();
    }

    // Calculate the visual bottom of the wrapped line
    int visualBottom = cursorLine + wrappedLines - 1;

    // Set the scroll offset to ensure the bottom of the wrapped line is visible
    m_scrollOffsetY = qMax(0, visualBottom - linesPerPage() + 1);
    if (isVisible())
      Q_EMIT scrollOffsetYChanged();
  }
}

/**
 * @brief Removes characters from the terminal buffer starting from the cursor
 *        position.
 *
 * @param direction The direction to remove characters: either LeftDirection or
 *                  RightDirection.
 *
 * @param len The number of characters to remove. Defaults to INT_MAX if a
 *            negative value is provided.
 *
 * This method removes characters from the terminal buffer either to the left or
 * right of the current cursor position:
 *
 * - If `direction` is `RightDirection`, it removes characters starting from the
 *   cursor and moving to the right, up to the specified length or the end of
 *   theline.
 * - If `direction` is `LeftDirection`, it removes characters to the left of the
 *   cursor, up to the specified length.
 *
 * Characters removed are replaced with a clear character (`'\x7F'`).
 *
 * @see replaceData(), setCursorPosition()
 */
void Widgets::Terminal::removeStringFromCursor(const Direction direction, int len)
{
  // Obtain (x, y) position
  const auto positionX = m_cursorPosition.x();
  const auto positionY = m_cursorPosition.y();

  // Ensure valid length
  if (len < 0)
    len = INT_MAX;

  // Cap bytes to remove (right)
  int removeSize = 0;
  if (direction == RightDirection) {
    qsizetype l1 = m_data[positionY].size() - positionX;
    qsizetype l2 = static_cast<qsizetype>(len);
    removeSize   = qMin(l1, l2);
  }

  // Cap bytes to remove (left)
  else
    removeSize = qMin(len, m_cursorPosition.x());

  // Removal operation
  int offset = 0;
  const QChar clearChar('\x7F');
  for (int i = 0; i < removeSize; ++i) {
    // Get offset depending on removal direction
    if (direction == LeftDirection)
      offset = -1;
    else if (direction == RightDirection)
      offset = i;

    // Replace data in console screen
    replaceData(m_cursorPosition.x() + offset, positionY, clearChar);
  }
}

/**
 * @brief Initializes the terminal's data buffer.
 *
 * Clears the existing data buffer and reservers memory for the scrollback.
 *
 * This function is typically used to reset the terminal state, ensuring
 * efficient memory management for upcoming operations.
 */
void Widgets::Terminal::initBuffer()
{
  m_data.clear();
  m_data.squeeze();
  m_scrollOffsetY = 0;
  m_data.reserve(MAX_LINES);
  m_colorData.clear();
  m_colorData.squeeze();

  // Pre-allocate color memory only when ANSI mode is active
  if (ansiColors()) {
    m_colorData.reserve(MAX_LINES);
    m_currentColor = QColor();
  }
}

//--------------------------------------------------------------------------------------------------
// ANSI processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a single character in the context of normal text input.
 *
 * @param byte The character to be processed.
 * @param text A reference to a QString that accumulates printable characters.
 *
 * This method handles normal text input processing, managing different
 * character cases:
 * - ESC (0x1B) with VT-100 on -> flush text and enter Escape state.
 * - CR (`\r`) with VT-100 on -> move cursor to column 0.
 * - LF (`\n`) -> move cursor to next line.
 * - BS (`\b`) with VT-100 on -> move cursor left one column.
 * - DEL (0x7F) with VT-100 on -> delete character under cursor.
 * - TAB (`\t`) with VT-100 on -> advance to next 8-column tab stop.
 * - Printable characters -> append to accumulated @p text.
 *
 * This function helps manage cursor positioning and character input depending
 * on the current state.
 *
 * @see appendString(), setCursorPosition(), vt100emulation()
 */
void Widgets::Terminal::processText(const QChar& byte, QString& text)
{
  const ushort code = byte.unicode();
  const bool vt     = vt100emulation();

  // LF is always handled regardless of VT-100 mode
  if (code == '\n') {
    appendString(text);
    text.clear();
    setCursorPosition(0, m_cursorPosition.y() + 1);
    return;
  }

  // Without VT-100 emulation only printable characters are accumulated
  if (!vt) {
    if (byte.isPrint())
      text.append(byte);

    return;
  }

  // VT-100 control bytes
  switch (code) {
    case 0x1b:
      appendString(text);
      text.clear();
      m_state = Escape;
      return;
    case '\r':
      appendString(text);
      text.clear();
      setCursorPosition(0, m_cursorPosition.y());
      return;
    case '\b':
      if (m_cursorPosition.x() == 0)
        return;

      appendString(text);
      text.clear();
      setCursorPosition(m_cursorPosition.x() - 1, m_cursorPosition.y());
      return;
    case 0x7F:
      appendString(text);
      text.clear();
      removeStringFromCursor(RightDirection, 1);
      return;
    case '\t': {
      appendString(text);
      text.clear();
      const int nextTab = (m_cursorPosition.x() / 8 + 1) * 8;
      const int spaces  = nextTab - m_cursorPosition.x();
      text.fill(' ', spaces);
      appendString(text);
      text.clear();
      return;
    }
    default:
      break;
  }

  if (byte.isPrint())
    text.append(byte);
}

/**
 * @brief Processes the character immediately following ESC (0x1B).
 *
 * @param byte The character to be processed as part of the escape sequence.
 * @param text Accumulated printable text (flushed before entering this state).
 *
 * Dispatches to the correct follow-on state or performs a single-character
 * ESC action:
 * - `[` -> CSI (Format state for cursor/SGR sequences)
 * - `(` -> Designate character set (ResetFont state, consumed silently)
 * - `]` -> OSC string (title, colour palette, etc.)
 * - `7` -> Save cursor position (DECSC)
 * - `8` -> Restore cursor position (DECRC)
 * - `M` -> Reverse index (scroll up one line)
 * - All others -> silently ignored, return to Text state
 *
 * @see processFormat(), m_state
 */
void Widgets::Terminal::processEscape(const QChar& byte, QString& text)
{
  (void)text;

  m_formatValues.clear();
  m_currentFormatValue = 0;
  m_privateMode        = false;

  switch (byte.toLatin1()) {
    case '[':
      m_state = Format;
      return;
    case '(':
      m_state = ResetFont;
      return;
    case ']':
      m_state = OSC;
      return;
    case '7':
      m_savedCursorPosition = m_cursorPosition;
      m_state               = Text;
      return;
    case '8':
      setCursorPosition(m_savedCursorPosition);
      m_state = Text;
      return;
    case 'M':
      setCursorPosition(m_cursorPosition.x(), qMax(0, m_cursorPosition.y() - 1));
      m_state = Text;
      return;
    default:
      m_state = Text;
      return;
  }
}

/**
 * @brief Processes one byte of a CSI (ESC[...) parameter or final sequence.
 *
 * @param byte The character to process.
 * @param text Accumulated printable text (unused here; passed for API symmetry).
 *
 * Accumulates numeric parameters separated by `;`, then dispatches on the
 * final letter:
 * - `m`     -- SGR: apply ANSI colors/attributes
 * - `A`-`F` -- cursor movement (up/down/right/left/next-line/prev-line)
 * - `H`/`f` -- cursor position (CUP / HVP), 1-based row;col
 * - `G`     -- cursor horizontal absolute (1-based column)
 * - `d`     -- cursor vertical absolute (1-based row)
 * - `J`     -- erase in display (0=to end, 1=to start, 2/3=full clear)
 * - `K`     -- erase in line (0=to end, 1=to start, 2=whole line)
 * - `P`     -- delete characters
 * - `s`/`u` -- save/restore cursor position (ANSI)
 * - `h`/`l` -- DEC private mode set/reset (`?25h` show cursor, `?25l` hide)
 * - All other final letters -> silently consumed, return to Text state
 *
 * @see setCursorPosition(), removeStringFromCursor(), applyAnsiColor()
 */
void Widgets::Terminal::processFormat(const QChar& byte, QString& text)
{
  (void)text;

  // Obtain format value
  if (byte >= '0' && byte <= '9') {
    m_currentFormatValue = m_currentFormatValue * 10 + (byte.cell() - '0');
    return;
  }

  // DEC private mode prefix -- silently note it, stay in Format state
  if (byte == '?' || byte == '>' || byte == '=') {
    m_privateMode = true;
    return;
  }

  // Semicolon: store current value and prepare for next
  if (byte == ';') {
    m_formatValues.append(m_currentFormatValue);
    m_currentFormatValue = 0;
    m_state              = Format;
    return;
  }

  // Final-byte dispatch; unknown finals fall through to the silent-letter rule
  if (dispatchCsiFinal(byte))
    return;

  m_state = Text;
}

/** @brief Dispatches the final byte of a CSI sequence; returns true if handled. */
bool Widgets::Terminal::dispatchCsiFinal(const QChar& byte)
{
  const char final = byte.toLatin1();
  switch (final) {
    case 'm':
      if (!m_privateMode) {
        m_formatValues.append(m_currentFormatValue);
        if (ansiColors())
          applyAnsiColor(m_formatValues);
      }

      m_state = Text;
      return true;

    case 's':
      if (!m_privateMode)
        m_savedCursorPosition = m_cursorPosition;

      m_state = Text;
      return true;

    case 'u':
      if (!m_privateMode)
        setCursorPosition(m_savedCursorPosition);

      m_state = Text;
      return true;

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      handleCsiCursorMove(final);
      m_state = Text;
      return true;

    case 'H':
    case 'f':
    case 'G':
    case 'd':
      handleCsiCursorAbsolute(final);
      m_state = Text;
      return true;

    case 'J':
      handleCsiEraseDisplay();
      m_state = Text;
      return true;

    case 'K':
      handleCsiEraseLine();
      m_state = Text;
      return true;

    case 'P':
      if (!m_privateMode) {
        removeStringFromCursor(LeftDirection, m_currentFormatValue);
        removeStringFromCursor(RightDirection);
      }

      m_state = Text;
      return true;

    case 'h':
    case 'l':
      handleCsiDecPrivateMode(byte);
      m_state = Text;
      return true;

    default:
      // Any other uppercase/lowercase letter terminates the sequence silently
      if ((final >= 'A' && final <= 'Z') || (final >= 'a' && final <= 'z')) {
        m_state = Text;
        return true;
      }

      return false;
  }
}

/** @brief Handles CSI cursor movement letters A-F (CUU/CUD/CUF/CUB/CNL/CPL). */
void Widgets::Terminal::handleCsiCursorMove(char final)
{
  if (m_privateMode)
    return;

  const int value = m_currentFormatValue ? m_currentFormatValue : 1;
  const int cx    = m_cursorPosition.x();
  const int cy    = m_cursorPosition.y();
  switch (final) {
    case 'A':
      setCursorPosition(cx, qMax(0, cy - value));
      break;
    case 'B':
      setCursorPosition(cx, cy + value);
      break;
    case 'C':
      setCursorPosition(cx + value, cy);
      break;
    case 'D':
      setCursorPosition(qMax(0, cx - value), cy);
      break;
    case 'E':
      setCursorPosition(0, cy + value);
      break;
    case 'F':
      setCursorPosition(0, qMax(0, cy - value));
      break;
    default:
      break;
  }
}

/** @brief Handles CSI absolute cursor placement (H/f/G/d). */
void Widgets::Terminal::handleCsiCursorAbsolute(char final)
{
  if (m_privateMode)
    return;

  if (final == 'H' || final == 'f') {
    const int row = qMax(0, m_formatValues.value(0, 1) - 1);
    const int arg =
      m_currentFormatValue > 0 ? m_currentFormatValue - 1 : m_formatValues.value(1, 1) - 1;
    setCursorPosition(qMax(0, arg), row);
    return;
  }

  const int v = qMax(0, m_currentFormatValue > 0 ? m_currentFormatValue - 1 : 0);
  if (final == 'G')
    setCursorPosition(v, m_cursorPosition.y());

  else if (final == 'd')
    setCursorPosition(m_cursorPosition.x(), v);
}

/** @brief Handles CSI Erase-in-Display (J). */
void Widgets::Terminal::handleCsiEraseDisplay()
{
  if (m_privateMode)
    return;

  const int cy = m_cursorPosition.y();
  switch (m_currentFormatValue) {
    case 0:
      // Erase from cursor to end of screen
      removeStringFromCursor(RightDirection);
      if (cy + 1 < m_data.size()) {
        m_data.erase(m_data.begin() + cy + 1, m_data.end());
        if (ansiColors() && cy + 1 < m_colorData.size())
          m_colorData.erase(m_colorData.begin() + cy + 1, m_colorData.end());
      }

      break;
    case 1:
      // Erase from start of screen to cursor
      removeStringFromCursor(LeftDirection);
      if (cy > 0) {
        m_data.erase(m_data.begin(), m_data.begin() + cy);
        if (ansiColors() && cy < m_colorData.size())
          m_colorData.erase(m_colorData.begin(), m_colorData.begin() + cy);
      }

      setCursorPosition(m_cursorPosition.x(), 0);
      break;
    case 2:
    case 3:
      clear();
      break;
    default:
      break;
  }
}

/** @brief Handles CSI Erase-in-Line (K). */
void Widgets::Terminal::handleCsiEraseLine()
{
  if (m_privateMode)
    return;

  switch (m_currentFormatValue) {
    case 0:
      removeStringFromCursor(RightDirection);
      break;
    case 1:
      removeStringFromCursor(LeftDirection);
      break;
    case 2:
      removeStringFromCursor(RightDirection);
      removeStringFromCursor(LeftDirection);
      break;
    default:
      break;
  }
}

/** @brief Handles CSI DEC private mode set/reset for cursor visibility (h/l). */
void Widgets::Terminal::handleCsiDecPrivateMode(const QChar& byte)
{
  if (m_privateMode && m_currentFormatValue == 25) {
    m_cursorHidden = (byte == 'l');
    m_stateChanged = true;
  }
}

/**
 * @brief Processes a reset font command in the terminal's state machine.
 *
 * @param byte The character to be processed (unused in this method).
 * @param text A reference to a QString (unused in this method).
 *
 * This method simply resets the terminal's state back to `Text`, ending any
 * font reset operations. This is typically used to handle the completion of
 * a font reset escape sequence.
 *
 * @see m_state
 */
void Widgets::Terminal::processResetFont(const QChar& byte, QString& text)
{
  (void)byte;
  (void)text;
  m_state = Text;
}

/** @brief Consumes one byte while in OSC state (BEL terminator or ESC -> CSI). */
void Widgets::Terminal::processOsc(const QChar& byte)
{
  const char latin = byte.toLatin1();
  if (latin == 0x07)
    m_state = Text;

  else if (latin == 0x1b)
    m_state = Escape;
}

/** @brief Consumes one byte while ignoring an unknown CSI sequence. */
void Widgets::Terminal::processIgnoreSeq(const QChar& byte)
{
  if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z'))
    m_state = Text;
}

//--------------------------------------------------------------------------------------------------
// Color management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the ANSI color palette based on the current theme.
 *
 * Determines whether to use light or dark ANSI colors based on the background
 * color luminance. Uses standard UNIX/Linux terminal color palette.
 *
 * Light themes (dark background) use bright colors, dark themes (light
 * background) use darker colors for better contrast.
 */
void Widgets::Terminal::updateAnsiColorPalette()
{
  const auto theme         = &Misc::ThemeManager::instance();
  const QColor consoleBase = theme->getColor("console_base");
  const QColor consoleText = theme->getColor("console_text");
  const bool isDarkTheme   = consoleText.lightness() > consoleBase.lightness();

  // Dark-theme palette; index order: 0=Black 1=Red 2=Green 3=Yellow 4=Blue 5=Magenta 6=Cyan 7=White
  if (isDarkTheme) {
    // Standard ANSI colors (dark theme)
    m_ansiStandardColors[0] = QColor(0, 0, 0);
    m_ansiStandardColors[1] = QColor(205, 49, 49);
    m_ansiStandardColors[2] = QColor(13, 188, 121);
    m_ansiStandardColors[3] = QColor(229, 229, 16);
    m_ansiStandardColors[4] = QColor(36, 114, 200);
    m_ansiStandardColors[5] = QColor(188, 63, 188);
    m_ansiStandardColors[6] = QColor(17, 168, 205);
    m_ansiStandardColors[7] = QColor(229, 229, 229);

    // Bright ANSI colors (dark theme); index 0 is gray (bright black)
    m_ansiBrightColors[0] = QColor(102, 102, 102);
    m_ansiBrightColors[1] = QColor(241, 76, 76);
    m_ansiBrightColors[2] = QColor(35, 209, 139);
    m_ansiBrightColors[3] = QColor(245, 245, 67);
    m_ansiBrightColors[4] = QColor(59, 142, 234);
    m_ansiBrightColors[5] = QColor(214, 112, 214);
    m_ansiBrightColors[6] = QColor(41, 184, 219);
    m_ansiBrightColors[7] = QColor(255, 255, 255);
  }

  // Light-theme palette; same index order as the dark palette above
  else {
    // Standard ANSI colors (light theme)
    m_ansiStandardColors[0] = QColor(0, 0, 0);
    m_ansiStandardColors[1] = QColor(170, 0, 0);
    m_ansiStandardColors[2] = QColor(0, 140, 0);
    m_ansiStandardColors[3] = QColor(170, 140, 0);
    m_ansiStandardColors[4] = QColor(0, 0, 170);
    m_ansiStandardColors[5] = QColor(170, 0, 170);
    m_ansiStandardColors[6] = QColor(0, 140, 170);
    m_ansiStandardColors[7] = QColor(170, 170, 170);

    // Bright ANSI colors (light theme); index 0 is gray (bright black)
    m_ansiBrightColors[0] = QColor(85, 85, 85);
    m_ansiBrightColors[1] = QColor(210, 0, 0);
    m_ansiBrightColors[2] = QColor(0, 170, 0);
    m_ansiBrightColors[3] = QColor(210, 170, 0);
    m_ansiBrightColors[4] = QColor(0, 0, 210);
    m_ansiBrightColors[5] = QColor(210, 0, 210);
    m_ansiBrightColors[6] = QColor(0, 170, 210);
    m_ansiBrightColors[7] = QColor(85, 85, 85);
  }
}

/**
 * @brief Applies ANSI SGR (Select Graphic Rendition) color codes.
 *
 * @param codes List of ANSI SGR codes to apply.
 *
 * Supports:
 * - 0: Reset to default colors
 * - 1: Bold/bright (makes current color brighter)
 * - 30-37: Standard foreground colors
 * - 40-47: Standard background colors
 * - 90-97: Bright foreground colors
 * - 100-107: Bright background colors
 * - 38;5;N: 256-color foreground (N = 0-255)
 * - 48;5;N: 256-color background (N = 0-255)
 * - 38;2;R;G;B: RGB foreground
 * - 48;2;R;G;B: RGB background
 */
void Widgets::Terminal::applyAnsiColor(const QList<int>& codes)
{
  for (int i = 0; i < codes.size(); ++i)
    i += applyAnsiSgrCode(codes, i);
}

/** @brief Applies one SGR code at @p i in @p codes; returns number of extra params consumed. */
int Widgets::Terminal::applyAnsiSgrCode(const QList<int>& codes, int i)
{
  const int code = codes[i];

  // Reset to default colors (invalid QColor = use theme default at render)
  if (code == 0) {
    m_currentColor   = QColor();
    m_currentBgColor = QColor();
    return 0;
  }

  // Bold/bright - make current color lighter
  if (code == 1) {
    if (!m_currentColor.isValid())
      m_currentColor = m_palette.color(QPalette::Text);

    m_currentColor = m_currentColor.lighter(130);
    return 0;
  }

  // Standard foreground colors (30-37)
  if (code >= 30 && code <= 37) {
    m_currentColor = m_ansiStandardColors[code - 30];
    return 0;
  }

  // Standard background colors (40-47)
  if (code >= 40 && code <= 47) {
    m_currentBgColor = m_ansiStandardColors[code - 40];
    return 0;
  }

  // Bright foreground colors (90-97)
  if (code >= 90 && code <= 97) {
    m_currentColor = m_ansiBrightColors[code - 90];
    return 0;
  }

  // Bright background colors (100-107)
  if (code >= 100 && code <= 107) {
    m_currentBgColor = m_ansiBrightColors[code - 100];
    return 0;
  }

  // Extended color codes 38 (foreground) and 48 (background)
  const bool isFg = (code == 38);
  const bool isBg = (code == 48);
  if (!isFg && !isBg)
    return 0;

  QColor& target = isFg ? m_currentColor : m_currentBgColor;

  // 256-color: 38;5;N or 48;5;N
  if (i + 2 < codes.size() && codes[i + 1] == 5) {
    target = getColor256(codes[i + 2]);
    return 2;
  }

  // RGB: 38;2;R;G;B or 48;2;R;G;B
  if (i + 4 < codes.size() && codes[i + 1] == 2) {
    target = QColor(codes[i + 2], codes[i + 3], codes[i + 4]);
    return 4;
  }

  return 0;
}

/**
 * @brief Converts a 256-color palette index to a QColor.
 *
 * @param index Color index (0-255).
 * @return QColor corresponding to the index.
 *
 * Color ranges:
 * - 0-7: Standard colors
 * - 8-15: Bright colors
 * - 16-231: 6x6x6 RGB cube
 * - 232-255: Grayscale ramp
 */
QColor Widgets::Terminal::getColor256(int index) const
{
  return getColor256Static(index);
}

/**
 * @brief Static version of getColor256 for use without instance.
 */
QColor Widgets::Terminal::getColor256Static(int index)
{
  // Standard colors (0-7)
  if (index < 8) {
    static const QColor standard[8] = {
      QColor(0, 0, 0),        // Black
      QColor(170, 0, 0),      // Red
      QColor(0, 170, 0),      // Green
      QColor(170, 85, 0),     // Yellow
      QColor(0, 0, 170),      // Blue
      QColor(170, 0, 170),    // Magenta
      QColor(0, 170, 170),    // Cyan
      QColor(170, 170, 170),  // White
    };
    return standard[index];
  }

  // Bright colors (8-15)
  if (index < 16) {
    static const QColor bright[8] = {
      QColor(85, 85, 85),     // Bright Black
      QColor(255, 85, 85),    // Bright Red
      QColor(85, 255, 85),    // Bright Green
      QColor(255, 255, 85),   // Bright Yellow
      QColor(85, 85, 255),    // Bright Blue
      QColor(255, 85, 255),   // Bright Magenta
      QColor(85, 255, 255),   // Bright Cyan
      QColor(255, 255, 255),  // Bright White
    };
    return bright[index - 8];
  }

  // 216-color RGB cube (16-231): 16 + 36*r + 6*g + b
  if (index < 232) {
    const int adjusted = index - 16;
    const int r        = (adjusted / 36) % 6;
    const int g        = (adjusted / 6) % 6;
    const int b        = adjusted % 6;

    return QColor(r ? (r * 40 + 55) : 0, g ? (g * 40 + 55) : 0, b ? (b * 40 + 55) : 0);
  }

  // Grayscale ramp (232-255): 24 steps
  const int gray = 8 + (index - 232) * 10;
  return QColor(gray, gray, gray);
}

/**
 * @brief Formats a debug message with optional ANSI colors.
 *
 * @param type Message type (QtDebugMsg, QtWarningMsg, etc.).
 * @param message The message content.
 * @param useAnsiColors Whether to include ANSI color codes.
 * @return Formatted message string.
 *
 * This static function provides consistent debug message formatting
 * across the application, with optional ANSI color codes for terminal
 * output.
 */
QString Widgets::Terminal::formatDebugMessage(QtMsgType type,
                                              const QString& message,
                                              bool useAnsiColors)
{
  QString prefix;
  QString ansiColor;
  QString ansiReset;

  if (useAnsiColors)
    ansiReset = QStringLiteral("\033[0m");

  switch (type) {
    case QtInfoMsg:
      prefix = QStringLiteral("[INFO]");
      if (useAnsiColors)
        ansiColor = QStringLiteral("\033[36m");

      break;

    case QtDebugMsg:
      prefix = QStringLiteral("[DEBG]");
      if (useAnsiColors)
        ansiColor = QStringLiteral("\033[32m");

      break;

    case QtWarningMsg:
      prefix = QStringLiteral("[WARN]");
      if (useAnsiColors)
        ansiColor = QStringLiteral("\033[33m");

      break;

    case QtCriticalMsg:
      prefix = QStringLiteral("[CRIT]");
      if (useAnsiColors)
        ansiColor = QStringLiteral("\033[31m");

      break;

    case QtFatalMsg:
      prefix = QStringLiteral("[FATL]");
      if (useAnsiColors)
        ansiColor = QStringLiteral("\033[91m");

      break;

    default:
      break;
  }

  if (useAnsiColors)
    return QStringLiteral("%1%2 %3%4").arg(ansiColor, prefix, message, ansiReset);
  else
    return QStringLiteral("%1 %2").arg(prefix, message);
}

/**
 * @brief Sets the cursor position to a specified point.
 *
 * @param position The new cursor position as a QPoint object.
 *
 * Updates the cursor position to the specified point if it differs from the
 * current position, and emits the cursorMoved() signal to indicate the change.
 *
 * @see cursorMoved()
 */
void Widgets::Terminal::setCursorPosition(const QPoint& position)
{
  const QPoint clamped(position.x(), qBound(0, position.y(), MAX_LINES));
  if (m_cursorPosition != clamped) {
    m_cursorPosition = clamped;
    Q_EMIT cursorMoved();
  }
}

/**
 * @brief Sets the cursor position to specified coordinates.
 *
 * @param x The new x-coordinate of the cursor.
 * @param y The new y-coordinate of the cursor.
 *
 * Calls the overloaded `setCursorPosition()` function with a QPoint constructed
 * from the provided coordinates.
 *
 * @see setCursorPosition(const QPoint)
 */
void Widgets::Terminal::setCursorPosition(const int x, const int y)
{
  setCursorPosition(QPoint(x, y));
}

/**
 * @brief Replaces or inserts a character in the terminal buffer at a specified
 * position.
 *
 * @param x The x-coordinate (column) where the character should be replaced or
 *          inserted.
 * @param y The y-coordinate (line) where the character should be replaced or
 *          inserted.
 * @param byte The character to be placed at the specified position.
 *
 * This method ensures that:
 * - The buffer is resized to accommodate the line specified by `y` if it does
 *   not already exist.
 * - The line at `y` is long enough to hold the character at position `x`,
 *   padding with spaces if necessary.
 * - The character (`byte`) is printable; otherwise, it is replaced with a dot.
 *
 * If the position `x` is within the length of the current line, the character
 * is replaced. If `x` exceeds the current length, the character is appended to
 * the line.
 *
 * @note Non-printable characters are replaced with a dot (`'.'`).
 *
 * @see lineCount(), m_data
 */
void Widgets::Terminal::replaceData(int x, int y, const QChar& byte)
{
  if (y >= m_data.size())
    m_data.resize(y + 1);

  QString& line = m_data[y];

  // Maintain per-character color data when ANSI mode is active
  if (ansiColors()) {
    if (y >= m_colorData.size())
      m_colorData.resize(y + 1);

    QList<CharColor>& colorLine = m_colorData[y];

    if (x > line.size()) {
      const int padCount = x - line.size();
      for (int i = 0; i < padCount; ++i)
        colorLine.append(CharColor());
    }

    while (colorLine.size() < line.size())
      colorLine.append(CharColor());

    const CharColor charColor(m_currentColor, m_currentBgColor);
    if (x >= 0 && x < colorLine.size())
      colorLine[x] = charColor;
    else if (x >= 0)
      colorLine.append(charColor);
  }

  if (x > line.size())
    line = line.leftJustified(x, ' ');

  if (x >= 0 && x < line.size())
    line[x] = byte.isPrint() ? byte : ' ';
  else if (x >= 0)
    line.append(byte.isPrint() ? byte : ' ');
}

/**
 * @brief Determines whether a given character should end a text selection.
 *
 * This function is used to decide if a given character (`c`) marks the boundary
 * for ending a text selection operation, such as when selecting a word in a
 * terminal. The selection ends if the character is a space, a non-character, or
 * not a letter or number.
 *
 * @param c The character to evaluate.
 * @return true if the character should end the selection, false otherwise.
 *
 * The selection will end if:
 * 1. `c` is a space character (e.g., space, tab).
 * 2. `c` is a non-character, which includes control characters or other
 *    special non-printable characters.
 * 3. `c` is neither a letter nor a number, meaning punctuation marks, symbols,
 *    and other non-alphanumeric characters will end the selection.
 */
bool Widgets::Terminal::shouldEndSelection(const QChar& c)
{
  bool end = false;
  end |= c.isSpace();
  end |= c.isNonCharacter();
  end |= (!c.isLetter() && !c.isNumber());
  return end;
}

//--------------------------------------------------------------------------------------------------
// Input event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel events for scrolling the terminal content.
 *
 * @param event A pointer to the QWheelEvent object containing details of the
 * event.
 *
 * This method manages vertical scrolling of the terminal content when the mouse
 * wheel is used:
 * - Determines the number of steps to scroll based on the delta values in the
 *   wheel event.
 * - Converts wheel steps to line scrolling steps and adjusts the scroll offset
 *   accordingly.
 * - If scrolling up, autoscroll is disabled to prevent the view from
 *   auto-resetting.
 * - Ensures that the scroll offset remains within valid bounds, and re-enables
 *   autoscroll if the end of the content is reached.
 *
 * @note This method is responsible for maintaining a smooth and user-friendly
 * scrolling experience, including handling scenarios where autoscroll is
 * temporarily disabled.
 *
 * @see setScrollOffsetY(), setAutoscroll(), linesPerPage(), lineCount()
 */
void Widgets::Terminal::wheelEvent(QWheelEvent* event)
{
  // Calculate the number of steps
  int numSteps    = 0;
  auto pixelDelta = event->pixelDelta();
  auto angleDelta = event->angleDelta();
  if (!pixelDelta.isNull())
    numSteps = pixelDelta.y();
  else
    numSteps = angleDelta.y();

  // Convert steps to lines
  if (numSteps > 0)
    numSteps = qMax(1, numSteps / m_cHeight);
  else if (numSteps < 0)
    numSteps = qMin(-1, numSteps / m_cHeight);

  // Disable auto scroll when scrolling up
  if (numSteps > 0 && autoscroll() && linesPerPage() < lineCount())
    setAutoscroll(false);

  // Update scroll offset by the number of lines, each step scrolls a few lines
  if (numSteps != 0) {
    // Calculate maximum lines that can be displayed in the viewport
    const int maxScrollOffset = qMax(0, lineCount() - linesPerPage() + 2);

    // Calculate the new scroll offset
    int offset = m_scrollOffsetY - numSteps;

    // Clamp the offset to stay within valid bounds
    offset = qMax(0, offset);
    offset = qMin(offset, maxScrollOffset);

    // Re-enable autoscroll
    if (offset == maxScrollOffset && !autoscroll())
      setAutoscroll(true);

    // Update offset
    setScrollOffsetY(offset);
  }

  m_stateChanged = true;
  event->accept();
}

/**
 * @brief Handles mouse move events for updating the text selection.
 *
 * @param event A pointer to the QMouseEvent object containing details of the
 * event.
 *
 * If mouse tracking is enabled and there is an active selection
 * (`m_selectionStartCursor` is set), this method:
 * - Updates the end of the selection as the mouse moves.
 * - Dynamically adjusts the selection start and end points based on the current
 *   cursor position, allowing selection in any direction.
 * - Updates the terminal view to visually reflect the selection change.
 *
 * @see positionToCursor(), update()
 */
void Widgets::Terminal::mouseMoveEvent(QMouseEvent* event)
{
  if (!m_mouseTracking)
    return;

  // Determine the current cursor position based on the mouse event
  QPoint currentCursorPos = positionToCursor(event->pos());

  // Inverted selection (cursor before anchor)
  if ((m_selectionStartCursor.y() > currentCursorPos.y())
      || (m_selectionStartCursor.y() == currentCursorPos.y()
          && m_selectionStartCursor.x() > currentCursorPos.x())) {
    m_selectionStart = currentCursorPos;
    m_selectionEnd   = m_selectionStartCursor;
  }

  // Normal selection (from top-left to bottom-right or similar)
  else {
    m_selectionStart = m_selectionStartCursor;
    m_selectionEnd   = currentCursorPos;
  }

  m_stateChanged = true;
  Q_EMIT selectionChanged();
}

/**
 * @brief Handles mouse press events for starting text selection.
 *
 * @param event A pointer to the QMouseEvent object containing details of the
 * event.
 *
 * When the left mouse button is pressed:
 * - Enables mouse tracking (`m_mouseTracking`) to allow text selection.
 * - Clears any existing selection and sets the initial selection point
 *   (`m_selectionStartCursor`).
 * - Requests a view update to reflect the start of the selection visually.
 *
 * @see update(), positionToCursor()
 */
void Widgets::Terminal::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    m_mouseTracking        = true;
    m_selectionStartCursor = positionToCursor(event->pos());
    m_selectionStart       = m_selectionStartCursor;
    m_selectionEnd         = m_selectionStartCursor;
    m_stateChanged         = true;

    forceActiveFocus();
    Q_EMIT selectionChanged();
  }
}

/**
 * @brief Handles mouse release events for finalizing text selection.
 *
 * @param event A pointer to the QMouseEvent object containing details of the
 * event.
 *
 * When the left mouse button is released:
 * - If the start and end of the selection are the same, clears the selection.
 * - Disables mouse tracking and resets the selection starting point.
 * - Requests a view update to finalize the visual representation of the
 *   selection.
 *
 * @see update(), positionToCursor()
 */
void Widgets::Terminal::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (m_selectionStart == m_selectionEnd) {
      m_selectionStart = QPoint();
      m_selectionEnd   = QPoint();
    }

    m_selectionStartCursor = QPoint();
    m_mouseTracking        = false;
    m_stateChanged         = true;
    Q_EMIT selectionChanged();
  }
}

/**
 * @brief Handles mouse double-click events for selecting the word under the
 * cursor.
 *
 * @param event A pointer to the QMouseEvent object containing details of the
 *              event.
 *
 * When the user double-clicks within the terminal, this method:
 * - Determines the cursor's position based on the double-click location.
 * - Expands the selection to include the entire word under the cursor.
 * - Emits the `selectionChanged()` signal to update the visual representation
 *   of the selection.
 *
 * @note This method ensures that a double-click selects a word, similar to
 *       behavior in standard text editors or terminal emulators.
 *
 * @see positionToCursor(), selectionChanged()
 */
void Widgets::Terminal::mouseDoubleClickEvent(QMouseEvent* event)
{
  auto cursorPos = positionToCursor(event->pos());
  if (cursorPos.y() >= 0 && cursorPos.y() < m_data.size()) {
    const QString& line = m_data[cursorPos.y()];

    // Expand selection to word boundaries
    int wordStartX = cursorPos.x();
    int wordEndX   = cursorPos.x();
    while (wordStartX > 0 && !shouldEndSelection(line[wordStartX - 1]))
      wordStartX--;

    while (wordEndX < line.size() && !shouldEndSelection(line[wordEndX]))
      wordEndX++;

    m_selectionStart = QPoint(wordStartX, cursorPos.y());
    m_selectionEnd   = QPoint(wordEndX, cursorPos.y());
    m_stateChanged   = true;
    Q_EMIT selectionChanged();
  }
}
