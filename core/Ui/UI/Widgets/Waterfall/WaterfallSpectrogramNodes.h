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

#include <QImage>
#include <QRectF>
#include <vector>

#include "UI/Widgets/Waterfall/WaterfallTiles.h"

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGNode)
QT_FORWARD_DECLARE_CLASS(QSGSimpleTextureNode)

namespace Widgets {

class WaterfallRingTexture;

/**
 * @brief The spectrogram's scene-graph half: which rows changed, and the quads that draw them. The
 *        ring path owns ONE persistent GPU texture and moves only the changed scanlines; the tile
 *        path is the no-QRhi fallback and rebuilds a texture per dirty 64-row band. Everything but
 *        the two marking calls runs inside the item's updatePaintNode.
 */
class WaterfallSpectrogramNodes {
public:
  WaterfallSpectrogramNodes();
  WaterfallSpectrogramNodes(WaterfallSpectrogramNodes&&)                 = delete;
  WaterfallSpectrogramNodes(const WaterfallSpectrogramNodes&)            = delete;
  WaterfallSpectrogramNodes& operator=(WaterfallSpectrogramNodes&&)      = delete;
  WaterfallSpectrogramNodes& operator=(const WaterfallSpectrogramNodes&) = delete;
  ~WaterfallSpectrogramNodes();

  void reset(int imageHeight);
  void release();
  void forgetNodes();

  void markRow(int physicalRow);
  void markAll();

  void sync(QSGNode* root,
            QQuickWindow* window,
            const QImage& image,
            const QRectF& sourceRect,
            const QRectF& plotRect,
            int topRow);

private:
  void releaseTileNodes();
  void releaseRingNodes();
  void syncTileNode(QQuickWindow* window, const QImage& image, int tile);
  void assignTileQuad(int tile, const QRectF& dstRect, const QRectF& srcRect);
  void stageRingUploads(const QImage& image);

  void syncTiles(QSGNode* root,
                 QQuickWindow* window,
                 const QImage& image,
                 const QRectF& sourceRect,
                 const QRectF& plotRect,
                 int topRow);
  void syncRing(QSGNode* root,
                const QImage& image,
                const QRectF& sourceRect,
                const QRectF& plotRect,
                int topRow);

  [[nodiscard]] int tileCount() const noexcept;

  int m_imageHeight;
  bool m_tilesResized;
  bool m_ringUnavailable;
  bool m_ringFullDirty;

  QSGSimpleTextureNode* m_ringNode;
  QSGSimpleTextureNode* m_ringAliasNode;
  WaterfallRingTexture* m_ringTexture;

  std::vector<quint8> m_rowDirty;
  std::vector<int> m_dirtyRows;
  std::vector<quint8> m_tileDirty;
  std::vector<QSGSimpleTextureNode*> m_tileNodes;
  std::vector<QSGSimpleTextureNode*> m_tileAliasNodes;
  std::vector<WaterfallTiles::Piece> m_pieces;
};

}  // namespace Widgets
