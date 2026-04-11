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

#include "Misc/HelpCenter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "Misc/ThemeManager.h"

//--------------------------------------------------------------------------------------------------
// GitHub URL helpers
//--------------------------------------------------------------------------------------------------

static const QString kGitHubBranch = QStringLiteral("master");
static const QString kGitHubRepo   = QStringLiteral("Serial-Studio/Serial-Studio");

static const QString kBase = QStringLiteral("https://raw.githubusercontent.com/%1/%2/doc/help/")
                               .arg(kGitHubRepo, kGitHubBranch);

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the HelpCenter singleton.
 */
Misc::HelpCenter::HelpCenter() : m_loading(false), m_currentIndex(-1), m_pendingPreloads(0)
{
  // Build initial theme colors JSON and react to theme changes
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &HelpCenter::onThemeChanged);
}

/**
 * @brief Returns the singleton instance of the HelpCenter class.
 */
Misc::HelpCenter& Misc::HelpCenter::instance()
{
  static HelpCenter singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a network operation is in progress.
 */
bool Misc::HelpCenter::loading() const noexcept
{
  return m_loading;
}

/**
 * @brief Returns the number of currently visible (filtered) pages.
 */
int Misc::HelpCenter::count() const noexcept
{
  return m_filteredPages.count();
}

/**
 * @brief Returns the index of the currently selected page, or -1.
 */
int Misc::HelpCenter::currentIndex() const noexcept
{
  return m_currentIndex;
}

/**
 * @brief Returns the current search filter text.
 */
const QString& Misc::HelpCenter::searchFilter() const noexcept
{
  return m_searchFilter;
}

/**
 * @brief Returns the raw markdown content for the selected page.
 */
const QString& Misc::HelpCenter::pageContent() const noexcept
{
  return m_pageContent;
}

/**
 * @brief Returns the ID of the currently selected page.
 */
QString Misc::HelpCenter::pageId() const
{
  if (m_currentIndex >= 0 && m_currentIndex < m_filteredPages.count())
    return m_filteredPages.at(m_currentIndex).toMap().value("id").toString();

  return {};
}

/**
 * @brief Returns the title of the currently selected page.
 */
QString Misc::HelpCenter::pageTitle() const
{
  if (m_currentIndex >= 0 && m_currentIndex < m_filteredPages.count())
    return m_filteredPages.at(m_currentIndex).toMap().value("title").toString();

  return {};
}

/**
 * @brief Returns the filtered pages list for the sidebar.
 */
const QVariantList& Misc::HelpCenter::pages() const noexcept
{
  return m_filteredPages;
}

/**
 * @brief Returns a JSON string of theme colors for the WebEngineView.
 */
const QString& Misc::HelpCenter::themeColors() const noexcept
{
  return m_themeColors;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a page by index and triggers content fetching.
 */
void Misc::HelpCenter::setCurrentIndex(int index)
{
  // Guard against redundant selection
  if (m_currentIndex == index)
    return;

  m_currentIndex = index;
  Q_EMIT currentIndexChanged();

  // Clear previous content
  m_pageContent.clear();
  Q_EMIT pageContentChanged();

  // Fetch content for the newly selected page
  if (index >= 0 && index < m_filteredPages.count())
    fetchPage(index);
}

/**
 * @brief Sets the search filter and re-applies filtering to the pages list.
 */
void Misc::HelpCenter::setSearchFilter(const QString& filter)
{
  if (m_searchFilter == filter)
    return;

  m_searchFilter = filter;
  Q_EMIT searchFilterChanged();
  applyFilter();
}

/**
 * @brief Navigates to an internal help page by matching a link string against
 *        page IDs, file names, or titles. Returns true if the page was found.
 */
bool Misc::HelpCenter::navigateToPage(const QString& link)
{
  // Normalize the link by stripping fragment and extension
  auto normalized    = link.trimmed();
  const auto hashIdx = normalized.indexOf('#');
  if (hashIdx >= 0)
    normalized = normalized.left(hashIdx);

  if (normalized.endsWith(".md", Qt::CaseInsensitive))
    normalized.chop(3);

  if (normalized.isEmpty())
    return false;

  // Search in filtered pages first, then all pages
  for (int i = 0; i < m_filteredPages.count(); ++i) {
    const auto page  = m_filteredPages.at(i).toMap();
    const auto id    = page.value("id").toString();
    const auto title = page.value("title").toString();
    auto file        = page.value("file").toString();
    if (file.endsWith(".md", Qt::CaseInsensitive))
      file.chop(3);

    if (id.compare(normalized, Qt::CaseInsensitive) == 0
        || file.compare(normalized, Qt::CaseInsensitive) == 0
        || title.compare(normalized, Qt::CaseInsensitive) == 0) {
      setCurrentIndex(i);
      return true;
    }
  }

  // If filter is active, clear it and try again in the full list
  if (!m_searchFilter.isEmpty()) {
    setSearchFilter(QString());
    return navigateToPage(link);
  }

  return false;
}

/**
 * @brief Opens the Help Center and navigates to the given @a pageId.
 *        If the manifest has not been loaded yet, the page ID is stored
 *        and navigation occurs after the manifest arrives.
 */
void Misc::HelpCenter::showPage(const QString& pageId)
{
  // Clear search filter so all pages are visible
  if (!m_searchFilter.isEmpty())
    setSearchFilter(QString());

  // Manifest already loaded, navigate immediately
  if (!m_allPages.isEmpty()) {
    if (!pageId.isEmpty())
      navigateToPage(pageId);

    return;
  }

  // Store pending page and fetch manifest
  m_pendingPageId = pageId;
  fetchManifest();
}

//--------------------------------------------------------------------------------------------------
// Network reply handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the help.json manifest fetch response.
 */
void Misc::HelpCenter::onManifestReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  // Parse the manifest
  if (reply->error() == QNetworkReply::NoError) {
    const auto doc = QJsonDocument::fromJson(reply->readAll());
    m_allPages     = doc.array();
    applyFilter();

    // Navigate to pending page or auto-select first
    if (!m_pendingPageId.isEmpty()) {
      navigateToPage(m_pendingPageId);
      m_pendingPageId.clear();
    }

    else if (!m_filteredPages.isEmpty())
      setCurrentIndex(0);

    // Preload all page contents for full-text search
    preloadAllPages();
  }

  // Clear loading state
  m_loading = false;
  Q_EMIT loadingChanged();
}

/**
 * @brief Handles the page content fetch response and caches it.
 */
void Misc::HelpCenter::onPageReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto raw = QString::fromUtf8(reply->readAll());

    // Store in memory
    const auto id = reply->property("pageId").toString();
    if (!id.isEmpty())
      m_pageContents.insert(id, raw);

    // Expose raw markdown to QML
    m_pageContent = raw;
  }

  else
    m_pageContent = tr("Failed to load page: %1").arg(reply->errorString());

  // Reset loading flag
  m_loading = false;
  Q_EMIT loadingChanged();
  Q_EMIT pageContentChanged();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fetches the help.json manifest from GitHub. No-op if already
 *        loaded or currently loading.
 */
void Misc::HelpCenter::fetchManifest()
{
  if (m_loading || !m_allPages.isEmpty())
    return;

  m_loading = true;
  Q_EMIT loadingChanged();

  const auto url = QUrl(kBase + "help.json");
  auto* reply    = m_nam.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished, this, &HelpCenter::onManifestReply);
}

/**
 * @brief Applies the current search filter to rebuild the filtered pages list.
 *        Searches in title, section, and preloaded page content.
 */
void Misc::HelpCenter::applyFilter()
{
  m_filteredPages.clear();

  for (const auto& entry : std::as_const(m_allPages)) {
    const auto obj = entry.toObject();
    if (!m_searchFilter.isEmpty()) {
      const auto id      = obj.value("id").toString();
      const auto title   = obj.value("title").toString();
      const auto section = obj.value("section").toString();

      // Search in title and section
      bool matches = title.contains(m_searchFilter, Qt::CaseInsensitive)
                  || section.contains(m_searchFilter, Qt::CaseInsensitive);

      // Search in preloaded content
      if (!matches) {
        const auto it = m_pageContents.constFind(id);
        if (it != m_pageContents.constEnd())
          matches = it->contains(m_searchFilter, Qt::CaseInsensitive);
      }

      if (!matches)
        continue;
    }

    m_filteredPages.append(obj.toVariantMap());
  }

  Q_EMIT pagesChanged();
}

/**
 * @brief Fetches the content for the page at the given index. Uses preloaded
 *        content or network fetch.
 */
void Misc::HelpCenter::fetchPage(int index)
{
  if (index < 0 || index >= m_filteredPages.count())
    return;

  const auto page = m_filteredPages.at(index).toMap();
  const auto id   = page.value("id").toString();
  const auto file = page.value("file").toString();

  // Check in-memory content first
  const auto it = m_pageContents.constFind(id);
  if (it != m_pageContents.constEnd()) {
    m_pageContent = *it;
    Q_EMIT pageContentChanged();
    return;
  }

  // Fetch from network
  m_loading = true;
  Q_EMIT loadingChanged();

  auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(file));
  auto url     = QUrl::fromEncoded((kBase + encoded).toUtf8());
  auto* reply  = m_nam.get(QNetworkRequest(url));
  reply->setProperty("pageId", id);
  connect(reply, &QNetworkReply::finished, this, &HelpCenter::onPageReply);
}

/**
 * @brief Kicks off background preloading of all page contents for full-text
 *        search. Fetches from disk cache or network.
 */
void Misc::HelpCenter::preloadAllPages()
{
  // Fetch all pages not yet in cache for full-text search
  m_pendingPreloads = 0;

  for (const auto& entry : std::as_const(m_allPages)) {
    const auto obj  = entry.toObject();
    const auto id   = obj.value("id").toString();
    const auto file = obj.value("file").toString();

    if (m_pageContents.contains(id))
      continue;

    // Fetch from network
    ++m_pendingPreloads;
    auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(file));
    auto url     = QUrl::fromEncoded((kBase + encoded).toUtf8());
    auto* reply  = m_nam.get(QNetworkRequest(url));
    reply->setProperty("pageId", id);
    connect(reply, &QNetworkReply::finished, this, &HelpCenter::onPreloadReply);
  }
}

/**
 * @brief Handles a preload reply, stores content and re-applies the filter
 *        once all pages are loaded so content search is available.
 */
void Misc::HelpCenter::onPreloadReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto id  = reply->property("pageId").toString();
    const auto raw = QString::fromUtf8(reply->readAll());
    if (!id.isEmpty())
      m_pageContents.insert(id, raw);
  }

  // When all preloads finish, re-apply filter so content search is available
  --m_pendingPreloads;
  if (m_pendingPreloads <= 0 && !m_searchFilter.isEmpty())
    applyFilter();
}

//--------------------------------------------------------------------------------------------------
// Theme integration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the theme colors JSON string from the current ThemeManager
 *        colors and emits themeColorsChanged so the WebEngineView can update.
 */
void Misc::HelpCenter::onThemeChanged()
{
  static const auto* t = &Misc::ThemeManager::instance();
  const auto& colors   = t->colors();

  // Determine if this is a dark theme by checking background luminance
  const auto bgHex = colors.value(QStringLiteral("groupbox_background")).toString();
  const QColor bgCol(bgHex);
  const bool isDark = bgCol.isValid() && bgCol.lightnessF() < 0.5;

  // Build JSON object with the colors the WebView needs
  QJsonObject obj;
  obj[QStringLiteral("text")]      = colors.value(QStringLiteral("text")).toString();
  obj[QStringLiteral("bg")]        = colors.value(QStringLiteral("groupbox_background")).toString();
  obj[QStringLiteral("accent")]    = colors.value(QStringLiteral("accent")).toString();
  obj[QStringLiteral("border")]    = colors.value(QStringLiteral("groupbox_border")).toString();
  obj[QStringLiteral("base")]      = colors.value(QStringLiteral("base")).toString();
  obj[QStringLiteral("codeBg")]    = colors.value(QStringLiteral("base")).toString();
  obj[QStringLiteral("highlight")] = colors.value(QStringLiteral("highlight")).toString();
  obj[QStringLiteral("isDark")]    = isDark;

  m_themeColors = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
  Q_EMIT themeColorsChanged();
}
