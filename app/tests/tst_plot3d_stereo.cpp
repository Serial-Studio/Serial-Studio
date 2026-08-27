/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <algorithm>
#include <cmath>
#include <QtTest>
#include <vector>

#include "UI/Widgets/Plot3D/Plot3DStereo.h"

using namespace Widgets;

/**
 * @brief One theme's Plot3D-relevant palette, as read from app/rcc/themes/*.json.
 */
struct ThemePalette {
  const char* name;
  const char* innerBackground;
  const char* outerBackground;
  std::vector<const char*> colors;
};

/**
 * @brief Covers the stereo color policy behind the 3D plot's anaglyph mode (spec 0072): which
 *        channels an eye owns, that the isolated path leaves the rest alone, that both eyes
 *        form an image for every shipped palette entry, and that the blended fallback still
 *        produces two distinct ghosts when channel isolation is unavailable.
 */
class TstPlot3DStereo : public QObject {
  Q_OBJECT

private slots:
  void eyesOwnComplementaryChannels();
  void monoPassesThroughUntouched();
  void isolatedColorsCarryOnlyTheOwnedChannels();
  void isolatedAlphaIsNeverCapped();
  void everyThemeColorReachesBothEyes();
  void redAxisOnLightThemeReachesTheRedEye();
  void blueTraceOnDarkThemeReachesTheRedEye();
  void cyanEyeKeepsTheSourceHue();
  void blendedFallbackFillsFromTheBackground();
  void blendedFallbackCapsOnlyTheOverlaidEye();
  void slotsSeparateTheEyesAndShareMono();

private:
  [[nodiscard]] static std::vector<ThemePalette> palettes();
  [[nodiscard]] static bool sameChannel(const float a, const float b);
  [[nodiscard]] static float contrast(const float channel, const float background);
};

/**
 * @brief The shipped themes. Backgrounds are widget_base / widget_window; the colors are the
 *        widget_colors entries plus the axis and grid keys the 3D plot actually draws with.
 */
std::vector<ThemePalette> TstPlot3DStereo::palettes()
{
  return {
    {     "default",
     "#eff0f1", "#f6f7f9",
     {"#1B8EE1",
     "#9B6BD6",
     "#DB681A",
     "#389B4D",
     "#e04b5a",
     "#7fbf3f",
     "#3391e6",
     "#5f6670",
     "#A0A7B0"}},
    { "fluent-dark",
     "#2D2D2D", "#1A1A1A",
     {"#6CB8F0",
     "#C49AFF",
     "#F0A050",
     "#6CD880",
     "#FF6B6B",
     "#4EC969",
     "#4CC2FF",
     "#383838",
     "#5A5A5A"}},
    {"fluent-light",
     "#F5F5F5", "#FBFBFB",
     {"#1A85E0",
     "#7C4FD0",
     "#D87030",
     "#28984A",
     "#D13438",
     "#107C10",
     "#0078D4",
     "#D1D1D1",
     "#B8B8B8"}},
  };
}

/**
 * @brief Channel equality with slack for QColor's 16-bit storage: a float handed to
 *        QColor::fromRgbF and read back can move by ~8e-6, which is outside qFuzzyCompare's
 *        relative tolerance for values below 1.0 and would make exact comparisons flaky.
 */
bool TstPlot3DStereo::sameChannel(const float a, const float b)
{
  return std::abs(a - b) < 1e-3f;
}

/**
 * @brief Absolute separation between a channel and the background behind it.
 */
float TstPlot3DStereo::contrast(const float channel, const float background)
{
  return std::abs(channel - background);
}

/**
 * @brief The left eye owns red, the right owns green and blue, and neither owns the other's.
 *        A shared channel would put the two eyes back on top of each other.
 */
void TstPlot3DStereo::eyesOwnComplementaryChannels()
{
  const auto left  = Plot3DStereo::eyeChannels(Plot3DStereo::EyeMask::Left);
  const auto right = Plot3DStereo::eyeChannels(Plot3DStereo::EyeMask::Right);

  QVERIFY(left.red);
  QVERIFY(!left.green);
  QVERIFY(!left.blue);

  QVERIFY(!right.red);
  QVERIFY(right.green);
  QVERIFY(right.blue);

  QVERIFY(!(left.red && right.red));
  QVERIFY(!(left.green && right.green));
  QVERIFY(!(left.blue && right.blue));
}

/**
 * @brief Mono writes every channel and returns the source verbatim. This is the common path,
 *        and spec 0072 R5 requires it be untouched.
 */
void TstPlot3DStereo::monoPassesThroughUntouched()
{
  const auto mono = Plot3DStereo::eyeChannels(Plot3DStereo::EyeMask::None);
  QVERIFY(mono.red);
  QVERIFY(mono.green);
  QVERIFY(mono.blue);

  const QColor source(0x1B, 0x8E, 0xE1, 0x9C);
  const QColor background(0xEF, 0xF0, 0xF1);
  QCOMPARE(Plot3DStereo::isolatedEyeColor(source, Plot3DStereo::EyeMask::None), source);
  QCOMPARE(Plot3DStereo::blendedEyeColor(source, background, Plot3DStereo::EyeMask::None), source);
}

/**
 * @brief The isolated path leaves the unowned channels at zero rather than filling them: with
 *        a color-write mask in force nothing ever samples them.
 */
void TstPlot3DStereo::isolatedColorsCarryOnlyTheOwnedChannels()
{
  const QColor source(0x1B, 0x8E, 0xE1);

  const QColor left = Plot3DStereo::isolatedEyeColor(source, Plot3DStereo::EyeMask::Left);
  QVERIFY(sameChannel(left.redF(), Plot3DStereo::luminance(source)));
  QVERIFY(sameChannel(left.greenF(), 0.0f));
  QVERIFY(sameChannel(left.blueF(), 0.0f));

  const QColor right = Plot3DStereo::isolatedEyeColor(source, Plot3DStereo::EyeMask::Right);
  QVERIFY(sameChannel(right.redF(), 0.0f));
  QVERIFY(sameChannel(right.greenF(), source.greenF()));
  QVERIFY(sameChannel(right.blueF(), source.blueF()));
}

/**
 * @brief Nothing caps alpha on the isolated path. The cap exists only where the two eyes blend
 *        over each other, and disjoint channel writes make that impossible.
 */
void TstPlot3DStereo::isolatedAlphaIsNeverCapped()
{
  const QColor opaque(0x5F, 0x66, 0x70, 0xFF);
  const QColor opaqueLeft  = Plot3DStereo::isolatedEyeColor(opaque, Plot3DStereo::EyeMask::Left);
  const QColor opaqueRight = Plot3DStereo::isolatedEyeColor(opaque, Plot3DStereo::EyeMask::Right);
  QVERIFY(sameChannel(opaqueLeft.alphaF(), 1.0f));
  QVERIFY(sameChannel(opaqueRight.alphaF(), 1.0f));

  const QColor faded(0x5F, 0x66, 0x70, 100);
  const QColor left = Plot3DStereo::isolatedEyeColor(faded, Plot3DStereo::EyeMask::Left);
  QVERIFY(sameChannel(left.alphaF(), faded.alphaF()));
}

/**
 * @brief Every color the shipped themes draw the 3D plot with must form an image for BOTH
 *        eyes on its own background. This is the guarantee the retired per-channel merge did
 *        not give: it fed each eye a raw channel, which can sit on top of the background. The
 *        floor is deliberately below the tightest shipped entry (fluent-dark's minor grid, at
 *        0.080) so a theme tweak does not make this flaky; the sharp assertions live in the
 *        two named-failure cases below.
 */
void TstPlot3DStereo::everyThemeColorReachesBothEyes()
{
  constexpr float kMinContrast = 0.05f;

  for (const auto& theme : palettes()) {
    const QColor inner(QString::fromLatin1(theme.innerBackground));
    const QColor outer(QString::fromLatin1(theme.outerBackground));
    const QColor background = Plot3DStereo::midBackground(inner, outer);

    for (const char* entry : theme.colors) {
      const QColor source(QString::fromLatin1(entry));
      QVERIFY2(source.isValid(), entry);

      const QColor left  = Plot3DStereo::isolatedEyeColor(source, Plot3DStereo::EyeMask::Left);
      const QColor right = Plot3DStereo::isolatedEyeColor(source, Plot3DStereo::EyeMask::Right);

      const float red   = contrast(left.redF(), background.redF());
      const float green = contrast(right.greenF(), background.greenF());
      const float blue  = contrast(right.blueF(), background.blueF());

      QVERIFY2(red >= kMinContrast,
               qPrintable(QStringLiteral("%1 / %2 red eye")
                            .arg(QString::fromLatin1(theme.name), QString::fromLatin1(entry))));
      QVERIFY2(std::max(green, blue) >= kMinContrast,
               qPrintable(QStringLiteral("%1 / %2 cyan eye")
                            .arg(QString::fromLatin1(theme.name), QString::fromLatin1(entry))));
    }
  }
}

/**
 * @brief Historical failure one: the default theme's X axis is a red the old merge handed to
 *        the red eye as R=224 against a 239 background, leaving that eye nothing to see.
 */
void TstPlot3DStereo::redAxisOnLightThemeReachesTheRedEye()
{
  const QColor axis(QStringLiteral("#e04b5a"));
  const QColor background(QStringLiteral("#eff0f1"));

  const float raw      = contrast(axis.redF(), background.redF());
  const QColor left    = Plot3DStereo::isolatedEyeColor(axis, Plot3DStereo::EyeMask::Left);
  const float isolated = contrast(left.redF(), background.redF());

  QVERIFY(raw < 0.08f);
  QVERIFY(isolated > 0.35f);
}

/**
 * @brief Historical failure two: a blue trace on a dark theme gave the red eye R=27 against a
 *        26 background.
 */
void TstPlot3DStereo::blueTraceOnDarkThemeReachesTheRedEye()
{
  const QColor trace(QStringLiteral("#1B8EE1"));
  const QColor background(QStringLiteral("#1A1A1A"));

  const float raw      = contrast(trace.redF(), background.redF());
  const QColor left    = Plot3DStereo::isolatedEyeColor(trace, Plot3DStereo::EyeMask::Left);
  const float isolated = contrast(left.redF(), background.redF());

  QVERIFY(raw < 0.02f);
  QVERIFY(isolated > 0.30f);
}

/**
 * @brief The cyan eye carries the source's own green and blue, so a dataset stays
 *        recognizable in stereo instead of the plot going monochrome (spec 0072 R4).
 */
void TstPlot3DStereo::cyanEyeKeepsTheSourceHue()
{
  const QColor blue(QStringLiteral("#1B8EE1"));
  const QColor orange(QStringLiteral("#DB681A"));

  const auto right         = Plot3DStereo::EyeMask::Right;
  const QColor blueRight   = Plot3DStereo::isolatedEyeColor(blue, right);
  const QColor orangeRight = Plot3DStereo::isolatedEyeColor(orange, right);

  QVERIFY(blueRight.blueF() > blueRight.greenF());
  QVERIFY(orangeRight.greenF() > orangeRight.blueF());
  QVERIFY(blueRight != orangeRight);
}

/**
 * @brief The fallback has no write mask, so it fills the unowned channels from the background
 *        instead. That is what keeps them looking untouched where the two ghosts do not meet.
 */
void TstPlot3DStereo::blendedFallbackFillsFromTheBackground()
{
  const QColor source(QStringLiteral("#5f6670"));
  const QColor background(QStringLiteral("#eff0f1"));

  const QColor left =
    Plot3DStereo::blendedEyeColor(source, background, Plot3DStereo::EyeMask::Left);
  QVERIFY(sameChannel(left.greenF(), background.greenF()));
  QVERIFY(sameChannel(left.blueF(), background.blueF()));
  QVERIFY(sameChannel(left.redF(), Plot3DStereo::luminance(source)));

  const QColor right =
    Plot3DStereo::blendedEyeColor(source, background, Plot3DStereo::EyeMask::Right);
  QVERIFY(sameChannel(right.redF(), background.redF()));
  QVERIFY(sameChannel(right.greenF(), source.greenF()));
  QVERIFY(sameChannel(right.blueF(), source.blueF()));
}

/**
 * @brief Only the eye drawn second is capped. Capping the first as well would halve its
 *        surviving contrast to buy nothing, since the first pass leaves the second eye's own
 *        channels alone.
 */
void TstPlot3DStereo::blendedFallbackCapsOnlyTheOverlaidEye()
{
  const QColor opaque(0x1B, 0x8E, 0xE1, 0xFF);
  const QColor background(QStringLiteral("#eff0f1"));

  const QColor left =
    Plot3DStereo::blendedEyeColor(opaque, background, Plot3DStereo::EyeMask::Left);
  QVERIFY(sameChannel(left.alphaF(), 1.0f));

  const QColor right =
    Plot3DStereo::blendedEyeColor(opaque, background, Plot3DStereo::EyeMask::Right);
  QVERIFY(sameChannel(right.alphaF(), Plot3DStereo::kOverlaidEyeAlpha));

  const QColor faded(0x5F, 0x66, 0x70, 100);
  const QColor fadedRight =
    Plot3DStereo::blendedEyeColor(faded, background, Plot3DStereo::EyeMask::Right);
  QVERIFY(sameChannel(fadedRight.alphaF(), faded.alphaF()));
}

/**
 * @brief The two eyes accumulate into separate slots, and mono reuses the left eye's, so the
 *        non-stereo path touches exactly the buffers it always did.
 */
void TstPlot3DStereo::slotsSeparateTheEyesAndShareMono()
{
  QCOMPARE(Plot3DStereo::eyeSlot(Plot3DStereo::EyeMask::None), 0);
  QCOMPARE(Plot3DStereo::eyeSlot(Plot3DStereo::EyeMask::Left), 0);
  QCOMPARE(Plot3DStereo::eyeSlot(Plot3DStereo::EyeMask::Right), 1);
  QVERIFY(Plot3DStereo::eyeSlot(Plot3DStereo::EyeMask::Left)
          != Plot3DStereo::eyeSlot(Plot3DStereo::EyeMask::Right));
  QVERIFY(Plot3DStereo::kEyeSlots == 2);
}

QTEST_GUILESS_MAIN(TstPlot3DStereo)

#include "tst_plot3d_stereo.moc"
