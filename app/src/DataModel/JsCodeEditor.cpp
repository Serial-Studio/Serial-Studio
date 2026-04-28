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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/JsCodeEditor.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJavascriptHighlighter>
#include <QLineNumberArea>
#include <QLuaCompleter>
#include <QLuaHighlighter>
#include <QMessageBox>
#include <QTextDocument>
#include <QUrl>

#include "DataModel/FrameParser.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"

/** @brief Constructs the QML-side frame parser code editor. */
DataModel::JsCodeEditor::JsCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_sourceId(0)
  , m_language(0)
  , m_readingCode(false)
  , m_testDialog(&DataModel::FrameParser::instance(), nullptr)
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
          &DataModel::JsCodeEditor::onThemeChanged);

  // Emit modification signals when the editor document changes
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::JsCodeEditor::textChanged);

  // Push code to the project model on every edit (guarded by m_readingCode)
  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (!m_readingCode)
      ProjectModel::instance().storeFrameParserCode(m_sourceId, text());
  });

  // Keep display in sync when the project loads new parser code or language
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserLanguageChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  // Per-source language changes: only react when they target THIS editor
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceFrameParserLanguageChanged,
          this,
          [this](int sourceId) {
            if (sourceId == m_sourceId)
              readCode();
          });

  // Per-source code changes (from API or multi-source editor flows)
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceFrameParserCodeChanged,
          this,
          [this](int sourceId) {
            if (sourceId == m_sourceId)
              readCode();
          });

  connect(&DataModel::ProjectEditor::instance(),
          &DataModel::ProjectEditor::selectedSourceFrameParserCodeChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  // Resize backing widget to match QML item dimensions
  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::JsCodeEditor::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this, &DataModel::JsCodeEditor::resizeWidget);

  // Drive the render loop
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::JsCodeEditor::renderWidget);

  // Populate editor text from the current project state
  readCode();
}

/** @brief Returns the editor's current text. */
QString DataModel::JsCodeEditor::text() const
{
  return m_widget.toPlainText();
}

/** @brief Returns the source ID this editor is bound to. */
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

/** @brief Returns the active scripting language (0=JS, 1=Lua). */
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

  // Switch syntax highlighter and completer
  if (language == 1) {
    m_widget.setHighlighter(new QLuaHighlighter());
    m_widget.setCompleter(new QLuaCompleter(&m_widget));
  } else {
    m_widget.setHighlighter(new QJavascriptHighlighter());
    m_widget.setCompleter(nullptr);
  }

  Q_EMIT languageChanged();
}

/**
 * @brief Handles a user-initiated language switch from the QML combobox.
 */
void DataModel::JsCodeEditor::switchLanguage(const int language)
{
  if (m_language == language)
    return;

  // Warn user if the editor has been modified
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

  auto& parser = DataModel::FrameParser::instance();
  auto& model  = DataModel::ProjectModel::instance();

  // Detect which template the current code matches BEFORE switching
  const int tmplIdx = parser.detectTemplate(text());

  // Flip the project model language so templateCode() resolves to the
  // new-language path when setTemplateIdx runs
  model.updateSourceFrameParserLanguage(m_sourceId, language);

  // Load the equivalent template in the new language, or the default if
  // the current code doesn't match any known template
  if (tmplIdx >= 0)
    parser.setTemplateIdx(m_sourceId, tmplIdx);
  else
    parser.loadDefaultTemplate(m_sourceId, true);
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::JsCodeEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/** @brief Returns true when an undo step is available in the editor. */
bool DataModel::JsCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/** @brief Returns true when a redo step is available in the editor. */
bool DataModel::JsCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

/** @brief Cuts the current selection into the clipboard. */
void DataModel::JsCodeEditor::cut()
{
  m_widget.cut();
}

/** @brief Undoes the last edit. */
void DataModel::JsCodeEditor::undo()
{
  m_widget.undo();
}

/** @brief Redoes the previously undone edit. */
void DataModel::JsCodeEditor::redo()
{
  m_widget.redo();
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

/** @brief Copies the current selection to the clipboard. */
void DataModel::JsCodeEditor::copy()
{
  m_widget.copy();
}

/** @brief Pastes the clipboard contents into the editor. */
void DataModel::JsCodeEditor::paste()
{
  m_widget.paste();
}

/**
 * @brief Validates the current code and reloads the FrameParser engine.
 */
void DataModel::JsCodeEditor::apply()
{
  (void)DataModel::FrameParser::instance().loadScript(m_sourceId, text(), true);
}

/**
 * @brief Opens a file dialog to import an external script file.
 */
void DataModel::JsCodeEditor::import()
{
  // Build language-specific file filter
  const auto filter = (m_language == 1) ? QStringLiteral("*.lua") : QStringLiteral("*.js");
  const auto title =
    (m_language == 1) ? tr("Select Lua file to import") : tr("Select Javascript file to import");
  auto* dialog = new QFileDialog(nullptr, title, QDir::homePath(), filter);
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty()) {
      QFile file(path);
      if (file.open(QFile::ReadOnly)) {
        m_widget.setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
        apply();
      }
    }
    dialog->deleteLater();
  });

  dialog->open();
}

/**
 * @brief Validates the current code for syntax errors and reports the result.
 */
void DataModel::JsCodeEditor::evaluate()
{
  // Validate and report
  auto& parser = DataModel::FrameParser::instance();
  if (parser.loadScript(m_sourceId, text(), true)) {
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
  // Prevent reentrancy
  if (m_readingCode)
    return;

  m_readingCode = true;

  // Find code and language for the active source
  QString code;
  int lang            = 0;
  const auto& sources = DataModel::ProjectModel::instance().sources();
  for (const auto& src : sources) {
    if (src.sourceId == m_sourceId) {
      code = src.frameParserCode;
      lang = src.frameParserLanguage;
      break;
    }
  }

  if (code.isEmpty())
    code = DataModel::ProjectModel::instance().frameParserCode();

  // Sync language and load code into the editor
  setLanguage(lang);

  m_widget.setPlainText(code);
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/** @brief Selects all editor text. */
void DataModel::JsCodeEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Shows a template picker dialog and loads the selected template.
 */
void DataModel::JsCodeEditor::selectTemplate()
{
  auto& parser = DataModel::FrameParser::instance();

  bool ok;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Frame Parser Template"),
                                          tr("Choose a template to load:"),
                                          parser.templateNames(),
                                          0,
                                          false,
                                          &ok);

  if (!ok)
    return;

  const int idx = parser.templateNames().indexOf(name);
  if (idx < 0)
    return;

  if (m_sourceId > 0) {
    parser.setTemplateIdx(m_sourceId, idx);
    const QString code = parser.templateCode(m_sourceId);
    m_widget.setPlainText(code);
    m_widget.document()->clearUndoRedoStacks();
    m_widget.document()->setModified(false);
    Q_EMIT modifiedChanged();
    return;
  }

  parser.setTemplateIdx(0, idx);
}

/**
 * @brief Validates the current script and opens the test dialog on success.
 */
void DataModel::JsCodeEditor::testWithSampleData()
{
  auto& parser = DataModel::FrameParser::instance();
  if (parser.loadScript(m_sourceId, text(), true)) {
    m_testDialog.setSourceId(m_sourceId);
    m_testDialog.clear();
    m_testDialog.showNormal();
  }
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
  DataModel::FrameParser::instance().loadDefaultTemplate(m_sourceId, guiTrigger);
}

/**
 * @brief Updates the editor colour scheme to match the active theme.
 */
void DataModel::JsCodeEditor::onThemeChanged()
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

/**
 * @brief Captures the editor widget as a pixmap for QML painting.
 */
void DataModel::JsCodeEditor::renderWidget()
{
  if (isVisible()) {
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * @brief Resizes the backing widget to match the QML item dimensions.
 */
void DataModel::JsCodeEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    renderWidget();
  }
}

//--------------------------------------------------------------------------------------------------
// Event processing
//--------------------------------------------------------------------------------------------------

/** @brief Paints the cached editor pixmap into the QML scene. */
void DataModel::JsCodeEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/** @brief Forwards key-press events to the backing QCodeEditor widget. */
void DataModel::JsCodeEditor::keyPressEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards key-release events to the backing QCodeEditor widget. */
void DataModel::JsCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards input-method events (IME composition) to the backing widget. */
void DataModel::JsCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards focus-in events to the backing widget. */
void DataModel::JsCodeEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards focus-out events to the backing widget. */
void DataModel::JsCodeEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards mouse-press events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::JsCodeEditor::mousePressEvent(QMouseEvent* event)
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
void DataModel::JsCodeEditor::mouseMoveEvent(QMouseEvent* event)
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
void DataModel::JsCodeEditor::mouseReleaseEvent(QMouseEvent* event)
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
void DataModel::JsCodeEditor::mouseDoubleClickEvent(QMouseEvent* event)
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
void DataModel::JsCodeEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-enter events to the editor viewport. */
void DataModel::JsCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-move events to the editor viewport. */
void DataModel::JsCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drag-leave events to the editor viewport. */
void DataModel::JsCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/** @brief Forwards drop events to the editor viewport. */
void DataModel::JsCodeEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
