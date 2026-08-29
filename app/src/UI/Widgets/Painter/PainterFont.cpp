/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/Painter/PainterFont.h"

#  include <QRegularExpression>
#  include <QStringList>

#  include "Misc/CommonFonts.h"
#  include "SerialStudio.h"

/**
 * @brief Resolves a CSS-style font family to a real installed family.
 */
[[nodiscard]] static QString resolveFontFamily(const QString& family,
                                               const Misc::CommonFonts& commonFonts)
{
  const QString trimmed = family.trimmed();
  if (trimmed.isEmpty())
    return commonFonts.widgetFontFamily();

  const QString lower = trimmed.toLower();
  if (lower == QLatin1String("sans-serif") || lower == QLatin1String("system-ui")
      || lower == QLatin1String("ui-sans-serif"))
    return commonFonts.widgetFontFamily();

  if (lower == QLatin1String("monospace") || lower == QLatin1String("ui-monospace"))
    return commonFonts.monoFont().family();

  if (lower == QLatin1String("serif") || lower == QLatin1String("ui-serif")
      || lower == QLatin1String("cursive") || lower == QLatin1String("fantasy"))
    return commonFonts.uiFont().family();

  return trimmed;
}

/**
 * @brief Parses a "<size>px <family>" font shorthand into a QFont.
 */
QFont Widgets::PainterFont::parseFontSpec(const QString& spec, const Misc::CommonFonts& fonts)
{
  static const QRegularExpression sizeRe(QStringLiteral("(\\d+(?:\\.\\d+)?)\\s*px"));

  bool bold   = false;
  bool italic = false;
  qreal size  = 10.0;

  const auto sizeMatch = sizeRe.match(spec);
  if (sizeMatch.hasMatch())
    size = SerialStudio::toDouble(sizeMatch.captured(1));

  const QString lower = spec.toLower();
  if (lower.contains(QLatin1String("italic")))
    italic = true;

  if (lower.contains(QLatin1String("bold")) || lower.contains(QLatin1String(" 700"))
      || lower.contains(QLatin1String(" 800")) || lower.contains(QLatin1String(" 900")))
    bold = true;

  QString family;
  if (sizeMatch.hasMatch()) {
    const int sizeEnd  = sizeMatch.capturedEnd();
    const QString tail = spec.mid(sizeEnd).trimmed();
    static const QRegularExpression kSep(QStringLiteral("[,\\s]+"));
    const QStringList tokens = tail.split(kSep, Qt::SkipEmptyParts);
    if (!tokens.isEmpty())
      family = tokens.first();
  }

  QFont f(resolveFontFamily(family, fonts), 10);
  f.setPointSizeF(size * 0.75);
  f.setBold(bold);
  f.setItalic(italic);

  return f;
}

#endif  // BUILD_COMMERCIAL
