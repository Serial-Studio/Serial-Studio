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

#include "Export.h"

#include <AppInfo.h>
#include <IO/Manager.h>
#include <UI/Dashboard.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>

#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopServices>

namespace CSV
{
static Export *EXPORT = Q_NULLPTR;

/**
 * Connect JSON Parser & Serial Manager signals to begin registering JSON
 * dataframes into JSON list.
 */
Export::Export()
    : m_exportEnabled(true)
{
    const auto io = IO::Manager::getInstance();
    const auto te = Misc::TimerEvents::getInstance();
    connect(io, &IO::Manager::connectedChanged, this, &Export::closeFile);
    connect(io, &IO::Manager::frameReceived, this, &Export::registerFrame);
    connect(te, &Misc::TimerEvents::lowFreqTimeout, this, &Export::writeValues);
}

/**
 * Close file & finnish write-operations before destroying the class
 */
Export::~Export()
{
    closeFile();
}

/**
 * Returns a pointer to the only instance of this class
 */
Export *Export::getInstance()
{
    if (!EXPORT)
        EXPORT = new Export;

    return EXPORT;
}

/**
 * Returns @c true if the CSV output file is open
 */
bool Export::isOpen() const
{
    return m_csvFile.isOpen();
}

/**
 * Returns @c true if CSV export is enabled
 */
bool Export::exportEnabled() const
{
    return m_exportEnabled;
}

/**
 * Open the current CSV file in the Explorer/Finder window
 */
void Export::openCurrentCsv()
{
    if (isOpen())
        Misc::Utilities::revealFile(m_csvFile.fileName());
    else
        Misc::Utilities::showMessageBox(tr("CSV file not open"),
                                        tr("Cannot find CSV export file!"));
}

/**
 * Enables or disables data export
 */
void Export::setExportEnabled(const bool enabled)
{
    m_exportEnabled = enabled;
    emit enabledChanged();

    if (!exportEnabled() && isOpen())
    {
        m_frames.clear();
        closeFile();
    }
}

/**
 * Write all remaining JSON frames & close the CSV file
 */
void Export::closeFile()
{
    if (isOpen())
    {
        while (m_frames.count())
            writeValues();

        m_csvFile.close();
        m_textStream.setDevice(Q_NULLPTR);

        emit openChanged();
    }
}

/**
 * Creates a CSV file based on the JSON frames contained in the JSON list.
 * @note This function is called periodically every 1 second.
 */
void Export::writeValues()
{
    // Get separator sequence
    const auto sep = IO::Manager::getInstance()->separatorSequence();

    // Write each frame
    for (int i = 0; i < m_frames.count(); ++i)
    {
        auto frame = m_frames.at(i);
        auto fields = QString::fromUtf8(frame.data).split(sep);

        // File not open, create it & add cell titles
        if (!isOpen() && exportEnabled())
            createCsvFile(frame);

        // Write RX date/time
        m_textStream << frame.rxDateTime.toString("yyyy/MM/dd/ HH:mm:ss::zzz") << ",";

        // Write frame data
        for (int j = 0; j < fields.count(); ++j)
        {
            m_textStream << fields.at(j);
            if (j < fields.count() - 1)
                m_textStream << ",";
            else
                m_textStream << "\n";
        }
    }

    // Clear frames
    m_frames.clear();
}

/**
 * Creates a new CSV file corresponding to the current project title & field count
 */
void Export::createCsvFile(const RawFrame &frame)
{
    // Get project title
    const auto projectTitle = UI::Dashboard::getInstance()->title();

    // Get file name
    const QString fileName = frame.rxDateTime.toString("HH-mm-ss") + ".csv";

    // Get path
    // clang-format off
    const QString format = frame.rxDateTime.toString("yyyy/MMM/dd/");
    const QString path = QString("%1/Documents/%2/CSV/%3/%4").arg(QDir::homePath(),
                                                                  qApp->applicationName(),
                                                                  projectTitle, format);
    // clang-format on

    // Generate file path if required
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");

    // Open file
    m_csvFile.setFileName(dir.filePath(fileName));
    if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Misc::Utilities::showMessageBox(tr("CSV File Error"),
                                        tr("Cannot open CSV file for writing!"));
        closeFile();
        return;
    }

    // Add cell titles & force UTF-8 codec
    m_textStream.setDevice(&m_csvFile);
    m_textStream.setGenerateByteOrderMark(true);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_textStream.setCodec("UTF-8");
#else
    m_textStream.setEncoding(QStringConverter::Utf8);
#endif

    // Get number of fields
    const auto sep = IO::Manager::getInstance()->separatorSequence();
    const auto fields = QString::fromUtf8(frame.data).split(sep);

    // Add table titles
    m_textStream << "RX Date/Time,";
    for (int j = 0; j < fields.count(); ++j)
    {
        m_textStream << "Field " << j + 1;

        if (j < fields.count() - 1)
            m_textStream << ",";
        else
            m_textStream << "\n";
    }

    // Update UI
    emit openChanged();
}

/**
 * Appends the latest data from the device to the output buffer
 */
void Export::registerFrame(const QByteArray &data)
{
    // Ignore if device is not connected (we don't want to generate a CSV file when we
    // are reading another CSV file don't we?)
    if (!IO::Manager::getInstance()->connected())
        return;

    // Ignore if CSV export is disabled
    if (!exportEnabled())
        return;

    // Register raw frame to list
    RawFrame frame;
    frame.data = data;
    frame.rxDateTime = QDateTime::currentDateTime();
    m_frames.append(frame);
}
}
