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

#include <QLocale>
#include <QSet>
#include <QTest>

#include "Misc/LanguageTable.h"

// Every test function here is self-contained: no state is carried between slots, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Pins the language table that Translator and ThemeManager both read: enum/table
 *        alignment, locale round-trips, key uniqueness, and the script-direction flags.
 */
class TstLanguageTable : public QObject {
  Q_OBJECT

private slots:
  void tableMatchesEnumOrder();
  void entryForIsIdentityOverTheTable();
  void entryForClampsUnknownLanguage();
  void localeRoundTripsThroughTheTable();
  void territoryIsPinnedOnlyWhereItMatters();
  void keysAreUniqueAndWellFormed();
  void nativeNamesCoverEveryLanguage();
  void rightToLeftIsArabicAndHebrewOnly();
};

//--------------------------------------------------------------------------------------------------
// Table shape
//--------------------------------------------------------------------------------------------------

/**
 * @brief Row N must describe language N: every lookup in the table is an index cast.
 */
void TstLanguageTable::tableMatchesEnumOrder()
{
  const auto& entries = Misc::LanguageTable::entries();
  QCOMPARE(static_cast<int>(entries.size()), Misc::LanguageTable::kLanguageCount);

  for (std::size_t i = 0; i < entries.size(); ++i)
    QCOMPARE(static_cast<int>(entries.at(i).language), static_cast<int>(i));

  QCOMPARE(entries.front().language, Misc::Translator::English);
  QCOMPARE(entries.back().language, Misc::Translator::Vietnamese);
}

/**
 * @brief entryFor() returns the row that names the requested language.
 */
void TstLanguageTable::entryForIsIdentityOverTheTable()
{
  const auto& entries = Misc::LanguageTable::entries();
  for (const auto& entry : entries) {
    const auto& found = Misc::LanguageTable::entryFor(entry.language);
    QCOMPARE(found.language, entry.language);
    QCOMPARE(QString::fromLatin1(found.qmName), QString::fromLatin1(entry.qmName));
  }
}

/**
 * @brief A settings file from a build with more languages must degrade to English, not abort.
 */
void TstLanguageTable::entryForClampsUnknownLanguage()
{
  const auto beyond = static_cast<Misc::Translator::Language>(Misc::LanguageTable::kLanguageCount);
  QCOMPARE(Misc::LanguageTable::entryFor(beyond).language, Misc::Translator::English);
  QCOMPARE(Misc::LanguageTable::qmName(beyond), QStringLiteral("en_US"));
  QCOMPARE(Misc::LanguageTable::welcomeCode(beyond), QStringLiteral("EN"));

  const auto negative = static_cast<Misc::Translator::Language>(-1);
  QCOMPARE(Misc::LanguageTable::entryFor(negative).language, Misc::Translator::English);
}

//--------------------------------------------------------------------------------------------------
// Locales
//--------------------------------------------------------------------------------------------------

/**
 * @brief Each shipped locale maps back to the language that declared it; anything else is English.
 */
void TstLanguageTable::localeRoundTripsThroughTheTable()
{
  const auto& entries = Misc::LanguageTable::entries();
  for (const auto& entry : entries)
    QCOMPARE(Misc::LanguageTable::languageForLocale(entry.locale), entry.language);

  QCOMPARE(Misc::LanguageTable::languageForLocale(QLocale::Zulu), Misc::Translator::English);
  QCOMPARE(Misc::LanguageTable::languageForLocale(QLocale::AnyLanguage), Misc::Translator::English);
}

/**
 * @brief Only the three languages whose script or numerals depend on the territory pin one; the
 *        rest build a plain language locale.
 */
void TstLanguageTable::territoryIsPinnedOnlyWhereItMatters()
{
  QCOMPARE(Misc::LanguageTable::localeFor(Misc::Translator::Arabic).territory(),
           QLocale::SaudiArabia);
  QCOMPARE(Misc::LanguageTable::localeFor(Misc::Translator::Hebrew).territory(), QLocale::Israel);
  QCOMPARE(Misc::LanguageTable::localeFor(Misc::Translator::Vietnamese).territory(),
           QLocale::Vietnam);

  QCOMPARE(Misc::LanguageTable::localeFor(Misc::Translator::German).language(), QLocale::German);
  QCOMPARE(Misc::LanguageTable::localeFor(Misc::Translator::Japanese).language(),
           QLocale::Japanese);
}

//--------------------------------------------------------------------------------------------------
// Keys and names
//--------------------------------------------------------------------------------------------------

/**
 * @brief The .qm base names double as theme-translation keys and the welcome-text codes select a
 *        bundled resource, so a duplicate or malformed key silently mislabels a language.
 */
void TstLanguageTable::keysAreUniqueAndWellFormed()
{
  QSet<QString> qmNames;
  QSet<QString> welcomeCodes;

  const auto& entries = Misc::LanguageTable::entries();
  for (const auto& entry : entries) {
    const auto qmName      = QString::fromLatin1(entry.qmName);
    const auto welcomeCode = QString::fromLatin1(entry.welcomeCode);

    QCOMPARE(static_cast<int>(qmName.length()), 5);
    QCOMPARE(qmName.at(2), QChar('_'));
    QCOMPARE(static_cast<int>(welcomeCode.length()), 2);
    QVERIFY(welcomeCode == welcomeCode.toUpper());

    QVERIFY(!qmNames.contains(qmName));
    QVERIFY(!welcomeCodes.contains(welcomeCode));
    qmNames.insert(qmName);
    welcomeCodes.insert(welcomeCode);
  }

  QCOMPARE(static_cast<int>(qmNames.size()), Misc::LanguageTable::kLanguageCount);
  QCOMPARE(static_cast<int>(welcomeCodes.size()), Misc::LanguageTable::kLanguageCount);
}

/**
 * @brief The selector model is the table in order, with no blank rows.
 */
void TstLanguageTable::nativeNamesCoverEveryLanguage()
{
  const auto names = Misc::LanguageTable::nativeNames();
  QCOMPARE(static_cast<int>(names.count()), Misc::LanguageTable::kLanguageCount);

  for (const auto& name : names)
    QVERIFY(!name.trimmed().isEmpty());

  QCOMPARE(names.first(), QStringLiteral("English"));
}

/**
 * @brief Translator::rtl() reads this flag to flip the application layout direction.
 */
void TstLanguageTable::rightToLeftIsArabicAndHebrewOnly()
{
  const auto& entries = Misc::LanguageTable::entries();
  for (const auto& entry : entries) {
    const bool expected =
      entry.language == Misc::Translator::Arabic || entry.language == Misc::Translator::Hebrew;
    QCOMPARE(entry.rightToLeft, expected);
  }
}

QTEST_APPLESS_MAIN(TstLanguageTable)

#include "tst_language_table.moc"
