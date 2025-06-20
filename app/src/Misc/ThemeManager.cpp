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

/*
 * Orbbyt Dashboard - https://dashboard.orbbyt.com
 * Written by Alex Spataru <https://aspatru.com>
 *
 * Copyright (C) 2025 Orbbyt <https://orbbyt.com>
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

#include "ThemeManager.h"

#include <QDir>
#include <QPalette>
#include <QStyleHints>
#include <QJsonDocument>
#include <QGuiApplication>

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
      QStringLiteral("ironframe")
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
 * @brief Returns a list of theme names that are available.
 * @return QStringList containing the names of all loaded themes.
 */
const QStringList &Misc::ThemeManager::availableThemes() const
{
  return m_availableThemes;
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
  if (index < 0 || index >= availableThemes().count())
    filteredIndex = 0;

  // Update the theme name
  m_theme = filteredIndex;
  m_themeName = availableThemes().at(filteredIndex);
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

  // Hint Qt about the effective color scheme
  const auto bg = getColor(QStringLiteral("base"));
  const auto fg = getColor(QStringLiteral("text"));
  if (fg.lightness() > bg.lightness())
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
  else
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);

  // Update UI
  Q_EMIT themeChanged();
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
  m_theme = availableThemes().indexOf(m_themeName);
  m_colors = themeData.value("colors").toObject();

  // Update user interface
  Q_EMIT themeChanged();
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
