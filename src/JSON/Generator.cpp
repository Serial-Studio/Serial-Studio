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

#include <QFileInfo>
#include <QFileDialog>
#include <QRegularExpression>

#include <JSON/Editor.h>
#include <JSON/Generator.h>

#include <CSV/Player.h>
#include <IO/Manager.h>
#include <MQTT/Client.h>
#include <Misc/Utilities.h>

/**
 * Initializes the JSON Parser class and connects appropiate SIGNALS/SLOTS
 */
JSON::Generator::Generator()
    : m_opMode(kAutomatic)
{
    // clang-format off
    connect(&CSV::Player::instance(), &CSV::Player::openChanged,
            this, &JSON::Generator::reset);
    connect(&IO::Manager::instance(), &IO::Manager::deviceChanged,
            this, &JSON::Generator::reset);
    connect(&IO::Manager::instance(), &IO::Manager::frameReceived,
            this, &JSON::Generator::readData);
    // clang-format on

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

        // JSON contains no errors, load compacted JSON document & save settings
        else
        {
            writeSettings(path);
            m_jsonMapData = QString::fromUtf8(document.toJson(QJsonDocument::Compact));
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
 * Loads the last saved JSON map file (if any)
 */
void JSON::Generator::readSettings()
{
    auto path = m_settings.value("json_map_location", "").toString();
    if (!path.isEmpty())
        loadJsonMap(path);
}

/**
 * Saves the location of the last valid JSON map file that was opened (if any)
 */
void JSON::Generator::writeSettings(const QString &path)
{
    m_settings.setValue("json_map_location", path);
}

/**
 * Resets all the statistics related to the current device and the JSON map file
 */
void JSON::Generator::reset()
{
    m_json = QJsonObject();
    Q_EMIT jsonChanged(m_json);
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

    // Serial device sends JSON (auto mode)
    if (operationMode() == JSON::Generator::kAutomatic)
        m_json = QJsonDocument::fromJson(data, &m_error).object();

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

        // Create JSON document
        m_json = QJsonDocument::fromJson(json.toUtf8(), &m_error).object();
    }

    // No parse error, update UI & reset error counter
    if (m_error.error == QJsonParseError::NoError)
        Q_EMIT jsonChanged(m_json);
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Generator.cpp"
#endif
