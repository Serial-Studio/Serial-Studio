/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Window {
  id: root

  Widgets.WindowMirror {}

  //
  // Custom properties
  //
  property int titlebarHeight: 0

  width: 640
  height: 540
  minimumWidth: 520
  title: qsTr("OPC UA Tag Browser")
  minimumHeight: 420 + titlebarHeight

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window integration + browse session lifetime
  //
  //
  // Closing through the titlebar is a cancel: only the OK button commits the selection.
  //
  property bool commitOnClose: false

  onVisibleChanged: {
    if (visible) {
      _error.text = ""
      root.commitOnClose = false
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
      Cpp_IO_OpcUa.startBrowse()
    } else {
      Cpp_NativeWindow.removeWindow(root)
      if (root.commitOnClose)
        Cpp_IO_OpcUa.stopBrowse()
      else
        Cpp_IO_OpcUa.cancelBrowse()
    }

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Update window colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager

    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Surface browse failures in the status row
  //
  Connections {
    target: Cpp_IO_OpcUa

    function onBrowseFailed(reason) {
      _error.text = reason
    }
  }

  //
  // Top section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Titlebar text
  //
  Label {
    text: root.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Be able to drag/move the window
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  Page {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: column

      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Expand folders to browse the server. Tick the variables to subscribe to; ticking a folder selects every readable variable beneath it.")
      }

      Label {
        id: _error

        color: Cpp_ThemeManager.colors["error"]
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        visible: text.length > 0
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      }

      Label {
        visible: Cpp_IO_OpcUa.tagModel.overSoftLimit
        color: Cpp_ThemeManager.colors["error"]
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        text: qsTr("More than 512 channels selected; very wide frames slow the dashboard.")
      }

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        visible: Cpp_IO_OpcUa.tagModel.busy
        text: qsTr("Reading the address space...")
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      }

      Widgets.SearchField {
        id: _search

        Layout.fillWidth: true
        placeholderText: qsTr("Filter by name…")
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["base"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        TreeView {
          id: _tree

          clip: true
          reuseItems: true
          anchors.margins: 2
          anchors.fill: parent
          model: Cpp_IO_OpcUa.tagModel
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar {}

          //
          // The model walks down to the project's saved tags on its own; expansion is view
          // state, so it asks the view to open exactly those branches and nothing else.
          //
          Connections {
            target: Cpp_IO_OpcUa.tagModel

            function onExpandRequested(index) {
              Qt.callLater(() => {
                const row = _tree.rowAtIndex(index)
                if (row >= 0)
                  _tree.expand(row)
              })
            }
          }

          //
          // Row icon per node kind: folders, then the wire type of the variable.
          //
          function rowIcon(folder, typeCode) {
            if (folder)
              return Cpp_Misc_IconRegistry.icon("editor", "group", 16)

            if (typeCode === "bool")
              return Cpp_Misc_IconRegistry.icon("editor", "led", 16)

            if (typeCode === "str")
              return Cpp_Misc_IconRegistry.icon("editor", "dataset-values", 16)

            return Cpp_Misc_IconRegistry.icon("editor", "dataset", 16)
          }

          delegate: Item {
            id: _row

            required property int row
            required property int depth
            required property bool expanded
            required property bool hasChildren
            required property TreeView treeView

            readonly property real padding: 4
            readonly property real indentation: 16
            readonly property bool isFolder: model.folder === true
            readonly property bool canTick: isFolder || model.selectable === true
            readonly property bool filtered: _search.text.length > 0
                                             && !isFolder
                                             && String(model.name).toLowerCase()
                                                  .indexOf(_search.text.toLowerCase()) < 0

            visible: !filtered
            implicitWidth: _tree.width
            implicitHeight: filtered ? 0 : 24

            function toggle() {
              if (hasChildren)
                treeView.toggleExpanded(row)
            }

            //
            // Hover background + click-to-expand on the label area
            //
            Rectangle {
              anchors.fill: parent
              color: _hover.hovered ? Cpp_ThemeManager.colors["highlight"] : "transparent"
              opacity: _hover.hovered ? 0.35 : 1

              HoverHandler {
                id: _hover
              }

              MouseArea {
                anchors.fill: parent
                onDoubleClicked: _row.toggle()
                onClicked: {
                  if (_row.isFolder)
                    _row.toggle()
                  else if (_row.canTick)
                    Cpp_IO_OpcUa.tagModel.setChecked(_tree.index(_row.row, 0), model.checked !== 2)
                }
              }
            }

            RowLayout {
              spacing: 0
              anchors.fill: parent
              anchors.rightMargin: 8
              opacity: _row.canTick ? 1 : 0.5
              anchors.leftMargin: _row.padding + _row.depth * _row.indentation

              //
              // Greyed out once a browse proves the node is a leaf
              //
              Image {
                enabled: _row.hasChildren
                sourceSize: Qt.size(8, 8)
                rotation: _row.expanded ? 0 : 270
                Layout.alignment: Qt.AlignVCenter
                opacity: _row.hasChildren ? 1 : 0.2
                source: Cpp_Misc_IconRegistry.icon("editor", "indicator", 16)

                layer.enabled: !_row.hasChildren && Cpp_Misc_GraphicsBackend.effectsEnabled
                layer.effect: MultiEffect {
                  saturation: -1.0
                }

                MouseArea {
                  anchors.fill: parent
                  enabled: _row.hasChildren
                  onClicked: _row.toggle()
                }
              }

              Item {
                width: 6
              }

              CheckBox {
                padding: 0
                enabled: _row.canTick
                tristate: _row.isFolder
                Layout.alignment: Qt.AlignVCenter
                checkState: model.checked === 2 ? Qt.Checked
                          : model.checked === 1 ? Qt.PartiallyChecked
                          : Qt.Unchecked
                onClicked: Cpp_IO_OpcUa.tagModel.setChecked(
                             _tree.index(_row.row, 0), checkState !== Qt.Unchecked)
              }

              Item {
                width: 4
              }

              Image {
                sourceSize: Qt.size(12, 12)
                Layout.alignment: Qt.AlignVCenter
                source: _tree.rowIcon(_row.isFolder, model.typeCode)
              }

              Item {
                width: 4
              }

              Label {
                elide: Text.ElideRight
                maximumLineCount: 1
                wrapMode: Text.NoWrap
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                text: model.name
                color: Cpp_ThemeManager.colors["text"]
                font: _row.depth === 0 ? Cpp_Misc_CommonFonts.boldUiFont
                                       : Cpp_Misc_CommonFonts.uiFont
                ToolTip.delay: 500
                ToolTip.visible: _hover.hovered && String(model.nodeId).length > 0
                ToolTip.text: model.nodeId + (model.description ? "\n" + model.description : "")
              }

              Label {
                opacity: 0.7
                visible: !_row.isFolder
                Layout.rightMargin: 8
                Layout.alignment: Qt.AlignVCenter
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customMonoFont(0.85)
                text: (model.typeCode + (model.arrayLen > 1 ? "[" + model.arrayLen + "]" : ""))
                      + (model.unit ? " " + model.unit : "")
              }

              Label {
                opacity: 0.7
                visible: !_row.isFolder
                Layout.alignment: Qt.AlignVCenter
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85)
                text: model.access
              }
            }
          }
        }
      }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          opacity: 0.7
          color: palette.text
          Layout.fillWidth: true
          text: qsTr("%1 tag(s), %2 channel(s) selected")
                  .arg(Cpp_IO_OpcUa.tagModel.selectedCount)
                  .arg(Cpp_IO_OpcUa.tagModel.selectedIndices)
        }

        Widgets.IconButton {
          leftPadding: 8
          text: qsTr("Select All Readable")
          icon.source: "qrc:/icons/buttons/select-all.svg"
          onClicked: Cpp_IO_OpcUa.tagModel.selectAllReadable()
        }

        Widgets.IconButton {
          leftPadding: 8
          text: qsTr("Cancel")
          icon.source: "qrc:/icons/buttons/close.svg"
          onClicked: root.close()
        }

        Widgets.IconButton {
          leftPadding: 8
          text: qsTr("OK")
          icon.source: "qrc:/icons/buttons/apply.svg"
          onClicked: {
            root.commitOnClose = true
            root.close()
          }
        }
      }
    }
  }
}
