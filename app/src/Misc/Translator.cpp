/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
  QFile file(":/rcc/messages/Welcome_" + lang + ".txt");
  if (file.open(QFile::ReadOnly))
  {
    text = QString::fromUtf8(file.readAll());
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
