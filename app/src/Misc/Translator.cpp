/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Misc/Translator.h"

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
    default:
      lang = QStringLiteral("EN");
      break;
  }

  QString text = QObject::tr("Failed to load welcome text :(");
  QFile file(":/rcc/messages/gpl3/Welcome_" + lang + ".txt");
  if (file.open(QFile::ReadOnly))
  {
    text = QString::fromUtf8(file.readAll()).arg(qApp->applicationVersion());
    file.close();
  }

  return text;
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
