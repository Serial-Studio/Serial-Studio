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

#include "Misc/Translator.h"

#include <QApplication>
#include <QFile>

#include "Misc/LanguageTable.h"

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
 */
Misc::Translator::Language Misc::Translator::language() const
{
  return m_language;
}

/**
 * @brief Returns true if the active language uses a right-to-left script.
 */
bool Misc::Translator::rtl() const
{
  return LanguageTable::entryFor(m_language).rightToLeft;
}

/**
 * @brief Returns the language enum that matches the host operating system locale.
 */
Misc::Translator::Language Misc::Translator::systemLanguage() const
{
  return LanguageTable::languageForLocale(QLocale::system().language());
}

//--------------------------------------------------------------------------------------------------
// Text resources
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the localized welcome text shown on the console.
 */
QString Misc::Translator::welcomeConsoleText() const
{
  const auto lang = LanguageTable::welcomeCode(language());

  QString text = QObject::tr("Failed to load welcome text :(");
#ifdef BUILD_COMMERCIAL
  QString path   = ":/messages/trial/Welcome_" + lang + ".txt";
  const auto& tk = Licensing::CommercialToken::current();
  if (tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() > Licensing::FeatureTier::Trial)
    path = ":/messages/pro/Welcome_" + lang + ".txt";
#else
  QString path = ":/messages/gpl3/Welcome_" + lang + ".txt";
#endif

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    text = QString::fromUtf8(file.readAll());
    file.close();
  }

  return text + "\n";
}

/**
 * @brief Returns the bundled acknowledgements text.
 */
QString Misc::Translator::acknowledgementsText() const
{
  QString text = "";
  QFile file(QStringLiteral(":/messages/Acknowledgements.txt"));
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
  static QStringList list = LanguageTable::nativeNames();
  return list;
}

/**
 * @brief Sets the application language by enum value and reloads translations.
 */
void Misc::Translator::setLanguage(const Language language)
{
  m_language = language;
  m_settings.setValue(QStringLiteral("language"), m_language);

  setLanguage(LanguageTable::localeFor(language), LanguageTable::qmName(language));
}

/**
 * @brief Loads a specific .qm translation file for the given locale.
 */
void Misc::Translator::setLanguage(const QLocale& locale, const QString& language)
{
  qApp->removeTranslator(&m_translator);
  const auto qmPath = QStringLiteral(":/qm/%1.qm").arg(language);
  if (m_translator.load(locale, qmPath)) {
    qApp->installTranslator(&m_translator);
    qApp->setLayoutDirection(rtl() ? Qt::RightToLeft : Qt::LeftToRight);
    Q_EMIT languageChanged();
  }
}
