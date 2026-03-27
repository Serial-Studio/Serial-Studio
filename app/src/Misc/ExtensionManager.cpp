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

#include "Misc/ExtensionManager.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QTimer>

#include "API/Server.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"

//--------------------------------------------------------------------------------------------------
// Default repository URL
//--------------------------------------------------------------------------------------------------

static const QString kDefaultRepoUrl =
  QStringLiteral("https://raw.githubusercontent.com/serial-studio/extensions/main/manifest.json");

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ExtensionManager singleton.
 *
 * Loads the repository list from QSettings and reads the local installed
 * manifest from disk.
 */
Misc::ExtensionManager::ExtensionManager()
  : m_loading(false)
  , m_selectedIndex(-1)
  , m_downloadProgress(0)
  , m_pendingDownloads(0)
  , m_totalDownloads(0)
  , m_pendingManifests(0)
  , m_pendingExtensionMetas(0)
{
  // Load repository list from settings, default to community repo
  const auto saved = m_settings.value("ExtensionRepositories").toStringList();
  if (saved.isEmpty())
    m_repositories.append(kDefaultRepoUrl);
  else
    m_repositories = saved;

  // Load installed extension tracking and rebuild plugin list
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

  // Find the extension in the catalog
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

  // Clear previous readme and load new one
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
    } else if (!base.isEmpty()) {
      auto* reply = m_nam.get(QNetworkRequest(QUrl(base + "README.md")));
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

  // Clear previous catalog
  m_allExtensions = QJsonArray();
  m_filteredExtensions.clear();
  m_selectedIndex = -1;
  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExtensionsChanged();

  m_pendingExtensionMetas = 0;

  // Separate local and remote repos
  QStringList remoteRepos;
  QStringList localRepos;
  for (const auto& repo : std::as_const(m_repositories))
    if (isLocalRepo(repo))
      localRepos.append(repo);
    else
      remoteRepos.append(repo);

  // Load local manifests synchronously
  for (const auto& localPath : std::as_const(localRepos))
    loadLocalManifest(localPath);

  // Fetch remote manifests asynchronously
  m_pendingManifests = remoteRepos.count();
  if (m_pendingManifests > 0) {
    m_loading = true;
    Q_EMIT loadingChanged();

    for (const auto& repoUrl : std::as_const(remoteRepos)) {
      auto* reply = m_nam.get(QNetworkRequest(QUrl(repoUrl)));
      reply->setProperty("repoUrl", repoUrl);
      connect(reply, &QNetworkReply::finished, this, &ExtensionManager::onManifestReply);
    }
  } else {
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
 *
 * Prompts the user for confirmation before proceeding. Stops all running
 * plugins, removes all installed extension files, and restores the default
 * community repository URL.
 */
void Misc::ExtensionManager::resetRepositories()
{
  const auto result = Misc::Utilities::showMessageBox(
    tr("Reset Extensions"),
    tr("This will uninstall all extensions, remove all custom repositories, "
       "and restore the default settings. Continue?"),
    QMessageBox::Warning,
    QString(),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (result != QMessageBox::Yes)
    return;

  // Stop all running plugins
  stopAllPlugins();

  // Remove all installed extension files
  const auto ids = m_installedExtensions.keys();
  for (const auto& id : ids) {
    const auto info = m_installedExtensions.value(id).toObject();
    const auto type = info.value("type").toString();
    const auto dir  = extensionsPath() + "/" + type + "/" + id;
    QDir(dir).removeRecursively();
  }

  m_installedExtensions = QJsonObject();
  saveInstalledManifest();

  // Reset repos to default
  m_repositories.clear();
  m_repositories.append(kDefaultRepoUrl);
  m_settings.setValue("ExtensionRepositories", m_repositories);
  Q_EMIT repositoriesChanged();

  // Notify theme manager about removed themes
  Q_EMIT extensionUninstalled(QString());

  refreshRepositories();
}

/**
 * @brief Opens a directory picker to select a local extension repository folder.
 */
void Misc::ExtensionManager::browseLocalRepo()
{
  const auto path = QFileDialog::getExistingDirectory(
    nullptr, tr("Select Extension Repository Folder"), QDir::homePath());

  if (!path.isEmpty())
    addRepository(path);
}

//--------------------------------------------------------------------------------------------------
// Install / uninstall
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downloads and installs the currently selected addon.
 *
 * Builds a download queue from the extension's file list, creates the local
 * directory, and starts sequential file downloads.
 */
void Misc::ExtensionManager::installExtension()
{
  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExtensions.count())
    return;

  const auto addon   = m_filteredExtensions.at(m_selectedIndex).toMap();
  const auto id      = addon.value("id").toString();
  const auto type    = addon.value("type").toString();
  const auto base    = addon.value("_repoBase").toString();
  const auto isLocal = addon.value("_isLocal").toBool();

  // Collect base files + platform-specific files
  auto files           = addon.value("files").toList();
  const auto platforms = addon.value("platforms").toMap();
  if (!platforms.isEmpty()) {
    const auto key = currentPlatformKey();
    const auto os  = key.left(key.indexOf('/'));
    QVariantMap override;
    if (platforms.contains(key))
      override = platforms.value(key).toMap();
    else if (platforms.contains(os + "/*"))
      override = platforms.value(os + "/*").toMap();
    else if (platforms.contains("*"))
      override = platforms.value("*").toMap();

    const auto platFiles = override.value("files").toList();
    for (const auto& f : platFiles)
      if (!files.contains(f))
        files.append(f);
  }

  if (id.isEmpty() || files.isEmpty())
    return;

  // Reject IDs or types containing path traversal components
  if (id.contains("..") || id.contains('/') || id.contains('\\'))
    return;

  if (type.contains("..") || type.contains('/') || type.contains('\\'))
    return;

  // Determine install path based on extension type
  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  QDir().mkpath(installDir);

  // Local repos: copy files directly (paths are relative to info.json)
  if (isLocal) {
    for (const auto& f : std::as_const(files)) {
      const auto localName = f.toString();
      const auto dst       = installDir + "/" + localName;

      // Validate destination stays within install directory
      if (!isPathSafe(dst, installDir))
        continue;

      const auto src = base + localName;
      QDir().mkpath(QFileInfo(dst).absolutePath());
      QFile::copy(src, dst);
    }

    // Register the installed addon
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

  // Remote repos: build download queue (paths relative to info.json)
  m_downloadQueue.clear();
  for (const auto& f : files) {
    const auto localName = f.toString();
    const auto url       = resolveFileUrl(base, localName);
    m_downloadQueue.append({localName, url});
  }

  // Save metadata snapshot so async callbacks don't depend on selection
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
 * @brief Uninstalls the currently selected addon by removing its files.
 */
void Misc::ExtensionManager::uninstallExtension()
{
  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExtensions.count())
    return;

  const auto addon = m_filteredExtensions.at(m_selectedIndex).toMap();
  const auto id    = addon.value("id").toString();
  const auto type  = addon.value("type").toString();

  if (id.isEmpty() || !isInstalled(id))
    return;

  // Remove addon directory
  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  QDir(installDir).removeRecursively();

  m_installedExtensions.remove(id);
  m_pluginMetadataCache.remove(id);
  saveInstalledManifest();

  Q_EMIT extensionUninstalled(id);
  applyFilter();
  rebuildInstalledPlugins();
}

//--------------------------------------------------------------------------------------------------
// Auto-update
//--------------------------------------------------------------------------------------------------

/**
 * @brief Checks installed extensions against the catalog and auto-updates
 *        any that have a newer version available.
 *
 * Builds a queue of extension IDs that need updating, then processes them
 * one at a time by selecting each in the filtered list and calling
 * installExtension(). For remote installs (async), the next update is
 * triggered after extensionInstalled fires.
 */
void Misc::ExtensionManager::autoUpdateExtensions()
{
  // Don't start if already loading/installing
  if (m_loading)
    return;

  // Build update queue on first call
  if (m_autoUpdateQueue.isEmpty()) {
    const auto ids = m_installedExtensions.keys();
    for (const auto& id : ids)
      if (hasUpdate(id))
        m_autoUpdateQueue.append(id);
  }

  if (m_autoUpdateQueue.isEmpty())
    return;

  // Process next update
  const auto id = m_autoUpdateQueue.takeFirst();

  // Find the extension in the filtered list
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

  // Schedule next update (after current install completes or immediately)
  if (!m_autoUpdateQueue.isEmpty()) {
    if (found && m_loading) {
      // Remote install in progress — wait for completion
      connect(this,
              &ExtensionManager::extensionInstalled,
              this,
              &ExtensionManager::autoUpdateExtensions,
              Qt::SingleShotConnection);
    } else {
      // Local install completed synchronously, or extension not found
      QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Network reply handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a manifest.json fetch response from a repository.
 */
void Misc::ExtensionManager::onManifestReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  // Parse the manifest and fetch each addon's metadata
  if (reply->error() == QNetworkReply::NoError) {
    const auto doc     = QJsonDocument::fromJson(reply->readAll());
    const auto root    = doc.object();
    const auto addons  = root.value("extensions").toArray();
    const auto repoUrl = reply->property("repoUrl").toString();
    const auto baseUrl = repoUrl.left(repoUrl.lastIndexOf('/') + 1);

    for (const auto& entry : addons) {
      // Support both string paths (new) and inline objects (legacy)
      if (entry.isString()) {
        const auto metaPath  = entry.toString();
        const auto metaUrl   = baseUrl + metaPath;
        const auto addonBase = metaUrl.left(metaUrl.lastIndexOf('/') + 1);

        ++m_pendingExtensionMetas;
        auto* metaReply = m_nam.get(QNetworkRequest(QUrl(metaUrl)));
        metaReply->setProperty("addonBase", addonBase);
        connect(metaReply, &QNetworkReply::finished, this, &ExtensionManager::onExtensionMetaReply);
      } else if (entry.isObject()) {
        auto obj = entry.toObject();
        obj.insert("_repoBase", baseUrl);
        m_allExtensions.append(obj);
      }
    }
  }

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
 *
 * Resolves file paths relative to the extension.json location and appends
 * the extension to the catalog.
 */
void Misc::ExtensionManager::onExtensionMetaReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto doc       = QJsonDocument::fromJson(reply->readAll());
    auto obj             = doc.object();
    const auto addonBase = reply->property("addonBase").toString();
    obj.insert("_repoBase", addonBase);
    m_allExtensions.append(obj);
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

  reply->deleteLater();

  // Write the downloaded file to the install directory
  if (reply->error() == QNetworkReply::NoError)
    writeExtensionFile(reply);

  --m_pendingDownloads;
  m_downloadProgress = (m_totalDownloads > 0)
                       ? static_cast<float>(m_totalDownloads - m_pendingDownloads)
                           / static_cast<float>(m_totalDownloads)
                       : 1.0f;
  Q_EMIT downloadProgressChanged();

  // Continue with next file or finish
  if (!m_downloadQueue.isEmpty()) {
    downloadNextFile();
    return;
  }

  // All downloads complete — register using saved metadata snapshot
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

  // Clear loading state
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
  connect(reply, &QNetworkReply::finished, this, &ExtensionManager::onFileDownloadReply);
}

/**
 * @brief Applies search and category filters to rebuild the filtered list.
 *
 * Injects installation status into each extension entry for QML display.
 */
void Misc::ExtensionManager::applyFilter()
{
  m_filteredExtensions.clear();

  for (const auto& entry : std::as_const(m_allExtensions)) {
    const auto obj = entry.toObject();

    if (!m_filterType.isEmpty() && m_filterType != QStringLiteral("All")) {
      if (obj.value("type").toString() != m_filterType)
        continue;
    }

    if (!m_filterCategory.isEmpty() && m_filterCategory != QStringLiteral("All")) {
      const auto category = obj.value("category").toString();
      if (!category.contains(m_filterCategory, Qt::CaseInsensitive))
        continue;
    }

    if (!m_searchFilter.isEmpty()) {
      const auto title  = obj.value("title").toString();
      const auto desc   = obj.value("description").toString();
      const auto author = obj.value("author").toString();

      const bool matches = title.contains(m_searchFilter, Qt::CaseInsensitive)
                        || desc.contains(m_searchFilter, Qt::CaseInsensitive)
                        || author.contains(m_searchFilter, Qt::CaseInsensitive);

      if (!matches)
        continue;
    }

    auto map      = obj.toVariantMap();
    const auto id = obj.value("id").toString();
    map.insert("installed", isInstalled(id));
    map.insert("updateAvailable", hasUpdate(id));
    map.insert("installedVersion", installedVersion(id));
    map.insert("pluginRunning", isPluginRunning(id));

    // Check platform compatibility for plugins with platform overrides
    const auto plats = obj.value("platforms").toObject();
    if (!plats.isEmpty()) {
      const auto key = currentPlatformKey();
      const auto os  = key.left(key.indexOf('/'));
      const bool compatible =
        plats.contains(key) || plats.contains(os + "/*") || plats.contains("*");
      map.insert("platformAvailable", compatible);
    } else {
      map.insert("platformAvailable", true);
    }

    m_filteredExtensions.append(map);
  }

  // Include orphaned installed extensions (repo removed but still installed)
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

    // Build a minimal entry from installed metadata + info.json on disk
    QVariantMap map;
    map.insert("id", id);
    map.insert("type", type);
    map.insert("version", info.value("version").toString());
    map.insert("installed", true);
    map.insert("updateAvailable", false);
    map.insert("installedVersion", info.value("version").toString());
    map.insert("pluginRunning", isPluginRunning(id));

    // Try to read title/description from the installed info.json
    const auto addonJsonPath = extensionsPath() + "/" + type + "/" + id + "/info.json";
    QFile addonFile(addonJsonPath);
    if (addonFile.open(QIODevice::ReadOnly)) {
      const auto addonObj = QJsonDocument::fromJson(addonFile.readAll()).object();
      map.insert("title", addonObj.value("title").toString(id));
      map.insert("description", addonObj.value("description").toString());
      map.insert("author", addonObj.value("author").toString());
      map.insert("license", addonObj.value("license").toString());
      map.insert("category", addonObj.value("category").toString());
    } else {
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

  // Sort by type: plugins first, then themes, parsers, templates
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

  // Try to preserve current selection by matching extension ID
  const auto previousAddon = selectedExtension();
  const auto previousId    = previousAddon.value("id").toString();
  m_selectedIndex          = -1;

  if (!previousId.isEmpty()) {
    for (int i = 0; i < m_filteredExtensions.count(); ++i) {
      if (m_filteredExtensions.at(i).toMap().value("id").toString() == previousId) {
        m_selectedIndex = i;
        break;
      }
    }
  }

  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExtensionsChanged();
}

/**
 * @brief Rebuilds the installed plugins list for the start menu / toolbar.
 *
 * Uses a metadata cache to avoid re-reading info.json from disk on every call.
 * Only emits installedPluginsChanged() if the list actually changed.
 */
void Misc::ExtensionManager::rebuildInstalledPlugins()
{
  QVariantList plugins;
  const auto pluginIds = m_installedExtensions.keys();
  for (const auto& iid : pluginIds)
  {
    const auto info = m_installedExtensions.value(iid).toObject();
    if (info.value("type").toString() != QStringLiteral("plugin"))
      continue;

    // Check metadata cache before reading from disk
    QVariantMap entry;
    entry.insert("id", iid);
    entry.insert("running", isPluginRunning(iid));

    auto cacheIt = m_pluginMetadataCache.find(iid);
    if (cacheIt != m_pluginMetadataCache.end())
    {
      entry.insert("title", cacheIt->value("title"));
      const auto icon = cacheIt->value("icon").toString();
      if (!icon.isEmpty())
        entry.insert("icon", icon);
    }
    else
    {
      const auto pluginDir = extensionsPath() + "/plugin/" + iid;
      QVariantMap cached;

      QFile metaFile(pluginDir + "/info.json");
      if (metaFile.open(QIODevice::ReadOnly))
      {
        const auto meta = QJsonDocument::fromJson(metaFile.readAll()).object();
        const auto title = meta.value("title").toString(iid);
        entry.insert("title", title);
        cached.insert("title", title);

        const auto icon = meta.value("icon").toString();
        if (!icon.isEmpty())
        {
          const auto iconUrl =
            QStringLiteral("file://") + pluginDir + "/" + icon;
          entry.insert("icon", iconUrl);
          cached.insert("icon", iconUrl);
        }
      }
      else
      {
        entry.insert("title", iid);
        cached.insert("title", iid);
      }

      m_pluginMetadataCache.insert(iid, cached);
    }

    plugins.append(entry);
  }

  if (plugins != m_installedPlugins)
  {
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
 *
 * Format is "os/arch" where os is one of "darwin", "windows", "linux"
 * and arch is one of "arm64", "x86_64".
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
 *
 * If the metadata contains a "platforms" object, the function looks for the
 * current platform key (e.g. "darwin/arm64"), then falls back to a wildcard
 * key (e.g. "darwin/\*"), and finally returns the top-level fields.
 *
 * The returned object always contains "entry", "runtime", "terminal", and
 * "files" keys — merging the platform override on top of the base metadata.
 *
 * @param meta The parsed info.json content.
 * @return Resolved metadata object with platform-appropriate values.
 */
QJsonObject Misc::ExtensionManager::resolvePlatform(const QJsonObject& meta) const
{
  auto result = meta;

  const auto platforms = meta.value("platforms").toObject();
  if (platforms.isEmpty())
    return result;

  const auto key = currentPlatformKey();
  const auto os  = key.left(key.indexOf('/'));

  // Try exact match first, then wildcard
  QJsonObject override;
  if (platforms.contains(key))
    override = platforms.value(key).toObject();
  else if (platforms.contains(os + "/*"))
    override = platforms.value(os + "/*").toObject();
  else if (platforms.contains("*"))
    override = platforms.value("*").toObject();

  if (override.isEmpty())
    return result;

  // Merge platform overrides on top of base fields
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
 * @brief Convenience — launches the currently selected addon if it's a plugin.
 */
void Misc::ExtensionManager::launchSelectedPlugin()
{
  const auto addon = selectedExtension();
  launchPlugin(addon.value("id").toString());
}

/**
 * @brief Convenience — stops the currently selected addon if it's a running plugin.
 */
void Misc::ExtensionManager::stopSelectedPlugin()
{
  const auto addon = selectedExtension();
  stopPlugin(addon.value("id").toString());
}

/**
 * @brief Launches an installed plugin by ID.
 *
 * Reads the plugin's metadata to find the entry point script, then starts
 * it as a child process. The plugin is expected to connect to the API server
 * on port 7777 to interact with Serial Studio.
 */
void Misc::ExtensionManager::launchPlugin(const QString& id)
{
  if (id.isEmpty())
    return;

  if (isPluginRunning(id)) {
    m_pluginOutput[id] += QStringLiteral("[Already running]\n");
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  if (!isInstalled(id)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Plugin is not installed\n");
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  // Read plugin metadata
  const auto info = m_installedExtensions.value(id).toObject();
  const auto type = info.value("type").toString();
  if (type != QStringLiteral("plugin")) {
    m_pluginOutput[id] += QStringLiteral("[Error] Not a plugin (type: %1)\n").arg(type);
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  // Find the entry point from info.json
  const auto pluginDir = extensionsPath() + "/plugin/" + id;
  QFile metaFile(pluginDir + "/info.json");
  if (!metaFile.open(QIODevice::ReadOnly)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Cannot read %1/info.json\n").arg(pluginDir);
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  const auto metaDoc  = QJsonDocument::fromJson(metaFile.readAll());
  const auto resolved = resolvePlatform(metaDoc.object());
  const auto entry    = resolved.value("entry").toString();
  const auto runtime  = resolved.value("runtime").toString("python3");
  const auto terminal = resolved.value("terminal").toBool(false);
  const auto usesGrpc = resolved.value("grpc").toBool(false);

  // Check gRPC build support for plugins that require it
#ifndef ENABLE_GRPC
  if (usesGrpc)
  {
    m_pluginOutput[id] += QStringLiteral(
      "[Error] Plugin requires gRPC but this build does not include gRPC support.\n");
    Q_EMIT pluginOutputChanged(id);
    return;
  }
#endif

  // Enable the API server if not already running (gRPC follows automatically)
  if (!API::Server::instance().enabled())
  {
    const auto msg = usesGrpc
      ? tr("This plugin uses gRPC for high-performance data streaming. "
           "The API server needs to be enabled.\n\n"
           "Would you like to enable it now?")
      : tr("Plugins need the API server to communicate with Serial Studio. "
           "Would you like to enable it now?");

    const auto result = Misc::Utilities::showMessageBox(
      tr("API Server Required"), msg,
      QMessageBox::Question, QString(),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (result == QMessageBox::Yes)
      API::Server::instance().setEnabled(true);
    else
    {
      m_pluginOutput[id] += QStringLiteral("[Cancelled] API Server not enabled.\n");
      Q_EMIT pluginOutputChanged(id);
      return;
    }
  }

  if (entry.isEmpty()) {
    m_pluginOutput[id] += QStringLiteral("[Error] No 'entry' field in info.json\n");
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  const auto entryPath = pluginDir + "/" + entry;
  if (!QFile::exists(entryPath)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Entry point not found: %1\n").arg(entryPath);
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  // Validate entry point stays within plugin directory
  if (!isPathSafe(entryPath, pluginDir)) {
    m_pluginOutput[id] += QStringLiteral("[Error] Invalid entry point path\n");
    Q_EMIT pluginOutputChanged(id);
    return;
  }

  // Check plugin dependencies
  const auto deps = resolved.value("dependencies").toArray();
  for (const auto &dep : deps)
  {
    const auto obj = dep.toObject();
    const auto name = obj.value("name").toString();
    const auto exes = obj.value("executables").toArray();
    const auto url = obj.value("url").toString();

    // Skip pip-managed dependencies — handled by run.sh/run.cmd venv
    if (obj.contains("pip"))
      continue;

    // Skip dependencies with no executables to check
    if (exes.isEmpty())
      continue;

    // Look for any of the listed executables
    bool found = false;
    for (const auto &exe : exes)
    {
      if (!QStandardPaths::findExecutable(exe.toString()).isEmpty())
      {
        found = true;
        break;
      }
    }

    // Prompt user to download the missing dependency
    if (!found)
    {
      const auto result = Misc::Utilities::showMessageBox(
        tr("Missing Dependency"),
        tr("This plugin requires \"%1\" but it was not found on your "
           "system.\n\nWould you like to open the download page?")
          .arg(name),
        QMessageBox::Warning, QString(),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

      if (result == QMessageBox::Yes && !url.isEmpty())
        QDesktopServices::openUrl(QUrl(url));

      m_pluginOutput[id] +=
        QStringLiteral("[Error] Missing dependency: %1\n").arg(name);
      Q_EMIT pluginOutputChanged(id);
      return;
    }
  }

  // Build environment with common tool paths
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

  // Launch the plugin process
  auto* process = new QProcess(this);
  process->setWorkingDirectory(pluginDir);
  process->setProcessEnvironment(env);

  if (terminal)
    process->setProcessChannelMode(QProcess::ForwardedChannels);
  else {
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->setStandardInputFile(QProcess::nullDevice());
  }

  // Capture output for the log view
  const auto pluginId = id;
  m_pluginOutput.insert(id, QString());

  connect(process, &QProcess::readyRead, this, [this, pluginId, process]() {
    const auto text = QString::fromUtf8(process->readAll());
    m_pluginOutput[pluginId] += text;

    // Keep output buffer reasonable (last 64KB)
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
    const auto msg = QStringLiteral("[Error] Plugin failed to start or crashed.\n");
    m_pluginOutput[pluginId] += msg;
    Q_EMIT pluginOutputChanged(pluginId);
  });

  // Native executables need execute permission on Unix
  const bool isNative = runtime.isEmpty();
#ifndef Q_OS_WIN
  if (isNative)
    QFile::setPermissions(
      entryPath, QFile::permissions(entryPath) | QFileDevice::ExeUser | QFileDevice::ExeGroup);
#endif

  // Launch via system terminal or directly
  if (terminal) {
#ifdef Q_OS_MACOS
    process->start("open", {"-a", "Terminal", entryPath});
#elif defined(Q_OS_WIN)
    if (isNative)
      process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", entryPath});
    else
      process->start("cmd.exe", {"/c", "start", "cmd.exe", "/k", runtime, entryPath});
#else
    const QStringList terms = {
      "x-terminal-emulator", "gnome-terminal", "konsole", "xfce4-terminal", "xterm"};
    bool launched = false;
    for (const auto& term : terms) {
      if (QStandardPaths::findExecutable(term).isEmpty())
        continue;

      if (isNative)
        process->start(term, {"--", entryPath});
      else
        process->start(term, {"--", runtime, entryPath});

      launched = true;
      break;
    }

    if (!launched) {
      if (isNative)
        process->start(entryPath);
      else
        process->start(runtime, {entryPath});
    }
#endif
  } else {
    if (isNative)
      process->start(entryPath);
    else
      process->start(runtime, {entryPath});
  }

  if (!process->waitForStarted(3000)) {
    m_pluginOutput[id] +=
      QStringLiteral("[Error] Failed to start: ") + process->errorString() + "\n";
    Q_EMIT pluginOutputChanged(id);
    delete process;
    return;
  }

  const auto mode = terminal ? QStringLiteral(" (terminal)") : QString();
  m_pluginOutput[id] +=
    QStringLiteral("[Started] PID ") + QString::number(process->processId()) + mode + "\n";
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

  auto* process = it.value();
  m_pluginOutput[id] += QStringLiteral("[Stopping...]\n");
  Q_EMIT pluginOutputChanged(id);

  // Disconnect all signals to prevent onPluginFinished from firing
  process->disconnect(this);

  // Remove from map before terminating to avoid double-free
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
 *
 * Saves the list of running plugin IDs to QSettings so they can be
 * restored on the next launch.
 */
void Misc::ExtensionManager::stopAllPlugins()
{
  // Save running plugin IDs before stopping
  const auto ids = m_plugins.keys();
  m_settings.setValue("RunningPlugins", QStringList(ids));

  // Inline stop logic to avoid O(n*m) applyFilter() calls from stopPlugin()
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
 *
 * Reads the saved plugin ID list from QSettings and launches each one.
 * Called after the extension catalog is loaded and plugins are installed.
 */
void Misc::ExtensionManager::restoreRunningPlugins()
{
  // Wait for catalog to finish loading before restoring
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
    if (isInstalled(id) && !isPluginRunning(id))
      launchPlugin(id);
}

/**
 * @brief Handles plugin process termination.
 */
void Misc::ExtensionManager::onPluginFinished(const QString& id)
{
  auto it = m_plugins.find(id);
  if (it == m_plugins.end())
    return;

  auto* process        = it.value();
  const auto remaining = QString::fromUtf8(process->readAll());
  if (!remaining.isEmpty())
    m_pluginOutput[id] += remaining;

  const auto exitCode = process->exitCode();
  m_pluginOutput[id] += QStringLiteral("[Exited with code %1]\n").arg(exitCode);
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
 *
 * Supports both direct paths to manifest.json and directory paths
 * (in which case manifest.json is appended automatically).
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
    } else if (entry.isObject()) {
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

  // Validate destination stays within install directory
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
 *
 * Prevents path traversal attacks (e.g. "../../etc/passwd") from manifest data.
 *
 * @param filePath The resolved absolute path to validate.
 * @param baseDir  The directory the path must stay within.
 * @return True if the path is safe, false if it escapes baseDir.
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
