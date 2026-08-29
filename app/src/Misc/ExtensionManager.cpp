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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/ExtensionManager.h"

#include <algorithm>
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
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

#include "API/Server.h"
#include "Misc/Extensions/ExtensionCatalog.h"
#include "Misc/JsonValidator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "UI/Dashboard.h"

//--------------------------------------------------------------------------------------------------
// Default repository URL
//--------------------------------------------------------------------------------------------------

static const QString kDefaultRepoUrl =
  QStringLiteral("https://raw.githubusercontent.com/serial-studio/extensions/main/manifest.json");

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ExtensionManager singleton. The catalog view and the installed-plugin
 *        list are derived from the runner's process list and from the installer's manifest, so
 *        they are refreshed from those objects' signals rather than at each of their call sites.
 */
Misc::ExtensionManager::ExtensionManager()
  : m_loading(false)
  , m_dashboardWasAvailable(false)
  , m_selectedIndex(-1)
  , m_pendingManifests(0)
  , m_pendingExtensionMetas(0)
  , m_installer(m_nam, Misc::WorkspaceManager::instance())
  , m_autoUpdater(m_settings)
{
  m_nam.setTransferTimeout(15 * 1000);

  connect(&m_pluginRunner, &PluginRunner::runningChanged, this, [this] {
    Q_EMIT runningPluginsChanged();
    applyFilter();
    rebuildInstalledPlugins();
  });
  connect(
    &m_pluginRunner, &PluginRunner::outputChanged, this, &ExtensionManager::pluginOutputChanged);

  connect(&m_installer, &ExtensionInstaller::busyChanged, this, &ExtensionManager::loadingChanged);
  connect(&m_installer,
          &ExtensionInstaller::progressChanged,
          this,
          &ExtensionManager::downloadProgressChanged);
  connect(
    &m_installer, &ExtensionInstaller::installed, this, &ExtensionManager::onExtensionInstalled);

  const auto saved = m_settings.value("ExtensionRepositories").toStringList();
  if (saved.isEmpty())
    m_repositories.append(kDefaultRepoUrl);
  else
    m_repositories = saved;

  m_installer.reload();
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
 * @brief Returns whether a network operation is in progress; a catalog refresh and an install
 *        both raise the same flag, because the dialog blocks on either.
 */
bool Misc::ExtensionManager::loading() const noexcept
{
  return m_loading || m_installer.busy();
}

/**
 * @brief Returns whether installed extensions are checked for updates.
 */
bool Misc::ExtensionManager::updateCheckEnabled() const noexcept
{
  return m_autoUpdater.checkEnabled();
}

/**
 * @brief Returns whether available updates are installed without asking the user.
 */
bool Misc::ExtensionManager::automaticUpdates() const noexcept
{
  return m_autoUpdater.automaticUpdates();
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
  return m_installer.progress();
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
  return ExtensionCatalog::isLocalRepo(url);
}

/**
 * @brief Returns the current platform key (e.g. "darwin/arm64") for QML display.
 */
QString Misc::ExtensionManager::platformKey() const
{
  return ExtensionCatalog::currentPlatformKey();
}

/**
 * @brief Returns the list of supported extension types for filtering.
 */
QStringList Misc::ExtensionManager::extensionTypes() const
{
  return ExtensionCatalog::extensionTypes();
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

  if (type == QStringLiteral("widget"))
    return tr("Widget");

  if (type == QStringLiteral("All"))
    return tr("All Types");

  return type;
}

/**
 * @brief Returns whether the extension with the given ID is installed locally.
 */
bool Misc::ExtensionManager::isInstalled(const QString& id) const
{
  return m_installer.isInstalled(id);
}

/**
 * @brief Returns whether a newer version is available for the given addon.
 */
bool Misc::ExtensionManager::hasUpdate(const QString& id) const
{
  if (!isInstalled(id))
    return false;

  const auto localVer = m_installer.installedVersion(id);
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
  return m_installer.installedVersion(id);
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
  if (loading())
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
  m_installer.removeAll();

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
  if (loading())
    return;

  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExtensions.count())
    return;

  (void)m_installer.install(m_filteredExtensions.at(m_selectedIndex).toMap());
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

  const bool removed = m_installer.uninstall(id, type);
  m_pluginMetadataCache.remove(id);

  Q_EMIT extensionUninstalled(id);
  applyFilter();
  rebuildInstalledPlugins();

  return removed;
}

/**
 * @brief Republishes a finished install: the catalog entry, the plugin metadata cache and the
 *        installed-plugin list all become stale the moment the installer records it.
 */
void Misc::ExtensionManager::onExtensionInstalled(const QString& id)
{
  Q_EMIT extensionInstalled(id);
  m_pluginMetadataCache.remove(id);
  applyFilter();
  rebuildInstalledPlugins();
}

//--------------------------------------------------------------------------------------------------
// Auto-update
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes the catalog at startup so installed extensions are checked for updates; runs
 * only when the application's own update checks are enabled too.
 */
void Misc::ExtensionManager::checkForUpdatesOnStartup(const bool appUpdatesEnabled)
{
  if (!appUpdatesEnabled || !updateCheckEnabled())
    return;

  refreshRepositories();
}

/**
 * @brief Enables/disables update checks for installed extensions and persists the choice.
 */
void Misc::ExtensionManager::setUpdateCheckEnabled(const bool enabled)
{
  if (!m_autoUpdater.setCheckEnabled(enabled))
    return;

  Q_EMIT updatePolicyChanged();

  if (enabled)
    QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
}

/**
 * @brief Enables/disables silent installation of available extension updates.
 */
void Misc::ExtensionManager::setAutomaticUpdates(const bool enabled)
{
  if (!m_autoUpdater.setAutomaticUpdates(enabled))
    return;

  Q_EMIT updatePolicyChanged();

  if (enabled)
    QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
}

/**
 * @brief Returns the catalog display name for the given addon ID, or the ID itself.
 */
QString Misc::ExtensionManager::catalogName(const QString& id) const
{
  for (const auto& entry : std::as_const(m_allExtensions)) {
    const auto obj = entry.toObject();
    if (obj.value("id").toString() != id)
      continue;

    const auto name = obj.value("name").toString();
    return name.isEmpty() ? id : name;
  }

  return id;
}

/**
 * @brief Asks the user whether the pending extension updates should be installed; a remembered
 * "always update" choice skips the prompt on later refreshes.
 */
bool Misc::ExtensionManager::confirmAutoUpdate(const QStringList& ids)
{
  if (m_autoUpdater.automaticUpdates())
    return true;

  if (!m_autoUpdater.checkEnabled())
    return false;

  QStringList names;
  for (const auto& id : ids)
    names.append(catalogName(id));

  const auto answer = Misc::Utilities::showMessageBox(
    tr("Extension updates available"),
    tr("Newer versions are available for: %1.\n\nDo you want to update them now?")
      .arg(names.join(QStringLiteral(", "))),
    QMessageBox::Question,
    qApp->applicationName(),
    QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No,
    QMessageBox::Yes,
    {
      {QMessageBox::YesToAll, tr("Always update")}
  });

  if (answer == QMessageBox::YesToAll) {
    m_autoUpdater.rememberAlways();
    Q_EMIT updatePolicyChanged();
    return true;
  }

  return answer != QMessageBox::No;
}

/**
 * @brief Collects the installed extensions a newer version exists for and, with the user's
 *        consent, queues them; a refusal is remembered so the next check does not ask again.
 */
bool Misc::ExtensionManager::queuePendingUpdates()
{
  QStringList pending;
  const auto ids = m_installer.installedIds();
  for (const auto& id : ids)
    if (hasUpdate(id) && !m_autoUpdater.declined(id))
      pending.append(id);

  if (pending.isEmpty())
    return false;

  if (!confirmAutoUpdate(pending)) {
    m_autoUpdater.decline(pending);
    return false;
  }

  m_autoUpdater.enqueue(pending);
  return true;
}

/**
 * @brief Installs the queued extension updates one at a time, chaining the next one on the
 *        install that finishes so two downloads never overlap.
 */
void Misc::ExtensionManager::autoUpdateExtensions()
{
  if (loading() || !updateCheckEnabled())
    return;

  if (!m_autoUpdater.hasPending() && !queuePendingUpdates())
    return;

  const auto id = m_autoUpdater.takeNext();
  if (id.isEmpty())
    return;

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

  if (!m_autoUpdater.hasPending())
    return;

  if (found && loading()) {
    connect(
      this,
      &ExtensionManager::extensionInstalled,
      this,
      [this]() { QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions); },
      Qt::SingleShotConnection);
    return;
  }

  QTimer::singleShot(0, this, &ExtensionManager::autoUpdateExtensions);
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

//--------------------------------------------------------------------------------------------------
// Catalog view
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies search and category filters to rebuild the filtered list.
 */
void Misc::ExtensionManager::applyFilter()
{
  m_filteredExtensions.clear();

  const auto platform = ExtensionCatalog::currentPlatformKey();
  const ExtensionCatalog::CatalogFilter filter{m_filterType, m_filterCategory, m_searchFilter};

  for (const auto& entry : std::as_const(m_allExtensions)) {
    const auto obj = entry.toObject();
    if (!ExtensionCatalog::entryMatchesFilters(obj, filter))
      continue;

    const auto id = obj.value("id").toString();

    ExtensionCatalog::EntryState state;
    state.installed        = isInstalled(id);
    state.pluginRunning    = isPluginRunning(id);
    state.updateAvailable  = hasUpdate(id);
    state.installedVersion = installedVersion(id);

    m_filteredExtensions.append(ExtensionCatalog::buildEntryMap(obj, state, platform));
  }

  appendOrphanedInstalledEntries();

  std::stable_sort(m_filteredExtensions.begin(),
                   m_filteredExtensions.end(),
                   [](const QVariant& a, const QVariant& b) {
                     const auto ta = a.toMap().value("type").toString();
                     const auto tb = b.toMap().value("type").toString();
                     return ExtensionCatalog::typeSortRank(ta) < ExtensionCatalog::typeSortRank(tb);
                   });

  restoreSelectionByPreviousId();

  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExtensionsChanged();
}

/**
 * @brief Appends entries for installed extensions whose source repo is no longer available.
 */
void Misc::ExtensionManager::appendOrphanedInstalledEntries()
{
  QSet<QString> catalogIds;
  for (const auto& entry : std::as_const(m_filteredExtensions))
    catalogIds.insert(entry.toMap().value("id").toString());

  const auto installedIds = m_installer.installedIds();
  for (const auto& id : installedIds) {
    if (catalogIds.contains(id))
      continue;

    const auto info = m_installer.installedInfo(id);
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

    const auto addonJsonPath = m_installer.extensionsPath() + "/" + type + "/" + id + "/info.json";
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

    if (!ExtensionCatalog::matchesSearch(
          map.value("title").toString(), map.value("description").toString(), m_searchFilter))
      continue;

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

  const auto pluginDir = m_installer.extensionsPath() + "/plugin/" + iid;
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
  const auto pluginIds = m_installer.installedIds();
  for (const auto& iid : pluginIds) {
    const auto info = m_installer.installedInfo(iid);
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

//--------------------------------------------------------------------------------------------------
// Plugin management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list of currently running plugins.
 */
const QVariantList& Misc::ExtensionManager::runningPlugins() const noexcept
{
  return m_pluginRunner.running();
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
  return m_pluginRunner.isRunning(id);
}

/**
 * @brief Returns the captured stdout/stderr output for the given plugin.
 */
QString Misc::ExtensionManager::pluginOutput(const QString& id) const
{
  return m_pluginRunner.output(id);
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

  m_pluginRunner.clearUserClosed(id);

  if (!checkLaunchPreconditions(id))
    return;

  const auto pluginDir = m_installer.extensionsPath() + "/plugin/" + id;
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

  const auto title = resolved.value("title").toString(id);
  (void)m_pluginRunner.start(id, pluginDir, runtime, entryPath, title, terminal, hasPipDeps);
}

/**
 * @brief Verifies a plugin can be launched: not running, installed, and of type "plugin".
 */
bool Misc::ExtensionManager::checkLaunchPreconditions(const QString& id)
{
  if (isPluginRunning(id)) {
    m_pluginRunner.appendOutput(id, QStringLiteral("[Already running]\n"));
    return false;
  }

  if (!isInstalled(id)) {
    m_pluginRunner.appendOutput(id, QStringLiteral("[Error] Plugin is not installed\n"));
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"), tr("Plugin \"%1\" is not installed.").arg(id), QMessageBox::Critical);
    return false;
  }

  const auto info = m_installer.installedInfo(id);
  const auto type = info.value("type").toString();
  if (type != QStringLiteral("plugin")) {
    m_pluginRunner.appendOutput(id, QStringLiteral("[Error] Not a plugin (type: %1)\n").arg(type));
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
    m_pluginRunner.appendOutput(
      id, QStringLiteral("[Error] Cannot read %1/info.json\n").arg(pluginDir));
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"),
      tr("Cannot read plugin metadata file:\n%1/info.json").arg(pluginDir),
      QMessageBox::Critical);
    return false;
  }

  const auto metaDoc = QJsonDocument::fromJson(metaFile.readAll());
  resolvedOut =
    ExtensionCatalog::resolvePlatform(metaDoc.object(), ExtensionCatalog::currentPlatformKey());
  return true;
}

/**
 * @brief Verifies gRPC build support and prompts the user to enable the API server.
 */
bool Misc::ExtensionManager::ensureApiServerForLaunch(const QString& id, bool usesGrpc)
{
#ifndef ENABLE_GRPC
  if (usesGrpc) {
    m_pluginRunner.appendOutput(
      id,
      QStringLiteral(
        "[Error] Plugin requires gRPC but this build does not include gRPC support.\n"));
    Misc::Utilities::showMessageBox(
      tr("Plugin Error"),
      tr("Plugin \"%1\" requires gRPC but this build does not include gRPC support.").arg(id),
      QMessageBox::Critical);
    return false;
  }
#endif

  static auto& server = API::Server::instance();
  if (server.enabled())
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
    server.setEnabled(true);
    return true;
  }

  m_pluginRunner.appendOutput(id, QStringLiteral("[Cancelled] API Server not enabled.\n"));
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
    m_pluginRunner.appendOutput(id, QStringLiteral("[Error] No 'entry' field in info.json\n"));
    Misc::Utilities::showMessageBox(tr("Plugin Error"),
                                    tr("Plugin \"%1\" has no 'entry' field in info.json.").arg(id),
                                    QMessageBox::Critical);
    return false;
  }

  entryPathOut = pluginDir + "/" + entry;
  if (!QFile::exists(entryPathOut)) {
    m_pluginRunner.appendOutput(
      id, QStringLiteral("[Error] Entry point not found: %1\n").arg(entryPathOut));
    Misc::Utilities::showMessageBox(tr("Plugin Error"),
                                    tr("Entry point not found:\n%1").arg(entryPathOut),
                                    QMessageBox::Critical);
    return false;
  }

  if (!ExtensionCatalog::isPathSafe(entryPathOut, pluginDir)) {
    m_pluginRunner.appendOutput(id, QStringLiteral("[Error] Invalid entry point path\n"));
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

    m_pluginRunner.appendOutput(id, QStringLiteral("[Error] Missing dependency: %1\n").arg(name));
    return false;
  }

  return true;
}

/**
 * @brief Stops a running plugin by ID.
 */
void Misc::ExtensionManager::stopPlugin(const QString& id)
{
  m_pluginRunner.stop(id);
}

/**
 * @brief Stops all running plugins. Called on application shutdown.
 */
void Misc::ExtensionManager::stopAllPlugins()
{
  m_pluginRunner.stopAll();
}

/**
 * @brief Restores plugins that were running in the previous session. Waits for the catalog to
 *        finish loading first, because isInstalled() cannot answer until the manifest is in.
 */
void Misc::ExtensionManager::restoreRunningPlugins()
{
  if (loading()) {
    connect(
      this,
      &ExtensionManager::loadingChanged,
      this,
      [this]() {
        if (!loading())
          restoreRunningPlugins();
      },
      Qt::SingleShotConnection);
    return;
  }

  for (const auto& id : m_pluginRunner.restorableIds())
    if (isInstalled(id) && !isPluginRunning(id) && !m_pluginRunner.userClosed(id))
      launchPlugin(id);
}

/**
 * @brief Reacts to dashboard availability changes.
 */
void Misc::ExtensionManager::onDashboardAvailableChanged()
{
  static auto& dashboard = UI::Dashboard::instance();
  const bool available   = dashboard.available();

  if (available && !m_dashboardWasAvailable)
    restoreRunningPlugins();

  else if (!available && m_dashboardWasAvailable)
    stopAllPlugins();

  m_dashboardWasAvailable = available;
}

/**
 * @brief Reloads the installed-extension manifest and catalog after the user relocates the
 *        workspace directory, so isInstalled()/hasUpdate() no longer report the old folder's
 *        state against the new folder's paths.
 */
void Misc::ExtensionManager::onWorkspacePathChanged()
{
  m_installer.reload();
  applyFilter();
  rebuildInstalledPlugins();
}

//--------------------------------------------------------------------------------------------------
// Local repositories
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
