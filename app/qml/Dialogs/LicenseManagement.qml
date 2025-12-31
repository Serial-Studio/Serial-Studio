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

  property int titlebarHeight: 0
  property int calculatedHeight: {
    var contentHeight = contentRect.implicitHeight > 0 ? contentRect.implicitHeight : 280
    var buttonHeight = buttonRow.implicitHeight > 0 ? buttonRow.implicitHeight : 48
    return Math.min(340, Math.max(280, contentHeight + buttonHeight + 60 + titlebarHeight))
  }

  title: qsTr("Licensing")
  minimumWidth: 640
  maximumWidth: 640
  width: minimumWidth
  height: calculatedHeight
  minimumHeight: calculatedHeight
  maximumHeight: calculatedHeight

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

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

  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
  }

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
      id: mainLayout
      spacing: 12
      anchors.fill: parent
      anchors.margins: 16

      Rectangle {
        id: contentRect
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        implicitHeight: stackLayout.implicitHeight + 32
        Layout.preferredHeight: implicitHeight
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        DragHandler {
          target: null
        }

        StackLayout {
          id: stackLayout
          width: parent.width - 32
          anchors.centerIn: parent
          currentIndex: Cpp_Licensing_LemonSqueezy.busy ? 0 :
                        (Cpp_Licensing_LemonSqueezy.isActivated ? 2 : 1)

          Item {
            implicitHeight: busyLayout.implicitHeight
            implicitWidth: busyLayout.implicitWidth

            ColumnLayout {
              id: busyLayout
              spacing: 12
              anchors.centerIn: parent

              BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: Cpp_Licensing_LemonSqueezy.busy
              }

              Label {
                text: qsTr("Please wait...")
                Layout.alignment: Qt.AlignHCenter
                font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
              }
            }
          }

          Item {
            implicitHeight: activationLayout.implicitHeight
            implicitWidth: activationLayout.implicitWidth

            RowLayout {
              id: activationLayout
              spacing: 16
              width: parent.width

              Image {
                sourceSize.width: 128
                sourceSize.height: 128
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/rcc/icons/licensing/license.svg"
              }

              ColumnLayout {
                spacing: 8
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                Label {
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
                  text: qsTr("Activate Serial Studio Pro")
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: qsTr("Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.")
                }

                Label {
                  opacity: 0.8
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: qsTr("Your license includes 5 device activations.\nPlans include Monthly, Yearly, and Lifetime options.")
                }

                Item {
                  Layout.preferredHeight: 8
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
            }
          }

          Item {
            implicitHeight: licenseDetailsLayout.implicitHeight + 16
            implicitWidth: licenseDetailsLayout.implicitWidth

            ScrollView {
              id: licenseDetailsScroll
              anchors.fill: parent
              contentWidth: availableWidth
              ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

              GridLayout {
                id: licenseDetailsLayout
                width: parent.width
                columns: 3
                rowSpacing: 8
                columnSpacing: 12

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/plan.svg"
                }

                Label {
                  text: qsTr("Product") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: qsTr("Serial Studio %1").arg(Cpp_Licensing_LemonSqueezy.variantName)
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/user.svg"
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                }

                Label {
                  text: qsTr("Licensee") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: Cpp_Licensing_LemonSqueezy.customerName
                  visible: Cpp_Licensing_LemonSqueezy.customerName.length > 0
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/email.svg"
                }

                Label {
                  text: qsTr("Licensee E-Mail") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                  text: Cpp_Licensing_LemonSqueezy.customerEmail
                }

                Image {
                  sourceSize.width: 18
                  sourceSize.height: 18
                  source: "qrc:/rcc/icons/licensing/devices.svg"
                }

                Label {
                  text: qsTr("Device Usage") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }

                Label {
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
                }

                Label {
                  text: qsTr("Device ID") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                }

                TextField {
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
                }

                Label {
                  text: qsTr("License Key") + ":"
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  visible: Cpp_Licensing_LemonSqueezy.variantName.indexOf("Pro") !== -1
                }

                TextField {
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

      RowLayout {
        id: buttonRow
        spacing: 8
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          horizontalPadding: 8
          text: qsTr("Customer Portal")
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
