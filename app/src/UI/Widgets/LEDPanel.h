/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <UI/Widgets/Common/KLed.h>

namespace Widgets
{
class LEDPanel : public QWidget
{
  Q_OBJECT

public:
  LEDPanel(const int index = -1);
  ~LEDPanel();

private slots:
  void updateData();
  void onThemeChanged();

protected:
  void resizeEvent(QResizeEvent *event);

private:
  int m_index;

  QVector<KLed *> m_leds;
  QVector<QLabel *> m_titles;

  QWidget *m_dataContainer;
  QVBoxLayout *m_mainLayout;
  QGridLayout *m_gridLayout;
  QScrollArea *m_scrollArea;

  QTimer m_blinkerTimer;
};
} // namespace Widgets
