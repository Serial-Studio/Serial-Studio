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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/IconEngine.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QPainter>
#include <QSvgRenderer>
#include <QUrlQuery>

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the IconEngine singleton.
 */
Misc::IconEngine::IconEngine() : m_busy(false) {}

/**
 * @brief Returns the global IconEngine instance.
 */
Misc::IconEngine& Misc::IconEngine::instance()
{
  static IconEngine s;
  return s;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a network request is in progress.
 */
bool Misc::IconEngine::busy() const noexcept
{
  return m_busy;
}

/**
 * @brief Returns the display names of the last search results.
 */
const QStringList& Misc::IconEngine::iconNames() const noexcept
{
  return m_iconNames;
}

/**
 * @brief Returns preview URLs for the last search results.
 *
 * Each entry is a full Iconify API URL that returns an SVG image.
 */
const QStringList& Misc::IconEngine::iconPreviews() const noexcept
{
  return m_iconPreviews;
}

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves an action icon string to a URL suitable for QML Image.source.
 *
 * If @p icon starts with @c "data:image/svg+xml;base64," it is treated as
 * inline SVG data and resolved to an @c image://actionicon/ provider URL.
 * Otherwise it is treated as a bundled icon name from @c qrc:/rcc/actions/.
 *
 * @param icon The icon field from the Action struct.
 * @return A URL string usable in QML @c Image.source.
 */
QString Misc::IconEngine::resolveActionIconSource(const QString& icon)
{
  // Resolve inline SVG data URIs to image provider URLs
  if (isInlineSvg(icon)) {
    const auto base64 = icon.mid(QStringLiteral("data:image/svg+xml;base64,").length());
    return QStringLiteral("image://actionicon/%1").arg(base64);
  }

  // Fall back to bundled icon from resources
  return QStringLiteral("qrc:/rcc/actions/%1.svg").arg(icon);
}

/**
 * @brief Returns true if @p icon contains inline SVG data (base64 data URI).
 */
bool Misc::IconEngine::isInlineSvg(const QString& icon)
{
  return icon.startsWith(QStringLiteral("data:image/svg+xml;base64,"));
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Searches the Iconify API for icons matching @p query.
 *
 * Results are stored in iconNames() and iconPreviews(), then
 * searchResultsChanged() is emitted.
 */
void Misc::IconEngine::searchIcons(const QString& query)
{
  if (query.trimmed().isEmpty())
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  // Build the search URL
  QUrl url(QStringLiteral("https://api.iconify.design/search"));
  QUrlQuery params;
  params.addQueryItem(QStringLiteral("query"), query.trimmed());
  params.addQueryItem(QStringLiteral("limit"), QStringLiteral("96"));
  url.setQuery(params);

  // Send request
  auto* reply = m_manager.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished, this, [this, reply]() { onSearchFinished(reply); });
}

/**
 * @brief Downloads the SVG data for the icon at @p index in the current
 *        search results.
 *
 * On success, emits iconDownloaded() with a @c data:image/svg+xml;base64,...
 * string ready to be stored in the Action icon field.
 */
void Misc::IconEngine::downloadIcon(int index)
{
  if (index < 0 || index >= m_iconNames.size())
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  // Split the icon name into prefix and name (e.g. "mdi:home")
  const auto& name = m_iconNames.at(index);
  const auto parts = name.split(QStringLiteral(":"));
  if (parts.size() != 2) {
    m_busy = false;
    Q_EMIT busyChanged();
    Q_EMIT iconDownloadFailed(tr("Invalid icon identifier"));
    return;
  }

  // Build the SVG download URL and send request
  const auto urlStr =
    QStringLiteral("https://api.iconify.design/%1/%2.svg").arg(parts[0], parts[1]);

  auto* reply = m_manager.get(QNetworkRequest(QUrl(urlStr)));
  connect(reply, &QNetworkReply::finished, this, [this, reply]() { onDownloadFinished(reply); });
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the Iconify search API response.
 *
 * Parses the JSON response, extracts icon identifiers, builds preview
 * URLs, and emits searchResultsChanged().
 */
void Misc::IconEngine::onSearchFinished(QNetworkReply* reply)
{
  reply->deleteLater();

  m_iconNames.clear();
  m_iconPreviews.clear();

  if (reply->error() == QNetworkReply::NoError) {
    const auto data  = reply->readAll();
    const auto doc   = QJsonDocument::fromJson(data);
    const auto obj   = doc.object();
    const auto icons = obj.value(QStringLiteral("icons")).toArray();

    for (const auto& iconRef : icons) {
      const auto name = iconRef.toString();
      if (name.isEmpty())
        continue;

      m_iconNames.append(name);

      const auto parts = name.split(QStringLiteral(":"));
      if (parts.size() == 2) {
        m_iconPreviews.append(
          QStringLiteral("https://api.iconify.design/%1/%2.svg").arg(parts[0], parts[1]));
      } else {
        m_iconPreviews.append(QString());
      }
    }
  }

  // Update busy state and notify listeners
  m_busy = false;
  Q_EMIT busyChanged();
  Q_EMIT searchResultsChanged();
}

/**
 * @brief Handles the SVG download response.
 *
 * Encodes the SVG data as a base64 data URI and emits iconDownloaded().
 */
void Misc::IconEngine::onDownloadFinished(QNetworkReply* reply)
{
  reply->deleteLater();

  m_busy = false;
  Q_EMIT busyChanged();

  // Abort on network error
  if (reply->error() != QNetworkReply::NoError) {
    Q_EMIT iconDownloadFailed(reply->errorString());
    return;
  }

  // Read the SVG payload
  const auto svgData = reply->readAll();
  if (svgData.isEmpty()) {
    Q_EMIT iconDownloadFailed(tr("Empty SVG data received"));
    return;
  }

  // Encode as base64 data URI and emit result
  const auto base64 = svgData.toBase64();
  const auto dataUri =
    QStringLiteral("data:image/svg+xml;base64,%1").arg(QString::fromLatin1(base64));
  Q_EMIT iconDownloaded(dataUri);
}

//--------------------------------------------------------------------------------------------------
// ActionIconProvider
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the image provider for inline SVG action icons.
 */
Misc::ActionIconProvider::ActionIconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

/**
 * @brief Decodes a base64-encoded SVG and renders it to a QImage.
 *
 * The @p id parameter contains the raw base64 SVG data (without the data URI
 * prefix).
 */
QImage Misc::ActionIconProvider::requestImage(const QString& id,
                                              QSize* size,
                                              const QSize& requestedSize)
{
  // Decode the base64 SVG data
  const auto svgData = QByteArray::fromBase64(id.toLatin1());
  if (svgData.isEmpty()) {
    if (size)
      *size = QSize(0, 0);

    return {};
  }

  QSvgRenderer renderer(svgData);
  if (!renderer.isValid()) {
    if (size)
      *size = QSize(0, 0);

    return {};
  }

  // Determine output size
  const int w = requestedSize.width() > 0 ? requestedSize.width() : 64;
  const int h = requestedSize.height() > 0 ? requestedSize.height() : 64;

  // Paint the SVG onto a transparent image
  QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  renderer.render(&painter);
  painter.end();

  // Report the actual size back to the caller
  if (size)
    *size = image.size();

  return image;
}
