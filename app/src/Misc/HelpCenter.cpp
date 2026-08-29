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

#include "Misc/HelpCenter.h"

#include <algorithm>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "AppInfo.h"
#include "Misc/ThemeManager.h"

//--------------------------------------------------------------------------------------------------
// GitHub URL helpers
//--------------------------------------------------------------------------------------------------

static const QString kDevSlug              = QStringLiteral("dev");
static const QString kDevRef               = QStringLiteral("master");
static const QString kHelpCenterGitHubRepo = QStringLiteral("Serial-Studio/Serial-Studio");
static const QString kSite                 = QStringLiteral("https://serial-studio.com");
static const QString kVersionsUrl          = kSite + QStringLiteral("/help/versions.json");

/**
 * @brief Returns the raw.githubusercontent.com base URL of doc/help at @a ref.
 */
static QString baseUrl(const QString& ref)
{
  return QStringLiteral("https://raw.githubusercontent.com/%1/%2/doc/help/")
    .arg(kHelpCenterGitHubRepo, ref);
}

/**
 * @brief Packs a "v4.0.3" or "4.0.3" string into a single comparable integer.
 */
static int versionKey(const QString& version)
{
  const auto trimmed = version.startsWith(QLatin1Char('v')) ? version.mid(1) : version;
  const auto parts   = trimmed.split(QLatin1Char('.'));

  int key = 0;
  for (int i = 0; i < 3; ++i) {
    const auto part = i < parts.count() ? parts.at(i) : QString();
    key             = key * 1000 + std::clamp(part.section(QLatin1Char('-'), 0, 0).toInt(), 0, 999);
  }

  return key;
}

/**
 * @brief Returns the position of @a slug in a version list, or -1.
 */
static int indexOfSlug(const QVariantList& versions, const QString& slug)
{
  for (int i = 0; i < versions.count(); ++i)
    if (versions.at(i).toMap().value(QStringLiteral("slug")).toString() == slug)
      return i;

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the HelpCenter singleton.
 */
Misc::HelpCenter::HelpCenter()
  : m_loading(false)
  , m_epoch(0)
  , m_currentIndex(-1)
  , m_versionIndex(-1)
  , m_defaultVersion(-1)
  , m_pendingPreloads(0)
  , m_theme(&Misc::ThemeManager::instance())
{
  m_nam.setTransferTimeout(15 * 1000);

  onThemeChanged();
  connect(m_theme, &Misc::ThemeManager::themeChanged, this, &HelpCenter::onThemeChanged);
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

/**
 * @brief Returns the index of the documentation version being displayed.
 */
int Misc::HelpCenter::versionIndex() const noexcept
{
  return m_versionIndex;
}

/**
 * @brief Returns the list of documentation versions offered to the user.
 */
const QVariantList& Misc::HelpCenter::versions() const noexcept
{
  return m_versions;
}

/**
 * @brief Returns the human-readable label of the current documentation version.
 */
QString Misc::HelpCenter::versionLabel() const
{
  return currentVersion().value(QStringLiteral("label")).toString();
}

/**
 * @brief Returns a warning shown when the user browses documentation that does
 *        not correspond to the running application, or an empty string.
 */
QString Misc::HelpCenter::versionNotice() const
{
  if (m_versionIndex < 0 || m_versionIndex == m_defaultVersion)
    return {};

  return tr("Showing documentation for %1; this copy of Serial Studio is version %2.")
    .arg(versionLabel(), QStringLiteral(APP_VERSION));
}

/**
 * @brief Returns the serial-studio.com address of the current page and version.
 */
QString Misc::HelpCenter::onlineUrl() const
{
  auto path = currentVersion().value(QStringLiteral("path")).toString();
  if (path.isEmpty())
    path = QStringLiteral("/help");

  const auto id = pageId();
  if (!id.isEmpty())
    return kSite + path + QLatin1Char('/') + id;

  return kSite + path;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a page by index and triggers content fetching.
 */
void Misc::HelpCenter::setCurrentIndex(int index)
{
  if (m_currentIndex == index)
    return;

  m_currentIndex = index;
  Q_EMIT currentIndexChanged();

  m_pageContent.clear();
  Q_EMIT pageContentChanged();

  if (index >= 0 && index < m_filteredPages.count())
    fetchPage(index);
}

/**
 * @brief Switches the documentation version, dropping every cached page and
 *        re-fetching the manifest for the newly selected git ref.
 */
void Misc::HelpCenter::setVersionIndex(int index)
{
  if (index == m_versionIndex || index < 0 || index >= m_versions.count())
    return;

  const auto page = pageId();
  ++m_epoch;
  m_versionIndex = index;
  Q_EMIT versionIndexChanged();

  m_loading         = false;
  m_currentIndex    = -1;
  m_pendingPreloads = 0;
  m_allPages        = QJsonArray();
  m_pendingPageId   = page;
  m_pageContents.clear();
  m_filteredPages.clear();
  m_pageContent.clear();

  Q_EMIT pagesChanged();
  Q_EMIT loadingChanged();
  Q_EMIT currentIndexChanged();
  Q_EMIT pageContentChanged();

  fetchManifest();
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
  auto normalized    = link.trimmed();
  const auto hashIdx = normalized.indexOf('#');
  if (hashIdx >= 0)
    normalized = normalized.left(hashIdx);

  if (normalized.endsWith(".md", Qt::CaseInsensitive))
    normalized.chop(3);

  if (normalized.isEmpty())
    return false;

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

  if (!m_searchFilter.isEmpty()) {
    setSearchFilter(QString());
    return navigateToPage(link);
  }

  return false;
}

/**
 * @brief Opens the Help Center and navigates to the given @a pageId. If the manifest has not been
 * loaded yet, the page ID is stored and navigation occurs after the manifest arrives.
 */
void Misc::HelpCenter::showPage(const QString& pageId)
{
  if (!m_searchFilter.isEmpty())
    setSearchFilter(QString());

  if (!m_allPages.isEmpty()) {
    if (!pageId.isEmpty())
      navigateToPage(pageId);

    return;
  }

  m_pendingPageId = pageId;
  fetchManifest();
}

//--------------------------------------------------------------------------------------------------
// Network reply handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fetches the documentation version manifest published by
 *        serial-studio.com. No-op while another request is in flight.
 */
void Misc::HelpCenter::fetchVersions()
{
  if (m_loading)
    return;

  m_loading = true;
  Q_EMIT loadingChanged();

  auto* reply = m_nam.get(QNetworkRequest(QUrl(kVersionsUrl)));
  reply->setProperty("epoch", m_epoch);
  connect(reply, &QNetworkReply::finished, this, &HelpCenter::onVersionsReply);
}

/**
 * @brief Handles the versions.json fetch response and continues with the
 *        manifest of the resolved version. A failed fetch falls back to a
 *        single "latest" entry tracking the master branch.
 */
void Misc::HelpCenter::onVersionsReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();
  if (reply->property("epoch").toInt() != m_epoch)
    return;

  QJsonArray array;
  if (reply->error() == QNetworkReply::NoError)
    array = QJsonDocument::fromJson(reply->readAll()).array();

  applyVersions(array);

  m_loading = false;
  Q_EMIT loadingChanged();

  fetchManifest();
}

/**
 * @brief Handles the help.json manifest fetch response.
 */
void Misc::HelpCenter::onManifestReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();
  if (reply->property("epoch").toInt() != m_epoch)
    return;

  if (reply->error() == QNetworkReply::NoError) {
    const auto doc = QJsonDocument::fromJson(reply->readAll());
    m_allPages     = doc.array();
    applyFilter();

    const auto pending = m_pendingPageId;
    m_pendingPageId.clear();

    const bool restored = !pending.isEmpty() && navigateToPage(pending);
    if (!restored && !m_filteredPages.isEmpty())
      setCurrentIndex(0);

    preloadAllPages();
  }

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
  if (reply->property("epoch").toInt() != m_epoch)
    return;

  if (reply->error() == QNetworkReply::NoError) {
    const auto raw = QString::fromUtf8(reply->readAll());

    const auto id = reply->property("pageId").toString();
    if (!id.isEmpty())
      m_pageContents.insert(id, raw);

    m_pageContent = raw;
  }

  else
    m_pageContent = tr("Failed to load page: %1").arg(reply->errorString());

  m_loading = false;
  Q_EMIT loadingChanged();
  Q_EMIT pageContentChanged();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fetches the help.json manifest of the selected documentation version.
 *        No-op if already loaded or currently loading.
 */
void Misc::HelpCenter::fetchManifest()
{
  if (m_loading || !m_allPages.isEmpty())
    return;

  if (m_versions.isEmpty()) {
    fetchVersions();
    return;
  }

  m_loading = true;
  Q_EMIT loadingChanged();

  auto* reply = m_nam.get(QNetworkRequest(contentUrl(QStringLiteral("help.json"))));
  reply->setProperty("epoch", m_epoch);
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
    if (!m_searchFilter.isEmpty() && !pageMatchesFilter(obj))
      continue;

    m_filteredPages.append(obj.toVariantMap());
  }

  Q_EMIT pagesChanged();
}

/**
 * @brief Returns true when a page entry matches the active search filter.
 */
bool Misc::HelpCenter::pageMatchesFilter(const QJsonObject& obj) const
{
  const auto title   = obj.value("title").toString();
  const auto section = obj.value("section").toString();
  if (title.contains(m_searchFilter, Qt::CaseInsensitive)
      || section.contains(m_searchFilter, Qt::CaseInsensitive))
    return true;

  const auto id = obj.value("id").toString();
  const auto it = m_pageContents.constFind(id);
  if (it == m_pageContents.constEnd())
    return false;

  return it->contains(m_searchFilter, Qt::CaseInsensitive);
}

/**
 * @brief Rebuilds the version list from the fetched manifest and selects the
 *        version that matches the running application. An empty or unreadable
 *        manifest degrades to a single entry tracking the master branch.
 */
void Misc::HelpCenter::applyVersions(const QJsonArray& array)
{
  m_versions.clear();

  for (const auto& entry : array) {
    const auto obj = entry.toObject();
    if (!obj.value(QStringLiteral("ref")).toString().isEmpty())
      m_versions.append(obj.toVariantMap());
  }

  if (m_versions.isEmpty()) {
    QVariantMap fallback;
    fallback.insert(QStringLiteral("slug"), kDevSlug);
    fallback.insert(QStringLiteral("label"), tr("Latest"));
    fallback.insert(QStringLiteral("ref"), kDevRef);
    fallback.insert(QStringLiteral("path"), QStringLiteral("/help"));
    m_versions.append(fallback);
  }

  m_defaultVersion = resolveDefaultVersion();
  m_versionIndex   = m_defaultVersion;

  Q_EMIT versionsChanged();
  Q_EMIT versionIndexChanged();
}

/**
 * @brief Returns the version the running application should read: its own tag
 *        when documented, the development branch when the application is newer
 *        than every tag, otherwise the manifest's default.
 */
int Misc::HelpCenter::resolveDefaultVersion() const
{
  const auto tag = QStringLiteral("v" APP_VERSION);

  int marked = -1;
  int newest = 0;
  for (int i = 0; i < m_versions.count(); ++i) {
    const auto version = m_versions.at(i).toMap();
    const auto slug    = version.value(QStringLiteral("slug")).toString();
    if (slug.compare(tag, Qt::CaseInsensitive) == 0)
      return i;

    if (version.value(QStringLiteral("default")).toBool())
      marked = i;

    newest = std::max(newest, versionKey(slug));
  }

  const auto dev = indexOfSlug(m_versions, kDevSlug);
  if (dev >= 0 && versionKey(QStringLiteral(APP_VERSION)) > newest)
    return dev;

  return marked >= 0 ? marked : (m_versions.isEmpty() ? -1 : 0);
}

/**
 * @brief Returns the manifest entry of the version being displayed.
 */
QVariantMap Misc::HelpCenter::currentVersion() const
{
  if (m_versionIndex >= 0 && m_versionIndex < m_versions.count())
    return m_versions.at(m_versionIndex).toMap();

  return {};
}

/**
 * @brief Returns the download URL of @a file within the selected version.
 */
QUrl Misc::HelpCenter::contentUrl(const QString& file) const
{
  auto ref = currentVersion().value(QStringLiteral("ref")).toString();
  if (ref.isEmpty())
    ref = kDevRef;

  const auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(file));
  return QUrl::fromEncoded((baseUrl(ref) + encoded).toUtf8());
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

  const auto it = m_pageContents.constFind(id);
  if (it != m_pageContents.constEnd()) {
    m_pageContent = *it;
    Q_EMIT pageContentChanged();
    return;
  }

  m_loading = true;
  Q_EMIT loadingChanged();

  auto* reply = m_nam.get(QNetworkRequest(contentUrl(file)));
  reply->setProperty("pageId", id);
  reply->setProperty("epoch", m_epoch);
  connect(reply, &QNetworkReply::finished, this, &HelpCenter::onPageReply);
}

/**
 * @brief Kicks off background preloading of all page contents for full-text
 *        search. Fetches from disk cache or network.
 */
void Misc::HelpCenter::preloadAllPages()
{
  m_pendingPreloads = 0;

  for (const auto& entry : std::as_const(m_allPages)) {
    const auto obj  = entry.toObject();
    const auto id   = obj.value("id").toString();
    const auto file = obj.value("file").toString();

    if (m_pageContents.contains(id))
      continue;

    ++m_pendingPreloads;
    auto* reply = m_nam.get(QNetworkRequest(contentUrl(file)));
    reply->setProperty("pageId", id);
    reply->setProperty("epoch", m_epoch);
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
  if (reply->property("epoch").toInt() != m_epoch)
    return;

  if (reply->error() == QNetworkReply::NoError) {
    const auto id  = reply->property("pageId").toString();
    const auto raw = QString::fromUtf8(reply->readAll());
    if (!id.isEmpty())
      m_pageContents.insert(id, raw);
  }

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
  const auto& colors = m_theme->colors();

  const auto bgHex = colors.value(QStringLiteral("groupbox_background")).toString();
  const QColor bgCol(bgHex);
  const bool isDark = bgCol.isValid() && bgCol.lightnessF() < 0.5;

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
