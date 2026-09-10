/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/Painter/PainterBlur.h"

#  include <array>
#  include <cstdint>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

// Shadow blur radius is clamped to kMaxBlurRadius by the caller; max tap count is 2*radius+1
static constexpr int kMaxBlurTaps         = 2 * Widgets::PainterBlur::kMaxBlurRadius + 1;
static constexpr int kBlurReciprocalShift = 24;

//--------------------------------------------------------------------------------------------------
// Separable box-blur kernels
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the Q24 ceiling-reciprocal table so `(sum * inv) >> 24 == sum / n` exactly.
 */
static const std::array<int64_t, kMaxBlurTaps + 1>& blurReciprocalTable()
{
  static const auto table = []() {
    std::array<int64_t, kMaxBlurTaps + 1> t{};
    const int64_t one = int64_t(1) << kBlurReciprocalShift;
    for (int n = 1; n <= kMaxBlurTaps; ++n)
      t[n] = (one + n - 1) / n;

    return t;
  }();
  return table;
}

/**
 * @brief Horizontal pass of a box-blur with the given radius.
 */
static void boxBlurHorizontal(const QImage& src, QImage& dst, int radius)
{
  const int w     = src.width();
  const int h     = src.height();
  const auto& inv = blurReciprocalTable();
  for (int y = 0; y < h; ++y) {
    const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(y));
    QRgb* drow       = reinterpret_cast<QRgb*>(dst.scanLine(y));
    for (int x = 0; x < w; ++x) {
      int r = 0, g = 0, b = 0, a = 0, n = 0;
      const int x0 = qMax(0, x - radius);
      const int x1 = qMin(w - 1, x + radius);
      for (int k = x0; k <= x1; ++k) {
        r += qRed(srow[k]);
        g += qGreen(srow[k]);
        b += qBlue(srow[k]);
        a += qAlpha(srow[k]);
        ++n;
      }
      const int64_t recip = inv[n];
      drow[x]             = qRgba(static_cast<int>((r * recip) >> kBlurReciprocalShift),
                      static_cast<int>((g * recip) >> kBlurReciprocalShift),
                      static_cast<int>((b * recip) >> kBlurReciprocalShift),
                      static_cast<int>((a * recip) >> kBlurReciprocalShift));
    }
  }
}

/**
 * @brief Vertical pass of a box-blur with the given radius.
 */
static void boxBlurVertical(const QImage& src, QImage& dst, int radius)
{
  const int w     = src.width();
  const int h     = src.height();
  const auto& inv = blurReciprocalTable();
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      int r = 0, g = 0, b = 0, a = 0, n = 0;
      const int y0 = qMax(0, y - radius);
      const int y1 = qMin(h - 1, y + radius);
      for (int k = y0; k <= y1; ++k) {
        const QRgb px  = reinterpret_cast<const QRgb*>(src.constScanLine(k))[x];
        r             += qRed(px);
        g             += qGreen(px);
        b             += qBlue(px);
        a             += qAlpha(px);
        ++n;
      }
      const int64_t recip = inv[n];
      reinterpret_cast<QRgb*>(dst.scanLine(y))[x] =
        qRgba(static_cast<int>((r * recip) >> kBlurReciprocalShift),
              static_cast<int>((g * recip) >> kBlurReciprocalShift),
              static_cast<int>((b * recip) >> kBlurReciprocalShift),
              static_cast<int>((a * recip) >> kBlurReciprocalShift));
    }
  }
}

/**
 * @brief Three-pass separable box-blur applied in place to the given image.
 */
void Widgets::PainterBlur::applyBoxBlur(QImage& image, int radius)
{
  for (int pass = 0; pass < 3; ++pass) {
    QImage blurred(image.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(Qt::transparent);
    boxBlurHorizontal(image, blurred, radius);
    image.fill(Qt::transparent);
    boxBlurVertical(blurred, image, radius);
  }
}

#endif  // BUILD_COMMERCIAL
