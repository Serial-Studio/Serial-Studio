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

#include "DataModel/Editors/EmbeddedCodeEditor.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJavascriptHighlighter>
#include <QKeyEvent>
#include <QLineNumberArea>
#include <QLuaHighlighter>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QTextDocument>

#include "DataModel/Editors/SerialStudioCompleter.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the hidden editor widget for a QML host item. The host is only referenced here;
 *        every call into it happens later, so this is safe to run from a host member-init list.
 */
DataModel::EmbeddedCodeEditor::EmbeddedCodeEditor(QQuickPaintedItem& host,
                                                  Misc::ThemeManager& themeManager,
                                                  Misc::CommonFonts& commonFonts,
                                                  RenderGate gate)
  : m_dirty(true), m_gate(gate), m_host(host), m_themeManager(themeManager)
{
  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(2);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(commonFonts.monoFont());
  m_widget.setLayoutDirection(Qt::LeftToRight);
  m_widget.setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
  m_widget.setCompleter(new DataModel::SerialStudioCompleter(false, &m_widget));

  applyTheme();
}

/**
 * @brief Applies the QQuickPaintedItem flags every embedded editor needs. Called from the host
 *        constructor body, never from its member-init list.
 */
void DataModel::EmbeddedCodeEditor::configureHost()
{
  m_host.setMipmap(false);
  m_host.setAntialiasing(false);
  m_host.setOpaquePainting(true);
  m_host.setAcceptTouchEvents(true);
  m_host.setFlag(QQuickItem::ItemHasContents, true);
  m_host.setFlag(QQuickItem::ItemIsFocusScope, true);
  m_host.setFlag(QQuickItem::ItemAcceptsInputMethod, true);
  m_host.setAcceptedMouseButtons(Qt::AllButtons);
  m_host.setFillColor(m_themeManager.getColor(QStringLiteral("base")));
}

//--------------------------------------------------------------------------------------------------
// Document access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the backing editor widget.
 */
QCodeEditor& DataModel::EmbeddedCodeEditor::widget() noexcept
{
  return m_widget;
}

/**
 * @brief Returns the backing editor widget.
 */
const QCodeEditor& DataModel::EmbeddedCodeEditor::widget() const noexcept
{
  return m_widget;
}

/**
 * @brief Returns the editor's current text.
 */
QString DataModel::EmbeddedCodeEditor::text() const
{
  return m_widget.toPlainText();
}

/**
 * @brief Returns true when the editor document has unsaved edits.
 */
bool DataModel::EmbeddedCodeEditor::isModified() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/**
 * @brief Returns true when an undo step is available in the editor.
 */
bool DataModel::EmbeddedCodeEditor::undoAvailable() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isUndoAvailable();

  return false;
}

/**
 * @brief Returns true when a redo step is available in the editor.
 */
bool DataModel::EmbeddedCodeEditor::redoAvailable() const noexcept
{
  if (m_widget.document())
    return m_widget.document()->isRedoAvailable();

  return false;
}

/**
 * @brief Drops the undo history and clears the modified flag after a programmatic load.
 */
void DataModel::EmbeddedCodeEditor::resetDocumentState()
{
  SS_ASSERT(m_widget.document(), return);

  m_widget.document()->clearUndoRedoStacks();
  m_widget.document()->setModified(false);
}

/**
 * @brief Replaces the document with text and resets its undo history and modified flag.
 */
void DataModel::EmbeddedCodeEditor::setSourceText(const QString& text)
{
  m_widget.setPlainText(text);
  resetDocumentState();
}

/**
 * @brief Same as setSourceText(), but leaves the document untouched when the text already
 *        matches: the reload path uses it to avoid a redundant textChanged round trip.
 */
void DataModel::EmbeddedCodeEditor::syncSourceText(const QString& text)
{
  if (m_widget.toPlainText() != text)
    m_widget.setPlainText(text);

  resetDocumentState();
}

/**
 * @brief Marks the current content as saved without touching the undo history.
 */
void DataModel::EmbeddedCodeEditor::markSaved()
{
  SS_ASSERT(m_widget.document(), return);
  m_widget.document()->setModified(false);
}

/**
 * @brief Loads an external file into the editor; false when the file cannot be read.
 */
bool DataModel::EmbeddedCodeEditor::importFromFile(const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return false;

  m_widget.setPlainText(QString::fromUtf8(file.readAll()));
  file.close();
  return true;
}

//--------------------------------------------------------------------------------------------------
// Language and theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Swaps the highlighter/completer pair for the requested language; the editor widget
 *        replaces both without taking ownership, so the old pair goes through deleteLater.
 */
void DataModel::EmbeddedCodeEditor::setScriptLanguage(bool lua)
{
  auto* old_completer   = m_widget.completer();
  auto* old_highlighter = m_widget.highlighter();

  if (lua) {
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
}

/**
 * @brief Applies the active theme's code-editor colour scheme to the widget.
 */
void DataModel::EmbeddedCodeEditor::applyTheme()
{
  const auto name =
    m_themeManager.parameters().value(QStringLiteral("code-editor-theme")).toString();

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
// Clipboard and history
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cuts the current selection into the clipboard.
 */
void DataModel::EmbeddedCodeEditor::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last edit.
 */
void DataModel::EmbeddedCodeEditor::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes the previously undone edit.
 */
void DataModel::EmbeddedCodeEditor::redo()
{
  m_widget.redo();
}

/**
 * @brief Copies the current selection to the clipboard.
 */
void DataModel::EmbeddedCodeEditor::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes the clipboard contents into the editor.
 */
void DataModel::EmbeddedCodeEditor::paste()
{
  m_widget.paste();
}

/**
 * @brief Selects all editor text.
 */
void DataModel::EmbeddedCodeEditor::selectAll()
{
  m_widget.selectAll();
}

//--------------------------------------------------------------------------------------------------
// Rendering
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marks the cached pixmap stale; the next UI tick does the grab, so a burst of edits
 *        costs one widget render instead of one per event.
 */
void DataModel::EmbeddedCodeEditor::scheduleRender()
{
  m_dirty = true;
}

/**
 * @brief Whether a grab would be seen. Under RenderGate::WindowVisible the window is tested too:
 *        an item in a closed window still reports itself visible, so a project-editor tab left
 *        selected would otherwise keep rendering at UI rate behind a hidden window.
 */
bool DataModel::EmbeddedCodeEditor::renderable() const
{
  if (m_gate == RenderGate::ItemVisible)
    return m_host.isVisible();

  return m_host.isVisible() && m_host.window() && m_host.window()->isVisible();
}

/**
 * @brief Grabs the editor widget into a pixmap for QML rendering, at most once per UI tick and
 *        only when something changed; a focused editor always regrabs so the caret keeps blinking.
 */
void DataModel::EmbeddedCodeEditor::renderWidget()
{
  if (!renderable() || (!m_dirty && !m_host.hasActiveFocus()))
    return;

  m_dirty = false;
  syncWidgetPosition();
  m_pixmap = m_widget.grab();
  m_host.update();
}

/**
 * @brief Aligns the hidden widget's top-level position with the item's on-screen position so
 *        completer popups and drag auto-scroll resolve correct global coordinates.
 */
void DataModel::EmbeddedCodeEditor::syncWidgetPosition()
{
  if (!m_host.window())
    return;

  const QPoint global = m_host.mapToGlobal(QPointF(0, 0)).toPoint();
  if (m_widget.pos() != global)
    m_widget.move(global);
}

/**
 * @brief Resizes the backing QCodeEditor to match the QML item dimensions.
 */
void DataModel::EmbeddedCodeEditor::resizeWidget()
{
  if (m_host.width() > 0 && m_host.height() > 0) {
    m_widget.setFixedSize(static_cast<int>(m_host.width()), static_cast<int>(m_host.height()));
    scheduleRender();
  }
}

/**
 * @brief Paints the cached editor pixmap into the QML scene.
 */
void DataModel::EmbeddedCodeEditor::paint(QPainter* painter) const
{
  if (painter && m_host.isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

//--------------------------------------------------------------------------------------------------
// Event forwarding
//--------------------------------------------------------------------------------------------------

/**
 * @brief Routes ShortcutOverride to the editor widget so editing keys (undo, copy, paste...) are
 *        handled natively instead of being consumed by QML Shortcut bindings; true when consumed.
 */
bool DataModel::EmbeddedCodeEditor::handleShortcutOverride(QEvent* event)
{
  SS_ASSERT(event, return false);

  if (event->type() != QEvent::ShortcutOverride)
    return false;

  QCoreApplication::sendEvent(&m_widget, event);
  return event->isAccepted();
}

/**
 * @brief Forwards completer navigation/commit keys to the popup when visible; everything else
 *        goes straight to the editor widget so QCompleter's focus check cannot hide the popup.
 */
void DataModel::EmbeddedCodeEditor::handleKeyPress(QKeyEvent* event)
{
  SS_ASSERT(event, return);

  auto* completer = m_widget.completer();
  if (completer && completer->popup() && completer->popup()->isVisible()
      && SerialStudioCompleter::popupHandlesKey(event->key()))
    QCoreApplication::sendEvent(completer->popup(), event);
  else
    QCoreApplication::sendEvent(&m_widget, event);

  scheduleRender();
}

/**
 * @brief Forwards an event to the backing QCodeEditor widget.
 */
void DataModel::EmbeddedCodeEditor::forwardToWidget(QEvent* event)
{
  SS_ASSERT(event, return);
  QCoreApplication::sendEvent(&m_widget, event);
}

/**
 * @brief Forwards an event to the backing widget's viewport.
 */
void DataModel::EmbeddedCodeEditor::forwardToViewport(QEvent* event)
{
  SS_ASSERT(event, return);
  QCoreApplication::sendEvent(m_widget.viewport(), event);
}

/**
 * @brief Forwards a mouse event to the viewport after offsetting for the line-number gutter;
 *        takeFocus claims focus for the host item, which only the press handler does.
 */
void DataModel::EmbeddedCodeEditor::handleMouse(QMouseEvent* event, bool takeFocus)
{
  SS_ASSERT(event, return);

  const auto lineNumWidth = m_widget.lineNumberArea()->sizeHint().width();
  QMouseEvent copy(event->type(),
                   event->position() - QPointF(lineNumWidth, 0),
                   event->globalPosition(),
                   event->button(),
                   event->buttons(),
                   event->modifiers(),
                   event->pointingDevice());
  QCoreApplication::sendEvent(m_widget.viewport(), &copy);

  if (takeFocus)
    m_host.forceActiveFocus();

  scheduleRender();
}
