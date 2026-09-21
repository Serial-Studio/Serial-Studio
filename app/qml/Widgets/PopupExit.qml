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

//
// Shared popup exit, the counterpart of PopupEnter. No `from` values: a close that interrupts
// the entrance continues from wherever the popup already is instead of snapping to full size.
//
Transition {
  id: root

  property int duration: 120
  property real toScale: 0.98

  ParallelAnimation {
    NumberAnimation {
      to: 0
      property: "opacity"
      duration: root.duration
      easing.type: Easing.InCubic
    }

    NumberAnimation {
      property: "scale"
      duration: root.duration
      easing.type: Easing.InCubic
      to: Cpp_Misc_GraphicsBackend.reduceMotion ? 1 : root.toScale
    }
  }
}
