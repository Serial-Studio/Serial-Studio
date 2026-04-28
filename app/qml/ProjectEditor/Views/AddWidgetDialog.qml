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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Window {
  id: root

  //
  // Window configuration
  //
  property int workspaceId: -1
  property int titlebarHeight: 0

  width: 720
  height: 540
  minimumWidth: 620
  title: qsTr("Add Widget")
  minimumHeight: 420 + titlebarHeight

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Open the window for a specific workspace.
  //
  function open(wsId) {
    workspaceId = wsId
    refresh()
    show()
    raise()
    requestActivate()
  }

  //
  // Native window integration (mirrors ConstantsLibraryDialog)
  //
  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Refresh whenever the project changes so newly-added groups/datasets show
  // up in the picker and the "Already added" column stays accurate.
  //
  Connections {
    target: Cpp_JSON_ProjectModel
    function onGroupsChanged()           { if (root.visible) Qt.callLater(root.refresh) }
    function onEditorWorkspacesChanged() { if (root.visible) Qt.callLater(root.refresh) }
  }

  //
  // Shortcut to close
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Top title band (same treatment as DBCPreviewDialog / ConstantsLibraryDialog)
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

  DragHandler {
    target: null
    onActiveChanged: if (active) root.startSystemMove()
  }

  //
  // Data model — updated by refresh()
  //
  property var allWidgets: []
  property var existingKeys: []   // keys like "widgetType:groupId:relativeIndex"
  property string searchText: ""

  function widgetKey(row) {
    return row.widgetType + ":" + row.groupId + ":" + row.relativeIndex
  }

  function refresh() {
    allWidgets = Cpp_JSON_ProjectEditor.allWidgetsSummary()

    //
    // Build a set of widget keys already attached to the target workspace.
    //
    const existing = Cpp_JSON_ProjectEditor.widgetsForWorkspace(workspaceId)
    const keys = []
    for (let i = 0; i < existing.length; ++i)
      keys.push(widgetKey(existing[i]))

    existingKeys = keys
  }

  //
  // Filtered view
  //
  readonly property var filteredWidgets: {
    const q = String(searchText || "").toLowerCase().trim()
    const match = (s) => String(s || "").toLowerCase().indexOf(q) !== -1
    if (!q)
      return allWidgets

    return allWidgets.filter(w =>
      match(w.widgetLabel) ||
      match(w.groupTitle) ||
      match(w.datasetTitle)
    )
  }

  //
  // Content
  //
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
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        color: palette.text
        Layout.fillWidth: true
        text: qsTr("Available Widgets")
        font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
      }

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Click a row to add it to the workspace.")
      }

      Item { implicitHeight: 4 }

      Widgets.SearchField {
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["base"]
        placeholderText: qsTr("Search")
        onTextChanged: root.searchText = text
      }

      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: [
          { title: qsTr("Widget"), width: 160 },
          { title: qsTr("Group"),  width: 220 },
          { title: qsTr("Name"),   width: -1  },
          { title: "",             width: 48  }
        ]
      }

      ListView {
        id: list
        clip: true
        spacing: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        model: root.filteredWidgets
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: widgetRow

          property bool alreadyAdded: root.existingKeys.indexOf(root.widgetKey(modelData)) !== -1

          RowLayout {
            spacing: 0
            anchors.fill: parent

            Label {
              leftPadding: 8
              elide: Text.ElideRight
              Layout.preferredWidth: 160
              color: widgetRow.textColor
              text: modelData.widgetLabel
              Layout.alignment: Qt.AlignVCenter
              opacity: widgetRow.alreadyAdded ? 0.4 : 1
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: widgetRow.separatorColor
            }

            Label {
              leftPadding: 8
              elide: Text.ElideRight
              Layout.preferredWidth: 220
              text: modelData.groupTitle
              color: widgetRow.textColor
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: widgetRow.alreadyAdded ? 0.4 : 1
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: widgetRow.separatorColor
            }

            Label {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.isGroupWidget
                    ? qsTr("(entire group)")
                    : modelData.datasetTitle
              color: widgetRow.textColor
              font: Cpp_Misc_CommonFonts.monoFont
              opacity: widgetRow.alreadyAdded ? 0.4 : (modelData.isGroupWidget ? 0.65 : 1)
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: widgetRow.separatorColor
            }

            ToolButton {
              Layout.preferredWidth: 48
              Layout.preferredHeight: 26
              padding: 2
              flat: true
              hoverEnabled: true
              enabled: !widgetRow.alreadyAdded
              icon.width: 16
              icon.height: 16
              icon.source: widgetRow.alreadyAdded
                           ? "qrc:/rcc/icons/buttons/apply.svg"
                           : "qrc:/rcc/icons/buttons/plus.svg"
              icon.color: "transparent"

              background: Rectangle {
                border.width: 0
                color: "transparent"
              }

              ToolTip.visible: hovered
              ToolTip.delay: 400
              ToolTip.text: widgetRow.alreadyAdded
                            ? qsTr("Already in workspace")
                            : qsTr("Add to workspace")

              onClicked: {
                Cpp_JSON_ProjectModel.addWidgetToWorkspace(
                  root.workspaceId,
                  modelData.widgetType,
                  modelData.groupId,
                  modelData.relativeIndex
                )
                //
                // Optimistic local update so the row greys out immediately;
                // the next refresh() pulls the authoritative state.
                //
                const k = root.widgetKey(modelData)
                const next = root.existingKeys.slice()
                next.push(k)
                root.existingKeys = next
              }
            }
          }
        }

        Label {
          anchors.centerIn: parent
          opacity: 0.5
          color: palette.text
          visible: list.count === 0
          horizontalAlignment: Text.AlignHCenter
          text: root.allWidgets.length === 0
                ? qsTr("No widgets available.")
                : qsTr("No widgets match.")
        }
      }

      Item { implicitHeight: 4 }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          opacity: 0.7
          color: palette.text
          Layout.fillWidth: true
          text: root.allWidgets.length === root.filteredWidgets.length
                ? qsTr("%1 widgets").arg(root.allWidgets.length)
                : qsTr("%1 of %2 widgets").arg(root.filteredWidgets.length).arg(root.allWidgets.length)
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          horizontalPadding: 8
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          onClicked: root.close()
        }
      }
    }
  }
}
