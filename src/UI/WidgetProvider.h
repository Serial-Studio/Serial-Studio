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

#ifndef WIDGETPROVIDER_H
#define WIDGETPROVIDER_H

#include <QList>
#include <QObject>
#include <JSON/Frame.h>

namespace UI
{

class WidgetProvider : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int totalWidgetCount
               READ totalWidgetCount
               NOTIFY dataChanged)
    // clang-format on

signals:
    void dataChanged();

public:
    static WidgetProvider *getInstance();

    QList<JSON::Group *> mapGroup() const;
    QList<JSON::Group *> gyroGroup() const;
    QList<JSON::Dataset *> barDatasets() const;
    QList<JSON::Group *> accelerometerGroup() const;

    int totalWidgetCount() const;

    Q_INVOKABLE int mapGroupCount() const;
    Q_INVOKABLE int gyroGroupCount() const;
    Q_INVOKABLE int barDatasetCount() const;
    Q_INVOKABLE int accelerometerGroupCount() const;

    Q_INVOKABLE JSON::Group *mapGroupAt(const int index);
    Q_INVOKABLE JSON::Group *gyroGroupAt(const int index);
    Q_INVOKABLE JSON::Dataset *barDatasetAt(const int index);
    Q_INVOKABLE JSON::Group *accelerometerGroupAt(const int index);

    Q_INVOKABLE double gyroYaw(const int index);
    Q_INVOKABLE double gyroRoll(const int index);
    Q_INVOKABLE double gyroPitch(const int index);

    Q_INVOKABLE double accelerometerX(const int index);
    Q_INVOKABLE double accelerometerY(const int index);
    Q_INVOKABLE double accelerometerZ(const int index);

    Q_INVOKABLE double bar(const int index);
    Q_INVOKABLE double barMin(const int index);
    Q_INVOKABLE double barMax(const int index);

    Q_INVOKABLE double mapLatitude(const int index);
    Q_INVOKABLE double mapLongitude(const int index);

private slots:
    void updateModels();

private:
    WidgetProvider();
    QList<JSON::Group *> getWidgetGroup(const QString &handle);
    QList<JSON::Dataset *> getWidgetDatasets(const QString &handle);

private:
    QList<JSON::Group *> m_mapGroups;
    QList<JSON::Group *> m_gyroGroups;
    QList<JSON::Dataset *> m_barDatasets;
    QList<JSON::Group *> m_accelerometerGroups;
};
}

#endif
