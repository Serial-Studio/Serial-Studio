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

#include "Misc/CommonFonts.h"

#include <QApplication>
#include <QFontDatabase>

/**
 * @brief Constructs the CommonFonts object, registering common fonts and
 *        initializing member variables.
 */
Misc::CommonFonts::CommonFonts()
{
  // Set fallback mono font to system font
  QString monoFont;
#ifdef Q_OS_LINUX
  monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
#elif defined(Q_OS_MAC)
  monoFont = QStringLiteral("Menlo");
#elif defined(Q_OS_WIN)
  monoFont = QStringLiteral("Consolas");
#else
  monoFont = QStringLiteral("Courier");
#endif

  // Add custom mono font
  const auto path = QStringLiteral(":/rcc/fonts/JetBrainsMono-Regular.ttf");
  const auto id   = QFontDatabase::addApplicationFont(path);
  if (id != -1) {
    const auto families = QFontDatabase::applicationFontFamilies(id);
    if (!families.isEmpty())
      monoFont = families.at(0);
  }

  // Set the UI font to the system default
  m_uiFont = QApplication::font();

  // Configure bold font
  m_boldUiFont = m_uiFont;
#ifdef Q_OS_LINUX
  m_boldUiFont.setWeight(QFont::DemiBold);
#else
  m_boldUiFont.setBold(true);
#endif

  // Setup monospace font
  m_monoFont = QFont(monoFont);
  m_monoFont.setFixedPitch(true);
  m_monoFont.setStyleHint(QFont::Monospace);
  m_monoFont.setPointSizeF(m_uiFont.pointSizeF());

  // Verify that the font was loaded correctly
  if (m_monoFont.family() != monoFont)
    m_monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  // Load widget font settings (default family is the monospace font on first run)
  m_widgetFontFamily = m_settings.value("Widgets/FontFamily", m_monoFont.family()).toString();
  m_widgetFontScale  = m_settings.value("Widgets/FontScale", kScaleNormal).toDouble();
  m_widgetFontScale  = qBound(0.5, m_widgetFontScale, 3.0);
  m_widgetFontIndex  = availableFonts().indexOf(m_widgetFontFamily);
}

/**
 * @brief Provides the instance of CommonFonts.
 * @return A reference to the singleton instance of CommonFonts.
 */
Misc::CommonFonts& Misc::CommonFonts::instance()
{
  static CommonFonts instance;
  return instance;
}

/**
 * @brief Retrieves the UI font.
 * @return The UI font.
 */
const QFont& Misc::CommonFonts::uiFont() const
{
  return m_uiFont;
}

/**
 * @brief Retrieves the monospace font.
 * @return The monospace font.
 */
const QFont& Misc::CommonFonts::monoFont() const
{
  return m_monoFont;
}

/**
 * @brief Retrieves the bold UI font.
 * @return The bold UI font.
 */
const QFont& Misc::CommonFonts::boldUiFont() const
{
  return m_boldUiFont;
}

/**
 * @brief Creates a custom UI font with specified size and boldness.
 * @param fraction The fractional size of the font (1=100% default size)
 * @param bold True if the font should be bold, otherwise false.
 * @return The custom UI font.
 */
QFont Misc::CommonFonts::customUiFont(const double fraction, const bool bold)
{
  QFont font = bold ? m_boldUiFont : m_uiFont;
  font.setPointSizeF(m_uiFont.pointSizeF() * qMax(0.1, fraction));
  return font;
}

/**
 * @brief Creates a custom monospace font with specified size.
 * @param fraction The fractional size of the font (1=100% default size)
 * @param bold True if the font should be bold, otherwise false.
 * @return The custom monospace font.
 */
QFont Misc::CommonFonts::customMonoFont(const double fraction, const bool bold)
{
  QFont font = m_monoFont;
  font.setPointSizeF(m_monoFont.pointSizeF() * qMax(0.1, fraction));
  if (bold)
    font.setWeight(QFont::Medium);

  return font;
}

/**
 * @brief Returns the current widget font scale factor.
 */
double Misc::CommonFonts::widgetFontScale() const
{
  return m_widgetFontScale;
}

/**
 * @brief Returns the current widget font family name.
 */
QString Misc::CommonFonts::widgetFontFamily() const
{
  return m_widgetFontFamily;
}

/**
 * @brief Returns a monotonically incrementing counter that changes whenever
 *        the widget font scale or family is updated. QML bindings that call
 *        widgetFont() should read this property so the binding engine re-evaluates
 *        them on each font change:
 *        @code
 *        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))
 *        @endcode
 */
int Misc::CommonFonts::widgetFontRevision() const
{
  return m_widgetFontRevision;
}

/**
 * @brief Returns the index of the current widget font family in availableFonts().
 */
int Misc::CommonFonts::widgetFontIndex() const
{
  return m_widgetFontIndex;
}

/**
 * @brief Returns all available font families, sorted alphabetically.
 *
 * Private/alias families (e.g. ".AppleSystemUIFont" on macOS) are excluded.
 * Qt 6 marks these via QFontDatabase::isPrivateFamily(); as a second guard,
 * any family whose name starts with '.' is also filtered out.
 */
QStringList Misc::CommonFonts::availableFonts() const
{
  QStringList families;
  for (const QString& family : QFontDatabase::families()) {
    if (family.startsWith(QLatin1Char('.')))
      continue;

    if (QFontDatabase::isPrivateFamily(family))
      continue;

    families.append(family);
  }

  families.sort(Qt::CaseInsensitive);

  // Pin the default widget font (monoFont) to the top of the list
  const QString mono = m_monoFont.family();
  families.removeAll(mono);
  families.prepend(mono);

  return families;
}

/**
 * @brief Builds a widget font scaled by fraction and the global widget scale.
 * @param fraction Additional fraction multiplier (relative to UI font size)
 * @param bold     If true, applies medium weight
 * @return A QFont with the widget family and scaled point size
 */
QFont Misc::CommonFonts::widgetFont(const double fraction, const bool bold) const
{
  QFont font(m_widgetFontFamily);
  font.setPointSizeF(m_uiFont.pointSizeF() * qMax(0.1, fraction) * m_widgetFontScale);
  if (bold)
    font.setWeight(QFont::Medium);

  return font;
}

/**
 * @brief Sets the global widget font scale and persists it to QSettings.
 * @param scale Scale factor, clamped to [0.5, 3.0]
 */
void Misc::CommonFonts::setWidgetFontScale(const double scale)
{
  const double clamped = qBound(0.5, scale, 3.0);
  if (qFuzzyCompare(m_widgetFontScale, clamped))
    return;

  m_widgetFontScale = clamped;
  ++m_widgetFontRevision;
  m_settings.setValue("Widgets/FontScale", m_widgetFontScale);
  Q_EMIT fontsChanged();
}

/**
 * @brief Sets the global widget font family and persists it to QSettings.
 * @param family Font family name
 */
void Misc::CommonFonts::setWidgetFontFamily(const QString& family)
{
  if (m_widgetFontFamily == family)
    return;

  m_widgetFontFamily = family;
  m_widgetFontIndex  = availableFonts().indexOf(m_widgetFontFamily);
  ++m_widgetFontRevision;
  m_settings.setValue("Widgets/FontFamily", m_widgetFontFamily);
  Q_EMIT fontsChanged();
}
