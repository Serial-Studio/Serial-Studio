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

#pragma once

#include <optional>
#include <QByteArray>
#include <QImage>
#include <QRectF>

class QQuickWindow;

namespace UI::Automation {

/**
 * @brief One finished capture: PNG bytes, the pixel size of the image inside them, and the
 *        scale actually applied after the size cap.
 */
struct Capture {
  QByteArray png;
  int width;
  int height;
  double scale;
};

/**
 * @brief Renders a window to PNG for the screenshot command (spec 0101): grabs the swapchain,
 *        undoes the HDR output transform when the grab is floating point (the window composites
 *        SDR inside an FP16 layer, spec 0089), crops to an element, scales, and halves the scale
 *        until the base64 reply fits the named cap, which sits under the socket's write cap.
 */
class WindowCapture {
public:
  static constexpr qsizetype kMaxEncodedBytes = 4 * 1024 * 1024;
  static constexpr double kMinScale           = 0.1;
  static constexpr int kMaxShrinkSteps        = 6;

  [[nodiscard]] static std::optional<Capture> capture(QQuickWindow* window,
                                                      const QRectF& crop,
                                                      double scale);
  [[nodiscard]] static QImage toDisplayReferred(const QImage& grab, double sdrWhiteScale);
  [[nodiscard]] static double sdrWhiteScaleOf(const QQuickWindow* window);
  [[nodiscard]] static QByteArray encodePng(const QImage& image);
  [[nodiscard]] static bool isFloatFormat(QImage::Format format) noexcept;
};

}  // namespace UI::Automation
