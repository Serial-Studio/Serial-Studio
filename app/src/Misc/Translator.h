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

#pragma once

#include <QLocale>
#include <QObject>
#include <QSettings>
#include <QTranslator>

namespace Misc
{
/**
 * @brief The Translator class
 *
 * The @c Translator module provides the user interface with a list of available
 * translations, and loads the specified translation file during application
 * startup or when the user changes the language of the application.
 */
class Translator : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(Language language
             READ language
             WRITE setLanguage
             NOTIFY languageChanged)
  Q_PROPERTY(QStringList availableLanguages
             READ availableLanguages
             CONSTANT)
  Q_PROPERTY(QString welcomeConsoleText
             READ welcomeConsoleText
             NOTIFY languageChanged)
  Q_PROPERTY(QString acknowledgementsText
             READ acknowledgementsText
             NOTIFY languageChanged)
  // clang-format on

signals:
  void languageChanged();

private:
  explicit Translator();
  Translator(Translator &&) = delete;
  Translator(const Translator &) = delete;
  Translator &operator=(Translator &&) = delete;
  Translator &operator=(const Translator &) = delete;

public:
  static Translator &instance();

  enum Language
  {
    English,
    Spanish,
    Chinese,
    German,
    Russian,
    French,
    Japanese,
    Korean,
    Portuguese,
    Italian,
    Polish,
    Turkish,
    Ukrainian,
    Czech
  };
  Q_ENUM(Language);

  [[nodiscard]] Language language() const;
  [[nodiscard]] Language systemLanguage() const;
  [[nodiscard]] static QStringList &availableLanguages();

  [[nodiscard]] QString welcomeConsoleText() const;
  [[nodiscard]] QString acknowledgementsText() const;

public slots:
  void setLanguage(const Language language);
  void setLanguage(const QLocale &locale, const QString &language);

private:
  Language m_language;
  QSettings m_settings;
  QTranslator m_translator;
};
} // namespace Misc
