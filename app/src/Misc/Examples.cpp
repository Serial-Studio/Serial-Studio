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

#include "Misc/Examples.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>

#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"

//--------------------------------------------------------------------------------------------------
// GitHub URL helpers
//--------------------------------------------------------------------------------------------------

static const QString kGitHubBranch = QStringLiteral("master");
static const QString kGitHubRepo   = QStringLiteral("Serial-Studio/Serial-Studio");

static const QString kRawBase = QStringLiteral("https://raw.githubusercontent.com/%1/%2/examples/")
                                  .arg(kGitHubRepo, kGitHubBranch);

static const QString kApiContentsBase =
  QStringLiteral("https://api.github.com/repos/%1/contents/examples/").arg(kGitHubRepo);

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Examples singleton.
 */
Misc::Examples::Examples()
  : m_loading(false)
  , m_selectedIndex(-1)
  , m_downloadProgress(0)
  , m_pendingDownloads(0)
  , m_totalDownloads(0)
{}

/**
 * @brief Returns the singleton instance of the Examples class.
 */
Misc::Examples& Misc::Examples::instance()
{
  static Examples singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a network operation is in progress.
 */
bool Misc::Examples::loading() const noexcept
{
  return m_loading;
}

/**
 * @brief Returns the number of currently visible (filtered) examples.
 */
int Misc::Examples::count() const noexcept
{
  return m_filteredExamples.count();
}

/**
 * @brief Returns the index of the currently selected example, or -1.
 */
int Misc::Examples::selectedIndex() const noexcept
{
  return m_selectedIndex;
}

/**
 * @brief Returns the current download progress (0.0 to 1.0).
 */
float Misc::Examples::downloadProgress() const noexcept
{
  return m_downloadProgress;
}

/**
 * @brief Returns the current search filter text.
 */
const QString& Misc::Examples::searchFilter() const noexcept
{
  return m_searchFilter;
}

/**
 * @brief Returns the README markdown text for the selected example.
 */
const QString& Misc::Examples::selectedReadme() const noexcept
{
  return m_selectedReadme;
}

/**
 * @brief Returns a file:// URL to the cached screenshot for the selected
 *        example.
 */
const QUrl& Misc::Examples::selectedScreenshot() const noexcept
{
  return m_selectedScreenshot;
}

/**
 * @brief Returns the filtered examples list for the grid view.
 */
const QVariantList& Misc::Examples::examples() const noexcept
{
  return m_filteredExamples;
}

/**
 * @brief Returns the metadata map for the currently selected example.
 */
QVariantMap Misc::Examples::selectedExample() const
{
  if (m_selectedIndex >= 0 && m_selectedIndex < m_filteredExamples.count())
    return m_filteredExamples.at(m_selectedIndex).toMap();

  return {};
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects an example by index and triggers README/screenshot fetching.
 */
void Misc::Examples::setSelectedIndex(int index)
{
  if (m_selectedIndex == index)
    return;

  m_selectedIndex = index;
  Q_EMIT selectedIndexChanged();

  // Clear previous content
  m_selectedReadme.clear();
  m_selectedScreenshot.clear();
  Q_EMIT selectedReadmeChanged();
  Q_EMIT selectedScreenshotChanged();

  // Fetch content for the newly selected example
  if (index >= 0 && index < m_filteredExamples.count()) {
    const auto example = m_filteredExamples.at(index).toMap();
    const auto id      = example.value("id").toString();
    fetchReadme(id);
    if (example.value("hasScreenshot").toBool()) {
      const auto ssFile = example.value("screenshotFileName", "screenshot.png").toString();
      fetchScreenshot(id, ssFile);
    }
  }
}

/**
 * @brief Sets the search filter and re-applies filtering to the examples list.
 */
void Misc::Examples::setSearchFilter(const QString& filter)
{
  if (m_searchFilter == filter)
    return;

  m_searchFilter = filter;
  Q_EMIT searchFilterChanged();
  applyFilter();
}

//--------------------------------------------------------------------------------------------------
// Download slot
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initiates downloading all files of the selected example to a
 *        user-chosen directory. Uses the GitHub Contents API to list files,
 *        then downloads each one.
 */
void Misc::Examples::downloadExample()
{
  if (m_selectedIndex < 0 || m_selectedIndex >= m_filteredExamples.count())
    return;

  // Build download path inside the workspace Examples folder
  const auto example = m_filteredExamples.at(m_selectedIndex).toMap();
  const auto id      = example.value("id").toString();
  const auto dir     = Misc::WorkspaceManager::instance().path("Examples");
  m_downloadPath     = dir + "/" + id;
  QDir().mkpath(m_downloadPath);

  // Start loading
  m_loading          = true;
  m_downloadProgress = 0;
  Q_EMIT loadingChanged();
  Q_EMIT downloadProgressChanged();

  // Fetch the file listing from GitHub Contents API
  auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(id));
  auto url     = QUrl::fromEncoded((kApiContentsBase + encoded).toUtf8());
  auto* reply  = m_nam.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished, this, &Examples::onContentsReply);
}

//--------------------------------------------------------------------------------------------------
// Network reply handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the examples.json manifest fetch response.
 */
void Misc::Examples::onManifestReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  // Parse the manifest
  if (reply->error() == QNetworkReply::NoError) {
    const auto doc = QJsonDocument::fromJson(reply->readAll());
    m_allExamples  = doc.array();
    applyFilter();
  }

  // Clear loading state
  m_loading = false;
  Q_EMIT loadingChanged();
}

/**
 * @brief Handles the README.md fetch response and caches it.
 */
void Misc::Examples::onReadmeReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto raw = QString::fromUtf8(reply->readAll());

    // Cache the raw readme (unstripped)
    const auto id = reply->property("exampleId").toString();
    if (!id.isEmpty()) {
      const auto path = exampleCachePath(id);
      QDir().mkpath(path);
      QFile file(path + "/README.md");
      if (file.open(QIODevice::WriteOnly)) {
        file.write(raw.toUtf8());
        file.close();
      }
    }

    // Expose raw markdown to QML (WebView handles images)
    m_selectedReadme = raw;
  }

  else
    m_selectedReadme = tr("Failed to load README: %1").arg(reply->errorString());

  // Reset loading flag
  m_loading = false;
  Q_EMIT loadingChanged();
  Q_EMIT selectedReadmeChanged();
}

/**
 * @brief Handles the screenshot fetch response and caches it.
 */
void Misc::Examples::onScreenshotReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() == QNetworkReply::NoError) {
    const auto data = reply->readAll();
    const auto id   = reply->property("exampleId").toString();
    if (!id.isEmpty()) {
      // Cache the screenshot
      const auto path = exampleCachePath(id) + "/doc";
      QDir().mkpath(path);
      auto name = reply->property("fileName").toString();
      if (name.isEmpty())
        name = QStringLiteral("screenshot.png");

      const auto file_path = path + "/" + name;
      QFile file(file_path);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        m_selectedScreenshot = QUrl::fromLocalFile(file_path);
        Q_EMIT selectedScreenshotChanged();
      }
    }
  }
}

/**
 * @brief Handles the GitHub Contents API response listing all files in an
 *        example directory. Populates the download queue and starts fetching.
 */
void Misc::Examples::onContentsReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    m_loading = false;
    Q_EMIT loadingChanged();
    return;
  }

  // Parse the file listing
  const auto doc   = QJsonDocument::fromJson(reply->readAll());
  const auto files = doc.array();

  m_downloadQueue.clear();
  for (const auto& entry : files) {
    const auto obj  = entry.toObject();
    const auto type = obj.value("type").toString();

    // Only download files, not subdirectories
    if (type != "file")
      continue;

    const auto name         = obj.value("name").toString();
    const auto download_url = QUrl(obj.value("download_url").toString());
    if (!download_url.isEmpty())
      m_downloadQueue.append({name, download_url});
  }

  m_totalDownloads   = m_downloadQueue.count();
  m_pendingDownloads = m_totalDownloads;

  if (m_totalDownloads == 0) {
    m_loading = false;
    Q_EMIT loadingChanged();
    return;
  }

  downloadNextFile();
}

/**
 * @brief Handles individual file download completion and triggers the next
 *        download. Auto-opens the project file when all downloads complete.
 */
void Misc::Examples::onFileDownloadReply()
{
  auto* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  // Write the file to the download directory
  if (reply->error() == QNetworkReply::NoError) {
    const auto file_name = reply->property("fileName").toString();
    const auto file_path = m_downloadPath + "/" + file_name;

    // Create subdirectories if needed
    QFileInfo info(file_path);
    QDir().mkpath(info.absolutePath());

    QFile file(file_path);
    if (file.open(QIODevice::WriteOnly)) {
      file.write(reply->readAll());
      file.close();
    }
  }

  // Update progress
  --m_pendingDownloads;
  m_downloadProgress = static_cast<float>(m_totalDownloads - m_pendingDownloads)
                     / static_cast<float>(m_totalDownloads);
  Q_EMIT downloadProgressChanged();

  // Continue with next file or finish
  if (!m_downloadQueue.isEmpty()) {
    downloadNextFile();
    return;
  }

  // All downloads complete
  m_loading = false;
  Q_EMIT loadingChanged();

  // Always reveal the download folder in the file manager
  Misc::Utilities::revealFile(m_downloadPath);

  // Load the project file & show the editor if one was downloaded
  if (m_selectedIndex >= 0 && m_selectedIndex < m_filteredExamples.count()) {
    const auto example      = m_filteredExamples.at(m_selectedIndex).toMap();
    const auto project_file = example.value("projectFileName").toString();
    if (!project_file.isEmpty()) {
      const auto path = m_downloadPath + "/" + project_file;
      if (QFile::exists(path))
        Q_EMIT projectFileReady(path);
    }
  }
}

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Fetches the examples.json manifest from GitHub. No-op if already
 *        loaded or currently loading.
 */
void Misc::Examples::fetchManifest()
{
  if (m_loading || !m_allExamples.isEmpty())
    return;

  m_loading = true;
  Q_EMIT loadingChanged();

  const auto url = QUrl(kRawBase + "examples.json");
  auto* reply    = m_nam.get(QNetworkRequest(url));
  connect(reply, &QNetworkReply::finished, this, &Examples::onManifestReply);
}

/**
 * @brief Applies the current search filter to rebuild the filtered examples
 *        list.
 */
void Misc::Examples::applyFilter()
{
  m_filteredExamples.clear();

  for (const auto& entry : std::as_const(m_allExamples)) {
    const auto obj = entry.toObject();
    if (!m_searchFilter.isEmpty()) {
      const auto title    = obj.value("title").toString();
      const auto desc     = obj.value("description").toString();
      const auto category = obj.value("category").toString();

      const bool matches = title.contains(m_searchFilter, Qt::CaseInsensitive)
                        || desc.contains(m_searchFilter, Qt::CaseInsensitive)
                        || category.contains(m_searchFilter, Qt::CaseInsensitive);

      if (!matches)
        continue;
    }

    m_filteredExamples.append(obj.toVariantMap());
  }

  // Reset selection when filter changes
  m_selectedIndex = -1;
  Q_EMIT selectedIndexChanged();
  Q_EMIT filteredExamplesChanged();
}

/**
 * @brief Fetches the README.md for the given example from cache or GitHub.
 */
void Misc::Examples::fetchReadme(const QString& id)
{
  // Check cache first
  const auto cached = exampleCachePath(id) + "/README.md";
  if (QFile::exists(cached)) {
    QFile file(cached);
    if (file.open(QIODevice::ReadOnly)) {
      m_selectedReadme = QString::fromUtf8(file.readAll());
      Q_EMIT selectedReadmeChanged();
      return;
    }
  }

  // Fetch from GitHub
  m_loading = true;
  Q_EMIT loadingChanged();

  auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(id));
  auto url     = QUrl::fromEncoded((kRawBase + encoded + "/README.md").toUtf8());
  QNetworkRequest request(url);
  auto* reply = m_nam.get(request);
  reply->setProperty("exampleId", id);
  connect(reply, &QNetworkReply::finished, this, &Examples::onReadmeReply);
}

/**
 * @brief Fetches the screenshot for the given example from cache or GitHub.
 */
void Misc::Examples::fetchScreenshot(const QString& id, const QString& fileName)
{
  // Check cache first
  const auto cached = exampleCachePath(id) + "/doc/" + fileName;
  if (QFile::exists(cached)) {
    m_selectedScreenshot = QUrl::fromLocalFile(cached);
    Q_EMIT selectedScreenshotChanged();
    return;
  }

  // Fetch from GitHub
  auto encoded = QString::fromUtf8(QUrl::toPercentEncoding(id));
  auto url     = QUrl::fromEncoded((kRawBase + encoded + "/doc/" + fileName).toUtf8());
  auto* reply  = m_nam.get(QNetworkRequest(url));
  reply->setProperty("exampleId", id);
  reply->setProperty("fileName", fileName);
  connect(reply, &QNetworkReply::finished, this, &Examples::onScreenshotReply);
}

/**
 * @brief Downloads the next file in the queue.
 */
void Misc::Examples::downloadNextFile()
{
  if (m_downloadQueue.isEmpty())
    return;

  const auto [name, url] = m_downloadQueue.takeFirst();
  auto* reply            = m_nam.get(QNetworkRequest(url));
  reply->setProperty("fileName", name);
  connect(reply, &QNetworkReply::finished, this, &Examples::onFileDownloadReply);
}

/**
 * @brief Returns the base cache directory for examples.
 */
QString Misc::Examples::cachePath() const
{
  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/examples";
}

/**
 * @brief Returns the cache directory for a specific example by ID.
 */
QString Misc::Examples::exampleCachePath(const QString& id) const
{
  return cachePath() + "/" + id;
}

/**
 * @brief Removes markdown image tags (![alt](url)) from the given text.
 *
 * Qt's TextArea.MarkdownText attempts to resolve image URLs relative to qrc,
 * which causes "Cannot read resource" warnings. We strip images server-side
 * and show them in a dedicated Image element instead.
 */
QString Misc::Examples::stripMarkdownImages(const QString& markdown)
{
  static const QRegularExpression re(QStringLiteral("!\\[[^\\]]*\\]\\([^)]*\\)\\s*\\n?"));
  return QString(markdown).remove(re);
}
