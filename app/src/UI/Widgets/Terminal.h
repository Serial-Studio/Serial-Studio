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

#pragma once

#include <QTimer>
#include <QPalette>
#include <QQuickPaintedItem>

namespace Widgets
{
/**
 * @class Terminal
 * @brief A QML terminal widget with optional VT-100 emulation.
 *
 * The Terminal class is a QQuickPaintedItem that implements a fully interactive
 * terminal emulator. It supports VT-100 emulation and provides features like
 * text selection, autoscroll, and customizable fonts and color palettes. The
 * terminal can process escape sequences, manage cursor positions, and handle
 * mouse and keyboard events effectively.
 *
 * This class is suitable for embedding a terminal interface in QML-based GUI
 * applications, with multiple customizable features exposed as properties.
 */
class Terminal : public QQuickPaintedItem
{
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
  Q_PROPERTY(QPalette palette
             READ palette
             WRITE setPalette
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

public:
  Terminal(QQuickItem *parent = 0);
  void paint(QPainter *painter) override;

  enum Direction
  {
    LeftDirection,
    RightDirection
  };
  Q_ENUM(Direction);

  enum State
  {
    Text,
    Escape,
    Format,
    ResetFont
  };
  Q_ENUM(State);

  [[nodiscard]] int charWidth() const;
  [[nodiscard]] int charHeight() const;

  [[nodiscard]] QFont font() const;
  [[nodiscard]] QPalette palette() const;

  [[nodiscard]] bool autoscroll() const;
  [[nodiscard]] bool copyAvailable() const;
  [[nodiscard]] bool vt100emulation() const;

  [[nodiscard]] int lineCount() const;
  [[nodiscard]] int linesPerPage() const;
  [[nodiscard]] int scrollOffsetY() const;
  [[nodiscard]] int maxCharsPerLine() const;

  [[nodiscard]] QPoint cursorPosition() const;
  [[nodiscard]] QPoint positionToCursor(const QPoint &pos) const;

public slots:
  void copy();
  void clear();
  void selectAll();
  void setFont(const QFont &font);
  void setAutoscroll(const bool enabled);
  void setScrollOffsetY(const int offset);
  void setPalette(const QPalette &palette);
  void setVt100Emulation(const bool enabled);

private slots:
  void toggleCursor();
  void onThemeChanged();
  void append(const QString &data);
  void appendString(const QString &string);
  void removeStringFromCursor(const Direction direction = RightDirection,
                              int len = INT_MAX);

private:
  void initBuffer();
  void processText(const QChar &byte, QString &text);
  void processEscape(const QChar &byte, QString &text);
  void processFormat(const QChar &byte, QString &text);
  void processResetFont(const QChar &byte, QString &text);

  void setCursorPosition(const QPoint position);
  void setCursorPosition(const int x, const int y);
  void replaceData(qsizetype x, qsizetype y, QChar byte);

protected:
  bool shouldEndSelection(const QChar c);
  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
  QPalette m_palette;
  QStringList m_data;

  QFont m_font;
  int m_cWidth;
  int m_cHeight;

  int m_borderX;
  int m_borderY;
  int m_scrollOffsetY;

  QTimer m_cursorTimer;
  QPoint m_cursorPosition;

  QPoint m_selectionEnd;
  QPoint m_selectionStart;
  QPoint m_selectionStartCursor;

  State m_state;
  bool m_autoscroll;
  bool m_emulateVt100;
  bool m_cursorVisible;
  bool m_mouseTracking;

  int m_formatValue;
  int m_formatValueY;
  bool m_useFormatValueY;

  bool m_stateChanged;
};
} // namespace Widgets
