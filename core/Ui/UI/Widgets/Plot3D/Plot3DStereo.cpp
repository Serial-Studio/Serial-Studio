/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "UI/Widgets/Plot3D/Plot3DStereo.h"

#include <algorithm>
#include <QFile>
#include <QtGlobal>

#include "Core/SSAssert.h"

/**
 * @brief Accumulator slot an eye writes into. The left eye shares slot 0 with the mono
 *        picture, so the non-stereo path touches exactly the buffers it always did.
 */
int Widgets::Plot3DStereo::eyeSlot(const EyeMask mask)
{
  switch (mask) {
    case EyeMask::None:
    case EyeMask::Left:
      return 0;
    case EyeMask::Right:
      return 1;
  }

  Q_UNREACHABLE_RETURN(0);
}

/**
 * @brief Whether the vendored shaders were compiled into this binary. That is all this answers:
 *        it is a build-time fact, not a promise that a pipeline can be created. Whether the
 *        running backend can actually run a custom material is a separate, render-time question
 *        the caller must also ask; see EyeMaterialFactory::backendSupportsIsolation.
 */
bool Widgets::Plot3DStereo::shadersPresent()
{
  static const bool available = QFile::exists(QString::fromLatin1(kVertexShader))
                             && QFile::exists(QString::fromLatin1(kFragmentShader));

  return available;
}

/**
 * @brief Rec. 601 luminance, carried by the red eye so a color with little red of its own
 *        still forms an image for that eye instead of vanishing into the background.
 */
float Widgets::Plot3DStereo::luminance(const QColor& color)
{
  SS_ASSERT(color.isValid(), return 0.0f);
  return 0.299f * color.redF() + 0.587f * color.greenF() + 0.114f * color.blueF();
}

/**
 * @brief Channels an eye owns. Alpha is owned by both: the two strokes cover different pixels,
 *        so letting each contribute alpha makes the coverage their union.
 */
Widgets::Plot3DStereo::EyeChannels Widgets::Plot3DStereo::eyeChannels(const EyeMask mask)
{
  switch (mask) {
    case EyeMask::None:
      return EyeChannels{true, true, true};
    case EyeMask::Left:
      return EyeChannels{true, false, false};
    case EyeMask::Right:
      return EyeChannels{false, true, true};
  }

  Q_UNREACHABLE_RETURN(EyeChannels{true, true, true});
}

/**
 * @brief Vertex color for the channel-isolated path. Only the owned channels are ever
 *        sampled, so the rest are left at zero and the source alpha passes through: with the
 *        two eyes writing disjoint channels there is nothing to cap.
 */
QColor Widgets::Plot3DStereo::isolatedEyeColor(const QColor& color, const EyeMask mask)
{
  SS_ASSERT(color.isValid(), return color);
  if (mask == EyeMask::None)
    return color;

  const bool left = mask == EyeMask::Left;
  QColor out      = left ? QColor::fromRgbF(luminance(color), 0.0f, 0.0f)
                         : QColor::fromRgbF(0.0f, color.greenF(), color.blueF());

  out.setAlphaF(color.alphaF());
  return out;
}

/**
 * @brief Midpoint of the plot's radial background, the constant the blended fallback fills its
 *        unowned channels from. The gradient's two stops differ by at most 19/255 in the
 *        shipped themes, so one estimate stays within a few counts of the real pixel.
 */
QColor Widgets::Plot3DStereo::midBackground(const QColor& inner, const QColor& outer)
{
  SS_ASSERT(inner.isValid(), return outer);
  SS_ASSERT(outer.isValid(), return inner);

  return QColor::fromRgbF(0.5f * (inner.redF() + outer.redF()),
                          0.5f * (inner.greenF() + outer.greenF()),
                          0.5f * (inner.blueF() + outer.blueF()));
}

/**
 * @brief Vertex color for the fallback path, where both eyes blend into the same channels.
 *        Filling the unowned channels from the background is what keeps them looking
 *        untouched; only the eye drawn second is capped, since the first pass leaves that
 *        eye's own channels alone.
 */
QColor Widgets::Plot3DStereo::blendedEyeColor(const QColor& color,
                                              const QColor& background,
                                              const EyeMask mask)
{
  if (mask == EyeMask::None)
    return color;

  SS_ASSERT(background.isValid(), return color);
  SS_ASSERT(color.isValid(), return color);

  const bool left = mask == EyeMask::Left;
  QColor out = left ? QColor::fromRgbF(luminance(color), background.greenF(), background.blueF())
                    : QColor::fromRgbF(background.redF(), color.greenF(), color.blueF());

  out.setAlphaF(left ? color.alphaF() : std::min(color.alphaF(), kOverlaidEyeAlpha));
  return out;
}
