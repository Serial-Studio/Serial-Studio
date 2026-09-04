/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "DataModel/Editors/EditorFormatting.h"

#include <QCodeEditor>
#include <QTextCursor>
#include <QTextDocument>

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces the whole document with formatted in a single undo step, clamping the caret to
 *        the new length. A no-op when the formatter changed nothing.
 */
static void replaceDocument(QCodeEditor& editor, const QString& original, const QString& formatted)
{
  if (formatted == original)
    return;

  QTextCursor cursor = editor.textCursor();
  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  editor.setTextCursor(cursor);
}

namespace DataModel::EditorFormatting {

//--------------------------------------------------------------------------------------------------
// Formatting entry points
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reformats the entire editor contents.
 */
void formatDocument(QCodeEditor& editor, CodeFormatter::Language language)
{
  const QString original  = editor.toPlainText();
  const QString formatted = CodeFormatter::formatDocument(original, language);
  replaceDocument(editor, original, formatted);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void formatSelection(QCodeEditor& editor, CodeFormatter::Language language)
{
  const QString original = editor.toPlainText();

  QTextCursor cursor = editor.textCursor();
  QTextCursor first(editor.document());
  first.setPosition(qMin(cursor.selectionStart(), cursor.selectionEnd()));
  QTextCursor last(editor.document());
  last.setPosition(qMax(cursor.selectionStart(), cursor.selectionEnd()));

  const int firstLine     = first.blockNumber();
  const int lastLine      = last.blockNumber();
  const QString formatted = CodeFormatter::formatLineRange(original, language, firstLine, lastLine);
  replaceDocument(editor, original, formatted);
}

}  // namespace DataModel::EditorFormatting
