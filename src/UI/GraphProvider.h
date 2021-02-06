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

#ifndef GRAPH_PROVIDER_H
#define GRAPH_PROVIDER_H

#include <QVector>
#include <QObject>
#include <QVector>
#include <QVariant>
#include <QAbstractSeries>

#include <JSON/Dataset.h>

QT_CHARTS_USE_NAMESPACE

namespace UI
{
class GraphProvider : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(int graphCount
               READ graphCount
               NOTIFY dataUpdated)
    Q_PROPERTY(QVector<JSON::Dataset *> datasets
               READ datasets
               NOTIFY dataUpdated)
    Q_PROPERTY(int displayedPoints
               READ displayedPoints
               WRITE setDisplayedPoints
               NOTIFY displayedPointsUpdated)
    // clang-format on

signals:
    void dataUpdated();
    void displayedPointsUpdated();

public:
    static GraphProvider *getInstance();

    int graphCount() const;
    int displayedPoints() const;
    QVector<JSON::Dataset *> datasets() const;
    Q_INVOKABLE double getValue(const int index) const;
    Q_INVOKABLE double minimumValue(const int index) const;
    Q_INVOKABLE double maximumValue(const int index) const;
    Q_INVOKABLE JSON::Dataset *getDataset(const int index) const;

public slots:
    void setDisplayedPoints(const int points);
    void updateGraph(QAbstractSeries *series, const int index);

private:
    GraphProvider();

private slots:
    void resetData();
    void updateValues();
    void csvPlayerFixes();

private:
    int m_prevFramePos;
    int m_displayedPoints;
    QVector<double> m_maximumValues;
    QVector<double> m_minimumValues;
    QVector<QVector<double> *> m_points;
    QVector<JSON::Dataset *> m_datasets;
};
}

#endif
