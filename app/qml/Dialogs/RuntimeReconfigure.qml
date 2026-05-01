/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../Widgets" as Widgets
import "../MainWindow/Panes/SetupPanes" as SetupPanes

Widgets.SmartDialog {
  id: root

  staysOnTop: true
  fixedSize: false

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // dialogMode: "failed" = auto-connect never settled; "lost" = drop mid-session.
  //
  property int pageIndex: 0
  property string dialogMode: "failed"

  readonly property bool isLost: dialogMode === "lost"

  title: isLost
         ? qsTr("Connection Lost")
         : qsTr("Device Unavailable")

  onVisibleChanged: {
    if (visible)
      pageIndex = 0
  }

  //
  // Auto-dismiss when the operator establishes a connection.
  //
  Connections {
    target: Cpp_IO_Manager
    function onConnectedChanged() {
      if (Cpp_IO_Manager.isConnected && root.visible)
        root.close()
    }
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    StackLayout {
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: root.pageIndex

      //
      // Page 0 -- failure / loss summary
      //
      RowLayout {
        spacing: 16

        Image {
          Layout.preferredWidth: 64
          Layout.preferredHeight: 64
          sourceSize: Qt.size(64, 64)
          Layout.alignment: Qt.AlignTop
          fillMode: Image.PreserveAspectFit
          source: "qrc:/icons/notifications/warning.svg"
        }

        ColumnLayout {
          spacing: 12
          Layout.fillWidth: true
          Layout.fillHeight: true

          Label {
            wrapMode: Label.Wrap
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.boldUiFont
            text: root.isLost
                  ? qsTr("The connection to your device was lost.")
                  : qsTr("Serial Studio couldn't reach your device.")
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.uiFont
            text: root.isLost
                  ? qsTr("Check the cable, power, and that no other application "
                       + "has taken over the device. You can try reconnecting, "
                       + "switch to a different device, or quit.")
                  : qsTr("Make sure it's plugged in, powered on, and not already in use "
                       + "by another app. You can try again, pick a different device, "
                       + "or quit.")
          }

          Item { Layout.fillHeight: true }

          RowLayout {
            spacing: 12
            Layout.fillWidth: true

            Button {
              icon.width: 18
              icon.height: 18
              text: qsTr("Quit")
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: app.quitApplication()
              icon.color: Cpp_ThemeManager.colors["button_text"]
              icon.source: "qrc:/icons/buttons/close.svg"
            }

            Item { Layout.fillWidth: true }

            Button {
              icon.width: 18
              icon.height: 18
              font: Cpp_Misc_CommonFonts.uiFont
              text: qsTr("Pick Different Device")
              onClicked: root.pageIndex = 1
              icon.color: Cpp_ThemeManager.colors["button_text"]
              icon.source: "qrc:/icons/buttons/wrench.svg"
            }

            Button {
              highlighted: true
              icon.width: 18
              icon.height: 18
              font: Cpp_Misc_CommonFonts.uiFont
              text: root.isLost ? qsTr("Reconnect") : qsTr("Try Again")
              enabled: !Cpp_IO_Manager.isConnected
              icon.color: Cpp_ThemeManager.colors["button_text"]
              icon.source: "qrc:/icons/buttons/refresh.svg"
              onClicked: Cpp_IO_Manager.connectDevice()
            }
          }
        }
      }

      //
      // Page 1 -- driver picker
      //
      ColumnLayout {
        spacing: 8

        Label {
          wrapMode: Label.Wrap
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.uiFont
          text: qsTr("Pick the correct device, then press Connect.")
        }

        ComboBox {
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          enabled: !Cpp_IO_Manager.isConnected
          model: Cpp_IO_Manager.availableBuses
          currentIndex: Cpp_IO_Manager.busType
          displayText: qsTr("I/O Interface: %1").arg(currentText)
          onCurrentIndexChanged: {
            if (Cpp_IO_Manager.busType !== currentIndex)
              Cpp_IO_Manager.busType = currentIndex
          }
        }

        ScrollView {
          clip: true
          Layout.fillWidth: true
          Layout.fillHeight: true
          contentWidth: availableWidth

          SetupPanes.Hardware {
            width: parent.width
          }
        }

        RowLayout {
          spacing: 12
          Layout.fillWidth: true

          Button {
            icon.width: 18
            icon.height: 18
            text: qsTr("Quit")
            font: Cpp_Misc_CommonFonts.uiFont
            onClicked: app.quitApplication()
            icon.color: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/icons/buttons/close.svg"
          }

          Item { Layout.fillWidth: true }

          Button {
            highlighted: true
            icon.width: 18
            icon.height: 18
            text: qsTr("Connect")
            font: Cpp_Misc_CommonFonts.uiFont
            enabled: Cpp_IO_Manager.configurationOk && !Cpp_IO_Manager.isConnected
            icon.color: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/icons/buttons/connected.svg"
            onClicked: Cpp_IO_Manager.connectDevice()
          }
        }
      }
    }
  }
}
