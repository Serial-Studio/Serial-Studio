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

#include "AI/Conversation/HelpFetcher.h"
#include "AI/SentinelProbe.h"

namespace AI {

class Provider;
class Reply;
class ToolDispatcher;

/**
 * @brief Owns the chat history, the active streaming Reply, and the tool-call loop state.
 */
class Conversation : public QObject {
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

  /**
   * @brief Status pill rendered by QML for each tool-call card.
   */
  enum class CallStatus : int {
    Running         = 0,
    AwaitingConfirm = 1,
    Done            = 2,
    Error           = 3,
    Denied          = 4,
    Blocked         = 5,
  };
  Q_ENUM(CallStatus)

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
                          CallStatus status);
  void updateToolCallCard(const QString& callId,
                          CallStatus status,
                          const QJsonObject& result       = {},
                          const QJsonObject& verification = {});
  void runToolCall(const QString& callId, const QString& name, const QJsonObject& arguments);
  [[nodiscard]] QJsonObject runAutoVerify(const QString& name,
                                          const QJsonObject& arguments,
                                          const QJsonObject& reply);
  [[nodiscard]] QJsonObject verifySourceUpdate(const QJsonObject& arguments);
  void recordToolResult(const QString& callId, const QString& name, const QJsonObject& payload);
  bool dispatchMetaTool(const QString& callId, const QString& name, const QJsonObject& arguments);
  void dispatchByCallSafety(const QString& callId,
                            const QString& name,
                            const QJsonObject& arguments);
  void runMetaDescribe(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runMetaScriptingDocs(const QString& callId,
                            const QString& name,
                            const QJsonObject& arguments);
  void runMetaHowTo(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runMetaListCategories(const QString& callId,
                             const QString& name,
                             const QJsonObject& arguments);
  void runMetaSnapshot(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runMetaListCommands(const QString& callId,
                           const QString& name,
                           const QJsonObject& arguments);
  void runMetaExecuteCommand(const QString& callId,
                             const QString& name,
                             const QJsonObject& arguments);
  void runMetaLoadSkill(const QString& callId, const QString& name, const QJsonObject& arguments);
  void runMetaSearchDocs(const QString& callId, const QString& name, const QJsonObject& arguments);
  void resumeAfterToolBatch();
  void releaseOutstandingToolResult();
  void teardownReply();
  [[nodiscard]] bool shouldRetryAfterError() const;
  void scheduleTransientRetry(const QString& message);

  void setBusy(bool busy);
  void setAwaitingConfirmation(bool flag);
  void setLastError(const QString& message);
  void flushPendingStreamUpdate();
  void scheduleUiFlush();
  [[nodiscard]] QJsonArray dispatcherTools() const;
  [[nodiscard]] QJsonArray budgetedHistory(const QJsonArray& tools) const;

  /**
   * @brief Captured Confirm-state info pending user approval.
   */
  struct PendingCall {
    QString name;
    QJsonObject arguments;
  };

private:
  Provider* m_provider;
  ToolDispatcher* m_dispatcher;
  Reply* m_reply;

  QJsonArray m_history;
  QVariantList m_uiMessages;
  int m_assistantIndex;
  QString m_assistantText;
  QString m_assistantThinking;
  bool m_thinkingIsSynthetic;

  QJsonArray m_pendingThinkingBlocks;
  QJsonArray m_pendingToolUseBlocks;
  QJsonArray m_pendingToolResultBlocks;
  int m_outstandingToolResults;
  int m_toolCallCount;
  int m_retryCount;
  quint64 m_turnGeneration;
  bool m_cancelled;
  bool m_summaryForced;
  bool m_busy;
  QString m_lastError;

  QHash<QString, PendingCall> m_awaitingConfirm;
  bool m_lastAwaitingFlag;

  HelpFetcher m_helpFetcher;

  QTimer* m_streamFlushTimer;
  bool m_streamDirty;
  bool m_uiDirty;

  QTimer* m_autoSaveTimer;

  QString m_handoffSeed;
  QSet<QString> m_loadedSkills;
  SentinelProbe m_probe;
};

}  // namespace AI
