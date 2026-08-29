/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <QPainter>
#include <QQuickPaintedItem>

#include "DataModel/Dialogs/TransmitTestDialog.h"
#include "DataModel/Editors/EmbeddedCodeEditor.h"
#include "DataModel/Editors/ScriptTemplateCatalog.h"
#include "DataModel/Frame.h"

namespace Misc {
class ThemeManager;
class TimerEvents;
class Translator;
}  // namespace Misc

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief QML-embeddable code editor for output-widget transmit scripts.
 */
class OutputCodeEditor : public QQuickPaintedItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isModified
             READ isModified
             NOTIFY modifiedChanged)
  Q_PROPERTY(bool undoAvailable
             READ undoAvailable
             NOTIFY modifiedChanged)
  Q_PROPERTY(bool redoAvailable
             READ redoAvailable
             NOTIFY modifiedChanged)
  Q_PROPERTY(QString text
             READ text
             NOTIFY textChanged)
  // clang-format on

signals:
  void textChanged();
  void modifiedChanged();

public:
  explicit OutputCodeEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] QString text() const;

public slots:
  void cut();
  void undo();
  void redo();
  void copy();
  void paste();
  void importFile();
  void selectAll();
  void readCode();
  void formatDocument();
  void formatSelection();
  void selectTemplate();
  void testTransmitFunction();
  void reload(bool guiTrigger = false);

private slots:
  void onThemeChanged();
  void renderWidget();
  void resizeWidget();
  void scheduleRender();

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

public:
  [[nodiscard]] static QString defaultTemplate();

private:
  void loadTemplates();

private:
  bool m_readingCode;
  Misc::ThemeManager& m_themeManager;
  Misc::TimerEvents& m_timerEvents;
  Misc::Translator& m_translator;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::ProjectModel& m_projectModel;
  EmbeddedCodeEditor m_editor;
  ScriptTemplateCatalog m_templates;
  TransmitTestDialog m_testDialog;
};

}  // namespace DataModel
