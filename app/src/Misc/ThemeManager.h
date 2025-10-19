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

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QJsonObject>

//------------------------------------------------------------------------------
// Qt Stylesheet utilties...not used on Serial Studio, but useful reference...
//------------------------------------------------------------------------------

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
    result = result.arg(colorNames[i]);

  return result;
}

//------------------------------------------------------------------------------
// ThemeManager class declaration
//------------------------------------------------------------------------------

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
  Q_PROPERTY(QString themeName
             READ themeName
             NOTIFY themeChanged)
  Q_PROPERTY(QVariantMap colors
             READ colors
             NOTIFY themeChanged)
  Q_PROPERTY(QVariantMap parameters
             READ parameters
             NOTIFY themeChanged)
  Q_PROPERTY(QStringList availableThemes
             READ availableThemes
             NOTIFY languageChanged)
  // clang-format on

signals:
  void themeChanged();
  void languageChanged();

private:
  explicit ThemeManager();
  ThemeManager(ThemeManager &&) = delete;
  ThemeManager(const ThemeManager &) = delete;
  ThemeManager &operator=(ThemeManager &&) = delete;
  ThemeManager &operator=(const ThemeManager &) = delete;

public:
  static ThemeManager &instance();

  [[nodiscard]] int theme() const;
  [[nodiscard]] const QString &themeName() const;
  [[nodiscard]] const QVariantMap &colors() const;
  [[nodiscard]] const QVariantMap &parameters() const;
  [[nodiscard]] const QVector<QColor> &widgetColors() const;

  [[nodiscard]] const QStringList &availableThemes() const;
  [[nodiscard]] QColor getColor(const QString &name) const;

public slots:
  void setTheme(int index);

private slots:
  void loadSystemTheme();
  void updateLocalizedThemeNames();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  int m_theme;
  QString m_themeName;
  QSettings m_settings;
  QVariantMap m_colors;
  QVariantMap m_parameters;

  QStringList m_availableThemes;
  QStringList m_availableThemeNames;
  QMap<QString, QJsonObject> m_themes;

  QVector<QColor> m_widgetColors;
};
} // namespace Misc
