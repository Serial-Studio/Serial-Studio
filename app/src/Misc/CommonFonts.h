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

public:
  CommonFonts(CommonFonts &&) = delete;
  CommonFonts(const CommonFonts &) = delete;
  CommonFonts &operator=(CommonFonts &&) = delete;
  CommonFonts &operator=(const CommonFonts &) = delete;

  static CommonFonts &instance();

  [[nodiscard]] const QFont &uiFont() const;
  [[nodiscard]] const QFont &monoFont() const;
  [[nodiscard]] const QFont &boldUiFont() const;

  Q_INVOKABLE QFont customUiFont(int pixelSize = 12, bool bold = false);
  Q_INVOKABLE QFont customMonoFont(int pixelSize = 12);

private slots:
  void onLanguageChanged();

private:
  QFont m_uiFont;
  QFont m_monoFont;
  QFont m_boldUiFont;

  QString m_uiName;
  QString m_monoName;
};
} // namespace Misc
