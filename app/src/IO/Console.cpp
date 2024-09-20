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

#include <QFile>
#include <QPrinter>
#include <QDateTime>
#include <QFileDialog>
#include <QPrintDialog>
#include <QTextDocument>

#include "IO/Manager.h"
#include "IO/Console.h"
#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"

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
 * @brief Custom message handler for Qt debug, warning, critical, and fatal
 *        messages.
 *
 * This static function handles messages of various types (debug, warning,
 * critical, fatal) emitted by the Qt framework.
 *
 * It formats the message with a corresponding prefix based on the message type
 * (e.g., "[Debug]", "[Warning]").
 *
 * @param type The type of the message.
 * @param context The message log context (file, line, function).
 * @param msg The actual message that was emitted by the Qt system.
 *
 * @note The function appends the formatted message to the console handler.
 */
static void MessageHandler(QtMsgType type, const QMessageLogContext &context,
                           const QString &msg)
{
  // Skip empty messages
  if (msg.isEmpty())
    return;

  // Show message & warning level
  QString output;
  switch (type)
  {
    case QtInfoMsg:
      output = QStringLiteral("⧗ %1").arg(msg);
      break;
    case QtDebugMsg:
      output = QStringLiteral("\n[DEBUG]: %1").arg(msg);
      break;
    case QtWarningMsg:
      output = QStringLiteral("\n[WARNING]: %1").arg(msg);
      break;
    case QtCriticalMsg:
      output = QStringLiteral("\n[CRITICAL]: %1").arg(msg);
      break;
    case QtFatalMsg:
      output = QStringLiteral("\n[FATAL]: %1").arg(msg);
      break;
    default:
      break;
  }

  // Append context information (file and line)
  if (context.file && context.line > 0)
  {
    output.append(QStringLiteral(" (File: %1, Line: %2)")
                      .arg(context.file)
                      .arg(context.line));
  }

  // Add a newline at the end
  output.append("\n");

  // Use IO::Manager signal to avoid messing up tokens in console
  Q_EMIT IO::Manager::instance().dataReceived(output.toUtf8());
}

/**
 * Constructor function
 */
IO::Console::Console()
  : m_dataMode(DataMode::DataUTF8)
  , m_lineEnding(LineEnding::NoLineEnding)
  , m_displayMode(DisplayMode::DisplayPlainText)
  , m_historyItem(0)
  , m_echo(false)
  , m_autoscroll(true)
  , m_showTimestamp(false)
  , m_isStartingLine(true)
  , m_lastCharWasCR(false)
{
  // Clear buffer & reserve memory
  clear();

  // Read received data automatically
  auto dm = &Manager::instance();
  connect(dm, &Manager::dataSent, this, &IO::Console::onDataSent);
  connect(dm, &Manager::dataReceived, this, &IO::Console::onDataReceived);

  // Install custom message handler to redirect qDebug output to console
  qInstallMessageHandler(MessageHandler);
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
 * Returns @c true if the console shall display the commands that the user has
 * sent to the serial/network device.
 */
bool IO::Console::echo() const
{
  return m_echo;
}

/**
 * Returns @c true if the vertical position of the console display shall be
 * automatically moved to show latest data.
 */
bool IO::Console::autoscroll() const
{
  return m_autoscroll;
}

/**
 * Returns @c true if data buffer contains information that the user can export.
 */
bool IO::Console::saveAvailable() const
{
  return m_textBuffer.length() > 0;
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
QStringList &IO::Console::dataModes()
{
  static QStringList list;
  if (list.isEmpty())
  {
    list.append(tr("ASCII"));
    list.append(tr("HEX"));
  }

  return list;
}

/**
 * Returns a list with the available line endings options. This list must be
 * synchronized with the order of the @c LineEnding enums.
 */
QStringList &IO::Console::lineEndings()
{
  static QStringList list;
  if (list.isEmpty())
  {
    list.append(tr("No Line Ending"));
    list.append(tr("New Line"));
    list.append(tr("Carriage Return"));
    list.append(tr("CR + NL"));
  }

  return list;
}

/**
 * Returns a list with the available console display modes. This list must be
 * synchronized with the order of the @c DisplayMode enums.
 */
QStringList &IO::Console::displayModes()
{
  static QStringList list;
  if (list.isEmpty())
  {
    list.append(tr("Plain Text"));
    list.append(tr("Hexadecimal"));
  }

  return list;
}

/**
 * Validates the given @a text and adds space to display the text in a
 * byte-oriented view
 */
QString IO::Console::formatUserHex(const QString &text)
{
  // Remove spaces & stuff
  auto data = text.simplified();
  data = data.replace(QStringLiteral(" "), "");

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
 * Allows the user to export the information displayed on the console
 */
void IO::Console::save()
{
  // No data buffer received, abort
  if (!saveAvailable())
    return;

  // Get file name
  auto path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Export Console Data"),
                                           QDir::homePath(),
                                           tr("Text Files") + " (*.txt)");

  // Create file
  if (!path.isEmpty())
  {
    QFile file(path);
    if (file.open(QFile::WriteOnly))
    {
      file.write(m_textBuffer.toUtf8());
      file.close();
      Misc::Utilities::revealFile(path);
    }

    else
      Misc::Utilities::showMessageBox(tr("Error while exporting console data"),
                                      file.errorString());
  }
}

/**
 * Deletes all the text displayed by the current QML text document
 */
void IO::Console::clear()
{
  m_textBuffer.clear();
  m_textBuffer.reserve(10 * 1000);
  m_isStartingLine = true;
  m_lastCharWasCR = false;
  Q_EMIT dataReceived();
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
 * Enables or disables displaying sent data in the console screen. See @c echo()
 * for more information.
 */
void IO::Console::setEcho(const bool enabled)
{
  m_echo = enabled;
  Q_EMIT echoChanged();
}

/**
 * Creates a text document with current console output & prints it using native
 * system libraries/toolkits.
 */
void IO::Console::print()
{
  // Create text document
  QTextDocument document;
  document.setPlainText(m_textBuffer);
  document.setDefaultFont(Misc::CommonFonts::instance().monoFont());

  // Create printer object
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setFullPage(true);
  printer.setDocName(qApp->applicationName());
  printer.setPageOrientation(QPageLayout::Portrait);

  // Show print dialog
  QPrintDialog printDialog(&printer, Q_NULLPTR);
  if (printDialog.exec() == QDialog::Accepted)
  {
    document.print(&printer);
  }
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
 * Enables/disables autoscrolling of the console text.
 */
void IO::Console::setAutoscroll(const bool enabled)
{
  if (autoscroll() != enabled)
  {
    m_autoscroll = enabled;
    Q_EMIT autoscrollChanged();
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

  auto data = string;

  // Omit leading \n if a trailing \r was already rendered from previous payload
  if (m_lastCharWasCR && data.startsWith('\n'))
    data.removeFirst();

  // Record trailing \r
  m_lastCharWasCR = data.endsWith('\r');

  // Only use \n as line separator for rendering
  data = data.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
  data = data.replace(QStringLiteral("\n\r"), QStringLiteral("\n"));
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
  m_textBuffer.append(processedString);

  // Update UI
  Q_EMIT dataReceived();
  Q_EMIT stringReceived(processedString);
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
 * Converts the given @a data in HEX format into real binary data.
 */
QByteArray IO::Console::hexToBytes(const QString &data)
{
  QString withoutSpaces = data;
  withoutSpaces.replace(QStringLiteral(" "), "");

  QByteArray array;
  for (int i = 0; i < withoutSpaces.length(); i += 2)
  {
    auto chr1 = withoutSpaces.at(i);
    auto chr2 = withoutSpaces.at(i + 1);
    auto byte = QStringLiteral("%1%2").arg(chr1, chr2).toInt(Q_NULLPTR, 16);
    array.append(byte);
  }

  return array;
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
  for (int i = 0; i < copy.length(); ++i)
  {
    QByteArray line;
    for (int j = 0; j < qMin(characters, copy.length() - i); ++j)
      line.append(copy.at(i + j));

    i += characters;
    str.append(HexDump(line.data(), line.size()));
  }

  // Return string
  return str;
}
