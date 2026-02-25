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
#include <QSettings>
#include <QStringList>

namespace Misc {
/**
 * @class Misc::CommonFonts
 * @brief Centralized font management for consistent typography across the
 *        application.
 *
 * The CommonFonts class provides a singleton interface for accessing predefined
 * fonts used throughout the Serial Studio UI. It ensures consistent typography
 * and provides convenience methods for creating scaled or bold variants of the
 * standard fonts.
 *
 * Available Font Types:
 * - **uiFont**: Standard proportional font for general UI text
 * - **boldUiFont**: Bold variant of the UI font for emphasis
 * - **monoFont**: Monospaced font for code, data, and terminal displays
 *
 * Custom Font Creation:
 * The class provides methods to create custom font variants scaled by a
 * fractional multiplier and optionally made bold. This is useful for creating
 * hierarchical typography (headings, subheadings, etc.) while maintaining
 * consistency with the base font family.
 *
 * Typical Usage:
 * @code
 * // Get standard UI font
 * QFont uiFont = Misc::CommonFonts::instance().uiFont();
 *
 * // Create a larger bold heading font (1.5x base size)
 * QFont headingFont = Misc::CommonFonts::instance().customUiFont(1.5, true);
 *
 * // Get monospace font for data display
 * QFont dataFont = Misc::CommonFonts::instance().monoFont();
 * @endcode
 *
 * @note Fonts are loaded once during construction and remain constant for the
 *       application lifetime. Font changes trigger the fontsChanged() signal.
 */
class CommonFonts : public QObject {
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
  Q_PROPERTY(double widgetFontScale
             READ widgetFontScale
             WRITE setWidgetFontScale
             NOTIFY fontsChanged)
  Q_PROPERTY(QString widgetFontFamily
             READ widgetFontFamily
             WRITE setWidgetFontFamily
             NOTIFY fontsChanged)
  Q_PROPERTY(QStringList availableFonts
             READ availableFonts
             CONSTANT)
  Q_PROPERTY(int widgetFontRevision
             READ widgetFontRevision
             NOTIFY fontsChanged)
  Q_PROPERTY(int widgetFontIndex
             READ widgetFontIndex
             NOTIFY fontsChanged)
  // clang-format on

public:
  static constexpr double kScaleSmall      = 0.85;
  static constexpr double kScaleNormal     = 1.00;
  static constexpr double kScaleLarge      = 1.25;
  static constexpr double kScaleExtraLarge = 1.50;

signals:
  void fontsChanged();

private:
  explicit CommonFonts();
  CommonFonts(CommonFonts&&)                 = delete;
  CommonFonts(const CommonFonts&)            = delete;
  CommonFonts& operator=(CommonFonts&&)      = delete;
  CommonFonts& operator=(const CommonFonts&) = delete;

public:
  static CommonFonts& instance();

  [[nodiscard]] const QFont& uiFont() const;
  [[nodiscard]] const QFont& monoFont() const;
  [[nodiscard]] const QFont& boldUiFont() const;
  [[nodiscard]] double widgetFontScale() const;
  [[nodiscard]] QString widgetFontFamily() const;
  [[nodiscard]] QStringList availableFonts() const;
  [[nodiscard]] int widgetFontRevision() const;
  [[nodiscard]] int widgetFontIndex() const;

  Q_INVOKABLE QFont customUiFont(double fraction = 1, bool bold = false);
  Q_INVOKABLE QFont customMonoFont(double fraction = 1, bool bold = false);
  Q_INVOKABLE QFont widgetFont(double fraction = 1, bool bold = false) const;

  void setWidgetFontScale(double scale);
  void setWidgetFontFamily(const QString& family);

private:
  QSettings m_settings;
  QFont m_uiFont;
  QFont m_monoFont;
  QFont m_boldUiFont;
  QString m_widgetFontFamily;
  double m_widgetFontScale;
  int m_widgetFontIndex    = 0;
  int m_widgetFontRevision = 0;
};
}  // namespace Misc
