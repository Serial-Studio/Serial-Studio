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
 * @class ImageProvider
 * @brief Thread-safe QQuickImageProvider that serves decoded camera/image frames to QML.
 *
 * Registered with the QML engine under the scheme @c "serial-studio-img" so QML can
 * request images with URLs of the form:
 * @code
 *   image://serial-studio-img/<providerKey>/<frameCount>
 * @endcode
 *
 * The @c /<frameCount> suffix is a cache-bust token: it makes every URL unique so
 * Qt's scene graph never serves a stale cached texture between frames.
 * @c requestImage() strips the suffix via @c QString::section() before looking up
 * the stored @c QImage by its base @c providerKey.
 *
 * ### Thread safety
 * @c setImage() is called from the main thread (inside @c Widgets::ImageView::onFrameReady).
 * @c requestImage() is called from Qt's image-loading thread pool.
 * Both are protected by a @c QReadWriteLock.
 *
 * ### Lifecycle
 * Created once in @c ModuleManager::initializeQmlInterface() and passed to
 * @c QQmlEngine::addImageProvider(). The engine takes ownership and destroys
 * the instance on shutdown. A raw global pointer @c s_globalProvider is kept
 * so @c ImageView widgets can reach the live instance via @c ImageProvider::global().
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
