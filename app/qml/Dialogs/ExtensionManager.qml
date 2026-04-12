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
  title: qsTr("Extension Manager")

  readonly property int toolbarHeight: 40
  property bool showDetail: Cpp_ExtensionManager.selectedIndex >= 0
  property bool showRepos: false
  property bool fetchingData: Cpp_ExtensionManager.loading && Cpp_ExtensionManager.count === 0
  property bool readmeViewReady: false

  function pushReadme() {
    if (!readmeViewReady || !readmeView.visible)
      return

    var md = Cpp_ExtensionManager.selectedReadme
    if (!md || md === "") {
      readmeView.runJavaScript(
        "document.getElementById('content').innerHTML = '<p style=\"opacity:0.5\">No description available.</p>';")
      return
    }

    var escaped = md.replace(/\\/g, '\\\\')
                    .replace(/`/g, '\\`')
                    .replace(/\$/g, '\\$')
    readmeView.runJavaScript("renderMarkdown(`" + escaped + "`);")
  }

  function pushReadmeTheme() {
    if (!readmeViewReady)
      return

    var json = Cpp_HelpCenter.themeColors
    readmeView.runJavaScript("setTheme(" + json + ");")
  }

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
      visible: !root.showDetail && !root.showRepos
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: searchField.activeFocus ? Cpp_ThemeManager.colors["highlight"]
                                            : Cpp_ThemeManager.colors["groupbox_border"]

      RowLayout {
        spacing: 4
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4

        Button {
          id: searchIcon

          opacity: 0.8
          icon.width: 12
          icon.height: 12
          background: Item {}
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/rcc/icons/buttons/search.svg"
        }

        TextField {
          id: searchField

          background: Item {}
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.uiFont
          placeholderText: qsTr("Search extensions...")
          onTextChanged: Cpp_ExtensionManager.setSearchFilter(text)
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 8
          Layout.bottomMargin: 8
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ComboBox {
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
            text: Cpp_ExtensionManager.friendlyTypeName(modelData)
            font: Cpp_Misc_CommonFonts.uiFont
            highlighted: typeFilter.highlightedIndex === index
          }
        }

        Rectangle {
          implicitWidth: 1
          Layout.topMargin: 8
          Layout.bottomMargin: 8
          Layout.fillHeight: true
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ToolButton {
          icon.width: 18
          icon.height: 18
          background: Item {}
          text: qsTr("Refresh")
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/refresh.svg"
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
          icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
          onClicked: root.showRepos = true

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
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
        anchors.fill: parent
        anchors.margins: 4

        ToolButton {
          icon.width: 18
          icon.height: 18
          text: qsTr("Back")
          background: Item {}
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/backward.svg"
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
          icon.source: "qrc:/rcc/icons/buttons/download.svg"
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
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
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
          icon.source: "qrc:/rcc/icons/buttons/media-play.svg"
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
          icon.source: "qrc:/rcc/icons/buttons/media-stop.svg"
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
          icon.source: "qrc:/rcc/icons/buttons/clear.svg"
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
          text: qsTr("Fetching extensions...")
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
        x: (root.showDetail || root.showRepos) ? -width : 0

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

        Flickable {
          anchors.fill: parent
          anchors.margins: 8
          contentHeight: gridColumn.implicitHeight
          clip: true
          boundsBehavior: Flickable.StopAtBounds
          flickableDirection: Flickable.VerticalFlick

          Column {
            id: gridColumn

            width: parent.width
            spacing: 4

            Repeater {
              id: sectionRepeater

              model: {
                // Build ordered list of unique types from extensions
                var types = []
                var seen = {}
                var exts = Cpp_ExtensionManager.extensions
                for (var i = 0; i < exts.length; ++i) {
                  var t = exts[i].type || ""
                  if (t !== "" && !seen[t]) {
                    seen[t] = true
                    types.push(t)
                  }
                }
                return types
              }

              delegate: Column {
                width: gridColumn.width
                spacing: 4

                required property string modelData
                required property int index

                //
                // Section header
                //
                Item {
                  width: parent.width
                  height: 28
                  visible: Cpp_ExtensionManager.filterType === ""
                           || Cpp_ExtensionManager.filterType === "All"

                  RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    spacing: 8

                    Label {
                      text: Cpp_ExtensionManager.friendlyTypeName(modelData)
                      color: Cpp_ThemeManager.colors["pane_section_label"]
                      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
                      Component.onCompleted: font.capitalization = Font.AllUppercase
                    }

                    Rectangle {
                      Layout.fillWidth: true
                      implicitHeight: 1
                      color: Cpp_ThemeManager.colors["groupbox_border"]
                    }
                  }
                }

                //
                // Cards grid for this type
                //
                Flow {
                  width: parent.width
                  spacing: 10

                  Repeater {
                    model: {
                      var items = []
                      var exts = Cpp_ExtensionManager.extensions
                      for (var i = 0; i < exts.length; ++i) {
                        if ((exts[i].type || "") === modelData) {
                          var item = exts[i]
                          item._flatIndex = i
                          items.push(item)
                        }
                      }
                      return items
                    }

                    delegate: Rectangle {
                      id: card

                      width: 200
                      height: 186
              radius: 6
              color: cardMouse.containsMouse ? Cpp_ThemeManager.colors["highlight"]
                                             : Cpp_ThemeManager.colors["base"]
              border.width: 1
              border.color: cardMouse.containsMouse ? Cpp_ThemeManager.colors["accent"]
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
                                "#922b21", "#117a65", "#7e5109", "#2e4053"
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
                                "#561914", "#0a493d", "#4b3006", "#1b2631"
                              ]
                          return p[index % p.length]
                        }
                      }
                    }

                    Image {
                      smooth: true
                      mipmap: true
                      asynchronous: true
                      anchors.fill: parent
                      fillMode: Image.PreserveAspectCrop
                      visible: status === Image.Ready
                      source: {
                        const ss = modelData.screenshot || ""
                        if (ss === "")
                          return ""
                        const base = modelData._repoBase || ""
                        if (modelData._isLocal)
                          return "file://" + base + ss
                        return base + ss
                      }
                    }

                    Label {
                      anchors.centerIn: parent
                      text: (modelData.title || "?").charAt(0)
                      visible: (modelData.screenshot || "") === ""
                      opacity: 0.6
                      color: "#ffffff"
                      font: Cpp_Misc_CommonFonts.customMonoFont(2.5, true)
                    }
                  }

                  //
                  // Status badges (top-right overlay)
                  //
                  Row {
                    spacing: 4
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 6

                    Rectangle {
                      visible: modelData.pluginRunning || false
                      width: runLabel.implicitWidth + 8
                      height: runLabel.implicitHeight + 4
                      radius: 3
                      color: "#2ecc71"

                      Label {
                        id: runLabel
                        anchors.centerIn: parent
                        text: qsTr("Running")
                        font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
                        color: "#ffffff"
                      }
                    }

                    Rectangle {
                      visible: (modelData.installed || false)
                               && !(modelData.pluginRunning || false)
                      width: installedLabel.implicitWidth + 8
                      height: installedLabel.implicitHeight + 4
                      radius: 3
                      color: modelData.updateAvailable
                             ? Cpp_ThemeManager.colors["error"]
                             : Cpp_ThemeManager.colors["accent"]

                      Label {
                        id: installedLabel
                        anchors.centerIn: parent
                        text: modelData.updateAvailable ? qsTr("Update") : qsTr("Installed")
                        font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
                        color: "#ffffff"
                      }
                    }

                    Rectangle {
                      visible: modelData.platformAvailable === false
                               && !(modelData.installed || false)
                      width: unavailLabel.implicitWidth + 8
                      height: unavailLabel.implicitHeight + 4
                      radius: 3
                      color: Cpp_ThemeManager.colors["placeholder_text"]

                      Label {
                        id: unavailLabel
                        anchors.centerIn: parent
                        text: qsTr("Unavailable")
                        font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
                        color: "#ffffff"
                      }
                    }
                  }

                  //
                  // Hover description overlay (slides up)
                  //
                  Item {
                    anchors.fill: parent
                    anchors.bottom: parent.bottom
                    height: parent.height
                    clip: true

                    Rectangle {
                      width: parent.width
                      height: parent.height
                      radius: 4
                      opacity: 0.92
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
                          text: modelData.title || ""
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
                          text: modelData.description || ""
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
                  text: modelData.title || ""
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  color: cardMouse.containsMouse
                         ? Cpp_ThemeManager.colors["highlighted_text"]
                         : Cpp_ThemeManager.colors["text"]
                }

                //
                // Type + author
                //
                RowLayout {
                  spacing: 4
                  Layout.fillWidth: true

                  Label {
                    text: Cpp_ExtensionManager.friendlyTypeName(modelData.type || "")
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                  }

                  Item { Layout.fillWidth: true }

                  Label {
                    text: modelData.author || ""
                    font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                    elide: Text.ElideRight
                    Layout.maximumWidth: 100
                  }
                }
              }

              MouseArea {
                id: cardMouse

                hoverEnabled: true
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Cpp_ExtensionManager.setSelectedIndex(modelData._flatIndex)
              }
            }
          }
        }

        Item { implicitHeight: 6 }

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
          // Left: README markdown viewer
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

              // Recover gracefully if the Chromium render process dies
              // (e.g. GPU sandbox failure on Linux) instead of crashing SS.
              onRenderProcessTerminated: function(terminationStatus, exitCode) {
                console.warn("ExtensionManager readme view: render process terminated",
                             terminationStatus, "exit", exitCode)
                root.readmeViewReady = false
              }

              onNavigationRequested: function(request) {
                var url = request.url.toString()
                if (url.startsWith("qrc:"))
                  return

                request.reject()
                if (url.startsWith("ext:"))
                  Qt.openUrlExternally(url.substring(4))
                else if (!url.startsWith("copy:") && !url.startsWith("nav:"))
                  Qt.openUrlExternally(url)
              }
            }

            Connections {
              target: Cpp_ExtensionManager
              function onSelectedReadmeChanged() {
                root.pushReadme()
              }
            }

            Connections {
              target: Cpp_HelpCenter
              function onThemeColorsChanged() {
                root.pushReadmeTheme()
              }
            }
          }

          //
          // Right sidebar: details + screenshot/plugin log
          //
          ColumnLayout {
            spacing: 8
            Layout.minimumWidth: 280
            Layout.maximumWidth: 280
            Layout.fillHeight: true

            //
            // Details panel
            //
            Item {
              Layout.fillWidth: true
              Layout.minimumHeight: detailsCol.implicitHeight + 24

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              ColumnLayout {
                id: detailsCol

                spacing: 4
                anchors.fill: parent
                anchors.margins: 8

                Item { implicitHeight: 2 }

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

                Item { implicitHeight: 2 }

                GridLayout {
                  columns: 2
                  rowSpacing: 4
                  columnSpacing: 8
                  Layout.fillWidth: true

                  Label {
                    text: qsTr("Type:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }
                  Label {
                    text: Cpp_ExtensionManager.friendlyTypeName(
                            Cpp_ExtensionManager.selectedExtension.type || "")
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    Layout.fillWidth: true
                  }

                  Label {
                    text: qsTr("Author:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }
                  Label {
                    text: Cpp_ExtensionManager.selectedExtension.author || ""
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    Layout.fillWidth: true
                  }

                  Label {
                    text: qsTr("Version:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }
                  Label {
                    text: Cpp_ExtensionManager.selectedExtension.version || ""
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    Layout.fillWidth: true
                  }

                  Label {
                    text: qsTr("License:")
                    font: Cpp_Misc_CommonFonts.boldUiFont
                    color: Cpp_ThemeManager.colors["text"]
                  }
                  Label {
                    text: Cpp_ExtensionManager.selectedExtension.license || ""
                    font: Cpp_Misc_CommonFonts.uiFont
                    color: Cpp_ThemeManager.colors["text"]
                    Layout.fillWidth: true
                  }
                }
              }
            }

            //
            // Screenshot
            //
            Item {
              Layout.fillWidth: true
              Layout.minimumHeight: extensionScreenshot.status === Image.Ready
                                    ? Math.min(extensionScreenshot.sourceSize.height
                                               * (268 / extensionScreenshot.sourceSize.width), 200)
                                    : 140
              Layout.maximumHeight: Layout.minimumHeight
              visible: (Cpp_ExtensionManager.selectedExtension.screenshot || "") !== ""

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              Image {
                id: extensionScreenshot

                mipmap: true
                smooth: true
                anchors.margins: 1
                asynchronous: true
                anchors.fill: parent
                visible: status === Image.Ready
                fillMode: Image.PreserveAspectFit
                source: {
                  const ss = Cpp_ExtensionManager.selectedExtension.screenshot || ""
                  if (ss === "")
                    return ""

                  const base = Cpp_ExtensionManager.selectedExtension._repoBase || ""
                  const isLocal = Cpp_ExtensionManager.selectedExtension._isLocal || false
                  if (isLocal)
                    return "file://" + base + ss

                  return base + ss
                }
              }

              Label {
                anchors.centerIn: parent
                visible: !extensionScreenshot.visible
                text: qsTr("No preview")
                color: Cpp_ThemeManager.colors["placeholder_text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              }
            }

            //
            // Plugin output log
            //
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true
              opacity: (Cpp_ExtensionManager.selectedExtension.type || "") === "plugin" ? 1 : 0

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["console_base"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              ColumnLayout {
                spacing: 0
                anchors.fill: parent
                anchors.margins: 1

                Label {
                  text: qsTr("  PLUGIN OUTPUT")
                  font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
                  color: Cpp_ThemeManager.colors["pane_section_label"]
                  Layout.fillWidth: true
                  Layout.topMargin: 6
                  Layout.bottomMargin: 4
                  Component.onCompleted: font.capitalization = Font.AllUppercase
                }

                Rectangle {
                  implicitHeight: 1
                  Layout.fillWidth: true
                  color: Cpp_ThemeManager.colors["groupbox_border"]
                }

                ScrollView {
                  Layout.fillWidth: true
                  Layout.fillHeight: true
                  clip: true

                  TextArea {
                    id: pluginLogArea

                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    color: Cpp_ThemeManager.colors["console_text"]
                    font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)
                    background: Item {}
                    text: {
                      const id = Cpp_ExtensionManager.selectedExtension.id || ""
                      return id !== "" ? (Cpp_ExtensionManager.pluginOutput(id) || qsTr("No output yet. Run the plugin to see its log here."))
                                       : ""
                    }

                    Connections {
                      target: Cpp_ExtensionManager
                      function onPluginOutputChanged(pluginId) {
                        if (pluginId === (Cpp_ExtensionManager.selectedExtension.id || ""))
                          pluginLogArea.text = Cpp_ExtensionManager.pluginOutput(pluginId)
                      }
                    }
                  }
                }
              }
            }

            //
            // Spacer when no screenshot and not a plugin
            //
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true
              visible: (Cpp_ExtensionManager.selectedExtension.screenshot || "") === ""
                       && (Cpp_ExtensionManager.selectedExtension.type || "") !== "plugin"

              Rectangle {
                radius: 2
                border.width: 1
                anchors.fill: parent
                color: Cpp_ThemeManager.colors["groupbox_background"]
                border.color: Cpp_ThemeManager.colors["groupbox_border"]
              }

              Label {
                anchors.centerIn: parent
                text: qsTr("No preview available")
                color: Cpp_ThemeManager.colors["placeholder_text"]
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              }
            }
          }
        }
      }

      //
      // Page 2: Repository settings
      //
      Item {
        id: reposPage

        width: parent.width
        height: parent.height
        x: root.showRepos ? 0 : width

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

        ColumnLayout {
          spacing: 4
          anchors.fill: parent
          anchors.margins: 12

          Item {
            implicitHeight: 2
          }

          Label {
            text: qsTr("Repositories")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Label {
            text: qsTr("Add URLs to remote repositories or local folder paths.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["placeholder_text"]
          }

          Item {
            implicitHeight: 4
          }

          ListView {
            id: repoList

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: Cpp_ExtensionManager.repositories
            spacing: 4

            delegate: Rectangle {
              width: repoList.width
              height: repoRow.implicitHeight + 8
              radius: 4
              color: Cpp_ThemeManager.colors["base"]
              border.width: 1
              border.color: Cpp_ThemeManager.colors["mid"]

              RowLayout {
                id: repoRow

                spacing: 8
                anchors.fill: parent
                anchors.margins: 4

                Label {
                  visible: Cpp_ExtensionManager.isLocalRepo(modelData)
                  text: qsTr("LOCAL")
                  font: Cpp_Misc_CommonFonts.customUiFont(0.7, true)
                  color: Cpp_ThemeManager.colors["highlighted_text"]

                  background: Rectangle {
                    radius: 2
                    color: Cpp_ThemeManager.colors["accent"]
                    anchors.fill: parent
                    anchors.margins: -2
                  }
                }

                Label {
                  text: modelData
                  elide: Text.ElideMiddle
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
                  color: Cpp_ThemeManager.colors["text"]
                }

                ToolButton {
                  icon.width: 12
                  icon.height: 12
                  background: Item {}
                  icon.color: Cpp_ThemeManager.colors["text"]
                  icon.source: "qrc:/rcc/icons/buttons/close.svg"
                  onClicked: Cpp_ExtensionManager.removeRepository(index)

                  HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                  }
                }
              }
            }
          }

          Rectangle {
            radius: 2
            border.width: 1
            Layout.fillWidth: true
            implicitHeight: 36
            color: Cpp_ThemeManager.colors["base"]
            border.color: newRepoField.activeFocus ? Cpp_ThemeManager.colors["highlight"]
                                                   : Cpp_ThemeManager.colors["mid"]

            RowLayout {
              spacing: 4
              anchors.fill: parent
              anchors.leftMargin: 4
              anchors.rightMargin: 4

              TextField {
                id: newRepoField

                background: Item {}
                Layout.fillWidth: true
                font: Cpp_Misc_CommonFonts.customMonoFont(0.85, false)
                placeholderText: qsTr("URL or local path...")
                onAccepted: addRepoButton.clicked()
              }

              ToolButton {
                id: addRepoButton

                background: Item {}
                text: qsTr("Add")
                enabled: newRepoField.text.length > 0
                icon.color: Cpp_ThemeManager.colors["text"]
                onClicked: {
                  Cpp_ExtensionManager.addRepository(newRepoField.text)
                  newRepoField.text = ""
                }

                HoverHandler {
                  cursorShape: Qt.PointingHandCursor
                }
              }

              Rectangle {
                implicitWidth: 1
                Layout.topMargin: 6
                Layout.bottomMargin: 6
                Layout.fillHeight: true
                color: Cpp_ThemeManager.colors["mid"]
              }

              ToolButton {
                background: Item {}
                text: qsTr("Browse...")
                icon.width: 12
                icon.height: 12
                icon.color: Cpp_ThemeManager.colors["text"]
                icon.source: "qrc:/rcc/icons/toolbar/open-project.svg"
                onClicked: Cpp_ExtensionManager.browseLocalRepo()

                HoverHandler {
                  cursorShape: Qt.PointingHandCursor
                }
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
        visible: Cpp_ExtensionManager.count === 0 && searchField.text !== ""
                 && !Cpp_ExtensionManager.loading && !root.showDetail && !root.showRepos

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
          source: "qrc:/rcc/images/no-results.svg"
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
