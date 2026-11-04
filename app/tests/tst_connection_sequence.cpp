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
#include <QJsonDocument>
#include <QTest>

#include "Core/Checksum.h"
#include "Core/IO/ConnectionSequence.h"

using IO::ConnectionSequence;
using IO::ExpectPattern;
using IO::SequenceStep;

/**
 * @brief The Connection Sequence model of spec 0103 (AC7's model half): persistence, the
 *        validation verdicts with their step index, and the expect-pattern grammar.
 */
class TstConnectionSequence : public QObject {
  Q_OBJECT

private slots:
  void emptyByDefault();
  void roundTripsEveryKind();
  void restoresFromEveryShape();
  void validSequencePasses();
  void validationNamesTheStep_data();
  void validationNamesTheStep();
  void parsesPatterns();
  void rejectsMalformedPatterns();
  void matchesWithWildcards();
  void encodesSendBytes();
  void describesSteps();

private:
  [[nodiscard]] static ConnectionSequence everyKind();
};

/**
 * @brief A sequence holding one step of every kind, the shape the plan's example stores.
 */
ConnectionSequence TstConnectionSequence::everyKind()
{
  const auto json = QByteArrayLiteral(R"({
    "budgetMs": 2500,
    "steps": [
      {"kind": "break", "ms": 55},
      {"kind": "wait", "ms": 50},
      {"kind": "discard"},
      {"kind": "send", "data": "15 15", "hex": true, "eol": "", "checksum": ""},
      {"kind": "expect", "patterns": ["55", "5D 5?"], "timeoutMs": 100, "optional": true},
      {"kind": "baud", "rate": 115200},
      {"kind": "flow", "mode": "rtscts"},
      {"kind": "line", "line": "rts", "high": false}
    ]
  })");

  return ConnectionSequence::fromJson(QJsonDocument::fromJson(json).object());
}

/**
 * @brief An absent sequence is empty, keeps the ten-second budget and validates clean, so a
 *        project saved before spec 0103 behaves exactly as it did.
 */
void TstConnectionSequence::emptyByDefault()
{
  const auto sequence = ConnectionSequence::fromJson(QJsonObject());
  QVERIFY(sequence.isEmpty());
  QCOMPARE(sequence.budgetMs, IO::kDefaultSequenceBudgetMs);
  QVERIFY(sequence.validate().message.isEmpty());

  QVERIFY(ConnectionSequence::fromVariant(QVariant()).isEmpty());
}

/**
 * @brief Every kind survives a JSON round trip with the fields it reads.
 */
void TstConnectionSequence::roundTripsEveryKind()
{
  const auto sequence = everyKind();
  QCOMPARE(sequence.steps.size(), 8);
  QCOMPARE(sequence.budgetMs, 2500);

  const auto& steps = sequence.steps;
  QCOMPARE(steps.at(0).kind, SequenceStep::Kind::Break);
  QCOMPARE(steps.at(0).ms, 55);
  QCOMPARE(steps.at(1).kind, SequenceStep::Kind::Wait);
  QCOMPARE(steps.at(2).kind, SequenceStep::Kind::Discard);
  QCOMPARE(steps.at(3).kind, SequenceStep::Kind::Send);
  QVERIFY(steps.at(3).hex);
  QCOMPARE(steps.at(4).kind, SequenceStep::Kind::Expect);
  QCOMPARE(steps.at(4).patterns, QStringList({QStringLiteral("55"), QStringLiteral("5D 5?")}));
  QVERIFY(steps.at(4).optional);
  QCOMPARE(steps.at(5).rate, 115200);
  QCOMPARE(steps.at(6).flow, SequenceStep::Flow::RtsCts);
  QCOMPARE(steps.at(7).line, SequenceStep::Line::Rts);
  QVERIFY(!steps.at(7).high);

  const auto again = ConnectionSequence::fromJson(sequence.toJson());
  QCOMPARE(again.toJson(), sequence.toJson());
}

/**
 * @brief The three shapes a driver property arrives in restore the same sequence: the UI
 *        driver's QJsonObject, applyConnectionSettings' QVariantMap and QML's JSON text.
 */
void TstConnectionSequence::restoresFromEveryShape()
{
  const auto json = everyKind().toJson();

  const auto fromObject = ConnectionSequence::fromVariant(QVariant::fromValue(json));
  const auto fromMap    = ConnectionSequence::fromVariant(json.toVariantMap());
  const auto fromText   = ConnectionSequence::fromVariant(
    QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));

  QCOMPARE(fromObject.toJson(), json);
  QCOMPARE(fromMap.toJson(), json);
  QCOMPARE(fromText.toJson(), json);
}

/**
 * @brief A well-formed sequence reports no problem.
 */
void TstConnectionSequence::validSequencePasses()
{
  const auto error = everyKind().validate();
  QVERIFY2(error.message.isEmpty(), qPrintable(error.message));
  QCOMPARE(error.step, -1);
}

/**
 * @brief One malformed sequence per row, with the index of the step the error must name.
 */
void TstConnectionSequence::validationNamesTheStep_data()
{
  QTest::addColumn<QString>("json");
  QTest::addColumn<int>("step");

  const QString longPattern = QStringLiteral("AA").repeated(IO::kExpectBufferBytes + 1);

  QTest::newRow("negative wait") << R"({"steps":[{"kind":"discard"},{"kind":"wait","ms":-1}]})"
                                 << 1;
  QTest::newRow("zero break") << R"({"steps":[{"kind":"break","ms":0}]})" << 0;
  QTest::newRow("empty send") << R"({"steps":[{"kind":"wait","ms":1},{"kind":"send"}]})" << 1;
  QTest::newRow("bad hex") << R"({"steps":[{"kind":"send","data":"1G","hex":true}]})" << 0;
  QTest::newRow("odd hex") << R"({"steps":[{"kind":"send","data":"151","hex":true}]})" << 0;
  QTest::newRow("bad checksum") << R"({"steps":[{"kind":"send","data":"A","checksum":"NOPE"}]})"
                                << 0;
  QTest::newRow("no pattern") << R"({"steps":[{"kind":"expect","timeoutMs":5}]})" << 0;
  QTest::newRow("bad pattern") << R"({"steps":[{"kind":"expect","patterns":["5"]}]})" << 0;
  QTest::newRow("long pattern")
    << QStringLiteral(R"({"steps":[{"kind":"expect","patterns":["%1"]}]})").arg(longPattern) << 0;
  QTest::newRow("zero timeout")
    << R"({"steps":[{"kind":"expect","patterns":["55"],"timeoutMs":0}]})" << 0;
  QTest::newRow("zero baud") << R"({"steps":[{"kind":"baud","rate":0}]})" << 0;
  QTest::newRow("unknown kind") << R"({"steps":[{"kind":"teleport"}]})" << 0;
  QTest::newRow("unknown flow mode")
    << R"({"steps":[{"kind":"discard"},{"kind":"flow","mode":"dsrdtr"}]})" << 1;
  QTest::newRow("unknown line") << R"({"steps":[{"kind":"line","line":"cts"}]})" << 0;
  QTest::newRow("waits exceed budget")
    << R"({"budgetMs":100,"steps":[{"kind":"break","ms":60},{"kind":"wait","ms":50}]})" << 1;
  QTest::newRow("budget out of range") << R"({"budgetMs":0,"steps":[]})" << -1;
}

/**
 * @brief Each malformed sequence fails with a message and the index of the offending step.
 */
void TstConnectionSequence::validationNamesTheStep()
{
  QFETCH(QString, json);
  QFETCH(int, step);

  const auto sequence =
    ConnectionSequence::fromJson(QJsonDocument::fromJson(json.toUtf8()).object());
  const auto error = sequence.validate();
  QVERIFY(!error.message.isEmpty());
  QCOMPARE(error.step, step);
}

/**
 * @brief "??" is any byte, a '?' nibble is any nibble, case and spacing do not matter.
 */
void TstConnectionSequence::parsesPatterns()
{
  ExpectPattern pattern;
  QVERIFY(ExpectPattern::parse(QStringLiteral("5D ?? 7F"), pattern));
  QCOMPARE(pattern.value, QByteArray::fromHex("5D007F"));
  QCOMPARE(pattern.mask, QByteArray::fromHex("FF00FF"));

  QVERIFY(ExpectPattern::parse(QStringLiteral("5?"), pattern));
  QCOMPARE(pattern.value, QByteArray::fromHex("50"));
  QCOMPARE(pattern.mask, QByteArray::fromHex("F0"));

  QVERIFY(ExpectPattern::parse(QStringLiteral("?5"), pattern));
  QCOMPARE(pattern.value, QByteArray::fromHex("05"));
  QCOMPARE(pattern.mask, QByteArray::fromHex("0F"));

  QVERIFY(ExpectPattern::parse(QStringLiteral("5d??7f"), pattern));
  QCOMPARE(pattern.value, QByteArray::fromHex("5D007F"));
}

/**
 * @brief An odd digit count, a foreign character or an empty text never parses.
 */
void TstConnectionSequence::rejectsMalformedPatterns()
{
  ExpectPattern pattern;
  QVERIFY(!ExpectPattern::parse(QString(), pattern));
  QVERIFY(!ExpectPattern::parse(QStringLiteral("5"), pattern));
  QVERIFY(!ExpectPattern::parse(QStringLiteral("GG"), pattern));
  QVERIFY(!ExpectPattern::parse(QStringLiteral("5D 7"), pattern));
}

/**
 * @brief matchIn() returns the offset just past the first match, or -1, so the caller consumes
 *        through the reply and never matches it twice.
 */
void TstConnectionSequence::matchesWithWildcards()
{
  ExpectPattern pattern;
  QVERIFY(ExpectPattern::parse(QStringLiteral("5D ?? 7F"), pattern));

  QCOMPARE(pattern.matchIn(QByteArray::fromHex("015D337F00")), qsizetype(4));
  QCOMPARE(pattern.matchIn(QByteArray::fromHex("5DAA7F")), qsizetype(3));
  QCOMPARE(pattern.matchIn(QByteArray::fromHex("5DAA7E")), qsizetype(-1));
  QCOMPARE(pattern.matchIn(QByteArray::fromHex("5D")), qsizetype(-1));
  QCOMPARE(pattern.matchIn(QByteArray()), qsizetype(-1));

  QVERIFY(ExpectPattern::parse(QStringLiteral("5?"), pattern));
  QCOMPARE(pattern.matchIn(QByteArray::fromHex("125A")), qsizetype(2));
  QCOMPARE(pattern.matchIn(QByteArray::fromHex("1265")), qsizetype(-1));
}

/**
 * @brief A send step's bytes: hex pairs or escaped text, then the end of line, then the checksum
 *        over both, the order an action uses.
 */
void TstConnectionSequence::encodesSendBytes()
{
  SequenceStep hex;
  hex.kind = SequenceStep::Kind::Send;
  hex.hex  = true;
  hex.data = QStringLiteral("15 15");
  QCOMPARE(hex.sendBytes(), QByteArray::fromHex("1515"));

  SequenceStep text;
  text.kind = SequenceStep::Kind::Send;
  text.data = QStringLiteral("AT\\r");
  text.eol  = QStringLiteral("\\n");
  QCOMPARE(text.sendBytes(), QByteArray("AT\r\n"));

  SequenceStep summed = text;
  summed.checksum     = QStringLiteral("XOR-8");
  const auto expected = QByteArray("AT\r\n") + IO::checksum(summed.checksum, "AT\r\n");
  QCOMPARE(summed.sendBytes(), expected);
}

/**
 * @brief Every step has a console line naming its position and a kind label.
 */
void TstConnectionSequence::describesSteps()
{
  const auto sequence = everyKind();
  for (int i = 0; i < sequence.steps.size(); ++i) {
    const auto text = sequence.stepText(i);
    QVERIFY(text.contains(QString::number(i + 1)));
    QVERIFY(!ConnectionSequence::kindLabel(sequence.steps.at(i).kind).isEmpty());
  }

  QCOMPARE(ConnectionSequence::hexText(QByteArray::fromHex("5A00")), QStringLiteral("5A 00"));
}

QTEST_APPLESS_MAIN(TstConnectionSequence)

#include "tst_connection_sequence.moc"
