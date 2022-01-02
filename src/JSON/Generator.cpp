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

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
JSON::Generator::Generator()
    : m_frameCount(0)
    , m_opMode(kAutomatic)
    , m_processInSeparateThread(false)
{
    const auto io = &IO::Manager::instance();
    const auto cp = &CSV::Player::instance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(deviceChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(frameReceived(QByteArray)), this, SLOT(readData(QByteArray)));

    readSettings();
}

/**
 * Returns the only instance of the class
 */
JSON::Generator &JSON::Generator::instance()
{
    static Generator singleton;
    return singleton;
}

/**
 * Returns the JSON map data from the loaded file as a string
 */
QString JSON::Generator::jsonMapData() const
{
    return m_jsonMapData;
}

/**
 * Returns the file name (e.g. "JsonMap.json") of the loaded JSON map file
 */
QString JSON::Generator::jsonMapFilename() const
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
QString JSON::Generator::jsonMapFilepath() const
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
JSON::Generator::OperationMode JSON::Generator::operationMode() const
{
    return m_opMode;
}

/**
 * Returns @c true if JSON frames shall be generated in a separate thread
 */
bool JSON::Generator::processFramesInSeparateThread() const
{
    return m_processInSeparateThread;
}

/**
 * Creates a file dialog & lets the user select the JSON file map
 */
void JSON::Generator::loadJsonMap()
{
    // clang-format off
    auto file = QFileDialog::getOpenFileName(Q_NULLPTR,
                                             tr("Select JSON map file"),
                                             Editor::instance().jsonProjectsPath(),
                                             tr("JSON files") + " (*.json)");
    // clang-format on

    if (!file.isEmpty())
        loadJsonMap(file);
}

/**
 * Opens, validates & loads into memory the JSON file in the given @a path.
 */
void JSON::Generator::loadJsonMap(const QString &path)
{
    // Validate path
    if (path.isEmpty())
        return;

    // Close previous file (if open)
    if (m_jsonMap.isOpen())
    {
        m_jsonMap.close();
        m_jsonMapData = "";
        Q_EMIT jsonFileMapChanged();
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
    Q_EMIT jsonFileMapChanged();
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
void JSON::Generator::setOperationMode(const JSON::Generator::OperationMode &mode)
{
    m_opMode = mode;
    Q_EMIT operationModeChanged();
}

/**
 * Enables or disables multi-threaded frame processing
 */
void JSON::Generator::setProcessFramesInSeparateThread(const bool threaded)
{
    m_processInSeparateThread = threaded;
    Q_EMIT processFramesInSeparateThreadChanged();
}

/**
 * Loads the last saved JSON map file (if any)
 */
void JSON::Generator::readSettings()
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
void JSON::Generator::loadJFI(const JFI_Object &info)
{
    const bool csvOpen = CSV::Player::instance().isOpen();
    const bool devOpen = IO::Manager::instance().connected();
    const bool mqttSub = MQTT::Client::instance().isSubscribed();

    if (csvOpen || devOpen || mqttSub)
        Q_EMIT jsonChanged(info);

    else
        reset();
}

/**
 * Saves the location of the last valid JSON map file that was opened (if any)
 */
void JSON::Generator::writeSettings(const QString &path)
{
    m_settings.setValue("json_map_location", path);
}

/**
 * Create a new JFI event with the given @a JSON document and increment the frame count
 */
void JSON::Generator::loadJSON(const QJsonDocument &json)
{
    m_frameCount++;
    auto jfi = JFI_CreateNew(m_frameCount, QDateTime::currentDateTime(), json);
    loadJFI(jfi);
}

/**
 * Resets all the statistics related to the current device and the JSON map file
 */
void JSON::Generator::reset()
{
    m_frameCount = 0;
    Q_EMIT jsonChanged(JFI_Empty());
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
void JSON::Generator::readData(const QByteArray &data)
{
    // Data empty, abort
    if (data.isEmpty())
        return;

    // Increment received frames and process frame
    m_frameCount++;

    // Create new worker thread to read JSON data
    if (processFramesInSeparateThread())
    {
        // clang-format off
        QThread *thread = new QThread;
        Worker *worker = new Worker(data,
                                            m_frameCount,
                                            QDateTime::currentDateTime());
        worker->moveToThread(thread);
        // clang-format on

        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(worker, &JSON::Worker::jsonReady, this, &JSON::Generator::loadJFI);
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
void JSON::Generator::processFrame(const QByteArray &data, const quint64 frame,
                                   const QDateTime &time)
{
    // Serial device sends JSON (auto mode)
    if (operationMode() == JSON::Generator::kAutomatic)
        m_jfi.jsonDocument = QJsonDocument::fromJson(data, &m_error);

    // We need to use a map file, check if its loaded & replace values into map
    else
    {
        // Empty JSON map data
        if (jsonMapData().isEmpty())
            return;

        // Separate incoming data & add it to the JSON map
        auto json = jsonMapData();
        auto sepr = IO::Manager::instance().separatorSequence();
        auto list = QString::fromUtf8(data).split(sepr);
        for (int i = 0; i < list.count(); ++i)
            json.replace(QString("\"%%1\"").arg(i + 1), "\"" + list.at(i) + "\"");

        // Calculate dynamically generated values
        auto root = QJsonDocument::fromJson(json.toUtf8(), &m_error).object();
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
        m_jfi.jsonDocument = QJsonDocument(root);

        // Clear strings
        json.clear();
    }

    // No parse error, update UI & reset error counter
    if (m_error.error == QJsonParseError::NoError)
    {
        m_jfi.rxDateTime = time;
        m_jfi.frameNumber = frame;
        Q_EMIT jsonChanged(m_jfi);
    }
}

//----------------------------------------------------------------------------------------
// JSON worker object (executed for each frame on a new thread)
//----------------------------------------------------------------------------------------

/**
 * Constructor function, stores received frame data & the date/time that the frame data
 * was received.
 */
JSON::Worker::Worker(const QByteArray &data, const quint64 frame, const QDateTime &time)
    : m_time(time)
    , m_data(data)
    , m_frame(frame)
{
}

/**
 * Reads the frame & inserts its values on the JSON map, and/or extracts the JSON frame
 * directly from the serial data.
 */
void JSON::Worker::process()
{
    // Init variables
    QJsonParseError error;
    QJsonDocument document;

    // Serial device sends JSON (auto mode)
    if (Generator::instance().operationMode() == Generator::kAutomatic)
        document = QJsonDocument::fromJson(m_data, &error);

    // We need to use a map file, check if its loaded & replace values into map
    else
    {
        // Empty JSON map data
        if (Generator::instance().jsonMapData().isEmpty())
            return;

        // Separate incoming data & add it to the JSON map
        auto json = Generator::instance().jsonMapData();
        const auto sepr = IO::Manager::instance().separatorSequence();
        const auto list = QString::fromUtf8(m_data).split(sepr);
        for (int i = 0; i < list.count(); ++i)
            json.replace(QString("\"%%1\"").arg(i + 1), "\"" + list.at(i) + "\"");

        // Create json document
        const auto jsonData = json.toUtf8();
        const auto jsonDocument = QJsonDocument::fromJson(jsonData, &error);

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
        Q_EMIT jsonReady(JFI_CreateNew(m_frame, m_time, document));

    // Delete object
    Q_EMIT finished();
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Generator.cpp"
#endif
