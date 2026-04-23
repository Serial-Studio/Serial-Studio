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
#include <cstdint>
#include <QByteArray>
#include <QDir>
#include <QHash>
#include <QObject>
#include <QSettings>
#include <QString>

#include "DataModel/FrameConsumer.h"

namespace Widgets {

/**
 * @brief Image payload enqueued for async export.
 */
struct ImageExportItem {
  QByteArray data;
  QString format;
  int groupId;
  QString groupTitle;
  QString projectTitle;
};

/**
 * @brief Per-group session state tracked by @c ImageExportWorker.
 */
struct ImageSession {
  QDir dir;
  int frameIndex{0};
};

/**
 * @brief Worker that writes image files on a dedicated background thread.
 */
class ImageExportWorker : public DataModel::FrameConsumerWorker<ImageExportItem> {
  Q_OBJECT

public:
  using DataModel::FrameConsumerWorker<ImageExportItem>::FrameConsumerWorker;

  void closeResources() override;
  bool isResourceOpen() const override;

public slots:
  void closeGroup(int groupId);

protected:
  void processItems(const std::vector<ImageExportItem>& items) override;

private:
  bool ensureSession(int groupId, const QString& projectTitle, const QString& groupTitle);
  void zipAndClean(ImageSession& session);

  QHash<int, ImageSession> m_sessions;
};

/**
 * @brief Singleton consumer that exports decoded image frames to disk and
 *        zips them into per-group archives when sessions end.
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
