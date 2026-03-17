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
  // Helper: handle link clicks — internal links navigate, external open browser
  //
  function handleLink(link) {
    // External links — open in browser
    if (link.startsWith("http://") || link.startsWith("https://") || link.startsWith("mailto:")) {
      Qt.openUrlExternally(link)
      return
    }

    // Qt resolves relative markdown links against qrc — strip the prefix
    // e.g. "qrc:/serial-studio.com/gui/qml/Dialogs/Page-Name.md#anchor"
    var resolved = link
    if (resolved.startsWith("qrc:/") || resolved.startsWith("qrc://")) {
      var lastSlash = resolved.lastIndexOf("/")
      if (lastSlash >= 0)
        resolved = resolved.substring(lastSlash + 1)
    }

    // Split into page and anchor parts
    var hashIdx = resolved.indexOf("#")
    var pagePart = hashIdx >= 0 ? resolved.substring(0, hashIdx) : resolved
    var anchorPart = hashIdx >= 0 ? resolved.substring(hashIdx) : ""

    // Pure anchor link — scroll within current page
    if (pagePart === "" && anchorPart !== "") {
      var anchor = anchorPart.substring(1).replace(/-/g, " ").replace(/--/g, " & ")
      var pos = contentArea.text.toLowerCase().indexOf(anchor)
      if (pos >= 0) {
        contentArea.cursorPosition = pos
        var rect = contentArea.positionToRectangle(pos)
        contentScrollView.contentItem.contentY = Math.max(0, rect.y - 20)
      }
      return
    }

    // Internal page link — navigate via C++
    if (pagePart !== "" && Cpp_HelpCenter.navigateToPage(pagePart))
      return

    // Fallback — open externally
    Qt.openUrlExternally(link)
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
      // Right content area
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
        // Markdown content
        //
        ScrollView {
          id: contentScrollView

          anchors.fill: parent
          anchors.margins: 1
          contentWidth: availableWidth
          visible: Cpp_HelpCenter.pageContent !== ""

          TextArea {
            id: contentArea

            readOnly: true
            textFormat: TextArea.MarkdownText
            wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere
            font: Cpp_Misc_CommonFonts.uiFont
            onLinkActivated: (link) => root.handleLink(link)

            //
            // Force full markdown re-parse by clearing first
            //
            Connections {
              target: Cpp_HelpCenter
              function onPageContentChanged() {
                contentArea.text = ""
                reloadTimer.restart()
              }
            }

            Timer {
              id: reloadTimer
              interval: 1
              onTriggered: contentArea.text = Cpp_HelpCenter.pageContent
            }

            background: Rectangle {
              color: "transparent"
            }

            HoverHandler {
              acceptedButtons: Qt.NoButton
              cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
            }
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
