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

#include "ProjectEditor/Editors/TransformLibraryEditor.h"

#include <lua.h>

#include <QDir>
#include <QFileDialog>

#include "Core/Prompt/UserPrompt.h"
#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/ScriptDryRun.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side library editor. The view's Loader creates it only while the
 *        Lua Library node is selected, and readCode() holds the re-entrancy guard, so a project
 *        load that repopulates the editor never writes back.
 */
DataModel::TransformLibraryEditor::TransformLibraryEditor(QQuickItem* parent)
  : EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate::WindowVisible, parent)
  , m_lua(true)
  , m_readingCode(false)
  , m_initialLoad(true)
  , m_projectModel(DataModel::pipelineModules().projectModel)
{
  m_editor.setScriptLanguage(true);

  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(
    &widget, &QCodeEditor::textChanged, this, &DataModel::TransformLibraryEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    m_readingCode        = true;
    const QString code   = text();
    const QString stored = code.trimmed() == starterLibrary(m_lua).trimmed() ? QString() : code;
    if (m_lua)
      m_projectModel.setTransformLibraryCode(stored);
    else
      m_projectModel.setTransformLibraryJsCode(stored);

    m_readingCode = false;
  });

  connect(&m_projectModel, &DataModel::ProjectModel::jsonFileChanged, this, [this] {
    m_initialLoad = true;
  });

  connect(&m_projectModel,
          &DataModel::ProjectModel::transformLibraryChanged,
          this,
          &DataModel::TransformLibraryEditor::readCode);
  connect(&m_projectModel,
          &DataModel::ProjectModel::transformLibraryJsChanged,
          this,
          &DataModel::TransformLibraryEditor::readCode);

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::TransformLibraryEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns true while the editor edits the Lua library, false for the JavaScript one.
 */
bool DataModel::TransformLibraryEditor::lua() const noexcept
{
  return m_lua;
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::TransformLibraryEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::TransformLibraryEditor::undoAvailable() const noexcept
{
  return m_editor.undoAvailable();
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::TransformLibraryEditor::redoAvailable() const noexcept
{
  return m_editor.redoAvailable();
}

/**
 * @brief Returns the commented starter shown for an empty library; it is never stored.
 */
QString DataModel::TransformLibraryEditor::starterLibrary(bool lua)
{
  if (!lua)
    return tr("//\n"
              "// JavaScript Library\n"
              "//\n"
              "// Functions defined here are available to every JavaScript value\n"
              "// transform in this project, so a formula lives in one place. Each\n"
              "// dataset can pass its own constants through its Parameters table,\n"
              "// which the transform reads as params.<name>.\n"
              "//\n"
              "// Example:\n"
              "//    function rtd(raw, p) {\n"
              "//      return (raw * p.scale + p.offset) / p.r0;\n"
              "//    }\n"
              "//\n"
              "// Dataset transform:\n"
              "//    function transform(value) {\n"
              "//      return rtd(value, params);\n"
              "//    }\n"
              "//\n");

  return tr("--\n"
            "-- Lua Library\n"
            "--\n"
            "-- Functions defined here are available to every Lua value transform\n"
            "-- in this project, so a formula lives in one place. Each dataset can\n"
            "-- pass its own constants through its Parameters table, which the\n"
            "-- transform reads as params.<name>.\n"
            "--\n"
            "-- Example:\n"
            "--    function rtd(raw, p)\n"
            "--      return (raw * p.scale + p.offset) / p.r0\n"
            "--    end\n"
            "--\n"
            "-- Dataset transform:\n"
            "--    function transform(value)\n"
            "--      return rtd(value, params)\n"
            "--    end\n"
            "--\n");
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::TransformLibraryEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::TransformLibraryEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::TransformLibraryEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::TransformLibraryEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::TransformLibraryEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::TransformLibraryEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Reformats the entire library source.
 */
void DataModel::TransformLibraryEditor::formatDocument()
{
  EditorFormatting::formatDocument(
    m_editor.widget(), m_lua ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::TransformLibraryEditor::formatSelection()
{
  EditorFormatting::formatSelection(
    m_editor.widget(), m_lua ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript);
}

/**
 * @brief Opens a file dialog to import an external Lua file.
 */
void DataModel::TransformLibraryEditor::importFile()
{
  auto* dialog = new QFileDialog(nullptr,
                                 m_lua ? tr("Select Lua file to import")
                                       : tr("Select JavaScript file to import"),
                                 QDir::homePath(),
                                 m_lua ? QStringLiteral("*.lua") : QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this, [this, path]() { (void)m_editor.importFromFile(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Loads the library from the project model into the editor; a stored copy of the other
 *        language's starter (an artefact of the pre-fix switch) reads as empty.
 */
void DataModel::TransformLibraryEditor::readCode()
{
  if (m_readingCode)
    return;

  m_readingCode = true;

  QString code =
    m_lua ? m_projectModel.transformLibraryCode() : m_projectModel.transformLibraryJsCode();
  if (code.trimmed() == starterLibrary(!m_lua).trimmed())
    code.clear();

  if (code.isEmpty() && m_initialLoad)
    code = starterLibrary(m_lua);

  m_initialLoad = false;
  m_editor.syncSourceText(code);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the editor to the commented starter.
 */
void DataModel::TransformLibraryEditor::reload()
{
  m_editor.setSourceText(starterLibrary(m_lua));
  Q_EMIT modifiedChanged();
}

/**
 * @brief Switches the editor between the Lua and the JavaScript library: highlighter, starter and
 *        the model field it mirrors. The highlighter swap re-marks the document dirty, so the
 *        write-back is held off until the re-read has replaced the other language's text.
 */
void DataModel::TransformLibraryEditor::setLua(bool lua)
{
  if (m_lua == lua)
    return;

  m_lua         = lua;
  m_readingCode = true;
  m_editor.setScriptLanguage(lua);
  m_readingCode = false;
  m_initialLoad = true;
  readCode();
  Q_EMIT luaChanged();
}

/**
 * @brief Loads and runs the library in a throwaway sandboxed engine (interpreter, deadline hook)
 *        and reports the first error; nothing is installed.
 */
void DataModel::TransformLibraryEditor::evaluate()
{
  if (!m_lua) {
    evaluateJs();
    return;
  }

  ScriptDryRun session(ScriptDryRun::Language::Lua, kScriptDryRunBudgetMs, "library.validate");
  if (!session.valid()) {
    Core::Prompt::showMessageBox(tr("Code Validation Failed"),
                                 tr("Failed to create the validation engine."),
                                 Core::Prompt::Warning);
    return;
  }

  lua_State* L = session.luaState();
  DataModel::installLuaCompat(L);
  DataModel::NotificationCenter::installScriptApi(L);
  if (session.runLuaChunk(text(), "library") != LUA_OK) {
    const QString error =
      session.timedOut()
        ? tr("The library did not finish running within %1 ms. Check for an infinite loop at "
             "the top level.")
            .arg(session.budgetMs())
        : session.luaError();
    Core::Prompt::showMessageBox(tr("Code Validation Failed"), error, Core::Prompt::Warning);
    return;
  }

  Core::Prompt::showMessageBox(tr("Code Validation Successful"),
                               tr("No errors detected in the Lua library."),
                               Core::Prompt::Information);
}

/**
 * @brief JavaScript half of evaluate(): a throwaway QJSEngine under the dry-run deadline.
 */
void DataModel::TransformLibraryEditor::evaluateJs()
{
  ScriptDryRun session(
    ScriptDryRun::Language::JavaScript, kScriptDryRunBudgetMs, "library.validate");
  if (!session.valid()) {
    Core::Prompt::showMessageBox(tr("Code Validation Failed"),
                                 tr("Failed to create the validation engine."),
                                 Core::Prompt::Warning);
    return;
  }

  const auto result = session.evaluate(text(), QStringLiteral("library.js"));
  if (session.timedOut()) {
    Core::Prompt::showMessageBox(
      tr("Code Validation Failed"),
      tr("The library did not finish running within %1 ms. Check for an infinite loop at the "
         "top level.")
        .arg(session.budgetMs()),
      Core::Prompt::Warning);
    return;
  }

  if (result.isError()) {
    Core::Prompt::showMessageBox(tr("Code Validation Failed"),
                                 tr("Line %1: %2")
                                   .arg(result.property(QStringLiteral("lineNumber")).toInt())
                                   .arg(result.toString()),
                                 Core::Prompt::Warning);
    return;
  }

  Core::Prompt::showMessageBox(tr("Code Validation Successful"),
                               tr("No errors detected in the JavaScript library."),
                               Core::Prompt::Information);
}
