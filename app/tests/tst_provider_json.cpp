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
#include <QTest>

#include "AI/Providers/ProviderJson.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Covers the wire-shaping helpers every OpenAI-compatible provider shares: system-block
 *        flattening, history translation (tool calls included), tool schemas, the request body
 *        and the HTTP error mapping.
 */
class TstProviderJson : public QObject {
  Q_OBJECT

private slots:
  void sanitizeToolName_data();
  void sanitizeToolName();

  void flattenSystemBlocks();
  void flattenSystemBlocksSkipsEmpty();

  void translateHistoryPlainText();
  void translateHistoryDeveloperRole();
  void translateHistoryToolCallRoundTrip();
  void translateHistoryBackfillsDanglingToolCall();

  void translateTools();
  void chatCompletionsBody();
  void chatCompletionsBodyWithoutTools();

  void errorMessageFromBody_data();
  void errorMessageFromBody();

  void isTransientHttpStatus_data();
  void isTransientHttpStatus();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds a `{type, ...}` content block from key/value pairs.
 */
static QJsonObject block(const QString& type, const QJsonObject& extra)
{
  QJsonObject out = extra;
  out.insert(QStringLiteral("type"), type);
  return out;
}

/**
 * @brief Builds a `{role, content}` history message with array content.
 */
static QJsonObject message(const QString& role, const QJsonArray& content)
{
  QJsonObject out;
  out.insert(QStringLiteral("role"), role);
  out.insert(QStringLiteral("content"), content);
  return out;
}

//--------------------------------------------------------------------------------------------------
// sanitizeToolName
//--------------------------------------------------------------------------------------------------

void TstProviderJson::sanitizeToolName_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("expected");

  QTest::newRow("plain name") << QStringLiteral("read_file") << QStringLiteral("read_file");
  QTest::newRow("dotted") << QStringLiteral("project.read") << QStringLiteral("project_read");
  QTest::newRow("colon") << QStringLiteral("ui:tile") << QStringLiteral("ui_tile");
  QTest::newRow("both") << QStringLiteral("a.b:c.d") << QStringLiteral("a_b_c_d");
  QTest::newRow("empty") << QString() << QString();
}

/**
 * @brief Dots and colons are the only characters the vendors reject that the catalog produces.
 */
void TstProviderJson::sanitizeToolName()
{
  QFETCH(QString, input);
  QFETCH(QString, expected);
  QCOMPARE(AI::ProviderJson::sanitizeToolName(input), expected);
}

//--------------------------------------------------------------------------------------------------
// flattenSystemBlocks
//--------------------------------------------------------------------------------------------------

/**
 * @brief Blocks join with a blank line, in order.
 */
void TstProviderJson::flattenSystemBlocks()
{
  QJsonArray blocks;
  blocks.append(block(QStringLiteral("text"),
                      {
                        {QStringLiteral("text"), QStringLiteral("one")}
  }));
  blocks.append(block(QStringLiteral("text"),
                      {
                        {QStringLiteral("text"), QStringLiteral("two")}
  }));

  QCOMPARE(AI::ProviderJson::flattenSystemBlocks(blocks), QStringLiteral("one\n\ntwo"));
}

/**
 * @brief An empty block contributes nothing, not a stray separator.
 */
void TstProviderJson::flattenSystemBlocksSkipsEmpty()
{
  QJsonArray blocks;
  blocks.append(block(QStringLiteral("text"),
                      {
                        {QStringLiteral("text"), QString()}
  }));
  blocks.append(block(QStringLiteral("text"),
                      {
                        {QStringLiteral("text"), QStringLiteral("only")}
  }));

  QCOMPARE(AI::ProviderJson::flattenSystemBlocks(blocks), QStringLiteral("only"));
  QCOMPARE(AI::ProviderJson::flattenSystemBlocks(QJsonArray{}), QString());
}

//--------------------------------------------------------------------------------------------------
// translateHistory
//--------------------------------------------------------------------------------------------------

/**
 * @brief A string-content message passes through; the system text leads the array.
 */
void TstProviderJson::translateHistoryPlainText()
{
  QJsonObject user;
  user.insert(QStringLiteral("role"), QStringLiteral("user"));
  user.insert(QStringLiteral("content"), QStringLiteral("hello"));

  const auto out =
    AI::ProviderJson::translateHistory(QJsonArray{user}, QStringLiteral("sys"), false);

  QCOMPARE(out.size(), 2);
  QCOMPARE(out.at(0).toObject().value(QStringLiteral("role")).toString(), QStringLiteral("system"));
  QCOMPARE(out.at(0).toObject().value(QStringLiteral("content")).toString(), QStringLiteral("sys"));
  QCOMPARE(out.at(1).toObject().value(QStringLiteral("content")).toString(),
           QStringLiteral("hello"));
}

/**
 * @brief Reasoning models take the instructions as a developer message instead of a system one,
 *        and an empty system text produces no leading message at all.
 */
void TstProviderJson::translateHistoryDeveloperRole()
{
  const auto out = AI::ProviderJson::translateHistory({}, QStringLiteral("sys"), true);
  QCOMPARE(out.size(), 1);
  QCOMPARE(out.at(0).toObject().value(QStringLiteral("role")).toString(),
           QStringLiteral("developer"));

  QCOMPARE(AI::ProviderJson::translateHistory({}, QString(), true).size(), 0);
}

/**
 * @brief An assistant tool_use becomes a tool_calls entry with compact JSON arguments, and its
 *        answering tool_result becomes a separate tool message keyed by the same call id.
 */
void TstProviderJson::translateHistoryToolCallRoundTrip()
{
  QJsonObject input;
  input.insert(QStringLiteral("path"), QStringLiteral("a.json"));

  QJsonArray assistantBlocks;
  assistantBlocks.append(block(QStringLiteral("text"),
                               {
                                 {QStringLiteral("text"), QStringLiteral("working")}
  }));
  assistantBlocks.append(block(QStringLiteral("tool_use"),
                               {
                                 {   QStringLiteral("id"),       QStringLiteral("call_1")},
                                 { QStringLiteral("name"), QStringLiteral("project.read")},
                                 {QStringLiteral("input"),                          input}
  }));

  QJsonArray userBlocks;
  userBlocks.append(block(QStringLiteral("tool_result"),
                          {
                            {QStringLiteral("tool_use_id"),        QStringLiteral("call_1")},
                            {    QStringLiteral("content"), QStringLiteral("{\"ok\":true}")}
  }));

  QJsonArray history;
  history.append(message(QStringLiteral("assistant"), assistantBlocks));
  history.append(message(QStringLiteral("user"), userBlocks));

  const auto out = AI::ProviderJson::translateHistory(history, QString(), false);
  QCOMPARE(out.size(), 2);

  const auto assistant = out.at(0).toObject();
  QCOMPARE(assistant.value(QStringLiteral("content")).toString(), QStringLiteral("working"));

  const auto calls = assistant.value(QStringLiteral("tool_calls")).toArray();
  QCOMPARE(calls.size(), 1);

  const auto call = calls.at(0).toObject();
  QCOMPARE(call.value(QStringLiteral("id")).toString(), QStringLiteral("call_1"));
  QCOMPARE(call.value(QStringLiteral("type")).toString(), QStringLiteral("function"));

  const auto fn = call.value(QStringLiteral("function")).toObject();
  QCOMPARE(fn.value(QStringLiteral("name")).toString(), QStringLiteral("project_read"));
  QCOMPARE(fn.value(QStringLiteral("arguments")).toString(),
           QStringLiteral("{\"path\":\"a.json\"}"));

  const auto toolMsg = out.at(1).toObject();
  QCOMPARE(toolMsg.value(QStringLiteral("role")).toString(), QStringLiteral("tool"));
  QCOMPARE(toolMsg.value(QStringLiteral("tool_call_id")).toString(), QStringLiteral("call_1"));
}

/**
 * @brief A tool call no result answers gets a stub reply appended right after it: the vendors
 *        reject a history whose assistant tool_call is left dangling.
 */
void TstProviderJson::translateHistoryBackfillsDanglingToolCall()
{
  QJsonArray assistantBlocks;
  assistantBlocks.append(block(QStringLiteral("tool_use"),
                               {
                                 {   QStringLiteral("id"), QStringLiteral("call_9")},
                                 { QStringLiteral("name"),   QStringLiteral("tool")},
                                 {QStringLiteral("input"),            QJsonObject{}}
  }));

  QJsonArray history;
  history.append(message(QStringLiteral("assistant"), assistantBlocks));

  const auto out = AI::ProviderJson::translateHistory(history, QString(), false);
  QCOMPARE(out.size(), 2);

  const auto stub = out.at(1).toObject();
  QCOMPARE(stub.value(QStringLiteral("role")).toString(), QStringLiteral("tool"));
  QCOMPARE(stub.value(QStringLiteral("tool_call_id")).toString(), QStringLiteral("call_9"));
  QVERIFY(stub.value(QStringLiteral("content")).toString().contains(QStringLiteral("no_result")));
}

//--------------------------------------------------------------------------------------------------
// translateTools and chatCompletionsBody
//--------------------------------------------------------------------------------------------------

/**
 * @brief input_schema becomes parameters, the name is sanitized, and each entry is wrapped in a
 *        function tool.
 */
void TstProviderJson::translateTools()
{
  QJsonObject schema;
  schema.insert(QStringLiteral("type"), QStringLiteral("object"));

  QJsonObject tool;
  tool.insert(QStringLiteral("name"), QStringLiteral("ui.tile"));
  tool.insert(QStringLiteral("description"), QStringLiteral("desc"));
  tool.insert(QStringLiteral("input_schema"), schema);

  const auto out = AI::ProviderJson::translateTools(QJsonArray{tool});
  QCOMPARE(out.size(), 1);
  QCOMPARE(out.at(0).toObject().value(QStringLiteral("type")).toString(),
           QStringLiteral("function"));

  const auto fn = out.at(0).toObject().value(QStringLiteral("function")).toObject();
  QCOMPARE(fn.value(QStringLiteral("name")).toString(), QStringLiteral("ui_tile"));
  QCOMPARE(fn.value(QStringLiteral("description")).toString(), QStringLiteral("desc"));
  QCOMPARE(fn.value(QStringLiteral("parameters")).toObject(), schema);

  QCOMPARE(AI::ProviderJson::translateTools(QJsonArray{}).size(), 0);
}

/**
 * @brief The shared body always streams, names the model, and forbids tool use on request.
 */
void TstProviderJson::chatCompletionsBody()
{
  QJsonObject tool;
  tool.insert(QStringLiteral("name"), QStringLiteral("t"));

  const auto body = AI::ProviderJson::chatCompletionsBody(
    QStringLiteral("gpt-test"), {}, QStringLiteral("sys"), QJsonArray{tool}, true, false);

  QCOMPARE(body.value(QStringLiteral("model")).toString(), QStringLiteral("gpt-test"));
  QCOMPARE(body.value(QStringLiteral("stream")).toBool(), true);
  QCOMPARE(body.value(QStringLiteral("tool_choice")).toString(), QStringLiteral("none"));
  QCOMPARE(body.value(QStringLiteral("tools")).toArray().size(), 1);
  QCOMPARE(body.value(QStringLiteral("messages")).toArray().size(), 1);
}

/**
 * @brief With no tools, neither the tools array nor tool_choice is sent: some servers reject an
 *        empty tools list outright.
 */
void TstProviderJson::chatCompletionsBodyWithoutTools()
{
  const auto body = AI::ProviderJson::chatCompletionsBody(
    QStringLiteral("m"), {}, QString(), QJsonArray{}, false, false);

  QVERIFY(!body.contains(QStringLiteral("tools")));
  QVERIFY(!body.contains(QStringLiteral("tool_choice")));
  QCOMPARE(body.value(QStringLiteral("messages")).toArray().size(), 0);
}

//--------------------------------------------------------------------------------------------------
// Error mapping
//--------------------------------------------------------------------------------------------------

void TstProviderJson::errorMessageFromBody_data()
{
  QTest::addColumn<QByteArray>("body");
  QTest::addColumn<QString>("expected");

  QTest::newRow("vendor error") << QByteArray(R"({"error":{"message":"bad key"}})")
                                << QStringLiteral("bad key");
  QTest::newRow("no error member") << QByteArray(R"({"ok":true})") << QString();
  QTest::newRow("error without message") << QByteArray(R"({"error":{"code":42}})") << QString();
  QTest::newRow("not json") << QByteArray("<html>502</html>") << QString();
  QTest::newRow("empty") << QByteArray() << QString();
  QTest::newRow("array root") << QByteArray("[1,2,3]") << QString();
}

/**
 * @brief The extractor only reports a message the body actually carries; the caller supplies the
 *        translated fallback, so an empty return must be unambiguous.
 */
void TstProviderJson::errorMessageFromBody()
{
  QFETCH(QByteArray, body);
  QFETCH(QString, expected);
  QCOMPARE(AI::ProviderJson::errorMessageFromBody(body), expected);
}

void TstProviderJson::isTransientHttpStatus_data()
{
  QTest::addColumn<int>("status");
  QTest::addColumn<bool>("expected");

  QTest::newRow("bad request") << 400 << false;
  QTest::newRow("unauthorized") << 401 << false;
  QTest::newRow("forbidden") << 403 << false;
  QTest::newRow("timeout") << 408 << true;
  QTest::newRow("rate limited") << 429 << true;
  QTest::newRow("server error") << 500 << true;
  QTest::newRow("gateway timeout") << 504 << true;
  QTest::newRow("ok") << 200 << false;
}

/**
 * @brief Retry classification: a timeout, a rate limit and any 5xx are worth another attempt.
 */
void TstProviderJson::isTransientHttpStatus()
{
  QFETCH(int, status);
  QFETCH(bool, expected);
  QCOMPARE(AI::ProviderJson::isTransientHttpStatus(status), expected);
}

QTEST_APPLESS_MAIN(TstProviderJson)

#include "tst_provider_json.moc"
