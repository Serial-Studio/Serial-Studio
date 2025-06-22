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

#include <QFile>
#include <QApplication>

#include "Misc/Translator.h"

#ifdef BUILD_COMMERCIAL
#  include "Licensing/LemonSqueezy.h"
#endif

/**
 * Constructor function
 */
Misc::Translator::Translator()
{
  const auto sysLang = static_cast<int>(systemLanguage());
  const auto language = m_settings.value("language", sysLang).toInt();
  setLanguage(static_cast<Language>(language));
}

/**
 * Returns the only instance of the class
 */
Misc::Translator &Misc::Translator::instance()
{
  static Translator singleton;
  return singleton;
}

/**
 * Returns the current language ID, which corresponds to the indexes of the
 * languages returned by the \c availableLanguages() function.
 */
Misc::Translator::Language Misc::Translator::language() const
{
  return m_language;
}

/**
 * Returns the appropiate language ID based on the current locale settings of
 * the host's operating system.
 */
Misc::Translator::Language Misc::Translator::systemLanguage() const
{
  Language lang;
  switch (QLocale::system().language())
  {
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
    default:
      lang = English;
      break;
  }

  return lang;
}

/**
 * Returns the welcome text displayed on the console
 */
QString Misc::Translator::welcomeConsoleText() const
{
  QString lang;
  switch (language())
  {
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
    default:
      lang = QStringLiteral("EN");
      break;
  }

  // Define fallback text & default message path
  QString text = QObject::tr("Failed to load welcome text :(");
#ifdef BUILD_COMMERCIAL
  QString path = ":/rcc/messages/trial/Welcome_" + lang + ".txt";
  if (Licensing::LemonSqueezy::instance().isActivated())
    path = ":/rcc/messages/pro/Welcome_" + lang + ".txt";
#else
  QString path = ":/rcc/messages/gpl3/Welcome_" + lang + ".txt";
#endif

  // Read welcome text from resources
  QFile file(path);
  if (file.open(QFile::ReadOnly))
  {
    if (path.contains("pro"))
    {
      text = QString::fromUtf8(file.readAll())
                 .arg(qApp->applicationDisplayName());
    }

    else
      text = QString::fromUtf8(file.readAll());

    file.close();
  }

  // Return obtained data
  return text + "\n";
}

/**
 * Returns the acknowledgements text.
 */
QString Misc::Translator::acknowledgementsText() const
{
  QString text = "";
  QFile file(QStringLiteral(":/rcc/messages/Acknowledgements.txt"));
  if (file.open(QFile::ReadOnly))
  {
    text = QString::fromUtf8(file.readAll());
    file.close();
  }

  return text;
}

/**
 * Returns a list with the available translation languages.
 */
QStringList &Misc::Translator::availableLanguages()
{
  static QStringList list;
  if (list.isEmpty())
  {
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
  }

  return list;
}

/**
 * Changes the language of the application and emits the signals appropiate to
 * reload every string that uses the Qt translator system.
 *
 * @param language language ID based on the indexes of the \a
 * availableLanguages() function
 */
void Misc::Translator::setLanguage(const Language language)
{
  QString langName;
  QLocale locale;
  switch (language)
  {
    case English:
      langName = QStringLiteral("en_US");
      locale = QLocale(QLocale::English);
      break;
    case Spanish:
      langName = QStringLiteral("es_MX");
      locale = QLocale(QLocale::Spanish);
      break;
    case Chinese:
      langName = QStringLiteral("zh_CN");
      locale = QLocale(QLocale::Chinese);
      break;
    case German:
      langName = QStringLiteral("de_DE");
      locale = QLocale(QLocale::German);
      break;
    case Russian:
      langName = QStringLiteral("ru_RU");
      locale = QLocale(QLocale::Russian);
      break;
    case French:
      langName = QStringLiteral("fr_FR");
      locale = QLocale(QLocale::French);
      break;
    case Japanese:
      langName = QStringLiteral("ja_JP");
      locale = QLocale(QLocale::Japanese);
      break;
    case Korean:
      langName = QStringLiteral("ko_KR");
      locale = QLocale(QLocale::Korean);
      break;
    case Portuguese:
      langName = QStringLiteral("pt_BR");
      locale = QLocale(QLocale::Portuguese);
      break;
    case Italian:
      langName = QStringLiteral("it_IT");
      locale = QLocale(QLocale::Italian);
      break;
    case Polish:
      langName = QStringLiteral("pl_PL");
      locale = QLocale(QLocale::Polish);
      break;
    case Turkish:
      langName = QStringLiteral("tr_TR");
      locale = QLocale(QLocale::Turkish);
      break;
    case Ukrainian:
      langName = QStringLiteral("uk_UA");
      locale = QLocale(QLocale::Ukrainian);
      break;
    case Czech:
      langName = QStringLiteral("cs_CZ");
      locale = QLocale(QLocale::Czech);
      break;
    default:
      langName = QStringLiteral("en_US");
      locale = QLocale(QLocale::English);
      break;
  }

  m_language = language;
  m_settings.setValue(QStringLiteral("language"), m_language);

  setLanguage(locale, langName);
}

/**
 * Changes the language of the application and emits the signals neccesary to
 * reload every string that uses the Qt translator system.
 *
 * @param locale    user-set locale
 * @param language  name of the *.qm file to load from the "translations"
 *                  directory inside the application's resources
 */
void Misc::Translator::setLanguage(const QLocale &locale,
                                   const QString &language)
{
  qApp->removeTranslator(&m_translator);
  const auto qmPath = QStringLiteral(":/qm/%1.qm").arg(language);
  if (m_translator.load(locale, qmPath))
  {
    qApp->installTranslator(&m_translator);
    Q_EMIT languageChanged();
  }
}
