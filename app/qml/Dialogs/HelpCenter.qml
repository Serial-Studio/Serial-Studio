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
import QtWebEngine

import "../Widgets"

SmartDialog {
  id: root
  title: qsTr("Help Center")

  //
  // Allow resizing and maximizing
  //
  width: 920
  height: 620
  minimumWidth: 720
  minimumHeight: 480
  maximumWidth: 10000
  maximumHeight: 10000
  Component.onCompleted: {
    root.flags = Qt.Dialog
               | Qt.CustomizeWindowHint
               | Qt.WindowTitleHint
               | Qt.WindowCloseButtonHint
               | Qt.WindowMinMaxButtonsHint
  }

  //
  // Track whether the WebView HTML shell is ready to receive content
  //
  property bool webViewReady: false

  //
  // Push markdown content into the WebView via JS
  //
  function pushContent() {
    if (!webViewReady || !contentView.visible)
      return

    var md = Cpp_HelpCenter.pageContent
    if (md === "") {
      contentView.runJavaScript("document.getElementById('content').innerHTML = '';")
      return
    }

    // Escape the markdown for safe JS string injection
    var escaped = md.replace(/\\/g, '\\\\')
                    .replace(/`/g, '\\`')
                    .replace(/\$/g, '\\$')
    contentView.runJavaScript("renderMarkdown(`" + escaped + "`);")
  }

  //
  // Push theme colors into the WebView
  //
  function pushTheme() {
    if (!webViewReady)
      return

    var json = Cpp_HelpCenter.themeColors
    contentView.runJavaScript("setTheme(" + json + ");")
  }

  //
  // Fetch manifest when dialog opens
  //
  onVisibleChanged: {
    if (visible)
      Cpp_HelpCenter.fetchManifest()
  }

  //
  // Reset state on close
  //
  onClosing: {
    searchField.text = ""
    Cpp_HelpCenter.currentIndex = -1
    Cpp_HelpCenter.searchFilter = ""
  }

  //
  // Track whether we are still fetching the manifest
  //
  property bool fetchingData: Cpp_HelpCenter.count === 0 && Cpp_HelpCenter.searchFilter === ""

  dialogContent: ColumnLayout {
    spacing: 8

    //
    // Busy indicator while fetching manifest
    //
    ColumnLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      visible: root.fetchingData

      Item { Layout.fillHeight: true }

      BusyIndicator {
        running: parent.visible
        Layout.alignment: Qt.AlignHCenter
      }

      Label {
        text: qsTr("Fetching help pages...")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["text"]
        Layout.alignment: Qt.AlignHCenter
      }

      Item { Layout.fillHeight: true }
    }

    //
    // Main content area: sidebar + page content
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.fillHeight: true
      visible: !root.fetchingData

      //
      // Left sidebar
      //
      ColumnLayout {
        spacing: 4
        Layout.minimumWidth: 220
        Layout.maximumWidth: 220
        Layout.fillHeight: true

        //
        // Search bar
        //
        SearchField {
          id: searchField

          Layout.fillWidth: true
          placeholderText: qsTr("Search...")
          onTextChanged: Cpp_HelpCenter.searchFilter = text
        }

        //
        // Page list with section headers
        //
        Rectangle {
          radius: 2
          border.width: 1
          Layout.fillWidth: true
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          ListView {
            id: sidebarList

            clip: true
            anchors.fill: parent
            anchors.margins: 4
            model: Cpp_HelpCenter.pages
            currentIndex: Cpp_HelpCenter.currentIndex

            section.property: "section"
            section.delegate: Item {
              width: sidebarList.width
              height: sectionLabel.implicitHeight + 12

              Label {
                id: sectionLabel

                y: 8
                anchors.left: parent.left
                anchors.leftMargin: 4
                text: section
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
                color: Cpp_ThemeManager.colors["pane_section_label"]
                Component.onCompleted: font.capitalization = Font.AllUppercase
              }
            }

            delegate: Rectangle {
              width: sidebarList.width
              height: pageLabel.implicitHeight + 8
              radius: 2
              color: {
                if (index === Cpp_HelpCenter.currentIndex)
                  return Cpp_ThemeManager.colors["highlight"]

                return pageMouse.containsMouse
                         ? Cpp_ThemeManager.colors["base"]
                         : "transparent"
              }

              Label {
                id: pageLabel

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 6
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.title
                elide: Text.ElideRight
                font: Cpp_Misc_CommonFonts.uiFont
                color: index === Cpp_HelpCenter.currentIndex
                         ? Cpp_ThemeManager.colors["highlighted_text"]
                         : Cpp_ThemeManager.colors["text"]
              }

              MouseArea {
                id: pageMouse

                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Cpp_HelpCenter.currentIndex = index
              }
            }
          }
        }
      }

      //
      // Right content area — WebEngineView
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

        //
        // Busy indicator while loading
        //
        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent
          visible: Cpp_HelpCenter.loading && Cpp_HelpCenter.pageContent === ""

          BusyIndicator {
            running: parent.visible
            Layout.alignment: Qt.AlignHCenter
          }

          Label {
            text: qsTr("Loading...")
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
            Layout.alignment: Qt.AlignHCenter
          }
        }

        //
        // WebEngineView for rendered markdown
        //
        WebEngineView {
          id: contentView

          anchors.fill: parent
          anchors.margins: 2
          visible: Cpp_HelpCenter.pageContent !== ""
          backgroundColor: "transparent"
          url: "qrc:/rcc/markdown-viewer.html"
          settings.localContentCanAccessRemoteUrls: true

          //
          // WebView is ready once the HTML shell has loaded
          //
          onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
              root.webViewReady = true
              root.pushTheme()
              root.pushContent()
            }
          }

          //
          // Handle navigation requests from the page (link clicks)
          //
          onNavigationRequested: function(request) {
            var url = request.url.toString()

            // Allow initial page load from qrc
            if (url.startsWith("qrc:"))
              return

            // External link — open in browser
            if (url.startsWith("ext:")) {
              request.reject()
              Qt.openUrlExternally(url.substring(4))
              return
            }

            // Copy code to clipboard
            if (url.startsWith("copy:")) {
              request.reject()
              var text = decodeURIComponent(url.substring(5))
              Cpp_Misc_Utilities.copyText(text)
              copyToast.show()
              return
            }

            // Internal page navigation
            if (url.startsWith("nav:")) {
              request.reject()
              var link = url.substring(4)

              // Decode percent-encoded characters
              link = decodeURIComponent(link)

              // Try internal navigation
              if (!Cpp_HelpCenter.navigateToPage(link))
                Qt.openUrlExternally(link)

              return
            }

            // Block all other navigations
            request.reject()
          }
        }

        //
        // React to content changes from C++
        //
        Connections {
          target: Cpp_HelpCenter
          function onPageContentChanged() {
            root.pushContent()
          }
        }

        //
        // React to theme changes from C++
        //
        Connections {
          target: Cpp_HelpCenter
          function onThemeColorsChanged() {
            root.pushTheme()
          }
        }

        //
        // Placeholder when no page selected
        //
        Label {
          anchors.centerIn: parent
          text: qsTr("Select a page from the sidebar")
          visible: !Cpp_HelpCenter.loading && Cpp_HelpCenter.pageContent === "" && Cpp_HelpCenter.currentIndex < 0
          color: Cpp_ThemeManager.colors["placeholder_text"]
          font: Cpp_Misc_CommonFonts.uiFont
        }

        //
        // "Copied to Clipboard" toast notification
        //
        Rectangle {
          id: copyToast

          opacity: 0
          radius: 4
          anchors.bottom: parent.bottom
          anchors.horizontalCenter: parent.horizontalCenter
          anchors.bottomMargin: 24
          width: copyToastLabel.implicitWidth + 24
          height: copyToastLabel.implicitHeight + 12
          color: Cpp_ThemeManager.colors["highlight"]

          function show() {
            copyToast.opacity = 1
            copyToastTimer.restart()
          }

          Label {
            id: copyToastLabel

            anchors.centerIn: parent
            text: qsTr("Copied to Clipboard")
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["highlighted_text"]
          }

          Timer {
            id: copyToastTimer

            interval: 1500
            onTriggered: copyToast.opacity = 0
          }

          Behavior on opacity {
            NumberAnimation {
              duration: 150
            }
          }
        }
      }
    }

    //
    // Bottom row: View on GitHub + count + close
    //
    RowLayout {
      spacing: 4
      Layout.topMargin: 4
      Layout.fillWidth: true
      visible: !root.fetchingData

      Button {
        icon.width: 18
        icon.height: 18
        horizontalPadding: 8
        text: qsTr("View Online")
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/rcc/icons/buttons/website.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: {
          var url = "https://serial-studio.com/help"
          var pageId = Cpp_HelpCenter.pageId
          if (pageId !== "")
            url += "#" + pageId

          Qt.openUrlExternally(url)
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Label {
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("%1 pages").arg(Cpp_HelpCenter.count)
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
}
