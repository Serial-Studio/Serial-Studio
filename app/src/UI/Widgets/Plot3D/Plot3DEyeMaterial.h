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

#include "UI/Widgets/Plot3D/Plot3DStereo.h"

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGMaterialShader)

namespace Widgets {
/**
 * @brief Vertex-colored material that writes only the channels one stereo eye owns, leaving
 *        the rest of the framebuffer untouched. This is what makes the two eyes independent:
 *        neither pass can attenuate the other, so an eye's contrast no longer depends on
 *        whether the other one happens to overlap it.
 */
class EyeMaterial : public QSGMaterial {
public:
  explicit EyeMaterial(const Plot3DStereo::EyeMask mask);

  [[nodiscard]] QSGMaterialType* type() const override;
  [[nodiscard]] Plot3DStereo::EyeMask mask() const;
  [[nodiscard]] int compare(const QSGMaterial* other) const override;
  [[nodiscard]] QSGMaterialShader* createShader(
    QSGRendererInterface::RenderMode renderMode) const override;

private:
  Plot3DStereo::EyeMask m_mask;
};

namespace EyeMaterialFactory {
/**
 * @brief Node-creation hook handed to the stroke builders. A plain function pointer, so an
 *        eye's material costs no capture state and no allocation until a node is built.
 */
using Factory = QSGMaterial* (*)();

[[nodiscard]] Factory forEye(const Plot3DStereo::EyeMask mask);
[[nodiscard]] bool backendSupportsIsolation(QQuickWindow* window);
}  // namespace EyeMaterialFactory
}  // namespace Widgets
