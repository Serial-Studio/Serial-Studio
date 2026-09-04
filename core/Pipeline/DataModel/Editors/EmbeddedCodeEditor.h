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

#pragma once

#include <QCodeEditor>
#include <QPixmap>
#include <QSyntaxStyle>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QQuickPaintedItem;

namespace Misc {
class CommonFonts;
class ThemeManager;
}  // namespace Misc

namespace DataModel {

/**
 * @brief Offscreen QCodeEditor plumbing for a QML-embedded editor item: the hidden top-level
 *        widget and its highlighter/completer pair, the syntax style, the gated per-tick pixmap
 *        grab and the event forwarding. The five embedded editors carried byte-identical copies
 *        of all of it; the host item now owns one of these and forwards.
 */
class EmbeddedCodeEditor {
public:
  /**
   * @brief Which visibility test gates the per-tick grab. A QQuickItem inside a closed window
   *        still reports itself visible, so an editor hosted in a window that can be closed while
   *        its item stays instantiated must test the window too (2026-08-18).
   */
  enum class RenderGate {
    ItemVisible,
    WindowVisible
  };

  EmbeddedCodeEditor(QQuickPaintedItem& host,
                     Misc::ThemeManager& themeManager,
                     Misc::CommonFonts& commonFonts,
                     RenderGate gate);

  EmbeddedCodeEditor(const EmbeddedCodeEditor&)            = delete;
  EmbeddedCodeEditor& operator=(const EmbeddedCodeEditor&) = delete;

  [[nodiscard]] QCodeEditor& widget() noexcept;
  [[nodiscard]] const QCodeEditor& widget() const noexcept;

  [[nodiscard]] QString text() const;
  [[nodiscard]] bool renderable() const;
  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] bool importFromFile(const QString& path);
  [[nodiscard]] bool handleShortcutOverride(QEvent* event);

  void cut();
  void undo();
  void redo();
  void copy();
  void paste();
  void selectAll();
  void markSaved();
  void applyTheme();
  void configureHost();
  void renderWidget();
  void resizeWidget();
  void scheduleRender();
  void setScriptLanguage(bool lua);
  void paint(QPainter* painter) const;
  void forwardToWidget(QEvent* event);
  void handleKeyPress(QKeyEvent* event);
  void forwardToViewport(QEvent* event);
  void setSourceText(const QString& text);
  void syncSourceText(const QString& text);
  void handleMouse(QMouseEvent* event, bool takeFocus);

private:
  void syncWidgetPosition();
  void resetDocumentState();

private:
  bool m_dirty;
  RenderGate m_gate;
  QPixmap m_pixmap;
  QSyntaxStyle m_style;
  QCodeEditor m_widget;
  QQuickPaintedItem& m_host;
  Misc::ThemeManager& m_themeManager;
};

}  // namespace DataModel
