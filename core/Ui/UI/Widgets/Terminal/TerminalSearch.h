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

#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>

namespace Widgets {
/**
 * @brief In-buffer search state for the terminal: the query, its case mode, the match list, and
 *        which match navigation is sitting on. Never reaches for the terminal's buffer; refresh()
 *        is handed the lines to scan, and mutating calls report whether anything changed so the
 *        facade knows when to emit searchResultsChanged() and repaint.
 */
class TerminalSearch {
public:
  TerminalSearch();

  [[nodiscard]] bool dirty() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] int currentRow() const;
  [[nodiscard]] int matchCount() const;
  [[nodiscard]] int currentIndex() const;
  [[nodiscard]] int currentMatchNumber() const;
  [[nodiscard]] const QString& query() const;
  [[nodiscard]] const QList<QPoint>& matches() const;

  void markDirty();
  void refresh(const QStringList& lines);

  [[nodiscard]] bool next();
  [[nodiscard]] bool clear();
  [[nodiscard]] bool previous();
  [[nodiscard]] bool setQuery(const QString& query, bool caseSensitive);

private:
  int m_current;
  bool m_dirty;
  bool m_caseSensitive;
  QString m_query;
  QList<QPoint> m_matches;
};
}  // namespace Widgets
