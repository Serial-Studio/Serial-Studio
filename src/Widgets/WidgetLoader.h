/*
 * Copyright (c) 2021 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef WIDGETS_LOADER_H
#define WIDGETS_LOADER_H

#include <QWidget>
#include <QObject>
#include <QPainter>
#include <QMainWindow>
#include <QQuickPaintedItem>

#include <UI/Dashboard.h>

namespace Widgets
{
class WidgetWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void visibleChanged();

private:
    void showEvent(QShowEvent *event)
    {
        event->accept();
        emit visibleChanged();
    }

    void hideEvent(QHideEvent *event)
    {
        event->accept();
        emit visibleChanged();
    }
};

class WidgetLoader : public QQuickPaintedItem
{
    // clang-format off
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int widgetIndex
               READ widgetIndex
               WRITE setWidgetIndex
               NOTIFY widgetIndexChanged)
    Q_PROPERTY(int relativeIndex
               READ relativeIndex
               NOTIFY widgetIndexChanged)
    Q_PROPERTY(QString widgetIcon
               READ widgetIcon
               NOTIFY widgetIndexChanged)
    Q_PROPERTY(QString widgetTitle
               READ widgetTitle
               NOTIFY widgetIndexChanged)
    Q_PROPERTY(bool widgetVisible
               READ widgetVisible
               NOTIFY widgetVisibleChanged)
    // clang-format on

signals:
    void widgetIndexChanged();
    void widgetVisibleChanged();

public:
    WidgetLoader(QQuickItem *parent = 0);
    ~WidgetLoader();

    virtual bool event(QEvent *event) override;
    virtual void paint(QPainter *painter) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    int widgetIndex() const;
    int relativeIndex() const;
    bool widgetVisible() const;
    QString widgetIcon() const;
    QString widgetTitle() const;
    UI::Dashboard::WidgetType widgetType() const;

public slots:
    void displayWindow();
    void setWidgetIndex(const int index);

private slots:
    void updateWidgetSize();
    void updateWidgetWindow();
    void updateWidgetVisible();

protected:
    void processMouseEvents(QMouseEvent *event);
    void processWheelEvents(QWheelEvent *event);

private:
    int m_index;
    QWidget *m_widget;
    WidgetWindow m_window;
    bool m_widgetVisible;
};
}

#endif
