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

#include "DataModel/Editors/ControlScriptEditor.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJSEngine>

#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ScriptDryRun.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Default template
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the bundled setup()/loop() starter template.
 */
static QString defaultControlScript()
{
  QFile file(QStringLiteral(":/scripts/control/default_template.js"));
  if (file.open(QFile::ReadOnly))
    return QString::fromUtf8(file.readAll());

  return QStringLiteral("function setup() {\n}\n\nfunction loop() {\n}\n");
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side control-script editor. The Project Editor is a separate top-level
 *        window that stays instantiated once opened, hence the window-visibility render gate.
 */
DataModel::ControlScriptEditor::ControlScriptEditor(QQuickItem* parent)
  : EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate::WindowVisible, parent)
  , m_readingCode(false)
  , m_initialLoad(true)
  , m_projectEditor(DataModel::ProjectEditor::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
{
  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::ControlScriptEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    if (m_projectEditor.currentView() != DataModel::ProjectEditor::ControlScriptView)
      return;

    m_readingCode = true;
    m_projectModel.setControlScriptCode(text());
    m_readingCode = false;
  });

  connect(&m_projectModel, &DataModel::ProjectModel::jsonFileChanged, this, [this] {
    m_initialLoad = true;
  });

  connect(&m_projectModel,
          &DataModel::ProjectModel::controlScriptChanged,
          this,
          &DataModel::ControlScriptEditor::readCode);

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::ControlScriptEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::ControlScriptEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::ControlScriptEditor::undoAvailable() const noexcept
{
  return m_editor.undoAvailable();
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::ControlScriptEditor::redoAvailable() const noexcept
{
  return m_editor.redoAvailable();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::ControlScriptEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::ControlScriptEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::ControlScriptEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::ControlScriptEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::ControlScriptEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::ControlScriptEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Reformats the entire control-script source.
 */
void DataModel::ControlScriptEditor::formatDocument()
{
  EditorFormatting::formatDocument(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::ControlScriptEditor::formatSelection()
{
  EditorFormatting::formatSelection(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::ControlScriptEditor::importFile()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
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
 * @brief Loads the control script from the project model into the editor.
 */
void DataModel::ControlScriptEditor::readCode()
{
  if (m_readingCode)
    return;

  m_readingCode = true;

  QString code = m_projectModel.controlScriptCode();
  if (code.isEmpty() && m_initialLoad)
    code = defaultControlScript();

  m_initialLoad = false;
  m_editor.syncSourceText(code);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the editor to the default setup()/loop() template.
 */
void DataModel::ControlScriptEditor::reload()
{
  m_editor.setSourceText(defaultControlScript());
  Q_EMIT modifiedChanged();
}

/**
 * @brief Compiles the script (without running it) and reports syntax errors. Top-level code does
 *        run during a compile, so the evaluation is deadline-guarded: a runaway loop reports a
 *        timeout instead of freezing the editor.
 */
void DataModel::ControlScriptEditor::evaluate()
{
  ScriptDryRun session(
    ScriptDryRun::Language::JavaScript, kScriptDryRunBudgetMs, "controlScript.validate");
  if (!session.valid()) {
    Misc::Utilities::showMessageBox(tr("Code Validation Failed"),
                                    tr("Failed to create the validation engine."),
                                    QMessageBox::Warning);
    return;
  }

  const auto result = session.evaluate(text(), QStringLiteral("control-script.js"));
  if (session.timedOut()) {
    Misc::Utilities::showMessageBox(
      tr("Code Validation Failed"),
      tr("The script did not finish evaluating within %1 ms. Check for an infinite loop at the "
         "top level.")
        .arg(session.budgetMs()),
      QMessageBox::Warning);
    return;
  }

  if (result.isError()) {
    Misc::Utilities::showMessageBox(tr("Code Validation Failed"),
                                    tr("Line %1: %2")
                                      .arg(result.property(QStringLiteral("lineNumber")).toInt())
                                      .arg(result.toString()),
                                    QMessageBox::Warning);
    return;
  }

  const auto globals  = session.jsEngine()->globalObject();
  const bool hasSetup = globals.property(QStringLiteral("setup")).isCallable();
  const bool hasLoop  = globals.property(QStringLiteral("loop")).isCallable();
  if (!hasSetup && !hasLoop) {
    Misc::Utilities::showMessageBox(tr("Code Validation Failed"),
                                    tr("The script must define a setup() and/or loop() function."),
                                    QMessageBox::Warning);
    return;
  }

  Misc::Utilities::showMessageBox(tr("Code Validation Successful"),
                                  tr("No syntax errors detected in the control loop."),
                                  QMessageBox::Information);
}
