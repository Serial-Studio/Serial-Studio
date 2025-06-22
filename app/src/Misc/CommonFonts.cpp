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

#include <QApplication>
#include <QFontDatabase>

#include "Misc/CommonFonts.h"

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
  const auto path = QStringLiteral(":/rcc/fonts/IBMPlexMono-Regular.ttf");
  const auto id = QFontDatabase::addApplicationFont(path);
  if (id != -1)
  {
    const auto families = QFontDatabase::applicationFontFamilies(id);
    if (!families.isEmpty())
    {
      monoFont = families.at(0);
      QFontDatabase::addApplicationFont(":/rcc/fonts/IBMPlexMono-Medium.ttf");
    }
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
