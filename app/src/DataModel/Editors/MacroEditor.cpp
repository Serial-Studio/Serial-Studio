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

#include <QAbstractItemView>
#include <QCompleter>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJavascriptHighlighter>
#include <QLineNumberArea>
#include <QLuaHighlighter>
#include <QTextDocument>

#include "DataModel/Editors/SerialStudioCompleter.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side macro editor.
 */
DataModel::MacroEditor::MacroEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_dirty(true)
  , m_language(0)
  , m_themeManager(Misc::ThemeManager::instance())
  , m_commonFonts(Misc::CommonFonts::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
{
  setMipmap(false);
  setAntialiasing(false);
  setOpaquePainting(true);
  setAcceptTouchEvents(true);
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFillColor(m_themeManager.getColor(QStringLiteral("base")));

  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(2);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(m_commonFonts.monoFont());
  m_widget.setLayoutDirection(Qt::LeftToRight);
  m_widget.setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
  m_widget.setCompleter(new DataModel::SerialStudioCompleter(false, &m_widget));

  onThemeChanged();
  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::MacroEditor::onThemeChanged);

  connect(&m_widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::MacroEditor::textChanged);
  connect(&m_widget, &QCodeEditor::textChanged, this, &DataModel::MacroEditor::scheduleRender);
  connect(&m_widget, &QCodeEditor::selectionChanged, this, &DataModel::MacroEditor::scheduleRender);
  connect(
    &m_widget, &QCodeEditor::cursorPositionChanged, this, &DataModel::MacroEditor::scheduleRender);

  connect(this, &QQuickPaintedItem::widthChanged, this, &DataModel::MacroEditor::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this, &DataModel::MacroEditor::resizeWidget);
  connect(
    &m_timerEvents, &Misc::TimerEvents::uiTimeout, this, &DataModel::MacroEditor::renderWidget);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::MacroEditor::text() const
{
  return m_widget.toPlainText();
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
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::MacroEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::MacroEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::MacroEditor::redo()
{
  m_widget.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::MacroEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Clears the editor content and its undo history.
 */
void DataModel::MacroEditor::clear()
{
  m_widget.setPlainText(QString());
  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::MacroEditor::paste()
{
  m_widget.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::MacroEditor::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Switches the highlighter/completer pair to the given language
 *        (SerialStudio::ScriptLanguage encoding); replaced objects go through deleteLater
 *        like the parser editor does.
 */
void DataModel::MacroEditor::setLanguage(int language)
{
  if (m_language == language)
    return;

  m_language            = language;
  auto* old_completer   = m_widget.completer();
  auto* old_highlighter = m_widget.highlighter();

  if (language == SerialStudio::Lua) {
    m_widget.setHighlighter(new QLuaHighlighter());
    m_widget.setLanguageHint(QCodeEditor::LanguageHint::Lua);
    m_widget.setCompleter(new DataModel::SerialStudioCompleter(true, &m_widget));
  } else {
    m_widget.setHighlighter(new QJavascriptHighlighter());
    m_widget.setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
    m_widget.setCompleter(new DataModel::SerialStudioCompleter(false, &m_widget));
  }

  if (old_completer)
    old_completer->deleteLater();

  if (old_highlighter)
    old_highlighter->deleteLater();

  Q_EMIT languageChanged();
  scheduleRender();
}

/**
 * @brief Replaces the editor content (macro load): the undo-history and modified-state reset
 *        always runs, even when the incoming text matches the current content.
 */
void DataModel::MacroEditor::setText(const QString& text)
{
  if (m_widget.toPlainText() != text)
    m_widget.setPlainText(text);

  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
  scheduleRender();
}

/**
 * @brief Marks the current content as saved (macro save), clearing the modified flag.
 */
void DataModel::MacroEditor::markSaved()
{
  m_widget.document()->setModified(false);
  Q_EMIT modifiedChanged();
}

//--------------------------------------------------------------------------------------------------
// Theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the current theme to the code editor widget.
 */
void DataModel::MacroEditor::onThemeChanged()
{
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();

  const auto path =
    QDir::isAbsolutePath(name) ? name : QStringLiteral(":/themes/code-editor/%1.xml").arg(name);

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
 * @brief Marks the cached pixmap stale; the next UI tick does the grab, so a burst of edits
 *        costs one widget render instead of one per event.
 */
void DataModel::MacroEditor::scheduleRender()
{
  m_dirty = true;
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering, at most once per UI tick and
 *        only when something changed; a focused editor always regrabs so the caret keeps blinking.
 */
void DataModel::MacroEditor::renderWidget()
{
  if (!isVisible() || (!m_dirty && !hasActiveFocus()))
    return;

  m_dirty = false;
  syncWidgetPosition();
  m_pixmap = m_widget.grab();
  update();
}

/**
 * @brief Aligns the hidden widget's top-level position with the item's on-screen position so
 *        completer popups and drag auto-scroll resolve correct global coordinates.
 */
void DataModel::MacroEditor::syncWidgetPosition()
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
void DataModel::MacroEditor::resizeWidget()
{
  if (width() > 0 && height() > 0) {
    m_widget.setFixedSize(static_cast<int>(width()), static_cast<int>(height()));
    scheduleRender();
  }
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::MacroEditor::paint(QPainter* painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::MacroEditor::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride) {
    QCoreApplication::sendEvent(&m_widget, event);
    if (event->isAccepted())
      return true;
  }

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::MacroEditor::keyPressEvent(QKeyEvent* event)
{
  auto* completer = m_widget.completer();
  if (completer && completer->popup() && completer->popup()->isVisible()
      && SerialStudioCompleter::popupHandlesKey(event->key()))
    QCoreApplication::sendEvent(completer->popup(), event);
  else
    QCoreApplication::sendEvent(&m_widget, event);

  scheduleRender();
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::MacroEditor::keyReleaseEvent(QKeyEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::MacroEditor::inputMethodEvent(QInputMethodEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
  scheduleRender();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::MacroEditor::focusInEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
  scheduleRender();
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::MacroEditor::focusOutEvent(QFocusEvent* event)
{
  QCoreApplication::sendEvent(&m_widget, event);
  scheduleRender();
}

/** @brief Forwards mouse-press events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::MacroEditor::mousePressEvent(QMouseEvent* event)
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
  scheduleRender();
}

/** @brief Forwards mouse-move events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::MacroEditor::mouseMoveEvent(QMouseEvent* event)
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
  scheduleRender();
}

/** @brief Forwards mouse-release events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::MacroEditor::mouseReleaseEvent(QMouseEvent* event)
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
  scheduleRender();
}

/** @brief Forwards double-click events to the backing widget after offsetting for the line-number
 * gutter. */
void DataModel::MacroEditor::mouseDoubleClickEvent(QMouseEvent* event)
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
  scheduleRender();
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::MacroEditor::wheelEvent(QWheelEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
  scheduleRender();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::MacroEditor::dragEnterEvent(QDragEnterEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::MacroEditor::dragMoveEvent(QDragMoveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::MacroEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::MacroEditor::dropEvent(QDropEvent* event)
{
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}
