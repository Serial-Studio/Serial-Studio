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

#include <memory>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "AI/ChatStore.h"
#include "AI/KeyVault.h"
#include "AI/MemoryStore.h"

namespace AI {

class CommandRegistry;
class Conversation;
class Provider;
class ToolDispatcher;

/**
 * @brief Application-wide singleton wiring the AI Assistant feature.
 */
class Assistant : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool hasAnyKey
             READ hasAnyKey
             NOTIFY keysChanged)
  Q_PROPERTY(int currentProvider
             READ currentProvider
             WRITE setCurrentProvider
             NOTIFY currentProviderChanged)
  Q_PROPERTY(bool isProAvailable
             READ isProAvailable
             CONSTANT)
  Q_PROPERTY(QStringList providerNames
             READ providerNames
             CONSTANT)
  Q_PROPERTY(QObject* conversation
             READ conversationObject
             CONSTANT)
  Q_PROPERTY(QVariantList chatList
             READ chatList
             NOTIFY chatListChanged)
  Q_PROPERTY(QString activeChatId
             READ activeChatId
             NOTIFY activeChatChanged)
  Q_PROPERTY(bool busy
             READ busy
             NOTIFY busyChanged)
  Q_PROPERTY(int cacheReadTokens
             READ cacheReadTokens
             NOTIFY cacheStatsChanged)
  Q_PROPERTY(int cacheCreatedTokens
             READ cacheCreatedTokens
             NOTIFY cacheStatsChanged)
  Q_PROPERTY(bool autoApproveEdits
             READ autoApproveEdits
             WRITE setAutoApproveEdits
             NOTIFY autoApproveEditsChanged)
  Q_PROPERTY(bool allowDeviceControl
             READ allowDeviceControl
             WRITE setAllowDeviceControl
             NOTIFY allowDeviceControlChanged)
  Q_PROPERTY(bool contextProbeEnabled
             READ contextProbeEnabled
             WRITE setContextProbeEnabled
             NOTIFY contextProbeEnabledChanged)
  Q_PROPERTY(bool memoryEnabled
             READ memoryEnabled
             WRITE setMemoryEnabled
             NOTIFY memoryEnabledChanged)
  Q_PROPERTY(bool handoffSeedingEnabled
             READ handoffSeedingEnabled
             WRITE setHandoffSeedingEnabled
             NOTIFY handoffSeedingEnabledChanged)
  Q_PROPERTY(bool skillRoutingEnabled
             READ skillRoutingEnabled
             WRITE setSkillRoutingEnabled
             NOTIFY skillRoutingEnabledChanged)
  Q_PROPERTY(bool autoVerifyEnabled
             READ autoVerifyEnabled
             WRITE setAutoVerifyEnabled
             NOTIFY autoVerifyEnabledChanged)
  Q_PROPERTY(bool contextDegraded
             READ contextDegraded
             NOTIFY contextHealthChanged)
  Q_PROPERTY(QString degradationDetail
             READ degradationDetail
             NOTIFY contextHealthChanged)
  Q_PROPERTY(bool activeChatSeeded
             READ activeChatSeeded
             NOTIFY activeChatChanged)
  // clang-format on

signals:
  void keysChanged();
  void currentProviderChanged();
  void requestKeyManager();
  void errorOccurred(const QString& message);
  void busyChanged();
  void cacheStatsChanged();
  void autoApproveEditsChanged();
  void allowDeviceControlChanged();
  void contextProbeEnabledChanged();
  void memoryEnabledChanged();
  void handoffSeedingEnabledChanged();
  void skillRoutingEnabledChanged();
  void autoVerifyEnabledChanged();
  void contextHealthChanged();
  void memoryChanged();
  void memoryProposed(const QString& category, const QString& text);
  void droppedPathAdded(const QString& displayName, bool isDir);
  void chatListChanged();
  void activeChatChanged();

private:
  explicit Assistant();
  Assistant(Assistant&&)                 = delete;
  Assistant(const Assistant&)            = delete;
  Assistant& operator=(Assistant&&)      = delete;
  Assistant& operator=(const Assistant&) = delete;

public:
  ~Assistant() override;

  [[nodiscard]] static Assistant& instance();

  [[nodiscard]] int currentProvider() const noexcept;
  [[nodiscard]] bool hasAnyKey() const;
  [[nodiscard]] bool isProAvailable() const;
  [[nodiscard]] QStringList providerNames() const;
  [[nodiscard]] bool busy() const noexcept;
  [[nodiscard]] QObject* conversationObject() const noexcept;
  [[nodiscard]] QVariantList chatList() const;
  [[nodiscard]] QString activeChatId() const noexcept;
  [[nodiscard]] int cacheReadTokens() const noexcept;
  [[nodiscard]] int cacheCreatedTokens() const noexcept;
  [[nodiscard]] bool autoApproveEdits() const noexcept;
  [[nodiscard]] bool allowDeviceControl() const noexcept;
  [[nodiscard]] bool contextProbeEnabled() const noexcept;
  [[nodiscard]] bool memoryEnabled() const noexcept;
  [[nodiscard]] bool handoffSeedingEnabled() const noexcept;
  [[nodiscard]] bool skillRoutingEnabled() const noexcept;
  [[nodiscard]] bool autoVerifyEnabled() const noexcept;
  [[nodiscard]] bool contextDegraded() const;
  [[nodiscard]] QString degradationDetail() const;
  [[nodiscard]] bool activeChatSeeded() const;
  [[nodiscard]] QString memoryIndex() const;
  [[nodiscard]] QString handoffSeedText() const;

  Q_INVOKABLE [[nodiscard]] bool hasKey(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QString redactedKey(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QString keyVendorUrl(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QStringList availableModels(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QString currentModel(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QString modelDisplayName(int providerIdx, const QString& modelId) const;
  Q_INVOKABLE [[nodiscard]] bool requiresApiKey(int providerIdx) const;
  Q_INVOKABLE [[nodiscard]] QString localBaseUrl() const;
  Q_INVOKABLE [[nodiscard]] QVariantList memoryList() const;
  Q_INVOKABLE [[nodiscard]] bool addMemory(const QString& category, const QString& text);
  Q_INVOKABLE [[nodiscard]] bool updateMemory(const QString& id, const QString& text);

  void reportCacheStats(int readTokens, int createdTokens);

  void setCurrentProvider(int providerIdx);
  void setAutoApproveEdits(bool enabled);
  void setAllowDeviceControl(bool enabled);
  void setContextProbeEnabled(bool enabled);
  void setMemoryEnabled(bool enabled);
  void setHandoffSeedingEnabled(bool enabled);
  void setSkillRoutingEnabled(bool enabled);
  void setAutoVerifyEnabled(bool enabled);

public slots:
  void openKeyManager();
  void selectProvider(int idx);
  void requestProviderSwitch(int idx);
  void setKey(int providerIdx, const QString& plaintext);
  void setModel(int providerIdx, const QString& model);
  void clearKey(int providerIdx);
  void setLocalBaseUrl(const QString& url);
  void refreshLocalModels();
  void sendMessage(const QString& userText);
  void cancel();
  void approveToolCall(const QString& callId);
  void denyToolCall(const QString& callId);
  void approveToolCallGroup(const QString& family);
  void denyToolCallGroup(const QString& family);
  void clearConversation();
  void newChat();
  void newChatFromHandoff(const QString& id);
  void deleteMemory(const QString& id);
  void switchChat(const QString& id);
  void deleteChat(const QString& id);
  void renameChat(const QString& id, const QString& title);
  void addDroppedPath(const QString& localPath);
  void clearDroppedPaths();

private slots:
  void onConversationBusyChanged();
  void onConversationError(const QString& message);

private:
  void rebuildProviders();
  void rewireConversationProvider();
  void restoreModelSelections();
  void initChats();
  void loadActiveSnapshot();
  void persistActiveChat();
  [[nodiscard]] Provider* providerAt(int idx) const;
  [[nodiscard]] static QString modelSettingsKey(int providerIdx);

private:
  CommandRegistry& m_commandRegistry;
  KeyVault m_vault;
  mutable QSettings m_settings;
  std::unique_ptr<QNetworkAccessManager> m_nam;
  std::unique_ptr<Provider> m_anthropic;
  std::unique_ptr<Provider> m_openai;
  std::unique_ptr<Provider> m_gemini;
  std::unique_ptr<Provider> m_deepseek;
  std::unique_ptr<Provider> m_openrouter;
  std::unique_ptr<Provider> m_groq;
  std::unique_ptr<Provider> m_mistral;
  std::unique_ptr<Provider> m_local;
  std::unique_ptr<ToolDispatcher> m_dispatcher;
  std::unique_ptr<Conversation> m_conversation;
  ChatStore m_chats;
  MemoryStore m_memory;
  QString m_activeChatId;
  int m_currentProvider;
  int m_cacheReadTokens;
  int m_cacheCreatedTokens;
  bool m_autoApproveEdits;
  bool m_allowDeviceControl;
  bool m_contextProbeEnabled;
  bool m_memoryEnabled;
  bool m_handoffSeedingEnabled;
  bool m_skillRoutingEnabled;
  bool m_autoVerifyEnabled;
};

}  // namespace AI
