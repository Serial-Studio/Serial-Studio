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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects
import QtWebEngine

import "../Widgets"

SmartDialog {
  id: root

  title: qsTr("Examples Browser")

  //
  // Fixed toolbar height so the window size stays stable across page transitions
  //
  readonly property int toolbarHeight: 40

  //
  // Track page transitions
  //
  property bool showDetail: Cpp_Examples.selectedIndex >= 0

  //
  // Track fetch state
  //
  property bool fetchingData: Cpp_Examples.count === 0 && Cpp_Examples.searchFilter === ""

  //
  // Track whether the WebView HTML shell is ready to receive content
  //
  property bool readmeViewReady: false

  //
  // Push markdown content into the WebView via JS
  //
  function pushReadme() {
    if (!readmeViewReady || !readmeView.visible)
      return

    var md = Cpp_Examples.selectedReadme
    if (!md || md === "") {
      if (Cpp_Examples.loading)
        readmeView.runJavaScript("document.getElementById('content').innerHTML = '<p style=\"opacity:0.5\">Loading...</p>';")
      else
        readmeView.runJavaScript("document.getElementById('content').innerHTML = '<p style=\"opacity:0.5\">No README available.</p>';")
      return
    }

    var escaped = md.replace(/\\/g, '\\\\')
                    .replace(/`/g, '\\`')
                    .replace(/\$/g, '\\$')
    readmeView.runJavaScript("renderMarkdown(`" + escaped + "`);")
  }

  //
  // Push theme colors into the WebView
  //
  function pushReadmeTheme() {
    if (!readmeViewReady)
      return

    var json = Cpp_HelpCenter.themeColors
    readmeView.runJavaScript("setTheme(" + json + ");")
  }

  //
  // Fetch manifest when dialog opens
  //
  onVisibleChanged: {
    if (visible)
      Cpp_Examples.fetchManifest()
  }

  //
  // Reset state on close
  //
  onClosing: {
    Cpp_Examples.selectedIndex = -1
    Cpp_Examples.searchFilter = ""
  }

  dialogContent: ColumnLayout {
    spacing: 8
    anchors.centerIn: parent

    //
    // Search bar (grid page only)
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      Layout.minimumWidth: 860
      Layout.maximumWidth: 860
      implicitHeight: root.toolbarHeight
      visible: !root.showDetail && !root.fetchingData
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: searchField.activeFocus ? Cpp_ThemeManager.colors["highlight"] :
                                              Cpp_ThemeManager.colors["groupbox_border"]

      TextField {
        id: searchField

        background: Item {}
        anchors.fill: parent
        anchors.leftMargin: 4
        font: Cpp_Misc_CommonFonts.uiFont
        rightPadding: searchIcon.width + 16
        placeholderText: qsTr("Search in Examples...")
        onTextChanged: Cpp_Examples.searchFilter = text

        Button {
          id: searchIcon

          opacity: 0.8
          icon.width: 12
          icon.height: 12
          background: Item {}
          anchors.rightMargin: 4
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/search.svg"
        }
      }
    }

    //
    // Flat toolbar (detail page only)
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      Layout.minimumWidth: 860
      Layout.maximumWidth: 860
      implicitHeight: root.toolbarHeight
      visible: root.showDetail
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      RowLayout {
        id: toolbarRow

        spacing: 4
        anchors.fill: parent
        anchors.margins: 4

        ToolButton {
          icon.width: 18
          icon.height: 18
          text: qsTr("Back")
          background: Item {}
          onClicked: Cpp_Examples.selectedIndex = -1
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/backward.svg"

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 4
          Layout.bottomMargin: 4
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Item {
          Layout.fillWidth: true
        }

        Label {
          elide: Text.ElideRight
          Layout.maximumWidth: 380
          text: Cpp_Examples.selectedExample.title || ""
          font: Cpp_Misc_CommonFonts.customUiFont(1.0, true)
        }

        Label {
          color: "#f39c12"
          text: qsTr("Pro")
          font: Cpp_Misc_CommonFonts.boldUiFont
          visible: Cpp_Examples.selectedExample.requiresPro || false
        }

        Item {
          Layout.fillWidth: true
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 4
          Layout.bottomMargin: 4
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          text: qsTr("Download && Open")
          enabled: !Cpp_Examples.loading
          onClicked: Cpp_Examples.downloadExample()
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/open.svg"

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          text: qsTr("View on GitHub")
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/website.svg"
          onClicked: {
            var id = Cpp_Examples.selectedExample.id || ""
            if (id !== "")
              Qt.openUrlExternally(
                    "https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/"
                    + encodeURIComponent(id))
          }

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }
      }
    }

    //
    // Download progress bar
    //
    ProgressBar {
      to: 1
      from: 0
      Layout.fillWidth: true
      value: Cpp_Examples.downloadProgress
      visible: Cpp_Examples.loading && Cpp_Examples.downloadProgress > 0
    }

    //
    // Page container with slide animation
    //
    Item {
      id: pageContainer

      clip: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 860
      Layout.maximumWidth: 860
      Layout.minimumHeight: 520

      //
      // Busy indicator while fetching manifest
      //
      ColumnLayout {
        spacing: 8
        anchors.centerIn: parent
        visible: root.fetchingData

        BusyIndicator {
          running: parent.visible
          Layout.alignment: Qt.AlignHCenter
        }

        Label {
          text: qsTr("Fetching examples...")
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["text"]
          Layout.alignment: Qt.AlignHCenter
        }
      }

      //
      // Page 0: Grid view
      //
      Item {
        id: gridPage
        visible: !root.fetchingData

        width: parent.width
        height: parent.height
        x: root.showDetail ? -width : 0

        Behavior on x {
          NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
          }
        }

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ScrollView {
          anchors.fill: parent
          anchors.margins: 8

          GridView {
            id: gridView

            cellWidth: 210
            cellHeight: 194
            clip: true
            model: Cpp_Examples.examples
            anchors.topMargin: 0
            anchors.leftMargin: 2

            delegate: Rectangle {
              id: card

              width: 200
              height: 186
              radius: 6
              color: cardMouse.containsMouse
                     ? Cpp_ThemeManager.colors["highlight"]
                     : Cpp_ThemeManager.colors["base"]
              border.width: 1
              border.color: cardMouse.containsMouse
                            ? Cpp_ThemeManager.colors["accent"]
                            : Cpp_ThemeManager.colors["mid"]

              Behavior on color {
                ColorAnimation { duration: 150 }
              }

              Behavior on border.color {
                ColorAnimation { duration: 150 }
              }

              ColumnLayout {
                spacing: 4
                anchors.fill: parent
                anchors.margins: 6

                //
                // Screenshot or gradient placeholder
                //
                Item {
                  Layout.fillWidth: true
                  Layout.fillHeight: true

                  Rectangle {
                    id: thumbRect

                    anchors.fill: parent
                    radius: 4
                    clip: true
                    color: "transparent"

                    gradient: Gradient {
                      orientation: Gradient.Horizontal
                      GradientStop {
                        position: 0
                        color: {
                          var p = [
                                "#2c3e50", "#1a5276", "#1e8449", "#7d3c98",
                                "#a04000", "#1b4f72", "#196f3d", "#6c3483",
                                "#922b21", "#117a65", "#7e5109", "#2e4053",
                                "#1f618d", "#239b56", "#884ea0", "#ba4a00",
                                "#2471a3", "#28b463", "#a569bd", "#cb4335",
                                "#148f77", "#b9770e", "#5b2c6f", "#d35400"
                              ]
                          return p[index % p.length]
                        }
                      }
                      GradientStop {
                        position: 1
                        color: {
                          var p = [
                                "#1a252f", "#0e2f43", "#12522c", "#4a2460",
                                "#612600", "#0f2d42", "#0e4124", "#3f1f4e",
                                "#561914", "#0a493d", "#4b3006", "#1b2631",
                                "#133a54", "#155d34", "#512e60", "#702a00",
                                "#154360", "#186a3b", "#633974", "#7b281e",
                                "#0c5647", "#6e4708", "#361842", "#7e3300"
                              ]
                          return p[index % p.length]
                        }
                      }
                    }

                    Image {
                      id: thumbImg

                      smooth: true
                      mipmap: true
                      asynchronous: true
                      anchors.fill: parent
                      fillMode: Image.PreserveAspectCrop
                      visible: status === Image.Ready
                      source: modelData.hasScreenshot
                              ? "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/examples/"
                                + encodeURIComponent(modelData.id)
                                + "/doc/"
                                + (modelData.screenshotFileName ? modelData.screenshotFileName : "screenshot.png")
                              : ""
                    }

                    Label {
                      anchors.centerIn: parent
                      text: modelData.title.charAt(0)
                      visible: !modelData.hasScreenshot
                      opacity: 0.6
                      color: "#ffffff"
                      font: Cpp_Misc_CommonFonts.customMonoFont(2.5, true)
                    }
                  }

                  //
                  // Hover description overlay (slides up from bottom)
                  //
                  Item {
                    id: hoverOverlay

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.height
                    clip: true

                    Rectangle {
                      id: overlayPanel

                      width: parent.width
                      height: parent.height
                      radius: 4
                      opacity: 0.9
                      y: cardMouse.containsMouse ? 0 : parent.height
                      color: Cpp_ThemeManager.colors["groupbox_background"]
                      border.width: 1
                      border.color: Cpp_ThemeManager.colors["groupbox_border"]

                      Behavior on y {
                        NumberAnimation {
                          duration: 250
                          easing.type: Easing.OutCubic
                        }
                      }

                      ColumnLayout {
                        spacing: 4
                        anchors.fill: parent
                        anchors.margins: 8

                        Label {
                          text: modelData.title
                          Layout.fillWidth: true
                          elide: Text.ElideRight
                          color: Cpp_ThemeManager.colors["text"]
                          font: Cpp_Misc_CommonFonts.boldUiFont
                        }

                        Rectangle {
                          implicitHeight: 1
                          Layout.fillWidth: true
                          color: Cpp_ThemeManager.colors["groupbox_border"]
                        }

                        Label {
                          text: modelData.description
                          Layout.fillWidth: true
                          Layout.fillHeight: true
                          wrapMode: Text.WordWrap
                          elide: Text.ElideRight
                          maximumLineCount: 5
                          color: Cpp_ThemeManager.colors["text"]
                          font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                        }
                      }
                    }
                  }
                }

                //
                // Title
                //
                Label {
                  text: modelData.title
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  color: cardMouse.containsMouse
                         ? Cpp_ThemeManager.colors["highlighted_text"]
                         : Cpp_ThemeManager.colors["text"]
                }

                //
                // Category + difficulty
                //
                RowLayout {
                  spacing: 4
                  Layout.fillWidth: true

                  Label {
                    text: modelData.category
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                  }

                  Item { Layout.fillWidth: true }

                  Label {
                    text: modelData.difficulty
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    color: modelData.difficulty === "Advanced"
                           ? "#e74c3c"
                           : modelData.difficulty === "Intermediate"
                             ? "#f39c12"
                             : "#2ecc71"
                  }
                }
              }

              MouseArea {
                id: cardMouse

                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Cpp_Examples.selectedIndex = index
              }
            }
          }
        }
      }

      //
      // Page 1: Detail view
      //
      Item {
        id: detailPage

        width: parent.width
        height: parent.height
        x: root.showDetail ? 0 : width

        Behavior on x {
          NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
          }
        }

        RowLayout {
          spacing: 8
          anchors.fill: parent

          //
          // README panel (WebEngineView)
          //
          Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
              radius: 2
              border.width: 1
              anchors.fill: parent
              color: Cpp_ThemeManager.colors["groupbox_background"]
              border.color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            WebEngineView {
              id: readmeView

              anchors.fill: parent
              anchors.margins: 2
              backgroundColor: "transparent"
              url: "qrc:/rcc/markdown-viewer.html"
              settings.localContentCanAccessRemoteUrls: true

              onLoadingChanged: function(loadRequest) {
                if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
                  root.readmeViewReady = true
                  root.pushReadmeTheme()
                  root.pushReadme()
                }
              }

              onNavigationRequested: function(request) {
                var url = request.url.toString()

                // Allow initial page load from qrc
                if (url.startsWith("qrc:"))
                  return

                // Copy code to clipboard
                if (url.startsWith("copy:")) {
                  request.reject()
                  var text = decodeURIComponent(url.substring(5))
                  Cpp_Misc_Utilities.copyText(text)
                  exCopyToast.show()
                  return
                }

                // Open all links externally
                request.reject()
                if (url.startsWith("ext:"))
                  Qt.openUrlExternally(url.substring(4))
                else if (url.startsWith("nav:"))
                  Qt.openUrlExternally(url.substring(4))
              }
            }

            //
            // React to content changes
            //
            Connections {
              target: Cpp_Examples
              function onSelectedReadmeChanged() {
                root.pushReadme()
              }
            }

            //
            // React to theme changes
            //
            Connections {
              target: Cpp_HelpCenter
              function onThemeColorsChanged() {
                root.pushReadmeTheme()
              }
            }

            //
            // "Copied to Clipboard" toast notification
            //
            Rectangle {
              id: exCopyToast

              opacity: 0
              radius: 4
              anchors.bottom: parent.bottom
              anchors.horizontalCenter: parent.horizontalCenter
              anchors.bottomMargin: 24
              width: exCopyToastLabel.implicitWidth + 24
              height: exCopyToastLabel.implicitHeight + 12
              color: Cpp_ThemeManager.colors["highlight"]

              function show() {
                exCopyToast.opacity = 1
                exCopyToastTimer.restart()
              }

              Label {
                id: exCopyToastLabel

                anchors.centerIn: parent
                text: qsTr("Copied to Clipboard")
                font: Cpp_Misc_CommonFonts.uiFont
                color: Cpp_ThemeManager.colors["highlighted_text"]
              }

              Timer {
                id: exCopyToastTimer

                interval: 1500
                onTriggered: exCopyToast.opacity = 0
              }

              Behavior on opacity {
                NumberAnimation {
                  duration: 150
                }
              }
            }
          }

          //
          // Sidebar
          //
          ColumnLayout {
            spacing: 8
            Layout.minimumWidth: 280
            Layout.maximumWidth: 280
            Layout.fillHeight: true

            //
            // Screenshot
            //
            Item {
              Layout.fillWidth: true
              Layout.minimumHeight: screenshotImage.status === Image.Ready
                                    ? Math.min(screenshotImage.sourceSize.height * (268 / screenshotImage.sourceSize.width), 220)
                                    : 160
              Layout.maximumHeight: Layout.minimumHeight

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              Image {
                id: screenshotImage

                mipmap: true
                smooth: true
                anchors.margins: 1
                asynchronous: true
                anchors.fill: parent
                visible: status === Image.Ready
                fillMode: Image.PreserveAspectFit
                source: Cpp_Examples.selectedScreenshot || ""
              }

              Label {
                anchors.centerIn: parent
                visible: !screenshotImage.visible
                text: qsTr("No screenshot available")
                color: Cpp_ThemeManager.colors["placeholder_text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              }

              MouseArea {
                anchors.fill: parent
                enabled: screenshotImage.visible
                cursorShape: screenshotImage.visible ? Qt.PointingHandCursor : Qt.ArrowCursor
                onClicked: screenshotPopup.show()
              }
            }

            //
            // Details panel
            //
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              ColumnLayout {
                spacing: 4
                anchors.fill: parent
                anchors.margins: 8

                Item {
                  implicitHeight: 2
                }

                Label {
                  text: qsTr("Details")
                  font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
                  color: Cpp_ThemeManager.colors["pane_section_label"]
                  Component.onCompleted: font.capitalization = Font.AllUppercase
                }

                Rectangle {
                  implicitHeight: 1
                  Layout.fillWidth: true
                  color: Cpp_ThemeManager.colors["groupbox_border"]
                }

                Item {
                  implicitHeight: 2
                }

                Label {
                  Layout.fillWidth: true
                  wrapMode: Text.WordWrap
                  text: Cpp_Examples.selectedExample.description || ""
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["text"]
                }

                Item {
                  implicitHeight: 4
                }

                Label {
                  text: qsTr("Info")
                  font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
                  color: Cpp_ThemeManager.colors["pane_section_label"]
                  Component.onCompleted: font.capitalization = Font.AllUppercase
                }

                Rectangle {
                  implicitHeight: 1
                  Layout.fillWidth: true
                  color: Cpp_ThemeManager.colors["groupbox_border"]
                }

                Item {
                  implicitHeight: 2
                }

                GridLayout {
                  columns: 2
                  rowSpacing: 4
                  columnSpacing: 8
                  Layout.fillWidth: true

                  Label {
                    text: qsTr("Category:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }

                  Label {
                    text: Cpp_Examples.selectedExample.category || ""
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    Layout.fillWidth: true
                  }

                  Label {
                    text: qsTr("Difficulty:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }

                  Label {
                    font: Cpp_Misc_CommonFonts.uiFont
                    text: Cpp_Examples.selectedExample.difficulty || ""
                    Layout.fillWidth: true
                    color: {
                      var d = Cpp_Examples.selectedExample.difficulty || ""
                      return d === "Advanced" ? "#e74c3c"
                                              : d === "Intermediate" ? "#f39c12"
                                                                     : "#2ecc71"
                    }
                  }

                  Label {
                    text: qsTr("Project:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                    visible: Cpp_Examples.selectedExample.hasProjectFile || false
                  }

                  Label {
                    text: Cpp_Examples.selectedExample.projectFileName || ""
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                    visible: Cpp_Examples.selectedExample.hasProjectFile || false
                  }
                }

                Item { Layout.fillHeight: true }
              }
            }
          }
        }
      }

      //
      // No search results state
      //
      ColumnLayout {
        spacing: 8
        anchors.centerIn: parent
        visible: Cpp_Examples.count === 0 && searchField.text !== ""

        Image {
          sourceSize: Qt.size(96, 96)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/rcc/images/no-results.svg"
        }

        Item {
          implicitHeight: 4
        }

        Label {
          text: qsTr("No Results Found")
          Layout.alignment: Qt.AlignHCenter
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(1.25, true)
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.uiFont
          Layout.alignment: Qt.AlignHCenter
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Check the spelling or try a different search term.")
        }
      }
    }

    //
    // Bottom row: count + close
    //
    RowLayout {
      spacing: 4
      Layout.topMargin: 4
      Layout.fillWidth: true

      Label {
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("%1 examples").arg(Cpp_Examples.count)
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }

      Item {
        Layout.fillWidth: true
      }

      Button {
        icon.width: 18
        icon.height: 18
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
      }
    }
  }

  //
  // Fullscreen screenshot popup window
  //
  Window {
    id: screenshotPopup

    visible: false
    width: 800
    height: 600
    minimumWidth: 640
    minimumHeight: 480
    title: qsTr("Screenshot Preview")
    color: Cpp_ThemeManager.colors["window"]
    flags: Qt.Dialog | Qt.WindowCloseButtonHint

    Rectangle {
      radius: 2
      border.width: 1
      anchors.fill: parent
      anchors.margins: 16
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      Image {
        mipmap: true
        smooth: true
        asynchronous: true
        anchors.margins: 8
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: Cpp_Examples.selectedScreenshot || ""
      }
    }

    Shortcut {
      sequences: [StandardKey.Close, "Escape"]
      onActivated: screenshotPopup.close()
    }
  }
}
