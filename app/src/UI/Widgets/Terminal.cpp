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
 */
Widgets::Terminal::Terminal(QQuickItem* parent)
  : QQuickPaintedItem(parent)
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

  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setActiveFocusOnTab(true);
  setMipmap(true);
  setOpaquePainting(true);

  loadWelcomeGuide();
  setFont(Console::Handler::instance().font());
  connect(&Console::Handler::instance(), &Console::Handler::fontChanged, this, [this] {
    setFont(Console::Handler::instance().font());
  });

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
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged,
          this,
          &Widgets::Terminal::loadWelcomeGuide);
#endif

  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged, this, [this] {
    loadWelcomeGuide();
  });

  m_cursorTimer.start(200);
  m_cursorTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_cursorTimer, &QTimer::timeout, this, &Widgets::Terminal::toggleCursor);

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

  const bool rtl     = Misc::Translator::instance().rtl();
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
  const bool rtl       = Misc::Translator::instance().rtl();
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
  const bool rtlMode            = Misc::Translator::instance().rtl();
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

      y     += lineHeight;
      start  = end;
    }
  }

  painter->setLayoutDirection(savedDir);
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

  const bool rtl = Misc::Translator::instance().rtl();
  const int x    = rtl ? m_borderX : (width() - scrollbarWidth - m_borderX);
  int y          = (m_scrollOffsetY / static_cast<float>(lineCount() - linesPerPage()))
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
  if (!vt100emulation() || !IO::ConnectionManager::instance().isConnected()
      || !IO::ConnectionManager::instance().readWrite()) {
    QQuickPaintedItem::keyPressEvent(event);
    return;
  }

  const QByteArray seq = translateKeyToVt100(event);
  if (!seq.isEmpty()) {
    const int deviceId = Console::Handler::instance().currentDeviceId();
    if (deviceId >= 0)
      (void)IO::ConnectionManager::instance().writeDataToDevice(deviceId, seq);
    else
      (void)IO::ConnectionManager::instance().writeData(seq);

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
    if (Misc::Translator::instance().rtl())
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

  Q_EMIT fontChanged();
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
  const auto theme = &Misc::ThemeManager::instance();
  m_palette.setColor(QPalette::Text, theme->getColor("console_text"));
  m_palette.setColor(QPalette::Base, theme->getColor("console_base"));
  m_palette.setColor(QPalette::Window, theme->getColor("console_border"));
  m_palette.setColor(QPalette::Highlight, theme->getColor("console_highlight"));
  setFillColor(m_palette.color(QPalette::Base));
  updateAnsiColorPalette();
  // clang-format on

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
  QString text;
  text.reserve(data.size());

  int pos       = 0;
  const int len = data.size();

  while (pos < len) {
    if (m_state == Text) [[likely]] {
      const int runStart = pos;
      pos                = scanPrintableRun(data, pos);

      if (pos > runStart)
        text.append(QStringView(data).mid(runStart, pos - runStart));

      if (pos < len && m_state == Text) {
        processText(data[pos], text);
        ++pos;
      }

      continue;
    }

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
 * @brief Appends a string to the terminal's data buffer, updating the cursor position.
 */
void Widgets::Terminal::appendString(QStringView string)
{
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
      offset = -1;
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
  m_data.clear();
  m_data.squeeze();
  m_scrollOffsetY = 0;
  m_data.reserve(MAX_LINES);
  m_colorData.clear();
  m_colorData.squeeze();

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
 */
void Widgets::Terminal::processText(const QChar& byte, QString& text)
{
  const ushort code = byte.unicode();
  const bool vt     = vt100emulation();

  if (code == '\n') {
    appendString(text);
    text.clear();
    setCursorPosition(0, m_cursorPosition.y() + 1);
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
 */
void Widgets::Terminal::processFormat(const QChar& byte, QString& text)
{
  (void)text;

  if (byte >= '0' && byte <= '9') {
    static constexpr int kMaxCsiParam = 1000000;
    if (m_currentFormatValue < kMaxCsiParam)
      m_currentFormatValue = m_currentFormatValue * 10 + (byte.cell() - '0');

    return;
  }

  if (byte == '?' || byte == '>' || byte == '=') {
    m_privateMode = true;
    return;
  }

  if (byte == ';') {
    m_formatValues.append(m_currentFormatValue);
    m_currentFormatValue = 0;
    m_state              = Format;
    return;
  }

  if (dispatchCsiFinal(byte))
    return;

  m_state = Text;
}

/**
 * @brief Dispatches the final byte of a CSI sequence; returns true if handled.
 */
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
      if ((final >= 'A' && final <= 'Z') || (final >= 'a' && final <= 'z')) {
        m_state = Text;
        return true;
      }

      return false;
  }
}

/**
 * @brief Handles CSI cursor movement letters A-F (CUU/CUD/CUF/CUB/CNL/CPL).
 */
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

/**
 * @brief Handles CSI absolute cursor placement (H/f/G/d).
 */
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

/**
 * @brief Handles CSI Erase-in-Display (J).
 */
void Widgets::Terminal::handleCsiEraseDisplay()
{
  if (m_privateMode)
    return;

  const int cy = m_cursorPosition.y();
  switch (m_currentFormatValue) {
    case 0:
      removeStringFromCursor(RightDirection);
      if (cy + 1 < m_data.size()) {
        m_data.erase(m_data.begin() + cy + 1, m_data.end());
        if (ansiColors() && cy + 1 < m_colorData.size())
          m_colorData.erase(m_colorData.begin() + cy + 1, m_colorData.end());
      }

      break;
    case 1:
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

/**
 * @brief Handles CSI Erase-in-Line (K).
 */
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

/**
 * @brief Handles CSI DEC private mode set/reset for cursor visibility (h/l).
 */
void Widgets::Terminal::handleCsiDecPrivateMode(const QChar& byte)
{
  if (m_privateMode && m_currentFormatValue == 25) {
    m_cursorHidden = (byte == 'l');
    m_stateChanged = true;
  }
}

/**
 * @brief Processes a reset-font terminator and returns the state machine to Text.
 */
void Widgets::Terminal::processResetFont(const QChar& byte, QString& text)
{
  (void)byte;
  (void)text;
  m_state = Text;
}

/**
 * @brief Consumes one byte while in OSC state (BEL terminator or ESC -> CSI).
 */
void Widgets::Terminal::processOsc(const QChar& byte)
{
  const char latin = byte.toLatin1();
  if (latin == 0x07)
    m_state = Text;

  else if (latin == 0x1b)
    m_state = Escape;
}

/**
 * @brief Consumes one byte while ignoring an unknown CSI sequence.
 */
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
 */
void Widgets::Terminal::updateAnsiColorPalette()
{
  const auto theme         = &Misc::ThemeManager::instance();
  const QColor consoleBase = theme->getColor("console_base");
  const QColor consoleText = theme->getColor("console_text");
  const bool isDarkTheme   = consoleText.lightness() > consoleBase.lightness();

  if (isDarkTheme) {
    m_ansiStandardColors[0] = QColor(0, 0, 0);
    m_ansiStandardColors[1] = QColor(205, 49, 49);
    m_ansiStandardColors[2] = QColor(13, 188, 121);
    m_ansiStandardColors[3] = QColor(229, 229, 16);
    m_ansiStandardColors[4] = QColor(36, 114, 200);
    m_ansiStandardColors[5] = QColor(188, 63, 188);
    m_ansiStandardColors[6] = QColor(17, 168, 205);
    m_ansiStandardColors[7] = QColor(229, 229, 229);

    m_ansiBrightColors[0] = QColor(102, 102, 102);
    m_ansiBrightColors[1] = QColor(241, 76, 76);
    m_ansiBrightColors[2] = QColor(35, 209, 139);
    m_ansiBrightColors[3] = QColor(245, 245, 67);
    m_ansiBrightColors[4] = QColor(59, 142, 234);
    m_ansiBrightColors[5] = QColor(214, 112, 214);
    m_ansiBrightColors[6] = QColor(41, 184, 219);
    m_ansiBrightColors[7] = QColor(255, 255, 255);
  }

  else {
    m_ansiStandardColors[0] = QColor(0, 0, 0);
    m_ansiStandardColors[1] = QColor(170, 0, 0);
    m_ansiStandardColors[2] = QColor(0, 140, 0);
    m_ansiStandardColors[3] = QColor(170, 140, 0);
    m_ansiStandardColors[4] = QColor(0, 0, 170);
    m_ansiStandardColors[5] = QColor(170, 0, 170);
    m_ansiStandardColors[6] = QColor(0, 140, 170);
    m_ansiStandardColors[7] = QColor(170, 170, 170);

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
 */
void Widgets::Terminal::applyAnsiColor(const QList<int>& codes)
{
  for (int i = 0; i < codes.size(); ++i)
    i += applyAnsiSgrCode(codes, i);
}

/**
 * @brief Applies one SGR code at @p i in @p codes; returns number of extra params consumed.
 */
int Widgets::Terminal::applyAnsiSgrCode(const QList<int>& codes, int i)
{
  const int code = codes[i];

  if (code == 0) {
    m_currentColor   = QColor();
    m_currentBgColor = QColor();
    return 0;
  }

  if (code == 1) {
    if (!m_currentColor.isValid())
      m_currentColor = m_palette.color(QPalette::Text);

    m_currentColor = m_currentColor.lighter(130);
    return 0;
  }

  if (code >= 30 && code <= 37) {
    m_currentColor = m_ansiStandardColors[code - 30];
    return 0;
  }

  if (code >= 40 && code <= 47) {
    m_currentBgColor = m_ansiStandardColors[code - 40];
    return 0;
  }

  if (code >= 90 && code <= 97) {
    m_currentColor = m_ansiBrightColors[code - 90];
    return 0;
  }

  if (code >= 100 && code <= 107) {
    m_currentBgColor = m_ansiBrightColors[code - 100];
    return 0;
  }

  const bool isFg = (code == 38);
  const bool isBg = (code == 48);
  if (!isFg && !isBg)
    return 0;

  QColor& target = isFg ? m_currentColor : m_currentBgColor;

  if (i + 2 < codes.size() && codes[i + 1] == 5) {
    target = getColor256(codes[i + 2]);
    return 2;
  }

  if (i + 4 < codes.size() && codes[i + 1] == 2) {
    target = QColor(codes[i + 2], codes[i + 3], codes[i + 4]);
    return 4;
  }

  return 0;
}

/**
 * @brief Converts a 256-color palette index to a QColor.
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
  if (index < 8) {
    static const QColor standard[8] = {
      QColor(0, 0, 0),
      QColor(170, 0, 0),
      QColor(0, 170, 0),
      QColor(170, 85, 0),
      QColor(0, 0, 170),
      QColor(170, 0, 170),
      QColor(0, 170, 170),
      QColor(170, 170, 170),
    };
    return standard[index];
  }

  if (index < 16) {
    static const QColor bright[8] = {
      QColor(85, 85, 85),
      QColor(255, 85, 85),
      QColor(85, 255, 85),
      QColor(255, 255, 85),
      QColor(85, 85, 255),
      QColor(255, 85, 255),
      QColor(85, 255, 255),
      QColor(255, 255, 255),
    };
    return bright[index - 8];
  }

  if (index < 232) {
    const int adjusted = index - 16;
    const int r        = (adjusted / 36) % 6;
    const int g        = (adjusted / 6) % 6;
    const int b        = adjusted % 6;

    return QColor(r ? (r * 40 + 55) : 0, g ? (g * 40 + 55) : 0, b ? (b * 40 + 55) : 0);
  }

  const int gray = 8 + (index - 232) * 10;
  return QColor(gray, gray, gray);
}

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
                       qBound(0, position.y(), MAX_LINES));
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
 * @brief Handles mouse double-click events for selecting the word under the cursor.
 */
void Widgets::Terminal::mouseDoubleClickEvent(QMouseEvent* event)
{
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
