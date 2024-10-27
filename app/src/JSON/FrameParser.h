/*
 * Copyright (c) 2022-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QEvent>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QQuickPaintedItem>

#include <QJSValue>
#include <QJSEngine>

#include <qsourcehighliter.h>

namespace JSON
{
class FrameParser : public QQuickPaintedItem
{
  Q_OBJECT
  Q_PROPERTY(QString text READ text NOTIFY textChanged)
  Q_PROPERTY(bool isModified READ isModified NOTIFY modifiedChanged)

signals:
  void textChanged();
  void modifiedChanged();

public:
  FrameParser(QQuickItem *parent = 0);

  static const QString &defaultCode();

  [[nodiscard]] QString text() const;
  [[nodiscard]] bool isModified() const;
  [[nodiscard]] QStringList parse(const QString &frame,
                                  const QString &separator);

public slots:
  void cut();
  void undo();
  void redo();
  void help();
  void copy();
  void paste();
  void apply();
  void reload();
  void import();
  void selectAll();

private slots:
  void onThemeChanged();

private:
  bool save(const bool silent = false);
  bool loadScript(const QString &script);

private slots:
  void readCode();
  void renderWidget();
  void resizeWidget();

private:
  virtual void paint(QPainter *painter) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;
  virtual void inputMethodEvent(QInputMethodEvent *event) override;
  virtual void focusInEvent(QFocusEvent *event) override;
  virtual void focusOutEvent(QFocusEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
  virtual void wheelEvent(QWheelEvent *event) override;
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dragMoveEvent(QDragMoveEvent *event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;

private:
  QPixmap m_pixmap;
  QJSEngine m_engine;
  QJSValue m_parseFunction;
  QPlainTextEdit m_textEdit;
  QSourceHighlite::QSourceHighliter *m_highlighter;
};
} // namespace JSON
