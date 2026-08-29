/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;
class QNetworkAccessManager;

namespace Misc {
class WorkspaceManager;

/**
 * @brief Owns everything an extension install touches on disk: the download queue, the files it
 *        writes, and installed.json -- the record of what is installed and at which version.
 *        ExtensionManager owns the catalog (what exists to install) and asks here for the install
 *        state; that is the split. Nothing here reads the catalog, and nothing there writes a file.
 */
class ExtensionInstaller : public QObject {
  Q_OBJECT

signals:
  void busyChanged();
  void progressChanged();
  void installed(const QString& id);

public:
  explicit ExtensionInstaller(QNetworkAccessManager& network,
                              WorkspaceManager& workspace,
                              QObject* parent = nullptr);

  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] float progress() const noexcept;
  [[nodiscard]] bool isInstalled(const QString& id) const;
  [[nodiscard]] QStringList installedIds() const;
  [[nodiscard]] QString extensionsPath() const;
  [[nodiscard]] QString installedVersion(const QString& id) const;
  [[nodiscard]] QJsonObject installedInfo(const QString& id) const;

  [[nodiscard]] bool install(const QVariantMap& entry);
  [[nodiscard]] bool uninstall(const QString& id, const QString& type);

  void reload();
  void removeAll();

private slots:
  void downloadNextFile();
  void onFileDownloadReply();

private:
  void finishInstall();
  void loadInstalledManifest();
  void saveInstalledManifest();
  void writeExtensionFile(QNetworkReply* reply);
  void startDownloads(const QVariantMap& entry, const QVariantList& files);
  void copyLocalFiles(const QVariantList& files, const QString& base, const QString& installDir);

  [[nodiscard]] QString installedManifestPath() const;
  [[nodiscard]] bool installLocal(const QVariantMap& entry,
                                  const QVariantList& files,
                                  const QString& installDir);

private:
  bool m_busy;
  float m_progress;
  int m_pendingDownloads;
  int m_totalDownloads;

  QString m_currentInstallId;
  QString m_currentInstallRepoBase;
  QVariantMap m_currentInstallMeta;
  QJsonObject m_installedExtensions;
  QList<QPair<QString, QUrl>> m_downloadQueue;
  QSet<QNetworkReply*> m_replies;

  QNetworkAccessManager& m_network;
  WorkspaceManager& m_workspace;
};
}  // namespace Misc
