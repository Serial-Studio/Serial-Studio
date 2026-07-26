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

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window properties
  //
  staysOnTop: true
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight
  title: qsTr("Allow Widget Extension?")

  //
  // Package under review, plus the ones waiting behind it
  //
  property var queue: []
  property string packageId: ""
  readonly property var info: root.packageId.length > 0
                              ? Cpp_UI_WidgetExtensions.packageInfo(root.packageId)
                              : ({})

  //
  // Queues one or more package ids and shows the first undecided one
  //
  function enqueue(ids) {
    for (var i = 0; i < ids.length; ++i) {
      if (root.queue.indexOf(ids[i]) < 0 && ids[i] !== root.packageId)
        root.queue.push(ids[i])
    }

    if (root.packageId.length <= 0)
      root.showNext()
  }

  //
  // Moves to the next queued package, closing the dialog when none is left
  //
  function showNext() {
    root.packageId = root.queue.length > 0 ? root.queue.shift() : ""
    if (root.packageId.length <= 0) {
      root.close()
      return
    }

    root.show()
    root.raise()
  }

  //
  // Records the decision and moves on
  //
  function decide(allow) {
    if (allow)
      Cpp_UI_WidgetExtensions.grantConsent(root.packageId)
    else
      Cpp_UI_WidgetExtensions.declineConsent(root.packageId)

    root.showNext()
  }

  //
  // Dialog contents
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12

    //
    // Header: package artwork and identity
    //
    RowLayout {
      spacing: 16
      Layout.fillWidth: true

      Image {
        Layout.preferredWidth: 48
        Layout.preferredHeight: 48
        sourceSize: Qt.size(48, 48)
        Layout.alignment: Qt.AlignTop
        fillMode: Image.PreserveAspectFit
        source: {
          const art = Cpp_UI_WidgetExtensions.iconUrl(root.packageId, 48)
          return art.length > 0 ? art
                                : Cpp_Misc_IconRegistry.icon("commands", "extensions", 48)
        }
      }

      ColumnLayout {
        spacing: 4
        Layout.fillWidth: true

        Label {
          wrapMode: Label.Wrap
          Layout.fillWidth: true
          Layout.maximumWidth: 420
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.boldUiFont
          text: root.info["title"] !== undefined ? root.info["title"] : root.packageId
        }

        Label {
          opacity: 0.8
          wrapMode: Label.Wrap
          Layout.fillWidth: true
          Layout.maximumWidth: 420
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.9)
          visible: root.info["description"] !== undefined
                   && root.info["description"].length > 0
          text: root.info["description"] !== undefined ? root.info["description"] : ""
        }
      }
    }

    //
    // Identity block: what it is, who published it, and where it came from
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      Layout.preferredHeight: detailsLayout.implicitHeight + 16

      GridLayout {
        id: detailsLayout

        columns: 2
        rowSpacing: 4
        columnSpacing: 12
        anchors.margins: 8
        anchors.fill: parent

        Repeater {
          model: [
            { "label": qsTr("Identifier"), "value": root.packageId },
            { "label": qsTr("Author"),     "value": root.info["author"] },
            { "label": qsTr("Version"),    "value": root.info["version"] },
            { "label": qsTr("License"),    "value": root.info["license"] },
            { "label": qsTr("Installed in"), "value": root.info["path"] }
          ]

          delegate: RowLayout {
            id: detailRow

            required property var modelData

            spacing: 12
            Layout.fillWidth: true
            Layout.columnSpan: 2
            visible: detailRow.modelData["value"] !== undefined
                     && String(detailRow.modelData["value"]).length > 0

            Label {
              opacity: 0.7
              Layout.minimumWidth: 96
              color: Cpp_ThemeManager.colors["text"]
              text: detailRow.modelData["label"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.9)
            }

            Label {
              elide: Label.ElideMiddle
              Layout.fillWidth: true
              Layout.maximumWidth: 380
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.9)
              text: String(detailRow.modelData["value"])
            }
          }
        }
      }
    }

    //
    // The privilege statement: plain language, no promises the architecture cannot keep
    //
    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 480
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.uiFont
      text: qsTr("This widget runs inside Serial Studio with the same privileges as the "
               + "application itself. It can read and change files on this computer, use "
               + "the network, and do anything Serial Studio can do. Serial Studio does not "
               + "restrict what it does, and cannot undo what it does.")
    }

    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 480
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.uiFont
      text: qsTr("Allow it only if you trust its author. Your answer is remembered for this "
               + "version of the package; a later update asks again.")
    }

    //
    // Decision buttons
    //
    RowLayout {
      spacing: 12
      Layout.topMargin: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Don't Allow")
        onClicked: root.decide(false)
        font: Cpp_Misc_CommonFonts.uiFont
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        highlighted: true
        text: qsTr("Allow")
        horizontalPadding: 8
        onClicked: root.decide(true)
        font: Cpp_Misc_CommonFonts.uiFont
        icon.source: "qrc:/icons/buttons/apply.svg"
      }
    }
  }
}
