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
import QtQuick.Controls

//
// Ruler context menu (right-click outside cursor mode). Naming a new marker needs a prompt
// the menu cannot host, so that one action leaves as a signal.
//
Menu {
  id: root

  //
  // The PlotWidget whose ruler this menu edits
  //
  required property Item plot

  //
  // World X of the press that opened the menu, and the marker it landed on (-1 = none)
  //
  property real pressWorldX: 0
  property int pressMarker: -1

  //
  // The user asked for a new marker at @p worldX; the plot prompts for its name
  //
  signal addMarkerRequested(real worldX)

  MenuItem {
    text: qsTr("Add marker here...")
    onTriggered: root.addMarkerRequested(root.pressWorldX)
  }

  MenuItem {
    visible: root.pressMarker >= 0
    height: visible ? implicitHeight : 0
    text: qsTr("Remove marker \"%1\"").arg(root.pressMarker >= 0
                                         ? root.plot.xMarkers[root.pressMarker].name : "")
    onTriggered: root.plot.removeMarker(root.pressMarker)
  }

  MenuItem {
    text: qsTr("Clear all markers")
    enabled: root.plot.xMarkers.length > 0
    onTriggered: root.plot.clearMarkers()
  }

  MenuSeparator {}

  MenuItem {
    enabled: !root.plot.logX
    text: root.plot.timeAxis ? qsTr("Set time zero here") : qsTr("Set zero here")
    onTriggered: root.plot.setZeroAt(root.pressWorldX)
  }

  MenuItem {
    enabled: root.plot.xZeroSet
    text: root.plot.timeAxis ? qsTr("Reset time zero") : qsTr("Reset zero")
    onTriggered: root.plot.resetZero()
  }

  MenuSeparator {}

  MenuItem {
    checkable: true
    text: qsTr("Hover marker")
    checked: root.plot.hoverMarkerEnabled
    onTriggered: root.plot.setHoverMarker(checked)
  }
}
