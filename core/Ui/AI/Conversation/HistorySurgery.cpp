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

#include "AI/Conversation/HistorySurgery.h"

#include "AI/Logging.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Tool-use / tool-result inspection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the ordered, deduped tool_use ids declared by an assistant message.
 */
QStringList AI::HistorySurgery::collectAssistantToolUseIds(const QJsonArray& content,
                                                           QSet<QString>& outIds)
{
  static const QString kKeyType     = QStringLiteral("type");
  static const QString kKeyId       = QStringLiteral("id");
  static const QString kTypeToolUse = QStringLiteral("tool_use");

  QStringList ordered;
  for (const auto& bv : content) {
    const auto block = bv.toObject();
    if (block.value(kKeyType).toString() != kTypeToolUse)
      continue;

    const auto tid = block.value(kKeyId).toString();
    if (tid.isEmpty() || outIds.contains(tid))
      continue;

    ordered.append(tid);
    outIds.insert(tid);
  }
  return ordered;
}

/**
 * @brief Returns the tool_use ids declared by the message immediately preceding @p userIdx
 *        when it is an assistant message with block content; an empty set otherwise.
 */
QSet<QString> AI::HistorySurgery::precedingAssistantToolUseIds(const QJsonArray& history,
                                                               int userIdx)
{
  QSet<QString> ids;
  if (userIdx <= 0 || userIdx > history.size())
    return ids;

  const auto prev = history.at(userIdx - 1).toObject();
  if (prev.value(QStringLiteral("role")).toString() != QStringLiteral("assistant"))
    return ids;

  const auto content = prev.value(QStringLiteral("content"));
  if (!content.isArray())
    return ids;

  (void)collectAssistantToolUseIds(content.toArray(), ids);
  return ids;
}

/**
 * @brief Filters a user message's content, keeping non-tool_result blocks and tool_result
 *        blocks whose id matches an assistant tool_use id (deduped via seenResultIds).
 */
QJsonArray AI::HistorySurgery::keepValidUserContent(const QJsonValue& userContent,
                                                    const QSet<QString>& assistantIds,
                                                    QSet<QString>& seenResultIds)
{
  static const QString kKeyType        = QStringLiteral("type");
  static const QString kKeyToolUseId   = QStringLiteral("tool_use_id");
  static const QString kTypeToolResult = QStringLiteral("tool_result");

  QJsonArray kept;
  if (userContent.isArray()) {
    for (const auto& bv : userContent.toArray()) {
      const auto block = bv.toObject();
      if (block.value(kKeyType).toString() != kTypeToolResult) {
        kept.append(block);
        continue;
      }

      const auto tid = block.value(kKeyToolUseId).toString();
      if (assistantIds.contains(tid) && !seenResultIds.contains(tid)) {
        kept.append(block);
        seenResultIds.insert(tid);
      }
    }
  } else if (userContent.isString()) {
    QJsonObject textBlock;
    textBlock[kKeyType]               = QStringLiteral("text");
    textBlock[QStringLiteral("text")] = userContent.toString();
    kept.append(textBlock);
  }
  return kept;
}

/**
 * @brief Builds synthetic tool_result blocks for every tool_use id that lacks a real result.
 */
QJsonArray AI::HistorySurgery::synthesizeMissingResults(const QStringList& orderedToolUseIds,
                                                        const QSet<QString>& seenResultIds)
{
  static const QString kKeyType        = QStringLiteral("type");
  static const QString kKeyToolUseId   = QStringLiteral("tool_use_id");
  static const QString kKeyContent     = QStringLiteral("content");
  static const QString kTypeToolResult = QStringLiteral("tool_result");
  static const QString kSyntheticResult =
    QStringLiteral("{\"ok\":false,\"error\":\"unresolved\",\"note\":\"synthesized after a "
                   "cancelled or interrupted tool batch\"}");

  QJsonArray out;
  for (const auto& tid : orderedToolUseIds) {
    if (seenResultIds.contains(tid))
      continue;

    QJsonObject block;
    block[kKeyType]      = kTypeToolResult;
    block[kKeyToolUseId] = tid;
    block[kKeyContent]   = kSyntheticResult;
    out.append(block);
  }
  return out;
}

//--------------------------------------------------------------------------------------------------
// Repair passes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Strips tool_result blocks whose tool_use is not declared by the immediately
 *        preceding assistant message, dropping user messages left without content. The API
 *        rejects the whole request on a single orphan, so corruption left by interrupted
 *        tool batches, pruning, or restored sessions must be removed before every send.
 */
void AI::HistorySurgery::stripOrphanToolResults(QJsonArray& history)
{
  static const QString kKeyType        = QStringLiteral("type");
  static const QString kKeyToolUseId   = QStringLiteral("tool_use_id");
  static const QString kTypeToolResult = QStringLiteral("tool_result");

  for (int i = 0; i < history.size(); ++i) {
    const auto msg = history.at(i).toObject();
    if (msg.value(QStringLiteral("role")).toString() != QStringLiteral("user"))
      continue;

    const auto contentValue = msg.value(QStringLiteral("content"));
    if (!contentValue.isArray())
      continue;

    const auto validIds = precedingAssistantToolUseIds(history, i);

    QSet<QString> seen;
    QJsonArray kept;
    bool mutated = false;
    for (const auto& bv : contentValue.toArray()) {
      const auto block = bv.toObject();
      if (block.value(kKeyType).toString() != kTypeToolResult) {
        kept.append(block);
        continue;
      }

      const auto tid = block.value(kKeyToolUseId).toString();
      if (!validIds.contains(tid) || seen.contains(tid)) {
        mutated = true;
        continue;
      }

      seen.insert(tid);
      kept.append(block);
    }

    if (!mutated)
      continue;

    qCWarning(serialStudioAI) << "Stripped orphan tool_result block(s) at history index" << i;
    if (kept.isEmpty()) {
      history.removeAt(i);
      --i;
      continue;
    }

    auto fixed                       = msg;
    fixed[QStringLiteral("content")] = kept;
    history[i]                       = fixed;
  }
}

/**
 * @brief Reconciles tool pairs for the assistant message at index i; advances i across an
 *        inserted synthetic user message. Returns true if the message was modified.
 */
bool AI::HistorySurgery::reconcileHistoryToolPairsAt(QJsonArray& history, int& i)
{
  static const QString kKeyRole       = QStringLiteral("role");
  static const QString kKeyContent    = QStringLiteral("content");
  static const QString kRoleAssistant = QStringLiteral("assistant");
  static const QString kRoleUser      = QStringLiteral("user");

  SS_ASSERT(i >= 0 && i < history.size(), return false);
  const auto msg = history.at(i).toObject();
  if (msg.value(kKeyRole).toString() != kRoleAssistant)
    return false;

  const auto contentValue = msg.value(kKeyContent);
  if (!contentValue.isArray())
    return false;

  QSet<QString> assistantIds;
  const QStringList orderedToolUseIds =
    collectAssistantToolUseIds(contentValue.toArray(), assistantIds);
  if (assistantIds.isEmpty())
    return false;

  const int nextIdx      = i + 1;
  const bool hasNextUser = nextIdx < history.size()
                        && history.at(nextIdx).toObject().value(kKeyRole).toString() == kRoleUser;

  QSet<QString> seenResultIds;
  QJsonArray keptContent;
  if (hasNextUser) {
    const auto userMsg = history.at(nextIdx).toObject();
    keptContent = keepValidUserContent(userMsg.value(kKeyContent), assistantIds, seenResultIds);
  }

  const QJsonArray synthesized = synthesizeMissingResults(orderedToolUseIds, seenResultIds);

  QJsonArray newContent;
  for (const auto& bv : synthesized)
    newContent.append(bv);

  for (const auto& bv : keptContent)
    newContent.append(bv);

  if (hasNextUser) {
    auto userMsg         = history.at(nextIdx).toObject();
    userMsg[kKeyContent] = newContent;
    history[nextIdx]     = userMsg;
    return true;
  }

  if (!synthesized.isEmpty()) {
    QJsonObject userMsg;
    userMsg[kKeyRole]    = kRoleUser;
    userMsg[kKeyContent] = newContent;
    history.insert(nextIdx, userMsg);
    ++i;
    SS_ASSERT_LOG(i < history.size());
    return true;
  }
  return false;
}

/**
 * @brief Pairs every assistant.tool_use with a tool_result, synthesizing or pruning as
 *        needed. Runs after pruneHistory so a prune cut can never ship an unpaired block.
 */
void AI::HistorySurgery::reconcileHistoryToolPairs(QJsonArray& history)
{
  stripOrphanToolResults(history);

  for (int i = 0; i < history.size(); ++i)
    (void)reconcileHistoryToolPairsAt(history, i);
}

//--------------------------------------------------------------------------------------------------
// Aging and pruning
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replaces an aged tool_result's payload (text and Gemini structured form) with a
 *        compact elision marker.
 */
QJsonObject AI::HistorySurgery::elideAgedToolResult(QJsonObject block)
{
  block[QStringLiteral("content")] =
    QStringLiteral("[old result removed from the transcript to save space; the call itself "
                   "SUCCEEDED when it ran. Not a size limit -- re-issue the same call only if "
                   "you need this data again.]");
  if (block.contains(QStringLiteral("_gemini_response"))) {
    QJsonObject elided;
    elided[QStringLiteral("elided")] =
      QStringLiteral("aged out of transcript; original call succeeded -- re-issue only if the "
                     "data is needed again");
    block[QStringLiteral("_gemini_response")] = elided;
  }
  return block;
}

/**
 * @brief True for tool names whose results are never elided: fs.* content and the bounded
 *        discovery lookups. Eliding a discovery payload forces the model into blind retry
 *        loops, so only in-budget-by-construction tools may join this set.
 */
[[nodiscard]] static bool isElisionExemptTool(const QString& toolName)
{
  const bool isFsContent = toolName == QStringLiteral("fs.read")
                        || toolName == QStringLiteral("fs.search")
                        || toolName == QStringLiteral("fs.list");
  const bool isDiscovery =
    toolName == QStringLiteral("meta.describeCommand")
    || toolName == QStringLiteral("meta.listCommands")
    || toolName == QStringLiteral("meta.listCategories")
    || toolName == QStringLiteral("meta.searchDocs") || toolName == QStringLiteral("meta.howTo")
    || toolName == QStringLiteral("meta.search") || toolName == QStringLiteral("project.search")
    || toolName == QStringLiteral("project.group.get");
  return isFsContent || isDiscovery;
}

/**
 * @brief Rewrites one aged user turn's blocks, eliding every oversized non-exempt
 *        tool_result. Returns true when at least one block was replaced.
 */
[[nodiscard]] static bool elideAgedTurn(const QJsonArray& blocks, QJsonArray& out)
{
  bool mutated = false;
  for (const auto& bv : blocks) {
    auto block          = bv.toObject();
    const auto toolName = block.value(QStringLiteral("_tool_name")).toString();
    const bool oversized =
      block.value(QStringLiteral("content")).toString().size() > AI::HistorySurgery::kElideMinChars;
    if (!isElisionExemptTool(toolName) && oversized
        && block.value(QStringLiteral("type")).toString() == QStringLiteral("tool_result")) {
      block   = AI::HistorySurgery::elideAgedToolResult(block);
      mutated = true;
    }
    out.append(block);
  }
  return mutated;
}

/**
 * @brief Stubs older tool_result blocks; keeps the kKeepRecentUserTurns most recent verbatim.
 */
void AI::HistorySurgery::ageHistoryToolResults(QJsonArray& history)
{
  int recentToolResultTurns = 0;
  for (int i = history.size() - 1; i >= 0; --i) {
    auto msg = history.at(i).toObject();
    if (msg.value(QStringLiteral("role")).toString() != QStringLiteral("user"))
      continue;

    const auto contentValue = msg.value(QStringLiteral("content"));
    if (!contentValue.isArray())
      continue;

    const auto blocks  = contentValue.toArray();
    bool hasToolResult = false;
    for (const auto& bv : blocks)
      if (bv.toObject().value(QStringLiteral("type")).toString() == QStringLiteral("tool_result")) {
        hasToolResult = true;
        break;
      }

    if (!hasToolResult)
      continue;

    if (recentToolResultTurns < kKeepRecentUserTurns) {
      ++recentToolResultTurns;
      continue;
    }

    QJsonArray newBlocks;
    if (elideAgedTurn(blocks, newBlocks)) {
      msg[QStringLiteral("content")] = newBlocks;
      history[i]                     = msg;
    }
  }
}

/**
 * @brief Index of the first fresh user turn at or after start, or -1 if none. A fresh turn
 *        carries text and no tool_result, so cutting there never splits a tool pair.
 */
int AI::HistorySurgery::firstFreshUserTurnAt(const QJsonArray& history, int start)
{
  for (int i = qMax(0, start); i < history.size(); ++i) {
    const auto msg = history.at(i).toObject();
    if (msg.value(QStringLiteral("role")).toString() != QStringLiteral("user"))
      continue;

    const auto blocks  = msg.value(QStringLiteral("content")).toArray();
    bool fresh         = false;
    bool hasToolResult = false;
    for (const auto& bv : blocks) {
      const auto type = bv.toObject().value(QStringLiteral("type")).toString();
      fresh           = fresh || type == QStringLiteral("text");
      hasToolResult   = hasToolResult || type == QStringLiteral("tool_result");
    }

    if (fresh && !hasToolResult)
      return i;
  }

  return -1;
}

/**
 * @brief Caps unbounded history growth so a long session cannot exhaust memory; the cut
 *        lands on a fresh user turn, never mid tool batch. Returns true when it cut.
 */
bool AI::HistorySurgery::pruneHistory(QJsonArray& history, int maxItems)
{
  SS_ASSERT(maxItems > 0, return false);
  if (history.size() <= maxItems)
    return false;

  const int cut = firstFreshUserTurnAt(history, history.size() - maxItems);
  if (cut <= 0)
    return false;

  QJsonArray pruned;
  for (int i = cut; i < history.size(); ++i)
    pruned.append(history.at(i));

  history = pruned;
  SS_ASSERT_LOG(history.size() <= maxItems);
  return true;
}
