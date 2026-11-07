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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>

namespace Misc {

/**
 * @brief Serves a monochrome icon pack's artwork tinted (spec 0115 R21): the id is
 *        `<pack>/<category>/<tier>/<name>.svg?tint=RRGGBB`, the bytes come from
 *        :/iconpacks, every fill and stroke becomes the tint, and the result is rasterized at
 *        the requested size. Runs on Qt's pixmap-reader thread: no registry state is touched.
 */
class IconPackImageProvider : public QQuickImageProvider {
public:
  explicit IconPackImageProvider();

  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
  [[nodiscard]] QByteArray tinted(const QString& path, const QString& tint);
  [[nodiscard]] static QByteArray recolour(const QByteArray& svg, const QString& tint);
  [[nodiscard]] static int tierOf(const QString& path);

private:
  QMutex m_mutex;
  QHash<QString, QByteArray> m_cache;
};

}  // namespace Misc
