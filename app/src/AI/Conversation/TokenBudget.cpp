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

#include "AI/Conversation/TokenBudget.h"

#include <QJsonDocument>
#include <QList>

#include "AI/Conversation/HistorySurgery.h"
#include "SSAssert.h"

/**
 * @brief Rough token estimate (~4 bytes/token) of a serialized block array.
 */
int AI::TokenBudget::estimateTokens(const QJsonArray& blocks)
{
  const auto bytes = QJsonDocument(blocks).toJson(QJsonDocument::Compact).size();
  return static_cast<int>(bytes / kBytesPerToken);
}

/**
 * @brief Tokens left for transcript history once the output reservation, the system-prompt
 *        reserve and the advertised tool surface are subtracted from the context window.
 *        A non-positive result means "do not trim", which the caller honours.
 */
int AI::TokenBudget::historyBudget(const Window& window, const QJsonArray& tools)
{
  return window.contextWindowTokens - window.maxOutputTokens - window.systemReserveTokens
       - estimateTokens(tools);
}

/**
 * @brief Returns the longest recent suffix of history that fits @p budget, cut only at fresh
 *        user-turn boundaries so tool_use/tool_result pairs stay intact. Suffix sums over one
 *        serialization pass keep the boundary scan arithmetic; a non-positive budget returns
 *        the history untouched.
 */
QJsonArray AI::TokenBudget::budgetedHistory(const QJsonArray& history, int budget)
{
  if (budget <= 0)
    return history;

  const auto n = history.size();
  QList<qint64> suffix_bytes(n + 1, 0);
  for (auto i = n - 1; i >= 0; --i) {
    const auto bytes = QJsonDocument(QJsonArray{history.at(i)}).toJson(QJsonDocument::Compact);
    suffix_bytes[i]  = suffix_bytes[i + 1] + bytes.size();
  }

  const auto suffixTokens = [&suffix_bytes](int from) {
    return static_cast<int>(suffix_bytes.at(from) / kBytesPerToken);
  };

  if (suffixTokens(0) <= budget)
    return history;

  const auto suffixFrom = [&history](int from) {
    QJsonArray out;
    for (int i = from; i < history.size(); ++i)
      out.append(history.at(i));

    return out;
  };

  QList<int> boundaries;
  for (int at = HistorySurgery::firstFreshUserTurnAt(history, 0); at >= 0;
       at     = HistorySurgery::firstFreshUserTurnAt(history, at + 1))
    boundaries.append(at);

  int chosen = boundaries.isEmpty() ? 0 : boundaries.constLast();
  for (const int b : boundaries)
    if (suffixTokens(b) <= budget) {
      chosen = b;
      break;
    }

  SS_ASSERT(chosen >= 0 && chosen <= history.size(),
            chosen = qBound(0, chosen, static_cast<int>(history.size())));
  return suffixFrom(chosen);
}
