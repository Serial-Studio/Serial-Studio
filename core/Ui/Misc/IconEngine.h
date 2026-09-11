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
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QQuickImageProvider>

namespace Misc {

/**
 * @brief Online icon search via the Iconify API and inline SVG resolution for action icons;
 *        previews come from the batched collection endpoint because the per-icon SVG endpoint
 *        answers a grid-sized burst with HTTP 429.
 */
class IconEngine : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(QStringList iconNames
             READ iconNames
             NOTIFY searchResultsChanged)
  Q_PROPERTY(QStringList iconPreviews
             READ iconPreviews
             NOTIFY previewsChanged)
  Q_PROPERTY(bool loadingPreviews
             READ loadingPreviews
             NOTIFY loadingPreviewsChanged)
  // clang-format on

signals:
  void busyChanged();
  void previewsChanged();
  void loadingPreviewsChanged();
  void searchResultsChanged();
  void searchFailed(const QString& error);
  void iconDownloaded(const QString& svgData);
  void iconDownloadFailed(const QString& error);

private:
  explicit IconEngine();
  IconEngine(IconEngine&&)                 = delete;
  IconEngine(const IconEngine&)            = delete;
  IconEngine& operator=(IconEngine&&)      = delete;
  IconEngine& operator=(const IconEngine&) = delete;

public:
  [[nodiscard]] static IconEngine& instance();

  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] bool loadingPreviews() const noexcept;
  [[nodiscard]] const QStringList& iconNames() const noexcept;
  [[nodiscard]] const QStringList& iconPreviews() const noexcept;

  [[nodiscard]] Q_INVOKABLE static QString resolveActionIconSource(const QString& icon);
  [[nodiscard]] Q_INVOKABLE static bool isInlineSvg(const QString& icon);

public slots:
  void searchIcons(const QString& query);
  void downloadIcon(int index);

private:
  void refreshPreviews();
  void requestNextBatch();
  void refreshLoadingState();
  void queuePreviewFetches();
  void resolvePendingDownload();
  void cacheIconSet(const QJsonObject& root);
  void failPendingDownload(const QString& error);
  void scheduleBatch(const QStringList& batch, int delayMs);
  void onSearchFinished(QNetworkReply* reply, int generation);
  void onBatchFinished(QNetworkReply* reply, int generation, const QStringList& batch);

  [[nodiscard]] static QString toDataUri(const QString& svg);
  [[nodiscard]] static QNetworkRequest makeRequest(const QUrl& url);
  [[nodiscard]] static QString toPreviewSource(const QString& svg);
  [[nodiscard]] static int retryDelayMs(const QNetworkReply* reply, int attempt);
  [[nodiscard]] static QString buildSvg(const QJsonObject& icon, int defW, int defH);

private:
  bool m_busy;
  bool m_loading;
  bool m_inFlight;
  int m_attempt;
  int m_generation;
  QString m_pendingDownload;
  QStringList m_iconNames;
  QStringList m_iconPreviews;
  QList<QStringList> m_pendingBatches;
  QHash<QString, QString> m_svgCache;
  QNetworkAccessManager m_manager;

  static constexpr int kMaxAttempts     = 4;
  static constexpr int kPreviewBatch    = 48;
  static constexpr int kSearchLimit     = 96;
  static constexpr int kBatchSpacingMs  = 150;
  static constexpr int kMaxCachedIcons  = 4096;
  static constexpr int kTransferTimeout = 15000;
};

/**
 * @brief QQuickImageProvider for rendering inline base64-encoded SVG icons.
 */
class ActionIconProvider : public QQuickImageProvider {
public:
  explicit ActionIconProvider();

  QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;
};

}  // namespace Misc
