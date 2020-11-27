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

#ifndef GROUP_H
#define GROUP_H

#include <QList>
#include <QObject>
#include <QVariant>
#include <QJsonArray>
#include <QJsonObject>

class Dataset;
class Group : public QObject
{
   Q_OBJECT

   Q_PROPERTY(QString title READ title CONSTANT)
   Q_PROPERTY(QString widget READ widget CONSTANT)
   Q_PROPERTY(int datasetCount READ count CONSTANT)
   Q_PROPERTY(QList<Dataset *> datasets READ datasets CONSTANT)

public:
   Group(QObject *parent = nullptr);
   ~Group();

   int count() const;
   QString title() const;
   QString widget() const;
   QList<Dataset *> datasets() const;
   Q_INVOKABLE Dataset *getDataset(const int index);

   bool read(const QJsonObject &object);

private:
   QString m_title;
   QString m_widget;
   QList<Dataset *> m_datasets;
};

#endif
