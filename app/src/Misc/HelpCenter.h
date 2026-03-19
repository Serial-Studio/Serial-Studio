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

#include <QHash>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QVariantList>

namespace Misc {
/**
 * @brief Provides an in-app help center that fetches and displays
 *        documentation pages from GitHub with local caching.
 *
 * The HelpCenter class loads a manifest of help pages and allows users
 * to browse, search, and read documentation without leaving Serial Studio.
 *
 * Page content is delivered as raw markdown to QML, where a WebEngineView
 * renders it via showdown.js + highlight.js + KaTeX.
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
  Q_PROPERTY(QString pageTitle
             READ pageTitle
             NOTIFY currentIndexChanged)
  Q_PROPERTY(QVariantList pages
             READ pages
             NOTIFY pagesChanged)
  Q_PROPERTY(QString themeColors
             READ themeColors
             NOTIFY themeColorsChanged)
  // clang-format on

signals:
  void loadingChanged();
  void pagesChanged();
  void currentIndexChanged();
  void searchFilterChanged();
  void pageContentChanged();
  void themeColorsChanged();

private:
  explicit HelpCenter();
  HelpCenter(HelpCenter&&)                 = delete;
  HelpCenter(const HelpCenter&)            = delete;
  HelpCenter& operator=(HelpCenter&&)      = delete;
  HelpCenter& operator=(const HelpCenter&) = delete;

public:
  static HelpCenter& instance();

  [[nodiscard]] bool loading() const noexcept;
  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int currentIndex() const noexcept;
  [[nodiscard]] const QString& searchFilter() const noexcept;
  [[nodiscard]] const QString& pageContent() const noexcept;
  [[nodiscard]] QString pageTitle() const;
  [[nodiscard]] const QVariantList& pages() const noexcept;
  [[nodiscard]] const QString& themeColors() const noexcept;

public slots:
  void fetchManifest();
  void setCurrentIndex(int index);
  void setSearchFilter(const QString& filter);
  Q_INVOKABLE void showPage(const QString& pageId);
  Q_INVOKABLE bool navigateToPage(const QString& link);

private slots:
  void onManifestReply();
  void onPageReply();
  void onPreloadReply();
  void onThemeChanged();
  void applyFilter();
  void fetchPage(int index);
  void preloadAllPages();

private:
  bool m_loading;
  int m_currentIndex;
  int m_pendingPreloads;

  QString m_pendingPageId;
  QString m_searchFilter;
  QString m_pageContent;
  QString m_themeColors;

  QJsonArray m_allPages;
  QVariantList m_filteredPages;
  QHash<QString, QString> m_pageContents;

  QNetworkAccessManager m_nam;
};
}  // namespace Misc
