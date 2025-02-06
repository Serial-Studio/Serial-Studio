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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

#pragma once

#include <QFont>
#include <QObject>

namespace Misc
{
/**
 * @class Misc::CommonFonts
 * @brief A class providing common fonts for the user interface.
 */
class CommonFonts : public QObject
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(const QFont& uiFont
             READ uiFont
             NOTIFY fontsChanged)
  Q_PROPERTY(const QFont& monoFont
             READ monoFont
             NOTIFY fontsChanged)
  Q_PROPERTY(const QFont& boldUiFont
             READ boldUiFont
             NOTIFY fontsChanged)
  // clang-format on

signals:
  void fontsChanged();

private:
  explicit CommonFonts();
  CommonFonts(CommonFonts &&) = delete;
  CommonFonts(const CommonFonts &) = delete;
  CommonFonts &operator=(CommonFonts &&) = delete;
  CommonFonts &operator=(const CommonFonts &) = delete;

public:
  static CommonFonts &instance();

  [[nodiscard]] const QFont &uiFont() const;
  [[nodiscard]] const QFont &monoFont() const;
  [[nodiscard]] const QFont &boldUiFont() const;

  Q_INVOKABLE QFont customUiFont(qreal fraction = 1, bool bold = false);
  Q_INVOKABLE QFont customMonoFont(qreal fraction = 1);

private:
  QFont m_uiFont;
  QFont m_monoFont;
  QFont m_boldUiFont;
};
} // namespace Misc
