/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation/ToolTurnRunner.h"

#include <algorithm>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVariantMap>

#include "AI/CommandRegistry.h"
#include "Core/SSAssert.h"

/**
 * @brief Binds the chat model the cards are rendered into and the command registry the category
 *        badge is derived from; both outlive this object.
 */
AI::ToolTurnRunner::ToolTurnRunner(QVariantList& uiMessages,
                                   const int& assistantIndex,
                                   const CommandRegistry& commands)
  : m_outstandingToolResults(0)
  , m_uiMessages(uiMessages)
  , m_assistantIndex(assistantIndex)
  , m_commands(commands)
{}

//--------------------------------------------------------------------------------------------------
// Pending confirmations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Parks a Confirm-tagged call until the user answers.
 */
void AI::ToolTurnRunner::awaitConfirmation(const QString& callId,
                                           const QString& name,
                                           const QJsonObject& args)
{
  SS_ASSERT(!callId.isEmpty(), return);
  SS_ASSERT_LOG(!name.isEmpty());

  m_awaitingConfirm.insert(callId, PendingCall{name, args});
}

/**
 * @brief Removes and returns one parked call, or nothing when the id is unknown: a double click
 *        on Approve must not run the same tool twice.
 */
std::optional<AI::ToolTurnRunner::PendingCall> AI::ToolTurnRunner::takePending(
  const QString& callId)
{
  const auto it = m_awaitingConfirm.constFind(callId);
  if (it == m_awaitingConfirm.constEnd())
    return std::nullopt;

  const auto pending = it.value();
  m_awaitingConfirm.erase(it);
  return pending;
}

/**
 * @brief Every parked call id, for the paths that answer the whole batch at once (cancel, and a
 *        stream error that ends the turn).
 */
QStringList AI::ToolTurnRunner::pendingIds() const
{
  return m_awaitingConfirm.keys();
}

/**
 * @brief The parked call ids whose tool name is @p family or sits under it, for the group
 *        approve/deny buttons.
 */
QStringList AI::ToolTurnRunner::pendingInFamily(const QString& family) const
{
  QStringList ids;
  if (family.isEmpty())
    return ids;

  for (auto it = m_awaitingConfirm.constBegin(); it != m_awaitingConfirm.constEnd(); ++it)
    if (it.value().name.startsWith(family + QLatin1Char('.')) || it.value().name == family)
      ids.append(it.key());

  return ids;
}

/**
 * @brief True while any call is still waiting on the user.
 */
bool AI::ToolTurnRunner::hasPending() const noexcept
{
  return !m_awaitingConfirm.isEmpty();
}

/**
 * @brief Drops every parked call; a cleared or cancelled turn owes the model no answer.
 */
void AI::ToolTurnRunner::clearPending()
{
  m_awaitingConfirm.clear();
}

//--------------------------------------------------------------------------------------------------
// Outstanding results
//--------------------------------------------------------------------------------------------------

/**
 * @brief Counts one tool result the turn still owes the model.
 */
void AI::ToolTurnRunner::noteOutstandingResult() noexcept
{
  ++m_outstandingToolResults;
}

/**
 * @brief Settles one owed tool result, never below zero: a denial and an async completion can
 *        both land for the same call if the user denies while the worker is finishing.
 */
void AI::ToolTurnRunner::releaseOutstandingResult() noexcept
{
  if (m_outstandingToolResults > 0)
    --m_outstandingToolResults;
}

/**
 * @brief Forgets every owed result; a cancelled turn resumes nothing.
 */
void AI::ToolTurnRunner::resetOutstanding() noexcept
{
  m_outstandingToolResults = 0;
}

/**
 * @brief Tool results the turn still owes the model.
 */
int AI::ToolTurnRunner::outstandingResults() const noexcept
{
  return m_outstandingToolResults;
}

/**
 * @brief The turn may resume only when nothing is outstanding: no tool result pending, no
 *        confirmation waiting, and no reply still streaming. The three conditions are ONE rule,
 *        so every completion path shares this verdict.
 */
bool AI::ToolTurnRunner::batchComplete(bool replyLive) const noexcept
{
  return m_outstandingToolResults <= 0 && m_awaitingConfirm.isEmpty() && !replyLive;
}

//--------------------------------------------------------------------------------------------------
// Tool-call cards
//--------------------------------------------------------------------------------------------------

/**
 * @brief The indexes of the ops a card lists, in batch order: the ops stricter than Confirm
 *        first (up to 200), then the others until the cap, so the op that raised the batch's tier
 *        does not hide behind "+N more" (spec 0098 R9).
 */
static QList<qsizetype> listedOpIndexes(const AI::CommandRegistry& registry,
                                        const QStringList& planned)
{
  constexpr qsizetype kMaxListedOps = 50;
  constexpr qsizetype kMaxStrictOps = 200;
  const int confirm                 = AI::CommandRegistry::strictness(AI::Safety::Confirm);

  QList<bool> strict(planned.size(), false);
  qsizetype strictCount = 0;
  for (qsizetype i = 0; i < planned.size(); ++i) {
    const auto tier  = AI::CommandRegistry::strictness(registry.safetyOf(planned.at(i)));
    strict[i]        = tier > confirm || AI::CommandRegistry::isDispatcher(planned.at(i));
    strictCount     += strict.at(i) ? 1 : 0;
  }

  auto strictBudget = std::min(strictCount, kMaxStrictOps);
  auto budget       = std::max<qsizetype>(0, kMaxListedOps - strictBudget);
  QList<qsizetype> indexes;
  for (qsizetype i = 0; i < planned.size(); ++i) {
    if (strict.at(i)) {
      if (strictBudget > 0) {
        indexes.append(i);
        --strictBudget;
      }

      continue;
    }

    if (budget > 0) {
      indexes.append(i);
      --budget;
    }
  }

  return indexes;
}

/**
 * @brief A batch op's params as one compact line, never cut inside a surrogate pair.
 */
static QString opDetail(const QJsonArray& ops, const qsizetype index)
{
  constexpr int kMaxDetailLength = 96;
  if (index < 0 || index >= ops.size())
    return QString();

  const auto params = ops.at(index).toObject().value(QStringLiteral("params")).toObject();
  auto detail       = QString::fromUtf8(QJsonDocument(params).toJson(QJsonDocument::Compact));
  if (detail.size() <= kMaxDetailLength)
    return detail;

  auto cut = kMaxDetailLength - 1;
  if (detail.at(cut - 1).isHighSurrogate())
    --cut;

  return detail.left(cut) + QChar(0x2026);
}

/**
 * @brief The operations a call will run, one line each, for its approval card (spec 0098 R9): the
 *        command plus, for a batch, a compact view of its params. The total travels beside it.
 */
static QVariantList plannedOpsSummary(const AI::CommandRegistry& registry,
                                      const QString& name,
                                      const QJsonObject& arguments)
{
  const auto planned   = AI::CommandRegistry::plannedCommands(name, arguments);
  const auto ops       = arguments.value(QStringLiteral("ops")).toArray();
  const bool fromBatch = AI::CommandRegistry::isDispatcher(name);

  QVariantList out;
  for (const auto i : listedOpIndexes(registry, planned)) {
    const auto detail = fromBatch ? opDetail(ops, i) : QString();

    QVariantMap op;
    op[QStringLiteral("command")] = planned.at(i);
    op[QStringLiteral("detail")]  = detail;
    out.append(op);
  }

  return out;
}

/**
 * @brief Returns "discovery" for read-only / meta calls, "execution" otherwise.
 */
QString AI::ToolTurnRunner::categoryOf(const QString& name) const
{
  if (name.startsWith(QStringLiteral("meta.")))
    return QStringLiteral("discovery");

  if (m_commands.safetyOf(name) == AI::Safety::Safe)
    return QStringLiteral("discovery");

  return QStringLiteral("execution");
}

/**
 * @brief Adds a ToolCallCard payload to the active assistant message; false when there is no
 *        active message to attach it to.
 */
bool AI::ToolTurnRunner::appendCard(const QString& callId,
                                    const QString& name,
                                    const QJsonObject& arguments,
                                    ToolCallStatus status)
{
  if (m_assistantIndex < 0 || m_assistantIndex >= m_uiMessages.size())
    return false;

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
  card[QStringLiteral("category")] = categoryOf(name);
  card[QStringLiteral("args")]     = QJsonDocument(arguments).toJson(QJsonDocument::Indented);
  card[QStringLiteral("status")]   = static_cast<int>(status);
  card[QStringLiteral("result")]   = QString();

  const auto planned = AI::CommandRegistry::plannedCommands(name, arguments);
  if (!planned.isEmpty()) {
    card[QStringLiteral("ops")]      = plannedOpsSummary(m_commands, name, arguments);
    card[QStringLiteral("opsTotal")] = static_cast<int>(planned.size());
  }

  calls.append(card);
  map.insert(QStringLiteral("toolCalls"), calls);
  m_uiMessages[m_assistantIndex] = map;
  return true;
}

/**
 * @brief Updates the status (and optional result) of an existing ToolCallCard; false when no
 *        message carries that call id.
 */
bool AI::ToolTurnRunner::updateCard(const QString& callId,
                                    ToolCallStatus status,
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
      return true;
    }
  }

  return false;
}
