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

#include "Misc/IconPackImageProvider.h"

#include <QFile>
#include <QMutexLocker>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>

static constexpr int kMaxCachedEntries = 512;

static const QString kDefaultTint = QStringLiteral("000000");

/**
 * @brief Registers as an image provider: the pixmap reader asks for QImages.
 */
Misc::IconPackImageProvider::IconPackImageProvider()
  : QQuickImageProvider(QQuickImageProvider::Image)
{}

/**
 * @brief Six lowercase hex digits or the default; anything else never reaches the markup.
 */
static QString validTint(const QString& candidate)
{
  static const QRegularExpression kHex(QStringLiteral("^[0-9a-f]{6}$"));
  return kHex.match(candidate).hasMatch() ? candidate : kDefaultTint;
}

/**
 * @brief Renders the tinted icon; an unknown file or a malformed id yields an empty image, which
 *        QML reports as an error rather than drawing a blank.
 */
QImage Misc::IconPackImageProvider::requestImage(const QString& id,
                                                 QSize* size,
                                                 const QSize& requestedSize)
{
  const auto query = id.indexOf(QChar('?'));
  const auto path  = query < 0 ? id : id.left(query);
  QString tint     = kDefaultTint;
  if (query >= 0) {
    const auto marker = QStringLiteral("tint=");
    const auto at     = id.indexOf(marker, query);
    if (at >= 0)
      tint = validTint(id.mid(at + marker.length(), 6).toLower());
  }

  if (path.contains(QStringLiteral("..")) || path.startsWith(QChar('/'))) {
    if (size)
      *size = QSize(0, 0);

    return {};
  }

  const auto svg = tinted(path, tint);
  if (svg.isEmpty()) {
    if (size)
      *size = QSize(0, 0);

    return {};
  }

  QSvgRenderer renderer(svg);
  if (!renderer.isValid()) {
    if (size)
      *size = QSize(0, 0);

    return {};
  }

  const int fallback = tierOf(path) * 2;
  const int w        = requestedSize.width() > 0 ? requestedSize.width() : fallback;
  const int h        = requestedSize.height() > 0 ? requestedSize.height() : fallback;

  QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  renderer.render(&painter);
  painter.end();

  if (size)
    *size = image.size();

  return image;
}

/**
 * @brief Returns the recoloured SVG bytes for @p path under @p tint, from the cache when seen.
 */
QByteArray Misc::IconPackImageProvider::tinted(const QString& path, const QString& tint)
{
  const auto key = path + QChar('#') + tint;

  QMutexLocker lock(&m_mutex);
  const auto hit = m_cache.constFind(key);
  if (hit != m_cache.constEnd())
    return *hit;

  QFile file(QStringLiteral(":/iconpacks/") + path);
  if (!file.open(QFile::ReadOnly))
    return {};

  const auto bytes = recolour(file.readAll(), tint);
  if (m_cache.size() >= kMaxCachedEntries)
    m_cache.clear();

  m_cache.insert(key, bytes);
  return bytes;
}

/**
 * @brief Replaces every hex, rgb() and url() fill and stroke value with @p tint and gives a root
 *        without a fill one, so paths relying on the default black inherit it. Keyword colours
 *        stay: "none" marks stroke icons, and the pack build's knockout masks are drawn in white
 *        and black so a badge hole survives the recolouring.
 */
QByteArray Misc::IconPackImageProvider::recolour(const QByteArray& svg, const QString& tint)
{
  static const QRegularExpression kAttribute(QStringLiteral(
    R"re(((fill|stroke|stop-color)=")(#[0-9A-Fa-f]{3,8}|url\([^"]*\)|rgba?\([^"]*\))("))re"));
  static const QRegularExpression kStyle(QStringLiteral(
    R"re(((fill|stroke|stop-color)\s*:\s*)(#[0-9A-Fa-f]{3,8}|url\([^;"]*\)|rgba?\([^;"]*\)))re"));

  auto text      = QString::fromUtf8(svg);
  const auto hex = QStringLiteral("#") + tint;
  text.replace(kAttribute, QStringLiteral("\\1") + hex + QStringLiteral("\\3"));
  text.replace(kStyle, QStringLiteral("\\1") + hex);

  const auto open = text.indexOf(QStringLiteral("<svg"));
  const auto end  = open < 0 ? -1 : text.indexOf(QChar('>'), open);
  if (open >= 0 && end > open && !text.mid(open, end - open).contains(QStringLiteral(" fill=")))
    text.insert(open + 4, QStringLiteral(" fill=\"") + hex + QStringLiteral("\""));

  return text.toUtf8();
}

/**
 * @brief The tier folder named in `<pack>/<category>/<tier>/<name>.svg`, 16 when unreadable.
 */
int Misc::IconPackImageProvider::tierOf(const QString& path)
{
  const auto parts = path.split(QChar('/'), Qt::SkipEmptyParts);
  if (parts.size() < 4)
    return 16;

  bool ok        = false;
  const int tier = parts[2].toInt(&ok);
  return ok && tier > 0 ? tier : 16;
}
