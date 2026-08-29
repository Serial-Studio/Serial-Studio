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

//
// Icon vocabulary for the project flow diagram. Every icon resolves through
// Misc::IconRegistry, never through a hardcoded qrc path.
//
QtObject {
  id: icons

  //
  // Device card icon for a bus type.
  //
  function busTypeIcon(busType) {
    const names = ["uart", "network", "bluetooth", "audio", "modbus",
                   "canbus", "usb", "hid", "process", "mqtt", "opcua",
                   "s7", "ethernetip", "iec104"]
    return Cpp_Misc_IconRegistry.icon("devices", names[busType] || "uart", 24)
  }

  //
  // Group card icon: the group's widget when it has a known one, and the
  // output-panel icon for output groups.
  //
  function groupIcon(grp) {
    if (grp.groupType === SerialStudio.GroupOutput)
      return Cpp_Misc_IconRegistry.icon("widgets", "output-panel", 16)

    const w = (grp.widget || "").toLowerCase()
    switch (w) {
      case "multiplot":
      case "accelerometer":
      case "gyroscope":
      case "gps":
      case "image":
      case "painter":
      case "plot3d":
      case "datagrid":
      case "barpanel":
        return Cpp_Misc_IconRegistry.icon("widgets", w, 16)
      default:
        return Cpp_Misc_IconRegistry.icon("widgets", "group", 16)
    }
  }

  //
  // Dataset pill icon.
  //
  function datasetIcon() {
    return Cpp_Misc_IconRegistry.icon("editor", "dataset", 16)
  }

  //
  // Control pill icon for an output widget type.
  //
  function outputWidgetIcon(type) {
    let name = "output-button"
    switch (type) {
      case SerialStudio.OutputSlider:    name = "output-slider";    break
      case SerialStudio.OutputToggle:    name = "output-toggle";    break
      case SerialStudio.OutputTextField: name = "output-textfield"; break
      case SerialStudio.OutputKnob:      name = "output-knob";      break
    }

    return Cpp_Misc_IconRegistry.icon("editor", name, 16)
  }
}
