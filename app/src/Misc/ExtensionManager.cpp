/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "Misc/ExtensionManager.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTimer>

#include "API/Server.h"
#include "Misc/JsonValidator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Default repository URL
//--------------------------------------------------------------------------------------------------

static const QString kDefaultRepoUrl =
  QStringLiteral("https://raw.githubusercontent.com/serial-studio/extensions/main/manifest.json");

/**
 * @brief Picks the best matching platform override map for the given platform key.
 */
static QVariantMap selectPlatformOverride(const QVariantMap& platforms, const QString& platformKey)
{
  if (platforms.isEmpty())
    return {};

  if (platforms.contains(platformKey))
    return platforms.value(platformKey).toMap();

  const auto os       = platformKey.left(platformKey.indexOf('/'));
  const auto wildcard = os + QStringLiteral("/*");
  if (platforms.contains(wildcard))
    return platforms.value(wildcard).toMap();

  if (platforms.contains(QStringLiteral("*")))
    return platforms.value(QStringLiteral("*")).toMap();

  return {};
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ExtensionManager singleton.
 */
Misc::ExtensionManager::ExtensionManager()
  : m_loading(false)
  , m_selectedIndex(-1)
  , m_downloadProgress(0)
  , m_pendingDownloads(0)
  , m_totalDownloads(0)
  , m_pendingManifests(0)
  , m_pendingExtensionMetas(0)
  , m_dashboardWasAvailable(false)
{
  m_nam.setTransferTimeout(15 * 1000);

  const auto saved = m_settings.value("ExtensionRepositories").toStringList();
  if (saved.isEmpty())
    m_repositories.append(kDefaultRepoUrl);
  else
    m_repositories = saved;

  loadInstalledManifest();
  applyFilter();
  rebuildInstalledPlugins();
}

/**
 * @brief Returns the singleton instance of the ExtensionManager class.
 */
Misc::ExtensionManager& Misc::ExtensionManager::instance()
{
  static ExtensionManager singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a network operation is in progress.
 */
bool Misc::ExtensionManager::loading() const noexcept
{
  return m_loading;
}

/**
 * @brief Returns the number of currently visible (filtered) extensions.
 */
int Misc::ExtensionManager::count() const noexcept
{
  return m_filteredExtensions.count();
}

/**
 * @brief Returns the index of the currently selected addon, or -1.
 */
int Misc::ExtensionManager::selectedIndex() const noexcept
{
  return m_selectedIndex;
}

/**
 * @brief Returns the current download progress (0.0 to 1.0).
 */
float Misc::ExtensionManager::downloadProgress() const noexcept
{
  return m_downloadProgress;
}

/**
 * @brief Returns the current search filter text.
 */
const QString& Misc::ExtensionManager::searchFilter() const noexcept
{
  return m_searchFilter;
}

/**
 * @brief Returns the current category filter.
 */
const QString& Misc::ExtensionManager::filterCategory() const noexcept
{
  return m_filterCategory;
}

/**
 * @brief Returns the current type filter.
 */
const QString& Misc::ExtensionManager::filterType() const noexcept
{
  return m_filterType;
}

/**
 * @brief Returns the list of configured repository URLs.
 */
const QStringList& Misc::ExtensionManager::repositories() const noexcept
{
  return m_repositories;
}

/**
 * @brief Returns the filtered extensions list for the grid view.
 */
const QVariantList& Misc::ExtensionManager::extensions() const noexcept
{
  return m_filteredExtensions;
}

/**
 * @brief Returns the README markdown content for the selected extension.
 */
const QString& Misc::ExtensionManager::selectedReadme() const noexcept
{
  return m_selectedReadme;
}

/**
 * @brief Returns the metadata map for the currently selected extension.
 */
QVariantMap Misc::ExtensionManager::selectedExtension() const
{
  if (m_selectedIndex >= 0 && m_selectedIndex < m_filteredExtensions.count())
    return m_filteredExtensions.at(m_selectedIndex).toMap();

  return {};
}

/**
 * @brief Returns whether the given URL points to a local directory.
 */
bool Misc::ExtensionManager::isLocalRepo(const QString& url) const
{
  return url.startsWith('/') || url.startsWith("file://");
}

/**
 * @brief Returns the current platform key (e.g. "darwin/arm64") for QML display.
 */
QString Misc::ExtensionManager::platformKey() const
{
  return currentPlatformKey();
}

/**
 * @brief Returns the list of supported extension types for filtering.
 */
QStringList Misc::ExtensionManager::extensionTypes() const
{
  return {
    QStringLiteral("All"),
    QStringLiteral("theme"),
    QStringLiteral("frame-parser"),
    QStringLiteral("project-template"),
    QStringLiteral("plugin"),
  };
}

/**
 * @brief Returns a user-friendly display name for the given extension type ID.
 */
QString Misc::ExtensionManager::friendlyTypeName(const QString& type) const
{
  if (type == QStringLiteral("theme"))
    return tr("Theme");

  if (type == QStringLiteral("frame-parser"))
    return tr("Frame Parser");

  if (type == QStringLiteral("project-template"))
    return tr("Project Template");

  if (type == QStringLiteral("plugin"))
    return tr("Plugin");

  if (type == QStringLiteral("All"))
    return tr("All Types");

  return type;
}

/**
 * @brief Returns whether the extension with the given ID is installed locally.
 */
bool Misc::ExtensionManager::isInstalled(const QString& id) const
{
  return m_installedExtensions.contains(id);
}

/**
 * @brief Returns whether a newer version is available for the given addon.
 */
bool Misc::ExtensionManager::hasUpdate(const QString& id) const
{
  if (!isInstalled(id))
    return false;

  const auto installed = m_installedExtensions.value(id).toObject();
  const auto localVer  = installed.value("version").toString();

  for (const auto& entry : std::as_const(m_allExtensions)) {
    const auto obj = entry.toObject();
    if (obj.value("id").toString() == id)
      return obj.value("version").toString() != localVer;
  }

  return false;
}

/**
 * @brief Returns the installed version string for the given addon, or empty.
 */
QString Misc::ExtensionManager::installedVersion(const QString& id) const
{
  if (!isInstalled(id))
    return {};

  return m_installedExtensions.value(id).toObject().value("version").toString();
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects an extension by index in the filtered list.
 */
void Misc::ExtensionManager::setSelectedIndex(int index)
{
  if (m_selectedIndex == index)
    return;

  m_selectedIndex = index;
  Q_EMIT selectedIndexChanged();

  m_selectedReadme.clear();
  Q_EMIT selectedReadmeChanged();

  if (index >= 0 && index < m_filteredExtensions.count()) {
    const auto ext   = m_filteredExtensions.at(index).toMap();
    const auto base  = ext.value("_repoBase").toString();
    const auto local = ext.value("_isLocal").toBool();

    if (local && !base.isEmpty()) {
      QFile file(base + "README.md");
      if (file.open(QIODevice::ReadOnly)) {
        m_selectedReadme = QString::fromUtf8(file.readAll());
        Q_EMIT selectedReadmeChanged();
      }
    }

    else if (!base.isEmpty()) {
      auto* reply = m_nam.get(QNetworkRequest(QUrl(base + "README.md")));
      m_activeReplies.insert(reply);
      connect(reply, &QNetworkReply::finished, this, &ExtensionManager::onReadmeReply);
    }
  }
}

/**
 * @brief Sets the search filter and re-applies filtering.
 */
void Misc::ExtensionManager::setSearchFilter(const QString& filter)
{
  if (m_searchFilter == filter)
    return;

  m_searchFilter = filter;
  Q_EMIT searchFilterChanged();
  applyFilter();
}

/**
 * @brief Sets the category filter and re-applies filtering.
 */
void Misc::ExtensionManager::setFilterCategory(const QString& category)
{
  if (m_filterCategory == category)
    return;

  m_filterCategory = category;
  Q_EMIT filterCategoryChanged();
  applyFilter();
}

/**
 * @brief Sets the extension type filter and re-applies filtering.
 */
void Misc::ExtensionManager::setFilterType(const QString& type)
{
  if (m_filterType == type)
    return;

  m_filterType = type;
  Q_EMIT filterTypeChanged();
  applyFilter();
}

//--------------------------------------------------------------------------------------------------
// Repository management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fetches manifest.json from each configured repository and merges
 *        the results into the extension catalog.
 */
void Misc::ExtensionManager::refreshRepositories()
{
  if (m_loading)
    return;

  for (auto* reply : std::as_const(m_activeReplies)) {
    if (reply) {
      reply->disconnect(this);
      reply->abort();
      reply->deleteLater();
    }
  }

  m_activeReplies.clear();

  m_allExtensions = QJsonArray();
  m_filteredExtensions.clear();
  m_selectedIndex         = -1;
  m_pendingManifests      = 0;
  m_pendingExtensionMetas = 0;
  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExtensionsChanged();

  QStringList remoteRepos;
  QStringList localRepos;
  for (const auto& repo : std::as_const(m_repositories))
    if (isLocalRepo(repo))
      localRepos.append(repo);
    else
      remoteRepos.append(repo);

  for (const auto& localPath : std::as_const(localRepos))
    loadLocalManifest(localPath);

  m_pendingManifests = remoteRepos.count();
  if (m_pendingManifests > 0) {
    m_loading = true;
    Q_EMIT loadingChanged();

    for (const auto& repoUrl : std::as_const(remoteRepos)) {
      auto* reply = m_nam.get(QNetworkRequest(QUrl(repoUrl)));
      reply->setProperty("repoUrl", repoUrl);
      m_activeReplies.insert(reply);
      connect(reply, &QNetworkReply::finished, this, &ExtensionManager::onManifestReply);
    }
  }

  else {
    applyFilter();
    rebuildInstalledPlugins();
  }
}

/**
 * @brief Adds a new repository URL and persists the list.
 */
void Misc::ExtensionManager::addRepository(const QString& url)
{
  if (url.isEmpty() || m_repositories.contains(url))
    return;

  m_repositories.append(url);
  m_settings.setValue("ExtensionRepositories", m_repositories);
  Q_EMIT repositoriesChanged();
  refreshRepositories();
}

/**
 * @brief Removes a repository by index and persists the list.
 */
void Misc::ExtensionManager::removeRepository(int index)
{
  if (index < 0 || index >= m_repositories.count())
    return;

  m_repositories.removeAt(index);
  m_settings.setValue("ExtensionRepositories", m_repositories);
  Q_EMIT repositoriesChanged();
  refreshRepositories();
}

/**
 * @brief Resets the repository list and uninstalls all extensions.
 */
void Misc::ExtensionManager::resetRepositories()
{
  const auto result = Misc::Utilities::showMessageBox(
    tr("Reset Extensions"),
    tr("This uninstalls all extensions, removes all custom repositories, "
       "and restores the default settings. Continue?"),
    QMessageBox::Warning,
    QString(),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (result != QMessageBox::Yes)
    return;

  stopAllPlugins();

  const auto ids = m_installedExtensions.keys();
  for (const auto& id : ids) {
    const auto info = m_installedExtensions.value(id).toObject();
    const auto type = info.value("type").toString();
    const auto dir  = extensionsPath() + "/" + type + "/" + id;
    QDir(dir).removeRecursively();
  }

  m_installedExtensions = QJsonObject();
  saveInstalledManifest();

  m_repositories.clear();
  m_repositories.append(kDefaultRepoUrl);
  m_settings.setValue("ExtensionRepositories", m_repositories);
  Q_EMIT repositoriesChanged();

  Q_EMIT extensionUninstalled(QString());

  refreshRepositories();
}

/**
 * @brief Opens a directory picker to select a local extension repository folder.
 */
void Misc::ExtensionManager::browseLocalRepo()
{
  auto* dialog = new QFileDialog(
    qApp->activeWindow(), tr("Select Extension Repository Folder"), QDir::homePath());

  dialog->setFileMode(QFileDialog::Directory);
  dialog->setOption(QFileDialog::ShowDirsOnly, true);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  connect(dialog, &QFileDialog::fileSelected, this, [this](const QString& path) {
    if (path.isEmpty())
      return;

    QMetaObject::invokeMethod(this, [this, path]() { addRepository(path); }, Qt::QueuedConnection);
  });

  dialog->open();
}

//--------------------------------------------------------------------------------------------------
// Install / uninstall
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downloads and installs the currently selected addon.
 */
void Misc::ExtensionManager::installExtension()
{
  if (m_loading)
    return;

  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExtensions.count())
    return;

  const auto addon   = m_filteredExtensions.at(m_selectedIndex).toMap();
  const auto id      = addon.value("id").toString();
  const auto type    = addon.value("type").toString();
  const auto base    = addon.value("_repoBase").toString();
  const auto isLocal = addon.value("_isLocal").toBool();

  auto files           = addon.value("files").toList();
  const auto platforms = addon.value("platforms").toMap();
  const auto override  = selectPlatformOverride(platforms, currentPlatformKey());
  const auto platFiles = override.value("files").toList();
  for (const auto& f : platFiles)
    if (!files.contains(f))
      files.append(f);

  if (id.isEmpty() || files.isEmpty())
    return;

  if (id.contains("..") || id.contains('/') || id.contains('\\'))
    return;

  if (type.contains("..") || type.contains('/') || type.contains('\\'))
    return;

  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  QDir().mkpath(installDir);

  if (isLocal) {
    for (const auto& f : std::as_const(files)) {
      const auto localName = f.toString();
      const auto dst       = installDir + "/" + localName;

      if (!isPathSafe(dst, installDir))
        continue;

      const auto src = base + localName;
      QDir().mkpath(QFileInfo(dst).absolutePath());
      QFile::copy(src, dst);
    }

    QJsonObject info;
    info.insert("version", addon.value("version").toString());
    info.insert("type", type);
    info.insert("repoBase", base);
    m_installedExtensions.insert(id, info);
    saveInstalledManifest();

    Q_EMIT extensionInstalled(id);
    m_pluginMetadataCache.remove(id);
    applyFilter();
    rebuildInstalledPlugins();
    return;
  }

  m_downloadQueue.clear();
  for (const auto& f : files) {
    const auto localName = f.toString();
    const auto url       = resolveFileUrl(base, localName);
    m_downloadQueue.append({localName, url});
  }

  m_currentInstallId       = id;
  m_currentInstallRepoBase = base;
  m_currentInstallMeta     = addon;
  m_loading                = true;
  m_downloadProgress       = 0;
  m_totalDownloads         = m_downloadQueue.count();
  m_pendingDownloads       = m_totalDownloads;
  Q_EMIT loadingChanged();
  Q_EMIT downloadProgressChanged();

  downloadNextFile();
}

/**
 * @brief Uninstalls the currently selected addon, returning false on an invalid selection
 *        or a partial-delete failure.
 */
bool Misc::ExtensionManager::uninstallExtension()
{
  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExtensions.count())
    return false;

  const auto addon = m_filteredExtensions.at(m_selectedIndex).toMap();
  const auto id    = addon.value("id").toString();
  const auto type  = addon.value("type").toString();

  if (id.isEmpty() || !isInstalled(id))
    return false;

  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  const bool removed    = QDir(installDir).removeRecursively();

  m_installedExtensions.remove(id);
  m_pluginMetadataCache.remove(id);
  saveInstalledManifest();

  Q_EMIT extensionUninstalled(id);
  applyFilter();
  rebuildInstalledPlugins();

  return removed;
}

//--------------------------------------------------------------------------------------------------
// Auto-update
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks installed extensions against the catalog and auto-updates any that have a newer
 * version available.
 */
void Misc::ExtensionManager::autoUpdateExtensions()
{
  if (m_loading)
    return;

  if (m_autoUpdateQueue.isEmpty()) {
    const auto ids = m_installedExtensions.keys();
    for (const auto& id : ids)
      if (hasUpdate(id))
        m_autoUpdateQueue.append(id);
  }

  if (m_autoUpdateQueue.isEmpty())
    return;

  const auto id = m_autoUpdateQueue.takeFirst();

  bool found = false;
  for (int i = 0; i < m_filteredExtensions.count(); ++i) {
    if (m_filteredExtensions.at(i).toMap().value("id").toString() != id)
      continue;

    m_selectedIndex = i;
    Q_EMIT selectedIndexChanged();
    installExtension();
    found = true;
    break;
  }

  if (!m_autoUpdateQueue.isEmpty()) {
    if (found && m_loading) {
      connect(
        this,
        &ExtensionManager::extensionInstalled,
        this,
        [this]() { QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions); },
        Qt::SingleShotConnection);
    }

    else {
      QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Network reply handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parses a successful manifest.json reply and queues each addon for metadata fetch.
 */
void Misc::ExtensionManager::parseManifest(QNetworkReply* reply)
{
  const auto parsed = Misc::JsonValidator::parseAndValidate(reply->readAll());
  if (!parsed.valid || !parsed.document.isObject()) {
    qWarning() << "[ExtensionManager] Rejected manifest JSON:" << parsed.errorMessage;
    return;
  }

  const auto root    = parsed.document.object();
  const auto addons  = root.value("extensions").toArray();
  const auto repoUrl = reply->property("repoUrl").toString();
  const auto baseUrl = repoUrl.left(repoUrl.lastIndexOf('/') + 1);

  for (const auto& entry : addons) {
    if (entry.isString()) {
      const auto metaPath  = entry.toString();
      const auto metaUrl   = baseUrl + metaPath;
      const auto addonBase = metaUrl.left(metaUrl.lastIndexOf('/') + 1);

      ++m_pendingExtensionMetas;
      auto* metaReply = m_nam.get(QNetworkRequest(QUrl(metaUrl)));
      metaReply->setProperty("addonBase", addonBase);
      m_activeReplies.insert(metaReply);
      connect(metaReply, &QNetworkReply::finished, this, &ExtensionManager::onExtensionMetaReply);
      continue;
    }

    if (entry.isObject()) {
      auto obj = entry.toObject();
      obj.insert("_repoBase", baseUrl);
      m_allExtensions.append(obj);
    }
  }
}

/**
 * @brief Handles a manifest.json fetch response from a repository.
 */
void Misc::ExtensionManager::onManifestReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_activeReplies.remove(reply);
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError)
    parseManifest(reply);

  --m_pendingManifests;
  if (m_pendingManifests <= 0 && m_pendingExtensionMetas <= 0) {
    m_loading = false;
    Q_EMIT loadingChanged();
    applyFilter();
    rebuildInstalledPlugins();
    QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
  }
}

/**
 * @brief Handles an extension metadata (info.json) fetch response.
 */
void Misc::ExtensionManager::onExtensionMetaReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_activeReplies.remove(reply);
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto parsed = Misc::JsonValidator::parseAndValidate(reply->readAll());
    auto obj          = parsed.valid ? parsed.document.object() : QJsonObject();
    if (!obj.isEmpty() && !obj.value("id").toString().isEmpty()) {
      const auto addonBase = reply->property("addonBase").toString();
      obj.insert("_repoBase", addonBase);
      m_allExtensions.append(obj);
    }
  }

  --m_pendingExtensionMetas;
  if (m_pendingManifests <= 0 && m_pendingExtensionMetas <= 0) {
    m_loading = false;
    Q_EMIT loadingChanged();
    applyFilter();
    rebuildInstalledPlugins();
    QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
  }
}

/**
 * @brief Handles the README.md fetch response for the selected extension.
 */
void Misc::ExtensionManager::onReadmeReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_activeReplies.remove(reply);
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError)
    m_selectedReadme = QString::fromUtf8(reply->readAll());

  Q_EMIT selectedReadmeChanged();
}

/**
 * @brief Handles individual file download completion during extension install.
 */
void Misc::ExtensionManager::onFileDownloadReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_activeReplies.remove(reply);
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError)
    writeExtensionFile(reply);

  --m_pendingDownloads;
  m_downloadProgress = (m_totalDownloads > 0)
                       ? static_cast<float>(m_totalDownloads - m_pendingDownloads)
                           / static_cast<float>(m_totalDownloads)
                       : 1.0f;
  Q_EMIT downloadProgressChanged();

  if (!m_downloadQueue.isEmpty()) {
    downloadNextFile();
    return;
  }

  QJsonObject info;
  info.insert("version", m_currentInstallMeta.value("version").toString());
  info.insert("type", m_currentInstallMeta.value("type").toString());
  info.insert("repoBase", m_currentInstallRepoBase);

  QJsonArray fileList;
  for (const auto& f : m_currentInstallMeta.value("files").toList())
    fileList.append(f.toString());

  info.insert("files", fileList);
  m_installedExtensions.insert(m_currentInstallId, info);
  saveInstalledManifest();

  m_loading = false;
  Q_EMIT loadingChanged();
  Q_EMIT extensionInstalled(m_currentInstallId);
  m_pluginMetadataCache.remove(m_currentInstallId);
  applyFilter();
  rebuildInstalledPlugins();
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downloads the next file in the install queue.
 */
void Misc::ExtensionManager::downloadNextFile()
{
  if (m_downloadQueue.isEmpty())
    return;

  const auto [localName, url] = m_downloadQueue.takeFirst();
  auto* reply                 = m_nam.get(QNetworkRequest(url));
  reply->setProperty("localName", localName);
  m_activeReplies.insert(reply);
  connect(reply, &QNetworkReply::finished, this, &ExtensionManager::onFileDownloadReply);
}

/**
 * @brief Applies search and category filters to rebuild the filtered list.
 */
void Misc::ExtensionManager::applyFilter()
{
  m_filteredExtensions.clear();

  for (const auto& entry : std::as_const(m_allExtensions)) {
    const auto obj = entry.toObject();
    if (!catalogEntryMatchesFilters(obj))
      continue;

    m_filteredExtensions.append(buildCatalogEntryMap(obj));
  }

  appendOrphanedInstalledEntries();

  static const QMap<QString, int> typeOrder = {
    {          QStringLiteral("plugin"), 0},
    {           QStringLiteral("theme"), 1},
    {    QStringLiteral("frame-parser"), 2},
    {QStringLiteral("project-template"), 3},
  };

  std::stable_sort(m_filteredExtensions.begin(),
                   m_filteredExtensions.end(),
                   [](const QVariant& a, const QVariant& b) {
                     const auto ta = a.toMap().value("type").toString();
                     const auto tb = b.toMap().value("type").toString();
                     return typeOrder.value(ta, 99) < typeOrder.value(tb, 99);
                   });

  restoreSelectionByPreviousId();

  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExtensionsChanged();
}

/**
 * @brief Returns true when @p entry passes the current type/category/search filters.
 */
bool Misc::ExtensionManager::catalogEntryMatchesFilters(const QJsonObject& entry) const
{
  if (!m_filterType.isEmpty() && m_filterType != QStringLiteral("All"))
    if (entry.value("type").toString() != m_filterType)
      return false;

  if (!m_filterCategory.isEmpty() && m_filterCategory != QStringLiteral("All")) {
    const auto category = entry.value("category").toString();
    if (!category.contains(m_filterCategory, Qt::CaseInsensitive))
      return false;
  }

  if (m_searchFilter.isEmpty())
    return true;

  const auto title  = entry.value("title").toString();
  const auto desc   = entry.value("description").toString();
  const auto author = entry.value("author").toString();

  return title.contains(m_searchFilter, Qt::CaseInsensitive)
      || desc.contains(m_searchFilter, Qt::CaseInsensitive)
      || author.contains(m_searchFilter, Qt::CaseInsensitive);
}

/**
 * @brief Builds the QML-friendly variant map for a catalog entry, including install state.
 */
QVariantMap Misc::ExtensionManager::buildCatalogEntryMap(const QJsonObject& entry) const
{
  auto map      = entry.toVariantMap();
  const auto id = entry.value("id").toString();
  map.insert("installed", isInstalled(id));
  map.insert("updateAvailable", hasUpdate(id));
  map.insert("installedVersion", installedVersion(id));
  map.insert("pluginRunning", isPluginRunning(id));

  const auto plats = entry.value("platforms").toObject();
  if (plats.isEmpty()) {
    map.insert("platformAvailable", true);
    return map;
  }

  const auto key        = currentPlatformKey();
  const auto os         = key.left(key.indexOf('/'));
  const bool compatible = plats.contains(key) || plats.contains(os + "/*") || plats.contains("*");
  map.insert("platformAvailable", compatible);
  return map;
}

/**
 * @brief Appends entries for installed extensions whose source repo is no longer available.
 */
void Misc::ExtensionManager::appendOrphanedInstalledEntries()
{
  QSet<QString> catalogIds;
  for (const auto& entry : std::as_const(m_filteredExtensions))
    catalogIds.insert(entry.toMap().value("id").toString());

  const auto installedIds = m_installedExtensions.keys();
  for (const auto& id : installedIds) {
    if (catalogIds.contains(id))
      continue;

    const auto info = m_installedExtensions.value(id).toObject();
    const auto type = info.value("type").toString();

    if (!m_filterType.isEmpty() && m_filterType != QStringLiteral("All"))
      if (type != m_filterType)
        continue;

    QVariantMap map;
    map.insert("id", id);
    map.insert("type", type);
    map.insert("version", info.value("version").toString());
    map.insert("installed", true);
    map.insert("updateAvailable", false);
    map.insert("installedVersion", info.value("version").toString());
    map.insert("pluginRunning", isPluginRunning(id));

    const auto addonJsonPath = extensionsPath() + "/" + type + "/" + id + "/info.json";
    QFile addonFile(addonJsonPath);
    if (addonFile.open(QIODevice::ReadOnly)) {
      const auto addonObj = QJsonDocument::fromJson(addonFile.readAll()).object();
      map.insert("title", addonObj.value("title").toString(id));
      map.insert("description", addonObj.value("description").toString());
      map.insert("author", addonObj.value("author").toString());
      map.insert("license", addonObj.value("license").toString());
      map.insert("category", addonObj.value("category").toString());
    }

    else {
      map.insert("title", id);
      map.insert("description", tr("Installed (repository no longer available)"));
      map.insert("author", QString());
    }

    if (!m_searchFilter.isEmpty()) {
      const auto title = map.value("title").toString();
      const auto desc  = map.value("description").toString();
      if (!title.contains(m_searchFilter, Qt::CaseInsensitive)
          && !desc.contains(m_searchFilter, Qt::CaseInsensitive))
        continue;
    }

    m_filteredExtensions.append(map);
  }
}

/**
 * @brief Restores the selected extension index by matching the previous entry's ID.
 */
void Misc::ExtensionManager::restoreSelectionByPreviousId()
{
  const auto previousAddon = selectedExtension();
  const auto previousId    = previousAddon.value("id").toString();
  m_selectedIndex          = -1;

  if (previousId.isEmpty())
    return;

  for (int i = 0; i < m_filteredExtensions.count(); ++i) {
    if (m_filteredExtensions.at(i).toMap().value("id").toString() == previousId) {
      m_selectedIndex = i;
      return;
    }
  }
}

/**
 * @brief Loads (and caches) the title/icon metadata for an installed plugin.
 */
QVariantMap Misc::ExtensionManager::loadPluginMetadata(const QString& iid)
{
  auto cacheIt = m_pluginMetadataCache.find(iid);
  if (cacheIt != m_pluginMetadataCache.end())
    return cacheIt.value();

  const auto pluginDir = extensionsPath() + "/plugin/" + iid;
  QVariantMap cached;

  QFile metaFile(pluginDir + "/info.json");
  if (!metaFile.open(QIODevice::ReadOnly)) {
    cached.insert("title", iid);
    m_pluginMetadataCache.insert(iid, cached);
    return cached;
  }

  const auto meta  = QJsonDocument::fromJson(metaFile.readAll()).object();
  const auto title = meta.value("title").toString(iid);
  cached.insert("title", title);

  const auto icon = meta.value("icon").toString();
  if (!icon.isEmpty())
    cached.insert("icon", QStringLiteral("file://") + pluginDir + "/" + icon);

  m_pluginMetadataCache.insert(iid, cached);
  return cached;
}

/**
 * @brief Rebuilds the installed plugins list for the start menu / toolbar.
 */
void Misc::ExtensionManager::rebuildInstalledPlugins()
{
  QVariantList plugins;
  const auto pluginIds = m_installedExtensions.keys();
  for (const auto& iid : pluginIds) {
    const auto info = m_installedExtensions.value(iid).toObject();
    if (info.value("type").toString() != QStringLiteral("plugin"))
      continue;

    QVariantMap entry;
    entry.insert("id", iid);
    entry.insert("running", isPluginRunning(iid));

    const auto meta = loadPluginMetadata(iid);
    entry.insert("title", meta.value("title"));

    const auto icon = meta.value("icon").toString();
    if (!icon.isEmpty())
      entry.insert("icon", icon);

    plugins.append(entry);
  }

  if (plugins != m_installedPlugins) {
    m_installedPlugins = plugins;
    Q_EMIT installedPluginsChanged();
  }
}

/**
 * @brief Returns the base extensions directory path.
 */
QString Misc::ExtensionManager::extensionsPath() const
{
  return Misc::WorkspaceManager::instance().path("Extensions");
}

/**
 * @brief Returns the themes extension directory path.
 */
QString Misc::ExtensionManager::themesPath() const
{
  return extensionsPath() + "/theme";
}

/**
 * @brief Returns the path to the installed.json tracking file.
 */
QString Misc::ExtensionManager::installedManifestPath() const
{
  return extensionsPath() + "/installed.json";
}

/**
 * @brief Returns the platform key for the current OS and CPU architecture.
 */
QString Misc::ExtensionManager::currentPlatformKey() const
{
#if defined(Q_OS_MACOS)
  const auto os = QStringLiteral("darwin");
#elif defined(Q_OS_WIN)
  const auto os = QStringLiteral("windows");
#else
  const auto os = QStringLiteral("linux");
#endif

  const auto arch = QSysInfo::currentCpuArchitecture();
  auto normalized = arch;
  if (arch == QStringLiteral("arm64") || arch == QStringLiteral("aarch64"))
    normalized = QStringLiteral("arm64");
  else if (arch == QStringLiteral("x86_64") || arch == QStringLiteral("amd64"))
    normalized = QStringLiteral("x86_64");

  return os + "/" + normalized;
}

/**
 * @brief Resolves platform-specific overrides from a plugin's metadata.
 */
QJsonObject Misc::ExtensionManager::resolvePlatform(const QJsonObject& meta) const
{
  auto result = meta;

  const auto platforms = meta.value("platforms").toObject();
  if (platforms.isEmpty())
    return result;

  const auto key = currentPlatformKey();
  const auto os  = key.left(key.indexOf('/'));

  QJsonObject override;
  if (platforms.contains(key))
    override = platforms.value(key).toObject();
  else if (platforms.contains(os + "/*"))
    override = platforms.value(os + "/*").toObject();
  else if (platforms.contains("*"))
    override = platforms.value("*").toObject();

  if (override.isEmpty())
    return result;

  for (auto it = override.begin(); it != override.end(); ++it)
    result.insert(it.key(), it.value());

  return result;
}

//--------------------------------------------------------------------------------------------------
// Plugin management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list of currently running plugins.
 */
const QVariantList& Misc::ExtensionManager::runningPlugins() const noexcept
{
  return m_runningPlugins;
}

/**
 * @brief Returns the list of installed plugins with id, title, icon, and running state.
 */
const QVariantList& Misc::ExtensionManager::installedPlugins() const noexcept
{
  return m_installedPlugins;
}

/**
 * @brief Returns whether the plugin with the given ID is currently running.
 */
bool Misc::ExtensionManager::isPluginRunning(const QString& id) const
{
  return m_plugins.contains(id);
}

/**
 * @brief Returns the captured stdout/stderr output for the given plugin.
 */
QString Misc::ExtensionManager::pluginOutput(const QString& id) const
{
  return m_pluginOutput.value(id, QString());
}

/**
 * @brief Convenience -- launches the currently selected addon if it's a plugin.
 */
void Misc::ExtensionManager::launchSelectedPlugin()
{
  const auto addon = selectedExtension();
  launchPlugin(addon.value("id").toString());
}

/**
 * @brief Convenience -- stops the currently selected addon if it's a running plugin.
 */
void Misc::ExtensionManager::stopSelectedPlugin()
{
  const auto addon = selectedExtension();
  stopPlugin(addon.value("id").toString());
}

/**
 * @brief Launches an installed plugin by ID.
 */
void Misc::ExtensionManager::launchPlugin(const QString& id)
{
  if (id.isEmpty())
    return;

  m_userClosedPlugins.remove(id);

  if (!checkLaunchPreconditions(id))
    return;

  const auto pluginDir = extensionsPath() + "/plugin/" + id;
  QJsonObject resolved;
  if (!readPluginMetadata(id, pluginDir, resolved))
    return;

  const auto entry    = resolved.value("entry").toString();
  const auto runtime  = resolved.value("runtime").toString("python3");
  const auto terminal = resolved.value("terminal").toBool(false);
  const auto usesGrpc = resolved.value("grpc").toBool(false);

  if (!ensureApiServerForLaunch(id, usesGrpc))
    return;

  QString entryPath;
  if (!resolveAndValidateEntry(id, pluginDir, entry, entryPath))
    return;

  bool hasPipDeps = false;
  if (!checkPluginDependencies(id, resolved.value("dependencies").toArray(), hasPipDeps))
    return;

  auto* process = new QProcess(this);
  process->setWorkingDirectory(pluginDir);
  process->setProcessEnvironment(buildPluginEnvironment());

  if (terminal)
    process->setProcessChannelMode(QProcess::ForwardedChannels);
  else {
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setStandardInputFile(QProcess::nullDevice());
  }

  m_pluginOutput.insert(id, QString());
  wirePluginProcessSignals(process, id);

  startPluginProcess(process, runtime, entryPath, terminal);

  if (!process->waitForStarted(3000)) {
    m_pluginOutput[id] +=
      QStringLiteral("[Error] Failed to start: ") + process->errorString() + "\n";
    Q_EMIT pluginOutputChanged(id);
    delete process;
    return;
  }

  registerRunningPlugin(id, process, resolved, pluginDir, terminal, hasPipDeps);
}

/**
 * @brief Verifies a plugin can be launched: not running, installed, and of type "plugin".
 */
bool Misc::ExtensionManager::checkLaunchPreconditions(const QString& id)
{
  if (isPluginRunning(id)) {
    m_pluginOutput[id] += QStringLiteral("[Already running]\n");
    Q_EMIT pluginOutputChanged(id);
    return false;
  }

  if (!isInstalled(id)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Plugin is not installed\n");
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"), tr("Plugin \"%1\" is not installed.").arg(id), QMessageBox::Critical);
    return false;
  }

  const auto info = m_installedExtensions.value(id).toObject();
  const auto type = info.value("type").toString();
  if (type != QStringLiteral("plugin")) {
    m_pluginOutput[id] += QStringLiteral("[Error] Not a plugin (type: %1)\n").arg(type);
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"),
      tr("Extension \"%1\" is not a plugin (type: %2).").arg(id, type),
      QMessageBox::Critical);
    return false;
  }

  return true;
}

/**
 * @brief Reads the plugin info.json and resolves platform-specific overrides.
 */
bool Misc::ExtensionManager::readPluginMetadata(const QString& id,
                                                const QString& pluginDir,
                                                QJsonObject& resolvedOut)
{
  QFile metaFile(pluginDir + "/info.json");
  if (!metaFile.open(QIODevice::ReadOnly)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Cannot read %1/info.json\n").arg(pluginDir);
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"),
      tr("Cannot read plugin metadata file:\n%1/info.json").arg(pluginDir),
      QMessageBox::Critical);
    return false;
  }

  const auto metaDoc = QJsonDocument::fromJson(metaFile.readAll());
  resolvedOut        = resolvePlatform(metaDoc.object());
  return true;
}

/**
 * @brief Verifies gRPC build support and prompts the user to enable the API server.
 */
bool Misc::ExtensionManager::ensureApiServerForLaunch(const QString& id, bool usesGrpc)
{
#ifndef ENABLE_GRPC
  if (usesGrpc) {
    m_pluginOutput[id] += QStringLiteral(
      "[Error] Plugin requires gRPC but this build does not include gRPC support.\n");
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"),
      tr("Plugin \"%1\" requires gRPC but this build does not include gRPC support.").arg(id),
      QMessageBox::Critical);
    return false;
  }
#endif

  if (API::Server::instance().enabled())
    return true;

  const auto msg = usesGrpc ? tr("This plugin uses gRPC for high-performance data streaming. "
                                 "The API server needs to be enabled.\n\n"
                                 "Would you like to enable it now?")
                            : tr("Plugins need the API server to communicate with Serial Studio. "
                                 "Would you like to enable it now?");

  const auto result = Misc::Utilities::showMessageBox(tr("API Server Required"),
                                                      msg,
                                                      QMessageBox::Question,
                                                      QString(),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes);

  if (result == QMessageBox::Yes) {
    API::Server::instance().setEnabled(true);
    return true;
  }

  m_pluginOutput[id] += QStringLiteral("[Cancelled] API Server not enabled.\n");
  Q_EMIT pluginOutputChanged(id);
  return false;
}

/**
 * @brief Resolves the absolute entry-point path and validates it stays within the plugin dir.
 */
bool Misc::ExtensionManager::resolveAndValidateEntry(const QString& id,
                                                     const QString& pluginDir,
                                                     const QString& entry,
                                                     QString& entryPathOut)
{
  if (entry.isEmpty()) {
    m_pluginOutput[id] += QStringLiteral("[Error] No 'entry' field in info.json\n");
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(tr("Plugin Error"),
                                    tr("Plugin \"%1\" has no 'entry' field in info.json.").arg(id),
                                    QMessageBox::Critical);
    return false;
  }

  entryPathOut = pluginDir + "/" + entry;
  if (!QFile::exists(entryPathOut)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Entry point not found: %1\n").arg(entryPathOut);
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(tr("Plugin Error"),
                                    tr("Entry point not found:\n%1").arg(entryPathOut),
                                    QMessageBox::Critical);
    return false;
  }

  if (!isPathSafe(entryPathOut, pluginDir)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Invalid entry point path\n");
    Q_EMIT pluginOutputChanged(id);
    Misc::Utilities::showMessageBox(tr("Plugin Error"),
                                    tr("Plugin \"%1\" has an invalid entry point path.").arg(id),
                                    QMessageBox::Critical);
    return false;
  }

  return true;
}

/**
 * @brief Walks the plugin's dependency list, prompting for any missing executables.
 */
bool Misc::ExtensionManager::checkPluginDependencies(const QString& id,
                                                     const QJsonArray& deps,
                                                     bool& hasPipDepsOut)
{
  hasPipDepsOut = false;

  for (const auto& dep : deps) {
    const auto obj  = dep.toObject();
    const auto name = obj.value("name").toString();
    const auto exes = obj.value("executables").toArray();
    const auto url  = obj.value("url").toString();

    if (obj.contains("pip")) {
      hasPipDepsOut = true;
      continue;
    }

    if (exes.isEmpty())
      continue;

    bool found = false;
    for (const auto& exe : exes) {
      if (!QStandardPaths::findExecutable(exe.toString()).isEmpty()) {
        found = true;
        break;
      }
    }

    if (found)
      continue;

    const auto result =
      Misc::Utilities::showMessageBox(tr("Missing Dependency"),
                                      tr("This plugin requires \"%1\" but it was not found on your "
                                         "system.\n\nWould you like to open the download page?")
                                        .arg(name),
                                      QMessageBox::Warning,
                                      QString(),
                                      QMessageBox::Yes | QMessageBox::Cancel,
                                      QMessageBox::Yes);

    if (result == QMessageBox::Yes && !url.isEmpty())
      QDesktopServices::openUrl(QUrl(url));

    m_pluginOutput[id] += QStringLiteral("[Error] Missing dependency: %1\n").arg(name);
    Q_EMIT pluginOutputChanged(id);
    return false;
  }

  return true;
}

/**
 * @brief Builds the QProcessEnvironment for plugin processes, prepending common tool paths.
 */
QProcessEnvironment Misc::ExtensionManager::buildPluginEnvironment() const
{
  auto env  = QProcessEnvironment::systemEnvironment();
  auto path = env.value("PATH");
#ifdef Q_OS_MACOS
  const QStringList extraPaths = {
    "/opt/homebrew/bin",
    "/opt/homebrew/sbin",
    "/usr/local/bin",
    QDir::homePath() + "/.local/bin",
  };

  for (const auto& p : extraPaths)
    if (!path.contains(p) && QDir(p).exists())
      path = p + ":" + path;
#endif
  env.insert("PATH", path);
  env.insert("TK_SILENCE_DEPRECATION", "1");
  return env;
}

/**
 * @brief Connects readyRead/finished/errorOccurred handlers for a plugin's QProcess.
 */
void Misc::ExtensionManager::wirePluginProcessSignals(QProcess* process, const QString& id)
{
  const auto pluginId = id;

  connect(process, &QProcess::readyRead, this, [this, pluginId, process]() {
    const auto text           = QString::fromUtf8(process->readAll());
    m_pluginOutput[pluginId] += text;

    auto& buf = m_pluginOutput[pluginId];
    if (buf.size() > 65536)
      buf = buf.right(32768);

    Q_EMIT pluginOutputChanged(pluginId);
  });

  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this,
          [this, pluginId]() { onPluginFinished(pluginId); });

  connect(process, &QProcess::errorOccurred, this, [this, pluginId](QProcess::ProcessError err) {
    Q_UNUSED(err)
    const auto msg            = QStringLiteral("[Error] Plugin failed to start or crashed.\n");
    m_pluginOutput[pluginId] += msg;
    Q_EMIT pluginOutputChanged(pluginId);
  });
}

/**
 * @brief Starts a plugin process directly or via the platform-native terminal emulator.
 */
void Misc::ExtensionManager::startPluginProcess(QProcess* process,
                                                const QString& runtime,
                                                const QString& entryPath,
                                                bool terminal)
{
  const bool isNative = runtime.isEmpty();
#ifndef Q_OS_WIN
  if (isNative)
    QFile::setPermissions(
      entryPath, QFile::permissions(entryPath) | QFileDevice::ExeUser | QFileDevice::ExeGroup);
#endif

  if (!terminal) {
    if (isNative)
      process->start(entryPath);
    else
      process->start(runtime, {entryPath});

    return;
  }

#ifdef Q_OS_MACOS
  Q_UNUSED(isNative)
  process->start("open", {"-a", "Terminal", entryPath});
#elif defined(Q_OS_WIN)
  if (isNative)
    process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", entryPath});
  else
    process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", runtime, entryPath});
#else
  const QStringList terms = {
    "x-terminal-emulator", "gnome-terminal", "konsole", "xfce4-terminal", "xterm"};
  for (const auto& term : terms) {
    if (QStandardPaths::findExecutable(term).isEmpty())
      continue;

    if (isNative)
      process->start(term, {"--", entryPath});
    else
      process->start(term, {"--", runtime, entryPath});

    return;
  }

  if (isNative)
    process->start(entryPath);
  else
    process->start(runtime, {entryPath});
#endif
}

/**
 * @brief Records a successfully started plugin and refreshes related UI state.
 */
void Misc::ExtensionManager::registerRunningPlugin(const QString& id,
                                                   QProcess* process,
                                                   const QJsonObject& resolved,
                                                   const QString& pluginDir,
                                                   bool terminal,
                                                   bool hasPipDeps)
{
  const auto mode = terminal ? QStringLiteral(" (terminal)") : QString();
  m_pluginOutput[id] +=
    QStringLiteral("[Started] PID ") + QString::number(process->processId()) + mode + "\n";

  if (hasPipDeps && !QDir(pluginDir + "/venv").exists()) {
    m_pluginOutput[id] +=
      QStringLiteral("[Setup] Installing required packages -- this may take a moment...\n");
  }

  Q_EMIT pluginOutputChanged(id);

  m_plugins.insert(id, process);

  QVariantMap entry_map;
  entry_map.insert("id", id);
  entry_map.insert("title", resolved.value("title").toString(id));
  m_runningPlugins.append(entry_map);
  Q_EMIT runningPluginsChanged();
  applyFilter();
  rebuildInstalledPlugins();
}

/**
 * @brief Stops a running plugin by ID.
 */
void Misc::ExtensionManager::stopPlugin(const QString& id)
{
  auto it = m_plugins.find(id);
  if (it == m_plugins.end())
    return;

  if (UI::Dashboard::instance().available())
    m_userClosedPlugins.insert(id);

  auto* process       = it.value();
  m_pluginOutput[id] += QStringLiteral("[Stopping...]\n");
  Q_EMIT pluginOutputChanged(id);

  process->disconnect(this);

  m_plugins.erase(it);

  process->terminate();
  if (!process->waitForFinished(3000))
    process->kill();

  const auto remaining = QString::fromUtf8(process->readAll());
  if (!remaining.isEmpty())
    m_pluginOutput[id] += remaining;

  m_pluginOutput[id] += QStringLiteral("[Stopped]\n");
  Q_EMIT pluginOutputChanged(id);

  delete process;

  for (int i = 0; i < m_runningPlugins.count(); ++i) {
    if (m_runningPlugins.at(i).toMap().value("id").toString() == id) {
      m_runningPlugins.removeAt(i);
      break;
    }
  }

  Q_EMIT runningPluginsChanged();
  applyFilter();
  rebuildInstalledPlugins();
}

/**
 * @brief Stops all running plugins. Called on application shutdown.
 */
void Misc::ExtensionManager::stopAllPlugins()
{
  const auto ids = m_plugins.keys();
  m_settings.setValue("RunningPlugins", QStringList(ids));

  for (const auto& id : ids) {
    auto* process = m_plugins.value(id);
    if (!process)
      continue;

    process->disconnect(this);
    process->terminate();
    if (!process->waitForFinished(3000))
      process->kill();

    delete process;
  }

  m_plugins.clear();
  m_runningPlugins.clear();
  Q_EMIT runningPluginsChanged();
  applyFilter();
  rebuildInstalledPlugins();
}

/**
 * @brief Restores plugins that were running in the previous session.
 */
void Misc::ExtensionManager::restoreRunningPlugins()
{
  if (m_loading) {
    connect(
      this,
      &ExtensionManager::loadingChanged,
      this,
      [this]() {
        if (!m_loading)
          restoreRunningPlugins();
      },
      Qt::SingleShotConnection);
    return;
  }

  const auto ids = m_settings.value("RunningPlugins").toStringList();
  for (const auto& id : ids)
    if (isInstalled(id) && !isPluginRunning(id) && !m_userClosedPlugins.contains(id))
      launchPlugin(id);
}

/**
 * @brief Reacts to dashboard availability changes.
 */
void Misc::ExtensionManager::onDashboardAvailableChanged()
{
  const bool available = UI::Dashboard::instance().available();

  if (available && !m_dashboardWasAvailable)
    restoreRunningPlugins();

  else if (!available && m_dashboardWasAvailable)
    stopAllPlugins();

  m_dashboardWasAvailable = available;
}

/**
 * @brief Handles plugin process termination.
 */
void Misc::ExtensionManager::onPluginFinished(const QString& id)
{
  auto it = m_plugins.find(id);
  if (it == m_plugins.end())
    return;

  if (UI::Dashboard::instance().available())
    m_userClosedPlugins.insert(id);

  auto* process        = it.value();
  const auto remaining = QString::fromUtf8(process->readAll());
  if (!remaining.isEmpty())
    m_pluginOutput[id] += remaining;

  const auto exitCode  = process->exitCode();
  m_pluginOutput[id]  += QStringLiteral("[Exited with code %1]\n").arg(exitCode);
  Q_EMIT pluginOutputChanged(id);

  m_plugins.erase(it);
  process->deleteLater();

  for (int i = 0; i < m_runningPlugins.count(); ++i) {
    if (m_runningPlugins.at(i).toMap().value("id").toString() == id) {
      m_runningPlugins.removeAt(i);
      break;
    }
  }

  Q_EMIT runningPluginsChanged();
  applyFilter();
  rebuildInstalledPlugins();
}

//--------------------------------------------------------------------------------------------------
// Local manifest & file I/O
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads a manifest.json from a local filesystem directory.
 */
void Misc::ExtensionManager::loadLocalManifest(const QString& repoPath)
{
  auto path = repoPath;
  if (path.startsWith("file://"))
    path = QUrl(path).toLocalFile();

  QFileInfo info(path);
  if (info.isDir())
    path = path + "/manifest.json";

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
    return;

  const auto doc     = QJsonDocument::fromJson(file.readAll());
  const auto root    = doc.object();
  const auto addons  = root.value("extensions").toArray();
  const auto repoDir = QFileInfo(path).absolutePath() + "/";

  for (const auto& entry : addons) {
    if (entry.isString()) {
      const auto metaPath  = repoDir + entry.toString();
      const auto addonBase = QFileInfo(metaPath).absolutePath() + "/";

      QFile metaFile(metaPath);
      if (!metaFile.open(QIODevice::ReadOnly))
        continue;

      auto obj = QJsonDocument::fromJson(metaFile.readAll()).object();
      obj.insert("_repoBase", addonBase);
      obj.insert("_isLocal", true);
      m_allExtensions.append(obj);
    }

    else if (entry.isObject()) {
      auto obj = entry.toObject();
      obj.insert("_repoBase", repoDir);
      obj.insert("_isLocal", true);
      m_allExtensions.append(obj);
    }
  }
}

/**
 * @brief Loads the installed addon tracking manifest from disk.
 */
void Misc::ExtensionManager::loadInstalledManifest()
{
  QFile file(installedManifestPath());
  if (!file.open(QIODevice::ReadOnly))
    return;

  const auto doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isObject())
    m_installedExtensions = doc.object();
}

/**
 * @brief Saves the installed addon tracking manifest to disk.
 */
void Misc::ExtensionManager::saveInstalledManifest()
{
  QDir().mkpath(extensionsPath());
  QFile file(installedManifestPath());
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(m_installedExtensions).toJson(QJsonDocument::Indented));
    file.close();
  }
}

/**
 * @brief Writes a downloaded addon file to the appropriate install directory.
 */
void Misc::ExtensionManager::writeExtensionFile(QNetworkReply* reply)
{
  const auto localName  = reply->property("localName").toString();
  const auto type       = m_currentInstallMeta.value("type").toString();
  const auto installDir = extensionsPath() + "/" + type + "/" + m_currentInstallId;
  const auto filePath   = installDir + "/" + localName;

  if (!isPathSafe(filePath, installDir))
    return;

  QDir().mkpath(QFileInfo(filePath).absolutePath());

  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(reply->readAll());
    file.close();
  }
}

/**
 * @brief Validates that a resolved file path stays within the expected base directory.
 */
bool Misc::ExtensionManager::isPathSafe(const QString& filePath, const QString& baseDir) const
{
  const auto canonical = QFileInfo(filePath).absoluteFilePath();
  const auto base      = QFileInfo(baseDir).absoluteFilePath();
  return canonical.startsWith(base + "/") || canonical == base;
}

/**
 * @brief Resolves a relative file path against a repository base URL.
 */
QUrl Misc::ExtensionManager::resolveFileUrl(const QString& repoBaseUrl,
                                            const QString& relativePath) const
{
  return QUrl(repoBaseUrl + relativePath);
}
