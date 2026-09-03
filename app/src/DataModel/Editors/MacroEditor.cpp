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

#include "DataModel/Editors/MacroEditor.h"

#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side macro editor. The macros dialog and the annotations decoder tab
 *        live inside the main window, so the item-visibility gate is enough here.
 */
DataModel::MacroEditor::MacroEditor(QQuickItem* parent)
  : EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate::ItemVisible, parent), m_language(0)
{
  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::MacroEditor::textChanged);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::MacroEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns the macro language (0 = JS, 1 = Lua).
 */
int DataModel::MacroEditor::language() const noexcept
{
  return m_language;
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::MacroEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::MacroEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::MacroEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::MacroEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::MacroEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Clears the editor content and its undo history.
 */
void DataModel::MacroEditor::clear()
{
  m_editor.setSourceText(QString());
  Q_EMIT modifiedChanged();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::MacroEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::MacroEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Switches the highlighter/completer pair to the given language
 *        (SerialStudio::ScriptLanguage encoding).
 */
void DataModel::MacroEditor::setLanguage(int language)
{
  if (m_language == language)
    return;

  m_language = language;
  m_editor.setScriptLanguage(language == SerialStudio::Lua);

  Q_EMIT languageChanged();
  scheduleRender();
}

/**
 * @brief Replaces the editor content (macro load): the undo-history and modified-state reset
 *        always runs, even when the incoming text matches the current content.
 */
void DataModel::MacroEditor::setText(const QString& text)
{
  m_editor.syncSourceText(text);
  Q_EMIT modifiedChanged();
  scheduleRender();
}

/**
 * @brief Marks the current content as saved (macro save), clearing the modified flag.
 */
void DataModel::MacroEditor::markSaved()
{
  m_editor.markSaved();
  Q_EMIT modifiedChanged();
}
