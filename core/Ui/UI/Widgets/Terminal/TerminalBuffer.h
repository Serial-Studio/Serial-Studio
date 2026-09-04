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

#pragma once

#include <QColor>
#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QStringView>

#include "UI/Widgets/Terminal/AnsiPalette.h"
#include "UI/Widgets/Terminal/AnsiStateMachine.h"

namespace Widgets {
/**
 * @brief Stores foreground and background colors for a character.
 */
struct CharColor {
  QColor foreground;
  QColor background;

  CharColor() : foreground(), background() {}

  CharColor(const QColor& fg, const QColor& bg = QColor()) : foreground(fg), background(bg) {}
};

/**
 * @brief The terminal's line store: text rows, their per-character color rows, the duplicate
 *        repeat counts and the cursor that writes into them, plus the scrollback cap that trims
 *        the front. GUI-thread value class, no QObject: mutations the widget has to publish are
 *        recorded and drained by the facade, keeping the per-character path free of cross-TU calls.
 */
class TerminalBuffer {
public:
  /**
   * @brief Reports whether a control character opened an escape sequence. The buffer does not own
   *        the parser, so ESC is handed back to the facade instead of being consumed here.
   */
  enum FeedResult {
    Handled,
    BeginEscape
  };

  explicit TerminalBuffer(const AnsiPalette& palette);

  [[nodiscard]] int columns() const { return m_columns; }

  [[nodiscard]] int maxLines() const { return m_maxLines; }

  [[nodiscard]] const QPoint& cursor() const { return m_cursor; }

  [[nodiscard]] const QStringList& lines() const { return m_data; }

  [[nodiscard]] const QList<int>& repeatCounts() const { return m_repeatCounts; }

  [[nodiscard]] int lineCount() const { return static_cast<int>(m_data.size()); }

  [[nodiscard]] const QList<QList<CharColor>>& colorRows() const { return m_colorData; }

  [[nodiscard]] int visualBottomRow() const;

  void reset();
  void squeeze();
  void reserveColorRows();
  void setColumns(int columns);
  void setMaxLines(int maxLines);
  void setAnsiColors(bool enabled);
  void setShowTimestamps(bool enabled);
  void setCollapseDuplicates(bool enabled);

  [[nodiscard]] int takeDroppedLines();
  [[nodiscard]] bool takeCursorMoved();

  [[nodiscard]] int trimToMaxLines();
  [[nodiscard]] bool collapseCompletedLine();
  [[nodiscard]] FeedResult processText(const QChar& byte, QString& text, bool vt100);

  void setCursor(int x, int y);
  void setCursor(const QPoint& position);
  void appendText(QStringView string);
  void eraseRowsAfter(int row);
  void eraseRowsBefore(int row);
  void removeFromCursor(AnsiEraseDirection direction, int length);

  [[nodiscard]] static bool hasTimestampPrefix(QStringView line);
  [[nodiscard]] static QPoint clampPoint(const QPoint& point, const QStringList& lines);

  /**
   * @brief Returns true when @p line contains any RTL-direction character.
   */
  [[nodiscard]] static bool lineHasRtlChar(QStringView line)
  {
    for (const QChar c : line) {
      const auto dir = c.direction();
      if (dir == QChar::DirR || dir == QChar::DirAL)
        return true;
    }

    return false;
  }

  /**
   * @brief Scans a printable-character run starting at @p pos in @p data; returns end offset.
   */
  [[nodiscard]] static int scanPrintableRun(const QString& data, int pos)
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

private:
  void trimFront(int linesToDrop);
  void replaceData(int x, int y, const QChar& byte);
  [[nodiscard]] QStringView lineContentView(QStringView line) const;

private:
  const AnsiPalette& m_palette;

  QStringList m_data;
  QList<QList<CharColor>> m_colorData;
  QList<int> m_repeatCounts;

  QPoint m_cursor;

  int m_columns;
  int m_maxLines;
  int m_droppedLines;

  bool m_cursorMoved;
  bool m_ansiColors;
  bool m_showTimestamps;
  bool m_collapseDuplicates;
};
}  // namespace Widgets
