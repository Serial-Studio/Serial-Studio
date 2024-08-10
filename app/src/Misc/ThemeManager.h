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

#ifndef _UTILITIES_THEME_MANAGER_H
#define _UTILITIES_THEME_MANAGER_H

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QJsonObject>

inline QString QSS(const QString &style, const QColor &color)
{
  return QString(style).arg(color.name());
}

inline QString QSS(const QString &style, const QColor &c1, const QColor &c2)
{
  return QString(style).arg(c1.name(), c2.name());
}

inline QString QSS(const QString &style, const QColor &c1, const QColor &c2,
                   const QColor &c3)
{
  return QString(style).arg(c1.name(), c2.name(), c3.name());
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

Q_SIGNALS:
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
  [[nodiscard]] QStringList availableThemes() const;
  [[nodiscard]] QColor getColor(const QString &name) const;

public Q_SLOTS:
  void setTheme(int index);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  int m_theme;
  QString m_themeName;

  QSettings m_settings;
  QJsonObject m_colors;
  QMap<QString, QJsonObject> m_themes;
};
} // namespace Misc

#endif
