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

#pragma once

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <mapcontrol.h>
#include <UI/DashboardWidget.h>

class QNetworkReply;

namespace Widgets
{
class GPS : public DashboardWidgetBase
{
    Q_OBJECT

public:
    GPS(const int index = -1);

private Q_SLOTS:
    void updateData();

protected:
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    int m_index;
    QLabel *m_label;
    QVBoxLayout m_layout;
    QPushButton *m_zoomIn;
    QPushButton *m_zoomOut;
    QWidget m_titleWidget;
    QHBoxLayout m_titleLayout;
    qmapcontrol::MapControl m_mapControl;
};
}
