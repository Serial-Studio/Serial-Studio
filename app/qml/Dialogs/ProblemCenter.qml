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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartWindow {
  id: root

  //
  // Window geometry & title
  //
  width: 720
  height: 480
  minimumWidth: 640
  minimumHeight: 420
  category: "ProblemCenter"
  title: qsTr("Problem Center")

  //
  // Titlebar height reported by the native window integration
  //
  property int titlebarHeight: 0

  //
  // Severity filter: -1 shows every finding, 0/1/2 pin one severity
  //
  property int severityFilter: -1

  //
  // Row count the list actually shows under the active severity filter
  //
  readonly property int filteredCount: {
    if (severityFilter === 2)
      return Cpp_Misc_ProblemCenter.errorCount

    if (severityFilter === 1)
      return Cpp_Misc_ProblemCenter.warningCount

    if (severityFilter === 0)
      return Cpp_Misc_ProblemCenter.infoCount

    return Cpp_Misc_ProblemCenter.totalCount
  }

  //
  // Native window integration (unified titlebar on macOS)
  //
  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Re-apply native styling when the theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Severity (0 info, 1 warning, 2 error) to icon; called outside any render
  // slot so the request size is the delegate's 16 px image
  //
  function iconForSeverity(severity) {
    switch (severity) {
    case 2: return Cpp_Misc_IconRegistry.icon("notifications", "critical", 16)
    case 1: return Cpp_Misc_IconRegistry.icon("notifications", "warning", 16)
    default: return Cpp_Misc_IconRegistry.icon("notifications", "info", 16)
    }
  }

  //
  // Severity to title color
  //
  function colorForSeverity(severity) {
    switch (severity) {
    case 2: return Cpp_ThemeManager.colors["alarm_critical"]
    case 1: return Cpp_ThemeManager.colors["alarm_warning"]
    default: return Cpp_ThemeManager.colors["link"]
    }
  }

  //
  // Severity to translated label
  //
  function labelForSeverity(severity) {
    switch (severity) {
    case 2: return qsTr("Error")
    case 1: return qsTr("Warning")
    default: return qsTr("Information")
    }
  }

  //
  // Window background
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["window"]
  }

  //
  // Titlebar drag strip; SmartWindow provides no drag surface of its own, so each derived
  // window covers its unified-titlebar area with a handler
  //
  Item {
    height: root.titlebarHeight
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }
  }

  //
  // Titlebar text, shown only when the native titlebar is unified
  //
  Label {
    text: root.title
    anchors.topMargin: 6
    anchors.top: parent.top
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    anchors.horizontalCenter: parent.horizontalCenter
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
  }

  //
  // Content area with the theme palette
  //
  Page {
    padding: 0
    anchors.margins: 16
    anchors.fill: parent
    anchors.topMargin: 8 + root.titlebarHeight

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
      spacing: 8
      anchors.fill: parent

      //
      // Header: severity counts, filter and manual re-run
      //
      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("%1 errors, %2 warnings, %3 notices")
                .arg(Cpp_Misc_ProblemCenter.errorCount)
                .arg(Cpp_Misc_ProblemCenter.warningCount)
                .arg(Cpp_Misc_ProblemCenter.infoCount)
        }

        Item {
          Layout.fillWidth: true
        }

        ComboBox {
          id: filterBox

          currentIndex: 0
          font: Cpp_Misc_CommonFonts.uiFont
          Layout.minimumWidth: 160
          model: [qsTr("All Severities"),
                  root.labelForSeverity(2),
                  root.labelForSeverity(1),
                  root.labelForSeverity(0)]
          onCurrentIndexChanged: {
            if (count <= 0)
              return

            root.severityFilter = currentIndex === 0 ? -1 : 3 - currentIndex
          }
        }

        Widgets.IconButton {
          iconSize: 16
          horizontalPadding: 8
          opacity: enabled ? 1 : 0.5
          icon.source: "qrc:/icons/buttons/test.svg"
          enabled: !Cpp_Misc_ConnectionDiagnostics.running
          onClicked: Cpp_Misc_ConnectionDiagnostics.runAll()
          text: Cpp_Misc_ConnectionDiagnostics.running ?
                  qsTr("Running Diagnostics") + "..." :
                  qsTr("Run Diagnostics")
        }

        Widgets.IconButton {
          iconSize: 16
          horizontalPadding: 8
          text: qsTr("Refresh")
          onClicked: Cpp_Misc_ProblemCenter.runNow()
          icon.source: "qrc:/icons/buttons/refresh.svg"
        }

        Widgets.IconButton {
          iconSize: 16
          text: qsTr("Clear")
          horizontalPadding: 8
          opacity: enabled ? 1 : 0.5
          onClicked: Cpp_Misc_ProblemCenter.clear()
          icon.source: "qrc:/icons/buttons/clear.svg"
          enabled: Cpp_Misc_ProblemCenter.totalCount > 0
        }
      }

      //
      // Findings list
      //
      Rectangle {
        clip: true
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: Cpp_ThemeManager.colors["table_cell_bg"]
        border.color: Cpp_ThemeManager.colors["groupbox_hard_border"]

        ListView {
          id: listView

          clip: true
          spacing: 0
          anchors.margins: 2
          anchors.fill: parent
          model: Cpp_Misc_ProblemCenter
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
          }

          delegate: Rectangle {
            id: findingRow

            required property int index
            required property int severity
            required property string jump
            required property string title
            required property string remedy
            required property string checkerId
            required property string explanation

            readonly property bool filtered: root.severityFilter >= 0
                                             && root.severityFilter !== findingRow.severity

            width: listView.width
            visible: !findingRow.filtered
            height: findingRow.filtered ? 0 : rowContent.implicitHeight + 16
            color: (findingRow.index % 2 === 0)
                   ? Cpp_ThemeManager.colors["table_cell_bg"]
                   : Cpp_ThemeManager.colors["alternate_base"]

            RowLayout {
              id: rowContent

              spacing: 8
              anchors.margins: 8
              anchors.fill: parent

              Image {
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                sourceSize: Qt.size(16, 16)
                Layout.alignment: Qt.AlignTop
                source: root.iconForSeverity(findingRow.severity)
              }

              ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                RowLayout {
                  spacing: 6
                  Layout.fillWidth: true

                  Label {
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    text: findingRow.title
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: root.colorForSeverity(findingRow.severity)
                  }

                  Label {
                    leftPadding: 6
                    rightPadding: 6
                    text: findingRow.checkerId
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    background: Rectangle {
                      radius: 3
                      border.width: 1
                      color: "transparent"
                      border.color: Cpp_ThemeManager.colors["groupbox_border"]
                    }
                  }
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Text.WordWrap
                  text: findingRow.explanation
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["text"]
                  visible: findingRow.explanation.length > 0
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Text.WordWrap
                  text: findingRow.remedy
                  font: Cpp_Misc_CommonFonts.uiFont
                  visible: findingRow.remedy.length > 0
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                }
              }

              Widgets.IconButton {
                iconSize: 16
                text: qsTr("Go To")
                horizontalPadding: 8
                Layout.alignment: Qt.AlignVCenter
                visible: findingRow.jump.length > 0
                icon.source: "qrc:/icons/buttons/apply.svg"
                onClicked: Cpp_Misc_ProblemCenter.activate(findingRow.index)
              }
            }
          }

        }

        //
        // Empty state: sibling of the ListView (a Flickable child would be parented into the
        // zero-height contentItem and clipped) and aware of the severity filter
        //
        ColumnLayout {
          spacing: 12
          anchors.centerIn: parent
          visible: root.filteredCount === 0
          width: Math.min(parent.width - 32, 320)

          Image {
            opacity: 0.35
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            sourceSize: Qt.size(48, 48)
            Layout.alignment: Qt.AlignHCenter
            fillMode: Image.PreserveAspectFit
            source: Cpp_Misc_IconRegistry.icon("notifications", "info", 48)
          }

          Label {
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["text"]
            horizontalAlignment: Text.AlignHCenter
            font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
            text: Cpp_Misc_ProblemCenter.totalCount > 0
                  ? qsTr("No problems match the current filter")
                  : qsTr("No problems detected")
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("Project, link and script checks run automatically.")
          }
        }
      }

      //
      // Footer: last run time and close button
      //
      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["placeholder_text"]
          visible: Cpp_Misc_ProblemCenter.lastRunTime !== ""
          text: qsTr("Last checked at %1").arg(Cpp_Misc_ProblemCenter.lastRunTime)
        }

        Item {
          Layout.fillWidth: true
        }

        Widgets.IconButton {
          iconSize: 16
          text: qsTr("Close")
          horizontalPadding: 8
          onClicked: root.close()
          icon.source: "qrc:/icons/buttons/close.svg"
        }
      }
    }
  }
}
