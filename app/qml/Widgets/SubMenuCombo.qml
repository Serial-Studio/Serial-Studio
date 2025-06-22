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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Popup {
  id: root

  //
  // Set widget properties
  //
  width: 180
  focus: true
  modal: false
  closePolicy: Popup.CloseOnPressOutside
  height: Math.max(minimumHeight, Math.min(_listView.contentHeight + 16, maximumHeight))

  //
  // External data
  //
  property var model
  property int minimumHeight: 32
  property int maximumHeight: 320
  property string valueRole: "id"
  property string iconRole: "icon"
  property string textRole: "text"
  property string checkedRole: "checked"
  property bool showCheckable: false
  property alias placeholderText: _placeholder.text
  property alias currentIndex: _listView.currentIndex

  //
  // Signals
  //
  signal valueSelected(var value)

  //
  // Internal state
  //
  property var currentValue: null

  //
  // Control background
  //
  background: Rectangle {
    border.width: 1
    color: Cpp_ThemeManager.colors["start_menu_background"]
    border.color: Cpp_ThemeManager.colors["start_menu_border"]
  }

  //
  // Place holder text
  //
  Label {
    id: _placeholder

    opacity: 0.8
    anchors.centerIn: parent
    visible: _listView.count <= 0
    text: qsTr("No Data Available")
    color: Cpp_ThemeManager.colors["start_menu_text"]
  }

  //
  // Menu content
  //
  ListView {
    id: _listView

    clip: true
    currentIndex: -1
    model: root.model
    interactive: true
    anchors.fill: parent
    flickableDirection: Flickable.AutoFlickIfNeeded

    delegate: Item {
      required property var modelData

      height: 24
      width: root.width - 16

      Rectangle {
        anchors.fill: parent
        visible: _mouseArea.containsMouse
        color: Cpp_ThemeManager.colors["start_menu_highlight"]
      }

      RowLayout {
        id: _layout
        spacing: 0
        anchors.fill: parent

        Component.onCompleted: {
          if (implicitWidth > root.width - 16)
            root.width = implicitWidth + 32
        }

        Item {
          implicitWidth: 6
        }

        Image {
          sourceSize.width: 16
          sourceSize.height: 16
          source: modelData[root.iconRole]
        }

        Item {
          implicitWidth: 4
        }

        Label {
          Layout.fillWidth: true
          elide: Label.ElideRight
          text: modelData[root.textRole]
          color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                            Cpp_ThemeManager.colors["start_menu_text"]
        }

        Item {
          implicitWidth: 4
          visible: root.showCheckable
        }

        ToolButton {
          icon.width: 16
          icon.height: 16
          background: Item{}
          icon.source: "qrc:/rcc/icons/buttons/apply.svg"
          visible: modelData[root.checkedRole] !== undefined ? modelData[root.checkedRole] :
                                                               root.showCheckable &&  root.currentValue === modelData[root.valueRole]
          icon.color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                                 Cpp_ThemeManager.colors["start_menu_text"]
        }
      }

      MouseArea {
        id: _mouseArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
          root.currentValue = modelData[root.valueRole]
          root.valueSelected(root.currentValue)
          root.close()
        }
      }
    }
  }
}
