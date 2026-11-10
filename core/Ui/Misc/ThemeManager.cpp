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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "ThemeManager.h"

#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QJsonArray>
#include <QPalette>
#include <QStyleHints>
#include <QTimer>

#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/Translator.h"
#include "Core/WorkspaceManager.h"
#include "Misc/ExtensionManager.h"
#include "Misc/ThemeSectionCheck.h"

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief How many themes were bundled while the selection was still stored as a list index. Every
 *        theme bundled since sits after them, so a stored index at or past this count moves up by
 *        that many: without it, a legacy "System" or extension choice would open as a new theme,
 *        which spec 0113 R11 forbids.
 */
static constexpr int kLegacyBuiltInThemes = 3;

/**
 * @brief Converts a QJsonObject to a QVariantMap.
 */
static QVariantMap jsonObjectToVariantMap(const QJsonObject& obj)
{
  QVariantMap map;
  for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    map.insert(it.key(), it.value().toVariant());

  if (map.contains("start-icon")) {
    auto str          = map["start-icon"].toString();
    str               = str.replace("/rcc/", "/");
    map["start-icon"] = str;
  }

  return map;
}

/**
 * @brief Extracts a vector of QColor objects from a JSON object.
 */
static QVector<QColor> extractWidgetColors(const QJsonObject& colorsObject)
{
  QVector<QColor> result;
  const QJsonArray array = colorsObject.value("widget_colors").toArray();
  result.reserve(array.size());

  for (const auto& val : array)
    if (val.isString())
      result.append(QColor(val.toString()));

  return result;
}

/**
 * @brief Extracts device color gradient pairs from a JSON object.
 */
static QVector<QPair<QColor, QColor>> extractDeviceColors(const QJsonObject& colorsObject)
{
  QVector<QPair<QColor, QColor>> result;
  const QJsonArray array = colorsObject.value("device_colors").toArray();
  result.reserve(array.size());

  for (const auto& val : array) {
    if (!val.isObject())
      continue;

    const auto obj = val.toObject();
    result.append({QColor(obj.value("top").toString()), QColor(obj.value("bottom").toString())});
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the ThemeManager object and initializes theme loading.
 */
Misc::ThemeManager::ThemeManager()
  : m_theme(0)
  , m_colorMap(QQmlPropertyMap::create(this))
  , m_applyingTheme(false)
  , m_persistSettings(true)
{
  m_catalog.loadBuiltInThemes();
  loadUserThemes();

  int themeIndex       = 0;
  const auto savedName = m_settings.value("ApplicationThemeName").toString();
  if (!savedName.isEmpty()) {
    const int idx = m_catalog.indexOf(savedName);
    themeIndex    = (idx >= 0) ? idx : 0;
  }

  else {
    themeIndex = m_settings.value("ApplicationTheme", 0).toInt();
    if (themeIndex >= kLegacyBuiltInThemes)
      themeIndex += ThemeCatalog::builtInCount() - kLegacyBuiltInThemes;

    if (themeIndex < 0 || themeIndex >= m_catalog.count())
      themeIndex = 0;

    m_settings.remove("ApplicationTheme");
  }

  setTheme(themeIndex);

  updateLocalizedThemeNames();
  auto& translator = Core::services().translator;
  connect(&translator,
          &Misc::Translator::languageChanged,
          this,
          &Misc::ThemeManager::updateLocalizedThemeNames);

  qApp->installEventFilter(this);
}

/**
 * @brief Provides a reference to the singleton instance of the ThemeManager.
 */
Misc::ThemeManager& Misc::ThemeManager::instance()
{
  static ThemeManager instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Class member access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Retrieves the current theme index.
 */
int Misc::ThemeManager::theme() const
{
  return m_theme;
}

/**
 * @brief Returns whether the loaded theme is dark, by the same convention the color-scheme
 *        hint uses: text lighter than base. Holds for user themes, not just the bundled ones.
 */
bool Misc::ThemeManager::isDarkTheme() const
{
  return getColor(QStringLiteral("text")).lightness()
       > getColor(QStringLiteral("base")).lightness();
}

/**
 * @brief Retrieves the current name of the loaded theme.
 */
const QString& Misc::ThemeManager::themeName() const
{
  return m_themeName;
}

/**
 * @brief Returns the current theme's color map.
 */
const QVariantMap& Misc::ThemeManager::colors() const
{
  return m_colors;
}

/**
 * @brief Returns the per-key color map QML binds to. Reading one color through it costs one
 *        property lookup and re-evaluates on that key alone; the QVariantMap property it
 *        replaced converted all 118 entries on every read, for every one of the ~2700 bindings
 *        that read a color (G2).
 */
QQmlPropertyMap* Misc::ThemeManager::colorMap()
{
  return m_colorMap;
}

/**
 * @brief Returns the current theme's parameter map.
 */
const QVariantMap& Misc::ThemeManager::parameters() const
{
  return m_parameters;
}

/**
 * @brief Returns the theme's lettering roles (spec 0113), empty for a colour-only theme.
 */
const QVariantMap& Misc::ThemeManager::fonts() const
{
  return m_fonts;
}

/**
 * @brief Returns the theme's named materials: a recipe plus its parameters, by name.
 */
const QVariantMap& Misc::ThemeManager::materials() const
{
  return m_materials;
}

/**
 * @brief Returns which material each part of the window is made of, by surface name.
 */
const QVariantMap& Misc::ThemeManager::surfaces() const
{
  return m_surfaces;
}

/**
 * @brief Returns the theme's per-control style rules, by control class.
 */
const QVariantMap& Misc::ThemeManager::styles() const
{
  return m_styles;
}

/**
 * @brief Returns the looks the theme selects for the LED panel ("led") and the output controls
 *        ("output"); an absent key means the look every theme had before spec 0113.
 */
const QVariantMap& Misc::ThemeManager::widgetLooks() const
{
  return m_widgetLooks;
}

/**
 * @brief Returns the list of widget accent colors defined in the current theme.
 */
const QVector<QColor>& Misc::ThemeManager::widgetColors() const
{
  return m_widgetColors;
}

/**
 * @brief Returns the list of per-device caption gradient color pairs.
 */
const QVector<QPair<QColor, QColor>>& Misc::ThemeManager::deviceColors() const
{
  return m_deviceColors;
}

/**
 * @brief Returns a list of theme names that are available.
 */
const QStringList& Misc::ThemeManager::availableThemes() const
{
  return m_catalog.localizedNames();
}

/**
 * @brief Returns a @c QColor object for the given component @a name.
 */
QColor Misc::ThemeManager::getColor(const QString& name) const
{
  if (colors().contains(name))
    return QColor(colors()[name].toString());

  return QColor(qRgb(0xff, 0x00, 0xff));
}

/**
 * @brief Returns the theme colour for an AlarmSeverity tier (0=Info, 1=Ok, 2=Warning, 3=Critical).
 */
QColor Misc::ThemeManager::alarmColorForSeverity(int severity) const
{
  switch (severity) {
    case 0:
      return getColor(QStringLiteral("alarm_info"));
    case 1:
      return getColor(QStringLiteral("alarm_ok"));
    case 3:
      return getColor(QStringLiteral("alarm_critical"));
    default:
      return getColor(QStringLiteral("alarm_warning"));
  }
}

/**
 * @brief Returns @p base dressed in the theme's lettering role @p role: its family at its one
 *        weight, a size scale, capitals, letter spacing. The base family stays second so a
 *        script the typeface lacks falls back to it (spec 0113 R10); a theme without the role
 *        returns @p base untouched, which keeps the older themes identical.
 */
QFont Misc::ThemeManager::lettering(const QString& role, const QFont& base) const
{
  const auto rule = m_fonts.value(role).toMap();
  if (rule.isEmpty())
    return base;

  QFont font(base);
  const auto family = rule.value(QStringLiteral("family")).toString();
  if (!family.isEmpty() && QFontDatabase::hasFamily(family)) {
    font.setFamilies(QStringList{family, base.family()});
    font.setWeight(QFont::Normal);
  }

  bool scaled        = false;
  const double asked = SerialStudio::toDouble(rule.value(QStringLiteral("scale")), &scaled);
  const double scale = scaled ? qBound(0.5, asked, 2.0) : 1.0;
  if (font.pixelSize() > 0)
    font.setPixelSize(qMax(1, qRound(font.pixelSize() * scale)));
  else if (font.pointSizeF() > 0)
    font.setPointSizeF(font.pointSizeF() * scale);

  if (rule.value(QStringLiteral("capitals")).toBool())
    font.setCapitalization(QFont::AllUppercase);

  const double spacing = SerialStudio::toDouble(rule.value(QStringLiteral("spacing")));
  if (!qFuzzyIsNull(spacing))
    font.setLetterSpacing(QFont::AbsoluteSpacing, qBound(-2.0, spacing, 8.0));

  return font;
}

//--------------------------------------------------------------------------------------------------
// Theme loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the optional sections of a material theme out of @p data. A section the file
 *        does not carry becomes an empty map, so switching from a material theme back to a
 *        colour-only one clears every material, rule and look.
 */
void Misc::ThemeManager::applyThemeSections(const QJsonObject& data)
{
  m_fonts       = data.value(QStringLiteral("fonts")).toObject().toVariantMap();
  m_materials   = data.value(QStringLiteral("materials")).toObject().toVariantMap();
  m_surfaces    = data.value(QStringLiteral("surfaces")).toObject().toVariantMap();
  m_styles      = data.value(QStringLiteral("styles")).toObject().toVariantMap();
  m_widgetLooks = data.value(QStringLiteral("widgets")).toObject().toVariantMap();

  const auto unknown = unknownThemeNames(data);
  for (const auto& message : unknown)
    qWarning().noquote() << "[Theme]" << m_themeName << message << "; ignored";
}

/**
 * @brief Sets the current theme to the theme at the specified index.
 */
void Misc::ThemeManager::setTheme(const int index)
{
  int filteredIndex = index;
  if (index < 0 || index >= m_catalog.count())
    filteredIndex = 0;

  m_theme     = filteredIndex;
  m_themeName = m_catalog.titleAt(filteredIndex);
  if (m_persistSettings)
    m_settings.setValue("ApplicationThemeName", m_themeName);

  if (m_themeName == ThemeCatalog::systemTitle()) {
    loadSystemTheme();
    return;
  }

  m_applyingTheme = true;

  auto data      = m_catalog.theme(m_themeName);
  m_colors       = jsonObjectToVariantMap(data.value("colors").toObject());
  m_widgetColors = extractWidgetColors(data.value("colors").toObject());
  m_deviceColors = extractDeviceColors(data.value("colors").toObject());
  m_parameters   = jsonObjectToVariantMap(data.value("parameters").toObject());
  applyThemeSections(data);
  Misc::syncColorMap(*m_colorMap, m_colors);

  m_palette.setColor(QPalette::Mid, getColor("mid"));
  m_palette.setColor(QPalette::Dark, getColor("dark"));
  m_palette.setColor(QPalette::Text, getColor("text"));
  m_palette.setColor(QPalette::Base, getColor("base"));
  m_palette.setColor(QPalette::Link, getColor("link"));
  m_palette.setColor(QPalette::Light, getColor("light"));
  m_palette.setColor(QPalette::Window, getColor("window"));
  m_palette.setColor(QPalette::Shadow, getColor("shadow"));
  m_palette.setColor(QPalette::Accent, getColor("accent"));
  m_palette.setColor(QPalette::Button, getColor("button"));
  m_palette.setColor(QPalette::Midlight, getColor("midlight"));
  m_palette.setColor(QPalette::Highlight, getColor("highlight"));
  m_palette.setColor(QPalette::WindowText, getColor("window_text"));
  m_palette.setColor(QPalette::BrightText, getColor("bright_text"));
  m_palette.setColor(QPalette::ButtonText, getColor("button_text"));
  m_palette.setColor(QPalette::ToolTipBase, getColor("tooltip_base"));
  m_palette.setColor(QPalette::ToolTipText, getColor("tooltip_text"));
  m_palette.setColor(QPalette::LinkVisited, getColor("link_visited"));
  m_palette.setColor(QPalette::AlternateBase, getColor("alternate_base"));
  m_palette.setColor(QPalette::PlaceholderText, getColor("placeholder_text"));
  m_palette.setColor(QPalette::HighlightedText, getColor("highlighted_text"));

  Q_EMIT themeChanged();

  const auto palette = m_palette;
  const auto bg      = getColor(QStringLiteral("base"));
  const auto fg      = getColor(QStringLiteral("text"));
  QTimer::singleShot(0, this, [this, palette, bg, fg]() {
    qApp->setPalette(palette);
    if (fg.lightness() > bg.lightness())
      qApp->styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    else
      qApp->styleHints()->setColorScheme(Qt::ColorScheme::Light);

    m_applyingTheme = false;
  });
}

/**
 * @brief Toggles whether theme changes get written to QSettings.
 */
void Misc::ThemeManager::setSettingsPersistent(const bool persistent)
{
  m_persistSettings = persistent;
}

//--------------------------------------------------------------------------------------------------
// Automatic theme detection based on system theme
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the system-resolved theme (Light or Dark) without changing the selected theme
 * index.
 */
void Misc::ThemeManager::loadSystemTheme()
{
  m_applyingTheme = true;

  qApp->setPalette(QPalette());
  qApp->styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
  const auto scheme = qApp->styleHints()->colorScheme();

  QString resolved;
  if (scheme == Qt::ColorScheme::Dark)
    resolved = QStringLiteral("Fluent Dark");
  else if (scheme == Qt::ColorScheme::Light)
    resolved = QStringLiteral("Fluent Light");
  else
    resolved = QStringLiteral("Fluent Light");

  const auto data = m_catalog.theme(resolved);

  m_themeName    = ThemeCatalog::systemTitle();
  m_theme        = m_catalog.indexOf(m_themeName);
  m_colors       = jsonObjectToVariantMap(data.value("colors").toObject());
  m_widgetColors = extractWidgetColors(data.value("colors").toObject());
  m_deviceColors = extractDeviceColors(data.value("colors").toObject());
  m_parameters   = jsonObjectToVariantMap(data.value("parameters").toObject());
  applyThemeSections(data);
  Misc::syncColorMap(*m_colorMap, m_colors);

  Q_EMIT themeChanged();

  QTimer::singleShot(0, this, [this]() { m_applyingTheme = false; });
}

//--------------------------------------------------------------------------------------------------
// i18n utilities
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the localized names of available themes based on current UI language.
 */
void Misc::ThemeManager::updateLocalizedThemeNames()
{
  auto& translator = Core::services().translator;
  m_catalog.updateLocalizedNames(translator.language());
  Q_EMIT languageChanged();
}

//--------------------------------------------------------------------------------------------------
// Event filter for detecting OS theme changes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Event filter to intercept application-wide events.
 */

bool Misc::ThemeManager::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::ApplicationPaletteChange
      && m_themeName == ThemeCatalog::systemTitle() && !m_applyingTheme) {
    loadSystemTheme();
    return true;
  }

  return QObject::eventFilter(watched, event);
}

//--------------------------------------------------------------------------------------------------
// User addon theme support
//--------------------------------------------------------------------------------------------------

/**
 * @brief Scans the user addons directory for installed theme JSON files.
 */
void Misc::ThemeManager::loadUserThemes()
{
  auto& workspaceManager = Core::services().workspaceManager;
  m_catalog.reloadUserThemes(workspaceManager.path("Extensions/theme"));
}

/**
 * @brief Reloads user themes when a new addon is installed.
 */
void Misc::ThemeManager::onExtensionInstalled(const QString& id)
{
  const auto previousUserThemes = m_catalog.userThemes();

  loadUserThemes();

  static auto& ext   = Misc::ExtensionManager::instance();
  const auto info    = ext.selectedExtension();
  const bool isTheme = id.isEmpty() || info.value("type").toString() == QStringLiteral("theme");

  if (isTheme) {
    const auto userThemes = m_catalog.userThemes();
    for (const auto& name : std::as_const(userThemes)) {
      if (previousUserThemes.contains(name))
        continue;

      const int idx = m_catalog.indexOf(name);
      if (idx < 0)
        continue;

      setTheme(idx);
      updateLocalizedThemeNames();
      Q_EMIT languageChanged();
      return;
    }
  }

  const int idx = m_catalog.indexOf(m_themeName);
  if (idx >= 0)
    m_theme = idx;

  updateLocalizedThemeNames();
  Q_EMIT languageChanged();
}

/**
 * @brief Reloads user themes when an addon is uninstalled.
 */
void Misc::ThemeManager::onExtensionUninstalled(const QString& id)
{
  Q_UNUSED(id)
  const auto currentName = m_themeName;

  loadUserThemes();

  const int idx = m_catalog.indexOf(currentName);
  if (idx >= 0)
    m_theme = idx;
  else
    setTheme(0);

  updateLocalizedThemeNames();
  Q_EMIT languageChanged();
}

/**
 * @brief Reloads user themes from the new workspace directory after the user relocates it, so the
 *        theme list reflects the current folder instead of the one present at construction.
 */
void Misc::ThemeManager::onWorkspacePathChanged()
{
  onExtensionUninstalled(QString());
}
