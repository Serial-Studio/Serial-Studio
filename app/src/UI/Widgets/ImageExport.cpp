/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "UI/Widgets/ImageExport.h"

#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QImage>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QVideoFrameInput>

#include "IO/ConnectionManager.h"
#include "Misc/WorkspaceManager.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// VideoExportWorker
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stops every active recorder and waits for muxer trailers to flush.
 *
 * Skips the QEventLoop spin used in mid-session stops -- the worker thread is
 * winding down here, and re-entering its own event loop from inside a
 * destructor risks deadlocks and PAC failures on macOS 26 if a queued signal
 * fires after the recorder has already started disposing of its private d-ptr.
 */
Widgets::VideoExportWorker::~VideoExportWorker()
{
  for (auto& [groupId, session] : m_sessions) {
    if (session.recorder && session.recording) {
      session.recording = false;
      session.recorder->stop();
    }

    if (session.captureSession) {
      session.captureSession->setRecorder(nullptr);
      session.captureSession->setVideoFrameInput(nullptr);
    }

    session.frameInput.reset();
    session.recorder.reset();
    session.captureSession.reset();
  }

  m_sessions.clear();
}

/**
 * @brief Returns @c true if any per-group video recorder is currently active.
 */
bool Widgets::VideoExportWorker::isResourceOpen() const
{
  return !m_sessions.empty();
}

/**
 * @brief Stops every active recorder, finalising each .mp4 trailer.
 *
 * Called on full disconnect or application shutdown.
 */
void Widgets::VideoExportWorker::closeResources()
{
  for (auto& [groupId, session] : m_sessions)
    stopSession(session);

  m_sessions.clear();
}

/**
 * @brief Stops only the recorder for @p groupId and finalises its file.
 */
void Widgets::VideoExportWorker::closeGroup(int groupId)
{
  // Drain any pending frames before stopping so the recorder picks them up
  processData();

  auto it = m_sessions.find(groupId);
  if (it == m_sessions.end())
    return;

  stopSession(it->second);
  m_sessions.erase(it);
  Q_EMIT resourceOpenChanged();
}

/**
 * @brief Decodes each queued image and pushes it into its session's QVideoSink.
 */
void Widgets::VideoExportWorker::processItems(const std::vector<ImageExportItem>& items)
{
  for (const auto& item : items) {
    if (item.data.isEmpty())
      continue;

    QImage img = QImage::fromData(item.data);
    if (img.isNull())
      continue;

    // Convert to RGB32 so QVideoFrame accepts the buffer directly
    if (img.format() != QImage::Format_RGB32)
      img = img.convertToFormat(QImage::Format_RGB32);

    auto sit = m_sessions.find(item.groupId);
    if (sit == m_sessions.end()) {
      if (!ensureSession(item.groupId, item.projectTitle, item.groupTitle, img))
        continue;

      sit = m_sessions.find(item.groupId);
      if (sit == m_sessions.end())
        continue;
    }

    auto& session = sit->second;
    if (!session.frameInput || !session.recording)
      continue;

    // Reject frames whose resolution changed mid-stream
    if (img.size() != session.frameSize) {
      img = img.scaled(session.frameSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
              .convertToFormat(QImage::Format_RGB32);
      if (img.size() != session.frameSize) {
        QImage padded(session.frameSize, QImage::Format_RGB32);
        padded.fill(Qt::black);
        QPainter p(&padded);
        const int x = (session.frameSize.width() - img.width()) / 2;
        const int y = (session.frameSize.height() - img.height()) / 2;
        p.drawImage(x, y, img);
        p.end();
        img = padded;
      }
    }

    QVideoFrame frame(QVideoFrameFormat(session.frameSize, QVideoFrameFormat::Format_BGRA8888));
    if (!frame.map(QVideoFrame::WriteOnly))
      continue;

    // RGB32 <-> BGRA8888 -- copy line-by-line (strides may differ)
    const qsizetype dstStride = frame.bytesPerLine(0);
    const qsizetype srcStride = img.bytesPerLine();
    const qsizetype copyBytes = std::min(dstStride, srcStride);
    uchar* dst                = frame.bits(0);
    const uchar* src          = img.constBits();
    for (int y = 0; y < session.frameSize.height(); ++y)
      std::memcpy(dst + y * dstStride, src + y * srcStride, static_cast<size_t>(copyBytes));

    frame.unmap();

    // Real-time pacing: elapsed-microseconds frame timestamps
    const auto elapsedUs =
      std::chrono::duration_cast<std::chrono::microseconds>(item.timestamp - session.startTime)
        .count();
    frame.setStartTime(elapsedUs);
    frame.setEndTime(elapsedUs);

    session.frameInput->sendVideoFrame(frame);
    ++session.frameCount;
  }
}

/**
 * @brief Sets up the QMediaRecorder pipeline for a new per-group .mp4.
 */
bool Widgets::VideoExportWorker::ensureSession(int groupId,
                                               const QString& projectTitle,
                                               const QString& groupTitle,
                                               const QImage& firstFrame)
{
  const auto dt      = QDateTime::currentDateTime();
  const auto stamp   = dt.toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss-zzz"));
  const auto base    = Misc::WorkspaceManager::instance().path(QStringLiteral("Video Recordings"));
  const QString path = QStringLiteral("%1/%2/%3").arg(base, projectTitle, groupTitle);

  QDir dir(path);
  if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
    qWarning() << "ImageExport: failed to create directory:" << path;
    return false;
  }

  const QString file = dir.filePath(stamp + QStringLiteral(".mp4"));

  VideoSession session;
  session.outputPath     = file;
  session.frameSize      = firstFrame.size();
  session.startTime      = std::chrono::steady_clock::now();
  session.captureSession = std::make_unique<QMediaCaptureSession>();
  session.recorder       = std::make_unique<QMediaRecorder>();
  session.frameInput     = std::make_unique<QVideoFrameInput>();

  // QMediaCaptureSession routes the pushed frames to the recorder.
  session.captureSession->setVideoFrameInput(session.frameInput.get());
  session.captureSession->setRecorder(session.recorder.get());

  QMediaFormat format;
  format.setFileFormat(QMediaFormat::FileFormat::MPEG4);
  format.setVideoCodec(QMediaFormat::VideoCodec::H264);
  session.recorder->setMediaFormat(format);
  session.recorder->setQuality(QMediaRecorder::HighQuality);
  session.recorder->setOutputLocation(QUrl::fromLocalFile(file));

  session.recorder->record();

  // Wait for QMediaRecorder to transition to RecordingState
  QEventLoop loop;
  QTimer timeoutTimer;
  timeoutTimer.setSingleShot(true);
  timeoutTimer.setInterval(1500);
  QObject::connect(session.recorder.get(),
                   &QMediaRecorder::recorderStateChanged,
                   &loop,
                   [&loop](QMediaRecorder::RecorderState state) {
                     if (state == QMediaRecorder::RecordingState)
                       loop.quit();
                   });
  QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
  if (session.recorder->recorderState() != QMediaRecorder::RecordingState) {
    timeoutTimer.start();
    loop.exec();
  }

  if (session.recorder->recorderState() != QMediaRecorder::RecordingState) {
    qWarning() << "ImageExport: recorder failed to start for" << file << "--"
               << session.recorder->errorString();
    return false;
  }

  session.recording = true;

  m_sessions.emplace(groupId, std::move(session));
  Q_EMIT resourceOpenChanged();
  return true;
}

/**
 * @brief Stops the recorder and waits for the muxer trailer to flush to disk.
 */
void Widgets::VideoExportWorker::stopSession(VideoSession& session)
{
  if (!session.recorder || !session.recording)
    return;

  session.recording = false;
  session.recorder->stop();

  // Wait for .mp4 muxer to flush its moov atom
  if (session.recorder->recorderState() != QMediaRecorder::StoppedState) {
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.setInterval(3000);
    QObject::connect(session.recorder.get(),
                     &QMediaRecorder::recorderStateChanged,
                     &loop,
                     [&loop](QMediaRecorder::RecorderState state) {
                       if (state == QMediaRecorder::StoppedState)
                         loop.quit();
                     });
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start();
    loop.exec();
  }

  // Drop empty output files
  if (session.frameCount == 0)
    QFile::remove(session.outputPath);

  // Detach inputs before tearing down (QMediaCaptureSession holds raw pointers)
  if (session.captureSession) {
    session.captureSession->setRecorder(nullptr);
    session.captureSession->setVideoFrameInput(nullptr);
  }

  session.frameInput.reset();
  session.recorder.reset();
  session.captureSession.reset();
}

//--------------------------------------------------------------------------------------------------
// ImageExport
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the singleton, starts the encoding thread, and restores
 *        the persisted "save by default" preference.
 */
Widgets::ImageExport::ImageExport()
  : DataModel::FrameConsumer<ImageExportItem>(
      {.queueCapacity = 512, .flushThreshold = 8, .timerIntervalMs = 33})
{
  m_exportEnabled = m_settings.value(QStringLiteral("ImageExport/enabled"), false).toBool();

  initializeWorker();
  setConsumerEnabled(true);
}

/**
 * @brief Default destructor.
 */
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
 * @brief Factory called by @c FrameConsumer to create the encoder worker.
 */
DataModel::FrameConsumerWorkerBase* Widgets::ImageExport::createWorker()
{
  return new VideoExportWorker(&m_pendingQueue, &m_consumerEnabled, &m_queueSize);
}

/**
 * @brief Returns the current "save videos by default" preference.
 */
bool Widgets::ImageExport::exportEnabled() const
{
  return m_exportEnabled;
}

/**
 * @brief Returns the group-level directory path used for video exports.
 */
QString Widgets::ImageExport::imagesPath(const QString& groupTitle,
                                         const QString& projectTitle) const
{
  const auto base = Misc::WorkspaceManager::instance().path(QStringLiteral("Video Recordings"));
  return QStringLiteral("%1/%2/%3").arg(base, projectTitle, groupTitle);
}

/**
 * @brief Asks the worker to stop and finalise every active recording.
 */
void Widgets::ImageExport::closeSession()
{
  QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
}

/**
 * @brief Asks the worker to stop and finalise the recording for @p groupId.
 */
void Widgets::ImageExport::closeSession(int groupId)
{
  QMetaObject::invokeMethod(static_cast<VideoExportWorker*>(m_worker),
                            "closeGroup",
                            Qt::QueuedConnection,
                            Q_ARG(int, groupId));
}

/**
 * @brief Wires connection-manager signals so recordings stop on disconnect or pause.
 */
void Widgets::ImageExport::setupExternalConnections()
{
  connect(
    &IO::ConnectionManager::instance(), &IO::ConnectionManager::connectedChanged, this, [this] {
      if (!IO::ConnectionManager::instance().isConnected())
        closeSession();
    });

  connect(&IO::ConnectionManager::instance(), &IO::ConnectionManager::pausedChanged, this, [this] {
    if (IO::ConnectionManager::instance().paused())
      closeSession();
  });
}

/**
 * @brief Persists the "save videos by default" preference and notifies QML.
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
 * @brief Enqueues a decoded image frame for asynchronous video encoding.
 */
void Widgets::ImageExport::enqueueImage(const QByteArray& data,
                                        const QString& format,
                                        int groupId,
                                        const QString& groupTitle,
                                        const QString& projectTitle)
{
  // Skip during playback -- we never re-encode replayed frames.
  if (SerialStudio::isAnyPlayerOpen())
    return;

  ImageExportItem item;
  item.data         = data;
  item.format       = format;
  item.groupId      = groupId;
  item.groupTitle   = groupTitle;
  item.projectTitle = projectTitle;
  item.timestamp    = std::chrono::steady_clock::now();
  enqueueData(item);
}
