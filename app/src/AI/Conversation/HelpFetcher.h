/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkReply;

namespace AI {

/**
 * @brief Fetches Serial Studio help pages for the meta.fetchHelp tool. The model chooses the
 *        URL, so every request and every redirect target is re-checked against an
 *        exactly-anchored https allowlist and oversized transfers are aborted mid-flight;
 *        fetches started before abortPending() never emit, so a cancelled turn drops them.
 */
class HelpFetcher : public QObject {
  Q_OBJECT

public:
  static constexpr int kFetchTimeoutMs    = 15 * 1000;
  static constexpr int kMaxFetchBytes     = 32 * 1024;
  static constexpr int kMaxIndexBytes     = 64 * 1024;
  static constexpr int kMaxTransportBytes = 1 * 1024 * 1024;

  static constexpr const char* kToolName = "meta.fetchHelp";

  explicit HelpFetcher(QObject* parent = nullptr);

  [[nodiscard]] static bool urlAllowed(const QUrl& url);
  [[nodiscard]] static QUrl pageUrl(const QString& path);

signals:
  void fetchFinished(const QString& callId, const QJsonObject& result);

public slots:
  void fetchPage(const QString& callId, const QString& path);
  void abortPending();

private:
  void fetchIndex(const QString& callId, const QUrl& missedUrl);
  void completePage(const QString& callId, const QUrl& url, QNetworkReply* reply);
  void completeIndex(const QString& callId, const QUrl& missedUrl, QNetworkReply* reply);
  void harden(QNetworkReply* reply) const;

  QNetworkAccessManager m_nam;
  quint64 m_epoch;
};

}  // namespace AI
