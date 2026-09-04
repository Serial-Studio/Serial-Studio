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

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QVariantList>

namespace Misc {
class ThemeManager;

/**
 * @brief Provides an in-app help center that fetches and displays documentation
 *        pages from GitHub with local caching.
 */
class HelpCenter : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool loading
             READ loading
             NOTIFY loadingChanged)
  Q_PROPERTY(int count
             READ count
             NOTIFY pagesChanged)
  Q_PROPERTY(int currentIndex
             READ currentIndex
             WRITE setCurrentIndex
             NOTIFY currentIndexChanged)
  Q_PROPERTY(QString searchFilter
             READ searchFilter
             WRITE setSearchFilter
             NOTIFY searchFilterChanged)
  Q_PROPERTY(QString pageContent
             READ pageContent
             NOTIFY pageContentChanged)
  Q_PROPERTY(QString pageId
             READ pageId
             NOTIFY currentIndexChanged)
  Q_PROPERTY(QString pageTitle
             READ pageTitle
             NOTIFY currentIndexChanged)
  Q_PROPERTY(QVariantList pages
             READ pages
             NOTIFY pagesChanged)
  Q_PROPERTY(QString themeColors
             READ themeColors
             NOTIFY themeColorsChanged)
  Q_PROPERTY(QVariantList versions
             READ versions
             NOTIFY versionsChanged)
  Q_PROPERTY(int versionIndex
             READ versionIndex
             WRITE setVersionIndex
             NOTIFY versionIndexChanged)
  Q_PROPERTY(QString versionLabel
             READ versionLabel
             NOTIFY versionIndexChanged)
  Q_PROPERTY(QString versionNotice
             READ versionNotice
             NOTIFY versionIndexChanged)
  // clang-format on

signals:
  void loadingChanged();
  void pagesChanged();
  void currentIndexChanged();
  void searchFilterChanged();
  void pageContentChanged();
  void themeColorsChanged();
  void versionsChanged();
  void versionIndexChanged();

private:
  explicit HelpCenter();
  HelpCenter(HelpCenter&&)                 = delete;
  HelpCenter(const HelpCenter&)            = delete;
  HelpCenter& operator=(HelpCenter&&)      = delete;
  HelpCenter& operator=(const HelpCenter&) = delete;

public:
  [[nodiscard]] static HelpCenter& instance();

  [[nodiscard]] QString pageId() const;
  [[nodiscard]] QString pageTitle() const;
  [[nodiscard]] QString versionLabel() const;
  [[nodiscard]] QString versionNotice() const;

  [[nodiscard]] Q_INVOKABLE QString onlineUrl() const;

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] bool loading() const noexcept;
  [[nodiscard]] int currentIndex() const noexcept;
  [[nodiscard]] int versionIndex() const noexcept;

  [[nodiscard]] const QVariantList& pages() const noexcept;
  [[nodiscard]] const QVariantList& versions() const noexcept;
  [[nodiscard]] const QString& themeColors() const noexcept;
  [[nodiscard]] const QString& pageContent() const noexcept;
  [[nodiscard]] const QString& searchFilter() const noexcept;

public slots:
  void fetchManifest();
  void setCurrentIndex(int index);
  void setVersionIndex(int index);
  void showPage(const QString& pageId);
  bool navigateToPage(const QString& link);
  void setSearchFilter(const QString& filter);

private slots:
  void fetchVersions();
  void onVersionsReply();
  void onManifestReply();
  void onPageReply();
  void onPreloadReply();
  void onThemeChanged();
  void applyFilter();
  void fetchPage(int index);
  void preloadAllPages();

private:
  void applyVersions(const QJsonArray& array);
  [[nodiscard]] int resolveDefaultVersion() const;
  [[nodiscard]] QUrl contentUrl(const QString& file) const;
  [[nodiscard]] bool pageMatchesFilter(const QJsonObject& obj) const;
  [[nodiscard]] QVariantMap currentVersion() const;

private:
  bool m_loading;
  int m_epoch;
  int m_currentIndex;
  int m_versionIndex;
  int m_defaultVersion;
  int m_pendingPreloads;

  QString m_pendingPageId;
  QString m_searchFilter;
  QString m_pageContent;
  QString m_themeColors;

  QJsonArray m_allPages;
  QVariantList m_versions;
  QVariantList m_filteredPages;
  QHash<QString, QString> m_pageContents;

  QNetworkAccessManager m_nam;
  Misc::ThemeManager* m_theme;
};
}  // namespace Misc
