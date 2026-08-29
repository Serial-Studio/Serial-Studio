/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#pragma once

#include <QColor>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QPalette>
#include <QQuickPaintedItem>
#include <QRect>
#include <QTimer>

#include "UI/Widgets/Terminal/AnsiPalette.h"
#include "UI/Widgets/Terminal/AnsiStateMachine.h"
#include "UI/Widgets/Terminal/TerminalBuffer.h"
#include "UI/Widgets/Terminal/TerminalSearch.h"

namespace Console {
class Handler;
}  // namespace Console

namespace IO {
class ConnectionManager;
}  // namespace IO

#ifdef BUILD_COMMERCIAL
namespace Licensing {
class LemonSqueezy;
}  // namespace Licensing
#endif

namespace Misc {
class ThemeManager;
class Translator;
class TimerEvents;
}  // namespace Misc

namespace Widgets {
/**
 * @brief QML terminal widget with optional VT-100 emulation. The widget owns the painter, the
 *        selection and the input handlers; the line store, the escape-sequence parser, the ANSI
 *        color palette and the in-buffer search are separate objects it drives. AnsiSink is
 *        inherited privately: the state machine is the only caller of those operations.
 */
class Terminal
  : public QQuickPaintedItem
  , private AnsiSink {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QFont font
             READ font
             WRITE setFont
             NOTIFY fontChanged)
  Q_PROPERTY(int charWidth
             READ charWidth
             NOTIFY fontChanged)
  Q_PROPERTY(int charHeight
             READ charHeight
             NOTIFY fontChanged)
  Q_PROPERTY(bool autoscroll
             READ autoscroll
             WRITE setAutoscroll
             NOTIFY autoscrollChanged)
  Q_PROPERTY(QPalette colorPalette
             READ colorPalette
             WRITE setColorPalette
             NOTIFY colorPaletteChanged)
  Q_PROPERTY(bool copyAvailable
             READ copyAvailable
             NOTIFY selectionChanged)
  Q_PROPERTY(bool vt100emulation
             READ vt100emulation
             WRITE setVt100Emulation
             NOTIFY vt100EmulationChanged)
  Q_PROPERTY(int scrollOffsetY
             READ scrollOffsetY
             WRITE setScrollOffsetY
             NOTIFY scrollOffsetYChanged)
  Q_PROPERTY(bool ansiColors
             READ ansiColors
             WRITE setAnsiColors
             NOTIFY ansiColorsChanged)
  Q_PROPERTY(int terminalColumns
             READ terminalColumns
             NOTIFY terminalSizeChanged)
  Q_PROPERTY(int terminalRows
             READ terminalRows
             NOTIFY terminalSizeChanged)
  Q_PROPERTY(bool paused
             READ paused
             WRITE setPaused
             NOTIFY pausedChanged)
  Q_PROPERTY(bool searchActive
             READ searchActive
             NOTIFY searchResultsChanged)
  Q_PROPERTY(int searchMatchCount
             READ searchMatchCount
             NOTIFY searchResultsChanged)
  Q_PROPERTY(int searchCurrentMatch
             READ searchCurrentMatch
             NOTIFY searchResultsChanged)
  // clang-format on

signals:
  void fontChanged();
  void cursorMoved();
  void selectionChanged();
  void autoscrollChanged();
  void colorPaletteChanged();
  void copyAvailableChanged();
  void scrollOffsetYChanged();
  void vt100EmulationChanged();
  void pausedChanged();
  void ansiColorsChanged();
  void terminalSizeChanged();
  void searchResultsChanged();

public:
  Terminal(QQuickItem* parent = 0);
  void paint(QPainter* painter) override;

  enum Direction {
    LeftDirection,
    RightDirection
  };

  Q_ENUM(Direction);

  [[nodiscard]] int charWidth() const;
  [[nodiscard]] int charHeight() const;

  [[nodiscard]] const QFont& font() const;
  [[nodiscard]] const QPalette& colorPalette() const;

  [[nodiscard]] bool paused() const;
  [[nodiscard]] bool autoscroll() const;
  [[nodiscard]] bool ansiColors() const;
  [[nodiscard]] bool copyAvailable() const;
  [[nodiscard]] bool vt100emulation() const;

  [[nodiscard]] int lineCount() const;
  [[nodiscard]] int linesPerPage() const;
  [[nodiscard]] bool searchActive() const;
  [[nodiscard]] int searchMatchCount() const;
  [[nodiscard]] int searchCurrentMatch() const;
  [[nodiscard]] int scrollOffsetY() const;
  [[nodiscard]] int maxCharsPerLine() const;
  [[nodiscard]] int terminalColumns() const;
  [[nodiscard]] int terminalRows() const;

  [[nodiscard]] const QPoint& cursorPosition() const;
  [[nodiscard]] QPoint positionToCursor(const QPoint& pos) const;

  static QString formatDebugMessage(QtMsgType type, const QString& message, bool useAnsiColors);

public slots:
  void copy();
  void clear();
  void selectAll();
  void setFont(const QFont& font);
  void setPaused(const bool paused);
  void setAutoscroll(const bool enabled);
  void setScrollOffsetY(const int offset);
  void setColorPalette(const QPalette& palette);
  void setAnsiColors(const bool enabled);
  void setVt100Emulation(const bool enabled);
  void setSearchQuery(const QString& query, const bool caseSensitive);
  void searchNext();
  void searchPrevious();
  void clearSearch();

private slots:
  void toggleCursor();
  void onThemeChanged();
  void loadWelcomeGuide();
  void applyScrollbackLimit();
  void append(const QString& data);
  void appendString(QStringView string);
  void removeStringFromCursor(const Widgets::Terminal::Direction direction = RightDirection,
                              int len                                      = INT_MAX);

private:
  void initBuffer();
  void syncAutoscroll();
  void syncBufferGeometry();
  void drainBufferEvents();
  void applyLineDrop(int linesToDrop);
  void refreshSearchMatches();
  void scrollToCurrentMatch();

  [[nodiscard]] QPoint currentCursor() const override;
  void eraseAllRows() override;
  void eraseRowsAfter(int row) override;
  void eraseRowsBefore(int row) override;
  void setCursorHidden(bool hidden) override;
  void moveCursor(const QPoint& position) override;
  void applySgrCodes(const QList<int>& codes) override;
  void eraseFromCursor(AnsiEraseDirection direction, int length) override;

  void setCursorPosition(const QPoint& position);
  void setCursorPosition(const int x, const int y);

  [[nodiscard]] int findCharAtPixelX(const QString& line,
                                     int segStart,
                                     int segEnd,
                                     int pixelX) const;
  [[nodiscard]] int calcCursorPixelX(
    QPainter* painter, const QString& line, int segStart, int cursorCol, int segEnd) const;
  void drawCursor(QPainter* painter, int firstLine, int lastVLine, int lineHeight);
  void drawRepeatBadge(QPainter* painter, int count, int segmentWidth, int y, bool rtlMode);
  void drawSegmentSelection(
    QPainter* painter, const QString& line, int lineIndex, int segStart, int segEnd, int y);
  void renderFastSegment(
    QPainter* painter, const QString& segment, const QColor& textColor, int x, int y);
  void renderAnsiSegment(QPainter* painter,
                         const QString& segment,
                         int segStart,
                         const QList<CharColor>* colorLine,
                         const QColor& defaultFg,
                         int x,
                         int y);
  void paintSegment(QPainter* painter,
                    const QString& segment,
                    int segStart,
                    const QList<CharColor>* colorLine,
                    const QColor& defaultFg,
                    int x,
                    int y,
                    int ascent,
                    bool rtlMode);

  void paintSelectionHighlights(QPainter* painter, int firstLine, int lastVLine, int lineHeight);
  void paintSearchHighlights(QPainter* painter, int firstLine, int lastVLine, int lineHeight);
  void drawSegmentMatch(QPainter* painter,
                        const QFontMetrics& fm,
                        const QString& line,
                        const QPoint& match,
                        bool isCurrent,
                        int segStart,
                        int segEnd,
                        int y);
  void paintTextContent(QPainter* painter, int firstLine, int lastVLine, int lineHeight);
  void paintScrollbar(QPainter* painter);
  [[nodiscard]] QRect scrollbarTrackRect() const;
  [[nodiscard]] QRect scrollbarThumbRect() const;
  [[nodiscard]] int scrollOffsetForThumbY(const int thumbY) const;
  [[nodiscard]] bool isOverScrollbar(const QPoint& pos) const;
  [[nodiscard]] bool handleScrollbarPress(const QPoint& pos);
  void applyScrollbarOffset(const int offset);

  [[nodiscard]] QByteArray translateEnterKey() const;

protected:
  bool shouldEndSelection(const QChar& c);
  void keyPressEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseUngrabEvent() override;
  void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
  Console::Handler& m_consoleHandler;
  Misc::ThemeManager& m_themeManager;
  IO::ConnectionManager& m_connectionManager;
#ifdef BUILD_COMMERCIAL
  Licensing::LemonSqueezy& m_lemonSqueezy;
#endif
  Misc::Translator& m_translator;
  Misc::TimerEvents& m_timerEvents;

  QPalette m_palette;

  QFont m_font;
  int m_cWidth;
  int m_cHeight;

  int m_borderX;
  int m_borderY;
  int m_scrollOffsetY;
  int m_dragThumbGrabY;

  QTimer m_cursorTimer;

  QPoint m_selectionEnd;
  QPoint m_selectionStart;
  QPoint m_selectionStartCursor;

  bool m_paused;
  bool m_autoscroll;
  bool m_ansiColors;
  bool m_emulateVt100;
  bool m_cursorVisible;
  bool m_mouseTracking;
  bool m_draggingScrollbar;

  bool m_stateChanged;
  bool m_cursorHidden;

  AnsiPalette m_ansiPalette;
  TerminalBuffer m_buffer;
  AnsiStateMachine m_ansi;
  TerminalSearch m_search;

  QFont m_badgeFont;
  QFontMetrics m_badgeMetrics;
};
}  // namespace Widgets
