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
// Shared popup entrance: grows from Popup.transformOrigin rather than sliding, because a
// Popup's x/y are usually bound and scale is not. Reduce Motion keeps the fade, drops the growth.
//
Transition {
  id: root

  property int duration: 160
  property real fromScale: 0.96

  ParallelAnimation {
    NumberAnimation {
      from: 0; to: 1
      property: "opacity"
      duration: root.duration
      easing.type: Easing.OutCubic
    }

    NumberAnimation {
      to: 1
      property: "scale"
      duration: root.duration
      easing.type: Easing.OutCubic
      from: Cpp_Misc_GraphicsBackend.reduceMotion ? 1 : root.fromScale
    }
  }
}
