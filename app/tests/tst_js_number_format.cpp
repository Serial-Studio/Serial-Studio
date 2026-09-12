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

#include <cmath>
#include <cstring>
#include <limits>
#include <QFile>
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QRandomGenerator>
#include <QTest>

#include "DataModel/Scripting/ScriptCells.h"

// Spec 0086 R5: formatJsNumber must be byte-equal to ECMAScript Number::toString, the text the list
// path got from QJSValue::toString(). Two oracles: the Node-generated corpus committed under
// tests/fixtures (regenerate with tests/scripts/gen_js_number_corpus.py) and the QJSEngine that
// ships with the app, swept over seeded random doubles.

namespace {

constexpr int kRandomSweep        = 20000;
constexpr qsizetype kTextCapacity = 40;
constexpr quint32 kSweepSeed      = 0x0086u;

/**
 * @brief Reassembles the double whose IEEE-754 bits the fixture key spells.
 */
double doubleFromHex(const QString& hex, bool& ok)
{
  const quint64 bits = hex.toULongLong(&ok, 16);
  double value       = 0.0;
  std::memcpy(&value, &bits, sizeof(value));
  return value;
}

/**
 * @brief formatJsNumber as a QString, empty on overflow.
 */
QString formatted(double value)
{
  char text[kTextCapacity];
  const qsizetype len = DataModel::formatJsNumber(value, text, kTextCapacity);
  return len > 0 ? QString::fromLatin1(text, len) : QString();
}

}  // namespace

/**
 * @brief The ECMAScript layout rules on the hand-picked boundaries, the Node corpus, and a
 * QJSEngine parity sweep; the overflow contract at the end.
 */
class TstJsNumberFormat : public QObject {
  Q_OBJECT

private slots:
  void boundaryCases();
  void corpusMatchesNode();
  void randomSweepMatchesQJSEngine();
  void overflowReportsZero();
};

/**
 * @brief The layout boundaries of ECMA-262 6.1.6.1.20 spelled out: fixed for 1e-6 .. 1e21, exponent
 *        outside, specials, negative zero folded to "0".
 */
void TstJsNumberFormat::boundaryCases()
{
  QCOMPARE(formatted(0.0), QStringLiteral("0"));
  QCOMPARE(formatted(-0.0), QStringLiteral("0"));
  QCOMPARE(formatted(1.0), QStringLiteral("1"));
  QCOMPARE(formatted(-1.5), QStringLiteral("-1.5"));
  QCOMPARE(formatted(0.1), QStringLiteral("0.1"));
  QCOMPARE(formatted(0.1 + 0.2), QStringLiteral("0.30000000000000004"));
  QCOMPARE(formatted(1e-6), QStringLiteral("0.000001"));
  QCOMPARE(formatted(1e-7), QStringLiteral("1e-7"));
  QCOMPARE(formatted(1.5e-7), QStringLiteral("1.5e-7"));
  QCOMPARE(formatted(1e20), QStringLiteral("100000000000000000000"));
  QCOMPARE(formatted(1e21), QStringLiteral("1e+21"));
  QCOMPARE(formatted(1.5e21), QStringLiteral("1.5e+21"));
  QCOMPARE(formatted(123456789012345680000.0), QStringLiteral("123456789012345680000"));
  QCOMPARE(formatted(9007199254740993.0), QStringLiteral("9007199254740992"));
  QCOMPARE(formatted(5e-324), QStringLiteral("5e-324"));
  QCOMPARE(formatted(1.7976931348623157e308), QStringLiteral("1.7976931348623157e+308"));
  QCOMPARE(formatted(std::numeric_limits<double>::quiet_NaN()), QStringLiteral("NaN"));
  QCOMPARE(formatted(std::numeric_limits<double>::infinity()), QStringLiteral("Infinity"));
  QCOMPARE(formatted(-std::numeric_limits<double>::infinity()), QStringLiteral("-Infinity"));
}

/**
 * @brief Every entry of tests/fixtures/js-number-format.json (bits -> Node's String(x)) formats to
 *        the same bytes.
 */
void TstJsNumberFormat::corpusMatchesNode()
{
#ifndef SS_JS_NUMBER_CORPUS
  QSKIP("SS_JS_NUMBER_CORPUS not defined by the build");
#else
  QFile file(QStringLiteral(SS_JS_NUMBER_CORPUS));
  if (!file.open(QIODevice::ReadOnly))
    QSKIP("corpus fixture not found; run tests/scripts/gen_js_number_corpus.py");

  const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QVERIFY(doc.isObject());
  const QJsonObject corpus = doc.object();
  QVERIFY(corpus.size() > 2000);

  int mismatches = 0;
  for (auto it = corpus.constBegin(); it != corpus.constEnd(); ++it) {
    bool ok            = false;
    const double value = doubleFromHex(it.key(), ok);
    QVERIFY2(ok, qPrintable(it.key()));

    const QString expected = it.value().toString();
    const QString actual   = formatted(value);
    if (actual != expected) {
      ++mismatches;
      qWarning("bits %s: expected %s, got %s",
               qPrintable(it.key()),
               qPrintable(expected),
               qPrintable(actual));
    }
  }

  QCOMPARE(mismatches, 0);
#endif
}

/**
 * @brief The engine the app ships is the authority the list path used: over seeded random doubles
 *        spanning the whole exponent range, formatJsNumber equals QJSValue::toString().
 */
void TstJsNumberFormat::randomSweepMatchesQJSEngine()
{
  QJSEngine engine;
  QRandomGenerator rng(kSweepSeed);

  int mismatches = 0;
  for (int i = 0; i < kRandomSweep; ++i) {
    const int exponent    = rng.bounded(-40, 41);
    const double mantissa = rng.generateDouble() * 10.0;
    const double sign     = rng.bounded(2) == 0 ? 1.0 : -1.0;
    double value          = sign * mantissa * std::pow(10.0, exponent);
    if (i % 5 == 0)
      value = static_cast<double>(rng.bounded(qint64(-2000000000), qint64(2000000000)));

    const QString expected = engine.toScriptValue(value).toString();
    const QString actual   = formatted(value);
    if (actual != expected) {
      ++mismatches;
      qWarning("value %.17g: expected %s, got %s", value, qPrintable(expected), qPrintable(actual));
    }
  }

  QCOMPARE(mismatches, 0);
}

/**
 * @brief A buffer too small for the text reports 0 and never writes past it.
 */
void TstJsNumberFormat::overflowReportsZero()
{
  char text[4];
  QCOMPARE(DataModel::formatJsNumber(0.30000000000000004, text, sizeof(text)), qsizetype(0));
  QCOMPARE(DataModel::formatJsNumber(-std::numeric_limits<double>::infinity(), text, sizeof(text)),
           qsizetype(0));
  QCOMPARE(DataModel::formatJsNumber(1.0, text, sizeof(text)), qsizetype(1));
  QCOMPARE(text[0], '1');
}

QTEST_GUILESS_MAIN(TstJsNumberFormat)

#include "tst_js_number_format.moc"
