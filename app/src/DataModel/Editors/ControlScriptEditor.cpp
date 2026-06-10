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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Editors/ControlScriptEditor.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJavascriptHighlighter>
#include <QJSEngine>
#include <QLineNumberArea>
#include <QTextCursor>
#include <QTextDocument>

#include "DataModel/Editors/CodeFormatter.h"
#include "DataModel/Editors/SerialStudioCompleter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
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
 * @brief Constructs the QML-side control-script editor.
 */
DataModel::ControlScriptEditor::ControlScriptEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent), m_readingCode(false)
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
  m_widget.setTabReplaceSize(2);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());
  m_widget.setLayoutDirection(Qt::LeftToRight);
  m_widget.setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
  m_widget.setCompleter(new DataModel::SerialStudioCompleter(false, &m_widget));

  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::ControlScriptEditor::onThemeChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::ControlScriptEditor::textChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] {
    if (m_readingCode)
      return;

    auto& editor = DataModel::ProjectEditor::instance();
    if (editor.currentView() != DataModel::ProjectEditor::ControlScriptView)
      return;

    DataModel::ProjectModel::instance().setControlScriptCode(text());
  });

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::controlScriptChanged,
          this,
          &DataModel::ControlScriptEditor::readCode);

  connect(
    this, &QQuickPaintedItem::widthChanged, this, &DataModel::ControlScriptEditor::resizeWidget);
  connect(
    this, &QQuickPaintedItem::heightChanged, this, &DataModel::ControlScriptEditor::resizeWidget);
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::uiTimeout,
          this,
          &DataModel::ControlScriptEditor::renderWidget);

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
  return m_widget.toPlainText();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::ControlScriptEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::ControlScriptEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::ControlScriptEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::ControlScriptEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::ControlScriptEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::ControlScriptEditor::redo()
{
  m_widget.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::ControlScriptEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::ControlScriptEditor::paste()
{
  m_widget.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::ControlScriptEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Reformats the entire control-script source.
 */
void DataModel::ControlScriptEditor::formatDocument()
{
  const QString original = m_widget.toPlainText();
  const QString formatted =
    CodeFormatter::formatDocument(original, CodeFormatter::Language::JavaScript);
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
void DataModel::ControlScriptEditor::formatSelection()
{
  const QString original = m_widget.toPlainText();

  QTextCursor cursor = m_widget.textCursor();
  QTextCursor first(m_widget.document());
  first.setPosition(qMin(cursor.selectionStart(), cursor.selectionEnd()));
  QTextCursor last(m_widget.document());
  last.setPosition(qMax(cursor.selectionStart(), cursor.selectionEnd()));

  const int firstLine     = first.blockNumber();
  const int lastLine      = last.blockNumber();
  const QString formatted = CodeFormatter::formatLineRange(
    original, CodeFormatter::Language::JavaScript, firstLine, lastLine);
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
 * @brief Opens a file dialog to import an external JS file.
 */
void DataModel::ControlScriptEditor::import()
{
  auto* dialog = new QFileDialog(
    nullptr, tr("Select Javascript file to import"), QDir::homePath(), QStringLiteral("*.js"));
  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    dialog->deleteLater();
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(
      this,
      [this, path]() {
        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
          m_widget.setPlainText(QString::fromUtf8(file.readAll()));
          file.close();
        }
      },
      Qt::QueuedConnection);
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

  QString code = DataModel::ProjectModel::instance().controlScriptCode();
  if (code.isEmpty())
    code = defaultControlScript();

  if (m_widget.toPlainText() != code)
    m_widget.setPlainText(code);

  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);

  m_readingCode = false;
  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the editor to the default setup()/loop() template.
 */
void DataModel::ControlScriptEditor::reload()
{
  m_widget.setPlainText(defaultControlScript());
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

/**
 * @brief Compiles the script (without running it) and reports syntax errors.
 */
void DataModel::ControlScriptEditor::evaluate()
{
  QJSEngine engine;
  const auto result = engine.evaluate(text(), QStringLiteral("control-script.js"));
  if (result.isError()) {
    Misc::Utilities::showMessageBox(tr("Code Validation Failed"),
                                    tr("Line %1: %2")
                                      .arg(result.property(QStringLiteral("lineNumber")).toInt())
                                      .arg(result.toString()),
                                    QMessageBox::Warning);
    return;
  }

  const bool hasSetup = engine.globalObject().property(QStringLiteral("setup")).isCallable();
  const bool hasLoop  = engine.globalObject().property(QStringLiteral("loop")).isCallable();
  if (!hasSetup && !hasLoop) {
    Misc::Utilities::showMessageBox(tr("Code Validation Failed"),
                                    tr("The script must define a setup() and/or loop() function."),
                                    QMessageBox::Warning);
    return;
  }

  Misc::Utilities::showMessageBox(tr("Code Validation Successful"),
                                  tr("No syntax errors detected in the control script."),
                                  QMessageBox::Information);
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::ControlScriptEditor::onThemeChanged()
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

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering.
 */
void DataModel::ControlScriptEditor::renderWidget()
{
  if (isVisible()) {
    syncWidgetPosition();
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * @brief Aligns the hidden widget's top-level position with the item's on-screen position so
 *        completer popups and drag auto-scroll resolve correct global coordinates.
 */
void DataModel::ControlScriptEditor::syncWidgetPosition()
{
  if (!window())
    return;

  const QPoint global = mapToGlobal(QPointF(0, 0)).toPoint();
  if (m_widget.pos() != global)
    m_widget.move(global);
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::ControlScriptEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    renderWidget();
  }
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::ControlScriptEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::ControlScriptEditor::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride) {
    QCoreApplication::sendEvent(&m_widget, event);
    if (event->isAccepted())
      return true;
  }

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards key presses to the completer popup when visible, else to the editor widget.
 */
void DataModel::ControlScriptEditor::keyPressEvent(QKeyEvent* event)
{
  auto* completer = m_widget.completer();
  if (completer && completer->popup() && completer->popup()->isVisible())
    QCoreApplication::sendEvent(completer->popup(), event);
  else
    QCoreApplication::sendEvent(&m_widget, event);

  renderWidget();
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::ControlScriptEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::ControlScriptEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
  renderWidget();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::ControlScriptEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::ControlScriptEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/** @brief Forwards mouse-press events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::ControlScriptEditor::mousePressEvent(QMouseEvent* event)
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
  renderWidget();
}

/** @brief Forwards mouse-move events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::ControlScriptEditor::mouseMoveEvent(QMouseEvent* event)
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
  renderWidget();
}

/** @brief Forwards mouse-release events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::ControlScriptEditor::mouseReleaseEvent(QMouseEvent* event)
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
  renderWidget();
}

/** @brief Forwards double-click events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::ControlScriptEditor::mouseDoubleClickEvent(QMouseEvent* event)
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
  renderWidget();
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::ControlScriptEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
  renderWidget();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::ControlScriptEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::ControlScriptEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::ControlScriptEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::ControlScriptEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
