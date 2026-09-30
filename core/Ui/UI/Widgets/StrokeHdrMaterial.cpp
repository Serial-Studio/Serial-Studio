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

#include "UI/Widgets/StrokeHdrMaterial.h"

#include <cstring>
#include <QMatrix4x4>
#include <QSGMaterialShader>

#include "Core/SSAssert.h"

static constexpr int kMatrixBytes     = 64;
static constexpr int kOpacityOffset   = 64;
static constexpr int kIntensityOffset = 68;
static constexpr int kUniformBytes    = 72;

static constexpr char kVertexShader[]   = ":/serial-studio.com/shaders/stroke_hdr.vert.qsb";
static constexpr char kFragmentShader[] = ":/serial-studio.com/shaders/stroke_hdr.frag.qsb";

namespace Widgets::StrokeHdrDetail {
/**
 * @brief Shader half of StrokeHdrMaterial: ordinary premultiplied vertex-color drawing with
 *        the per-material intensity forwarded into the vendored fragment stage's boost.
 */
class StrokeHdrShader : public QSGMaterialShader {
public:
  StrokeHdrShader();

  bool updateUniformData(RenderState& state,
                         QSGMaterial* newMaterial,
                         QSGMaterial* oldMaterial) override;
};

/**
 * @brief Points the stages at the vendored shaders (app/shaders/stroke_hdr.*).
 */
StrokeHdrShader::StrokeHdrShader()
{
  setShaderFileName(VertexStage, QString::fromLatin1(kVertexShader));
  setShaderFileName(FragmentStage, QString::fromLatin1(kFragmentShader));
}

/**
 * @brief Fills the std140 block the vendored stages declare: combined matrix at offset 0,
 *        opacity at 64, intensity at 68. Writes into the renderer's buffer only, no
 *        allocation.
 */
bool StrokeHdrShader::updateUniformData(RenderState& state,
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

  const auto* material = static_cast<const StrokeHdrMaterial*>(newMaterial);
  const auto* previous = static_cast<const StrokeHdrMaterial*>(oldMaterial);
  SS_ASSERT(material != nullptr, return changed);

  if (previous == nullptr || previous->intensity() != material->intensity()) {
    const float intensity = material->intensity();
    std::memcpy(buffer->data() + kIntensityOffset, &intensity, sizeof(intensity));
    changed = true;
  }

  return changed;
}

}  // namespace Widgets::StrokeHdrDetail

//--------------------------------------------------------------------------------------------------
// Material
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a blended stroke material at neutral intensity (1 = stock appearance).
 */
Widgets::StrokeHdrMaterial::StrokeHdrMaterial() : m_intensity(1.0f)
{
  setFlag(QSGMaterial::Blending, true);
}

/**
 * @brief One shared type; equal-intensity materials remain batchable through compare().
 */
QSGMaterialType* Widgets::StrokeHdrMaterial::type() const
{
  static QSGMaterialType strokeType;
  return &strokeType;
}

/**
 * @brief Returns the emissive boost applied by the fragment stage (1 = none).
 */
float Widgets::StrokeHdrMaterial::intensity() const noexcept
{
  return m_intensity;
}

/**
 * @brief Sets the emissive boost; called at scene-graph sync only, and the caller marks the
 *        node's material dirty when the value moved.
 */
void Widgets::StrokeHdrMaterial::setIntensity(const float intensity) noexcept
{
  m_intensity = intensity;
}

/**
 * @brief Orders by intensity so the batch renderer merges equal-boost strokes and keeps
 *        different boosts apart.
 */
int Widgets::StrokeHdrMaterial::compare(const QSGMaterial* other) const
{
  SS_ASSERT(other != nullptr, return 1);
  SS_ASSERT(other->type() == type(), return 1);

  const auto* material = static_cast<const StrokeHdrMaterial*>(other);
  if (m_intensity < material->intensity())
    return -1;

  if (m_intensity > material->intensity())
    return 1;

  return 0;
}

/**
 * @brief Hands the render thread a shader bound to the vendored stages.
 */
QSGMaterialShader* Widgets::StrokeHdrMaterial::createShader(
  QSGRendererInterface::RenderMode renderMode) const
{
  Q_UNUSED(renderMode)
  return new StrokeHdrDetail::StrokeHdrShader;
}
