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
#include <QObject>
#include <QThread>
#include <QSettings>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include "Frame.h"
#include "FrameInfo.h"

namespace JSON
{
/**
 * Generates a JSON document by combining the JSON map file and the received
 * data from the microcontroller device.
 *
 * This code is executed on another thread in order to avoid blocking the
 * user interface.
 */
class JSONWorker : public QObject
{
    Q_OBJECT

signals:
    void finished();
    void jsonReady(const JFI_Object &info);

public:
    JSONWorker(const QByteArray &data, const quint64 frame, const QDateTime &time);

public slots:
    void process();

private:
    QDateTime m_time;
    QByteArray m_data;
    quint64 m_frame;
};

/**
 * @brief The Generator class
 *
 * The JSON generator class receives raw frame data from the I/O manager
 * class and generates a JSON file that represents the project title,
 * the groups that compose the project and the datasets that compose each
 * group.
 *
 * As described in the documentation of the @c Frame class, the process
 * of receiving data and generating the Serial Studio user interface is:
 * 1) Physical device sends data
 * 2) I/O driver receives data
 * 3) I/O manager processes the data and separates the frames
 * 4) JSON generator creates a JSON file with the data contained in each frame.
 * 5) UI dashboard class receives the JSON file.
 * 6) TimerEvents class notifies the UI dashboard that the user interface should
 *    be re-generated.
 * 7) UI dashboard feeds JSON data to a @c Frame object.
 * 8) The @c Frame object creates a model of the JSON data with the values of
 *    the latest received frame.
 * 9) UI dashboard updates the widgets with the C++ model provided by this class.
 */
class Generator : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QString jsonMapFilename
               READ jsonMapFilename
               NOTIFY jsonFileMapChanged)
    Q_PROPERTY(QString jsonMapFilepath
               READ jsonMapFilepath
               NOTIFY jsonFileMapChanged)
    Q_PROPERTY(OperationMode operationMode
               READ operationMode
               WRITE setOperationMode
               NOTIFY operationModeChanged)
    Q_PROPERTY(bool processFramesInSeparateThread
               READ processFramesInSeparateThread
               WRITE setProcessFramesInSeparateThread
               NOTIFY processFramesInSeparateThreadChanged)
    // clang-format on

signals:
    void jsonFileMapChanged();
    void operationModeChanged();
    void jsonChanged(const JFI_Object &info);
    void processFramesInSeparateThreadChanged();

public:
    enum OperationMode
    {
        kManual = 0x00,
        kAutomatic = 0x01,
    };
    Q_ENUM(OperationMode)

public:
    static Generator *getInstance();

    QString jsonMapData() const;
    QString jsonMapFilename() const;
    QString jsonMapFilepath() const;
    OperationMode operationMode() const;
    bool processFramesInSeparateThread() const;

public slots:
    void loadJsonMap();
    void loadJsonMap(const QString &path);
    void setProcessFramesInSeparateThread(const bool threaded);
    void setOperationMode(const JSON::Generator::OperationMode &mode);

private:
    Generator();

public slots:
    void readSettings();
    void loadJFI(const JFI_Object &object);
    void writeSettings(const QString &path);
    void loadJSON(const QJsonDocument &json);

private slots:
    void reset();
    void readData(const QByteArray &data);
    void processFrame(const QByteArray &data, const quint64 frame, const QDateTime &time);

private:
    QFile m_jsonMap;
    quint64 m_frameCount;
    QSettings m_settings;
    QString m_jsonMapData;
    OperationMode m_opMode;
    bool m_processInSeparateThread;
};
}
