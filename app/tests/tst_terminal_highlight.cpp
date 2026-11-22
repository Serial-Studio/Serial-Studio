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

#include <QColor>
#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantMap>
#include <vector>

#include "Console/HighlightRules.h"
#include "UI/Widgets/Terminal/TerminalHighlighter.h"

// Console colour rules (spec 0127 R8/R9, AC4): where each rule's keyword is found in a row, which
// rule wins an overlap, the bounds on a pathological row, and the rule store: enabled rules reach
// the paint list, disabled ones do not, and the table survives a restart. The overlay only reads
// rows, so "the stored characters never change" holds by its signature; the row is checked anyway.
// Settings go to a temporary INI folder.

using Widgets::collectKeywordSpans;
using Widgets::KeywordSpan;

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief One paint-ready rule.
 */
[[nodiscard]] static Console::KeywordRule keyword(const QString& text,
                                                  const Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
  Console::KeywordRule rule;
  rule.keyword     = text;
  rule.foreground  = QColor(Qt::white);
  rule.background  = QColor(Qt::red);
  rule.sensitivity = cs;
  return rule;
}

/**
 * @brief One rule as the editor submits it.
 */
[[nodiscard]] static QVariantMap stored(const QString& text,
                                        const bool enabled       = true,
                                        const bool caseSensitive = true)
{
  QVariantMap map;
  map.insert(QStringLiteral("keyword"), text);
  map.insert(QStringLiteral("caseSensitive"), caseSensitive);
  map.insert(QStringLiteral("foreground"), QStringLiteral("#ffffff"));
  map.insert(QStringLiteral("background"), QStringLiteral("#112233"));
  map.insert(QStringLiteral("enabled"), enabled);
  return map;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class TerminalHighlightTest : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void init();
  void everyOccurrenceIsFound();
  void caseSensitivityFollowsTheRule();
  void earlierRuleWinsAnOverlap();
  void noRulesAndEmptyRowsYieldNothing();
  void spansAreBounded();
  void rowIsNeverModified();
  void enabledRulesReachThePaintList();
  void invalidRulesAreRefused();
  void tableSurvivesARestart();
  void tableIsCapped();

private:
  QTemporaryDir m_settingsDir;
};

/**
 * @brief Routes QSettings to a temporary INI folder and takes the SS_ASSERT recovery branch.
 */
void TerminalHighlightTest::initTestCase()
{
  qputenv("SS_ASSERT_NONFATAL", "1");
  QVERIFY(m_settingsDir.isValid());
  QCoreApplication::setOrganizationName(QStringLiteral("SerialStudioTests"));
  QCoreApplication::setApplicationName(QStringLiteral("tst_terminal_highlight"));
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
}

/**
 * @brief Every case starts with no stored rule.
 */
void TerminalHighlightTest::init()
{
  QSettings settings;
  settings.clear();
  settings.sync();
}

/**
 * @brief Each occurrence of the keyword is one span, in row order, with the rule's index.
 */
void TerminalHighlightTest::everyOccurrenceIsFound()
{
  std::vector<KeywordSpan> spans;
  const QString row = QStringLiteral("ERROR: link ERROR, no ERRORS? yes ERROR");
  collectKeywordSpans(row, {keyword(QStringLiteral("ERROR"))}, spans);

  QCOMPARE(spans.size(), size_t(4));
  QCOMPARE(spans[0].start, 0);
  QCOMPARE(spans[1].start, 12);
  QCOMPARE(spans[2].start, 22);
  QCOMPARE(spans[3].start, 34);
  for (const auto& span : spans) {
    QCOMPARE(span.length, 5);
    QCOMPARE(span.rule, 0);
    QCOMPARE(row.mid(span.start, span.length), QStringLiteral("ERROR"));
  }
}

/**
 * @brief A case-sensitive rule ignores other casings; a case-insensitive one takes them all.
 */
void TerminalHighlightTest::caseSensitivityFollowsTheRule()
{
  std::vector<KeywordSpan> spans;
  const QString row = QStringLiteral("ok OK Ok oK");

  collectKeywordSpans(row, {keyword(QStringLiteral("OK"))}, spans);
  QCOMPARE(spans.size(), size_t(1));
  QCOMPARE(spans[0].start, 3);

  collectKeywordSpans(row, {keyword(QStringLiteral("OK"), Qt::CaseInsensitive)}, spans);
  QCOMPARE(spans.size(), size_t(4));
}

/**
 * @brief Rules are tried in table order and a later rule cannot recolour part of an earlier
 *        rule's match; its other occurrences are still coloured.
 */
void TerminalHighlightTest::earlierRuleWinsAnOverlap()
{
  std::vector<KeywordSpan> spans;
  const QString row = QStringLiteral("TIMEOUT then OUT");
  collectKeywordSpans(
    row, {keyword(QStringLiteral("TIMEOUT")), keyword(QStringLiteral("OUT"))}, spans);

  QCOMPARE(spans.size(), size_t(2));
  QCOMPARE(spans[0].start, 0);
  QCOMPARE(spans[0].rule, 0);
  QCOMPARE(spans[1].start, 13);
  QCOMPARE(spans[1].rule, 1);

  collectKeywordSpans(
    row, {keyword(QStringLiteral("OUT")), keyword(QStringLiteral("TIMEOUT"))}, spans);
  QCOMPARE(spans.size(), size_t(2));
  QCOMPARE(spans[0].start, 4);
  QCOMPARE(spans[0].rule, 0);
  QCOMPARE(spans[1].start, 13);
}

/**
 * @brief Nothing to find means no span and no failure.
 */
void TerminalHighlightTest::noRulesAndEmptyRowsYieldNothing()
{
  std::vector<KeywordSpan> spans = {
    {1, 2, 3}
  };
  collectKeywordSpans(QStringLiteral("ERROR"), {}, spans);
  QVERIFY(spans.empty());

  collectKeywordSpans(QString(), {keyword(QStringLiteral("ERROR"))}, spans);
  QVERIFY(spans.empty());

  collectKeywordSpans(QStringLiteral("ERR"), {keyword(QStringLiteral("ERROR"))}, spans);
  QVERIFY(spans.empty());

  collectKeywordSpans(QStringLiteral("ERROR"), {keyword(QString())}, spans);
  QVERIFY(spans.empty());
}

/**
 * @brief A row of nothing but keywords stops at the span cap, and a keyword past the scan limit
 *        is not looked for.
 */
void TerminalHighlightTest::spansAreBounded()
{
  std::vector<KeywordSpan> spans;
  collectKeywordSpans(QString(2000, QLatin1Char('x')), {keyword(QStringLiteral("x"))}, spans);
  QCOMPARE(spans.size(), size_t(Widgets::kMaxKeywordSpans));

  QString far(Widgets::kMaxKeywordScan + 10, QLatin1Char('-'));
  far.append(QStringLiteral("ERROR"));
  collectKeywordSpans(far, {keyword(QStringLiteral("ERROR"))}, spans);
  QVERIFY(spans.empty());
}

/**
 * @brief Collecting spans leaves the row exactly as it was received.
 */
void TerminalHighlightTest::rowIsNeverModified()
{
  std::vector<KeywordSpan> spans;
  const QString row  = QStringLiteral("[10:06:13] ERROR code 7");
  const QString copy = row;
  collectKeywordSpans(row, {keyword(QStringLiteral("ERROR")), keyword(QStringLiteral("7"))}, spans);

  QCOMPARE(spans.size(), size_t(2));
  QCOMPARE(row, copy);
}

/**
 * @brief An enabled rule is in the paint list with its colours; a disabled one is stored but not
 *        painted, and enabling it brings it back.
 */
void TerminalHighlightTest::enabledRulesReachThePaintList()
{
  Console::HighlightRules rules;
  rules.addRule(stored(QStringLiteral("ERROR")));
  rules.addRule(stored(QStringLiteral("warn"), false, false));

  QCOMPARE(rules.rules().size(), 2);
  QCOMPARE(rules.compiled().size(), size_t(1));
  QCOMPARE(rules.compiled()[0].keyword, QStringLiteral("ERROR"));
  QCOMPARE(rules.compiled()[0].background, QColor(QStringLiteral("#112233")));
  QCOMPARE(rules.compiled()[0].sensitivity, Qt::CaseSensitive);

  rules.setRuleEnabled(1, true);
  QCOMPARE(rules.compiled().size(), size_t(2));
  QCOMPARE(rules.compiled()[1].sensitivity, Qt::CaseInsensitive);

  rules.moveRule(1, 0);
  QCOMPARE(rules.compiled()[0].keyword, QStringLiteral("warn"));
  rules.removeRule(0);
  QCOMPARE(rules.compiled().size(), size_t(1));
}

/**
 * @brief A rule with no keyword or an overlong one is refused; an unreadable colour falls back.
 */
void TerminalHighlightTest::invalidRulesAreRefused()
{
  Console::HighlightRules rules;
  rules.addRule(stored(QString()));
  rules.addRule(stored(QString(Console::kMaxHighlightKeyword + 1, QLatin1Char('a'))));
  QVERIFY(rules.rules().isEmpty());

  auto odd = stored(QStringLiteral("OK"));
  odd.insert(QStringLiteral("foreground"), QStringLiteral("not-a-color"));
  rules.addRule(odd);
  QCOMPARE(rules.compiled().size(), size_t(1));
  QVERIFY(rules.compiled()[0].foreground.isValid());
}

/**
 * @brief A second store built over the same settings holds the same table.
 */
void TerminalHighlightTest::tableSurvivesARestart()
{
  {
    Console::HighlightRules first;
    first.addRule(stored(QStringLiteral("ERROR")));
    first.addRule(stored(QStringLiteral("OK"), false));
  }

  Console::HighlightRules second;
  QCOMPARE(second.rules().size(), 2);
  QCOMPARE(second.compiled().size(), size_t(1));
  QCOMPARE(second.compiled()[0].keyword, QStringLiteral("ERROR"));
}

/**
 * @brief The table stops at its cap.
 */
void TerminalHighlightTest::tableIsCapped()
{
  Console::HighlightRules rules;
  for (int i = 0; i < Console::kMaxHighlightRules + 5; ++i)
    rules.addRule(stored(QStringLiteral("k%1").arg(i)));

  QCOMPARE(rules.rules().size(), Console::kMaxHighlightRules);
  QCOMPARE(rules.maxRules(), Console::kMaxHighlightRules);
}

QTEST_GUILESS_MAIN(TerminalHighlightTest)

#include "tst_terminal_highlight.moc"
