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

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "Core/SSAssert.h"
#include "Misc/Extensions/ExtensionCatalog.h"
#include "Misc/WorkspaceManager.h"

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

/**
 * @brief Returns the directory an extension of this type and id is installed into.
 */
QString Misc::ExtensionInstaller::installDirFor(const QString& type, const QString& id) const
{
  return extensionsPath() + "/" + type + "/" + id;
}

/**
 * @brief Returns the reason the last install was refused or aborted, or empty.
 */
QString Misc::ExtensionInstaller::lastError() const
{
  return m_lastError;
}

//--------------------------------------------------------------------------------------------------
// Install & uninstall
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the extension described by @p entry. Files land in a sibling `<id>.staging`
 *        directory, each verified against its catalog digest, and only a complete verified set
 *        replaces the installed version. An entry whose files carry no digests is catalog v1 and
 *        is refused: there is nothing to verify a download against (K3, K5).
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

  const auto digests = ExtensionCatalog::mergeDigests(entry.value("sha256").toMap(),
                                                      selection.value("sha256").toMap());

  m_lastError.clear();
  if (id.isEmpty() || files.isEmpty())
    return false;

  if (!ExtensionCatalog::isSafePathComponent(id) || !ExtensionCatalog::isSafePathComponent(type)) {
    m_lastError = tr("Extension id or type is not a safe path component.");
    Q_EMIT installFailed(id, m_lastError);
    return false;
  }

  QString reason;
  const auto parsed = ExtensionCatalog::parseFileList(files, digests, &reason);
  if (parsed.isEmpty()) {
    m_lastError = reason.isEmpty() ? tr("Catalog entry lists no verifiable files.") : reason;
    qWarning() << "[ExtensionInstaller] refusing" << id << ":" << m_lastError;
    Q_EMIT installFailed(id, m_lastError);
    return false;
  }

  if (isLocal)
    return installLocal(entry, parsed);

  startDownloads(entry, parsed);
  return true;
}

/**
 * @brief Stages a local repository's files, verifies them, and swaps them over the installed
 *        version. A local folder is still verified: the digests are what say the folder holds
 *        the package the catalog described.
 */
bool Misc::ExtensionInstaller::installLocal(const QVariantMap& entry,
                                            const QList<ExtensionCatalog::CatalogFile>& files)
{
  const auto id   = entry.value("id").toString();
  const auto type = entry.value("type").toString();
  const auto base = entry.value("_repoBase").toString();
  SS_ASSERT(!id.isEmpty(), return false);

  m_currentInstallId       = id;
  m_currentInstallRepoBase = base;
  m_currentInstallMeta     = entry;
  m_currentFiles           = files;
  m_currentInstallDir      = installDirFor(type, id);
  m_currentStagingDir      = m_currentInstallDir + ".staging";

  QDir(m_currentStagingDir).removeRecursively();
  if (!copyLocalFiles(files, base, m_currentStagingDir)) {
    abortInstall(tr("A file of this extension is missing or does not match its digest."));
    return false;
  }

  if (!commitStagedInstall()) {
    abortInstall(tr("The verified files could not replace the installed version."));
    return false;
  }

  m_installedExtensions.insert(id, buildInstalledRecord());
  saveInstalledManifest();

  Q_EMIT installed(id);
  return true;
}

/**
 * @brief Copies each declared file into the staging directory, refusing a path that would leave
 *        it and a file whose bytes do not match its digest. Returns false on the first failure,
 *        so a partial copy never reaches the install directory.
 */
bool Misc::ExtensionInstaller::copyLocalFiles(const QList<ExtensionCatalog::CatalogFile>& files,
                                              const QString& base,
                                              const QString& stagingDir)
{
  for (const auto& file : files) {
    const auto dst = stagingDir + "/" + file.path;
    if (!ExtensionCatalog::isPathSafe(dst, stagingDir))
      return false;

    QFile source(base + file.path);
    if (!source.open(QIODevice::ReadOnly))
      return false;

    const auto payload = source.readAll();
    source.close();
    if (!file.selfMetadata && !ExtensionCatalog::digestMatches(payload, file))
      return false;

    QDir().mkpath(QFileInfo(dst).absolutePath());
    QFile target(dst);
    if (!target.open(QIODevice::WriteOnly))
      return false;

    if (target.write(payload) != payload.size())
      return false;

    target.close();
  }

  return true;
}

/**
 * @brief Queues every file of a remote install into a fresh staging directory and starts the
 *        first download.
 */
void Misc::ExtensionInstaller::startDownloads(const QVariantMap& entry,
                                              const QList<ExtensionCatalog::CatalogFile>& files)
{
  const auto base = entry.value("_repoBase").toString();

  m_currentInstallId       = entry.value("id").toString();
  m_currentInstallRepoBase = base;
  m_currentInstallMeta     = entry;
  m_currentFiles           = files;
  m_currentInstallDir      = installDirFor(entry.value("type").toString(), m_currentInstallId);
  m_currentStagingDir      = m_currentInstallDir + ".staging";

  QDir(m_currentStagingDir).removeRecursively();
  QDir().mkpath(m_currentStagingDir);

  m_busy     = true;
  m_progress = 0;
  Q_EMIT busyChanged();
  Q_EMIT progressChanged();

  m_downloadQueue.clear();
  for (const auto& file : files) {
    if (!file.selfMetadata) {
      m_downloadQueue.append(file);
      continue;
    }

    if (!writeStagedPayload(file.path, metadataPayload(entry))) {
      abortInstall(tr("The extension metadata could not be staged."));
      return;
    }
  }

  m_totalDownloads   = m_downloadQueue.count();
  m_pendingDownloads = m_totalDownloads;
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

  const auto file = m_downloadQueue.takeFirst();
  auto* reply     = m_network.get(
    QNetworkRequest(ExtensionCatalog::resolveFileUrl(m_currentInstallRepoBase, file.path)));
  SS_ASSERT(reply != nullptr, return);

  reply->setProperty("localName", file.path);
  m_replies.insert(reply);
  connect(reply, &QNetworkReply::finished, this, &ExtensionInstaller::onFileDownloadReply);
}

/**
 * @brief Handles one file's completion. A transport failure or a digest mismatch aborts the whole
 *        install: recording a partial download as installed is what left plugins with missing
 *        files and no repair path but uninstall (K3).
 */
void Misc::ExtensionInstaller::onFileDownloadReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  m_replies.remove(reply);
  reply->deleteLater();

  if (!m_busy)
    return;

  if (reply->error() != QNetworkReply::NoError) {
    abortInstall(tr("Download failed: %1").arg(reply->errorString()));
    return;
  }

  if (!writeStagedFile(reply)) {
    abortInstall(tr("A downloaded file did not match the digest published for it."));
    return;
  }

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
 * @brief Swaps the verified staging directory over the installed version and records the install.
 *        installed.json is written last, so a crash between the swap and the record leaves files
 *        on disk the next launch does not know about, never a record without its files.
 */
void Misc::ExtensionInstaller::finishInstall()
{
  if (!commitStagedInstall()) {
    abortInstall(tr("The verified files could not replace the installed version."));
    return;
  }

  m_installedExtensions.insert(m_currentInstallId, buildInstalledRecord());
  saveInstalledManifest();

  m_busy = false;
  Q_EMIT busyChanged();
  Q_EMIT installed(m_currentInstallId);
}

/**
 * @brief Drops the staging directory and ends the install; the previously installed version and
 *        its installed.json record are left exactly as they were.
 */
void Misc::ExtensionInstaller::abortInstall(const QString& reason)
{
  m_lastError = reason;
  qWarning() << "[ExtensionInstaller] install of" << m_currentInstallId << "aborted:" << reason;

  for (auto* reply : std::as_const(m_replies)) {
    if (!reply)
      continue;

    reply->disconnect(this);
    reply->abort();
    reply->deleteLater();
  }

  m_replies.clear();
  m_downloadQueue.clear();
  if (!m_currentStagingDir.isEmpty())
    QDir(m_currentStagingDir).removeRecursively();

  m_busy     = false;
  m_progress = 0;
  Q_EMIT busyChanged();
  Q_EMIT progressChanged();
  Q_EMIT installFailed(m_currentInstallId, reason);
}

/**
 * @brief Moves the staged directory into place: the previous version is kept under `.previous`
 *        until the new one is in, and is restored when the second rename fails, so a failed swap
 *        can never leave the extension half-installed.
 */
bool Misc::ExtensionInstaller::commitStagedInstall()
{
  const auto previousDir = m_currentInstallDir + ".previous";
  QDir(previousDir).removeRecursively();

  const bool hadPrevious = QFileInfo::exists(m_currentInstallDir);
  if (hadPrevious && !QDir().rename(m_currentInstallDir, previousDir))
    return false;

  QDir().mkpath(QFileInfo(m_currentInstallDir).absolutePath());
  if (!QDir().rename(m_currentStagingDir, m_currentInstallDir)) {
    if (hadPrevious)
      (void)QDir().rename(previousDir, m_currentInstallDir);

    return false;
  }

  QDir(previousDir).removeRecursively();
  return true;
}

/**
 * @brief Builds the installed.json record: version, type, repository and the verified file list
 *        with its digests, which is what a later repair check compares against.
 */
QJsonObject Misc::ExtensionInstaller::buildInstalledRecord() const
{
  QJsonArray fileList;
  for (const auto& file : m_currentFiles) {
    QJsonObject row;
    row.insert("path", file.path);
    if (!file.sha256.isEmpty())
      row.insert("sha256", file.sha256);

    if (file.size > 0)
      row.insert("size", static_cast<double>(file.size));

    fileList.append(row);
  }

  QJsonObject info;
  info.insert("version", m_currentInstallMeta.value("version").toString());
  info.insert("type", m_currentInstallMeta.value("type").toString());
  info.insert("repoBase", m_currentInstallRepoBase);
  info.insert("files", fileList);
  return info;
}

//--------------------------------------------------------------------------------------------------
// Manifest & file I/O
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes one downloaded file into the staging directory after checking its digest and its
 *        resolved path. Returns false on any refusal, which aborts the install.
 */
bool Misc::ExtensionInstaller::writeStagedFile(QNetworkReply* reply)
{
  SS_ASSERT(reply != nullptr, return false);

  const auto localName = reply->property("localName").toString();
  const auto match     = std::find_if(
    m_currentFiles.constBegin(),
    m_currentFiles.constEnd(),
    [&localName](const ExtensionCatalog::CatalogFile& file) { return file.path == localName; });
  if (match == m_currentFiles.constEnd())
    return false;

  const auto payload = reply->readAll();
  if (!ExtensionCatalog::digestMatches(payload, *match))
    return false;

  return writeStagedPayload(localName, payload);
}

/**
 * @brief Writes verified bytes into the staging directory, refusing a path that would leave it.
 *        Everything that lands in staging goes through here, so the containment check has one
 *        home rather than one per producer.
 */
bool Misc::ExtensionInstaller::writeStagedPayload(const QString& localName,
                                                  const QByteArray& payload)
{
  const auto filePath = m_currentStagingDir + "/" + localName;
  if (!ExtensionCatalog::isPathSafe(filePath, m_currentStagingDir))
    return false;

  QDir().mkpath(QFileInfo(filePath).absolutePath());

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly))
    return false;

  const bool written = file.write(payload) == payload.size();
  file.close();
  return written;
}

/**
 * @brief The bytes to install as the entry's own info.json. The metadata document the catalog was
 *        read from is written back verbatim when the repository served one (`_metaRaw`); an entry
 *        published inline in a manifest has no such file, so the entry itself is serialized,
 *        minus the underscore-prefixed fields the manager attaches at runtime.
 */
QByteArray Misc::ExtensionInstaller::metadataPayload(const QVariantMap& entry)
{
  const auto raw = entry.value("_metaRaw").toString().toUtf8();
  if (!raw.isEmpty())
    return raw;

  QVariantMap clean;
  for (auto it = entry.cbegin(); it != entry.cend(); ++it)
    if (!it.key().startsWith(QLatin1Char('_')))
      clean.insert(it.key(), it.value());

  return QJsonDocument(QJsonObject::fromVariantMap(clean)).toJson(QJsonDocument::Indented);
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
