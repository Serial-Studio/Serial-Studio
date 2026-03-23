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

#pragma once

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace Misc {
/**
 * @brief Provides an in-app catalog of Serial Studio example projects with
 *        download, preview, and search capabilities.
 *
 * The Examples class loads a bundled manifest of example metadata and allows
 * users to browse, search, and download example projects directly from GitHub
 * without leaving Serial Studio.
 */
class Examples : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool loading
             READ loading
             NOTIFY loadingChanged)
  Q_PROPERTY(int count
             READ count
             NOTIFY filteredExamplesChanged)
  Q_PROPERTY(int selectedIndex
             READ selectedIndex
             WRITE setSelectedIndex
             NOTIFY selectedIndexChanged)
  Q_PROPERTY(QString searchFilter
             READ searchFilter
             WRITE setSearchFilter
             NOTIFY searchFilterChanged)
  Q_PROPERTY(QVariantList examples
             READ examples
             NOTIFY filteredExamplesChanged)
  Q_PROPERTY(QVariantMap selectedExample
             READ selectedExample
             NOTIFY selectedIndexChanged)
  Q_PROPERTY(QString selectedReadme
             READ selectedReadme
             NOTIFY selectedReadmeChanged)
  Q_PROPERTY(QUrl selectedScreenshot
             READ selectedScreenshot
             NOTIFY selectedScreenshotChanged)
  Q_PROPERTY(float downloadProgress
             READ downloadProgress
             NOTIFY downloadProgressChanged)
  // clang-format on

signals:
  void loadingChanged();
  void searchFilterChanged();
  void selectedIndexChanged();
  void selectedReadmeChanged();
  void downloadProgressChanged();
  void selectedScreenshotChanged();
  void filteredExamplesChanged();
  void projectFileReady(const QString& path);

private:
  explicit Examples();
  Examples(Examples&&)                 = delete;
  Examples(const Examples&)            = delete;
  Examples& operator=(Examples&&)      = delete;
  Examples& operator=(const Examples&) = delete;

public:
  [[nodiscard]] static Examples& instance();

  [[nodiscard]] bool loading() const noexcept;
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int selectedIndex() const noexcept;
  [[nodiscard]] float downloadProgress() const noexcept;
  [[nodiscard]] const QString& searchFilter() const noexcept;
  [[nodiscard]] const QString& selectedReadme() const noexcept;
  [[nodiscard]] const QUrl& selectedScreenshot() const noexcept;
  [[nodiscard]] const QVariantList& examples() const noexcept;
  [[nodiscard]] QVariantMap selectedExample() const;

public slots:
  void fetchManifest();
  void setSelectedIndex(int index);
  void setSearchFilter(const QString& filter);
  void downloadExample();

private slots:
  void onManifestReply();
  void onReadmeReply();
  void onScreenshotReply();
  void onContentsReply();
  void onFileDownloadReply();
  void applyFilter();
  void fetchReadme(const QString& id);
  void fetchScreenshot(const QString& id, const QString& fileName);
  void downloadNextFile();

  [[nodiscard]] QString cachePath() const;
  [[nodiscard]] QString exampleCachePath(const QString& id) const;
  [[nodiscard]] static QString stripMarkdownImages(const QString& markdown);

private:
  bool m_loading;
  int m_selectedIndex;
  float m_downloadProgress;
  int m_pendingDownloads;
  int m_totalDownloads;

  QString m_searchFilter;
  QString m_selectedReadme;
  QUrl m_selectedScreenshot;
  QString m_downloadPath;

  QJsonArray m_allExamples;
  QVariantList m_filteredExamples;

  QList<QPair<QString, QUrl>> m_downloadQueue;
  QNetworkAccessManager m_nam;
};
}  // namespace Misc
