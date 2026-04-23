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
#include <QJsonObject>
#include <QObject>
#include <QPalette>
#include <QSettings>

template<typename... Colors>
inline QString QSS(const QString& style, const Colors&... colors)
{
  QStringList colorNames;
  (colorNames << ... << colors.name());
  QString result = style;
  for (int i = 0; i < colorNames.size(); ++i)
    result = result.arg(colorNames[i]);

  return result;
}

namespace Misc {
/**
 * @brief Manages the application's color themes.
 */
class ThemeManager : public QObject {
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
  ThemeManager(ThemeManager&&)                 = delete;
  ThemeManager(const ThemeManager&)            = delete;
  ThemeManager& operator=(ThemeManager&&)      = delete;
  ThemeManager& operator=(const ThemeManager&) = delete;

public:
  [[nodiscard]] static ThemeManager& instance();

  [[nodiscard]] int theme() const;
  [[nodiscard]] const QString& themeName() const;
  [[nodiscard]] const QVariantMap& colors() const;
  [[nodiscard]] const QVariantMap& parameters() const;
  [[nodiscard]] const QVector<QColor>& widgetColors() const;
  [[nodiscard]] const QVector<QPair<QColor, QColor>>& deviceColors() const;

  [[nodiscard]] const QStringList& availableThemes() const;
  [[nodiscard]] QColor getColor(const QString& name) const;

  [[nodiscard]] const QPalette& palette() const { return m_palette; }

public slots:
  void setTheme(int index);
  void onExtensionInstalled(const QString& id);
  void onExtensionUninstalled(const QString& id);

private slots:
  void loadUserThemes();
  void loadSystemTheme();
  void updateLocalizedThemeNames();

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  int m_theme;
  QPalette m_palette;
  QString m_themeName;
  QSettings m_settings;
  QVariantMap m_colors;
  QVariantMap m_parameters;

  QStringList m_availableThemes;
  QStringList m_availableThemeNames;
  QStringList m_userThemeNames;
  QMap<QString, QJsonObject> m_themes;

  bool m_applyingTheme;
  QVector<QColor> m_widgetColors;
  QVector<QPair<QColor, QColor>> m_deviceColors;
};
}  // namespace Misc
