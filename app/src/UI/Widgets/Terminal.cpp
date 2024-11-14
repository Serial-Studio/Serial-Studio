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

#include <QPainter>
#include <QClipboard>
#include <QFontMetrics>
#include <QGuiApplication>

#include "IO/Console.h"
#include "IO/Manager.h"
#include "Misc/Translator.h"
#include "Misc/TimerEvents.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "UI/Widgets/Terminal.h"

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
 * - Connects to the IO Console handler to receive and append new data for
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
Widgets::Terminal::Terminal(QQuickItem *parent)
  : QQuickPaintedItem(parent)
  , m_scrollOffsetY(0)
  , m_state(Text)
  , m_autoscroll(true)
  , m_emulateVt100(false)
  , m_cursorVisible(true)
  , m_mouseTracking(false)
  , m_formatValue(0)
  , m_formatValueY(0)
  , m_useFormatValueY(false)
{
  // Initialize data buffer
  initBuffer();

  // Configure QML item flags to accept mouse input
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Set rendering hints
  setMipmap(false);
  setAntialiasing(true);
  setOpaquePainting(true);

  // Set font
  setFont(Misc::CommonFonts::instance().monoFont());

  // Set palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &Widgets::Terminal::onThemeChanged);

  // Receive data from the IO::Console handler
  connect(&IO::Console::instance(), &IO::Console::displayString, this,
          &Widgets::Terminal::append, Qt::QueuedConnection);

  // Clear the screen when device is connected/disconnected
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this, [=] {
    if (IO::Manager::instance().connected())
      clear();
  });

  // Redraw widget as soon as it is visible
  connect(this, &Widgets::Terminal::visibleChanged, this, [=] {
    if (isVisible())
      update();
  });

  // Change character widths when changing language
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, [=] { setFont(Misc::CommonFonts::instance().monoFont()); });

  // Blink the cursor
  m_cursorTimer.start(200);
  m_cursorTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_cursorTimer, &QTimer::timeout, this,
          &Widgets::Terminal::toggleCursor);

  // Redraw the widget only when necessary
  m_stateChanged = true;
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout24Hz, this,
          [=] {
            if (isVisible() && m_stateChanged)
            {
              m_stateChanged = false;
              update();
            }
          });
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
void Widgets::Terminal::paint(QPainter *painter)
{
  // Skip if item is not visible
  if (!isVisible() || !painter)
    return;

  // Set font and prepare painter
  painter->setFont(m_font);
  int lineHeight = m_cHeight;

  // Calculate the range of lines to be painted
  const int firstLine = m_scrollOffsetY;
  const int lastVLine = qMin(firstLine + linesPerPage(), lineCount() - 1);

  // Draw selection rectangles
  int y = m_borderY;
  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i)
  {
    // Obtain the line data
    const QString &line = m_data[i];

    // Check if this line is within the selection range
    bool lineFullySelected = !m_selectionEnd.isNull()
                             && i >= m_selectionStart.y()
                             && i < m_selectionEnd.y();

    // Handle empty lines
    if (line.isEmpty())
    {
      // Draw selection rectangle if required for this line
      if (lineFullySelected)
      {
        QRect selectionRect(m_borderX, y, width() - 2 * m_borderX, m_cHeight);
        painter->fillRect(selectionRect, m_palette.color(QPalette::Highlight));
      }

      // Go to next line
      y += lineHeight;
      continue;
    }

    // If the line is fully selected, draw a single rectangle
    if (lineFullySelected)
    {
      QRect selectionRect(m_borderX, y, width() - 2 * m_borderX, m_cHeight);
      painter->fillRect(selectionRect, m_palette.color(QPalette::Highlight));
      y += lineHeight;
      continue;
    }

    // Render selection for line with word-wrapping and variable-width chars
    int start = 0;
    while (start < line.length())
    {
      const int lineEnd = qMin<int>(start + maxCharsPerLine(), line.length());

      if (!m_selectionEnd.isNull() && i >= m_selectionStart.y()
          && i <= m_selectionEnd.y())
      {
        int selectionStartX, selectionEndX;

        // Specific case for the first line of the selection
        if (i == m_selectionStart.y())
        {
          selectionStartX = qMax(m_selectionStart.x(), start);
          selectionEndX = (i == m_selectionEnd.y())
                              ? qMin(m_selectionEnd.x(), lineEnd)
                              : lineEnd;
        }

        // Specific case for the last line of the selection
        else if (i == m_selectionEnd.y())
        {
          selectionStartX = start;
          selectionEndX = qMin(m_selectionEnd.x(), lineEnd);
        }

        // Entire line selected within the bounds of start and end
        else
        {
          selectionStartX = start;
          selectionEndX = lineEnd;
        }

        if (selectionStartX < selectionEndX)
        {
          int startX = m_borderX;
          int selectionWidth = 0;

          // Cap for selection width
          int maxSelectionWidth = width() - 2 * m_borderX;

          for (int j = start; j < selectionEndX; ++j)
          {
            int charWidth = painter->fontMetrics().horizontalAdvance(line[j]);

            if (j < selectionStartX)
              startX += charWidth;
            else
              selectionWidth += charWidth;
          }

          // Cap the selection width to fit within the allowed area
          selectionWidth
              = qMin(selectionWidth, maxSelectionWidth - (startX - m_borderX));

          // Draw the selection rectangle
          QRect selectionRect(startX, y, selectionWidth, m_cHeight);
          painter->fillRect(selectionRect,
                            m_palette.color(QPalette::Highlight));
        }
      }

      y += lineHeight;
      start = lineEnd;
    }
  }

  // Draw characters one by one with variable width handling
  y = m_borderY;
  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i)
  {
    // Obtain line data
    const QString &line = m_data[i];

    // Skip empty lines, but draw line break
    if (line.isEmpty())
    {
      y += lineHeight;
      continue;
    }

    // Render line with word-wrapping
    int start = 0;
    while (start < line.length())
    {
      const int end = qMin<int>(start + maxCharsPerLine(), line.length());
      const QString segment = line.mid(start, end - start);
      int x = m_borderX;

      for (int j = 0; j < segment.length(); ++j)
      {
        // Get character to render
        const QString character = segment.mid(j, 1);

        // Measure the character width dynamically
        int charWidth = painter->fontMetrics().horizontalAdvance(character);

        // Draw the character at the centered position within its cell
        painter->setPen(m_palette.color(QPalette::Text));
        painter->drawText(x, y, charWidth, m_cHeight, Qt::AlignCenter,
                          character);

        // Move to the next character position
        x += charWidth;
      }

      y += lineHeight;
      start = end;
    }
  }

  // Draw cursor if visible
  if (m_cursorVisible && m_cursorPosition.y() >= firstLine
      && m_cursorPosition.y() <= lastVLine)
  {
    // clang-format off
    const int cursorX = m_cursorPosition.x() * m_cWidth + m_borderX;
    const int cursorY = (m_cursorPosition.y() - firstLine + 1) * lineHeight + m_borderY;
    // clang-format on

    // Draw the cursor as a filled block character
    painter->setPen(m_palette.color(QPalette::Text));
    painter->drawText(cursorX, cursorY, QStringLiteral("█"));
  }

  // Draw scrollbar if required
  if (!autoscroll() && lineCount() > linesPerPage())
  {
    // Get available height
    const int availableHeight = height() - 2 * m_borderY;

    // Set dimensions
    const int scrollbarWidth = 6;
    int scrollbarHeight = qMax(20.0, qPow(availableHeight, 2) / lineCount());
    if (scrollbarHeight > availableHeight / 2)
      scrollbarHeight = availableHeight / 2;

    // Set scrollbar position
    int x = width() - scrollbarWidth - m_borderX;
    int y = (m_scrollOffsetY / static_cast<float>(lineCount() - linesPerPage()))
                * (availableHeight - scrollbarHeight)
            - m_borderY;
    y = qMax(m_borderY, y);

    // Draw the scrollbar
    QRect scrollbarRect(x, y, scrollbarWidth, scrollbarHeight);
    QBrush scrollbarBrush(m_palette.color(QPalette::Window));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(scrollbarBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(scrollbarRect, scrollbarWidth / 2,
                             scrollbarWidth / 2);
  }
}

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

/**
 * @brief Gets the current font used by the terminal.
 *
 * @return The QFont object representing the terminal's current font.
 */
QFont Widgets::Terminal::font() const
{
  return m_font;
}

/**
 * @brief Gets the current color palette used by the terminal.
 *
 * @return The QPalette object representing the terminal's color palette.
 */
QPalette Widgets::Terminal::palette() const
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
  return (!m_selectionEnd.isNull() || !m_selectionStart.isNull())
         && !m_data.isEmpty();
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
  const auto realValue = (width() - 2 * m_borderX) / m_cWidth;
  return qMax<int>(84, realValue);
}

/**
 * @brief Gets the current cursor position within the terminal.
 *
 * @return A QPoint representing the current cursor position, in character
 *         coordinates.
 */
QPoint Widgets::Terminal::cursorPosition() const
{
  return m_cursorPosition;
}

/**
 * @brief Converts a pixel position to a cursor position.
 *
 * @param pos The pixel position within the terminal.
 * @return A QPoint representing the cursor position corresponding to the given
 *         pixel position.
 */
QPoint Widgets::Terminal::positionToCursor(const QPoint &pos) const
{
  int y = (pos.y() - m_borderY) / m_cHeight + m_scrollOffsetY;
  int actualY = 0;
  int actualX = 0;
  int remainingY = y;

  for (int i = 0; i < m_data.size() && i <= y; ++i)
  {
    const QString &line = m_data[i];

    if (line.isEmpty())
    {
      if (remainingY == 0)
        return QPoint(0, i);

      remainingY--;
    }

    else
    {
      int lines = (line.length() + maxCharsPerLine() - 1) / maxCharsPerLine();
      if (remainingY < lines)
      {
        actualY = i;
        int x = pos.x() - m_borderX;
        int widthSum = 0;
        for (int j = 0; j < line.length(); ++j)
        {
          int charWidth = QFontMetrics(m_font).horizontalAdvance(line[j]);
          if (widthSum + charWidth > x)
          {
            actualX = j;
            return QPoint(actualX, actualY);
          }
          widthSum += charWidth;
        }

        actualX = line.length();
        return QPoint(actualX, actualY);
      }

      remainingY -= lines;
    }
  }

  if (!m_data.isEmpty())
  {
    actualY = m_data.size() - 1;
    actualX = m_data.last().length();
  }

  return QPoint(qMax(0, actualX), qMax(0, actualY));
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
  // Ensure that there is a valid selection
  if (!copyAvailable())
    return;

  // Determine the correct order of start and end for selection
  QString copiedText;
  QPoint start = m_selectionStart;
  QPoint end = m_selectionEnd;

  if (start.y() > end.y() || (start.y() == end.y() && start.x() > end.x()))
    std::swap(start, end);

  // Iterate over the lines within the selection range
  for (int lineIndex = start.y(); lineIndex <= end.y(); ++lineIndex)
  {
    const QString &line = m_data[lineIndex];

    int startX = (lineIndex == start.y()) ? start.x() : 0;
    int endX = (lineIndex == end.y()) ? end.x() : line.size();

    // Adjust if the selection is inverted
    if (start.y() == end.y() && start.x() > end.x())
      std::swap(startX, endX);

    // Check if the selection spans the entire line
    if (lineIndex != start.y() && lineIndex != end.y())
    {
      startX = 0;
      endX = line.size();
    }

    // Extract the selected portion of the line
    if (startX < endX)
      copiedText.append(line.mid(startX, endX - startX));

    // If the selection spans the entire line, add a newline character
    if (lineIndex != end.y() || (startX == 0 && endX == line.size()))
      copiedText.append('\n');
  }

  // Copy the selected text to the clipboard
  QClipboard *clipboard = QGuiApplication::clipboard();
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
  // Skip if there is no data to select
  if (m_data.isEmpty())
    return;

  // Set selection start at the beginning (top-left corner)
  m_selectionStart = QPoint(0, 0);

  // Set selection end at the last character of the last line
  int lastLineIndex = m_data.size() - 1;
  int lastCharIndex = m_data[lastLineIndex].size();
  m_selectionEnd = QPoint(lastCharIndex, lastLineIndex);

  // Since we're selecting everything, we do not need a "start cursor"
  m_selectionStartCursor = m_selectionStart;

  // Emit signal to indicate that the selection has changed
  m_stateChanged = true;
  Q_EMIT selectionChanged();
}

/**
 * @brief Sets the font used for rendering the terminal text.
 *
 * @param font The QFont object to be used for terminal text.
 *
 * Updates the internal font, recalculates character dimensions, and adjusts the
 * terminal border accordingly. Emits the fontChanged() signal to notify about
 * the change.
 */
void Widgets::Terminal::setFont(const QFont &font)
{
  // Update font
  m_font = font;

  // Ensure that antialiasing is enabled
  m_font.setStyleStrategy(QFont::PreferAntialias);

  // Get size of font (in pixels)
  auto metrics = QFontMetrics(m_font);

  // Update character sizes
  m_cHeight = metrics.height();
  m_cWidth = metrics.horizontalAdvance("M");

  // Update terminal border
  m_borderX = qMax(m_cWidth, m_cHeight) / 2;
  m_borderY = qMax(m_cWidth, m_cHeight) / 2;

  // Notify QML
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
  if (m_scrollOffsetY != offset)
  {
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
void Widgets::Terminal::setPalette(const QPalette &palette)
{
  m_palette = palette;
  Q_EMIT colorPaletteChanged();
}

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
 * @brief Toggles the visibility of the cursor.
 *
 * Flips the visibility state of the cursor, which is typically used to create
 * a blinking cursor effect.
 */
void Widgets::Terminal::toggleCursor()
{
  m_stateChanged = true;
  m_cursorVisible = !m_cursorVisible;
}

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
  update();
  // clang-format on
}

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
void Widgets::Terminal::append(const QString &data)
{
  QString text;
  auto it = data.constBegin();

  while (it != data.constEnd())
  {
    auto byte = *it;
    switch (m_state)
    {
      case Text:
        processText(byte, text);
        break;
      case Escape:
        processEscape(byte, text);
        break;
      case Format:
        processFormat(byte, text);
        break;
      case ResetFont:
        processResetFont(byte, text);
        break;
    }

    ++it;
  }

  appendString(text);
  m_stateChanged = true;
}

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
void Widgets::Terminal::appendString(const QString &string)
{
  // Register each character in the provided string
  foreach (QChar character, string)
  {
    // Obtain the current (x, y) cursor position
    int cursorX = m_cursorPosition.x();
    int cursorY = m_cursorPosition.y();

    // Replace data in the console buffer at the current cursor position
    replaceData(cursorX, cursorY, character);

    // Increment the cursor position to the right after placing the character
    setCursorPosition(cursorX + 1, cursorY);

    // If we've reached the end of a wrapped line, move to the next line
    if (m_cursorPosition.x() >= maxCharsPerLine())
      setCursorPosition(0, m_cursorPosition.y() + 1);
  }

  // Adjust the scroll offset if autoscroll is enabled
  if (autoscroll())
  {
    // Calculate the total number of wrapped lines for the current line
    int cursorLine = m_cursorPosition.y();
    int wrappedLines = 1;
    if (cursorLine < m_data.size())
    {
      int lineLength = m_data[cursorLine].length();
      wrappedLines = (lineLength + maxCharsPerLine() - 1) / maxCharsPerLine();
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
void Widgets::Terminal::removeStringFromCursor(const Direction direction,
                                               int len)
{
  // Obtain (x, y) position
  const auto positionX = m_cursorPosition.x();
  const auto positionY = m_cursorPosition.y();

  // Ensure valid length
  if (len < 0)
    len = INT_MAX;

  // Cap bytes to remove (right)
  int removeSize = 0;
  if (direction == RightDirection)
  {
    qsizetype l1 = m_data[positionY].size() - positionX;
    qsizetype l2 = static_cast<qsizetype>(len);
    removeSize = qMin(l1, l2);
  }

  // Cap bytes to remove (left)
  else
    removeSize = qMin(len, m_cursorPosition.x());

  // Removal operation
  int offset = 0;
  const QChar clearChar('\x7F');
  for (int i = 0; i < removeSize; ++i)
  {
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
 * Clears the existing data buffer and reserves memory for 1024 * 100 lines of
 * terminal content.
 *
 * This function is typically used to reset the terminal state, ensuring
 * efficient memory management for upcoming operations.
 */
void Widgets::Terminal::initBuffer()
{
  m_data.clear();
  m_data.squeeze();
  m_data.reserve(1024 * 100);
}

/**
 * @brief Processes a single character in the context of normal text input.
 *
 * @param byte The character to be processed.
 * @param text A reference to a QString that accumulates printable characters.
 *
 * This method handles normal text input processing, managing different
 * character cases:
 * - If the character is an escape character (`0x1b`) and VT-100 emulation is
 *   enabled, it switches the state to `Escape` after appending the current
 *   accumulated text.
 * - If the character is a newline (`'\n'`), the accumulated text is appended,
 *   and a new line is created in the buffer.
 * - If the character is a backspace (`'\b'`) and VT-100 emulation is enabled,
 *   the cursor is moved one position to the left.
 * - If the character is printable, it is appended to the accumulated `text`.
 *
 * This function helps manage cursor positioning and character input depending
 * on the current state.
 *
 * @see appendString(), setCursorPosition(), vt100emulation()
 */
void Widgets::Terminal::processText(const QChar &byte, QString &text)
{
  if (byte.toLatin1() == 0x1b && vt100emulation())
  {
    appendString(text);
    text.clear();
    m_state = Escape;
  }

  else if (byte == '\n')
  {
    appendString(text);
    text.clear();
    m_data.append("");
    setCursorPosition(0, m_cursorPosition.y() + 1);
  }

  else if (byte == '\b' && vt100emulation())
  {
    if (m_cursorPosition.x())
    {
      appendString(text);
      text.clear();
      setCursorPosition(m_cursorPosition.x() - 1, m_cursorPosition.y());
    }
  }

  else if (byte.isPrint())
    text.append(byte);
}

/**
 * @brief Processes an escape sequence character.
 *
 * @param byte The character to be processed as part of the escape sequence.
 * @param text A reference to a QString (currently unused in this method).
 *
 * This method handles the initial part of an escape sequence:
 * - Resets the format values (`m_formatValue`, `m_formatValueY`) and related
 *   state.
 * - If the character is `'['`, it switches to `Format` state to process
 *   additional formatting characters.
 * - If the character is `'('`, it switches to `ResetFont` state for further
 *   processing.
 *
 * @see processFormat(), m_state, m_formatValue, m_formatValueY
 */
void Widgets::Terminal::processEscape(const QChar &byte, QString &text)
{
  (void)text;

  m_formatValue = 0;
  m_formatValueY = 0;
  m_useFormatValueY = false;

  if (byte == '[')
    m_state = Format;

  else if (byte == '(')
    m_state = ResetFont;
}

/**
 * @brief Processes characters in the context of a terminal format command.
 *
 * @param byte The character to be processed as part of a format command.
 * @param text A reference to a QString (currently unused in this method).
 *
 * This method handles terminal formatting commands, including text formatting,
 * cursor movement, and screen clearing:
 * - Numeric characters are accumulated as format values (`m_formatValue`,
 *   `m_formatValueY`).
 * - The `';'` character indicates multiple format values, and subsequent values
 *   are processed.
 * - The `'m'` character exits the formatting state and returns to normal text.
 * - Cursor movement commands (`'A'`, `'B'`, `'C'`, `'D'`) adjust the cursor
 *   position.
 * - The `'H'` character moves the cursor to the specified position
 *   (`m_formatValue`, `m_formatValueY`).
 * - The `'J'` character clears the screen based on `m_formatValue`.
 * - The `'K'` character clears part of the current line, depending on
 *   `m_formatValue`.
 * - The `'P'` character removes characters from the cursor position.
 * - If an unrecognized character is received, the state is reset to `Text`.
 *
 * @note Some functions (`J` and `K` with certain values) are not implemented
 *       and will produce warnings.
 *
 * @see setCursorPosition(), removeStringFromCursor(), m_state, m_formatValue
 */
void Widgets::Terminal::processFormat(const QChar &byte, QString &text)
{
  (void)text;

  // Obtain format value
  if (byte >= '0' && byte <= '9')
  {
    if (m_useFormatValueY)
      m_formatValueY = m_formatValueY * 10 + (byte.cell() - '0');
    else
      m_formatValue = m_formatValue * 10 + (byte.cell() - '0');
  }

  // Control sequences
  else
  {
    // Text formatting (ignored)
    if (byte == ';')
    {
      m_formatValueY = 0;
      m_useFormatValueY = true;
      m_state = Format;
    }

    // Exit text formatting
    else if (byte == 'm')
      m_state = Text;

    // Cursor movement
    else if (byte >= 'A' && byte <= 'D')
    {
      if (!m_formatValue)
        m_formatValue++;

      int x = 0;
      int y = 0;
      switch (byte.toLatin1())
      {
        case 'A':
          x = m_cursorPosition.x();
          y = qMax(0, m_cursorPosition.y() - m_formatValue);
          setCursorPosition(x, y);
          break;
        case 'B':
          x = m_cursorPosition.x();
          y = m_cursorPosition.y() + m_formatValue;
          setCursorPosition(x, y);
          break;
        case 'C':
          x = m_cursorPosition.x() + m_formatValue;
          y = m_cursorPosition.y();
          setCursorPosition(x, y);
          break;
        case 'D':
          x = qMax(0, m_cursorPosition.x() - m_formatValue);
          y = m_cursorPosition.y();
          setCursorPosition(x, y);
          break;
        default:
          break;
      }

      m_state = Text;
    }

    // Move cursor to current format value?
    else if (byte == 'H')
    {
      setCursorPosition(m_formatValue, m_formatValueY);
      m_state = Text;
    }

    // J function
    else if (byte == 'J')
    {
      switch (m_formatValue)
      {
        case 0:
        case 1:
        case 2:
          clear();
          break;
        default:
          qWarning() << "J" << m_formatValue << "function not implemented!";
          break;
      }

      m_state = Text;
    }

    // K function
    else if (byte == 'K')
    {
      switch (m_formatValue)
      {
        case 0:
          removeStringFromCursor(RightDirection);
          break;
        case 1:
        case 2:
          qWarning() << "K" << m_formatValue << "function not implemented!";
          break;
      }

      m_state = Text;
    }

    // P function
    else if (byte == 'P')
    {
      removeStringFromCursor(LeftDirection, m_formatValue);
      removeStringFromCursor(RightDirection);
      m_state = Text;
    }

    // Reset state
    else
      m_state = Text;
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
void Widgets::Terminal::processResetFont(const QChar &byte, QString &text)
{
  (void)byte;
  (void)text;
  m_state = Text;
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
void Widgets::Terminal::setCursorPosition(const QPoint position)
{
  if (m_cursorPosition != position)
  {
    m_cursorPosition = position;
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
void Widgets::Terminal::replaceData(qsizetype x, qsizetype y, QChar byte)
{
  // Make sure the line at y exists in the buffer
  if (y >= lineCount())
    m_data.resize(y + 1);

  // Get reference to current line
  QString &currentLine = m_data[y];

  // Ensure the current line is long enough to hold the character at x
  if (x > currentLine.size())
    currentLine = currentLine.leftJustified(x, ' ');

  // Ensure that byte is a printable character
  if (!byte.isPrint())
    byte = '.';

  // Replace or insert the character at the cursor position
  if (x < currentLine.size())
    currentLine[x] = byte;
  else
    currentLine.append(byte);
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
bool Widgets::Terminal::shouldEndSelection(const QChar c)
{
  bool end = false;
  end |= c.isSpace();
  end |= c.isNonCharacter();
  end |= (!c.isLetter() && !c.isNumber());
  return end;
}

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
void Widgets::Terminal::wheelEvent(QWheelEvent *event)
{
  // Calculate the number of steps
  int numSteps = 0;
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
  if (numSteps != 0)
  {
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
void Widgets::Terminal::mouseMoveEvent(QMouseEvent *event)
{
  if (!m_mouseTracking)
    return;

  // Determine the current cursor position based on the mouse event
  QPoint currentCursorPos = positionToCursor(event->pos());

  // Check if selection is inverted (from bottom-right to top-left or similar)
  if ((m_selectionStartCursor.y() > currentCursorPos.y())
      || (m_selectionStartCursor.y() == currentCursorPos.y()
          && m_selectionStartCursor.x() > currentCursorPos.x()))
  {
    m_selectionStart = currentCursorPos;
    m_selectionEnd = m_selectionStartCursor;
  }

  // Normal selection (from top-left to bottom-right or similar)
  else
  {
    m_selectionStart = m_selectionStartCursor;
    m_selectionEnd = currentCursorPos;
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
void Widgets::Terminal::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    m_mouseTracking = true;
    m_selectionStartCursor = positionToCursor(event->pos());
    m_selectionStart = m_selectionStartCursor;
    m_selectionEnd = m_selectionStartCursor;
    m_stateChanged = true;
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
void Widgets::Terminal::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (m_selectionStart == m_selectionEnd)
    {
      m_selectionStart = QPoint();
      m_selectionEnd = QPoint();
    }

    m_selectionStartCursor = QPoint();
    m_mouseTracking = false;
    m_stateChanged = true;
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
void Widgets::Terminal::mouseDoubleClickEvent(QMouseEvent *event)
{
  auto cursorPos = positionToCursor(event->pos());
  if (cursorPos.y() >= 0 && cursorPos.y() < m_data.size())
  {
    const QString &line = m_data[cursorPos.y()];

    // Find word boundaries by expanding to the left and right
    int wordStartX = cursorPos.x();
    int wordEndX = cursorPos.x();

    // Expand to the left until a space or start of the line is found
    while (wordStartX > 0 && !shouldEndSelection(line[wordStartX - 1]))
      wordStartX--;

    // Expand to the right until a space or end of the line is found
    while (wordEndX < line.size() && !shouldEndSelection(line[wordEndX]))
      wordEndX++;

    // Set selection start and end points
    m_selectionStart = QPoint(wordStartX, cursorPos.y());
    m_selectionEnd = QPoint(wordEndX, cursorPos.y());

    // Update view to reflect the selection
    m_stateChanged = true;
    Q_EMIT selectionChanged();
  }
}
