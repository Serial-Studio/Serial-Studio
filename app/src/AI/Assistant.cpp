/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Assistant.h"

#include <QFileInfo>
#include <QMessageBox>

#include "AI/CommandRegistry.h"
#include "AI/Conversation.h"
#include "AI/FileSandbox.h"
#include "AI/Logging.h"
#include "AI/Providers/AnthropicProvider.h"
#include "AI/Providers/DeepSeekProvider.h"
#include "AI/Providers/GeminiProvider.h"
#include "AI/Providers/GroqProvider.h"
#include "AI/Providers/LocalProvider.h"
#include "AI/Providers/MistralProvider.h"
#include "AI/Providers/OpenAIProvider.h"
#include "AI/Providers/OpenRouterProvider.h"
#include "AI/ToolDispatcher.h"
#include "Licensing/CommercialToken.h"
#include "Misc/Utilities.h"

static constexpr int kTitleLimit = 60;

//--------------------------------------------------------------------------------------------------
// Singleton accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide AI Assistant.
 */
AI::Assistant& AI::Assistant::instance()
{
  static Assistant singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Construction and destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the QNAM, dispatcher, providers, and conversation.
 */
AI::Assistant::Assistant()
  : QObject(nullptr)
  , m_currentProvider(0)
  , m_cacheReadTokens(0)
  , m_cacheCreatedTokens(0)
  , m_autoApproveEdits(false)
  , m_allowDeviceControl(false)
{
  m_nam          = std::make_unique<QNetworkAccessManager>(this);
  m_dispatcher   = std::make_unique<ToolDispatcher>(this);
  m_conversation = std::make_unique<Conversation>(this);

  rebuildProviders();
  restoreModelSelections();

  const int storedProvider = m_settings.value(QStringLiteral("ai/currentProvider"), 0).toInt();
  if (storedProvider >= 0 && storedProvider < kProviderCount)
    m_currentProvider = storedProvider;

  m_autoApproveEdits = m_settings.value(QStringLiteral("ai/autoApproveEdits"), false).toBool();

  m_allowDeviceControl = m_settings.value(QStringLiteral("ai/allowDeviceControl"), false).toBool();
  CommandRegistry::instance().setDeviceControlAllowed(m_allowDeviceControl);

  m_conversation->setDispatcher(m_dispatcher.get());
  rewireConversationProvider();

  connect(
    m_conversation.get(), &Conversation::busyChanged, this, &Assistant::onConversationBusyChanged);
  connect(
    m_conversation.get(), &Conversation::errorOccurred, this, &Assistant::onConversationError);

  initChats();
}

/**
 * @brief Persists the active chat, then releases owned resources.
 */
AI::Assistant::~Assistant()
{
  if (m_conversation && !m_activeChatId.isEmpty())
    m_chats.saveChat(m_activeChatId,
                     m_conversation->snapshot(),
                     m_conversation->firstUserText().left(kTitleLimit),
                     m_conversation->messageCount());
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected provider.
 */
int AI::Assistant::currentProvider() const noexcept
{
  return m_currentProvider;
}

/**
 * @brief Returns true if any provider has a stored key, or the Local provider is wired.
 */
bool AI::Assistant::hasAnyKey() const
{
  if (m_vault.hasAnyKey())
    return true;

  return m_local != nullptr;
}

/**
 * @brief Returns true when the AI Assistant is available: build-integrity gated only, with no
 *        license tier required, since the user supplies the API key for their chosen provider.
 */
bool AI::Assistant::isProAvailable() const
{
  return SS_LICENSE_GUARD();
}

/**
 * @brief Returns true while the conversation is awaiting a reply.
 */
bool AI::Assistant::busy() const noexcept
{
  return m_conversation && m_conversation->busy();
}

/**
 * @brief Returns the Conversation as a QObject* for QML binding.
 */
QObject* AI::Assistant::conversationObject() const noexcept
{
  return m_conversation.get();
}

/**
 * @brief Returns the saved chats as metadata rows for QML, newest first.
 */
QVariantList AI::Assistant::chatList() const
{
  return m_chats.list();
}

/**
 * @brief Returns the id of the currently displayed chat.
 */
QString AI::Assistant::activeChatId() const noexcept
{
  return m_activeChatId;
}

/**
 * @brief Returns the most recent cache_read_input_tokens reported by a provider.
 */
int AI::Assistant::cacheReadTokens() const noexcept
{
  return m_cacheReadTokens;
}

/**
 * @brief Returns the most recent cache_creation_input_tokens reported by a provider.
 */
int AI::Assistant::cacheCreatedTokens() const noexcept
{
  return m_cacheCreatedTokens;
}

/**
 * @brief Returns whether Confirm-gated edits run without per-call approval.
 */
bool AI::Assistant::autoApproveEdits() const noexcept
{
  return m_autoApproveEdits;
}

/**
 * @brief Persists the toggle and notifies QML listeners.
 */
void AI::Assistant::setAutoApproveEdits(bool enabled)
{
  if (m_autoApproveEdits == enabled)
    return;

  m_autoApproveEdits = enabled;
  m_settings.setValue(QStringLiteral("ai/autoApproveEdits"), enabled);
  Q_EMIT autoApproveEditsChanged();
}

/**
 * @brief Returns whether device-gated tools (driver config, writes, connect) are unblocked.
 */
bool AI::Assistant::allowDeviceControl() const noexcept
{
  return m_allowDeviceControl;
}

/**
 * @brief Persists the device-control gate, warning the user before enabling it.
 */
void AI::Assistant::setAllowDeviceControl(bool enabled)
{
  if (m_allowDeviceControl == enabled)
    return;

  if (enabled) {
    const int result = Misc::Utilities::showMessageBox(
      tr("Allow AI Device Control?"),
      tr("This lets the AI assistant configure devices, open and close connections, "
         "and send data to your hardware.\n\n"
         "Every device action still requires your explicit per-call approval in the "
         "chat, even when auto-approve is enabled. Only enable this if you trust the "
         "configured AI provider with hardware access."),
      QMessageBox::Warning,
      QString(),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);

    if (result == QMessageBox::No) {
      m_allowDeviceControl = false;
      m_settings.setValue(QStringLiteral("ai/allowDeviceControl"), false);
      Q_EMIT allowDeviceControlChanged();
      return;
    }
  }

  m_allowDeviceControl = enabled;
  m_settings.setValue(QStringLiteral("ai/allowDeviceControl"), m_allowDeviceControl);
  CommandRegistry::instance().setDeviceControlAllowed(m_allowDeviceControl);
  Q_EMIT allowDeviceControlChanged();
}

/**
 * @brief Returns the display names of the registered providers in order.
 */
QStringList AI::Assistant::providerNames() const
{
  QStringList names;
  if (m_anthropic)
    names.append(m_anthropic->displayName());

  if (m_openai)
    names.append(m_openai->displayName());

  if (m_gemini)
    names.append(m_gemini->displayName());

  if (m_deepseek)
    names.append(m_deepseek->displayName());

  if (m_openrouter)
    names.append(m_openrouter->displayName());

  if (m_groq)
    names.append(m_groq->displayName());

  if (m_mistral)
    names.append(m_mistral->displayName());

  if (m_local)
    names.append(m_local->displayName());

  return names;
}

//--------------------------------------------------------------------------------------------------
// QML-invokable per-provider queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true when the provider at the given index is ready to use.
 */
bool AI::Assistant::hasKey(int providerIdx) const
{
  if (static_cast<ProviderId>(providerIdx) == ProviderId::Local)
    return m_local != nullptr;

  return m_vault.hasKey(static_cast<ProviderId>(providerIdx));
}

/**
 * @brief Returns a redacted form of the stored key safe to display.
 */
QString AI::Assistant::redactedKey(int providerIdx) const
{
  const auto k = m_vault.key(static_cast<ProviderId>(providerIdx));
  return KeyVault::redact(k);
}

/**
 * @brief Returns the vendor URL for the provider at the given index.
 */
QString AI::Assistant::keyVendorUrl(int providerIdx) const
{
  const auto* p = providerAt(providerIdx);
  return p ? p->keyVendorUrl() : QString();
}

/**
 * @brief Returns the available model list for the given provider.
 */
QStringList AI::Assistant::availableModels(int providerIdx) const
{
  const auto* p = providerAt(providerIdx);
  return p ? p->availableModels() : QStringList();
}

/**
 * @brief Returns the current model selection for the given provider.
 */
QString AI::Assistant::currentModel(int providerIdx) const
{
  const auto* p = providerAt(providerIdx);
  return p ? p->currentModel() : QString();
}

/**
 * @brief Returns a friendly label for a model id (e.g. "Claude Haiku 4.5").
 */
QString AI::Assistant::modelDisplayName(int providerIdx, const QString& modelId) const
{
  const auto* p = providerAt(providerIdx);
  return p ? p->modelDisplayName(modelId) : modelId;
}

/**
 * @brief Sets and persists the model for the given provider.
 */
void AI::Assistant::setModel(int providerIdx, const QString& model)
{
  auto* p = providerAt(providerIdx);
  if (!p)
    return;

  if (p->currentModel() == model)
    return;

  p->setCurrentModel(model);

  m_settings.beginGroup(QStringLiteral("ai/model"));
  m_settings.setValue(modelSettingsKey(providerIdx), p->currentModel());
  m_settings.endGroup();

  qCDebug(serialStudioAI) << "Model changed for provider" << providerIdx << "to"
                          << p->currentModel();
}

/**
 * @brief Updates the cache-stats properties from a Reply.
 */
void AI::Assistant::reportCacheStats(int readTokens, int createdTokens)
{
  if (m_cacheReadTokens == readTokens && m_cacheCreatedTokens == createdTokens)
    return;

  m_cacheReadTokens    = readTokens;
  m_cacheCreatedTokens = createdTokens;
  Q_EMIT cacheStatsChanged();
}

//--------------------------------------------------------------------------------------------------
// Mutators
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the active provider by index, persisting the choice.
 */
void AI::Assistant::setCurrentProvider(int providerIdx)
{
  if (m_currentProvider == providerIdx)
    return;

  m_currentProvider = providerIdx;
  m_settings.setValue(QStringLiteral("ai/currentProvider"), providerIdx);
  rewireConversationProvider();
  Q_EMIT currentProviderChanged();
}

/**
 * @brief Asks QML to display the Key Manager dialog.
 */
void AI::Assistant::openKeyManager()
{
  Q_EMIT requestKeyManager();
}

/**
 * @brief Slot variant of setCurrentProvider for QML.
 */
void AI::Assistant::selectProvider(int idx)
{
  setCurrentProvider(idx);
}

/**
 * @brief Switches provider, prompting via Misc::Utilities if a conversation is in progress.
 */
void AI::Assistant::requestProviderSwitch(int idx)
{
  if (idx == m_currentProvider)
    return;

  const bool hasHistory = m_conversation && !m_conversation->messages().isEmpty();

  if (hasHistory) {
    const auto choice = Misc::Utilities::showMessageBox(
      tr("Switch AI provider?"),
      tr("Switching to a different provider clears the current conversation. "
         "Do you want to continue?"),
      QMessageBox::Question,
      tr("Assistant"),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);

    if (choice != QMessageBox::Yes)
      return;

    m_conversation->clear();
  }

  setCurrentProvider(idx);
}

/**
 * @brief Stores an API key for the given provider, encrypted at rest.
 */
void AI::Assistant::setKey(int providerIdx, const QString& plaintext)
{
  m_vault.setKey(static_cast<ProviderId>(providerIdx), plaintext);
  Q_EMIT keysChanged();
}

/**
 * @brief Removes the stored key for the given provider.
 */
void AI::Assistant::clearKey(int providerIdx)
{
  m_vault.clearKey(static_cast<ProviderId>(providerIdx));
  Q_EMIT keysChanged();
}

/**
 * @brief Forwards a user message to the active conversation after the availability gate.
 */
void AI::Assistant::sendMessage(const QString& userText)
{
  if (!isProAvailable()) {
    Q_EMIT errorOccurred(tr("AI Assistant is not available in this build"));
    return;
  }

  if (!hasAnyKey()) {
    Q_EMIT errorOccurred(tr("Set an API key first"));
    return;
  }

  rewireConversationProvider();
  m_conversation->start(userText);
}

/**
 * @brief Aborts the in-flight reply, if any.
 */
void AI::Assistant::cancel()
{
  if (m_conversation)
    m_conversation->cancel();
}

/**
 * @brief Approves a Confirm-tagged tool call by id.
 */
void AI::Assistant::approveToolCall(const QString& callId)
{
  if (m_conversation)
    m_conversation->approveToolCall(callId);
}

/**
 * @brief Denies a Confirm-tagged tool call by id.
 */
void AI::Assistant::denyToolCall(const QString& callId)
{
  if (m_conversation)
    m_conversation->denyToolCall(callId);
}

/**
 * @brief Approves every pending Confirm whose tool name shares this family.
 */
void AI::Assistant::approveToolCallGroup(const QString& family)
{
  if (m_conversation)
    m_conversation->approveToolCallGroup(family);
}

/**
 * @brief Denies every pending Confirm whose tool name shares this family.
 */
void AI::Assistant::denyToolCallGroup(const QString& family)
{
  if (m_conversation)
    m_conversation->denyToolCallGroup(family);
}

/**
 * @brief Starts a fresh chat (kept as the legacy "clear" entry point).
 */
void AI::Assistant::clearConversation()
{
  newChat();
}

/**
 * @brief Persists the current chat, then opens a new empty one and switches to it.
 */
void AI::Assistant::newChat()
{
  if (m_conversation && m_conversation->messageCount() == 0 && !m_activeChatId.isEmpty())
    return;

  if (m_conversation && m_conversation->busy())
    m_conversation->cancel();

  persistActiveChat();

  m_activeChatId = m_chats.createChat();
  m_chats.setLastActiveId(m_activeChatId);
  if (m_conversation)
    m_conversation->clear();

  AI::FileSandbox::instance().clearDroppedPaths();

  Q_EMIT chatListChanged();
  Q_EMIT activeChatChanged();
}

/**
 * @brief Persists the current chat and loads another saved chat into the conversation.
 */
void AI::Assistant::switchChat(const QString& id)
{
  if (id == m_activeChatId || !m_chats.contains(id))
    return;

  if (m_conversation && m_conversation->busy())
    m_conversation->cancel();

  persistActiveChat();

  m_activeChatId = id;
  m_chats.setLastActiveId(id);
  loadActiveSnapshot();

  AI::FileSandbox::instance().clearDroppedPaths();
  Q_EMIT activeChatChanged();
}

/**
 * @brief Deletes a chat; if it was active, falls back to the newest remaining (or a new) chat.
 */
void AI::Assistant::deleteChat(const QString& id)
{
  if (!m_chats.contains(id))
    return;

  const bool wasActive = (id == m_activeChatId);
  m_chats.removeChat(id);

  if (wasActive) {
    const auto rows = m_chats.list();
    m_activeChatId  = rows.isEmpty() ? m_chats.createChat()
                                     : rows.first().toMap().value(QStringLiteral("id")).toString();
    m_chats.setLastActiveId(m_activeChatId);
    loadActiveSnapshot();
    Q_EMIT activeChatChanged();
  }

  Q_EMIT chatListChanged();
}

/**
 * @brief Sets a user-chosen title on a chat.
 */
void AI::Assistant::renameChat(const QString& id, const QString& title)
{
  m_chats.renameChat(id, title.trimmed());
  Q_EMIT chatListChanged();
}

/**
 * @brief Authorizes a user-dropped file/folder for sandboxed reads this session.
 */
void AI::Assistant::addDroppedPath(const QString& localPath)
{
  const auto canonical = AI::FileSandbox::instance().registerDroppedPath(localPath);
  if (canonical.isEmpty()) {
    qCWarning(serialStudioAI) << "Ignoring undroppable path:" << localPath;
    return;
  }

  const QFileInfo info(canonical);
  Q_EMIT droppedPathAdded(info.fileName(), info.isDir());
}

/**
 * @brief Drops every session-authorized path from the read allow-list.
 */
void AI::Assistant::clearDroppedPaths()
{
  AI::FileSandbox::instance().clearDroppedPaths();
}

/**
 * @brief Re-emits the conversation busy signal; persists the active chat when it goes idle.
 */
void AI::Assistant::onConversationBusyChanged()
{
  if (m_conversation && !m_conversation->busy())
    persistActiveChat();

  Q_EMIT busyChanged();
}

/**
 * @brief Re-emits conversation errors as the Assistant's.
 */
void AI::Assistant::onConversationError(const QString& message)
{
  Q_EMIT errorOccurred(message);
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the provider adapters with key-getter lambdas.
 */
void AI::Assistant::rebuildProviders()
{
  m_anthropic = std::make_unique<AnthropicProvider>(
    *m_nam, [this]() { return m_vault.key(ProviderId::Anthropic); });
  m_openai =
    std::make_unique<OpenAIProvider>(*m_nam, [this]() { return m_vault.key(ProviderId::OpenAI); });
  m_gemini =
    std::make_unique<GeminiProvider>(*m_nam, [this]() { return m_vault.key(ProviderId::Gemini); });
  m_deepseek = std::make_unique<DeepSeekProvider>(
    *m_nam, [this]() { return m_vault.key(ProviderId::DeepSeek); });
  m_openrouter = std::make_unique<OpenRouterProvider>(
    *m_nam, [this]() { return m_vault.key(ProviderId::OpenRouter); });
  m_groq =
    std::make_unique<GroqProvider>(*m_nam, [this]() { return m_vault.key(ProviderId::Groq); });
  m_mistral = std::make_unique<MistralProvider>(
    *m_nam, [this]() { return m_vault.key(ProviderId::Mistral); });

  auto* local = new LocalProvider(*m_nam);
  connect(local, &LocalProvider::modelsChanged, this, []() {
    qCDebug(serialStudioAI) << "Local model list updated";
  });
  m_local.reset(local);
}

/**
 * @brief Points the conversation at the currently selected provider.
 */
void AI::Assistant::rewireConversationProvider()
{
  if (!m_conversation)
    return;

  m_conversation->setProvider(providerAt(m_currentProvider));
}

/**
 * @brief Picks the active chat on startup (last active, newest, or a fresh one) and loads it.
 */
void AI::Assistant::initChats()
{
  if (m_chats.isEmpty()) {
    m_activeChatId = m_chats.createChat();
  } else {
    m_activeChatId = m_chats.lastActiveId();
    if (m_activeChatId.isEmpty() || !m_chats.contains(m_activeChatId))
      m_activeChatId = m_chats.list().first().toMap().value(QStringLiteral("id")).toString();
  }

  m_chats.setLastActiveId(m_activeChatId);
  loadActiveSnapshot();
}

/**
 * @brief Loads the active chat's snapshot into the conversation, or clears it when empty.
 */
void AI::Assistant::loadActiveSnapshot()
{
  if (!m_conversation)
    return;

  const auto snap = m_chats.loadChat(m_activeChatId);
  if (snap.isEmpty())
    m_conversation->clear();
  else
    m_conversation->loadSnapshot(snap);
}

/**
 * @brief Writes the conversation's current state back to the active chat file.
 */
void AI::Assistant::persistActiveChat()
{
  if (!m_conversation || m_activeChatId.isEmpty())
    return;

  m_chats.saveChat(m_activeChatId,
                   m_conversation->snapshot(),
                   m_conversation->firstUserText().left(kTitleLimit),
                   m_conversation->messageCount());
  Q_EMIT chatListChanged();
}

/**
 * @brief Loads each provider's previously-selected model from QSettings.
 */
void AI::Assistant::restoreModelSelections()
{
  m_settings.beginGroup(QStringLiteral("ai/model"));
  for (int i = 0; i < kProviderCount; ++i) {
    const auto stored = m_settings.value(modelSettingsKey(i)).toString();
    if (stored.isEmpty())
      continue;

    auto* p = providerAt(i);
    if (!p)
      continue;

    if (i == static_cast<int>(ProviderId::Local)) {
      p->setCurrentModel(stored);
      continue;
    }

    if (!p->availableModels().contains(stored)) {
      qCInfo(serialStudioAI) << "Discarding stale model id" << stored << "for provider" << i
                             << "-- not in availableModels";
      m_settings.remove(modelSettingsKey(i));
      continue;
    }

    p->setCurrentModel(stored);
  }
  m_settings.endGroup();
}

/**
 * @brief Returns the QSettings sub-key name for the provider's model.
 */
QString AI::Assistant::modelSettingsKey(int providerIdx)
{
  switch (static_cast<ProviderId>(providerIdx)) {
    case ProviderId::Anthropic:
      return QStringLiteral("anthropic");
    case ProviderId::OpenAI:
      return QStringLiteral("openai");
    case ProviderId::Gemini:
      return QStringLiteral("gemini");
    case ProviderId::DeepSeek:
      return QStringLiteral("deepseek");
    case ProviderId::OpenRouter:
      return QStringLiteral("openrouter");
    case ProviderId::Groq:
      return QStringLiteral("groq");
    case ProviderId::Mistral:
      return QStringLiteral("mistral");
    case ProviderId::Local:
      return QStringLiteral("local");
  }
  return QStringLiteral("unknown");
}

/**
 * @brief Returns the Provider* matching the integer index, or nullptr.
 */
AI::Provider* AI::Assistant::providerAt(int idx) const
{
  switch (static_cast<ProviderId>(idx)) {
    case ProviderId::Anthropic:
      return m_anthropic.get();
    case ProviderId::OpenAI:
      return m_openai.get();
    case ProviderId::Gemini:
      return m_gemini.get();
    case ProviderId::DeepSeek:
      return m_deepseek.get();
    case ProviderId::OpenRouter:
      return m_openrouter.get();
    case ProviderId::Groq:
      return m_groq.get();
    case ProviderId::Mistral:
      return m_mistral.get();
    case ProviderId::Local:
      return m_local.get();
  }
  return nullptr;
}

/**
 * @brief Returns true when the given provider needs a stored API key.
 */
bool AI::Assistant::requiresApiKey(int providerIdx) const
{
  return static_cast<ProviderId>(providerIdx) != ProviderId::Local;
}

// code-verify off  (cold settings paths; dynamic_cast is one-shot per user action)
/**
 * @brief Returns the configured base URL of the local model provider.
 */
QString AI::Assistant::localBaseUrl() const
{
  if (auto* lp = dynamic_cast<LocalProvider*>(m_local.get()))
    return lp->baseUrl();

  return LocalProvider::defaultBaseUrl();
}

/**
 * @brief Updates the local-model base URL and triggers a fresh model query.
 */
void AI::Assistant::setLocalBaseUrl(const QString& url)
{
  if (auto* lp = dynamic_cast<LocalProvider*>(m_local.get())) {
    lp->setBaseUrl(url);
    Q_EMIT keysChanged();
  }
}

/**
 * @brief Re-queries the local model list (typically after the user edits the URL).
 */
void AI::Assistant::refreshLocalModels()
{
  if (auto* lp = dynamic_cast<LocalProvider*>(m_local.get()))
    lp->refreshModels();
}

// code-verify on
