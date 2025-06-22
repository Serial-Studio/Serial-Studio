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

  Q_INVOKABLE QFont customUiFont(double fraction = 1, bool bold = false);
  Q_INVOKABLE QFont customMonoFont(double fraction = 1, bool bold = false);

private:
  QFont m_uiFont;
  QFont m_monoFont;
  QFont m_boldUiFont;
};
} // namespace Misc
