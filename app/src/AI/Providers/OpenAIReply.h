/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QString>

#include "AI/Providers/Provider.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace AI {

class SseEventReader;

/**
 * @brief Real OpenAI Chat Completions streaming Reply.
 */
class OpenAIReply : public Reply {
  Q_OBJECT

public:
  OpenAIReply(QNetworkAccessManager& nam,
              const QString& apiKey,
              const QByteArray& requestBody,
              QObject* parent = nullptr);

  OpenAIReply(QNetworkAccessManager& nam,
              const QString& endpointUrl,
              const QString& authHeader,
              const QString& apiKey,
              const QByteArray& requestBody,
              const QString& providerLabel,
              int transferTimeoutMs = -1,
              bool parseThinkTags   = false,
              QObject* parent       = nullptr);

  void abort() override;

private:
  /**
   * @brief Per-tool-call accumulator keyed by the streamed `index`.
   */
  struct ToolCallState {
    QString id;
    QString name;
    QByteArray arguments;
    bool emitted;
  };

  /**
   * @brief Scanner state for inline <think>...</think> blocks in the content stream.
   */
  enum class ThinkScan : int {
    Detect      = 0,
    Thinking    = 1,
    Passthrough = 2,
  };

  void onSseEvent(const QString& name, const QJsonObject& data);
  void onSseError(const QString& reason);
  void onReplyReadyRead();
  void onReplyFinished();

  void processChoiceDelta(const QJsonObject& choice);
  void routeContentChunk(const QString& chunk);
  void processThinkCarry(bool atEnd);
  void emitPendingToolCalls();
  void finishOk();
  void finishWithError(const QString& message);

private:
  void issueRequest();

private:
  QNetworkAccessManager& m_nam;
  QString m_endpointUrl;
  QString m_authHeader;
  QString m_apiKey;
  QString m_providerLabel;
  QByteArray m_requestBody;
  QNetworkReply* m_reply;
  SseEventReader* m_sse;
  QHash<int, ToolCallState> m_toolCalls;
  QString m_finishReason;
  int m_transferTimeoutMs;
  bool m_parseThinkTags;
  ThinkScan m_thinkScan;
  QString m_thinkCarry;
  bool m_finished;
};

}  // namespace AI
