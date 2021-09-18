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

#include "Generator.h"

#include <Logger.h>
#include <CSV/Player.h>
#include <IO/Manager.h>
#include <MQTT/Client.h>
#include <Misc/Utilities.h>
#include <ConsoleAppender.h>

#include <QFileInfo>
#include <QFileDialog>

using namespace JSON;

/*
 * Only instance of the class
 */
static Generator *INSTANCE = nullptr;

/*
 * Regular expresion used to check if there are still unmatched values
 * on the JSON map file.
 */
static const QRegExp UNMATCHED_VALUES_REGEX("(%\b([0-9]|[1-9][0-9])\b)");

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
Generator::Generator()
    : m_frameCount(0)
    , m_opMode(kAutomatic)
{
    auto io = IO::Manager::getInstance();
    auto cp = CSV::Player::getInstance();
    connect(cp, SIGNAL(openChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(deviceChanged()), this, SLOT(reset()));
    connect(io, SIGNAL(frameReceived(QByteArray)), this, SLOT(readData(QByteArray)));
    m_workerThread.start();

    LOG_TRACE() << "Class initialized";
}

/**
 * Returns the only instance of the class
 */
Generator *Generator::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Generator();

    return INSTANCE;
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
 * Creates a file dialog & lets the user select the JSON file map
 */
void Generator::loadJsonMap()
{
    // clang-format off
    auto file = QFileDialog::getOpenFileName(Q_NULLPTR,
                                             tr("Select JSON map file"),
                                             QDir::homePath(),
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
            LOG_TRACE() << "JSON parse error" << error.errorString();

            m_jsonMap.close();
            writeSettings("");
            Misc::Utilities::showMessageBox(tr("JSON parse error"), error.errorString());
        }

        // JSON contains no errors, load data & save settings
        else
        {
            LOG_TRACE() << "JSON map loaded successfully";

            writeSettings(path);
            m_jsonMapData = QString::fromUtf8(data);
        }

        // Get rid of warnings
        Q_UNUSED(document);
    }

    // Open error
    else
    {
        LOG_TRACE() << "JSON file error" << m_jsonMap.errorString();

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
void Generator::setOperationMode(const OperationMode mode)
{
    m_opMode = mode;
    emit operationModeChanged();

    LOG_TRACE() << "Operation mode set to" << mode;
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
 * Saves the location of the last valid JSON map file that was opened (if any)
 */
void Generator::writeSettings(const QString &path)
{
    m_settings.setValue("json_map_location", path);
}

/**
 * Notifies the rest of the application that a new JSON frame has been received. The JFI
 * also contains RX date/time and frame number.
 *
 * Read the "FrameInfo.h" file for more information.
 */
void Generator::loadJFI(const JFI_Object &info)
{
    bool csvOpen = CSV::Player::getInstance()->isOpen();
    bool devOpen = IO::Manager::getInstance()->connected();
    bool mqttSub = MQTT::Client::getInstance()->isSubscribed();

    if (csvOpen || devOpen || mqttSub)
    {
        if (JFI_Valid(info))
            emit jsonChanged(info);
    }

    else
        reset();
}

/**
 * Create a new JFI event with the given @a JSON document and increment the frame count
 */
void Generator::loadJSON(const QJsonDocument &json)
{
    auto jfi = JFI_CreateNew(m_frameCount, QDateTime::currentDateTime(), json);
    m_frameCount++;
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

    // Increment received frames
    m_frameCount++;

    // Create new worker thread to read JSON data
    QThread *thread = new QThread;
    JSONWorker *worker = new JSONWorker(data, m_frameCount, QDateTime::currentDateTime());
    worker->moveToThread(thread);
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(worker, &JSONWorker::jsonReady, this, &Generator::loadJFI);
    thread->start();
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
    , m_engine(nullptr)
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
    if (Generator::getInstance()->operationMode() == Generator::kAutomatic)
        document = QJsonDocument::fromJson(m_data, &error);

    // We need to use a map file, check if its loaded & replace values into map
    else
    {
        // Initialize javscript engine
        m_engine = new QJSEngine(this);

        // Empty JSON map data
        if (Generator::getInstance()->jsonMapData().isEmpty())
            return;

        // Init conversion status boolean
        bool ok = true;

        // Separate incoming data & add it to the JSON map
        auto json = Generator::getInstance()->jsonMapData();
        auto list = QString::fromUtf8(m_data).split(
            IO::Manager::getInstance()->separatorSequence());
        for (int i = 0; i < list.count(); ++i)
        {
            // Get value at i & insert it into json
            auto str = list.at(i);
            auto mod = json.arg(str);

            // If JSON after insertion is different we're good to go
            if (json != mod)
                json = mod;

            // JSON is the same after insertion -> format error
            else
            {
                ok = false;
                break;
            }
        }

        // Test that JSON does not contain unmatched values
        if (ok)
            ok = !(json.contains(UNMATCHED_VALUES_REGEX));

        // There was an error & the JSON map is incomplete (or misses received
        // info from the microcontroller).
        if (!ok)
            return;

        // Create json document
        auto jsonDocument = QJsonDocument::fromJson(json.toUtf8(), &error);

        // Calculate dynamically generated values
        auto root = jsonDocument.object();
        auto groups = root.value("g").toArray();
        for (int i = 0; i < groups.count(); ++i)
        {
            // Get group
            auto group = groups.at(i).toObject();

            // Evaluate each dataset of the current group
            auto datasets = group.value("d").toArray();
            for (int j = 0; j < datasets.count(); ++j)
            {
                // Get dataset object & value
                auto dataset = datasets.at(j).toObject();
                auto value = dataset.value("v").toString();

                // Evaluate code in dataset value (if any)
                auto jsValue = m_engine->evaluate(value);

                // Code execution correct, replace value in JSON
                if (!jsValue.isError())
                {
                    dataset.remove("v");
                    dataset.insert("v", jsValue.toString());
                    datasets.replace(j, dataset);
                }
            }

            // Replace group datasets
            group.remove("d");
            group.insert("d", datasets);
            groups.replace(i, group);
        }

        // Replace root document group objects
        root.remove("g");
        root.insert("g", groups);

        // Create JSON document
        document = QJsonDocument(root);

        // Delete javacript engine
        m_engine->deleteLater();
    }

    // No parse error, update UI & reset error counter
    if (error.error == QJsonParseError::NoError)
        emit jsonReady(JFI_CreateNew(m_frame, m_time, document));

    // Delete object in 500 ms
    QTimer::singleShot(500, this, SIGNAL(finished()));
}
