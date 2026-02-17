/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "Bar.h"

namespace Widgets {
/**
 * @class Widgets::Gauge
 * @brief Circular gauge widget for displaying normalized value data.
 *
 * The Gauge class provides a circular/arc-style gauge display, inheriting all
 * functionality from the Bar widget but presenting the data in a traditional
 * gauge visualization rather than a linear bar.
 *
 * This widget shares all features of the Bar class including:
 * - Value range normalization
 * - Alarm threshold visualization
 * - Unit display
 * - Alarm state indication
 *
 * The only difference from Bar is the visual presentation style, making it
 * suitable for dashboards where circular gauges are preferred over linear
 * bars.
 *
 * @see Widgets::Bar for detailed functionality documentation
 */
class Gauge : public Bar {
  Q_OBJECT

public:
  explicit Gauge(const int index = -1, QQuickItem* parent = nullptr);

private slots:
  void updateData() override;
};
}  // namespace Widgets
