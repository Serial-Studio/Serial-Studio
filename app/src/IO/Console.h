/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QObject>
#include "IO/CircularBuffer.h"

namespace IO
{
/**
 * @brief The Console class
 *
 * The console class receives data from the @c IO::Manager class and
 * processes it so that it can be easily appended to a text edit widget.
 *
 * The class also controls various UI-related factors, such as the display
 * format of the data (e.g. ASCII or HEX), history of sent commands and
 * exporting of the RX data.
 */
class Console : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool echo
             READ echo
             WRITE setEcho
             NOTIFY echoChanged)
  Q_PROPERTY(bool showTimestamp
             READ showTimestamp
             WRITE setShowTimestamp
             NOTIFY showTimestampChanged)
  Q_PROPERTY(bool saveAvailable
             READ saveAvailable
             NOTIFY saveAvailableChanged)
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
  Q_PROPERTY(QStringList dataModes
             READ dataModes
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList lineEndings
             READ lineEndings
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList displayModes
             READ displayModes
             NOTIFY languageChanged)
  // clang-format on

signals:
  void echoChanged();
  void dataModeChanged();
  void languageChanged();
  void lineEndingChanged();
  void displayModeChanged();
  void historyItemChanged();
  void textDocumentChanged();
  void showTimestampChanged();
  void saveAvailableChanged();
  void displayString(const QString &text);

private:
  explicit Console();
  Console(Console &&) = delete;
  Console(const Console &) = delete;
  Console &operator=(Console &&) = delete;
  Console &operator=(const Console &) = delete;

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

  static Console &instance();

  [[nodiscard]] bool echo() const;
  [[nodiscard]] bool saveAvailable() const;
  [[nodiscard]] bool showTimestamp() const;

  [[nodiscard]] DataMode dataMode() const;
  [[nodiscard]] LineEnding lineEnding() const;
  [[nodiscard]] DisplayMode displayMode() const;
  [[nodiscard]] QString currentHistoryString() const;

  [[nodiscard]] QStringList dataModes() const;
  [[nodiscard]] QStringList lineEndings() const;
  [[nodiscard]] QStringList displayModes() const;

  Q_INVOKABLE QString formatUserHex(const QString &text);

public slots:
  void save();
  void clear();
  void print();
  void historyUp();
  void historyDown();
  void setupExternalConnections();
  void send(const QString &data);
  void setEcho(const bool enabled);
  void setShowTimestamp(const bool enabled);
  void setDataMode(const IO::Console::DataMode &mode);
  void setLineEnding(const IO::Console::LineEnding &mode);
  void setDisplayMode(const IO::Console::DisplayMode &mode);
  void append(const QString &str, const bool addTimestamp = false);

private slots:
  void onDataSent(const QByteArray &data);
  void addToHistory(const QString &command);
  void onDataReceived(const QByteArray &data);

private:
  QByteArray hexToBytes(const QString &data);
  QString dataToString(const QByteArray &data);
  QString plainTextStr(const QByteArray &data);
  QString hexadecimalStr(const QByteArray &data);

private:
  DataMode m_dataMode;
  LineEnding m_lineEnding;
  DisplayMode m_displayMode;

  int m_historyItem;

  bool m_echo;
  bool m_showTimestamp;
  bool m_isStartingLine;
  bool m_lastCharWasCR;

  QStringList m_historyItems;

  QString m_printFont;
  CircularBuffer<QByteArray, char> m_textBuffer;
};
} // namespace IO
