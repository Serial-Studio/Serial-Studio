/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

class QUrl;
class QNetworkRequest;

namespace AI {

/**
 * @brief Backend capability hints used to shape the assistant tool surface.
 */
struct ProviderCapabilities {
  bool structuredSystemBlocks = false;
  bool promptCaching          = false;
  bool parallelToolCalls      = false;
  bool thinking               = false;
  bool developerRole          = false;
  bool structuredToolResults  = false;
  bool needsSmallToolSurface  = true;
  bool slowFirstToken         = false;
  int toolResultByteBudget    = 4096;
  int contextWindowTokens     = 128000;
  int maxOutputTokens         = 8192;

  /**
   * @brief Output reservation for one request, capped at a quarter of the context window: an
   *        8k local model would otherwise reserve its entire window for output, the history
   *        budget would go negative, and the budgeter's "do not trim" fallback would hand the
   *        server a prompt it truncates from the front -- dropping the system prompt (J1).
   */
  [[nodiscard]] int budgetedOutputTokens() const noexcept
  {
    return qMin(maxOutputTokens, qMax(1, contextWindowTokens / 4));
  }

  /**
   * @brief System-prompt reservation, capped against the window for the same reason, so a small
   *        window still yields a positive history budget to cut against.
   */
  [[nodiscard]] int budgetedSystemReserve(int desiredTokens) const noexcept
  {
    return qMin(desiredTokens, qMax(1, contextWindowTokens / 4));
  }
};

/**
 * @brief Streamed reply handle returned by Provider::sendMessage. The finalization latch, the
 *        stream budget and the transport policy live here rather than in each backend: three
 *        verbatim copies drifted apart once already (J5, J6).
 */
class Reply : public QObject {
  Q_OBJECT

public:
  static constexpr qint64 kMaxStreamedReplyBytes = 8 * 1024 * 1024;

  explicit Reply(QObject* parent = nullptr)
    : QObject(parent), m_transientError(false), m_finished(false), m_streamedBytes(0)
  {}

  ~Reply() override = default;

  virtual void abort() = 0;

  [[nodiscard]] bool transientError() const noexcept { return m_transientError; }

  [[nodiscard]] static bool isTransportAllowed(const QUrl& url);
  [[nodiscard]] static bool endsTurnOnParseError(const QString& reason);
  static void applyStreamPolicy(QNetworkRequest& request);

signals:
  void partialText(const QString& chunk);
  void partialThinking(const QString& chunk);
  void thinkingBlockFinished(const QJsonObject& block);
  void toolCallRequested(const QString& callId,
                         const QString& toolName,
                         const QJsonObject& arguments,
                         const QJsonObject& extras = QJsonObject());
  void cacheStatsAvailable(int readTokens, int createdTokens);
  void finished();
  void errorOccurred(const QString& message);

protected:
  void setTransientError(bool transient) noexcept { m_transientError = transient; }

  [[nodiscard]] bool isFinished() const noexcept { return m_finished; }

  void finishOk();
  void finishWithError(const QString& message);
  [[nodiscard]] bool streamBudgetBreached(qsizetype bytes);

  /**
   * @brief Charges streamed bytes against the per-reply budget; a true return means the
   *        cumulative cap is breached and the reply must abort with an error. Transfer
   *        timeouts are inactivity-based, so a runaway server needs this hard cap.
   */
  [[nodiscard]] bool chargeStreamBudget(qsizetype bytes) noexcept
  {
    m_streamedBytes += static_cast<qint64>(bytes);
    return m_streamedBytes > kMaxStreamedReplyBytes;
  }

private:
  bool m_transientError;
  bool m_finished;
  qint64 m_streamedBytes;
};

/**
 * @brief Abstract chat-completion backend (Anthropic, OpenAI, Gemini).
 */
class Provider {
public:
  Provider()                           = default;
  virtual ~Provider()                  = default;
  Provider(Provider&&)                 = delete;
  Provider(const Provider&)            = delete;
  Provider& operator=(Provider&&)      = delete;
  Provider& operator=(const Provider&) = delete;

  [[nodiscard]] virtual QString displayName() const  = 0;
  [[nodiscard]] virtual QString keyVendorUrl() const = 0;

  [[nodiscard]] virtual QStringList availableModels() const = 0;
  [[nodiscard]] virtual QString defaultModel() const        = 0;

  /**
   * @brief Returns a user-friendly label for a model id (e.g. "Claude Haiku 4.5").
   */
  [[nodiscard]] virtual QString modelDisplayName(const QString& modelId) const { return modelId; }

  [[nodiscard]] virtual ProviderCapabilities capabilities() const { return {}; }

  [[nodiscard]] QString currentModel() const
  {
    return m_currentModel.isEmpty() ? defaultModel() : m_currentModel;
  }

  virtual void setCurrentModel(const QString& model)
  {
    if (model.isEmpty() || !availableModels().contains(model))
      return;

    m_currentModel = model;
  }

  [[nodiscard]] virtual Reply* sendMessage(const QJsonArray& history,
                                           const QJsonArray& tools,
                                           bool forbidToolUse = false) = 0;

protected:
  QString m_currentModel;
};

}  // namespace AI
