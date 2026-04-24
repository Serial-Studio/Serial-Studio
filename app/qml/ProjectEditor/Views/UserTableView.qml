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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  title: Cpp_JSON_ProjectEditor.selectedUserTable
  icon: "qrc:/rcc/icons/project-editor/treeview/shared-table.svg"

  //
  // Table layout constants — match the ProjectEditor convention used by
  // TableDelegate (ActionView, DatasetView, GroupView, SourceView).
  //
  readonly property int rowHeight: 30
  readonly property int colTypeWidth: 140
  readonly property int colNameWidth: 220
  readonly property int colActionWidth: 40
  readonly property int minBannerHeight: 84

  property string tableName: Cpp_JSON_ProjectEditor.selectedUserTable
  property var registers: []

  //
  // Skip one external refresh after our own edit. Without this, updateRegister
  // fires tablesChanged, refresh() rebuilds `registers`, the ListView destroys
  // the delegate that emitted onTextEdited, and the user loses focus mid-type.
  //
  property int suppressRefresh: 0

  function refresh() {
    if (tableName.length > 0)
      registers = Cpp_JSON_ProjectModel.registersForTable(tableName)
    else
      registers = []
  }

  function commitRegister(oldName, newName, computed, value) {
    suppressRefresh += 1
    Cpp_JSON_ProjectModel.updateRegister(tableName, oldName, newName, computed, value)
  }

  onTableNameChanged: Qt.callLater(refresh)
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onTablesChanged() {
      if (root.suppressRefresh > 0) {
        root.suppressRefresh -= 1
        return
      }
      Qt.callLater(root.refresh)
    }
  }

  //
  // Constants library — preset physics/math values that can be inserted into
  // this table as constant registers in one click.
  //
  ConstantsLibraryDialog {
    id: constantsLibraryDialog
  }

  Page {
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

    anchors {
      fill: parent
      leftMargin: -9
      topMargin: -16
      rightMargin: -9
      bottomMargin: -10
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Banner with icon on left, title/description, and action buttons
      //
      Rectangle {
        id: banner
        readonly property int topPad: 12
        readonly property int bottomPad: 12

        Layout.topMargin: -1
        Layout.fillWidth: true
        implicitHeight: Math.max(root.minBannerHeight,
                                 bannerLayout.implicitHeight + topPad + bottomPad)
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.top: parent.top
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        RowLayout {
          id: bannerLayout
          spacing: 16
          anchors {
            leftMargin: 14
            rightMargin: 14
            left: parent.left
            right: parent.right
            topMargin: banner.topPad
            bottomMargin: banner.bottomPad
            verticalCenter: parent.verticalCenter
          }

          Image {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 56
            sourceSize: Qt.size(56, 56)
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
            source: "qrc:/rcc/icons/project-editor/summary/shared-table.svg"
          }

          ColumnLayout {
            spacing: 4
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            Label {
              text: root.tableName
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
            }

            Label {
              opacity: 0.65
              Layout.fillWidth: true
              wrapMode: Text.WordWrap
              color: Cpp_ThemeManager.colors["text"]
              text: qsTr("Registers shared across all transforms in the project.")
            }
          }

          Label {
            Layout.alignment: Qt.AlignVCenter
            opacity: 0.6
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("%1 registers").arg(root.registers.length)
          }
        }
      }

      //
      // Secondary toolbar — ActionView-style horizontal Flickable with icon
      // buttons and left/right fade indicators.
      //
      Rectangle {
        id: toolbar

        z: 2
        Layout.fillWidth: true
        height: toolbarLayout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Rectangle {
          height: 1
          width: parent.width
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Flickable {
          id: toolbarFlick

          clip: true
          contentHeight: height
          height: toolbarLayout.implicitHeight
          boundsBehavior: Flickable.StopAtBounds
          flickableDirection: Flickable.HorizontalFlick
          contentWidth: toolbarLayout.implicitWidth + 16

          anchors {
            margins: 8
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          ScrollBar.horizontal: ScrollBar {
            height: 3
            policy: toolbarFlick.contentWidth > toolbarFlick.width
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
          }

          RowLayout {
            id: toolbarLayout

            spacing: 4
            anchors.verticalCenter: parent.verticalCenter
            width: Math.max(implicitWidth, toolbarFlick.width)

            //
            // Add register
            //
            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Add Register")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.promptAddRegister(root.tableName)
              ToolTip.text: qsTr("Add register")
              icon.source: "qrc:/rcc/icons/project-editor/actions/add-register.svg"
            }

            //
            // Insert preset constant
            //
            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Insert Constant")
              Layout.alignment: Qt.AlignVCenter
              onClicked: constantsLibraryDialog.open(root.tableName)
              ToolTip.text: qsTr("Insert constant")
              icon.source: "qrc:/rcc/icons/project-editor/actions/insert-constant.svg"
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Import")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.importTableFromCsv(root.tableName)
              ToolTip.text: qsTr("Import registers from CSV")
              icon.source: "qrc:/rcc/icons/project-editor/actions/import-table.svg"
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Export")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.exportTableToCsv(root.tableName)
              ToolTip.text: qsTr("Export registers to CSV")
              icon.source: "qrc:/rcc/icons/project-editor/actions/export-table.svg"
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 16
            }

            //
            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Rename")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.promptRenameTable(root.tableName)
              ToolTip.text: qsTr("Rename table")
              icon.source: "qrc:/rcc/icons/project-editor/actions/rename-table.svg"
            }

            //
            // Delete table
            //
            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Delete")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.confirmDeleteTable(root.tableName)
              ToolTip.text: qsTr("Delete table")
              icon.source: "qrc:/rcc/icons/project-editor/actions/delete-table.svg"
            }

            //
            // Spacer
            //
            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              Layout.maximumHeight: 48
              Layout.alignment: Qt.AlignVCenter
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            //
            // Help
            //
            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Help")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              onClicked: app.showHelpCenter("data-tables")
              icon.source: "qrc:/rcc/icons/code-editor/help.svg"
              ToolTip.text: qsTr("Open help documentation for shared memory")
            }
          }
        }

        //
        // Left fade indicator
        //
        Rectangle {
          z: 10
          width: 16
          anchors.top: toolbarFlick.top
          anchors.left: toolbarFlick.left
          visible: toolbarFlick.contentX > 4
          anchors.bottom: toolbarFlick.bottom

          gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: Cpp_ThemeManager.colors["groupbox_background"] }
            GradientStop { position: 1; color: "transparent" }
          }
        }

        //
        // Right fade indicator
        //
        Rectangle {
          z: 10
          width: 16
          anchors.top: toolbarFlick.top
          anchors.right: toolbarFlick.right
          anchors.bottom: toolbarFlick.bottom
          visible: toolbarFlick.contentX + toolbarFlick.width < toolbarFlick.contentWidth - 4

          gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: "transparent" }
            GradientStop { position: 1; color: Cpp_ThemeManager.colors["groupbox_background"] }
          }
        }
      }

      //
      // Column headers — match the gradient + vertical-separator pattern
      // used by ProjectTableHeader.
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        rowHeight: root.rowHeight
        columns: [
          { title: qsTr("Permissions"), width: root.colTypeWidth   },
          { title: qsTr("Register Name"), width: root.colNameWidth },
          { title: qsTr("Default Value"), width: -1                },
          { title: "",                  width: root.colActionWidth }
        ]
      }

      //
      // Register list
      //
      ListView {
        id: regList
        clip: true
        spacing: 0
        interactive: true
        model: root.registers
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: regRow
          rowHeight: root.rowHeight

          RowLayout {
            spacing: 0
            anchors.fill: parent

            Item { width: 8 }

            //
            // Permissions picker — transparent background, drop-down arrow
            // provided by ComboBox. The content is a small SVG glyph (R or
            // R/W) followed by the label, matching the rest of the project
            // editor's iconography.
            //
            ComboBox {
              id: typeCombo
              Layout.preferredWidth: root.colTypeWidth - 8
              Layout.preferredHeight: 24
              Layout.alignment: Qt.AlignVCenter
              model: [qsTr("Read-Only"), qsTr("Read/Write")]
              currentIndex: modelData.type === "computed" ? 1 : 0

              background: Item {}

              contentItem: RowLayout {
                spacing: 6

                Image {
                  Layout.preferredWidth: 14
                  Layout.preferredHeight: 14
                  Layout.alignment: Qt.AlignVCenter
                  fillMode: Image.PreserveAspectFit
                  sourceSize: Qt.size(14, 14)
                  source: typeCombo.currentIndex === 1
                          ? "qrc:/rcc/icons/project-editor/permissions/read-write.svg"
                          : "qrc:/rcc/icons/project-editor/permissions/read-only.svg"
                }

                Label {
                  leftPadding: 0
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  color: regRow.textColor
                  text: typeCombo.displayText
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.monoFont
                  horizontalAlignment: Text.AlignLeft
                  verticalAlignment: Text.AlignVCenter
                }
              }

              onActivated: (idx) => {
                const computed = idx === 1
                if (computed !== (modelData.type === "computed")) {
                  root.commitRegister(modelData.name, modelData.name, computed, modelData.value)
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            Item { width: 8 }

            //
            // Register name — transparent text field that merges visually with
            // the row; focus ring is a thin highlight line along the bottom.
            //
            TextField {
              id: nameField
              topPadding: 0
              leftPadding: 0
              rightPadding: 0
              bottomPadding: 0
              selectByMouse: true
              text: modelData.name
              color: regRow.textColor
              Layout.preferredHeight: 24
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.preferredWidth: root.colNameWidth - 8

             background: Item {}

              onTextEdited: {
                if (text !== modelData.name && text.length > 0) {
                  root.commitRegister(modelData.name,
                                      text,
                                      modelData.type === "computed",
                                      modelData.value)
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            Item { width: 8 }

            //
            // Default-value field — same transparent treatment as name.
            //
            TextField {
              id: valueField
              topPadding: 0
              leftPadding: 0
              rightPadding: 0
              bottomPadding: 0
              selectByMouse: true
              Layout.fillWidth: true
              color: regRow.textColor
              Layout.preferredHeight: 24
              text: String(modelData.value)
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont

              background: Item {}

              onTextEdited: {
                const newVal = modelData.valueType === "number"
                                 ? parseFloat(text || "0")
                                 : text
                root.commitRegister(modelData.name,
                                    modelData.name,
                                    modelData.type === "computed",
                                    newVal)
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            //
            // Delete register — transparent button, SVG icon.
            //
            ToolButton {
              id: deleteRegBtn
              padding: 2
              flat: true
              icon.width: 16
              icon.height: 16
              hoverEnabled: true
              icon.color: "transparent"
              Layout.preferredHeight: 26
              Layout.preferredWidth: root.colActionWidth
              icon.source: "qrc:/rcc/icons/buttons/trash.svg"

              background: Rectangle {
                border.width: 0
                color: "transparent"
              }

              ToolTip.delay: 400
              ToolTip.visible: hovered
              ToolTip.text: qsTr("Delete register")

              onClicked: Cpp_JSON_ProjectModel.confirmDeleteRegister(root.tableName, modelData.name)
            }
          }
        }

        footer: Item {
          height: 40
          width: ListView.view ? ListView.view.width : 0

          Label {
            anchors.centerIn: parent
            opacity: 0.5
            color: Cpp_ThemeManager.colors["text"]
            visible: root.registers.length === 0
            text: qsTr("No registers.")
          }
        }
      }
    }
  }
}
