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

#include "Console.h"
#include "Manager.h"

#include <QFile>
#include <Logger.h>
#include <QTextCodec>
#include <QFileDialog>
#include <Misc/Utilities.h>
#include <ConsoleAppender.h>

using namespace IO;
static Console *INSTANCE = nullptr;

/**
 * Set maximum scrollback to 10'000 lines
 */
static const int SCROLLBACK = 10 * 1000;

/**
 * Constructor function
 */
Console::Console()
    : m_dataMode(DataMode::DataUTF8)
    , m_lineEnding(LineEnding::NoLineEnding)
    , m_displayMode(DisplayMode::DisplayPlainText)
    , m_historyItem(0)
    , m_echo(false)
    , m_autoscroll(true)
    , m_showTimestamp(true)
    , m_timestampAdded(false)
{
    clear();
    auto m = Manager::getInstance();
    connect(m, &Manager::dataReceived, this, &Console::onDataReceived);
    LOG_INFO() << "Class initialized";
}

/**
 * Returns the only instance of the class
 */
Console *Console::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Console;

    return INSTANCE;
}

/**
 * Returns @c true if the console shall display the commands that the user has sent
 * to the serial/network device.
 */
bool Console::echo() const
{
    return m_echo;
}

/**
 * Returns @c true if the vertical position of the console display shall be automatically
 * moved to show latest data.
 */
bool Console::autoscroll() const
{
    return m_autoscroll;
}

/**
 * Returns @c true if data buffer contains information that the user can export.
 */
bool Console::saveAvailable() const
{
    return lineCount() > 0;
}

/**
 * Returns @c true if a timestamp should be shown before each displayed data block.
 */
bool Console::showTimestamp() const
{
    return m_showTimestamp;
}

/**
 * Returns the type of data that the user inputs to the console. There are two possible
 * values:
 * - @c DataMode::DataUTF8        the user is sending data formated in the UTF-8 codec.
 * - @c DataMode::DataHexadecimal the user is sending binary data represented in
 *                                hexadecimal format, we must do a conversion to obtain
 *                                and send the appropiate binary data to the target
 *                                device.
 */
Console::DataMode Console::dataMode() const
{
    return m_dataMode;
}

/**
 * Returns the line ending character that is added to each datablock sent by the user.
 * Possible values are:
 * - @c LineEnding::NoLineEnding                  leave data as-it-is
 * - @c LineEnding::NewLine,                      add '\n' to the data sent by the user
 * - @c LineEnding::CarriageReturn,               add '\r' to the data sent by the user
 * - @c LineEnding::BothNewLineAndCarriageReturn  add '\r\n' to the data sent by the user
 */
Console::LineEnding Console::lineEnding() const
{
    return m_lineEnding;
}

/**
 * Returns the display format of the console. Posible values are:
 * - @c DisplayMode::DisplayPlainText   display incoming data as an UTF-8 stream
 * - @c DisplayMode::DisplayHexadecimal display incoming data in hexadecimal format
 */
Console::DisplayMode Console::displayMode() const
{
    return m_displayMode;
}

/**
 * Returns the current command history string selected by the user.
 *
 * @note the user can navigate through sent commands using the Up/Down keys on the
 *       keyboard. This behaviour is managed by the @c historyUp() & @c historyDown()
 *       functions.
 */
QString Console::currentHistoryString() const
{
    if (m_historyItem < m_historyItems.count() && m_historyItem >= 0)
        return m_historyItems.at(m_historyItem);

    return "";
}

/**
 * Returns the number of lines of the log file
 */
int Console::lineCount() const
{
    return m_data.count();
}

/**
 * Returns the string at the given @a line of the log file
 */
QString Console::getLine(const int line) const
{
    if (line < lineCount())
        return m_data.at(line);

    return "";
}

/**
 * Returns a list with the available data (sending) modes. This list must be synchronized
 * with the order of the @c DataMode enums.
 */
QStringList Console::dataModes() const
{
    QStringList list;
    list.append(tr("ASCII"));
    list.append(tr("HEX"));
    return list;
}

/**
 * Returns a list with the available line endings options. This list must be synchronized
 * with the order of the @c LineEnding enums.
 */
QStringList Console::lineEndings() const
{
    QStringList list;
    list.append(tr("No line ending"));
    list.append(tr("New line"));
    list.append(tr("Carriage return"));
    list.append(tr("NL + CR"));
    return list;
}

/**
 * Returns a list with the available console display modes. This list must be synchronized
 * with the order of the @c DisplayMode enums.
 */
QStringList Console::displayModes() const
{
    QStringList list;
    list.append(tr("Plain text"));
    list.append(tr("Hexadecimal"));
    return list;
}

/**
 * Allows the user to export the information displayed on the console
 */
void Console::save()
{
    // No data buffer received, abort
    if (!saveAvailable())
        return;

    // Get file name
    auto path
        = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Export console data"),
                                       QDir::homePath(), tr("Text files") + " (*.txt)");

    // Create file
    if (!path.isEmpty())
    {
        QFile file(path);
        if (file.open(QFile::WriteOnly))
        {
            QByteArray data;
            for (int i = 0; i < lineCount(); ++i)
            {
                data.append(getLine(i).toUtf8());
                data.append("\r");
                data.append("\n");
            }

            file.write(data);
            file.close();

            Misc::Utilities::revealFile(path);
        }

        else
            Misc::Utilities::showMessageBox(tr("File save error"), file.errorString());
    }
}

/**
 * Deletes all the text displayed by the current QML text document
 */
void Console::clear()
{
    m_data.clear();
    m_data.reserve(SCROLLBACK);
    emit dataReceived();
}

/**
 * Comamnds sent by the user are stored in a @c QStringList, in which the first items
 * are the oldest commands.
 *
 * The user can navigate the list using the up/down keys. This function allows the user
 * to navigate the list from most recent command to oldest command.
 */
void Console::historyUp()
{
    if (m_historyItem > 0)
    {
        --m_historyItem;
        emit historyItemChanged();
    }
}

/**
 * Comamnds sent by the user are stored in a @c QStringList, in which the first items
 * are the oldest commands.
 *
 * The user can navigate the list using the up/down keys. This function allows the user
 * to navigate the list from oldst command to most recent command.
 */
void Console::historyDown()
{
    if (m_historyItem < m_historyItems.count() - 1)
    {
        ++m_historyItem;
        emit historyItemChanged();
    }
}

/**
 * Sends the given @a data to the currently connected device using the options specified
 * by the user with the rest of the functions of this class.
 *
 * @note @c data is added to the history of sent commands, regardless if the data writing
 *       was successfull or not.
 */
void Console::send(const QString &data)
{
    // Check conditions
    if (data.isEmpty() || !Manager::getInstance()->connected())
        return;

    // Add user command to history
    addToHistory(data);

    // Convert data to byte array
    QByteArray bin;
    if (dataMode() == DataMode::DataHexadecimal)
        bin = hexToBytes(data);
    else
        bin = data.toUtf8();

    // Add EOL character
    switch (lineEnding())
    {
        case LineEnding::NoLineEnding:
            break;
        case LineEnding::NewLine:
            bin.append("\n");
            break;
        case LineEnding::CarriageReturn:
            bin.append("\r");
            break;
        case LineEnding::BothNewLineAndCarriageReturn:
            bin.append("\r");
            bin.append("\n");
            break;
    }

    // Write data to device
    auto bytes = Manager::getInstance()->writeData(bin);

    // Write success, notify UI & log bytes written
    if (bytes > 0)
    {
        // Get sent byte array
        auto sent = bin;
        sent.chop(bin.length() - bytes);

        // Display sent data on console (if allowed)
        if (echo())
        {
            append(dataToString(bin), showTimestamp());
            m_timestampAdded = false;
        }
    }

    // Write error
    else
        LOG_WARNING() << Manager::getInstance()->device()->errorString();
}

/**
 * Enables or disables displaying sent data in the console screen. See @c echo() for more
 * information.
 */
void Console::setEcho(const bool enabled)
{
    m_echo = enabled;
    emit echoChanged();
}

/**
 * Changes the data mode for user commands. See @c dataMode() for more information.
 */
void Console::setDataMode(const DataMode mode)
{
    m_dataMode = mode;
    emit dataModeChanged();
}

/**
 * Enables/disables displaying a timestamp of each received data block.
 */
void Console::setShowTimestamp(const bool enabled)
{
    m_showTimestamp = enabled;
    emit showTimestampChanged();
}

/**
 * Enables/disables autoscrolling of the console text.
 */
void Console::setAutoscroll(const bool enabled)
{
    m_autoscroll = enabled;
    emit autoscrollChanged();
}

/**
 * Changes line ending mode for sent user commands. See @c lineEnding() for more
 * information.
 */
void Console::setLineEnding(const LineEnding mode)
{
    m_lineEnding = mode;
    emit lineEndingChanged();
}

/**
 * Changes the display mode of the console. See @c displayMode() for more information.
 */
void Console::setDisplayMode(const DisplayMode mode)
{
    m_displayMode = mode;
    emit displayModeChanged();
}

/**
 * Inserts the given @a string into the list of lines of the console, if @a addTimestamp
 * is set to @c true, an timestamp is added for each line.
 */
void Console::append(const QString &string, const bool addTimestamp)
{
    // Get current date
    QString timestamp;
    if (addTimestamp)
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        timestamp = dateTime.toString("HH:mm:ss.zzz -> ");
    }

    // Change CR + NL to new line
    QString data = string;
    data = data.replace("\r\n", "\n");

    // Add first item if necessary
    if (lineCount() == 0)
        m_data.append("");

    // Construct string to insert
    QString str;
    for (int i = 0; i < data.length(); ++i)
    {
        if (!m_timestampAdded)
        {
            str = m_data.last();
            str.append(timestamp);
            m_data.replace(lineCount() - 1, str);
            m_timestampAdded = true;
        }

        if (string.at(i) == "\n" || string.at(i) == "\r")
        {
            m_data.append("");
            m_timestampAdded = false;
        }

        else
        {
            str = m_data.last();
            str.append(data.at(i));
            m_data.replace(lineCount() - 1, str);
        }
    }

    // Remove extra lines
    while (lineCount() > SCROLLBACK)
        m_data.removeFirst();

    // Update UI
    emit dataReceived();
}

/**
 * Displays the given @a data in the console. @c QByteArray to ~@c QString conversion is
 * done by the @c dataToString() function, which displays incoming data either in UTF-8
 * or in hexadecimal mode.
 */
void Console::onDataReceived(const QByteArray &data)
{
    append(dataToString(data), showTimestamp());
}

/**
 * Registers the given @a command to the list of sent commands.
 */
void Console::addToHistory(const QString &command)
{
    // Remove old commands from history
    while (m_historyItems.count() > 100)
        m_historyItems.removeFirst();

    // Register command
    m_historyItems.append(command);
    m_historyItem = m_historyItems.count();
    emit historyItemChanged();
}

/**
 * Converts the given @a data in HEX format into real binary data.
 */
QByteArray Console::hexToBytes(const QString &data)
{
    QString withoutSpaces = data;
    withoutSpaces.replace(" ", "");

    QByteArray array;
    for (int i = 0; i < withoutSpaces.length(); i += 2)
    {
        auto chr1 = withoutSpaces.at(i);
        auto chr2 = withoutSpaces.at(i + 1);
        auto byte = QString("%1%2").arg(chr1, chr2).toInt(nullptr, 16);
        array.append(byte);
    }

    return array;
}

/**
 * Converts the given @a data to a string according to the console display mode set by the
 * user.
 */
QString Console::dataToString(const QByteArray &data)
{
    switch (displayMode())
    {
        case DisplayMode::DisplayPlainText:
            return plainTextStr(data);
            break;
        case DisplayMode::DisplayHexadecimal:
            return hexadecimalStr(data);
            break;
        default:
            return "";
            break;
    }
}

/**
 * Converts the given @a data into an UTF-8 string
 */
QString Console::plainTextStr(const QByteArray &data)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    return codec->toUnicode(data);
}

/**
 * Converts the given @a data into a HEX representation string.
 */
QString Console::hexadecimalStr(const QByteArray &data)
{
    // Convert to hex string with spaces between bytes
    QString str;
    QString hex = QString::fromUtf8(data.toHex());
    for (int i = 0; i < hex.length(); ++i)
    {
        str.append(hex.at(i));
        if ((i + 1) % 2 == 0)
            str.append(" ");
    }

    // Add new line & carriage returns
    str.replace("0a", "0a\r");
    str.replace("0d", "0d\n");

    // Return string
    return str;
}
