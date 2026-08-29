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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Console/Handler.h"

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFontDatabase>
#include <QFontInfo>
#include <QFontMetrics>

#include "AppState.h"
#include "Console/TextFormat.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"
#include "SessionContext.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the "HH:mm:ss.zzz -> " stamp for the current time, reusing the previous
 *        string while the millisecond has not advanced (chunks arrive faster than the clock
 *        ticks, and the locale-aware formatting is the expensive part).
 */
static const QString& cachedTimestampStr()
{
  static qint64 s_lastMs = -1;
  static QString s_cached;

  const qint64 now = QDateTime::currentMSecsSinceEpoch();
  if (now != s_lastMs) {
    s_lastMs = now;
    s_cached = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz -> "));
  }

  return s_cached;
}

/**
 * @brief Returns the line prefix for a stamped chunk: the cached clock string, cyan-wrapped
 *        while ANSI rendering is on. Empty when the caller wants no stamp.
 */
static QString decoratedTimestamp(bool addTimestamp, bool ansiColors)
{
  if (!addTimestamp)
    return {};

  const QString& timeStr = cachedTimestampStr();
  if (!ansiColors)
    return timeStr;

  const QString ansiCyan  = QStringLiteral("\033[36m");
  const QString ansiReset = QStringLiteral("\033[0m");
  return QStringLiteral("%1%2%3").arg(ansiCyan, timeStr, ansiReset);
}

/**
 * @brief Constructs the console handler singleton.
 */
Console::Handler::Handler()
  : m_dataMode(DataMode::DataUTF8)
  , m_lineEnding(LineEnding::NoLineEnding)
  , m_displayMode(DisplayMode::DisplayPlainText)
  , m_encoding(SerialStudio::EncUtf8)
  , m_historyItem(0)
  , m_checksumMethod(0)
  , m_scrollbackLines(1000)
  , m_echo(true)
  , m_showTimestamp(false)
  , m_collapseDuplicates(false)
  , m_searchCaseSensitive(false)
  , m_ansiColorsEnabled(false)
  , m_vt100Emulation(true)
  , m_ansiColors(true)
  , m_lineState()
  , m_currentDeviceId(-1)
  , m_fontFamilyIndex(0)
  , m_textBuffer(10 * 1024)
  , m_commonFonts(&Misc::CommonFonts::instance())
  , m_connectionManager(nullptr)
  , m_appState(nullptr)
  , m_projectModel(nullptr)
  , m_annotations(new AnnotationModel(this))
  , m_annotationDecoder(new AnnotationDecoder(m_annotations, this))
  , m_annotationFilter(new AnnotationFilter(this))
{
  m_annotationFilter->setSourceModel(m_annotations);
  clear();
  const auto defaultFont = m_commonFonts->monoFont();
  m_fontFamily           = m_settings.value("Console/FontFamily", defaultFont.family()).toString();
  m_fontSize             = m_settings.value("Console/FontSize", defaultFont.pointSize()).toInt();
  m_echo                 = m_settings.value("Console/Echo", true).toBool();
  m_showTimestamp        = m_settings.value("Console/ShowTimestamp", false).toBool();
  m_collapseDuplicates   = m_settings.value("Console/CollapseDuplicates", false).toBool();
  m_searchCaseSensitive  = m_settings.value("Console/SearchCaseSensitive", false).toBool();
  m_vt100Emulation       = m_settings.value("Console/VT100Emulation", true).toBool();
  m_ansiColors           = m_settings.value("Console/AnsiColors", true).toBool();
  m_checksumMethod       = m_settings.value("Console/ChecksumMethod", 0).toInt();
  m_scrollbackLines      = m_settings.value("Console/ScrollbackLines", 1000).toInt();
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

  m_scrollbackLines = qBound(100, m_scrollbackLines, 100000);

  const int checksumCount = IO::availableChecksums().count();
  if (m_checksumMethod < 0 || m_checksumMethod >= checksumCount)
    m_checksumMethod = 0;

  m_ansiColorsEnabled = m_vt100Emulation && m_ansiColors;
  m_fontFamilyIndex   = availableFonts().indexOf(m_fontFamily);

  static auto& timerEvents = Misc::TimerEvents::instance();
  connect(&timerEvents, &Misc::TimerEvents::uiTimeout, this, [this]() {
    if (!m_pendingDisplay.isEmpty()) {
      Q_EMIT displayString(m_pendingDisplay);
      m_pendingDisplay.clear();
    }

    m_annotations->commitPending();
  });

  updateFont();
}

/**
 * @brief Returns this session's console handler. The object is owned by the SessionContext and
 *        built by the composition root, so a reach before adoption is a named fatal instead of
 *        an out-of-order lazy construction (spec 0039 M2, wave B1).
 */
Console::Handler& Console::Handler::instance()
{
  return SessionContext::current().console();
}

//--------------------------------------------------------------------------------------------------
// Status & configuration queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the console shall display sent commands.
 */
bool Console::Handler::echo() const
{
  return m_echo;
}

/**
 * @brief Returns true if a timestamp should be shown before each data block.
 */
bool Console::Handler::showTimestamp() const
{
  return m_showTimestamp;
}

/**
 * @brief Returns true if consecutive duplicate console lines collapse into one entry.
 */
bool Console::Handler::collapseDuplicates() const
{
  return m_collapseDuplicates;
}

/**
 * @brief Returns true if console search matches case-sensitively.
 */
bool Console::Handler::searchCaseSensitive() const
{
  return m_searchCaseSensitive;
}

/**
 * @brief Returns true if ANSI color codes are used in console output.
 */
bool Console::Handler::ansiColorsEnabled() const
{
  return m_ansiColorsEnabled;
}

/**
 * @brief Returns true if VT-100 terminal emulation is enabled.
 */
bool Console::Handler::vt100Emulation() const
{
  if (hasImageWidget())
    return false;

  return m_vt100Emulation;
}

/**
 * @brief Returns true if ANSI color rendering is enabled.
 */
bool Console::Handler::ansiColors() const
{
  if (hasImageWidget())
    return false;

  return m_ansiColors;
}

/**
 * @brief Returns the index of the currently selected checksum method.
 */
int Console::Handler::checksumMethod() const
{
  return m_checksumMethod;
}

/**
 * @brief Returns the data mode for outgoing commands.
 */
Console::Handler::DataMode Console::Handler::dataMode() const
{
  return m_dataMode;
}

/**
 * @brief Returns the line ending appended to each sent data block.
 */
Console::Handler::LineEnding Console::Handler::lineEnding() const
{
  return m_lineEnding;
}

/**
 * @brief Returns the display format of the console.
 */
Console::Handler::DisplayMode Console::Handler::displayMode() const
{
  return m_displayMode;
}

/**
 * @brief Returns the selected text encoding as an int (TextEncoding enum).
 */
int Console::Handler::encoding() const
{
  return static_cast<int>(m_encoding);
}

/**
 * @brief Returns the current command history entry.
 */
QString Console::Handler::currentHistoryString() const
{
  if (m_historyItem < m_historyItems.count() && m_historyItem >= 0)
    return m_historyItems.at(m_historyItem);

  return "";
}

//--------------------------------------------------------------------------------------------------
// Available options lists
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list of available data sending modes.
 */
QStringList Console::Handler::dataModes() const
{
  QStringList list;
  list.append(tr("ASCII"));
  list.append(tr("HEX"));
  return list;
}

/**
 * @brief Returns the list of available line ending options.
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
 * @brief Returns the list of available console display modes.
 */
QStringList Console::Handler::displayModes() const
{
  QStringList list;
  list.append(tr("Text"));
  list.append(tr("Hex"));
  return list;
}

/**
 * @brief Returns the list of supported text encodings for QML.
 */
QStringList Console::Handler::textEncodings() const
{
  return SerialStudio::textEncodings();
}

/**
 * @brief Returns a list of supported checksum methods including "None".
 */
QStringList Console::Handler::checksumMethods() const
{
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
 * @brief Returns the current console font.
 */
QFont Console::Handler::font() const
{
  return m_font;
}

/**
 * @brief Returns the current console font size in points.
 */
int Console::Handler::fontSize() const
{
  return m_fontSize;
}

/**
 * @brief Returns the number of lines retained in the console scrollback buffer.
 */
int Console::Handler::scrollbackLines() const
{
  return m_scrollbackLines;
}

/**
 * @brief Returns the current console font family.
 */
QString Console::Handler::fontFamily() const
{
  return m_fontFamily;
}

/**
 * @brief Returns the index of the current font family in availableFonts().
 */
int Console::Handler::fontFamilyIndex() const
{
  return m_fontFamilyIndex;
}

/**
 * @brief Returns the list of all available monospace fonts.
 */
QStringList Console::Handler::availableFonts() const
{
  QStringList monospaceFonts;
  const auto allFonts = QFontDatabase::families();
  auto defaultFamily  = m_commonFonts->monoFont().family();
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
 * @brief Returns the character width for the default monospace font.
 */
int Console::Handler::defaultCharWidth() const
{
  SS_ASSERT(m_commonFonts != nullptr, return 8);
  const auto defaultFont = m_commonFonts->monoFont();
  const QFontMetrics metrics(defaultFont);
  return metrics.horizontalAdvance("M");
}

/**
 * @brief Returns the character height for the default monospace font.
 */
int Console::Handler::defaultCharHeight() const
{
  SS_ASSERT(m_commonFonts != nullptr, return 16);
  const auto defaultFont = m_commonFonts->monoFont();
  const QFontMetrics metrics(defaultFont);
  return metrics.height();
}

//--------------------------------------------------------------------------------------------------
// Buffer & history management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the number of bytes stored in the console buffer.
 */
qsizetype Console::Handler::bufferLength() const
{
  return m_textBuffer.size();
}

/**
 * @brief Validates that @a text contains only hex characters and complete byte pairs.
 */
bool Console::Handler::validateUserHex(const QString& text)
{
  QString cleanText = text.simplified().remove(' ');

  static QRegularExpression hexPattern("^[0-9A-Fa-f]*$");
  if (!hexPattern.match(cleanText).hasMatch())
    return false;

  if (cleanText.length() % 2 != 0)
    return false;

  return true;
}

/**
 * @brief Reformats @a text as spaced byte pairs for byte-oriented display.
 */
QString Console::Handler::formatUserHex(const QString& text)
{
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
 * @brief Clears the console text and per-device buffer for the current device.
 */
void Console::Handler::clear()
{
  m_textBuffer.clear();
  m_lineState = TextFormat::LineState{};

  auto it = m_deviceState.find(m_currentDeviceId);
  if (it != m_deviceState.end()) {
    it->second.buffer.clear();
    it->second.line = TextFormat::LineState{};
  }

  Q_EMIT cleared();
}

/**
 * @brief Navigates to an older entry in the command history.
 */
void Console::Handler::historyUp()
{
  if (m_historyItem > 0) {
    --m_historyItem;
    Q_EMIT historyItemChanged();
  }
}

/**
 * @brief Navigates to a newer entry in the command history.
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
 * @brief Configures signal/slot connections with dependent modules.
 */
void Console::Handler::setupExternalConnections()
{
  m_connectionManager = &IO::ConnectionManager::instance();
  m_appState          = &AppState::instance();
  m_projectModel      = &DataModel::ProjectModel::instance();

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &Console::Handler::languageChanged);

  auto notifyTerminal = [this] {
    Q_EMIT vt100EmulationChanged();
    Q_EMIT ansiColorsChanged();
    Q_EMIT imageWidgetActiveChanged();
  };

  connect(m_projectModel, &DataModel::ProjectModel::groupsChanged, this, notifyTerminal);
  connect(m_appState, &AppState::operationModeChanged, this, notifyTerminal);
  connect(m_connectionManager, &IO::ConnectionManager::connectedChanged, this, notifyTerminal);

  connect(m_projectModel,
          &DataModel::ProjectModel::sourceStructureChanged,
          this,
          &Console::Handler::onDevicesChanged,
          Qt::QueuedConnection);
  connect(m_connectionManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &Console::Handler::onDevicesChanged);
}

//--------------------------------------------------------------------------------------------------
// Data send & receive
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends @a data to the connected device using the current send options.
 */
void Console::Handler::send(const QString& data)
{
  SS_ASSERT(m_connectionManager != nullptr, return);
  if (!m_connectionManager->isConnected())
    return;

  if (!data.isEmpty())
    addToHistory(data);

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
      (void)m_connectionManager->writeDataToDevice(m_currentDeviceId, bin);
    else
      (void)m_connectionManager->writeData(bin);
  }
}

//--------------------------------------------------------------------------------------------------
// Settings modification slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enables or disables a timestamp for each received data block.
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
 * @brief Enables or disables collapsing of consecutive duplicate console lines.
 */
void Console::Handler::setCollapseDuplicates(const bool enabled)
{
  if (collapseDuplicates() != enabled) {
    m_collapseDuplicates = enabled;
    m_settings.setValue("Console/CollapseDuplicates", m_collapseDuplicates);
    Q_EMIT collapseDuplicatesChanged();
  }
}

/**
 * @brief Enables or disables case-sensitive console search.
 */
void Console::Handler::setSearchCaseSensitive(const bool enabled)
{
  if (searchCaseSensitive() != enabled) {
    m_searchCaseSensitive = enabled;
    m_settings.setValue("Console/SearchCaseSensitive", m_searchCaseSensitive);
    Q_EMIT searchCaseSensitiveChanged();
  }
}

/**
 * @brief Enables or disables ANSI color codes in console output.
 */
void Console::Handler::setAnsiColorsEnabled(const bool enabled)
{
  if (ansiColorsEnabled() != enabled) {
    m_ansiColorsEnabled = enabled;
    Q_EMIT ansiColorsEnabledChanged();
  }
}

/**
 * @brief Enables or disables VT-100 terminal emulation and persists the setting.
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
 * @brief Enables or disables ANSI color rendering and persists the setting.
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
 * @brief Enables or disables echoing sent data to the console.
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
 * @brief Sets the console font size.
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
 * @brief Sets the number of lines retained in the console scrollback buffer.
 */
void Console::Handler::setScrollbackLines(const int lines)
{
  const int constrainedLines = qBound(100, lines, 100000);
  if (m_scrollbackLines != constrainedLines) {
    m_scrollbackLines = constrainedLines;
    m_settings.setValue("Console/ScrollbackLines", m_scrollbackLines);
    Q_EMIT scrollbackLinesChanged();
  }
}

/**
 * @brief Sets the currently selected checksum method by index.
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
 * @brief Sets the console font family.
 */
void Console::Handler::setFontFamily(const QString& family)
{
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
 * @brief Changes the data mode used for user commands.
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
 * @brief Changes the line ending mode for sent user commands.
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
 * @brief Changes the display mode of the console.
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
 */
void Console::Handler::setEncoding(const int encoding)
{
  if (encoding < 0 || encoding > SerialStudio::EncEucKr)
    return;

  const auto newEncoding = static_cast<SerialStudio::TextEncoding>(encoding);
  if (m_encoding == newEncoding)
    return;

  m_encoding = newEncoding;
  m_settings.setValue("Console/Encoding", static_cast<int>(m_encoding));
  Q_EMIT encodingChanged();
}

//--------------------------------------------------------------------------------------------------
// Display helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends @a string to the console, optionally prefixing timestamps.
 */
void Console::Handler::append(const QString& string, const bool addTimestamp)
{
  if (string.isEmpty())
    return;

  const auto timestamp = decoratedTimestamp(addTimestamp, ansiColorsEnabled());
  const auto processed = TextFormat::formatIncoming(string, m_lineState, timestamp);

  m_textBuffer.append(processed.toUtf8());
  m_pendingDisplay.append(processed);
}

/**
 * @brief Displays @a data in the console without hex formatting.
 */
void Console::Handler::displayDebugData(const QString& data)
{
  append(data, showTimestamp());
}

/**
 * @brief Displays raw data from the playback path (no device routing).
 */
void Console::Handler::hotpathRxData(const QByteArray& data)
{
  if (data.isEmpty())
    return;

  m_annotationDecoder->feed(data);
  append(dataToString(data), showTimestamp());
}

/**
 * @brief The frame annotation store (spec 0059) fed by this console's raw byte stream.
 */
QObject* Console::Handler::annotations() const noexcept
{
  return m_annotations;
}

/**
 * @brief The user decoder that fills the annotation store.
 */
QObject* Console::Handler::annotationDecoder() const noexcept
{
  return m_annotationDecoder;
}

/**
 * @brief Row/class filter proxy over the annotation store, for the table view.
 */
QObject* Console::Handler::annotationFilter() const noexcept
{
  return m_annotationFilter;
}

/**
 * @brief Routes incoming raw data to the per-device console buffer.
 */
void Console::Handler::hotpathRxDeviceData(int deviceId, const QByteArray& data)
{
  if (data.isEmpty())
    return;

  if (m_currentDeviceId < 0 || deviceId == m_currentDeviceId)
    m_annotationDecoder->feed(data);

  const auto str = dataToString(data);
  if (str.isEmpty())
    return;

  const auto processed = appendToDevice(deviceId, str, showTimestamp());
  Q_EMIT deviceDataReady(deviceId, processed);
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
 */
void Console::Handler::displaySentData(int deviceId, QByteArrayView data)
{
  if (!echo())
    return;

  const auto str       = dataToString(data);
  const auto processed = appendToDevice(deviceId, str, showTimestamp());
  Q_EMIT deviceDataReady(deviceId, processed);
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
 * @brief Returns the combobox index of the current device, or -1 when none is selected.
 */
int Console::Handler::currentDeviceIndex() const noexcept
{
  return static_cast<int>(m_deviceSourceIds.indexOf(m_currentDeviceId));
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
 */
void Console::Handler::setCurrentDeviceId(int deviceId)
{
  if (m_currentDeviceId == deviceId)
    return;

  m_currentDeviceId = deviceId;

  m_textBuffer.clear();
  m_lineState = TextFormat::LineState{};
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
 */
void Console::Handler::setCurrentDeviceIndex(int index)
{
  if (index >= 0 && index < m_deviceSourceIds.size())
    setCurrentDeviceId(m_deviceSourceIds.at(index));
}

/**
 * @brief Rebuilds the device name list from project sources and connection state.
 */
void Console::Handler::onDevicesChanged()
{
  SS_ASSERT(m_connectionManager != nullptr, return);
  SS_ASSERT(m_appState != nullptr, return);
  SS_ASSERT(m_projectModel != nullptr, return);

  const auto opMode   = m_appState->operationMode();
  const auto& sources = m_projectModel->sources();

  QStringList names;
  QList<int> ids;

  if (opMode == SerialStudio::ProjectFile && sources.size() > 1) {
    for (const auto& src : sources) {
      const auto label = src.title.isEmpty() ? tr("Device %1").arg(src.sourceId) : src.title;
      names.append(label);
      ids.append(src.sourceId);
    }
  }

  if (names == m_deviceNames && ids == m_deviceSourceIds) {
    if (m_connectionManager->isConnected() && !ids.isEmpty() && !ids.contains(m_currentDeviceId))
      setCurrentDeviceId(ids.first());

    return;
  }

  m_deviceNames     = names;
  m_deviceSourceIds = ids;

  if (!m_connectionManager->isConnected()) {
    m_deviceState.clear();
    m_currentDeviceId = -1;
    Q_EMIT deviceNamesChanged();
    Q_EMIT currentDeviceIdChanged();
    return;
  }

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
 * @brief Appends a string to the per-device buffer and returns the processed (timestamp-prefixed)
 *        form. Only the current device reaches the visible console (the no-selection fallback
 *        applies solely outside multi-device mode), so device streams never interleave.
 */
QString Console::Handler::appendToDevice(int deviceId, const QString& str, bool addTimestamp)
{
  if (str.isEmpty())
    return QString();

  auto& state          = m_deviceState[deviceId];
  const auto timestamp = decoratedTimestamp(addTimestamp, ansiColorsEnabled());
  const auto processed = TextFormat::formatIncoming(str, state.line, timestamp);

  static constexpr int kMaxDeviceBuffer = 10 * 1024;
  state.buffer.append(processed);
  if (state.buffer.size() > kMaxDeviceBuffer) {
    const int excess = state.buffer.size() - kMaxDeviceBuffer;
    state.buffer.remove(0, excess);
  }

  if (deviceId == m_currentDeviceId || (m_currentDeviceId < 0 && m_deviceSourceIds.isEmpty())) {
    m_textBuffer.append(processed.toUtf8());
    m_pendingDisplay.append(processed);
  }

  return processed;
}

/**
 * @brief Returns whether the active project contains an image widget.
 */
bool Console::Handler::hasImageWidget() const
{
  SS_ASSERT(m_connectionManager != nullptr, return false);
  SS_ASSERT(m_appState != nullptr, return false);
  SS_ASSERT(m_projectModel != nullptr, return false);

  if (!m_connectionManager->isConnected())
    return false;

  if (m_appState->operationMode() != SerialStudio::ProjectFile)
    return false;

  const auto& groups = m_projectModel->groups();
  for (const auto& g : groups)
    if (g.widget == QLatin1String("image"))
      return true;

  return false;
}

/**
 * @brief Returns true when project mode is active and at least one image widget is present.
 */
bool Console::Handler::imageWidgetActive() const
{
  return hasImageWidget();
}

/**
 * @brief Updates the font based on current family and size settings.
 */
void Console::Handler::updateFont()
{
  QFont testFont(m_fontFamily, m_fontSize);
  QFontInfo fontInfo(testFont);

  if (!fontInfo.fixedPitch()) {
    const auto defaultFont = m_commonFonts->monoFont();
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
 * @brief Registers @a command in the list of sent commands.
 */
void Console::Handler::addToHistory(const QString& command)
{
  while (m_historyItems.count() > 100)
    m_historyItems.removeFirst();

  m_historyItems.append(command);
  m_historyItem = m_historyItems.count();
  Q_EMIT historyItemChanged();
}

/**
 * @brief Converts @a data to a string according to the active display mode.
 */
QString Console::Handler::dataToString(QByteArrayView data)
{
  switch (displayMode()) {
    case DisplayMode::DisplayPlainText:
      return plainTextStr(data);
    case DisplayMode::DisplayHexadecimal:
      return TextFormat::hexDump(data);
    default:
      return "";
  }
}

/**
 * @brief Converts raw received bytes to a display string. Control characters survive only while
 *        VT-100 emulation is on, where the terminal itself interprets them.
 */
QString Console::Handler::plainTextStr(QByteArrayView data)
{
  const QString utf8Data = SerialStudio::decodeText(data, m_encoding);
  if (vt100Emulation())
    return utf8Data;

  return TextFormat::filterControlChars(utf8Data);
}
