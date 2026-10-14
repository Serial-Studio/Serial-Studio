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

#include "UI/Widgets/SdfStrokeMaterial.h"

#include <cstring>
#include <QMatrix4x4>
#include <QSGMaterialShader>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// std140 layout of app/shaders/stroke_sdf.{vert,frag}: mat4, vec4 color, then four floats
static constexpr int kMatrixBytes     = 64;
static constexpr int kColorOffset     = 64;
static constexpr int kOpacityOffset   = 80;
static constexpr int kFaceOffset      = 84;
static constexpr int kFeatherOffset   = 88;
static constexpr int kIntensityOffset = 92;
static constexpr int kUniformBytes    = 96;

static constexpr char kVertexShader[]   = ":/serial-studio.com/shaders/stroke_sdf.vert.qsb";
static constexpr char kFragmentShader[] = ":/serial-studio.com/shaders/stroke_sdf.frag.qsb";

namespace Widgets::SdfStrokeDetail {
/**
 * @brief Shader half of SdfStrokeMaterial: binds the vendored stages and fills the uniform
 *        block the two stages share.
 */
class SdfStrokeShader : public QSGMaterialShader {
public:
  SdfStrokeShader();

  bool updateUniformData(RenderState& state,
                         QSGMaterial* newMaterial,
                         QSGMaterial* oldMaterial) override;
};

/**
 * @brief Points the stages at the vendored shaders (app/shaders/stroke_sdf.*).
 */
SdfStrokeShader::SdfStrokeShader()
{
  setShaderFileName(VertexStage, QString::fromLatin1(kVertexShader));
  setShaderFileName(FragmentStage, QString::fromLatin1(kFragmentShader));
}

/**
 * @brief Fills the std140 block: the combined matrix and inherited opacity when the renderer
 *        marks them dirty, the stroke style whenever it differs from the previous material.
 *        Writes into the renderer's buffer only, no allocation.
 */
bool SdfStrokeShader::updateUniformData(RenderState& state,
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

  const auto* material = static_cast<const SdfStrokeMaterial*>(newMaterial);
  const auto* previous = static_cast<const SdfStrokeMaterial*>(oldMaterial);
  SS_ASSERT(material != nullptr, return changed);

  if (previous == nullptr || previous->compare(material) != 0) {
    const float color[4]  = {static_cast<float>(material->color().redF()),
                             static_cast<float>(material->color().greenF()),
                             static_cast<float>(material->color().blueF()),
                             static_cast<float>(material->color().alphaF())};
    const float face      = material->face();
    const float feather   = material->feather();
    const float intensity = material->intensity();
    std::memcpy(buffer->data() + kColorOffset, color, sizeof(color));
    std::memcpy(buffer->data() + kFaceOffset, &face, sizeof(face));
    std::memcpy(buffer->data() + kFeatherOffset, &feather, sizeof(feather));
    std::memcpy(buffer->data() + kIntensityOffset, &intensity, sizeof(intensity));
    changed = true;
  }

  return changed;
}

}  // namespace Widgets::SdfStrokeDetail

//--------------------------------------------------------------------------------------------------
// Material
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a blended stroke material: opaque white, a 1 px face, a 1 px feather, SDR.
 *        RequiresFullMatrix keeps the batch renderer from merging nodes: a merge rewrites only
 *        the position attribute into batch coordinates, not the segment endpoints beside it.
 */
Widgets::SdfStrokeMaterial::SdfStrokeMaterial()
  : m_color(Qt::white), m_face(1.0f), m_feather(1.0f), m_intensity(1.0f)
{
  setFlag(QSGMaterial::Blending, true);
  setFlag(QSGMaterial::RequiresFullMatrix, true);
}

/**
 * @brief One shared type; equal-style materials share pipeline state through compare(). Nodes
 *        are never merged (RequiresFullMatrix), whatever the index type.
 */
QSGMaterialType* Widgets::SdfStrokeMaterial::type() const
{
  static QSGMaterialType strokeType;
  return &strokeType;
}

/**
 * @brief Orders by style (color, then face, feather, intensity) so the batch renderer merges
 *        identical strokes and keeps different ones apart.
 */
int Widgets::SdfStrokeMaterial::compare(const QSGMaterial* other) const
{
  SS_ASSERT(other != nullptr, return 1);
  SS_ASSERT(other->type() == type(), return 1);

  const auto* material = static_cast<const SdfStrokeMaterial*>(other);
  const QRgb mine      = m_color.rgba();
  const QRgb theirs    = material->color().rgba();
  if (mine != theirs)
    return mine < theirs ? -1 : 1;

  const float lhs[3] = {m_face, m_feather, m_intensity};
  const float rhs[3] = {material->face(), material->feather(), material->intensity()};
  for (int i = 0; i < 3; ++i)
    if (lhs[i] != rhs[i])
      return lhs[i] < rhs[i] ? -1 : 1;

  return 0;
}

/**
 * @brief Hands the render thread a shader bound to the vendored stages.
 */
QSGMaterialShader* Widgets::SdfStrokeMaterial::createShader(
  QSGRendererInterface::RenderMode renderMode) const
{
  Q_UNUSED(renderMode)
  return new SdfStrokeDetail::SdfStrokeShader;
}

/**
 * @brief Returns the stroke color (straight alpha; the fragment stage premultiplies).
 */
const QColor& Widgets::SdfStrokeMaterial::color() const noexcept
{
  return m_color;
}

/**
 * @brief Returns the opaque half-width in pixels.
 */
float Widgets::SdfStrokeMaterial::face() const noexcept
{
  return m_face;
}

/**
 * @brief Returns the anti-aliasing ramp width in pixels beyond the face.
 */
float Widgets::SdfStrokeMaterial::feather() const noexcept
{
  return m_feather;
}

/**
 * @brief Returns the emissive boost applied by the fragment stage (1 = none).
 */
float Widgets::SdfStrokeMaterial::intensity() const noexcept
{
  return m_intensity;
}

/**
 * @brief Sets the stroke style; returns true when anything moved, so the caller marks the
 *        node's material dirty only then.
 */
bool Widgets::SdfStrokeMaterial::setStyle(const QColor& color,
                                          const float face,
                                          const float feather,
                                          const float intensity)
{
  SS_ASSERT(face > 0.0f && feather > 0.0f, return false);
  SS_ASSERT(intensity >= 1.0f, return false);

  if (m_color == color && m_face == face && m_feather == feather && m_intensity == intensity)
    return false;

  m_color     = color;
  m_face      = face;
  m_feather   = feather;
  m_intensity = intensity;
  return true;
}
