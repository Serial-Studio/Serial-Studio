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

#include <Logger.h>
#include <QTextCodec>
#include <ConsoleAppender.h>

using namespace IO;
static Console *INSTANCE = nullptr;

Console::Console()
    : m_dataMode(DataMode::DataUTF8)
    , m_lineEnding(LineEnding::NoLineEnding)
    , m_displayMode(DisplayMode::DisplayPlainText)
    , m_historyItem(0)
    , m_echo(false)
    , m_autoscroll(true)
    , m_showTimestamp(true)
    , m_dateAdded(false)
    , m_cursor(nullptr)
    , m_document(nullptr)
{
    auto m = Manager::getInstance();
    connect(m, &Manager::dataReceived, this, &Console::onDataReceived);
}

Console *Console::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Console;

    return INSTANCE;
}

bool Console::echo() const
{
    return m_echo;
}

bool Console::autoscroll() const
{
    return m_autoscroll;
}

bool Console::showTimestamp() const
{
    return m_showTimestamp;
}

Console::DataMode Console::dataMode() const
{
    return m_dataMode;
}

Console::LineEnding Console::lineEnding() const
{
    return m_lineEnding;
}

Console::DisplayMode Console::displayMode() const
{
    return m_displayMode;
}

QString Console::currentHistoryString() const
{
    return m_historyItems.at(m_historyItem);
}

QQuickTextDocument *Console::document()
{
    return m_document;
}

QStringList Console::dataModes() const
{
    QStringList list;
    list.append(tr("ASCII"));
    list.append(tr("HEX"));
    return list;
}

QStringList Console::lineEndings() const
{
    QStringList list;
    list.append(tr("No line ending"));
    list.append(tr("New line"));
    list.append(tr("Carriage return"));
    list.append(tr("NL + CR"));
    return list;
}

QStringList Console::displayModes() const
{
    QStringList list;
    list.append(tr("Plain text"));
    list.append(tr("Hexadecimal"));
    return list;
}

void Console::copy() { }

void Console::save() { }

void Console::clear()
{
    if (document())
        document()->textDocument()->clear();
}

void Console::historyUp()
{
    if (m_historyItem < m_historyItems.count() - 1)
    {
        ++m_historyItem;
        emit historyItemChanged();
    }
}

void Console::historyDown()
{
    if (m_historyItem > 0)
    {
        --m_historyItem;
        emit historyItemChanged();
    }
}

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
            append(dataToString(bin));
            m_dateAdded = false;
            if (m_cursor)
                m_cursor->insertBlock();
        }
    }

    // Write error
    else
        LOG_WARNING() << Manager::getInstance()->device()->errorString();
}

void Console::setEcho(const bool enabled)
{
    m_echo = enabled;
    emit echoChanged();
}

void Console::setDataMode(const DataMode mode)
{
    m_dataMode = mode;
    emit dataModeChanged();
}

void Console::setShowTimestamp(const bool enabled)
{
    m_showTimestamp = enabled;
    emit showTimestampChanged();
}

void Console::setAutoscroll(const bool enabled)
{
    m_autoscroll = enabled;
    emit autoscrollChanged();

    if (document())
        document()->textDocument()->setMaximumBlockCount(autoscroll() ? 500 : 0);
}

void Console::setLineEnding(const LineEnding mode)
{
    m_lineEnding = mode;
    emit lineEndingChanged();
}

void Console::setDisplayMode(const DisplayMode mode)
{
    m_displayMode = mode;
    emit displayModeChanged();
}

void Console::setTextDocument(QQuickTextDocument *document)
{
    // Delete previous text cursor
    if (m_cursor)
        delete m_cursor;

    // Re-assign pointer & register text cursor
    m_document = document;
    m_cursor = new QTextCursor(m_document->textDocument());
    m_cursor->movePosition(QTextCursor::End);

    // Configure text doucment
    m_document->textDocument()->setMaximumBlockCount(autoscroll() ? 500 : 0);
    m_document->textDocument()->setUndoRedoEnabled(false);
    emit textDocumentChanged();
}

void Console::onDataReceived(const QByteArray &data)
{
    append(dataToString(data));
}

void Console::addToHistory(const QString &history)
{
    // Remove old commands from history
    while (m_historyItems.count() > 100)
        m_historyItems.removeFirst();

    // Register command
    m_historyItems.append(history);
}

void Console::append(const QString &str)
{
    if (m_cursor)
    {
        // Replace line breaks with <br>
        QString text;

        // Get current date
        QDateTime dateTime = QDateTime::currentDateTime();
        auto now = dateTime.toString("HH:mm:ss.zzz -> ");
        if (!showTimestamp())
            now = "";

        // Change all line breaks to "\n"
        QString data = str;
        data = data.replace(QRegExp("\r?\n"), QChar('\n'));

        // Construct string to insert
        for (int i = 0; i < data.length(); ++i)
        {
            if (!m_dateAdded)
            {
                text.append(now);
                m_dateAdded = true;
            }

            if (str.at(i) == "\n")
            {
                text.append("\n");
                m_dateAdded = false;
            }

            else
                text.append(data.at(i));
        }

        // Insert text on console
        m_cursor->insertText(text);
    }
}

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

QString Console::plainTextStr(const QByteArray &data)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    return codec->toUnicode(data);
}

QString Console::hexadecimalStr(const QByteArray &data)
{
    QString str;
    QString hex = QString::fromUtf8(data.toHex());
    for (int i = 0; i < hex.length(); ++i)
    {
        str.append(hex.at(i));
        if ((i + 1) % 2 == 0)
            str.append(" ");
    }

    return str;
}
