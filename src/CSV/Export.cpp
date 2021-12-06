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
#include <JSON/Generator.h>
#include <JSON/FrameInfo.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>

#include <QDir>
#include <QUrl>
#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
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
    auto io = IO::Manager::getInstance();
    auto ge = JSON::Generator::getInstance();
    auto te = Misc::TimerEvents::getInstance();
    connect(io, &IO::Manager::connectedChanged, this, &Export::closeFile);
    connect(te, &Misc::TimerEvents::lowFreqTimeout, this, &Export::writeValues);
    connect(ge, &JSON::Generator::jsonChanged, this, &Export::registerFrame);
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
        m_jsonList.clear();
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
        while (m_jsonList.count())
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
    // Sort JSON frames so that they are ordered from least-recent to most-recent
    JFI_SortList(&m_jsonList);

    // Export JSON frames
    for (int k = 0; k < m_jsonList.count(); ++k)
    {
        // Get project title & cell values
        auto dateTime = m_jsonList.first().rxDateTime;
        auto json = m_jsonList.first().jsonDocument.object();
        auto projectTitle = JFI_Value(json, "title", "t").toString();

        // Validate JSON & title
        if (json.isEmpty() || projectTitle.isEmpty())
        {
            m_jsonList.removeFirst();
            break;
        }

        // Get cell titles & values
        StringList titles;
        StringList values;
        auto groups = JFI_Value(json, "groups", "g").toArray();
        for (int i = 0; i < groups.count(); ++i)
        {
            // Get group & dataset array
            auto group = groups.at(i).toObject();
            auto datasets = JFI_Value(group, "datasets", "d").toArray();
            if (group.isEmpty() || datasets.isEmpty())
                continue;

            // Get group title
            auto groupTitle = JFI_Value(group, "title", "t").toVariant().toString();

            // Get dataset titles & values
            for (int j = 0; j < datasets.count(); ++j)
            {
                // Get dataset object & fields
                auto dataset = datasets.at(j).toObject();
                auto datasetTitle = JFI_Value(dataset, "title", "t").toString();
                auto datasetUnits = JFI_Value(dataset, "units", "u").toString();
                auto datasetValue = JFI_Value(dataset, "value", "v").toString();

                // Construct dataset title from group, dataset title & units
                QString title;
                if (datasetUnits.isEmpty())
                    title = QString("(%1) %2").arg(groupTitle, datasetTitle);
                else
                    title = QString("(%1) %2 [%3]")
                                .arg(groupTitle, datasetTitle, datasetUnits);

                // Add dataset title & value to lists
                titles.append(title);
                values.append(datasetValue);
            }
        }

        // Abort if cell titles are empty
        if (titles.isEmpty())
        {
            m_jsonList.removeFirst();
            break;
        }

        // Prepend current time
        titles.prepend("RX Date/Time");
        values.prepend(dateTime.toString("yyyy/MM/dd/ HH:mm:ss::zzz"));

        // File not open, create it & add cell titles
        if (!isOpen() && exportEnabled())
        {
            // Get file name and path
            QString format = dateTime.toString("yyyy/MMM/dd/");
            QString fileName = dateTime.toString("HH-mm-ss") + ".csv";
            QString path = QString("%1/Documents/%2/CSV/%3/%4")
                               .arg(QDir::homePath(), qApp->applicationName(),
                                    projectTitle, format);

            // Generate file path if required
            QDir dir(path);
            if (!dir.exists())
                dir.mkpath(".");

            // Open file
            m_csvFile.setFileName(dir.filePath(fileName));
            if (!m_csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QMessageBox::critical(Q_NULLPTR, tr("CSV File Error"),
                                      tr("Cannot open CSV file for writing!"),
                                      QMessageBox::Ok);
                closeFile();
                return;
            }

            // Add cell titles & force UTF-8 codec
            m_textStream.setDevice(&m_csvFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            m_textStream.setCodec("UTF-8");
#else
            m_textStream.setEncoding(QStringConverter::Utf8);
#endif
            m_textStream.setGenerateByteOrderMark(true);
            for (int i = 0; i < titles.count(); ++i)
            {
                m_textStream << titles.at(i).toUtf8();
                if (i < titles.count() - 1)
                    m_textStream << ",";
                else
                    m_textStream << "\n";
            }

            // Update UI
            emit openChanged();
        }

        // Write cell values
        for (int i = 0; i < values.count(); ++i)
        {
            m_textStream << values.at(i).toUtf8();
            if (i < values.count() - 1)
                m_textStream << ",";
            else
                m_textStream << "\n";
        }

        // Remove JSON from list
        m_jsonList.removeFirst();
    }
}

/**
 * Obtains the latest JSON dataframe & appends it to the JSON list, which is later read,
 * sorted and written to the CSV file by the @c writeValues() function.
 */
void Export::registerFrame(const JFI_Object &info)
{
    // Ignore if device is not connected (we don't want to generate a CSV file when we
    // are reading another CSV file don't we?)
    if (!IO::Manager::getInstance()->connected())
        return;

    // Ignore is CSV export is disabled
    if (!exportEnabled())
        return;

    // Update JSON list
    m_jsonList.append(info);
}
}
