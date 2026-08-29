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
import QtQuick.Layouts
import QtQuick.Controls

//
// Marker name prompt: centered over the plot-area overlay it is parented to
//
Popup {
  id: root

  //
  // The PlotWidget the accepted marker is added to
  //
  required property Item plot

  //
  // World X the marker will be placed at once the name is accepted
  //
  property real worldX: 0

  padding: 8
  modal: true
  focus: true
  x: Math.round((parent.width - width) / 2)
  y: Math.round((parent.height - height) / 2)

  function openAt(worldXValue) {
    worldX = worldXValue
    _markerName.text = qsTr("M%1").arg(root.plot.xMarkers.length + 1)
    open()
    _markerName.forceActiveFocus()
    _markerName.selectAll()
  }

  function accept() {
    const name = _markerName.text.trim()
    root.plot.addMarker(worldX, name.length > 0
                                ? name : qsTr("M%1").arg(root.plot.xMarkers.length + 1))
    close()
  }

  contentItem: RowLayout {
    spacing: 6

    Label {
      text: qsTr("Marker name:")
      color: Cpp_ThemeManager.colors["widget_text"]
    }

    TextField {
      id: _markerName

      Layout.preferredWidth: 140
      onAccepted: root.accept()
    }

    Button {
      text: qsTr("Add")
      onClicked: root.accept()
    }

    Button {
      text: qsTr("Cancel")
      onClicked: root.close()
    }
  }
}
