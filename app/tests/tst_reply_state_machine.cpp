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

#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUrl>

#include "AI/KeyVault.h"
#include "AI/Providers/AnthropicReply.h"
#include "AI/Providers/GeminiReply.h"
#include "AI/Providers/OpenAIReply.h"
#include "AI/Providers/Provider.h"
#include "support/FakeTransport.h"

// Every test function here is self-contained: each builds its own transport and reply, so Qt
// Test's declaration-order execution is never load-bearing.

/**
 * @brief Drives the three streaming reply state machines against canned HTTP responses: one
 *        finished per reply on every path, HTTP failures classified as transient or not, the
 *        shared parse-error policy, and the transport rules that keep a key off the wire in the
 *        clear (spec 0075, J5/J6/J8).
 */
class TstReplyStateMachine : public QObject {
  Q_OBJECT

private slots:
  void anthropicStreamsTextAndFinishesOnce();
  void anthropicHttp401IsNotTransient();
  void openAiHttp429IsTransient();
  void geminiPromptBlockEndsTheTurn();
  void recoverableParseErrorDoesNotEndTheTurn();
  void fatalParseErrorEndsTheTurn();
  void plainHttpToRemoteHostIsRefused();
  void plainHttpToLoopbackIsAllowed();
  void httpsIsAllowedAndCredentialsNeverLogged();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Waits for a reply to finish, so a case asserts on a settled state machine.
 */
static bool waitForFinished(QSignalSpy& finished)
{
  return finished.count() > 0 || finished.wait(5000);
}

/**
 * @brief Builds one Anthropic SSE body out of the frames a real stream carries.
 */
static QByteArray anthropicStream(const QByteArray& text)
{
  QByteArray out;
  out.append("event: message_start\ndata: {\"message\":{\"usage\":{}}}\n\n");
  out.append("event: content_block_start\ndata: {\"index\":0,\"content_block\":"
             "{\"type\":\"text\"}}\n\n");
  out.append("event: content_block_delta\ndata: {\"index\":0,\"delta\":{\"type\":\"text_delta\","
             "\"text\":\"");
  out.append(text);
  out.append("\"}}\n\n");
  out.append("event: content_block_stop\ndata: {\"index\":0}\n\n");
  out.append("event: message_stop\ndata: {}\n\n");
  return out;
}

//--------------------------------------------------------------------------------------------------
// Happy path
//--------------------------------------------------------------------------------------------------

/**
 * @brief A well-formed stream publishes its text and completes exactly once.
 */
void TstReplyStateMachine::anthropicStreamsTextAndFinishesOnce()
{
  Test::FakeTransport transport;
  transport.enqueue(QStringLiteral("api.anthropic.com"), 200, anthropicStream("hello"));

  AI::AnthropicReply reply(transport, QStringLiteral("sk-test-key"), QByteArrayLiteral("{}"));
  QSignalSpy text(&reply, &AI::Reply::partialText);
  QSignalSpy errors(&reply, &AI::Reply::errorOccurred);
  QSignalSpy finished(&reply, &AI::Reply::finished);

  QVERIFY(waitForFinished(finished));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(errors.count(), 0);
  QCOMPARE(text.count(), 1);
  QCOMPARE(text.at(0).at(0).toString(), QStringLiteral("hello"));
  QVERIFY(!reply.transientError());
}

//--------------------------------------------------------------------------------------------------
// HTTP failures
//--------------------------------------------------------------------------------------------------

/**
 * @brief A rejected key is a permanent failure: retrying it would burn the turn budget.
 */
void TstReplyStateMachine::anthropicHttp401IsNotTransient()
{
  Test::FakeTransport transport;
  transport.enqueue(QStringLiteral("api.anthropic.com"),
                    401,
                    QByteArrayLiteral("{\"error\":{\"message\":\"invalid key\"}}"));

  AI::AnthropicReply reply(transport, QStringLiteral("sk-test-key"), QByteArrayLiteral("{}"));
  QSignalSpy errors(&reply, &AI::Reply::errorOccurred);
  QSignalSpy finished(&reply, &AI::Reply::finished);

  QVERIFY(waitForFinished(finished));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(errors.count(), 1);
  QVERIFY(!reply.transientError());
}

/**
 * @brief Rate limiting is transient, which is what lets the conversation retry the same turn.
 */
void TstReplyStateMachine::openAiHttp429IsTransient()
{
  Test::FakeTransport transport;
  transport.enqueue(QStringLiteral("api.openai.com"),
                    429,
                    QByteArrayLiteral("{\"error\":{\"message\":\"slow down\"}}"));

  AI::OpenAIReply reply(transport, QStringLiteral("sk-test-key"), QByteArrayLiteral("{}"));
  QSignalSpy errors(&reply, &AI::Reply::errorOccurred);
  QSignalSpy finished(&reply, &AI::Reply::finished);

  QVERIFY(waitForFinished(finished));
  QCOMPARE(finished.count(), 1);
  QCOMPARE(errors.count(), 1);
  QVERIFY(reply.transientError());
}

/**
 * @brief A safety block is reported as an error rather than as an empty successful reply.
 */
void TstReplyStateMachine::geminiPromptBlockEndsTheTurn()
{
  Test::FakeTransport transport;
  transport.enqueue(
    QStringLiteral("generativelanguage"),
    200,
    QByteArrayLiteral("data: {\"promptFeedback\":{\"blockReason\":\"SAFETY\"}}\n\n"));

  AI::GeminiReply reply(transport,
                        QUrl(QStringLiteral("https://generativelanguage.googleapis.com/v1/x")),
                        QStringLiteral("key"),
                        QByteArrayLiteral("{}"));
  QSignalSpy errors(&reply, &AI::Reply::errorOccurred);
  QSignalSpy finished(&reply, &AI::Reply::finished);

  QVERIFY(waitForFinished(finished));
  QCOMPARE(errors.count(), 1);
  QCOMPARE(finished.count(), 1);
}

//--------------------------------------------------------------------------------------------------
// Parse-error policy (one rule for every backend)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One malformed frame is skipped: a provider hiccup must not throw the turn away.
 */
void TstReplyStateMachine::recoverableParseErrorDoesNotEndTheTurn()
{
  QVERIFY(!AI::Reply::endsTurnOnParseError(QStringLiteral("illegal value")));
}

/**
 * @brief Losing the stream buffer is unrecoverable: the reply cannot claim a complete answer.
 */
void TstReplyStateMachine::fatalParseErrorEndsTheTurn()
{
  QVERIFY(AI::Reply::endsTurnOnParseError(QStringLiteral("buffer_overflow")));
  QVERIFY(AI::Reply::endsTurnOnParseError(QStringLiteral("payload_too_large")));
}

//--------------------------------------------------------------------------------------------------
// Transport policy
//--------------------------------------------------------------------------------------------------

/**
 * @brief A cleartext endpoint on another host never receives the key.
 */
void TstReplyStateMachine::plainHttpToRemoteHostIsRefused()
{
  QVERIFY(!AI::Reply::isTransportAllowed(QUrl(QStringLiteral("http://example.com/v1"))));
  QVERIFY(!AI::Reply::isTransportAllowed(QUrl(QStringLiteral("http://192.168.1.10:11434/v1"))));
  QVERIFY(!AI::Reply::isTransportAllowed(QUrl(QStringLiteral("ftp://example.com"))));
  QVERIFY(!AI::Reply::isTransportAllowed(QUrl(QStringLiteral("not a url"))));

  Test::FakeTransport transport;
  AI::OpenAIReply reply(transport,
                        QStringLiteral("http://example.com/v1/chat/completions"),
                        QStringLiteral("Authorization"),
                        QStringLiteral("sk-test-key"),
                        QByteArrayLiteral("{}"),
                        QStringLiteral("Local"));
  QSignalSpy errors(&reply, &AI::Reply::errorOccurred);
  QSignalSpy finished(&reply, &AI::Reply::finished);

  QVERIFY(waitForFinished(finished));
  QCOMPARE(errors.count(), 1);
  QCOMPARE(finished.count(), 1);
}

/**
 * @brief A local model server on this machine stays reachable over plain http.
 */
void TstReplyStateMachine::plainHttpToLoopbackIsAllowed()
{
  QVERIFY(AI::Reply::isTransportAllowed(QUrl(QStringLiteral("http://localhost:11434/v1"))));
  QVERIFY(AI::Reply::isTransportAllowed(QUrl(QStringLiteral("http://127.0.0.1:11434/v1"))));
  QVERIFY(AI::Reply::isTransportAllowed(QUrl(QStringLiteral("http://[::1]:11434/v1"))));
}

/**
 * @brief https is allowed anywhere, and no part of a key is printable through the redactor.
 */
void TstReplyStateMachine::httpsIsAllowedAndCredentialsNeverLogged()
{
  QVERIFY(AI::Reply::isTransportAllowed(QUrl(QStringLiteral("https://api.openai.com/v1"))));

  const auto key      = QStringLiteral("sk-ant-api03-ABCDEFGHIJKLMNOP");
  const auto redacted = AI::KeyVault::redact(key);
  QCOMPARE(redacted, QStringLiteral("***"));
  QVERIFY(!redacted.contains(key.left(4)));
  QVERIFY(!redacted.contains(key.right(4)));
}

QTEST_GUILESS_MAIN(TstReplyStateMachine)

#include "tst_reply_state_machine.moc"
