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

#include "Editor.h"
#include "Generator.h"
#include "FrameInfo.h"

#include <CSV/Player.h>
#include <IO/Manager.h>
#include <MQTT/Client.h>
#include <Misc/Utilities.h>

#include <QFileInfo>
#include <QFileDialog>
#include <QRegularExpression>

namespace JSON
{
static Generator *GENERATOR = Q_NULLPTR;

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
Generator::Generator()
    : m_frameCount(0)
    , m_opMode(kAutomatic)
    , m_processInSeparateThread(false)
{
    const auto io = IO::Manager::getInstance();
    const auto cp = CSV::Player::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(deviceChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(frameReceived(QByteArray)), this, SLOT(readData(QByteArray)));

    readSettings();
}

/**
 * Returns the only instance of the class
 */
Generator *Generator::getInstance()
{
    if (!GENERATOR)
        GENERATOR = new Generator();

    return GENERATOR;
}

/**
 * Returns the JSON map data from the loaded file as a string
 */
QString Generator::jsonMapData() const
{
    return m_jsonMapData;
}

/**
 * Returns the file name (e.g. "JsonMap.json") of the loaded JSON map file
 */
QString Generator::jsonMapFilename() const
{
    if (m_jsonMap.isOpen())
    {
        auto fileInfo = QFileInfo(m_jsonMap.fileName());
        return fileInfo.fileName();
    }

    return "";
}

/**
 * Returns the file path of the loaded JSON map file
 */
QString Generator::jsonMapFilepath() const
{
    if (m_jsonMap.isOpen())
    {
        auto fileInfo = QFileInfo(m_jsonMap.fileName());
        return fileInfo.filePath();
    }

    return "";
}

/**
 * Returns the operation mode
 */
Generator::OperationMode Generator::operationMode() const
{
    return m_opMode;
}

/**
 * Returns @c true if JSON frames shall be generated in a separate thread
 */
bool Generator::processFramesInSeparateThread() const
{
    return m_processInSeparateThread;
}

/**
 * Creates a file dialog & lets the user select the JSON file map
 */
void Generator::loadJsonMap()
{
    // clang-format off
    auto file = QFileDialog::getOpenFileName(Q_NULLPTR,
                                             tr("Select JSON map file"),
                                             Editor::getInstance()->jsonProjectsPath(),
                                             tr("JSON files") + " (*.json)");
    // clang-format on

    if (!file.isEmpty())
        loadJsonMap(file);
}

/**
 * Opens, validates & loads into memory the JSON file in the given @a path.
 */
void Generator::loadJsonMap(const QString &path)
{
    // Validate path
    if (path.isEmpty())
        return;

    // Close previous file (if open)
    if (m_jsonMap.isOpen())
    {
        m_jsonMap.close();
        m_jsonMapData = "";
        emit jsonFileMapChanged();
    }

    // Try to open the file (read only mode)
    m_jsonMap.setFileName(path);
    if (m_jsonMap.open(QFile::ReadOnly))
    {
        // Read data & validate JSON from file
        QJsonParseError error;
        auto data = m_jsonMap.readAll();
        auto document = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError)
        {
            m_jsonMap.close();
            writeSettings("");
            Misc::Utilities::showMessageBox(tr("JSON parse error"), error.errorString());
        }

        // JSON contains no errors, load data & save settings
        else
        {
            writeSettings(path);
            m_jsonMapData = QString::fromUtf8(data);
        }

        // Get rid of warnings
        Q_UNUSED(document);
    }

    // Open error
    else
    {
        m_jsonMapData = "";
        writeSettings("");
        Misc::Utilities::showMessageBox(tr("Cannot read JSON file"),
                                        tr("Please check file permissions & location"));
        m_jsonMap.close();
    }

    // Update UI
    emit jsonFileMapChanged();
}

/**
 * Changes the operation mode of the JSON parser. There are two possible op.
 * modes:
 *
 * @c kManual serial data only contains the comma-separated values, and we need
 *            to use a JSON map file (given by the user) to know what each value
 *            means. This method is recommended when we need to transfer &
 *            display a large amount of information from the microcontroller
 *            unit to the computer.
 *
 * @c kAutomatic serial data contains the JSON data frame, good for simple
 *               applications or for prototyping.
 */
void Generator::setOperationMode(const JSON::Generator::OperationMode mode)
{
    m_opMode = mode;
    emit operationModeChanged();
}

/**
 * Enables or disables multi-threaded frame processing
 */
void Generator::setProcessFramesInSeparateThread(const bool threaded)
{
    m_processInSeparateThread = threaded;
    emit processFramesInSeparateThreadChanged();
}

/**
 * Loads the last saved JSON map file (if any)
 */
void Generator::readSettings()
{
    auto path = m_settings.value("json_map_location", "").toString();
    if (!path.isEmpty())
        loadJsonMap(path);
}

/**
 * Notifies the rest of the application that a new JSON frame has been received. The JFI
 * also contains RX date/time and frame number.
 *
 * Read the "FrameInfo.h" file for more information.
 */
void Generator::loadJFI(const JFI_Object &info)
{
    const bool csvOpen = CSV::Player::getInstance()->isOpen();
    const bool devOpen = IO::Manager::getInstance()->connected();
    const bool mqttSub = MQTT::Client::getInstance()->isSubscribed();

    if (csvOpen || devOpen || mqttSub)
        emit jsonChanged(info);

    else
        reset();
}

/**
 * Saves the location of the last valid JSON map file that was opened (if any)
 */
void Generator::writeSettings(const QString &path)
{
    m_settings.setValue("json_map_location", path);
}

/**
 * Create a new JFI event with the given @a JSON document and increment the frame count
 */
void Generator::loadJSON(const QJsonDocument &json)
{
    m_frameCount++;
    auto jfi = JFI_CreateNew(m_frameCount, QDateTime::currentDateTime(), json);
    loadJFI(jfi);
}

/**
 * Resets all the statistics related to the current device and the JSON map file
 */
void Generator::reset()
{
    m_frameCount = 0;
    emit jsonChanged(JFI_Empty());
}

/**
 * Tries to parse the given data as a JSON document according to the selected
 * operation mode.
 *
 * Possible operation modes:
 * - Auto:   serial data contains the JSON data frame
 * - Manual: serial data only contains the comma-separated values, and we need
 *           to use a JSON map file (given by the user) to know what each value
 *           means
 *
 * If JSON parsing is successfull, then the class shall notify the rest of the
 * application in order to process packet data.
 */
void Generator::readData(const QByteArray &data)
{
    // CSV-replay active, abort
    if (CSV::Player::getInstance()->isOpen())
        return;

    // Data empty, abort
    if (data.isEmpty())
        return;

    // Increment received frames and process frame
    m_frameCount++;

    // Create new worker thread to read JSON data
    if (processFramesInSeparateThread())
    {
        QThread *thread = new QThread;
        JSONWorker *worker
            = new JSONWorker(data, m_frameCount, QDateTime::currentDateTime());
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, &JSONWorker::jsonReady, this, &Generator::loadJFI);
        thread->start();
    }

    // Process frames in main thread
    else
        processFrame(data, m_frameCount, QDateTime::currentDateTime());
}

/**
 * Reads the frame & inserts its values on the JSON map, and/or extracts the JSON frame
 * directly from the serial data.
 */
void Generator::processFrame(const QByteArray &data, const quint64 frame,
                             const QDateTime &time)
{
    // Init variables
    QJsonParseError error;
    QJsonDocument document;

    // Serial device sends JSON (auto mode)
    if (operationMode() == Generator::kAutomatic)
        document = QJsonDocument::fromJson(data, &error);

    // We need to use a map file, check if its loaded & replace values into map
    else
    {
        // Empty JSON map data
        if (jsonMapData().isEmpty())
            return;

        // Separate incoming data & add it to the JSON map
        auto json = jsonMapData();
        auto sepr = IO::Manager::getInstance()->separatorSequence();
        auto list = QString::fromUtf8(data).split(sepr);
        for (int i = 0; i < list.count(); ++i)
            json.replace(QString("\"%%1\"").arg(i + 1), "\"" + list.at(i) + "\"");

        // Create json document
        auto jsonData = json.toUtf8();
        auto jsonDocument = QJsonDocument::fromJson(jsonData, &error);

        // Calculate dynamically generated values
        auto root = jsonDocument.object();
        auto groups = JFI_Value(root, "groups", "g").toArray();
        for (int i = 0; i < groups.count(); ++i)
        {
            // Get group
            auto group = groups.at(i).toObject();

            // Evaluate each dataset of the current group
            auto datasets = JFI_Value(group, "datasets", "d").toArray();
            for (int j = 0; j < datasets.count(); ++j)
            {
                auto dataset = datasets.at(j).toObject();
                auto value = JFI_Value(dataset, "value", "v").toString();
                dataset.remove("v");
                dataset.remove("value");
                dataset.insert("value", value);
                datasets.replace(j, dataset);
            }

            // Replace group datasets
            group.remove("d");
            group.remove("datasets");
            group.insert("datasets", datasets);
            groups.replace(i, group);
        }

        // Replace root document group objects
        root.remove("g");
        root.remove("groups");
        root.insert("groups", groups);

        // Create JSON document
        document = QJsonDocument(root);
    }

    // No parse error, update UI & reset error counter
    if (error.error == QJsonParseError::NoError)
        emit jsonChanged(JFI_CreateNew(frame, time, document));
}

//----------------------------------------------------------------------------------------
// JSON worker object (executed for each frame on a new thread)
//----------------------------------------------------------------------------------------

/**
 * Constructor function, stores received frame data & the date/time that the frame data
 * was received.
 */
JSONWorker::JSONWorker(const QByteArray &data, const quint64 frame, const QDateTime &time)
    : m_time(time)
    , m_data(data)
    , m_frame(frame)
{
}

/**
 * Reads the frame & inserts its values on the JSON map, and/or extracts the JSON frame
 * directly from the serial data.
 */
void JSONWorker::process()
{
    // Init variables
    QJsonParseError error;
    QJsonDocument document;

    // Serial device sends JSON (auto mode)
    const auto generator = Generator::getInstance();
    if (generator->operationMode() == Generator::kAutomatic)
        document = QJsonDocument::fromJson(m_data, &error);

    // We need to use a map file, check if its loaded & replace values into map
    else
    {
        // Empty JSON map data
        if (generator->jsonMapData().isEmpty())
            return;

        // Separate incoming data & add it to the JSON map
        auto json = generator->jsonMapData();
        auto sepr = IO::Manager::getInstance()->separatorSequence();
        auto list = QString::fromUtf8(m_data).split(sepr);
        for (int i = 0; i < list.count(); ++i)
            json.replace(QString("\"%%1\"").arg(i + 1), "\"" + list.at(i) + "\"");

        // Create json document
        auto jsonData = json.toUtf8();
        auto jsonDocument = QJsonDocument::fromJson(jsonData, &error);

        // Calculate dynamically generated values
        auto root = jsonDocument.object();
        auto groups = JFI_Value(root, "groups", "g").toArray();
        for (int i = 0; i < groups.count(); ++i)
        {
            // Get group
            auto group = groups.at(i).toObject();

            // Evaluate each dataset of the current group
            auto datasets = JFI_Value(group, "datasets", "d").toArray();
            for (int j = 0; j < datasets.count(); ++j)
            {
                auto dataset = datasets.at(j).toObject();
                auto value = JFI_Value(dataset, "value", "v").toString();
                dataset.remove("v");
                dataset.remove("value");
                dataset.insert("value", value);
                datasets.replace(j, dataset);
            }

            // Replace group datasets
            group.remove("d");
            group.remove("datasets");
            group.insert("datasets", datasets);
            groups.replace(i, group);
        }

        // Replace root document group objects
        root.remove("g");
        root.remove("groups");
        root.insert("groups", groups);

        // Create JSON document
        document = QJsonDocument(root);
    }

    // No parse error, update UI & reset error counter
    if (error.error == QJsonParseError::NoError)
        emit jsonReady(JFI_CreateNew(m_frame, m_time, document));

    // Delete object in 500 ms
    QTimer::singleShot(500, this, SIGNAL(finished()));
}
}
