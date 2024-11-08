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

#include <QDir>
#include <QPalette>
#include <QStyleHints>
#include <QApplication>
#include <QJsonDocument>

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
  m_availableThemes = {
    QStringLiteral("Default"),
    QStringLiteral("Outdoor Day"),
    QStringLiteral("Outdoor Night"),
    QStringLiteral("Breeze Light"),
    QStringLiteral("Breeze Dark"),
    QStringLiteral("macOS Light"),
    QStringLiteral("macOS Dark"),
    QStringLiteral("Yaru Light"),
    QStringLiteral("Yaru Dark"),
    QStringLiteral("Deep Purple"),
    QStringLiteral("Deep Blue"),
    QStringLiteral("Deep Red"),
    QStringLiteral("Deep Green"),
    QStringLiteral("Pulse"),
    QStringLiteral("Resistance"),
    QStringLiteral("Dominion"),
  };
  // clang-format on

  // Load theme files
  foreach (auto theme, m_availableThemes)
  {
    QFile file(QStringLiteral(":/rcc/themes/%1.json").arg(theme));
    if (file.open(QFile::ReadOnly))
    {
      auto document = QJsonDocument::fromJson(file.readAll());
      auto title = document.object().value("title").toString();
      auto colors = document.object().value("colors").toObject();
      m_themes.insert(title, colors);

      file.close();
    }
  }

  // Set application theme
  setTheme(m_settings.value("Theme", 0).toInt());

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

  return QColor("#f0f");
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
  auto filteredIndex = index;
  if (index < 0 || index >= availableThemes().count())
    filteredIndex = 0;

  // Update the theme & obtain the colors
  m_theme = filteredIndex;
  m_settings.setValue("Theme", filteredIndex);
  m_themeName = availableThemes().at(filteredIndex);
  m_colors = m_themes.value(m_themeName);

  // Tell OS if we prefer dark mode or light mode
  auto bg = getColor("base");
  auto fg = getColor("text");
  if (fg.lightness() > bg.lightness())
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
  else
    qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);

  // Set application palette
  QPalette palette;
  palette.setColor(QPalette::Mid, getColor("mid"));
  palette.setColor(QPalette::Dark, getColor("dark"));
  palette.setColor(QPalette::Text, getColor("text"));
  palette.setColor(QPalette::Base, getColor("base"));
  palette.setColor(QPalette::Link, getColor("link"));
  palette.setColor(QPalette::Light, getColor("light"));
  palette.setColor(QPalette::Window, getColor("window"));
  palette.setColor(QPalette::Shadow, getColor("shadow"));
  palette.setColor(QPalette::Accent, getColor("accent"));
  palette.setColor(QPalette::Button, getColor("button"));
  palette.setColor(QPalette::Midlight, getColor("midlight"));
  palette.setColor(QPalette::Highlight, getColor("highlight"));
  palette.setColor(QPalette::WindowText, getColor("window_text"));
  palette.setColor(QPalette::BrightText, getColor("bright_text"));
  palette.setColor(QPalette::ButtonText, getColor("button_text"));
  palette.setColor(QPalette::ToolTipBase, getColor("tooltip_base"));
  palette.setColor(QPalette::ToolTipText, getColor("tooltip_text"));
  palette.setColor(QPalette::LinkVisited, getColor("link_visited"));
  palette.setColor(QPalette::AlternateBase, getColor("alternate_base"));
  palette.setColor(QPalette::PlaceholderText, getColor("placeholder_text"));
  palette.setColor(QPalette::HighlightedText, getColor("highlighted_text"));
  qApp->setPalette(palette);

  // Update UI
  Q_EMIT themeChanged();
}
