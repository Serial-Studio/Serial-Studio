/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("File Transmission")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32
  maximumHeight: column.implicitHeight + 32

  //
  // Stop transmission & clear file when dialog is closed
  //
  onClosing: Cpp_IO_FileTransmission.closeFile()

  //
  // Make window stay on top
  //
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Close window when device is disconnected
  //
  Connections {
    target: Cpp_IO_Manager
    function onConnectedChanged() {
      if (!Cpp_IO_Manager.connected)
        root.close()
    }
  }

  //
  // Save settings
  //
  Settings {
    category: "FileTransmission"
    property alias interval: _interval.value
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
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

    //
    // Window controls
    //
    ColumnLayout {
      id: column
      anchors.centerIn: parent

      GroupBox {
        Layout.fillWidth: true
        Layout.fillHeight: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          spacing: 2
          anchors.fill: parent

          //
          // File selection
          //
          Label {
            text: qsTr("File Selection:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_FileTransmission.active
          } RowLayout {
            spacing: 4
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_FileTransmission.active

            TextField {
              enabled: false
              Layout.fillWidth: true
              Layout.minimumWidth: 256
              Layout.alignment: Qt.AlignVCenter
              text: Cpp_IO_FileTransmission.fileName
            }

            Button {
              text: qsTr("Select File...")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_IO_FileTransmission.openFile()
            }
          }

          //
          // Spacer
          //
          Item {
            implicitHeight: 4
          }

          //
          // Interval selection
          //
          Label {
            text: qsTr("Transmission Interval:")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_FileTransmission.active
          } RowLayout {
            spacing: 4
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_FileTransmission.active

            SpinBox {
              id: _interval
              from: 0
              to: 10000
              editable: true
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              value: Cpp_IO_FileTransmission.lineTransmissionInterval
              onValueChanged: {
                if (value !== Cpp_IO_FileTransmission.lineTransmissionInterval)
                  Cpp_IO_FileTransmission.lineTransmissionInterval = value
              }
            }

            Label {
              text: qsTr("msecs")
              Layout.alignment: Qt.AlignVCenter
              color: Cpp_ThemeManager.colors["text"]
            }
          }

          //
          // Spacer
          //
          Item {
            implicitHeight: 4
          }

          //
          // Progress + Start/Stop button
          //
          RowLayout {
            spacing: 4
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: Cpp_IO_FileTransmission.fileOpen
            Behavior on opacity {NumberAnimation{}}

            ColumnLayout {
              spacing: 4
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter

              Label {
                text: qsTr("Progress: %1").arg(Cpp_IO_FileTransmission.transmissionProgress) + "%"
              }

              ProgressBar {
                from: 0
                to: 100
                Layout.fillWidth: true
                value: Cpp_IO_FileTransmission.transmissionProgress
              }
            }

            Button {
              Layout.alignment: Qt.AlignBottom
              checked: Cpp_IO_FileTransmission.active
              text: (Cpp_IO_FileTransmission.transmissionProgress > 0 &&
                     Cpp_IO_FileTransmission.transmissionProgress < 100) ?
                      (Cpp_IO_FileTransmission.active ? qsTr("Pause Transmission") :
                                                        qsTr("Resume Transmission")) :
                      (Cpp_IO_FileTransmission.active ? qsTr("Stop Transmission") :
                                                        qsTr("Begin Transmission"))
              onClicked: {
                if (Cpp_IO_FileTransmission.active)
                  Cpp_IO_FileTransmission.stopTransmission()
                else
                  Cpp_IO_FileTransmission.beginTransmission()
              }
            }
          }
        }
      }
    }
  }
}
