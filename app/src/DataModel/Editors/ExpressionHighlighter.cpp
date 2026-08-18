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

#include "DataModel/Editors/ExpressionHighlighter.h"

#include <QSyntaxStyle>

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compiles the patterns once; the language is small enough that five regexes cover it.
 */
DataModel::ExpressionHighlighter::ExpressionHighlighter(QTextDocument* document)
  : QStyleSyntaxHighlighter(document)
  , m_number(QStringLiteral("\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b"))
  , m_braced(QStringLiteral("\\{[^}]*\\}"))
  , m_comment(QStringLiteral("#[^\n]*"))
  , m_builtin(QStringLiteral("\\b(v|t|n|dt|pi|e|nan|inf)\\b"))
  , m_function(QStringLiteral("\\b([A-Za-z_][A-Za-z0-9_]*)\\s*\\("))
{}

//--------------------------------------------------------------------------------------------------
// Highlighting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Colours one line: sibling names, numbers, built-ins and calls first, comments last so a
 *        commented-out expression is greyed out whole.
 */
void DataModel::ExpressionHighlighter::highlightBlock(const QString& text)
{
  auto* style = syntaxStyle();
  if (!style)
    return;

  auto braced = m_braced.globalMatch(text);
  while (braced.hasNext()) {
    const auto match = braced.next();
    setFormat(match.capturedStart(), match.capturedLength(), style->getFormat("String"));
  }

  auto numbers = m_number.globalMatch(text);
  while (numbers.hasNext()) {
    const auto match = numbers.next();
    setFormat(match.capturedStart(), match.capturedLength(), style->getFormat("Number"));
  }

  auto builtins = m_builtin.globalMatch(text);
  while (builtins.hasNext()) {
    const auto match = builtins.next();
    setFormat(match.capturedStart(), match.capturedLength(), style->getFormat("Keyword"));
  }

  auto functions = m_function.globalMatch(text);
  while (functions.hasNext()) {
    const auto match = functions.next();
    setFormat(match.capturedStart(1), match.capturedLength(1), style->getFormat("Function"));
  }

  auto comments = m_comment.globalMatch(text);
  while (comments.hasNext()) {
    const auto match = comments.next();
    setFormat(match.capturedStart(), match.capturedLength(), style->getFormat("Comment"));
  }
}
