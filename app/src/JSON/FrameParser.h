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

#include <QJSValue>
#include <QJSEngine>

#include <qsourcehighliter.h>

#include <UI/DeclarativeWidget.h>

namespace JSON
{
class FrameParser : public UI::DeclarativeWidget
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
  void execEvent(void *function, void *event);

private slots:
  void readCode();

private:
  QJSEngine m_engine;
  QJSValue m_parseFunction;
  QPlainTextEdit m_textEdit;
  QSourceHighlite::QSourceHighliter *m_highlighter;
};
} // namespace JSON
