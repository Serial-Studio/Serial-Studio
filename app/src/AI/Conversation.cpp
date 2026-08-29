/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation.h"

#include <QByteArray>
#include <QJsonDocument>

#include "AI/Assistant.h"
#include "AI/CommandRegistry.h"
#include "AI/ContextBuilder.h"
#include "AI/Conversation/ChatDigest.h"
#include "AI/Conversation/HistorySurgery.h"
#include "AI/Conversation/MetaToolCatalog.h"
#include "AI/Conversation/ReplyAssembly.h"
#include "AI/Conversation/TokenBudget.h"
#include "AI/DocSearch.h"
#include "AI/Logging.h"
#include "AI/Providers/Provider.h"
#include "AI/Redactor.h"
#include "AI/SkillRouter.h"
#include "AI/ToolDispatcher.h"
#include "DataModel/ProjectModel.h"
#include "Licensing/CommercialToken.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction / destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates an idle conversation; provider and dispatcher are wired later. The command
 *        registry and the project model are captured here because both already exist when
 *        the Assistant builds this object; the Assistant itself is resolved lazily instead,
 *        since this constructor runs inside its own.
 */
AI::Conversation::Conversation(QObject* parent)
  : QObject(parent)
  , m_provider(nullptr)
  , m_dispatcher(nullptr)
  , m_reply(nullptr)
  , m_commands(CommandRegistry::instance())
  , m_project(DataModel::ProjectModel::instance())
  , m_assistantIndex(-1)
  , m_thinkingIsSynthetic(false)
  , m_outstandingToolResults(0)
  , m_toolCallCount(0)
  , m_retryCount(0)
  , m_turnGeneration(0)
  , m_cancelled(false)
  , m_summaryForced(false)
  , m_busy(false)
  , m_lastAwaitingFlag(false)
  , m_metaTools(*this, m_helpFetcher)
  , m_autoVerify(m_commands)
  , m_streamFlushTimer(new QTimer(this))
  , m_streamDirty(false)
  , m_uiDirty(false)
  , m_autoSaveTimer(new QTimer(this))
{
  m_streamFlushTimer->setInterval(kStreamFlushMs);
  m_streamFlushTimer->setSingleShot(false);
  connect(m_streamFlushTimer, &QTimer::timeout, this, &Conversation::flushPendingStreamUpdate);

  connect(&m_helpFetcher, &HelpFetcher::fetchFinished, this, &Conversation::onHelpFetchFinished);

  m_autoSaveTimer->setInterval(kAutoSaveDebounceMs);
  m_autoSaveTimer->setSingleShot(true);
  connect(m_autoSaveTimer, &QTimer::timeout, this, [this] {
    if (!m_project.modified())
      return;

    if (m_project.jsonFilePath().isEmpty())
      return;

    m_project.setSuppressMessageBoxes(true);
    const bool ok = m_project.saveJsonFile(false);
    m_project.setSuppressMessageBoxes(false);
    if (!ok)
      qCWarning(serialStudioAI) << "AI auto-save failed";
    else
      qCDebug(serialStudioAI) << "AI auto-save:" << m_project.jsonFilePath();
  });
}

/**
 * @brief Lazily resolves the Assistant singleton. Conversation is constructed inside the
 *        Assistant constructor, so an init-list capture would re-enter the Meyers guard;
 *        every caller below runs long after that constructor returned.
 */
AI::Assistant& AI::Conversation::assistant()
{
  static auto& singleton = Assistant::instance();
  return singleton;
}

/**
 * @brief Aborts any in-flight reply and frees owned resources.
 */
AI::Conversation::~Conversation()
{
  teardownReply();
}

//--------------------------------------------------------------------------------------------------
// Wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the active provider. The conversation does not take ownership.
 */
void AI::Conversation::setProvider(Provider* provider)
{
  m_provider = provider;
}

/**
 * @brief Sets the tool dispatcher. The conversation does not take ownership, and forwards
 *        it to the two sub-objects that dispatch through it.
 */
void AI::Conversation::setDispatcher(ToolDispatcher* dispatcher)
{
  m_dispatcher = dispatcher;
  m_metaTools.setDispatcher(dispatcher);
  m_autoVerify.setDispatcher(dispatcher);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QVariantList of UI message rows.
 */
QVariantList AI::Conversation::messages() const
{
  return m_uiMessages;
}

/**
 * @brief Returns true when a request is in flight or a tool batch is pending.
 */
bool AI::Conversation::busy() const noexcept
{
  return m_busy;
}

/**
 * @brief Returns true when at least one tool call is awaiting user approval.
 */
bool AI::Conversation::awaitingConfirmation() const noexcept
{
  return !m_awaitingConfirm.isEmpty();
}

/**
 * @brief Returns the most recent error message, or empty.
 */
QString AI::Conversation::lastError() const noexcept
{
  return m_lastError;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a new user message after gating on build availability and idle state.
 */
void AI::Conversation::start(const QString& userText)
{
  const auto trimmed = userText.trimmed();
  if (trimmed.isEmpty())
    return;

  if (!SS_LICENSE_GUARD()) {
    setLastError(tr("AI Assistant is not available in this build"));
    Q_EMIT errorOccurred(m_lastError);
    return;
  }

  if (!m_provider || !m_dispatcher) {
    setLastError(tr("AI subsystem not initialized"));
    Q_EMIT errorOccurred(m_lastError);
    return;
  }

  if (m_busy) {
    setLastError(tr("Already busy with a previous request"));
    Q_EMIT errorOccurred(m_lastError);
    return;
  }

  ++m_turnGeneration;
  m_helpFetcher.abortPending();
  m_cancelled     = false;
  m_summaryForced = false;
  m_toolCallCount = 0;
  m_retryCount    = 0;
  setLastError(QString());

  m_pendingThinkingBlocks   = QJsonArray();
  m_pendingToolUseBlocks    = QJsonArray();
  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;

  const bool was_degraded = m_probe.degraded();
  m_probe.ensureKey(probeComplianceKey());
  if (m_probe.degraded() != was_degraded)
    Q_EMIT probeStateChanged();

  appendUserMessage(trimmed);
  injectRoutedSkill(trimmed);
  maybeProposeMemory(trimmed);
  setBusy(true);
  issueRequest();
}

/**
 * @brief Returns the provider+model key the probe's compliance memory is filed under.
 */
QString AI::Conversation::probeComplianceKey() const
{
  if (!m_provider)
    return QStringLiteral("none");

  return m_provider->displayName() + QLatin1Char('/') + m_provider->currentModel();
}

/**
 * @brief Validates the completed visible reply against the sentinel contract; runs only
 *        on final replies (no pending tool calls) and only while the probe is enabled.
 *        Notifies on any change of the (degraded, failure, drifted) tuple, not just the
 *        boolean, so the banner detail refreshes when the failure kind shifts.
 */
void AI::Conversation::evaluateProbe()
{
  if (!assistant().contextProbeEnabled())
    return;

  if (m_assistantText.isEmpty() || !m_pendingToolUseBlocks.isEmpty())
    return;

  const bool was_degraded = m_probe.degraded();
  const auto was_failure  = m_probe.lastFailure();
  const auto was_drifted  = m_probe.driftedSegment();
  const auto outcome      = m_probe.evaluateReply(m_assistantText);
  qCDebug(serialStudioAI) << "SentinelProbe: outcome" << static_cast<int>(outcome);

  if (m_probe.degraded() != was_degraded || m_probe.lastFailure() != was_failure
      || m_probe.driftedSegment() != was_drifted)
    Q_EMIT probeStateChanged();
}

/**
 * @brief Deterministic skill routing: injects the synthetic meta.loadSkill pair after the
 *        user turn so weak models get domain knowledge without asking; the injected user
 *        message carries a tool_result, so the budgeter's fresh-user-turn cuts skip it.
 */
void AI::Conversation::injectRoutedSkill(const QString& userText)
{
  if (!assistant().skillRoutingEnabled())
    return;

  static const SkillRouter router;
  const auto skill = router.match(userText, m_loadedSkills);
  if (skill.isEmpty())
    return;

  const int budget = m_provider ? m_provider->capabilities().toolResultByteBudget : 4096;
  const auto pair  = SkillRouter::buildInjectionPair(skill, budget);
  if (pair.isEmpty())
    return;

  for (const auto& msg : pair)
    m_history.append(msg);

  m_loadedSkills.insert(skill);

  auto row                              = m_uiMessages.last().toMap();
  row[QStringLiteral("loadedSkill")]    = skill;
  m_uiMessages[m_uiMessages.size() - 1] = row;
  Q_EMIT messagesChanged();

  qCDebug(serialStudioAI) << "SkillRouter: injected" << skill << "for this turn";
}

/**
 * @brief Deterministic memory proposal: remember-phrasing in the user's message surfaces
 *        the confirmation chip directly, because weak models never volunteer the
 *        assistant.memory.propose call (observed 2026-07-14); nothing persists without the
 *        user's click, and the store's secret scrub still gates the write.
 */
void AI::Conversation::maybeProposeMemory(const QString& userText)
{
  if (!assistant().memoryEnabled())
    return;

  static const QStringList kTriggers = {
    QStringLiteral("remember that"),
    QStringLiteral("remember this"),
    QStringLiteral("remember my"),
    QStringLiteral("please remember"),
    QStringLiteral("don't forget"),
    QStringLiteral("do not forget"),
    QStringLiteral("keep in mind"),
    QStringLiteral("from now on"),
    QStringLiteral("i always"),
    QStringLiteral("i never"),
    QStringLiteral("i prefer"),
    QStringLiteral("always use"),
    QStringLiteral("never use"),
    QStringLiteral("my preference"),
  };

  const auto lower = userText.toLower();
  bool matched     = false;
  for (const auto& trigger : kTriggers) {
    if (lower.contains(trigger)) {
      matched = true;
      break;
    }
  }

  if (!matched)
    return;

  static const QStringList kCorrectionCues = {
    QStringLiteral("never"),
    QStringLiteral("don't"),
    QStringLiteral("do not"),
    QStringLiteral("stop "),
    QStringLiteral("instead"),
  };

  QString category = QStringLiteral("user");
  for (const auto& cue : kCorrectionCues) {
    if (lower.contains(cue)) {
      category = QStringLiteral("feedback");
      break;
    }
  }

  const auto fact = userText.simplified().left(400);
  Q_EMIT memoryProposed(category, fact);
  qCDebug(serialStudioAI) << "MemoryTrigger: deterministic proposal surfaced, category="
                          << category;
}

/**
 * @brief Aborts the in-flight reply and cancels any pending confirmations.
 */
void AI::Conversation::cancel()
{
  ++m_turnGeneration;
  m_helpFetcher.abortPending();
  m_cancelled = true;
  m_streamFlushTimer->stop();
  m_streamDirty = false;
  m_uiDirty     = false;
  if (m_reply)
    m_reply->abort();

  if (!m_awaitingConfirm.isEmpty()) {
    for (auto it = m_awaitingConfirm.constBegin(); it != m_awaitingConfirm.constEnd(); ++it)
      updateToolCallCard(it.key(), CallStatus::Denied);

    m_awaitingConfirm.clear();
    setAwaitingConfirmation(false);
  }

  m_pendingThinkingBlocks   = QJsonArray();
  m_pendingToolUseBlocks    = QJsonArray();
  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;

  setBusy(false);
}

/**
 * @brief Approves a pending Confirm-tagged tool call by id.
 */
void AI::Conversation::approveToolCall(const QString& callId)
{
  const auto it = m_awaitingConfirm.constFind(callId);
  if (it == m_awaitingConfirm.constEnd())
    return;

  const auto pending = it.value();
  m_awaitingConfirm.erase(it);
  setAwaitingConfirmation(!m_awaitingConfirm.isEmpty());

  runToolCall(callId, pending.name, pending.arguments);

  if (m_outstandingToolResults == 0 && !m_awaitingConfirm.isEmpty())
    return;

  if (m_outstandingToolResults == 0)
    resumeAfterToolBatch();
}

/**
 * @brief Denies a pending Confirm-tagged tool call and feeds back a denial result.
 */
void AI::Conversation::denyToolCall(const QString& callId)
{
  const auto it = m_awaitingConfirm.constFind(callId);
  if (it == m_awaitingConfirm.constEnd())
    return;

  const auto pending = it.value();
  m_awaitingConfirm.erase(it);
  setAwaitingConfirmation(!m_awaitingConfirm.isEmpty());

  QJsonObject denial;
  denial[QStringLiteral("ok")]    = false;
  denial[QStringLiteral("error")] = QStringLiteral("user_denied");
  recordToolResult(callId, pending.name, denial);
  updateToolCallCard(callId, CallStatus::Denied);

  releaseOutstandingToolResult();
  if (m_outstandingToolResults == 0 && m_awaitingConfirm.isEmpty())
    resumeAfterToolBatch();
}

/**
 * @brief Approves every pending Confirm whose tool name starts with prefix.
 */
void AI::Conversation::approveToolCallGroup(const QString& family)
{
  if (family.isEmpty())
    return;

  QStringList ids;
  for (auto it = m_awaitingConfirm.constBegin(); it != m_awaitingConfirm.constEnd(); ++it)
    if (it.value().name.startsWith(family + QLatin1Char('.')) || it.value().name == family)
      ids.append(it.key());

  for (const auto& id : ids)
    approveToolCall(id);
}

/**
 * @brief Denies every pending Confirm whose tool name starts with prefix.
 */
void AI::Conversation::denyToolCallGroup(const QString& family)
{
  if (family.isEmpty())
    return;

  QStringList ids;
  for (auto it = m_awaitingConfirm.constBegin(); it != m_awaitingConfirm.constEnd(); ++it)
    if (it.value().name.startsWith(family + QLatin1Char('.')) || it.value().name == family)
      ids.append(it.key());

  for (const auto& id : ids)
    denyToolCall(id);
}

/**
 * @brief Clears history and UI state. Aborts any in-flight reply.
 */
void AI::Conversation::clear()
{
  cancel();
  m_history = QJsonArray();
  m_uiMessages.clear();
  m_handoffSeed.clear();
  m_loadedSkills.clear();

  const bool was_degraded = m_probe.degraded();
  m_probe.reset(probeComplianceKey());
  if (was_degraded)
    Q_EMIT probeStateChanged();

  m_assistantIndex = -1;
  m_assistantText.clear();
  m_pendingThinkingBlocks   = QJsonArray();
  m_pendingToolUseBlocks    = QJsonArray();
  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;
  m_awaitingConfirm.clear();
  setLastError(QString());

  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();
}

//--------------------------------------------------------------------------------------------------
// Reply slot handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Appends streamed text and schedules a coalesced UI refresh.
 */
void AI::Conversation::onPartialText(const QString& chunk)
{
  if (m_cancelled || m_assistantIndex < 0)
    return;

  if (m_thinkingIsSynthetic && !m_assistantThinking.isEmpty()) {
    m_assistantThinking.clear();
    m_thinkingIsSynthetic = false;
  }

  m_assistantText.append(chunk);
  m_streamDirty = true;
  if (!m_streamFlushTimer->isActive())
    m_streamFlushTimer->start();
}

/**
 * @brief Appends streamed thinking text to the live assistant preamble.
 */
void AI::Conversation::onPartialThinking(const QString& chunk)
{
  if (m_cancelled || m_assistantIndex < 0)
    return;

  if (m_thinkingIsSynthetic) {
    m_assistantThinking.clear();
    m_thinkingIsSynthetic = false;
  }

  m_assistantThinking.append(chunk);
  m_streamDirty = true;
  if (!m_streamFlushTimer->isActive())
    m_streamFlushTimer->start();
}

/**
 * @brief Stores a completed provider thinking block (text + signature) tagged with the
 *        model that produced it, so the next request can echo it back verbatim.
 */
void AI::Conversation::onThinkingBlockFinished(const QJsonObject& block)
{
  if (m_cancelled || !m_provider)
    return;

  auto tagged                      = block;
  tagged[QStringLiteral("_model")] = m_provider->currentModel();
  m_pendingThinkingBlocks.append(tagged);
}

/**
 * @brief Pushes accumulated text/thinking into the live row and emits one coalesced
 *        messagesChanged for any dirty tool-card updates riding the same tick.
 */
void AI::Conversation::flushPendingStreamUpdate()
{
  if (!m_streamDirty && !m_uiDirty) {
    m_streamFlushTimer->stop();
    return;
  }

  const bool had_stream = m_streamDirty;
  m_streamDirty         = false;
  m_uiDirty             = false;

  if (had_stream && m_assistantIndex >= 0 && m_assistantIndex < m_uiMessages.size()) {
    auto map = m_uiMessages.at(m_assistantIndex).toMap();
    map.insert(QStringLiteral("text"),
               SentinelProbe::stripForDisplay(ReplyAssembly::rewriteHelpLinks(m_assistantText)));
    map.insert(QStringLiteral("thinking"), m_assistantThinking);
    map.insert(QStringLiteral("streaming"), true);
    m_uiMessages[m_assistantIndex] = map;
  }

  Q_EMIT messagesChanged();
}

/**
 * @brief Marks the UI rows dirty and arms the coalescing timer, so tool-card bursts share
 *        one messagesChanged per tick instead of forcing a full model rebind each.
 */
void AI::Conversation::scheduleUiFlush()
{
  m_uiDirty = true;
  if (!m_streamFlushTimer->isActive())
    m_streamFlushTimer->start();
}

/**
 * @brief Records a tool-use request and dispatches per safety tag.
 */
void AI::Conversation::onToolCallRequested(const QString& callId,
                                           const QString& requestedName,
                                           const QJsonObject& arguments,
                                           const QJsonObject& extras)
{
  if (m_cancelled)
    return;

  const QString name = m_dispatcher->canonicalToolName(requestedName);
  ++m_toolCallCount;
  if (m_toolCallCount > kMaxToolCalls) {
    qCWarning(serialStudioAI) << "Tool-call budget exceeded; forcing summary";
    m_summaryForced = true;

    m_pendingToolUseBlocks.append(ReplyAssembly::makeToolUseBlock(callId, name, arguments, extras));

    QJsonObject denial;
    denial[QStringLiteral("error")] =
      tr("Tool-call budget reached for this turn; no further tools will run.");
    appendToolCallCard(callId, name, arguments, CallStatus::Blocked);
    recordToolResult(callId, name, denial);
    updateToolCallCard(callId, CallStatus::Blocked, denial);
    return;
  }

  m_pendingToolUseBlocks.append(ReplyAssembly::makeToolUseBlock(callId, name, arguments, extras));

  ++m_outstandingToolResults;

  if (m_metaTools.dispatch(callId, name, arguments))
    return;

  dispatchByCallSafety(callId, name, arguments);
}

/**
 * @brief Routes a non-meta tool call by its CommandRegistry safety tag.
 */
void AI::Conversation::dispatchByCallSafety(const QString& callId,
                                            const QString& requestedName,
                                            const QJsonObject& arguments)
{
  const QString name = m_dispatcher->canonicalToolName(requestedName);
  const auto safety  = m_commands.safetyOf(name);
  qCDebug(serialStudioAI) << "Tool call" << name << "safety=" << static_cast<int>(safety);

  if (safety == Safety::Blocked) {
    QJsonObject denial;
    denial[QStringLiteral("ok")]    = false;
    denial[QStringLiteral("error")] = QStringLiteral("blocked");
    appendToolCallCard(callId, name, arguments, CallStatus::Blocked);
    recordToolResult(callId, name, denial);
    updateToolCallCard(callId, CallStatus::Blocked, denial);
    releaseOutstandingToolResult();
    return;
  }

  if (safety == Safety::Confirm || safety == Safety::AlwaysConfirm) {
    bool autoApprove = false;
    if (safety == Safety::Confirm)
      autoApprove = assistant().autoApproveEdits();

    if (autoApprove) {
      appendToolCallCard(callId, name, arguments, CallStatus::Running);
      runToolCall(callId, name, arguments);
      return;
    }

    appendToolCallCard(callId, name, arguments, CallStatus::AwaitingConfirm);
    PendingCall pending;
    pending.name      = name;
    pending.arguments = arguments;
    m_awaitingConfirm.insert(callId, pending);
    setAwaitingConfirmation(true);
    return;
  }

  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  runToolCall(callId, name, arguments);
}

/**
 * @brief Handles end-of-stream: either continue with tool results or end the turn.
 */
void AI::Conversation::onReplyFinished()
{
  m_streamFlushTimer->stop();
  if (m_streamDirty || m_uiDirty)
    flushPendingStreamUpdate();

  if (m_cancelled) {
    teardownReply();
    setBusy(false);
    return;
  }

  evaluateProbe();

  const auto assistantMsg = ReplyAssembly::makeAssistantMessage(
    m_pendingThinkingBlocks, m_assistantText, m_pendingToolUseBlocks);
  if (!assistantMsg.isEmpty())
    m_history.append(assistantMsg);

  if (m_assistantIndex >= 0 && m_assistantIndex < m_uiMessages.size()) {
    auto map = m_uiMessages.at(m_assistantIndex).toMap();
    map.insert(QStringLiteral("streaming"), false);

    const auto finalText = map.value(QStringLiteral("text")).toString();
    map.insert(QStringLiteral("text"), ReplyAssembly::rewriteHelpLinks(finalText));

    const auto rowText  = map.value(QStringLiteral("text")).toString();
    const auto rowCalls = map.value(QStringLiteral("toolCalls")).toList();
    if (rowText.isEmpty() && rowCalls.isEmpty()) {
      map.insert(QStringLiteral("text"),
                 tr("(The model returned an empty response. Try "
                    "rephrasing, switching to a different model, or "
                    "checking that the request is allowed by the "
                    "provider's safety filters.)"));
    }

    m_uiMessages[m_assistantIndex] = map;
    Q_EMIT messagesChanged();
  }

  m_pendingThinkingBlocks = QJsonArray();
  m_pendingToolUseBlocks  = QJsonArray();
  m_assistantText.clear();
  m_assistantThinking.clear();
  m_assistantIndex = -1;
  m_retryCount     = 0;

  teardownReply();

  if (!m_awaitingConfirm.isEmpty())
    return;

  if (m_outstandingToolResults > 0)
    return;

  if (!m_pendingToolResultBlocks.isEmpty()) {
    resumeAfterToolBatch();
    return;
  }

  setBusy(false);
}

/**
 * @brief Records a network or stream error and ends the turn. Transient failures (429,
 *        5xx, timeouts) retry with backoff when nothing has streamed yet, instead of
 *        throwing away the whole turn.
 */
void AI::Conversation::onReplyError(const QString& message)
{
  qCWarning(serialStudioAI) << "Reply error:" << message;

  if (m_cancelled) {
    teardownReply();
    setBusy(false);
    return;
  }

  if (shouldRetryAfterError()) {
    scheduleTransientRetry(message);
    return;
  }

  setLastError(message);
  Q_EMIT errorOccurred(message);

  m_streamFlushTimer->stop();
  if (m_streamDirty || m_uiDirty)
    flushPendingStreamUpdate();

  if (m_assistantIndex >= 0 && m_assistantIndex < m_uiMessages.size()) {
    auto map = m_uiMessages.at(m_assistantIndex).toMap();
    map.insert(QStringLiteral("streaming"), false);
    m_uiMessages[m_assistantIndex] = map;
  }

  QVariantMap errorRow;
  errorRow[QStringLiteral("role")]      = QStringLiteral("error");
  errorRow[QStringLiteral("text")]      = message;
  errorRow[QStringLiteral("toolCalls")] = QVariantList();
  m_uiMessages.append(errorRow);
  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();

  if (!m_awaitingConfirm.isEmpty()) {
    for (auto it = m_awaitingConfirm.constBegin(); it != m_awaitingConfirm.constEnd(); ++it)
      updateToolCallCard(it.key(), CallStatus::Error);

    m_awaitingConfirm.clear();
    setAwaitingConfirmation(false);
  }

  m_assistantText.clear();
  m_assistantThinking.clear();
  m_assistantIndex          = -1;
  m_pendingThinkingBlocks   = QJsonArray();
  m_pendingToolUseBlocks    = QJsonArray();
  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;
  teardownReply();
  setBusy(false);
}

/**
 * @brief Returns true when the failed request is safe to retry: transient cause, retry
 *        budget left, not cancelled, and no partial output recorded yet.
 */
bool AI::Conversation::shouldRetryAfterError() const
{
  if (m_cancelled || m_retryCount >= kMaxTransientRetries)
    return false;

  if (!m_reply || !m_reply->transientError())
    return false;

  return m_assistantText.isEmpty() && m_pendingToolUseBlocks.isEmpty()
      && m_pendingThinkingBlocks.isEmpty();
}

/**
 * @brief Drops the empty streaming placeholder row and re-issues the request after an
 *        exponential backoff (1.5s, 3s). The retry callback is generation-guarded so a
 *        timer armed for a cancelled turn can never fire into a newer one (it would
 *        double-issue and interleave two live replies).
 */
void AI::Conversation::scheduleTransientRetry(const QString& message)
{
  ++m_retryCount;
  qCInfo(serialStudioAI) << "Transient provider error, retry" << m_retryCount << "of"
                         << kMaxTransientRetries << ":" << message;

  m_streamFlushTimer->stop();
  m_streamDirty = false;
  teardownReply();

  if (m_assistantIndex >= 0 && m_assistantIndex < m_uiMessages.size()) {
    m_uiMessages.removeAt(m_assistantIndex);
    m_assistantIndex = -1;
    Q_EMIT messagesChanged();
    Q_EMIT messageCountChanged();
  }

  m_assistantText.clear();
  m_assistantThinking.clear();

  const int delayMs        = kRetryBaseMs * (1 << (m_retryCount - 1));
  const quint64 generation = m_turnGeneration;
  QTimer::singleShot(delayMs, this, [this, generation]() {
    if (generation == m_turnGeneration && !m_cancelled && m_busy)
      issueRequest();
  });
}

//--------------------------------------------------------------------------------------------------
// Internals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Issues a fresh request to the active provider with the current history.
 */
void AI::Conversation::issueRequest()
{
  if (!m_provider || !m_dispatcher) {
    setLastError(tr("AI subsystem not initialized"));
    Q_EMIT errorOccurred(m_lastError);
    setBusy(false);
    return;
  }

  pruneHistory();

  reconcileHistoryToolPairs();

  ageHistoryToolResults();

  beginAssistantMessage();

  if (m_provider->capabilities().slowFirstToken)
    m_assistantThinking = tr("Waiting for %1 to respond. Loading the model and processing "
                             "the prompt can take a while on local hardware...")
                            .arg(m_provider->displayName());
  else
    m_assistantThinking = tr("Sending request to %1...").arg(m_provider->displayName());

  m_thinkingIsSynthetic = true;
  if (m_assistantIndex >= 0 && m_assistantIndex < m_uiMessages.size()) {
    auto map = m_uiMessages.at(m_assistantIndex).toMap();
    map.insert(QStringLiteral("thinking"), m_assistantThinking);
    m_uiMessages[m_assistantIndex] = map;
    Q_EMIT messagesChanged();
  }

  const auto tools   = dispatcherTools();
  const auto history = budgetedHistory(tools);
  qCDebug(serialStudioAI) << "Request: history_items=" << history.size() << "tools=" << tools.size()
                          << "loaded_skills=" << m_loadedSkills.size()
                          << "handoff_seeded=" << !m_handoffSeed.isEmpty();

  m_reply = m_provider->sendMessage(history, tools, m_summaryForced);
  if (!m_reply) {
    setLastError(tr("Provider returned no reply"));
    Q_EMIT errorOccurred(m_lastError);
    setBusy(false);
    return;
  }

  connect(m_reply, &Reply::partialText, this, &Conversation::onPartialText);
  connect(m_reply, &Reply::partialThinking, this, &Conversation::onPartialThinking);
  connect(m_reply, &Reply::thinkingBlockFinished, this, &Conversation::onThinkingBlockFinished);
  connect(m_reply, &Reply::toolCallRequested, this, &Conversation::onToolCallRequested);
  connect(m_reply, &Reply::finished, this, &Conversation::onReplyFinished);
  connect(m_reply, &Reply::errorOccurred, this, &Conversation::onReplyError);
  connect(m_reply, &Reply::cacheStatsAvailable, this, [](int read, int created) {
    assistant().reportCacheStats(read, created);
  });
}

/**
 * @brief Pairs every assistant.tool_use with a tool_result, synthesizing or pruning as
 *        needed. Runs after pruneHistory so a prune cut can never ship an unpaired block.
 */
void AI::Conversation::reconcileHistoryToolPairs()
{
  HistorySurgery::reconcileHistoryToolPairs(m_history);
}

/**
 * @brief Stubs older tool_result blocks; keeps the most recent tool turns verbatim.
 */
void AI::Conversation::ageHistoryToolResults()
{
  HistorySurgery::ageHistoryToolResults(m_history);
}

/**
 * @brief Caps unbounded history/UI growth so a long session cannot exhaust memory. The UI
 *        rows are trimmed here rather than in HistorySurgery because dropping them moves
 *        the live assistant row and has to notify QML.
 */
void AI::Conversation::pruneHistory()
{
  (void)HistorySurgery::pruneHistory(m_history, kMaxHistoryItems);

  if (m_uiMessages.size() > kMaxUiMessageRows) {
    const int drop = m_uiMessages.size() - kMaxUiMessageRows;
    m_uiMessages.erase(m_uiMessages.begin(), m_uiMessages.begin() + drop);
    if (m_assistantIndex >= 0)
      m_assistantIndex -= drop;

    SS_ASSERT_LOG(m_uiMessages.size() == kMaxUiMessageRows);
    SS_ASSERT(m_assistantIndex < m_uiMessages.size(), m_assistantIndex = -1);
    Q_EMIT messagesChanged();
    Q_EMIT messageCountChanged();
  }
}

/**
 * @brief Feeds a finished meta.fetchHelp result back into the turn and resumes the batch
 *        when it was the last outstanding call. Fetches invalidated by a newer turn never
 *        reach here: HelpFetcher drops them at abortPending().
 */
void AI::Conversation::onHelpFetchFinished(const QString& callId, const QJsonObject& result)
{
  const bool ok = result.value(QStringLiteral("ok")).toBool();
  recordToolResult(callId, QString::fromLatin1(HelpFetcher::kToolName), result);
  updateToolCallCard(callId, ok ? CallStatus::Done : CallStatus::Error, result);
  releaseOutstandingToolResult();

  if (m_outstandingToolResults == 0 && m_awaitingConfirm.isEmpty() && !m_reply)
    resumeAfterToolBatch();
}

/**
 * @brief Adds a user message to both history and the UI message list.
 */
void AI::Conversation::appendUserMessage(const QString& text)
{
  QJsonObject content;
  content[QStringLiteral("type")] = QStringLiteral("text");
  content[QStringLiteral("text")] = text;

  QJsonObject userMsg;
  userMsg[QStringLiteral("role")]    = QStringLiteral("user");
  userMsg[QStringLiteral("content")] = QJsonArray{content};
  m_history.append(userMsg);

  QVariantMap row;
  row[QStringLiteral("role")]      = QStringLiteral("user");
  row[QStringLiteral("text")]      = text;
  row[QStringLiteral("toolCalls")] = QVariantList();
  m_uiMessages.append(row);
  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();
}

/**
 * @brief Adds a placeholder assistant row that subsequent deltas fill.
 */
void AI::Conversation::beginAssistantMessage()
{
  m_assistantText.clear();
  m_assistantThinking.clear();
  QVariantMap row;
  row[QStringLiteral("role")]      = QStringLiteral("assistant");
  row[QStringLiteral("text")]      = QString();
  row[QStringLiteral("thinking")]  = QString();
  row[QStringLiteral("streaming")] = true;
  row[QStringLiteral("toolCalls")] = QVariantList();
  m_uiMessages.append(row);
  m_assistantIndex = m_uiMessages.size() - 1;
  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();
}

/**
 * @brief Returns "discovery" for read-only / meta calls, "execution" otherwise.
 */
static QString toolCallCategory(const AI::CommandRegistry& commands, const QString& name)
{
  if (name.startsWith(QStringLiteral("meta.")))
    return QStringLiteral("discovery");

  if (commands.safetyOf(name) == AI::Safety::Safe)
    return QStringLiteral("discovery");

  return QStringLiteral("execution");
}

/**
 * @brief Adds a ToolCallCard payload to the active assistant message.
 */
void AI::Conversation::appendToolCallCard(const QString& callId,
                                          const QString& name,
                                          const QJsonObject& arguments,
                                          CallStatus status)
{
  if (m_assistantIndex < 0 || m_assistantIndex >= m_uiMessages.size())
    return;

  auto map   = m_uiMessages.at(m_assistantIndex).toMap();
  auto calls = map.value(QStringLiteral("toolCalls")).toList();

  QString family    = name;
  const int lastDot = family.lastIndexOf(QLatin1Char('.'));
  if (lastDot > 0)
    family.truncate(lastDot);

  QVariantMap card;
  card[QStringLiteral("callId")]   = callId;
  card[QStringLiteral("name")]     = name;
  card[QStringLiteral("family")]   = family;
  card[QStringLiteral("category")] = toolCallCategory(m_commands, name);
  card[QStringLiteral("args")]     = QJsonDocument(arguments).toJson(QJsonDocument::Indented);
  card[QStringLiteral("status")]   = static_cast<int>(status);
  card[QStringLiteral("result")]   = QString();

  calls.append(card);
  map.insert(QStringLiteral("toolCalls"), calls);
  m_uiMessages[m_assistantIndex] = map;
  scheduleUiFlush();
}

/**
 * @brief Updates the status (and optional result) of an existing ToolCallCard.
 */
void AI::Conversation::updateToolCallCard(const QString& callId,
                                          CallStatus status,
                                          const QJsonObject& result,
                                          const QJsonObject& verification)
{
  for (int i = m_uiMessages.size() - 1; i >= 0; --i) {
    auto map     = m_uiMessages.at(i).toMap();
    auto calls   = map.value(QStringLiteral("toolCalls")).toList();
    bool changed = false;
    for (int c = 0; c < calls.size(); ++c) {
      auto card = calls.at(c).toMap();
      if (card.value(QStringLiteral("callId")).toString() != callId)
        continue;

      card.insert(QStringLiteral("status"), static_cast<int>(status));
      if (!result.isEmpty())
        card.insert(QStringLiteral("result"),
                    QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Indented)));

      if (!verification.isEmpty())
        card.insert(QStringLiteral("verification"), verification.toVariantMap());

      calls[c] = card;
      changed  = true;
      break;
    }
    if (changed) {
      map.insert(QStringLiteral("toolCalls"), calls);
      m_uiMessages[i] = map;
      scheduleUiFlush();
      return;
    }
  }
}

/**
 * @brief Executes a single tool call and feeds its result back.
 */
void AI::Conversation::runToolCall(const QString& callId,
                                   const QString& name,
                                   const QJsonObject& arguments)
{
  bool found = false;
  for (int i = m_uiMessages.size() - 1; i >= 0 && !found; --i) {
    const auto map   = m_uiMessages.at(i).toMap();
    const auto calls = map.value(QStringLiteral("toolCalls")).toList();
    for (const auto& cv : calls)
      if (cv.toMap().value(QStringLiteral("callId")).toString() == callId) {
        found = true;
        break;
      }
  }
  if (!found)
    appendToolCallCard(callId, name, arguments, CallStatus::Running);
  else
    updateToolCallCard(callId, CallStatus::Running);

  const auto reply = m_dispatcher->executeCommand(name, arguments);
  const bool ok    = reply.value(QStringLiteral("ok")).toBool();
  qCDebug(serialStudioAI) << "Tool" << name << "result_ok=" << ok;

  auto effective = reply;
  const auto verification =
    assistant().autoVerifyEnabled() ? m_autoVerify.verify(name, arguments, reply) : QJsonObject();
  if (!verification.isEmpty())
    effective[QStringLiteral("verification")] = verification;

  recordToolResult(callId, name, effective);
  updateToolCallCard(callId, ok ? CallStatus::Done : CallStatus::Error, effective, verification);

  if (ok && name == QStringLiteral("assistant.memory.propose"))
    Q_EMIT memoryProposed(arguments.value(QStringLiteral("category")).toString(),
                          arguments.value(QStringLiteral("text")).toString());

  releaseOutstandingToolResult();

  const bool isMeta = name.startsWith(QStringLiteral("meta."));
  const bool isExplicit =
    (name == QStringLiteral("project.save") || name == QStringLiteral("project.new")
     || name == QStringLiteral("project.open"));
  const auto safety     = m_commands.safetyOf(name);
  const bool isReadOnly = (safety == Safety::Safe);
  if (ok && !isMeta && !isExplicit && !isReadOnly)
    m_autoSaveTimer->start();
}

/**
 * @brief Stores a tool_result block to be sent back in the next request.
 */
void AI::Conversation::recordToolResult(const QString& callId,
                                        const QString& name,
                                        const QJsonObject& payload)
{
  if (!m_busy) {
    qCWarning(serialStudioAI) << "Dropping tool result for" << name << "(" << callId
                              << "): no turn in flight (late completion after error/cancel)";
    return;
  }

  const auto scrubbed = AI::Redactor::scrubObject(payload);

  constexpr int kFsResultByteBudget = 48 * 1024;
  const bool isFsReadResult         = name == QStringLiteral("fs.read")
                           || name == QStringLiteral("fs.search")
                           || name == QStringLiteral("fs.list");
  const int kMaxToolResultBytes =
    isFsReadResult ? kFsResultByteBudget
                   : qBound(2048,
                            m_provider ? m_provider->capabilities().toolResultByteBudget : 4096,
                            16 * 1024);

  QJsonObject effective = scrubbed;
  auto contentBytes     = QJsonDocument(scrubbed).toJson(QJsonDocument::Compact);
  if (contentBytes.size() > kMaxToolResultBytes) {
    effective    = ReplyAssembly::makeTruncatedResult(scrubbed, contentBytes, kMaxToolResultBytes);
    contentBytes = QJsonDocument(effective).toJson(QJsonDocument::Compact);
    qCDebug(serialStudioAI) << "Tool result for" << name << "truncated to" << contentBytes.size()
                            << "bytes";
  }

  m_pendingToolResultBlocks.append(
    ReplyAssembly::makeToolResultBlock(callId, name, effective, contentBytes));
}

/**
 * @brief Decrements the outstanding tool-result counter without underflowing past zero.
 */
void AI::Conversation::releaseOutstandingToolResult()
{
  if (m_outstandingToolResults > 0)
    --m_outstandingToolResults;
}

/**
 * @brief Records that the model already holds a skill body, so the deterministic router
 *        does not inject it again later in the same chat.
 */
void AI::Conversation::noteSkillLoaded(const QString& skillId)
{
  m_loadedSkills.insert(skillId);
}

/**
 * @brief Runs a doc-corpus query for the meta.searchDocs handler. The index is resolved
 *        here rather than injected because its constructor parses the whole BM25 corpus:
 *        capturing it would move that cost into Assistant construction.
 */
QList<AI::DocSearch::Hit> AI::Conversation::searchDocs(const QString& query, int k) const
{
  static auto& docSearch = AI::DocSearch::instance();
  return docSearch.search(query, k);
}

/**
 * @brief Sends the accumulated tool_result blocks and continues the turn.
 */
void AI::Conversation::resumeAfterToolBatch()
{
  if (m_cancelled) {
    setBusy(false);
    return;
  }

  if (m_pendingToolResultBlocks.isEmpty()) {
    setBusy(false);
    return;
  }

  QJsonArray content = m_pendingToolResultBlocks;

  if (m_summaryForced) {
    QJsonObject text;
    text[QStringLiteral("type")] = QStringLiteral("text");
    text[QStringLiteral("text")] =
      tr("You have reached the tool-call budget for this turn. Do not request more "
         "tools. Summarize what you found so far, and if the task is incomplete, say "
         "which steps remain so the user can tell you to continue.");
    content.append(text);
  }

  QJsonObject userMsg;
  userMsg[QStringLiteral("role")]    = QStringLiteral("user");
  userMsg[QStringLiteral("content")] = content;
  m_history.append(userMsg);

  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;
  m_retryCount              = 0;

  issueRequest();
}

/**
 * @brief Disconnects and deletes the active reply, if any.
 */
void AI::Conversation::teardownReply()
{
  if (!m_reply)
    return;

  m_reply->disconnect(this);
  m_reply->deleteLater();
  m_reply = nullptr;
}

/**
 * @brief Sets and notifies the busy property.
 */
void AI::Conversation::setBusy(bool busy)
{
  if (m_busy == busy)
    return;

  m_busy = busy;
  Q_EMIT busyChanged();
}

/**
 * @brief Notifies awaitingConfirmation when the underlying flag flips.
 */
void AI::Conversation::setAwaitingConfirmation(bool flag)
{
  if (m_lastAwaitingFlag == flag)
    return;

  m_lastAwaitingFlag = flag;
  Q_EMIT awaitingConfirmationChanged();
}

/**
 * @brief Sets and notifies the lastError property.
 */
void AI::Conversation::setLastError(const QString& message)
{
  if (m_lastError == message)
    return;

  m_lastError = message;
  Q_EMIT lastErrorChanged();
}

/**
 * @brief Returns the AI tool surface: the meta discovery tools plus a small curated set of
 *        dispatcher commands. Cached per (small-surface, memory) flag pair, which is valid
 *        because the dispatcher catalog and the API registry are fixed after startup.
 */
QJsonArray AI::Conversation::dispatcherTools() const
{
  if (!m_dispatcher)
    return {};

  const auto caps      = m_provider ? m_provider->capabilities() : ProviderCapabilities{};
  const bool memory_on = assistant().memoryEnabled();
  const int cache_key  = (caps.needsSmallToolSurface ? 1 : 0) | (memory_on ? 2 : 0);

  static QHash<int, QJsonArray> s_cache;
  const auto cached = s_cache.constFind(cache_key);
  if (cached != s_cache.constEnd())
    return cached.value();

  auto remapped =
    MetaToolCatalog::metaTools(ContextBuilder::howToTasks(), ContextBuilder::skillIds());
  const auto essentials =
    MetaToolCatalog::essentialToolNames(caps.needsSmallToolSurface, memory_on);

  QHash<QString, QJsonObject> by_name;
  const auto raw = m_dispatcher->availableTools();
  for (const auto& v : raw) {
    const auto obj = v.toObject();
    by_name.insert(obj.value(QStringLiteral("name")).toString(), obj);
  }

  for (const auto& essentialName : essentials) {
    const auto it = by_name.constFind(essentialName);
    if (it != by_name.constEnd())
      remapped.append(MetaToolCatalog::remapDispatcherTool(it.value()));
  }

  s_cache.insert(cache_key, remapped);
  return remapped;
}

//--------------------------------------------------------------------------------------------------
// Snapshot (round-trips m_history + m_uiMessages through the ChatStore)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a serializable snapshot of the current history and UI rows.
 */
QJsonObject AI::Conversation::snapshot() const
{
  QJsonArray loaded;
  for (const auto& skill : m_loadedSkills)
    loaded.append(skill);

  QJsonObject probe;
  probe[QStringLiteral("degraded")] = m_probe.degraded();
  probe[QStringLiteral("failure")]  = static_cast<int>(m_probe.lastFailure());
  probe[QStringLiteral("drifted")]  = m_probe.driftedSegment();

  QJsonObject doc;
  doc[QStringLiteral("schema")]       = 1;
  doc[QStringLiteral("history")]      = m_history;
  doc[QStringLiteral("messages")]     = QJsonArray::fromVariantList(m_uiMessages);
  doc[QStringLiteral("handoff")]      = buildHandoffDigest();
  doc[QStringLiteral("handoffSeed")]  = m_handoffSeed;
  doc[QStringLiteral("loadedSkills")] = loaded;
  doc[QStringLiteral("probe")]        = probe;
  return doc;
}

/**
 * @brief Loads a captured snapshot into memory, resetting all in-flight turn state and
 *        downgrading any stale Running/AwaitingConfirm tool cards to Done.
 */
void AI::Conversation::loadSnapshot(const QJsonObject& doc)
{
  cancel();

  m_history    = doc.value(QStringLiteral("history")).toArray();
  m_uiMessages = doc.value(QStringLiteral("messages")).toArray().toVariantList();

  m_handoffSeed = doc.value(QStringLiteral("handoffSeed")).toString();
  m_loadedSkills.clear();
  const auto loaded = doc.value(QStringLiteral("loadedSkills")).toArray();
  for (const auto& v : loaded)
    m_loadedSkills.insert(v.toString());

  const bool was_degraded = m_probe.degraded();
  const auto was_failure  = m_probe.lastFailure();
  const auto was_drifted  = m_probe.driftedSegment();
  m_probe.reset(probeComplianceKey());
  const auto probe = doc.value(QStringLiteral("probe")).toObject();
  if (probe.value(QStringLiteral("degraded")).toBool()) {
    const int failure = probe.value(QStringLiteral("failure")).toInt();
    const auto kind   = failure == static_cast<int>(SentinelProbe::Outcome::Mutated)
                        ? SentinelProbe::Outcome::Mutated
                        : SentinelProbe::Outcome::Missing;
    m_probe.restoreLatch(true, kind, probe.value(QStringLiteral("drifted")).toString().left(64));
  }

  if (m_probe.degraded() != was_degraded || m_probe.lastFailure() != was_failure
      || m_probe.driftedSegment() != was_drifted)
    Q_EMIT probeStateChanged();

  m_assistantIndex = -1;
  m_assistantText.clear();
  m_assistantThinking.clear();
  m_pendingThinkingBlocks   = QJsonArray();
  m_pendingToolUseBlocks    = QJsonArray();
  m_pendingToolResultBlocks = QJsonArray();
  m_outstandingToolResults  = 0;
  m_awaitingConfirm.clear();
  setLastError(QString());

  ChatDigest::downgradeStaleToolCards(m_uiMessages);

  pruneHistory();

  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();
}

/**
 * @brief Returns the text of the first user row, used to title a chat.
 */
QString AI::Conversation::firstUserText() const
{
  return ChatDigest::firstUserText(m_uiMessages);
}

/**
 * @brief Returns the number of UI message rows in the conversation.
 */
int AI::Conversation::messageCount() const noexcept
{
  return static_cast<int>(m_uiMessages.size());
}

//--------------------------------------------------------------------------------------------------
// Handoff digest & probe state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Seeds this conversation with a previous chat's handoff digest; consumed by the
 *        system-prompt tail for the life of the chat. Re-capped here because seeds also
 *        arrive from on-disk chat files that a user (or corruption) may have grown.
 */
void AI::Conversation::setHandoffSeed(const QString& digest)
{
  m_handoffSeed = digest.left(kMaxDigestChars);
}

/**
 * @brief Returns the handoff digest this chat was seeded with, or empty.
 */
QString AI::Conversation::handoffSeed() const
{
  return m_handoffSeed;
}

/**
 * @brief Builds the deterministic handoff digest from the visible chat (no model call).
 */
QString AI::Conversation::buildHandoffDigest() const
{
  return ChatDigest::buildHandoffDigest(m_uiMessages, kMaxDigestChars);
}

/**
 * @brief Returns true while this conversation has a latched context-degradation verdict.
 */
bool AI::Conversation::probeDegraded() const noexcept
{
  return m_probe.degraded();
}

/**
 * @brief Returns a translated description of the latched degradation for the QML banner.
 */
QString AI::Conversation::probeDetail() const
{
  if (!m_probe.degraded())
    return {};

  if (m_probe.lastFailure() == SentinelProbe::Outcome::Missing)
    return tr("The model stopped reproducing its context-integrity line. Long "
              "conversations degrade silently; recent replies may be less reliable.");

  return tr("The model altered its context-integrity line (drifted segment: %1). Long "
            "conversations degrade silently; recent replies may be less reliable.")
    .arg(m_probe.driftedSegment());
}

//--------------------------------------------------------------------------------------------------
// Context-window budgeting
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the longest recent suffix of history that fits the provider context window,
 *        cut only at fresh user-turn boundaries so tool_use/tool_result pairs stay intact.
 *        Without a provider there is no window to fit, so the history is sent whole.
 */
QJsonArray AI::Conversation::budgetedHistory(const QJsonArray& tools) const
{
  if (!m_provider)
    return m_history;

  const auto caps = m_provider->capabilities();
  const TokenBudget::Window window{
    caps.contextWindowTokens, caps.maxOutputTokens, kSystemReserveTokens};
  return TokenBudget::budgetedHistory(m_history, TokenBudget::historyBudget(window, tools));
}
