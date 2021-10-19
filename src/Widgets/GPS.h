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

#ifndef WIDGETS_GPS_H
#define WIDGETS_GPS_H

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QNetworkAccessManager>

class QNetworkReply;

namespace Widgets
{
class GPS : public QWidget
{
    Q_OBJECT

public:
    GPS(const int index = -1);
    ~GPS();

private slots:
    void openMap();
    void queryCity();
    void updateData();
    void processGeoResponse(QNetworkReply *reply);

protected:
    void leaveEvent(QEvent *event);
    void enterEvent(QEnterEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    int m_index;
    qreal m_lat;
    qreal m_lon;
    qreal m_alt;
    QLabel m_mapLabel;
    QLabel m_posLabel;
    QVBoxLayout m_layout;
    QWidget *m_dataContainer;
    QGridLayout *m_gridLayout;
    QVector<QLabel *> m_icons;
    QVector<QLabel *> m_units;
    QVector<QLabel *> m_titles;
    QVector<QLabel *> m_values;

    QElapsedTimer m_networkThrottle;
    QNetworkAccessManager m_manager;
};
}

#endif
