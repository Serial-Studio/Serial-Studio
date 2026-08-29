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
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "Licensing/CommercialToken.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction / destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates an idle conversation; provider and dispatcher are wired later.
 */
AI::Conversation::Conversation(QObject* parent)
  : QObject(parent)
  , m_provider(nullptr)
  , m_dispatcher(nullptr)
  , m_reply(nullptr)
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
  connect(m_autoSaveTimer, &QTimer::timeout, this, [] {
    static auto& project = DataModel::ProjectModel::instance();
    if (!project.modified())
      return;

    if (project.jsonFilePath().isEmpty())
      return;

    project.setSuppressMessageBoxes(true);
    const bool ok = project.saveJsonFile(false);
    project.setSuppressMessageBoxes(false);
    if (!ok)
      qCWarning(serialStudioAI) << "AI auto-save failed";
    else
      qCDebug(serialStudioAI) << "AI auto-save:" << project.jsonFilePath();
  });
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
 * @brief Sets the tool dispatcher. The conversation does not take ownership.
 */
void AI::Conversation::setDispatcher(ToolDispatcher* dispatcher)
{
  m_dispatcher = dispatcher;
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
  static auto& assistant = Assistant::instance();
  if (!assistant.contextProbeEnabled())
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
  static auto& assistant = Assistant::instance();
  if (!assistant.skillRoutingEnabled())
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
  static auto& assistant = Assistant::instance();
  if (!assistant.memoryEnabled())
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

  if (dispatchMetaTool(callId, name, arguments))
    return;

  dispatchByCallSafety(callId, name, arguments);
}

/**
 * @brief Auto-handles meta-tool calls; returns true when consumed.
 */
bool AI::Conversation::dispatchMetaTool(const QString& callId,
                                        const QString& name,
                                        const QJsonObject& arguments)
{
  if (name == QStringLiteral("meta.listCategories")) {
    runMetaListCategories(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.snapshot")) {
    runMetaSnapshot(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.listCommands")) {
    runMetaListCommands(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.describeCommand")) {
    runMetaDescribe(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.executeCommand")) {
    runMetaExecuteCommand(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.fetchHelp")) {
    const auto path = arguments.value(QStringLiteral("path")).toString();
    appendToolCallCard(callId, name, arguments, CallStatus::Running);
    m_helpFetcher.fetchPage(callId, path);
    return true;
  }

  if (name == QStringLiteral("meta.fetchScriptingDocs")) {
    runMetaScriptingDocs(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.howTo")) {
    runMetaHowTo(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.loadSkill")) {
    runMetaLoadSkill(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.searchDocs")) {
    runMetaSearchDocs(callId, name, arguments);
    return true;
  }

  if (name == QStringLiteral("meta.search")) {
    appendToolCallCard(callId, name, arguments, CallStatus::Running);
    const auto query = arguments.value(QStringLiteral("query")).toString().trimmed();
    QJsonObject reply;
    if (query.isEmpty()) {
      reply[QStringLiteral("ok")]    = false;
      reply[QStringLiteral("error")] = QStringLiteral("missing_query");
      reply[QStringLiteral("hint")] =
        QStringLiteral("query cannot be empty; use meta.listCommands to enumerate the catalog.");
    } else {
      reply = m_dispatcher->searchCommands(query,
                                           arguments.value(QStringLiteral("offset")).toInt(0),
                                           arguments.value(QStringLiteral("limit")).toInt(0));
    }
    const bool ok = reply.value(QStringLiteral("ok")).toBool();
    recordToolResult(callId, name, reply);
    updateToolCallCard(callId, ok ? CallStatus::Done : CallStatus::Error, reply);
    releaseOutstandingToolResult();
    return true;
  }

  return false;
}

/**
 * @brief meta.listCategories: returns the dispatcher's category list.
 */
void AI::Conversation::runMetaListCategories(const QString& callId,
                                             const QString& name,
                                             const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto reply = m_dispatcher->listCategories();
  recordToolResult(callId, name, reply);
  updateToolCallCard(callId, CallStatus::Done, reply);
  releaseOutstandingToolResult();
}

/**
 * @brief meta.snapshot: returns the dispatcher's current state snapshot.
 */
void AI::Conversation::runMetaSnapshot(const QString& callId,
                                       const QString& name,
                                       const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  QJsonObject reply;
  reply[QStringLiteral("ok")]       = true;
  reply[QStringLiteral("snapshot")] = m_dispatcher->getSnapshot();
  recordToolResult(callId, name, reply);
  updateToolCallCard(callId, CallStatus::Done, reply);
  releaseOutstandingToolResult();
}

/**
 * @brief meta.listCommands: lists available commands filtered by prefix.
 */
void AI::Conversation::runMetaListCommands(const QString& callId,
                                           const QString& name,
                                           const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto prefix    = arguments.value(QStringLiteral("prefix")).toString();
  const int offset     = arguments.value(QStringLiteral("offset")).toInt(0);
  const int limit      = arguments.value(QStringLiteral("limit")).toInt(0);
  const bool namesOnly = arguments.value(QStringLiteral("namesOnly")).toBool(false);
  const auto reply     = m_dispatcher->listCommands(prefix, offset, limit, namesOnly);
  recordToolResult(callId, name, reply);
  updateToolCallCard(callId, CallStatus::Done, reply);
  releaseOutstandingToolResult();
}

/**
 * @brief meta.executeCommand: dispatches the inner tool with the same safety policy.
 */
void AI::Conversation::runMetaExecuteCommand(const QString& callId,
                                             const QString& name,
                                             const QJsonObject& arguments)
{
  const auto target    = arguments.value(QStringLiteral("name")).toString();
  const auto innerArgs = arguments.value(QStringLiteral("arguments")).toObject();

  if (target.isEmpty()) {
    QJsonObject err;
    err[QStringLiteral("ok")]    = false;
    err[QStringLiteral("error")] = QStringLiteral("missing_name");
    appendToolCallCard(callId, name, arguments, CallStatus::Error);
    recordToolResult(callId, name, err);
    updateToolCallCard(callId, CallStatus::Error, err);
    releaseOutstandingToolResult();
    return;
  }

  dispatchByCallSafety(callId, target, innerArgs);
}

/**
 * @brief meta.loadSkill: returns the markdown body of a registered skill.
 */
void AI::Conversation::runMetaLoadSkill(const QString& callId,
                                        const QString& name,
                                        const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto skillId = arguments.value(QStringLiteral("name")).toString();
  const auto body    = AI::ContextBuilder::skillBody(skillId);

  QJsonObject reply;
  if (body.isEmpty()) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("unknown_skill");
    QJsonArray known;
    for (const auto& s : AI::ContextBuilder::skillIds())
      known.append(s);

    reply[QStringLiteral("availableSkills")] = known;
    recordToolResult(callId, name, reply);
    updateToolCallCard(callId, CallStatus::Error, reply);
  } else {
    reply[QStringLiteral("ok")]    = true;
    reply[QStringLiteral("skill")] = skillId;
    reply[QStringLiteral("body")]  = body;
    m_loadedSkills.insert(skillId);
    recordToolResult(callId, name, reply);
    updateToolCallCard(callId, CallStatus::Done, reply);
  }
  releaseOutstandingToolResult();
}

/**
 * @brief meta.searchDocs: BM25-style doc search via DocSearch singleton.
 */
void AI::Conversation::runMetaSearchDocs(const QString& callId,
                                         const QString& name,
                                         const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto query = arguments.value(QStringLiteral("query")).toString();
  const int k      = qBound(1, arguments.value(QStringLiteral("k")).toInt(5), 10);

  static auto& docSearch = AI::DocSearch::instance();
  const auto hits        = docSearch.search(query, k);

  QJsonArray rows;
  for (const auto& h : hits) {
    QJsonObject row;
    row[QStringLiteral("id")]     = h.id;
    row[QStringLiteral("source")] = h.source;
    row[QStringLiteral("title")]  = h.title;
    row[QStringLiteral("body")]   = h.body;
    row[QStringLiteral("score")]  = h.score;
    rows.append(row);
  }

  QJsonObject reply;
  reply[QStringLiteral("ok")]    = true;
  reply[QStringLiteral("query")] = query;
  reply[QStringLiteral("hits")]  = rows;
  reply[QStringLiteral("count")] = rows.size();
  if (rows.isEmpty()) {
    reply[QStringLiteral("hint")] =
      QStringLiteral("No matches. Try rephrasing with command-shaped terms (e.g. "
                     "'project.dataset.add' instead of 'add a channel'), or fall back to "
                     "meta.listCommands{prefix} / meta.fetchHelp{path: 'help.json'}.");
  }

  recordToolResult(callId, name, reply);
  updateToolCallCard(callId, CallStatus::Done, reply);
  releaseOutstandingToolResult();
}

/**
 * @brief Returns the skill id whose body documents @a commandName, or empty.
 */
static QString skillForCommand(const QString& commandName)
{
  if (commandName.startsWith(QStringLiteral("project.workspace."))
      || commandName == QStringLiteral("project.dataset.setOption")
      || commandName == QStringLiteral("project.dataset.setOptions"))
    return QStringLiteral("dashboard_layout");

  if (commandName.startsWith(QStringLiteral("project.frameParser.")))
    return QStringLiteral("frame_parsers");

  if (commandName.startsWith(QStringLiteral("project.painter.")))
    return QStringLiteral("painter");

  if (commandName.startsWith(QStringLiteral("project.outputWidget.")))
    return QStringLiteral("output_widgets");

  if (commandName == QStringLiteral("project.dataset.setTransformCode")
      || commandName == QStringLiteral("project.dataset.transform.dryRun")
      || commandName.startsWith(QStringLiteral("project.dataTable.")))
    return QStringLiteral("transforms");

  if (commandName.startsWith(QStringLiteral("project.mqtt.")))
    return QStringLiteral("mqtt");

  if (commandName.startsWith(QStringLiteral("io.canbus."))
      || commandName.startsWith(QStringLiteral("io.modbus.")))
    return QStringLiteral("can_modbus");

  if (commandName.startsWith(QStringLiteral("project.")))
    return QStringLiteral("project_basics");

  return QString();
}

/**
 * @brief meta.describeCommand handler: returns command schema or not_found.
 */
void AI::Conversation::runMetaDescribe(const QString& callId,
                                       const QString& name,
                                       const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto target = arguments.value(QStringLiteral("name")).toString();
  QJsonObject reply;
  if (target.isEmpty()) {
    reply[QStringLiteral("ok")]    = false;
    reply[QStringLiteral("error")] = QStringLiteral("missing_name");
  } else {
    const auto desc = m_dispatcher->describeCommand(target);
    if (desc.isEmpty()) {
      reply[QStringLiteral("ok")]    = false;
      reply[QStringLiteral("error")] = QStringLiteral("not_found");
      reply[QStringLiteral("name")]  = target;
    } else {
      reply[QStringLiteral("ok")]      = true;
      reply[QStringLiteral("command")] = desc;
      const auto skill                 = skillForCommand(target);
      if (!skill.isEmpty())
        reply[QStringLiteral("loadSkillFirst")] = skill;
    }
  }
  recordToolResult(callId, name, reply);
  updateToolCallCard(callId,
                     reply.value(QStringLiteral("ok")).toBool() ? CallStatus::Done
                                                                : CallStatus::Error,
                     reply);
  releaseOutstandingToolResult();
}

/**
 * @brief meta.fetchScriptingDocs handler: returns the canonical doc body for a kind.
 */
void AI::Conversation::runMetaScriptingDocs(const QString& callId,
                                            const QString& name,
                                            const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto kind = arguments.value(QStringLiteral("kind")).toString();
  const auto body = ContextBuilder::scriptingDocFor(kind);

  QJsonObject result;
  if (body.isEmpty()) {
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("Unknown kind '%1'. Valid: frame_parser_js, "
                     "frame_parser_lua, transform_js, transform_lua, "
                     "output_widget_js, painter_js, control_script_js, "
                     "sdk_js, sdk_lua.")
        .arg(kind);
    updateToolCallCard(callId, CallStatus::Error, result);
  } else {
    result[QStringLiteral("ok")]      = true;
    result[QStringLiteral("kind")]    = kind;
    result[QStringLiteral("content")] = body;
    updateToolCallCard(callId, CallStatus::Done, result);
  }

  recordToolResult(callId, name, result);
  releaseOutstandingToolResult();
}

/**
 * @brief meta.howTo handler: returns a canned step-by-step recipe by task id.
 */
void AI::Conversation::runMetaHowTo(const QString& callId,
                                    const QString& name,
                                    const QJsonObject& arguments)
{
  appendToolCallCard(callId, name, arguments, CallStatus::Running);
  const auto task   = arguments.value(QStringLiteral("task")).toString();
  const auto recipe = ContextBuilder::howToRecipe(task);

  QJsonObject result;
  if (recipe.isEmpty()) {
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("Unknown task '%1'. Valid tasks: %2")
        .arg(task, ContextBuilder::howToTasks().join(QStringLiteral(", ")));
    updateToolCallCard(callId, CallStatus::Error, result);
  } else {
    result[QStringLiteral("ok")]    = true;
    result[QStringLiteral("task")]  = task;
    result[QStringLiteral("steps")] = recipe;
    updateToolCallCard(callId, CallStatus::Done, result);
  }

  recordToolResult(callId, name, result);
  releaseOutstandingToolResult();
}

/**
 * @brief Routes a non-meta tool call by its CommandRegistry safety tag.
 */
void AI::Conversation::dispatchByCallSafety(const QString& callId,
                                            const QString& requestedName,
                                            const QJsonObject& arguments)
{
  const QString name           = m_dispatcher->canonicalToolName(requestedName);
  static auto& commandRegistry = AI::CommandRegistry::instance();
  const auto safety            = commandRegistry.safetyOf(name);
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
    if (safety == Safety::Confirm) {
      static auto& assistant = Assistant::instance();
      autoApprove            = assistant.autoApproveEdits();
    }

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
    static auto& assistant = Assistant::instance();
    assistant.reportCacheStats(read, created);
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
static QString toolCallCategory(const QString& name)
{
  if (name.startsWith(QStringLiteral("meta.")))
    return QStringLiteral("discovery");

  static auto& commandRegistry = AI::CommandRegistry::instance();
  if (commandRegistry.safetyOf(name) == AI::Safety::Safe)
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
  card[QStringLiteral("category")] = toolCallCategory(name);
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

  auto effective          = reply;
  const auto verification = runAutoVerify(name, arguments, reply);
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
  static auto& commandRegistry = AI::CommandRegistry::instance();
  const auto safety            = commandRegistry.safetyOf(name);
  const bool isReadOnly        = (safety == Safety::Safe);
  if (ok && !isMeta && !isExplicit && !isReadOnly)
    m_autoSaveTimer->start();
}

/**
 * @brief Maps an apply-class mutation to its Safe-tier read-back check, mirroring the arg
 *        construction assistant.script.apply proved out; returns false when no map exists.
 */
static bool readBackCommandFor(const QString& name,
                               const QJsonObject& arguments,
                               QString& cmd,
                               QJsonObject& args)
{
  if (name == QStringLiteral("project.frameParser.setCode")) {
    cmd  = QStringLiteral("project.frameParser.dryCompile");
    args = arguments;
    return true;
  }

  if (name == QStringLiteral("project.dataset.setTransformCode")) {
    cmd  = QStringLiteral("project.dataset.transform.dryRun");
    args = arguments;
    if (!args.contains(QStringLiteral("values")))
      args[QStringLiteral("values")] = QJsonArray{0.0};

    return true;
  }

  if (name == QStringLiteral("project.painter.setCode")) {
    cmd  = QStringLiteral("project.painter.dryRun");
    args = arguments;
    return true;
  }

  if (name == QStringLiteral("project.outputWidget.update")
      && arguments.contains(QStringLiteral("transmitFunction"))) {
    cmd  = QStringLiteral("project.outputWidget.dryRun");
    args = QJsonObject{
      {QStringLiteral("code"), arguments.value(QStringLiteral("transmitFunction"))}
    };
    return true;
  }

  return false;
}

/**
 * @brief Harness-enforced verification after a successful apply-class mutation: runs the
 *        matching Safe-tier read-back (asserted, never Confirm-class) and returns a
 *        verification object for the tool result and card, or empty when not applicable.
 */
QJsonObject AI::Conversation::runAutoVerify(const QString& name,
                                            const QJsonObject& arguments,
                                            const QJsonObject& reply)
{
  static auto& assistant = Assistant::instance();
  if (!assistant.autoVerifyEnabled() || !reply.value(QStringLiteral("ok")).toBool())
    return {};

  if (name == QStringLiteral("assistant.script.apply")) {
    QJsonObject v;
    v[QStringLiteral("ok")]     = true;
    v[QStringLiteral("method")] = QStringLiteral("internal dry-run");
    qCDebug(serialStudioAI) << "AutoVerify:" << name << "via internal dry-run ok= true";
    return v;
  }

  if (name == QStringLiteral("assistant.project.bulkApply")) {
    const int failures = reply.value(QStringLiteral("failureCount")).toInt();
    QJsonObject v;
    v[QStringLiteral("ok")]     = failures == 0;
    v[QStringLiteral("method")] = QStringLiteral("batch failure scan");
    if (failures > 0)
      v[QStringLiteral("detail")] = tr("%1 operation(s) failed").arg(failures);

    qCDebug(serialStudioAI) << "AutoVerify:" << name
                            << "via batch failure scan ok=" << (failures == 0);
    return v;
  }

  if (name == QStringLiteral("project.source.update"))
    return verifySourceUpdate(arguments);

  QString cmd;
  QJsonObject args;
  if (!readBackCommandFor(name, arguments, cmd, args))
    return {};

  static auto& registry = AI::CommandRegistry::instance();
  const bool safe       = registry.safetyOf(cmd) == Safety::Safe;
  if (!safe) {
    qCWarning(serialStudioAI) << "AutoVerify: refusing non-Safe read-back" << cmd;
    return {};
  }

  const auto check = m_dispatcher->executeCommand(cmd, args);
  bool check_ok    = check.value(QStringLiteral("ok")).toBool();
  const auto inner = check.value(QStringLiteral("result")).toObject();
  if (check_ok && inner.contains(QStringLiteral("ok")))
    check_ok = inner.value(QStringLiteral("ok")).toBool();

  QJsonObject v;
  v[QStringLiteral("ok")]     = check_ok;
  v[QStringLiteral("method")] = cmd;
  if (!check_ok)
    v[QStringLiteral("detail")] =
      QString::fromUtf8(QJsonDocument(check).toJson(QJsonDocument::Compact)).left(300);

  qCDebug(serialStudioAI) << "AutoVerify:" << name << "via" << cmd << "ok=" << check_ok;
  return v;
}

/**
 * @brief Read-back verification for project.source.update: fetches the Safe-tier source
 *        list and confirms every requested field actually round-tripped, because generic
 *        CRUD patches are where weak models misfire most (observed 2026-07-14).
 */
QJsonObject AI::Conversation::verifySourceUpdate(const QJsonObject& arguments)
{
  QJsonObject v;
  v[QStringLiteral("method")] = QStringLiteral("project.source.list");

  const auto check =
    m_dispatcher->executeCommand(QStringLiteral("project.source.list"), QJsonObject());
  if (!check.value(QStringLiteral("ok")).toBool()) {
    v[QStringLiteral("ok")]     = false;
    v[QStringLiteral("detail")] = tr("Source list read-back failed");
    qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list "
                               "ok= false (list failed)";
    return v;
  }

  const int source_id = arguments.value(Keys::SourceId).toInt(-1);
  const auto rows =
    check.value(QStringLiteral("result")).toObject().value(QStringLiteral("sources")).toArray();

  QJsonObject row;
  for (const auto& r : rows) {
    const auto obj = r.toObject();
    if (obj.value(Keys::SourceId).toInt(-1) == source_id) {
      row = obj;
      break;
    }
  }

  if (row.isEmpty()) {
    v[QStringLiteral("ok")]     = false;
    v[QStringLiteral("detail")] = tr("Source %1 not found after update").arg(source_id);
    qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list "
                               "ok= false (source missing)";
    return v;
  }

  QStringList mismatched;
  for (auto it = arguments.constBegin(); it != arguments.constEnd(); ++it) {
    if (it.key() == Keys::SourceId || !row.contains(it.key()))
      continue;

    if (row.value(it.key()) != it.value())
      mismatched.append(it.key());
  }

  v[QStringLiteral("ok")] = mismatched.isEmpty();
  if (!mismatched.isEmpty())
    v[QStringLiteral("detail")] =
      tr("Fields did not round-trip: %1").arg(mismatched.join(QStringLiteral(", ")));

  qCDebug(serialStudioAI) << "AutoVerify: project.source.update via project.source.list ok="
                          << mismatched.isEmpty();
  return v;
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

  const auto caps        = m_provider ? m_provider->capabilities() : ProviderCapabilities{};
  static auto& assistant = Assistant::instance();
  const bool memory_on   = assistant.memoryEnabled();
  const int cache_key    = (caps.needsSmallToolSurface ? 1 : 0) | (memory_on ? 2 : 0);

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

  for (int i = 0; i < m_uiMessages.size(); ++i) {
    auto map     = m_uiMessages.at(i).toMap();
    auto calls   = map.value(QStringLiteral("toolCalls")).toList();
    bool changed = false;
    for (int j = 0; j < calls.size(); ++j) {
      auto card        = calls.at(j).toMap();
      const auto state = card.value(QStringLiteral("status")).toInt();
      if (state == static_cast<int>(CallStatus::Running)
          || state == static_cast<int>(CallStatus::AwaitingConfirm)) {
        card[QStringLiteral("status")] = static_cast<int>(CallStatus::Done);
        calls[j]                       = card;
        changed                        = true;
      }
    }
    if (changed) {
      map.insert(QStringLiteral("toolCalls"), calls);
      m_uiMessages[i] = map;
    }
  }

  pruneHistory();

  Q_EMIT messagesChanged();
  Q_EMIT messageCountChanged();
}

/**
 * @brief Returns the text of the first user row, used to title a chat.
 */
QString AI::Conversation::firstUserText() const
{
  for (const auto& row : m_uiMessages) {
    const auto map = row.toMap();
    if (map.value(QStringLiteral("role")).toString() == QStringLiteral("user"))
      return map.value(QStringLiteral("text")).toString();
  }
  return {};
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
 * @brief Builds the deterministic handoff digest from the visible chat (no model call):
 *        last user asks, recent completed non-meta tool actions, and the tail of the last
 *        reply, secret-scrubbed and capped. Scans the tail in reverse and stops once the
 *        digest inputs are full, so cost stays constant-bounded on long chats.
 */
QString AI::Conversation::buildHandoffDigest() const
{
  if (m_uiMessages.isEmpty())
    return {};

  QStringList asks;
  QStringList actions;
  QString last_reply;
  for (int i = static_cast<int>(m_uiMessages.size()) - 1; i >= 0; --i) {
    if (asks.size() >= 3 && !last_reply.isEmpty())
      break;

    const auto map  = m_uiMessages.at(i).toMap();
    const auto role = map.value(QStringLiteral("role")).toString();
    if (role == QStringLiteral("user") && asks.size() < 3) {
      const auto text =
        map.value(QStringLiteral("text")).toString().left(480).simplified().left(120);
      if (!text.isEmpty())
        asks.prepend(text);
    }

    if (role != QStringLiteral("assistant"))
      continue;

    if (last_reply.isEmpty())
      last_reply = map.value(QStringLiteral("text")).toString().left(800).simplified().left(200);

    const auto calls = map.value(QStringLiteral("toolCalls")).toList();
    for (const auto& c : calls) {
      const auto card = c.toMap();
      const auto name = card.value(QStringLiteral("name")).toString();
      const auto done =
        card.value(QStringLiteral("status")).toInt() == static_cast<int>(CallStatus::Done);
      if (done && !name.startsWith(QStringLiteral("meta.")) && !actions.contains(name)
          && actions.size() < 10)
        actions.append(name);
    }
  }

  QString out;
  out += QStringLiteral("Asked: ") + asks.join(QStringLiteral(" | ")) + QLatin1Char('\n');
  if (!actions.isEmpty())
    out += QStringLiteral("Actions: ") + actions.join(QStringLiteral(", ")) + QLatin1Char('\n');

  if (!last_reply.isEmpty())
    out += QStringLiteral("Last reply: ") + last_reply + QLatin1Char('\n');

  (void)Redactor::scrub(out);
  out.truncate(kMaxDigestChars);
  return out;
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
