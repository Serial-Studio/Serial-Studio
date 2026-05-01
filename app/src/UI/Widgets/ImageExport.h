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

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <QByteArray>
#include <QDir>
#include <QImage>
#include <QObject>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QUrl>
#include <unordered_map>

#include "DataModel/FrameConsumer.h"

QT_BEGIN_NAMESPACE
class QMediaCaptureSession;
class QMediaRecorder;
class QVideoFrameInput;
QT_END_NAMESPACE

namespace Widgets {

/**
 * @brief Image payload enqueued for async video encoding.
 */
struct ImageExportItem {
  QByteArray data;
  QString format;
  int groupId;
  QString groupTitle;
  QString projectTitle;
  std::chrono::steady_clock::time_point timestamp;
};

/**
 * @brief Per-group video-recording session state owned by @c VideoExportWorker.
 *
 * Each pinned image widget streams into its own .mp4 via Qt Multimedia. The
 * QMediaRecorder + QMediaCaptureSession + QVideoSink trio produce a hardware-
 * accelerated H.264 file on macOS / Windows; on Linux the platform's
 * GStreamer plugins decide the backend.
 */
struct VideoSession {
  QString outputPath;
  QSize frameSize;
  std::chrono::steady_clock::time_point startTime;
  std::unique_ptr<QMediaCaptureSession> captureSession;
  std::unique_ptr<QMediaRecorder> recorder;
  std::unique_ptr<QVideoFrameInput> frameInput;
  bool recording{false};
  qint64 frameCount{0};
};

/**
 * @brief Worker that encodes image streams into .mp4 files on a background thread.
 */
class VideoExportWorker : public DataModel::FrameConsumerWorker<ImageExportItem> {
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<ImageExportItem>::FrameConsumerWorker;

  ~VideoExportWorker() override;

  void closeResources() override;
  [[nodiscard]] bool isResourceOpen() const override;

public slots:
  void closeGroup(int groupId);

protected:
  void processItems(const std::vector<ImageExportItem>& items) override;

private:
  bool ensureSession(int groupId,
                     const QString& projectTitle,
                     const QString& groupTitle,
                     const QImage& firstFrame);
  void stopSession(VideoSession& session);

  std::unordered_map<int, VideoSession> m_sessions;
};

/**
 * @brief Singleton consumer that encodes per-group image streams into .mp4
 *        videos via Qt Multimedia (Pro feature).
 */
class ImageExport : public DataModel::FrameConsumer<ImageExportItem> {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool exportEnabled
             READ exportEnabled
             WRITE setExportEnabled
             NOTIFY enabledChanged)
  // clang-format on

signals:
  void enabledChanged();

private:
  explicit ImageExport();
  ImageExport(ImageExport&&)                 = delete;
  ImageExport(const ImageExport&)            = delete;
  ImageExport& operator=(ImageExport&&)      = delete;
  ImageExport& operator=(const ImageExport&) = delete;

  ~ImageExport();

public:
  [[nodiscard]] static ImageExport& instance();

  [[nodiscard]] bool exportEnabled() const;
  [[nodiscard]] Q_INVOKABLE QString imagesPath(const QString& groupTitle,
                                               const QString& projectTitle) const;

public slots:
  void closeSession();
  void closeSession(int groupId);
  void setupExternalConnections();
  void setExportEnabled(bool enabled);
  void enqueueImage(const QByteArray& data,
                    const QString& format,
                    int groupId,
                    const QString& groupTitle,
                    const QString& projectTitle);

protected:
  DataModel::FrameConsumerWorkerBase* createWorker() override;

private:
  QSettings m_settings;
  bool m_exportEnabled;
};

}  // namespace Widgets
