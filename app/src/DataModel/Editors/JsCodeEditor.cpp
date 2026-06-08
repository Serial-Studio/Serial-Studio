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

#include "DataModel/Editors/JsCodeEditor.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJavascriptHighlighter>
#include <QJsonObject>
#include <QLineNumberArea>
#include <QLuaCompleter>
#include <QLuaHighlighter>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextDocument>
#include <QUrl>

#include "DataModel/Editors/CodeFormatter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

/**
 * @brief Constructs the QML-side frame parser code editor.
 */
DataModel::JsCodeEditor::JsCodeEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_sourceId(0), m_language(0), m_readingCode(false)
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

  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(4);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());
  m_widget.setLayoutDirection(Qt::LeftToRight);

  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::JsCodeEditor::onThemeChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::JsCodeEditor::textChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (!m_readingCode)
      ProjectModel::instance().storeFrameParserCode(m_sourceId, text());
  });

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserLanguageChanged,
          this,
          &DataModel::JsCodeEditor::readCode);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceFrameParserLanguageChanged,
          this,
          [this](int sourceId) {
            if (sourceId == m_sourceId)
              readCode();
          });

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

  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::JsCodeEditor::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this, &DataModel::JsCodeEditor::resizeWidget);

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::JsCodeEditor::renderWidget);

  readCode();
}

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::JsCodeEditor::text() const
{
  return m_widget.toPlainText();
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

  auto& parser = DataModel::FrameParser::instance();
  auto& model  = DataModel::ProjectModel::instance();

  const int tmplIdx = parser.detectTemplate(text());

  model.updateSourceFrameParserLanguage(m_sourceId, language);

  if (tmplIdx >= 0)
    parser.setTemplateIdx(m_sourceId, tmplIdx);
  else
    parser.loadDefaultTemplate(m_sourceId, true);
}

/**
 * @brief Switches into/out of Native, converting the template to its equivalent both ways.
 */
void DataModel::JsCodeEditor::switchNativeLanguage(const int language)
{
  Q_ASSERT(language == SerialStudio::Native || m_language == SerialStudio::Native);

  auto& model  = DataModel::ProjectModel::instance();
  auto& parser = DataModel::FrameParser::instance();

  if (language == SerialStudio::Native) {
    const int tmplIdx = parser.detectTemplate(text());
    model.updateSourceFrameParserLanguage(m_sourceId, language);

    QString template_id;
    QJsonObject template_params;
    if (tmplIdx >= 0
        && DataModel::FrameParser::nativeEquivalentForFile(
          parser.templateFiles().at(tmplIdx), template_id, template_params)) {
      model.updateSourceFrameParserParams(m_sourceId, template_params);
      model.updateSourceFrameParserTemplate(m_sourceId, template_id);
    } else if (model.frameParserTemplate(m_sourceId).isEmpty()) {
      parser.loadDefaultTemplate(m_sourceId, true);
    }

    parser.readCode();
    return;
  }

  const QString file = DataModel::FrameParser::fileForNativeTemplate(
    model.frameParserTemplate(m_sourceId), model.frameParserParams(m_sourceId));
  model.updateSourceFrameParserLanguage(m_sourceId, language);

  const int idx = static_cast<int>(parser.templateFiles().indexOf(file));
  if (idx >= 0)
    parser.setTemplateIdx(m_sourceId, idx);
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

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::JsCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::JsCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::JsCodeEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::JsCodeEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
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

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::JsCodeEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
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
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
          m_widget.setPlainText(QString::fromUtf8(file.readAll()));
          file.close();
          apply();
        }
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
  if (m_readingCode)
    return;

  m_readingCode = true;

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

  setLanguage(lang);

  m_widget.setPlainText(code);
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::JsCodeEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Reformats the entire editor contents.
 */
void DataModel::JsCodeEditor::formatDocument()
{
  const auto lang =
    (m_language == 1) ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript;
  const QString original  = m_widget.toPlainText();
  const QString formatted = CodeFormatter::formatDocument(original, lang);
  if (formatted == original)
    return;

  QTextCursor cursor = m_widget.textCursor();
  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_widget.setTextCursor(cursor);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::JsCodeEditor::formatSelection()
{
  const auto lang =
    (m_language == 1) ? CodeFormatter::Language::Lua : CodeFormatter::Language::JavaScript;
  const QString original = m_widget.toPlainText();

  QTextCursor cursor = m_widget.textCursor();
  QTextCursor first(m_widget.document());
  first.setPosition(qMin(cursor.selectionStart(), cursor.selectionEnd()));
  QTextCursor last(m_widget.document());
  last.setPosition(qMax(cursor.selectionStart(), cursor.selectionEnd()));

  const int firstLine     = first.blockNumber();
  const int lastLine      = last.blockNumber();
  const QString formatted = CodeFormatter::formatLineRange(original, lang, firstLine, lastLine);
  if (formatted == original)
    return;

  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_widget.setTextCursor(cursor);
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
 * @brief Loads the current script into the live engine; true when the test dialog may open.
 */
bool DataModel::JsCodeEditor::prepareParserTest()
{
  auto& parser = DataModel::FrameParser::instance();
  return parser.loadScript(m_sourceId, text(), true);
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
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();

  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/themes/code-editor/%1.xml").arg(name);

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

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::JsCodeEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * @brief Forwards key-press events to the backing QCodeEditor widget.
 */
void DataModel::JsCodeEditor::keyPressEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::JsCodeEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::JsCodeEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::JsCodeEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
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

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::JsCodeEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::JsCodeEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::JsCodeEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
