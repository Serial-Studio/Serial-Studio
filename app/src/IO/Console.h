/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  Q_PROPERTY(QStringList checksumMethods
             READ checksumMethods
             NOTIFY languageChanged)
  Q_PROPERTY(int checksumMethod
             READ checksumMethod
             WRITE setChecksumMethod
             NOTIFY checksumMethodChanged)
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
  void checksumMethodChanged();
  void displayString(QStringView text);

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
  [[nodiscard]] int checksumMethod() const;

  [[nodiscard]] DataMode dataMode() const;
  [[nodiscard]] LineEnding lineEnding() const;
  [[nodiscard]] DisplayMode displayMode() const;
  [[nodiscard]] QString currentHistoryString() const;

  [[nodiscard]] QStringList dataModes() const;
  [[nodiscard]] QStringList lineEndings() const;
  [[nodiscard]] QStringList displayModes() const;
  [[nodiscard]] QStringList checksumMethods() const;

  Q_INVOKABLE bool validateUserHex(const QString &text);
  Q_INVOKABLE QString formatUserHex(const QString &text);

public slots:
  void clear();
  void historyUp();
  void historyDown();
  void setupExternalConnections();
  void send(const QString &data);
  void setEcho(const bool enabled);
  void setChecksumMethod(const int method);
  void setShowTimestamp(const bool enabled);
  void setDataMode(const IO::Console::DataMode &mode);
  void setLineEnding(const IO::Console::LineEnding &mode);
  void setDisplayMode(const IO::Console::DisplayMode &mode);
  void append(const QString &str, const bool addTimestamp = false);

  void hotpathRxData(QByteArrayView data);
  void displaySentData(QByteArrayView data);

private slots:
  void addToHistory(const QString &command);

private:
  QString dataToString(QByteArrayView data);
  QString plainTextStr(QByteArrayView data);
  QString hexadecimalStr(QByteArrayView data);

private:
  DataMode m_dataMode;
  LineEnding m_lineEnding;
  DisplayMode m_displayMode;

  int m_historyItem;
  int m_checksumMethod;

  bool m_echo;
  bool m_showTimestamp;
  bool m_isStartingLine;
  bool m_lastCharWasCR;

  QStringList m_historyItems;
  CircularBuffer<QByteArray, char> m_textBuffer;
};
} // namespace IO
