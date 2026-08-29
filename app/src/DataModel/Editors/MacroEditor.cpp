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

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML-side macro editor. The macros dialog and the annotations decoder tab
 *        live inside the main window, so the item-visibility gate is enough here.
 */
DataModel::MacroEditor::MacroEditor(QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_language(0)
  , m_themeManager(Misc::ThemeManager::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_editor(*this,
             m_themeManager,
             Misc::CommonFonts::instance(),
             EmbeddedCodeEditor::RenderGate::ItemVisible)
{
  m_editor.configureHost();

  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &DataModel::MacroEditor::onThemeChanged);

  auto& widget = m_editor.widget();
  connect(&widget, &QCodeEditor::textChanged, this, [this] { Q_EMIT modifiedChanged(); });
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::MacroEditor::textChanged);
  connect(&widget, &QCodeEditor::textChanged, this, &DataModel::MacroEditor::scheduleRender);
  connect(&widget, &QCodeEditor::selectionChanged, this, &DataModel::MacroEditor::scheduleRender);
  connect(
    &widget, &QCodeEditor::cursorPositionChanged, this, &DataModel::MacroEditor::scheduleRender);

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
  return m_editor.text();
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
  return m_editor.isModified();
}

//--------------------------------------------------------------------------------------------------
// Editor operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::MacroEditor::cut()
{
  m_editor.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::MacroEditor::undo()
{
  m_editor.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::MacroEditor::redo()
{
  m_editor.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::MacroEditor::copy()
{
  m_editor.copy();
}

/**
 * @brief Clears the editor content and its undo history.
 */
void DataModel::MacroEditor::clear()
{
  m_editor.setSourceText(QString());
  Q_EMIT modifiedChanged();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::MacroEditor::paste()
{
  m_editor.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::MacroEditor::selectAll()
{
  m_editor.selectAll();
}

/**
 * @brief Switches the highlighter/completer pair to the given language
 *        (SerialStudio::ScriptLanguage encoding).
 */
void DataModel::MacroEditor::setLanguage(int language)
{
  if (m_language == language)
    return;

  m_language = language;
  m_editor.setScriptLanguage(language == SerialStudio::Lua);

  Q_EMIT languageChanged();
  scheduleRender();
}

/**
 * @brief Replaces the editor content (macro load): the undo-history and modified-state reset
 *        always runs, even when the incoming text matches the current content.
 */
void DataModel::MacroEditor::setText(const QString& text)
{
  m_editor.syncSourceText(text);
  Q_EMIT modifiedChanged();
  scheduleRender();
}

/**
 * @brief Marks the current content as saved (macro save), clearing the modified flag.
 */
void DataModel::MacroEditor::markSaved()
{
  m_editor.markSaved();
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
  m_editor.applyTheme();
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
  m_editor.scheduleRender();
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering, at most once per UI tick and
 *        only when something changed; a focused editor always regrabs so the caret keeps blinking.
 */
void DataModel::MacroEditor::renderWidget()
{
  m_editor.renderWidget();
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::MacroEditor::resizeWidget()
{
  m_editor.resizeWidget();
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::MacroEditor::paint(QPainter* painter)
{
  m_editor.paint(painter);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::MacroEditor::event(QEvent* event)
{
  if (m_editor.handleShortcutOverride(event))
    return true;

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::MacroEditor::keyPressEvent(QKeyEvent* event)
{
  m_editor.handleKeyPress(event);
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::MacroEditor::keyReleaseEvent(QKeyEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::MacroEditor::inputMethodEvent(QInputMethodEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::MacroEditor::focusInEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::MacroEditor::focusOutEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards mouse-press events to the backing widget, claiming focus for the item.
 */
void DataModel::MacroEditor::mousePressEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, true);
}

/**
 * @brief Forwards mouse-move events to the backing widget.
 */
void DataModel::MacroEditor::mouseMoveEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards mouse-release events to the backing widget.
 */
void DataModel::MacroEditor::mouseReleaseEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards double-click events to the backing widget.
 */
void DataModel::MacroEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::MacroEditor::wheelEvent(QWheelEvent* event)
{
  m_editor.forwardToViewport(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::MacroEditor::dragEnterEvent(QDragEnterEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::MacroEditor::dragMoveEvent(QDragMoveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::MacroEditor::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::MacroEditor::dropEvent(QDropEvent* event)
{
  m_editor.forwardToViewport(event);
}
