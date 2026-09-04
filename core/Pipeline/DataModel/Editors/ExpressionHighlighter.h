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

#include <QRegularExpression>
#include <QStyleSyntaxHighlighter>

namespace DataModel {

/**
 * @brief Highlighter for the arithmetic transform language (spec 0060). The JavaScript
 *        highlighter used to stand in for it, which coloured keywords the language does not have
 *        and left its `#` comments looking like errors.
 */
class ExpressionHighlighter : public QStyleSyntaxHighlighter {
  Q_OBJECT

public:
  explicit ExpressionHighlighter(QTextDocument* document = nullptr);

protected:
  void highlightBlock(const QString& text) override;

private:
  QRegularExpression m_number;
  QRegularExpression m_braced;
  QRegularExpression m_comment;
  QRegularExpression m_builtin;
  QRegularExpression m_function;
};

}  // namespace DataModel
