/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef WIDGETS_COMMON_BASE_H
#define WIDGETS_COMMON_BASE_H

#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

namespace Widgets
{
class BaseWidget : public QWidget
{
    Q_OBJECT

signals:
    void resized();

public:
    BaseWidget();

    void setValue(const QString &label);
    void setWidget(QWidget *widget, Qt::Alignment alignment = Qt::AlignHCenter,
                   bool autoresize = true);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    int m_index;
    QLabel m_label;
    QWidget *m_widget;
    bool m_resizeWidget;
    QHBoxLayout m_layout;
    int m_labelTextWidth;
};
}

#endif
