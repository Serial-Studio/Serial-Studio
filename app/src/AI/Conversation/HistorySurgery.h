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

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QString>
#include <QStringList>

namespace AI::HistorySurgery {

/// Number of most recent tool-result user turns kept verbatim by ageHistoryToolResults().
inline constexpr int kKeepRecentUserTurns = 2;

/// Minimum serialized tool_result payload length before aging is allowed to elide it.
inline constexpr int kElideMinChars = 64;

[[nodiscard]] QStringList collectAssistantToolUseIds(const QJsonArray& content,
                                                     QSet<QString>& outIds);

[[nodiscard]] QSet<QString> precedingAssistantToolUseIds(const QJsonArray& history, int userIdx);

[[nodiscard]] QJsonArray keepValidUserContent(const QJsonValue& userContent,
                                              const QSet<QString>& assistantIds,
                                              QSet<QString>& seenResultIds);

[[nodiscard]] QJsonArray synthesizeMissingResults(const QStringList& orderedToolUseIds,
                                                  const QSet<QString>& seenResultIds);

[[nodiscard]] QJsonObject elideAgedToolResult(QJsonObject block);

[[nodiscard]] int firstFreshUserTurnAt(const QJsonArray& history, int start);

void stripOrphanToolResults(QJsonArray& history);

[[nodiscard]] bool reconcileHistoryToolPairsAt(QJsonArray& history, int& i);

void reconcileHistoryToolPairs(QJsonArray& history);

void ageHistoryToolResults(QJsonArray& history);

[[nodiscard]] bool pruneHistory(QJsonArray& history, int maxItems);

}  // namespace AI::HistorySurgery
