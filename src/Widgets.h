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
class Widgets : public QObject
{
   Q_OBJECT

signals:
   void dataChanged();

public:
   static Widgets *getInstance();

   QList<Group *> barGroup() const;
   QList<Group *> mapGroup() const;
   QList<Group *> gyroGroup() const;
   QList<Group *> tankGroup() const;
   QList<Group *> gaugeGroup() const;
   QList<Group *> accelerometerGroup() const;

   int barGroupCount() const;
   int mapGroupCount() const;
   int gyroGroupCount() const;
   int tankGroupCount() const;
   int gaugeGroupCount() const;
   int accelerometerGroupCount() const;

   Group *barGroupAt(const int index);
   Group *mapGroupAt(const int index);
   Group *gyroGroupAt(const int index);
   Group *tankGroupAt(const int index);
   Group *gaugeGroupAt(const int index);
   Group *accelerometerGroupAt(const int index);

   double gyroX(const int index);
   double gyroY(const int index);
   double gyroZ(const int index);

   double accelerometerX(const int index);
   double accelerometerY(const int index);
   double accelerometerZ(const int index);

   double bar(const int index);
   double tank(const int index);
   double gauge(const int index);
   double barMin(const int index);
   double barMax(const int index);
   double tankMin(const int index);
   double tankMax(const int index);
   double gaugeMin(const int index);
   double gaugeMax(const int index);

   double mapLatitude(const int index);
   double mapLongitude(const int index);

private slots:
   void updateModels();

private:
   Widgets();
   QList<Group *> getWidgetGroup(const QString &handle);

private:
   QList<Group *> m_barGroups;
   QList<Group *> m_mapGroups;
   QList<Group *> m_gyroGroups;
   QList<Group *> m_tankGroups;
   QList<Group *> m_gaugeGroups;
   QList<Group *> m_accelerometerGroups;
};

#endif
