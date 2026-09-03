/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

#include "AI/Conversation/TokenBudget.h"
#include "AI/Providers/Provider.h"
#include "support/FakeProvider.h"

// The turn loop itself links the whole application, so what is pinned here is the rule the loop
// applies: how a provider window becomes a history budget, and the scripted-reply contract the
// loop consumes. Every test function is self-contained.

/**
 * @brief Pins the context-window budgeting a turn is built with. A local 8k model used to
 *        reserve more than its whole window, the budget went negative, the budgeter's "do not
 *        trim" fallback sent everything, and the server truncated the system prompt away with
 *        the trust boundary and the hardware-write rules in it (spec 0075, J1).
 */
class TstConversationTurn : public QObject {
  Q_OBJECT

private slots:
  void largeWindowKeepsTheFullReserve();
  void smallWindowCapsBothReservations();
  void smallWindowYieldsAPositiveHistoryBudget();
  void smallWindowActuallyTrimsHistory();
  void scriptedProviderDeliversItsEventsInOrder();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/// System-prompt reservation the conversation asks for, mirroring
/// Conversation::kSystemReserveTokens.
static constexpr int kSystemReserveTokens = 28000;

/**
 * @brief Builds the window a conversation derives from a provider's capabilities.
 */
static AI::TokenBudget::Window windowFor(const AI::ProviderCapabilities& caps)
{
  return AI::TokenBudget::Window{caps.contextWindowTokens,
                                 caps.budgetedOutputTokens(),
                                 caps.budgetedSystemReserve(kSystemReserveTokens)};
}

/**
 * @brief Builds a history of alternating user/assistant turns, each carrying a text block, so a
 *        trim has real fresh-user-turn boundaries to cut at.
 */
static QJsonArray syntheticHistory(int turns, int charsPerTurn)
{
  QJsonArray history;
  for (int i = 0; i < turns; ++i) {
    QJsonObject block;
    block.insert(QStringLiteral("type"), QStringLiteral("text"));
    block.insert(QStringLiteral("text"), QString(charsPerTurn, QLatin1Char('x')));

    QJsonObject message;
    message.insert(QStringLiteral("role"),
                   (i % 2) == 0 ? QStringLiteral("user") : QStringLiteral("assistant"));
    message.insert(QStringLiteral("content"), QJsonArray{block});
    history.append(message);
  }

  return history;
}

//--------------------------------------------------------------------------------------------------
// Window derivation
//--------------------------------------------------------------------------------------------------

/**
 * @brief A cloud-sized window keeps the declared output and system reservations untouched.
 */
void TstConversationTurn::largeWindowKeepsTheFullReserve()
{
  AI::ProviderCapabilities caps;
  caps.contextWindowTokens = 200000;
  caps.maxOutputTokens     = 8192;

  QCOMPARE(caps.budgetedOutputTokens(), 8192);
  QCOMPARE(caps.budgetedSystemReserve(kSystemReserveTokens), kSystemReserveTokens);
}

/**
 * @brief An 8k window caps both reservations at a quarter of itself.
 */
void TstConversationTurn::smallWindowCapsBothReservations()
{
  AI::ProviderCapabilities caps;
  caps.contextWindowTokens = 8192;
  caps.maxOutputTokens     = 8192;

  QCOMPARE(caps.budgetedOutputTokens(), 2048);
  QCOMPARE(caps.budgetedSystemReserve(kSystemReserveTokens), 2048);
}

/**
 * @brief The capped window leaves room for history, which is what makes trimming possible; the
 *        uncapped arithmetic is negative, and a non-positive budget means "send everything".
 */
void TstConversationTurn::smallWindowYieldsAPositiveHistoryBudget()
{
  AI::ProviderCapabilities caps;
  caps.contextWindowTokens = 8192;
  caps.maxOutputTokens     = 8192;

  const AI::TokenBudget::Window uncapped{
    caps.contextWindowTokens, caps.maxOutputTokens, kSystemReserveTokens};
  QVERIFY(AI::TokenBudget::historyBudget(uncapped, QJsonArray()) <= 0);
  QVERIFY(AI::TokenBudget::historyBudget(windowFor(caps), QJsonArray()) > 0);
}

/**
 * @brief With the capped window, a history larger than the model can hold is cut down instead of
 *        being handed whole to a server that truncates from the front.
 */
void TstConversationTurn::smallWindowActuallyTrimsHistory()
{
  AI::ProviderCapabilities caps;
  caps.contextWindowTokens = 8192;
  caps.maxOutputTokens     = 8192;

  const auto history = syntheticHistory(40, 2048);
  const int budget   = AI::TokenBudget::historyBudget(windowFor(caps), QJsonArray());
  const auto trimmed = AI::TokenBudget::budgetedHistory(history, budget);

  QVERIFY(trimmed.size() < history.size());
  QVERIFY(AI::TokenBudget::estimateTokens(trimmed) < AI::TokenBudget::estimateTokens(history));

  const AI::TokenBudget::Window uncapped{
    caps.contextWindowTokens, caps.maxOutputTokens, kSystemReserveTokens};
  const auto untrimmed = AI::TokenBudget::budgetedHistory(
    history, AI::TokenBudget::historyBudget(uncapped, QJsonArray()));
  QCOMPARE(untrimmed.size(), history.size());
}

//--------------------------------------------------------------------------------------------------
// Scripted provider contract
//--------------------------------------------------------------------------------------------------

/**
 * @brief The scripted double the turn-loop tests drive publishes text, then the tool call, then
 *        completion, in the order the loop depends on.
 */
void TstConversationTurn::scriptedProviderDeliversItsEventsInOrder()
{
  Test::FakeProvider provider;
  provider.script({
    {    Test::ReplyEvent::Text,QStringLiteral("thinking out loud"),{},                                                             {}                                                                         },
    {Test::ReplyEvent::ToolCall,
     {},
     QStringLiteral("fs.read"),
     QJsonObject{{QStringLiteral("path"), QStringLiteral("notes.txt")}}                                                                     },
    {    Test::ReplyEvent::Done,                                  {}, {},                                                                 {}},
  });

  auto* reply = provider.sendMessage(QJsonArray(), QJsonArray());
  QVERIFY(reply != nullptr);

  QSignalSpy text(reply, &AI::Reply::partialText);
  QSignalSpy tools(reply, &AI::Reply::toolCallRequested);
  QSignalSpy finished(reply, &AI::Reply::finished);

  QVERIFY(finished.count() > 0 || finished.wait(5000));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(text.count(), 1);
  QCOMPARE(text.at(0).at(0).toString(), QStringLiteral("thinking out loud"));
  QCOMPARE(tools.count(), 1);
  QCOMPARE(tools.at(0).at(1).toString(), QStringLiteral("fs.read"));
}

QTEST_GUILESS_MAIN(TstConversationTurn)

#include "tst_conversation_turn.moc"
