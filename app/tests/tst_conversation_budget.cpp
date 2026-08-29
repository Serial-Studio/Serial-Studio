/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <initializer_list>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QTest>

#include "AI/Conversation/HistorySurgery.h"
#include "AI/Conversation/TokenBudget.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief The context-window budgeter that decides how much transcript each request carries.
 *
 * Trimming is only safe at a fresh user turn: cut anywhere else and a tool_use loses its
 * tool_result, which the provider rejects outright. These cases pin the arithmetic, the
 * "never trim when the window is unknown" escapes, and the boundary rule itself.
 */
class TstConversationBudget : public QObject {
  Q_OBJECT

private slots:
  void estimate_isZeroForEmpty();
  void estimate_isMonotonicInContent();
  void estimate_scalesWithPayloadSize();

  void historyBudget_subtractsEveryReservation();
  void historyBudget_goesNonPositiveOnATinyWindow();

  void budgeted_returnsHistoryOnNonPositiveBudget();
  void budgeted_returnsHistoryWhenEverythingFits();
  void budgeted_keepsNewestTurnOnAMinimalBudget();
  void budgeted_alwaysCutsAtAFreshUserTurn();
  void budgeted_isMonotonicInBudget();
  void budgeted_keepsWholeHistoryWithoutAnyBoundary();
};

//--------------------------------------------------------------------------------------------------
// Fixtures
//--------------------------------------------------------------------------------------------------

static QJsonObject textBlock(const QString& text)
{
  QJsonObject b;
  b[QStringLiteral("type")] = QStringLiteral("text");
  b[QStringLiteral("text")] = text;
  return b;
}

static QJsonObject message(const QString& role, const QJsonArray& content)
{
  QJsonObject m;
  m[QStringLiteral("role")]    = role;
  m[QStringLiteral("content")] = content;
  return m;
}

static QJsonObject toolUseBlock(const QString& id)
{
  QJsonObject b;
  b[QStringLiteral("type")]  = QStringLiteral("tool_use");
  b[QStringLiteral("id")]    = id;
  b[QStringLiteral("name")]  = QStringLiteral("io.getStatus");
  b[QStringLiteral("input")] = QJsonObject();
  return b;
}

static QJsonObject toolResultBlock(const QString& id, const QString& body)
{
  QJsonObject b;
  b[QStringLiteral("type")]        = QStringLiteral("tool_result");
  b[QStringLiteral("tool_use_id")] = id;
  b[QStringLiteral("content")]     = body;
  return b;
}

/**
 * @brief Builds @p turns complete exchanges, each a fresh user ask followed by an assistant
 *        tool call and its result, so every turn boundary is a legitimate cut point and
 *        every non-boundary index sits inside a tool pair.
 */
static QJsonArray transcript(int turns, int padding = 200)
{
  const QString filler(padding, QLatin1Char('x'));

  QJsonArray history;
  for (int i = 0; i < turns; ++i) {
    const auto id = QStringLiteral("call%1").arg(i);
    history.append(message(QStringLiteral("user"),
                           QJsonArray{textBlock(QStringLiteral("ask %1 %2").arg(i).arg(filler))}));
    history.append(message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(id)}));
    history.append(message(QStringLiteral("user"), QJsonArray{toolResultBlock(id, filler)}));
    history.append(
      message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("done"))}));
  }
  return history;
}

/**
 * @brief True when @p candidate is a trailing sub-array of @p full.
 */
static bool isSuffixOf(const QJsonArray& candidate, const QJsonArray& full)
{
  if (candidate.size() > full.size())
    return false;

  const auto offset = full.size() - candidate.size();
  for (int i = 0; i < candidate.size(); ++i)
    if (candidate.at(i) != full.at(offset + i))
      return false;

  return true;
}

//--------------------------------------------------------------------------------------------------
// estimateTokens
//--------------------------------------------------------------------------------------------------

void TstConversationBudget::estimate_isZeroForEmpty()
{
  QCOMPARE(AI::TokenBudget::estimateTokens(QJsonArray()), 0);
}

void TstConversationBudget::estimate_isMonotonicInContent()
{
  QJsonArray blocks;
  int previous = AI::TokenBudget::estimateTokens(blocks);
  for (int i = 0; i < 12; ++i) {
    blocks.append(textBlock(QStringLiteral("chunk %1 of filler text").arg(i)));
    const int current = AI::TokenBudget::estimateTokens(blocks);
    QVERIFY2(current >= previous, "adding a block lowered the token estimate");
    previous = current;
  }
  QVERIFY(previous > 0);
}

void TstConversationBudget::estimate_scalesWithPayloadSize()
{
  const QJsonArray small{textBlock(QString(100, QLatin1Char('a')))};
  const QJsonArray large{textBlock(QString(4000, QLatin1Char('a')))};

  QVERIFY(AI::TokenBudget::estimateTokens(large) > AI::TokenBudget::estimateTokens(small));
  QVERIFY(AI::TokenBudget::estimateTokens(large) >= 4000 / AI::TokenBudget::kBytesPerToken);
}

//--------------------------------------------------------------------------------------------------
// historyBudget
//--------------------------------------------------------------------------------------------------

void TstConversationBudget::historyBudget_subtractsEveryReservation()
{
  const AI::TokenBudget::Window window{200000, 8000, 28000};
  const QJsonArray tools{textBlock(QString(4000, QLatin1Char('t')))};

  const int toolTokens = AI::TokenBudget::estimateTokens(tools);
  QCOMPARE(AI::TokenBudget::historyBudget(window, tools), 200000 - 8000 - 28000 - toolTokens);
  QCOMPARE(AI::TokenBudget::historyBudget(window, QJsonArray()),
           200000 - 8000 - 28000 - AI::TokenBudget::estimateTokens(QJsonArray()));
}

void TstConversationBudget::historyBudget_goesNonPositiveOnATinyWindow()
{
  const AI::TokenBudget::Window window{8000, 4000, 28000};
  QVERIFY(AI::TokenBudget::historyBudget(window, QJsonArray()) <= 0);
}

//--------------------------------------------------------------------------------------------------
// budgetedHistory
//--------------------------------------------------------------------------------------------------

/**
 * @brief A window too small to model is not a licence to ship a corrupt transcript: the
 *        budgeter hands the history back whole and lets the provider reject it.
 */
void TstConversationBudget::budgeted_returnsHistoryOnNonPositiveBudget()
{
  const auto history = transcript(4);

  QCOMPARE(AI::TokenBudget::budgetedHistory(history, 0), history);
  QCOMPARE(AI::TokenBudget::budgetedHistory(history, -5000), history);
}

void TstConversationBudget::budgeted_returnsHistoryWhenEverythingFits()
{
  const auto history = transcript(4);
  QCOMPARE(AI::TokenBudget::budgetedHistory(history, 1000000), history);
}

void TstConversationBudget::budgeted_keepsNewestTurnOnAMinimalBudget()
{
  const auto history = transcript(5);

  int lastFresh = -1;
  for (int at = AI::HistorySurgery::firstFreshUserTurnAt(history, 0); at >= 0;
       at     = AI::HistorySurgery::firstFreshUserTurnAt(history, at + 1))
    lastFresh = at;

  QVERIFY(lastFresh > 0);

  const auto trimmed = AI::TokenBudget::budgetedHistory(history, 1);
  QCOMPARE(trimmed.size(), history.size() - lastFresh);
  QVERIFY(isSuffixOf(trimmed, history));
}

/**
 * @brief The integrity property the whole budgeter exists for: whatever it returns starts on
 *        a fresh user turn, so no tool_result is ever shipped without its tool_use.
 */
void TstConversationBudget::budgeted_alwaysCutsAtAFreshUserTurn()
{
  const auto history = transcript(6);

  for (const int budget : {1, 50, 200, 800, 2000, 20000}) {
    const auto trimmed = AI::TokenBudget::budgetedHistory(history, budget);
    QVERIFY(isSuffixOf(trimmed, history));

    if (trimmed.size() == history.size())
      continue;

    QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(trimmed, 0), 0);
  }
}

void TstConversationBudget::budgeted_isMonotonicInBudget()
{
  const auto history = transcript(6);

  qsizetype previous = 0;
  for (const int budget : {1, 100, 400, 1600, 6400, 25600, 1000000}) {
    const auto trimmed = AI::TokenBudget::budgetedHistory(history, budget);
    QVERIFY2(trimmed.size() >= previous, "a larger budget returned less history");
    previous = trimmed.size();
  }
  QCOMPARE(previous, history.size());
}

/**
 * @brief With no fresh user turn to cut at there is no safe boundary, so the budgeter keeps
 *        the history whole rather than slicing a tool pair in half.
 */
void TstConversationBudget::budgeted_keepsWholeHistoryWithoutAnyBoundary()
{
  const QString filler(400, QLatin1Char('z'));
  const QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("c0"))}),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("c0"), filler)}),
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("c1"))}),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("c1"), filler)})};

  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 0), -1);
  QCOMPARE(AI::TokenBudget::budgetedHistory(history, 1), history);
}

QTEST_APPLESS_MAIN(TstConversationBudget)

#include "tst_conversation_budget.moc"
