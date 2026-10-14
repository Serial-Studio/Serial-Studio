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

#pragma once

#include <QColor>
#include <QSGMaterial>
#include <QSGRendererInterface>

QT_FORWARD_DECLARE_CLASS(QSGMaterialShader)

namespace Widgets {
/**
 * @brief Signed-distance plot stroke material (spec 0097): one uniform color, the stroke's face
 *        and feather half-widths and an emissive intensity (1 = SDR). The fragment stage derives
 *        coverage from the distance to each SdfStroke segment, so geometry stays one quad per
 *        segment at any turn angle. Set during scene-graph sync only (GUI blocked).
 */
class SdfStrokeMaterial : public QSGMaterial {
public:
  SdfStrokeMaterial();

  [[nodiscard]] QSGMaterialType* type() const override;
  [[nodiscard]] int compare(const QSGMaterial* other) const override;
  [[nodiscard]] QSGMaterialShader* createShader(
    QSGRendererInterface::RenderMode renderMode) const override;

  [[nodiscard]] const QColor& color() const noexcept;
  [[nodiscard]] float face() const noexcept;
  [[nodiscard]] float feather() const noexcept;
  [[nodiscard]] float intensity() const noexcept;

  [[nodiscard]] bool setStyle(const QColor& color, float face, float feather, float intensity);

private:
  QColor m_color;
  float m_face;
  float m_feather;
  float m_intensity;
};
}  // namespace Widgets
