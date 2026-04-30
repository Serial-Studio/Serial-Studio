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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import SerialStudio.UI as SS_Ui

import "../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: 30

  //
  // Custom properties
  //
  readonly property int iconSize: 18
  required property SS_Ui.TaskBar taskBar
  property var startMenu: null
  property var combinedSearchResults: []

  //
  // Rebuilds the unified search list — widget hits first, then start menu
  // actions — whenever the filter or widget index changes.
  //
  function refreshCombinedSearchResults() {
    var widgetHits = taskBar ? taskBar.searchResults : []
    var menuHits = []
    if (startMenu && typeof startMenu.searchableItems === "function") {
      var items = startMenu.searchableItems(taskBar ? taskBar.searchFilter : "")
      for (var i = 0; i < items.length; ++i) {
        menuHits.push({
          "isStartMenuAction": true,
          "widgetName": items[i].name,
          "widgetIcon": items[i].icon,
          "groupName": qsTr("Start Menu"),
          "run": items[i].run
        })
      }
    }

    combinedSearchResults = widgetHits.concat(menuHits)
    if (searchResultsList)
      searchResultsList.currentIndex = combinedSearchResults.length > 0 ? 0 : -1
  }

  // Snapshot entry fields before dismissSearch() rebuilds the model.
  function triggerSearchEntry(entry) {
    if (!entry)
      return

    if (entry["isStartMenuAction"]) {
      var run = entry["run"]
      taskBar.dismissSearch()
      if (typeof run === "function")
        run()
    } else if (entry["isWorkspace"]) {
      var gid = entry["groupId"]
      taskBar.dismissSearch()
      taskBar.activeGroupId = gid
    } else {
      var wid = entry["windowId"]
      var grp = entry["groupId"]
      taskBar.dismissSearch()
      taskBar.navigateToWidget(wid, grp)
    }
  }

  Component.onCompleted: refreshCombinedSearchResults()
  onStartMenuChanged: refreshCombinedSearchResults()
  Connections {
    target: taskBar
    function onSearchResultsChanged() { refreshCombinedSearchResults() }
    function onSearchFilterChanged()  { refreshCombinedSearchResults() }
  }

  //
  // Signals
  //
  signal startClicked()
  signal settingsClicked()
  signal extendWindowClicked()
  signal newWorkspaceRequested()
  signal editWorkspaceRequested(int workspaceId, string currentName)

  //
  // Focus the search field (called externally)
  //
  function focusSearch() {
    searchField.forceActiveFocus(Qt.MouseFocusReason)
    searchPopup.open()
  }

  //
  // Taskbar background
  //
  Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["taskbar_top"]
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["taskbar_bottom"]
      }
    }
  }

  //
  // Taskbar components
  //
  RowLayout {
    anchors {
      leftMargin: 2
      rightMargin: 2
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
    }

    spacing: 2

    //
    // Start Menu
    //
    Widgets.TaskbarButton {
      id: start

      iconSize: 16
      startMenu: true
      text: qsTr("Menu")
      implicitWidth: start.layout.implicitWidth + 8
      icon.source: Cpp_ThemeManager.parameters["start-icon"]
      onClicked: {
        root.startClicked()
        taskBar.activeWindow = null
      }
    }

    //
    // Search field
    //
    Item {
      id: searchContainer

      Layout.preferredHeight: 22
      Layout.preferredWidth: 172
      Layout.alignment: Qt.AlignVCenter

      function clearSearch() {
        searchField.text = ""
        searchField.focus = false
      }

      TextField {
        id: searchField

        rightPadding: 4
        leftPadding: 26
        anchors.fill: parent
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["text"]
        verticalAlignment: Text.AlignVCenter
        placeholderText: qsTr("Search…")
        selectionColor: Cpp_ThemeManager.colors["highlight"]
        selectedTextColor: Cpp_ThemeManager.colors["highlighted_text"]
        placeholderTextColor: Cpp_ThemeManager.colors["placeholder_text"]

        background: Rectangle {
          radius: 4
          border.width: 1
          color: Cpp_ThemeManager.colors["base"]
          border.color: searchField.activeFocus
                        ? Cpp_ThemeManager.colors["highlight"]
                        : Cpp_ThemeManager.colors["window_border"]
        }

        onTextChanged: taskBar.searchFilter = text

        Connections {
          target: taskBar
          function onSearchFilterChanged() {
            if (taskBar.searchFilter === "" && searchField.text !== "")
              searchField.text = ""
          }

          function onSearchDismissed() {
            searchField.text = ""
            searchField.focus = false
            searchPopup.close()
          }

          function onActiveWindowChanged() {
            if (taskBar.activeWindow && searchField.activeFocus)
              taskBar.dismissSearch()
          }
        }

        onActiveFocusChanged: {
          if (!activeFocus)
            searchPopup.close()
        }

        Keys.onEscapePressed: {
          text = ""
          focus = false
        }

        Keys.onDownPressed: {
          if (searchResultsList.count > 0) {
            searchResultsList.currentIndex
              = Math.min(searchResultsList.currentIndex + 1,
                         searchResultsList.count - 1)
          }
        }

        Keys.onUpPressed: {
          if (searchResultsList.count > 0) {
            searchResultsList.currentIndex
              = Math.max(searchResultsList.currentIndex - 1, 0)
          }
        }

        Keys.onEnterPressed: searchField.triggerSelection()
        Keys.onReturnPressed: searchField.triggerSelection()

        function triggerSelection() {
          if (root.combinedSearchResults.length === 0)
            return

          var idx = searchResultsList.currentIndex >= 0
                    ? searchResultsList.currentIndex : 0
          var entry = root.combinedSearchResults[idx]
          root.triggerSearchEntry(entry)
        }

        Button {
          x: 2
          enabled: false
          icon.width: 14
          icon.height: 14
          background: Item {}
          icon.source: "qrc:/icons/buttons/search.svg"
          anchors.verticalCenter: parent.verticalCenter
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }
      }

      Popup {
        id: searchPopup

        width: 320
        padding: 4
        y: -height - searchContainer.y + 1
        visible: searchField.activeFocus
                 && root.combinedSearchResults.length > 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
          border.width: 1
          color: Cpp_ThemeManager.colors["start_menu_background"]
          border.color: Cpp_ThemeManager.colors["start_menu_border"]
        }

        contentItem: ListView {
          id: searchResultsList

          clip: true
          spacing: 2
          model: root.combinedSearchResults
          boundsBehavior: Flickable.StopAtBounds
          implicitHeight: Math.min(contentHeight, 300)

          delegate: Item {
            id: searchDelegate

            required property int index
            required property var modelData

            height: 28
            width: searchResultsList.width

            readonly property bool hovered: _searchMa.containsMouse
                                            || searchResultsList.currentIndex === index

            Rectangle {
              anchors.fill: parent
              visible: searchDelegate.hovered
              color: Cpp_ThemeManager.colors["start_menu_highlight"]
            }

            RowLayout {
              spacing: 8
              anchors.fill: parent
              anchors.leftMargin: 6
              anchors.rightMargin: 6

              Image {
                Layout.preferredWidth: 16
                Layout.preferredHeight: 16
                sourceSize: Qt.size(16, 16)
                source: searchDelegate.modelData["widgetIcon"]
              }

              Label {
                elide: Text.ElideRight
                Layout.fillWidth: true
                text: searchDelegate.modelData["widgetName"]
                font: Cpp_Misc_CommonFonts.uiFont
                color: searchDelegate.hovered
                       ? Cpp_ThemeManager.colors["start_menu_highlighted_text"]
                       : Cpp_ThemeManager.colors["start_menu_text"]
              }

              Label {
                opacity: 0.5
                elide: Text.ElideRight
                Layout.maximumWidth: 120
                text: searchDelegate.modelData["groupName"]
                font: Cpp_Misc_CommonFonts.uiFont
                color: searchDelegate.hovered
                       ? Cpp_ThemeManager.colors["start_menu_highlighted_text"]
                       : Cpp_ThemeManager.colors["start_menu_text"]
              }
            }

            MouseArea {
              id: _searchMa

              hoverEnabled: true
              anchors.fill: parent
              onEntered: searchResultsList.currentIndex = searchDelegate.index
              onClicked: root.triggerSearchEntry(searchDelegate.modelData)
            }
          }
        }
      }
    }

    //
    // Shortcuts
    //
    Widgets.TaskbarButton {
      forceVisible: true
      visible: !app.runtimeMode
      icon.source: "qrc:/icons/taskbar/settings.svg"
      onClicked: {
        app.showSettingsDialog()
        taskBar.activeWindow = null
      }
    } Widgets.TaskbarButton {
      visible: !app.runtimeMode
      forceVisible: !app.runtimeMode
      focused: Cpp_UI_Dashboard.terminalEnabled
      icon.source: "qrc:/icons/taskbar/console.svg"
      onClicked: {
        taskBar.activeWindow = null
        Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled
      }
    } Widgets.TaskbarButton {
      visible: Cpp_CommercialBuild
      forceVisible: Cpp_CommercialBuild
      focused: Cpp_UI_Dashboard.notificationLogEnabled
      icon.source: "qrc:/icons/taskbar/notifications.svg"
      onClicked: {
        taskBar.activeWindow = null
        Cpp_UI_Dashboard.notificationLogEnabled = !Cpp_UI_Dashboard.notificationLogEnabled
      }
    } Widgets.TaskbarButton {
      forceVisible: true
      focused: Cpp_IO_Manager.paused
      icon.source: Cpp_IO_Manager.paused ?
                     "qrc:/icons/taskbar/resume.svg" :
                     "qrc:/icons/taskbar/pause.svg"
      onClicked: {
        taskBar.activeWindow = null
        Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
      }
    }

    //
    // Taskbar Buttons
    //
    Item {
      id: buttonsContainer

      implicitHeight: 24
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter

      property bool showNavButtons: buttonsView.contentWidth > buttonsView.width

      RowLayout {
        spacing: 4
        anchors.fill: parent

        Button {
          icon.width: 24
          icon.height: 24
          background: Item{}
          Layout.preferredWidth: 24
          Layout.preferredHeight: 24
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/icons/buttons/backward.svg"
          icon.color: Cpp_ThemeManager.colors["taskbar_text"]
          onClicked: {
            taskBar.activeWindow = null
            buttonsView.contentX = Math.max(0, buttonsView.contentX - 150)
          }
        }

        ListView {
          id: buttonsView

          clip: true
          spacing: 2
          interactive: true
          Layout.fillWidth: true
          Layout.fillHeight: true
          orientation: ListView.Horizontal
          boundsBehavior: Flickable.StopAtBounds
          model: taskBar ? taskBar.taskbarButtons : null

          delegate: Widgets.TaskbarButton {
            required property var model

            id: button

            text: model.widgetName
            icon.source: SerialStudio.dashboardWidgetIcon(model.widgetType)
            forceVisible: Cpp_UI_Dashboard.showTaskbarButtons
                          || (taskBar && taskBar.hasMaximizedWindow)

            width: opacity > 0 ? 144 : 0
            Behavior on width { NumberAnimation{} }

            Component.onCompleted: updateState()

            onClicked: {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                if (taskBar.windowState(window) !== SS_Ui.TaskbarModel.WindowNormal)
                  taskBar.showWindow(window)

                taskBar.activeWindow = window
              }
            }

            TapHandler {
              acceptedButtons: Qt.RightButton
              onTapped: {
                if (taskBar && taskBar.activeGroupId >= 1000) {
                  _tbRemoveWindowId = button.model.windowId
                  _tbContextMenu.popup()
                }
              }
            }

            function updateState() {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                let state = taskBar.windowState(window)
                button.open      = (state !== SS_Ui.TaskbarModel.WindowClosed)
                button.minimized = (state === SS_Ui.TaskbarModel.WindowMinimized)
                button.focused   = (state === SS_Ui.TaskbarModel.WindowNormal && taskBar.activeWindow === window)
              }
            }

            Connections {
              target: taskBar
              function onActiveWindowChanged() { button.updateState() }
              function onWindowStatesChanged() { button.updateState() }
            }
          }
        }

        Button {
          icon.width: 24
          icon.height: 24
          background: Item{}
          Layout.preferredWidth: 24
          Layout.preferredHeight: 24
          Layout.alignment: Qt.AlignVCenter
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/icons/buttons/forward.svg"
          icon.color: Cpp_ThemeManager.colors["taskbar_text"]
          onClicked: {
            taskBar.activeWindow = null
            buttonsView.contentX = Math.min(buttonsView.contentWidth - buttonsView.width, buttonsView.contentX + 150)
          }
        }
      }
    }

    //
    // Workspace switcher
    //
    ComboBox {
      id: _switcher

      textRole: "text"
      Layout.maximumWidth: 220
      Layout.alignment: Qt.AlignVCenter
      model: taskBar ? taskBar.workspaceModel : []
      currentIndex: taskBar ? Math.max(0, taskBar.activeGroupIndex) : 0
      onActivated: {
        if (taskBar && currentIndex !== taskBar.activeGroupIndex)
          taskBar.activeGroupIndex = currentIndex
      }

      popup.width: 240
      popup.x: _switcher.width - popup.width
      popup.y: -popup.height - _switcher.y + 1

      popup.background: Rectangle {
        border.width: 1
        color: Cpp_ThemeManager.colors["start_menu_background"]
        border.color: Cpp_ThemeManager.colors["start_menu_border"]
      }

      popup.enter: Transition {
        NumberAnimation {
          duration: 150
          from: 0; to: 1
          property: "opacity"
          easing.type: Easing.OutCubic
        }
      }

      popup.exit: Transition {
        NumberAnimation {
          duration: 100
          from: 1; to: 0
          property: "opacity"
          easing.type: Easing.InCubic
        }
      }

      indicator: Item {}

      background: Rectangle {
        border.width: 0
        color: "transparent"
      }

      delegate: ItemDelegate {
        width: _switcher.popup.width

        contentItem: RowLayout {
          spacing: 8

          Image {
            source: modelData["icon"] || ""
            sourceSize: Qt.size(16, 16)
            fillMode: Image.PreserveAspectFit
          }

          Label {
            text: modelData["text"]
            elide: Text.ElideRight
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
            font: text === _switcher.currentText
                  ? Cpp_Misc_CommonFonts.boldUiFont
                  : Cpp_Misc_CommonFonts.uiFont
            color: hovered
                   ? Cpp_ThemeManager.colors["start_menu_highlighted_text"]
                   : Cpp_ThemeManager.colors["start_menu_text"]
          }
        }

        background: Rectangle {
          color: hovered
                 ? Cpp_ThemeManager.colors["start_menu_highlight"]
                 : "transparent"
        }
      }

      contentItem: RowLayout {
        spacing: 4
        anchors.verticalCenter: parent.verticalCenter

        Label {
          Layout.fillWidth: true
          text: _switcher.currentText
          horizontalAlignment: Text.AlignRight
          font: Cpp_Misc_CommonFonts.boldUiFont
          verticalAlignment: Text.AlignVCenter
          color: Cpp_ThemeManager.colors["pane_caption_foreground"]
        }

        Canvas {
          id: _canvas

          width: 18
          height: 18
          opacity: 0.8
          Layout.alignment: Qt.AlignVCenter
          Connections {
            target: Cpp_ThemeManager
            function onThemeChanged() {
              _canvas.requestPaint()
            }
          }

          onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.fillStyle = Cpp_ThemeManager.colors["pane_caption_foreground"];

            const spacing = 2;
            const triangleWidth = 8;
            const triangleHeight = 4;

            const centerX = width / 2;
            const totalHeight = triangleHeight * 2 + spacing;
            const topY = (height - totalHeight) / 2;
            const downTopY = topY + triangleHeight + spacing;

            // Up Triangle
            ctx.beginPath();
            ctx.moveTo(centerX, topY);
            ctx.lineTo(centerX - triangleWidth / 2, topY + triangleHeight);
            ctx.lineTo(centerX + triangleWidth / 2, topY + triangleHeight);
            ctx.closePath();
            ctx.fill();

            // Down Triangle
            ctx.beginPath();
            ctx.moveTo(centerX, downTopY + triangleHeight);
            ctx.lineTo(centerX - triangleWidth / 2, downTopY);
            ctx.lineTo(centerX + triangleWidth / 2, downTopY);
            ctx.closePath();
            ctx.fill();
          }
        }
      }
    }

    //
    // Auto-layout button
    //
    Button {
      icon.width: 24
      icon.height: 24
      background: Item{}
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/icons/buttons/auto-layout.svg"
      icon.color: taskBar.windowManager.autoLayoutEnabled ?
                    Cpp_ThemeManager.colors["taskbar_highlight"] :
                    Cpp_ThemeManager.colors["taskbar_text"]
      onClicked: {
        taskBar.activeWindow = null
        taskBar.windowManager.autoLayoutEnabled = !taskBar.windowManager.autoLayoutEnabled
      }
    }

    //
    // Edit workspace button (only for user workspaces)
    //
    Button {
      icon.width: 16
      icon.height: 16
      background: Item{}
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      opacity: enabled ? 1 : 0.5
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/icons/buttons/wrench.svg"
      enabled: taskBar && taskBar.activeGroupId >= 1000
      icon.color: Cpp_ThemeManager.colors["taskbar_text"]
      onClicked: {
        var model = taskBar.workspaceModel
        for (var i = 0; i < model.length; ++i) {
          if (model[i]["id"] === taskBar.activeGroupId) {
            root.editWorkspaceRequested(taskBar.activeGroupId,
                                        model[i]["text"])
            return
          }
        }
      }
    }

    //
    // New workspace button (hidden in operator runtime mode)
    //
    Button {
      icon.width: 16
      icon.height: 16
      background: Item{}
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/icons/buttons/plus.svg"
      icon.color: Cpp_ThemeManager.colors["taskbar_text"]
      onClicked: root.newWorkspaceRequested()
      visible: !(typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
    }

    Item {
      implicitWidth: 4
    }
  }

  //
  // Taskbar border
  //
  Rectangle {
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    height: 1
    color: Cpp_ThemeManager.colors["taskbar_border"]
  }

  //
  // Taskbar button context menu for removing widgets from user workspaces
  //
  property int _tbRemoveWindowId: -1

  Menu {
    id: _tbContextMenu

    MenuItem {
      text: qsTr("Remove from Workspace")
      onTriggered: taskBar.removeWidgetFromActiveWorkspace(root._tbRemoveWindowId)
    }
  }

}
