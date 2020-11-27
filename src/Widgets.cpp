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

#include "Group.h"
#include "Widgets.h"
#include "QmlBridge.h"

static Widgets* INSTANCE = Q_NULLPTR;

Widgets* Widgets::getInstance() {
    if (!INSTANCE)
        INSTANCE = new Widgets;

    return INSTANCE;
}

QList<Group*> Widgets::barGroups() const {
    return m_barGroups;
}

QList<Group*> Widgets::mapGroups() const {
    return m_mapGroups;
}

QList<Group*> Widgets::gyroGroups() const {
    return m_gyroGroups;
}

QList<Group*> Widgets::tankGroups() const {
    return m_tankGroups;
}

QList<Group*> Widgets::gaugeGroups() const {
    return m_gaugeGroups;
}

QList<Group*> Widgets::accelerometerGroups() const {
    return m_accelerometerGroups;
}

Group* Widgets::barGroupAt(const int index) {
    if (barGroups().count() > index)
        return barGroups().at(index);

    return Q_NULLPTR;
}

Group* Widgets::mapGroupAt(const int index) {
    if (mapGroups().count() > index)
        return mapGroups().at(index);

    return Q_NULLPTR;
}

Group* Widgets::gyroGroupAt(const int index) {
    if (gyroGroups().count() > index)
        return gyroGroups().at(index);

    return Q_NULLPTR;
}

Group* Widgets::tankGroupAt(const int index) {
    if (tankGroups().count() > index)
        return tankGroups().at(index);

    return Q_NULLPTR;
}

Group* Widgets::gaugeGroupAt(const int index) {
    if (gaugeGroups().count() > index)
        return gaugeGroups().at(index);

    return Q_NULLPTR;
}

Group* Widgets::accelerometerGroupAt(const int index) {
    if (accelerometerGroups().count() > index)
        return accelerometerGroups().at(index);

    return Q_NULLPTR;
}

qreal Widgets::gyroX(const int index) const {}

qreal Widgets::gyroY(const int index) const {}

qreal Widgets::gyroZ(const int index) const {}

qreal Widgets::accelerometerX(const int index) const {}

qreal Widgets::accelerometerY(const int index) const {}

qreal Widgets::accelerometerZ(const int index) const {}

qreal Widgets::bar(const int index) const {}

qreal Widgets::tank(const int index) const {}

qreal Widgets::gauge(const int index) const {}

qreal Widgets::barMin(const int index) const {}

qreal Widgets::barMax(const int index) const {}

qreal Widgets::tankMin(const int index) const {}

qreal Widgets::tankMax(const int index) const {}

qreal Widgets::gaugeMin(const int index) const {}

qreal Widgets::gaugeMax(const int index) const {}

qreal Widgets::mapLatitude(const int index) const {}

qreal Widgets::mapLongitude(const int index) const {}

void Widgets::updateModels() {}

QList<Group*> Widgets::getWidgetGroup(const QString& handle) {

}
