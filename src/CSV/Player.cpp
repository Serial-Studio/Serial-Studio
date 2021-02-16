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

#include "Player.h"

#include <QtMath>
#include <QFileDialog>
#include <QApplication>

#include <qtcsv/stringdata.h>
#include <qtcsv/reader.h>

#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <Logger.h>
#include <ConsoleAppender.h>

#include <IO/Manager.h>
#include <Misc/Utilities.h>
#include <JSON/Generator.h>

using namespace CSV;

//
// Here be dragons...
//

/*
 * Only instance of the class
 */
static Player *INSTANCE = nullptr;

/**
 * Constructor function
 */
Player::Player()
    : m_framePos(0)
    , m_playing(false)
    , m_timestamp("")
{
    connect(this, SIGNAL(playerStateChanged()), this, SLOT(updateData()));
    LOG_TRACE() << "Class initialized";
}

/**
 * Returns the only instance of the class
 */
Player *Player::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Player;

    return INSTANCE;
}

/**
 * Returns @c true if an CSV file is open for reading
 */
bool Player::isOpen() const
{
    return m_csvFile.isOpen();
}

/**
 * Returns the CSV playback progress in a range from 0.0 to 1.0
 */
qreal Player::progress() const
{
    return ((qreal)framePosition()) / frameCount();
}

/**
 * Returns @c true if the user is currently re-playing the CSV file at real-time
 * speed.
 */
bool Player::isPlaying() const
{
    return m_playing;
}

/**
 * Returns the short filename of the current CSV file
 */
QString Player::filename() const
{
    if (isOpen())
    {
        auto fileInfo = QFileInfo(m_csvFile.fileName());
        return fileInfo.fileName();
    }

    return "";
}

/**
 * Returns the total number of frames in the CSV file. This can be calculated
 * by getting the number of rows of the CSV and substracting 1 (because the
 * title cells do not count as a valid frame).
 */
int Player::frameCount() const
{
    return m_csvData.count() - 1;
}

/**
 * Returns the current row that we are using to create the JSON data that is
 * feed to the JsonParser class.
 */
int Player::framePosition() const
{
    return m_framePos;
}

/**
 * Returns the timestamp of the current data frame / row.
 */
QString Player::timestamp() const
{
    return m_timestamp;
}

/**
 * Enables CSV playback at 'live' speed (as it happened when CSV file was
 * saved to the computer).
 */
void Player::play()
{
    m_playing = true;
    emit playerStateChanged();
}

/**
 * Pauses the CSV playback so that the user can see WTF happened at
 * certain point of the mission.
 */
void Player::pause()
{
    m_playing = false;
    emit playerStateChanged();
}

/**
 * Toggles play/pause state
 */
void Player::toggle()
{
    m_playing = !m_playing;
    emit playerStateChanged();
}

/**
 * Lets the user select a CSV file
 */
void Player::openFile()
{
    // Get file name
    auto file = QFileDialog::getOpenFileName(
        Q_NULLPTR, tr("Select CSV file"), QDir::homePath(), tr("CSV files") + " (*.csv)");

    // Open CSV file
    if (!file.isEmpty())
        openFile(file);
}

/**
 * Closes the file & cleans up internal variables. This helps us to reduice
 * memory usage & prepare the module to load another CSV file.
 */
void Player::closeFile()
{
    m_framePos = 0;
    m_model.clear();
    m_csvFile.close();
    m_csvData.clear();
    m_playing = false;
    m_datasetIndexes.clear();
    m_timestamp = "--.--";

    emit openChanged();
    emit timestampChanged();
    emit playerStateChanged();

    LOG_INFO() << "CSV file closed";
}

/**
 * Reads & processes the next CSV row (until we get to the last row)
 */
void Player::nextFrame()
{
    if (framePosition() < frameCount())
    {
        ++m_framePos;
        updateData();
    }
}

/**
 * Reads & processes the previous CSV row (until we get to the first row)
 */
void Player::previousFrame()
{
    if (framePosition() > 0)
    {
        --m_framePos;
        updateData();
    }
}

/**
 * Opens a CSV file and valitates it by comparing every data row with the title
 * row. If one of the data rows does not correspond to the title row, the CSV
 * is considered to be invalid.
 */
void Player::openFile(const QString &filePath)
{
    // Check that manual JSON mode is activaded
    auto opMode = JSON::Generator::getInstance()->operationMode();
    auto jsonOpen = !JSON::Generator::getInstance()->jsonMapData().isEmpty();
    if (opMode != JSON::Generator::kManual || !jsonOpen)
    {
        Misc::Utilities::showMessageBox(
            tr("Invalid configuration for CSV player"),
            tr("You need to select a JSON map file in order to use "
               "this feature"));
        return;
    }

    // File name empty, abort
    if (filePath.isEmpty())
        return;

    // Close previous file
    closeFile();

    // Device is connected, warn user & disconnect
    auto sm = IO::Manager::getInstance();
    if (sm->connected())
    {
        LOG_TRACE() << "Serial device open, asking user what to do...";
        auto response = Misc::Utilities::showMessageBox(
            tr("Serial port open, do you want to continue?"),
            tr("In order to use this feature, its necessary "
               "to disconnect from the serial port"),
            qAppName(), QMessageBox::No | QMessageBox::Yes);
        if (response == QMessageBox::Yes)
            sm->disconnectDevice();
        else
            return;
    }

    // Try to open the current file
    m_csvFile.setFileName(filePath);
    LOG_INFO() << "Trying to open CSV file...";
    if (m_csvFile.open(QIODevice::ReadOnly))
    {
        // Read CSV file into string matrix
        LOG_INFO() << "CSV file read, processing CSV data...";
        m_csvData = QtCSV::Reader::readToList(m_csvFile);

        // Validate CSV file (brutality works sometimes)
        LOG_INFO() << "CSV frame count" << frameCount();
        LOG_INFO() << "Validating CSV file...";
        bool valid = true;
        for (int i = 1; i < frameCount(); ++i)
        {
            valid &= validateRow(i);
            if (!valid)
                break;
        }
        LOG_INFO() << "CSV valid:" << valid;

        // Read first row & update UI
        if (valid)
        {
            // Read first data & emit UI signals
            updateData();
            emit openChanged();

            // Play next frame (to force UI to generate groups, graphs & widgets)
            // Note: nextFrame() MUST BE CALLED AFTER emiting the openChanged() signal in
            //       order for this monstrosity to work
            nextFrame();
        }

        // Show error to the user
        else
        {
            Misc::Utilities::showMessageBox(
                tr("There is an error with the data in the CSV file"),
                tr("Please verify that the CSV file was created with Serial "
                   "Studio"));
        }
    }

    // Open error
    else
    {
        LOG_TRACE() << "CSV file read error" << m_csvFile.errorString();
        Misc::Utilities::showMessageBox(tr("Cannot read CSV file"),
                                        tr("Please check file permissions & location"));
        closeFile();
    }
}

/**
 * Reads a specific row from the @a progress range (which can have a value
 * ranging from 0.0 to 1.0).
 */
void Player::setProgress(const qreal progress)
{
    // Ensure that progress value is between 0 and 1
    auto validProgress = progress;
    if (validProgress > 1)
        validProgress = 1;
    else if (validProgress < 0)
        validProgress = 0;

    // Pause player to avoid messing the scheduled timer (if playing)
    if (isPlaying())
        pause();

    // Calculate frame position & update data
    m_framePos = qCeil(frameCount() * validProgress);
    if (validProgress == 0)
        m_framePos = 0;
    else if (validProgress == 1)
        m_framePos = frameCount();

    // Update CSV values
    updateData();
}

/**
 * Generates a JSON data frame by combining the values of the current CSV
 * row & the structure of the JSON map file loaded in the @c JsonParser class.
 *
 * If playback is enabled, this function calculates the difference in
 * milliseconds between the current row and the next row & schedules a re-call
 * of this function using a timer.
 */
void Player::updateData()
{
    // File not open, abort
    if (!isOpen())
        return;

    // Update timestamp string
    bool error;
    auto timestamp = getCellValue(framePosition() + 1, 0, &error);
    if (!error)
    {
        m_timestamp = timestamp;
        emit timestampChanged();
    }

    // Construct JSON from CSV & instruct the parser to use this document as
    // input source for the QML bridge
    auto json = getJsonFrame(framePosition() + 1);
    if (!json.isEmpty())
        JSON::Generator::getInstance()->setJsonDocument(json);

    // If the user wants to 'play' the CSV, get time difference between this
    // frame and the next frame & schedule an automated update
    if (isPlaying())
    {
        // Get first frame
        if (framePosition() < frameCount())
        {
            bool error = false;
            auto currTime = getCellValue(framePosition() + 1, 0, &error);
            auto nextTime = getCellValue(framePosition() + 2, 0, &error);

            // No error, calculate difference & schedule update
            if (!error)
            {
                auto format = "yyyy/MM/dd/ HH:mm:ss::zzz"; // Same as in Export.cpp
                auto currDateTime = QDateTime::fromString(currTime, format);
                auto nextDateTime = QDateTime::fromString(nextTime, format);
                auto msecsToNextF = currDateTime.msecsTo(nextDateTime);
                QTimer::singleShot(msecsToNextF, Qt::PreciseTimer, this,
                                   SLOT(nextFrame()));
            }

            // Error - pause playback
            else
            {
                pause();
                LOG_WARNING() << "Error getting timestamp difference";
            }
        }

        // Pause at end of CSV
        else
        {
            pause();
            LOG_INFO() << "CSV playback finished";
        }
    }
}

/**
 * Checks that the row at the given @a position has the same number of elements
 * as the title row (the first row of the CSV file). Also, we do another
 * validation by checking that the timestamp cell has the correct title as the
 * one used in the @c Export class.
 */
bool Player::validateRow(const int position)
{
    // Ensure that position is valid
    if (m_csvData.count() <= position)
        return false;

    // Get titles & value list
    auto titles = m_csvData.at(0);
    auto list = m_csvData.at(position);

    // Check that row value count is the same
    if (titles.count() != list.count())
    {
        LOG_WARNING() << "Mismatched CSV data on frame" << framePosition();
        closeFile();
        return false;
    }

    // Check that this CSV is valid by checking the time title, this value must
    // be the same one that is used in Export.cpp
    auto rxTitle = "RX Date/Time";
    if (titles.first() != rxTitle)
    {
        LOG_WARNING() << "Invalid CSV file (title format does not match)";
        closeFile();
        return false;
    }

    // Valid row
    return true;
}

/**
 * Generates a JSON data frame by combining the values of the current CSV
 * row & the structure of the JSON map file loaded in the @c JsonParser class.
 *
 * The details of how this is done are a bit fuzzy, and the methods used here
 * are pretty ugly & unorthodox, but they work. Brutality works.
 */
QJsonDocument Player::getJsonFrame(const int row)
{
    // Create the group/dataset model only one time
    if (m_model.isEmpty())
    {
        LOG_TRACE() << "Generating group/dataset model from CSV...";

        auto titles = m_csvData.at(0);
        for (int i = 1; i < titles.count(); ++i)
        {
            // Construct group string
            QString group;
            auto title = titles.at(i);
            auto glist = title.split(")");
            for (int j = 0; j < glist.count() - 1; ++j)
                group.append(glist.at(j));

            // Remove the '(' from group name
            if (!group.isEmpty())
                group.remove(0, 1);

            // Get dataset name & remove units
            QString dataset = glist.last();
            if (dataset.endsWith("]"))
            {
                while (!dataset.endsWith("["))
                    dataset.chop(1);
            }

            // Remove extra spaces from dataset
            while (dataset.startsWith(" "))
                dataset.remove(0, 1);
            while (dataset.endsWith(" ") || dataset.endsWith("["))
                dataset.chop(1);

            // Register group with dataset map
            if (!m_model.contains(group))
            {
                QSet<QString> set;
                set.insert(dataset);
                m_model.insert(group, set);
            }

            // Update existing group/dataset model
            else if (!m_model.value(group).contains(dataset))
            {
                auto set = m_model.value(group);
                if (!set.contains(dataset))
                    set.insert(dataset);

                m_model.remove(group);
                m_model.insert(group, set);
            }

            // Register dataset index with group key
            if (!m_datasetIndexes.contains(group))
            {
                QMap<QString, int> map;
                map.insert(dataset, i);
                m_datasetIndexes.insert(group, map);
            }

            // Register dataset index with existing group key
            else
            {
                auto map = m_datasetIndexes.value(group);
                map.insert(dataset, i);
                m_datasetIndexes.remove(group);
                m_datasetIndexes.insert(group, map);
            }
        }

        LOG_TRACE() << "Group/dataset model created successfully";
    }

    // Check that row is valid
    if (m_csvData.count() <= row)
    {
        LOG_WARNING() << "Invalid row requested";
        return QJsonDocument();
    }

    // Read CSV row & JSON template from JSON parser
    auto values = m_csvData.at(row);
    auto mapData = JSON::Generator::getInstance()->jsonMapData();
    QJsonDocument jsonTemplate = QJsonDocument::fromJson(mapData.toUtf8());

    // Replace JSON title
    auto json = jsonTemplate.object();
    json["t"] = tr("Replay of %1").arg(filename());

    // Replace values in JSON  with values in row using the model.
    // This is very ugly code, somebody please fix it :(
    auto groups = json.value("g").toArray();
    foreach (auto groupKey, m_model.keys())
    {
        for (int i = 0; i < groups.count(); ++i)
        {
            auto group = groups.at(i).toObject();

            if (group.value("t") == groupKey)
            {
                auto datasetKeys = m_model.value(groupKey);
                auto datasets = group.value("d").toArray();
                foreach (auto datasetKey, datasetKeys)
                {
                    for (int j = 0; j < datasets.count(); ++j)
                    {
                        auto dataset = datasets.at(j).toObject();
                        if (dataset.value("t") == datasetKey)
                        {
                            auto value = values.at(getDatasetIndex(groupKey, datasetKey));
                            dataset.remove("v");
                            dataset.insert("v", value);
                        }

                        datasets.replace(j, dataset);
                    }
                }

                group.remove("d");
                group.insert("d", datasets);
            }

            groups.replace(i, group);
        }
    }

    // Update groups from JSON
    json.remove("g");
    json.insert("g", groups);

    // Return new JSON document
    return QJsonDocument(json);
}

/**
 * Safely returns the value in the cell at the given @a row & @a column. If an
 * error occurs or the cell does not exist, the value of @a error shall be set
 * to @c true.
 */
QString Player::getCellValue(int row, int column, bool *error)
{
    if (m_csvData.count() > row)
    {
        auto list = m_csvData.at(row);
        if (list.count() > column)
        {
            if (error)
                *error = false;

            return list.at(column);
        }
    }

    if (error)
        *error = true;

    return "";
}

/**
 * Returns the column/index for the dataset key that belongs to the given
 * group key.
 */
int Player::getDatasetIndex(const QString &groupKey, const QString &datasetKey)
{
    if (m_datasetIndexes.contains(groupKey))
    {
        auto map = m_datasetIndexes.value(groupKey);
        if (map.contains(datasetKey))
            return map.value(datasetKey);
    }

    return 0;
}
