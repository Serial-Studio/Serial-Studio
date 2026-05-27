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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property DataGridWidget model

  //
  // Window flags
  //
  readonly property bool rtl: Cpp_Misc_Translator.rtl
  readonly property bool hasToolbar: root.height >= 220

  //
  // Pause/resume binding
  //
  property bool running: true
  onRunningChanged: {
    if (model)
      model.paused = !root.running
  }

  //
  // Row sizing
  //
  readonly property real rowHeight: 30
  readonly property real headerHeight: 32

  //
  // Toolbar: Pause/Resume affordance (only when there's room)
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0
    LayoutMirroring.enabled: root.rtl
    LayoutMirroring.childrenInherit: true

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DashboardToolButton {
      checked: !root.running
      icon.source: root.running ? "qrc:/icons/dashboard-buttons/pause.svg"
                                : "qrc:/icons/dashboard-buttons/resume.svg"
      onClicked: root.running = !root.running
      text: root.running ? qsTr("Pause") : qsTr("Resume")
      ToolTip.text: root.running ? qsTr("Pause") : qsTr("Resume")
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Table container
  //
  Item {
    id: table

    clip: true
    anchors {
      margins: 1
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    //
    // Single source of truth for the column boundary: header AND rows bind to this so
    // the value column never drifts away from the "Value" header on resize.
    //
    readonly property real separatorWidth: 1
    readonly property real columnWidth: Math.max(0, (table.width - separatorWidth) / 2)

    //
    // Header row: gradient + bottom border, matches ProjectEditor TableDelegate
    //
    Rectangle {
      id: header

      height: root.headerHeight
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      gradient: Gradient {
        GradientStop {
          position: 0
          color: Cpp_ThemeManager.colors["table_bg_header_top"]
        }

        GradientStop {
          position: 1
          color: Cpp_ThemeManager.colors["table_bg_header_bottom"]
        }
      }

      Rectangle {
        height: 1
        color: Cpp_ThemeManager.colors["table_border_header"]
        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
        }
      }

      RowLayout {
        spacing: 0
        anchors.fill: parent
        LayoutMirroring.enabled: root.rtl
        LayoutMirroring.childrenInherit: true

        Label {
          text: root.model ? root.model.titleHeader : ""
          leftPadding: 8
          rightPadding: 8
          elide: Label.ElideRight
          Layout.fillWidth: false
          Layout.fillHeight: true
          Layout.preferredWidth: table.columnWidth
          verticalAlignment: Label.AlignVCenter
          horizontalAlignment: Label.AlignLeft
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["table_fg_header"]
        }

        Rectangle {
          Layout.fillWidth: false
          Layout.fillHeight: true
          Layout.preferredWidth: table.separatorWidth
          color: Cpp_ThemeManager.colors["table_separator"]
        }

        Label {
          text: root.model ? root.model.valueHeader : ""
          leftPadding: 8
          rightPadding: 8
          elide: Label.ElideRight
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.preferredWidth: table.columnWidth
          verticalAlignment: Label.AlignVCenter
          horizontalAlignment: Label.AlignLeft
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["table_fg_header"]
        }
      }
    }

    //
    // Scrollable rows
    //
    ScrollView {
      id: scroller

      clip: true
      anchors {
        left: parent.left
        top: header.bottom
        right: parent.right
        bottom: parent.bottom
      }

      ScrollBar.vertical.policy: (rowsView.contentHeight > scroller.height)
                                 ? ScrollBar.AlwaysOn
                                 : ScrollBar.AsNeeded

      ListView {
        id: rowsView

        clip: true
        cacheBuffer: 0
        reuseItems: true
        interactive: true
        boundsBehavior: Flickable.StopAtBounds

        model: root.model ? root.model.rowsModel : null

        delegate: Rectangle {
          id: rowItem

          required property int index
          required property string title
          required property string value

          width: ListView.view.width
          height: root.rowHeight
          color: Cpp_ThemeManager.colors["table_cell_bg"]

          Rectangle {
            height: 1
            color: Cpp_ThemeManager.colors["table_separator"]
            anchors {
              left: parent.left
              right: parent.right
              bottom: parent.bottom
            }
          }

          RowLayout {
            spacing: 0
            anchors.fill: parent
            LayoutMirroring.enabled: root.rtl
            LayoutMirroring.childrenInherit: true

            //
            // Title column
            //
            Label {
              id: titleLabel

              text: rowItem.title
              leftPadding: 8
              rightPadding: 8
              elide: Label.ElideRight
              wrapMode: Text.NoWrap
              maximumLineCount: 1
              Layout.fillWidth: false
              Layout.fillHeight: true
              Layout.preferredWidth: table.columnWidth
              verticalAlignment: Label.AlignVCenter
              horizontalAlignment: Label.AlignLeft
              font: Cpp_Misc_CommonFonts.widgetFont()
              color: Cpp_ThemeManager.colors["table_text"]

              HoverHandler {
                id: titleHover

                enabled: titleLabel.truncated
              }
              ToolTip.delay: 600
              ToolTip.text: rowItem.title
              ToolTip.visible: titleHover.hovered && titleLabel.truncated
            }

            //
            // Vertical separator at the same x as the header separator
            //
            Rectangle {
              Layout.fillWidth: false
              Layout.fillHeight: true
              Layout.preferredWidth: table.separatorWidth
              color: Cpp_ThemeManager.colors["table_separator"]
            }

            //
            // Value column
            //
            Label {
              id: valueLabel

              text: rowItem.value.length > 0 ? rowItem.value : qsTr("Awaiting data…")
              leftPadding: 8
              rightPadding: 8
              elide: Label.ElideRight
              wrapMode: Text.NoWrap
              maximumLineCount: 1
              Layout.fillWidth: true
              Layout.fillHeight: true
              Layout.preferredWidth: table.columnWidth
              verticalAlignment: Label.AlignVCenter
              horizontalAlignment: Label.AlignLeft
              font: Cpp_Misc_CommonFonts.widgetFont()
              color: rowItem.value.length > 0
                     ? Cpp_ThemeManager.colors["table_text"]
                     : Cpp_ThemeManager.colors["placeholder_text"]
              opacity: rowItem.value.length > 0 ? 1.0 : 0.7

              HoverHandler {
                id: valueHover

                enabled: valueLabel.truncated && rowItem.value.length > 0
              }
              ToolTip.delay: 600
              ToolTip.text: rowItem.value
              ToolTip.visible: valueHover.hovered && valueLabel.truncated
                               && rowItem.value.length > 0
            }
          }
        }
      }
    }
  }
}
