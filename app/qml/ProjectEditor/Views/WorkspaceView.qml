/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_Misc_IconRegistry.icon("widgets", "group", 16)

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  property int workspaceId: Cpp_JSON_ProjectEditor.selectedWorkspaceId
  property string workspaceName: Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
  property string workspaceIcon: Cpp_JSON_ProjectModel.workspaceIcon(workspaceId)
  property var widgets: []

  title: workspaceName.length > 0 ? workspaceName : qsTr("Workspace")

  actionComponent: EditorNavActions {}

  function refresh() {
    workspaceName = Cpp_JSON_ProjectModel.workspaceTitle(workspaceId)
    workspaceIcon = Cpp_JSON_ProjectModel.workspaceIcon(workspaceId)
    widgets = workspaceId >= 0
              ? Cpp_JSON_ProjectEditor.widgetsForWorkspace(workspaceId)
              : []
  }

  onWorkspaceIdChanged: Qt.callLater(refresh)
  onVisibleChanged: if (visible) Qt.callLater(refresh)
  Component.onCompleted: Qt.callLater(refresh)

  Connections {
    target: Cpp_JSON_ProjectModel
    function onEditorWorkspacesChanged() { Qt.callLater(root.refresh) }
    function onGroupsChanged()           { Qt.callLater(root.refresh) }
    function onWidgetDisplayChanged()    { Qt.callLater(root.refresh) }
  }

  //
  // Apply the icon chosen in the shared picker to this workspace.
  //
  Connections {
    target: actionIconPicker

    function onIconSelected(icon) {
      if (Cpp_JSON_ProjectEditor.currentView === ProjectEditor.WorkspaceView)
        Cpp_JSON_ProjectModel.setWorkspaceIcon(root.workspaceId, icon)
    }
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
      // Secondary toolbar: Move Up, Move Down, Change Icon, Rename, Delete
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

        RowLayout {
          id: toolbarLayout

          spacing: 4
          anchors {
            margins: 8
            topMargin: 0
            bottomMargin: 0
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Add Widget")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Add widget to workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: addWidgetDialog.open(root.workspaceId)
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "add-dataset", 24)
          }

          Item { Layout.fillWidth: true }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Up")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move workspace up")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && root.workspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(root.workspaceId, -1)
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "move-up", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Move Down")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Move workspace down")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
                     && root.workspaceId >= 5000
            onClicked: Cpp_JSON_ProjectEditor.moveWorkspace(root.workspaceId, 1)
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "move-down", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Change Icon")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Change workspace icon")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: {
              actionIconPicker.selectedIcon = Cpp_Misc_IconEngine.isInlineSvg(
                root.workspaceIcon) ? "" : root.workspaceIcon
              actionIconPicker.showNormal()
            }
            icon.source: root.workspaceIcon.length > 0
                         ? Cpp_Misc_IconEngine.resolveActionIconSource(root.workspaceIcon)
                         : Cpp_Misc_IconRegistry.icon("widgets", "workspace", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Rename")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Rename workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.promptRenameWorkspace(root.workspaceId)
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "rename-table", 24)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete workspace")
            enabled: Cpp_JSON_ProjectModel.customizeWorkspaces
            onClicked: Cpp_JSON_ProjectModel.confirmDeleteWorkspace(root.workspaceId)
            icon.source: Cpp_Misc_IconRegistry.icon("editor", "delete-table", 24)
          }
        }
      }

      //
      // Picker window for the "Add Widget" toolbar button.
      //
      AddWidgetDialog {
        id: addWidgetDialog
      }

      //
      // Widgets list: header + rows via the shared ProjectTable components.
      //
      Widgets.ProjectTableHeader {
        Layout.fillWidth: true
        columns: Cpp_JSON_ProjectModel.customizeWorkspaces
                 ? [
                     { title: qsTr("Group"),         width: 200 },
                     { title: qsTr("Widget"),        width: -1  },
                     { title: qsTr("Display Title"), width: 180 },
                     { title: qsTr("Freeze Title"),  width: 110 },
                     { title: qsTr("Type"),          width: 100 },
                     { title: "",                    width: 40  }
                   ]
                 : [
                     { title: qsTr("Group"),   width: 220 },
                     { title: qsTr("Widget"),  width: -1  },
                     { title: qsTr("Type"),    width: 100 }
                   ]
      }

      ListView {
        id: list

        clip: true
        spacing: 0
        model: root.widgets
        Layout.fillWidth: true
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        delegate: Widgets.ProjectTableRow {
          id: refRow

          readonly property bool typeKnown: (modelData.widgetTypeName || "").length > 0
          readonly property bool groupResolved: modelData.groupTitle.length > 0
          readonly property bool unknownRow: !refRow.groupResolved && !refRow.typeKnown
          readonly property bool unresolvedRow: !refRow.groupResolved && refRow.typeKnown
          readonly property color rowTextColor: refRow.unknownRow
                                                ? Cpp_ThemeManager.colors["alarm"]
                                                : (refRow.unresolvedRow
                                                   ? Cpp_ThemeManager.colors["error"]
                                                   : refRow.textColor)

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            Label {
              Layout.preferredWidth: Cpp_JSON_ProjectModel.customizeWorkspaces ? 200 : 220
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.groupTitle.length > 0
                    ? modelData.groupTitle
                    : qsTr("(unknown)")
              color: refRow.rowTextColor
              font: Cpp_Misc_CommonFonts.monoFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Label {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              leftPadding: 8
              elide: Text.ElideRight
              text: modelData.datasetTitle.length > 0
                    ? modelData.datasetTitle
                    : qsTr("(group widget)")
              color: refRow.rowTextColor
              font: Cpp_Misc_CommonFonts.monoFont
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
            }

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 180
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces

              TextField {
                anchors.margins: 3
                anchors.fill: parent
                text: modelData.displayTitle
                enabled: modelData.uniqueId >= 0
                font: Cpp_Misc_CommonFonts.uiFont
                placeholderText: modelData.fallbackTitle
                onEditingFinished: {
                  if (text !== modelData.displayTitle)
                    Cpp_JSON_ProjectModel.setWidgetDisplayTitle(modelData.widgetType,
                                                                modelData.uniqueId, text)
                }

                background: Item {}
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
            }

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 110
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces

              ComboBox {
                id: _freezeCombo

                readonly property bool paintsTitle:
                  SerialStudio.dashboardWidgetPaintsTitle(modelData.widgetType)
                readonly property var modeKeys:
                  paintsTitle ? ["bar", "painted", "hidden"] : ["bar", "hidden"]

                flat: true
                anchors.fill: parent
                anchors.margins: 3
                font: Cpp_Misc_CommonFonts.uiFont
                enabled: modelData.uniqueId >= 0
                model: paintsTitle
                       ? [qsTr("Title Bar"), qsTr("Painted Title"), qsTr("Hidden")]
                       : [qsTr("Title Bar"), qsTr("Hidden")]
                currentIndex: Math.max(0, modeKeys.indexOf(modelData.freezeTitleMode))
                onActivated: (index) => {
                  Cpp_JSON_ProjectModel.setFreezeTitleMode(modelData.widgetType,
                                                           modelData.uniqueId, modeKeys[index])
                }

                contentItem: Text {
                  elide: Text.ElideRight
                  font: _freezeCombo.font
                  color: refRow.textColor
                  text: _freezeCombo.displayText
                  horizontalAlignment: Text.AlignLeft
                  verticalAlignment: Text.AlignVCenter
                  leftPadding: root.rtl ? _freezeCombo.indicator.width : 6
                  rightPadding: root.rtl ? 6 : _freezeCombo.indicator.width
                }
              }
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
            }

            Label {
              opacity: 0.7
              elide: Text.ElideRight
              Layout.preferredWidth: 100
              color: refRow.rowTextColor
              leftPadding: root.rtl ? 0 : 8
              rightPadding: root.rtl ? 8 : 0
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.uiFont
              text: modelData.widgetTypeName || qsTr("(unknown)")
            }

            Rectangle {
              implicitWidth: 1
              Layout.fillHeight: true
              color: refRow.separatorColor
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces
            }

            Item {
              Layout.fillHeight: true
              Layout.preferredWidth: 40
              visible: Cpp_JSON_ProjectModel.customizeWorkspaces

              ToolButton {
                id: removeBtn

                padding: 2
                flat: true
                icon.width: 16
                icon.height: 16
                implicitWidth: 32
                hoverEnabled: true
                ToolTip.delay: 400
                implicitHeight: 26
                anchors.centerIn: parent
                ToolTip.visible: hovered
                icon.color: "transparent"
                icon.source: "qrc:/icons/buttons/trash.svg"
                ToolTip.text: qsTr("Remove widget from workspace")

                background: Rectangle {
                  border.width: 0
                  color: "transparent"
                }

                onClicked: Cpp_JSON_ProjectModel.removeWidgetFromWorkspace(
                             root.workspaceId, index)
              }
            }
          }
        }

        Label {
          opacity: 0.5
          color: Cpp_ThemeManager.colors["text"]
          anchors.centerIn: parent
          visible: list.count === 0
          horizontalAlignment: Text.AlignHCenter
          text: qsTr("No widgets in this workspace.")
        }
      }
    }
  }
}
