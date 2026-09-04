/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QString>

#include "AI/Providers/Provider.h"

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

namespace AI {

class SseEventReader;

/**
 * @brief Real Gemini streamGenerateContent streaming Reply.
 */
class GeminiReply : public Reply {
  Q_OBJECT

public:
  GeminiReply(QNetworkAccessManager& nam,
              const QUrl& endpoint,
              const QString& apiKey,
              const QByteArray& requestBody,
              QObject* parent = nullptr);

  void abort() override;

private:
  void onSseEvent(const QString& name, const QJsonObject& data);
  void onSseError(const QString& reason);
  void onReplyReadyRead();
  void onReplyFinished();

  void processChunk(const QJsonObject& chunk);
  void handleHttpError(int status);

private:
  QNetworkAccessManager& m_nam;
  QString m_apiKey;
  QByteArray m_requestBody;
  QNetworkReply* m_reply;
  SseEventReader* m_sse;
};

}  // namespace AI
