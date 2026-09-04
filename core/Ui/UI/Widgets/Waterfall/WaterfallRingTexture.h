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
#include <atomic>
#include <QByteArray>
#include <QImage>
#include <QSGTexture>
#include <QSize>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QRhi)
QT_FORWARD_DECLARE_CLASS(QRhiResourceUpdateBatch)
QT_FORWARD_DECLARE_CLASS(QRhiTexture)

namespace Widgets {

/**
 * @brief One persistent GPU texture holding a whole spectrogram ring (spec 0075, R15.1): a tick
 *        uploads only the changed scanlines. `stage*()` copies out of the live QImage in the
 *        item's synchronization phase, so `commitTextureOperations()` -- later, on the render
 *        thread -- reads only memory this object owns; the scroll rides on the quads' source rects.
 */
class WaterfallRingTexture : public QSGTexture {
  Q_OBJECT

public:
  /**
   * @brief Scanlines that can be staged between two syncs before the tick escalates to one full
   *        image upload. One row is the steady state; the extra slots absorb a Campbell-mode
   *        burst and the tick that follows a missed sync.
   */
  static constexpr int kStagedRowSlots = 8;

  explicit WaterfallRingTexture(const QSize& size);
  WaterfallRingTexture(WaterfallRingTexture&&)                 = delete;
  WaterfallRingTexture(const WaterfallRingTexture&)            = delete;
  WaterfallRingTexture& operator=(WaterfallRingTexture&&)      = delete;
  WaterfallRingTexture& operator=(const WaterfallRingTexture&) = delete;
  ~WaterfallRingTexture() override;

  [[nodiscard]] qint64 comparisonKey() const override;
  [[nodiscard]] QRhiTexture* rhiTexture() const override;
  [[nodiscard]] QSize textureSize() const override;
  [[nodiscard]] bool hasAlphaChannel() const override;
  [[nodiscard]] bool hasMipmaps() const override;
  void commitTextureOperations(QRhi* rhi, QRhiResourceUpdateBatch* resourceUpdates) override;

  [[nodiscard]] bool failed() const noexcept;
  [[nodiscard]] bool fullUploadPending() const noexcept;
  [[nodiscard]] int stagedRowCount() const noexcept;
  [[nodiscard]] int stagedRowAt(int slot) const noexcept;

  void stageImage(const QImage& image);
  void stageRow(const QImage& image, int row);

  [[nodiscard]] static bool supported(const QQuickWindow* window, const QSize& size);
  [[nodiscard]] static bool captureRowIfChanged(const float* row,
                                                int bins,
                                                std::vector<float>& cache);

private:
  QSize m_size;
  int m_bytesPerRow;
  QRhiTexture* m_texture;

  QImage m_stagingImage;
  bool m_fullUpload;
  int m_stagedRows;
  std::array<int, kStagedRowSlots> m_stagedRowIndex;
  std::array<QByteArray, kStagedRowSlots> m_stagedRowData;

  std::atomic<bool> m_failed;
};

}  // namespace Widgets
