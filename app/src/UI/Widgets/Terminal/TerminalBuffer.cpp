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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Terminal/TerminalBuffer.h"

#include <climits>

#include "SSAssert.h"

/**
 * @brief Cap on the upfront buffer reservation so large scrollback settings do not
 *        front-load multi-megabyte allocations; growth beyond it is amortized.
 */
constexpr int MAX_UPFRONT_RESERVE = 10000;

/**
 * @brief Column count used until the widget reports its own geometry; matches the floor that
 *        the terminal's maxCharsPerLine() never goes below.
 */
constexpr int DEFAULT_COLUMNS = 84;

//--------------------------------------------------------------------------------------------------
// Construction & configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty buffer that writes the colors @p palette currently selects; the facade
 *        owns that palette and keeps it alive for the buffer's whole lifetime.
 */
Widgets::TerminalBuffer::TerminalBuffer(const AnsiPalette& palette)
  : m_palette(palette)
  , m_columns(DEFAULT_COLUMNS)
  , m_maxLines(MAX_UPFRONT_RESERVE)
  , m_droppedLines(0)
  , m_cursorMoved(false)
  , m_ansiColors(false)
  , m_showTimestamps(false)
  , m_collapseDuplicates(false)
{}

/**
 * @brief Sets the character columns a row holds before it wraps; the cursor clamp and every
 *        wrapped-row count read it, so the facade pushes its live geometry before feeding data.
 */
void Widgets::TerminalBuffer::setColumns(int columns)
{
  SS_ASSERT(columns > 0, columns = DEFAULT_COLUMNS);
  m_columns = columns;
}

/**
 * @brief Sets the scrollback cap. Raising it only lets future appends grow further; the trim of
 *        an already-oversized buffer is the caller's separate trimToMaxLines() step.
 */
void Widgets::TerminalBuffer::setMaxLines(int maxLines)
{
  SS_ASSERT(maxLines > 0, maxLines = MAX_UPFRONT_RESERVE);
  m_maxLines = maxLines;
}

/**
 * @brief Enables or disables the per-character color rows.
 */
void Widgets::TerminalBuffer::setAnsiColors(bool enabled)
{
  m_ansiColors = enabled;
}

/**
 * @brief Tells the buffer whether rows carry a timestamp prefix, which duplicate collapsing
 *        skips before comparing two rows.
 */
void Widgets::TerminalBuffer::setShowTimestamps(bool enabled)
{
  m_showTimestamps = enabled;
}

/**
 * @brief Enables or disables merging of consecutive identical rows.
 */
void Widgets::TerminalBuffer::setCollapseDuplicates(bool enabled)
{
  m_collapseDuplicates = enabled;
}

//--------------------------------------------------------------------------------------------------
// Drained state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns and clears the number of rows dropped from the front since the last drain; the
 *        facade shifts its selection, scroll offset and search state by that amount.
 */
int Widgets::TerminalBuffer::takeDroppedLines()
{
  const int dropped = m_droppedLines;
  m_droppedLines    = 0;
  return dropped;
}

/**
 * @brief Returns and clears whether the cursor moved since the last drain.
 */
bool Widgets::TerminalBuffer::takeCursorMoved()
{
  const bool moved = m_cursorMoved;
  m_cursorMoved    = false;
  return moved;
}

//--------------------------------------------------------------------------------------------------
// Geometry
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the last visual row the cursor's line occupies once word wrapping is applied;
 *        this is the value autoscroll pins to the bottom of the viewport.
 */
int Widgets::TerminalBuffer::visualBottomRow() const
{
  const int row = m_cursor.y();

  int wrappedLines = 1;
  if (row < m_data.size()) {
    const int lineLength = static_cast<int>(m_data[row].length());
    wrappedLines         = (lineLength + m_columns - 1) / m_columns;
  }

  return row + wrappedLines - 1;
}

//--------------------------------------------------------------------------------------------------
// Lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Empties every row store and re-reserves them from the scrollback cap.
 */
void Widgets::TerminalBuffer::reset()
{
  const int reserveLines = qMin(m_maxLines, MAX_UPFRONT_RESERVE);

  m_data.clear();
  m_data.squeeze();
  m_data.reserve(reserveLines);
  m_colorData.clear();
  m_colorData.squeeze();
  m_repeatCounts.clear();
  m_repeatCounts.squeeze();
  m_repeatCounts.reserve(reserveLines);

  m_droppedLines = 0;

  if (m_ansiColors)
    m_colorData.reserve(reserveLines);
}

/**
 * @brief Releases the capacity a front-erase strands; a QList front-erase keeps it otherwise.
 */
void Widgets::TerminalBuffer::squeeze()
{
  m_data.squeeze();
  m_colorData.squeeze();
  m_repeatCounts.squeeze();
}

/**
 * @brief Reserves the color rows when ANSI colors are switched on mid-session.
 */
void Widgets::TerminalBuffer::reserveColorRows()
{
  m_colorData.reserve(qMin(m_maxLines, MAX_UPFRONT_RESERVE));
}

//--------------------------------------------------------------------------------------------------
// Cursor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Moves the cursor, clamped to the column count and the scrollback cap; a real move is
 *        recorded for takeCursorMoved() rather than signalled.
 */
void Widgets::TerminalBuffer::setCursor(const QPoint& position)
{
  const QPoint clamped(qBound(0, position.x(), m_columns), qBound(0, position.y(), m_maxLines));
  if (m_cursor != clamped) {
    m_cursor      = clamped;
    m_cursorMoved = true;
  }
}

/**
 * @brief Moves the cursor to the given coordinates.
 */
void Widgets::TerminalBuffer::setCursor(int x, int y)
{
  setCursor(QPoint(x, y));
}

//--------------------------------------------------------------------------------------------------
// Scrollback trimming
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops the rows that now exceed the scrollback cap; returns how many went, so the caller
 *        can shift the view state that indexes into the buffer.
 */
int Widgets::TerminalBuffer::trimToMaxLines()
{
  const int excess = static_cast<int>(m_data.size()) - m_maxLines;
  if (excess <= 0)
    return 0;

  trimFront(excess);
  return excess;
}

/**
 * @brief Drops @p linesToDrop rows from the front, keeping the color rows, the repeat counts and
 *        the cursor row in lockstep; the count is accumulated for the facade's drain.
 */
void Widgets::TerminalBuffer::trimFront(int linesToDrop)
{
  SS_ASSERT(linesToDrop > 0, return);
  SS_ASSERT(linesToDrop <= m_data.size(), linesToDrop = static_cast<int>(m_data.size()));

  m_data.erase(m_data.begin(), m_data.begin() + linesToDrop);

  // code-verify off
  // Trim color rows in lockstep with text rows regardless of the ansiColors() toggle: rows
  // recorded while colors were on must not survive as a stale, misaligned front.
  if (!m_colorData.isEmpty()) {
    const int colorDrop = qMin(linesToDrop, static_cast<int>(m_colorData.size()));
    m_colorData.erase(m_colorData.begin(), m_colorData.begin() + colorDrop);
  }
  // code-verify on

  if (!m_repeatCounts.isEmpty()) {
    const int countDrop = qMin(linesToDrop, static_cast<int>(m_repeatCounts.size()));
    m_repeatCounts.erase(m_repeatCounts.begin(), m_repeatCounts.begin() + countDrop);
  }

  if (m_cursor.y() >= linesToDrop)
    m_cursor.setY(m_cursor.y() - linesToDrop);
  else
    m_cursor.setY(0);

  m_droppedLines += linesToDrop;
}

//--------------------------------------------------------------------------------------------------
// Text lane
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p string at the cursor, wrapping at the column count and trimming the front
 *        first when the row store is already at the scrollback cap.
 */
void Widgets::TerminalBuffer::appendText(QStringView string)
{
  const int linesToDrop = static_cast<int>(m_data.size()) - m_maxLines + 1;
  if (m_data.size() >= m_maxLines && linesToDrop > 0)
    trimFront(linesToDrop);

  for (const auto& character : string) {
    const int cursorX = m_cursor.x();
    const int cursorY = m_cursor.y();
    replaceData(cursorX, cursorY, character);
    setCursor(cursorX + 1, cursorY);

    if (m_cursor.x() >= m_columns)
      setCursor(0, m_cursor.y() + 1);
  }
}

/**
 * @brief Applies one control character of the text lane: line feeds close and possibly collapse
 *        the row, the VT-100 controls move or blank the cursor, and printable bytes accumulate in
 *        @p text until a control byte flushes them.
 */
Widgets::TerminalBuffer::FeedResult Widgets::TerminalBuffer::processText(const QChar& byte,
                                                                         QString& text,
                                                                         bool vt100)
{
  const ushort code = byte.unicode();

  if (code == '\n') {
    appendText(text);
    text.clear();

    const int next_row = m_cursor.y() + (collapseCompletedLine() ? 0 : 1);
    setCursor(0, next_row);
    return Handled;
  }

  if (!vt100) {
    if (byte.isPrint())
      text.append(byte);

    return Handled;
  }

  switch (code) {
    case 0x1b:
      appendText(text);
      text.clear();
      return BeginEscape;
    case '\r':
      appendText(text);
      text.clear();
      setCursor(0, m_cursor.y());
      return Handled;
    case '\b':
      if (m_cursor.x() == 0)
        return Handled;

      appendText(text);
      text.clear();
      setCursor(m_cursor.x() - 1, m_cursor.y());
      return Handled;
    case 0x7F:
      appendText(text);
      text.clear();
      removeFromCursor(AnsiEraseDirection::Right, 1);
      return Handled;
    case '\t': {
      appendText(text);
      text.clear();
      const int nextTab = (m_cursor.x() / 8 + 1) * 8;
      const int spaces  = nextTab - m_cursor.x();
      text.fill(' ', spaces);
      appendText(text);
      text.clear();
      return Handled;
    }
    default:
      break;
  }

  if (byte.isPrint())
    text.append(byte);

  return Handled;
}

/**
 * @brief Replaces or inserts a character at a specified position, growing the row store, the
 *        color row and the repeat counts to reach it.
 */
void Widgets::TerminalBuffer::replaceData(int x, int y, const QChar& byte)
{
  if (y >= m_data.size())
    m_data.resize(y + 1);

  while (m_repeatCounts.size() < m_data.size())
    m_repeatCounts.append(1);

  QString& line = m_data[y];

  if (m_ansiColors) {
    if (y >= m_colorData.size())
      m_colorData.resize(y + 1);

    QList<CharColor>& colorLine = m_colorData[y];

    if (x > line.size()) {
      const int padCount = x - static_cast<int>(line.size());
      for (int i = 0; i < padCount; ++i)
        colorLine.append(CharColor());
    }

    while (colorLine.size() < line.size())
      colorLine.append(CharColor());

    const CharColor charColor(m_palette.foreground(), m_palette.background());
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
 * @brief Blanks up to @p length characters on one side of the cursor within its row.
 */
void Widgets::TerminalBuffer::removeFromCursor(AnsiEraseDirection direction, int length)
{
  const auto positionX = m_cursor.x();
  const auto positionY = m_cursor.y();

  if (length < 0)
    length = INT_MAX;

  int removeSize = 0;
  if (direction == AnsiEraseDirection::Right) {
    const qsizetype lineLen =
      (positionY >= 0 && positionY < m_data.size()) ? m_data[positionY].size() : qsizetype(0);
    const qsizetype l1 = lineLen - positionX;
    const qsizetype l2 = static_cast<qsizetype>(length);
    removeSize         = static_cast<int>(qMin(qMax(l1, qsizetype(0)), l2));
  }

  else
    removeSize = qMin(length, positionX);

  int offset = 0;
  const QChar clearChar('\x7F');
  for (int i = 0; i < removeSize; ++i) {
    if (direction == AnsiEraseDirection::Left)
      offset = -1 - i;
    else
      offset = i;

    replaceData(m_cursor.x() + offset, positionY, clearChar);
  }
}

//--------------------------------------------------------------------------------------------------
// Duplicate collapsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @p point pulled inside the rows and columns @p lines still holds, or a null
 *        point when there are none. Every row removal has to run its selection endpoints through
 *        this: an endpoint left past the last row indexes the line list out of bounds the next
 *        time the widget copies or highlights (F2).
 */
QPoint Widgets::TerminalBuffer::clampPoint(const QPoint& point, const QStringList& lines)
{
  if (lines.isEmpty())
    return QPoint();

  const int row = qBound(0, point.y(), static_cast<int>(lines.size()) - 1);
  return QPoint(qBound(0, point.x(), static_cast<int>(lines.at(row).size())), row);
}

/**
 * @brief Returns true when @p line starts with a well-formed "HH:mm:ss.zzz -> " stamp.
 */
bool Widgets::TerminalBuffer::hasTimestampPrefix(QStringView line)
{
  constexpr int kStampLen        = 16;
  constexpr int kDigitIndices[9] = {0, 1, 3, 4, 6, 7, 9, 10, 11};

  if (line.size() < kStampLen)
    return false;

  if (line[2] != QLatin1Char(':') || line[5] != QLatin1Char(':') || line[8] != QLatin1Char('.'))
    return false;

  if (line[12] != QLatin1Char(' ') || line[13] != QLatin1Char('-') || line[14] != QLatin1Char('>')
      || line[15] != QLatin1Char(' '))
    return false;

  for (const int index : kDigitIndices)
    if (!line[index].isDigit())
      return false;

  return true;
}

/**
 * @brief Returns the comparable content of @p line for duplicate collapsing, skipping the
 *        timestamp stamp prefix when timestamps are enabled and the prefix shape matches.
 */
QStringView Widgets::TerminalBuffer::lineContentView(QStringView line) const
{
  constexpr int kStampLen = 16;

  if (!m_showTimestamps || !hasTimestampPrefix(line))
    return line;

  return line.mid(kStampLen);
}

/**
 * @brief Merges the just-completed line into the previous row when duplicate collapsing is
 *        on and both rows carry identical content; returns true when the row was merged so
 *        the caller reuses the freed row index for the next line.
 */
bool Widgets::TerminalBuffer::collapseCompletedLine()
{
  if (!m_collapseDuplicates)
    return false;

  const int y = m_cursor.y();
  if (y < 1 || y != m_data.size() - 1)
    return false;

  const auto current  = lineContentView(m_data[y]);
  const auto previous = lineContentView(m_data[y - 1]);
  if (current.trimmed().isEmpty() || current != previous)
    return false;

  m_data.removeLast();
  if (m_colorData.size() > m_data.size())
    m_colorData.resize(m_data.size());

  if (m_repeatCounts.size() > m_data.size())
    m_repeatCounts.resize(m_data.size());

  if (y - 1 < m_repeatCounts.size() && m_repeatCounts[y - 1] < INT_MAX)
    ++m_repeatCounts[y - 1];

  return true;
}

//--------------------------------------------------------------------------------------------------
// Row erasure
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every row below @p row, keeping the color rows and repeat counts in lockstep so
 *        the paint pass never reads a misaligned row, and pulling the cursor back inside the
 *        rows that survived.
 */
void Widgets::TerminalBuffer::eraseRowsAfter(int row)
{
  row = qBound(-1, row, static_cast<int>(m_data.size()));
  if (row + 1 >= m_data.size())
    return;

  m_data.erase(m_data.begin() + row + 1, m_data.end());

  // code-verify off
  // Trim the color rows in lockstep regardless of the ansiColors() toggle: rows recorded while
  // colors were on must not survive as a stale, misaligned tail (the trimFront() rule).
  if (row + 1 < m_colorData.size())
    m_colorData.erase(m_colorData.begin() + row + 1, m_colorData.end());
  // code-verify on

  if (row + 1 < m_repeatCounts.size())
    m_repeatCounts.erase(m_repeatCounts.begin() + row + 1, m_repeatCounts.end());

  if (m_cursor.y() >= m_data.size()) {
    m_cursor.setY(qMax<int>(0, static_cast<int>(m_data.size()) - 1));
    m_cursorMoved = true;
  }
}

/**
 * @brief Drops every row above @p row. A front erase shifts every row index, so it goes through
 *        the same trim the scrollback cap uses: the color rows, the repeat counts and the cursor
 *        move with it, and the dropped count reaches the facade, which is what rebases the
 *        selection and the scroll offset.
 */
void Widgets::TerminalBuffer::eraseRowsBefore(int row)
{
  const int drop = qMin(row, static_cast<int>(m_data.size()));
  if (drop <= 0)
    return;

  trimFront(drop);
}
