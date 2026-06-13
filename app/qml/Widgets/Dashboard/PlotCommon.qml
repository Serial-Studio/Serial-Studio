/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import QtQuick

import SerialStudio

QtObject {
  id: root

  readonly property var interpolationModes: [SerialStudio.InterpolationNone,
                                             SerialStudio.InterpolationLinear,
                                             SerialStudio.InterpolationZoh,
                                             SerialStudio.InterpolationStem]

  //
  // Time-axis plots downsample only the visible X window at screen resolution (zoom
  // narrows the sample scan); other plots keep the zoom-scaled full-range detail.
  //
  function setDownsampleFactor(plot, model)
  {
    if (!plot || !model)
      return

    if (plot.timeAxis) {
      model.dataW = plot.plotArea.width
      model.dataH = plot.plotArea.height
      model.setVisibleXWindow(plot.xVisibleMin, plot.xVisibleMax)
    }

    else {
      const z = plot.zoom
      model.dataW = plot.plotArea.width * z
      model.dataH = plot.plotArea.height * z
    }
  }

  function normalizeInterpolationMode(value)
  {
    const idx = root.interpolationModes.indexOf(value)
    return idx >= 0 ? value : SerialStudio.InterpolationLinear
  }

  function nextInterpolationMode(current)
  {
    const idx = root.interpolationModes.indexOf(current)
    return root.interpolationModes[(idx + 1) % root.interpolationModes.length]
  }

  function modeLabel(mode)
  {
    if (mode === SerialStudio.InterpolationNone)
      return qsTr("None")

    if (mode === SerialStudio.InterpolationZoh)
      return qsTr("ZOH")

    if (mode === SerialStudio.InterpolationStem)
      return qsTr("Stem")

    return qsTr("Linear")
  }

  function canShowAreaUnderPlot(mode)
  {
    return mode !== SerialStudio.InterpolationNone
      && mode !== SerialStudio.InterpolationStem
  }
}
