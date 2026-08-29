/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Editors/OutputCodeEditor.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>

#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side output widget transmit-function editor. It lives in the Project
 *        Editor window, which stays instantiated once opened, hence the window-visibility gate.
 */
DataModel::OutputCodeEditor::OutputCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_readingCode(false)
  , m_themeManager(Misc::ThemeManager::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_translator(Misc::Translator::instance())
  , m_projectEditor(DataModel::ProjectEditor::instance())
  , m_projectModel(DataModel::ProjectModel::instance())
  , m_editor(*this,
             m_themeManager,
             Misc::CommonFonts::instance(),
             EmbeddedCodeEditor::RenderGate::WindowVisible)
  , m_templates(QStringLiteral(":/scripts/output/templates.json"),
                QStringLiteral(":/scripts/output"),
                QStringLiteral(".js"),
                "DataModel::OutputCodeEditor")
  , m_testDialog(nullptr)
{
  m_editor.configureHost();

  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::OutputCodeEditor::onThemeChanged);

  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::OutputCodeEditor::textChanged);

  connect(&widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    if (m_projectEditor.currentView() != DataModel::ProjectEditor::OutputWidgetView)
      return;

    const auto& sel = m_projectEditor.selectedOutputWidget();
    if (sel.groupId < 0 || sel.widgetId < 0)
      return;

    m_projectEditor.setSelectedOutputWidgetTransmitFunction(text());
  });

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

  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(
    this, &QQuickPaintedItem::heightChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::OutputCodeEditor::scheduleRender);
  connect(
    &widget, &QCodeEditor::selectionChanged, this, &DataModel::OutputCodeEditor::scheduleRender);
  connect(&widget,
          &QCodeEditor::cursorPositionChanged,
          this,
          &DataModel::OutputCodeEditor::scheduleRender);
  connect(&m_timerEvents,
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::OutputCodeEditor::renderWidget);

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
}

/**
 * @brief Shows a dialog to pick and load a built-in template.
 */
void DataModel::OutputCodeEditor::selectTemplate()
{
  if (m_templates.isEmpty())
    return;

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Output Widget Template"),
                                          tr("Choose a template to load:"),
                                          m_templates.names(),
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = m_templates.names().indexOf(name);
  if (idx < 0 || idx >= m_templates.files().size())
    return;

  QFile file(m_templates.files().at(idx));
  if (file.open(QFile::ReadOnly)) {
    m_editor.setSourceText(QString::fromUtf8(file.readAll()));
    Q_EMIT modifiedChanged();
    file.close();
  }
}

/**
 * @brief Opens the transmit test dialog with the current editor code.
 */
void DataModel::OutputCodeEditor::testTransmitFunction()
{
  m_testDialog.setTransmitCode(text());
  m_testDialog.clear();
  m_testDialog.showNormal();
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
  return defaultScriptTemplateCode(QStringLiteral(":/scripts/output/templates.json"),
                                   QStringLiteral(":/scripts/output"),
                                   QStringLiteral(".js"),
                                   "DataModel::OutputCodeEditor");
}

/**
 * @brief Rebuilds the cached list of output-widget templates from resources.
 */
void DataModel::OutputCodeEditor::loadTemplates()
{
  m_templates.reload();
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::OutputCodeEditor::onThemeChanged()
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
void DataModel::OutputCodeEditor::scheduleRender()
{
  m_editor.scheduleRender();
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering.
 */
void DataModel::OutputCodeEditor::renderWidget()
{
  m_editor.renderWidget();
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::OutputCodeEditor::resizeWidget()
{
  m_editor.resizeWidget();
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::OutputCodeEditor::paint(QPainter* painter)
{
  m_editor.paint(painter);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::OutputCodeEditor::event(QEvent* event)
{
  if (m_editor.handleShortcutOverride(event))
    return true;

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::OutputCodeEditor::keyPressEvent(QKeyEvent* event)
{
  m_editor.handleKeyPress(event);
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::OutputCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::OutputCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::OutputCodeEditor::focusInEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::OutputCodeEditor::focusOutEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards mouse-press events to the backing widget, claiming focus for the item.
 */
void DataModel::OutputCodeEditor::mousePressEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, true);
}

/**
 * @brief Forwards mouse-move events to the backing widget.
 */
void DataModel::OutputCodeEditor::mouseMoveEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards mouse-release events to the backing widget.
 */
void DataModel::OutputCodeEditor::mouseReleaseEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards double-click events to the backing widget.
 */
void DataModel::OutputCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::OutputCodeEditor::wheelEvent(QWheelEvent* event)
{
  m_editor.forwardToViewport(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::OutputCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::OutputCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::OutputCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::OutputCodeEditor::dropEvent(QDropEvent* event)
{
  m_editor.forwardToViewport(event);
}
