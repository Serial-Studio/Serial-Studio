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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets
import "../../Widgets/Dashboard" as Dashboard

Widgets.Pane {
  id: root

  title: qsTr("Console")
  icon: Cpp_Misc_IconRegistry.icon("widgets", "terminal", 16)

  Layout.minimumWidth: terminal.implicitWidth
  Layout.minimumHeight: terminal.implicitHeight

  //
  // Utility functions between terminal widget & main window
  //
  function sendData() {
    terminal.sendData()
  } function clear() {
    terminal.clear()
  } function copy() {
    terminal.copy()
  } function selectAll() {
    terminal.selectAll()
  }

  //
  // Use page item to set application palette
  //
  Widgets.ThemedPage {
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

    Dashboard.Terminal {
      id: terminal

      minimal: false
      anchors.fill: parent
    }
  }
}
