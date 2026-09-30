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

#include <QSGMaterial>
#include <QSGRendererInterface>

QT_FORWARD_DECLARE_CLASS(QSGMaterialShader)

namespace Widgets {
/**
 * @brief Vertex-colored stroke material with an emissive intensity uniform (spec 0089): the
 *        vendored fragment stage boosts the interpolated color in linear light so curve
 *        strokes exceed SDR white under an HDR window's output transform. Intensity is set
 *        during scene-graph sync only (GUI blocked), never from another thread.
 */
class StrokeHdrMaterial : public QSGMaterial {
public:
  StrokeHdrMaterial();

  [[nodiscard]] QSGMaterialType* type() const override;
  [[nodiscard]] float intensity() const noexcept;
  [[nodiscard]] int compare(const QSGMaterial* other) const override;
  [[nodiscard]] QSGMaterialShader* createShader(
    QSGRendererInterface::RenderMode renderMode) const override;

  void setIntensity(float intensity) noexcept;

private:
  float m_intensity;
};
}  // namespace Widgets
