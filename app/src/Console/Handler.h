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

#include <QFont>
#include <QObject>
#include <QSettings>

#include "IO/CircularBuffer.h"
#include "IO/HAL_Driver.h"

namespace Console
{
/**
 * @brief The Console::Handler class
 *
 * The handler class receives data from the @c IO::Manager class and
 * processes it so that it can be easily appended to a text edit widget.
 *
 * The class also controls various UI-related factors, such as the display
 * format of the data (e.g. ASCII or HEX), history of sent commands and
 * exporting of the RX data.
 */
class Handler : public QObject
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
  Q_PROPERTY(bool ansiColorsEnabled
             READ ansiColorsEnabled
             WRITE setAnsiColorsEnabled
             NOTIFY ansiColorsEnabledChanged)
  Q_PROPERTY(Console::Handler::DataMode dataMode
             READ dataMode
             WRITE setDataMode
             NOTIFY dataModeChanged)
  Q_PROPERTY(Console::Handler::LineEnding lineEnding
             READ lineEnding
             WRITE setLineEnding
             NOTIFY lineEndingChanged)
  Q_PROPERTY(Console::Handler::DisplayMode displayMode
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
  Q_PROPERTY(QFont font
             READ font
             NOTIFY fontChanged)
  Q_PROPERTY(QString fontFamily
             READ fontFamily
             WRITE setFontFamily
             NOTIFY fontFamilyChanged)
  Q_PROPERTY(int fontSize
             READ fontSize
             WRITE setFontSize
             NOTIFY fontSizeChanged)
  Q_PROPERTY(QStringList availableFonts
             READ availableFonts
             CONSTANT)
  Q_PROPERTY(int defaultCharWidth
             READ defaultCharWidth
             CONSTANT)
  Q_PROPERTY(int defaultCharHeight
             READ defaultCharHeight
             CONSTANT)
  // clang-format on

signals:
  void echoChanged();
  void fontChanged();
  void fontSizeChanged();
  void dataModeChanged();
  void languageChanged();
  void lineEndingChanged();
  void fontFamilyChanged();
  void displayModeChanged();
  void historyItemChanged();
  void textDocumentChanged();
  void showTimestampChanged();
  void ansiColorsEnabledChanged();
  void checksumMethodChanged();
  void displayString(const QString &text);
  void cleared();

private:
  explicit Handler();
  Handler(Handler &&) = delete;
  Handler(const Handler &) = delete;
  Handler &operator=(Handler &&) = delete;
  Handler &operator=(const Handler &) = delete;

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

  static Handler &instance();

  [[nodiscard]] bool echo() const;
  [[nodiscard]] bool showTimestamp() const;
  [[nodiscard]] bool ansiColorsEnabled() const;
  [[nodiscard]] int checksumMethod() const;

  [[nodiscard]] DataMode dataMode() const;
  [[nodiscard]] LineEnding lineEnding() const;
  [[nodiscard]] DisplayMode displayMode() const;
  [[nodiscard]] QString currentHistoryString() const;

  [[nodiscard]] QStringList dataModes() const;
  [[nodiscard]] QStringList lineEndings() const;
  [[nodiscard]] QStringList displayModes() const;
  [[nodiscard]] QStringList checksumMethods() const;

  [[nodiscard]] QFont font() const;
  [[nodiscard]] int fontSize() const;
  [[nodiscard]] QString fontFamily() const;
  [[nodiscard]] QStringList availableFonts() const;

  [[nodiscard]] int defaultCharWidth() const;
  [[nodiscard]] int defaultCharHeight() const;
  [[nodiscard]] qsizetype bufferLength() const;

  Q_INVOKABLE bool validateUserHex(const QString &text);
  Q_INVOKABLE QString formatUserHex(const QString &text);

public slots:
  void clear();
  void historyUp();
  void historyDown();
  void setupExternalConnections();
  void send(const QString &data);
  void setEcho(const bool enabled);
  void setFontSize(const int size);
  void setChecksumMethod(const int method);
  void setFontFamily(const QString &family);
  void setShowTimestamp(const bool enabled);
  void setAnsiColorsEnabled(const bool enabled);
  void setDataMode(const Console::Handler::DataMode &mode);
  void setLineEnding(const Console::Handler::LineEnding &mode);
  void setDisplayMode(const Console::Handler::DisplayMode &mode);
  void append(const QString &str, const bool addTimestamp = false);

  void displaySentData(QByteArrayView data);
  void displayDebugData(const QString &data);
  void hotpathRxData(const IO::ByteArrayPtr &data);

private slots:
  void updateFont();
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
  bool m_ansiColorsEnabled;
  bool m_isStartingLine;
  bool m_lastCharWasCR;

  QFont m_font;
  int m_fontSize;
  QString m_fontFamily;
  QSettings m_settings;

  QStringList m_historyItems;
  IO::CircularBuffer<QByteArray, char> m_textBuffer;
};
} // namespace Console
