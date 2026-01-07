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

import QtCore as Core

import "../Widgets"

SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("File Transmission")

  //
  // Stop transmission & clear file when dialog is closed
  //
  onClosing: Cpp_IO_FileTransmission.closeFile()

  //
  // Close window when device is disconnected
  //
  Connections {
    target: Cpp_IO_Manager
    function onConnectedChanged() {
      if (!Cpp_IO_Manager.isConnected)
        root.close()
    }
  }

  //
  // Save settings
  //
  Core.Settings {
    category: "FileTransmission"
    property alias interval: _interval.value
  }

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    id: column
    spacing: 4
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
