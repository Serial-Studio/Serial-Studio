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

#ifndef UI_DATA_PROVIDER_H
#define UI_DATA_PROVIDER_H

#include <QObject>
#include <JSON/Frame.h>
#include <JSON/FrameInfo.h>

namespace UI
{
class DataProvider : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString title
               READ title
               NOTIFY updated)
    Q_PROPERTY(int groupCount
               READ groupCount
               NOTIFY updated)
    // clang-format on

signals:
    void updated();
    void dataReset();

public:
    static DataProvider *getInstance();

    QString title();
    int groupCount();

    JSON::Frame *latestFrame();

    Q_INVOKABLE bool frameValid() const;
    Q_INVOKABLE JSON::Group *getGroup(const int index);

private:
    DataProvider();

private slots:
    void resetData();
    void updateData();
    void selectLatestJSON(const JFI_Object &frameInfo);

private:
    JSON::Frame m_latestFrame;
    JFI_Object m_latestJsonFrame;
};
}

#endif
