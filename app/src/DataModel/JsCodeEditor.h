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

#pragma once

#include <QCodeEditor>
#include <QEvent>
#include <QPainter>
#include <QQuickPaintedItem>
#include <QSyntaxStyle>

#include "DataModel/FrameParserTestDialog.h"

namespace DataModel {

/**
 * @brief QML-embeddable code editor for the JavaScript / Lua frame parser.
 */
class JsCodeEditor : public QQuickPaintedItem {
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
  Q_PROPERTY(int sourceId
             READ  sourceId
             WRITE setSourceId
             NOTIFY sourceIdChanged)
  Q_PROPERTY(int language
             READ  language
             WRITE setLanguage
             NOTIFY languageChanged)
  // clang-format on

signals:
  void textChanged();
  void modifiedChanged();
  void sourceIdChanged();
  void languageChanged();

public:
  explicit JsCodeEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] int language() const noexcept;
  [[nodiscard]] QString text() const;

public slots:
  void cut();
  void undo();
  void redo();
  void help();
  void copy();
  void paste();
  void apply();
  void import();
  void evaluate();
  void readCode();
  void selectAll();
  void selectTemplate();
  void testWithSampleData();
  void setSourceId(const int sourceId);
  void setLanguage(const int language);
  void switchLanguage(const int language);
  void reload(const bool guiTrigger = false);
  void loadDefaultTemplate(const bool guiTrigger = false);

private slots:
  void onThemeChanged();
  void renderWidget();
  void resizeWidget();

private:
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
  int m_sourceId;
  int m_language;
  bool m_readingCode;
  QPixmap m_pixmap;
  QSyntaxStyle m_style;
  QCodeEditor m_widget;
  FrameParserTestDialog m_testDialog;
};

}  // namespace DataModel
