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

#include "Misc/Extensions/ExtensionInstaller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "Misc/Extensions/ExtensionCatalog.h"
#include "Misc/WorkspaceManager.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an idle installer. The installed manifest is not read here: the facade calls
 *        reload() at the point in its own construction where the workspace is known to exist.
 */
Misc::ExtensionInstaller::ExtensionInstaller(QNetworkAccessManager& network,
                                             WorkspaceManager& workspace,
                                             QObject* parent)
  : QObject(parent)
  , m_busy(false)
  , m_progress(0)
  , m_pendingDownloads(0)
  , m_totalDownloads(0)
  , m_network(network)
  , m_workspace(workspace)
{}

//--------------------------------------------------------------------------------------------------
// Install state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a download-driven install is in progress.
 */
bool Misc::ExtensionInstaller::busy() const noexcept
{
  return m_busy;
}

/**
 * @brief Returns the progress of the running install (0.0 to 1.0).
 */
float Misc::ExtensionInstaller::progress() const noexcept
{
  return m_progress;
}

/**
 * @brief Returns whether the extension with the given ID is recorded as installed.
 */
bool Misc::ExtensionInstaller::isInstalled(const QString& id) const
{
  return m_installedExtensions.contains(id);
}

/**
 * @brief Returns the IDs of every installed extension.
 */
QStringList Misc::ExtensionInstaller::installedIds() const
{
  return m_installedExtensions.keys();
}

/**
 * @brief Returns the recorded manifest entry of an installed extension, empty when absent.
 */
QJsonObject Misc::ExtensionInstaller::installedInfo(const QString& id) const
{
  return m_installedExtensions.value(id).toObject();
}

/**
 * @brief Returns the installed version string for the given extension, or empty.
 */
QString Misc::ExtensionInstaller::installedVersion(const QString& id) const
{
  return installedInfo(id).value("version").toString();
}

/**
 * @brief Returns the base extensions directory path.
 */
QString Misc::ExtensionInstaller::extensionsPath() const
{
  return m_workspace.path("Extensions");
}

/**
 * @brief Returns the path to the installed.json tracking file.
 */
QString Misc::ExtensionInstaller::installedManifestPath() const
{
  return extensionsPath() + "/installed.json";
}

//--------------------------------------------------------------------------------------------------
// Install & uninstall
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the extension described by @p entry, copying it from a local repository or
 *        starting the download queue for a remote one. Returns false when the entry names no
 *        files, or when its identity would escape the extensions directory.
 */
bool Misc::ExtensionInstaller::install(const QVariantMap& entry)
{
  const auto id      = entry.value("id").toString();
  const auto type    = entry.value("type").toString();
  const auto isLocal = entry.value("_isLocal").toBool();

  auto files           = entry.value("files").toList();
  const auto platforms = entry.value("platforms").toMap();
  const auto selection =
    ExtensionCatalog::selectPlatformOverride(platforms, ExtensionCatalog::currentPlatformKey());

  const auto platFiles = selection.value("files").toList();
  for (const auto& f : platFiles)
    if (!files.contains(f))
      files.append(f);

  if (id.isEmpty() || files.isEmpty())
    return false;

  if (!ExtensionCatalog::isSafePathComponent(id))
    return false;

  if (!ExtensionCatalog::isSafePathComponent(type))
    return false;

  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  QDir().mkpath(installDir);

  if (isLocal)
    return installLocal(entry, files, installDir);

  startDownloads(entry, files);
  return true;
}

/**
 * @brief Copies a local repository's files into the install directory and records the install.
 */
bool Misc::ExtensionInstaller::installLocal(const QVariantMap& entry,
                                            const QVariantList& files,
                                            const QString& installDir)
{
  const auto id   = entry.value("id").toString();
  const auto type = entry.value("type").toString();
  const auto base = entry.value("_repoBase").toString();
  SS_ASSERT(!id.isEmpty(), return false);

  copyLocalFiles(files, base, installDir);

  QJsonObject info;
  info.insert("version", entry.value("version").toString());
  info.insert("type", type);
  info.insert("repoBase", base);
  m_installedExtensions.insert(id, info);
  saveInstalledManifest();

  Q_EMIT installed(id);
  return true;
}

/**
 * @brief Copies each declared file into the install directory, skipping any whose resolved path
 *        would leave it.
 */
void Misc::ExtensionInstaller::copyLocalFiles(const QVariantList& files,
                                              const QString& base,
                                              const QString& installDir)
{
  for (const auto& f : files) {
    const auto localName = f.toString();
    const auto dst       = installDir + "/" + localName;

    if (!ExtensionCatalog::isPathSafe(dst, installDir))
      continue;

    const auto src = base + localName;
    QDir().mkpath(QFileInfo(dst).absolutePath());
    QFile::copy(src, dst);
  }
}

/**
 * @brief Queues every file of a remote install and starts the first download.
 */
void Misc::ExtensionInstaller::startDownloads(const QVariantMap& entry, const QVariantList& files)
{
  const auto base = entry.value("_repoBase").toString();

  m_downloadQueue.clear();
  for (const auto& f : files) {
    const auto localName = f.toString();
    m_downloadQueue.append({localName, ExtensionCatalog::resolveFileUrl(base, localName)});
  }

  m_currentInstallId       = entry.value("id").toString();
  m_currentInstallRepoBase = base;
  m_currentInstallMeta     = entry;
  m_busy                   = true;
  m_progress               = 0;
  m_totalDownloads         = m_downloadQueue.count();
  m_pendingDownloads       = m_totalDownloads;
  Q_EMIT busyChanged();
  Q_EMIT progressChanged();

  downloadNextFile();
}

/**
 * @brief Removes an installed extension's directory and its manifest record, returning false when
 *        the directory could only be deleted in part.
 */
bool Misc::ExtensionInstaller::uninstall(const QString& id, const QString& type)
{
  SS_ASSERT(!id.isEmpty(), return false);

  const auto installDir = extensionsPath() + "/" + type + "/" + id;
  const bool removed    = QDir(installDir).removeRecursively();

  m_installedExtensions.remove(id);
  saveInstalledManifest();

  return removed;
}

/**
 * @brief Deletes every installed extension and empties the manifest; the caller is responsible for
 *        stopping anything that runs out of those directories first.
 */
void Misc::ExtensionInstaller::removeAll()
{
  const auto ids = m_installedExtensions.keys();
  for (const auto& id : ids) {
    const auto info = installedInfo(id);
    const auto type = info.value("type").toString();
    const auto dir  = extensionsPath() + "/" + type + "/" + id;
    QDir(dir).removeRecursively();
  }

  m_installedExtensions = QJsonObject();
  saveInstalledManifest();
}

/**
 * @brief Re-reads installed.json, dropping what the previous workspace recorded.
 */
void Misc::ExtensionInstaller::reload()
{
  m_installedExtensions = QJsonObject();
  loadInstalledManifest();
}

//--------------------------------------------------------------------------------------------------
// Download pipeline
//--------------------------------------------------------------------------------------------------

/**
 * @brief Downloads the next file in the install queue.
 */
void Misc::ExtensionInstaller::downloadNextFile()
{
  if (m_downloadQueue.isEmpty())
    return;

  const auto [localName, url] = m_downloadQueue.takeFirst();
  auto* reply                 = m_network.get(QNetworkRequest(url));
  SS_ASSERT(reply != nullptr, return);

  reply->setProperty("localName", localName);
  m_replies.insert(reply);
  connect(reply, &QNetworkReply::finished, this, &ExtensionInstaller::onFileDownloadReply);
}

/**
 * @brief Handles individual file download completion during an install.
 */
void Misc::ExtensionInstaller::onFileDownloadReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_replies.remove(reply);
  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError)
    writeExtensionFile(reply);

  --m_pendingDownloads;
  m_progress = (m_totalDownloads > 0) ? static_cast<float>(m_totalDownloads - m_pendingDownloads)
                                          / static_cast<float>(m_totalDownloads)
                                      : 1.0f;
  Q_EMIT progressChanged();

  if (!m_downloadQueue.isEmpty()) {
    downloadNextFile();
    return;
  }

  finishInstall();
}

/**
 * @brief Records the finished install and announces it; the file list is stored so a later
 *        uninstall knows what the install brought in.
 */
void Misc::ExtensionInstaller::finishInstall()
{
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

  m_busy = false;
  Q_EMIT busyChanged();
  Q_EMIT installed(m_currentInstallId);
}

//--------------------------------------------------------------------------------------------------
// Manifest & file I/O
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes a downloaded extension file to the install directory, refusing any path that
 *        would leave it.
 */
void Misc::ExtensionInstaller::writeExtensionFile(QNetworkReply* reply)
{
  SS_ASSERT(reply != nullptr, return);

  const auto localName  = reply->property("localName").toString();
  const auto type       = m_currentInstallMeta.value("type").toString();
  const auto installDir = extensionsPath() + "/" + type + "/" + m_currentInstallId;
  const auto filePath   = installDir + "/" + localName;

  if (!ExtensionCatalog::isPathSafe(filePath, installDir))
    return;

  QDir().mkpath(QFileInfo(filePath).absolutePath());

  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(reply->readAll());
    file.close();
  }
}

/**
 * @brief Loads the installed-extension tracking manifest from disk.
 */
void Misc::ExtensionInstaller::loadInstalledManifest()
{
  QFile file(installedManifestPath());
  if (!file.open(QIODevice::ReadOnly))
    return;

  const auto doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isObject())
    m_installedExtensions = doc.object();
}

/**
 * @brief Saves the installed-extension tracking manifest to disk.
 */
void Misc::ExtensionInstaller::saveInstalledManifest()
{
  QDir().mkpath(extensionsPath());
  QFile file(installedManifestPath());
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(m_installedExtensions).toJson(QJsonDocument::Indented));
    file.close();
  }
}
