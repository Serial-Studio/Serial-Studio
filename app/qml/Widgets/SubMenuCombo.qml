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

  Component.onCompleted: {
    contentItem.LayoutMirroring.enabled = Qt.binding(() => Cpp_Misc_Translator.rtl)
    contentItem.LayoutMirroring.childrenInherit = true
  }

  //
  // External data
  //
  property var model
  property int minimumHeight: 32
  property int maximumHeight: 320
  property string valueRole: "id"
  property string iconRole: "icon"
  property string textRole: "text"
  property bool showCheckable: false
  property string checkedRole: "checked"
  property alias placeholderText: _placeholder.text
  property alias currentIndex: _listView.currentIndex

  //
  // Signals
  //
  signal valueSelected(var value)
  signal valueRightClicked(var value, var text, real globalX, real globalY)

  //
  // Internal state
  //
  property var currentValue: null

  //
  // Cascading folders: a `folder` row opens a child popup as the next submenu level.
  //
  property var childPopup: null
  property var parentCombo: null
  property var folderComponent: null

  onClosed: if (childPopup) childPopup.close()

  function setRootModel(rootModel) {
    if (childPopup)
      childPopup.close()

    root.model = rootModel
  }

  function folderRow(node) {
    if (node.isFolder)
      return { "folder": true, "separator": false, "children": node.children,
               "id": node.id, "text": node.text, "icon": node.icon }

    var entry = { "id": node.id, "separator": false,
                  "text": node.text, "icon": node.icon }
    if (root.currentValue !== null && node.id === root.currentValue)
      entry["checked"] = true

    return entry
  }

  function folderModel(node) {
    var rows = []
    var kids = node.children
    for (var i = 0; i < kids.length; ++i)
      rows.push(root.folderRow(kids[i]))

    return rows
  }

  function openFolder(node) {
    if (folderComponent === null)
      folderComponent = Qt.createComponent(Qt.resolvedUrl("SubMenuCombo.qml"))

    if (childPopup === null && folderComponent.status === Component.Ready) {
      childPopup = folderComponent.createObject(root.parent, { "parentCombo": root })
      childPopup.valueSelected.connect((v) => root.valueSelected(v))
      childPopup.valueRightClicked.connect((v, t, gx, gy) => root.valueRightClicked(v, t, gx, gy))
    }

    if (childPopup === null)
      return

    if (childPopup.childPopup)
      childPopup.childPopup.close()

    childPopup.showCheckable = root.showCheckable
    childPopup.currentValue = root.currentValue
    childPopup.maximumHeight = root.maximumHeight
    childPopup.model = root.folderModel(node)

    var overlayWidth = root.parent ? root.parent.width : root.x + root.width + childPopup.width
    var rightX = root.x + root.width - 1
    childPopup.x = (rightX + childPopup.width > overlayWidth)
                   ? root.x - childPopup.width + 1
                   : rightX
    childPopup.y = root.y
    childPopup.open()
  }

  function closeChain() {
    root.close()
    if (parentCombo)
      parentCombo.closeChain()
  }

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

    ScrollBar.vertical: ScrollBar {
      policy: ScrollBar.AsNeeded
    }

    delegate: Item {
      required property var modelData

      readonly property bool isSeparator: modelData[root.valueRole] === "__separator__"
                                        || modelData["separator"] === true
      readonly property bool isHeader: modelData["header"] === true
      readonly property bool isFolder: modelData["folder"] === true

      width: root.width - 16
      height: isSeparator ? 9 : 24

      //
      // Separator line
      //
      Rectangle {
        visible: isSeparator
        opacity: 0.5
        implicitHeight: 1
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        color: Cpp_ThemeManager.colors["start_menu_text"]
      }

      //
      // Hover highlight
      //
      Rectangle {
        anchors.fill: parent
        visible: !isSeparator && !isHeader && _mouseArea.containsMouse
        color: Cpp_ThemeManager.colors["start_menu_highlight"]
      }

      RowLayout {
        id: _layout

        spacing: 0
        anchors.fill: parent
        visible: !isSeparator

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
          elide: Label.ElideRight
          font: isHeader ? Cpp_Misc_CommonFonts.boldUiFont : Cpp_Misc_CommonFonts.uiFont
          Layout.fillWidth: true
          text: modelData[root.textRole]
          color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                            Cpp_ThemeManager.colors["start_menu_text"]
        }

        ToolButton {
          icon.width: 12
          icon.height: 12
          visible: isFolder
          background: Item {}
          icon.source: "qrc:/icons/buttons/forward.svg"
          icon.color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                                 Cpp_ThemeManager.colors["start_menu_text"]
        }

        Item {
          implicitWidth: 4
          visible: root.showCheckable && !isFolder
        }

        ToolButton {
          icon.width: 16
          icon.height: 16
          background: Item{}
          icon.source: "qrc:/icons/buttons/apply.svg"
          //
          // Only show the checkmark when the model entry carries `checked`
          //
          visible: modelData[root.checkedRole] === true
          icon.color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                                 Cpp_ThemeManager.colors["start_menu_text"]
        }
      }

      MouseArea {
        id: _mouseArea

        hoverEnabled: true
        anchors.fill: parent
        enabled: !isSeparator && !isHeader
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onContainsMouseChanged: if (containsMouse && isFolder) root.openFolder(modelData)
        onClicked: (mouse) => {
          if (isFolder) {
            root.openFolder(modelData)
          } else if (mouse.button === Qt.RightButton) {
            const global = mapToGlobal(mouse.x, mouse.y)
            root.valueRightClicked(modelData[root.valueRole],
                                   modelData[root.textRole],
                                   global.x, global.y)
          } else {
            root.currentValue = modelData[root.valueRole]
            root.valueSelected(root.currentValue)
            root.closeChain()
          }
        }
      }
    }
  }
}
