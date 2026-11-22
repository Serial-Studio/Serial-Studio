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

#include <QByteArray>
#include <QCoreApplication>
#include <QList>
#include <QPair>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantMap>

#include "Console/AutoReply.h"
#include "Console/AutoReplyRule.h"
#include "Core/Runtime.h"

// Console auto-reply (spec 0127 R4-R7, AC2/AC3): every match mode against text and hexadecimal
// messages, the rule refusals, the line and idle-gap message assembly, and the safety rules: never
// armed from storage, disarmed by a closed link and by a reply storm, and never armed in a sealed
// session. The responder sends through callbacks, so a struct stands in for the device. Settings
// go to a temporary INI folder. The sealing case runs last because a seal cannot be undone.

using Mode = Console::ReplyMatchMode;

//--------------------------------------------------------------------------------------------------
// Fake link and rule builders
//--------------------------------------------------------------------------------------------------

struct FakeLink {
  bool connected = true;
  QList<QPair<int, QByteArray>> sent;
  QStringList notes;

  [[nodiscard]] Console::ReplyLink callbacks()
  {
    Console::ReplyLink link;
    link.connected = [this] {
      return connected;
    };
    link.send = [this](int device, const QByteArray& bytes) {
      sent.append({device, bytes});
    };
    link.note = [this](const QString& text) {
      notes.append(text);
    };
    return link;
  }
};

/**
 * @brief A rule that answers @p reply, as text ending in LF unless @p replyHex.
 */
[[nodiscard]] static Console::ReplyRule rule(const QString& match,
                                             const Mode mode,
                                             const bool hex,
                                             const QString& reply = QStringLiteral("ACK"),
                                             const bool replyHex  = false,
                                             const bool enabled   = true)
{
  Console::ReplyRule out;
  out.match            = match;
  out.mode             = mode;
  out.hex              = hex;
  out.enabled          = enabled;
  out.reply.payload    = reply;
  out.reply.hex        = replyHex;
  out.reply.lineEnding = replyHex ? 0 : 1;
  return out;
}

/**
 * @brief Whether @p candidate, compiled, matches @p message.
 */
[[nodiscard]] static bool matches(const Console::ReplyRule& candidate, const QByteArray& message)
{
  Console::CompiledReplyRule compiled;
  if (!Console::compileReplyRule(candidate, compiled))
    return false;

  return Console::replyRuleMatches(compiled, message);
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class ConsoleAutoReplyTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void matchModes_data();
  void matchModes();
  void ruleRefusals_data();
  void ruleRefusals();
  void longMessageIsJudgedOnItsHead();
  void ruleSurvivesItsStoredForm();
  void rateLimiterAllowsFiftyPerWindow();
  void storedRulesNeverArmThemselves();
  void textRulesAnswerLineByLine();
  void twoMatchingRulesSendTwoReplies();
  void hexRulesAnswerAfterTheGap();
  void disabledAndDisarmedNeverReply();
  void closedLinkDisarmsAndRefusesToArm();
  void replyStormDisarmsWithOneNote();
  void sealedSessionNeverArms();

private:
  QTemporaryDir m_settingsDir;
};

/**
 * @brief Routes QSettings to a temporary INI folder and takes the SS_ASSERT recovery branch.
 */
void ConsoleAutoReplyTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
  QVERIFY(m_settingsDir.isValid());
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_console_auto_reply"));
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
}

/**
 * @brief Every case starts with no stored rule.
 */
void ConsoleAutoReplyTest::init()
{
  QSettings settings;
  settings.clear();
  settings.sync();
}

/**
 * @brief One row per mode, kind and outcome.
 */
void ConsoleAutoReplyTest::matchModes_data()
{
  QTest::addColumn<QString>("match");
  QTest::addColumn<int>("mode");
  QTest::addColumn<bool>("hex");
  QTest::addColumn<QByteArray>("message");
  QTest::addColumn<bool>("expected");

  const auto bytes = QByteArray::fromHex("5d017f0a");
  const int starts = static_cast<int>(Mode::StartsWith);
  const int ends   = static_cast<int>(Mode::EndsWith);
  const int equals = static_cast<int>(Mode::Equals);
  const int has    = static_cast<int>(Mode::Contains);
  const int pat    = static_cast<int>(Mode::Pattern);
  const int regex  = static_cast<int>(Mode::Regex);

  QTest::newRow("text starts") << "OK" << starts << false << QByteArray("OK 12") << true;
  QTest::newRow("text starts miss") << "OK" << starts << false << QByteArray("NOK") << false;
  QTest::newRow("text ends") << "READY" << ends << false << QByteArray("boot READY") << true;
  QTest::newRow("text ends miss") << "READY" << ends << false << QByteArray("READY?") << false;
  QTest::newRow("text equals") << "PING" << equals << false << QByteArray("PING") << true;
  QTest::newRow("text equals miss") << "PING" << equals << false << QByteArray("PING ") << false;
  QTest::newRow("text contains") << "ERR" << has << false << QByteArray("x ERR y") << true;
  QTest::newRow("text is case sensitive") << "err" << has << false << QByteArray("ERR") << false;
  QTest::newRow("hex starts") << "5D 01" << starts << true << bytes << true;
  QTest::newRow("hex starts miss") << "01 5D" << starts << true << bytes << false;
  QTest::newRow("hex ends") << "7f0a" << ends << true << bytes << true;
  QTest::newRow("hex equals") << "5D017F0A" << equals << true << bytes << true;
  QTest::newRow("hex equals miss") << "5D017F" << equals << true << bytes << false;
  QTest::newRow("hex contains") << "01 7F" << has << true << bytes << true;
  QTest::newRow("pattern wildcard") << "5D ?? 7F" << pat << true << bytes << true;
  QTest::newRow("pattern nibble") << "5? 01" << pat << true << bytes << true;
  QTest::newRow("pattern alternative") << "AA BB|01 7F" << pat << true << bytes << true;
  QTest::newRow("pattern miss") << "5D ?? 7E" << pat << true << bytes << false;
  QTest::newRow("pattern on text") << "4F 4B" << pat << false << QByteArray("is OK") << true;
  QTest::newRow("regex mid line") << "ERR[0-9]+" << regex << false << QByteArray("a ERR42 b")
                                  << true;
  QTest::newRow("regex miss") << "^ERR[0-9]+$" << regex << false << QByteArray("a ERR42 b")
                              << false;
}

/**
 * @brief Each mode compares the way its name says, on text and on bytes.
 */
void ConsoleAutoReplyTest::matchModes()
{
  QFETCH(QString, match);
  QFETCH(int, mode);
  QFETCH(bool, hex);
  QFETCH(QByteArray, message);
  QFETCH(bool, expected);

  const auto candidate = rule(match, static_cast<Mode>(mode), hex);
  QVERIFY2(Console::replyRuleProblem(candidate).isEmpty(),
           qPrintable(Console::replyRuleProblem(candidate)));
  QCOMPARE(matches(candidate, message), expected);
}

/**
 * @brief One row per rule that must not be stored.
 */
void ConsoleAutoReplyTest::ruleRefusals_data()
{
  QTest::addColumn<QString>("match");
  QTest::addColumn<int>("mode");
  QTest::addColumn<bool>("hex");
  QTest::addColumn<QString>("reply");
  QTest::addColumn<bool>("replyHex");

  const int starts = static_cast<int>(Mode::StartsWith);
  const int pat    = static_cast<int>(Mode::Pattern);
  const int regex  = static_cast<int>(Mode::Regex);

  QTest::newRow("empty match") << "" << starts << false << "ACK" << false;
  QTest::newRow("empty reply") << "OK" << starts << false << "" << false;
  QTest::newRow("match too long") << QString(300, QLatin1Char('a')) << starts << false << "ACK"
                                  << false;
  QTest::newRow("hex with odd digits") << "5D 0" << starts << true << "ACK" << false;
  QTest::newRow("hex with a letter") << "5G" << starts << true << "ACK" << false;
  QTest::newRow("regex on a hex rule") << "5D.*" << regex << true << "ACK" << false;
  QTest::newRow("invalid regex") << "ERR(" << regex << false << "ACK" << false;
  QTest::newRow("half a pattern byte") << "5D ?" << pat << true << "ACK" << false;
  QTest::newRow("pattern with text") << "hello" << pat << false << "ACK" << false;
  QTest::newRow("reply is not hex") << "OK" << starts << false << "ZZ" << true;
}

/**
 * @brief A refused rule names its problem, compiles to nothing and is never added to the table.
 */
void ConsoleAutoReplyTest::ruleRefusals()
{
  QFETCH(QString, match);
  QFETCH(int, mode);
  QFETCH(bool, hex);
  QFETCH(QString, reply);
  QFETCH(bool, replyHex);

  const auto candidate = rule(match, static_cast<Mode>(mode), hex, reply, replyHex);
  QVERIFY(!Console::replyRuleProblem(candidate).isEmpty());

  Console::CompiledReplyRule compiled;
  QVERIFY(!Console::compileReplyRule(candidate, compiled));

  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(candidate));
  QVERIFY(responder.rules().isEmpty());
  QVERIFY(!responder.ruleProblem(Console::replyRuleToMap(candidate)).isEmpty());
}

/**
 * @brief A message longer than the cap is judged on its first 4096 bytes only.
 */
void ConsoleAutoReplyTest::longMessageIsJudgedOnItsHead()
{
  const auto contains = rule(QStringLiteral("NEEDLE"), Mode::Contains, false);
  QByteArray inside(4000, 'a');
  inside.append("NEEDLE");
  QVERIFY(matches(contains, inside + QByteArray(8000, 'b')));

  QByteArray outside(5000, 'a');
  outside.append("NEEDLE");
  QVERIFY(!matches(contains, outside));
  QVERIFY(!matches(contains, QByteArray()));
}

/**
 * @brief A rule written to its stored map and read back is the same rule.
 */
void ConsoleAutoReplyTest::ruleSurvivesItsStoredForm()
{
  auto original             = rule(QStringLiteral("5D ?? 7F"), Mode::Pattern, true);
  original.reply.payload    = QStringLiteral("0103");
  original.reply.hex        = true;
  original.reply.lineEnding = 0;
  original.reply.checksum   = QStringLiteral("XOR-8");

  const auto restored = Console::replyRuleFromMap(Console::replyRuleToMap(original));
  QCOMPARE(restored.match, original.match);
  QCOMPARE(restored.mode, original.mode);
  QCOMPARE(restored.hex, original.hex);
  QCOMPARE(restored.enabled, original.enabled);

  Console::CompiledReplyRule compiled;
  QVERIFY(Console::compileReplyRule(restored, compiled));
  QCOMPARE(compiled.replyBytes, QByteArray::fromHex("010302"));
}

/**
 * @brief Fifty replies pass inside one window, the next is refused, and a new window starts over.
 */
void ConsoleAutoReplyTest::rateLimiterAllowsFiftyPerWindow()
{
  Console::ReplyRateLimiter limiter;
  for (int i = 0; i < Console::kMaxRepliesPerSecond; ++i)
    QVERIFY(limiter.allow(100 + i));

  QVERIFY(!limiter.allow(200));
  QVERIFY(limiter.allow(100 + Console::kReplyRateWindowMs));
  limiter.reset();
  QCOMPARE(limiter.count, 0);
}

/**
 * @brief Rules persist; the armed state does not. A responder built over stored, enabled rules is
 *        disarmed and ignores what it is fed.
 */
void ConsoleAutoReplyTest::storedRulesNeverArmThemselves()
{
  {
    FakeLink link;
    Console::AutoReply first(link.callbacks());
    first.addRule(Console::replyRuleToMap(rule(QStringLiteral("OK"), Mode::StartsWith, false)));
    first.arm();
    QVERIFY(first.armed());
  }

  FakeLink link;
  Console::AutoReply second(link.callbacks());
  QCOMPARE(second.rules().size(), 1);
  QVERIFY(!second.armed());
  second.feed(0, QByteArray("OK\n"));
  QVERIFY(link.sent.isEmpty());
}

/**
 * @brief A text rule sees one line at a time, however the chunks split it, and the reply goes to
 *        the device the line came from.
 */
void ConsoleAutoReplyTest::textRulesAnswerLineByLine()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(rule(QStringLiteral("OK"), Mode::StartsWith, false)));
  responder.arm();
  QVERIFY(responder.armed());

  responder.feed(3, QByteArray("OK 1\r\nNOK\nO"));
  QCOMPARE(link.sent.size(), 1);
  QCOMPARE(link.sent.at(0).first, 3);
  QCOMPARE(link.sent.at(0).second, QByteArray("ACK\n"));

  responder.feed(3, QByteArray("K"));
  QCOMPARE(link.sent.size(), 1);
  responder.feed(3, QByteArray(" 2\n"));
  QCOMPARE(link.sent.size(), 2);

  responder.feed(7, QByteArray("OK\n"));
  QCOMPARE(link.sent.size(), 3);
  QCOMPARE(link.sent.at(2).first, 7);
  QCOMPARE(responder.replyCount(), quint64(3));
}

/**
 * @brief Every matching rule answers, in table order.
 */
void ConsoleAutoReplyTest::twoMatchingRulesSendTwoReplies()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(
    rule(QStringLiteral("STATUS"), Mode::Contains, false, QStringLiteral("ONE"))));
  responder.addRule(Console::replyRuleToMap(
    rule(QStringLiteral("STATUS?"), Mode::Equals, false, QStringLiteral("TWO"))));
  responder.addRule(Console::replyRuleToMap(
    rule(QStringLiteral("OTHER"), Mode::Equals, false, QStringLiteral("THREE"))));
  responder.arm();

  responder.feed(0, QByteArray("STATUS?\n"));
  QCOMPARE(link.sent.size(), 2);
  QCOMPARE(link.sent.at(0).second, QByteArray("ONE\n"));
  QCOMPARE(link.sent.at(1).second, QByteArray("TWO\n"));
}

/**
 * @brief A hexadecimal rule waits for the line to go quiet, then judges everything it received.
 */
void ConsoleAutoReplyTest::hexRulesAnswerAfterTheGap()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.setGapMs(30);
  responder.addRule(Console::replyRuleToMap(
    rule(QStringLiteral("01 03 ?? ??"), Mode::Pattern, true, QStringLiteral("0183"), true)));
  responder.arm();

  responder.feed(0, QByteArray::fromHex("0103"));
  responder.feed(0, QByteArray::fromHex("0010"));
  QVERIFY(link.sent.isEmpty());

  QTRY_COMPARE_WITH_TIMEOUT(link.sent.size(), 1, 3000);
  QCOMPARE(link.sent.at(0).second, QByteArray::fromHex("0183"));

  QTest::qWait(80);
  QCOMPARE(link.sent.size(), 1);
}

/**
 * @brief An unticked rule never answers, and a disarmed responder ignores every chunk.
 */
void ConsoleAutoReplyTest::disabledAndDisarmedNeverReply()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(
    rule(QStringLiteral("OK"), Mode::StartsWith, false, QStringLiteral("ACK"), false, false)));
  responder.arm();
  QVERIFY(!responder.armed());

  responder.setRuleEnabled(0, true);
  responder.arm();
  QVERIFY(responder.armed());
  responder.disarm();
  responder.feed(0, QByteArray("OK\n"));
  QVERIFY(link.sent.isEmpty());

  responder.arm();
  responder.setRuleEnabled(0, false);
  QVERIFY(!responder.armed());
}

/**
 * @brief Closing the link disarms, and a closed link cannot be armed.
 */
void ConsoleAutoReplyTest::closedLinkDisarmsAndRefusesToArm()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(rule(QStringLiteral("OK"), Mode::StartsWith, false)));
  responder.arm();
  QVERIFY(responder.armed());

  link.connected = false;
  responder.onLinkChanged();
  QVERIFY(!responder.armed());
  responder.arm();
  QVERIFY(!responder.armed());

  link.connected = true;
  responder.onLinkChanged();
  QVERIFY(!responder.armed());
}

/**
 * @brief The fifty-first reply inside one second disarms the responder and writes one note.
 */
void ConsoleAutoReplyTest::replyStormDisarmsWithOneNote()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(rule(QStringLiteral("x"), Mode::Equals, false)));
  responder.arm();

  QByteArray storm;
  for (int i = 0; i < 80; ++i)
    storm.append("x\n");

  responder.feed(0, storm);
  QCOMPARE(link.sent.size(), Console::kMaxRepliesPerSecond);
  QVERIFY(!responder.armed());
  QCOMPARE(link.notes.size(), 1);

  responder.feed(0, storm);
  QCOMPARE(link.sent.size(), Console::kMaxRepliesPerSecond);
  QCOMPARE(link.notes.size(), 1);
}

/**
 * @brief Once the process is sealed the responder reports itself unavailable and never arms.
 */
void ConsoleAutoReplyTest::sealedSessionNeverArms()
{
  FakeLink link;
  Console::AutoReply responder(link.callbacks());
  responder.addRule(Console::replyRuleToMap(rule(QStringLiteral("OK"), Mode::StartsWith, false)));
  QVERIFY(responder.available());

  Core::Runtime::seal(QStringLiteral(":/ssdeploy-test"), QString());
  QVERIFY(!responder.available());
  responder.arm();
  QVERIFY(!responder.armed());
  responder.feed(0, QByteArray("OK\n"));
  QVERIFY(link.sent.isEmpty());
}

QTEST_GUILESS_MAIN(ConsoleAutoReplyTest)

#include "tst_console_auto_reply.moc"
