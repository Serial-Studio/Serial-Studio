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

#include "Misc/Translator.h"
#include "Misc/CommonFonts.h"

/**
 * @brief Constructs the CommonFonts object, registering common fonts and
 *        initializing member variables.
 */
Misc::CommonFonts::CommonFonts()
{
  // Lambda function to register any number of fonts
  const auto addFonts = [](const auto &...fonts) {
    const auto addFont = [](const auto &font) {
      if (QFontDatabase::addApplicationFont(font) == -1)
        qWarning() << "Failed to load font: " << font;
    };
    (addFont(fonts), ...);
  };

  // Add common fonts to application database
  // clang-format off
  addFonts(
      QStringLiteral(":/rcc/fonts/NotoSans.ttf"),
      QStringLiteral(":/rcc/fonts/NotoSansSC.ttf"),
      QStringLiteral(":/rcc/fonts/NotoSansMono.ttf")
      );
  // clang-format on

  // Load appropiate fonts for current language
  onLanguageChanged();
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &Misc::CommonFonts::onLanguageChanged);
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
 * @brief Creates a custom UI font with specified pixel size and boldness.
 * @param pixelSize The pixel size of the font.
 * @param bold True if the font should be bold, otherwise false.
 * @return The custom UI font.
 */
QFont Misc::CommonFonts::customUiFont(const int pixelSize, const bool bold)
{
  auto weight = bold ? QStringLiteral("Bold") : QStringLiteral("Regular");
  QFont font = QFontDatabase::font(m_uiName, weight, 12);
  font.setPixelSize(qMax(1, pixelSize));
  return font;
}

/**
 * @brief Creates a custom monospace font with specified pixel size.
 * @param pixelSize The pixel size of the font.
 * @return The custom monospace font.
 */
QFont Misc::CommonFonts::customMonoFont(const int pixelSize)
{
  QFont font = QFontDatabase::font(m_monoName, QStringLiteral("Regular"), 12);
  font.setPixelSize(qMax(1, pixelSize));
  return font;
}

/**
 * @brief Updates the UI and monospace fonts when the language is changed.
 *
 * This function sets the appropriate UI and monospace font names based on the
 * current language selected in the translator. It then loads the fonts with
 * specified attributes such as style and size from the QFontDatabase.
 */
void Misc::CommonFonts::onLanguageChanged()
{
  // Obtain fonts to use for current language
  switch (Misc::Translator::instance().language())
  {
    case Misc::Translator::English:
    case Misc::Translator::Spanish:
    case Misc::Translator::German:
    case Misc::Translator::Russian:
    case Misc::Translator::French:
      m_uiName = QStringLiteral("Noto Sans");
      m_monoName = QStringLiteral("Noto Sans Mono");
      break;
    case Misc::Translator::Chinese:
      m_uiName = QStringLiteral("Noto Sans SC");
      m_monoName = QStringLiteral("Noto Sans Mono");
      break;
  }

  // Load fonts
  m_uiFont = QFontDatabase::font(m_uiName, QStringLiteral("Regular"), 12);
  m_boldUiFont = QFontDatabase::font(m_uiName, QStringLiteral("Bold"), 12);
  m_monoFont = QFontDatabase::font(m_monoName, QStringLiteral("Regular"), 12);

  // Set font properties
  m_uiFont.setPixelSize(12);
  m_monoFont.setPixelSize(12);
  m_boldUiFont.setPixelSize(12);
  m_monoFont.setStyleHint(QFont::Monospace);

  // Update both app-level font & QML-dependent fonts
  qApp->setFont(m_uiFont);
  Q_EMIT fontsChanged();
}
