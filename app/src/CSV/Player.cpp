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

#include "Player.h"

#include <QtMath>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QApplication>

#include "IO/Manager.h"
#include "UI/Dashboard.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "DataModel/FrameBuilder.h"

/**
 * Constructor function
 */
CSV::Player::Player()
  : m_framePos(0)
  , m_playing(false)
  , m_timestamp("")
  , m_startTimestampSeconds(0.0)
  , m_useHighPrecisionTimestamps(false)
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
double CSV::Player::progress() const
{
  const auto count = frameCount();
  if (count <= 0)
    return 0.0;

  return static_cast<double>(framePosition()) / count;
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
  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  // Initialize elapsed timer and record the starting CSV timestamp
  m_startTimestamp = getDateTime(m_framePos);
  m_elapsedTimer.start();

  // Update high-precision timestamp baseline for seeking support
  if (m_useHighPrecisionTimestamps && m_framePos < m_timestampCache.size())
    m_startTimestampSeconds = m_timestampCache[m_framePos];

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
  if (m_playing)
    pause();
  else
    play();
}

/**
 * Lets the user select a CSV file
 */
void CSV::Player::openFile()
{
  auto *dialog = new QFileDialog(nullptr, tr("Select CSV file"),
                                 Misc::WorkspaceManager::instance().path("CSV"),
                                 tr("CSV files (*.csv)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            if (!path.isEmpty())
              openFile(path);

            dialog->deleteLater();
          });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);

  dialog->open();
}

/**
 * Closes the file & cleans up internal variables. This helps us to reduice
 * memory usage & prepare the module to load another CSV file.
 */
void CSV::Player::closeFile()
{
  if (!isOpen())
    return;

  m_playing = false;
  m_framePos = 0;
  m_csvFile.close();
  m_csvData.clear();
  m_csvData.squeeze();
  m_timestamp = "--.--";
  m_timestampCache.clear();
  m_useHighPrecisionTimestamps = false;
  m_startTimestampSeconds = 0.0;

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());

  Q_EMIT openChanged();
  Q_EMIT timestampChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Reads & processes the next CSV row, capped at the last row.
 *
 * Moves the frame position forward by one, up to the last frame in the CSV.
 * Clears plot data and repopulates the dashboard with the appropriate frames
 * for synchronized display.
 */
void CSV::Player::nextFrame()
{
  if (framePosition() < frameCount() - 1)
  {
    // Increase the frame position
    ++m_framePos;

    // Clear only plot data (preserves widget layout)
    UI::Dashboard::instance().clearPlotData();

    // Populate the dashboard with a range of frames up to the new position
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

    // Keep timestamp and data in sync
    updateData();
  }
}

/**
 * @brief Reads & processes the previous CSV row, capped at the first row.
 *
 * Moves the frame position backward by one, down to the first frame in the CSV.
 * Clears plot data and repopulates the dashboard with the appropriate frames
 * for synchronized display.
 */
void CSV::Player::previousFrame()
{
  if (framePosition() > 0)
  {
    // Decrease the frame position
    --m_framePos;

    // Clear only plot data (preserves widget layout)
    UI::Dashboard::instance().clearPlotData();

    // Populate the dashboard with a range of frames up to the new position
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

    // Keep timestamp and data in sync
    updateData();
  }
}

/**
 * @brief Opens a CSV file, processes its data, and prepares it for playback.
 *
 * This function attempts to open the specified CSV file for reading and
 * processes the data for replaying. It checks if a device is isConnected and,
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
  if (IO::Manager::instance().isConnected())
  {
    auto response = Misc::Utilities::showMessageBox(
        tr("Device Connection Active"),
        tr("To use this feature, you must disconnect from the device. "
           "Do you want to proceed?"),
        QMessageBox::Warning, qAppName(), QMessageBox::No | QMessageBox::Yes);
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

    // Detect timestamp format and validate
    // First try high-precision format (fractional seconds)
    bool error = false;
    QString firstCell = getCellValue(1, 0, error);
    double firstTimestamp = error ? -1.0 : getTimestampSeconds(firstCell);

    if (firstTimestamp >= 0.0)
    {
      // High-precision timestamps detected - build cache
      m_timestampCache.clear();
      m_timestampCache.reserve(m_csvData.count());

      // Cache all timestamps for efficient playback
      for (int i = 0; i < m_csvData.count(); ++i)
      {
        bool err = false;
        QString cell = getCellValue(i, 0, err);
        double ts = err ? 0.0 : getTimestampSeconds(cell);
        m_timestampCache.append(ts);
      }

      m_startTimestampSeconds = m_timestampCache[1];
      m_useHighPrecisionTimestamps = true;
    }
    // Fall back to legacy date/time format
    else if (getDateTime(1).isValid())
    {
      m_useHighPrecisionTimestamps = false;
      m_timestampCache.clear();
    }
    // Neither format worked - prompt user
    else
    {
      m_useHighPrecisionTimestamps = false;
      m_timestampCache.clear();

      // Ask user to select date/time column or set interval manually
      if (!promptUserForDateTimeOrInterval())
      {
        closeFile();
        return;
      }
    }

    // Send the header row before removing it
    sendHeaderFrame();

    // Remove the header row from the data
    m_framePos = 0;
    m_csvData.removeFirst();

    // Adjust cached timestamps if header was removed
    if (m_useHighPrecisionTimestamps && !m_timestampCache.isEmpty())
    {
      m_timestampCache.removeFirst();
      if (!m_timestampCache.isEmpty())
        m_startTimestampSeconds = m_timestampCache[0];
    }

    // Begin reading data
    if (m_csvData.count() >= 1)
    {
      updateData();
      Q_EMIT openChanged();
    }

    // Handle case where CSV file does not contain at least two frames
    else
    {
      Misc::Utilities::showMessageBox(
          tr("Insufficient Data in CSV File"),
          tr("The CSV file must contain at least one data row to "
             "proceed. Please check the file and try again."),
          QMessageBox::Critical);
      closeFile();
    }
  }

  // Open error
  else
  {
    Misc::Utilities::showMessageBox(
        tr("Cannot read CSV file"),
        tr("Please check file permissions & location"), QMessageBox::Critical);
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
void CSV::Player::setProgress(const double progress)
{
  // Clamp progress between 0 and 1
  const auto validProgress = std::clamp(progress, 0.0, 1.0);

  // Pause if playing to avoid interference with the timer
  if (isPlaying())
    pause();

  // Calculate new frame position based on progress
  int newFramePos = qMin(frameCount() - 1, qCeil(frameCount() * validProgress));

  // Only process if position changes
  if (newFramePos != m_framePos)
  {
    // Update frame position
    m_framePos = newFramePos;

    // Clear only plot data (preserves widget layout)
    UI::Dashboard::instance().clearPlotData();

    // Calculate frames to load around the new frame position
    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame = std::max(1, m_framePos - framesToLoad);
    int endFrame = std::min(frameCount() - 1, m_framePos);

    // Populate dashboard with frames within capped range
    processFrameBatch(startFrame, endFrame);

    // Update with current data
    updateData();
  }
}

/**
 * @brief Updates the timestamp display for the current frame position.
 */
void CSV::Player::updateTimestampDisplay()
{
  bool err = true;
  auto ts = getCellValue(m_framePos, 0, err);
  if (err)
    return;

  double seconds = getTimestampSeconds(ts);
  if (seconds >= 0.0)
    m_timestamp = formatTimestamp(seconds);

  else
  {
    auto frameTime = getDateTime(ts);
    if (frameTime.isValid() && m_startTimestamp.isValid())
    {
      qint64 elapsedMs = m_startTimestamp.msecsTo(frameTime);
      m_timestamp = formatTimestamp(elapsedMs / 1000.0);
    }

    else
      m_timestamp = ts;
  }

  Q_EMIT timestampChanged();
}

/**
 * @brief Processes current frame and schedules next frame for playback.
 */
void CSV::Player::updateData()
{
  if (!isOpen())
    return;

  updateTimestampDisplay();
  if (!isPlaying())
    return;

  IO::Manager::instance().processPayload(getFrame(framePosition()));

  if (framePosition() >= frameCount() - 1)
  {
    pause();
    return;
  }

  const qint64 elapsedMs = m_elapsedTimer.elapsed();
  qint64 msUntilNext = 0;

  if (m_useHighPrecisionTimestamps)
  {
    if (framePosition() + 1 >= m_timestampCache.size())
    {
      pause();
      return;
    }

    const double targetTime = m_startTimestampSeconds + (elapsedMs / 1000.0);
    const double nextTime = m_timestampCache[framePosition() + 1];
    msUntilNext
        = qMax(0LL, static_cast<qint64>((nextTime - targetTime) * 1000.0));
  }

  else
  {
    const QDateTime targetTime = m_startTimestamp.addMSecs(elapsedMs);
    const auto nextTime = getDateTime(framePosition() + 1);
    if (!nextTime.isValid())
    {
      pause();
      return;
    }

    msUntilNext = targetTime.msecsTo(nextTime);
  }

  if (msUntilNext <= 0)
  {
    constexpr int kMaxBatchSize = 100;
    int processed = 0;
    while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize
           && msUntilNext <= 0)
    {
      ++m_framePos;
      IO::Manager::instance().processPayload(getFrame(m_framePos));
      ++processed;

      if (m_useHighPrecisionTimestamps)
      {
        if (m_framePos < m_timestampCache.size())
        {
          const double target
              = m_startTimestampSeconds + (m_elapsedTimer.elapsed() / 1000.0);
          const double next = m_timestampCache[m_framePos];
          msUntilNext
              = qMax(0LL, static_cast<qint64>((next - target) * 1000.0));
        }
      }

      else
      {
        auto target = m_startTimestamp.addMSecs(m_elapsedTimer.elapsed());
        auto next = getDateTime(m_framePos);
        if (next.isValid())
          msUntilNext = target.msecsTo(next);
      }
    }

    updateTimestampDisplay();

    if (m_framePos < frameCount() - 1)
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this,
                         [this] {
                           if (isOpen() && isPlaying())
                             updateData();
                         });
    else
      pause();
  }

  else
  {
    QTimer::singleShot(msUntilNext, Qt::PreciseTimer, this, [this] {
      if (!isOpen() || !isPlaying())
        return;

      ++m_framePos;
      updateData();
    });
  }
}

/**
 * @brief Processes a batch of frames synchronously for efficient scrollback.
 *
 * This function processes frames from startFrame to endFrame (inclusive)
 * without using timers, which is more efficient for scrollback operations
 * where we need to rebuild plot history.
 *
 * @param startFrame The first frame index to process.
 * @param endFrame The last frame index to process (inclusive).
 */
void CSV::Player::processFrameBatch(int startFrame, int endFrame)
{
  if (!isOpen())
    return;

  for (int i = startFrame; i <= endFrame; ++i)
    IO::Manager::instance().processPayload(getFrame(i));
}

/**
 * @brief Registers CSV headers with Quick Plot
 *
 * Extracts column names from the CSV header row and explicitly registers
 * them with the FrameBuilder. Skips the first column (timestamp) as it
 * is not used for plotting. This should be called before the header row
 * is removed from m_csvData.
 */
void CSV::Player::sendHeaderFrame()
{
  if (m_csvData.isEmpty())
    return;

  const auto &headerRow = m_csvData.first();
  if (headerRow.size() <= 1)
    return;

  QStringList headers;
  for (int i = 1; i < headerRow.size(); ++i)
    headers.append(headerRow[i]);

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(headers);
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
        tr("The CSV file does not contain any data or headers."),
        QMessageBox::Critical);
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
        Misc::Utilities::showMessageBox(tr("Invalid Selection"),
                                        tr("The selected column is not valid."),
                                        QMessageBox::Critical);
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
  // If the cell is a pure number (fractional seconds), don't try to parse as
  // date
  bool isNumber = false;
  cell.toDouble(&isNumber);
  if (isNumber)
    return QDateTime();

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
 * @brief Parses a timestamp in fractional seconds format from the given @a row.
 *
 * @param row The row index to parse the timestamp from.
 * @return The timestamp in seconds, or -1.0 if invalid.
 */
double CSV::Player::getTimestampSeconds(int row)
{
  // Use cached value if available and valid
  if (m_useHighPrecisionTimestamps && row >= 0 && row < m_timestampCache.size())
    return m_timestampCache[row];

  // Fallback to parsing from cell
  bool error = false;
  QString cell = getCellValue(row, 0, error);
  if (error)
    return -1.0;

  return getTimestampSeconds(cell);
}

/**
 * @brief Parses a timestamp in fractional seconds format from the given @a
 * cell.
 *
 * This method attempts to parse the cell as a decimal number representing
 * seconds since Unix epoch with nanosecond precision.
 *
 * @param cell The cell string to parse.
 * @return The timestamp in seconds, or -1.0 if invalid.
 */
double CSV::Player::getTimestampSeconds(const QString &cell)
{
  bool ok = false;
  double timestamp = cell.toDouble(&ok);

  if (ok && timestamp >= 0.0)
    return timestamp;

  return -1.0;
}

/**
 * @brief Formats a timestamp in fractional seconds to HH:MM:SS.mmm format.
 *
 * @param seconds The timestamp in seconds.
 * @return Formatted timestamp string.
 */
QString CSV::Player::formatTimestamp(double seconds) const
{
  int hours = static_cast<int>(seconds / 3600.0);
  int minutes = static_cast<int>((seconds - hours * 3600.0) / 60.0);
  double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
      .arg(hours, 2, 10, QChar('0'))
      .arg(minutes, 2, 10, QChar('0'))
      .arg(secs, 6, 'f', 3, QChar('0'));
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
const QString CSV::Player::getCellValue(const int row, const int column,
                                        bool &error)
{
  static auto defaultValue = QLatin1String("");

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
