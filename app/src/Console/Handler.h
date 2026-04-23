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
#include <QList>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <unordered_map>

#include "IO/CircularBuffer.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

namespace Console {
/**
 * @brief Receives device data and formats it for the console text view.
 */
class Handler : public QObject {
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
  Q_PROPERTY(bool vt100Emulation
             READ vt100Emulation
             WRITE setVt100Emulation
             NOTIFY vt100EmulationChanged)
  Q_PROPERTY(bool ansiColors
             READ ansiColors
             WRITE setAnsiColors
             NOTIFY ansiColorsChanged)
  Q_PROPERTY(bool imageWidgetActive
             READ imageWidgetActive
             NOTIFY imageWidgetActiveChanged)
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
  Q_PROPERTY(int encoding
             READ encoding
             WRITE setEncoding
             NOTIFY encodingChanged)
  Q_PROPERTY(QStringList textEncodings
             READ textEncodings
             NOTIFY languageChanged)
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
  Q_PROPERTY(int fontFamilyIndex
             READ fontFamilyIndex
             NOTIFY fontFamilyChanged)
  Q_PROPERTY(QStringList availableFonts
             READ availableFonts
             CONSTANT)
  Q_PROPERTY(int defaultCharWidth
             READ defaultCharWidth
             CONSTANT)
  Q_PROPERTY(int defaultCharHeight
             READ defaultCharHeight
             CONSTANT)
  Q_PROPERTY(int currentDeviceId
             READ currentDeviceId
             WRITE setCurrentDeviceId
             NOTIFY currentDeviceIdChanged)
  Q_PROPERTY(QStringList deviceNames
             READ deviceNames
             NOTIFY deviceNamesChanged)
  Q_PROPERTY(bool multiDeviceMode
             READ multiDeviceMode
             NOTIFY deviceNamesChanged)
  // clang-format on

signals:
  void cleared();
  void echoChanged();
  void fontChanged();
  void fontSizeChanged();
  void dataModeChanged();
  void encodingChanged();
  void languageChanged();
  void ansiColorsChanged();
  void lineEndingChanged();
  void fontFamilyChanged();
  void displayModeChanged();
  void historyItemChanged();
  void deviceNamesChanged();
  void textDocumentChanged();
  void showTimestampChanged();
  void checksumMethodChanged();
  void vt100EmulationChanged();
  void currentDeviceIdChanged();
  void ansiColorsEnabledChanged();
  void imageWidgetActiveChanged();
  void displayString(const QString& text);
  void deviceDataReady(int deviceId, const QString& text);

private:
  explicit Handler();
  Handler(Handler&&)                 = delete;
  Handler(const Handler&)            = delete;
  Handler& operator=(Handler&&)      = delete;
  Handler& operator=(const Handler&) = delete;

public:
  enum class DisplayMode {
    DisplayPlainText,
    DisplayHexadecimal
  };
  Q_ENUM(DisplayMode)

  enum class DataMode {
    DataUTF8,
    DataHexadecimal
  };
  Q_ENUM(DataMode)

  enum class LineEnding {
    NoLineEnding,
    NewLine,
    CarriageReturn,
    BothNewLineAndCarriageReturn
  };
  Q_ENUM(LineEnding)

  [[nodiscard]] static Handler& instance();

  [[nodiscard]] bool echo() const;
  [[nodiscard]] bool showTimestamp() const;
  [[nodiscard]] bool ansiColorsEnabled() const;
  [[nodiscard]] bool vt100Emulation() const;
  [[nodiscard]] bool ansiColors() const;
  [[nodiscard]] bool imageWidgetActive() const;
  [[nodiscard]] int checksumMethod() const;

  [[nodiscard]] DataMode dataMode() const;
  [[nodiscard]] LineEnding lineEnding() const;
  [[nodiscard]] DisplayMode displayMode() const;
  [[nodiscard]] int encoding() const;
  [[nodiscard]] QString currentHistoryString() const;

  [[nodiscard]] QStringList dataModes() const;
  [[nodiscard]] QStringList lineEndings() const;
  [[nodiscard]] QStringList displayModes() const;
  [[nodiscard]] QStringList checksumMethods() const;
  [[nodiscard]] QStringList textEncodings() const;

  [[nodiscard]] QFont font() const;
  [[nodiscard]] int fontSize() const;
  [[nodiscard]] QString fontFamily() const;
  [[nodiscard]] int fontFamilyIndex() const;
  [[nodiscard]] QStringList availableFonts() const;

  [[nodiscard]] int defaultCharWidth() const;
  [[nodiscard]] int defaultCharHeight() const;
  [[nodiscard]] qsizetype bufferLength() const;

  [[nodiscard]] int currentDeviceId() const noexcept;
  [[nodiscard]] bool multiDeviceMode() const noexcept;
  [[nodiscard]] const QStringList& deviceNames() const noexcept;

  [[nodiscard]] Q_INVOKABLE bool validateUserHex(const QString& text);
  [[nodiscard]] Q_INVOKABLE QString formatUserHex(const QString& text);

public slots:
  void clear();
  void historyUp();
  void historyDown();
  void setupExternalConnections();
  void send(const QString& data);
  void setEcho(const bool enabled);
  void setFontSize(const int size);
  void setChecksumMethod(const int method);
  void setFontFamily(const QString& family);
  void setShowTimestamp(const bool enabled);
  void setAnsiColorsEnabled(const bool enabled);
  void setVt100Emulation(const bool enabled);
  void setAnsiColors(const bool enabled);
  void setDataMode(const Console::Handler::DataMode& mode);
  void setLineEnding(const Console::Handler::LineEnding& mode);
  void setDisplayMode(const Console::Handler::DisplayMode& mode);
  void setEncoding(const int encoding);
  void append(const QString& str, const bool addTimestamp = false);

  void setCurrentDeviceId(int deviceId);
  void setCurrentDeviceIndex(int index);

  void displaySentData(QByteArrayView data);
  void displaySentData(int deviceId, QByteArrayView data);
  void displayDebugData(const QString& data);
  void hotpathRxData(const IO::ByteArrayPtr& data);
  void hotpathRxDeviceData(int deviceId, const IO::ByteArrayPtr& data);

private slots:
  void updateFont();
  void onDevicesChanged();
  void addToHistory(const QString& command);

private:
  struct DeviceConsoleState {
    QString buffer;
    bool isStartingLine = true;
    bool lastCharWasCR  = false;
  };

  bool hasImageWidget() const;
  QString dataToString(QByteArrayView data);
  QString plainTextStr(QByteArrayView data);
  QString hexadecimalStr(QByteArrayView data);
  void appendToDevice(int deviceId, const QString& str, bool addTimestamp);

private:
  DataMode m_dataMode;
  LineEnding m_lineEnding;
  DisplayMode m_displayMode;
  SerialStudio::TextEncoding m_encoding;

  int m_historyItem;
  int m_checksumMethod;

  bool m_echo;
  bool m_showTimestamp;
  bool m_ansiColorsEnabled;
  bool m_vt100Emulation;
  bool m_ansiColors;
  bool m_isStartingLine;
  bool m_lastCharWasCR;

  int m_currentDeviceId;
  QList<int> m_deviceSourceIds;
  QStringList m_deviceNames;
  std::unordered_map<int, DeviceConsoleState> m_deviceState;

  QFont m_font;
  int m_fontSize;
  int m_fontFamilyIndex;
  QString m_fontFamily;
  QSettings m_settings;

  QStringList m_historyItems;
  IO::CircularBuffer<QByteArray, char> m_textBuffer;

  QString m_pendingDisplay;
};
}  // namespace Console
