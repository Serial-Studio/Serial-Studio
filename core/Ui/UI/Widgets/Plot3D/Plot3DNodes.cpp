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

#include "UI/Widgets/Plot3D/Plot3DNodes.h"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGSimpleTextureNode>

#include "Core/SSAssert.h"
#include "UI/Widgets/Plot3D/Plot3DEyeMaterial.h"

using EyeMask = Widgets::Plot3DStereo::EyeMask;

static constexpr double kGridHalfWidth  = 0.5;
static constexpr double kAxisHalfWidth  = 0.75;
static constexpr double kTraceHalfWidth = 1.0;

/**
 * @brief Starts with no nodes and every texture pending: the first updatePaintNode builds the
 *        tree, and the background tile is always uploaded on that first pass.
 */
Widgets::Plot3DNodes::Plot3DNodes()
  : m_stereo(false)
  , m_bgUpload(true)
  , m_labelUpload(false)
  , m_indicatorUpload(false)
  , m_bgNode(nullptr)
  , m_labelNode(nullptr)
  , m_indicatorNode(nullptr)
  , m_gridNodes{}
  , m_axisNodes{}
  , m_traceNodes{}
{}

//--------------------------------------------------------------------------------------------------
// Tree lifetime
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every slot WITHOUT deleting: the scene graph owns the nodes through the root it
 *        just released, so the pointers are already dangling and only the slots need clearing.
 */
void Widgets::Plot3DNodes::forget() noexcept
{
  m_bgNode        = nullptr;
  m_labelNode     = nullptr;
  m_indicatorNode = nullptr;
  m_gridNodes.fill(nullptr);
  m_axisNodes.fill(nullptr);
  m_traceNodes.fill(nullptr);
}

/**
 * @brief Re-arms all three texture uploads, which a freshly built root needs: its nodes carry no
 *        texture yet and the tiles were rasterized against the previous tree.
 */
void Widgets::Plot3DNodes::requireUploads() noexcept
{
  m_bgUpload        = true;
  m_labelUpload     = true;
  m_indicatorUpload = true;
}

/**
 * @brief The paint side rasterized a new background tile.
 */
void Widgets::Plot3DNodes::markBackgroundDirty() noexcept
{
  m_bgUpload = true;
}

/**
 * @brief The paint side rasterized a new grid-interval label tile.
 */
void Widgets::Plot3DNodes::markLabelDirty() noexcept
{
  m_labelUpload = true;
}

/**
 * @brief The paint side rasterized a new camera-indicator tile.
 */
void Widgets::Plot3DNodes::markIndicatorDirty() noexcept
{
  m_indicatorUpload = true;
}

/**
 * @brief Switches the stroke nodes between the mono and the channel-isolating material. A node's
 *        material is chosen once, at creation, so the nodes are dropped on the edge or a mono
 *        picture would keep rendering through an eye's color-write mask.
 */
void Widgets::Plot3DNodes::setStereo(bool stereo)
{
  if (m_stereo == stereo)
    return;

  releaseStrokeNodes();
  m_stereo = stereo;
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
 * @brief Drops every stroke node so the builders rebuild them with the current material.
 */
void Widgets::Plot3DNodes::releaseStrokeNodes()
{
  for (int slot = 0; slot < Plot3DStereo::kEyeSlots; ++slot) {
    releaseDetachedNode(m_gridNodes[slot]);
    releaseDetachedNode(m_axisNodes[slot]);
    releaseDetachedNode(m_traceNodes[slot]);
  }
}

//--------------------------------------------------------------------------------------------------
// Node synchronization: render thread, inside updatePaintNode, GUI thread blocked
//--------------------------------------------------------------------------------------------------

/**
 * @brief Node-creation material for one eye slot: an isolating material only while stereo is on,
 *        so mono and the blended fallback both keep the stock vertex-color material.
 */
Widgets::GpuStroke::MaterialFactory Widgets::Plot3DNodes::eyeFactory(const int slot) const
{
  SS_ASSERT(slot >= 0 && slot < Plot3DStereo::kEyeSlots, return nullptr);
  if (!m_stereo)
    return nullptr;

  return EyeMaterialFactory::forEye(slot == 0 ? EyeMask::Left : EyeMask::Right);
}

/**
 * @brief Rebuilds one stroke node per eye slot from its accumulated polyline; the builder
 *        frees and nulls a node when its slot has nothing left to draw, which is how the
 *        second eye's nodes are released the moment stereo is switched off.
 */
void Widgets::Plot3DNodes::syncStroke(EyeArray<QSGGeometryNode*>& nodes,
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

/**
 * @brief Rebuilds the grid stroke nodes.
 */
void Widgets::Plot3DNodes::syncGrid(const EyeArray<std::vector<QPointF>>& px,
                                    const EyeArray<std::vector<QColor>>& colors)
{
  syncStroke(m_gridNodes, px, colors, kGridHalfWidth);
}

/**
 * @brief Rebuilds the axis stroke nodes, which are drawn heavier than the grid.
 */
void Widgets::Plot3DNodes::syncAxis(const EyeArray<std::vector<QPointF>>& px,
                                    const EyeArray<std::vector<QColor>>& colors)
{
  syncStroke(m_axisNodes, px, colors, kAxisHalfWidth);
}

/**
 * @brief Rebuilds one trace node per eye slot, stroking the gradient polyline or emitting one
 *        dot per sample when interpolation is off.
 */
void Widgets::Plot3DNodes::syncTrace(const EyeArray<std::vector<QPointF>>& px,
                                     const EyeArray<std::vector<QColor>>& colors,
                                     bool interpolate)
{
  for (int slot = 0; slot < Plot3DStereo::kEyeSlots; ++slot) {
    SS_ASSERT_LOG(px[slot].size() == colors[slot].size());
    if (px[slot].size() != colors[slot].size())
      continue;

    const auto* points  = px[slot].data();
    const auto* tints   = colors[slot].data();
    const auto count    = static_cast<qsizetype>(px[slot].size());
    const auto material = eyeFactory(slot);

    m_traceNodes[slot] = interpolate
                         ? GpuStroke::buildStrokeNode(
                             m_traceNodes[slot], points, tints, count, kTraceHalfWidth, material)
                         : GpuStroke::buildPointNode(
                             m_traceNodes[slot], points, tints, count, kTraceHalfWidth, material);
  }
}

/**
 * @brief Rebuilds one tile node in place, re-uploading its texture only when the tile was
 *        rasterized again. A null tile releases the node.
 */
void Widgets::Plot3DNodes::syncTile(QSGSimpleTextureNode*& slot,
                                    QQuickWindow* window,
                                    const QImage& tile,
                                    const QPointF& topLeft,
                                    bool& needsUpload)
{
  SS_ASSERT(window != nullptr, return);

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
    auto* texture = window->createTextureFromImage(tile);
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
 * @brief Rebuilds the grid-interval label tile node.
 */
void Widgets::Plot3DNodes::syncLabel(QQuickWindow* window,
                                     const QImage& tile,
                                     const QPointF& topLeft)
{
  syncTile(m_labelNode, window, tile, topLeft, m_labelUpload);
}

/**
 * @brief Rebuilds the camera-indicator tile node.
 */
void Widgets::Plot3DNodes::syncIndicator(QQuickWindow* window,
                                         const QImage& tile,
                                         const QPointF& topLeft)
{
  syncTile(m_indicatorNode, window, tile, topLeft, m_indicatorUpload);
}

/**
 * @brief Refreshes the background tile node, re-uploading the tile only when the theme or the
 *        item size marked it dirty, and appends it as the first child of @p root.
 */
void Widgets::Plot3DNodes::syncBackground(QQuickWindow* window,
                                          QSGNode* root,
                                          const QImage& tile,
                                          const QRectF& rect)
{
  SS_ASSERT(root != nullptr, return);
  SS_ASSERT(window != nullptr, return);

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

  if (m_bgUpload && !tile.isNull()) {
    auto* texture = window->createTextureFromImage(tile);
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
 * @brief Appends the scene nodes in draw order, flipping the trace and grid around the same
 *        camera-angle threshold the layered composite used.
 */
void Widgets::Plot3DNodes::append(QSGNode* root, bool gridOnTop) const
{
  SS_ASSERT(root != nullptr, return);
  SS_ASSERT_LOG(m_bgNode != nullptr);

  const auto appendAll = [root](const EyeArray<QSGGeometryNode*>& nodes) {
    for (auto* node : nodes)
      if (node)
        root->appendChildNode(node);
  };

  if (gridOnTop)
    appendAll(m_traceNodes);

  appendAll(m_gridNodes);
  appendAll(m_axisNodes);

  if (!gridOnTop)
    appendAll(m_traceNodes);

  if (m_labelNode)
    root->appendChildNode(m_labelNode);

  if (m_indicatorNode)
    root->appendChildNode(m_indicatorNode);
}
