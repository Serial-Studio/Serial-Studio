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
class Widgets : public QObject {
    Q_OBJECT

signals:
    void dataChanged();

public:
    static Widgets* getInstance();

    QList<Group*> barGroups() const;
    QList<Group*> mapGroups() const;
    QList<Group*> gyroGroups() const;
    QList<Group*> tankGroups() const;
    QList<Group*> gaugeGroups() const;
    QList<Group*> accelerometerGroups() const;

    Group* barGroupAt(const int index);
    Group* mapGroupAt(const int index);
    Group* gyroGroupAt(const int index);
    Group* tankGroupAt(const int index);
    Group* gaugeGroupAt(const int index);
    Group* accelerometerGroupAt(const int index);

    qreal gyroX(const int index) const;
    qreal gyroY(const int index) const;
    qreal gyroZ(const int index) const;

    qreal accelerometerX(const int index) const;
    qreal accelerometerY(const int index) const;
    qreal accelerometerZ(const int index) const;

    qreal bar(const int index) const;
    qreal tank(const int index) const;
    qreal gauge(const int index) const;
    qreal barMin(const int index) const;
    qreal barMax(const int index) const;
    qreal tankMin(const int index) const;
    qreal tankMax(const int index) const;
    qreal gaugeMin(const int index) const;
    qreal gaugeMax(const int index) const;

    qreal mapLatitude(const int index) const;
    qreal mapLongitude(const int index) const;

private slots:
    void updateModels();

private:
    QList<Group*> getWidgetGroup(const QString& handle);

private:
    QList<Group*> m_barGroups;
    QList<Group*> m_mapGroups;
    QList<Group*> m_gyroGroups;
    QList<Group*> m_tankGroups;
    QList<Group*> m_gaugeGroups;
    QList<Group*> m_accelerometerGroups;
};

#endif
