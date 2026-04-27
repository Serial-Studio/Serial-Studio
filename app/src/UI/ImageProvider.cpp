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

#include "UI/ImageProvider.h"

static UI::ImageProvider* s_globalProvider = nullptr;

//--------------------------------------------------------------------------------------------------
// Constructor & global access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the QML image provider.
 */
UI::ImageProvider::ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

/**
 * @brief Returns the globally registered image provider instance.
 */
UI::ImageProvider* UI::ImageProvider::global()
{
  return s_globalProvider;
}

/**
 * @brief Sets the globally registered image provider instance.
 */
void UI::ImageProvider::setGlobal(UI::ImageProvider* provider)
{
  s_globalProvider = provider;
}

//--------------------------------------------------------------------------------------------------
// QQuickImageProvider interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a cached image scaled to the requested size, or an empty image if missing.
 */
QImage UI::ImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
  // Extract the provider key by stripping the version suffix
  const QString key = id.section(QLatin1Char('/'), 0, 0);

  // Look up the cached image under a read lock
  QReadLocker locker(&m_lock);
  const auto it = m_images.constFind(key);
  if (it == m_images.cend())
    return QImage();

  QImage img = it.value();
  if (size)
    *size = img.size();

  if (!requestedSize.isEmpty())
    img = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  return img;
}

/**
 * @brief Stores or replaces an image under the given identifier.
 */
void UI::ImageProvider::setImage(const QString& id, const QImage& image)
{
  QWriteLocker locker(&m_lock);
  m_images.insert(id, image);
}