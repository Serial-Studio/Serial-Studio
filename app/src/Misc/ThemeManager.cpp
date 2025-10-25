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
#include <QJsonArray>
#include <QStyleHints>
#include <QJsonDocument>
#include <QGuiApplication>

#include "Misc/Translator.h"

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Converts a QJsonObject to a QVariantMap.
 *
 * Iterates over all key-value pairs in the input QJsonObject and
 * inserts them into a QVariantMap.
 *
 * @param obj The QJsonObject to convert.
 * @return A QVariantMap with the same keys as the input object and
 *         QVariant-converted values.
 */
static QVariantMap jsonObjectToVariantMap(const QJsonObject &obj)
{
  QVariantMap map;
  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    map.insert(it.key(), it.value().toVariant());

  return map;
}

/**
 * @brief Extracts a vector of QColor objects from a JSON object.
 *
 * Reads the array at the "widget_colors" key from the input QJsonObject.
 * For each string in the array, constructs a QColor and adds it to the
 * result vector. Non-string entries are ignored.
 *
 * @return QVector<QColor> containing a QColor for each valid string in
 *                         the "widget_colors" array.
 */
static QVector<QColor> extractWidgetColors(const QJsonObject &colorsObject)
{
  QVector<QColor> result;
  const QJsonArray array = colorsObject.value("widget_colors").toArray();
  result.reserve(array.size());

  for (const auto &val : array)
  {
    if (val.isString())
      result.append(QColor(val.toString()));
  }

  return result;
}

//------------------------------------------------------------------------------
// Constructor & singleton access
//------------------------------------------------------------------------------

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
      QStringLiteral("rust"),
      QStringLiteral("gunmetal"),
      QStringLiteral("midday"),
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

//------------------------------------------------------------------------------
// Class member access functions
//------------------------------------------------------------------------------

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
 * @brief Returns the current theme's color map.
 *
 * The map contains key-value pairs where each key is a color role and the value
 * is its color, typically represented as a hex string (e.g., "#RRGGBB").
 *
 * @return const reference to a QVariantMap of color definitions.
 */
const QVariantMap &Misc::ThemeManager::colors() const
{
  return m_colors;
}

/**
 * @brief Returns the current theme's parameter map.
 *
 * Theme parameters may include additional metadata like icon paths or
 * editor theme identifiers used to configure the application appearance.
 *
 * @return const reference to a QVariantMap of theme parameters.
 */
const QVariantMap &Misc::ThemeManager::parameters() const
{
  return m_parameters;
}

/**
 * @brief Returns the list of widget accent colors defined in the current theme.
 *
 * This corresponds to the "widget_colors" array in the theme's "colors"
 * section. These colors are typically used for dynamic UI elements such as
 * highlights, indicators, or charts where a sequence of accent colors is
 * required.
 *
 * @return const reference to a QVector of QColor objects representing the
 * widget colors.
 */
const QVector<QColor> &Misc::ThemeManager::widgetColors() const
{
  return m_widgetColors;
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

//------------------------------------------------------------------------------
// Theme loading
//------------------------------------------------------------------------------

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
  auto data = m_themes.value(m_themeName);
  m_colors = jsonObjectToVariantMap(data.value("colors").toObject());
  m_widgetColors = extractWidgetColors(data.value("colors").toObject());
  m_parameters = jsonObjectToVariantMap(data.value("parameters").toObject());

  // Hint Qt about the effective color scheme and update user interface
  QMetaObject::invokeMethod(
      this,
      [this]() {
        const auto bg = getColor(QStringLiteral("base"));
        const auto fg = getColor(QStringLiteral("text"));
        if (fg.lightness() > bg.lightness())
          qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
        else
          qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);

        Q_EMIT themeChanged();
      },
      Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
// Automatic theme detection based on system theme
//------------------------------------------------------------------------------

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
  const auto data = m_themes.value(resolved);

  // Set theme data
  m_themeName = QStringLiteral("System");
  m_theme = m_availableThemes.indexOf(m_themeName);
  m_colors = jsonObjectToVariantMap(data.value("colors").toObject());
  m_widgetColors = extractWidgetColors(data.value("colors").toObject());
  m_parameters = jsonObjectToVariantMap(data.value("parameters").toObject());

  // Update user interface
  QMetaObject::invokeMethod(
      this, [this]() { Q_EMIT themeChanged(); }, Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
// i18n utilities
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// Event filter for detecting OS theme changes
//------------------------------------------------------------------------------

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
