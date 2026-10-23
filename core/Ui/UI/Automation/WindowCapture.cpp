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

#include "UI/Automation/WindowCapture.h"

#include <qfloat16.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <QBuffer>
#include <QQuickItem>
#include <QQuickWindow>

#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "Misc/HdrTransfer.h"

using namespace UI::Automation;

static constexpr const char* kHdrOutputClass = "Misc::HdrOutput";
static constexpr int kLutSize                = 4096;
static constexpr int kChannels               = 4;
static constexpr double kMinSdrScale         = 1e-6;
static constexpr double kHalf                = 0.5;

//--------------------------------------------------------------------------------------------------
// Inverse output transform
//--------------------------------------------------------------------------------------------------

/**
 * @brief Linear light to encoded sRGB, tabulated once from the shared transfer header so the
 *        per-pixel loop does no transcendental math.
 */
static const std::array<quint8, kLutSize>& encodeTable()
{
  static const std::array<quint8, kLutSize> table = [] {
    std::array<quint8, kLutSize> built{};
    for (int i = 0; i < kLutSize; ++i) {
      const float linear            = static_cast<float>(i) / static_cast<float>(kLutSize - 1);
      const float coded             = Misc::HdrTransfer::oetfExt(linear) * 255.0f;
      built[static_cast<size_t>(i)] = static_cast<quint8>(std::lround(coded));
    }

    return built;
  }();

  return table;
}

/**
 * @brief The table index for a linear value, clamped to SDR white; a NaN reads as black.
 */
[[nodiscard]] static size_t encodeIndex(const float linear) noexcept
{
  if (!(linear > 0.0f))
    return 0;

  if (linear >= 1.0f)
    return static_cast<size_t>(kLutSize - 1);

  return static_cast<size_t>(linear * static_cast<float>(kLutSize - 1) + 0.5f);
}

/**
 * @brief Decodes one scanline of scene-referred floats into 8-bit display-referred pixels.
 */
template<typename Float>
static void decodeRow(const Float* in, QRgb* out, const int width, const float inverse)
{
  const auto& lut = encodeTable();
  for (int x = 0; x < width; ++x) {
    const Float* pixel = in + static_cast<ptrdiff_t>(x) * kChannels;
    const quint8 r     = lut[encodeIndex(static_cast<float>(pixel[0]) * inverse)];
    const quint8 g     = lut[encodeIndex(static_cast<float>(pixel[1]) * inverse)];
    const quint8 b     = lut[encodeIndex(static_cast<float>(pixel[2]) * inverse)];
    out[x]             = qRgb(r, g, b);
  }
}

/**
 * @brief Whether @p format is one of the floating-point layouts an HDR swapchain grab uses.
 */
bool WindowCapture::isFloatFormat(const QImage::Format format) noexcept
{
  static constexpr QImage::Format kFloatFormats[] = {
    QImage::Format_RGBX16FPx4,
    QImage::Format_RGBA16FPx4,
    QImage::Format_RGBA16FPx4_Premultiplied,
    QImage::Format_RGBX32FPx4,
    QImage::Format_RGBA32FPx4,
    QImage::Format_RGBA32FPx4_Premultiplied,
  };

  for (const QImage::Format candidate : kFloatFormats)
    if (candidate == format)
      return true;

  return false;
}

/**
 * @brief An 8-bit copy of a floating-point grab with the output transform undone: divide by the
 *        SDR white scale, clamp to SDR white, linear to sRGB. An 8-bit grab passes through.
 */
QImage WindowCapture::toDisplayReferred(const QImage& grab, const double sdrWhiteScale)
{
  if (grab.isNull() || !isFloatFormat(grab.format()))
    return grab;

  const bool half     = grab.format() == QImage::Format_RGBX16FPx4
                     || grab.format() == QImage::Format_RGBA16FPx4
                     || grab.format() == QImage::Format_RGBA16FPx4_Premultiplied;
  const float inverse = 1.0f / static_cast<float>(std::max(sdrWhiteScale, kMinSdrScale));

  QImage out(grab.size(), QImage::Format_RGB32);
  SS_ASSERT(!out.isNull(), return QImage());
  out.setDevicePixelRatio(grab.devicePixelRatio());
  for (int y = 0; y < grab.height(); ++y) {
    QRgb* row = reinterpret_cast<QRgb*>(out.scanLine(y));
    if (half)
      decodeRow(
        reinterpret_cast<const qfloat16*>(grab.constScanLine(y)), row, grab.width(), inverse);
    else
      decodeRow(reinterpret_cast<const float*>(grab.constScanLine(y)), row, grab.width(), inverse);
  }

  return out;
}

/**
 * @brief The SDR white multiplier of the window's active HDR output helper, 1 when the window
 *        renders SDR. The helper (Misc::HdrOutput, instantiated by HdrSurface.qml) is found by
 *        class name and read through its properties, so this file links without the graphics
 *        backend the helper itself depends on.
 */
double WindowCapture::sdrWhiteScaleOf(const QQuickWindow* window)
{
  SS_ASSERT(window != nullptr, return 1.0);
  const QQuickItem* content = window->contentItem();
  if (content == nullptr)
    return 1.0;

  const auto objects = content->findChildren<QObject*>();
  for (const QObject* object : objects) {
    if (!object->inherits(kHdrOutputClass) || !object->property("active").toBool())
      continue;

    if (object->property("window").value<QQuickWindow*>() != window)
      continue;

    return std::max(SerialStudio::toDouble(object->property("sdrWhiteScale")), kMinSdrScale);
  }

  return 1.0;
}

//--------------------------------------------------------------------------------------------------
// Encoding
//--------------------------------------------------------------------------------------------------

/**
 * @brief PNG bytes for @p image, empty when the encoder refuses.
 */
QByteArray WindowCapture::encodePng(const QImage& image)
{
  QByteArray bytes;
  QBuffer buffer(&bytes);
  if (image.isNull() || !buffer.open(QIODevice::WriteOnly))
    return QByteArray();

  if (!image.save(&buffer, "PNG"))
    return QByteArray();

  return bytes;
}

/**
 * @brief Grabs @p window, undoes the HDR transform, crops to @p crop (window coordinates, empty
 *        for the whole window), scales, and halves the scale until the base64 form of the PNG
 *        fits the cap or the minimum scale is reached.
 */
std::optional<Capture> WindowCapture::capture(QQuickWindow* window,
                                              const QRectF& crop,
                                              const double scale)
{
  SS_ASSERT(window != nullptr, return std::nullopt);
  const QImage grab = window->grabWindow();
  if (grab.isNull())
    return std::nullopt;

  QImage image = toDisplayReferred(grab, sdrWhiteScaleOf(window));
  if (!crop.isEmpty()) {
    const qreal ratio = window->devicePixelRatio();
    const QRectF scaled(crop.topLeft() * ratio, crop.size() * ratio);
    const QRect pixels = scaled.toAlignedRect().intersected(image.rect());
    if (pixels.isEmpty())
      return std::nullopt;

    image = image.copy(pixels);
  }

  double applied = std::clamp(scale, kMinScale, 1.0);
  for (int step = 0; step < kMaxShrinkSteps; ++step) {
    const QImage sized =
      applied < 1.0
        ? image.scaled(image.size() * applied, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        : image;
    const QByteArray png = encodePng(sized);
    if (png.isEmpty())
      return std::nullopt;

    const qsizetype encoded = ((png.size() + 2) / 3) * 4;
    if (encoded <= kMaxEncodedBytes || applied <= kMinScale)
      return Capture{png, sized.width(), sized.height(), applied};

    applied = std::max(kMinScale, applied * kHalf);
  }

  return std::nullopt;
}
