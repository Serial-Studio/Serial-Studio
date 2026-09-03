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

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include "AI/Conversation/AsyncToolRunner.h"
#include "AI/Conversation/AutoVerifier.h"
#include "AI/Conversation/HelpFetcher.h"
#include "AI/Conversation/MetaToolRunner.h"
#include "AI/Conversation/ToolTurnRunner.h"
#include "AI/SentinelProbe.h"

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace AI {

class Reply;
class Provider;
class Assistant;
class ToolDispatcher;
class CommandRegistry;

/**
 * @brief Owns the chat history, the active streaming Reply, and the tool-call loop state.
 *        Implements MetaToolSink so the meta.* runner reaches the card/tool-result
 *        bookkeeping without ever touching the streaming state kept private here.
 */
class Conversation
  : public QObject
  , public MetaToolSink {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QVariantList messages
             READ messages
             NOTIFY messagesChanged)
  Q_PROPERTY(int messageCount
             READ messageCount
             NOTIFY messageCountChanged)
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(bool awaitingConfirmation
             READ awaitingConfirmation
             NOTIFY awaitingConfirmationChanged)
  Q_PROPERTY(QString lastError
             READ lastError
             NOTIFY lastErrorChanged)
  // clang-format on

public:
  static constexpr int kMaxToolCalls        = 25;
  static constexpr int kMaxDigestChars      = 600;
  static constexpr int kMaxHistoryItems     = 400;
  static constexpr int kMaxUiMessageRows    = 600;
  static constexpr int kMaxTransientRetries = 2;
  static constexpr int kSystemReserveTokens = 28000;
  static constexpr int kStreamFlushMs       = 33;
  static constexpr int kAutoSaveDebounceMs  = 800;
  static constexpr int kRetryBaseMs         = 1500;

  using CallStatus = ToolCallStatus;

  explicit Conversation(QObject* parent = nullptr);
  ~Conversation() override;

  void setProvider(Provider* provider);
  void setDispatcher(ToolDispatcher* dispatcher);

  [[nodiscard]] QJsonObject snapshot() const;
  void loadSnapshot(const QJsonObject& doc);
  [[nodiscard]] QString firstUserText() const;
  [[nodiscard]] int messageCount() const noexcept;

  void setHandoffSeed(const QString& digest);
  [[nodiscard]] QString handoffSeed() const;
  [[nodiscard]] QString buildHandoffDigest() const;
  [[nodiscard]] bool probeDegraded() const noexcept;
  [[nodiscard]] QString probeDetail() const;

  [[nodiscard]] QVariantList messages() const;
  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] bool awaitingConfirmation() const noexcept;
  [[nodiscard]] QString lastError() const noexcept;

signals:
  void messagesChanged();
  void messageCountChanged();
  void busyChanged();
  void awaitingConfirmationChanged();
  void lastErrorChanged();
  void errorOccurred(const QString& message);
  void probeStateChanged();
  void memoryProposed(const QString& category, const QString& text);

public slots:
  void start(const QString& userText);
  void cancel();
  void approveToolCall(const QString& callId);
  void denyToolCall(const QString& callId);
  void approveToolCallGroup(const QString& family);
  void denyToolCallGroup(const QString& family);
  void clear();

private slots:
  void onPartialText(const QString& chunk);
  void onPartialThinking(const QString& chunk);
  void onThinkingBlockFinished(const QJsonObject& block);
  void onToolCallRequested(const QString& callId,
                           const QString& name,
                           const QJsonObject& arguments,
                           const QJsonObject& extras = QJsonObject());
  void onReplyFinished();
  void onReplyError(const QString& message);
  void onHelpFetchFinished(const QString& callId, const QJsonObject& result);
  void onAsyncToolFinished(const QString& callId,
                           const QString& name,
                           const QJsonObject& arguments,
                           const QJsonObject& reply,
                           quint64 generation);

private:
  void issueRequest();
  void ageHistoryToolResults();
  void pruneHistory();
  void reconcileHistoryToolPairs();
  void injectRoutedSkill(const QString& userText);
  void maybeProposeMemory(const QString& userText);
  void evaluateProbe();
  [[nodiscard]] QString probeComplianceKey() const;
  void appendUserMessage(const QString& text);
  void beginAssistantMessage();
  void appendToolCallCard(const QString& callId,
                          const QString& name,
                          const QJsonObject& arguments,
                          CallStatus status) override;
  void updateToolCallCard(const QString& callId,
                          CallStatus status,
                          const QJsonObject& result       = {},
                          const QJsonObject& verification = {}) override;
  void runToolCall(const QString& callId, const QString& name, const QJsonObject& arguments);
  [[nodiscard]] bool runToolCallAsync(const QString& callId,
                                      const QString& name,
                                      const QJsonObject& arguments);
  void finishToolCall(const QString& callId,
                      const QString& name,
                      const QJsonObject& arguments,
                      const QJsonObject& reply);
  void maybeResumeAfterToolBatch();
  void recordToolResult(const QString& callId,
                        const QString& name,
                        const QJsonObject& payload) override;
  void dispatchByCallSafety(const QString& callId,
                            const QString& name,
                            const QJsonObject& arguments) override;
  void noteSkillLoaded(const QString& skillId) override;
  [[nodiscard]] QList<DocSearch::Hit> searchDocs(const QString& query, int k) const override;
  void resumeAfterToolBatch();
  void releaseOutstandingToolResult() override;
  void teardownReply();
  [[nodiscard]] bool shouldRetryAfterError() const;
  void scheduleTransientRetry(const QString& message);

  void setBusy(bool busy);
  void setAwaitingConfirmation(bool flag);
  void setLastError(const QString& message);
  void flushPendingStreamUpdate();
  void scheduleUiFlush();
  [[nodiscard]] static Assistant& assistant();
  [[nodiscard]] QJsonArray dispatcherTools() const;
  [[nodiscard]] QJsonArray budgetedHistory(const QJsonArray& tools) const;

private:
  Provider* m_provider;
  ToolDispatcher* m_dispatcher;
  Reply* m_reply;

  const CommandRegistry& m_commands;
  DataModel::ProjectModel& m_project;

  QJsonArray m_history;
  QVariantList m_uiMessages;
  int m_assistantIndex;
  QString m_assistantText;
  QString m_assistantThinking;
  bool m_thinkingIsSynthetic;

  QJsonArray m_pendingThinkingBlocks;
  QJsonArray m_pendingToolUseBlocks;
  QJsonArray m_pendingToolResultBlocks;
  int m_toolCallCount;
  int m_retryCount;
  quint64 m_turnGeneration;
  bool m_cancelled;
  bool m_summaryForced;
  bool m_busy;
  QString m_lastError;

  bool m_lastAwaitingFlag;

  ToolTurnRunner m_tools;
  HelpFetcher m_helpFetcher;
  MetaToolRunner m_metaTools;
  AutoVerifier m_autoVerify;

  QTimer* m_streamFlushTimer;
  bool m_streamDirty;
  bool m_uiDirty;

  QTimer* m_autoSaveTimer;

  QString m_handoffSeed;
  QSet<QString> m_loadedSkills;
  SentinelProbe m_probe;

  AsyncToolRunner m_asyncTools;
};

}  // namespace AI
