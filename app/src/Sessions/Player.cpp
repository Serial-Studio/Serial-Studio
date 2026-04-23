/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Sessions/Player.h"

#  include <QApplication>
#  include <QDateTime>
#  include <QFileDialog>
#  include <QFileInfo>
#  include <QJsonDocument>
#  include <QJsonParseError>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QTimer>
#  include <QtMath>

#  include "AppState.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/ProjectModel.h"
#  include "IO/ConnectionManager.h"
#  include "Misc/Utilities.h"
#  include "Misc/WorkspaceManager.h"
#  include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs an application-wide event filter and wires state changes.
 */
Sessions::Player::Player()
  : m_sessionId(-1)
  , m_framePos(0)
  , m_playing(false)
  , m_multiSource(false)
  , m_timestamp("--.--")
  , m_startTimestampSeconds(0.0)
  , m_preSessionCaptured(false)
  , m_preSessionOperationMode(SerialStudio::QuickPlot)
{
  qApp->installEventFilter(this);
  connect(this, &Sessions::Player::playerStateChanged, this, &Sessions::Player::updateData);
}

/**
 * @brief Closes the database before the singleton destructs.
 */
Sessions::Player::~Player()
{
  closeFile();
}

/**
 * @brief Returns the only instance of the class.
 */
Sessions::Player& Sessions::Player::instance()
{
  static Player singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Playback status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true while a session is loaded and ready to play.
 */
bool Sessions::Player::isOpen() const
{
  return m_db.isOpen() && m_sessionId >= 0 && !m_timestampsNs.empty();
}

/**
 * @brief Returns @c true while frames are advancing on the live pipeline.
 */
bool Sessions::Player::isPlaying() const
{
  return m_playing;
}

/**
 * @brief Returns the number of frames in the loaded session.
 */
int Sessions::Player::frameCount() const
{
  return static_cast<int>(m_timestampsNs.size());
}

/**
 * @brief Returns the zero-based index of the current playback frame.
 */
int Sessions::Player::framePosition() const
{
  return m_framePos;
}

/**
 * @brief Returns a normalized playback position in the range [0, 1].
 */
double Sessions::Player::progress() const
{
  const auto count = frameCount();
  if (count <= 0)
    return 0.0;

  return static_cast<double>(framePosition()) / count;
}

/**
 * @brief Returns the short filename of the loaded database.
 */
QString Sessions::Player::filename() const
{
  if (!m_filePath.isEmpty())
    return QFileInfo(m_filePath).fileName();

  return QString();
}

/**
 * @brief Returns the formatted timestamp of the current frame.
 */
const QString& Sessions::Player::timestamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// Playback control
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts real-time playback from the current frame.
 */
void Sessions::Player::play()
{
  if (!isOpen())
    return;

  // Wrap to the first frame if parked at the end
  if (m_framePos >= frameCount() - 1)
    m_framePos = 0;

  m_elapsedTimer.start();
  m_startTimestampSeconds = m_timestampsNs[static_cast<size_t>(m_framePos)] / 1e9;

  m_playing = true;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Pauses playback without changing the current frame.
 */
void Sessions::Player::pause()
{
  if (!isOpen())
    return;

  m_playing = false;
  Q_EMIT playerStateChanged();
}

/**
 * @brief Toggles between play and pause.
 */
void Sessions::Player::toggle()
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
 * @brief Prompts the user to pick a .db file from the SQLite workspace folder.
 */
void Sessions::Player::openFile()
{
  auto* dialog = new QFileDialog(nullptr,
                                 tr("Open Session File"),
                                 Misc::WorkspaceManager::instance().path("Session Databases"),
                                 tr("Session files (*.db)"));

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
 * @brief Closes the active database, frees cached frames, and resets state.
 */
void Sessions::Player::closeFile()
{
  // Nothing to do if no database was opened
  if (!m_db.isValid() && m_connectionName.isEmpty())
    return;

  // Stop playback before tearing resources down
  m_playing  = false;
  m_framePos = 0;

  // Close database connection
  if (m_db.isOpen())
    m_db.close();

  const auto connName = m_connectionName;
  m_db                = QSqlDatabase();
  if (!connName.isEmpty())
    QSqlDatabase::removeDatabase(connName);

  // Reset per-session caches
  m_filePath.clear();
  m_connectionName.clear();
  m_sessionId             = -1;
  m_timestamp             = "--.--";
  m_startTimestampSeconds = 0.0;
  m_multiSource           = false;
  m_columnUniqueIds.clear();
  m_timestampsNs.clear();
  m_columnToSource.clear();
  m_sourceColumnCount.clear();

  DataModel::FrameBuilder::instance().registerQuickPlotHeaders(QStringList());

  // Restore project + operation mode that existed before the session
  restorePreSessionState();

  Q_EMIT openChanged();
  Q_EMIT timestampChanged();
  Q_EMIT playerStateChanged();
}

/**
 * @brief Opens @p filePath, loads the latest session, and emits @c openChanged.
 */
void Sessions::Player::openFile(const QString& filePath)
{
  Q_ASSERT(!filePath.isEmpty());

  if (filePath.isEmpty())
    return;

  closeFile();

  // Snapshot mode + project path before restoreProjectFromSession mutates them
  capturePreSessionState();

  // Ask the user to disconnect live devices
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

  // Open the database with a unique connection name
  m_filePath       = filePath;
  m_connectionName = QStringLiteral("ss_sqlite_player_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(filePath);

  if (!m_db.open()) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    tr("Please check file permissions and try again."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  // WAL + busy_timeout to coexist with a live Export writer
  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  // Pick the newest session and restore its embedded project
  const int newestId = latestSessionId();
  if (newestId >= 0)
    (void)restoreProjectFromSession(newestId);

  openSessionInternal(newestId);
}

/**
 * @brief Opens a specific session from @p filePath — called by DatabaseManager.
 */
void Sessions::Player::openFile(const QString& filePath, int sessionId)
{
  Q_ASSERT(!filePath.isEmpty());
  Q_ASSERT(sessionId >= 0);

  if (filePath.isEmpty())
    return;

  closeFile();

  // Snapshot pre-session state before any mutation.
  capturePreSessionState();

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

  m_filePath       = filePath;
  m_connectionName = QStringLiteral("ss_sqlite_player_%1").arg(QDateTime::currentMSecsSinceEpoch());

  m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
  m_db.setDatabaseName(filePath);

  if (!m_db.open()) {
    Misc::Utilities::showMessageBox(tr("Cannot open session file"),
                                    tr("Please check file permissions and try again."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  // WAL + busy_timeout so we coexist with a live Export writer
  QSqlQuery pragma(m_db);
  pragma.exec("PRAGMA journal_mode=WAL");
  pragma.exec("PRAGMA busy_timeout=5000");

  // Restore the project stored inside this session so widgets render
  (void)restoreProjectFromSession(sessionId);

  openSessionInternal(sessionId);
}

/**
 * @brief Common session-loading logic shared by both openFile overloads.
 */
void Sessions::Player::openSessionInternal(int sessionId)
{
  if (sessionId < 0) {
    Misc::Utilities::showMessageBox(tr("No sessions found"),
                                    tr("This file does not contain any recording sessions."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  m_sessionId = sessionId;

  // Load column order, build the timestamp index, wire sources if needed
  loadColumnOrder();
  if (m_columnUniqueIds.empty()) {
    Misc::Utilities::showMessageBox(tr("Session has no columns"),
                                    tr("The selected session is missing its column definitions."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  buildTimestampIndex();
  if (m_timestampsNs.empty()) {
    Misc::Utilities::showMessageBox(tr("Session has no readings"),
                                    tr("The selected session does not contain any frames."),
                                    QMessageBox::Critical);
    closeFile();
    return;
  }

  // ProjectFile mode uses the project's parser; QuickPlot needs synthetic headers
  const auto mode = AppState::instance().operationMode();
  if (mode == SerialStudio::ProjectFile) {
    const auto& sources = DataModel::ProjectModel::instance().sources();
    if (sources.size() > 1)
      buildMultiSourceMapping();
  } else {
    // QuickPlot fallback — register synthetic headers
    QStringList headers;
    headers.reserve(static_cast<int>(m_columnUniqueIds.size()));
    for (int uid : m_columnUniqueIds)
      headers.append(QStringLiteral("uid_%1").arg(uid));

    DataModel::FrameBuilder::instance().registerQuickPlotHeaders(headers);
  }

  m_framePos              = 0;
  m_startTimestampSeconds = m_timestampsNs.front() / 1e9;

  // Push the first frame so the dashboard shows something before Play
  updateData();
  Q_EMIT openChanged();
  Q_EMIT playerStateChanged();
}

//--------------------------------------------------------------------------------------------------
// Progress & seeking
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeks to a normalized position and rebuilds plot history around it.
 */
void Sessions::Player::setProgress(const double progress)
{
  if (!isOpen())
    return;

  const auto clamped = std::clamp(progress, 0.0, 1.0);
  if (isPlaying())
    pause();

  const int newPos = qMin(frameCount() - 1, qCeil(frameCount() * clamped));
  if (newPos == m_framePos)
    return;

  m_framePos = newPos;

  UI::Dashboard::instance().clearPlotData();

  // Rehydrate the plot buffer so scrolling backwards shows history
  const int toLoad   = UI::Dashboard::instance().points();
  const int startIdx = std::max(0, m_framePos - toLoad);
  processFrameBatch(startIdx, m_framePos);

  updateData();
}

/**
 * @brief Advances one frame and rebuilds the trailing plot window.
 */
void Sessions::Player::nextFrame()
{
  if (m_framePos < frameCount() - 1) {
    ++m_framePos;
    UI::Dashboard::instance().clearPlotData();
    const int toLoad   = UI::Dashboard::instance().points();
    const int startIdx = std::max(0, m_framePos - toLoad);
    processFrameBatch(startIdx, m_framePos);
    updateData();
  }
}

/**
 * @brief Steps one frame back and rebuilds the trailing plot window.
 */
void Sessions::Player::previousFrame()
{
  if (m_framePos > 0) {
    --m_framePos;
    UI::Dashboard::instance().clearPlotData();
    const int toLoad   = UI::Dashboard::instance().points();
    const int startIdx = std::max(0, m_framePos - toLoad);
    processFrameBatch(startIdx, m_framePos);
    updateData();
  }
}

//--------------------------------------------------------------------------------------------------
// Data processing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pushes the current frame and, if playing, schedules the next one.
 */
void Sessions::Player::updateData()
{
  if (!isOpen())
    return;

  updateTimestampDisplay();

  if (!isPlaying())
    return;

  injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]));

  if (m_framePos >= frameCount() - 1) {
    pause();
    return;
  }

  // Schedule the next frame on the recording's wall-clock delta
  const qint64 elapsedMs = m_elapsedTimer.elapsed();
  const double nextSec   = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] / 1e9;
  const double targetSec = m_startTimestampSeconds + (elapsedMs / 1000.0);
  qint64 msUntilNext     = qMax(0LL, static_cast<qint64>((nextSec - targetSec) * 1000.0));

  // Burst-emit late frames in one chunk to prevent runaway queues
  if (msUntilNext <= 0) {
    constexpr int kMaxBatchSize = 100;
    int processed               = 0;

    while (m_framePos < frameCount() - 1 && processed < kMaxBatchSize && msUntilNext <= 0) {
      ++m_framePos;
      injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(m_framePos)]));
      ++processed;

      if (m_framePos + 1 < frameCount()) {
        const double next   = m_timestampsNs[static_cast<size_t>(m_framePos + 1)] / 1e9;
        const double target = m_startTimestampSeconds + (m_elapsedTimer.elapsed() / 1000.0);
        msUntilNext         = qMax(0LL, static_cast<qint64>((next - target) * 1000.0));
      }

      else {
        pause();
        return;
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
 * @brief Synchronously emits a contiguous frame range (used by seek/step).
 */
void Sessions::Player::processFrameBatch(int startFrame, int endFrame)
{
  if (!isOpen())
    return;

  Q_ASSERT(startFrame >= 0);
  Q_ASSERT(endFrame < frameCount());

  for (int i = startFrame; i <= endFrame; ++i)
    injectFrame(buildFrameAt(m_timestampsNs[static_cast<size_t>(i)]));
}

//--------------------------------------------------------------------------------------------------
// Session / schema loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the most recent session_id in the database, or -1 if none.
 */
int Sessions::Player::latestSessionId() const
{
  QSqlQuery q(m_db);
  q.prepare("SELECT session_id FROM sessions ORDER BY started_at DESC LIMIT 1");
  if (!q.exec() || !q.next())
    return -1;

  return q.value(0).toInt();
}

/**
 * @brief Loads the project JSON embedded in a session row.
 */
bool Sessions::Player::restoreProjectFromSession(int sessionId)
{
  Q_ASSERT(m_db.isOpen());
  Q_ASSERT(sessionId >= 0);

  // Read the per-session snapshot, falling back to the global metadata row
  QString projectJson;
  {
    QSqlQuery q(m_db);
    q.prepare("SELECT project_json FROM sessions WHERE session_id = ?");
    q.bindValue(0, sessionId);
    if (q.exec() && q.next())
      projectJson = q.value(0).toString();
  }

  if (projectJson.isEmpty()) {
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM project_metadata WHERE key = 'project_json'");
    if (q.exec() && q.next())
      projectJson = q.value(0).toString();
  }

  if (projectJson.isEmpty())
    return false;

  // Parse the blob and hand the document to ProjectModel
  QJsonParseError parseError{};
  const auto doc = QJsonDocument::fromJson(projectJson.toUtf8(), &parseError);
  if (parseError.error != QJsonParseError::NoError || doc.isEmpty()) {
    qWarning() << "[Sessions::Player] Embedded project JSON is malformed:"
               << parseError.errorString();
    return false;
  }

  // Flip to ProjectFile mode before loading
  AppState::instance().setOperationMode(SerialStudio::ProjectFile);
  if (!DataModel::ProjectModel::instance().loadFromJsonDocument(doc)) {
    qWarning() << "[Sessions::Player] ProjectModel rejected the embedded JSON";
    return false;
  }

  return true;
}

/**
 * @brief Snapshots the active operation mode and project file path.
 */
void Sessions::Player::capturePreSessionState()
{
  if (m_preSessionCaptured)
    return;

  m_preSessionOperationMode = AppState::instance().operationMode();
  m_preSessionProjectPath   = DataModel::ProjectModel::instance().jsonFilePath();
  m_preSessionCaptured      = true;
}

/**
 * @brief Restores the operation mode and project captured before the session.
 */
void Sessions::Player::restorePreSessionState()
{
  if (!m_preSessionCaptured)
    return;

  auto& pm = DataModel::ProjectModel::instance();
  if (!m_preSessionProjectPath.isEmpty() && QFileInfo::exists(m_preSessionProjectPath))
    (void)pm.openJsonFile(m_preSessionProjectPath);
  else
    pm.newJsonFile();

  AppState::instance().setOperationMode(m_preSessionOperationMode);

  m_preSessionCaptured = false;
  m_preSessionProjectPath.clear();
  m_preSessionOperationMode = SerialStudio::QuickPlot;
}

/**
 * @brief Reads the session's column order into @c m_columnUniqueIds.
 */
void Sessions::Player::loadColumnOrder()
{
  m_columnUniqueIds.clear();

  QSqlQuery q(m_db);
  q.prepare("SELECT unique_id FROM columns WHERE session_id = ? ORDER BY column_id ASC");
  q.bindValue(0, m_sessionId);

  if (!q.exec()) {
    qWarning() << "[Sessions::Player] column query failed:" << q.lastError().text();
    return;
  }

  while (q.next())
    m_columnUniqueIds.push_back(q.value(0).toInt());
}

/**
 * @brief Builds the column→sourceId map used by multi-source playback.
 */
void Sessions::Player::buildMultiSourceMapping()
{
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

  // Use the project mapping so mismatched schemas still play back
  for (int col = 0; col < uidSourcePairs.size(); ++col) {
    const int srcId       = uidSourcePairs[col].second;
    m_columnToSource[col] = srcId;
    m_sourceColumnCount[srcId]++;
  }

  m_multiSource = !m_columnToSource.isEmpty() && m_sourceColumnCount.size() > 1;
}

/**
 * @brief Populates @c m_timestampsNs with every distinct timestamp, ascending.
 */
void Sessions::Player::buildTimestampIndex()
{
  m_timestampsNs.clear();

  QSqlQuery q(m_db);
  q.setForwardOnly(true);
  q.prepare("SELECT DISTINCT timestamp_ns FROM readings "
            "WHERE session_id = ? ORDER BY timestamp_ns ASC");
  q.bindValue(0, m_sessionId);

  if (!q.exec()) {
    qWarning() << "[Sessions::Player] timestamp index query failed:" << q.lastError().text();
    return;
  }

  while (q.next())
    m_timestampsNs.push_back(q.value(0).toLongLong());
}

//--------------------------------------------------------------------------------------------------
// Frame synthesis
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reconstructs a CSV-style payload for the given timestamp.
 */
QByteArray Sessions::Player::buildFrameAt(qint64 timestampNs)
{
  QStringList cells;
  cells.reserve(static_cast<int>(m_columnUniqueIds.size()));
  for (size_t i = 0; i < m_columnUniqueIds.size(); ++i)
    cells.append(QString());

  // Reverse index: uniqueId → column position
  QMap<int, int> uidToCol;
  for (int i = 0; i < static_cast<int>(m_columnUniqueIds.size()); ++i)
    uidToCol.insert(m_columnUniqueIds[static_cast<size_t>(i)], i);

  // Pull the row set for this timestamp (prefer final_*, ordered by reading_id)
  QSqlQuery q(m_db);
  q.prepare("SELECT unique_id, final_numeric_value, final_string_value, is_numeric "
            "FROM readings WHERE session_id = ? AND timestamp_ns = ? "
            "ORDER BY reading_id");
  q.bindValue(0, m_sessionId);
  q.bindValue(1, timestampNs);

  if (!q.exec()) [[unlikely]] {
    qWarning() << "[Sessions::Player] frame query failed:" << q.lastError().text();
    return QByteArray();
  }

  while (q.next()) {
    const int uid = q.value(0).toInt();
    const auto it = uidToCol.constFind(uid);
    if (it == uidToCol.constEnd())
      continue;

    const bool isNumeric = q.value(3).toInt() != 0;
    if (isNumeric) {
      const double v    = q.value(1).toDouble();
      cells[it.value()] = QString::number(v, 'g', 17);
    } else {
      cells[it.value()] = q.value(2).toString();
    }
  }

  QByteArray frame = cells.join(',').toUtf8();
  frame.append('\n');
  return frame;
}

/**
 * @brief Pushes @p frame into the live pipeline via ConnectionManager.
 */
void Sessions::Player::injectFrame(const QByteArray& frame)
{
  if (frame.isEmpty() || (frame.size() == 1 && frame.at(0) == '\n'))
    return;

  if (!m_multiSource) {
    IO::ConnectionManager::instance().processPayload(frame);
    return;
  }

  // Multi-source: slice the CSV row into per-source payloads by column position
  const auto fields = QString::fromUtf8(frame).trimmed().split(',');

  QMap<int, QStringList> sourceFields;
  for (int col = 0; col < fields.size(); ++col) {
    auto it = m_columnToSource.constFind(col);
    if (it == m_columnToSource.constEnd())
      continue;

    sourceFields[it.value()].append(fields[col]);
  }

  QMap<int, QByteArray> sourcePayloads;
  for (auto it = sourceFields.constBegin(); it != sourceFields.constEnd(); ++it)
    sourcePayloads[it.key()] = it.value().join(',').toUtf8() + '\n';

  IO::ConnectionManager::instance().processMultiSourcePayload(frame, sourcePayloads);
}

//--------------------------------------------------------------------------------------------------
// Display helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates @c m_timestamp to HH:MM:SS.mmm of the current frame's offset.
 */
void Sessions::Player::updateTimestampDisplay()
{
  if (m_framePos < 0 || m_framePos >= frameCount())
    return;

  const double seconds = m_timestampsNs[static_cast<size_t>(m_framePos)] / 1e9;
  m_timestamp          = formatTimestamp(seconds);
  Q_EMIT timestampChanged();
}

/**
 * @brief Formats @p seconds as HH:MM:SS.mmm for the player status label.
 */
QString Sessions::Player::formatTimestamp(double seconds) const
{
  int hours   = static_cast<int>(seconds / 3600.0);
  int minutes = static_cast<int>((seconds - hours * 3600.0) / 60.0);
  double secs = seconds - hours * 3600.0 - minutes * 60.0;

  return QString("%1:%2:%3")
    .arg(hours, 2, 10, QChar('0'))
    .arg(minutes, 2, 10, QChar('0'))
    .arg(secs, 6, 'f', 3, QChar('0'));
}

//--------------------------------------------------------------------------------------------------
// Event handling
//--------------------------------------------------------------------------------------------------

/**
 * @brief Intercepts global key events while a session is loaded.
 */
bool Sessions::Player::eventFilter(QObject* obj, QEvent* event)
{
  if (isOpen() && event->type() == QEvent::KeyPress) {
    auto* keyEvent = static_cast<QKeyEvent*>(event);
    return handleKeyPress(keyEvent);
  }

  return QObject::eventFilter(obj, event);
}

/**
 * @brief Maps Space / arrow / media keys to play/pause and frame stepping.
 */
bool Sessions::Player::handleKeyPress(QKeyEvent* keyEvent)
{
  bool handled = false;
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
      break;
  }

  return handled;
}

#endif  // BUILD_COMMERCIAL
