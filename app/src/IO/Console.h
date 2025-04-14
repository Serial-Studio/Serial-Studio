/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  Q_PROPERTY(int scrollback
             READ scrollback
             WRITE setScrollback
             NOTIFY scrollbackChanged)
  // clang-format on

signals:
  void echoChanged();
  void dataModeChanged();
  void languageChanged();
  void scrollbackChanged();
  void lineEndingChanged();
  void displayModeChanged();
  void historyItemChanged();
  void textDocumentChanged();
  void showTimestampChanged();
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
  [[nodiscard]] bool showTimestamp() const;

  [[nodiscard]] int scrollback() const;
  [[nodiscard]] DataMode dataMode() const;
  [[nodiscard]] LineEnding lineEnding() const;
  [[nodiscard]] DisplayMode displayMode() const;
  [[nodiscard]] QString currentHistoryString() const;

  [[nodiscard]] QStringList dataModes() const;
  [[nodiscard]] QStringList lineEndings() const;
  [[nodiscard]] QStringList displayModes() const;

  Q_INVOKABLE bool validateUserHex(const QString &text);
  Q_INVOKABLE QString formatUserHex(const QString &text);

  static QByteArray hexToBytes(const QString &data);

public slots:
  void clear();
  void historyUp();
  void historyDown();
  void setupExternalConnections();
  void send(const QString &data);
  void setEcho(const bool enabled);
  void setScrollback(const int lines);
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
  QString dataToString(const QByteArray &data);
  QString plainTextStr(const QByteArray &data);
  QString hexadecimalStr(const QByteArray &data);

private:
  DataMode m_dataMode;
  LineEnding m_lineEnding;
  DisplayMode m_displayMode;

  int m_scrollback;
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
