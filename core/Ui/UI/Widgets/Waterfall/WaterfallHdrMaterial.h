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

#pragma once

#include <QSGMaterial>
#include <QSGRendererInterface>

QT_FORWARD_DECLARE_CLASS(QSGMaterialShader)
QT_FORWARD_DECLARE_CLASS(QSGTexture)

namespace Widgets {
/**
 * @brief Spectrogram material for HDR windows (spec 0089): samples the 16-bit magnitude ring,
 *        applies the colormap through a 256x1 LUT texture on the GPU, and ramps top-of-scale
 *        magnitudes to the emissive boost. The primary ring node's material owns both textures
 *        (deleted with the node on the render thread); the seam alias borrows them.
 */
class WaterfallHdrMaterial : public QSGMaterial {
public:
  /**
   * @brief Who deletes the ring and LUT textures: the primary ring node's material Owns
   *        them, the seam alias Borrows -- the knob the double-free safety hinges on.
   */
  enum class TextureOwnership {
    Owns,
    Borrows
  };

  explicit WaterfallHdrMaterial(TextureOwnership ownership);
  WaterfallHdrMaterial(WaterfallHdrMaterial&&)                 = delete;
  WaterfallHdrMaterial(const WaterfallHdrMaterial&)            = delete;
  WaterfallHdrMaterial& operator=(WaterfallHdrMaterial&&)      = delete;
  WaterfallHdrMaterial& operator=(const WaterfallHdrMaterial&) = delete;
  ~WaterfallHdrMaterial() override;

  [[nodiscard]] QSGMaterialType* type() const override;
  [[nodiscard]] QSGTexture* ring() const noexcept;
  [[nodiscard]] QSGTexture* lut() const noexcept;
  [[nodiscard]] float boost() const noexcept;
  [[nodiscard]] int compare(const QSGMaterial* other) const override;
  [[nodiscard]] QSGMaterialShader* createShader(
    QSGRendererInterface::RenderMode renderMode) const override;

  void setRing(QSGTexture* ring) noexcept;
  void setLut(QSGTexture* lut);
  void setBoost(float boost) noexcept;

private:
  float m_boost;
  TextureOwnership m_ownership;
  QSGTexture* m_ring;
  QSGTexture* m_lut;
};
}  // namespace Widgets
