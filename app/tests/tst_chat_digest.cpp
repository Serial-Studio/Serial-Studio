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

#include <QtTest>

#include "AI/Conversation/ChatDigest.h"
#include "AI/Conversation/ToolCallStatus.h"

//--------------------------------------------------------------------------------------------------
// Chat transcript digests (spec 0070)
//--------------------------------------------------------------------------------------------------
//
// The handoff digest is what a new chat inherits when the user continues an old one, and it is
// built without a model call: three most recent user asks, the completed non-meta tool actions,
// and the tail of the last reply, secret-scrubbed and capped. The suite pins the three rules that
// silently ruin a handoff if they drift: the reverse scan must stop with the *latest* asks (not
// the earliest), meta.* and unfinished cards must never be advertised as actions, and the secret
// scrub must run before the cap so a truncation cannot slice a key in half and leak the prefix.
//
//--------------------------------------------------------------------------------------------------

class TstChatDigest : public QObject {
  Q_OBJECT

private slots:
  void firstUserTextPicksFirstUserRow();
  void firstUserTextIsEmptyWithoutUserRows();
  void handoffDigestIsEmptyForEmptyChat();
  void handoffDigestKeepsLastThreeAsksInOrder();
  void handoffDigestListsOnlyCompletedNonMetaActions();
  void handoffDigestScrubsSecretsBeforeCapping();
  void handoffDigestHonoursCharacterCap();
  void staleCardsDowngradeOnlyUnfinishedStatuses();

private:
  [[nodiscard]] static QVariantMap userRow(const QString& text);
  [[nodiscard]] static QVariantMap assistantRow(const QString& text, const QVariantList& calls);
  [[nodiscard]] static QVariantMap toolCard(const QString& name, AI::ToolCallStatus status);
};

//--------------------------------------------------------------------------------------------------
// Row builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a user transcript row.
 */
QVariantMap TstChatDigest::userRow(const QString& text)
{
  QVariantMap row;
  row[QStringLiteral("role")]      = QStringLiteral("user");
  row[QStringLiteral("text")]      = text;
  row[QStringLiteral("toolCalls")] = QVariantList();
  return row;
}

/**
 * @brief Builds an assistant transcript row carrying @a calls tool cards.
 */
QVariantMap TstChatDigest::assistantRow(const QString& text, const QVariantList& calls)
{
  QVariantMap row;
  row[QStringLiteral("role")]      = QStringLiteral("assistant");
  row[QStringLiteral("text")]      = text;
  row[QStringLiteral("toolCalls")] = calls;
  return row;
}

/**
 * @brief Builds one tool-call card in the given status.
 */
QVariantMap TstChatDigest::toolCard(const QString& name, AI::ToolCallStatus status)
{
  QVariantMap card;
  card[QStringLiteral("callId")] = name + QStringLiteral("-id");
  card[QStringLiteral("name")]   = name;
  card[QStringLiteral("status")] = static_cast<int>(status);
  return card;
}

//--------------------------------------------------------------------------------------------------
// Chat title
//--------------------------------------------------------------------------------------------------

/**
 * @brief The chat title comes from the earliest user row, not the latest.
 */
void TstChatDigest::firstUserTextPicksFirstUserRow()
{
  const QVariantList rows{assistantRow(QStringLiteral("preamble"), {}),
                          userRow(QStringLiteral("first ask")),
                          userRow(QStringLiteral("second ask"))};

  QCOMPARE(AI::ChatDigest::firstUserText(rows), QStringLiteral("first ask"));
}

/**
 * @brief A transcript with no user row yields no title rather than a placeholder.
 */
void TstChatDigest::firstUserTextIsEmptyWithoutUserRows()
{
  const QVariantList rows{assistantRow(QStringLiteral("only me"), {})};

  QVERIFY(AI::ChatDigest::firstUserText(rows).isEmpty());
  QVERIFY(AI::ChatDigest::firstUserText(QVariantList()).isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Handoff digest
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty transcript produces no digest at all, not an "Asked:" header.
 */
void TstChatDigest::handoffDigestIsEmptyForEmptyChat()
{
  QVERIFY(AI::ChatDigest::buildHandoffDigest(QVariantList(), 600).isEmpty());
}

/**
 * @brief The reverse scan keeps the three most recent asks and re-orders them
 *        chronologically; the oldest ask falls out.
 */
void TstChatDigest::handoffDigestKeepsLastThreeAsksInOrder()
{
  const QVariantList rows{userRow(QStringLiteral("ask one")),
                          userRow(QStringLiteral("ask two")),
                          userRow(QStringLiteral("ask three")),
                          userRow(QStringLiteral("ask four")),
                          assistantRow(QStringLiteral("the reply"), {})};

  const auto digest = AI::ChatDigest::buildHandoffDigest(rows, 600);
  QVERIFY(digest.contains(QStringLiteral("Asked: ask two | ask three | ask four")));
  QVERIFY(!digest.contains(QStringLiteral("ask one")));
  QVERIFY(digest.contains(QStringLiteral("Last reply: the reply")));
}

/**
 * @brief Only Done, non-meta cards become advertised actions: a discovery call or a call
 *        that errored must never read as work the previous chat completed.
 */
void TstChatDigest::handoffDigestListsOnlyCompletedNonMetaActions()
{
  const QVariantList calls{
    toolCard(QStringLiteral("project.dataset.add"), AI::ToolCallStatus::Done),
    toolCard(QStringLiteral("meta.snapshot"), AI::ToolCallStatus::Done),
    toolCard(QStringLiteral("io.connect"), AI::ToolCallStatus::Error)};
  const QVariantList rows{assistantRow(QString(), calls), userRow(QStringLiteral("do it"))};

  const auto digest = AI::ChatDigest::buildHandoffDigest(rows, 600);
  QVERIFY(digest.contains(QStringLiteral("Actions: project.dataset.add")));
  QVERIFY(!digest.contains(QStringLiteral("meta.snapshot")));
  QVERIFY(!digest.contains(QStringLiteral("io.connect")));
}

/**
 * @brief Secrets are redacted before the cap is applied, so a truncation can never publish
 *        the surviving prefix of a key.
 */
void TstChatDigest::handoffDigestScrubsSecretsBeforeCapping()
{
  const QVariantList rows{userRow(QStringLiteral("my key is AKIAIOSFODNN7EXAMPLE ok"))};

  const auto digest = AI::ChatDigest::buildHandoffDigest(rows, 600);
  QVERIFY(!digest.contains(QStringLiteral("AKIAIOSFODNN7EXAMPLE")));
  QVERIFY(digest.contains(QStringLiteral("[REDACTED:aws_access_key_id]")));
}

/**
 * @brief The digest never exceeds the caller's character cap.
 */
void TstChatDigest::handoffDigestHonoursCharacterCap()
{
  const QVariantList rows{userRow(QString(300, QLatin1Char('a'))),
                          assistantRow(QString(300, QLatin1Char('b')), {})};

  QCOMPARE(static_cast<int>(AI::ChatDigest::buildHandoffDigest(rows, 32).size()), 32);
}

//--------------------------------------------------------------------------------------------------
// Restored transcripts
//--------------------------------------------------------------------------------------------------

/**
 * @brief A chat closed mid-turn restores with no live spinners: Running and AwaitingConfirm
 *        settle to Done, while terminal statuses keep their own verdict.
 */
void TstChatDigest::staleCardsDowngradeOnlyUnfinishedStatuses()
{
  const QVariantList calls{toolCard(QStringLiteral("a.run"), AI::ToolCallStatus::Running),
                           toolCard(QStringLiteral("b.ask"), AI::ToolCallStatus::AwaitingConfirm),
                           toolCard(QStringLiteral("c.bad"), AI::ToolCallStatus::Error),
                           toolCard(QStringLiteral("d.no"), AI::ToolCallStatus::Denied)};
  QVariantList rows{userRow(QStringLiteral("go")), assistantRow(QString(), calls)};

  AI::ChatDigest::downgradeStaleToolCards(rows);

  const auto restored = rows.at(1).toMap().value(QStringLiteral("toolCalls")).toList();
  QCOMPARE(static_cast<int>(restored.size()), 4);
  QCOMPARE(restored.at(0).toMap().value(QStringLiteral("status")).toInt(),
           static_cast<int>(AI::ToolCallStatus::Done));
  QCOMPARE(restored.at(1).toMap().value(QStringLiteral("status")).toInt(),
           static_cast<int>(AI::ToolCallStatus::Done));
  QCOMPARE(restored.at(2).toMap().value(QStringLiteral("status")).toInt(),
           static_cast<int>(AI::ToolCallStatus::Error));
  QCOMPARE(restored.at(3).toMap().value(QStringLiteral("status")).toInt(),
           static_cast<int>(AI::ToolCallStatus::Denied));
  QCOMPARE(rows.at(0).toMap().value(QStringLiteral("role")).toString(), QStringLiteral("user"));
}

QTEST_APPLESS_MAIN(TstChatDigest)

#include "tst_chat_digest.moc"
