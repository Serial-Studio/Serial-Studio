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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QApplication>
#include <QFontDatabase>

#include "Misc/CommonFonts.h"

/**
 * @brief Constructs the CommonFonts object, registering common fonts and
 *        initializing member variables.
 */
Misc::CommonFonts::CommonFonts()
{
  // Platform-specific font selection
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
  m_monoFont.setPointSizeF(m_uiFont.pointSizeF());
  m_monoFont.setStyleHint(QFont::Monospace);

  // Verify that the font was loaded correctly
  if (m_monoFont.family() != monoFont)
    m_monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

/**
 * @brief Provides the instance of CommonFonts.
 * @return A reference to the singleton instance of CommonFonts.
 */
Misc::CommonFonts &Misc::CommonFonts::instance()
{
  static CommonFonts instance;
  return instance;
}

/**
 * @brief Retrieves the UI font.
 * @return The UI font.
 */
const QFont &Misc::CommonFonts::uiFont() const
{
  return m_uiFont;
}

/**
 * @brief Retrieves the monospace font.
 * @return The monospace font.
 */
const QFont &Misc::CommonFonts::monoFont() const
{
  return m_monoFont;
}

/**
 * @brief Retrieves the bold UI font.
 * @return The bold UI font.
 */
const QFont &Misc::CommonFonts::boldUiFont() const
{
  return m_boldUiFont;
}

/**
 * @brief Creates a custom UI font with specified size and boldness.
 * @param fraction The fractional size of the font (1=100% of normal font size)
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
 * @param fraction The fractional size of the font (1=100% of normal font size)
 * @return The custom monospace font.
 */
QFont Misc::CommonFonts::customMonoFont(const double fraction)
{
  QFont font = m_monoFont;
  font.setPointSizeF(m_monoFont.pointSizeF() * qMax(0.1, fraction));
  return font;
}
