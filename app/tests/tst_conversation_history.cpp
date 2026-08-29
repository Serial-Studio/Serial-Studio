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

#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTest>

#include "AI/Conversation/HistorySurgery.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief The history-maintenance passes AI::Conversation runs before every provider request.
 *
 * These are the transcript-integrity gates: the Anthropic-shaped APIs reject an entire request
 * when one tool_use lacks its tool_result or one tool_result names an undeclared tool_use, so a
 * defect here does not degrade the assistant, it takes it offline. The suite drives synthetic
 * histories through each pass, including the malformed orderings that interrupted tool batches,
 * cancelled turns and restored session files actually produce.
 */
class TstConversationHistory : public QObject {
  Q_OBJECT

private slots:
  void collectIds_ordersAndDedupes();
  void collectIds_skipsEmptyAndForeignBlocks();

  void precedingIds_boundaries();
  void precedingIds_happyPath();

  void keepValidUserContent_promotesStringToTextBlock();
  void keepValidUserContent_dropsUnknownAndDuplicateIds();

  void synthesize_onlyForMissingIds();
  void synthesize_marksPayloadUnresolved();

  void stripOrphans_removesUndeclaredResults();
  void stripOrphans_dropsMessageLeftEmpty();
  void stripOrphans_keepsWellFormedTurn();
  void stripOrphans_ignoresStringContent();

  void reconcile_synthesizesMissingResultInNextUserTurn();
  void reconcile_appendsUserTurnWhenNoneFollows();
  void reconcile_insertsBetweenTwoAssistantTurns();
  void reconcile_leavesToollessAssistantAlone();
  void reconcile_isIdempotent();
  void reconcile_pairsEveryToolUseAcrossMalformedHistory();

  void aging_keepsTwoMostRecentToolTurns();
  void aging_elidesOlderOversizedResults();
  void aging_respectsElideMinChars();
  void aging_neverElidesExemptTools_data();
  void aging_neverElidesExemptTools();
  void aging_replacesGeminiMirror();

  void firstFreshUserTurn_skipsToolResultTurns();
  void firstFreshUserTurn_honoursStartOffset();
  void firstFreshUserTurn_reportsMissing();

  void prune_isNoOpUnderCap();
  void prune_cutsAtFreshUserBoundary();
  void prune_refusesCutAtOrigin();
};

//--------------------------------------------------------------------------------------------------
// Block builders
//--------------------------------------------------------------------------------------------------

static QJsonObject textBlock(const QString& text)
{
  QJsonObject b;
  b[QStringLiteral("type")] = QStringLiteral("text");
  b[QStringLiteral("text")] = text;
  return b;
}

static QJsonObject toolUseBlock(const QString& id, const QString& name = QStringLiteral("io.get"))
{
  QJsonObject b;
  b[QStringLiteral("type")]  = QStringLiteral("tool_use");
  b[QStringLiteral("id")]    = id;
  b[QStringLiteral("name")]  = name;
  b[QStringLiteral("input")] = QJsonObject();
  return b;
}

static QJsonObject toolResultBlock(const QString& id,
                                   const QString& content  = QStringLiteral("ok"),
                                   const QString& toolName = QString())
{
  QJsonObject b;
  b[QStringLiteral("type")]        = QStringLiteral("tool_result");
  b[QStringLiteral("tool_use_id")] = id;
  b[QStringLiteral("content")]     = content;
  if (!toolName.isEmpty())
    b[QStringLiteral("_tool_name")] = toolName;

  return b;
}

static QJsonObject message(const QString& role, const QJsonArray& content)
{
  QJsonObject m;
  m[QStringLiteral("role")]    = role;
  m[QStringLiteral("content")] = content;
  return m;
}

static QJsonObject userText(const QString& text)
{
  return message(QStringLiteral("user"), QJsonArray{textBlock(text)});
}

static QJsonArray contentOf(const QJsonArray& history, int index)
{
  return history.at(index).toObject().value(QStringLiteral("content")).toArray();
}

static QString roleOf(const QJsonArray& history, int index)
{
  return history.at(index).toObject().value(QStringLiteral("role")).toString();
}

/**
 * @brief Returns the tool_use_id of every tool_result block in a message's content, in order.
 */
static QStringList resultIdsOf(const QJsonArray& content)
{
  QStringList ids;
  for (const auto& v : content) {
    const auto b = v.toObject();
    if (b.value(QStringLiteral("type")).toString() == QStringLiteral("tool_result"))
      ids.append(b.value(QStringLiteral("tool_use_id")).toString());
  }
  return ids;
}

/**
 * @brief Body long enough for the aging pass to consider it worth eliding.
 */
static QString longPayload()
{
  return QString(AI::HistorySurgery::kElideMinChars + 40, QLatin1Char('x'));
}

//--------------------------------------------------------------------------------------------------
// collectAssistantToolUseIds
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::collectIds_ordersAndDedupes()
{
  const QJsonArray content{toolUseBlock(QStringLiteral("a")),
                           toolUseBlock(QStringLiteral("b")),
                           toolUseBlock(QStringLiteral("a"))};

  QSet<QString> seen;
  const auto ordered = AI::HistorySurgery::collectAssistantToolUseIds(content, seen);

  QCOMPARE(ordered, QStringList({QStringLiteral("a"), QStringLiteral("b")}));
  QCOMPARE(seen.size(), 2);
}

void TstConversationHistory::collectIds_skipsEmptyAndForeignBlocks()
{
  const QJsonArray content{textBlock(QStringLiteral("hello")),
                           toolUseBlock(QString()),
                           toolResultBlock(QStringLiteral("z")),
                           toolUseBlock(QStringLiteral("real"))};

  QSet<QString> seen;
  const auto ordered = AI::HistorySurgery::collectAssistantToolUseIds(content, seen);

  QCOMPARE(ordered, QStringList({QStringLiteral("real")}));
  QVERIFY(!seen.contains(QString()));
  QVERIFY(!seen.contains(QStringLiteral("z")));
}

//--------------------------------------------------------------------------------------------------
// precedingAssistantToolUseIds
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::precedingIds_boundaries()
{
  const QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t1"))}),
    userText(QStringLiteral("hi"))};

  QVERIFY(AI::HistorySurgery::precedingAssistantToolUseIds(history, 0).isEmpty());
  QVERIFY(AI::HistorySurgery::precedingAssistantToolUseIds(history, -3).isEmpty());
  QVERIFY(AI::HistorySurgery::precedingAssistantToolUseIds(history, 99).isEmpty());

  const QJsonArray userFirst{userText(QStringLiteral("a")), userText(QStringLiteral("b"))};
  QVERIFY(AI::HistorySurgery::precedingAssistantToolUseIds(userFirst, 1).isEmpty());
}

void TstConversationHistory::precedingIds_happyPath()
{
  const QJsonArray history{
    userText(QStringLiteral("go")),
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("t1")), toolUseBlock(QStringLiteral("t2"))}),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("t1"))})};

  const auto ids = AI::HistorySurgery::precedingAssistantToolUseIds(history, 2);
  QCOMPARE(ids.size(), 2);
  QVERIFY(ids.contains(QStringLiteral("t1")));
  QVERIFY(ids.contains(QStringLiteral("t2")));
}

//--------------------------------------------------------------------------------------------------
// keepValidUserContent
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::keepValidUserContent_promotesStringToTextBlock()
{
  QSet<QString> seen;
  const auto kept = AI::HistorySurgery::keepValidUserContent(
    QJsonValue(QStringLiteral("plain")), QSet<QString>(), seen);

  QCOMPARE(kept.size(), 1);
  QCOMPARE(kept.at(0).toObject().value(QStringLiteral("type")).toString(), QStringLiteral("text"));
  QCOMPARE(kept.at(0).toObject().value(QStringLiteral("text")).toString(), QStringLiteral("plain"));
}

void TstConversationHistory::keepValidUserContent_dropsUnknownAndDuplicateIds()
{
  const QJsonArray content{textBlock(QStringLiteral("note")),
                           toolResultBlock(QStringLiteral("known")),
                           toolResultBlock(QStringLiteral("known")),
                           toolResultBlock(QStringLiteral("stranger"))};

  QSet<QString> assistantIds{QStringLiteral("known")};
  QSet<QString> seen;
  const auto kept = AI::HistorySurgery::keepValidUserContent(content, assistantIds, seen);

  QCOMPARE(kept.size(), 2);
  QCOMPARE(kept.at(0).toObject().value(QStringLiteral("type")).toString(), QStringLiteral("text"));
  QCOMPARE(resultIdsOf(kept), QStringList({QStringLiteral("known")}));
  QVERIFY(seen.contains(QStringLiteral("known")));
}

//--------------------------------------------------------------------------------------------------
// synthesizeMissingResults
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::synthesize_onlyForMissingIds()
{
  const QStringList ordered{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};
  QSet<QString> seen{QStringLiteral("b")};

  const auto out = AI::HistorySurgery::synthesizeMissingResults(ordered, seen);
  QCOMPARE(resultIdsOf(out), QStringList({QStringLiteral("a"), QStringLiteral("c")}));

  const QSet<QString> all{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")};
  QVERIFY(AI::HistorySurgery::synthesizeMissingResults(ordered, all).isEmpty());
}

void TstConversationHistory::synthesize_marksPayloadUnresolved()
{
  const auto out =
    AI::HistorySurgery::synthesizeMissingResults(QStringList{QStringLiteral("a")}, QSet<QString>());

  QCOMPARE(out.size(), 1);
  const auto block = out.at(0).toObject();
  QCOMPARE(block.value(QStringLiteral("type")).toString(), QStringLiteral("tool_result"));
  QVERIFY(block.value(QStringLiteral("content")).toString().contains(QStringLiteral("unresolved")));
}

//--------------------------------------------------------------------------------------------------
// stripOrphanToolResults
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::stripOrphans_removesUndeclaredResults()
{
  QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t1"))}),
    message(
      QStringLiteral("user"),
      QJsonArray{toolResultBlock(QStringLiteral("t1")), toolResultBlock(QStringLiteral("ghost"))})};

  AI::HistorySurgery::stripOrphanToolResults(history);

  QCOMPARE(history.size(), 2);
  QCOMPARE(resultIdsOf(contentOf(history, 1)), QStringList({QStringLiteral("t1")}));
}

void TstConversationHistory::stripOrphans_dropsMessageLeftEmpty()
{
  QJsonArray history{
    userText(QStringLiteral("first")),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("ghost"))}),
    userText(QStringLiteral("last"))};

  AI::HistorySurgery::stripOrphanToolResults(history);

  QCOMPARE(history.size(), 2);
  QCOMPARE(contentOf(history, 0).at(0).toObject().value(QStringLiteral("text")).toString(),
           QStringLiteral("first"));
  QCOMPARE(contentOf(history, 1).at(0).toObject().value(QStringLiteral("text")).toString(),
           QStringLiteral("last"));
}

void TstConversationHistory::stripOrphans_keepsWellFormedTurn()
{
  const QJsonArray original{
    userText(QStringLiteral("go")),
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("t1")), toolUseBlock(QStringLiteral("t2"))}),
    message(
      QStringLiteral("user"),
      QJsonArray{toolResultBlock(QStringLiteral("t1")), toolResultBlock(QStringLiteral("t2"))})};

  QJsonArray history = original;
  AI::HistorySurgery::stripOrphanToolResults(history);
  QCOMPARE(history, original);
}

void TstConversationHistory::stripOrphans_ignoresStringContent()
{
  QJsonObject stringUser;
  stringUser[QStringLiteral("role")]    = QStringLiteral("user");
  stringUser[QStringLiteral("content")] = QStringLiteral("legacy plain text");

  const QJsonArray original{stringUser};
  QJsonArray history = original;
  AI::HistorySurgery::stripOrphanToolResults(history);
  QCOMPARE(history, original);
}

//--------------------------------------------------------------------------------------------------
// reconcileHistoryToolPairs
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::reconcile_synthesizesMissingResultInNextUserTurn()
{
  QJsonArray history{
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("t1")), toolUseBlock(QStringLiteral("t2"))}),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("t1"))})};

  AI::HistorySurgery::reconcileHistoryToolPairs(history);

  QCOMPARE(history.size(), 2);
  QCOMPARE(resultIdsOf(contentOf(history, 1)),
           QStringList({QStringLiteral("t2"), QStringLiteral("t1")}));
}

void TstConversationHistory::reconcile_appendsUserTurnWhenNoneFollows()
{
  QJsonArray history{
    userText(QStringLiteral("go")),
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t1"))})};

  AI::HistorySurgery::reconcileHistoryToolPairs(history);

  QCOMPARE(history.size(), 3);
  QCOMPARE(roleOf(history, 2), QStringLiteral("user"));
  QCOMPARE(resultIdsOf(contentOf(history, 2)), QStringList({QStringLiteral("t1")}));
}

void TstConversationHistory::reconcile_insertsBetweenTwoAssistantTurns()
{
  QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t1"))}),
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t2"))})};

  AI::HistorySurgery::reconcileHistoryToolPairs(history);

  QCOMPARE(history.size(), 4);
  QCOMPARE(roleOf(history, 0), QStringLiteral("assistant"));
  QCOMPARE(roleOf(history, 1), QStringLiteral("user"));
  QCOMPARE(roleOf(history, 2), QStringLiteral("assistant"));
  QCOMPARE(roleOf(history, 3), QStringLiteral("user"));
  QCOMPARE(resultIdsOf(contentOf(history, 1)), QStringList({QStringLiteral("t1")}));
  QCOMPARE(resultIdsOf(contentOf(history, 3)), QStringList({QStringLiteral("t2")}));
}

void TstConversationHistory::reconcile_leavesToollessAssistantAlone()
{
  const QJsonArray original{
    userText(QStringLiteral("hi")),
    message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("hello"))})};

  QJsonArray history = original;
  AI::HistorySurgery::reconcileHistoryToolPairs(history);
  QCOMPARE(history, original);
}

/**
 * @brief The pass runs before every send, so a transcript it already repaired must survive a
 *        second run untouched; a non-idempotent repair would grow the history each turn.
 */
void TstConversationHistory::reconcile_isIdempotent()
{
  QJsonArray history{
    userText(QStringLiteral("go")),
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("t1")), toolUseBlock(QStringLiteral("t2"))}),
    message(QStringLiteral("user"), QJsonArray{toolResultBlock(QStringLiteral("t2"))})};

  AI::HistorySurgery::reconcileHistoryToolPairs(history);
  const QJsonArray once = history;
  AI::HistorySurgery::reconcileHistoryToolPairs(history);

  QCOMPARE(history, once);
}

/**
 * @brief End-to-end integrity property: after the pass, every assistant tool_use id is answered
 *        exactly once by the immediately following user turn, and no user turn carries a
 *        tool_result the preceding assistant never declared. This is the invariant the provider
 *        APIs enforce, checked over a history that mixes every malformation the suite covers.
 */
void TstConversationHistory::reconcile_pairsEveryToolUseAcrossMalformedHistory()
{
  QJsonArray history{
    userText(QStringLiteral("start")),
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("a1")), toolUseBlock(QStringLiteral("a2"))}),
    message(
      QStringLiteral("user"),
      QJsonArray{toolResultBlock(QStringLiteral("a2")), toolResultBlock(QStringLiteral("ghost"))}),
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("b1"))}),
    message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("no tools here"))}),
    userText(QStringLiteral("follow up")),
    message(QStringLiteral("assistant"),
            QJsonArray{toolUseBlock(QStringLiteral("c1")), toolUseBlock(QStringLiteral("c1"))})};

  AI::HistorySurgery::reconcileHistoryToolPairs(history);

  for (int i = 0; i < history.size(); ++i) {
    if (roleOf(history, i) != QStringLiteral("assistant"))
      continue;

    QSet<QString> declared;
    const auto ordered =
      AI::HistorySurgery::collectAssistantToolUseIds(contentOf(history, i), declared);
    if (ordered.isEmpty())
      continue;

    QVERIFY2(i + 1 < history.size(), "tool_use turn left without a following user turn");
    QCOMPARE(roleOf(history, i + 1), QStringLiteral("user"));

    auto answered = resultIdsOf(contentOf(history, i + 1));
    answered.sort();
    auto expected = ordered;
    expected.sort();
    QCOMPARE(answered, expected);
  }

  for (int i = 0; i < history.size(); ++i) {
    if (roleOf(history, i) != QStringLiteral("user"))
      continue;

    const auto declared = AI::HistorySurgery::precedingAssistantToolUseIds(history, i);
    for (const auto& id : resultIdsOf(contentOf(history, i)))
      QVERIFY2(declared.contains(id), "orphan tool_result survived reconciliation");
  }
}

//--------------------------------------------------------------------------------------------------
// ageHistoryToolResults
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds n user turns, each carrying one oversized tool_result, oldest first.
 */
static QJsonArray agingHistory(int turns, const QString& toolName = QString())
{
  QJsonArray history;
  for (int i = 0; i < turns; ++i) {
    const auto id = QStringLiteral("t%1").arg(i);
    history.append(message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(id)}));
    history.append(
      message(QStringLiteral("user"), QJsonArray{toolResultBlock(id, longPayload(), toolName)}));
  }
  return history;
}

static QString firstResultContent(const QJsonArray& history, int index)
{
  return contentOf(history, index).at(0).toObject().value(QStringLiteral("content")).toString();
}

void TstConversationHistory::aging_keepsTwoMostRecentToolTurns()
{
  QJsonArray history = agingHistory(3);
  AI::HistorySurgery::ageHistoryToolResults(history);

  QCOMPARE(firstResultContent(history, 3), longPayload());
  QCOMPARE(firstResultContent(history, 5), longPayload());
}

void TstConversationHistory::aging_elidesOlderOversizedResults()
{
  QJsonArray history = agingHistory(4);
  AI::HistorySurgery::ageHistoryToolResults(history);

  QVERIFY(firstResultContent(history, 1).contains(QStringLiteral("old result removed")));
  QVERIFY(firstResultContent(history, 3).contains(QStringLiteral("old result removed")));
  QCOMPARE(firstResultContent(history, 5), longPayload());
  QCOMPARE(firstResultContent(history, 7), longPayload());
}

void TstConversationHistory::aging_respectsElideMinChars()
{
  const QString shortBody(AI::HistorySurgery::kElideMinChars, QLatin1Char('y'));

  QJsonArray history                = agingHistory(3);
  auto oldest                       = history.at(1).toObject();
  oldest[QStringLiteral("content")] = QJsonArray{toolResultBlock(QStringLiteral("t0"), shortBody)};
  history[1]                        = oldest;

  AI::HistorySurgery::ageHistoryToolResults(history);
  QCOMPARE(firstResultContent(history, 1), shortBody);
}

void TstConversationHistory::aging_neverElidesExemptTools_data()
{
  QTest::addColumn<QString>("toolName");

  QTest::newRow("fs.read") << QStringLiteral("fs.read");
  QTest::newRow("fs.search") << QStringLiteral("fs.search");
  QTest::newRow("fs.list") << QStringLiteral("fs.list");
  QTest::newRow("meta.describeCommand") << QStringLiteral("meta.describeCommand");
  QTest::newRow("meta.listCommands") << QStringLiteral("meta.listCommands");
  QTest::newRow("meta.listCategories") << QStringLiteral("meta.listCategories");
  QTest::newRow("meta.searchDocs") << QStringLiteral("meta.searchDocs");
  QTest::newRow("meta.howTo") << QStringLiteral("meta.howTo");
  QTest::newRow("meta.search") << QStringLiteral("meta.search");
  QTest::newRow("project.search") << QStringLiteral("project.search");
  QTest::newRow("project.group.get") << QStringLiteral("project.group.get");
}

void TstConversationHistory::aging_neverElidesExemptTools()
{
  QFETCH(QString, toolName);

  QJsonArray history = agingHistory(4, toolName);
  AI::HistorySurgery::ageHistoryToolResults(history);

  QCOMPARE(firstResultContent(history, 1), longPayload());
  QCOMPARE(firstResultContent(history, 3), longPayload());
}

void TstConversationHistory::aging_replacesGeminiMirror()
{
  QJsonArray history = agingHistory(3);
  auto oldest        = history.at(1).toObject();
  auto block         = contentOf(history, 1).at(0).toObject();
  QJsonObject mirror;
  mirror[QStringLiteral("payload")]         = QStringLiteral("structured");
  block[QStringLiteral("_gemini_response")] = mirror;
  oldest[QStringLiteral("content")]         = QJsonArray{block};
  history[1]                                = oldest;

  AI::HistorySurgery::ageHistoryToolResults(history);

  const auto aged =
    contentOf(history, 1).at(0).toObject().value(QStringLiteral("_gemini_response")).toObject();
  QVERIFY(aged.contains(QStringLiteral("elided")));
  QVERIFY(!aged.contains(QStringLiteral("payload")));
}

//--------------------------------------------------------------------------------------------------
// firstFreshUserTurnAt
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::firstFreshUserTurn_skipsToolResultTurns()
{
  const QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{toolUseBlock(QStringLiteral("t1"))}),
    message(QStringLiteral("user"),
            QJsonArray{textBlock(QStringLiteral("mixed")), toolResultBlock(QStringLiteral("t1"))}),
    userText(QStringLiteral("fresh"))};

  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 0), 2);
  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, -5), 2);
}

void TstConversationHistory::firstFreshUserTurn_honoursStartOffset()
{
  const QJsonArray history{userText(QStringLiteral("one")),
                           userText(QStringLiteral("two")),
                           userText(QStringLiteral("three"))};

  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 0), 0);
  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 1), 1);
  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 2), 2);
  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 3), -1);
}

void TstConversationHistory::firstFreshUserTurn_reportsMissing()
{
  const QJsonArray history{
    message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("only me"))})};

  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(history, 0), -1);
  QCOMPARE(AI::HistorySurgery::firstFreshUserTurnAt(QJsonArray(), 0), -1);
}

//--------------------------------------------------------------------------------------------------
// pruneHistory
//--------------------------------------------------------------------------------------------------

void TstConversationHistory::prune_isNoOpUnderCap()
{
  const QJsonArray original{userText(QStringLiteral("a")), userText(QStringLiteral("b"))};

  QJsonArray history = original;
  QVERIFY(!AI::HistorySurgery::pruneHistory(history, 2));
  QCOMPARE(history, original);

  QVERIFY(!AI::HistorySurgery::pruneHistory(history, 10));
  QCOMPARE(history, original);
}

void TstConversationHistory::prune_cutsAtFreshUserBoundary()
{
  QJsonArray history;
  for (int i = 0; i < 6; ++i) {
    history.append(userText(QStringLiteral("ask %1").arg(i)));
    history.append(
      message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("reply"))}));
  }

  QVERIFY(AI::HistorySurgery::pruneHistory(history, 4));
  QCOMPARE(history.size(), 4);
  QCOMPARE(roleOf(history, 0), QStringLiteral("user"));
  QCOMPARE(contentOf(history, 0).at(0).toObject().value(QStringLiteral("text")).toString(),
           QStringLiteral("ask 4"));
}

/**
 * @brief A cut at index 0 would drop nothing while still rewriting the array, so the pass
 *        declines it: the caller relies on `false` meaning "history untouched".
 */
void TstConversationHistory::prune_refusesCutAtOrigin()
{
  const QJsonArray original{
    userText(QStringLiteral("only fresh turn")),
    message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("a"))}),
    message(QStringLiteral("assistant"), QJsonArray{textBlock(QStringLiteral("b"))})};

  QJsonArray history = original;
  QVERIFY(!AI::HistorySurgery::pruneHistory(history, 1));
  QCOMPARE(history, original);
}

QTEST_APPLESS_MAIN(TstConversationHistory)

#include "tst_conversation_history.moc"
