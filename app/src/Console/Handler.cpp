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

#include "Console/Handler.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFontDatabase>
#include <QFontInfo>
#include <QFontMetrics>

#include "AppState.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * Constructor function
 */
Console::Handler::Handler()
  : m_dataMode(DataMode::DataUTF8)
  , m_lineEnding(LineEnding::NoLineEnding)
  , m_displayMode(DisplayMode::DisplayPlainText)
  , m_encoding(SerialStudio::EncUtf8)
  , m_historyItem(0)
  , m_checksumMethod(0)
  , m_echo(true)
  , m_showTimestamp(false)
  , m_ansiColorsEnabled(false)
  , m_vt100Emulation(true)
  , m_ansiColors(true)
  , m_isStartingLine(true)
  , m_lastCharWasCR(false)
  , m_currentDeviceId(-1)
  , m_fontFamilyIndex(0)
  , m_textBuffer(10 * 1024)
{
  // Restore persisted settings
  clear();

  const auto defaultFont = Misc::CommonFonts::instance().monoFont();
  m_fontFamily           = m_settings.value("Console/FontFamily", defaultFont.family()).toString();
  m_fontSize             = m_settings.value("Console/FontSize", defaultFont.pointSize()).toInt();
  m_echo                 = m_settings.value("Console/Echo", true).toBool();
  m_showTimestamp        = m_settings.value("Console/ShowTimestamp", false).toBool();
  m_vt100Emulation       = m_settings.value("Console/VT100Emulation", true).toBool();
  m_ansiColors           = m_settings.value("Console/AnsiColors", true).toBool();
  m_checksumMethod       = m_settings.value("Console/ChecksumMethod", 0).toInt();
  m_dataMode             = static_cast<DataMode>(m_settings.value("Console/DataMode", 0).toInt());
  m_lineEnding  = static_cast<LineEnding>(m_settings.value("Console/LineEnding", 0).toInt());
  m_displayMode = static_cast<DisplayMode>(m_settings.value("Console/DisplayMode", 0).toInt());

  const int encInt = m_settings.value("Console/Encoding", 0).toInt();
  if (encInt >= 0 && encInt <= SerialStudio::EncEucKr)
    m_encoding = static_cast<SerialStudio::TextEncoding>(encInt);

  if (m_fontSize < 6)
    m_fontSize = 6;
  else if (m_fontSize > 72)
    m_fontSize = 72;

  const int checksumCount = IO::availableChecksums().count();
  if (m_checksumMethod < 0 || m_checksumMethod >= checksumCount)
    m_checksumMethod = 0;

  m_ansiColorsEnabled = m_vt100Emulation && m_ansiColors;
  m_fontFamilyIndex   = availableFonts().indexOf(m_fontFamily);

  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [this]() {
    if (!m_pendingDisplay.isEmpty()) {
      Q_EMIT displayString(m_pendingDisplay);
      m_pendingDisplay.clear();
    }
  });

  updateFont();
}

/**
 * Returns the only instance of the class
 */
Console::Handler& Console::Handler::instance()
{
  static Handler singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Status & configuration queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true if the console shall display sent commands
 */
bool Console::Handler::echo() const
{
  return m_echo;
}

/**
 * Returns @c true if a timestamp should be shown before each displayed data
 * block.
 */
bool Console::Handler::showTimestamp() const
{
  return m_showTimestamp;
}

/**
 * Returns @c true if ANSI color codes should be used in console output.
 */
bool Console::Handler::ansiColorsEnabled() const
{
  return m_ansiColorsEnabled;
}

/**
 * Returns @c true if VT-100 terminal emulation is enabled.
 * Always returns @c false when the project contains an image widget,
 * to prevent crashes caused by conflicting terminal escape processing.
 */
bool Console::Handler::vt100Emulation() const
{
  // Disable VT-100 when image widgets are active to prevent crashes
  if (hasImageWidget())
    return false;

  return m_vt100Emulation;
}

/**
 * Returns @c true if ANSI color rendering is enabled.
 * Always returns @c false when the project contains an image widget.
 */
bool Console::Handler::ansiColors() const
{
  // Disable ANSI colors when image widgets are active
  if (hasImageWidget())
    return false;

  return m_ansiColors;
}

/**
 * @brief Returns the index of the currently selected checksum method.
 *
 * The index corresponds to the position in the list returned by
 * checksumMethods(), where 0 typically represents "None".
 *
 * @return The index of the active checksum method.
 */
int Console::Handler::checksumMethod() const
{
  return m_checksumMethod;
}

/**
 * Returns the type of data that the user inputs to the console. There are two
 * possible values:
 * - @c DataMode::DataUTF8
 * - @c DataMode::DataHexadecimal
 */
Console::Handler::DataMode Console::Handler::dataMode() const
{
  return m_dataMode;
}

/**
 * Returns the line ending character that is added to each datablock sent by the
 * user. Possible values are:
 * - @c LineEnding::NoLineEnding
 * - @c LineEnding::NewLine
 * - @c LineEnding::CarriageReturn
 * - @c LineEnding::BothNewLineAndCarriageReturn
 */
Console::Handler::LineEnding Console::Handler::lineEnding() const
{
  return m_lineEnding;
}

/**
 * Returns the display format of the console. Posible values are:
 * - @c DisplayMode::DisplayPlainText   display incoming data as an UTF-8 stream
 * - @c DisplayMode::DisplayHexadecimal display incoming data in hexadecimal
 * format
 */
Console::Handler::DisplayMode Console::Handler::displayMode() const
{
  return m_displayMode;
}

/**
 * @brief Returns the selected text encoding as an int (TextEncoding enum).
 *
 * Returning int instead of the enum keeps the property binding trivial in
 * QML, where the ComboBox `currentIndex` is already an int.
 */
int Console::Handler::encoding() const
{
  return static_cast<int>(m_encoding);
}

/**
 * Returns the current command history string selected by the user.
 *
 * @note the user can navigate through sent commands using the Up/Down keys on
 * the keyboard. This behaviour is managed by the @c historyUp() & @c
 * historyDown() functions.
 */
QString Console::Handler::currentHistoryString() const
{
  // Return the selected history entry, or empty if out of range
  if (m_historyItem < m_historyItems.count() && m_historyItem >= 0)
    return m_historyItems.at(m_historyItem);

  return "";
}

//--------------------------------------------------------------------------------------------------
// Available options lists
//--------------------------------------------------------------------------------------------------

/**
 * Returns a list with the available data (sending) modes. This list must be
 * synchronized with the order of the @c DataMode enums.
 */
QStringList Console::Handler::dataModes() const
{
  QStringList list;
  list.append(tr("ASCII"));
  list.append(tr("HEX"));
  return list;
}

/**
 * Returns a list with the available line endings options. This list must be
 * synchronized with the order of the @c LineEnding enums.
 */
QStringList Console::Handler::lineEndings() const
{
  QStringList list;
  list.append(tr("No Line Ending"));
  list.append(tr("New Line"));
  list.append(tr("Carriage Return"));
  list.append(tr("CR + NL"));
  return list;
}

/**
 * Returns a list with the available console display modes. This list must be
 * synchronized with the order of the @c DisplayMode enums.
 */
QStringList Console::Handler::displayModes() const
{
  QStringList list;
  list.append(tr("Plain Text"));
  list.append(tr("Hexadecimal"));
  return list;
}

/**
 * @brief Returns the list of supported text encodings for QML.
 *
 * Delegates to `SerialStudio::textEncodings()` so the list stays in sync with
 * the central `TextEncoding` enum.  Re-emitted when the language changes so
 * translated labels refresh automatically.
 */
QStringList Console::Handler::textEncodings() const
{
  return SerialStudio::textEncodings();
}

/**
 * @brief Returns a list of supported checksum methods including "None".
 *
 * This list is typically used for populating UI elements such as dropdowns
 * or configuration panels. It includes a "None" option followed by all
 * supported checksum algorithms defined in the IO namespace.
 *
 * @return A QStringList of available checksum method names.
 */
QStringList Console::Handler::checksumMethods() const
{
  // Build method list once, replacing empty entry with "No Checksum"
  static QStringList list;
  if (list.isEmpty()) {
    list            = IO::availableChecksums();
    const int index = list.indexOf(QLatin1String(""));
    if (index >= 0)
      list[index] = tr("No Checksum");
  }

  return list;
}

//--------------------------------------------------------------------------------------------------
// Text formatting & validation
//--------------------------------------------------------------------------------------------------

/**
 * Returns the current console font
 */
QFont Console::Handler::font() const
{
  return m_font;
}

/**
 * Returns the current console font size in points
 */
int Console::Handler::fontSize() const
{
  return m_fontSize;
}

/**
 * Returns the current console font family
 */
QString Console::Handler::fontFamily() const
{
  return m_fontFamily;
}

/**
 * Returns the index of the current font family in the availableFonts() list
 */
int Console::Handler::fontFamilyIndex() const
{
  return m_fontFamilyIndex;
}

/**
 * Returns a list of all available monospace fonts
 */
QStringList Console::Handler::availableFonts() const
{
  // Collect all fixed-pitch fonts, sorted with default mono font first
  QStringList monospaceFonts;
  const auto allFonts = QFontDatabase::families();
  auto defaultFamily  = Misc::CommonFonts::instance().monoFont().family();

  for (const auto& family : allFonts) {
    QFontInfo fontInfo(family);
    if (fontInfo.fixedPitch() && !monospaceFonts.contains(family))
      monospaceFonts.append(family);
  }

  monospaceFonts.sort(Qt::CaseInsensitive);
  const int idx = monospaceFonts.indexOf(defaultFamily);
  if (idx > 0)
    monospaceFonts.move(idx, 0);

  return monospaceFonts;
}

/**
 * Returns the character width for the default monospace font
 */
int Console::Handler::defaultCharWidth() const
{
  const auto defaultFont = Misc::CommonFonts::instance().monoFont();
  const QFontMetrics metrics(defaultFont);
  return metrics.horizontalAdvance("M");
}

/**
 * Returns the character height for the default monospace font
 */
int Console::Handler::defaultCharHeight() const
{
  const auto defaultFont = Misc::CommonFonts::instance().monoFont();
  const QFontMetrics metrics(defaultFont);
  return metrics.height();
}

//--------------------------------------------------------------------------------------------------
// Buffer & history management
//--------------------------------------------------------------------------------------------------

/**
 * Returns the number of bytes stored in the console buffer.
 */
qsizetype Console::Handler::bufferLength() const
{
  return m_textBuffer.size();
}

/**
 * Validates the given @a text to ensure it contains only valid HEX characters
 * and consists of complete byte pairs (even-length string).
 * Returns true if the input is valid, otherwise false.
 */
bool Console::Handler::validateUserHex(const QString& text)
{
  // Strip spaces and validate hex format
  QString cleanText = text.simplified().remove(' ');

  static QRegularExpression hexPattern("^[0-9A-Fa-f]*$");
  if (!hexPattern.match(cleanText).hasMatch())
    return false;

  if (cleanText.length() % 2 != 0)
    return false;

  return true;
}

/**
 * Validates the given @a text and adds space to display the text in a
 * byte-oriented view
 */
QString Console::Handler::formatUserHex(const QString& text)
{
  // Strip non-hex characters and reformat as spaced byte pairs
  static QRegularExpression exp("[^0-9A-Fa-f]");
  QString data = text.simplified().remove(exp);

  QString str;
  for (int i = 0; i < data.length(); ++i) {
    str.append(data.at(i));
    if ((i + 1) % 2 == 0 && i > 0)
      str.append(" ");
  }

  while (str.endsWith(" "))
    str.chop(1);

  return str;
}

/**
 * Deletes all the text displayed by the current QML text document
 */
void Console::Handler::clear()
{
  // Reset the text buffer and line state
  m_textBuffer.clear();
  m_isStartingLine = true;
  m_lastCharWasCR  = false;

  // Clear the current device's per-device state
  auto it = m_deviceState.find(m_currentDeviceId);
  if (it != m_deviceState.end()) {
    it->second.buffer.clear();
    it->second.isStartingLine = true;
    it->second.lastCharWasCR  = false;
  }

  Q_EMIT cleared();
}

/**
 * Comamnds sent by the user are stored in a @c QStringList, in which the first
 * items are the oldest commands.
 *
 * The user can navigate the list using the up/down keys. This function allows
 * the user to navigate the list from most recent command to oldest command.
 */
void Console::Handler::historyUp()
{
  if (m_historyItem > 0) {
    --m_historyItem;
    Q_EMIT historyItemChanged();
  }
}

/**
 * Comamnds sent by the user are stored in a @c QStringList, in which the first
 * items are the oldest commands.
 *
 * The user can navigate the list using the up/down keys. This function allows
 * the user to navigate the list from oldst command to most recent command.
 */
void Console::Handler::historyDown()
{
  if (m_historyItem < m_historyItems.count() - 1) {
    ++m_historyItem;
    Q_EMIT historyItemChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// External module connections
//--------------------------------------------------------------------------------------------------

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void Console::Handler::setupExternalConnections()
{
  // Wire language, project model, and connection signals
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &Console::Handler::languageChanged);

  auto notifyTerminal = [this] {
    Q_EMIT vt100EmulationChanged();
    Q_EMIT ansiColorsChanged();
    Q_EMIT imageWidgetActiveChanged();
  };

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::groupsChanged,
          this,
          notifyTerminal);

  connect(&AppState::instance(), &AppState::operationModeChanged, this, notifyTerminal);

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          notifyTerminal);

  // Rebuild device list when sources or connection state change
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceStructureChanged,
          this,
          &Console::Handler::onDevicesChanged,
          Qt::QueuedConnection);

  connect(&IO::ConnectionManager::instance(),
          &IO::ConnectionManager::connectedChanged,
          this,
          &Console::Handler::onDevicesChanged);
}

//--------------------------------------------------------------------------------------------------
// Data send & receive
//--------------------------------------------------------------------------------------------------

/**
 * Sends the given @a data to the currently isConnected device using the options
 * specified by the user with the rest of the functions of this class.
 *
 * @note @c data is added to the history of sent commands, regardless if the
 * data writing was successfull or not.
 */
void Console::Handler::send(const QString& data)
{
  // Validate connection and record history
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  if (!data.isEmpty())
    addToHistory(data);

  // Encode data according to current mode and user-selected text encoding
  QByteArray bin;
  if (dataMode() == DataMode::DataHexadecimal)
    bin = SerialStudio::hexToBytes(data);
  else
    bin = SerialStudio::encodeText(SerialStudio::resolveEscapeSequences(data), m_encoding);

  switch (lineEnding()) {
    case LineEnding::NoLineEnding:
      break;
    case LineEnding::NewLine:
      bin.append('\n');
      break;
    case LineEnding::CarriageReturn:
      bin.append('\r');
      break;
    case LineEnding::BothNewLineAndCarriageReturn:
      bin.append('\r');
      bin.append('\n');
      break;
  }

  const auto checksums = IO::availableChecksums();
  if (m_checksumMethod >= 0 && m_checksumMethod < checksums.count()) {
    const auto checksumName = checksums.at(m_checksumMethod);
    auto checksum           = IO::checksum(checksumName, bin);
    if (!checksum.isEmpty())
      bin.append(checksum);
  }

  if (!bin.isEmpty()) {
    if (m_currentDeviceId >= 0)
      (void)IO::ConnectionManager::instance().writeDataToDevice(m_currentDeviceId, bin);
    else
      (void)IO::ConnectionManager::instance().writeData(bin);
  }
}

//--------------------------------------------------------------------------------------------------
// Settings modification slots
//--------------------------------------------------------------------------------------------------

/**
 * Enables/disables displaying a timestamp of each received data block.
 */
void Console::Handler::setShowTimestamp(const bool enabled)
{
  if (showTimestamp() != enabled) {
    m_showTimestamp = enabled;
    m_settings.setValue("Console/ShowTimestamp", m_showTimestamp);
    Q_EMIT showTimestampChanged();
  }
}

/**
 * Enables/disables ANSI color codes in console output.
 */
void Console::Handler::setAnsiColorsEnabled(const bool enabled)
{
  if (ansiColorsEnabled() != enabled) {
    m_ansiColorsEnabled = enabled;
    Q_EMIT ansiColorsEnabledChanged();
  }
}

/**
 * Enables/disables VT-100 terminal emulation and persists the setting.
 */
void Console::Handler::setVt100Emulation(const bool enabled)
{
  if (m_vt100Emulation != enabled) {
    m_vt100Emulation = enabled;
    m_settings.setValue("Console/VT100Emulation", m_vt100Emulation);
    Q_EMIT vt100EmulationChanged();
    if (!enabled)
      setAnsiColors(false);
    else
      setAnsiColorsEnabled(m_ansiColors);
  }
}

/**
 * Enables/disables ANSI color rendering and persists the setting.
 */
void Console::Handler::setAnsiColors(const bool enabled)
{
  if (m_ansiColors != enabled) {
    m_ansiColors = enabled;
    m_settings.setValue("Console/AnsiColors", m_ansiColors);
    Q_EMIT ansiColorsChanged();
    setAnsiColorsEnabled(m_vt100Emulation && m_ansiColors);
  }
}

/**
 * Enables/disables showing the sent data on the console
 */
void Console::Handler::setEcho(const bool enabled)
{
  if (echo() != enabled) {
    m_echo = enabled;
    m_settings.setValue("Console/Echo", m_echo);
    Q_EMIT echoChanged();
  }
}

/**
 * Sets the console font size
 */
void Console::Handler::setFontSize(const int size)
{
  const int constrainedSize = qBound(6, size, 72);
  if (m_fontSize != constrainedSize) {
    m_fontSize = constrainedSize;
    m_settings.setValue("Console/FontSize", m_fontSize);
    updateFont();
    Q_EMIT fontSizeChanged();
  }
}

/**
 * @brief Sets the currently selected checksum method by index.
 *
 * Updates the internal checksum method if the provided index differs
 * from the current one, and emits the checksumMethodChanged() signal.
 *
 * @param method The new checksum method index (from checksumMethods()).
 */
void Console::Handler::setChecksumMethod(const int method)
{
  if (checksumMethod() != method && method >= 0 && method < IO::availableChecksums().count()) {
    m_checksumMethod = method;
    m_settings.setValue("Console/ChecksumMethod", m_checksumMethod);
    Q_EMIT checksumMethodChanged();
  }
}

/**
 * Sets the console font family
 */
void Console::Handler::setFontFamily(const QString& family)
{
  // Only accept fixed-pitch font families
  if (m_fontFamily != family) {
    QFont testFont(family);
    QFontInfo fontInfo(testFont);
    if (!fontInfo.fixedPitch())
      return;

    m_fontFamily      = family;
    m_fontFamilyIndex = availableFonts().indexOf(m_fontFamily);
    m_settings.setValue("Console/FontFamily", m_fontFamily);
    updateFont();
    Q_EMIT fontFamilyChanged();
  }
}

/**
 * Changes the data mode for user commands. See @c dataMode() for more
 * information.
 */
void Console::Handler::setDataMode(const Console::Handler::DataMode& mode)
{
  if (m_dataMode != mode) {
    m_dataMode = mode;
    m_settings.setValue("Console/DataMode", static_cast<int>(m_dataMode));
    Q_EMIT dataModeChanged();
  }
}

/**
 * Changes line ending mode for sent user commands. See @c lineEnding() for more
 * information.
 */
void Console::Handler::setLineEnding(const Console::Handler::LineEnding& mode)
{
  if (m_lineEnding != mode) {
    m_lineEnding = mode;
    m_settings.setValue("Console/LineEnding", static_cast<int>(m_lineEnding));
    Q_EMIT lineEndingChanged();
  }
}

/**
 * Changes the display mode of the console. See @c displayMode() for more
 * information.
 */
void Console::Handler::setDisplayMode(const Console::Handler::DisplayMode& mode)
{
  if (m_displayMode != mode) {
    m_displayMode = mode;
    m_settings.setValue("Console/DisplayMode", static_cast<int>(m_displayMode));
    Q_EMIT displayModeChanged();
  }
}

/**
 * @brief Changes the text encoding used by send() and plainTextStr().
 *
 * The incoming value comes from a QML ComboBox `currentIndex`, so it is
 * already a valid `SerialStudio::TextEncoding` index — we clamp defensively
 * and guard-return on no-op changes.
 */
void Console::Handler::setEncoding(const int encoding)
{
  // Clamp the incoming int to a valid TextEncoding value
  if (encoding < 0 || encoding > SerialStudio::EncEucKr)
    return;

  // Guard-return if the encoding is unchanged
  const auto newEncoding = static_cast<SerialStudio::TextEncoding>(encoding);
  if (m_encoding == newEncoding)
    return;

  // Persist the new encoding and notify QML
  m_encoding = newEncoding;
  m_settings.setValue("Console/Encoding", static_cast<int>(m_encoding));
  Q_EMIT encodingChanged();
}

//--------------------------------------------------------------------------------------------------
// Display helpers
//--------------------------------------------------------------------------------------------------

/**
 * Inserts the given @a string into the list of lines of the console, if @a
 * addTimestamp is set to @c true, an timestamp is added for each line.
 */
void Console::Handler::append(const QString& string, const bool addTimestamp)
{
  // Validate input
  if (string.isEmpty())
    return;

  // Normalize line endings
  auto data = string;
  if (m_lastCharWasCR && data.startsWith('\n'))
    data.removeFirst();

  m_lastCharWasCR = data.endsWith('\r');

  data = data.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  data = data.replace(QStringLiteral("\r"), QStringLiteral("\n"));

  QString timestamp;
  if (addTimestamp) {
    QDateTime dateTime    = QDateTime::currentDateTime();
    const QString timeStr = dateTime.toString(QStringLiteral("HH:mm:ss.zzz -> "));

    if (ansiColorsEnabled()) {
      const QString ansiCyan  = QStringLiteral("\033[36m");
      const QString ansiReset = QStringLiteral("\033[0m");
      timestamp               = QStringLiteral("%1%2%3").arg(ansiCyan, timeStr, ansiReset);
    }

    else {
      timestamp = timeStr;
    }
  }

  QString processedString;
  processedString.reserve(data.length() + timestamp.length() * 4);

  // Single-pass newline scan to avoid O(n^2) QStringList removal
  int pos = 0;
  while (pos < data.length()) {
    const int nlPos = data.indexOf('\n', pos);
    const int end   = (nlPos < 0) ? data.length() : nlPos;

    // Process the segment before the newline (or remaining text)
    if (end > pos) {
      const auto segment = QStringView(data).mid(pos, end - pos);
      if (m_isStartingLine && !segment.trimmed().isEmpty())
        processedString.append(timestamp);

      processedString.append(segment);
      m_isStartingLine = false;
    }

    // Append the newline itself
    if (nlPos >= 0) {
      if (m_isStartingLine)
        processedString.append(timestamp);

      processedString.append('\n');
      m_isStartingLine = true;
      pos              = nlPos + 1;
    }

    else
      pos = end;
  }

  m_textBuffer.append(processedString.toUtf8());

  // Buffer for throttled display (flushed by m_displayFlushTimer)
  m_pendingDisplay.append(processedString);
}

/**
 * Displays the given @a data in the console without Hex formatting
 */
void Console::Handler::displayDebugData(const QString& data)
{
  append(data, showTimestamp());
}

/**
 * @brief Displays raw data from the playback path (no device routing).
 */
void Console::Handler::hotpathRxData(const IO::ByteArrayPtr& data)
{
  if (!data)
    return;

  append(dataToString(*data), showTimestamp());
}

/**
 * @brief Routes incoming raw data to the per-device console buffer.
 *
 * Converts raw bytes to a display string, appends to the device's buffer,
 * and if this device is currently selected, displays it live.
 *
 * @param deviceId Source device identifier.
 * @param data     Raw incoming bytes.
 */
void Console::Handler::hotpathRxDeviceData(int deviceId, const IO::ByteArrayPtr& data)
{
  if (!data)
    return;

  const auto str = dataToString(*data);
  if (str.isEmpty())
    return;

  // Store in per-device buffer
  appendToDevice(deviceId, str, showTimestamp());

  // Always emit for export
  Q_EMIT deviceDataReady(deviceId, str);
}

/**
 * @brief Echoes sent data to the console (legacy overload for device 0).
 */
void Console::Handler::displaySentData(QByteArrayView data)
{
  if (echo())
    displaySentData(m_currentDeviceId >= 0 ? m_currentDeviceId : -1, data);
}

/**
 * @brief Echoes sent data to a specific device's console buffer.
 * @param deviceId Target device.
 * @param data     Bytes that were sent.
 */
void Console::Handler::displaySentData(int deviceId, QByteArrayView data)
{
  if (!echo())
    return;

  const auto str = dataToString(data);
  appendToDevice(deviceId, str, showTimestamp());
  Q_EMIT deviceDataReady(deviceId, str);
}

//--------------------------------------------------------------------------------------------------
// Multi-device console management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current device ID for the console.
 */
int Console::Handler::currentDeviceId() const noexcept
{
  return m_currentDeviceId;
}

/**
 * @brief Returns true when more than one device is connected.
 */
bool Console::Handler::multiDeviceMode() const noexcept
{
  return m_deviceNames.size() > 1;
}

/**
 * @brief Returns the list of connected device names for the QML combobox.
 */
const QStringList& Console::Handler::deviceNames() const noexcept
{
  return m_deviceNames;
}

/**
 * @brief Switches the console view to the given @p deviceId.
 *
 * Guard-returns if unchanged. Clears the display, replays the selected
 * device's buffer, and emits currentDeviceIdChanged().
 */
void Console::Handler::setCurrentDeviceId(int deviceId)
{
  // Guard against no-op changes
  if (m_currentDeviceId == deviceId)
    return;

  m_currentDeviceId = deviceId;

  // Replay the selected device's buffer
  m_textBuffer.clear();
  m_isStartingLine = true;
  m_lastCharWasCR  = false;
  Q_EMIT cleared();

  auto it = m_deviceState.find(m_currentDeviceId);
  if (it != m_deviceState.end() && !it->second.buffer.isEmpty()) {
    m_textBuffer.append(it->second.buffer.toUtf8());
    Q_EMIT displayString(it->second.buffer);
  }

  Q_EMIT currentDeviceIdChanged();
}

/**
 * @brief Maps a QML combobox index to a device source ID.
 * @param index Combobox index.
 */
void Console::Handler::setCurrentDeviceIndex(int index)
{
  if (index >= 0 && index < m_deviceSourceIds.size())
    setCurrentDeviceId(m_deviceSourceIds.at(index));
}

/**
 * @brief Rebuilds the device name list from project sources and connection state.
 *
 * Called when sources change or connection state changes. Resets to device 0
 * if the current device is no longer available.
 */
void Console::Handler::onDevicesChanged()
{
  // Gather current connection and source state
  const auto& mgr     = IO::ConnectionManager::instance();
  const auto opMode   = AppState::instance().operationMode();
  const auto& sources = DataModel::ProjectModel::instance().sources();

  QStringList names;
  QList<int> ids;

  // Multi-device mode: ProjectFile with multiple sources
  if (opMode == SerialStudio::ProjectFile && sources.size() > 1) {
    for (const auto& src : sources) {
      const auto label = src.title.isEmpty() ? tr("Device %1").arg(src.sourceId) : src.title;
      names.append(label);
      ids.append(src.sourceId);
    }
  }

  // Check if the list actually changed
  if (names == m_deviceNames && ids == m_deviceSourceIds)
    return;

  m_deviceNames     = names;
  m_deviceSourceIds = ids;

  // If not connected, clear all device state
  if (!mgr.isConnected()) {
    m_deviceState.clear();
    m_currentDeviceId = -1;
    Q_EMIT deviceNamesChanged();
    Q_EMIT currentDeviceIdChanged();
    return;
  }

  // Reset to first device if current is gone
  if (!ids.isEmpty() && !ids.contains(m_currentDeviceId))
    setCurrentDeviceId(ids.first());
  else if (ids.isEmpty())
    m_currentDeviceId = -1;

  Q_EMIT deviceNamesChanged();
}

//--------------------------------------------------------------------------------------------------
// Internal utilities
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends a string to the per-device buffer and optionally displays it.
 *
 * The display string is processed through the same timestamp/newline logic as
 * append(), but state is tracked per-device. If the device is the currently
 * selected one, the processed string is also shown live in the console.
 *
 * @param deviceId    Target device ID.
 * @param str         The string to append.
 * @param addTimestamp Whether to prepend timestamps.
 */
void Console::Handler::appendToDevice(int deviceId, const QString& str, bool addTimestamp)
{
  if (str.isEmpty())
    return;

  // Get or create per-device state
  auto& state = m_deviceState[deviceId];

  // Process the string through timestamp/newline logic
  auto data = str;
  if (state.lastCharWasCR && data.startsWith('\n'))
    data.removeFirst();

  state.lastCharWasCR = data.endsWith('\r');

  data = data.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  data = data.replace(QStringLiteral("\r"), QStringLiteral("\n"));

  QString timestamp;
  if (addTimestamp) {
    QDateTime dateTime    = QDateTime::currentDateTime();
    const QString timeStr = dateTime.toString(QStringLiteral("HH:mm:ss.zzz -> "));

    if (ansiColorsEnabled()) {
      const QString ansiCyan  = QStringLiteral("\033[36m");
      const QString ansiReset = QStringLiteral("\033[0m");
      timestamp               = QStringLiteral("%1%2%3").arg(ansiCyan, timeStr, ansiReset);
    }

    else {
      timestamp = timeStr;
    }
  }

  QString processedString;
  processedString.reserve(data.length() + timestamp.length() * 4);

  // Single-pass newline scan
  int pos = 0;
  while (pos < data.length()) {
    const int nlPos = data.indexOf('\n', pos);
    const int end   = (nlPos < 0) ? data.length() : nlPos;

    if (end > pos) {
      const auto segment = QStringView(data).mid(pos, end - pos);
      if (state.isStartingLine && !segment.trimmed().isEmpty())
        processedString.append(timestamp);

      processedString.append(segment);
      state.isStartingLine = false;
    }

    if (nlPos >= 0) {
      if (state.isStartingLine)
        processedString.append(timestamp);

      processedString.append('\n');
      state.isStartingLine = true;
      pos                  = nlPos + 1;
    }

    else
      pos = end;
  }

  // Cap per-device buffer at 10 KB
  static constexpr int kMaxDeviceBuffer = 10 * 1024;
  state.buffer.append(processedString);
  if (state.buffer.size() > kMaxDeviceBuffer) {
    const int excess = state.buffer.size() - kMaxDeviceBuffer;
    state.buffer.remove(0, excess);
  }

  // Buffer for throttled display if this is the active device
  if (deviceId == m_currentDeviceId || m_currentDeviceId < 0) {
    m_textBuffer.append(processedString.toUtf8());
    m_pendingDisplay.append(processedString);
  }
}

bool Console::Handler::hasImageWidget() const
{
  // Only check project groups when connected in project mode
  if (!IO::ConnectionManager::instance().isConnected())
    return false;

  if (AppState::instance().operationMode() != SerialStudio::ProjectFile)
    return false;

  const auto& groups = DataModel::ProjectModel::instance().groups();
  for (const auto& g : groups)
    if (g.widget == QLatin1String("image"))
      return true;

  return false;
}

/**
 * @brief Returns @c true when project mode is active and at least one image widget is present.
 *
 * Used by QML to disable and dim VT-100/ANSI checkboxes in this state.
 */
bool Console::Handler::imageWidgetActive() const
{
  return hasImageWidget();
}

/**
 * Updates the font based on current family and size settings
 */
void Console::Handler::updateFont()
{
  // Validate font is fixed-pitch, fall back to default if not
  QFont testFont(m_fontFamily, m_fontSize);
  QFontInfo fontInfo(testFont);

  if (!fontInfo.fixedPitch()) {
    const auto defaultFont = Misc::CommonFonts::instance().monoFont();
    m_fontFamily           = defaultFont.family();
    m_fontFamilyIndex      = availableFonts().indexOf(m_fontFamily);
    m_fontSize             = defaultFont.pointSize();
    m_settings.setValue("Console/FontFamily", m_fontFamily);
    m_settings.setValue("Console/FontSize", m_fontSize);
    testFont = defaultFont;
  }

  m_font = testFont;
  m_font.setStyleStrategy(QFont::PreferAntialias);
  Q_EMIT fontChanged();
}

/**
 * Registers the given @a command to the list of sent commands.
 */
void Console::Handler::addToHistory(const QString& command)
{
  // Trim history to max 100 entries and append the new command
  while (m_historyItems.count() > 100)
    m_historyItems.removeFirst();

  m_historyItems.append(command);
  m_historyItem = m_historyItems.count();
  Q_EMIT historyItemChanged();
}

/**
 * Converts the given @a data to a string according to the console display mode
 * set by the user.
 */
QString Console::Handler::dataToString(QByteArrayView data)
{
  switch (displayMode()) {
    case DisplayMode::DisplayPlainText:
      return plainTextStr(data);
    case DisplayMode::DisplayHexadecimal:
      return hexadecimalStr(data);
    default:
      return "";
  }
}

/**
 * @brief Converts raw received bytes to a display string.
 *
 * @param data The raw bytes to convert.
 * @return A QString ready for the terminal widget.
 *
 * In VT-100 emulation mode every byte (except NUL) is passed through
 * unchanged so that the terminal's own state machine can process control
 * characters such as backspace (0x08), which zsh's line editor sends
 * constantly for in-place redraws.  Replacing those with '.' would make
 * typed commands appear corrupted (e.g. "ls" → "l.ls").
 *
 * In plain-text mode non-printable characters that are not recognised
 * control codes are substituted with '.' for readability.
 */
QString Console::Handler::plainTextStr(QByteArrayView data)
{
  // Decode raw bytes using the user-selected text encoding
  QString utf8Data = SerialStudio::decodeText(data, m_encoding);

  if (vt100Emulation())
    return utf8Data;

  // Batch printable runs, replace non-printable chars with dots
  QString filteredData;
  filteredData.reserve(utf8Data.size());

  int i = 0;
  while (i < utf8Data.size()) {
    // Scan a run of printable characters
    const int runStart = i;
    while (i < utf8Data.size()) {
      const ushort unicode = utf8Data[i].unicode();

      // clang-format off
      const bool printable = (unicode != '\0')
                             && ((unicode >= 0x20 && unicode < 0x7F)
                                 || (unicode >= 0x80)
                                 || (unicode == '\r')
                                 || (unicode == '\n')
                                 || (unicode == '\t')
                                 || (unicode == 0x1B));
      // clang-format on

      if (!printable)
        break;

      ++i;
    }

    // Bulk-append printable run
    if (i > runStart)
      filteredData.append(QStringView(utf8Data).mid(runStart, i - runStart));

    // Replace non-printable char with dot
    if (i < utf8Data.size()) {
      filteredData.append('.');
      ++i;
    }
  }

  return filteredData;
}

/**
 * Converts the given @a data into a HEX representation string.
 */
QString Console::Handler::hexadecimalStr(QByteArrayView data)
{
  // Format data as hex dump with 16-byte rows and ASCII sidebar
  QString out;
  constexpr auto rowSize = 16;

  for (int i = 0; i < data.length(); i += rowSize) {
    out += QStringLiteral("%1 | ").arg(i, 6, 16, QLatin1Char('0'));

    for (int j = 0; j < rowSize; ++j) {
      if (i + j < data.length()) {
        out += QStringLiteral("%1 ").arg(
          static_cast<unsigned char>(data[i + j]), 2, 16, QLatin1Char('0'));
      }

      else
        out += QStringLiteral("   ");

      if ((j + 1) == 8)
        out += ' ';
    }

    out += QStringLiteral("| ");
    for (int j = 0; j < rowSize; ++j) {
      if (i + j >= data.length()) {
        out += ' ';
        continue;
      }

      const char c = data[i + j];
      if (std::isprint(static_cast<unsigned char>(c)))
        out += c;
      else
        out += '.';
    }

    out += QStringLiteral(" |\n");
  }

  out += "\n";
  return out;
}
