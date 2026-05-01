/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
  // True while a popup or the search field has focus
  //
  readonly property bool isBusy: (searchField && searchField.activeFocus)
                                 || (searchPopup && searchPopup.opened)
                                 || (_switcher && _switcher.popup && _switcher.popup.opened)
                                 || (_tbContextMenu && _tbContextMenu.opened)
                                 || (startMenu && startMenu.visible)

  //
  // Rebuilds the unified search list -- widget hits first, then start menu
  // actions -- whenever the filter or widget index changes.
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

  //
  // Snapshot entry fields before dismissSearch() rebuilds the model.
  //
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
    if (!Cpp_UI_TaskbarSettings.searchEnabled)
      return

    searchField.forceActiveFocus(Qt.MouseFocusReason)
    searchPopup.open()
  }

  //
  // Trigger the Start menu (called externally via shortcut)
  //
  function toggleStartMenu() {
    root.startClicked()
    taskBar.activeWindow = null
  }

  //
  // Cycle workspaces (called externally via shortcut). delta = +1 / -1.
  //
  function cycleWorkspace(delta) {
    if (!taskBar)
      return

    const list = taskBar.workspaceModel
    if (!list || list.length === 0)
      return

    const cur = Math.max(0, taskBar.activeGroupIndex)
    const next = ((cur + delta) % list.length + list.length) % list.length
    if (next !== cur)
      taskBar.activeGroupIndex = next
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
      Layout.alignment: Qt.AlignVCenter
      Layout.preferredWidth: visible ? 172 : 0
      visible: Cpp_UI_TaskbarSettings.searchEnabled

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
    // Pinned shortcut buttons -- registry-driven, drag-reorderable
    //
    Row {
      id: pinRow

      spacing: 2
      Layout.alignment: Qt.AlignVCenter
      Layout.preferredHeight: 24
      Layout.preferredWidth: displayedPins.length === 0
                             ? 0
                             : displayedPins.length * pinSlotWidth
                               + (displayedPins.length - 1) * spacing

      //
      // True when the given pin id should appear in the current build/mode
      //
      function pinVisible(id) {
        switch (id) {
        case "settings":          return !app.runtimeMode
        case "console":           return !app.runtimeMode
        case "notifications":     return Cpp_CommercialBuild
        case "pause":             return true
        case "file_transmission": return Cpp_CommercialBuild
                                         && (!app.runtimeMode
                                             || Cpp_IO_FileTransmission.runtimeAccessAllowed)
        }
        return false
      }

      //
      // Filtered + ordered model -- pinned IDs whose registry entry is visible.
      //
      readonly property var displayedPins: {
        // Touch the reactive inputs so this binding re-evaluates on change.
        void Cpp_UI_TaskbarSettings.pinnedButtons
        void app.runtimeMode
        const out = []
        const ids = Cpp_UI_TaskbarSettings.pinnedButtons
        for (let i = 0; i < ids.length; ++i) {
          if (pinVisible(ids[i]))
            out.push(ids[i])
        }
        return out
      }

      //
      // Per-pin slot width (kept in sync with TaskbarButton.implicitWidth)
      //
      readonly property int pinSlotWidth: 24

      Repeater {
        model: pinRow.displayedPins

        delegate: Widgets.TaskbarButton {
          id: pinButton

          required property string modelData
          property real dragOffset: 0
          property bool isDragging: false

          height: 24
          forceVisible: true
          z: isDragging ? 10 : 0
          width: pinRow.pinSlotWidth
          draggable: !app.runtimeMode
          opacity: isDragging ? 0.7 : 1.0

          //
          // Visual offset applied while the user is reordering this pin.
          //
          transform: Translate {
            x: pinButton.dragOffset
            Behavior on x {
              enabled: !pinButton.isDragging
              NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
            }
          }

          ToolTip.text: {
            switch (modelData) {
            case "settings":          return qsTr("Settings")
            case "console":           return qsTr("Console")
            case "notifications":     return qsTr("Notifications")
            case "pause":             return Cpp_IO_Manager.paused ? qsTr("Resume") : qsTr("Pause")
            case "file_transmission": return qsTr("File Transmission")
            }
            return ""
          }

          icon.source: {
            switch (modelData) {
            case "settings":          return "qrc:/icons/taskbar/settings.svg"
            case "console":           return "qrc:/icons/taskbar/console.svg"
            case "notifications":     return "qrc:/icons/taskbar/notifications.svg"
            case "pause":             return Cpp_IO_Manager.paused
                                             ? "qrc:/icons/taskbar/resume.svg"
                                             : "qrc:/icons/taskbar/pause.svg"
            case "file_transmission": return "qrc:/icons/taskbar/file-transmission.svg"
            }
            return ""
          }

          //
          // Stateful pins light up when their backing state is active
          //
          focused: {
            switch (modelData) {
            case "console":       return Cpp_UI_Dashboard.terminalEnabled
            case "notifications": return Cpp_UI_Dashboard.notificationLogEnabled
            case "pause":         return Cpp_IO_Manager.paused
            }
            return false
          }

          onClicked: {
            switch (modelData) {
            case "settings":
              app.showSettingsDialog()
              taskBar.activeWindow = null
              break
            case "console":
              taskBar.activeWindow = null
              Cpp_UI_Dashboard.terminalEnabled = !Cpp_UI_Dashboard.terminalEnabled
              break
            case "notifications":
              taskBar.activeWindow = null
              Cpp_UI_Dashboard.notificationLogEnabled = !Cpp_UI_Dashboard.notificationLogEnabled
              break
            case "pause":
              taskBar.activeWindow = null
              Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
              break
            case "file_transmission":
              taskBar.activeWindow = null
              app.showFileTransmission()
              break
            }
          }

          onDragStarted: isDragging = true

          onDragMoved: (dx) => { dragOffset = dx }

          onDragEnded: {
            const slotW = pinRow.pinSlotWidth + pinRow.spacing
            const delta = Math.round(dragOffset / slotW)
            dragOffset = 0
            isDragging = false

            if (delta !== 0) {
              const ids = pinRow.displayedPins
              const fromIdx = ids.indexOf(modelData)
              const targetIdx = Math.max(0, Math.min(ids.length - 1, fromIdx + delta))
              if (fromIdx !== targetIdx) {
                //
                // Translate the visible-only delta into the full pinnedButtons index
                //
                const neighborId = ids[targetIdx]
                const fullIds = Cpp_UI_TaskbarSettings.pinnedButtons.slice()
                const fullTarget = fullIds.indexOf(neighborId)
                Cpp_UI_TaskbarSettings.movePinnedButton(modelData, fullTarget)
              }
            }
          }
        }
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
            forceVisible: Cpp_UI_TaskbarSettings.showTaskbarButtons
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
