/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/Translator.h"

#include <QApplication>
#include <QFile>

#ifdef BUILD_COMMERCIAL
#  include "Licensing/CommercialToken.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Translator and applies the saved or system language.
 */
Misc::Translator::Translator()
{
  const auto sysLang  = static_cast<int>(systemLanguage());
  const auto language = m_settings.value("language", sysLang).toInt();
  setLanguage(static_cast<Language>(language));
}

/**
 * @brief Returns the singleton Translator instance.
 */
Misc::Translator& Misc::Translator::instance()
{
  static Translator singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// Language queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current language enum value.
 *
 * Returns the current language ID, which corresponds to the indexes of the
 * languages returned by the \c availableLanguages() function.
 */
Misc::Translator::Language Misc::Translator::language() const
{
  return m_language;
}

/**
 * @brief Returns the language enum that matches the host operating system locale.
 */
Misc::Translator::Language Misc::Translator::systemLanguage() const
{
  // Map system locale to internal language enum
  Language lang;
  switch (QLocale::system().language()) {
    case QLocale::English:
      lang = English;
      break;
    case QLocale::Spanish:
      lang = Spanish;
      break;
    case QLocale::Chinese:
      lang = Chinese;
      break;
    case QLocale::German:
      lang = German;
      break;
    case QLocale::Russian:
      lang = Russian;
      break;
    case QLocale::French:
      lang = French;
      break;
    case QLocale::Japanese:
      lang = Japanese;
      break;
    case QLocale::Korean:
      lang = Korean;
      break;
    case QLocale::Portuguese:
      lang = Portuguese;
      break;
    case QLocale::Italian:
      lang = Italian;
      break;
    case QLocale::Polish:
      lang = Polish;
      break;
    case QLocale::Turkish:
      lang = Turkish;
      break;
    case QLocale::Ukrainian:
      lang = Ukrainian;
      break;
    case QLocale::Czech:
      lang = Czech;
      break;
    case QLocale::Hindi:
      lang = Hindi;
      break;
    case QLocale::Dutch:
      lang = Dutch;
      break;
    case QLocale::Romanian:
      lang = Romanian;
      break;
    case QLocale::Swedish:
      lang = Swedish;
      break;
    default:
      lang = English;
      break;
  }

  return lang;
}

//--------------------------------------------------------------------------------------------------
// Text resources
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the localized welcome text shown on the console.
 */
QString Misc::Translator::welcomeConsoleText() const
{
  // Determine language code for the welcome text file
  QString lang;
  switch (language()) {
    case English:
      lang = QStringLiteral("EN");
      break;
    case Spanish:
      lang = QStringLiteral("ES");
      break;
    case Chinese:
      lang = QStringLiteral("ZH");
      break;
    case German:
      lang = QStringLiteral("DE");
      break;
    case Russian:
      lang = QStringLiteral("RU");
      break;
    case French:
      lang = QStringLiteral("FR");
      break;
    case Japanese:
      lang = QStringLiteral("JA");
      break;
    case Korean:
      lang = QStringLiteral("KO");
      break;
    case Portuguese:
      lang = QStringLiteral("PT");
      break;
    case Italian:
      lang = QStringLiteral("IT");
      break;
    case Polish:
      lang = QStringLiteral("PL");
      break;
    case Turkish:
      lang = QStringLiteral("TR");
      break;
    case Ukrainian:
      lang = QStringLiteral("UK");
      break;
    case Czech:
      lang = QStringLiteral("CZ");
      break;
    case Hindi:
      lang = QStringLiteral("HI");
      break;
    case Dutch:
      lang = QStringLiteral("NL");
      break;
    case Romanian:
      lang = QStringLiteral("RO");
      break;
    case Swedish:
      lang = QStringLiteral("SV");
      break;
    default:
      lang = QStringLiteral("EN");
      break;
  }

  // Define fallback text & default message path
  QString text = QObject::tr("Failed to load welcome text :(");
#ifdef BUILD_COMMERCIAL
  QString path   = ":/rcc/messages/trial/Welcome_" + lang + ".txt";
  const auto& tk = Licensing::CommercialToken::current();
  if (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Hobbyist)
    path = ":/rcc/messages/pro/Welcome_" + lang + ".txt";
#else
  QString path = ":/rcc/messages/gpl3/Welcome_" + lang + ".txt";
#endif

  // Read welcome text from resources
  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    file.close();
  }

  // Return obtained data
  return text + "\n";
}

/**
 * @brief Returns the bundled acknowledgements text.
 */
QString Misc::Translator::acknowledgementsText() const
{
  // Read acknowledgements text from bundled resources
  QString text = "";
  QFile file(QStringLiteral(":/rcc/messages/Acknowledgements.txt"));
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    file.close();
  }

  return text;
}

//--------------------------------------------------------------------------------------------------
// Language selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list of available translation language names.
 */
QStringList& Misc::Translator::availableLanguages()
{
  // Build the list of available translations on first call
  static QStringList list;
  if (list.isEmpty()) {
    list.append(QStringLiteral("English"));
    list.append(QStringLiteral("Español"));
    list.append(QStringLiteral("简体中文"));
    list.append(QStringLiteral("Deutsch"));
    list.append(QStringLiteral("Русский"));
    list.append(QStringLiteral("Français"));
    list.append(QStringLiteral("日本語"));
    list.append(QStringLiteral("한국어"));
    list.append(QStringLiteral("Português"));
    list.append(QStringLiteral("Italiano"));
    list.append(QStringLiteral("Polski"));
    list.append(QStringLiteral("Türkçe"));
    list.append(QStringLiteral("Українська"));
    list.append(QStringLiteral("Čeština"));
    list.append(QStringLiteral("हिन्दी"));
    list.append(QStringLiteral("Nederlands"));
    list.append(QStringLiteral("Română"));
    list.append(QStringLiteral("Svenska"));
  }

  return list;
}

/**
 * @brief Sets the application language by enum value and reloads translations.
 *
 * Changes the language of the application and emits the signals appropiate to
 * reload every string that uses the Qt translator system.
 *
 * @param language language ID based on the indexes of the \a
 * availableLanguages() function
 */
void Misc::Translator::setLanguage(const Language language)
{
  // Map language enum to locale name and QLocale
  QString langName;
  QLocale locale;
  switch (language) {
    case English:
      langName = QStringLiteral("en_US");
      locale   = QLocale(QLocale::English);
      break;
    case Spanish:
      langName = QStringLiteral("es_MX");
      locale   = QLocale(QLocale::Spanish);
      break;
    case Chinese:
      langName = QStringLiteral("zh_CN");
      locale   = QLocale(QLocale::Chinese);
      break;
    case German:
      langName = QStringLiteral("de_DE");
      locale   = QLocale(QLocale::German);
      break;
    case Russian:
      langName = QStringLiteral("ru_RU");
      locale   = QLocale(QLocale::Russian);
      break;
    case French:
      langName = QStringLiteral("fr_FR");
      locale   = QLocale(QLocale::French);
      break;
    case Japanese:
      langName = QStringLiteral("ja_JP");
      locale   = QLocale(QLocale::Japanese);
      break;
    case Korean:
      langName = QStringLiteral("ko_KR");
      locale   = QLocale(QLocale::Korean);
      break;
    case Portuguese:
      langName = QStringLiteral("pt_BR");
      locale   = QLocale(QLocale::Portuguese);
      break;
    case Italian:
      langName = QStringLiteral("it_IT");
      locale   = QLocale(QLocale::Italian);
      break;
    case Polish:
      langName = QStringLiteral("pl_PL");
      locale   = QLocale(QLocale::Polish);
      break;
    case Turkish:
      langName = QStringLiteral("tr_TR");
      locale   = QLocale(QLocale::Turkish);
      break;
    case Ukrainian:
      langName = QStringLiteral("uk_UA");
      locale   = QLocale(QLocale::Ukrainian);
      break;
    case Czech:
      langName = QStringLiteral("cs_CZ");
      locale   = QLocale(QLocale::Czech);
      break;
    case Hindi:
      langName = QStringLiteral("hi_IN");
      locale   = QLocale(QLocale::Hindi);
      break;
    case Dutch:
      langName = QStringLiteral("nl_NL");
      locale   = QLocale(QLocale::Dutch);
      break;
    case Romanian:
      langName = QStringLiteral("ro_RO");
      locale   = QLocale(QLocale::Romanian);
      break;
    case Swedish:
      langName = QStringLiteral("sv_SE");
      locale   = QLocale(QLocale::Swedish);
      break;
    default:
      langName = QStringLiteral("en_US");
      locale   = QLocale(QLocale::English);
      break;
  }

  m_language = language;
  m_settings.setValue(QStringLiteral("language"), m_language);

  setLanguage(locale, langName);
}

/**
 * @brief Loads a specific .qm translation file for the given locale.
 *
 * Changes the language of the application and emits the signals neccesary to
 * reload every string that uses the Qt translator system.
 *
 * @param locale    user-set locale
 * @param language  name of the *.qm file to load from the "translations"
 *                  directory inside the application's resources
 */
void Misc::Translator::setLanguage(const QLocale& locale, const QString& language)
{
  // Load the translation file and install it
  qApp->removeTranslator(&m_translator);
  const auto qmPath = QStringLiteral(":/qm/%1.qm").arg(language);
  if (m_translator.load(locale, qmPath)) {
    qApp->installTranslator(&m_translator);
    Q_EMIT languageChanged();
  }
}
