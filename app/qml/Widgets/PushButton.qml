/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Controls

Button {
  //
  // Voice the Button Pressed event on every click (spec 0087 R16)
  //
  onClicked: Cpp_UI_Alarms.playEvent("button")
}
