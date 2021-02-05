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

#ifndef IO_CONSOLE_H
#define IO_CONSOLE_H

#include <QObject>
#include <QWidget>
#include <QTextCursor>
#include <QQuickTextDocument>
#include <QtQuick/QQuickPaintedItem>

namespace IO
{

class Console : public QObject
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(bool echo
               READ echo
               WRITE setEcho
               NOTIFY echoChanged)
    Q_PROPERTY(bool autoscroll
               READ autoscroll
               WRITE setAutoscroll
               NOTIFY autoscrollChanged)
    Q_PROPERTY(bool showTimestamp
               READ showTimestamp
               WRITE setShowTimestamp
               NOTIFY showTimestampChanged)
    Q_PROPERTY(bool saveAvailable
               READ saveAvailable
               NOTIFY dataReceived)
    Q_PROPERTY(IO::Console::DataMode dataMode
               READ dataMode
               WRITE setDataMode
               NOTIFY dataModeChanged)
    Q_PROPERTY(IO::Console::LineEnding lineEnding
               READ lineEnding
               WRITE setLineEnding
               NOTIFY lineEndingChanged)
    Q_PROPERTY(IO::Console::DisplayMode displayMode
               READ displayMode
               WRITE setDisplayMode
               NOTIFY displayModeChanged)
    Q_PROPERTY(QString currentHistoryString
               READ currentHistoryString
               NOTIFY historyItemChanged)
    // clang-format on

signals:
    void echoChanged();
    void dataReceived();
    void dataModeChanged();
    void autoscrollChanged();
    void lineEndingChanged();
    void displayModeChanged();
    void historyItemChanged();
    void textDocumentChanged();
    void showTimestampChanged();

public:
    enum class DisplayMode
    {
        DisplayPlainText,
        DisplayHexadecimal
    };
    Q_ENUM(DisplayMode)

    enum class DataMode
    {
        DataUTF8,
        DataHexadecimal
    };
    Q_ENUM(DataMode)

    enum class LineEnding
    {
        NoLineEnding,
        NewLine,
        CarriageReturn,
        BothNewLineAndCarriageReturn
    };
    Q_ENUM(LineEnding)

    static Console *getInstance();

    bool echo() const;
    bool autoscroll() const;
    bool saveAvailable() const;
    bool showTimestamp() const;

    DataMode dataMode() const;
    LineEnding lineEnding() const;
    DisplayMode displayMode() const;
    QString currentHistoryString() const;

    QTextDocument *document();

    Q_INVOKABLE QStringList dataModes() const;
    Q_INVOKABLE QStringList lineEndings() const;
    Q_INVOKABLE QStringList displayModes() const;

public slots:
    void save();
    void clear();
    void historyUp();
    void historyDown();
    void send(const QString &data);
    void setEcho(const bool enabled);
    void setDataMode(const DataMode mode);
    void setAutoscroll(const bool enabled);
    void setShowTimestamp(const bool enabled);
    void setLineEnding(const LineEnding mode);
    void setDisplayMode(const DisplayMode mode);
    void setTextDocument(QQuickTextDocument *document);

private slots:
    void append(const QString &str);
    void addToHistory(const QString &command);
    void onDataReceived(const QByteArray &data);

private:
    Console();
    QByteArray hexToBytes(const QString &data);
    QString dataToString(const QByteArray &data);
    QString plainTextStr(const QByteArray &data);
    QString hexadecimalStr(const QByteArray &data);

private:
    DataMode m_dataMode;
    QByteArray m_dataBuffer;
    LineEnding m_lineEnding;
    DisplayMode m_displayMode;

    int m_historyItem;

    bool m_echo;
    bool m_autoscroll;
    bool m_showTimestamp;
    bool m_timestampAdded;

    QStringList m_historyItems;

    QTextCursor *m_cursor;
    QQuickTextDocument *m_document;
};
}

#endif
