/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Controls as Controls

Controls.SpinBox {
  //
  // Voice the Toggle Changed event on a user edit only, never on a programmatic value
  // (spec 0087 R16)
  //
  onValueModified: Cpp_UI_Alarms.playEvent("toggle")
}
