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

#include "UI/Widgets/Waterfall/WaterfallRingTexture.h"

#include <cstring>
#include <QQuickWindow>
#include <rhi/qrhi.h>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// QImage::Format_RGB32 stores 0xffRRGGBB, whose little-endian byte order is exactly BGRA8
static constexpr QRhiTexture::Format kRingFormat = QRhiTexture::BGRA8;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the staging side of a ring texture for a @p size spectrogram; the GPU texture
 *        itself is created lazily on the render thread, where a QRhiTexture may legally exist.
 */
Widgets::WaterfallRingTexture::WaterfallRingTexture(const QSize& size)
  : m_size(size)
  , m_bytesPerRow(size.width() * 4)
  , m_texture(nullptr)
  , m_fullUpload(false)
  , m_stagedRows(0)
  , m_stagedRowIndex{}
  , m_failed(false)
{
  SS_ASSERT(!size.isEmpty(), m_failed.store(true, std::memory_order_relaxed));

  for (auto& row : m_stagedRowData)
    row.resize(m_bytesPerRow > 0 ? m_bytesPerRow : 1);
}

/**
 * @brief Frees the GPU texture. Runs on the render thread: the scene graph destroys the node that
 *        owns this texture there, which is the only context a QRhiTexture may be released in.
 */
Widgets::WaterfallRingTexture::~WaterfallRingTexture()
{
  delete m_texture;
}

//--------------------------------------------------------------------------------------------------
// QSGTexture contract
//--------------------------------------------------------------------------------------------------

/**
 * @brief Identity for the renderer's batching: one texture per widget, so the address is unique.
 */
qint64 Widgets::WaterfallRingTexture::comparisonKey() const
{
  return reinterpret_cast<qint64>(this);
}

/**
 * @brief The owned GPU texture, or null before the first successful commit (and forever after a
 *        failed creation, which is what makes the item fall back to the tile path).
 */
QRhiTexture* Widgets::WaterfallRingTexture::rhiTexture() const
{
  return m_texture;
}

/**
 * @brief Pixel size of the ring; fixed for the object's lifetime, so a history resize builds a
 *        new texture rather than reshaping this one.
 */
QSize Widgets::WaterfallRingTexture::textureSize() const
{
  return m_size;
}

/**
 * @brief The spectrogram is opaque; declaring it so keeps the renderer out of the blended pass.
 */
bool Widgets::WaterfallRingTexture::hasAlphaChannel() const
{
  return false;
}

/**
 * @brief No mip chain: the quads sample the ring at close to 1:1 and mips would need a per-tick
 *        regeneration of the whole image.
 */
bool Widgets::WaterfallRingTexture::hasMipmaps() const
{
  return false;
}

/**
 * @brief Creates the texture on first use and enqueues whatever was staged at the last sync onto
 *        the frame's resource update batch: one sub-rect per changed scanline, or one full image
 *        after a rebuild. Runs on the render thread during the prepare step, so it may only touch
 *        memory this object owns -- never the item's live QImage.
 */
void Widgets::WaterfallRingTexture::commitTextureOperations(
  QRhi* rhi, QRhiResourceUpdateBatch* resourceUpdates)
{
  SS_ASSERT(rhi != nullptr, return);
  SS_ASSERT(resourceUpdates != nullptr, return);

  if (m_failed.load(std::memory_order_relaxed))
    return;

  if (!m_texture) {
    m_texture = rhi->newTexture(kRingFormat, m_size);
    if (!m_texture || !m_texture->create()) {
      delete m_texture;
      m_texture = nullptr;
      m_failed.store(true, std::memory_order_relaxed);
      return;
    }
  }

  if (m_fullUpload) {
    if (!m_stagingImage.isNull()) {
      QRhiTextureSubresourceUploadDescription sub(m_stagingImage);
      sub.setSourceSize(m_size);
      sub.setDestinationTopLeft(QPoint(0, 0));
      resourceUpdates->uploadTexture(
        m_texture, QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, sub)));
    }

    m_fullUpload   = false;
    m_stagedRows   = 0;
    m_stagingImage = QImage();
    return;
  }

  for (int i = 0; i < m_stagedRows; ++i) {
    QRhiTextureSubresourceUploadDescription sub;
    sub.setData(m_stagedRowData[static_cast<std::size_t>(i)]);
    sub.setDataStride(static_cast<quint32>(m_bytesPerRow));
    sub.setSourceSize(QSize(m_size.width(), 1));
    sub.setDestinationTopLeft(QPoint(0, m_stagedRowIndex[static_cast<std::size_t>(i)]));
    resourceUpdates->uploadTexture(m_texture,
                                   QRhiTextureUploadDescription(QRhiTextureUploadEntry(0, 0, sub)));
  }

  m_stagedRows = 0;
}

//--------------------------------------------------------------------------------------------------
// Staging (synchronization phase only)
//--------------------------------------------------------------------------------------------------

/**
 * @brief True once texture creation failed; the item reads it at the next sync and switches to
 *        the per-band tile path for good.
 */
bool Widgets::WaterfallRingTexture::failed() const noexcept
{
  return m_failed.load(std::memory_order_relaxed);
}

/**
 * @brief True while a whole-image upload is staged and waiting for the next commit.
 */
bool Widgets::WaterfallRingTexture::fullUploadPending() const noexcept
{
  return m_fullUpload;
}

/**
 * @brief Scanlines staged since the last commit.
 */
int Widgets::WaterfallRingTexture::stagedRowCount() const noexcept
{
  return m_stagedRows;
}

/**
 * @brief Destination row of the scanline staged in @p slot, or -1 when the slot is unused.
 */
int Widgets::WaterfallRingTexture::stagedRowAt(const int slot) const noexcept
{
  if (slot < 0 || slot >= m_stagedRows)
    return -1;

  return m_stagedRowIndex[static_cast<std::size_t>(slot)];
}

/**
 * @brief Stages the whole spectrogram for re-upload. The QImage is held by implicit share, so the
 *        widget's next scanline write detaches its own copy and the render thread keeps reading
 *        the buffer that was current at this sync; the pending per-row stages are dropped because
 *        the full image already carries them.
 */
void Widgets::WaterfallRingTexture::stageImage(const QImage& image)
{
  if (image.isNull() || image.size() != m_size)
    return;

  m_stagingImage = image;
  m_fullUpload   = true;
  m_stagedRows   = 0;
}

/**
 * @brief Copies one colorized scanline into a staging slot. Past kStagedRowSlots the tick
 *        escalates to a full upload rather than growing the buffer, so the staging cost is fixed
 *        for the object's lifetime and no tick allocates.
 */
void Widgets::WaterfallRingTexture::stageRow(const QImage& image, const int row)
{
  if (m_fullUpload || image.isNull() || image.size() != m_size)
    return;

  if (row < 0 || row >= m_size.height())
    return;

  if (m_stagedRows >= kStagedRowSlots) {
    stageImage(image);
    return;
  }

  const auto slot = static_cast<std::size_t>(m_stagedRows);
  SS_ASSERT(m_stagedRowData[slot].size() == static_cast<qsizetype>(m_bytesPerRow), return);

  std::memcpy(m_stagedRowData[slot].data(),
              image.constScanLine(row),
              static_cast<std::size_t>(m_bytesPerRow));
  m_stagedRowIndex[slot] = row;
  ++m_stagedRows;
}

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a ring texture can back @p size on @p window's graphics device: the QImage-to-RHI
 *        byte-order match is little-endian only, the device must expose BGRA8, and the ring must
 *        fit the driver's maximum texture dimension. A false answer keeps the tile fallback.
 */
bool Widgets::WaterfallRingTexture::supported(const QQuickWindow* window, const QSize& size)
{
#if Q_BYTE_ORDER != Q_LITTLE_ENDIAN
  Q_UNUSED(window)
  Q_UNUSED(size)
  return false;
#else
  if (!window || size.isEmpty())
    return false;

  QRhi* rhi = window->rhi();
  if (!rhi || !rhi->isTextureFormatSupported(kRingFormat))
    return false;

  const int maxDimension = rhi->resourceLimit(QRhi::TextureSizeMax);
  return size.width() <= maxDimension && size.height() <= maxDimension;
#endif
}

/**
 * @brief Answers whether @p row differs from the last row handed to this cache and, when it does,
 *        adopts it. This is the spectrogram's idle gate (R15.1): a source that pushed no new
 *        samples re-runs the same FFT over the same ring and produces a bit-identical row, so the
 *        widget writes no scanline, dirties no texture and schedules no frame.
 */
bool Widgets::WaterfallRingTexture::captureRowIfChanged(const float* row,
                                                        const int bins,
                                                        std::vector<float>& cache)
{
  SS_ASSERT(row != nullptr, return false);
  SS_ASSERT(bins > 0, return false);

  const auto count = static_cast<std::size_t>(bins);
  if (cache.size() == count && std::memcmp(cache.data(), row, count * sizeof(float)) == 0)
    return false;

  cache.assign(row, row + count);
  return true;
}
