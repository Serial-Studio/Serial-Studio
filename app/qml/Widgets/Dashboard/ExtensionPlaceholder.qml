/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

Item {
  id: root

  implicitWidth: 280
  implicitHeight: 200

  //
  // Widget data inputs (kept for delegate symmetry; the tile renders no data)
  //
  property var color
  property var model
  property var windowRoot
  property string widgetId

  //
  // Failure description: which widget could not be created, and why
  //
  property string title: ""
  property string reason: ""
  property string extensionId: ""

  //
  // A package waiting for a decision is not broken, it is inert until the user answers
  //
  readonly property bool awaitingConsent: root.extensionId.length > 0
                                          && Cpp_UI_WidgetExtensions.consentRequired(root.extensionId)
                                          && !Cpp_UI_WidgetExtensions.consentGranted(root.extensionId)

  //
  // A failed widget is a visible, explained tile: never a blank slot
  //
  ColumnLayout {
    spacing: 12
    anchors.centerIn: parent
    width: Math.min(parent.width - 32, 420)

    Image {
      sourceSize: Qt.size(32, 32)
      Layout.alignment: Qt.AlignHCenter
      fillMode: Image.PreserveAspectFit
      source: Cpp_Misc_IconRegistry.icon("notifications", "warning", 32)
    }

    Label {
      elide: Label.ElideRight
      text: root.title.length > 0 ? root.title : qsTr("Widget Extension")
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.boldUiFont
      horizontalAlignment: Label.AlignHCenter
    }

    Label {
      wrapMode: Label.WordWrap
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["placeholder_text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.95, false)
      horizontalAlignment: Label.AlignHCenter
      text: {
        if (root.awaitingConsent)
          return qsTr("This widget is installed but has not been allowed to run.")

        return root.reason.length > 0 ? root.reason
                                      : qsTr("This widget could not be loaded.")
      }
    }

    Button {
      visible: root.awaitingConsent
      text: qsTr("Review and Allow…")
      Layout.alignment: Qt.AlignHCenter
      onClicked: app.showExtensionConsent(root.extensionId)
    }

    Button {
      visible: !root.awaitingConsent
      text: qsTr("Open Problem Center")
      Layout.alignment: Qt.AlignHCenter
      onClicked: app.showProblemCenter()
    }
  }
}
