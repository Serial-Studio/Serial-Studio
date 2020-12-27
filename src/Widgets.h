/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef WIDGETS_H
#define WIDGETS_H

#include <QList>
#include <QObject>

class Group;
class Dataset;

class Widgets : public QObject
{
   Q_OBJECT

signals:
   void dataChanged();

public:
   static Widgets *getInstance();

   QList<Group *> mapGroup() const;
   QList<Group *> gyroGroup() const;
   QList<Dataset *> barDatasets() const;
   QList<Group *> accelerometerGroup() const;

   Q_INVOKABLE int mapGroupCount() const;
   Q_INVOKABLE int gyroGroupCount() const;
   Q_INVOKABLE int barDatasetCount() const;
   Q_INVOKABLE int accelerometerGroupCount() const;

   Q_INVOKABLE Group *mapGroupAt(const int index);
   Q_INVOKABLE Group *gyroGroupAt(const int index);
   Q_INVOKABLE Dataset *barDatasetAt(const int index);
   Q_INVOKABLE Group *accelerometerGroupAt(const int index);

   Q_INVOKABLE double gyroX(const int index);
   Q_INVOKABLE double gyroY(const int index);
   Q_INVOKABLE double gyroZ(const int index);

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
   Widgets();
   QList<Group *> getWidgetGroup(const QString &handle);
   QList<Dataset *> getWidgetDatasets(const QString &handle);

private:
   QList<Group *> m_mapGroups;
   QList<Group *> m_gyroGroups;
   QList<Dataset *> m_barDatasets;
   QList<Group *> m_accelerometerGroups;
};

#endif
