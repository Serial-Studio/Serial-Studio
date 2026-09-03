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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  property var paletteModel: null
  property var searchSections: []
  property var flatSearchNodes: []
  property int searchIndex: -1

  //
  // True while a popup or the search field has focus
  //
  readonly property bool isBusy: (searchField && searchField.activeFocus)
                                 || (searchPopup && searchPopup.opened)
                                 || (_tbContextMenu && _tbContextMenu.opened)
                                 || (startMenu && startMenu.visible)

  //
  // Rebuilds the sectioned search list from the shared palette model (parity with the palette).
  //
  function refreshSearch() {
    var secs = paletteModel ? paletteModel.searchSections(taskBar ? taskBar.searchFilter : "") : []
    var flat = []
    for (var s = 0; s < secs.length; ++s)
      for (var k = 0; k < secs[s].items.length; ++k) {
        secs[s].items[k]._idx = flat.length
        flat.push(secs[s].items[k])
      }

    root.searchSections = secs
    root.flatSearchNodes = flat
    root.searchIndex = flat.length > 0 ? 0 : -1
  }

  //
  // Routes activation through the shared model; a folder opens the palette drilled into it.
  //
  function activateSearchNode(node) {
    if (!node || !paletteModel)
      return

    if (node.isFolder) {
      taskBar.dismissSearch()
      root.workspaceSwitcherRequested()
    } else {
      taskBar.dismissSearch()
    }

    paletteModel.activate(node)
  }

  Component.onCompleted: refreshSearch()
  onPaletteModelChanged: refreshSearch()
  Connections {
    target: taskBar
    function onSearchResultsChanged() { refreshSearch() }
    function onSearchFilterChanged()  { refreshSearch() }
  }

  //
  // Signals
  //
  signal startClicked()
  signal settingsClicked()
  signal extendWindowClicked()
  signal workspaceSwitcherRequested()
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
  // Shared styling for the monochrome taskbar buttons: without hover/press feedback they
  // read as static glyphs instead of clickable controls
  //
  component TrayButton: Widgets.IconButton {
    id: _tray

    padding: 4
    iconSize: 16
    Layout.preferredWidth: 24
    Layout.preferredHeight: 24
    Layout.alignment: Qt.AlignVCenter
    icon.color: _tray.enabled && (_tray.highlighted || _tray.hovered)
                ? Cpp_ThemeManager.colors["taskbar_highlight"]
                : Cpp_ThemeManager.colors["taskbar_text"]

    background: Rectangle {
      radius: 3
      border.width: 1
      border.color: Cpp_ThemeManager.colors["taskbar_checked_button_border"]
      opacity: _tray.enabled && (_tray.highlighted || _tray.down) ? 1 : 0

      Behavior on opacity { NumberAnimation { duration: 100 } }

      gradient: Gradient {
        GradientStop {
          position: _tray.down ? 1 : 0
          color: Cpp_ThemeManager.colors["taskbar_checked_button_top"]
        }

        GradientStop {
          position: _tray.down ? 0 : 1
          color: Cpp_ThemeManager.colors["taskbar_checked_button_bottom"]
        }
      }
    }
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

      Widgets.LineField {
        id: searchField

        readonly property bool rtl: Cpp_Misc_Translator.rtl

        anchors.fill: parent
        leftPadding: rtl ? 4 : 26
        rightPadding: rtl ? 26 : 4
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["text"]
        verticalAlignment: Text.AlignVCenter
        // code-verify off
        placeholderText: qsTr("Search… (%1)").arg(Qt.platform.os === "osx" ? "⌘+K" : "Ctrl+K")
        // code-verify on
        selectionColor: Cpp_ThemeManager.colors["highlight"]
        horizontalAlignment: rtl ? Text.AlignRight : Text.AlignLeft
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
          if (root.flatSearchNodes.length > 0)
            root.searchIndex = Math.min(root.searchIndex + 1, root.flatSearchNodes.length - 1)
        }

        Keys.onUpPressed: {
          if (root.flatSearchNodes.length > 0)
            root.searchIndex = Math.max(root.searchIndex - 1, 0)
        }

        Keys.onEnterPressed: searchField.triggerSelection()
        Keys.onReturnPressed: searchField.triggerSelection()

        function triggerSelection() {
          if (root.flatSearchNodes.length === 0)
            return

          var idx = root.searchIndex >= 0 ? root.searchIndex : 0
          root.activateSearchNode(root.flatSearchNodes[idx])
        }

        Widgets.IconButton {
          enabled: false
          iconSize: 14
          background: Item {}
          x: searchField.rtl ? searchField.width - width - 2 : 2
          icon.source: "qrc:/icons/buttons/search.svg"
          anchors.verticalCenter: parent.verticalCenter
        }
      }

      Popup {
        id: searchPopup

        width: 320
        padding: 4
        x: Cpp_Misc_Translator.rtl ? searchContainer.width - width : 0
        y: -height - searchContainer.y + 1
        visible: searchField.activeFocus
                 && root.flatSearchNodes.length > 0
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
          border.width: 1
          color: Cpp_ThemeManager.colors["start_menu_background"]
          border.color: Cpp_ThemeManager.colors["start_menu_border"]

          MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: (wheel) => { wheel.accepted = true }
          }
        }

        contentItem: Flickable {
          id: _searchFlick

          clip: true
          contentWidth: width
          contentHeight: _tbSectionColumn.height
          boundsBehavior: Flickable.StopAtBounds
          implicitHeight: Math.min(contentHeight, 320)

          ScrollBar.vertical: ScrollBar {}

          Column {
            id: _tbSectionColumn

            spacing: 6
            width: _searchFlick.width

            Repeater {
              model: root.searchSections

              delegate: Column {
                id: _tbSection

                spacing: 2
                width: _tbSectionColumn.width

                required property var modelData

                RowLayout {
                  spacing: 6
                  width: parent.width
                  visible: _tbSection.modelData.title.length > 0

                  Label {
                    leftPadding: 6
                    text: _tbSection.modelData.title
                    color: Cpp_ThemeManager.colors["start_menu_text"]
                    font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
                    Component.onCompleted: font.capitalization = Font.AllUppercase
                  }

                  Rectangle {
                    height: 1
                    opacity: 0.4
                    Layout.rightMargin: 6
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    color: Cpp_ThemeManager.colors["start_menu_border"]
                  }
                }

                Repeater {
                  model: _tbSection.modelData.items

                  delegate: Item {
                    id: _tbRow

                    height: 28
                    width: _tbSection.width

                    required property var modelData

                    readonly property bool hovered: _tbRowMa.containsMouse
                                                    || root.searchIndex === _tbRow.modelData._idx

                    Rectangle {
                      anchors.fill: parent
                      visible: _tbRow.hovered
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
                        source: _tbRow.modelData.iconId && _tbRow.modelData.iconId.length
                                ? Cpp_Misc_IconRegistry.iconById(_tbRow.modelData.iconId, 16)
                                : _tbRow.modelData.icon
                      }

                      Label {
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        text: _tbRow.modelData.text
                        font: Cpp_Misc_CommonFonts.uiFont
                        LayoutMirroring.enabled: false
                        horizontalAlignment: Cpp_Misc_Translator.rtl ? Text.AlignRight
                                                                     : Text.AlignLeft
                        color: _tbRow.hovered
                               ? Cpp_ThemeManager.colors["start_menu_highlighted_text"]
                               : Cpp_ThemeManager.colors["start_menu_text"]
                      }

                      Label {
                        opacity: 0.5
                        elide: Text.ElideRight
                        Layout.maximumWidth: 120
                        LayoutMirroring.enabled: false
                        font: Cpp_Misc_CommonFonts.uiFont
                        text: _tbRow.modelData.subtitle !== undefined
                              ? _tbRow.modelData.subtitle : ""
                        visible: _tbRow.modelData.subtitle !== undefined
                                 && _tbRow.modelData.subtitle.length > 0
                        horizontalAlignment: Cpp_Misc_Translator.rtl ? Text.AlignRight
                                                                     : Text.AlignLeft
                        color: _tbRow.hovered
                               ? Cpp_ThemeManager.colors["start_menu_highlighted_text"]
                               : Cpp_ThemeManager.colors["start_menu_text"]
                      }
                    }

                    MouseArea {
                      id: _tbRowMa

                      hoverEnabled: true
                      anchors.fill: parent
                      onEntered: root.searchIndex = _tbRow.modelData._idx
                      onClicked: root.activateSearchNode(_tbRow.modelData)
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
    // Pinned shortcut buttons: registry-driven, drag-reorderable
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
        case "clock":             return true
        case "stopwatch":         return true
        case "pause":             return true
        case "file_transmission": return Cpp_CommercialBuild
                                         && (!app.runtimeMode
                                             || Cpp_IO_FileTransmission.runtimeAccessAllowed)
        case "ai_assistant":      return Cpp_CommercialBuild && !app.runtimeMode
        }
        return false
      }

      //
      // Filtered + ordered model: pinned IDs whose registry entry is visible.
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
            case "clock":             return qsTr("Clock")
            case "stopwatch":         return qsTr("Stopwatch")
            case "file_transmission": return qsTr("File Transmission")
            case "ai_assistant":      return qsTr("AI Assistant")
            case "pause":             return Cpp_IO_Manager.paused ? qsTr("Resume") : qsTr("Pause")
            }
            return ""
          }

          icon.source: {
            switch (modelData) {
            case "settings":          return Cpp_Misc_IconRegistry.icon("commands", "settings", 24)
            case "console":           return Cpp_Misc_IconRegistry.icon("commands", "console", 24)
            case "notifications":     return Cpp_Misc_IconRegistry.icon(
                                             "widgets", "notification-log", 24)
            case "clock":             return Cpp_Misc_IconRegistry.icon("widgets", "clock", 24)
            case "stopwatch":         return Cpp_Misc_IconRegistry.icon("widgets", "stopwatch", 24)
            case "file_transmission": return Cpp_Misc_IconRegistry.icon(
                                             "commands", "file-transmission", 24)
            case "ai_assistant":      return Cpp_Misc_IconRegistry.icon("commands", "ai", 24)
            case "pause":             return Cpp_IO_Manager.paused
                                             ? Cpp_Misc_IconRegistry.icon("commands", "resume", 24)
                                             : Cpp_Misc_IconRegistry.icon("commands", "pause", 24)
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
            case "clock":         return Cpp_UI_Dashboard.clockEnabled
            case "stopwatch":     return Cpp_UI_Dashboard.stopwatchEnabled
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
            case "clock":
              taskBar.activeWindow = null
              Cpp_UI_Dashboard.clockEnabled = !Cpp_UI_Dashboard.clockEnabled
              break
            case "stopwatch":
              taskBar.activeWindow = null
              Cpp_UI_Dashboard.stopwatchEnabled = !Cpp_UI_Dashboard.stopwatchEnabled
              break
            case "pause":
              taskBar.activeWindow = null
              Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused
              break
            case "file_transmission":
              taskBar.activeWindow = null
              app.showFileTransmission()
              break
            case "ai_assistant":
              taskBar.activeWindow = null
              app.showAIAssistant()
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

        TrayButton {
          iconSize: 12
          mirrorIconInRtl: true
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/icons/buttons/backward.svg"
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
            id: button

            required property var model

            text: model.widgetName
            forceHidden: Cpp_UI_Dashboard.frozen
            icon.source: SerialStudio.dashboardWidgetIcon(model.widgetType)
            forceVisible: Cpp_UI_TaskbarSettings.showTaskbarButtons
                          || (taskBar && taskBar.hasMaximizedWindow)

            width: opacity > 0 ? 144 : 0
            Behavior on width { NumberAnimation{} }

            Component.onCompleted: updateState()

            onClicked: {
              const window = taskBar.windowData(model.windowId)
              if (window !== null) {
                if (taskBar.windowState(window) !== SS_Ui.TaskbarModel.WindowNormal) {
                  if (Cpp_UI_Dashboard.frozen)
                    return

                  taskBar.showWindow(window)
                }

                taskBar.activeWindow = window
              }
            }

            TapHandler {
              acceptedButtons: Qt.RightButton
              onTapped: {
                if (taskBar && taskBar.activeGroupId >= 1000 && !Cpp_UI_Dashboard.frozen
                    && !app.runtimeMode) {
                  root.tbRemoveWindowId = button.model.windowId
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

        TrayButton {
          iconSize: 12
          mirrorIconInRtl: true
          visible: buttonsContainer.showNavButtons
          icon.source: "qrc:/icons/buttons/forward.svg"
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
    Item {
      id: _switcher

      implicitHeight: 24
      Layout.maximumWidth: 220
      Layout.alignment: Qt.AlignVCenter
      implicitWidth: _switcherLayout.implicitWidth

      readonly property string activeTitle: {
        if (!taskBar)
          return ""

        const m = taskBar.workspaceModel
        for (let i = 0; i < m.length; ++i)
          if (m[i].id === taskBar.activeGroupId)
            return m[i].text

        return m.length > 0 ? m[0].text : ""
      }

      MouseArea {
        anchors.fill: parent
        onPressed: root.workspaceSwitcherRequested()
      }

      RowLayout {
        id: _switcherLayout

        spacing: 4
        anchors.fill: parent

        Label {
          Layout.fillWidth: true
          text: _switcher.activeTitle
          elide: Text.ElideRight
          LayoutMirroring.enabled: false
          horizontalAlignment: Cpp_Misc_Translator.rtl ? Text.AlignLeft
                                                       : Text.AlignRight
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

            ctx.beginPath();
            ctx.moveTo(centerX, topY);
            ctx.lineTo(centerX - triangleWidth / 2, topY + triangleHeight);
            ctx.lineTo(centerX + triangleWidth / 2, topY + triangleHeight);
            ctx.closePath();
            ctx.fill();

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
    // Auto-layout button: opens the layout gallery, which is where the pattern, the split
    // ratio and manual placement all live (spec 0053)
    //
    TrayButton {
      id: _layoutButton

      enabled: !Cpp_UI_Dashboard.frozen
      icon.source: "qrc:/icons/buttons/auto-layout.svg"
      highlighted: taskBar.windowManager.autoLayoutEnabled
      visible: !(app.runtimeMode && Cpp_UI_Dashboard.frozen)
      onClicked: {
        taskBar.activeWindow = null
        _layoutPicker.open()
      }

      //
      // Right-aligned to the button, since it sits near the window edge; Popup.margins is what
      // keeps the final position inside the window
      //
      LayoutPatternPicker {
        id: _layoutPicker

        y: -height - 6
        windowManager: taskBar.windowManager
        x: Cpp_Misc_Translator.rtl ? 0 : _layoutButton.width - width
      }
    }

    //
    // Passive parse-thinning indicator: visible while the fair-share governor decimates any
    // source; details live in the problem center (spec 0051)
    //
    Rectangle {
      id: _thinningBadge

      radius: 5
      Layout.preferredWidth: 10
      Layout.preferredHeight: 10
      visible: Cpp_UI_Dashboard.thinningActive
      color: Cpp_ThemeManager.colors["alarm"]
      Layout.alignment: Qt.AlignVCenter

      ToolTip.delay: 700
      ToolTip.visible: _thinningHover.hovered
      ToolTip.text: qsTr("Data is arriving faster than scripts can process it; some frames " +
                         "are being thinned. See the problem center for details.")

      HoverHandler {
        id: _thinningHover
      }
    }

    //
    // Freeze dashboard button + passive frozen indicator (Pro)
    //
    TrayButton {
      id: _freezeButton

      readonly property bool freezeAllowed: Cpp_CommercialBuild
                                            && (Cpp_Licensing_LemonSqueezy.isActivated
                                                || Cpp_Licensing_Trial.trialEnabled)

      opacity: freezeAllowed ? 1 : 0.5
      highlighted: Cpp_UI_Dashboard.frozen
      icon.source: "qrc:/icons/buttons/freeze.svg"
      visible: Cpp_AppState.operationMode === SerialStudio.ProjectFile && !app.runtimeMode
      ToolTip.delay: 700
      ToolTip.visible: hovered
      ToolTip.text: Cpp_UI_Dashboard.frozen ? qsTr("Unfreeze Dashboard")
                                            : qsTr("Freeze Dashboard")
      onClicked: {
        if (freezeAllowed)
          Cpp_UI_Dashboard.setFrozen(!Cpp_UI_Dashboard.frozen)
        else
          app.showLicenseDialog()
      }
    }

    //
    // Edit workspace button (only for user workspaces)
    //
    TrayButton {
      visible: !app.runtimeMode
      enabled: taskBar && taskBar.activeGroupId >= 1000
      icon.source: "qrc:/icons/buttons/workspace-settings.svg"
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
    // MQTT status indicator (visible only when publisher enabled on a Pro build)
    //
    Loader {
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      active: Cpp_CommercialBuild
      Layout.alignment: Qt.AlignVCenter
      visible: active && Cpp_MQTT_Publisher.enabled
      sourceComponent: Component {
        TrayButton {
          id: mqttIndicator

          anchors.fill: parent
          icon.color: "transparent"
          opacity: Cpp_MQTT_Publisher.isConnected ? 1 : 0.5
          icon.source: Cpp_MQTT_Publisher.isConnected
                       ? "qrc:/icons/buttons/mqtt-on.svg"
                       : "qrc:/icons/buttons/mqtt-off.svg"
          ToolTip.visible: hovered && !mqttStatusPopup.opened
          ToolTip.text: Cpp_MQTT_Publisher.isConnected
                        ? qsTr("MQTT: Connected to %1").arg(Cpp_MQTT_Publisher.brokerEndpoint)
                        : qsTr("MQTT: Not connected")
          onClicked: mqttStatusPopup.opened ? mqttStatusPopup.close() : mqttStatusPopup.open()

          Popup {
            id: mqttStatusPopup

            width: 280
            padding: 10
            y: -implicitHeight - mqttIndicator.y + 1
            x: Cpp_Misc_Translator.rtl ? 0 : mqttIndicator.width - width
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

            background: Rectangle {
              border.width: 1
              color: Cpp_ThemeManager.colors["start_menu_background"]
              border.color: Cpp_ThemeManager.colors["start_menu_border"]
            }

            contentItem: ColumnLayout {
              spacing: 6

              Label {
                font: Cpp_Misc_CommonFonts.boldUiFont
                color: Cpp_ThemeManager.colors["start_menu_text"]
                text: qsTr("MQTT Publisher")
              }

              GridLayout {
                columns: 2
                rowSpacing: 4
                columnSpacing: 12
                Layout.fillWidth: true

                Label {
                  text: qsTr("Status:")
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }
                Label {
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  text: Cpp_MQTT_Publisher.isConnected
                        ? qsTr("Connected")
                        : qsTr("Disconnected")
                  color: Cpp_MQTT_Publisher.isConnected
                         ? Cpp_ThemeManager.colors["highlight"]
                         : Cpp_ThemeManager.colors["alarm"]
                }

                Label {
                  text: qsTr("Broker:")
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }
                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: Cpp_MQTT_Publisher.brokerEndpoint
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }

                Label {
                  text: qsTr("Mode:")
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }
                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: Cpp_MQTT_Publisher.modeLabel
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }

                Label {
                  text: qsTr("Messages sent:")
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }
                Label {
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.uiFont
                  text: Cpp_MQTT_Publisher.messagesSent
                  color: Cpp_ThemeManager.colors["start_menu_text"]
                }
              }

              Button {
                Layout.fillWidth: true
                text: qsTr("Open MQTT Settings")
                font: Cpp_Misc_CommonFonts.uiFont
                visible: !(typeof CLI_RUNTIME_MODE !== "undefined" && CLI_RUNTIME_MODE === true)
                onClicked: {
                  mqttStatusPopup.close()
                  app.showProjectEditor()
                  Cpp_JSON_ProjectEditor.selectMqttPublisher()
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
  property int tbRemoveWindowId: -1

  Menu {
    id: _tbContextMenu

    MenuItem {
      text: qsTr("Remove from Workspace")
      onTriggered: taskBar.removeWidgetFromActiveWorkspace(root.tbRemoveWindowId)
    }
  }
}
