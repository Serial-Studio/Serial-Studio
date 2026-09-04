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

#include "UI/Widgets/Waterfall/WaterfallSpectrogramNodes.h"

#include <QQuickWindow>
#include <QSGNode>
#include <QSGSimpleTextureNode>

#include "Core/SSAssert.h"
#include "UI/Widgets/Waterfall/WaterfallRingTexture.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty node set; nothing is created until the first synchronization phase.
 */
Widgets::WaterfallSpectrogramNodes::WaterfallSpectrogramNodes()
  : m_imageHeight(0)
  , m_tilesResized(true)
  , m_ringUnavailable(false)
  , m_ringFullDirty(true)
  , m_ringNode(nullptr)
  , m_ringAliasNode(nullptr)
  , m_ringTexture(nullptr)
{}

/**
 * @brief Deletes nothing: every node here is a child of the item's scene-graph root, which owns
 *        it and takes it down on the render thread. Holding raw observers is what lets the item
 *        forget them wholesale when that root is replaced.
 */
Widgets::WaterfallSpectrogramNodes::~WaterfallSpectrogramNodes() = default;

//--------------------------------------------------------------------------------------------------
// Dirty-row bookkeeping (GUI thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-sizes the per-row marks for a spectrogram of @p imageHeight rows and arms a full
 *        upload, which is what a history rebuild, a color-map change or a clear need.
 */
void Widgets::WaterfallSpectrogramNodes::reset(const int imageHeight)
{
  m_imageHeight = qMax(0, imageHeight);
  m_rowDirty.assign(static_cast<std::size_t>(m_imageHeight), 0);
  m_dirtyRows.clear();
  m_dirtyRows.reserve(static_cast<std::size_t>(WaterfallRingTexture::kStagedRowSlots));
  m_tilesResized = true;
  markAll();
}

/**
 * @brief Drops the row bookkeeping when the widget's image goes away (hidden widget, R15.1). The
 *        nodes themselves die with the scene-graph root the item releases in the same step.
 */
void Widgets::WaterfallSpectrogramNodes::release()
{
  m_imageHeight = 0;
  m_rowDirty.clear();
  m_rowDirty.shrink_to_fit();
  m_dirtyRows.clear();
  m_dirtyRows.shrink_to_fit();
  m_tileDirty.clear();
  m_tileDirty.shrink_to_fit();

  m_tilesResized  = true;
  m_ringFullDirty = true;
}

/**
 * @brief Forgets every cached node pointer after the scene-graph root that owned them was
 *        destroyed, and re-arms a full upload so the rebuilt tree starts from real pixels.
 */
void Widgets::WaterfallSpectrogramNodes::forgetNodes()
{
  m_ringNode      = nullptr;
  m_ringAliasNode = nullptr;
  m_ringTexture   = nullptr;
  m_tileNodes.clear();
  m_tileAliasNodes.clear();

  m_tilesResized = true;
  markAll();
}

/**
 * @brief Records @p physicalRow as changed for both upload paths: the band that carries it, and
 *        the scanline itself. More rows than the ring can stage between two syncs escalate to one
 *        full upload instead of growing a list, so the bookkeeping allocates nothing per tick.
 */
void Widgets::WaterfallSpectrogramNodes::markRow(const int physicalRow)
{
  const auto tile = static_cast<std::size_t>(physicalRow / WaterfallTiles::kTileRows);
  if (tile < m_tileDirty.size())
    m_tileDirty[tile] = 1;

  const auto row = static_cast<std::size_t>(physicalRow);
  if (m_ringFullDirty || physicalRow < 0 || row >= m_rowDirty.size() || m_rowDirty[row])
    return;

  if (m_dirtyRows.size() >= static_cast<std::size_t>(WaterfallRingTexture::kStagedRowSlots)) {
    markAll();
    return;
  }

  m_rowDirty[row] = 1;
  m_dirtyRows.push_back(physicalRow);
}

/**
 * @brief Marks the whole image as changed; used when the content, not one row, moved.
 */
void Widgets::WaterfallSpectrogramNodes::markAll()
{
  m_tileDirty.assign(static_cast<std::size_t>(tileCount()), 1);

  m_ringFullDirty = true;
  for (const int row : m_dirtyRows)
    m_rowDirty[static_cast<std::size_t>(row)] = 0;

  m_dirtyRows.clear();
}

/**
 * @brief Number of fixed-height texture bands the current image is split into.
 */
int Widgets::WaterfallSpectrogramNodes::tileCount() const noexcept
{
  return WaterfallTiles::tileCount(m_imageHeight);
}

//--------------------------------------------------------------------------------------------------
// Synchronization phase
//--------------------------------------------------------------------------------------------------

/**
 * @brief Attaches the spectrogram quads to @p root, taking the persistent ring texture when the
 *        device can back one and the per-band tiles otherwise. A ring texture that failed to
 *        create latches the fallback for the rest of the widget's life.
 */
void Widgets::WaterfallSpectrogramNodes::sync(QSGNode* root,
                                              QQuickWindow* window,
                                              const QImage& image,
                                              const QRectF& sourceRect,
                                              const QRectF& plotRect,
                                              const int topRow)
{
  SS_ASSERT(root != nullptr, return);

  if (image.isNull() || plotRect.isEmpty() || sourceRect.isEmpty()) {
    releaseTileNodes();
    releaseRingNodes();
    return;
  }

  if (m_ringTexture && m_ringTexture->failed())
    m_ringUnavailable = true;

  if (!m_ringUnavailable && WaterfallRingTexture::supported(window, image.size())) {
    releaseTileNodes();
    syncRing(root, image, sourceRect, plotRect, topRow);
    return;
  }

  releaseRingNodes();
  syncTiles(root, window, image, sourceRect, plotRect, topRow);
}

//--------------------------------------------------------------------------------------------------
// Ring path (one persistent texture)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops the ring texture together with the node that owns it. Runs on the render thread,
 *        the only context a QSGTexture may be destroyed in; the alias node borrows the texture
 *        and is deleted first so the borrowed pointer never outlives its owner.
 */
void Widgets::WaterfallSpectrogramNodes::releaseRingNodes()
{
  delete m_ringAliasNode;
  delete m_ringNode;

  m_ringAliasNode = nullptr;
  m_ringNode      = nullptr;
  m_ringTexture   = nullptr;
  m_ringFullDirty = true;
}

/**
 * @brief Copies the scanlines that changed since the last sync into the texture's staging
 *        buffers. Reading the live image is safe here and only here: the GUI thread is blocked
 *        for the synchronization phase, and by upload time it may already be writing the next row.
 */
void Widgets::WaterfallSpectrogramNodes::stageRingUploads(const QImage& image)
{
  SS_ASSERT(m_ringTexture != nullptr, return);

  if (m_ringFullDirty) {
    m_ringTexture->stageImage(image);
    m_ringFullDirty = false;
    return;
  }

  for (const int row : m_dirtyRows) {
    m_ringTexture->stageRow(image, row);
    m_rowDirty[static_cast<std::size_t>(row)] = 0;
  }

  m_dirtyRows.clear();
}

/**
 * @brief Draws the ring-ordered history out of ONE persistent GPU texture: the newest row sits at
 *        a moving physical index and the quads apply the scroll through their source rectangles,
 *        so a tick uploads the changed scanline instead of re-creating a texture. The visible span
 *        crosses the ring seam at most once, which is what the borrowing alias node draws.
 */
void Widgets::WaterfallSpectrogramNodes::syncRing(QSGNode* root,
                                                  const QImage& image,
                                                  const QRectF& sourceRect,
                                                  const QRectF& plotRect,
                                                  const int topRow)
{
  if (m_ringTexture && m_ringTexture->textureSize() != image.size())
    releaseRingNodes();

  if (!m_ringNode) {
    // code-verify off
    // Scene-graph nodes default to QSGNode::OwnedByParent, so appending transfers ownership to the
    // root; the primary node owns the texture and the alias must never own it, or the shared
    // QSGTexture would be freed twice.
    m_ringNode      = new QSGSimpleTextureNode;
    m_ringAliasNode = new QSGSimpleTextureNode;
    m_ringTexture   = new WaterfallRingTexture(image.size());
    // code-verify on
    m_ringNode->setOwnsTexture(true);
    m_ringNode->setFiltering(QSGTexture::Linear);
    m_ringNode->setTexture(m_ringTexture);
    m_ringAliasNode->setOwnsTexture(false);
    m_ringAliasNode->setFiltering(QSGTexture::Linear);
    m_ringAliasNode->setTexture(m_ringTexture);
    m_ringFullDirty = true;
  }

  stageRingUploads(image);
  m_ringNode->markDirty(QSGNode::DirtyMaterial);

  m_ringNode->setRect(QRectF());
  m_ringAliasNode->setRect(QRectF());

  WaterfallTiles::decompose(sourceRect, plotRect, image.height(), topRow, m_pieces, image.height());
  for (const auto& piece : m_pieces) {
    auto* node = m_ringNode->rect().isEmpty() ? m_ringNode : m_ringAliasNode;
    node->setRect(piece.dst);
    node->setSourceRect(piece.src);
  }

  root->appendChildNode(m_ringNode);
  root->appendChildNode(m_ringAliasNode);
}

//--------------------------------------------------------------------------------------------------
// Tile fallback (one texture per 64-row band)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drops every spectrogram band node together with the texture each one owns. Runs on the
 *        render thread, which is the only place a QSGTexture may be destroyed.
 */
void Widgets::WaterfallSpectrogramNodes::releaseTileNodes()
{
  for (auto* node : m_tileAliasNodes)
    delete node;

  for (auto* node : m_tileNodes)
    delete node;

  m_tileAliasNodes.clear();
  m_tileNodes.clear();
}

/**
 * @brief Creates the pair of nodes for one band and uploads its rows when they changed. The
 *        primary node owns the texture, so the scene graph frees it on the render thread; the
 *        alias node borrows it for the second quad the ring seam needs and must never own it.
 */
void Widgets::WaterfallSpectrogramNodes::syncTileNode(QQuickWindow* window,
                                                      const QImage& image,
                                                      const int tile)
{
  const auto slot = static_cast<std::size_t>(tile);
  SS_ASSERT(slot < m_tileNodes.size(), return);
  SS_ASSERT(window != nullptr, return);

  bool upload = false;
  if (!m_tileNodes[slot]) {
    // code-verify off
    // Owned by the root once appended, exactly as the background nodes are.
    m_tileNodes[slot] = new QSGSimpleTextureNode;
    // code-verify on
    m_tileNodes[slot]->setOwnsTexture(true);
    m_tileNodes[slot]->setFiltering(QSGTexture::Linear);
    upload = true;
  }

  if (!m_tileAliasNodes[slot]) {
    // code-verify off
    // Owned by the root once appended; it aliases the primary node's texture and must never own
    // it, or the shared QSGTexture would be freed twice.
    m_tileAliasNodes[slot] = new QSGSimpleTextureNode;
    // code-verify on
    m_tileAliasNodes[slot]->setOwnsTexture(false);
    m_tileAliasNodes[slot]->setFiltering(QSGTexture::Linear);
  }

  if (slot < m_tileDirty.size() && m_tileDirty[slot]) {
    upload            = true;
    m_tileDirty[slot] = 0;
  }

  if (upload) {
    const int top  = tile * WaterfallTiles::kTileRows;
    const int rows = qMin(WaterfallTiles::kTileRows, image.height() - top);
    const QImage band(
      image.constScanLine(top), image.width(), rows, image.bytesPerLine(), image.format());
    m_tileNodes[slot]->setTexture(window->createTextureFromImage(band));
  }

  m_tileAliasNodes[slot]->setTexture(m_tileNodes[slot]->texture());
  m_tileNodes[slot]->setRect(QRectF());
  m_tileAliasNodes[slot]->setRect(QRectF());
}

/**
 * @brief Points one band's quad at @p srcRect (band-local rows) and @p dstRect. A band the ring
 *        seam shows twice in one frame takes the alias node for its second piece.
 */
void Widgets::WaterfallSpectrogramNodes::assignTileQuad(const int tile,
                                                        const QRectF& dstRect,
                                                        const QRectF& srcRect)
{
  const auto slot = static_cast<std::size_t>(tile);
  SS_ASSERT(slot < m_tileNodes.size(), return);
  SS_ASSERT(m_tileNodes[slot] != nullptr && m_tileAliasNodes[slot] != nullptr, return);

  auto* node = m_tileNodes[slot]->rect().isEmpty() ? m_tileNodes[slot] : m_tileAliasNodes[slot];
  if (!node->texture())
    return;

  node->setRect(dstRect);
  node->setSourceRect(srcRect);
}

/**
 * @brief Draws the ring-ordered history as one textured quad per visible band, so the GPU samples
 *        the spectrogram and a tick uploads only the band that changed. The visible logical span
 *        maps to at most two physically contiguous runs -- the ring seam -- and each run is then
 *        split at the band boundaries it crosses.
 */
void Widgets::WaterfallSpectrogramNodes::syncTiles(QSGNode* root,
                                                   QQuickWindow* window,
                                                   const QImage& image,
                                                   const QRectF& sourceRect,
                                                   const QRectF& plotRect,
                                                   const int topRow)
{
  const int tiles = tileCount();
  if (m_tilesResized || static_cast<int>(m_tileNodes.size()) != tiles) {
    releaseTileNodes();
    m_tileNodes.assign(static_cast<std::size_t>(tiles), nullptr);
    m_tileAliasNodes.assign(static_cast<std::size_t>(tiles), nullptr);
    m_tilesResized = false;
  }

  for (int tile = 0; tile < tiles; ++tile)
    syncTileNode(window, image, tile);

  WaterfallTiles::decompose(sourceRect, plotRect, image.height(), topRow, m_pieces);
  for (const auto& piece : m_pieces)
    assignTileQuad(piece.tile, piece.dst, piece.src);

  for (int tile = 0; tile < tiles; ++tile) {
    root->appendChildNode(m_tileNodes[static_cast<std::size_t>(tile)]);
    root->appendChildNode(m_tileAliasNodes[static_cast<std::size_t>(tile)]);
  }
}
