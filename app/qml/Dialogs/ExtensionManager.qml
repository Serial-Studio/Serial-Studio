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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets
import "ExtensionManager" as ExtensionPages

Widgets.SmartDialog {
  id: root

  title: qsTr("Extension Manager")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  property bool showRepos: false
  readonly property int toolbarHeight: 40
  property bool showDetail: Cpp_ExtensionManager.selectedIndex >= 0
  property bool fetchingData: Cpp_ExtensionManager.loading && Cpp_ExtensionManager.count === 0

  onVisibleChanged: {
    if (visible) {
      Cpp_ExtensionManager.refreshRepositories()
    }

    else {
      Cpp_ExtensionManager.setSelectedIndex(-1)
      Cpp_ExtensionManager.setSearchFilter("")
      Cpp_ExtensionManager.setFilterType("")
      root.showRepos = false
    }
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 8
    anchors.centerIn: parent

    //
    // Search bar (grid page only)
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.minimumWidth: 860
      Layout.maximumWidth: 860
      visible: !root.showDetail && !root.showRepos

      Widgets.SearchField {
        id: searchField

        Layout.fillWidth: true
        placeholderText: qsTr("Search extensions…")
        onTextChanged: Cpp_ExtensionManager.setSearchFilter(text)
      }

      Widgets.Combo {
        id: typeFilter

        implicitWidth: 160
        font: Cpp_Misc_CommonFonts.uiFont
        model: Cpp_ExtensionManager.extensionTypes()
        displayText: Cpp_ExtensionManager.friendlyTypeName(currentText)
        onCurrentTextChanged: Cpp_ExtensionManager.setFilterType(currentText)

        Connections {
          target: Cpp_ExtensionManager
          function onFilterTypeChanged() {
            var ft = Cpp_ExtensionManager.filterType
            var types = Cpp_ExtensionManager.extensionTypes()
            for (var i = 0; i < types.length; ++i) {
              if (types[i] === ft || (ft === "" && types[i] === "All")) {
                typeFilter.currentIndex = i
                return
              }
            }

            typeFilter.currentIndex = 0
          }
        }

        delegate: ItemDelegate {
          width: typeFilter.width
          font: Cpp_Misc_CommonFonts.uiFont
          highlighted: typeFilter.highlightedIndex === index
          text: Cpp_ExtensionManager.friendlyTypeName(modelData)
        }
      }

      ToolButton {
        icon.width: 18
        icon.height: 18
        background: Item {}
        text: qsTr("Refresh")
        icon.color: Cpp_ThemeManager.colors["text"]
        icon.source: "qrc:/icons/buttons/refresh.svg"
        onClicked: Cpp_ExtensionManager.refreshRepositories()

        HoverHandler {
          cursorShape: Qt.PointingHandCursor
        }
      }

      ToolButton {
        icon.width: 18
        icon.height: 18
        background: Item {}
        text: qsTr("Repos")
        visible: Cpp_CommercialBuild
        icon.color: Cpp_ThemeManager.colors["text"]
        icon.source: Cpp_Misc_IconRegistry.icon("commands", "settings", 18)
        onClicked: root.showRepos = true

        HoverHandler {
          cursorShape: Qt.PointingHandCursor
        }
      }
    }

    //
    // Toolbar (detail page / repos page)
    //
    Rectangle {
      radius: 2
      border.width: 1
      Layout.fillWidth: true
      Layout.minimumWidth: 860
      Layout.maximumWidth: 860
      implicitHeight: root.toolbarHeight
      visible: root.showDetail || root.showRepos
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      Label {
        anchors.centerIn: parent
        elide: Text.ElideRight
        font: Cpp_Misc_CommonFonts.customUiFont(1.0, true)
        text: root.showRepos ? qsTr("Repository Settings")
                             : (Cpp_ExtensionManager.selectedExtension.title || "")
      }

      RowLayout {
        spacing: 4
        anchors.margins: 4
        anchors.fill: parent

        ToolButton {
          icon.width: 18
          icon.height: 18
          text: qsTr("Back")
          background: Item {}
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/backward.svg"
          onClicked: {
            if (root.showRepos)
              root.showRepos = false
            else
              Cpp_ExtensionManager.setSelectedIndex(-1)
          }

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

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 4
          Layout.bottomMargin: 4
          Layout.fillHeight: true
          visible: root.showDetail
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          text: qsTr("Install")
          enabled: !Cpp_ExtensionManager.loading
                   && (Cpp_ExtensionManager.selectedExtension.platformAvailable !== false)
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/download.svg"
          onClicked: Cpp_ExtensionManager.installExtension()
          visible: root.showDetail && !Cpp_ExtensionManager.isInstalled(Cpp_ExtensionManager.selectedExtension.id || "")

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          text: qsTr("Uninstall")
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/close.svg"
          onClicked: Cpp_ExtensionManager.uninstallExtension()

          visible: {
            if (!root.showDetail)
              return false

            const a = Cpp_ExtensionManager.selectedExtension
            if (!Cpp_ExtensionManager.isInstalled(a.id || ""))
              return false

            if ((a.type || "") === "plugin")
              return !Cpp_ExtensionManager.isPluginRunning(a.id || "")

            return true
          }

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          text: qsTr("Run")
          background: Item {}
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/media-play.svg"
          onClicked: Cpp_ExtensionManager.launchSelectedPlugin()

          visible: {
            if (!root.showDetail)
              return false

            const a = Cpp_ExtensionManager.selectedExtension
            return (a.type || "") === "plugin"
                && Cpp_ExtensionManager.isInstalled(a.id || "")
                && !Cpp_ExtensionManager.isPluginRunning(a.id || "")
          }

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          text: qsTr("Stop")
          background: Item {}
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/media-stop.svg"
          onClicked: Cpp_ExtensionManager.stopSelectedPlugin()

          visible: {
            if (!root.showDetail)
              return false

            const a = Cpp_ExtensionManager.selectedExtension
            return (a.type || "") === "plugin"
                && Cpp_ExtensionManager.isPluginRunning(a.id || "")
          }

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 4
          Layout.bottomMargin: 4
          Layout.fillHeight: true
          visible: root.showRepos
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          visible: root.showRepos
          text: qsTr("Reset")
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/icons/buttons/clear.svg"
          onClicked: Cpp_ExtensionManager.resetRepositories()

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
      value: Cpp_ExtensionManager.downloadProgress
      visible: Cpp_ExtensionManager.loading && Cpp_ExtensionManager.downloadProgress > 0
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
      // Busy indicator while fetching
      //
      ColumnLayout {
        spacing: 8
        anchors.centerIn: parent
        visible: root.fetchingData && Cpp_ExtensionManager.loading

        BusyIndicator {
          running: parent.visible
          Layout.alignment: Qt.AlignHCenter
        }

        Label {
          text: qsTr("Fetching extensions…")
          font: Cpp_Misc_CommonFonts.boldUiFont
          color: Cpp_ThemeManager.colors["text"]
          Layout.alignment: Qt.AlignHCenter
        }
      }

      //
      // Page 0: Grid view
      //
      ExtensionPages.GridPage {
        id: gridPage

        showRepos: root.showRepos
        showDetail: root.showDetail
        fetchingData: root.fetchingData
      }

      //
      // Page 1: Detail view
      //
      ExtensionPages.DetailPage {
        id: detailPage

        showDetail: root.showDetail
      }

      //
      // Page 2: Repository settings
      //
      ExtensionPages.ReposPage {
        id: reposPage

        showRepos: root.showRepos
      }

      //
      // No search results state
      //
      ColumnLayout {
        spacing: 8
        anchors.centerIn: parent
        visible: Cpp_ExtensionManager.count === 0 && searchField.text !== ""
                 && !Cpp_ExtensionManager.loading && !root.showDetail && !root.showRepos

        Image {
          sourceSize: Qt.size(96, 96)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/images/no-results.svg"
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

      //
      // Empty state (no extensions available at all)
      //
      ColumnLayout {
        spacing: 8
        anchors.centerIn: parent
        visible: Cpp_ExtensionManager.count === 0 && searchField.text === ""
                 && !Cpp_ExtensionManager.loading && !root.showDetail && !root.showRepos

        Image {
          sourceSize: Qt.size(96, 96)
          Layout.alignment: Qt.AlignHCenter
          source: "qrc:/images/no-results.svg"
        }

        Item {
          implicitHeight: 4
        }

        Label {
          text: qsTr("No Extensions Available")
          Layout.alignment: Qt.AlignHCenter
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(1.25, true)
        }

        Label {
          opacity: 0.7
          font: Cpp_Misc_CommonFonts.uiFont
          Layout.alignment: Qt.AlignHCenter
          color: Cpp_ThemeManager.colors["text"]
          text: qsTr("Add a repository URL or local path in the Repos settings, then refresh.")
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
        text: qsTr("%1 extensions").arg(Cpp_ExtensionManager.count)
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/close.svg"
      }
    }
  }
}
