/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Conversation/ChatDigest.h"

#include <QStringList>
#include <QVariantMap>

#include "AI/Conversation/ToolCallStatus.h"
#include "AI/Redactor.h"

/**
 * @brief Returns the text of the first user row, used to title a chat.
 */
QString AI::ChatDigest::firstUserText(const QVariantList& uiMessages)
{
  for (const auto& row : uiMessages) {
    const auto map = row.toMap();
    if (map.value(QStringLiteral("role")).toString() == QStringLiteral("user"))
      return map.value(QStringLiteral("text")).toString();
  }
  return {};
}

/**
 * @brief Builds the deterministic handoff digest from the visible chat (no model call):
 *        last user asks, recent completed non-meta tool actions, and the tail of the last
 *        reply, secret-scrubbed and capped. Scans the tail in reverse and stops once the
 *        digest inputs are full, so cost stays constant-bounded on long chats.
 */
QString AI::ChatDigest::buildHandoffDigest(const QVariantList& uiMessages, int maxChars)
{
  if (uiMessages.isEmpty())
    return {};

  QStringList asks;
  QStringList actions;
  QString last_reply;
  for (int i = static_cast<int>(uiMessages.size()) - 1; i >= 0; --i) {
    if (asks.size() >= 3 && !last_reply.isEmpty())
      break;

    const auto map  = uiMessages.at(i).toMap();
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
        card.value(QStringLiteral("status")).toInt() == static_cast<int>(ToolCallStatus::Done);
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
  out.truncate(maxChars);
  return out;
}

/**
 * @brief Downgrades tool cards left Running or AwaitingConfirm by a chat that was closed
 *        mid-turn, so a restored transcript never shows a spinner nothing will ever finish.
 */
void AI::ChatDigest::downgradeStaleToolCards(QVariantList& uiMessages)
{
  for (int i = 0; i < uiMessages.size(); ++i) {
    auto map     = uiMessages.at(i).toMap();
    auto calls   = map.value(QStringLiteral("toolCalls")).toList();
    bool changed = false;
    for (int j = 0; j < calls.size(); ++j) {
      auto card        = calls.at(j).toMap();
      const auto state = card.value(QStringLiteral("status")).toInt();
      if (state == static_cast<int>(ToolCallStatus::Running)
          || state == static_cast<int>(ToolCallStatus::AwaitingConfirm)) {
        card[QStringLiteral("status")] = static_cast<int>(ToolCallStatus::Done);
        calls[j]                       = card;
        changed                        = true;
      }
    }
    if (changed) {
      map.insert(QStringLiteral("toolCalls"), calls);
      uiMessages[i] = map;
    }
  }
}
