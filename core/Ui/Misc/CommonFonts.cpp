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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/CommonFonts.h"

#include <QApplication>
#include <QDebug>
#include <QFontDatabase>
#include <QThread>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Bundled font families loaded on demand (spec 0120 R15)
//--------------------------------------------------------------------------------------------------

/**
 * @brief One bundled face and the family it belongs to. Explicit, because the file prefix alone
 *        cannot tell families apart (Tiny5Mono-CRT* is "Tiny5 Mono CRT", not "Tiny5 Mono").
 */
struct BundledFace {
  QLatin1StringView family;
  QLatin1StringView file;
};

// clang-format off
static constexpr BundledFace kBundledFaces[] = {
  {QLatin1StringView("Tiny5 Mono"),        QLatin1StringView("Tiny5Mono-Regular.ttf")         },
  {QLatin1StringView("Tiny5 Mono"),        QLatin1StringView("Tiny5Mono-Bold.ttf")            },
  {QLatin1StringView("Tiny5 Mono"),        QLatin1StringView("Tiny5Mono-Italic.ttf")          },
  {QLatin1StringView("Tiny5 Mono"),        QLatin1StringView("Tiny5Mono-BoldItalic.ttf")      },
  {QLatin1StringView("Tiny5 Mono CRT"),    QLatin1StringView("Tiny5Mono-CRT.ttf")             },
  {QLatin1StringView("Tiny5 Mono CRT"),    QLatin1StringView("Tiny5Mono-CRTBold.ttf")         },
  {QLatin1StringView("Tiny5 Mono CRT"),    QLatin1StringView("Tiny5Mono-CRTItalic.ttf")       },
  {QLatin1StringView("Tiny5 Mono CRT"),    QLatin1StringView("Tiny5Mono-CRTBoldItalic.ttf")   },
  {QLatin1StringView("Tiny5 Mono LCD"),    QLatin1StringView("Tiny5Mono-LCD.ttf")             },
  {QLatin1StringView("Tiny5 Mono LCD"),    QLatin1StringView("Tiny5Mono-LCDBold.ttf")         },
  {QLatin1StringView("Tiny5 Mono LCD"),    QLatin1StringView("Tiny5Mono-LCDItalic.ttf")       },
  {QLatin1StringView("Tiny5 Mono LCD"),    QLatin1StringView("Tiny5Mono-LCDBoldItalic.ttf")   },
  {QLatin1StringView("Tiny5 Mono Matrix"), QLatin1StringView("Tiny5Mono-Matrix.ttf")          },
  {QLatin1StringView("Tiny5 Mono Matrix"), QLatin1StringView("Tiny5Mono-MatrixBold.ttf")      },
  {QLatin1StringView("Tiny5 Mono Matrix"), QLatin1StringView("Tiny5Mono-MatrixItalic.ttf")    },
  {QLatin1StringView("Tiny5 Mono Matrix"), QLatin1StringView("Tiny5Mono-MatrixBoldItalic.ttf")},
  {QLatin1StringView("Tiny5 Mono SC"),     QLatin1StringView("Tiny5MonoSC-Regular.ttf")       },
  {QLatin1StringView("Tiny5 Mono SC"),     QLatin1StringView("Tiny5MonoSC-Bold.ttf")          },
  {QLatin1StringView("Tiny5 Mono SC"),     QLatin1StringView("Tiny5MonoSC-Italic.ttf")        },
  {QLatin1StringView("Tiny5 Mono SC"),     QLatin1StringView("Tiny5MonoSC-BoldItalic.ttf")    },
};
// clang-format on

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the CommonFonts singleton and registers application fonts.
 */
Misc::CommonFonts::CommonFonts() : m_widgetFontIndex(0), m_widgetFontRevision(0)
{
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

  QFontDatabase::addApplicationFont(":/fonts/GeistMono-Bold.ttf");
  QFontDatabase::addApplicationFont(":/fonts/GeistMono-Medium.ttf");

  const auto id = QFontDatabase::addApplicationFont(":/fonts/GeistMono-Regular.ttf");
  if (id != -1) {
    const auto families = QFontDatabase::applicationFontFamilies(id);
    if (!families.isEmpty())
      monoFont = families.at(0);
  }

  m_uiFont = QApplication::font();

  m_boldUiFont = m_uiFont;
#ifdef Q_OS_LINUX
  m_boldUiFont.setWeight(QFont::DemiBold);
#else
  m_boldUiFont.setBold(true);
#endif

  m_monoFont = QFont(monoFont);
  m_monoFont.setFixedPitch(true);
  m_monoFont.setStyleHint(QFont::Monospace);
  m_monoFont.setPointSizeF(m_uiFont.pointSizeF());

  if (m_monoFont.family() != monoFont)
    m_monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  m_widgetFontFamily = m_settings.value("CommonFonts/FontFamily", m_monoFont.family()).toString();
  ensureFamilyLoaded(m_widgetFontFamily);
  m_widgetFontScale =
    SerialStudio::toDouble(m_settings.value("CommonFonts/FontScale", kScaleNormal));
  m_widgetFontScale = qBound(0.5, m_widgetFontScale, 3.0);

  const auto fonts  = availableFonts();
  m_widgetFontIndex = fonts.indexOf(m_widgetFontFamily);
  if (m_widgetFontIndex < 0) {
    m_widgetFontFamily = m_monoFont.family();
    m_widgetFontIndex  = fonts.indexOf(m_widgetFontFamily);
  }
}

/**
 * @brief Returns the singleton instance.
 */
Misc::CommonFonts& Misc::CommonFonts::instance()
{
  static CommonFonts instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Standard font accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UI font.
 */
const QFont& Misc::CommonFonts::uiFont() const
{
  return m_uiFont;
}

/**
 * @brief Returns the monospace font.
 */
const QFont& Misc::CommonFonts::monoFont() const
{
  return m_monoFont;
}

/**
 * @brief Returns the bold UI font.
 */
const QFont& Misc::CommonFonts::boldUiFont() const
{
  return m_boldUiFont;
}

//--------------------------------------------------------------------------------------------------
// Custom font builders
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a custom UI font with specified size and boldness.
 */
QFont Misc::CommonFonts::customUiFont(const double fraction, const bool bold)
{
  QFont font = bold ? m_boldUiFont : m_uiFont;
  font.setPointSizeF(m_uiFont.pointSizeF() * qMax(0.1, fraction));
  return font;
}

/**
 * @brief Creates a custom monospace font with specified size and boldness.
 */
QFont Misc::CommonFonts::customMonoFont(const double fraction, const bool bold)
{
  QFont font = m_monoFont;
  font.setPointSizeF(m_monoFont.pointSizeF() * qMax(0.1, fraction));
  if (bold)
    font.setWeight(QFont::Medium);

  return font;
}

//--------------------------------------------------------------------------------------------------
// Widget font properties
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current widget font scale factor.
 */
double Misc::CommonFonts::widgetFontScale() const
{
  return m_widgetFontScale;
}

/**
 * @brief Returns the current widget font family name.
 */
QString Misc::CommonFonts::widgetFontFamily() const
{
  return m_widgetFontFamily;
}

/**
 * @brief Returns a counter that changes whenever widget font settings update.
 */
int Misc::CommonFonts::widgetFontRevision() const
{
  return m_widgetFontRevision;
}

/**
 * @brief Returns the index of the current widget font family in availableFonts().
 */
int Misc::CommonFonts::widgetFontIndex() const
{
  return m_widgetFontIndex;
}

/**
 * @brief Returns all available font families, sorted alphabetically.
 */
QStringList Misc::CommonFonts::availableFonts() const
{
  QStringList families;
  for (const QString& family : QFontDatabase::families()) {
    if (family.startsWith(QLatin1Char('.')))
      continue;

    if (QFontDatabase::isPrivateFamily(family))
      continue;

    families.append(family);
  }

  for (const auto& bundled : bundledFamilies())
    if (!families.contains(bundled))
      families.append(bundled);

  families.sort(Qt::CaseInsensitive);

  const QString mono = m_monoFont.family();
  families.removeAll(mono);
  families.prepend(mono);

  return families;
}

/**
 * @brief The bundled families that load on demand, listed before any of their files is loaded.
 */
QStringList Misc::CommonFonts::bundledFamilies()
{
  QStringList families;
  for (const auto& face : kBundledFaces)
    if (!families.contains(face.family))
      families.append(QString(face.family));

  SS_ASSERT_LOG(!families.isEmpty());
  return families;
}

/**
 * @brief Registers @p family's bundled faces the first time it is chosen or restored (spec 0120
 *        R15): the 20 Tiny5 Mono faces held 19 MB resident and 30 ms at every startup for a font
 *        most users never pick. A family that is not bundled, or already loaded, is a no-op.
 */
void Misc::CommonFonts::ensureFamilyLoaded(const QString& family)
{
  SS_ASSERT_LOG(qApp == nullptr || QThread::currentThread() == qApp->thread());

  if (family.isEmpty() || m_loadedFamilies.contains(family))
    return;

  bool bundled = false;
  for (const auto& face : kBundledFaces) {
    if (family != face.family)
      continue;

    bundled         = true;
    const auto path = QStringLiteral(":/fonts/") + face.file;
    if (QFontDatabase::addApplicationFont(path) < 0)
      qWarning() << "[CommonFonts] Could not load bundled font" << path;
  }

  if (bundled)
    m_loadedFamilies.insert(family);
}

/**
 * @brief Builds a widget font scaled by fraction and the global widget scale.
 */
QFont Misc::CommonFonts::widgetFont(const double fraction, const bool bold) const
{
  QFont font(m_widgetFontFamily);
  font.setPointSizeF(m_uiFont.pointSizeF() * qMax(0.1, fraction) * m_widgetFontScale);
  if (bold)
    font.setWeight(QFont::Medium);

  return font;
}

/**
 * @brief Returns the size-aware fraction for widgetFont(): 1.0 while the widget's driving
 *        dimension is at or above reference, shrinking linearly to a 0.7 readability floor
 *        below it. Shared by dashboard widgets so small windows tighten uniformly. A negative
 *        size is a normal QML layout transient (pre-polish geometry) and clamps to the floor.
 */
double Misc::CommonFonts::autoScale(const double size, const double reference) const
{
  if (reference <= 0)
    return 1.0;

  return qBound(0.7, qMax(0.0, size) / reference, 1.0);
}

//--------------------------------------------------------------------------------------------------
// Font configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the global widget font scale and persists it.
 */
void Misc::CommonFonts::setWidgetFontScale(const double scale)
{
  const double clamped = qBound(0.5, scale, 3.0);
  if (qFuzzyCompare(m_widgetFontScale, clamped))
    return;

  m_widgetFontScale = clamped;
  ++m_widgetFontRevision;
  m_settings.setValue("CommonFonts/FontScale", m_widgetFontScale);
  Q_EMIT fontsChanged();
}

/**
 * @brief Sets the global widget font family and persists it.
 */
void Misc::CommonFonts::setWidgetFontFamily(const QString& family)
{
  if (m_widgetFontFamily == family)
    return;

  ensureFamilyLoaded(family);
  m_widgetFontFamily = family;
  m_widgetFontIndex  = availableFonts().indexOf(m_widgetFontFamily);
  ++m_widgetFontRevision;
  m_settings.setValue("CommonFonts/FontFamily", m_widgetFontFamily);
  Q_EMIT fontsChanged();
}
