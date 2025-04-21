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

#include <QFile>
#include <QDateTime>

#include "IO/Manager.h"
#include "IO/Console.h"
#include "Misc/Translator.h"

/**
 * Constructor function
 */
IO::Console::Console()
  : m_dataMode(DataMode::DataUTF8)
  , m_lineEnding(LineEnding::NoLineEnding)
  , m_displayMode(DisplayMode::DisplayPlainText)
  , m_scrollback(1000)
  , m_historyItem(0)
  , m_echo(true)
  , m_showTimestamp(false)
  , m_isStartingLine(true)
  , m_lastCharWasCR(false)
  , m_textBuffer(10 * 1024)
{
  clear();
}

/**
 * Returns the only instance of the class
 */
IO::Console &IO::Console::instance()
{
  static Console singleton;
  return singleton;
}

/**
 * @brief Returns @c true if the console shall display sent commands
 */
bool IO::Console::echo() const
{
  return m_echo;
}

/**
 * Returns @c true if a timestamp should be shown before each displayed data
 * block.
 */
bool IO::Console::showTimestamp() const
{
  return m_showTimestamp;
}

/**
 * Returns the number of lines that terminal display widgets should limit
 * scrollback to. You can change this value by calling the setScrollback()
 * function.
 *
 * @note This value is not used directly by the IO::Console class, as it does
 *       not take into account line numbers.
 */
int IO::Console::scrollback() const
{
  return m_scrollback;
}

/**
 * Returns the type of data that the user inputs to the console. There are two
 * possible values:
 * - @c DataMode::DataUTF8        the user is sending data formated in the UTF-8
 * codec.
 * - @c DataMode::DataHexadecimal the user is sending binary data represented in
 *                                hexadecimal format, we must do a conversion to
 * obtain and send the appropiate binary data to the target device.
 */
IO::Console::DataMode IO::Console::dataMode() const
{
  return m_dataMode;
}

/**
 * Returns the line ending character that is added to each datablock sent by the
 * user. Possible values are:
 * - @c LineEnding::NoLineEnding                  leave data as-it-is
 * - @c LineEnding::NewLine,                      add '\n' to the data sent by
 * the user
 * - @c LineEnding::CarriageReturn,               add '\r' to the data sent by
 * the user
 * - @c LineEnding::BothNewLineAndCarriageReturn  add '\r\n' to the data sent by
 * the user
 */
IO::Console::LineEnding IO::Console::lineEnding() const
{
  return m_lineEnding;
}

/**
 * Returns the display format of the console. Posible values are:
 * - @c DisplayMode::DisplayPlainText   display incoming data as an UTF-8 stream
 * - @c DisplayMode::DisplayHexadecimal display incoming data in hexadecimal
 * format
 */
IO::Console::DisplayMode IO::Console::displayMode() const
{
  return m_displayMode;
}

/**
 * Returns the current command history string selected by the user.
 *
 * @note the user can navigate through sent commands using the Up/Down keys on
 * the keyboard. This behaviour is managed by the @c historyUp() & @c
 * historyDown() functions.
 */
QString IO::Console::currentHistoryString() const
{
  if (m_historyItem < m_historyItems.count() && m_historyItem >= 0)
    return m_historyItems.at(m_historyItem);

  return "";
}

/**
 * Returns a list with the available data (sending) modes. This list must be
 * synchronized with the order of the @c DataMode enums.
 */
QStringList IO::Console::dataModes() const
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
QStringList IO::Console::lineEndings() const
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
QStringList IO::Console::displayModes() const
{
  QStringList list;
  list.append(tr("Plain Text"));
  list.append(tr("Hexadecimal"));
  return list;
}

/**
 * Validates the given @a text to ensure it contains only valid HEX characters
 * and consists of complete byte pairs (even-length string).
 * Returns true if the input is valid, otherwise false.
 */
bool IO::Console::validateUserHex(const QString &text)
{
  // Remove spaces to check the actual HEX content
  QString cleanText = text.simplified().remove(' ');

  // Match the string against a HEX pattern (only [0-9A-Fa-f])
  static QRegularExpression hexPattern("^[0-9A-Fa-f]*$");
  if (!hexPattern.match(cleanText).hasMatch())
    return false;

  // Check for even length to ensure complete byte pairs
  if (cleanText.length() % 2 != 0)
    return false;

  return true;
}

/**
 * Validates the given @a text and adds space to display the text in a
 * byte-oriented view
 */
QString IO::Console::formatUserHex(const QString &text)
{
  // Remove spaces and ensure the input is a valid HEX string
  static QRegularExpression exp("[^0-9A-Fa-f]");
  QString data = text.simplified().remove(exp);

  // Convert to hex string with spaces between bytes
  QString str;
  for (int i = 0; i < data.length(); ++i)
  {
    str.append(data.at(i));
    if ((i + 1) % 2 == 0 && i > 0)
      str.append(" ");
  }

  // Chop last space
  while (str.endsWith(" "))
    str.chop(1);

  // Return string
  return str;
}

/**
 * Converts the given @a data in HEX format into real binary data.
 */
QByteArray IO::Console::hexToBytes(const QString &data)
{
  // Remove spaces from the input data
  QString withoutSpaces = data;
  withoutSpaces.replace(QStringLiteral(" "), "");

  // Check if the length of the string is even
  if (withoutSpaces.length() % 2 != 0)
  {
    qWarning() << data << "is not a valid hexadecimal array";
    return QByteArray();
  }

  // Iterate over the string in steps of 2
  bool ok;
  QByteArray array;
  for (int i = 0; i < withoutSpaces.length(); i += 2)
  {
    // Get two characters (a hex pair)
    auto chr1 = withoutSpaces.at(i);
    auto chr2 = withoutSpaces.at(i + 1);

    // Convert the hex pair into a byte
    QString byteStr = QStringLiteral("%1%2").arg(chr1, chr2);
    int byte = byteStr.toInt(&ok, 16);

    // If the conversion fails, return an empty array
    if (!ok)
    {
      qWarning() << data << "is not a valid hexadecimal array";
      return QByteArray();
    }

    // Append the byte to the result array
    array.append(static_cast<char>(byte));
  }

  return array;
}

/**
 * Deletes all the text displayed by the current QML text document
 */
void IO::Console::clear()
{
  m_textBuffer.clear();
  m_isStartingLine = true;
  m_lastCharWasCR = false;
}

/**
 * Comamnds sent by the user are stored in a @c QStringList, in which the first
 * items are the oldest commands.
 *
 * The user can navigate the list using the up/down keys. This function allows
 * the user to navigate the list from most recent command to oldest command.
 */
void IO::Console::historyUp()
{
  if (m_historyItem > 0)
  {
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
void IO::Console::historyDown()
{
  if (m_historyItem < m_historyItems.count() - 1)
  {
    ++m_historyItem;
    Q_EMIT historyItemChanged();
  }
}

/**
 * Configures the signal/slot connections with the rest of the modules of the
 * application.
 */
void IO::Console::setupExternalConnections()
{
  // Read received data automatically
  auto dm = &Manager::instance();
  connect(dm, &Manager::dataSent, this, &IO::Console::onDataSent,
          Qt::QueuedConnection);
  connect(dm, &Manager::dataReceived, this, &IO::Console::onDataReceived,
          Qt::QueuedConnection);

  // Update lists when language changes
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Console::languageChanged);
}

/**
 * Sends the given @a data to the currently isConnected device using the options
 * specified by the user with the rest of the functions of this class.
 *
 * @note @c data is added to the history of sent commands, regardless if the
 * data writing was successfull or not.
 */
void IO::Console::send(const QString &data)
{
  // Check conditions
  if (!Manager::instance().isConnected())
    return;

  // Add user command to history
  if (!data.isEmpty())
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

  // Write data to device
  if (!bin.isEmpty())
    Manager::instance().writeData(bin);
}

/**
 * Enables/disables displaying a timestamp of each received data block.
 */
void IO::Console::setShowTimestamp(const bool enabled)
{
  if (showTimestamp() != enabled)
  {
    m_showTimestamp = enabled;
    Q_EMIT showTimestampChanged();
  }
}

/**
 * Enables/disables showing the sent data on the console
 */
void IO::Console::setEcho(const bool enabled)
{
  if (echo() != enabled)
  {
    m_echo = enabled;
    Q_EMIT echoChanged();
  }
}

/**
 * Modifies the number of lines that can be displayed by the terminal widget.
 */
void IO::Console::setScrollback(const int lines)
{
  if (scrollback() != lines)
  {
    m_scrollback = lines;
    Q_EMIT scrollbackChanged();
  }
}

/**
 * Changes the data mode for user commands. See @c dataMode() for more
 * information.
 */
void IO::Console::setDataMode(const IO::Console::DataMode &mode)
{
  m_dataMode = mode;
  Q_EMIT dataModeChanged();
}

/**
 * Changes line ending mode for sent user commands. See @c lineEnding() for more
 * information.
 */
void IO::Console::setLineEnding(const IO::Console::LineEnding &mode)
{
  m_lineEnding = mode;
  Q_EMIT lineEndingChanged();
}

/**
 * Changes the display mode of the console. See @c displayMode() for more
 * information.
 */
void IO::Console::setDisplayMode(const IO::Console::DisplayMode &mode)
{
  m_displayMode = mode;
  Q_EMIT displayModeChanged();
}

/**
 * Inserts the given @a string into the list of lines of the console, if @a
 * addTimestamp is set to @c true, an timestamp is added for each line.
 */
void IO::Console::append(const QString &string, const bool addTimestamp)
{
  // Abort on empty strings
  if (string.isEmpty())
    return;

  // Omit leading \n if a trailing \r was already rendered from previous payload
  auto data = string;
  if (m_lastCharWasCR && data.startsWith('\n'))
    data.removeFirst();

  // Record trailing \r
  m_lastCharWasCR = data.endsWith('\r');

  // Only use \n as line separator for rendering
  data = data.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  data = data.replace(QStringLiteral("\r"), QStringLiteral("\n"));

  // Get timestamp
  QString timestamp;
  if (addTimestamp)
  {
    QDateTime dateTime = QDateTime::currentDateTime();
    timestamp = dateTime.toString(QStringLiteral("HH:mm:ss.zzz -> "));
  }

  // Initialize final string
  QString processedString;
  processedString.reserve(data.length() + timestamp.length());

  // Create list with lines (keep separators)
  QStringList tokens;
  QString currentToken;
  for (int i = 0; i < data.length(); ++i)
  {
    if (data.at(i) == '\n')
    {
      tokens.append(currentToken);
      tokens.append(QStringLiteral("\n"));
      currentToken.clear();
    }

    else
      currentToken += data.at(i);
  }

  // Add last item to list
  if (!currentToken.isEmpty())
    tokens.append(currentToken);

  // Process lines
  while (!tokens.isEmpty())
  {
    auto token = tokens.first();
    if (m_isStartingLine && !token.simplified().isEmpty())
      processedString.append(timestamp);

    processedString.append(token);
    m_isStartingLine = (token == QStringLiteral("\n"));
    tokens.removeFirst();
  }

  // Add data to saved text buffer
  m_textBuffer.append(processedString.toUtf8());

  // Update UI
  QMetaObject::invokeMethod(
      this, [=] { Q_EMIT displayString(processedString); },
      Qt::QueuedConnection);
}

/**
 * Displays the given @a data in the console. @c QByteArray to ~@c QString
 * conversion is done by the @c dataToString() function, which displays incoming
 * data either in UTF-8 or in hexadecimal mode.
 */
void IO::Console::onDataSent(const QByteArray &data)
{
  if (echo())
    append(dataToString(data) + QStringLiteral("\n"), showTimestamp());
}

/**
 * Displays the given @a data in the console
 */
void IO::Console::onDataReceived(const QByteArray &data)
{
  append(dataToString(data), showTimestamp());
}

/**
 * Registers the given @a command to the list of sent commands.
 */
void IO::Console::addToHistory(const QString &command)
{
  // Remove old commands from history
  while (m_historyItems.count() > 100)
    m_historyItems.removeFirst();

  // Register command
  m_historyItems.append(command);
  m_historyItem = m_historyItems.count();
  Q_EMIT historyItemChanged();
}

/**
 * Converts the given @a data to a string according to the console display mode
 * set by the user.
 */
QString IO::Console::dataToString(const QByteArray &data)
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
 * @brief Converts a QByteArray to a QString, preserving printable characters
 *        and line breaks.
 *
 * Non-printable characters are replaced with a dot ('.') for readability.
 *
 * @param data The QByteArray to convert.
 * @return QString A human-readable string representation of the input data.
 */
QString IO::Console::plainTextStr(const QByteArray &data)
{
  // Filter out non-printable characters, but keep line breaks
  QString filteredData;
  filteredData.reserve(data.size());
  for (int i = 0; i < data.size(); ++i)
  {
    // clang-format off
    unsigned char ch = static_cast<unsigned char>(data[i]);
    bool printable = (data[i] == '\r') ||
                     (data[i] == '\n') ||
                      std::isprint(ch) ||
                      std::iscntrl(ch) ||
                      std::isspace(ch);
    // clang-format on

    printable = printable && (data[i] != '\0');
    filteredData += printable ? data[i] : '.';
  }

  // Return the filtered data
  return filteredData;
}

/**
 * Converts the given @a data into a HEX representation string.
 */
QString IO::Console::hexadecimalStr(const QByteArray &data)
{
  // Initialize parameters
  QString out;
  constexpr auto rowSize = 16;

  // Print hexadecimal row by row
  for (int i = 0; i < data.length(); i += rowSize)
  {
    // Add offset to output
    out += QStringLiteral("%1 | ").arg(i, 6, 16, QLatin1Char('0'));

    // Print hexadecimal bytes
    for (int j = 0; j < rowSize; ++j)
    {
      // Print existing data
      if (i + j < data.length())
      {
        out += QStringLiteral("%1 ").arg(
            static_cast<unsigned char>(data[i + j]), 2, 16, QLatin1Char('0'));
      }

      // Space out inexistent data
      else
        out += QStringLiteral("   ");

      // Add padding in 8th byte
      if ((j + 1) == 8)
        out += ' ';
    }

    // Add ASCII representation
    out += QStringLiteral("| ");
    for (int j = 0; j < rowSize; ++j)
    {
      // Add existing data
      if (i + j < data.length())
      {
        char c = data[i + j];
        if (std::isprint(static_cast<unsigned char>(c)))
          out += c;
        else
          out += '.';
      }

      // Add space for inexisting data
      else
        out += ' ';
    }

    // Add line break
    out += QStringLiteral(" |\n");
  }

  // Add additional line break & return data
  out += "\n";
  return out;
}
