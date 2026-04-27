/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/OutputCodeEditor.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJavascriptHighlighter>
#include <QLineNumberArea>
#include <QTextDocument>

#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/ScriptTemplates.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/** @brief Constructs the QML-side output widget transmit-function editor. */
DataModel::OutputCodeEditor::OutputCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_readingCode(false), m_testDialog(nullptr)
{
  setMipmap(false);
  setAntialiasing(false);
  setOpaquePainting(true);
  setAcceptTouchEvents(true);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFillColor(Misc::ThemeManager::instance().getColor(QStringLiteral("base")));

  // Configure code editor
  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(4);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());

  // Apply current theme
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::OutputCodeEditor::onThemeChanged);

  // Emit modification signals
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::OutputCodeEditor::textChanged);

  // Push code to the selected output widget on every edit
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    auto& editor = DataModel::ProjectEditor::instance();
    if (editor.currentView() != DataModel::ProjectEditor::OutputWidgetView)
      return;

    auto& pm  = DataModel::ProjectModel::instance();
    auto& sel = editor.selectedOutputWidget();
    if (sel.groupId < 0 || sel.widgetId < 0)
      return;

    DataModel::OutputWidget updated = sel;
    updated.transmitFunction        = text();
    pm.updateOutputWidget(sel.groupId, sel.widgetId, updated, false);
  });

  // Reload when the output widget model changes
  connect(&DataModel::ProjectEditor::instance(),
          &DataModel::ProjectEditor::outputWidgetModelChanged,
          this,
          &DataModel::OutputCodeEditor::readCode);

  // Resize and render
  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(
    this, &QQuickPaintedItem::heightChanged, this, &DataModel::OutputCodeEditor::resizeWidget);
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::OutputCodeEditor::renderWidget);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DataModel::OutputCodeEditor::loadTemplates);

  loadTemplates();

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/** @brief Returns the editor's current text. */
QString DataModel::OutputCodeEditor::text() const
{
  return m_widget.toPlainText();
}

/** @brief Returns true when the editor document has unsaved edits. */
bool DataModel::OutputCodeEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/** @brief Returns true when an undo step is available in the editor. */
bool DataModel::OutputCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/** @brief Returns true when a redo step is available in the editor. */
bool DataModel::OutputCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/** @brief Cuts the current selection into the clipboard. */
void DataModel::OutputCodeEditor::cut()
{
  m_widget.cut();
}

/** @brief Undoes the last edit. */
void DataModel::OutputCodeEditor::undo()
{
  m_widget.undo();
}

/** @brief Redoes the previously undone edit. */
void DataModel::OutputCodeEditor::redo()
{
  m_widget.redo();
}

/** @brief Copies the current selection to the clipboard. */
void DataModel::OutputCodeEditor::copy()
{
  m_widget.copy();
}

/** @brief Pastes the clipboard contents into the editor. */
void DataModel::OutputCodeEditor::paste()
{
  m_widget.paste();
}

/** @brief Selects all editor text. */
void DataModel::OutputCodeEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::OutputCodeEditor::import()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty()) {
      QFile file(path);
      if (file.open(QFile::ReadOnly)) {
        m_widget.setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
      }
    }
    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Loads the transmit function from the currently selected output widget.
 */
void DataModel::OutputCodeEditor::readCode()
{
  // Prevent reentrancy
  if (m_readingCode)
    return;

  m_readingCode = true;

  auto& editor    = DataModel::ProjectEditor::instance();
  const auto& sel = editor.selectedOutputWidget();

  QString code = sel.transmitFunction;
  if (code.isEmpty())
    code = defaultTemplate();

  m_widget.setPlainText(code);
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Shows a dialog to pick and load a built-in template.
 */
void DataModel::OutputCodeEditor::selectTemplate()
{
  // No templates loaded
  if (m_templateNames.isEmpty())
    return;

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Output Widget Template"),
                                          tr("Choose a template to load:"),
                                          m_templateNames,
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = m_templateNames.indexOf(name);
  if (idx < 0 || idx >= m_templateFiles.size())
    return;

  QFile file(m_templateFiles.at(idx));
  if (file.open(QFile::ReadOnly)) {
    m_widget.setPlainText(QString::fromUtf8(file.readAll()));
    m_widget.document()->clearUndoRedoStacks();
    m_widget.document()->setModified(false);
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
  m_widget.setPlainText(defaultTemplate());
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

/**
 * @brief Loads the default transmit function template from resources.
 */
QString DataModel::OutputCodeEditor::defaultTemplate()
{
  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/rcc/scripts/output/templates.json"), "DataModel::OutputCodeEditor");

  QString defaultFile;
  for (const auto& tmpl : templates) {
    if (tmpl.isDefault) {
      defaultFile = tmpl.file;
      break;
    }
  }

  if (defaultFile.isEmpty() && !templates.isEmpty())
    defaultFile = templates.constFirst().file;

  if (defaultFile.isEmpty())
    return {};

  return readTextResource(templateResourcePath(
    QStringLiteral(":/rcc/scripts/output"), defaultFile, QStringLiteral(".js")));
}

/** @brief Rebuilds the cached list of output-widget templates from resources. */
void DataModel::OutputCodeEditor::loadTemplates()
{
  m_defaultTemplateFile.clear();
  m_templateNames.clear();
  m_templateFiles.clear();

  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/rcc/scripts/output/templates.json"), "DataModel::OutputCodeEditor");

  for (const auto& tmpl : templates) {
    m_templateNames.append(tmpl.name);
    m_templateFiles.append(templateResourcePath(
      QStringLiteral(":/rcc/scripts/output"), tmpl.file, QStringLiteral(".js")));
    if (m_defaultTemplateFile.isEmpty() && tmpl.isDefault)
      m_defaultTemplateFile = tmpl.file;
  }

  if (m_defaultTemplateFile.isEmpty() && !templates.isEmpty())
    m_defaultTemplateFile = templates.constFirst().file;
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::OutputCodeEditor::onThemeChanged()
{
  // Resolve theme path
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();

  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/rcc/themes/code-editor/%1.xml").arg(name);

  // Load syntax style XML
  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_widget.setSyntaxStyle(&m_style);
    file.close();
  }
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering.
 */
void DataModel::OutputCodeEditor::renderWidget()
{
  if (isVisible()) {
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::OutputCodeEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    renderWidget();
  }
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/** @brief Paints the cached editor pixmap into the QML scene. */
void DataModel::OutputCodeEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/** @brief Forwards key-press events to the backing QCodeEditor widget. */
void DataModel::OutputCodeEditor::keyPressEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards key-release events to the backing QCodeEditor widget. */
void DataModel::OutputCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards input-method events (IME composition) to the backing widget. */
void DataModel::OutputCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards focus-in events to the backing widget. */
void DataModel::OutputCodeEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards focus-out events to the backing widget. */
void DataModel::OutputCodeEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards mouse-press events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::OutputCodeEditor::mousePressEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
  forceActiveFocus();
}

/** @brief Forwards mouse-move events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::OutputCodeEditor::mouseMoveEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
}

/** @brief Forwards mouse-release events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::OutputCodeEditor::mouseReleaseEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
}

/** @brief Forwards double-click events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::OutputCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);
}

/** @brief Forwards wheel events to the editor viewport. */
void DataModel::OutputCodeEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-enter events to the editor viewport. */
void DataModel::OutputCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-move events to the editor viewport. */
void DataModel::OutputCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-leave events to the editor viewport. */
void DataModel::OutputCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drop events to the editor viewport. */
void DataModel::OutputCodeEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
