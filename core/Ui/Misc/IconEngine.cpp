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

#include "Misc/IconEngine.h"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkReply>
#include <QPainter>
#include <QSvgRenderer>
#include <QTimer>
#include <QUrlQuery>
#include <utility>

#include "Core/SSAssert.h"
#include "Misc/IconRegistryLegacy.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the IconEngine singleton.
 */
Misc::IconEngine::IconEngine()
  : m_busy(false), m_loading(false), m_inFlight(false), m_attempt(0), m_generation(0)
{
  m_manager.setTransferTimeout(kTransferTimeout);
}

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
 * @brief Returns true while preview batches are still being fetched.
 */
bool Misc::IconEngine::loadingPreviews() const noexcept
{
  return m_loading;
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
 */
QString Misc::IconEngine::resolveActionIconSource(const QString& icon)
{
  if (isInlineSvg(icon)) {
    const auto base64 = icon.mid(QStringLiteral("data:image/svg+xml;base64,").length());
    return QStringLiteral("image://actionicon/%1").arg(base64);
  }

  if (icon.startsWith(QStringLiteral("qrc:")) || icon.startsWith(QChar(':'))
      || icon.contains(QStringLiteral("://")))
    return Misc::legacyIconPath(icon);

  return QStringLiteral("qrc:/actions/%1.svg").arg(icon);
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
 */
void Misc::IconEngine::searchIcons(const QString& query)
{
  const auto trimmed = query.trimmed();
  if (trimmed.isEmpty())
    return;

  ++m_generation;
  m_attempt  = 0;
  m_inFlight = false;
  m_pendingDownload.clear();
  m_pendingBatches.clear();
  refreshLoadingState();

  m_busy = true;
  Q_EMIT busyChanged();

  QUrl url(QStringLiteral("https://api.iconify.design/search"));
  QUrlQuery params;
  params.addQueryItem(QStringLiteral("query"), trimmed);
  params.addQueryItem(QStringLiteral("limit"), QString::number(kSearchLimit));
  url.setQuery(params);

  const int generation = m_generation;
  auto* reply          = m_manager.get(makeRequest(url));
  SS_ASSERT(reply != nullptr, return);

  connect(reply, &QNetworkReply::finished, this, [this, reply, generation]() {
    onSearchFinished(reply, generation);
  });
}

/**
 * @brief Emits the SVG for the icon at @p index, fetching its set when not cached yet.
 */
void Misc::IconEngine::downloadIcon(int index)
{
  if (index < 0 || index >= m_iconNames.size())
    return;

  const auto name = m_iconNames.at(index);
  const auto it   = m_svgCache.constFind(name);
  if (it != m_svgCache.constEnd()) {
    Q_EMIT iconDownloaded(toDataUri(*it));
    return;
  }

  m_busy            = true;
  m_pendingDownload = name;
  Q_EMIT busyChanged();

  m_pendingBatches.prepend(QStringList{name});
  requestNextBatch();
}

//--------------------------------------------------------------------------------------------------
// Search and preview resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the Iconify search API response and queues the preview batches.
 */
void Misc::IconEngine::onSearchFinished(QNetworkReply* reply, int generation)
{
  reply->deleteLater();
  if (generation != m_generation)
    return;

  m_busy = false;
  Q_EMIT busyChanged();

  m_iconNames.clear();

  if (reply->error() != QNetworkReply::NoError) {
    refreshPreviews();
    Q_EMIT searchResultsChanged();
    Q_EMIT searchFailed(reply->errorString());
    return;
  }

  const auto doc = QJsonDocument::fromJson(reply->readAll());
  if (!doc.isObject()) {
    refreshPreviews();
    Q_EMIT searchResultsChanged();
    Q_EMIT searchFailed(tr("The icon service returned a response that could not be read."));
    return;
  }

  const auto icons = doc.object().value(QStringLiteral("icons")).toArray();
  for (const auto& iconRef : icons) {
    const auto name = iconRef.toString();
    if (name.count(QChar(':')) == 1)
      m_iconNames.append(name);
  }

  refreshPreviews();
  queuePreviewFetches();
  Q_EMIT searchResultsChanged();

  refreshLoadingState();
  requestNextBatch();
}

/**
 * @brief Rebuilds the preview source list from the SVG cache.
 */
void Misc::IconEngine::refreshPreviews()
{
  m_iconPreviews.clear();
  m_iconPreviews.reserve(m_iconNames.size());

  for (const auto& name : std::as_const(m_iconNames)) {
    const auto it = m_svgCache.constFind(name);
    m_iconPreviews.append(it == m_svgCache.constEnd() ? QString() : toPreviewSource(*it));
  }

  Q_EMIT previewsChanged();
}

/**
 * @brief Groups the uncached results per icon set and splits them into request batches.
 */
void Misc::IconEngine::queuePreviewFetches()
{
  QMap<QString, QStringList> perSet;
  for (const auto& name : std::as_const(m_iconNames))
    if (!m_svgCache.contains(name))
      perSet[name.section(QChar(':'), 0, 0)].append(name);

  for (auto it = perSet.cbegin(); it != perSet.cend(); ++it)
    for (qsizetype i = 0; i < it.value().size(); i += kPreviewBatch)
      m_pendingBatches.append(it.value().mid(i, kPreviewBatch));
}

/**
 * @brief Requests the next queued batch, one collection request at a time.
 */
void Misc::IconEngine::requestNextBatch()
{
  if (m_inFlight || m_pendingBatches.isEmpty())
    return;

  const auto batch = m_pendingBatches.takeFirst();
  SS_ASSERT(!batch.isEmpty(), return);

  QStringList names;
  names.reserve(batch.size());
  for (const auto& name : batch)
    names.append(name.section(QChar(':'), 1));

  QUrl url(QStringLiteral("https://api.iconify.design/%1.json")
             .arg(batch.first().section(QChar(':'), 0, 0)));
  QUrlQuery params;
  params.addQueryItem(QStringLiteral("icons"), names.join(QChar(',')));
  url.setQuery(params);

  const int generation = m_generation;
  auto* reply          = m_manager.get(makeRequest(url));
  SS_ASSERT(reply != nullptr, return);

  m_inFlight = true;
  refreshLoadingState();
  connect(reply, &QNetworkReply::finished, this, [this, reply, generation, batch]() {
    onBatchFinished(reply, generation, batch);
  });
}

/**
 * @brief Publishes whether previews are still resolving, so the grid can spin per tile.
 */
void Misc::IconEngine::refreshLoadingState()
{
  const bool loading = m_inFlight || !m_pendingBatches.isEmpty();
  if (m_loading != loading) {
    m_loading = loading;
    Q_EMIT loadingPreviewsChanged();
  }
}

/**
 * @brief Handles a collection response: caches its icons, then paces the next batch. A 200 that
 *        did not carry the awaited icon counts as a failure, not a success: an absent prefix, a
 *        proxy page or a collection missing that key would otherwise leave m_busy latched with no
 *        message, since resolvePendingDownload() returns silently on a cache miss.
 */
void Misc::IconEngine::onBatchFinished(QNetworkReply* reply,
                                       int generation,
                                       const QStringList& batch)
{
  reply->deleteLater();
  if (generation != m_generation)
    return;

  m_inFlight = false;
  if (reply->error() == QNetworkReply::NoError) {
    m_attempt = 0;
    cacheIconSet(QJsonDocument::fromJson(reply->readAll()).object());
    refreshPreviews();
    resolvePendingDownload();
    if (!m_pendingDownload.isEmpty() && batch.contains(m_pendingDownload))
      failPendingDownload(tr("The icon service did not return %1.").arg(m_pendingDownload));
  } else if (m_attempt + 1 < kMaxAttempts) {
    ++m_attempt;
    scheduleBatch(batch, retryDelayMs(reply, m_attempt));
    return;
  } else {
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto why   = status == 429 ? tr("The icon service is rate-limiting requests, please "
                                          "wait a moment before searching again.")
                                     : reply->errorString();
    m_attempt        = 0;
    failPendingDownload(why);
    Q_EMIT searchFailed(why);
  }

  refreshLoadingState();
  QTimer::singleShot(kBatchSpacingMs, this, [this]() { requestNextBatch(); });
}

/**
 * @brief Re-queues @p batch at the head and retries it after @p delayMs.
 */
void Misc::IconEngine::scheduleBatch(const QStringList& batch, int delayMs)
{
  m_pendingBatches.prepend(batch);

  const int generation = m_generation;
  QTimer::singleShot(delayMs, this, [this, generation]() {
    if (generation == m_generation)
      requestNextBatch();
  });
}

/**
 * @brief Stores every icon of a collection response as a standalone SVG document.
 */
void Misc::IconEngine::cacheIconSet(const QJsonObject& root)
{
  const auto prefix = root.value(QStringLiteral("prefix")).toString();
  if (prefix.isEmpty())
    return;

  if (m_svgCache.size() > kMaxCachedIcons)
    m_svgCache.clear();

  const int defW   = root.value(QStringLiteral("width")).toInt(16);
  const int defH   = root.value(QStringLiteral("height")).toInt(16);
  const auto icons = root.value(QStringLiteral("icons")).toObject();

  for (auto it = icons.constBegin(); it != icons.constEnd(); ++it) {
    const auto svg = buildSvg(it.value().toObject(), defW, defH);
    if (!svg.isEmpty())
      m_svgCache.insert(QStringLiteral("%1:%2").arg(prefix, it.key()), svg);
  }
}

//--------------------------------------------------------------------------------------------------
// Download completion helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Emits the awaited icon once its collection response cached it.
 */
void Misc::IconEngine::resolvePendingDownload()
{
  if (m_pendingDownload.isEmpty())
    return;

  const auto it = m_svgCache.constFind(m_pendingDownload);
  if (it == m_svgCache.constEnd())
    return;

  const auto svg = *it;
  m_pendingDownload.clear();
  m_busy = false;

  Q_EMIT busyChanged();
  Q_EMIT iconDownloaded(toDataUri(svg));
}

/**
 * @brief Releases the busy state when the awaited icon could not be fetched.
 */
void Misc::IconEngine::failPendingDownload(const QString& error)
{
  if (m_pendingDownload.isEmpty())
    return;

  m_pendingDownload.clear();
  m_busy = false;

  Q_EMIT busyChanged();
  Q_EMIT iconDownloadFailed(error);
}

//--------------------------------------------------------------------------------------------------
// SVG assembly helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wraps an Iconify icon body into a standalone SVG document.
 */
QString Misc::IconEngine::buildSvg(const QJsonObject& icon, int defW, int defH)
{
  const auto body = icon.value(QStringLiteral("body")).toString();
  if (body.isEmpty())
    return {};

  const int left   = icon.value(QStringLiteral("left")).toInt(0);
  const int top    = icon.value(QStringLiteral("top")).toInt(0);
  const int width  = icon.value(QStringLiteral("width")).toInt(defW);
  const int height = icon.value(QStringLiteral("height")).toInt(defH);

  return QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1\" height=\"%2\" "
                        "viewBox=\"%3 %4 %5 %6\">%7</svg>")
    .arg(width)
    .arg(height)
    .arg(left)
    .arg(top)
    .arg(width)
    .arg(height)
    .arg(body);
}

/**
 * @brief Builds an Iconify request that identifies Serial Studio to the API.
 */
QNetworkRequest Misc::IconEngine::makeRequest(const QUrl& url)
{
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    QStringLiteral("SerialStudio/%1").arg(QStringLiteral(PROJECT_VERSION)));
  return request;
}

/**
 * @brief Encodes an SVG document as the base64 data URI stored in projects.
 */
QString Misc::IconEngine::toDataUri(const QString& svg)
{
  const auto base64 = svg.toUtf8().toBase64();
  return QStringLiteral("data:image/svg+xml;base64,%1").arg(QString::fromLatin1(base64));
}

/**
 * @brief Encodes an SVG document as an actionicon provider URL for QML previews.
 */
QString Misc::IconEngine::toPreviewSource(const QString& svg)
{
  const auto base64 = svg.toUtf8().toBase64();
  return QStringLiteral("image://actionicon/%1").arg(QString::fromLatin1(base64));
}

/**
 * @brief Returns the backoff for a failed batch, honouring a sane Retry-After header.
 */
int Misc::IconEngine::retryDelayMs(const QNetworkReply* reply, int attempt)
{
  bool ok           = false;
  const int seconds = reply->rawHeader("Retry-After").toInt(&ok);
  if (ok && seconds > 0 && seconds <= 30)
    return seconds * 1000;

  return 500 * (1 << qBound(1, attempt, 4));
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
 */
QImage Misc::ActionIconProvider::requestImage(const QString& id,
                                              QSize* size,
                                              const QSize& requestedSize)
{
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

  const int w = requestedSize.width() > 0 ? requestedSize.width() : 64;
  const int h = requestedSize.height() > 0 ? requestedSize.height() : 64;

  QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);

  QPainter painter(&image);
  renderer.render(&painter);
  painter.end();

  if (size)
    *size = image.size();

  return image;
}
