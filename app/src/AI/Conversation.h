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
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

class QUrl;
class QNetworkReply;

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
  static constexpr int kMaxToolCalls = 25;

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

  [[nodiscard]] QVariantList messages() const;
  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] bool awaitingConfirmation() const noexcept;
  [[nodiscard]] QString lastError() const noexcept;

signals:
  void messagesChanged();
  void busyChanged();
  void awaitingConfirmationChanged();
  void lastErrorChanged();
  void errorOccurred(const QString& message);

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
  void onToolCallRequested(const QString& callId,
                           const QString& name,
                           const QJsonObject& arguments);
  void onReplyFinished();
  void onReplyError(const QString& message);

private:
  void issueRequest();
  void ageHistoryToolResults();
  void reconcileHistoryToolPairs();
  bool reconcileHistoryToolPairsAt(int& i);
  void fetchHelpPage(const QString& callId, const QString& path);
  void fetchHelpIndex(const QString& callId, const QUrl& missedUrl);
  void completeHelpFetch(const QString& callId, const QUrl& url, QNetworkReply* reply);
  void appendUserMessage(const QString& text);
  void beginAssistantMessage();
  void appendToolCallCard(const QString& callId,
                          const QString& name,
                          const QJsonObject& arguments,
                          CallStatus status);
  void updateToolCallCard(const QString& callId, CallStatus status, const QJsonObject& result = {});
  void runToolCall(const QString& callId,
                   const QString& name,
                   const QJsonObject& arguments,
                   bool autoConfirmSafe);
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
  void teardownReply();

  [[nodiscard]] static QString rewriteHelpLinks(const QString& text);
  void setBusy(bool busy);
  void setAwaitingConfirmation(bool flag);
  void setLastError(const QString& message);
  void flushPendingStreamUpdate();
  [[nodiscard]] QJsonArray dispatcherTools() const;

  [[nodiscard]] static QString persistencePath();
  void saveToDisk() const;
  void restoreFromDisk();

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

  QJsonArray m_pendingToolUseBlocks;
  QJsonArray m_pendingToolResultBlocks;
  int m_outstandingToolResults;
  int m_toolCallCount;
  bool m_cancelled;
  bool m_summaryForced;
  bool m_busy;
  QString m_lastError;
  QString m_currentStopReason;

  QHash<QString, PendingCall> m_awaitingConfirm;
  bool m_lastAwaitingFlag;

  QNetworkAccessManager m_helpFetchNam;

  QTimer* m_streamFlushTimer;
  bool m_streamDirty;

  QTimer* m_autoSaveTimer;
};

}  // namespace AI
