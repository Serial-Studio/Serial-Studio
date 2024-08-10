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

#include "ThemeManager.h"

#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QPalette>

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
  QStringList themes = {
      QStringLiteral(":/rcc/themes/Breeze.json"),
      QStringLiteral(":/rcc/themes/Light.json"),
      QStringLiteral(":/rcc/themes/Dark.json"),
  };

  // Set application theme
  setTheme(m_settings.value("Theme", 1).toInt());

  // Automatically react to theme changes
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
 * @brief Returns a list of theme names that are available.
 * @return QStringList containing the names of all loaded themes.
 */
QStringList Misc::ThemeManager::availableThemes() const
{
  return m_themes.keys();
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

  return QColor("#f0f");
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
  if (event->type() == QEvent::ApplicationPaletteChange)
  {
    if (theme() == 0)
    {
      setTheme(0);
      return true;
    }
  }

  return QObject::eventFilter(watched, event);
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
  // Load colors from theme
  auto fixedIndex = index;
  if (index > 0 && index < availableThemes().count())
    m_themeName = availableThemes().at(index);
  else
    fixedIndex = 0;

  // Update the theme
  m_theme = fixedIndex;
  m_settings.setValue("Theme", fixedIndex);
  m_colors = m_themes.value(m_themeName);

  // Update UI
  Q_EMIT themeChanged();
}
