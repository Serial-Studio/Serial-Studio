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
#include <QPainter>
#include <QQuickPaintedItem>
#include <QSyntaxStyle>

namespace Misc {
class ThemeManager;
class CommonFonts;
class TimerEvents;
}  // namespace Misc

namespace DataModel {

/**
 * @brief QML-embeddable code editor for API Terminal macros: project-independent sibling of
 *        ControlScriptEditor with a switchable JS/Lua highlighter (spec 0046).
 */
class MacroEditor : public QQuickPaintedItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int language
             READ language
             WRITE setLanguage
             NOTIFY languageChanged)
  Q_PROPERTY(bool isModified
             READ isModified
             NOTIFY modifiedChanged)
  Q_PROPERTY(QString text
             READ text
             WRITE setText
             NOTIFY textChanged)
  // clang-format on

signals:
  void textChanged();
  void languageChanged();
  void modifiedChanged();

public:
  explicit MacroEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] QString text() const;
  [[nodiscard]] int language() const noexcept;
  [[nodiscard]] bool isModified() const noexcept;

public slots:
  void cut();
  void undo();
  void redo();
  void copy();
  void clear();
  void paste();
  void selectAll();
  void setLanguage(int language);
  void setText(const QString& text);
  void markSaved();

private slots:
  void renderWidget();
  void onThemeChanged();
  void resizeWidget();
  void scheduleRender();

private:
  void syncWidgetPosition();

private:
  bool event(QEvent* event) override;
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

private:
  bool m_dirty;
  int m_language;
  Misc::ThemeManager& m_themeManager;
  Misc::CommonFonts& m_commonFonts;
  Misc::TimerEvents& m_timerEvents;
  QPixmap m_pixmap;
  QSyntaxStyle m_style;
  QCodeEditor m_widget;
};

}  // namespace DataModel
