/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "DataModel/Editors/JsCodeEditor.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonObject>
#include <QMessageBox>
#include <QUrl>

#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side frame parser code editor. It lives in the Project Editor window,
 *        which stays instantiated once opened, hence the window-visibility render gate.
 */
DataModel::JsCodeEditor::JsCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_sourceId(0)
  , m_language(0)
  , m_readingCode(false)
  , m_themeManager(Misc::ThemeManager::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_projectEditor(DataModel::ProjectEditor::instance())
  , m_frameParser(DataModel::FrameParser::instance())
  , m_editor(*this,
             m_themeManager,
             Misc::CommonFonts::instance(),
             EmbeddedCodeEditor::RenderGate::WindowVisible)
{
  m_editor.configureHost();

  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::JsCodeEditor::onThemeChanged);

  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::JsCodeEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (!m_readingCode)
      m_projectModel.storeFrameParserCode(m_sourceId, text());
  });

  connect(&m_projectModel,
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(&m_projectModel,
          &DataModel::ProjectModel::frameParserLanguageChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(&m_projectModel,
          &DataModel::ProjectModel::sourceFrameParserLanguageChanged,
          this,
          [this](int sourceId) {
            if (sourceId == m_sourceId)
              readCode();
          });

  connect(&m_projectModel,
          &DataModel::ProjectModel::sourceFrameParserCodeChanged,
          this,
          [this](int sourceId) {
            if (sourceId == m_sourceId)
              readCode();
          });

  connect(&m_projectEditor,
          &DataModel::ProjectEditor::selectedSourceFrameParserCodeChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::JsCodeEditor::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this, &DataModel::JsCodeEditor::resizeWidget);

  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::JsCodeEditor::scheduleRender);
  connect(&widget, &QCodeEditor::selectionChanged, this, &DataModel::JsCodeEditor::scheduleRender);
  connect(
    &widget, &QCodeEditor::cursorPositionChanged, this, &DataModel::JsCodeEditor::scheduleRender);
  connect(
    &m_timerEvents, &Misc::TimerEvents::uiTimeout, this, &DataModel::JsCodeEditor::renderWidget);

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::JsCodeEditor::text() const
{
  return m_editor.text();
}

/**
 * @brief Returns the source ID this editor is bound to.
 */
int DataModel::JsCodeEditor::sourceId() const noexcept
{
  return m_sourceId;
}

/**
 * @brief Switches the editor to show the frame parser code for the source.
 */
void DataModel::JsCodeEditor::setSourceId(const int sourceId)
{
  if (m_sourceId == sourceId)
    return;

  m_sourceId = sourceId;
  Q_EMIT sourceIdChanged();
  readCode();
}

/**
 * @brief Returns the active scripting language (0=JS, 1=Lua).
 */
int DataModel::JsCodeEditor::language() const noexcept
{
  return m_language;
}

/**
 * @brief Switches the syntax highlighter and completer for the language.
 */
void DataModel::JsCodeEditor::setLanguage(const int language)
{
  if (m_language == language)
    return;

  m_language = language;
  m_editor.setScriptLanguage(language == 1);
  Q_EMIT languageChanged();
}

/**
 * @brief Handles a user-initiated language switch from the QML combobox.
 */
void DataModel::JsCodeEditor::switchLanguage(const int language)
{
  if (m_language == language)
    return;

  if (language == SerialStudio::Native || m_language == SerialStudio::Native) {
    switchNativeLanguage(language);
    return;
  }

  if (isModified()) {
    const auto answer = Misc::Utilities::showMessageBox(
      tr("Change Scripting Language?"),
      tr("Switching the scripting language replaces the current "
         "parser code with the equivalent template in the new language."
         "\n\nAny unsaved changes are lost. Continue?"),
      QMessageBox::Warning,
      QString(),
      QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes)
      return;
  }

  const int tmplIdx = m_frameParser.detectTemplate(text());

  m_projectModel.updateSourceFrameParserLanguage(m_sourceId, language);

  if (tmplIdx >= 0)
    m_frameParser.setTemplateIdx(m_sourceId, tmplIdx);
  else
    m_frameParser.loadDefaultTemplate(m_sourceId, true);
}

/**
 * @brief Switches into/out of Native, converting the template to its equivalent both ways.
 */
void DataModel::JsCodeEditor::switchNativeLanguage(const int language)
{
  SS_ASSERT(language == SerialStudio::Native || m_language == SerialStudio::Native, return);

  if (language == SerialStudio::Native) {
    const int tmplIdx = m_frameParser.detectTemplate(text());
    m_projectModel.updateSourceFrameParserLanguage(m_sourceId, language);

    QString template_id;
    QJsonObject template_params;
    if (tmplIdx >= 0
        && DataModel::FrameParser::nativeEquivalentForFile(
          m_frameParser.templateFiles().at(tmplIdx), template_id, template_params)) {
      m_projectModel.updateSourceFrameParserParams(m_sourceId, template_params);
      m_projectModel.updateSourceFrameParserTemplate(m_sourceId, template_id);
    } else if (m_projectModel.frameParserTemplate(m_sourceId).isEmpty()) {
      m_frameParser.loadDefaultTemplate(m_sourceId, true);
    }

    m_frameParser.readCode();
    return;
  }

  const QString file = DataModel::FrameParser::fileForNativeTemplate(
    m_projectModel.frameParserTemplate(m_sourceId), m_projectModel.frameParserParams(m_sourceId));
  m_projectModel.updateSourceFrameParserLanguage(m_sourceId, language);

  const int idx = static_cast<int>(m_frameParser.templateFiles().indexOf(file));
  if (idx >= 0)
    m_frameParser.setTemplateIdx(m_sourceId, idx);
  else
    m_frameParser.loadDefaultTemplate(m_sourceId, true);
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::JsCodeEditor::isModified() const noexcept
{
  return m_editor.isModified();
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::JsCodeEditor::undoAvailable() const noexcept
{
  return m_editor.undoAvailable();
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::JsCodeEditor::redoAvailable() const noexcept
{
  return m_editor.redoAvailable();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::JsCodeEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::JsCodeEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::JsCodeEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Opens the online documentation for the frame parser.
 */
void DataModel::JsCodeEditor::help()
{
  // clang-format off
  QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki/Data-Flow-in-Serial-Studio#frame-parser-function")));
  // clang-format on
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::JsCodeEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::JsCodeEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Validates the current code and reloads the FrameParser engine.
 */
void DataModel::JsCodeEditor::apply()
{
  (void)m_frameParser.loadScript(m_sourceId, text(), true);
}

/**
 * @brief Opens a file dialog to import an external script file.
 */
void DataModel::JsCodeEditor::importFile()
{
  const auto filter = (m_language == 1) ? QStringLiteral("*.lua") : QStringLiteral("*.js");
  const auto title =
    (m_language == 1) ? tr("Select Lua file to import") : tr("Select Javascript file to import");
  auto* dialog = new QFileDialog(qApp->activeWindow(), title, QDir::homePath(), filter);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        if (m_editor.importFromFile(path))
          apply();
      },
      Qt::QueuedConnection);
  });

  dialog->open();
}

/**
 * @brief Validates the current code for syntax errors and reports the result.
 */
void DataModel::JsCodeEditor::evaluate()
{
  if (m_frameParser.loadScript(m_sourceId, text(), true)) {
    Misc::Utilities::showMessageBox(tr("Code Validation Successful"),
                                    tr("No syntax errors detected in the parser code."),
                                    QMessageBox::Information);
  }
}

/**
 * @brief Reloads the editor text from the current project model code.
 */
void DataModel::JsCodeEditor::readCode()
{
  if (m_readingCode)
    return;

  m_readingCode = true;

  QString code;
  int lang            = 0;
  const auto& sources = m_projectModel.sources();
  for (const auto& src : sources) {
    if (src.sourceId == m_sourceId) {
      code = src.frameParserCode;
      lang = src.frameParserLanguage;
      break;
    }
  }

  if (code.isEmpty())
    code = m_projectModel.frameParserCode();

  setLanguage(lang);
  m_editor.setSourceText(code);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::JsCodeEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Reformats the entire editor contents.
 */
void DataModel::JsCodeEditor::formatDocument()
{
  const auto lang =
    (m_language == 1) ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript;
  EditorFormatting::formatDocument(m_editor.widget(), lang);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::JsCodeEditor::formatSelection()
{
  const auto lang =
    (m_language == 1) ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript;
  EditorFormatting::formatSelection(m_editor.widget(), lang);
}

/**
 * @brief Shows a template picker dialog and loads the selected template.
 */
void DataModel::JsCodeEditor::selectTemplate()
{
  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Frame Parser Template"),
                                          tr("Choose a template to load:"),
                                          m_frameParser.templateNames(),
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = m_frameParser.templateNames().indexOf(name);
  if (idx < 0)
    return;

  if (m_sourceId > 0) {
    m_frameParser.setTemplateIdx(m_sourceId, idx);
    m_editor.setSourceText(m_frameParser.templateCode(m_sourceId));
    Q_EMIT modifiedChanged();
    return;
  }

  m_frameParser.setTemplateIdx(0, idx);
}

/**
 * @brief Loads the current script into the live engine; true when the test dialog may open.
 */
bool DataModel::JsCodeEditor::prepareParserTest()
{
  return m_frameParser.loadScript(m_sourceId, text(), true);
}

/**
 * @brief Reloads the default template.
 */
void DataModel::JsCodeEditor::reload(const bool guiTrigger)
{
  loadDefaultTemplate(guiTrigger);
}

/**
 * @brief Loads the default CSV template.
 */
void DataModel::JsCodeEditor::loadDefaultTemplate(const bool guiTrigger)
{
  m_frameParser.loadDefaultTemplate(m_sourceId, guiTrigger);
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the editor colour scheme to match the active theme.
 */
void DataModel::JsCodeEditor::onThemeChanged()
{
  m_editor.applyTheme();
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the cached pixmap stale; the next UI tick does the grab, so a burst of edits
 *        costs one widget render instead of one per event.
 */
void DataModel::JsCodeEditor::scheduleRender()
{
  m_editor.scheduleRender();
}

/**
 * @brief Captures the editor widget as a pixmap for QML painting.
 */
void DataModel::JsCodeEditor::renderWidget()
{
  m_editor.renderWidget();
}

/**
 * @brief Resizes the backing widget to match the QML item dimensions.
 */
void DataModel::JsCodeEditor::resizeWidget()
{
  m_editor.resizeWidget();
}

//--------------------------------------------------------------------------------------------------
// Event processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::JsCodeEditor::paint(QPainter* painter)
{
  m_editor.paint(painter);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::JsCodeEditor::event(QEvent* event)
{
  if (m_editor.handleShortcutOverride(event))
    return true;

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::JsCodeEditor::keyPressEvent(QKeyEvent* event)
{
  m_editor.handleKeyPress(event);
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::JsCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::JsCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::JsCodeEditor::focusInEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::JsCodeEditor::focusOutEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards mouse-press events to the backing widget, claiming focus for the item.
 */
void DataModel::JsCodeEditor::mousePressEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, true);
}

/**
 * @brief Forwards mouse-move events to the backing widget.
 */
void DataModel::JsCodeEditor::mouseMoveEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards mouse-release events to the backing widget.
 */
void DataModel::JsCodeEditor::mouseReleaseEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards double-click events to the backing widget.
 */
void DataModel::JsCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::JsCodeEditor::wheelEvent(QWheelEvent* event)
{
  m_editor.forwardToViewport(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::JsCodeEditor::dropEvent(QDropEvent* event)
{
  m_editor.forwardToViewport(event);
}
