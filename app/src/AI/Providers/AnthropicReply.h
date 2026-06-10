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
 * @brief Real Anthropic Messages API streaming Reply.
 */
class AnthropicReply : public Reply {
  Q_OBJECT

public:
  AnthropicReply(QNetworkAccessManager& nam,
                 const QString& apiKey,
                 const QByteArray& requestBody,
                 QObject* parent = nullptr);

  void abort() override;

  [[nodiscard]] QString stopReason() const noexcept;

private:
  /**
   * @brief Per-content-block state captured between start and stop events.
   */
  struct BlockState {
    QString type;
    QString toolUseId;
    QString toolUseName;
    QByteArray toolInputJson;
    QString thinkingText;
    QString thinkingSignature;
    QString redactedData;
  };

  void onSseEvent(const QString& name, const QJsonObject& data);
  void onSseError(const QString& reason);
  void onReplyReadyRead();
  void onReplyFinished();
  void onReplyError();

  void handleMessageStart(const QJsonObject& data);
  void handleContentBlockStart(const QJsonObject& data);
  void handleContentBlockDelta(const QJsonObject& data);
  void handleContentBlockStop(const QJsonObject& data);
  void emitToolUseFromBlock(const BlockState& bs);

  void finishOk();
  void finishWithError(const QString& message);

private:
  QNetworkAccessManager& m_nam;
  QString m_apiKey;
  QByteArray m_requestBody;
  QNetworkReply* m_reply;
  SseEventReader* m_sse;
  QHash<int, BlockState> m_blocks;
  QString m_stopReason;
  bool m_finished;
};

}  // namespace AI
