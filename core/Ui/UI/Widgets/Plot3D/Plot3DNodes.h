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

#include <array>
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <vector>

#include "UI/Widgets/GpuStroke.h"
#include "UI/Widgets/Plot3D/Plot3DStereo.h"

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGNode)
QT_FORWARD_DECLARE_CLASS(QSGGeometryNode)
QT_FORWARD_DECLARE_CLASS(QSGSimpleTextureNode)

namespace Widgets {

/**
 * @brief The scene-graph half of the 3D plot: the six node slots, their texture-upload
 *        handshake and the eye material each stroke node is built with. Every method runs on
 *        the RENDER thread inside updatePaintNode with the GUI thread blocked, except the three
 *        mark*Dirty() calls, which the paint side makes after rasterizing a tile.
 */
class Plot3DNodes {
public:
  template<typename T>
  using EyeArray = std::array<T, Plot3DStereo::kEyeSlots>;

  Plot3DNodes();

  Plot3DNodes(Plot3DNodes&&)                 = delete;
  Plot3DNodes(const Plot3DNodes&)            = delete;
  Plot3DNodes& operator=(Plot3DNodes&&)      = delete;
  Plot3DNodes& operator=(const Plot3DNodes&) = delete;

  void forget() noexcept;
  void requireUploads() noexcept;
  void markBackgroundDirty() noexcept;
  void markLabelDirty() noexcept;
  void markIndicatorDirty() noexcept;
  void setStereo(bool stereo);

  void syncBackground(QQuickWindow* window, QSGNode* root, const QImage& tile, const QRectF& rect);
  void syncGrid(const EyeArray<std::vector<QPointF>>& px,
                const EyeArray<std::vector<QColor>>& colors);
  void syncAxis(const EyeArray<std::vector<QPointF>>& px,
                const EyeArray<std::vector<QColor>>& colors);
  void syncTrace(const EyeArray<std::vector<QPointF>>& px,
                 const EyeArray<std::vector<QColor>>& colors,
                 bool interpolate);
  void syncLabel(QQuickWindow* window, const QImage& tile, const QPointF& topLeft);
  void syncIndicator(QQuickWindow* window, const QImage& tile, const QPointF& topLeft);
  void append(QSGNode* root, bool gridOnTop) const;

private:
  void releaseStrokeNodes();
  void syncStroke(EyeArray<QSGGeometryNode*>& nodes,
                  const EyeArray<std::vector<QPointF>>& px,
                  const EyeArray<std::vector<QColor>>& colors,
                  const double halfWidth);
  void syncTile(QSGSimpleTextureNode*& slot,
                QQuickWindow* window,
                const QImage& tile,
                const QPointF& topLeft,
                bool& needsUpload);

  [[nodiscard]] GpuStroke::MaterialFactory eyeFactory(const int slot) const;

private:
  bool m_stereo;
  bool m_bgUpload;
  bool m_labelUpload;
  bool m_indicatorUpload;

  QSGSimpleTextureNode* m_bgNode;
  QSGSimpleTextureNode* m_labelNode;
  QSGSimpleTextureNode* m_indicatorNode;

  EyeArray<QSGGeometryNode*> m_gridNodes;
  EyeArray<QSGGeometryNode*> m_axisNodes;
  EyeArray<QSGGeometryNode*> m_traceNodes;
};

}  // namespace Widgets
