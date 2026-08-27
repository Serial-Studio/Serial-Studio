/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>

#include "SSAssert.h"
#include "UI/Widgets/GpuStroke.h"
#include "UI/Widgets/Plot3D.h"
#include "UI/Widgets/Plot3D/Plot3DEyeMaterial.h"
#include "UI/Widgets/Plot3D/Plot3DStereo.h"

//--------------------------------------------------------------------------------------------------
// Scene-graph node synchronization: render thread, inside updatePaintNode, GUI thread blocked
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds one trace node per eye slot, stroking the gradient polyline or emitting one
 *        dot per sample when interpolation is off.
 */
void Widgets::Plot3D::syncTraceNode()
{
  for (int slot = 0; slot < Plot3DStereo::kEyeSlots; ++slot) {
    SS_ASSERT_LOG(m_tracePx[slot].size() == m_traceColors[slot].size());
    if (m_tracePx[slot].size() != m_traceColors[slot].size())
      continue;

    const auto* px      = m_tracePx[slot].data();
    const auto* colors  = m_traceColors[slot].data();
    const auto count    = static_cast<qsizetype>(m_tracePx[slot].size());
    const auto material = eyeFactory(slot);

    m_traceNodes[slot] =
      m_interpolate
        ? GpuStroke::buildStrokeNode(m_traceNodes[slot], px, colors, count, 1.0, material)
        : GpuStroke::buildPointNode(m_traceNodes[slot], px, colors, count, 1.0, material);
  }
}

/**
 * @brief Releases one stroke node, but only while the scene graph has let go of it. Deleting a
 *        node the root still parents would leave the root a dangling child and crash on its next
 *        tree walk, so a node that is still owned is left alone: a stale picture is recoverable,
 *        a use-after-free on the render thread is not.
 */
static void releaseDetachedNode(QSGGeometryNode*& node)
{
  SS_ASSERT_LOG(!node || node->parent() == nullptr);
  if (node && node->parent())
    return;

  delete node;
  node = nullptr;
}

/**
 * @brief Drops every stroke node so the builders rebuild them. A node's material is chosen once,
 *        at creation, so toggling stereo would otherwise leave a mono picture rendering through
 *        an eye's color-write mask.
 */
void Widgets::Plot3D::releaseStrokeNodes()
{
  for (int slot = 0; slot < Plot3DStereo::kEyeSlots; ++slot) {
    releaseDetachedNode(m_gridNodes[slot]);
    releaseDetachedNode(m_axisNodes[slot]);
    releaseDetachedNode(m_traceNodes[slot]);
  }
}

/**
 * @brief Node-creation material for one eye slot: an isolating material only while stereo is
 *        on and the isolated path is available, so mono and the blended fallback both keep the
 *        stock vertex-color material and render exactly as they did.
 */
Widgets::GpuStroke::MaterialFactory Widgets::Plot3D::eyeFactory(const int slot) const
{
  SS_ASSERT(slot >= 0 && slot < Plot3DStereo::kEyeSlots, return nullptr);
  if (!m_anaglyph || !m_channelIsolation)
    return nullptr;

  return EyeMaterialFactory::forEye(slot == 0 ? EyeMask::Left : EyeMask::Right);
}

/**
 * @brief Rebuilds one tile node in place, re-uploading its texture only when the tile was
 *        rasterized again. A null tile releases the node.
 */
void Widgets::Plot3D::syncTileNode(QSGSimpleTextureNode*& slot,
                                   const QImage& tile,
                                   const QPointF& topLeft,
                                   bool& needsUpload)
{
  SS_ASSERT(window() != nullptr, return);

  if (tile.isNull()) {
    delete slot;
    slot        = nullptr;
    needsUpload = false;
    return;
  }

  if (!slot) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending transfers ownership to
    // the root and the slot is re-nulled whenever updatePaintNode is handed a null oldNode.
    slot = new QSGSimpleTextureNode;
    // code-verify on
    slot->setOwnsTexture(true);
    slot->setFiltering(QSGTexture::Linear);
    needsUpload = true;
  }

  if (needsUpload) {
    auto* texture = window()->createTextureFromImage(tile);
    SS_ASSERT_LOG(texture != nullptr);
    if (texture) {
      slot->setTexture(texture);
      needsUpload = false;
    }
  }

  const QSizeF size = tile.deviceIndependentSize();
  slot->setRect(QRectF(topLeft, size));
}

/**
 * @brief Appends the scene nodes in draw order, flipping the trace and grid around the same
 *        camera-angle threshold the layered composite used.
 */
void Widgets::Plot3D::appendSceneNodes(QSGNode* root)
{
  SS_ASSERT(root != nullptr, return);

  const bool gridOnTop = m_cameraAngleX <= 270.0 && m_cameraAngleX > 90.0;
  const auto append    = [root](const EyeArray<QSGGeometryNode*>& nodes) {
    for (auto* node : nodes)
      if (node)
        root->appendChildNode(node);
  };

  if (gridOnTop)
    append(m_traceNodes);

  append(m_gridNodes);
  append(m_axisNodes);

  if (!gridOnTop)
    append(m_traceNodes);

  if (m_labelNode)
    root->appendChildNode(m_labelNode);

  if (m_indicatorNode)
    root->appendChildNode(m_indicatorNode);
}

/**
 * @brief Refreshes the background tile node, rebuilding the tile only when the theme or the
 *        item size marked it dirty.
 */
void Widgets::Plot3D::syncBackgroundNode(QSGNode* root, const QRectF& rect)
{
  SS_ASSERT(root != nullptr, return);
  SS_ASSERT(window() != nullptr, return);

  if (!m_bgNode) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending below transfers
    // ownership to the root and the slot is re-nulled whenever updatePaintNode is handed a
    // null oldNode. There is no destructor release path to add.
    m_bgNode = new QSGSimpleTextureNode;
    // code-verify on
    m_bgNode->setOwnsTexture(true);
    m_bgNode->setFiltering(QSGTexture::Linear);
  }

  if (m_bgUpload && !m_bgTile.isNull()) {
    auto* texture = window()->createTextureFromImage(m_bgTile);
    SS_ASSERT_LOG(texture != nullptr);
    if (texture) {
      m_bgNode->setTexture(texture);
      m_bgUpload = false;
    }
  }

  m_bgNode->setRect(rect);
  root->appendChildNode(m_bgNode);
}

/**
 * @brief Rebuilds one stroke node per eye slot from its accumulated polyline; the builder
 *        frees and nulls a node when its slot has nothing left to draw, which is how the
 *        second eye's nodes are released the moment stereo is switched off.
 */
void Widgets::Plot3D::syncStrokeNodes(EyeArray<QSGGeometryNode*>& nodes,
                                      const EyeArray<std::vector<QPointF>>& px,
                                      const EyeArray<std::vector<QColor>>& colors,
                                      const double halfWidth)
{
  SS_ASSERT(halfWidth > 0.0, return);

  for (int slot = 0; slot < Plot3DStereo::kEyeSlots; ++slot) {
    SS_ASSERT_LOG(px[slot].size() == colors[slot].size());
    if (px[slot].size() != colors[slot].size())
      continue;

    nodes[slot] = GpuStroke::buildStrokeNode(nodes[slot],
                                             px[slot].data(),
                                             colors[slot].data(),
                                             static_cast<qsizetype>(px[slot].size()),
                                             halfWidth,
                                             eyeFactory(slot));
  }
}
