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
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <QLocale>
#include <QObject>
#include <QTranslator>

#ifdef QT_QML_LIB
#  include <QtQml>
#endif

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
