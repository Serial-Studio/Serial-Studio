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

#include "ThemeManager.h"

#include <QDir>
#include <QPalette>
#include <QStyleHints>
#include <QJsonDocument>
#include <QGuiApplication>

#include "Misc/Translator.h"

/**
 * @brief Constructs the ThemeManager object and initializes theme loading.
 *
 * The constructor sets up the @c ThemeManager by loading available themes from
 * JSON files located in the specified resource directory.
 *
 * It installs an event filter to listen for system-wide palette changes,
 * responding to changes by updating the application theme accordingly.
 */
Misc::ThemeManager::ThemeManager()
  : m_theme(0)
{
  // Set theme files
  // clang-format off
  const QStringList themes = {
      QStringLiteral("default"),
      QStringLiteral("light"),
      QStringLiteral("dark"),
      QStringLiteral("iron"),
      QStringLiteral("midnight"),
  };
  // clang-format on

  // Load theme files
  for (const auto &theme : std::as_const(themes))
  {
    QFile file(QStringLiteral(":/rcc/themes/%1.json").arg(theme));
    if (file.open(QFile::ReadOnly))
    {
      const auto document = QJsonDocument::fromJson(file.readAll());
      const auto title = document.object().value("title").toString();
      m_themes.insert(title, document.object());
      m_availableThemes.append(title);

      file.close();
    }
  }

  // Append "System" theme as last option
  m_availableThemes.append(QStringLiteral("System"));

  // Set application theme
  setTheme(m_settings.value("ApplicationTheme", 0).toInt());

  // Load localized theme names
  updateLocalizedThemeNames();
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &Misc::ThemeManager::updateLocalizedThemeNames);

  // Install event filter only once
  qApp->installEventFilter(this);
}

/**
 * @brief Provides a reference to the singleton instance of the ThemeManager.
 * @return Reference to the static instance of the ThemeManager.
 */
Misc::ThemeManager &Misc::ThemeManager::instance()
{
  static ThemeManager instance;
  return instance;
}

/**
 * @brief Retrieves the current theme index.
 * @return The index of the currently active theme.
 */
int Misc::ThemeManager::theme() const
{
  return m_theme;
}

/**
 * @brief Retrieves the current name of the loaded theme.
 * @return The index of the currently active theme.
 */
const QString &Misc::ThemeManager::themeName() const
{
  return m_themeName;
}

/**
 * @brief Fetches the color scheme of the current theme as a QJsonObject.
 * @return QJsonObject containing the color scheme of the current theme.
 */
const QJsonObject &Misc::ThemeManager::colors() const
{
  return m_colors;
}

/**
 * @brief Fetches the current theme as a QJsonObject.
 * @return QJsonObject containing all the properties of the loaded theme.
 */
const QJsonObject &Misc::ThemeManager::themeData() const
{
  return m_themeData;
}

/**
 * @brief Fetches the current theme parameters as a QJsonObject.
 * @return QJsonObject containing all the properties of the loaded theme.
 */
const QJsonObject &Misc::ThemeManager::parameters() const
{
  return m_parameters;
}

/**
 * @brief Returns a list of theme names that are available.
 * @return QStringList containing the names of all loaded themes.
 */
const QStringList &Misc::ThemeManager::availableThemes() const
{
  return m_availableThemeNames;
}

/**
 * @brief Returns a @c QColor object for the given component @a name.
 * @param name
 * @return QColor
 */
QColor Misc::ThemeManager::getColor(const QString &name) const
{
  if (colors().contains(name))
    return QColor(colors()[name].toString());

  return QColor(qRgb(0xff, 0x00, 0xff));
}

/**
 * @brief Sets the current theme to the theme at the specified index.
 *
 * @param index The index of the theme in the available themes list to set as
 * the current theme.
 *
 * Emits the @c themeChanged signal upon successful theme change.
 */
void Misc::ThemeManager::setTheme(const int index)
{
  // Validate index
  int filteredIndex = index;
  if (index < 0 || index >= m_availableThemes.count())
    filteredIndex = 0;

  // Update the theme name
  m_theme = filteredIndex;
  m_themeName = m_availableThemes.at(filteredIndex);
  m_settings.setValue("ApplicationTheme", filteredIndex);

  // Load theme (dark/light) automagically
  if (m_themeName == QStringLiteral("System"))
  {
    loadSystemTheme();
    return;
  }

  // Load actual theme data
  m_themeData = m_themes.value(m_themeName);
  m_colors = m_themeData.value("colors").toObject();
  m_parameters = m_themeData.value("parameters").toObject();

  // Hint Qt about the effective color scheme
  const auto bg = getColor(QStringLiteral("base"));
  const auto fg = getColor(QStringLiteral("text"));
  if (fg.lightness() > bg.lightness())
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
  else
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);

  // Update UI
  QMetaObject::invokeMethod(
      this, [this]() { Q_EMIT themeChanged(); }, Qt::QueuedConnection);
}

/**
 * @brief Applies the system-resolved theme (Light or Dark) without changing
 * the selected theme index.
 *
 * This method inspects the system's current color palette and dynamically
 * resolves whether the Light or Dark theme should be applied visually.
 *
 * It updates the internal theme data and color scheme used throughout the
 * application UI, without changing the user-selected theme index
 * (which remains as "System").
 *
 * This ensures that when "System" is selected, the app visually tracks the
 * system's theme preference while preserving user settings and theme list
 * integrity.
 *
 * @note This function is automatically invoked during startup or when the
 *       system palette changes, but only when the selected theme is "System".
 */
void Misc::ThemeManager::loadSystemTheme()
{
  // Get system color scheme
  qApp->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
  const auto scheme = qApp->styleHints()->colorScheme();

  // Get theme name to load
  QString resolved;
  if (scheme == Qt::ColorScheme::Dark)
    resolved = QStringLiteral("Dark");
  else if (scheme == Qt::ColorScheme::Light)
    resolved = QStringLiteral("Light");
  else
    resolved = QStringLiteral("Light");

  // Load theme data
  const auto themeData = m_themes.value(resolved);

  // Set theme data
  m_themeData = themeData;
  m_themeName = QStringLiteral("System");
  m_theme = m_availableThemes.indexOf(m_themeName);
  m_colors = themeData.value("colors").toObject();
  m_parameters = themeData.value("parameters").toObject();

  // Update user interface
  QMetaObject::invokeMethod(
      this, [this]() { Q_EMIT themeChanged(); }, Qt::QueuedConnection);
}

/**
 * @brief Updates the localized names of available themes based on current UI
 *        language.
 *
 * This method rebuilds the `m_availableThemeNames` list by extracting the
 * appropriate translation for each theme from its `translations` object in the
 * theme JSON data. It uses the current language as returned by
 * `Misc::Translator::instance().language()` to select the corresponding
 * translation key (e.g. "es_MX", "de_DE", etc).
 *
 * If a translation is missing or empty, the theme's default `title` is used as
 * fallback. The "System" theme is handled separately and translated via
 * `tr("System")`.
 *
 * After rebuilding the list, the `languageChanged()` signal is emitted to
 * notify the UI of the updated theme names.
 *
 * @see Misc::Translator::language()
 * @see availableThemes()
 */
void Misc::ThemeManager::updateLocalizedThemeNames()
{
  m_availableThemeNames.clear();
  const auto lang = Translator::instance().language();

  for (const auto &themeName : std::as_const(m_availableThemes))
  {
    if (themeName == QStringLiteral("System"))
    {
      m_availableThemeNames.append(tr("System"));
      continue;
    }

    const auto themeObj = m_themes.value(themeName);
    const auto translations = themeObj.value("translations").toObject();

    QString localized;
    switch (lang)
    {
      case Translator::Spanish:
        localized = translations.value("es_MX").toString();
        break;
      case Translator::Chinese:
        localized = translations.value("zh_CN").toString();
        break;
      case Translator::German:
        localized = translations.value("de_DE").toString();
        break;
      case Translator::Russian:
        localized = translations.value("ru_RU").toString();
        break;
      case Translator::French:
        localized = translations.value("fr_FR").toString();
        break;
      case Translator::Japanese:
        localized = translations.value("ja_JP").toString();
        break;
      case Translator::Korean:
        localized = translations.value("ko_KR").toString();
        break;
      case Translator::Portuguese:
        localized = translations.value("pt_BR").toString();
        break;
      case Translator::Italian:
        localized = translations.value("it_IT").toString();
        break;
      case Translator::Polish:
        localized = translations.value("pl_PL").toString();
        break;
      case Translator::Turkish:
        localized = translations.value("tr_TR").toString();
        break;
      case Translator::Ukrainian:
        localized = translations.value("uk_UA").toString();
        break;
      case Translator::Czech:
        localized = translations.value("cs_CZ").toString();
        break;
      case Translator::English:
      default:
        localized = themeObj.value("title").toString();
        break;
    }

    if (localized.isEmpty())
      localized = themeObj.value("title").toString();

    m_availableThemeNames.append(localized);
  }

  Q_EMIT languageChanged();
}

/**
 * @brief Event filter to intercept application-wide events.
 *
 * This method is an overridden event filter that specifically listens for the
 * @c QEvent::ApplicationPaletteChange event.
 *
 * When this event is detected, it triggers a theme update to match the new
 * system palette.
 *
 * @param watched The object where the event originated.
 * @param event The event that is being filtered.
 * @return true if the event was handled and should not be processed further
 */

bool Misc::ThemeManager::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::ApplicationPaletteChange
      && m_themeName == QStringLiteral("System"))
  {
    loadSystemTheme();
    return true;
  }

  return QObject::eventFilter(watched, event);
}
