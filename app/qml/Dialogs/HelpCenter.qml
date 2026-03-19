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
import QtWebView

import "../Widgets"

SmartDialog {
  id: root

  title: qsTr("Help Center")

  //
  // Allow resizing and maximizing
  //
  minimumWidth: 720
  minimumHeight: 480
  maximumWidth: 10000
  maximumHeight: 10000
  Component.onCompleted: {
    root.width = 920
    root.height = 620
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
    if (!webViewReady)
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

  contentItem: ColumnLayout {
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
      // Right content area — WebView and status panels are never
      // overlapped, because QtWebView does not support overlapping
      // QML items on top of a WebView.
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        //
        // Whether to show the WebView or a status panel
        //
        property bool showWebView: Cpp_HelpCenter.pageContent !== ""

        //
        // Status panel (busy indicator / placeholder) — shown when
        // there is no page content to display
        //
        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          visible: !parent.showWebView
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          //
          // Busy indicator while loading
          //
          ColumnLayout {
            spacing: 8
            anchors.centerIn: parent
            visible: Cpp_HelpCenter.loading

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
          // Placeholder when no page selected
          //
          Label {
            anchors.centerIn: parent
            text: qsTr("Select a page from the sidebar")
            visible: !Cpp_HelpCenter.loading && Cpp_HelpCenter.currentIndex < 0
            color: Cpp_ThemeManager.colors["placeholder_text"]
            font: Cpp_Misc_CommonFonts.uiFont
          }
        }

        //
        // Border drawn around the WebView using edge rectangles
        // (cannot overlap the WebView per QtWebView limitations)
        //
        Rectangle {
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.right: parent.right
          visible: parent.showWebView
          implicitHeight: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          visible: parent.showWebView
          implicitHeight: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.bottom: parent.bottom
          visible: parent.showWebView
          implicitWidth: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        Rectangle {
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          visible: parent.showWebView
          implicitWidth: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        //
        // WebView for rendered markdown — only visible when we
        // have content, so it never overlaps the status panel
        //
        WebView {
          id: contentView

          anchors.fill: parent
          anchors.margins: 1
          visible: parent.showWebView

          //
          // Load the HTML shell once the WebView becomes visible.
          // On Windows, WebView2 fails to initialize when its parent
          // is hidden (the RowLayout starts with visible: false while
          // the manifest is being fetched).
          //
          property bool htmlLoaded: false
          onVisibleChanged: {
            if (visible && !htmlLoaded) {
              htmlLoaded = true
              contentView.loadHtml(Cpp_HelpCenter.viewerHtml)
            }
          }

          //
          // WebView is ready once the HTML shell has loaded
          //
          onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebView.LoadSucceededStatus) {
              root.webViewReady = true
              root.pushTheme()
              root.pushContent()
            }
          }

          //
          // Handle navigation requests from the page (link clicks)
          // Page signals Qt by setting document.title to action strings
          //
          onTitleChanged: {
            var str = contentView.title

            // External link — open in browser
            if (str.startsWith("ext:")) {
              Qt.openUrlExternally(str.substring(4))
              return
            }

            // Copy code to clipboard
            if (str.startsWith("copy:")) {
              var text = decodeURIComponent(str.substring(5))
              Cpp_Misc_Utilities.copyText(text)
              return
            }

            // Internal page navigation
            if (str.startsWith("nav:")) {
              var link = decodeURIComponent(str.substring(4))
              if (!Cpp_HelpCenter.navigateToPage(link))
                Qt.openUrlExternally(link)

              return
            }
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
        text: qsTr("View on GitHub")
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/rcc/icons/buttons/website.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
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
