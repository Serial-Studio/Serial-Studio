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

#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>
#include <QtMath>

#include "AppState.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * Constructor function
 */
CSV::Player::Player()
  : m_framePos(0)
  , m_playing(false)
  , m_multiSource(false)
  , m_timestamp("")
  , m_startTimestampSeconds(0.0)
  , m_useHighPrecisionTimestamps(false)
{
  // Install event filter and connect playback update signal
  qApp->installEventFilter(this);
  connect(this, &CSV::Player::playerStateChanged, this, &CSV::Player::updateData);
}

/**
 * Returns the only instance of the class
 */
CSV::Player& CSV::Player::instance()
{
  static Player singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

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
  // Avoid division by zero when no frames are loaded
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

//--------------------------------------------------------------------------------------------------
// Frame information
//--------------------------------------------------------------------------------------------------

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
  // Return only the base filename for display
  if (isOpen()) {
    auto fileInfo = QFileInfo(m_csvFile.fileName());
    return fileInfo.fileName();
  }

  return "";
}

/**
 * Returns the timestamp of the current data frame / row.
 */
const QString& CSV::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * Enables CSV playback at 'live' speed (as it happened when CSV file was
 * saved to the computer).
 */
void CSV::Player::play()
{
  Q_ASSERT(isOpen());
  Q_ASSERT(!m_csvData.isEmpty());

  if (frameCount() <= 0)
    return;

  // Wrap around to start if at end of file
  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  m_startTimestamp = getDateTime(m_framePos);
  m_elapsedTimer.start();

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
  Q_ASSERT(isOpen());
  Q_ASSERT(!m_csvData.isEmpty());

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

//--------------------------------------------------------------------------------------------------
// File operations
//--------------------------------------------------------------------------------------------------

/**
 * Lets the user select a CSV file
 */
void CSV::Player::openFile()
{
  // Display file selection dialog for CSV files
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Select CSV file"),
                                 Misc::WorkspaceManager::instance().path("CSV"),
                                 tr("CSV files (*.csv)"));

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
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
  // Nothing to do if no file is open
  if (!isOpen())
    return;

  m_playing  = false;
  m_framePos = 0;
  m_csvFile.close();
  m_csvData.clear();
  m_csvData.squeeze();
  m_timestamp = "--.--";
  m_timestampCache.clear();
  m_useHighPrecisionTimestamps = false;
  m_startTimestampSeconds      = 0.0;
  m_multiSource                = false;
  m_columnToSource.clear();
  m_sourceColumnCount.clear();

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
  // Only advance if not at the last frame
  if (framePosition() < frameCount() - 1) {
    ++m_framePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

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
  // Only step back if not at the first frame
  if (framePosition() > 0) {
    --m_framePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(1, m_framePos - framesToLoad);
    processFrameBatch(startFrame, m_framePos);

    updateData();
  }
}

/**
 * @brief Reads all rows from a CSV text stream into m_csvData.
 *
 * Splits each line on commas, strips whitespace and quotes, and discards
 * rows that contain only empty cells. Stops after kMaxCsvRows to prevent
 * unbounded memory growth.
 *
 * @param stream The QTextStream positioned at the start of the CSV content.
 */
void CSV::Player::parseCsvRows(QTextStream& stream)
{
  Q_ASSERT(stream.device() != nullptr);
  Q_ASSERT(stream.device()->isOpen());

  constexpr int kMaxCsvRows = 10'000'000;

  while (!stream.atEnd() && m_csvData.size() < kMaxCsvRows) {
    QStringList row = stream.readLine().split(',');

    for (auto& item : row) {
      item = item.simplified();
      item.remove(QStringLiteral("\""));
    }

    bool isRowValid =
      !row.isEmpty()
      && std::any_of(row.cbegin(), row.cend(), [](const QString& item) { return !item.isEmpty(); });

    if (isRowValid)
      m_csvData.append(row);
  }

  if (m_csvData.size() >= kMaxCsvRows) [[unlikely]]
    qWarning() << "[CSV::Player] Row limit reached:" << kMaxCsvRows << "— file may be truncated";
}

/**
 * @brief Detects and initializes the timestamp strategy for the loaded CSV.
 *
 * Checks whether the first data cell is a high-precision numeric timestamp
 * or a parseable date/time string. If neither, prompts the user to select a
 * date/time column or provide a manual interval.
 *
 * On failure (user cancels the prompt), calls closeFile() and returns false.
 *
 * @return @c true if timestamps were successfully initialized, @c false if the
 *         user cancelled and the file was closed.
 */
void CSV::Player::initializeTimestamps()
{
  Q_ASSERT(!m_csvData.isEmpty());
  Q_ASSERT(m_csvFile.isOpen());

  // Check for high-precision numeric timestamps
  bool error            = false;
  QString firstCell     = getCellValue(1, 0, error);
  double firstTimestamp = error ? -1.0 : getTimestampSeconds(firstCell);

  if (firstTimestamp >= 0.0) {
    m_timestampCache.clear();
    m_timestampCache.reserve(m_csvData.count());

    for (int i = 0; i < m_csvData.count(); ++i) {
      bool err     = false;
      QString cell = getCellValue(i, 0, err);
      double ts    = err ? 0.0 : getTimestampSeconds(cell);
      m_timestampCache.append(ts);
    }

    m_useHighPrecisionTimestamps = true;
    m_startTimestampSeconds      = (m_timestampCache.size() > 1) ? m_timestampCache[1] : 0.0;
    return;
  }

  // Check for standard date/time format
  if (getDateTime(1).isValid()) {
    m_useHighPrecisionTimestamps = false;
    m_timestampCache.clear();
    return;
  }

  // No recognized timestamp — prompt user for a strategy
  m_useHighPrecisionTimestamps = false;
  m_timestampCache.clear();
}

/**
 * @brief Opens a CSV file, processes its data, and prepares it for playback.
 *
 * Checks for active device connections, reads and parses the CSV content,
 * initializes timestamp handling, and emits signals to update the UI.
 *
 * @param filePath The file path of the CSV file to be opened.
 */
void CSV::Player::openFile(const QString& filePath)
{
  Q_ASSERT(!filePath.isEmpty());
  Q_ASSERT(!m_playing);

  // Validate file path and close any open file
  if (filePath.isEmpty())
    return;

  closeFile();

  // Prompt user to disconnect if a device is active
  if (IO::ConnectionManager::instance().isConnected()) {
    auto response =
      Misc::Utilities::showMessageBox(tr("Device Connection Active"),
                                      tr("To use this feature, you must disconnect from the "
                                         "device. Do you want to proceed?"),
                                      QMessageBox::Warning,
                                      qAppName(),
                                      QMessageBox::No | QMessageBox::Yes);
    if (response == QMessageBox::Yes)
      IO::ConnectionManager::instance().disconnectDevice();
    else
      return;
  }

  // Open and read the CSV file
  m_csvFile.setFileName(filePath);
  if (!m_csvFile.open(QIODevice::ReadOnly)) {
    Misc::Utilities::showMessageBox(tr("Cannot read CSV file"),
                                    tr("Please check file permissions & location"),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  QTextStream in(&m_csvFile);
  parseCsvRows(in);

  // Detect timestamp format and build the cache
  initializeTimestamps();
  if (!m_useHighPrecisionTimestamps && m_timestampCache.isEmpty() && !getDateTime(1).isValid()) {
    if (!promptUserForDateTimeOrInterval()) {
      closeFile();
      return;
    }
  }

  // Prepare playback state
  sendHeaderFrame();
  m_framePos = 0;
  m_csvData.removeFirst();

  if (m_useHighPrecisionTimestamps && !m_timestampCache.isEmpty()) {
    m_timestampCache.removeFirst();
    if (!m_timestampCache.isEmpty())
      m_startTimestampSeconds = m_timestampCache[0];
  }

  // Verify that at least one data row remains
  if (m_csvData.count() >= 1) {
    updateData();
    Q_EMIT openChanged();
  } else {
    Misc::Utilities::showMessageBox(tr("Insufficient Data in CSV File"),
                                    tr("The CSV file must contain at least one data row to "
                                       "proceed. Please check the file and try again."),
                                    QMessageBox::Critical);
    closeFile();
  }
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

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
  Q_ASSERT(progress >= 0.0 && progress <= 1.0);
  Q_ASSERT(isOpen());

  // Clamp and pause before seeking
  const auto validProgress = std::clamp(progress, 0.0, 1.0);

  if (isPlaying())
    pause();

  if (frameCount() <= 0)
    return;

  int newFramePos = qMin(frameCount() - 1, qCeil(frameCount() * validProgress));

  if (newFramePos != m_framePos) {
    m_framePos = newFramePos;

    UI::Dashboard::instance().clearPlotData();

    int framesToLoad = UI::Dashboard::instance().points();
    int startFrame   = std::max(1, m_framePos - framesToLoad);
    int endFrame     = std::min(frameCount() - 1, m_framePos);

    processFrameBatch(startFrame, endFrame);

    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the timestamp display for the current frame position.
 */
void CSV::Player::updateTimestampDisplay()
{
  // Read timestamp cell for the current frame position
  bool err = true;
  auto ts  = getCellValue(m_framePos, 0, err);
  if (err)
    return;

  double seconds = getTimestampSeconds(ts);
  if (seconds >= 0.0)
    m_timestamp = formatTimestamp(seconds);

  else {
    auto frameTime = getDateTime(ts);
    if (frameTime.isValid() && m_startTimestamp.isValid()) {
      qint64 elapsedMs = m_startTimestamp.msecsTo(frameTime);
      m_timestamp      = formatTimestamp(elapsedMs / 1000.0);
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
  Q_ASSERT(!m_csvData.isEmpty() || !isOpen());
  Q_ASSERT(m_framePos >= 0);

  // Nothing to do if no file is loaded
  if (!isOpen())
    return;

  updateTimestampDisplay();
  if (!isPlaying())
    return;

  injectFrame(getFrame(framePosition()));

  if (framePosition() >= frameCount() - 1) {
    pause();
    return;
  }

  const qint64 elapsedMs = m_elapsedTimer.elapsed();
  qint64 msUntilNext     = 0;

  if (m_useHighPrecisionTimestamps) {
    if (framePosition() + 1 >= m_timestampCache.size()) {
      pause();
      return;
    }

    const double targetTime = m_startTimestampSeconds + (elapsedMs / 1000.0);
    const double nextTime   = m_timestampCache[framePosition() + 1];
    msUntilNext             = qMax(0LL, static_cast<qint64>((nextTime - targetTime) * 1000.0));
  }

  else {
    const QDateTime targetTime = m_startTimestamp.addMSecs(elapsedMs);
    const auto nextTime        = getDateTime(framePosition() + 1);
    if (!nextTime.isValid()) {
      pause();
      return;
    }

    msUntilNext = targetTime.msecsTo(nextTime);
  }

  if (msUntilNext <= 0) {
    constexpr int kMaxBatchSize = 100;
    int processed               = 0;
    while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize && msUntilNext <= 0) {
      ++m_framePos;
      injectFrame(getFrame(m_framePos));
      ++processed;

      if (m_useHighPrecisionTimestamps) {
        if (m_framePos + 1 < m_timestampCache.size()) {
          const double target = m_startTimestampSeconds + (m_elapsedTimer.elapsed() / 1000.0);
          const double next   = m_timestampCache[m_framePos + 1];
          msUntilNext         = qMax(0LL, static_cast<qint64>((next - target) * 1000.0));
        }

        else {
          pause();
          return;
        }
      }

      else {
        auto target = m_startTimestamp.addMSecs(m_elapsedTimer.elapsed());
        auto next   = getDateTime(m_framePos + 1);
        if (next.isValid())
          msUntilNext = target.msecsTo(next);
        else {
          pause();
          return;
        }
      }
    }

    updateTimestampDisplay();

    if (m_framePos < frameCount() - 1)
      QTimer::singleShot(qMax(0LL, msUntilNext), Qt::PreciseTimer, this, [this] {
        if (isOpen() && isPlaying())
          updateData();
      });
    else
      pause();
  }

  else {
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
  Q_ASSERT(startFrame <= endFrame);
  Q_ASSERT(startFrame >= 0);

  if (!isOpen())
    return;

  for (int i = startFrame; i <= endFrame; ++i)
    injectFrame(getFrame(i));
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
  // Need at least one row for headers
  if (m_csvData.isEmpty())
    return;

  const auto& headerRow = m_csvData.first();
  if (headerRow.size() <= 1)
    return;

  // In project mode with multiple sources, build multi-source mapping
  // and skip QuickPlot header registration (project defines the structure)
  if (AppState::instance().operationMode() == SerialStudio::ProjectFile) {
    const auto& sources = DataModel::ProjectModel::instance().sources();
    if (sources.size() > 1) {
      buildMultiSourceMapping();
      return;
    }
  }

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
  // Validate that CSV contains headers before prompting
  if (m_csvData.isEmpty() || m_csvData.first().isEmpty()) {
    Misc::Utilities::showMessageBox(tr("Invalid CSV"),
                                    tr("The CSV file does not contain any data or headers."),
                                    QMessageBox::Critical);
    return false;
  }

  const auto headerLabels = m_csvData.first().toList();

  bool ok;
  QStringList options;
  options << tr("Select a date/time column") << tr("Set interval manually");
  QString choice = QInputDialog::getItem(nullptr,
                                         tr("CSV Date/Time Selection"),
                                         tr("Choose how to handle the date/time data:"),
                                         options,
                                         0,
                                         false,
                                         &ok);

  if (!ok)
    return false;

  if (choice == tr("Set interval manually")) {
    const auto interval =
      QInputDialog::getInt(nullptr,
                           tr("Set Interval"),
                           tr("Please enter the interval between rows in milliseconds:"),
                           1000,
                           1,
                           1000000,
                           1,
                           &ok);

    if (ok) {
      generateDateTimeForRows(interval);
      return true;
    }
  }

  else {
    const auto column =
      QInputDialog::getItem(nullptr,
                            tr("Select Date/Time Column"),
                            tr("Please select the column that contains the date/time data:"),
                            headerLabels,
                            0,
                            false,
                            &ok);

    if (ok) {
      int columnIndex = headerLabels.indexOf(column);
      if (columnIndex == -1) {
        Misc::Utilities::showMessageBox(
          tr("Invalid Selection"), tr("The selected column is not valid."), QMessageBox::Critical);
        return false;
      }

      convertColumnToDateTime(columnIndex);
      return true;
    }
  }

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
  // Synthesize evenly-spaced timestamps for rows lacking date/time
  const auto startTime = QDateTime::currentDateTime();
  const auto format    = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");

  for (int i = 0; i < m_csvData.size(); ++i) {
    QString dateTimeString = startTime.addMSecs(static_cast<qint64>(i) * interval).toString(format);
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
  // Validate column index against header row
  if (m_csvData.isEmpty() || columnIndex < 0
      || columnIndex >= m_csvData.first().size())
    return;

  // Rewrite each row so the chosen column becomes the leading timestamp
  const auto format = QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz");
  for (int i = 1; i < m_csvData.size(); ++i) {
    // Read the timestamp from the user-selected column, not column 0
    bool error    = false;
    auto cellText = getCellValue(i, columnIndex, error);
    auto dateTime = error ? QDateTime() : getDateTime(cellText);
    if (!dateTime.isValid())
      dateTime = QDateTime::currentDateTime();

    m_csvData[i].remove(columnIndex);
    m_csvData[i].prepend(dateTime.toString(format));
  }
}

//--------------------------------------------------------------------------------------------------
// Date/time operations
//--------------------------------------------------------------------------------------------------

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
  // Retrieve the timestamp cell for this row
  bool error = false;
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
QDateTime CSV::Player::getDateTime(const QString& cell)
{
  // Skip numeric strings (handled by getTimestampSeconds)
  bool isNumber = false;
  cell.toDouble(&isNumber);
  if (isNumber)
    return QDateTime();

  QDateTime dateTime;

  static const QStringList formats = {QStringLiteral("yyyy/MM/dd HH:mm:ss::zzz"),
                                      QStringLiteral("yyyy/MM/dd/ HH:mm:ss::zzz"),
                                      QStringLiteral("yyyy/MM/dd HH:mm:ss"),
                                      QStringLiteral("yyyy/MM/dd/ HH:mm:ss")};

  for (const auto& format : formats) {
    dateTime = QDateTime::fromString(cell, format);
    if (dateTime.isValid())
      break;
  }

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
  // Use cached timestamp if available, otherwise parse from cell
  if (m_useHighPrecisionTimestamps && row >= 0 && row < m_timestampCache.size())
    return m_timestampCache[row];

  bool error   = false;
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
double CSV::Player::getTimestampSeconds(const QString& cell)
{
  // Accept only non-negative numeric values as valid timestamps
  bool ok          = false;
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
  // Split total seconds into hours, minutes, and fractional seconds
  int hours   = static_cast<int>(seconds / 3600.0);
  int minutes = static_cast<int>((seconds - hours * 3600.0) / 60.0);
  double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
    .arg(hours, 2, 10, QChar('0'))
    .arg(minutes, 2, 10, QChar('0'))
    .arg(secs, 6, 'f', 3, QChar('0'));
}

//--------------------------------------------------------------------------------------------------
// Frame building
//--------------------------------------------------------------------------------------------------

/**
 * Generates a frame from the data at the given @a row. The first item of each
 * row is ignored because it contains the RX date/time, which is used to
 * regulate the interval at which the frames are parsed.
 */
QByteArray CSV::Player::getFrame(const int row)
{
  Q_ASSERT(row >= 0);
  Q_ASSERT(row < m_csvData.count());

  // Timestamp column (index 0) is excluded from the data frame
  QByteArray frame;

  if (m_csvData.count() > row) {
    const auto& list = m_csvData[row];
    for (int i = 1; i < list.count(); ++i) {
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
const QString CSV::Player::getCellValue(const int row, const int column, bool& error)
{
  Q_ASSERT(row >= 0);
  Q_ASSERT(column >= 0);

  // Return empty string on out-of-bounds access
  static auto defaultValue = QLatin1String("");

  if (m_csvData.count() > row) {
    const auto& list = m_csvData[row];
    if (list.count() > column) {
      error = false;
      return list[column];
    }
  }

  error = true;
  return defaultValue;
}

//--------------------------------------------------------------------------------------------------
// Multi-source playback helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a column-to-source mapping for multi-source CSV playback.
 *
 * Iterates all groups/datasets from the project sorted by uniqueId (matching
 * the export column order) and maps each CSV column index to its sourceId.
 */
void CSV::Player::buildMultiSourceMapping()
{
  // Clear previous mapping before rebuilding
  m_columnToSource.clear();
  m_sourceColumnCount.clear();

  const auto& groups = DataModel::ProjectModel::instance().groups();

  // Collect (uniqueId, sourceId) pairs for all datasets
  QVector<QPair<int, int>> uidSourcePairs;
  for (const auto& g : groups)
    for (const auto& d : g.datasets)
      uidSourcePairs.append(qMakePair(d.uniqueId, d.sourceId));

  // Sort by uniqueId to match export column order
  std::sort(uidSourcePairs.begin(), uidSourcePairs.end(), [](const auto& a, const auto& b) {
    return a.first < b.first;
  });

  // Map column index (0-based, excluding timestamp) to sourceId
  for (int col = 0; col < uidSourcePairs.size(); ++col) {
    const int srcId       = uidSourcePairs[col].second;
    m_columnToSource[col] = srcId;
    m_sourceColumnCount[srcId]++;
  }

  m_multiSource = !m_columnToSource.isEmpty() && m_sourceColumnCount.size() > 1;
}

/**
 * @brief Injects a CSV frame through the appropriate pipeline path.
 *
 * For single-source or non-project mode, delegates to processPayload().
 * For multi-source project mode, splits the CSV row by source and calls
 * processMultiSourcePayload().
 *
 * @param frame CSV-formatted byte array (without timestamp column).
 */
void CSV::Player::injectFrame(const QByteArray& frame)
{
  // Skip empty frames
  if (frame.isEmpty())
    return;

  // Single-source: use standard path
  if (!m_multiSource) {
    IO::ConnectionManager::instance().processPayload(frame);
    return;
  }

  // Multi-source: split CSV columns by source
  const auto fields = QString::fromUtf8(frame).trimmed().split(',');

  QMap<int, QStringList> sourceFields;
  for (int col = 0; col < fields.size(); ++col) {
    auto it = m_columnToSource.find(col);
    if (it == m_columnToSource.end())
      continue;

    sourceFields[it.value()].append(fields[col]);
  }

  // Build per-source payloads
  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it)
    sourcePayloads[it.key()] = it.value().join(',').toUtf8() + '\n';

  IO::ConnectionManager::instance().processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

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
bool CSV::Player::eventFilter(QObject* obj, QEvent* event)
{
  // Only handle key events during active playback sessions
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
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
bool CSV::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  // Map media and arrow keys to playback actions
  bool handled;
  switch (keyEvent->key()) {
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
