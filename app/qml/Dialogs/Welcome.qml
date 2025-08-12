/*
 * Serial Studio - https://serial-studio.com/
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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Window {
  id: root

  //
  // Window options
  //
  title: Cpp_AppName
  width: minimumWidth
  height: minimumHeight
  minimumWidth: layout.implicitWidth
  maximumWidth: layout.implicitWidth
  minimumHeight: layout.implicitHeight
  maximumHeight: layout.implicitHeight
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Quit app
  //
  onClosing: {
    if (Cpp_CommercialBuild)
      if (!mainWindow.visible)
        Qt.quit()
  }

  //
  // Hide this aberration when user activates
  //
  Connections {
    target: Cpp_Licensing_LemonSqueezy
    function onActivatedChanged() {
      if (Cpp_Licensing_LemonSqueezy.isActivated && root.visible) {
        app.showMainWindow()
        root.close()
      }
    }
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
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
      spacing: 0
      id: layout
      anchors.centerIn: parent

      //
      // Window controls
      //
      RowLayout {
        spacing: 0
        Layout.fillWidth: true
        Layout.fillHeight: true

        //
        // NSIS-like banner
        //
        Image {
          id: banner
          Layout.leftMargin: -1
          sourceSize.width: 164
          source: "qrc:/rcc/images/dialog-banner.svg"
        } Rectangle {
          implicitWidth: 1
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        //
        // Spacer
        //
        Item {
          implicitWidth: 24
        }

        //
        // Text messages shown to the user
        //
        SwipeView {
          clip: true
          interactive: false
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.minimumHeight: banner.implicitHeight
          Layout.maximumHeight: banner.implicitHeight
          Layout.minimumWidth: Math.max(248, titleA.implicitWidth, titleB.implicitWidth, titleC.implicitWidth) + 48
          Layout.maximumWidth: Math.max(248, titleA.implicitWidth, titleB.implicitWidth, titleC.implicitWidth) + 48
          currentIndex : Cpp_Licensing_Trial.firstRun ? 0 : (Cpp_Licensing_Trial.trialEnabled ? 1 : (Cpp_Licensing_Trial.trialExpired ? 2 : -1))

          //
          // Welcome message
          //
          ColumnLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            opacity: Cpp_Licensing_Trial.firstRun ? 1 : 0
            Behavior on opacity {NumberAnimation{}}

            Item {
              implicitHeight: 24
            }

            Label {
              id: titleA
              Layout.fillWidth: true
              Layout.maximumWidth: parent.width
              text: qsTr("Welcome to %1!").arg(Cpp_AppName)
              font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
            }

            Item {
              implicitHeight: 16
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              text: qsTr("Serial Studio is a powerful real-time visualization tool, " +
                         "built for engineers, students, and makers.")
            }

            Item {
              implicitHeight: 12
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              text: qsTr("You can start a fully-functional 14-day trial, activate it " +
                         "with your license key, or download and compile the GPLv3 " +
                         "source code yourself.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Buying Pro supports the author directly and helps fund future development.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Building the GPLv3 version yourself helps grow the community and encourages technical contributions.")
            }

            Item {
              Layout.fillHeight: true
              implicitHeight: 12
            }

            RowLayout {
              spacing: 4
              opacity: Cpp_Licensing_Trial.busy ? 1 : 0

              Behavior on opacity {NumberAnimation{}}

              BusyIndicator {
                Layout.alignment: Qt.AlignVCenter
                running: Cpp_Licensing_Trial.busy
              }

              Label {
                Layout.fillWidth: true
                text: qsTr("Please wait...")
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignVCenter
                font: Cpp_Misc_CommonFonts.boldUiFont
              }
            }

            Item {
              Layout.fillHeight: true
              implicitHeight: 12
            }
          }

          //
          // Trial enabled message
          //
          ColumnLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            opacity: Cpp_Licensing_Trial.trialEnabled ? 1 : 0
            Behavior on opacity {NumberAnimation{}}


            Item {
              implicitHeight: 24
            }

            Label {
              id: titleB
              Layout.fillWidth: true
              Layout.maximumWidth: parent.width
              font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
              text: qsTr("%1 days remaining in your trial.").arg(Cpp_Licensing_Trial.daysRemaining)
            }

            Item {
              implicitHeight: 16
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              text: qsTr("You’re currently using the fully-featured trial " +
                         "of %1 Pro. It’s valid for 14 days of personal, " +
                         "non-commercial use.").arg(Cpp_AppName)
            }


            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Upgrade to a paid plan to keep using Serial Studio Pro.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Or, compile the GPLv3 source code to use it for free.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("To see available subscription plans, click \"Upgrade Now\" below.")
            }

            Item {
              implicitHeight: 24
            }

            RowLayout {
              Layout.fillWidth: true

              Switch {
                id: dontNagMe
                Layout.leftMargin: -6
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Label.WordWrap
                visible: Cpp_Licensing_Trial.daysRemaining > 1
                text: qsTr("Don't nag me about the trial.\nI understand that when it ends, I'll need to buy a license or build the GPLv3 version.")
              }
            }

            Item {
              Layout.fillHeight: true
            }
          }

          //
          // Trial expired message
          //
          ColumnLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            opacity: Cpp_Licensing_Trial.trialExpired ? 1 : 0
            Behavior on opacity {NumberAnimation{}}

            Item {
              implicitHeight: 24
            }

            Label {
              id: titleC
              Layout.fillWidth: true
              Layout.maximumWidth: parent.width
              font: Cpp_Misc_CommonFonts.customUiFont(1.2, true)
              text: qsTr("Your %1 trial has expired.").arg(Cpp_AppName)
            }

            Item {
              implicitHeight: 16
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              text: qsTr("Your trial period has ended. To continue using %1 with all " +
                         "Pro features, please upgrade to a paid plan.").arg(Cpp_AppName)
            }

            Item {
              implicitHeight: 12
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              text: qsTr("If you prefer, you can also compile the open-source " +
                         "version under the GPLv3 license.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Buying Pro supports the author directly and helps fund future development.")
            }

            Item {
              implicitHeight: 12
            }

            Widgets.InfoBullet {
              text: qsTr("Building the GPLv3 version yourself helps grow the community and encourages technical contributions.")
            }

            Item {
              implicitHeight: 16
            }

            Label {
              wrapMode: Label.WordWrap
              Layout.maximumWidth: parent.width
              font: Cpp_Misc_CommonFonts.boldUiFont
              text: qsTr("Thank you for trying %1!").arg(Cpp_AppName)
            }

            Item {
              Layout.fillHeight: true
            }
          }
        }

        //
        // Another spacer
        //
        Item {
          implicitWidth: 24
        }
      }

      //
      // Buttons
      //
      Rectangle {
        border.width: 1
        Layout.leftMargin: -1
        Layout.rightMargin: -1
        Layout.fillWidth: true
        Layout.bottomMargin: -1
        implicitHeight: buttonsLayout.implicitHeight + 16
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        RowLayout {
          id: buttonsLayout

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          Button {
            icon.width: 18
            icon.height: 18
            rightPadding: 8
            text: qsTr("Upgrade Now")
            Layout.alignment: Qt.AlignVCenter
            onClicked: Cpp_Licensing_LemonSqueezy.buy()
            highlighted: Cpp_Licensing_Trial.trialExpired
            icon.source: "qrc:/rcc/icons/buttons/buy.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
          }

          Button {
            icon.width: 18
            icon.height: 18
            rightPadding: 8
            text: qsTr("Activate")
            Layout.alignment: Qt.AlignVCenter
            onClicked: app.showLicenseDialog()
            icon.source: "qrc:/rcc/icons/buttons/activate.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
          }

          Item {
            Layout.fillWidth: true
          }

          Button {
            icon.width: 18
            icon.height: 18
            rightPadding: 8
            Layout.alignment: Qt.AlignVCenter
            highlighted: !Cpp_Licensing_Trial.trialExpired
            icon.source: "qrc:/rcc/icons/buttons/apply.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
            text: Cpp_Licensing_Trial.trialExpired ? qsTr("Open in Limited Mode") : (Cpp_Licensing_Trial.trialEnabled ? qsTr("Continue") : qsTr("Start Trial"))
            onClicked: {
              if (Cpp_Licensing_Trial.trialExpired) {
                app.showMainWindow()
                root.close()
              }

              else if (!Cpp_Licensing_Trial.trialEnabled)
                Cpp_Licensing_Trial.enableTrial()

              else {
                app.dontNag = dontNagMe.checked

                app.showMainWindow()
                root.close()
              }
            }
          }
        }
      }
    }
  }
}
