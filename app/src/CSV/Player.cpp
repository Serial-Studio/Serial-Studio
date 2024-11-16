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

#include "Player.h"

#include <QtMath>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QApplication>
#include <QStandardPaths>

#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "Misc/Utilities.h"

/**
 * Constructor function
 */
CSV::Player::Player()
  : m_framePos(0)
  , m_playing(false)
  , m_timestamp("")
{
  qApp->installEventFilter(this);
  connect(this, &CSV::Player::playerStateChanged, this,
          &CSV::Player::updateData);
}

/**
 * Returns the only instance of the class
 */
CSV::Player &CSV::Player::instance()
{
  static Player singleton;
  return singleton;
}

/**
 * Returns @c true if an CSV file is open for reading
 */
bool CSV::Player::isOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * Returns the CSV playback progress in a range from 0.0 to 1.0
 */
qreal CSV::Player::progress() const
{
  return ((qreal)framePosition()) / frameCount();
}

/**
 * Returns @c true if the user is currently re-playing the CSV file at real-time
 * speed.
 */
bool CSV::Player::isPlaying() const
{
  return m_playing;
}

/**
 * Returns the total number of frames in the CSV file. This can be calculated
 * by getting the number of rows of the CSV and substracting 1 (because the
 * title cells do not count as a valid frame).
 */
int CSV::Player::frameCount() const
{
  return m_csvData.count();
}

/**
 * Returns the current row that we are using to create the JSON data that is
 * feed to the JsonParser class.
 */
int CSV::Player::framePosition() const
{
  return m_framePos;
}

/**
 * Returns the short filename of the current CSV file
 */
QString CSV::Player::filename() const
{
  if (isOpen())
  {
    auto fileInfo = QFileInfo(m_csvFile.fileName());
    return fileInfo.fileName();
  }

  return "";
}

/**
 * Returns the default path for CSV files
 */
QString CSV::Player::csvFilesPath() const
{
  // Get file name and path
  const auto path = QStringLiteral("%1/%2/CSV/")
                        .arg(QStandardPaths::writableLocation(
                                 QStandardPaths::DocumentsLocation),
                             qApp->applicationDisplayName());

  // Generate file path if required
  QDir dir(path);
  if (!dir.exists())
    dir.mkpath(".");

  return path;
}

/**
 * Returns the timestamp of the current data frame / row.
 */
const QString &CSV::Player::timestamp() const
{
  return m_timestamp;
}

/**
 * Enables CSV playback at 'live' speed (as it happened when CSV file was
 * saved to the computer).
 */
void CSV::Player::play()
{
  m_playing = true;
  Q_EMIT playerStateChanged();
}

/**
 * Pauses the CSV playback so that the user can see WTF happened at
 * certain point of the mission.
 */
void CSV::Player::pause()
{
  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * Toggles play/pause state
 */
void CSV::Player::toggle()
{
  m_playing = !m_playing;
  Q_EMIT playerStateChanged();
}

/**
 * Lets the user select a CSV file
 */
void CSV::Player::openFile()
{
  // Get file name
  auto file = QFileDialog::getOpenFileName(
      nullptr, tr("Select CSV file"), csvFilesPath(),
      tr("CSV files") + QStringLiteral(" (*.csv)"));

  // Open CSV file
  if (!file.isEmpty())
    openFile(file);
}

/**
 * Closes the file & cleans up internal variables. This helps us to reduice
 * memory usage & prepare the module to load another CSV file.
 */
void CSV::Player::closeFile()
{
  m_framePos = 0;
  m_csvFile.close();
  m_csvData.clear();
  m_csvData.squeeze();
  m_playing = false;
  m_timestamp = "--.--";

  Q_EMIT openChanged();
  Q_EMIT timestampChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Reads & processes the next CSV row, capped at the last row.
 *
 * Moves the frame position forward by one, up to the last frame in the CSV.
 * Resets and repopulates the dashboard with the appropriate frames for
 * synchronized display.
 */
void CSV::Player::nextFrame()
{
  if (framePosition() < frameCount() - 1)
  {
    // Increase the frame position
    ++m_framePos;

    // Populate the dashboard with a range of frames up to the new position
    UI::Dashboard::instance().resetData(false);
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    for (int i = startFrame; i <= m_framePos; ++i)
      IO::Manager::instance().processPayload(getFrame(i));

    // Keep timestamp and data in sync
    updateData();
  }
}

/**
 * @brief Reads & processes the previous CSV row, capped at the first row.
 *
 * Moves the frame position backward by one, down to the first frame in the CSV.
 * Resets and repopulates the dashboard with the appropriate frames for
 * synchronized display.
 */
void CSV::Player::previousFrame()
{
  if (framePosition() > 0)
  {
    // Decrease the frame position
    --m_framePos;

    // Populate the dashboard with a range of frames up to the new position
    UI::Dashboard::instance().resetData(false);
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    for (int i = startFrame; i <= m_framePos; ++i)
      IO::Manager::instance().processPayload(getFrame(i));

    // Keep timestamp and data in sync
    updateData();
  }
}

/**
 * @brief Opens a CSV file, processes its data, and prepares it for playback.
 *
 * This function attempts to open the specified CSV file for reading and
 * processes the data for replaying. It checks if a device is connected and,
 * if so, asks the user to disconnect it.
 *
 * The function reads the CSV file into a string matrix (`m_csvData`), validates
 * the date/time format of the first column, and if necessary, prompts the user
 * to either select a valid date/time column or manually set an interval between
 * rows.
 *
 * Once the date/time column is validated, the function sorts the rows in
 * `m_csvData` based on the date/time in the first column. After sorting, it
 * signals the UI to update and starts playback of the first frame of the CSV
 * data.
 *
 * If the file cannot be opened or an error occurs (e.g., invalid CSV data), the
 * function displays an appropriate error message and aborts further processing.
 *
 * @param filePath The file path of the CSV file to be opened.
 */
void CSV::Player::openFile(const QString &filePath)
{
  // File name empty, abort
  if (filePath.isEmpty())
    return;

  // Close previous file
  closeFile();

  // Device is connected, warn user & disconnect
  if (IO::Manager::instance().connected())
  {
    auto response = Misc::Utilities::showMessageBox(
        tr("Device Connection Active"),
        tr("To use this feature, you must disconnect from the device. "
           "Do you want to proceed?"),
        qAppName(), QMessageBox::No | QMessageBox::Yes);
    if (response == QMessageBox::Yes)
      IO::Manager::instance().disconnectDevice();
    else
      return;
  }

  // Try to open the current file
  m_csvFile.setFileName(filePath);
  if (m_csvFile.open(QIODevice::ReadOnly))
  {
    // Read CSV file into string matrix
    QTextStream in(&m_csvFile);
    while (!in.atEnd())
    {
      // Read a line and split it into a list of items
      QStringList row = in.readLine().split(',');

      // Remove surrounding quotes and trim whitespace from each item
      for (auto &item : row)
      {
        item = item.simplified();
        item.remove(QStringLiteral("\""));
      }

      // Filter out rows that are empty or contain only empty items
      bool isRowValid
          = !row.isEmpty()
            && std::any_of(row.cbegin(), row.cend(),
                           [](const QString &item) { return !item.isEmpty(); });

      // Only register valid rows
      if (isRowValid)
        m_csvData.append(row);
    }

    // Validate the cell at (1,1) for date/time format
    if (!getDateTime(1).isValid())
    {
      // Ask user to select date/time column or set interval manually
      if (!promptUserForDateTimeOrInterval())
      {
        closeFile();
        return;
      }
    }

    // Remove the header row from the data
    m_framePos = 0;
    m_csvData.removeFirst();

    // Begin reading data
    if (m_csvData.count() >= 2)
    {
      updateData();
      nextFrame();
      Q_EMIT openChanged();
    }

    // Handle case where CSV file does not contain at least two frames
    else
    {
      Misc::Utilities::showMessageBox(
          tr("Insufficient Data in CSV File"),
          tr("The CSV file must contain at least two frames (data rows) to "
             "proceed. Please check the file and try again."));
      closeFile();
    }
  }

  // Open error
  else
  {
    Misc::Utilities::showMessageBox(
        tr("Cannot read CSV file"),
        tr("Please check file permissions & location"));
    closeFile();
  }
}

/**
 * @brief Adjusts the playback position in the CSV data based on a normalized
 *        progress value.
 *
 * This function sets the playback position to a specified point within the CSV
 * file, where the position is defined by a normalized progress value between
 * 0 and 1. A value of 0 represents the start of the CSV file, and 1 represents
 * the end. When the new position differs from the current playback position,
 * this function processes the update by resetting and reloading the necessary
 * frames.
 *
 * If moving backward, the dashboard plots are reset silently and reloaded with
 * frames in the range leading up to the new position, effectively simulating a
 * rewind effect. For forward movement, the function caps the frame range to
 * prevent overshooting the end of the CSV.
 *
 * - **Backward Movement**: The dashboard is reset, and frames are reloaded from
 *   a range capped by the current position back to `UI::Dashboard::points()`
 *   frames earlier, or the start of the CSV, whichever comes first.
 * - **Forward Movement**: The dashboard is similarly reset, and frames are
 *   reloaded up to the new position, but capped to prevent going beyond the
 *   end of the CSV file.
 *
 * @param progress A normalized value between 0.0 and 1.0 representing the
 *                 desired position in the CSV file.
 *
 * This function ensures that:
 * - The playback position does not exceed the CSV boundaries.
 * - Plot data accurately reflects the new position on both forward and
 *   backward movements.
 * - Timestamp tracking remains synchronized with the current playback position.
 *
 * @note A silent reset is performed to refresh the dashboard plots without
 *       emitting unnecessary signals.
 */
void CSV::Player::setProgress(const qreal progress)
{
  // Clamp progress between 0 and 1
  const auto validProgress = std::clamp(progress, 0.0, 1.0);

  // Pause if playing to avoid interference with the timer
  if (isPlaying())
    pause();

  // Calculate new frame position based on progress
  int newFramePos = qCeil(frameCount() * validProgress);

  // Only process if position changes
  if (newFramePos != m_framePos)
  {
    // Update frame position and reset dashboard data silently
    m_framePos = newFramePos;
    UI::Dashboard::instance().resetData(false);

    // Calculate frames to load around the new frame position
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    int endFrame = std::min(frameCount() - 1, m_framePos);

    // Populate dashboard with frames within capped range
    for (int i = startFrame; i <= endFrame; ++i)
      IO::Manager::instance().processPayload(getFrame(i));

    // Update with current data
    updateData();
  }
}

/**
 * Generates a JSON data frame by combining the values of the current CSV
 * row & the structure of the JSON map file loaded in the @c JsonParser class.
 *
 * If playback is enabled, this function calculates the difference in
 * milliseconds between the current row and the next row & schedules a re-call
 * of this function using a timer.
 */
void CSV::Player::updateData()
{
  // File not open, abort
  if (!isOpen())
    return;

  // Update timestamp string
  bool error = true;
  auto timestamp = getCellValue(framePosition(), 0, error);
  if (!error)
  {
    m_timestamp = timestamp;
    Q_EMIT timestampChanged();
  }

  // Construct frame from CSV and send it to the IO manager
  IO::Manager::instance().processPayload(getFrame(framePosition() + 1));

  // If the user wants to 'play' the CSV, get time difference between this
  // frame and the next frame & schedule an automated update
  if (isPlaying())
  {
    // Get first frame
    if (framePosition() < frameCount())
    {
      // Obtain time for current & next frame
      auto currTime = getDateTime(framePosition());
      auto nextTime = getDateTime(framePosition() + 1);

      // No error, calculate difference & schedule update
      if (currTime.isValid() && nextTime.isValid())
      {
        // Obtain millis between the two frames
        const auto msecsToNextF = abs(currTime.msecsTo(nextTime));

        // Jump to next frame
        QTimer::singleShot(msecsToNextF, Qt::PreciseTimer, this, [=] {
          if (isOpen() && isPlaying() && framePosition() < frameCount())
          {
            ++m_framePos;
            updateData();
          }
        });
      }

      // Error - pause playback
      else
      {
        pause();
        qWarning() << "Error getting timestamp difference" << currTime
                   << nextTime;
      }
    }

    // Pause at end of CSV
    else
      pause();
  }
}

/**
 * @brief Prompts the user to select how to handle date/time data in the CSV.
 *
 * This function checks if the CSV file has valid headers, then asks the user
 * to either select a date/time column or manually enter an interval between
 * rows in milliseconds. Depending on the user's choice, either date/time
 * values are generated based on the interval, or an existing column is
 * converted to date/time values.
 *
 * @return true if the user successfully provided a date/time option or
 *         interval, false if cancelled or invalid input.
 */
bool CSV::Player::promptUserForDateTimeOrInterval()
{
  // Check if there are headers available for the combobox
  if (m_csvData.isEmpty() || m_csvData.first().isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid CSV"),
        tr("The CSV file does not contain any data or headers."));
    return false;
  }

  // Obtain header labels
  const auto headerLabels = m_csvData.first().toList();

  // Ask the user if they want to select a date/time column or enter an interval
  bool ok;
  QStringList options;
  options << tr("Select a date/time column") << tr("Set interval manually");
  QString choice = QInputDialog::getItem(
      nullptr, tr("CSV Date/Time Selection"),
      tr("Choose how to handle the date/time data:"), options, 0, false, &ok);

  // Check if user cancelled
  if (!ok)
    return false;

  // Ask the user to input the interval in milliseconds
  if (choice == tr("Set interval manually"))
  {
    const auto interval = QInputDialog::getInt(
        nullptr, tr("Set Interval"),
        tr("Please enter the interval between rows in milliseconds:"), 1000, 1,
        1000000, 1, &ok);

    if (ok)
    {
      generateDateTimeForRows(interval);
      return true;
    }
  }

  // Ask user to pick a date/time column
  else
  {
    const auto column = QInputDialog::getItem(
        nullptr, tr("Select Date/Time Column"),
        tr("Please select the column that contains the date/time data:"),
        headerLabels, 0, false, &ok);

    if (ok)
    {
      // Find the index of the selected column
      int columnIndex = headerLabels.indexOf(column);
      if (columnIndex == -1)
      {
        Misc::Utilities::showMessageBox(
            tr("Invalid Selection"), tr("The selected column is not valid."));
        return false;
      }

      // Convert the selected column to date/time
      convertColumnToDateTime(columnIndex);
      return true;
    }
  }

  // Should not reach here
  return false;
}

/**
 * @brief Generates date/time values for each row based on a fixed interval.
 *
 * This function generates date/time strings starting from the current time
 * and increments them by a user-specified interval in milliseconds. It then
 * prepends the generated date/time string to each row of the CSV data.
 *
 * @param interval The interval in milliseconds between each row.
 */
void CSV::Player::generateDateTimeForRows(int interval)
{
  const auto startTime = QDateTime::currentDateTime();
  const auto format = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");

  for (int i = 0; i < m_csvData.size(); ++i)
  {
    QString dateTimeString = startTime.addMSecs(i * interval).toString(format);
    m_csvData[i].prepend(dateTimeString);
  }
}

/**
 * @brief Converts the specified column in the CSV data to a date/time format
 *        and moves it to the start of each row.
 *
 * This function processes the specified column (excluding the header row) in
 * the CSV data and converts each value to a `QDateTime` object using the
 * formats defined in the `getDateTime` function. If a valid `QDateTime` is not
 * found, the current date/time is used. The converted date/time string is then
 * moved to the start of each row.
 *
 * The header row (row 0) is excluded from this operation to ensure it remains
 * intact. The selected column is removed from its original position and moved
 * to the start of the row as a formatted date/time string.
 *
 * @param columnIndex The index of the column to convert and move to the start
 * of each row.
 */
void CSV::Player::convertColumnToDateTime(int columnIndex)
{
  const auto format = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");
  for (int i = 1; i < m_csvData.size(); ++i)
  {
    // Parse the date/time from the specified column
    auto dateTime = getDateTime(i);
    if (!dateTime.isValid())
      dateTime = QDateTime::currentDateTime();

    // Move the $TIME column to the start
    m_csvData[i].remove(columnIndex);
    m_csvData[i].prepend(dateTime.toString(format));
  }
}

/**
 * @brief Retrieves the date/time value from a specific cell in the CSV.
 *
 * This function attempts to parse the date/time value in the first column of
 * the specified cell. It tries several predefined date/time formats, returning
 * the first valid date/time it finds. If no valid date/time can be found, an
 * invalid QDateTime object is returned.
 *
 * @param cell The index of the cell to retrieve the date/time from.
 * @return QDateTime The parsed date/time value or an invalid QDateTime if
 *                   parsing fails.
 */
QDateTime CSV::Player::getDateTime(const int row)
{
  bool error;
  auto value = getCellValue(row, 0, error);

  if (!error)
    return getDateTime(value);

  return QDateTime();
}

/**
 * @brief Parses a date/time string from a given cell in the CSV data.
 *
 * This function attempts to convert a string into a `QDateTime` object using a
 * list of predefined date/time formats. It iterates through multiple formats to
 * find a valid match. If no valid format is found, an invalid `QDateTime`
 * object is returned.
 *
 * The function uses common date/time formats such as "yyyy/MM/dd HH:mm:ss::zzz"
 * and its variations to handle different possible formats that might be present
 * in the CSV file.
 *
 * @param cell The string representing the cell in the CSV that contains the
 *             date/time data.
 *
 * @return QDateTime The parsed `QDateTime` object. If no valid date/time format
 *                   is found, an invalid `QDateTime` is returned.
 */
QDateTime CSV::Player::getDateTime(const QString &cell)
{
  // Initialize parameters
  QDateTime dateTime;

  // Create a list of available date/time formats
  static const QStringList formats
      = {QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz"),
         QStringLiteral("yyyy/MM/dd/ HH:mm:ss::zzz"),
         QStringLiteral("yyyy/MM/dd HH:mm:ss"),
         QStringLiteral("yyyy/MM/dd/ HH:mm:ss")};

  // Try to obtain date/time string
  for (const auto &format : formats)
  {
    dateTime = QDateTime::fromString(cell, format);
    if (dateTime.isValid())
      break;
  }

  // Return date/time
  return dateTime;
}

/**
 * Generates a frame from the data at the given @a row. The first item of each
 * row is ignored because it contains the RX date/time, which is used to
 * regulate the interval at which the frames are parsed.
 */
QByteArray CSV::Player::getFrame(const int row)
{
  QByteArray frame;

  if (m_csvData.count() > row)
  {
    const auto &list = m_csvData[row];
    for (int i = 1; i < list.count(); ++i)
    {
      frame.append(list[i].toUtf8());
      if (i < list.count() - 1)
        frame.append(',');
      else
        frame.append('\n');
    }
  }

  return frame;
}

/**
 * Safely returns the value in the cell at the given @a row & @a column. If an
 * error occurs or the cell does not exist, the value of @a error shall be set
 * to @c true.
 */
const QString &CSV::Player::getCellValue(const int row, const int column,
                                         bool &error)
{
  static auto defaultValue = QStringLiteral("");

  if (m_csvData.count() > row)
  {
    const auto &list = m_csvData[row];
    if (list.count() > column)
    {
      error = false;
      return list[column];
    }
  }

  error = true;
  return defaultValue;
}

/**
 * @brief Event filter to capture and handle key events for playback controls.
 *
 * This function intercepts key events when the CSV player is open and routes
 * them to `handleKeyPress` for processing. If the key event corresponds to a
 * playback control (e.g., play/pause, next, previous), the event is handled;
 * otherwise, it is passed to the default event filter.
 *
 * @param obj The object that the event is dispatched to.
 * @param event The event being processed.
 * @return true if the event was handled, false otherwise.
 */
bool CSV::Player::eventFilter(QObject *obj, QEvent *event)
{
  if (isOpen() && event->type() == QEvent::KeyPress)
  {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

/**
 * @brief Handles key press events to control playback actions.
 *
 * This function is called by the `eventFilter` when a key press event occurs.
 * It checks for specific keys related to media control (e.g., space for
 * play/pause, arrow keys for frame navigation) and invokes the corresponding
 * playback function.
 *
 * Supported keys:
 * - Space, MediaPlay, MediaPause, MediaTogglePlayPause: Toggles play/pause.
 * - Left, Down, MediaPrevious: Moves to the previous frame.
 * - Right, Up, MediaNext: Moves to the next frame.
 *
 * @param keyEvent The key event to handle.
 * @return true if the event was handled, false otherwise.
 */
bool CSV::Player::handleKeyPress(QKeyEvent *keyEvent)
{
  bool handled;
  switch (keyEvent->key())
  {
    case Qt::Key_Space:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
      toggle();
      handled = true;
      break;
    case Qt::Key_Left:
    case Qt::Key_Down:
    case Qt::Key_MediaPrevious:
      previousFrame();
      handled = true;
      break;
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_MediaNext:
      nextFrame();
      handled = true;
      break;
    default:
      handled = false;
      break;
  }

  return handled;
}
