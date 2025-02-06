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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#include <QFile>
#include <QDateTime>

#include "IO/Manager.h"
#include "IO/Console.h"
#include "Misc/Translator.h"

/**
 * Generates a hexdump of the given data
 */
static QString HexDump(const char *data, const size_t size)
{
  QString result;
  char ascii[17];
  ascii[16] = '\0';

  for (size_t i = 0; i < size; ++i)
  {
    char hexBuffer[4];
    snprintf(hexBuffer, sizeof(hexBuffer), "%02X ",
             static_cast<quint8>(data[i]));
    result += hexBuffer;

    if (data[i] >= ' ' && data[i] <= '~')
      ascii[i % 16] = data[i];
    else
      ascii[i % 16] = '.';

    if ((i + 1) % 8 == 0 || i + 1 == size)
    {
      result += ' ';
      if ((i + 1) % 16 == 0)
        result += QString("|  %1 \n").arg(ascii);

      else if (i + 1 == size)
      {
        ascii[(i + 1) % 16] = '\0';

        if ((i + 1) % 16 <= 8)
          result += ' ';
        for (size_t j = (i + 1) % 16; j < 16; ++j)
          result += "   ";

        result += QString("|  %1 \n").arg(ascii);
      }
    }
  }

  return result;
}

/**
 * Constructor function
 */
IO::Console::Console()
  : m_dataMode(DataMode::DataUTF8)
  , m_lineEnding(LineEnding::NoLineEnding)
  , m_displayMode(DisplayMode::DisplayPlainText)
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

// Calculates the Fletcher-16 checksum for a QByteArray
static unsigned short fletcher16(unsigned short cksum_init,
                                 const unsigned char *data,
                                 unsigned int size)
{
  unsigned char a = (unsigned char)(cksum_init & 0xFF);
  unsigned char b = (unsigned char)((cksum_init >> 8) & 0xFF);

  for (unsigned int i = 0; i < size; i++)
  {
    a += data[i];
    b += a;
  }

  return (static_cast<unsigned short>(b) << 8) | a;
}



static QByteArray buildControlPacket(quint8 controlType, float value)
{
  // Packet layout:
  // [0]  = 0x47
  // [1]  = 0x41
  // [2]  = 0x03
  // [3:4] = 0x03 (unsigned short) -> 2 bytes
  // [5]  = controlType
  // [6:9] = float (value)
  //[10:11]= Fletcher-16 checksum

  QByteArray packet;
  packet.append(char(0x47)); // pre1
  packet.append(char(0x41)); // pre2
  packet.append(char(0x03)); // packid

  // packlen = 5, which is 0x0005 in little-endian
  unsigned short packlen = 5;
  packet.append(reinterpret_cast<const char*>(&packlen), sizeof(packlen));

  // controlType
  packet.append(char(controlType));

  //controltarget (float) - 4 bytes
  packet.append(reinterpret_cast<const char*>(&value), sizeof(value));

  // ----- Compute Fletcher-16 over these bytes -----
  unsigned short csum = fletcher16(
      0,                             // start with c=0
      reinterpret_cast<const unsigned char*>(packet.data()),
      packet.size()
      );

  // 5) Append 2-byte CRC at the end, little-endian
  packet.append(static_cast<char>(csum & 0xFF));
  packet.append(static_cast<char>((csum >> 8) & 0xFF));

  return packet;
}


/**
 * Sends the given @a data to the currently connected device using the options
 * specified by the user with the rest of the functions of this class.
 *
 * @note @c data is added to the history of sent commands, regardless if the
 * data writing was successfull or not.
 */
void IO::Console::send(const QString &data)
{
  // Check conditions
  if (!Manager::instance().connected())
    return;

         // Add user command to history
  if (!data.isEmpty())
    addToHistory(data);

         // ----------------------------------------------------------
         // 1) Intercept user commands for special packet encoding
         // ----------------------------------------------------------
         // We'll convert the user string to lowercase and trim to simplify matching
  QString lowerData = data.trimmed().toLower();

         // --- BANK ANGLE COMMAND ---
         // Check if it begins with "bank angle" (ignoring how many spaces/equal signs afterward)
  if (lowerData.startsWith("bank angle"))
  {
    int eqIndex = lowerData.indexOf('=');
    if (eqIndex != -1)
    {
      // Extract and convert everything after '=' into float
      float value = lowerData.mid(eqIndex + 1).trimmed().toFloat();

             // For "bank angle", we said controlType = 0
      QByteArray bin = buildControlPacket(0, value);

             // Send it out
      Manager::instance().writeData(bin);
      return; // Done
    }
  }

         // --- COG COMMAND ---
         // Check if it begins with "cog"
  else if (lowerData.startsWith("cog"))
  {
    int eqIndex = lowerData.indexOf('=');
    if (eqIndex != -1)
    {
      // Extract and convert everything after '=' into float
      float value = lowerData.mid(eqIndex + 1).trimmed().toFloat();

             // For "COG", we said controlType = 1
      QByteArray bin = buildControlPacket(1, value);

             // Send it out
      Manager::instance().writeData(bin);
      return;
    }
  }


// ----------------------------------------------------------
// 2) If no special command, fall back to normal data handling
// ----------------------------------------------------------

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
    if (m_isStartingLine)
      processedString.append(timestamp);

    auto token = tokens.first();
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
 * Converts the given @a data into an UTF-8 string
 */
QString IO::Console::plainTextStr(const QByteArray &data)
{
  QString str = QString::fromUtf8(data);

  if (str.toUtf8() != data)
    str = QString::fromLatin1(data);

  return str;
}

/**
 * Converts the given @a data into a HEX representation string.
 */
QString IO::Console::hexadecimalStr(const QByteArray &data)
{
  // Remove line breaks from data
  QByteArray copy = data;

  // Convert data to string with dump every ~80 chars
  QString str;
  const int characters = 80;
  for (int i = 0; i < copy.length(); i += characters)
  {
    QByteArray line;
    for (int j = 0; j < qMin(characters, copy.length() - i); ++j)
      line.append(copy.at(i + j));

    str.append(HexDump(line.data(), line.size()));
  }

  // Return string
  return str;
}
