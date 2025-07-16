/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
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
  title: qsTr("Licensing")
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + root.titlebarHeight + 32
  maximumHeight: column.implicitHeight + root.titlebarHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window registration
  //
  property real titlebarHeight: 0
  onVisibleChanged: {
    if (visible) {
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["base"])
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
    }

    else {
      root.titlebarHeight = 0
      Cpp_NativeWindow.removeWindow(root)
    }
  }

  //
  // Background + window title on macOS
  //
  Rectangle {
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["window"]

    //
    // Drag the window anywhere
    //
    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }

    //
    // Titlebar text
    //
    Label {
      text: root.title
      visible: root.titlebarHeight > 0
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

      anchors {
        topMargin: 6
        top: parent.top
        horizontalCenter: parent.horizontalCenter
      }
    }
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

    ColumnLayout {
      id: column
      spacing: 0
      anchors.centerIn: parent
      implicitWidth: controls.implicitWidth + 32

      //
      // License activation section
      //
      Rectangle {
        id: controls

        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.topMargin: -root.titlebarHeight / 2
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        implicitWidth: Cpp_Licensing_LemonSqueezy.isActivated ?
                         Math.max(560, licenseControls.implicitWidth + 32) :
                         Math.min(560, activationControls.implicitWidth + 32)

        implicitHeight: Cpp_Licensing_LemonSqueezy.isActivated ?
                          licenseControls.implicitHeight + 32 :
                          activationControls.implicitHeight + 32

        DragHandler {
          target: null
        }

        RowLayout {
          spacing: 12
          anchors.margins: 16
          anchors.fill: parent

          //
          // Activation key image
          //
          Image {
            sourceSize.width: 128
            Layout.minimumWidth: 128
            Layout.maximumWidth: 128
            Layout.minimumHeight: 128
            Layout.maximumHeight: 128
            Layout.alignment: Qt.AlignVCenter
            source: "qrc:/rcc/icons/licensing/license.svg"
            visible: !Cpp_Licensing_LemonSqueezy.isActivated
          }

          //
          // Spacer
          //
          Item {
            implicitWidth: 1
            visible: !Cpp_Licensing_LemonSqueezy.isActivated
          }

          //
          // Controls
          //
          Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter

            //
            // Busy indicator
            //
            ColumnLayout {
              anchors.fill: parent
              opacity: enabled ? 1 : 0
              enabled: Cpp_Licensing_LemonSqueezy.busy
              Behavior on opacity {NumberAnimation{}}

              Item {
                Layout.fillHeight: true
              }

              BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: Cpp_Licensing_LemonSqueezy.busy
              }

              Label {
                Layout.fillWidth: true
                text: qsTr("Please wait...")
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Qt.AlignHCenter
                font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
              }

              Item {
                Layout.fillHeight: true
              }
            }

            //
            // Activation text & controls
            //
            ColumnLayout {
              id: activationControls

              spacing: 0
              anchors.fill: parent
              opacity: enabled ? 1 : 0
              enabled: !Cpp_Licensing_LemonSqueezy.isActivated && !Cpp_Licensing_LemonSqueezy.busy

              Behavior on opacity {NumberAnimation{}}

              Label {
                Layout.fillWidth: true
                font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
                text: qsTr("Activate Serial Studio Pro")
              }

              Item {
                implicitHeight: 6
              }

              Label {
                Layout.fillWidth: true
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.")
              }

              Item {
                implicitHeight: 4
              }

              Label {
                opacity: 0.8
                Layout.fillWidth: true
                wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Your license includes 5 device activations.\nPlans include Monthly, Yearly, and Lifetime options.")
              }

              Item {
                implicitHeight: 12
                Layout.fillHeight: true
              }

              RowLayout {
                spacing: 4
                Layout.fillWidth: true

                TextField {
                  id: _key
                  Layout.fillWidth: true
                  echoMode: TextInput.Password
                  text: Cpp_Licensing_LemonSqueezy.license
                  placeholderText: qsTr("Paste your license key here…")
                  onTextChanged: {
                    if (Cpp_Licensing_LemonSqueezy.license !== text)
                      Cpp_Licensing_LemonSqueezy.license = text
                  }

                  MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.RightButton
                    onPressed: (mouse) => {
                                 if (mouse.button === Qt.RightButton) {
                                   _keyContextMenu.popup()
                                 }
                               }

                    Menu {
                      id: _keyContextMenu

                      MenuItem {
                        text: qsTr("Copy")
                        onTriggered: _key.copy()
                        enabled: _key.selectedText.length > 0
                      }

                      MenuItem {
                        text: qsTr("Paste")
                        enabled: _key.canPaste
                        onTriggered: _key.paste()
                      }

                      MenuItem {
                        text: qsTr("Select All")
                        enabled: _key.length > 0
                        onTriggered: _key.selectAll()
                      }
                    }
                  }
                }

                Button {
                  checkable: true
                  icon.color: palette.text
                  Layout.maximumWidth: height
                  Layout.alignment: Qt.AlignVCenter
                  icon.source: checked ? "qrc:/rcc/icons/buttons/invisible.svg" :
                                         "qrc:/rcc/icons/buttons/visible.svg"
                  onCheckedChanged: _key.echoMode = (checked ? TextField.Normal :
                                                               TextField.Password)
                }
              }
            }

            //
            // License information
            //
            ColumnLayout {
              id: licenseControls

              spacing: 0
              anchors.fill: parent
              opacity: enabled ? 1 : 0
              enabled: Cpp_Licensing_LemonSqueezy.isActivated && !Cpp_Licensing_LemonSqueezy.busy

              Behavior on opacity {NumberAnimation{}}

              GridLayout {
                columns: 3
                rowSpacing: 6
                columnSpacing: 8
                Layout.fillWidth: true

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/plan.svg"
                } Label {
                  text: qsTr("Product") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                } Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: qsTr("Serial Studio %1").arg(Cpp_Licensing_LemonSqueezy.variantName)
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/user.svg"
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                } Label {
                  text: qsTr("Licensee") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                } Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: Cpp_Licensing_LemonSqueezy.customerName
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/email.svg"
                } Label {
                  text: qsTr("Licensee E-Mail") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                } Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: Cpp_Licensing_LemonSqueezy.customerEmail
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/devices.svg"
                } Label {
                  text: qsTr("Device Usage") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                } Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: Cpp_Licensing_LemonSqueezy.seatLimit < 0
                        ? qsTr("%1 devices in use (Unlimited plan)").arg(Cpp_Licensing_LemonSqueezy.seatUsage)
                        : qsTr("%1 of %2 devices used")
                  .arg(Cpp_Licensing_LemonSqueezy.seatUsage)
                  .arg(Cpp_Licensing_LemonSqueezy.seatLimit)
                }

                  Image {
                    sourceSize.width: 18
                    sourceSize.height: 18
                    source: "qrc:/rcc/icons/licensing/uuid.svg"
                  } Label {
                    text: qsTr("Device ID") + ":"
                    font: Cpp_Misc_CommonFonts.boldUiFont
                  } TextField {
                    id: _uid
                    readOnly: true
                    Layout.fillWidth: true
                    onTextChanged: cursorPosition = 0
                    text: Cpp_Licensing_LemonSqueezy.instanceName

                    MouseArea {
                      anchors.fill: parent
                      acceptedButtons: Qt.RightButton
                      onPressed: (mouse) => {
                                   if (mouse.button === Qt.RightButton) {
                                     _uidContextMenu.popup()
                                   }
                                 }

                      Menu {
                        id: _uidContextMenu

                        MenuItem {
                          text: qsTr("Copy")
                          onTriggered: _uid.copy()
                          enabled: _uid.selectedText.length > 0
                        }

                        MenuItem {
                          text: qsTr("Select All")
                          enabled: _uid.length > 0
                          onTriggered: _uid.selectAll()
                        }
                      }
                    }
                  }

                  Image {
                    sourceSize.width: 18
                    sourceSize.height: 18
                    source: "qrc:/rcc/icons/licensing/key.svg"
                    visible: Cpp_Licensing_LemonSqueezy.variantName.indexOf("Pro") !== -1
                  } Label {
                    text: qsTr("License Key") + ":"
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    visible: Cpp_Licensing_LemonSqueezy.variantName.indexOf("Pro") !== -1
                  } TextField {
                    id: _lic
                    readOnly: true
                    Layout.fillWidth: true
                    onTextChanged: cursorPosition = 0
                    text: Cpp_Licensing_LemonSqueezy.license
                    visible: Cpp_Licensing_LemonSqueezy.variantName.indexOf("Pro") !== -1

                    MouseArea {
                      anchors.fill: parent
                      acceptedButtons: Qt.RightButton
                      onPressed: (mouse) => {
                                   if (mouse.button === Qt.RightButton) {
                                     _licContextMenu.popup()
                                   }
                                 }

                      Menu {
                        id: _licContextMenu

                        MenuItem {
                          text: qsTr("Copy")
                          onTriggered: _lic.copy()
                          enabled: _lic.selectedText.length > 0
                        }

                        MenuItem {
                          text: qsTr("Select All")
                          enabled: _lic.length > 0
                          onTriggered: _lic.selectAll()
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }

        //
        // Spacer
        //
        Item {
          implicitHeight: 12
        }

        //
        // Buttons
        //
        RowLayout {
          Layout.fillWidth: true

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 8
            text: qsTr("Customer Portal")
            Layout.alignment: Qt.AlignVCenter
            icon.color: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/buttons/lemonsqueezy.svg"
            onClicked: Cpp_Licensing_LemonSqueezy.openCustomerPortal()
          }

          Item {
            Layout.fillWidth: true
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 8
            text: qsTr("Buy License")
            onClicked: Cpp_Licensing_LemonSqueezy.buy()
            icon.source: "qrc:/rcc/icons/buttons/buy.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 8
            text: qsTr("Activate")
            opacity: enabled ? 1 : 0.5
            visible: !Cpp_Licensing_LemonSqueezy.isActivated
            onClicked: Cpp_Licensing_LemonSqueezy.activate()
            icon.source: "qrc:/rcc/icons/buttons/activate.svg"
            icon.color: Cpp_ThemeManager.colors["button_text"]
            enabled: Cpp_Licensing_LemonSqueezy.canActivate && !Cpp_Licensing_LemonSqueezy.busy
          }

          Button {
            icon.width: 18
            icon.height: 18
            horizontalPadding: 8
            text: qsTr("Deactivate")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_Licensing_LemonSqueezy.busy
            visible: Cpp_Licensing_LemonSqueezy.isActivated
            onClicked: Cpp_Licensing_LemonSqueezy.deactivate()
            icon.color: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/buttons/deactivate.svg"
          }
        }
      }
    }
  }
