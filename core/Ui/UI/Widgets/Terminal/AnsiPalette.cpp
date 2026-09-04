/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "UI/Widgets/Terminal/AnsiPalette.h"

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an unthemed palette: every base color is default-constructed and the SGR
 *        foreground/background start invalid, which the renderer reads as "use the theme
 *        default". rebuild() is what gives the base tables their values.
 */
Widgets::AnsiPalette::AnsiPalette()
  : m_foreground(), m_background(), m_brightColors{}, m_standardColors{}
{}

//--------------------------------------------------------------------------------------------------
// Current SGR colors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the foreground selected by the escape sequences seen so far; invalid when
 *        no SGR color is in effect.
 */
const QColor& Widgets::AnsiPalette::foreground() const
{
  return m_foreground;
}

/**
 * @brief Returns the background selected by the escape sequences seen so far; invalid when
 *        no SGR background is in effect.
 */
const QColor& Widgets::AnsiPalette::background() const
{
  return m_background;
}

/**
 * @brief Drops both SGR colors, as an SGR reset (code 0) does.
 */
void Widgets::AnsiPalette::resetColors()
{
  m_foreground = QColor();
  m_background = QColor();
}

/**
 * @brief Drops only the SGR foreground, leaving any active background in place; this is what
 *        a theme change or a buffer reset needs, since neither is an SGR reset.
 */
void Widgets::AnsiPalette::resetForeground()
{
  m_foreground = QColor();
}

//--------------------------------------------------------------------------------------------------
// Base palettes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns one of the eight standard (SGR 30-37) colors; the index is clamped.
 */
const QColor& Widgets::AnsiPalette::standardColor(int index) const
{
  return m_standardColors[qBound(0, index, 7)];
}

/**
 * @brief Returns one of the eight bright (SGR 90-97) colors; the index is clamped.
 */
const QColor& Widgets::AnsiPalette::brightColor(int index) const
{
  return m_brightColors[qBound(0, index, 7)];
}

/**
 * @brief Rebuilds both base palettes for the active theme, choosing the dark or light table
 *        from the relative lightness of the console text and background colors.
 */
void Widgets::AnsiPalette::rebuild(const QColor& consoleBase, const QColor& consoleText)
{
  const bool isDarkTheme = consoleText.lightness() > consoleBase.lightness();

  if (isDarkTheme) {
    m_standardColors[0] = QColor(0, 0, 0);
    m_standardColors[1] = QColor(205, 49, 49);
    m_standardColors[2] = QColor(13, 188, 121);
    m_standardColors[3] = QColor(229, 229, 16);
    m_standardColors[4] = QColor(36, 114, 200);
    m_standardColors[5] = QColor(188, 63, 188);
    m_standardColors[6] = QColor(17, 168, 205);
    m_standardColors[7] = QColor(229, 229, 229);

    m_brightColors[0] = QColor(102, 102, 102);
    m_brightColors[1] = QColor(241, 76, 76);
    m_brightColors[2] = QColor(35, 209, 139);
    m_brightColors[3] = QColor(245, 245, 67);
    m_brightColors[4] = QColor(59, 142, 234);
    m_brightColors[5] = QColor(214, 112, 214);
    m_brightColors[6] = QColor(41, 184, 219);
    m_brightColors[7] = QColor(255, 255, 255);
  }

  else {
    m_standardColors[0] = QColor(0, 0, 0);
    m_standardColors[1] = QColor(170, 0, 0);
    m_standardColors[2] = QColor(0, 140, 0);
    m_standardColors[3] = QColor(170, 140, 0);
    m_standardColors[4] = QColor(0, 0, 170);
    m_standardColors[5] = QColor(170, 0, 170);
    m_standardColors[6] = QColor(0, 140, 170);
    m_standardColors[7] = QColor(170, 170, 170);

    m_brightColors[0] = QColor(85, 85, 85);
    m_brightColors[1] = QColor(210, 0, 0);
    m_brightColors[2] = QColor(0, 170, 0);
    m_brightColors[3] = QColor(210, 170, 0);
    m_brightColors[4] = QColor(0, 0, 210);
    m_brightColors[5] = QColor(210, 0, 210);
    m_brightColors[6] = QColor(0, 170, 210);
    m_brightColors[7] = QColor(85, 85, 85);
  }
}

//--------------------------------------------------------------------------------------------------
// SGR application
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a whole SGR parameter run, letting each code consume the extended-color
 *        parameters that belong to it.
 */
void Widgets::AnsiPalette::applySgr(const QList<int>& codes, const QColor& defaultForeground)
{
  for (int i = 0; i < codes.size(); ++i)
    i += applySgrCode(codes, i, defaultForeground);
}

/**
 * @brief Applies the SGR code at @p i and returns how many further parameters it consumed,
 *        which is non-zero only for the 38/48 extended-color forms.
 */
int Widgets::AnsiPalette::applySgrCode(const QList<int>& codes,
                                       int i,
                                       const QColor& defaultForeground)
{
  SS_ASSERT(i >= 0 && i < codes.size(), return 0);

  const int code = codes[i];

  if (code == 0) {
    resetColors();
    return 0;
  }

  if (code == 1) {
    if (!m_foreground.isValid())
      m_foreground = defaultForeground;

    m_foreground = m_foreground.lighter(130);
    return 0;
  }

  if (code >= 30 && code <= 37) {
    m_foreground = m_standardColors[code - 30];
    return 0;
  }

  if (code >= 40 && code <= 47) {
    m_background = m_standardColors[code - 40];
    return 0;
  }

  if (code >= 90 && code <= 97) {
    m_foreground = m_brightColors[code - 90];
    return 0;
  }

  if (code >= 100 && code <= 107) {
    m_background = m_brightColors[code - 100];
    return 0;
  }

  const bool isFg = (code == 38);
  const bool isBg = (code == 48);
  if (!isFg && !isBg)
    return 0;

  QColor& target = isFg ? m_foreground : m_background;

  if (i + 2 < codes.size() && codes[i + 1] == 5) {
    target = indexedColor(codes[i + 2]);
    return 2;
  }

  if (i + 4 < codes.size() && codes[i + 1] == 2) {
    target = QColor(
      qBound(0, codes[i + 2], 255), qBound(0, codes[i + 3], 255), qBound(0, codes[i + 4], 255));
    return 4;
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------
// 256-color cube
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps an xterm-256 index to a color: 0-7 standard, 8-15 bright, 16-231 the 6x6x6
 *        RGB cube, 232-255 the grayscale ramp. The index arrives straight from device bytes
 *        so it is clamped rather than asserted.
 */
QColor Widgets::AnsiPalette::indexedColor(int index)
{
  const int value = qBound(0, index, 255);

  if (value < 8) {
    static const QColor standard[8] = {
      QColor(0, 0, 0),
      QColor(170, 0, 0),
      QColor(0, 170, 0),
      QColor(170, 85, 0),
      QColor(0, 0, 170),
      QColor(170, 0, 170),
      QColor(0, 170, 170),
      QColor(170, 170, 170),
    };
    return standard[value];
  }

  if (value < 16) {
    static const QColor bright[8] = {
      QColor(85, 85, 85),
      QColor(255, 85, 85),
      QColor(85, 255, 85),
      QColor(255, 255, 85),
      QColor(85, 85, 255),
      QColor(255, 85, 255),
      QColor(85, 255, 255),
      QColor(255, 255, 255),
    };
    return bright[value - 8];
  }

  if (value < 232) {
    const int adjusted = value - 16;
    const int r        = (adjusted / 36) % 6;
    const int g        = (adjusted / 6) % 6;
    const int b        = adjusted % 6;

    return QColor(r ? (r * 40 + 55) : 0, g ? (g * 40 + 55) : 0, b ? (b * 40 + 55) : 0);
  }

  const int gray = 8 + (value - 232) * 10;
  return QColor(gray, gray, gray);
}
