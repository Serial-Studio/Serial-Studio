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

#include "UI/Widgets/Plot3D/Plot3DEyeMaterial.h"

#include <cstring>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QSGMaterialShader>

#include "Core/SSAssert.h"

static constexpr int kMatrixBytes  = 64;
static constexpr int kUniformBytes = 68;

namespace Widgets::Plot3DEyeDetail {
/**
 * @brief Shader half of EyeMaterial: ordinary premultiplied vertex-color drawing, with the
 *        pipeline's color-write mask narrowed to the channels the eye owns. The shaders are
 *        vendored under app/shaders, so the uniform block below and the GLSL that reads it
 *        cannot drift apart when Qt reorganizes its own scene-graph shader set.
 */
class EyeShader : public QSGMaterialShader {
public:
  EyeShader();

  bool updateUniformData(RenderState& state,
                         QSGMaterial* newMaterial,
                         QSGMaterial* oldMaterial) override;
  bool updateGraphicsPipelineState(RenderState& state,
                                   GraphicsPipelineState* pipeline,
                                   QSGMaterial* newMaterial,
                                   QSGMaterial* oldMaterial) override;
};

/**
 * @brief Points the stages at the vendored shaders and opts into pipeline-state updates.
 *        Without that flag updateGraphicsPipelineState() is never called and every channel
 *        would be written, which is the defect this material exists to fix.
 */
EyeShader::EyeShader()
{
  setShaderFileName(VertexStage, QString::fromLatin1(Plot3DStereo::kVertexShader));
  setShaderFileName(FragmentStage, QString::fromLatin1(Plot3DStereo::kFragmentShader));
  setFlag(UpdatesGraphicsPipelineState, true);
}

/**
 * @brief Fills the std140 block the vendored vertex shader declares: the combined matrix at
 *        offset 0, the inherited opacity right after it.
 */
bool EyeShader::updateUniformData(RenderState& state,
                                  QSGMaterial* newMaterial,
                                  QSGMaterial* oldMaterial)
{
  Q_UNUSED(newMaterial)
  Q_UNUSED(oldMaterial)

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
    std::memcpy(buffer->data() + kMatrixBytes, &opacity, sizeof(opacity));
    changed = true;
  }

  return changed;
}

/**
 * @brief Narrows the color-write mask to the eye's own channels, leaving the blend equation
 *        the renderer derived from the material's Blending flag untouched.
 */
bool EyeShader::updateGraphicsPipelineState(RenderState& state,
                                            GraphicsPipelineState* pipeline,
                                            QSGMaterial* newMaterial,
                                            QSGMaterial* oldMaterial)
{
  Q_UNUSED(state)
  Q_UNUSED(oldMaterial)

  SS_ASSERT(pipeline != nullptr, return false);
  SS_ASSERT(newMaterial != nullptr, return false);

  const auto* material = static_cast<const EyeMaterial*>(newMaterial);
  const auto channels  = Plot3DStereo::eyeChannels(material->mask());

  GraphicsPipelineState::ColorMask written = GraphicsPipelineState::A;
  if (channels.red)
    written |= GraphicsPipelineState::R;

  if (channels.green)
    written |= GraphicsPipelineState::G;

  if (channels.blue)
    written |= GraphicsPipelineState::B;

  pipeline->colorWrite = written;
  return true;
}

}  // namespace Widgets::Plot3DEyeDetail

/**
 * @brief Builds a left-eye material for the stroke node factory.
 */
static QSGMaterial* makeLeftEyeMaterial()
{
  return new Widgets::EyeMaterial(Widgets::Plot3DStereo::EyeMask::Left);
}

/**
 * @brief Builds a right-eye material for the stroke node factory.
 */
static QSGMaterial* makeRightEyeMaterial()
{
  return new Widgets::EyeMaterial(Widgets::Plot3DStereo::EyeMask::Right);
}

//--------------------------------------------------------------------------------------------------
// Material
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a material writing only the channels @a mask owns. Batching is off: each eye
 *        owns whole nodes, so merging buys nothing, and opting out by contract beats relying on
 *        the index type happening to keep these batches unmerged.
 */
Widgets::EyeMaterial::EyeMaterial(const Plot3DStereo::EyeMask mask) : m_mask(mask)
{
  setFlag(QSGMaterial::Blending, true);
  setFlag(QSGMaterial::NoBatching, true);
}

/**
 * @brief One type per mask, so the renderer cannot batch two eyes into a single pipeline and
 *        apply one eye's color-write mask to both.
 */
QSGMaterialType* Widgets::EyeMaterial::type() const
{
  static QSGMaterialType monoType;
  static QSGMaterialType leftType;
  static QSGMaterialType rightType;

  if (m_mask == Plot3DStereo::EyeMask::Left)
    return &leftType;

  if (m_mask == Plot3DStereo::EyeMask::Right)
    return &rightType;

  return &monoType;
}

/**
 * @brief Returns the stereo eye this material writes for.
 */
Widgets::Plot3DStereo::EyeMask Widgets::EyeMaterial::mask() const
{
  return m_mask;
}

/**
 * @brief Orders by mask. Only ever reached between two materials of the same type(), so the
 *        cast is safe and the result is the ordering the renderer needs for batching.
 */
int Widgets::EyeMaterial::compare(const QSGMaterial* other) const
{
  SS_ASSERT(other != nullptr, return 1);
  SS_ASSERT(other->type() == type(), return 1);

  const auto* material = static_cast<const EyeMaterial*>(other);
  return static_cast<int>(m_mask) - static_cast<int>(material->mask());
}

/**
 * @brief Hands the render thread a shader bound to the vendored stages.
 */
QSGMaterialShader* Widgets::EyeMaterial::createShader(
  QSGRendererInterface::RenderMode renderMode) const
{
  Q_UNUSED(renderMode)
  return new Plot3DEyeDetail::EyeShader;
}

/**
 * @brief Whether the window's backend can run a custom material at all. The software adaptation
 *        has no shader pipeline, so an EyeMaterial there draws nothing rather than drawing worse
 *        - the one outcome the fail-soft rule forbids. Answered as a whitelist so a backend
 *        nobody has heard of yet degrades to the blended fallback instead of going blank.
 */
bool Widgets::EyeMaterialFactory::backendSupportsIsolation(QQuickWindow* window)
{
  SS_ASSERT(window != nullptr, return false);

  auto* renderer = window->rendererInterface();
  SS_ASSERT(renderer != nullptr, return false);

  switch (renderer->graphicsApi()) {
    case QSGRendererInterface::OpenGL:
    case QSGRendererInterface::Direct3D11:
    case QSGRendererInterface::Direct3D12:
    case QSGRendererInterface::Vulkan:
    case QSGRendererInterface::Metal:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Node-creation hook for one eye, or nullptr in mono so the stroke builders keep
 *        attaching the stock vertex-color material and the non-stereo path stays untouched.
 */
Widgets::EyeMaterialFactory::Factory Widgets::EyeMaterialFactory::forEye(
  const Plot3DStereo::EyeMask mask)
{
  if (mask == Plot3DStereo::EyeMask::Left)
    return &makeLeftEyeMaterial;

  if (mask == Plot3DStereo::EyeMask::Right)
    return &makeRightEyeMaterial;

  return nullptr;
}
