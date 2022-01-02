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

#pragma once

#include <QFile>
#include <QVector>
#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QTextStream>
#include <QJsonObject>

namespace CSV
{
/**
 * @brief The Export class
 *
 * The CSV export class receives data from the @c IO::Manager class and
 * exports the received frames into a CSV file selected by the user.
 *
 * CSV-data is generated periodically each time the @c Misc::TimerEvents
 * low-frequency timer expires (e.g. every 1 second). The idea behind this
 * is to allow exporting data, but avoid freezing the application when serial
 * data is received continuously.
 */
typedef struct
{
    QByteArray data;
    QDateTime rxDateTime;
} RawFrame;

class Export : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(bool isOpen
               READ isOpen
               NOTIFY openChanged)
    Q_PROPERTY(bool exportEnabled
               READ exportEnabled
               WRITE setExportEnabled
               NOTIFY enabledChanged)
    // clang-format on

Q_SIGNALS:
    void openChanged();
    void enabledChanged();

private:
    explicit Export();
    Export(Export &&) = delete;
    Export(const Export &) = delete;
    Export &operator=(Export &&) = delete;
    Export &operator=(const Export &) = delete;

    ~Export();

public:
    static Export &instance();

    bool isOpen() const;
    bool exportEnabled() const;

public Q_SLOTS:
    void closeFile();
    void openCurrentCsv();
    void setExportEnabled(const bool enabled);

private Q_SLOTS:
    void writeValues();
    void registerFrame(const QByteArray &data);
    void createCsvFile(const CSV::RawFrame &frame);

private:
    QFile m_csvFile;
    bool m_exportEnabled;
    QTextStream m_textStream;
    QVector<RawFrame> m_frames;
};
}
