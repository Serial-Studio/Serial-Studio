/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "FakeProvider.h"

#include <QTimer>
#include <QUuid>
#include <utility>

namespace Test {

FakeReply::FakeReply(QList<ReplyEvent> script, QObject* parent)
  : AI::Reply(parent), m_script(std::move(script)), m_index(0), m_aborted(false)
{
  QTimer::singleShot(0, this, &FakeReply::deliverNext);
}

FakeReply::~FakeReply() = default;

/**
 * @brief Stops the script where it stands; nothing further is delivered.
 */
void FakeReply::abort()
{
  m_aborted = true;
}

bool FakeReply::aborted() const
{
  return m_aborted;
}

int FakeReply::deliveredEvents() const
{
  return m_index;
}

/**
 * @brief Delivers one scripted event and schedules the next.
 */
void FakeReply::deliverNext()
{
  if (m_aborted || m_index >= m_script.size())
    return;

  const ReplyEvent event = m_script.at(m_index);
  ++m_index;

  switch (event.kind) {
    case ReplyEvent::Text:
      if (chargeStreamBudget(event.text.size()))
        Q_EMIT errorOccurred(QStringLiteral("streamed reply budget exceeded"));
      else
        Q_EMIT partialText(event.text);
      break;

    case ReplyEvent::ToolCall:
      Q_EMIT toolCallRequested(
        QUuid::createUuid().toString(QUuid::WithoutBraces), event.toolName, event.args);
      break;

    case ReplyEvent::Error:
      Q_EMIT errorOccurred(event.text);
      break;

    case ReplyEvent::Done:
      Q_EMIT finished();
      return;

    case ReplyEvent::BudgetBreach:
      (void)chargeStreamBudget(AI::Reply::kMaxStreamedReplyBytes + 1);
      Q_EMIT errorOccurred(QStringLiteral("streamed reply budget exceeded"));
      break;
  }

  QTimer::singleShot(0, this, &FakeReply::deliverNext);
}

FakeProvider::FakeProvider() : m_sendCount(0), m_lastForbidToolUse(false) {}

FakeProvider::~FakeProvider() = default;

/**
 * @brief Sets the event list every subsequent reply replays.
 */
void FakeProvider::script(QList<ReplyEvent> events)
{
  m_script = std::move(events);
}

int FakeProvider::sendCount() const
{
  return m_sendCount;
}

QJsonArray FakeProvider::lastHistory() const
{
  return m_lastHistory;
}

QJsonArray FakeProvider::lastTools() const
{
  return m_lastTools;
}

bool FakeProvider::lastForbidToolUse() const
{
  return m_lastForbidToolUse;
}

QString FakeProvider::displayName() const
{
  return QStringLiteral("Fake");
}

QString FakeProvider::keyVendorUrl() const
{
  return QStringLiteral("https://example.invalid/keys");
}

QStringList FakeProvider::availableModels() const
{
  return {QStringLiteral("fake-small"), QStringLiteral("fake-large")};
}

QString FakeProvider::defaultModel() const
{
  return QStringLiteral("fake-small");
}

AI::ProviderCapabilities FakeProvider::capabilities() const
{
  AI::ProviderCapabilities caps;
  caps.parallelToolCalls = true;
  return caps;
}

/**
 * @brief Records what the conversation sent and returns a reply replaying the script.
 */
AI::Reply* FakeProvider::sendMessage(const QJsonArray& history,
                                     const QJsonArray& tools,
                                     bool forbidToolUse)
{
  ++m_sendCount;
  m_lastHistory       = history;
  m_lastTools         = tools;
  m_lastForbidToolUse = forbidToolUse;
  return new FakeReply(m_script);
}

}  // namespace Test
