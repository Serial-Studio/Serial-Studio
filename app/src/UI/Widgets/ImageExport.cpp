/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/ImageExport.h"

#  include <JlCompress.h>

#  include <QDateTime>
#  include <QFile>

#  include "IO/Manager.h"
#  include "Misc/WorkspaceManager.h"
#  include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// ImageExportWorker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns @c true if any per-group session is currently open.
 */
bool Widgets::ImageExportWorker::isResourceOpen() const
{
  return !m_sessions.isEmpty();
}

/**
 * @brief Closes all open sessions, zipping each one.
 *
 * Called on full disconnect or application shutdown.
 */
void Widgets::ImageExportWorker::closeResources()
{
  for (auto& session : m_sessions)
    zipAndClean(session);

  m_sessions.clear();
}

/**
 * @brief Closes and zips only the session for the given group.
 *
 * Called when a single @c ImageView widget disables its export toggle.
 * @param groupId  Group ID identifying the session to close.
 */
void Widgets::ImageExportWorker::closeGroup(int groupId)
{
  processData();

  auto it = m_sessions.find(groupId);
  if (it == m_sessions.end())
    return;

  zipAndClean(it.value());
  m_sessions.erase(it);
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Writes a batch of image frames, routing each to its per-group session.
 *
 * Sessions are created lazily on the first frame for each group.
 * @param items  Batch of image payloads dequeued from the lock-free queue.
 */
void Widgets::ImageExportWorker::processItems(const std::vector<ImageExportItem>& items)
{
  for (const auto& item : items) {
    if (item.data.isEmpty())
      continue;

    if (!m_sessions.contains(item.groupId)) {
      if (!ensureSession(item.groupId, item.projectTitle, item.groupTitle))
        continue;
    }

    auto& session      = m_sessions[item.groupId];
    const QString ext  = item.format.toLower();
    const QString name = QStringLiteral("frame_%1.%2")
                           .arg(session.frameIndex, 4, 10, QChar('0'))
                           .arg(ext.isEmpty() ? QStringLiteral("bin") : ext);

    if (!session.dir.exists())
      session.dir.mkpath(QStringLiteral("."));

    QFile file(session.dir.filePath(name));
    if (file.open(QIODevice::WriteOnly)) {
      file.write(item.data);
      file.close();
      ++session.frameIndex;
    } else {
      qWarning() << "ImageExport: cannot write" << file.fileName();
    }
  }
}

/**
 * @brief Creates the timestamped session directory for a group.
 * @param groupId       Unique group ID used as the session map key.
 * @param projectTitle  Dashboard project title used as a sub-directory name.
 * @param groupTitle    Widget group title used as the directory name.
 * @return @c true on success, @c false if the directory could not be created.
 */
bool Widgets::ImageExportWorker::ensureSession(int groupId,
                                               const QString& projectTitle,
                                               const QString& groupTitle)
{
  const auto dt      = QDateTime::currentDateTime();
  const auto session = dt.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz"));

  const auto base    = Misc::WorkspaceManager::instance().path(QStringLiteral("Images"));
  const QString path = QStringLiteral("%1/%2/%3/%4").arg(base, projectTitle, groupTitle, session);

  QDir dir(path);
  if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
    qWarning() << "ImageExport: failed to create directory:" << path;
    return false;
  }

  m_sessions.insert(groupId, ImageSession{dir, 0});
  Q_EMIT resourceOpenChanged();
  return true;
}

/**
 * @brief Compresses a session directory into a ZIP archive and removes the raw folder.
 *
 * If compression fails the raw directory is kept for manual recovery.
 * @param session  The session whose directory should be zipped.
 */
void Widgets::ImageExportWorker::zipAndClean(ImageSession& session)
{
  if (session.frameIndex == 0)
    return;

  const QString dirPath = session.dir.absolutePath();
  const QString zipPath = dirPath + QStringLiteral(".zip");

  if (!JlCompress::compressDir(zipPath, dirPath, true)) {
    qWarning() << "ImageExport: failed to create archive:" << zipPath;
    return;
  }

  session.dir.removeRecursively();
}

//--------------------------------------------------------------------------------------------------
// ImageExport
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the singleton, initializing the worker thread and
 *        restoring the last-saved "save by default" preference from QSettings.
 *
 * The worker thread is always started and ready; @c m_exportEnabled controls
 * only whether newly created @c ImageView widgets inherit the export-on state.
 */
Widgets::ImageExport::ImageExport()
  : DataModel::FrameConsumer<ImageExportItem>(
      {.queueCapacity = 512, .flushThreshold = 32, .timerIntervalMs = 1000})
{
  m_exportEnabled = m_settings.value(QStringLiteral("ImageExport/enabled"), false).toBool();

  initializeWorker();
  setConsumerEnabled(true);
}

Widgets::ImageExport::~ImageExport() = default;

/**
 * @brief Returns the application-wide @c ImageExport singleton.
 */
Widgets::ImageExport& Widgets::ImageExport::instance()
{
  static ImageExport singleton;
  return singleton;
}

/**
 * @brief Factory method called by @c FrameConsumer to instantiate the worker.
 * @return Newly allocated @c ImageExportWorker on the worker thread.
 */
DataModel::FrameConsumerWorkerBase* Widgets::ImageExport::createWorker()
{
  return new ImageExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Returns @c true if the image export consumer is currently active.
 */
bool Widgets::ImageExport::exportEnabled() const
{
  return m_exportEnabled;
}

/**
 * @brief Returns the group-level directory path used for image exports.
 *
 * The path does not include the per-session timestamp subdirectory, making it
 * suitable for opening a stable folder in the file manager.
 * @param groupTitle    Widget group title.
 * @param projectTitle  Dashboard project title.
 * @return Absolute path to @c <workspace>/Images/<project>/<group>.
 */
QString Widgets::ImageExport::imagesPath(const QString& groupTitle,
                                         const QString& projectTitle) const
{
  const auto base = Misc::WorkspaceManager::instance().path(QStringLiteral("Images"));
  return QStringLiteral("%1/%2/%3").arg(base, projectTitle, groupTitle);
}

/**
 * @brief Signals the worker to close all sessions (used on disconnect).
 */
void Widgets::ImageExport::closeSession()
{
  QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
}

/**
 * @brief Signals the worker to close only the session for @p groupTitle.
 *
 * Called when a single @c ImageView widget disables its export toggle,
 * leaving other widgets' sessions untouched.
 * @param groupTitle  Group title identifying the session to close.
 */
void Widgets::ImageExport::closeSession(int groupId)
{
  QMetaObject::invokeMethod(static_cast<ImageExportWorker*>(m_worker),
                            "closeGroup",
                            Qt::QueuedConnection,
                            Q_ARG(int, groupId));
}

/**
 * @brief Connects @c IO::Manager signals so sessions close automatically on
 *        device disconnect or stream pause.
 */
void Widgets::ImageExport::setupExternalConnections()
{
  connect(&IO::Manager::instance(), &IO::Manager::connectedChanged, this, [this] {
    if (!IO::Manager::instance().isConnected())
      closeSession();
  });

  connect(&IO::Manager::instance(), &IO::Manager::pausedChanged, this, [this] {
    if (IO::Manager::instance().paused())
      closeSession();
  });
}

/**
 * @brief Sets the "save images by default" preference and persists it.
 *
 * This preference controls whether newly created @c ImageView widgets start
 * with export enabled. It does not affect the worker thread lifecycle.
 * The value is saved to QSettings under @c "ImageExport/enabled".
 * @param enabled  @c true to save by default, @c false to opt out.
 */
void Widgets::ImageExport::setExportEnabled(bool enabled)
{
  if (m_exportEnabled == enabled)
    return;

  m_exportEnabled = enabled;
  m_settings.setValue(QStringLiteral("ImageExport/enabled"), enabled);
  Q_EMIT enabledChanged();
}

/**
 * @brief Enqueues a decoded image frame for async export to disk.
 *
 * No-op during playback. The worker thread is always running; the per-widget
 * @c exportEnabled flag on @c ImageView is the sole gatekeeper for whether
 * frames are submitted here.
 * @param data          Raw bytes of the decoded image.
 * @param format        Format string such as "JPEG" or "PNG".
 * @param groupTitle    Widget group title (used for the directory structure).
 * @param projectTitle  Dashboard project title (used for the directory structure).
 */
void Widgets::ImageExport::enqueueImage(const QByteArray& data,
                                        const QString& format,
                                        int groupId,
                                        const QString& groupTitle,
                                        const QString& projectTitle)
{
  if (SerialStudio::isAnyPlayerOpen())
    return;

  ImageExportItem item{data, format, groupId, groupTitle, projectTitle};
  enqueueData(item);
}

#endif  // BUILD_COMMERCIAL
