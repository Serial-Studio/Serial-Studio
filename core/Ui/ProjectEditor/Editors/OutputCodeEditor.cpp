/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include "ProjectEditor/Editors/OutputCodeEditor.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QInputMethod>
#include <QJSEngine>

#include "Core/Prompt/UserPrompt.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/Translator.h"
#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/TransmitScriptEnvironment.h"
#include "ProjectEditor/ProjectEditor.h"

#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Preview.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Idle gap before a keystroke is validated: an engine per character is far too expensive
static constexpr int kValidateDebounceMs = 300;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side output widget transmit-function editor. It lives in the Project
 *        Editor window, which stays instantiated once opened, hence the window-visibility gate.
 */
DataModel::OutputCodeEditor::OutputCodeEditor(QQuickItem* parent)
  : EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate::WindowVisible, parent)
  , m_readingCode(false)
  , m_validating(false)
  , m_translator(Core::services().translator)
  , m_projectEditor(DataModel::ProjectEditor::instance())
  , m_projectModel(DataModel::pipelineModules().projectModel)
  , m_templates(QStringLiteral(":/scripts/output/templates.json"),
                QStringLiteral(":/scripts/output"),
                QStringLiteral(".js"),
                "DataModel::OutputCodeEditor")
{
  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::OutputCodeEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    scheduleValidation();
  });

  m_validateTimer.setSingleShot(true);
  m_validateTimer.setInterval(kValidateDebounceMs);
  connect(&m_validateTimer, &QTimer::timeout, this, &DataModel::OutputCodeEditor::validateNow);

  connect(&m_projectEditor,
          &DataModel::ProjectEditor::outputWidgetModelChanged,
          this,
          &DataModel::OutputCodeEditor::readCode);

  connect(&m_projectModel, &DataModel::ProjectModel::groupDataChanged, this, [this] {
    if (m_readingCode)
      return;

    const auto& sel = m_projectEditor.selectedOutputWidget();
    if (sel.groupId < 0 || sel.widgetId < 0)
      return;

    const auto& groups = m_projectModel.groups();
    if (sel.groupId < 0 || static_cast<size_t>(sel.groupId) >= groups.size())
      return;

    for (const auto& w : groups[sel.groupId].outputWidgets) {
      if (w.widgetId == sel.widgetId) {
        if (w.transmitFunction != text())
          readCode();

        return;
      }
    }
  });

  connect(&m_translator,
          &Misc::Translator::languageChanged,
          this,
          &DataModel::OutputCodeEditor::loadTemplates);

  loadTemplates();

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::OutputCodeEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::OutputCodeEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::OutputCodeEditor::undoAvailable() const noexcept
{
  return m_editor.undoAvailable();
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::OutputCodeEditor::redoAvailable() const noexcept
{
  return m_editor.redoAvailable();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::OutputCodeEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::OutputCodeEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::OutputCodeEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::OutputCodeEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::OutputCodeEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::OutputCodeEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Reformats the entire transmit-function source.
 */
void DataModel::OutputCodeEditor::formatDocument()
{
  EditorFormatting::formatDocument(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::OutputCodeEditor::formatSelection()
{
  EditorFormatting::formatSelection(m_editor.widget(), CodeFormatter::Language::JavaScript);
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::OutputCodeEditor::importFile()
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
 * @brief Loads the transmit function from the currently selected output widget.
 */
void DataModel::OutputCodeEditor::readCode()
{
  if (m_readingCode)
    return;

  m_readingCode = true;

  const auto& sel = m_projectEditor.selectedOutputWidget();

  QString code = sel.transmitFunction;
  if (code.isEmpty())
    code = defaultTemplate();

  m_editor.setSourceText(code);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
  refreshVerdict(false);
}

/**
 * @brief Resets the editor to the default transmit function template.
 */
void DataModel::OutputCodeEditor::reload(bool guiTrigger)
{
  Q_UNUSED(guiTrigger)
  m_editor.setSourceText(defaultTemplate());
  Q_EMIT modifiedChanged();
}

//--------------------------------------------------------------------------------------------------
// Templates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the default transmit function template from resources.
 */
QString DataModel::OutputCodeEditor::defaultTemplate()
{
  return defaultOutputWidgetTemplate();
}

/**
 * @brief Rebuilds the cached list of output-widget templates from resources.
 */
void DataModel::OutputCodeEditor::loadTemplates()
{
  m_templates.reload();
  Q_EMIT templatesChanged();
}

/**
 * @brief Restarts the validation debounce. Every keystroke would otherwise build a fresh engine and
 *        install the whole host surface into it, which is far too expensive to do per character.
 */
void DataModel::OutputCodeEditor::scheduleValidation()
{
  m_validateTimer.start();
}

/**
 * @brief Recomputes the verdict and persists the script only when it is valid, so an invalid edit
 *        leaves the project holding the last version that compiled. The write guard stays as it
 *        was: the editor is modal over the output-widget view, which is what makes the view and
 *        selection checks sufficient.
 */
void DataModel::OutputCodeEditor::validateNow()
{
  refreshVerdict(false);
}

/**
 * @brief Recomputes the verdict, persisting only when @p persist and the script may be stored.
 *        Reading a widget passes false: selecting one must not write the default template into a
 *        project the user only looked at. The latch guards re-entry, since compiling user code
 *        can spin a nested event loop, and the notify is gated on a changed verdict.
 */
void DataModel::OutputCodeEditor::refreshVerdict(const bool persist)
{
  if (m_validating)
    return;

  m_validating = true;
  m_validateTimer.stop();

  const auto previous = m_verdict;
  const auto code     = text();
  m_verdict           = checkTransmitScript(code, [](QJSEngine& engine) {
    return prepareTransmitScriptEngine(engine, 0, TransmitScriptSurface::Judging);
  });

  m_validating = false;
  if (m_verdict.status != previous.status || m_verdict.line != previous.line
      || m_verdict.message != previous.message)
    Q_EMIT validityChanged();

  if (m_verdict.persistable())
    Q_EMIT scriptAccepted(code);

  if (!persist || !m_verdict.persistable())
    return;

  if (m_projectEditor.currentView() != DataModel::ProjectEditor::OutputWidgetView)
    return;

  const auto& sel = m_projectEditor.selectedOutputWidget();
  if (sel.groupId < 0 || sel.widgetId < 0)
    return;

  m_projectEditor.setSelectedOutputWidgetTransmitFunction(code);
}

/**
 * @brief Flushes a pending input-method composition and validates immediately, so the last thing
 *        typed before the window closes is judged and stored rather than left in the debounce.
 */
void DataModel::OutputCodeEditor::commit()
{
  if (m_readingCode)
    return;

  flushInputMethod();
  refreshVerdict(false);
}

/**
 * @brief Lands a pending input-method composition in the buffer, so the last thing typed is part
 *        of what gets judged and stored.
 */
void DataModel::OutputCodeEditor::flushInputMethod()
{
  if (auto* ime = QGuiApplication::inputMethod()) {
    if (ime->isVisible() || !ime->inputItemRectangle().isEmpty())
      ime->commit();
  }
}

/**
 * @brief Hands the preview the widget this editor is bound to. The editor already owns the
 *        selection, so routing the configuration through it keeps the preview free of any
 *        project dependency of its own. Output controls are a Pro feature and the preview is
 *        built only there, so a GPL build has nothing to bind.
 */
void DataModel::OutputCodeEditor::bindPreview(QObject* preview)
{
#ifdef BUILD_COMMERCIAL
  auto* target = qobject_cast<Widgets::Output::Preview*>(preview);
  SS_ASSERT(target != nullptr, return);

  target->setConfig(m_projectEditor.selectedOutputWidget());
#else
  Q_UNUSED(preview);
#endif
}

/**
 * @brief Validates and stores the script, reporting why it was refused. The one path that writes
 *        the project, so the dialog's Close discards by construction, matching the value-transform
 *        editor. Returns whether the caller may close.
 */
bool DataModel::OutputCodeEditor::save()
{
  if (m_readingCode)
    return false;

  flushInputMethod();
  refreshVerdict(true);
  if (m_verdict.persistable())
    return true;

  reportVerdict();
  return false;
}

/**
 * @brief Says plainly what the verdict is, which is what the Validate action is for now that the
 *        editor carries no status strip.
 */
void DataModel::OutputCodeEditor::reportVerdict()
{
  if (m_verdict.status == TransmitScriptStatus::Empty) {
    Core::Prompt::showMessageBox(tr("No transmit function is defined."),
                                 tr("The control will send nothing until one is written."),
                                 Core::Prompt::Information,
                                 tr("Transmit Function Editor"));
    return;
  }

  if (m_verdict.ok()) {
    Core::Prompt::showMessageBox(tr("transmit(value) is defined and compiles."),
                                 QString(),
                                 Core::Prompt::Information,
                                 tr("Transmit Function Editor"));
    return;
  }

  Core::Prompt::showMessageBox(tr("The transmit function was not applied."),
                               verdictDetail(),
                               Core::Prompt::Warning,
                               tr("Transmit Function Editor"));
}

/**
 * @brief Human wording for the current verdict; the editor owns it because the assistant needs a
 *        different, longer correction for the same status.
 */
QString DataModel::OutputCodeEditor::verdictDetail() const
{
  switch (m_verdict.status) {
    case TransmitScriptStatus::Timeout:
      return tr("The script did not finish compiling. Is there an endless loop at the top level?");
    case TransmitScriptStatus::CompileError:
      return m_verdict.line > 0
             ? tr("Line %1: %2").arg(QString::number(m_verdict.line), m_verdict.message)
             : m_verdict.message;
    case TransmitScriptStatus::HostUnavailable:
      return tr("The script host is unavailable, so nothing can be validated right now.");
    default:
      break;
  }

  return tr("Define a function transmit(value) that returns the bytes to send.");
}

/**
 * @brief Whether the script currently in the editor would be accepted.
 */
bool DataModel::OutputCodeEditor::scriptValid() const noexcept
{
  return m_verdict.persistable();
}

/**
 * @brief The verdict as an int, so QML can distinguish a compile error from a missing entry point.
 */
int DataModel::OutputCodeEditor::scriptStatus() const noexcept
{
  return static_cast<int>(m_verdict.status);
}

/**
 * @brief Line the compile error happened on, or zero when there is none.
 */
int DataModel::OutputCodeEditor::scriptErrorLine() const noexcept
{
  return m_verdict.line;
}

/**
 * @brief The engine's own message for a compile error, empty for every other verdict.
 */
QString DataModel::OutputCodeEditor::scriptError() const
{
  return m_verdict.message;
}

/**
 * @brief Display names of the built-in transmit templates, in catalog order.
 */
QStringList DataModel::OutputCodeEditor::templateNames() const
{
  return m_templates.names();
}

/**
 * @brief Loads the template at @p index, replacing the script under edit.
 */
void DataModel::OutputCodeEditor::applyTemplate(const int index)
{
  if (index < 0 || index >= m_templates.files().size())
    return;

  QFile file(m_templates.files().at(index));
  if (!file.open(QFile::ReadOnly)) {
    qWarning() << "[Output] cannot read transmit template" << file.fileName();
    return;
  }

  m_editor.setSourceText(QString::fromUtf8(file.readAll()));
  file.close();

  Q_EMIT modifiedChanged();
  refreshVerdict(true);
}
