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

#include <QDebug>
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
#ifdef Q_OS_LINUX
  m_monoName = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
#elif defined(Q_OS_MAC)
  m_monoName = QStringLiteral("Menlo");
#elif defined(Q_OS_WIN)
  m_monoName = QStringLiteral("Consolas");
#else
  m_monoName = QStringLiteral("Courier");
#endif

  // Set the UI font to the system default
  m_uiFont = QApplication::font();
  m_boldUiFont = m_uiFont;
  m_boldUiFont.setBold(true);

  // Setup monospace font
  m_monoFont = QFont(m_monoName);
  m_monoFont.setPointSizeF(m_uiFont.pointSizeF());
  m_monoFont.setStyleHint(QFont::Monospace);

  // Verify that the font was loaded correctly
  if (m_monoFont.family() != m_monoName)
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
QFont Misc::CommonFonts::customUiFont(const qreal fraction, const bool bold)
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
QFont Misc::CommonFonts::customMonoFont(const qreal fraction)
{
  QFont font = QFontDatabase::font(m_monoName, QStringLiteral("Regular"), 12);
  font.setPointSizeF(m_monoFont.pointSizeF() * qMax(0.1, fraction));
  return font;
}
