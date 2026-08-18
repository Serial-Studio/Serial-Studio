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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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

  //
  // Sweep retention walks a doubling ladder (spec 0061): stepping one sweep at a time up to
  // 64 is a chore, and the memory cost doubles with it
  //
  readonly property var retentionLadder: [0, 1, 2, 4, 8, 16, 32, 64]

  //
  // Next/previous retention count from @p current in the @p direction, clamped to the ladder
  //
  function stepRetention(current, direction)
  {
    const ladder = root.retentionLadder
    let index = 0
    for (let i = 0; i < ladder.length; ++i)
      if (ladder[i] <= current)
        index = i

    const next = Math.max(0, Math.min(ladder.length - 1, index + direction))
    return ladder[next]
  }
}
