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

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QVariantList>
#include <QVariantMap>

#include "Misc/Extensions/ExtensionAutoUpdater.h"
#include "Misc/Extensions/ExtensionInstaller.h"
#include "Misc/Extensions/PluginRunner.h"
#include "Misc/ProblemCenter.h"

class QNetworkReply;

namespace Misc {
/**
 * @brief Owns the extension catalog: the repositories it is fetched from, the filtered view QML
 *        renders, and the launch decisions for installed plugins. Disk writes belong to
 *        ExtensionInstaller, running processes to PluginRunner and update policy to
 *        ExtensionAutoUpdater; this class holds those three and forwards their property surface.
 */
class ExtensionManager : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool loading
             READ loading
             NOTIFY loadingChanged)
  Q_PROPERTY(int count
             READ count
             NOTIFY filteredExtensionsChanged)
  Q_PROPERTY(int selectedIndex
             READ selectedIndex
             WRITE setSelectedIndex
             NOTIFY selectedIndexChanged)
  Q_PROPERTY(QString searchFilter
             READ searchFilter
             WRITE setSearchFilter
             NOTIFY searchFilterChanged)
  Q_PROPERTY(QString filterCategory
             READ filterCategory
             WRITE setFilterCategory
             NOTIFY filterCategoryChanged)
  Q_PROPERTY(QVariantList extensions
             READ extensions
             NOTIFY filteredExtensionsChanged)
  Q_PROPERTY(QVariantMap selectedExtension
             READ selectedExtension
             NOTIFY selectedIndexChanged)
  Q_PROPERTY(QStringList repositories
             READ repositories
             NOTIFY repositoriesChanged)
  Q_PROPERTY(QString filterType
             READ filterType
             WRITE setFilterType
             NOTIFY filterTypeChanged)
  Q_PROPERTY(QVariantList runningPlugins
             READ runningPlugins
             NOTIFY runningPluginsChanged)
  Q_PROPERTY(QString selectedReadme
             READ selectedReadme
             NOTIFY selectedReadmeChanged)
  Q_PROPERTY(float downloadProgress
             READ downloadProgress
             NOTIFY downloadProgressChanged)
  Q_PROPERTY(QVariantList installedPlugins
             READ installedPlugins
             NOTIFY installedPluginsChanged)
  Q_PROPERTY(bool updateCheckEnabled
             READ updateCheckEnabled
             WRITE setUpdateCheckEnabled
             NOTIFY updatePolicyChanged)
  Q_PROPERTY(bool automaticUpdates
             READ automaticUpdates
             WRITE setAutomaticUpdates
             NOTIFY updatePolicyChanged)
  // clang-format on

signals:
  void loadingChanged();
  void searchFilterChanged();
  void filterCategoryChanged();
  void filterTypeChanged();
  void installedPluginsChanged();
  void selectedIndexChanged();
  void filteredExtensionsChanged();
  void repositoriesChanged();
  void downloadProgressChanged();
  void runningPluginsChanged();
  void selectedReadmeChanged();
  void updatePolicyChanged();
  void pluginOutputChanged(const QString& id);
  void extensionInstalled(const QString& id);
  void extensionUninstalled(const QString& id);

private:
  explicit ExtensionManager();
  ExtensionManager(ExtensionManager&&)                 = delete;
  ExtensionManager(const ExtensionManager&)            = delete;
  ExtensionManager& operator=(ExtensionManager&&)      = delete;
  ExtensionManager& operator=(const ExtensionManager&) = delete;

public:
  [[nodiscard]] static ExtensionManager& instance();

  [[nodiscard]] bool loading() const noexcept;
  [[nodiscard]] bool updateCheckEnabled() const noexcept;
  [[nodiscard]] bool automaticUpdates() const noexcept;
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int selectedIndex() const noexcept;
  [[nodiscard]] float downloadProgress() const noexcept;
  [[nodiscard]] const QString& searchFilter() const noexcept;
  [[nodiscard]] const QString& filterCategory() const noexcept;
  [[nodiscard]] const QString& filterType() const noexcept;
  [[nodiscard]] const QStringList& repositories() const noexcept;
  [[nodiscard]] const QVariantList& extensions() const noexcept;
  [[nodiscard]] const QString& selectedReadme() const noexcept;
  [[nodiscard]] QVariantMap selectedExtension() const;

  Q_INVOKABLE [[nodiscard]] bool isInstalled(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] bool hasUpdate(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] bool isLocalRepo(const QString& url) const;
  Q_INVOKABLE [[nodiscard]] QString installedVersion(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QStringList extensionTypes() const;
  Q_INVOKABLE [[nodiscard]] QString friendlyTypeName(const QString& type) const;
  Q_INVOKABLE [[nodiscard]] bool isPluginRunning(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QString pluginOutput(const QString& id) const;
  Q_INVOKABLE [[nodiscard]] QString platformKey() const;
  [[nodiscard]] const QVariantList& runningPlugins() const noexcept;
  [[nodiscard]] const QVariantList& installedPlugins() const noexcept;

public slots:
  void refreshRepositories();
  void checkForUpdatesOnStartup(const bool appUpdatesEnabled);
  void setUpdateCheckEnabled(const bool enabled);
  void setAutomaticUpdates(const bool enabled);
  void setSelectedIndex(int index);
  void setSearchFilter(const QString& filter);
  void setFilterCategory(const QString& category);
  void setFilterType(const QString& type);
  void installExtension();
  bool uninstallExtension();
  void addRepository(const QString& url);
  void resetRepositories();
  void removeRepository(int index);
  void browseLocalRepo();
  void launchPlugin(const QString& id);
  void launchSelectedPlugin();
  void stopPlugin(const QString& id);
  void stopSelectedPlugin();
  void stopAllPlugins();
  void restoreRunningPlugins();
  void onDashboardAvailableChanged();
  void onWorkspacePathChanged();

private slots:
  void autoUpdateExtensions();
  void onManifestReply();
  void onExtensionMetaReply();
  void onReadmeReply();
  void applyFilter();
  void rebuildInstalledPlugins();
  void loadLocalManifest(const QString& repoPath);
  void onExtensionInstalled(const QString& id);

private:
  void parseManifest(QNetworkReply* reply);
  void collectCatalogFindings(QList<ProblemCenter::Finding>& findings) const;
  [[nodiscard]] QVariantMap loadPluginMetadata(const QString& iid);

  [[nodiscard]] QString catalogName(const QString& id) const;
  [[nodiscard]] bool confirmAutoUpdate(const QStringList& ids);
  [[nodiscard]] bool queuePendingUpdates();

  void appendOrphanedInstalledEntries();
  void restoreSelectionByPreviousId();

  [[nodiscard]] bool checkLaunchPreconditions(const QString& id);
  [[nodiscard]] bool readPluginMetadata(const QString& id,
                                        const QString& pluginDir,
                                        QJsonObject& resolvedOut);
  [[nodiscard]] bool ensureApiServerForLaunch(const QString& id, bool usesGrpc);
  [[nodiscard]] bool resolveAndValidateEntry(const QString& id,
                                             const QString& pluginDir,
                                             const QString& entry,
                                             QString& entryPathOut);
  [[nodiscard]] bool checkPluginDependencies(const QString& id,
                                             const QJsonArray& deps,
                                             bool& hasPipDepsOut);

private:
  bool m_loading;
  bool m_dashboardWasAvailable;
  int m_selectedIndex;
  int m_pendingManifests;
  int m_pendingExtensionMetas;

  QString m_filterType;
  QString m_searchFilter;
  QString m_selectedReadme;
  QString m_filterCategory;

  QStringList m_repositories;
  QJsonArray m_allExtensions;
  QVariantList m_installedPlugins;
  QVariantList m_filteredExtensions;
  QMap<QString, QVariantMap> m_pluginMetadataCache;

  QSettings m_settings;
  QNetworkAccessManager m_nam;
  QSet<QNetworkReply*> m_activeReplies;

  PluginRunner m_pluginRunner;
  ExtensionInstaller m_installer;
  ExtensionAutoUpdater m_autoUpdater;
};
}  // namespace Misc
