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

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_Misc_IconRegistry.icon("editor", "shared-table-alt", 16)

  actionComponent: EditorNavActions {}
  title: {
    const i = root.tableName.lastIndexOf('/')
    return i >= 0 ? root.tableName.substring(i + 1) : root.tableName
  }

  readonly property int rowHeight: 30
  readonly property int colTypeWidth: 140
  readonly property int colNameWidth: 220
  readonly property int colActionWidth: 80
  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property string tableName: Cpp_JSON_ProjectEditor.selectedUserTable

  function registerAccessCode(regName) {
    return "tableGet(" + JSON.stringify(root.tableName) + ", " + JSON.stringify(regName) + ")"
  }
  property var registers: []

  property bool committing: false
  function refresh() {
    if (tableName.length > 0)
      registers = Cpp_JSON_ProjectModel.registersForTable(tableName)
    else
      registers = []
  }

  function commitRegister(oldName, newName, computed, value) {
    root.committing = true
    const ok = Cpp_JSON_ProjectModel.updateRegister(tableName, oldName, newName, computed, value)
    root.committing = false

    if (!ok)
      Qt.callLater(root.refresh)

    return ok
  }

  onTableNameChanged: Qt.callLater(refresh)
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onTablesChanged() {
      if (root.committing)
        return

      Qt.callLater(root.refresh)
    }
  }

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

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Add Variable")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.promptAddRegister(root.tableName)
              ToolTip.text: qsTr("Add variable")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "add-register", 24)
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Insert Constant")
              Layout.alignment: Qt.AlignVCenter
              onClicked: constantsLibraryDialog.open(root.tableName)
              ToolTip.text: qsTr("Insert constant")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "insert-constant", 24)
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Import")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.importTableFromCsv(root.tableName)
              ToolTip.text: qsTr("Import variables from CSV")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "import-table", 24)
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Export")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.exportTableToCsv(root.tableName)
              ToolTip.text: qsTr("Export variables to CSV")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "export-table", 24)
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 16
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Rename")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.promptRenameTable(root.tableName)
              ToolTip.text: qsTr("Rename table")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "rename-table", 24)
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Delete")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.confirmDeleteTable(root.tableName)
              ToolTip.text: qsTr("Delete table")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "delete-table", 24)
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              Layout.maximumHeight: 48
              Layout.alignment: Qt.AlignVCenter
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Help")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              onClicked: app.showHelpCenter("data-tables")
              icon.source: Cpp_Misc_IconRegistry.icon("code", "help", 24)
              ToolTip.text: qsTr("Open help documentation for variables")
            }
          }
        }

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

      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        rowHeight: root.rowHeight
        columns: [
          { title: qsTr("Permissions"), width: root.colTypeWidth   },
          { title: qsTr("Variable Name"), width: root.colNameWidth },
          { title: qsTr("Default Value"), width: -1                },
          { title: "",                  width: root.colActionWidth }
        ]
      }

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

          //
          // Live per-row edit state; modelData goes stale after a successful edit.
          //
          property var committedValue: modelData.value
          property string committedName: modelData.name
          property bool committedComputed: modelData.type === "computed"

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            Item { width: 8 }

            Widgets.Combo {
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
                          ? Cpp_Misc_IconRegistry.icon("editor", "read-write", 16)
                          : Cpp_Misc_IconRegistry.icon("editor", "read-only", 16)
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
                if (computed !== regRow.committedComputed) {
                  if (root.commitRegister(regRow.committedName, regRow.committedName,
                                          computed, regRow.committedValue))
                    regRow.committedComputed = computed
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            Item { width: 8 }

            Widgets.LineField {
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
                if (text !== regRow.committedName && text.length > 0) {
                  if (root.commitRegister(regRow.committedName, text,
                                          regRow.committedComputed, regRow.committedValue))
                    regRow.committedName = text
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            Item { width: 8 }

            Widgets.LineField {
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
                if (root.commitRegister(regRow.committedName, regRow.committedName,
                                        regRow.committedComputed, newVal))
                  regRow.committedValue = newVal
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: regRow.separatorColor
            }

            RowLayout {
              spacing: 0
              Layout.fillHeight: true
              Layout.preferredWidth: root.colActionWidth

              ToolButton {
                id: copyRegBtn

                readonly property string accessCode: root.registerAccessCode(regRow.committedName)

                padding: 2
                flat: true
                icon.width: 16
                icon.height: 16
                hoverEnabled: true
                ToolTip.delay: 400
                ToolTip.visible: hovered
                icon.color: Cpp_ThemeManager.colors["text"]
                Layout.preferredHeight: 26
                icon.source: "qrc:/icons/buttons/copy.svg"
                Layout.preferredWidth: root.colActionWidth / 2
                ToolTip.text: qsTr("Copy access code %1 to clipboard").arg(copyRegBtn.accessCode)

                background: Rectangle {
                  border.width: 0
                  color: "transparent"
                }

                onClicked: {
                  Cpp_Misc_Utilities.copyText(copyRegBtn.accessCode)
                  copyToast.show()
                }
              }

              ToolButton {
                id: deleteRegBtn

                padding: 2
                flat: true
                icon.width: 16
                icon.height: 16
                hoverEnabled: true
                icon.color: Cpp_ThemeManager.colors["text"]
                Layout.preferredHeight: 26
                icon.source: "qrc:/icons/buttons/trash.svg"
                Layout.preferredWidth: root.colActionWidth / 2

                background: Rectangle {
                  border.width: 0
                  color: "transparent"
                }

                ToolTip.delay: 400
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Delete variable")

                onClicked: Cpp_JSON_ProjectModel.confirmDeleteRegister(root.tableName,
                                                                       regRow.committedName)
              }
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
            text: qsTr("No variables.")
          }
        }
      }
    }

    //
    // Floating "Variable access code copied" toast.
    //
    Rectangle {
      id: copyToast

      z: 1000
      opacity: 0
      radius: 4
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 24
      anchors.horizontalCenter: parent.horizontalCenter
      width: copyToastLabel.implicitWidth + 24
      height: copyToastLabel.implicitHeight + 12
      color: Cpp_ThemeManager.colors["highlight"]
      visible: opacity > 0

      function show() {
        copyToast.opacity = 1
        copyToastTimer.restart()
      }

      RowLayout {
        id: copyToastLabel

        spacing: 8
        anchors.centerIn: parent

        ToolButton {
          flat: true
          padding: 0
          enabled: false
          icon.width: 14
          icon.height: 14
          Layout.preferredWidth: 14
          Layout.preferredHeight: 14
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/icons/buttons/apply.svg"
          icon.color: Cpp_ThemeManager.colors["highlighted_text"]
          background: Item {}
        }

        Label {
          Layout.alignment: Qt.AlignVCenter
          text: qsTr("Register access code copied")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["highlighted_text"]
        }
      }

      Timer {
        id: copyToastTimer

        repeat: false
        interval: 1500
        onTriggered: copyToast.opacity = 0
      }

      Behavior on opacity {
        NumberAnimation { duration: 150 }
      }
    }
  }
}
