/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QCodeEditor>
#  include <QPainter>
#  include <QQuickPaintedItem>
#  include <QSyntaxStyle>

namespace DataModel {

/**
 * @brief QML-embeddable code editor for painter-widget paint() / onFrame() scripts.
 */
class PainterCodeEditor : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(bool isModified READ isModified NOTIFY modifiedChanged)
  Q_PROPERTY(bool undoAvailable READ undoAvailable NOTIFY modifiedChanged)
  Q_PROPERTY(bool redoAvailable READ redoAvailable NOTIFY modifiedChanged)
  Q_PROPERTY(QString text READ text NOTIFY textChanged)

signals:
  void textChanged();
  void modifiedChanged();

public:
  explicit PainterCodeEditor(QQuickItem* parent = nullptr);

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
  void commit();
  void import();
  void selectAll();
  void readCode();
  void formatDocument();
  void formatSelection();
  void selectTemplate();
  void reload(bool guiTrigger = false);

private slots:
  void onThemeChanged();
  void renderWidget();
  void resizeWidget();

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

public:
  [[nodiscard]] static QString defaultTemplate();

private:
  void loadTemplates();
  bool m_readingCode;
  QPixmap m_pixmap;
  QSyntaxStyle m_style;
  QCodeEditor m_widget;

  QString m_defaultTemplateFile;
  QStringList m_templateNames;
  QStringList m_templateFiles;
};

}  // namespace DataModel

#endif  // BUILD_COMMERCIAL
