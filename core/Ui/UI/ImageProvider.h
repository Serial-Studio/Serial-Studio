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

#include <QHash>
#include <QImage>
#include <QQuickImageProvider>
#include <QReadWriteLock>

namespace UI {

/**
 * @brief Thread-safe QQuickImageProvider that serves decoded camera/image frames to QML.
 */
class ImageProvider : public QQuickImageProvider {
public:
  explicit ImageProvider();

  static ImageProvider* global();
  static void setGlobal(ImageProvider* provider);

  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

  void setImage(const QString& id, const QImage& image);

private:
  QReadWriteLock m_lock;
  QHash<QString, QImage> m_images;
};

}  // namespace UI
