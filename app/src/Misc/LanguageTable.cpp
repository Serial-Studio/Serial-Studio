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

#include "Misc/LanguageTable.h"

//--------------------------------------------------------------------------------------------------
// The table
//--------------------------------------------------------------------------------------------------

// code-verify off
static const std::array<Misc::LanguageEntry, Misc::LanguageTable::kLanguageCount> kTable = {
  {
   {false,
     Misc::Translator::English,
     QLocale::English,
     QLocale::AnyTerritory,
     "en_US",
     "EN",
     "English"},
   {false,
     Misc::Translator::Spanish,
     QLocale::Spanish,
     QLocale::AnyTerritory,
     "es_MX",
     "ES",
     "Español"},
   {false,
     Misc::Translator::Chinese,
     QLocale::Chinese,
     QLocale::AnyTerritory,
     "zh_CN",
     "ZH",
     "简体中文"},
   {false,
     Misc::Translator::German,
     QLocale::German,
     QLocale::AnyTerritory,
     "de_DE",
     "DE",
     "Deutsch"},
   {false,
     Misc::Translator::Russian,
     QLocale::Russian,
     QLocale::AnyTerritory,
     "ru_RU",
     "RU",
     "Русский"},
   {false,
     Misc::Translator::French,
     QLocale::French,
     QLocale::AnyTerritory,
     "fr_FR",
     "FR",
     "Français"},
   {false,
     Misc::Translator::Japanese,
     QLocale::Japanese,
     QLocale::AnyTerritory,
     "ja_JP",
     "JA",
     "日本語"},
   {false,
     Misc::Translator::Korean,
     QLocale::Korean,
     QLocale::AnyTerritory,
     "ko_KR",
     "KO",
     "한국어"},
   {false,
     Misc::Translator::Portuguese,
     QLocale::Portuguese,
     QLocale::AnyTerritory,
     "pt_BR",
     "PT",
     "Português"},
   {false,
     Misc::Translator::Italian,
     QLocale::Italian,
     QLocale::AnyTerritory,
     "it_IT",
     "IT",
     "Italiano"},
   {false,
     Misc::Translator::Polish,
     QLocale::Polish,
     QLocale::AnyTerritory,
     "pl_PL",
     "PL",
     "Polski"},
   {false,
     Misc::Translator::Turkish,
     QLocale::Turkish,
     QLocale::AnyTerritory,
     "tr_TR",
     "TR",
     "Türkçe"},
   {false,
     Misc::Translator::Ukrainian,
     QLocale::Ukrainian,
     QLocale::AnyTerritory,
     "uk_UA",
     "UK",
     "Українська"},
   {false,
     Misc::Translator::Czech,
     QLocale::Czech,
     QLocale::AnyTerritory,
     "cs_CZ",
     "CZ",
     "Čeština"},
   {false, Misc::Translator::Hindi, QLocale::Hindi, QLocale::AnyTerritory, "hi_IN", "HI", "हिन्दी"},
   {false,
     Misc::Translator::Dutch,
     QLocale::Dutch,
     QLocale::AnyTerritory,
     "nl_NL",
     "NL",
     "Nederlands"},
   {false,
     Misc::Translator::Romanian,
     QLocale::Romanian,
     QLocale::AnyTerritory,
     "ro_RO",
     "RO",
     "Română"},
   {false,
     Misc::Translator::Swedish,
     QLocale::Swedish,
     QLocale::AnyTerritory,
     "sv_SE",
     "SV",
     "Svenska"},
   {true,
     Misc::Translator::Arabic,
     QLocale::Arabic,
     QLocale::SaudiArabia,
     "ar_SA",
     "AR",
     "العربية"},
   {true, Misc::Translator::Hebrew, QLocale::Hebrew, QLocale::Israel, "he_IL", "HE", "עברית"},
   {false,
     Misc::Translator::Vietnamese,
     QLocale::Vietnamese,
     QLocale::Vietnam,
     "vi_VN",
     "VI",
     "Tiếng Việt"},
   }
};

// code-verify on

//--------------------------------------------------------------------------------------------------
// Table access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns every shipped language, in Translator::Language order.
 */
const std::array<Misc::LanguageEntry, Misc::LanguageTable::kLanguageCount>& Misc::LanguageTable::
  entries()
{
  return kTable;
}

/**
 * @brief Returns the row for @p language. A value outside the enum degrades to English rather
 *        than asserting: a settings file written by a build that ships more languages reaches
 *        here on every launch, and the old switch answered it the same way.
 */
const Misc::LanguageEntry& Misc::LanguageTable::entryFor(const Translator::Language language)
{
  const auto index = static_cast<int>(language);
  if (index < 0 || index >= kLanguageCount)
    return kTable.front();

  return kTable.at(static_cast<std::size_t>(index));
}

/**
 * @brief Maps a host locale onto the closest shipped language; anything unlisted is English.
 */
Misc::Translator::Language Misc::LanguageTable::languageForLocale(const QLocale::Language locale)
{
  for (const auto& entry : kTable)
    if (entry.locale == locale)
      return entry.language;

  return Translator::English;
}

//--------------------------------------------------------------------------------------------------
// Derived values
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the QLocale a language loads its translation with; only the languages whose
 *        script differs per territory pin one.
 */
QLocale Misc::LanguageTable::localeFor(const Translator::Language language)
{
  const auto& entry = entryFor(language);
  if (entry.territory == QLocale::AnyTerritory)
    return QLocale(entry.locale);

  return QLocale(entry.locale, entry.territory);
}

/**
 * @brief Returns the ".qm" base name of a language, which is also the key its theme translations
 *        are published under.
 */
QString Misc::LanguageTable::qmName(const Translator::Language language)
{
  return QString::fromLatin1(entryFor(language).qmName);
}

/**
 * @brief Returns the two-letter code of the bundled welcome text for a language.
 */
QString Misc::LanguageTable::welcomeCode(const Translator::Language language)
{
  return QString::fromLatin1(entryFor(language).welcomeCode);
}

/**
 * @brief Returns the native display names, in enum order, for the language selector.
 */
QStringList Misc::LanguageTable::nativeNames()
{
  QStringList names;
  names.reserve(kLanguageCount);
  for (const auto& entry : kTable)
    names.append(QString::fromUtf8(entry.nativeName));

  return names;
}
