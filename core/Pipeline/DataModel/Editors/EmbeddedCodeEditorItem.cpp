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

#include "DataModel/Editors/EmbeddedCodeEditorItem.h"

#include <QPainter>

#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the offscreen editor for a QML host item and wires the plumbing every host
 *        shares: the theme hook, the UI-tick grab, the resize forward and the three document
 *        signals that mark the cached pixmap stale. @p gate picks the visibility test the grab
 *        may skip on: an item inside a closed window still reports itself visible.
 */
DataModel::EmbeddedCodeEditorItem::EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate gate,
                                                          QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_themeManager(Misc::ThemeManager::instance())
  , m_timerEvents(Misc::TimerEvents::instance())
  , m_editor(*this, m_themeManager, Misc::CommonFonts::instance(), gate)
{
  m_editor.configureHost();

  auto& widget = m_editor.widget();

  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &EmbeddedCodeEditorItem::applyEditorTheme);
  connect(
    &m_timerEvents, &Misc::TimerEvents::uiTimeout, this, &EmbeddedCodeEditorItem::renderWidget);

  connect(this, &QQuickPaintedItem::widthChanged, this, &EmbeddedCodeEditorItem::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this, &EmbeddedCodeEditorItem::resizeWidget);

  connect(&widget, &QCodeEditor::textChanged, this, &EmbeddedCodeEditorItem::scheduleRender);
  connect(&widget, &QCodeEditor::selectionChanged, this, &EmbeddedCodeEditorItem::scheduleRender);
  connect(
    &widget, &QCodeEditor::cursorPositionChanged, this, &EmbeddedCodeEditorItem::scheduleRender);
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the cached pixmap stale; the next UI tick does the grab, so a burst of edits
 *        costs one widget render instead of one per event.
 */
void DataModel::EmbeddedCodeEditorItem::scheduleRender()
{
  m_editor.scheduleRender();
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering, at most once per UI tick and
 *        only when something changed; a focused editor always regrabs so the caret keeps blinking.
 */
void DataModel::EmbeddedCodeEditorItem::renderWidget()
{
  m_editor.renderWidget();
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::EmbeddedCodeEditorItem::resizeWidget()
{
  m_editor.resizeWidget();
}

/**
 * @brief Re-applies the syntax style after a theme change.
 */
void DataModel::EmbeddedCodeEditorItem::applyEditorTheme()
{
  m_editor.applyTheme();
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::EmbeddedCodeEditorItem::paint(QPainter* painter)
{
  m_editor.paint(painter);
}

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...)
 *        are handled natively instead of being consumed by QML Shortcut bindings.
 */
bool DataModel::EmbeddedCodeEditorItem::event(QEvent* event)
{
  if (m_editor.handleShortcutOverride(event))
    return true;

  return QQuickPaintedItem::event(event);
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::EmbeddedCodeEditorItem::keyPressEvent(QKeyEvent* event)
{
  m_editor.handleKeyPress(event);
}

/**
 * @brief Forwards key-release events to the backing QCodeEditor widget.
 */
void DataModel::EmbeddedCodeEditorItem::keyReleaseEvent(QKeyEvent* event)
{
  m_editor.forwardToWidget(event);
}

/**
 * @brief Forwards input-method events (IME composition) to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::inputMethodEvent(QInputMethodEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-in events to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::focusInEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards focus-out events to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::focusOutEvent(QFocusEvent* event)
{
  m_editor.forwardToWidget(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards mouse-press events to the backing widget, claiming focus for the item.
 */
void DataModel::EmbeddedCodeEditorItem::mousePressEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, true);
}

/**
 * @brief Forwards mouse-move events to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::mouseMoveEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards mouse-release events to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::mouseReleaseEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards double-click events to the backing widget.
 */
void DataModel::EmbeddedCodeEditorItem::mouseDoubleClickEvent(QMouseEvent* event)
{
  m_editor.handleMouse(event, false);
}

/**
 * @brief Forwards wheel events to the editor viewport.
 */
void DataModel::EmbeddedCodeEditorItem::wheelEvent(QWheelEvent* event)
{
  m_editor.forwardToViewport(event);
  m_editor.scheduleRender();
}

/**
 * @brief Forwards drag-enter events to the editor viewport.
 */
void DataModel::EmbeddedCodeEditorItem::dragEnterEvent(QDragEnterEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-move events to the editor viewport.
 */
void DataModel::EmbeddedCodeEditorItem::dragMoveEvent(QDragMoveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drag-leave events to the editor viewport.
 */
void DataModel::EmbeddedCodeEditorItem::dragLeaveEvent(QDragLeaveEvent* event)
{
  m_editor.forwardToViewport(event);
}

/**
 * @brief Forwards drop events to the editor viewport.
 */
void DataModel::EmbeddedCodeEditorItem::dropEvent(QDropEvent* event)
{
  m_editor.forwardToViewport(event);
}
