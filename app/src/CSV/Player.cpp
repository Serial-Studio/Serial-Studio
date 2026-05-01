/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
#include <QSet>
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
 * @brief Constructs the CSV player and installs the global key-event filter.
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
 * @brief Returns the singleton CSV Player instance.
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
 * @brief Returns whether a CSV file is currently open.
 */
bool CSV::Player::isOpen() const
{
  return m_csvFile.isOpen();
}

/**
 * @brief Returns the CSV playback progress in the range 0.0 to 1.0.
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
 * @brief Returns whether playback is currently active.
 */
bool CSV::Player::isPlaying() const
{
  return m_playing;
}

//--------------------------------------------------------------------------------------------------
// Frame information
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the total number of data frames in the CSV file.
 */
int CSV::Player::frameCount() const
{
  return m_csvData.count();
}

/**
 * @brief Returns the current CSV row being replayed.
 */
int CSV::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns the base filename of the currently open CSV file.
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
 * @brief Returns the formatted timestamp of the current frame.
 */
const QString& CSV::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts CSV playback at the original capture speed.
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
 * @brief Pauses CSV playback.
 */
void CSV::Player::pause()
{
  Q_ASSERT(isOpen());
  Q_ASSERT(!m_csvData.isEmpty());

  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause.
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
 * @brief Prompts the user to select a CSV file to play back.
 */
void CSV::Player::openFile()
{
  // Display file selection dialog for CSV files
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Select CSV file"),
                                 Misc::WorkspaceManager::instance().path("CSV"),
                                 tr("CSV files (*.csv)"));

  dialog->setFileMode(QFileDialog::ExistingFile);

  connect(dialog, &QFileDialog::fileSelected, this, [this, dialog](const QString& path) {
    if (!path.isEmpty())
      openFile(path);

    dialog->deleteLater();
  });

  connect(dialog, &QFileDialog::rejected, dialog, &QFileDialog::deleteLater);

  dialog->open();
}

/**
 * @brief Closes the current CSV file and resets playback state.
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
 * @brief Advances to the next CSV row, capped at the last row.
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
 * @brief Steps back to the previous CSV row, capped at the first row.
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
 * @brief Reads CSV rows from the stream into m_csvData up to kMaxCsvRows.
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
    qWarning() << "[CSV::Player] Row limit reached:" << kMaxCsvRows << "-- file may be truncated";
}

/**
 * @brief Detects the CSV timestamp format and initializes the cache.
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

  // No recognized timestamp -- prompt user for a strategy
  m_useHighPrecisionTimestamps = false;
  m_timestampCache.clear();
}

/**
 * @brief Opens the CSV at filePath and prepares it for playback.
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
    Misc::Utilities::showMessageBox(
      tr("Cannot read CSV file"), tr("Check file permissions and location"), QMessageBox::Critical);
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
                                       "proceed. Check the file and try again."),
                                    QMessageBox::Critical);
    closeFile();
  }
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks playback to a normalized position between 0.0 and 1.0.
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
 * @brief Synchronously injects frames in [startFrame, endFrame] for scrollback.
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
 * @brief Registers CSV column names with Quick Plot (excluding the timestamp).
 */
void CSV::Player::sendHeaderFrame()
{
  // Need at least one row for headers
  if (m_csvData.isEmpty())
    return;

  const auto& headerRow = m_csvData.first();
  if (headerRow.size() <= 1)
    return;

  // Multi-source project: build mapping and skip QuickPlot header registration
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
 * @brief Prompts the user to pick a date/time column or a manual row interval.
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
 * @brief Prepends synthetic evenly-spaced date/time strings to each CSV row.
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
 * @brief Moves the selected column to the start of each row as a date/time.
 */
void CSV::Player::convertColumnToDateTime(int columnIndex)
{
  // Validate column index against header row
  if (m_csvData.isEmpty() || columnIndex < 0 || columnIndex >= m_csvData.first().size())
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
 * @brief Returns the parsed date/time from column 0 of the given row.
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
 * @brief Parses the given cell as a date/time using common CSV formats.
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
 * @brief Returns the fractional-second timestamp for the given row.
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
 * @brief Parses the cell as fractional seconds since the Unix epoch.
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
 * @brief Formats fractional seconds as HH:MM:SS.mmm.
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
 * @brief Builds a CSV frame byte array from the row, skipping the timestamp.
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
 * @brief Safely reads the cell at (row, column), setting error on failure.
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
 */
void CSV::Player::buildMultiSourceMapping()
{
  // Clear previous mapping before rebuilding
  m_columnToSource.clear();
  m_sourceColumnCount.clear();
  m_sourceColumnsByIndex.clear();

  const auto& groups = DataModel::ProjectModel::instance().groups();

  // Per-column metadata (uid, sourceId, parserIndex)
  struct ColMeta {
    int uid;
    int sourceId;
    int parserIndex;
  };

  QVector<ColMeta> cols;
  for (const auto& g : groups)
    for (const auto& d : g.datasets)
      cols.append({DataModel::dataset_unique_id(g, d), g.sourceId, d.index});

  // Sort by uniqueId to match the file's column layout produced by the export
  std::sort(
    cols.begin(), cols.end(), [](const ColMeta& a, const ColMeta& b) { return a.uid < b.uid; });

  // Walk the sorted layout
  for (int col = 0; col < cols.size(); ++col) {
    const auto& m         = cols[col];
    m_columnToSource[col] = m.sourceId;
    m_sourceColumnCount[m.sourceId]++;
  }

  // For each source, build the channel array the parser expects (parserIndex slots, sparse-aware)
  QMap<int, QMap<int, int>> indexToFileCol;
  QMap<int, int> sourceMaxIndex;
  for (int col = 0; col < cols.size(); ++col) {
    const auto& m = cols[col];
    if (m.parserIndex < 1)
      continue;

    auto& slotMap = indexToFileCol[m.sourceId];
    if (!slotMap.contains(m.parserIndex))
      slotMap.insert(m.parserIndex, col);

    sourceMaxIndex[m.sourceId] = std::max(sourceMaxIndex.value(m.sourceId, 0), m.parserIndex);
  }

  for (auto it = indexToFileCol.constBegin(); it != indexToFileCol.constEnd(); ++it) {
    const int srcId    = it.key();
    const int maxIndex = sourceMaxIndex.value(srcId, 0);
    QVector<int> ordered(maxIndex, -1);
    for (auto sit = it.value().constBegin(); sit != it.value().constEnd(); ++sit)
      ordered[sit.key() - 1] = sit.value();

    m_sourceColumnsByIndex[srcId] = std::move(ordered);
  }

  m_multiSource = !m_columnToSource.isEmpty() && m_sourceColumnCount.size() > 1;
}

/**
 * @brief Injects a CSV frame, splitting per source when in multi-source mode.
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

  // Multi-source: split CSV columns by source and reorder into parser-index order
  const auto fields = QString::fromUtf8(frame).trimmed().split(',');

  QMap<int, QStringList> sourceFields;
  QSet<int> sourcesPresent;
  for (auto it = m_sourceColumnsByIndex.constBegin(); it != m_sourceColumnsByIndex.constEnd();
       ++it) {
    const int srcId         = it.key();
    const auto& orderedCols = it.value();
    QStringList orderedCells;
    orderedCells.reserve(orderedCols.size());
    for (int col : orderedCols) {
      const QString cell = (col >= 0 && col < fields.size()) ? fields[col] : QString();
      orderedCells.append(cell);
      if (!cell.isEmpty())
        sourcesPresent.insert(srcId);
    }
    sourceFields.insert(srcId, std::move(orderedCells));
  }

  // Build per-source payloads, but only for sources that contributed data
  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it) {
    if (!sourcesPresent.contains(it.key()))
      continue;

    sourcePayloads[it.key()] = it.value().join(',').toUtf8() + '\n';
  }

  if (sourcePayloads.isEmpty())
    return;

  IO::ConnectionManager::instance().processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Captures key events and routes playback shortcuts to handleKeyPress.
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
 * @brief Maps media and arrow keys to playback actions.
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
