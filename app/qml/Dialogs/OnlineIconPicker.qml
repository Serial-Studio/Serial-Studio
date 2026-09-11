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

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Custom signals and properties
  //
  signal iconSelected(var icon)
  property int selectedIndex: -1

  //
  // Window options
  //
  staysOnTop: true
  title: qsTr("Search Online Icons")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Reset state when shown
  //
  onVisibleChanged: {
    if (visible) {
      root.selectedIndex = -1
      errorLabel.visible = false
      searchField.forceActiveFocus()
    }
  }

  //
  // Clears any previous error before starting a new search
  //
  function search(query) {
    errorLabel.visible = false
    root.selectedIndex = -1
    Cpp_Misc_IconEngine.searchIcons(query)
  }

  //
  // Download the selected icon when the user confirms
  //
  Connections {
    target: Cpp_Misc_IconEngine

    function onIconDownloaded(svgData) {
      root.iconSelected(svgData)
      root.close()
    }

    function onIconDownloadFailed(error) {
      errorLabel.text = qsTr("Download failed: %1").arg(error)
      errorLabel.visible = true
    }

    function onSearchFailed(error) {
      errorLabel.text = error
      errorLabel.visible = true
    }
  }

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 4

    //
    // Search bar
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.LineField {
        id: searchField

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        placeholderText: qsTr("Search icons (e.g. temperature, arrow, play)…")
        color: Cpp_ThemeManager.colors["text"]

        onAccepted: root.search(text)

        Keys.onEnterPressed: root.search(text)
        Keys.onReturnPressed: root.search(text)
      }

      Widgets.IconButton {
        iconSize: 14
        highlighted: true
        horizontalPadding: 8
        text: qsTr("Search…")
        icon.source: "qrc:/icons/buttons/search.svg"
        onClicked: root.search(searchField.text)
        enabled: !Cpp_Misc_IconEngine.busy && searchField.text.length > 0
      }
    }

    //
    // Results grid
    //
    Rectangle {
      radius: 2
      border.width: 1
      implicitWidth: 560
      implicitHeight: 480
      Layout.fillWidth: true
      Layout.fillHeight: true
      border.color: Cpp_ThemeManager.colors["groupbox_border"]
      color: Cpp_ThemeManager.colors["groupbox_background"]

      //
      // Loading indicator
      //
      BusyIndicator {
        anchors.centerIn: parent
        running: Cpp_Misc_IconEngine.busy
        visible: Cpp_Misc_IconEngine.busy
      }

      //
      // Empty state label
      //
      Label {
        anchors.centerIn: parent
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        visible: !Cpp_Misc_IconEngine.busy
                 && Cpp_Misc_IconEngine.iconNames.length === 0
        text: qsTr("Search for icons above to get started")
      }

      //
      // Icon grid
      //
      GridView {
        id: gridView

        clip: true
        cellWidth: 64
        cellHeight: 64
        anchors.fill: parent
        anchors.margins: 4
        ScrollBar.vertical: ScrollBar {}
        model: Cpp_Misc_IconEngine.iconNames
        visible: !Cpp_Misc_IconEngine.busy

        delegate: Item {
          id: iconDelegate

          width: 64
          height: 64

          required property int index
          required property string modelData

          Rectangle {
            width: 56
            height: 56
            radius: 4
            anchors.centerIn: parent
            color: root.selectedIndex === iconDelegate.index
               ? Cpp_ThemeManager.colors["highlight"]
               : iconDelegateArea.containsMouse
                 ? Cpp_ThemeManager.colors["alternate_base"]
                 : "transparent"
          }

          Image {
            id: iconPreview

            readonly property bool resolved: iconPreview.source.toString().length > 0

            asynchronous: true
            anchors.centerIn: parent
            sourceSize: Qt.size(40, 40)
            fillMode: Image.PreserveAspectFit
            source: Cpp_Misc_IconEngine.iconPreviews[iconDelegate.index] || ""

            Label {
              text: "?"
              anchors.centerIn: parent
              font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              visible: iconPreview.status === Image.Error
                       || (!iconPreview.resolved && !Cpp_Misc_IconEngine.loadingPreviews)
            }
          }

          //
          // Per-tile spinner: previews arrive one icon set at a time
          //
          BusyIndicator {
            width: 24
            height: 24
            running: visible
            anchors.centerIn: parent
            visible: !iconPreview.resolved && Cpp_Misc_IconEngine.loadingPreviews
          }

          MouseArea {
            id: iconDelegateArea

            hoverEnabled: true
            anchors.fill: parent
            onClicked: root.selectedIndex = iconDelegate.index
            onDoubleClicked: {
              root.selectedIndex = iconDelegate.index
              Cpp_Misc_IconEngine.downloadIcon(iconDelegate.index)
            }
          }

          ToolTip.delay: 500
          ToolTip.text: iconDelegate.modelData
          ToolTip.visible: iconDelegateArea.containsMouse
        }
      }
    }

    //
    // Error label
    //
    Label {
      id: errorLabel

      visible: false
      Layout.fillWidth: true
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["alarm"]
    }

    Item {
      implicitHeight: 4
    }

    //
    // Action buttons
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("OK")
        highlighted: true
        horizontalPadding: 8
        icon.source: "qrc:/icons/buttons/apply.svg"
        enabled: root.selectedIndex >= 0 && !Cpp_Misc_IconEngine.busy
        onClicked: Cpp_Misc_IconEngine.downloadIcon(root.selectedIndex)
      }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Cancel")
        icon.source: "qrc:/icons/buttons/close.svg"
        onClicked: root.close()
      }
    }
  }
}
