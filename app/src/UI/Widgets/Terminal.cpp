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

#include "UI/Widgets/Terminal.h"

#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPainter>

#include "Console/Handler.h"
#include "IO/ConnectionManager.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SSAssert.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#endif

/**
 * @brief Cap on the upfront buffer reservation so large scrollback settings do not
 *        front-load multi-megabyte allocations; growth beyond it is amortized.
 */
constexpr int MAX_UPFRONT_RESERVE = 10000;

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a Terminal object with the given parent item.
 */
Widgets::Terminal::Terminal(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_consoleHandler(Console::Handler::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_connectionManager(IO::ConnectionManager::instance())
#ifdef BUILD_COMMERCIAL
  , m_lemonSqueezy(Licensing::LemonSqueezy::instance())
#endif
  , m_translator(Misc::Translator::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_cWidth(0)
  , m_cHeight(0)
  , m_borderX(0)
  , m_borderY(0)
  , m_scrollOffsetY(0)
  , m_maxLines(m_consoleHandler.scrollbackLines())
  , m_dragThumbGrabY(0)
  , m_paused(false)
  , m_autoscroll(true)
  , m_ansiColors(false)
  , m_emulateVt100(false)
  , m_collapseDuplicates(m_consoleHandler.collapseDuplicates())
  , m_cursorVisible(true)
  , m_mouseTracking(false)
  , m_draggingScrollbar(false)
  , m_stateChanged(false)
  , m_cursorHidden(false)
  , m_ansi(*this)
  , m_badgeMetrics(QFont())
{
  initBuffer();

  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setActiveFocusOnTab(true);
  setMipmap(true);
  setOpaquePainting(true);

  loadWelcomeGuide();
  setFont(m_consoleHandler.font());
  connect(&m_consoleHandler, &Console::Handler::fontChanged, this, [this] {
    setFont(m_consoleHandler.font());
  });

  onThemeChanged();
  connect(
    &m_themeManager, &Misc::ThemeManager::themeChanged, this, &Widgets::Terminal::onThemeChanged);
  connect(&m_consoleHandler, &Console::Handler::displayString, this, &Widgets::Terminal::append);
  connect(&m_consoleHandler, &Console::Handler::collapseDuplicatesChanged, this, [this] {
    m_collapseDuplicates = m_consoleHandler.collapseDuplicates();
  });
  connect(&m_consoleHandler,
          &Console::Handler::scrollbackLinesChanged,
          this,
          &Widgets::Terminal::applyScrollbackLimit);
  connect(&m_consoleHandler, &Console::Handler::cleared, this, &Widgets::Terminal::clear);
  connect(&m_connectionManager, &IO::ConnectionManager::connectedChanged, this, [=, this] {
    if (m_connectionManager.isConnected())
      clear();
    else if (m_data.isEmpty())
      loadWelcomeGuide();
  });

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

#ifdef BUILD_COMMERCIAL
  connect(&m_lemonSqueezy,
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          &Widgets::Terminal::loadWelcomeGuide);
#endif

  connect(&m_translator, &Misc::Translator::languageChanged, this, [this] { loadWelcomeGuide(); });

  m_cursorTimer.start(200);
  m_cursorTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_cursorTimer, &QTimer::timeout, this, &Widgets::Terminal::toggleCursor);

  m_stateChanged = true;
  connect(&m_timerEvents, &Misc::TimerEvents::uiTimeout, this, [=, this] {
    if (m_search.dirty() && m_search.active() && isVisible())
      refreshSearchMatches();

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
 */
void Widgets::Terminal::drawSegmentSelection(
  QPainter* painter, const QString& line, int lineIndex, int segStart, int segEnd, int y)
{
  if (m_selectionEnd.isNull())
    return;

  if (lineIndex < m_selectionStart.y() || lineIndex > m_selectionEnd.y())
    return;

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

  int leadingOffset  = 0;
  int selectionWidth = 0;
  for (int j = segStart; j < selEndX; ++j) {
    const int charWidth = painter->fontMetrics().horizontalAdvance(line[j]);
    if (j < selStartX)
      leadingOffset += charWidth;
    else
      selectionWidth += charWidth;
  }

  const bool rtl     = m_translator.rtl();
  const int maxWidth = width() - 2 * m_borderX;
  int startX         = 0;

  if (rtl) {
    const int rightEdge = width() - m_borderX;
    startX              = rightEdge - leadingOffset - selectionWidth;
    selectionWidth      = qMin(selectionWidth, maxWidth);
  }

  else {
    startX         = m_borderX + leadingOffset;
    selectionWidth = qMin(selectionWidth, maxWidth - leadingOffset);
  }

  painter->fillRect(QRect(startX, y, selectionWidth, m_cHeight),
                    m_palette.color(QPalette::Highlight));
}

/**
 * @brief Renders a word-wrapped segment with a single uniform text color.
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
 */
void Widgets::Terminal::renderAnsiSegment(QPainter* painter,
                                          const QString& segment,
                                          int segStart,
                                          const QList<CharColor>* colorLine,
                                          const QColor& defaultFg,
                                          int x,
                                          int y)
{
  const auto& fm = painter->fontMetrics();
  int xPos       = x;
  int j          = 0;

  while (j < segment.length()) {
    const int charIndex = segStart + j;
    QColor runFg        = defaultFg;
    QColor runBg;

    if (colorLine && charIndex < colorLine->size()) {
      const CharColor& cc = (*colorLine)[charIndex];
      runFg               = cc.foreground.isValid() ? cc.foreground : defaultFg;
      runBg               = cc.background;
    }

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
 */
void Widgets::Terminal::drawCursor(QPainter* painter, int firstLine, int lastVLine, int lineHeight)
{
  const int cursorLine = m_cursorPosition.y();
  const int cursorCol  = m_cursorPosition.x();
  const bool rtl       = m_translator.rtl();
  const int emptyCursorX =
    rtl ? (width() - m_borderX - painter->fontMetrics().horizontalAdvance(QChar(0x2588)))
        : m_borderX;

  int visualLineY  = m_borderY;
  bool cursorDrawn = false;

  for (int i = firstLine; i <= lastVLine && i < m_data.size(); ++i) {
    const QString& line = m_data[i];

    if (line.isEmpty()) {
      if (i == cursorLine) {
        painter->setPen(m_palette.color(QPalette::Text));
        // code-verify off
        painter->drawText(emptyCursorX, visualLineY + m_cHeight, QStringLiteral("█"));
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
      start        = end;
    }

    if (cursorDrawn)
      break;
  }

  if (!cursorDrawn && cursorLine >= m_data.size()) {
    painter->setPen(m_palette.color(QPalette::Text));
    // code-verify off
    painter->drawText(emptyCursorX, visualLineY + m_cHeight, QStringLiteral("█"));
    // code-verify on
  }
}

/**
 * @brief Paints the "x N" repeat badge after the final wrapped segment of a collapsed row;
 *        skipped when the segment leaves no room before the border (paint-only decoration,
 *        the badge never enters the text buffer).
 */
void Widgets::Terminal::drawRepeatBadge(
  QPainter* painter, int count, int segmentWidth, int y, bool rtlMode)
{
  SS_ASSERT(painter != nullptr, return);
  SS_ASSERT(count > 1, return);

  const QString label   = QStringLiteral("\u00D7 %1").arg(count);
  const int padX        = qMax(2, m_cWidth / 2);
  const int badgeWidth  = m_badgeMetrics.horizontalAdvance(label) + padX * 2;
  const int badgeHeight = qMax(2, m_cHeight - 2);
  const int rightEdge   = width() - m_borderX;

  const int x =
    rtlMode ? (rightEdge - segmentWidth - padX - badgeWidth) : (m_borderX + segmentWidth + padX);
  if (x < m_borderX || x + badgeWidth > rightEdge)
    return;

  const QRect rect(x, y + (m_cHeight - badgeHeight) / 2, badgeWidth, badgeHeight);
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
  painter->setBrush(m_palette.color(QPalette::Highlight));
  painter->drawRoundedRect(rect, badgeHeight / 2.0, badgeHeight / 2.0);
  painter->setPen(m_palette.color(QPalette::Text));
  painter->setFont(m_badgeFont);
  painter->drawText(rect, Qt::AlignCenter, label);
  painter->restore();
}

/**
 * @brief Paints the terminal widget content.
 */
void Widgets::Terminal::paint(QPainter* painter)
{
  if (!isVisible() || !painter)
    return;

  painter->setFont(m_font);
  const int lineHeight = m_cHeight;
  const int firstLine  = m_scrollOffsetY;
  const int lastVLine  = qMin(firstLine + linesPerPage(), lineCount() - 1);

  paintSelectionHighlights(painter, firstLine, lastVLine, lineHeight);
  if (searchActive())
    paintSearchHighlights(painter, firstLine, lastVLine, lineHeight);

  paintTextContent(painter, firstLine, lastVLine, lineHeight);

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
      y     += lineHeight;
      start  = lineEnd;
    }
  }
}

/**
 * @brief Fills the highlight rectangle for the part of one match that falls inside a
 *        word-wrapped segment; the current match paints opaque, the rest translucent.
 */
void Widgets::Terminal::drawSegmentMatch(QPainter* painter,
                                         const QFontMetrics& fm,
                                         const QString& line,
                                         const QPoint& match,
                                         bool isCurrent,
                                         int segStart,
                                         int segEnd,
                                         int y)
{
  const int matchStart = match.x();
  const int matchEnd   = matchStart + static_cast<int>(m_search.query().length());
  const int selStartX  = qMax(matchStart, segStart);
  const int selEndX    = qMin(matchEnd, segEnd);
  if (selStartX >= selEndX)
    return;

  int leadingOffset = 0;
  int matchWidth    = 0;
  for (int j = segStart; j < selEndX; ++j) {
    const int charWidth = fm.horizontalAdvance(line[j]);
    if (j < selStartX)
      leadingOffset += charWidth;
    else
      matchWidth += charWidth;
  }

  const bool rtl     = m_translator.rtl();
  const int maxWidth = width() - 2 * m_borderX;

  int startX = 0;
  if (rtl) {
    const int rightEdge = width() - m_borderX;
    startX              = rightEdge - leadingOffset - matchWidth;
    matchWidth          = qMin(matchWidth, maxWidth);
  }

  else {
    startX     = m_borderX + leadingOffset;
    matchWidth = qMin(matchWidth, maxWidth - leadingOffset);
  }

  QColor fill = m_palette.color(QPalette::Highlight);
  if (!isCurrent)
    fill.setAlpha(110);

  painter->fillRect(QRect(startX, y, matchWidth, m_cHeight), fill);
}

/**
 * @brief Paints search-match highlights for the visible line range, walking the same
 *        wrapped-segment geometry as the selection pass.
 */
void Widgets::Terminal::paintSearchHighlights(QPainter* painter,
                                              int firstLine,
                                              int lastVLine,
                                              int lineHeight)
{
  const QList<QPoint>& searchMatches = m_search.matches();
  if (searchMatches.isEmpty())
    return;

  const QFontMetrics fm = painter->fontMetrics();

  const qsizetype matchCount = searchMatches.size();
  qsizetype k                = 0;
  while (k < matchCount && searchMatches[k].y() < firstLine)
    ++k;

  int y = m_borderY;
  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i) {
    const QString& line = m_data[i];

    const qsizetype lineFirst = k;
    while (k < matchCount && searchMatches[k].y() <= i)
      ++k;

    if (line.isEmpty()) {
      y += lineHeight;
      continue;
    }

    int start = 0;
    while (start < line.length()) {
      const int segEnd = qMin<int>(start + maxCharsPerLine(), line.length());
      for (qsizetype m = lineFirst; m < k; ++m)
        drawSegmentMatch(painter,
                         fm,
                         line,
                         searchMatches[m],
                         m == static_cast<qsizetype>(m_search.currentIndex()),
                         start,
                         segEnd,
                         y);

      y     += lineHeight;
      start  = segEnd;
    }
  }
}

/**
 * @brief Dispatches one text segment to the ANSI/fast/RTL painter helper.
 */
void Widgets::Terminal::paintSegment(QPainter* painter,
                                     const QString& segment,
                                     int segStart,
                                     const QList<CharColor>* colorLine,
                                     const QColor& defaultFg,
                                     int x,
                                     int y,
                                     int ascent,
                                     bool rtlMode)
{
  if (colorLine) {
    renderAnsiSegment(painter, segment, segStart, colorLine, defaultFg, x, y);
    return;
  }

  if (rtlMode) {
    painter->setPen(defaultFg);
    painter->drawText(QPointF(x, y + ascent), segment);
    return;
  }

  renderFastSegment(painter, segment, defaultFg, x, y);
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
  const bool rtlMode            = m_translator.rtl();
  const int rightEdge           = width() - m_borderX;
  const int ascent              = painter->fontMetrics().ascent();
  const auto& fm                = painter->fontMetrics();

  const auto savedDir = painter->layoutDirection();

  for (int i = firstLine; i <= lastVLine && y < height() - m_borderY; ++i) {
    const QString& line = m_data[i];

    if (line.isEmpty()) {
      y += lineHeight;
      continue;
    }

    if (rtlMode) {
      const bool lineHasRtl = lineHasRtlChar(line);
      painter->setLayoutDirection(lineHasRtl ? Qt::RightToLeft : Qt::LeftToRight);
    }

    const QList<CharColor>* colorLine = nullptr;
    if (ansiColors() && i < m_colorData.size())
      colorLine = &m_colorData[i];

    int start = 0;
    while (start < line.length()) {
      const int end         = qMin<int>(start + maxCharsPerLine(), line.length());
      const QString segment = line.mid(start, end - start);
      const int x           = rtlMode ? rightEdge - fm.horizontalAdvance(segment) : m_borderX;

      paintSegment(painter, segment, start, colorLine, defaultTextColor, x, y, ascent, rtlMode);

      if (end == line.length() && i < m_repeatCounts.size() && m_repeatCounts[i] > 1)
        drawRepeatBadge(painter, m_repeatCounts[i], fm.horizontalAdvance(segment), y, rtlMode);

      y     += lineHeight;
      start  = end;
    }
  }

  painter->setLayoutDirection(savedDir);
}

/**
 * @brief Returns the full-height scrollbar track rectangle, mirrored under RTL layouts.
 */
QRect Widgets::Terminal::scrollbarTrackRect() const
{
  const int scrollbarWidth = 6;
  const bool rtl           = m_translator.rtl();
  const int x = rtl ? m_borderX : (static_cast<int>(width()) - scrollbarWidth - m_borderX);
  const int trackHeight = qMax(0, static_cast<int>(height()) - 2 * m_borderY);
  return QRect(x, m_borderY, scrollbarWidth, trackHeight);
}

/**
 * @brief Returns the scrollbar thumb rectangle for the current scroll offset.
 */
QRect Widgets::Terminal::scrollbarThumbRect() const
{
  const QRect track         = scrollbarTrackRect();
  const int availableHeight = track.height();
  const int lines           = qMax(1, lineCount());

  int thumbHeight = qMax(20, availableHeight * availableHeight / lines);
  if (thumbHeight > availableHeight / 2)
    thumbHeight = availableHeight / 2;

  const int denom  = qMax(1, lineCount() - linesPerPage());
  const int travel = qMax(0, availableHeight - thumbHeight);
  const int offset = qBound(0, m_scrollOffsetY, denom);
  const int y      = track.y() + (travel * offset) / denom;
  return QRect(track.x(), y, track.width(), thumbHeight);
}

/**
 * @brief Maps a thumb top Y coordinate back to a scroll offset, clamped to valid range.
 */
int Widgets::Terminal::scrollOffsetForThumbY(const int thumbY) const
{
  const QRect track = scrollbarTrackRect();
  const QRect thumb = scrollbarThumbRect();
  const int travel  = qMax(1, track.height() - thumb.height());
  const int denom   = qMax(0, lineCount() - linesPerPage());
  const int local   = qBound(0, thumbY - track.y(), travel);
  const int offset  = (local * denom + travel / 2) / travel;
  return qBound(0, offset, denom);
}

/**
 * @brief Applies a scrollbar-driven scroll offset with the wheel handler's autoscroll
 *        rules: leaving the bottom disengages autoscroll, reaching it re-engages.
 */
void Widgets::Terminal::applyScrollbarOffset(const int offset)
{
  const int maxOffset = qMax(0, lineCount() - linesPerPage());
  const int clamped   = qBound(0, offset, maxOffset);

  if (clamped < maxOffset && autoscroll())
    setAutoscroll(false);
  else if (clamped >= maxOffset && !autoscroll())
    setAutoscroll(true);

  setScrollOffsetY(clamped);
  m_stateChanged = true;
}

/**
 * @brief Returns true when @p pos falls inside the interactive scrollbar band; the band
 *        only exists while the thumb is visible (manual scrolling, buffer overflow).
 */
bool Widgets::Terminal::isOverScrollbar(const QPoint& pos) const
{
  if (autoscroll() || lineCount() <= linesPerPage())
    return false;

  return scrollbarTrackRect().adjusted(-3, 0, 3, 0).contains(pos);
}

/**
 * @brief Consumes a left press on the scrollbar band: grabs the thumb for dragging or
 *        pages toward a track click; returns false when the press is outside the band.
 */
bool Widgets::Terminal::handleScrollbarPress(const QPoint& pos)
{
  if (!isOverScrollbar(pos))
    return false;

  const QRect thumb = scrollbarThumbRect();
  if (pos.y() >= thumb.top() && pos.y() <= thumb.bottom()) {
    m_draggingScrollbar = true;
    m_dragThumbGrabY    = pos.y() - thumb.y();
    setAutoscroll(false);
  }

  else if (pos.y() < thumb.top())
    applyScrollbarOffset(m_scrollOffsetY - linesPerPage());

  else
    applyScrollbarOffset(m_scrollOffsetY + linesPerPage());

  m_stateChanged = true;
  return true;
}

/**
 * @brief Draws the rounded scrollbar thumb while manual scrolling is active.
 */
void Widgets::Terminal::paintScrollbar(QPainter* painter)
{
  if (autoscroll() || lineCount() <= linesPerPage())
    return;

  const QRect thumb = scrollbarThumbRect();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(Qt::NoPen);
  painter->setBrush(m_palette.color(QPalette::Window));
  painter->drawRoundedRect(thumb, thumb.width() / 2.0, thumb.width() / 2.0);
}

//--------------------------------------------------------------------------------------------------
// Character metrics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the width of a single terminal character.
 */
int Widgets::Terminal::charWidth() const
{
  return m_cWidth;
}

/**
 * @brief Returns the height of a single terminal character.
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
 */
const QFont& Widgets::Terminal::font() const
{
  return m_font;
}

/**
 * @brief Gets the current color palette used by the terminal.
 */
const QPalette& Widgets::Terminal::colorPalette() const
{
  return m_palette;
}

/**
 * @brief Returns true while console display updates are frozen (data keeps flowing to the
 *        raw buffer and export; only this view stops appending).
 */
bool Widgets::Terminal::paused() const
{
  return m_paused;
}

/**
 * @brief Checks if autoscroll is enabled.
 */
bool Widgets::Terminal::autoscroll() const
{
  return m_autoscroll;
}

/**
 * @brief Checks if there is a valid text selection available for copying.
 */
bool Widgets::Terminal::copyAvailable() const
{
  return (!m_selectionEnd.isNull() || !m_selectionStart.isNull()) && !m_data.isEmpty();
}

/**
 * @brief Checks if VT-100 emulation mode is enabled.
 */
bool Widgets::Terminal::vt100emulation() const
{
  return m_emulateVt100;
}

/**
 * @brief Checks if ANSI color support is enabled.
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
 */
int Widgets::Terminal::lineCount() const
{
  return m_data.size();
}

/**
 * @brief Gets the number of lines that can be displayed per page.
 */
int Widgets::Terminal::linesPerPage() const
{
  if (m_cHeight <= 0)
    return 0;

  return static_cast<int>(qFloor((height() - 2 * m_borderY) / m_cHeight));
}

/**
 * @brief Gets the current vertical scroll offset.
 */
int Widgets::Terminal::scrollOffsetY() const
{
  return m_scrollOffsetY;
}

/**
 * @brief Calculates the maximum number of characters that can fit on a single line of the terminal.
 */
int Widgets::Terminal::maxCharsPerLine() const
{
  if (m_cWidth <= 0)
    return 84;

  const auto realValue = (width() - 2 * m_borderX) / m_cWidth;
  return qMax<int>(84, realValue);
}

/**
 * @brief Returns the number of character columns currently visible.
 */
int Widgets::Terminal::terminalColumns() const
{
  return maxCharsPerLine();
}

/**
 * @brief Returns the number of character rows currently visible.
 */
int Widgets::Terminal::terminalRows() const
{
  return linesPerPage();
}

/**
 * @brief Handles key press events and forwards them to the active driver as VT-100 byte sequences.
 */
void Widgets::Terminal::keyPressEvent(QKeyEvent* event)
{
  if (!vt100emulation() || !m_connectionManager.isConnected() || !m_connectionManager.readWrite()) {
    QQuickPaintedItem::keyPressEvent(event);
    return;
  }

  const QByteArray seq = translateKeyToVt100(event);
  if (!seq.isEmpty()) {
    const int deviceId = m_consoleHandler.currentDeviceId();
    if (deviceId >= 0)
      (void)m_connectionManager.writeDataToDevice(deviceId, seq);
    else
      (void)m_connectionManager.writeData(seq);

    event->accept();
    return;
  }

  QQuickPaintedItem::keyPressEvent(event);
}

/**
 * @brief Maps a Qt key event to its VT-100 byte sequence; empty if unmapped.
 */
QByteArray Widgets::Terminal::translateKeyToVt100(const QKeyEvent* event)
{
  QByteArray seq;
  const Qt::KeyboardModifiers mods = event->modifiers();
  const int key                    = event->key();

  if ((mods & Qt::ControlModifier) && key >= Qt::Key_A && key <= Qt::Key_Z) {
    seq.append(char(key - Qt::Key_A + 1));
    return seq;
  }

  if ((mods & Qt::ControlModifier) && key == Qt::Key_BracketLeft) {
    seq.append('\x1b');
    return seq;
  }

  if (key == Qt::Key_Return || key == Qt::Key_Enter)
    return translateEnterKey();

  seq = translateSpecialKey(key);
  if (!seq.isEmpty())
    return seq;

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
  static auto& consoleHandler = Console::Handler::instance();
  switch (consoleHandler.lineEnding()) {
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
  QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);

  if (newGeometry.size() != oldGeometry.size())
    Q_EMIT terminalSizeChanged();
}

/**
 * @brief Gets the current cursor position within the terminal.
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
 */
int Widgets::Terminal::findCharAtPixelX(const QString& line,
                                        int segStart,
                                        int segEnd,
                                        int pixelX) const
{
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
 */
int Widgets::Terminal::calcCursorPixelX(
  QPainter* painter, const QString& line, int segStart, int cursorCol, int segEnd) const
{
  int cursorX     = m_borderX;
  const int limit = qMin(cursorCol, segEnd);
  for (int j = segStart; j < limit; ++j)
    cursorX += painter->fontMetrics().horizontalAdvance(line[j]);

  return cursorX;
}

/**
 * @brief Converts a pixel position to a cursor position.
 */
QPoint Widgets::Terminal::positionToCursor(const QPoint& pos) const
{
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
      continue;
    }

    const int lines = (line.length() + maxCharsPerLine() - 1) / maxCharsPerLine();
    if (remainingY >= lines) {
      remainingY -= lines;
      continue;
    }

    const int segmentStart = qMax(0, remainingY) * maxCharsPerLine();
    const int segmentEnd   = qMin(segmentStart + maxCharsPerLine(), line.length());

    int relX = pos.x() - m_borderX;
    if (m_translator.rtl())
      relX = (width() - m_borderX) - pos.x();

    return QPoint(findCharAtPixelX(line, segmentStart, segmentEnd, relX), i);
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
 */
void Widgets::Terminal::copy()
{
  if (!copyAvailable())
    return;

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
 */
void Widgets::Terminal::selectAll()
{
  if (m_data.isEmpty())
    return;

  m_selectionStart       = QPoint(0, 0);
  int lastLineIndex      = m_data.size() - 1;
  int lastCharIndex      = m_data[lastLineIndex].size();
  m_selectionEnd         = QPoint(lastCharIndex, lastLineIndex);
  m_selectionStartCursor = m_selectionStart;

  m_stateChanged = true;
  Q_EMIT selectionChanged();
}

//--------------------------------------------------------------------------------------------------
// In-buffer search
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a search query is set.
 */
bool Widgets::Terminal::searchActive() const
{
  return m_search.active();
}

/**
 * @brief Returns the number of matches for the active search query.
 */
int Widgets::Terminal::searchMatchCount() const
{
  return m_search.matchCount();
}

/**
 * @brief Returns the 1-based index of the current match (0 when there is none).
 */
int Widgets::Terminal::searchCurrentMatch() const
{
  return m_search.currentMatchNumber();
}

/**
 * @brief Sets the search query and case mode, then rescans the buffer immediately.
 */
void Widgets::Terminal::setSearchQuery(const QString& query, const bool caseSensitive)
{
  if (!m_search.setQuery(query, caseSensitive))
    return;

  refreshSearchMatches();
  update();
}

/**
 * @brief Advances to the next match (wraps) and scrolls it into view.
 */
void Widgets::Terminal::searchNext()
{
  if (m_search.dirty())
    refreshSearchMatches();

  if (!m_search.next())
    return;

  Q_EMIT searchResultsChanged();
  scrollToCurrentMatch();
}

/**
 * @brief Moves to the previous match (wraps) and scrolls it into view.
 */
void Widgets::Terminal::searchPrevious()
{
  if (m_search.dirty())
    refreshSearchMatches();

  if (!m_search.previous())
    return;

  Q_EMIT searchResultsChanged();
  scrollToCurrentMatch();
}

/**
 * @brief Clears all search state and repaints without highlights.
 */
void Widgets::Terminal::clearSearch()
{
  if (!m_search.clear())
    return;

  Q_EMIT searchResultsChanged();
  m_stateChanged = true;
  update();
}

/**
 * @brief Rescans the line buffer for the active query and republishes the result counts;
 *        the clamping that keeps navigation valid after a trim lives in TerminalSearch.
 */
void Widgets::Terminal::refreshSearchMatches()
{
  m_search.refresh(m_data);
  m_stateChanged = true;
  Q_EMIT searchResultsChanged();
}

/**
 * @brief Suspends autoscroll and adjusts the scroll offset so the current match is visible.
 */
void Widgets::Terminal::scrollToCurrentMatch()
{
  int row = m_search.currentRow();
  if (row < 0)
    return;

  setAutoscroll(false);
  SS_ASSERT(row < lineCount(), row = lineCount() - 1);

  int offset = m_scrollOffsetY;
  if (row < offset)
    offset = row;
  else if (row >= offset + linesPerPage())
    offset = row - linesPerPage() + 1;

  setScrollOffsetY(qBound(0, offset, qMax(0, lineCount() - 1)));
  m_stateChanged = true;
  update();
}

//--------------------------------------------------------------------------------------------------
// Style setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the font used for rendering the terminal text.
 */
void Widgets::Terminal::setFont(const QFont& font)
{
  m_font = font;
  m_font.setStyleStrategy(QFont::PreferAntialias);

  auto metrics = QFontMetrics(m_font);
  m_cHeight    = metrics.height();
  m_cWidth     = metrics.horizontalAdvance("M");
  m_borderX    = qMax(m_cWidth, m_cHeight) / 2;
  m_borderY    = qMax(m_cWidth, m_cHeight) / 2;

  m_badgeFont = m_font;
  m_badgeFont.setPointSize(qMax(6, m_font.pointSize() - 2));
  m_badgeMetrics = QFontMetrics(m_badgeFont);

  Q_EMIT fontChanged();
}

/**
 * @brief Freezes or resumes console display updates.
 */
void Widgets::Terminal::setPaused(const bool paused)
{
  if (m_paused == paused)
    return;

  m_paused = paused;
  Q_EMIT pausedChanged();

  m_stateChanged = true;
  update();
}

/**
 * @brief Enables or disables autoscroll.
 */
void Widgets::Terminal::setAutoscroll(const bool enabled)
{
  m_autoscroll = enabled;
  Q_EMIT autoscrollChanged();
}

/**
 * @brief Sets the vertical scroll offset for the terminal.
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
 */
void Widgets::Terminal::setVt100Emulation(const bool enabled)
{
  m_emulateVt100 = enabled;
  Q_EMIT vt100EmulationChanged();
}

/**
 * @brief Enables or disables ANSI color support.
 */
void Widgets::Terminal::setAnsiColors(const bool enabled)
{
  m_ansiColors = enabled;

  if (enabled) {
    m_ansiPalette.resetForeground();
    m_colorData.reserve(qMin(m_maxLines, MAX_UPFRONT_RESERVE));
  }

  Q_EMIT ansiColorsChanged();
}

//--------------------------------------------------------------------------------------------------
// Cursor management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles the visibility of the cursor.
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
 */
void Widgets::Terminal::onThemeChanged()
{
  // clang-format off
  m_stateChanged = true;
  const auto theme = &m_themeManager;
  m_palette.setColor(QPalette::Text, theme->getColor("console_text"));
  m_palette.setColor(QPalette::Base, theme->getColor("console_base"));
  m_palette.setColor(QPalette::Window, theme->getColor("console_border"));
  m_palette.setColor(QPalette::Highlight, theme->getColor("console_highlight"));
  setFillColor(m_palette.color(QPalette::Base));
  m_ansiPalette.rebuild(theme->getColor("console_base"), theme->getColor("console_text"));
  // clang-format on

  if (ansiColors())
    m_ansiPalette.resetForeground();

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
  append(m_translator.welcomeConsoleText());

  auto key = QKeySequence(QStringLiteral("Ctrl+K")).toString(QKeySequence::NativeText);
#ifdef Q_OS_MAC
  // code-verify off (glyph must match the command symbol NativeText emits)
  key = key.replace("⌘", "⌘+");
  // code-verify on
#endif

  append(tr("Tip: Press %1 anywhere to open the command palette.").arg(key) + QStringLiteral("\n"));
  setAutoscroll(true);

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
 * @brief Appends a string of data to the terminal, processing each character accordingly.
 */
/**
 * @brief Scans a printable-character run starting at @p pos in @p data; returns end offset.
 */
int Widgets::Terminal::scanPrintableRun(const QString& data, int pos)
{
  const int len = data.size();
  while (pos < len) {
    const auto ch = data[pos].unicode();

    if (ch == 0x1b || ch == '\n' || ch == '\r' || ch == '\b' || ch == 0x7F || ch == '\t')
      break;

    if (ch < 0x20)
      break;

    ++pos;
  }

  return pos;
}

/**
 * @brief Returns true when @p line contains any RTL-direction character.
 */
bool Widgets::Terminal::lineHasRtlChar(QStringView line)
{
  for (const QChar c : line) {
    const auto dir = c.direction();
    if (dir == QChar::DirR || dir == QChar::DirAL)
      return true;
  }

  return false;
}

/**
 * @brief Appends data to the terminal, processing it through the VT-100 state machine.
 */
void Widgets::Terminal::append(const QString& data)
{
  if (m_paused)
    return;

  QString text;
  text.reserve(data.size());

  int pos       = 0;
  const int len = data.size();

  while (pos < len) {
    if (m_ansi.inTextState()) [[likely]] {
      const int runStart = pos;
      pos                = scanPrintableRun(data, pos);

      if (pos > runStart)
        text.append(QStringView(data).mid(runStart, pos - runStart));

      if (pos < len && m_ansi.inTextState()) {
        processText(data[pos], text);
        ++pos;
      }

      continue;
    }

    m_ansi.feed(data[pos]);
    ++pos;
  }

  appendString(text);
  m_stateChanged = true;
}

//--------------------------------------------------------------------------------------------------
// Buffer management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-reads the scrollback setting and trims the buffer front when it now exceeds
 *        the cap, squeezing afterwards because a QList front-erase strands its capacity;
 *        raising the cap only lets future appends grow further.
 */
void Widgets::Terminal::applyScrollbackLimit()
{
  m_maxLines = m_consoleHandler.scrollbackLines();
  SS_ASSERT(m_maxLines >= 100 && m_maxLines <= 100000,
            m_maxLines = qBound(100, m_maxLines, 100000));

  const int excess = static_cast<int>(m_data.size()) - m_maxLines;
  if (excess > 0) {
    trimExcessLines(excess);
    m_data.squeeze();
    m_colorData.squeeze();
    m_repeatCounts.squeeze();
  }

  m_stateChanged = true;
  update();
}

/**
 * @brief Drops @p linesToDrop rows from the front of the buffer, keeping the color rows,
 *        repeat counts, cursor, scroll offset, selection, and search state in lockstep.
 */
void Widgets::Terminal::trimExcessLines(int linesToDrop)
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

  if (!m_repeatCounts.isEmpty()) {
    const int countDrop = qMin(linesToDrop, static_cast<int>(m_repeatCounts.size()));
    m_repeatCounts.erase(m_repeatCounts.begin(), m_repeatCounts.begin() + countDrop);
  }

  m_search.markDirty();
  // code-verify on

  if (m_cursorPosition.y() >= linesToDrop)
    m_cursorPosition.setY(m_cursorPosition.y() - linesToDrop);
  else
    m_cursorPosition.setY(0);

  setScrollOffsetY(qMax(0, m_scrollOffsetY - linesToDrop));

  if (!m_selectionEnd.isNull() || !m_selectionStart.isNull()) {
    if (m_selectionEnd.y() < linesToDrop) {
      m_selectionStart = QPoint();
      m_selectionEnd   = QPoint();
    } else {
      m_selectionEnd.ry() -= linesToDrop;
      if (m_selectionStart.y() >= linesToDrop)
        m_selectionStart.ry() -= linesToDrop;
      else
        m_selectionStart = QPoint(0, 0);
    }

    Q_EMIT selectionChanged();
  }

  if (!m_selectionStartCursor.isNull()) {
    if (m_selectionStartCursor.y() >= linesToDrop)
      m_selectionStartCursor.ry() -= linesToDrop;
    else
      m_selectionStartCursor = QPoint(0, 0);
  }

  m_stateChanged = true;
}

/**
 * @brief Appends a string to the terminal's data buffer, updating the cursor position.
 */
void Widgets::Terminal::appendString(QStringView string)
{
  const int linesToDrop = m_data.size() - m_maxLines + 1;
  if (m_data.size() >= m_maxLines && linesToDrop > 0)
    trimExcessLines(linesToDrop);

  for (const auto& character : string) {
    int cursorX = m_cursorPosition.x();
    int cursorY = m_cursorPosition.y();
    replaceData(cursorX, cursorY, character);
    setCursorPosition(cursorX + 1, cursorY);

    if (m_cursorPosition.x() >= maxCharsPerLine())
      setCursorPosition(0, m_cursorPosition.y() + 1);
  }

  if (autoscroll()) {
    int cursorLine   = m_cursorPosition.y();
    int wrappedLines = 1;
    if (cursorLine < m_data.size()) {
      int lineLength = m_data[cursorLine].length();
      wrappedLines   = (lineLength + maxCharsPerLine() - 1) / maxCharsPerLine();
    }

    int visualBottom = cursorLine + wrappedLines - 1;

    m_scrollOffsetY = qMax(0, visualBottom - linesPerPage() + 1);
    if (isVisible())
      Q_EMIT scrollOffsetYChanged();
  }
}

/**
 * @brief Returns true when @p line starts with a well-formed "HH:mm:ss.zzz -> " stamp.
 */
bool Widgets::Terminal::hasTimestampPrefix(QStringView line)
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
QStringView Widgets::Terminal::lineContentView(QStringView line) const
{
  constexpr int kStampLen = 16;

  if (!m_consoleHandler.showTimestamp() || !hasTimestampPrefix(line))
    return line;

  return line.mid(kStampLen);
}

/**
 * @brief Merges the just-completed line into the previous row when duplicate collapsing is
 *        on and both rows carry identical content; returns true when the row was merged so
 *        the caller reuses the freed row index for the next line.
 */
bool Widgets::Terminal::collapseCompletedLine()
{
  if (!m_collapseDuplicates)
    return false;

  const int y = m_cursorPosition.y();
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

  m_search.markDirty();

  if (autoscroll()) {
    const int lastRow = m_data.size() - 1;
    const int length  = m_data[lastRow].length();
    const int wrapped = qMax(1, (length + maxCharsPerLine() - 1) / maxCharsPerLine());

    m_scrollOffsetY = qMax(0, lastRow + wrapped - linesPerPage());
    if (isVisible())
      Q_EMIT scrollOffsetYChanged();
  }

  m_stateChanged = true;
  return true;
}

/**
 * @brief Removes characters from the terminal buffer starting from the cursor position.
 */
void Widgets::Terminal::removeStringFromCursor(const Direction direction, int len)
{
  const auto positionX = m_cursorPosition.x();
  const auto positionY = m_cursorPosition.y();

  if (len < 0)
    len = INT_MAX;

  int removeSize = 0;
  if (direction == RightDirection) {
    const qsizetype lineLen =
      (positionY >= 0 && positionY < m_data.size()) ? m_data[positionY].size() : qsizetype(0);
    const qsizetype l1 = lineLen - positionX;
    const qsizetype l2 = static_cast<qsizetype>(len);
    removeSize         = static_cast<int>(qMin(qMax(l1, qsizetype(0)), l2));
  }

  else
    removeSize = qMin(len, m_cursorPosition.x());

  int offset = 0;
  const QChar clearChar('\x7F');
  for (int i = 0; i < removeSize; ++i) {
    if (direction == LeftDirection)
      offset = -1 - i;
    else if (direction == RightDirection)
      offset = i;

    replaceData(m_cursorPosition.x() + offset, positionY, clearChar);
  }
}

/**
 * @brief Initializes the terminal's data buffer.
 */
void Widgets::Terminal::initBuffer()
{
  const int reserveLines = qMin(m_maxLines, MAX_UPFRONT_RESERVE);

  m_data.clear();
  m_data.squeeze();
  m_scrollOffsetY = 0;
  m_data.reserve(reserveLines);
  m_colorData.clear();
  m_colorData.squeeze();
  m_repeatCounts.clear();
  m_repeatCounts.squeeze();
  m_repeatCounts.reserve(reserveLines);
  m_search.markDirty();

  if (ansiColors()) {
    m_colorData.reserve(reserveLines);
    m_ansiPalette.resetForeground();
  }
}

//--------------------------------------------------------------------------------------------------
// Text lane
//--------------------------------------------------------------------------------------------------

/**
 * @brief Processes a single character in the context of normal text input.
 */
void Widgets::Terminal::processText(const QChar& byte, QString& text)
{
  const ushort code = byte.unicode();
  const bool vt     = vt100emulation();

  if (code == '\n') {
    appendString(text);
    text.clear();

    const int next_row = m_cursorPosition.y() + (collapseCompletedLine() ? 0 : 1);
    setCursorPosition(0, next_row);
    return;
  }

  if (!vt) {
    if (byte.isPrint())
      text.append(byte);

    return;
  }

  switch (code) {
    case 0x1b:
      appendString(text);
      text.clear();
      m_ansi.beginEscape();
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

//--------------------------------------------------------------------------------------------------
// Escape-sequence sink
//--------------------------------------------------------------------------------------------------

/**
 * @brief AnsiSink: reports the cursor the escape sequences are addressing.
 */
QPoint Widgets::Terminal::currentCursor() const
{
  return m_cursorPosition;
}

/**
 * @brief AnsiSink: moves the cursor, clamped to the buffer geometry by setCursorPosition().
 */
void Widgets::Terminal::moveCursor(const QPoint& position)
{
  setCursorPosition(position);
}

/**
 * @brief AnsiSink: hides or shows the blinking cursor (DECTCEM).
 */
void Widgets::Terminal::setCursorHidden(bool hidden)
{
  m_cursorHidden = hidden;
  m_stateChanged = true;
}

/**
 * @brief AnsiSink: applies an SGR parameter run, but only while ANSI colors are on; the
 *        parser stays unaware of the toggle so its behavior does not fork.
 */
void Widgets::Terminal::applySgrCodes(const QList<int>& codes)
{
  if (ansiColors())
    m_ansiPalette.applySgr(codes, m_palette.color(QPalette::Text));
}

/**
 * @brief AnsiSink: blanks characters on either side of the cursor within its row.
 */
void Widgets::Terminal::eraseFromCursor(AnsiEraseDirection direction, int length)
{
  const Direction sense = (direction == AnsiEraseDirection::Left) ? LeftDirection : RightDirection;
  removeStringFromCursor(sense, length);
}

/**
 * @brief AnsiSink: drops every row below @p row, keeping the color rows and repeat counts in
 *        lockstep so the paint pass never reads a misaligned row.
 */
void Widgets::Terminal::eraseRowsAfter(int row)
{
  if (row + 1 >= m_data.size())
    return;

  m_data.erase(m_data.begin() + row + 1, m_data.end());
  if (ansiColors() && row + 1 < m_colorData.size())
    m_colorData.erase(m_colorData.begin() + row + 1, m_colorData.end());

  if (row + 1 < m_repeatCounts.size())
    m_repeatCounts.erase(m_repeatCounts.begin() + row + 1, m_repeatCounts.end());

  m_search.markDirty();
}

/**
 * @brief AnsiSink: drops every row above @p row, keeping the color rows and repeat counts in
 *        lockstep.
 */
void Widgets::Terminal::eraseRowsBefore(int row)
{
  if (row <= 0)
    return;

  m_data.erase(m_data.begin(), m_data.begin() + row);
  if (ansiColors() && row < m_colorData.size())
    m_colorData.erase(m_colorData.begin(), m_colorData.begin() + row);

  const auto countDrop = qMin<qsizetype>(row, m_repeatCounts.size());
  if (countDrop > 0)
    m_repeatCounts.erase(m_repeatCounts.begin(), m_repeatCounts.begin() + countDrop);

  m_search.markDirty();
}

/**
 * @brief AnsiSink: erases the whole display (CSI 2J / 3J).
 */
void Widgets::Terminal::eraseAllRows()
{
  clear();
}

//--------------------------------------------------------------------------------------------------
// Message formatting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a debug message with optional ANSI colors.
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
 */
void Widgets::Terminal::setCursorPosition(const QPoint& position)
{
  const QPoint clamped(qBound(0, position.x(), maxCharsPerLine()),
                       qBound(0, position.y(), m_maxLines));
  if (m_cursorPosition != clamped) {
    m_cursorPosition = clamped;
    Q_EMIT cursorMoved();
  }
}

/**
 * @brief Sets the cursor position to specified coordinates.
 */
void Widgets::Terminal::setCursorPosition(const int x, const int y)
{
  setCursorPosition(QPoint(x, y));
}

/**
 * @brief Replaces or inserts a character in the terminal buffer at a specified position.
 */
void Widgets::Terminal::replaceData(int x, int y, const QChar& byte)
{
  if (y >= m_data.size())
    m_data.resize(y + 1);

  while (m_repeatCounts.size() < m_data.size())
    m_repeatCounts.append(1);

  m_search.markDirty();
  QString& line = m_data[y];

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

    const CharColor charColor(m_ansiPalette.foreground(), m_ansiPalette.background());
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
 */
bool Widgets::Terminal::shouldEndSelection(const QChar& c)
{
  bool end  = false;
  end      |= c.isSpace();
  end      |= c.isNonCharacter();
  end      |= (!c.isLetter() && !c.isNumber());
  return end;
}

//--------------------------------------------------------------------------------------------------
// Input event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles mouse wheel events for scrolling the terminal content.
 */
void Widgets::Terminal::wheelEvent(QWheelEvent* event)
{
  int numSteps    = 0;
  auto pixelDelta = event->pixelDelta();
  auto angleDelta = event->angleDelta();
  if (!pixelDelta.isNull())
    numSteps = pixelDelta.y();
  else
    numSteps = angleDelta.y();

  if (numSteps > 0)
    numSteps = qMax(1, numSteps / m_cHeight);
  else if (numSteps < 0)
    numSteps = qMin(-1, numSteps / m_cHeight);

  if (numSteps > 0 && autoscroll() && linesPerPage() < lineCount())
    setAutoscroll(false);

  if (numSteps != 0) {
    const int maxScrollOffset = qMax(0, lineCount() - linesPerPage() + 2);

    int offset = m_scrollOffsetY - numSteps;

    offset = qMax(0, offset);
    offset = qMin(offset, maxScrollOffset);

    if (offset == maxScrollOffset && !autoscroll())
      setAutoscroll(true);

    setScrollOffsetY(offset);
  }

  m_stateChanged = true;
  event->accept();
}

/**
 * @brief Handles mouse move events for updating the text selection.
 */
void Widgets::Terminal::mouseMoveEvent(QMouseEvent* event)
{
  if (m_draggingScrollbar) {
    const int thumbY = event->position().toPoint().y() - m_dragThumbGrabY;
    applyScrollbarOffset(scrollOffsetForThumbY(thumbY));
    return;
  }

  if (!m_mouseTracking)
    return;

  QPoint currentCursorPos = positionToCursor(event->pos());

  if ((m_selectionStartCursor.y() > currentCursorPos.y())
      || (m_selectionStartCursor.y() == currentCursorPos.y()
          && m_selectionStartCursor.x() > currentCursorPos.x())) {
    m_selectionStart = currentCursorPos;
    m_selectionEnd   = m_selectionStartCursor;
  }

  else {
    m_selectionStart = m_selectionStartCursor;
    m_selectionEnd   = currentCursorPos;
  }

  m_stateChanged = true;
  Q_EMIT selectionChanged();
}

/**
 * @brief Handles mouse press events for starting text selection.
 */
void Widgets::Terminal::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    if (handleScrollbarPress(event->position().toPoint())) {
      forceActiveFocus();
      return;
    }

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
 */
void Widgets::Terminal::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton && m_draggingScrollbar) {
    m_draggingScrollbar = false;
    applyScrollbarOffset(m_scrollOffsetY);
    return;
  }

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
 * @brief Clears drag and selection-tracking state when the mouse grab is taken away.
 */
void Widgets::Terminal::mouseUngrabEvent()
{
  m_mouseTracking        = false;
  m_draggingScrollbar    = false;
  m_selectionStartCursor = QPoint();
  QQuickPaintedItem::mouseUngrabEvent();
}

/**
 * @brief Handles mouse double-click events for selecting the word under the cursor.
 */
void Widgets::Terminal::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (isOverScrollbar(event->position().toPoint()))
    return;

  auto cursorPos = positionToCursor(event->pos());
  if (cursorPos.y() >= 0 && cursorPos.y() < m_data.size()) {
    const QString& line = m_data[cursorPos.y()];

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
