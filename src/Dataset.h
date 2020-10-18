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

#ifndef DATASET_H
#define DATASET_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>

class Dataset : public QObject
{
   Q_OBJECT

   Q_PROPERTY(bool graph READ graph CONSTANT)
   Q_PROPERTY(QString title READ title CONSTANT)
   Q_PROPERTY(QString value READ value CONSTANT)
   Q_PROPERTY(QString units READ units CONSTANT)

public:
   Dataset(QObject *parent = nullptr);

   bool graph() const;
   QString title() const;
   QString value() const;
   QString units() const;

   bool read(const QJsonObject &object);

private:
   bool m_graph;
   QString m_title;
   QString m_value;
   QString m_units;
};

#endif
