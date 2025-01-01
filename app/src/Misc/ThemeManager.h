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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#pragma once

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QJsonObject>

/**
 * @brief Generates a styled QString using a format string and a variable number
 *        of QColor arguments.
 *
 * This function takes a QString representing a style (such as a CSS-like
 * string) and a variable number of QColor objects. It replaces placeholders in
 * the style string (e.g., %1, %2, etc.) with the hexadecimal color codes of the
 * provided QColor objects.
 *
 * The number of QColor arguments must match the number of placeholders in the
 * style string. If fewer colors are provided than placeholders, the
 * placeholders that are not matched will not be replaced. If more colors are
 * provided than placeholders, the extra colors will be ignored.
 *
 * @param style The format string that includes placeholders (e.g., %1, %2, ...)
 * @param colors A variable number of QColor objects to replace the placeholders
 *               in the format string.
 *
 * @return A QString with the placeholders replaced by the color codes.
 *
 * @note Example usage:
 * @code
 * QString styleString = QSS("background-color: %1; color: %2;", QColor("red"),
 * QColor("blue"));
 * // Result: "background-color: #ff0000; color: #0000ff;"
 * @endcode
 */
template<typename... Colors>
inline QString QSS(const QString &style, const Colors &...colors)
{
  QStringList colorNames;
  (colorNames << ... << colors.name());
  QString result = style;
  for (int i = 0; i < colorNames.size(); ++i)
  {
    result = result.arg(colorNames[i]);
  }
  return result;
}

/**
 * @class ThemeManager
 * @brief Manages the application's color themes.
 *
 * ThemeManager is a singleton class that handles the loading, switching, and
 * providing of color themes for the application.
 *
 * It reads theme configurations from JSON files and applies them as needed,
 * also responding to system color scheme changes.
 *
 * Usage:
 * - Access the singleton instance via @c ThemeManager::instance().
 * - Retrieve available themes using @c availableThemes().
 * - Change the theme using @c setTheme().
 * - Get the current theme's color scheme with @c colors().
 *
 * The class emits a @c themeChanged signal when the application theme is
 * changed, either manually or automatically in response to system palette
 * changes.
 *
 * @extends QObject
 */
namespace Misc
{
class ThemeManager : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int theme
             READ theme
             WRITE setTheme
             NOTIFY themeChanged)
  Q_PROPERTY(const QString& themeName
             READ themeName
             NOTIFY themeChanged)
  Q_PROPERTY(const QJsonObject& colors
             READ colors
             NOTIFY themeChanged)
  Q_PROPERTY(QStringList availableThemes
             READ availableThemes
             CONSTANT)
  // clang-format on

signals:
  void themeChanged();

private:
  explicit ThemeManager();

public:
  static ThemeManager &instance();
  ThemeManager(ThemeManager &&) = delete;
  ThemeManager(const ThemeManager &) = delete;
  ThemeManager &operator=(ThemeManager &&) = delete;
  ThemeManager &operator=(const ThemeManager &) = delete;

  [[nodiscard]] int theme() const;
  [[nodiscard]] const QString &themeName() const;
  [[nodiscard]] const QJsonObject &colors() const;
  [[nodiscard]] const QJsonObject &themeData() const;
  [[nodiscard]] const QStringList &availableThemes() const;
  [[nodiscard]] QColor getColor(const QString &name) const;

public slots:
  void setTheme(int index);

private:
  int m_theme;
  QString m_themeName;

  QSettings m_settings;
  QJsonObject m_colors;
  QJsonObject m_themeData;

  QStringList m_availableThemes;
  QMap<QString, QJsonObject> m_themes;
};
} // namespace Misc
