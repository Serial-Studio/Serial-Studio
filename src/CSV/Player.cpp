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

#include <QtMath>
#include <QFileDialog>
#include <QApplication>

#include <qtcsv/stringdata.h>
#include <qtcsv/reader.h>

#include <CSV/Player.h>
#include <IO/Manager.h>
#include <Misc/Utilities.h>

/**
 * Constructor function
 */
CSV::Player::Player()
    : m_framePos(0)
    , m_playing(false)
    , m_timestamp("")
{
    connect(this, SIGNAL(playerStateChanged()), this, SLOT(updateData()));
}

/**
 * Returns the only instance of the class
 */
CSV::Player &CSV::Player::instance()
{
    static Player singleton;
    return singleton;
}

/**
 * Returns @c true if an CSV file is open for reading
 */
bool CSV::Player::isOpen() const
{
    return m_csvFile.isOpen();
}

/**
 * Returns the CSV playback progress in a range from 0.0 to 1.0
 */
qreal CSV::Player::progress() const
{
    return ((qreal)framePosition()) / frameCount();
}

/**
 * Returns @c true if the user is currently re-playing the CSV file at real-time
 * speed.
 */
bool CSV::Player::isPlaying() const
{
    return m_playing;
}

/**
 * Returns the short filename of the current CSV file
 */
QString CSV::Player::filename() const
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
int CSV::Player::frameCount() const
{
    return m_csvData.count() - 1;
}

/**
 * Returns the current row that we are using to create the JSON data that is
 * feed to the JsonParser class.
 */
int CSV::Player::framePosition() const
{
    return m_framePos;
}

/**
 * Returns the timestamp of the current data frame / row.
 */
QString CSV::Player::timestamp() const
{
    return m_timestamp;
}

/**
 * Returns the default path for CSV files
 */
QString CSV::Player::csvFilesPath() const
{
    // Get file name and path
    // clang-format off
    QString path = QString("%1/Documents/%2/CSV/").arg(QDir::homePath(),
                                                       qApp->applicationName());
    // clang-format on

    // Generate file path if required
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");

    return path;
}

/**
 * Enables CSV playback at 'live' speed (as it happened when CSV file was
 * saved to the computer).
 */
void CSV::Player::play()
{
    m_playing = true;
    Q_EMIT playerStateChanged();
}

/**
 * Pauses the CSV playback so that the user can see WTF happened at
 * certain point of the mission.
 */
void CSV::Player::pause()
{
    m_playing = false;
    Q_EMIT playerStateChanged();
}

/**
 * Toggles play/pause state
 */
void CSV::Player::toggle()
{
    m_playing = !m_playing;
    Q_EMIT playerStateChanged();
}

/**
 * Lets the user select a CSV file
 */
void CSV::Player::openFile()
{
    // clang-format off

    // Get file name
    auto file = QFileDialog::getOpenFileName(
                Q_NULLPTR,
                tr("Select CSV file"),
                csvFilesPath(),
                tr("CSV files") + " (*.csv)");

    // Open CSV file
    if (!file.isEmpty())
        openFile(file);

    // clang-format on
}

/**
 * Closes the file & cleans up internal variables. This helps us to reduice
 * memory usage & prepare the module to load another CSV file.
 */
void CSV::Player::closeFile()
{
    m_framePos = 0;
    m_csvFile.close();
    m_csvData.clear();
    m_playing = false;
    m_timestamp = "--.--";

    Q_EMIT openChanged();
    Q_EMIT timestampChanged();
    Q_EMIT playerStateChanged();
}

/**
 * Reads & processes the next CSV row (until we get to the last row)
 */
void CSV::Player::nextFrame()
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
void CSV::Player::previousFrame()
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
void CSV::Player::openFile(const QString &filePath)
{
    // File name empty, abort
    if (filePath.isEmpty())
        return;

    // Close previous file
    closeFile();

    // Device is connected, warn user & disconnect
    if (IO::Manager::instance().connected())
    {
        auto response = Misc::Utilities::showMessageBox(
            tr("Serial port open, do you want to continue?"),
            tr("In order to use this feature, its necessary "
               "to disconnect from the serial port"),
            qAppName(), QMessageBox::No | QMessageBox::Yes);
        if (response == QMessageBox::Yes)
            IO::Manager::instance().disconnectDevice();
        else
            return;
    }

    // Try to open the current file
    m_csvFile.setFileName(filePath);
    if (m_csvFile.open(QIODevice::ReadOnly))
    {
        // Read CSV file into string matrix
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QVector<QString> row;
        auto csv = QtCSV::Reader::readToList(m_csvFile);
        for (int i = 0; i < csv.count(); ++i)
        {
            row.clear();
            for (int j = 0; j < csv.at(i).count(); ++j)
                row.append(csv[i][j]);

            m_csvData.append(row);
        }
#else
        m_csvData = QtCSV::Reader::readToList(m_csvFile);
#endif

        // Validate CSV file (brutality works sometimes)
        bool valid = true;
        for (int i = 1; i < frameCount(); ++i)
        {
            valid &= validateRow(i);
            if (!valid)
                break;
        }

        // Read first row & update UI
        if (valid)
        {
            // Read first data & Q_EMIT UI signals
            updateData();
            Q_EMIT openChanged();

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
        Misc::Utilities::showMessageBox(tr("Cannot read CSV file"),
                                        tr("Please check file permissions & location"));
        closeFile();
    }
}

/**
 * Reads a specific row from the @a progress range (which can have a value
 * ranging from 0.0 to 1.0).
 */
void CSV::Player::setProgress(const qreal &progress)
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
void CSV::Player::updateData()
{
    // File not open, abort
    if (!isOpen())
        return;

    // Update timestamp string
    bool error = true;
    auto timestamp = getCellValue(framePosition() + 1, 0, error);
    if (!error)
    {
        m_timestamp = timestamp;
        Q_EMIT timestampChanged();
    }

    // Construct frame from CSV and send it to the IO manager
    IO::Manager::instance().processPayload(getFrame(framePosition() + 1));

    // If the user wants to 'play' the CSV, get time difference between this
    // frame and the next frame & schedule an automated update
    if (isPlaying())
    {
        // Get first frame
        if (framePosition() < frameCount())
        {
            bool error = true;
            auto currTime = getCellValue(framePosition() + 1, 0, error);
            auto nextTime = getCellValue(framePosition() + 2, 0, error);

            // No error, calculate difference & schedule update
            if (!error)
            {
                auto format = "yyyy/MM/dd/ HH:mm:ss::zzz"; // Same as in Export.cpp
                auto currDateTime = QDateTime::fromString(currTime, format);
                auto nextDateTime = QDateTime::fromString(nextTime, format);
                auto msecsToNextF = currDateTime.msecsTo(nextDateTime);

                // clang-format off
                QTimer::singleShot(msecsToNextF,
                                   Qt::PreciseTimer,
                                   this,
                                   SLOT(nextFrame()));
                // clang-format on
            }

            // Error - pause playback
            else
            {
                pause();
                qWarning() << "Error getting timestamp difference";
            }
        }

        // Pause at end of CSV
        else
            pause();
    }
}

/**
 * Checks that the row at the given @a position has the same number of elements
 * as the title row (the first row of the CSV file). Also, we do another
 * validation by checking that the timestamp cell has the correct title as the
 * one used in the @c Export class.
 */
bool CSV::Player::validateRow(const int position)
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
        qWarning() << "Mismatched CSV data on frame" << framePosition();
        closeFile();
        return false;
    }

    // Check that this CSV is valid by checking the time title, this value must
    // be the same one that is used in Export.cpp
    auto rxTitle = "RX Date/Time";
    if (titles.first() != rxTitle)
    {
        qWarning() << "Invalid CSV file (title format does not match)";
        closeFile();
        return false;
    }

    // Valid row
    return true;
}

/**
 * Generates a frame from the data at the given @a row. The first item of each row is
 * ignored because it contains the RX date/time, which is used to regulate the interval
 * at which the frames are parsed.
 */
QByteArray CSV::Player::getFrame(const int row)
{
    QByteArray frame;
    auto sep = IO::Manager::instance().separatorSequence();

    if (m_csvData.count() > row)
    {
        auto list = m_csvData.at(row);
        for (int i = 1; i < list.count(); ++i)
        {
            frame.append(list.at(i).toUtf8());
            if (i < list.count() - 1)
                frame.append(sep.toUtf8());
            else
                frame.append('\n');
        }
    }

    return frame;
}

/**
 * Safely returns the value in the cell at the given @a row & @a column. If an
 * error occurs or the cell does not exist, the value of @a error shall be set
 * to @c true.
 */
QString CSV::Player::getCellValue(const int row, const int column, bool &error)
{
    if (m_csvData.count() > row)
    {
        auto list = m_csvData.at(row);
        if (list.count() > column)
        {
            error = false;
            return list.at(column);
        }
    }

    error = true;
    return "";
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Player.cpp"
#endif
