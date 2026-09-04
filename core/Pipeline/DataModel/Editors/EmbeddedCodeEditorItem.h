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

#include <QQuickPaintedItem>

#include "DataModel/Editors/EmbeddedCodeEditor.h"

namespace Misc {
class ThemeManager;
class TimerEvents;
}  // namespace Misc

namespace DataModel {

/**
 * @brief QQuickPaintedItem base for every QML-embedded code editor: it owns the offscreen
 *        QCodeEditor, the gated per-tick grab, the theme hook and the sixteen event overrides
 *        that forward into it. The five hosts carried byte-identical copies, so the
 *        hidden-widget rules could drift apart one host at a time; here they cannot.
 */
class EmbeddedCodeEditorItem : public QQuickPaintedItem {
  Q_OBJECT

public:
  explicit EmbeddedCodeEditorItem(EmbeddedCodeEditor::RenderGate gate, QQuickItem* parent);
  EmbeddedCodeEditorItem(EmbeddedCodeEditorItem&&)                 = delete;
  EmbeddedCodeEditorItem(const EmbeddedCodeEditorItem&)            = delete;
  EmbeddedCodeEditorItem& operator=(EmbeddedCodeEditorItem&&)      = delete;
  EmbeddedCodeEditorItem& operator=(const EmbeddedCodeEditorItem&) = delete;

protected slots:
  void renderWidget();
  void resizeWidget();
  void scheduleRender();
  void applyEditorTheme();

protected:
  [[nodiscard]] bool event(QEvent* event) override;
  void paint(QPainter* painter) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void inputMethodEvent(QInputMethodEvent* event) override;
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

protected:
  Misc::ThemeManager& m_themeManager;
  Misc::TimerEvents& m_timerEvents;
  EmbeddedCodeEditor m_editor;
};

}  // namespace DataModel
