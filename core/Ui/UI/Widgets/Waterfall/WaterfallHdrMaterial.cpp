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

#include "UI/Widgets/Waterfall/WaterfallHdrMaterial.h"

#include <cstring>
#include <QMatrix4x4>
#include <QSGMaterialShader>
#include <QSGTexture>

#include "Core/SSAssert.h"

static constexpr int kMatrixBytes   = 64;
static constexpr int kOpacityOffset = 64;
static constexpr int kBoostOffset   = 68;
static constexpr int kUniformBytes  = 72;
static constexpr int kRingBinding   = 1;
static constexpr int kLutBinding    = 2;

static constexpr char kVertexShader[]   = ":/serial-studio.com/shaders/waterfall_hdr.vert.qsb";
static constexpr char kFragmentShader[] = ":/serial-studio.com/shaders/waterfall_hdr.frag.qsb";

namespace Widgets::WaterfallHdrDetail {
/**
 * @brief Shader half: matrix/opacity/boost uniforms plus the ring and LUT samplers. The ring's
 *        staged scanlines are committed here, on the render thread's prepare step, which keeps
 *        the stage-at-sync / upload-at-prepare contract of the SDR path.
 */
class WaterfallHdrShader : public QSGMaterialShader {
public:
  WaterfallHdrShader();

  bool updateUniformData(RenderState& state,
                         QSGMaterial* newMaterial,
                         QSGMaterial* oldMaterial) override;
  void updateSampledImage(RenderState& state,
                          int binding,
                          QSGTexture** texture,
                          QSGMaterial* newMaterial,
                          QSGMaterial* oldMaterial) override;
};

/**
 * @brief Points the stages at the vendored shaders (app/shaders/waterfall_hdr.*).
 */
WaterfallHdrShader::WaterfallHdrShader()
{
  setShaderFileName(VertexStage, QString::fromLatin1(kVertexShader));
  setShaderFileName(FragmentStage, QString::fromLatin1(kFragmentShader));
}

/**
 * @brief Fills the std140 block: combined matrix at offset 0, opacity at 64, boost at 68.
 */
bool WaterfallHdrShader::updateUniformData(RenderState& state,
                                           QSGMaterial* newMaterial,
                                           QSGMaterial* oldMaterial)
{
  QByteArray* buffer = state.uniformData();
  SS_ASSERT(buffer != nullptr, return false);
  SS_ASSERT(buffer->size() >= kUniformBytes, return false);

  bool changed = false;
  if (state.isMatrixDirty()) {
    const QMatrix4x4 matrix = state.combinedMatrix();
    std::memcpy(buffer->data(), matrix.constData(), kMatrixBytes);
    changed = true;
  }

  if (state.isOpacityDirty()) {
    const float opacity = state.opacity();
    std::memcpy(buffer->data() + kOpacityOffset, &opacity, sizeof(opacity));
    changed = true;
  }

  const auto* material = static_cast<const WaterfallHdrMaterial*>(newMaterial);
  const auto* previous = static_cast<const WaterfallHdrMaterial*>(oldMaterial);
  SS_ASSERT(material != nullptr, return changed);

  if (previous == nullptr || previous->boost() != material->boost()) {
    const float boost = material->boost();
    std::memcpy(buffer->data() + kBoostOffset, &boost, sizeof(boost));
    changed = true;
  }

  return changed;
}

/**
 * @brief Hands the renderer the ring (binding 1) and LUT (binding 2) textures, committing any
 *        staged scanline uploads first.
 */
void WaterfallHdrShader::updateSampledImage(RenderState& state,
                                            const int binding,
                                            QSGTexture** texture,
                                            QSGMaterial* newMaterial,
                                            QSGMaterial* oldMaterial)
{
  Q_UNUSED(oldMaterial)

  SS_ASSERT(texture != nullptr, return);
  SS_ASSERT(newMaterial != nullptr, return);

  auto* material      = static_cast<WaterfallHdrMaterial*>(newMaterial);
  QSGTexture* sampled = binding == kRingBinding
                        ? material->ring()
                        : (binding == kLutBinding ? material->lut() : nullptr);

  if (sampled != nullptr)
    sampled->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());

  *texture = sampled;
}

}  // namespace Widgets::WaterfallHdrDetail

//--------------------------------------------------------------------------------------------------
// Material
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the material; only the primary ring node's material Owns the textures, so
 *        the seam alias can share both without a double free.
 */
Widgets::WaterfallHdrMaterial::WaterfallHdrMaterial(const TextureOwnership ownership)
  : m_boost(1.0f), m_ownership(ownership), m_ring(nullptr), m_lut(nullptr)
{}

/**
 * @brief Deletes the owned textures; runs on the render thread with the owning node.
 */
Widgets::WaterfallHdrMaterial::~WaterfallHdrMaterial()
{
  if (m_ownership == TextureOwnership::Owns) {
    delete m_ring;
    delete m_lut;
  }
}

/**
 * @brief One shared type for every spectrogram quad.
 */
QSGMaterialType* Widgets::WaterfallHdrMaterial::type() const
{
  static QSGMaterialType waterfallType;
  return &waterfallType;
}

/**
 * @brief Returns the magnitude ring texture.
 */
QSGTexture* Widgets::WaterfallHdrMaterial::ring() const noexcept
{
  return m_ring;
}

/**
 * @brief Returns the colormap LUT texture.
 */
QSGTexture* Widgets::WaterfallHdrMaterial::lut() const noexcept
{
  return m_lut;
}

/**
 * @brief Returns the emissive boost applied to top-of-scale magnitudes (1 = none).
 */
float Widgets::WaterfallHdrMaterial::boost() const noexcept
{
  return m_boost;
}

/**
 * @brief Points the material at the magnitude ring; ownership follows the ctor flag.
 */
void Widgets::WaterfallHdrMaterial::setRing(QSGTexture* ring) noexcept
{
  m_ring = ring;
}

/**
 * @brief Replaces the LUT texture, deleting the previous one when this material owns it (a
 *        colormap change on the primary node; the alias is re-pointed by its caller).
 */
void Widgets::WaterfallHdrMaterial::setLut(QSGTexture* lut)
{
  if (m_lut == lut)
    return;

  if (m_ownership == TextureOwnership::Owns)
    delete m_lut;

  m_lut = lut;
}

/**
 * @brief Sets the emissive boost; scene-graph sync only.
 */
void Widgets::WaterfallHdrMaterial::setBoost(const float boost) noexcept
{
  m_boost = boost;
}

/**
 * @brief Orders by texture identity then boost, so the two seam quads of one widget batch and
 *        distinct widgets stay apart.
 */
int Widgets::WaterfallHdrMaterial::compare(const QSGMaterial* other) const
{
  SS_ASSERT(other != nullptr, return 1);
  SS_ASSERT(other->type() == type(), return 1);

  const auto* material = static_cast<const WaterfallHdrMaterial*>(other);
  if (m_ring != material->ring())
    return m_ring < material->ring() ? -1 : 1;

  if (m_lut != material->lut())
    return m_lut < material->lut() ? -1 : 1;

  if (m_boost != material->boost())
    return m_boost < material->boost() ? -1 : 1;

  return 0;
}

/**
 * @brief Hands the render thread a shader bound to the vendored stages.
 */
QSGMaterialShader* Widgets::WaterfallHdrMaterial::createShader(
  QSGRendererInterface::RenderMode renderMode) const
{
  Q_UNUSED(renderMode)
  return new WaterfallHdrDetail::WaterfallHdrShader;
}
