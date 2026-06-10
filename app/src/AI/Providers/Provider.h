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
  int toolResultByteBudget    = 4096;
};

/**
 * @brief Streamed reply handle returned by Provider::sendMessage.
 */
class Reply : public QObject {
  Q_OBJECT

public:
  explicit Reply(QObject* parent = nullptr) : QObject(parent), m_transientError(false) {}

  ~Reply() override = default;

  virtual void abort() = 0;

  [[nodiscard]] bool transientError() const noexcept { return m_transientError; }

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

private:
  bool m_transientError;
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
