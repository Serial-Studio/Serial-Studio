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

  title: qsTr("File Transmission")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: column.implicitWidth
  preferredHeight: column.implicitHeight

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
    property alias timeout: _timeout.value
    property alias interval: _interval.value
    property alias maxRetries: _retries.value
    property alias blockSize: _blockSize.value
    property alias transferMode: _modeCombo.currentIndex
  }

  //
  // Dialog layout
  //
  dialogContent: ColumnLayout {
    id: column

    spacing: 4
    anchors.centerIn: parent

    //
    // File & protocol selection group
    //
    GroupBox {
      Layout.fillWidth: true
      Layout.minimumWidth: 420

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
        // Transfer mode selector
        //
        Label {
          opacity: enabled ? 1 : 0.5
          text: qsTr("Transfer Protocol:")
          enabled: !Cpp_IO_FileTransmission.active
        } RowLayout {
          spacing: 4
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          enabled: !Cpp_IO_FileTransmission.active

          ComboBox {
            id: _modeCombo

            Layout.fillWidth: true
            model: Cpp_IO_FileTransmission.transferModes
            currentIndex: Cpp_IO_FileTransmission.transferMode
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_IO_FileTransmission.transferMode)
                Cpp_IO_FileTransmission.transferMode = currentIndex
            }
          }
        }

        //
        // Spacer
        //
        Item {
          implicitHeight: 4
        }

        //
        // File selection
        //
        Label {
          opacity: enabled ? 1 : 0.5
          text: qsTr("File Selection:")
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
            text: qsTr("Select File…")
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
        // Interval (plain text / raw binary modes)
        //
        Label {
          opacity: enabled ? 1 : 0.5
          text: qsTr("Transmission Interval:")
          visible: _modeCombo.currentIndex <= 1
          enabled: !Cpp_IO_FileTransmission.active
        } RowLayout {
          spacing: 4
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          visible: _modeCombo.currentIndex <= 1
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
        // Block size (raw binary / ZMODEM)
        //
        Label {
          text: qsTr("Block Size:")
          opacity: enabled ? 1 : 0.5
          enabled: !Cpp_IO_FileTransmission.active
          visible: _modeCombo.currentIndex === 1 || _modeCombo.currentIndex === 5
        } RowLayout {
          spacing: 4
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          enabled: !Cpp_IO_FileTransmission.active
          visible: _modeCombo.currentIndex === 1 || _modeCombo.currentIndex === 5

          SpinBox {
            id: _blockSize

            from: 64
            to: 8192
            stepSize: 64
            editable: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            value: Cpp_IO_FileTransmission.blockSize
            onValueChanged: {
              if (value !== Cpp_IO_FileTransmission.blockSize)
                Cpp_IO_FileTransmission.blockSize = value
            }
          }

          Label {
            text: qsTr("bytes")
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
          }
        }

        //
        // Timeout (protocol modes only)
        //
        Label {
          text: qsTr("Timeout:")
          opacity: enabled ? 1 : 0.5
          visible: _modeCombo.currentIndex >= 2
          enabled: !Cpp_IO_FileTransmission.active
        } RowLayout {
          spacing: 4
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          visible: _modeCombo.currentIndex >= 2
          enabled: !Cpp_IO_FileTransmission.active

          SpinBox {
            id: _timeout

            to: 60000
            from: 1000
            stepSize: 1000
            editable: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            value: Cpp_IO_FileTransmission.protocolTimeout
            onValueChanged: {
              if (value !== Cpp_IO_FileTransmission.protocolTimeout)
                Cpp_IO_FileTransmission.protocolTimeout = value
            }
          }

          Label {
            text: qsTr("msecs")
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
          }
        }

        //
        // Max retries (protocol modes only)
        //
        Label {
          text: qsTr("Max Retries:")
          opacity: enabled ? 1 : 0.5
          visible: _modeCombo.currentIndex >= 2
          enabled: !Cpp_IO_FileTransmission.active
        } RowLayout {
          spacing: 4
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          visible: _modeCombo.currentIndex >= 2
          enabled: !Cpp_IO_FileTransmission.active

          SpinBox {
            id: _retries

            from: 1
            to: 100
            editable: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            value: Cpp_IO_FileTransmission.maxRetries
            onValueChanged: {
              if (value !== Cpp_IO_FileTransmission.maxRetries)
                Cpp_IO_FileTransmission.maxRetries = value
            }
          }
        }
      }
    }

    //
    // Progress & status group
    //
    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
      }

      ColumnLayout {
        spacing: 4
        anchors.fill: parent

        //
        // Status row: progress + speed + errors
        //
        RowLayout {
          spacing: 8
          Layout.fillWidth: true

          ColumnLayout {
            spacing: 2
            Layout.fillWidth: true

            Label {
              text: qsTr("Progress: %1%").arg(Cpp_IO_FileTransmission.transmissionProgress)
            }

            ProgressBar {
              from: 0
              to: 100
              Layout.fillWidth: true
              value: Cpp_IO_FileTransmission.transmissionProgress
            }
          }
        }

        //
        // Transfer details row
        //
        RowLayout {
          spacing: 16
          Layout.fillWidth: true

          Label {
            visible: text.length > 0
            text: Cpp_IO_FileTransmission.transferSpeed
            color: Cpp_ThemeManager.colors["text"]
            opacity: 0.8
          }

          Label {
            text: {
              var sent = Cpp_IO_FileTransmission.bytesSent
              var total = Cpp_IO_FileTransmission.bytesTotal
              if (total <= 0)
                return ""

              return qsTr("%1 / %2 bytes").arg(sent).arg(total)
            }
            color: Cpp_ThemeManager.colors["text"]
            opacity: 0.8
          }

          Label {
            visible: Cpp_IO_FileTransmission.errorCount > 0
            text: qsTr("Errors: %1").arg(Cpp_IO_FileTransmission.errorCount)
            color: Cpp_ThemeManager.colors["alarm"]
          }

          Item {
            Layout.fillWidth: true
          }
        }

        //
        // Status text
        //
        Label {
          Layout.fillWidth: true
          text: Cpp_IO_FileTransmission.statusText
          opacity: 0.7
          elide: Text.ElideRight
          visible: text.length > 0
          color: Cpp_ThemeManager.colors["text"]
        }

        //
        // Start / stop button
        //
        RowLayout {
          spacing: 4
          Layout.fillWidth: true

          Button {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: Cpp_IO_FileTransmission.fileOpen
            Behavior on opacity {NumberAnimation{}}
            checked: Cpp_IO_FileTransmission.active
            text: {
              var progress = Cpp_IO_FileTransmission.transmissionProgress
              if (progress > 0 && progress < 100)
                return Cpp_IO_FileTransmission.active ?
                      qsTr("Pause Transmission") :
                      qsTr("Resume Transmission")

              return Cpp_IO_FileTransmission.active ?
                    qsTr("Stop Transmission") :
                    qsTr("Begin Transmission")
            }
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

    //
    // Activity log group
    //
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
        spacing: 4
        anchors.fill: parent

        RowLayout {
          spacing: 4
          Layout.fillWidth: true

          Label {
            Layout.fillWidth: true
            text: qsTr("Activity Log")
          }

          Button {
            text: qsTr("Clear")
            onClicked: Cpp_IO_FileTransmission.clearLog()
          }
        }

        ScrollView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.minimumHeight: 120
          Layout.preferredHeight: 150

          TextArea {
            id: _logArea

            readOnly: true
            wrapMode: Text.Wrap
            font: Cpp_Misc_CommonFonts.monoFont
            color: Cpp_ThemeManager.colors["text"]
            text: Cpp_IO_FileTransmission.logEntries.join("\n")

            background: Rectangle {
              color: Cpp_ThemeManager.colors["base"]
              border.width: 1
              border.color: Cpp_ThemeManager.colors["mid"]
              radius: 2
            }

            //
            // Auto-scroll to bottom
            //
            onTextChanged: {
              _logArea.cursorPosition = _logArea.length
            }
          }
        }
      }
    }
  }
}
