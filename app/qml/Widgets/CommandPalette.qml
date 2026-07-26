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

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.impl

import SerialStudio

import "." as Widgets

FocusScope {
  id: root

  z: 5000
  focus: opened
  anchors.fill: parent
  opacity: opened ? 1 : 0
  visible: opened || opacity > 0
  onVisibleChanged: if (visible) Qt.callLater(_search.forceActiveFocus)

  //
  // Self-heal: reclaim focus if a dashboard grabber yanks it while the palette is open
  //
  onActiveFocusChanged: if (opened && !activeFocus) Qt.callLater(_search.forceActiveFocus)

  //
  // Logical open state, kept distinct from visibility so the fade/scale can play out on close.
  //
  property bool opened: false

  //
  // Host veto on opening (e.g. the Project Editor while locked or in the wrong operation mode).
  //
  property bool openable: true

  //
  // Publishes open state twice: the app-wide flag feeds dashboard focus-grabbers, the
  // window-local SmartWindow flag gates that window's shortcuts without cross-window bleed.
  //
  onOpenedChanged: {
    app.commandPaletteOpen = opened
    const w = root.Window.window
    if (w && w.paletteOpen !== undefined)
      w.paletteOpen = opened
  }

  //
  // External dashboard windows destroy an open palette wholesale (native close button);
  // without this the flags stay stuck true and every shortcut in the app goes dead.
  //
  Component.onDestruction: {
    if (!root.opened)
      return

    app.commandPaletteOpen = false
    const w = root.Window.window
    if (w && w.paletteOpen !== undefined)
      w.paletteOpen = false
  }

  //
  // A live model swap (dashboard shown/hidden) would orphan sections built from the old model;
  // fold the palette instead of rendering stale entries.
  //
  onModelChanged: root.opened = false

  //
  // Open/close fade; the dialog additionally scales in for a subtle pop.
  //
  Behavior on opacity {
    NumberAnimation {
      duration: 150
      easing.type: Easing.OutCubic
    }
  }

  //
  // The palette is entirely driven by an injected controller; it holds no context assumptions.
  //
  required property var model

  //
  // Context-provided chrome (dashboard = Workspaces, main window = Tools, ...).
  //
  property string title: qsTr("Commands")
  property string titleIcon: "qrc:/icons/buttons/utilities.svg"

  //
  // Breadcrumb of folder levels; each frame is { text, nodes, folderId }. folderId -1 is the root.
  //
  property var levelStack: []

  //
  // Ordered { title, items } sections for rendering, plus a flat list for keyboard navigation.
  //
  property var sections: []
  property var displayNodes: []
  property int currentIndex: -1

  //
  // True only while arrow keys drive the highlight; hover must never auto-scroll, or a row
  // sliding under the resting cursor re-fires onEntered and walks the list to the end.
  //
  property bool keyboardNav: false

  readonly property var currentNodes:
      levelStack.length > 0 ? levelStack[levelStack.length - 1].nodes : []

  readonly property int currentFolderId:
      levelStack.length > 0 ? levelStack[levelStack.length - 1].folderId : -1

  readonly property bool searching: _search.text.trim().length > 0
  readonly property int columns: Math.max(1, Math.floor((panel.width - 64) / 152))

  //
  // Header/hint collapse thresholds for small windows (R8).
  //
  readonly property bool compact: panel.width < 460

  function open() {
    if (!root.openable)
      return

    _search.text = ""
    const tree = root.model ? root.model.workspaceTree() : []
    root.levelStack = [{ text: root.title, nodes: tree, folderId: -1 }]
    root.recompute()
    root.opened = true
    _search.forceActiveFocus()
  }

  function close() {
    if (root.model && typeof root.model.dismiss === "function")
      root.model.dismiss()

    root.opened = false
  }

  function toggle() {
    if (root.opened)
      root.close()
    else
      root.open()
  }

  //
  // Rebuilds the sections and the flat navigation list from the model: browse for an empty query,
  // categorized search results otherwise.
  //
  function recompute() {
    if (!root.model) {
      root.sections = []
      root.displayNodes = []
      root.currentIndex = -1
      return
    }

    const query = _search.text.trim()
    let secs = query.length > 0
               ? root.model.searchSections(query)
               : root.model.browseSections(root.currentNodes, root.currentFolderId)

    let flat = []
    for (let s = 0; s < secs.length; ++s)
      for (let k = 0; k < secs[s].items.length; ++k) {
        secs[s].items[k]._idx = flat.length
        flat.push(secs[s].items[k])
      }

    root.sections = secs
    root.displayNodes = flat
    root.currentIndex = -1
  }

  //
  // Slide the result area in from the given direction: +1 on drill-in, -1 on go-up.
  //
  function slide(direction) {
    _slide.x = direction * panel.width
    _slideAnim.restart()
  }

  function enterFolder(node) {
    root.levelStack = root.levelStack.concat(
                        [{ text: node.text, nodes: node.children, folderId: node.id }])
    _search.text = ""
    root.recompute()
    root.slide(1)
  }

  function goUp() {
    if (root.levelStack.length <= 1) {
      root.close()
      return
    }

    root.levelStack = root.levelStack.slice(0, root.levelStack.length - 1)
    _search.text = ""
    root.recompute()
    root.slide(-1)
  }

  //
  // Jump to a shallower breadcrumb level; ignores the current level and out-of-range indices.
  //
  function goToLevel(level) {
    if (level < 0 || level >= root.levelStack.length - 1)
      return

    root.levelStack = root.levelStack.slice(0, level + 1)
    _search.text = ""
    root.recompute()
    root.slide(-1)
  }

  function activate(node) {
    if (!node)
      return

    root.model.activate(node)
    root.currentIndex = -1
  }

  function activateCurrent() {
    const idx = root.currentIndex >= 0 ? root.currentIndex : 0
    if (idx < 0 || idx >= root.displayNodes.length)
      return

    root.activate(root.displayNodes[idx])
  }

  function move(delta) {
    root.keyboardNav = true
    const count = root.displayNodes.length
    if (count === 0)
      return

    if (root.currentIndex < 0) {
      root.currentIndex = 0
      return
    }

    root.currentIndex = Math.max(0, Math.min(count - 1, root.currentIndex + delta))
  }

  //
  // Keeps the highlighted delegate inside the scroll viewport; without this, keyboard
  // navigation walks the highlight off-screen and Enter activates an invisible row.
  //
  function ensureVisible(item) {
    if (!item || _scroll.contentHeight <= _scroll.height)
      return

    const y = item.mapToItem(_sectionColumn, 0, 0).y
    const bottom = y + item.height
    if (y < _scroll.contentY) {
      _scroll.contentY = Math.max(0, y - 8)
    }

    else if (bottom > _scroll.contentY + _scroll.height) {
      const maxY = _scroll.contentHeight - _scroll.height
      _scroll.contentY = Math.min(maxY, bottom - _scroll.height + 8)
    }
  }

  //
  // The model raises folder drill-in and close as navigation outcomes of activation.
  //
  Connections {
    target: root.model
    function onEnterFolderRequested(node) { root.enterFolder(node) }
    function onCloseRequested() { root.close() }
  }

  //
  // Focus-independent close: Escape or Cmd+W / Ctrl+W, no matter which child holds focus.
  //
  Shortcut {
    enabled: root.opened
    onActivated: root.close()
    sequences: ["Escape", StandardKey.Close]
  }

  //
  // Modal catcher: swallows every pointer event (all buttons, hover and wheel) so the scene behind
  // the palette is inert without any dimming tint, while a press outside the dialog dismisses it.
  //
  MouseArea {
    hoverEnabled: true
    enabled: root.opened
    anchors.fill: parent
    onClicked: root.close()
    acceptedButtons: Qt.AllButtons
    onWheel: (wheel) => { wheel.accepted = true }
  }

  //
  // Drop shadow cast by the dialog.
  //
  RectangularShadow {
    blur: 28
    spread: 2
    anchors.fill: panel
    radius: panel.radius
    color: Qt.rgba(0, 0, 0, 0.35)
  }

  //
  // One browse grid cell (workspace, folder, tool, or the add-workspace action).
  //
  Component {
    id: _cellComponent

    Item {
      id: _cell

      width: 152
      height: 104

      required property var modelData

      readonly property bool hovered: _cellMouse.containsMouse
      readonly property bool itemEnabled: _cell.modelData.enabled !== false
      readonly property bool highlighted: _cell.modelData._idx === root.currentIndex

      onHighlightedChanged: {
        if (highlighted && root.keyboardNav)
          root.ensureVisible(_cell)
      }

      Rectangle {
        radius: 6
        border.width: 1
        anchors.fill: parent
        anchors.margins: 6
        opacity: _cell.itemEnabled ? 1.0 : 0.4
        color: _cell.highlighted ? Cpp_ThemeManager.colors["highlight"]
                                 : Cpp_ThemeManager.colors["groupbox_background"]
        border.color: _cell.highlighted || _cell.hovered
                      ? Cpp_ThemeManager.colors["highlight"]
                      : Cpp_ThemeManager.colors["groupbox_border"]

        Rectangle {
          opacity: 0.10
          anchors.fill: parent
          radius: parent.radius
          color: Cpp_ThemeManager.colors["highlight"]
          visible: _cell.hovered && !_cell.highlighted
        }

        ColumnLayout {
          spacing: 8
          anchors.centerIn: parent
          width: parent.width - 16

          IconImage {
            width: 32
            height: 32
            opacity: 0.9
            sourceSize: Qt.size(32, 32)
            source: _cell.modelData.iconId && _cell.modelData.iconId.length
                    ? Cpp_Misc_IconRegistry.iconById(_cell.modelData.iconId, 32)
                    : _cell.modelData.icon
            Layout.alignment: Qt.AlignHCenter
            color: _cell.modelData.isAdd === true
                   ? (_cell.highlighted ? Cpp_ThemeManager.colors["highlighted_text"]
                                        : Cpp_ThemeManager.colors["button_text"])
                   : "transparent"
          }

          Label {
            elide: Text.ElideRight
            Layout.fillWidth: true
            text: _cell.modelData.text
            font: Cpp_Misc_CommonFonts.uiFont
            horizontalAlignment: Text.AlignHCenter
            color: _cell.highlighted ? Cpp_ThemeManager.colors["highlighted_text"]
                                     : Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.7
            text: qsTr("Folder")
            Layout.alignment: Qt.AlignHCenter
            visible: _cell.modelData.isFolder === true
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
            color: _cell.highlighted ? Cpp_ThemeManager.colors["highlighted_text"]
                                     : Cpp_ThemeManager.colors["text"]
          }
        }

        MouseArea {
          id: _cellMouse

          hoverEnabled: true
          anchors.fill: parent
          enabled: _cell.itemEnabled
          cursorShape: _cell.itemEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
          onClicked: {
            root.currentIndex = _cell.modelData._idx
            root.activate(_cell.modelData)
          }
        }
      }
    }
  }

  //
  // One dense search row: icon + elided primary name + elided reduced-opacity path subtitle.
  //
  Component {
    id: _rowComponent

    Item {
      id: _row

      height: 34
      width: parent ? parent.width : 0

      required property var modelData

      readonly property bool hovered: _rowMouse.containsMouse
      readonly property bool itemEnabled: _row.modelData.enabled !== false
      readonly property bool highlighted: _row.modelData._idx === root.currentIndex

      onHighlightedChanged: {
        if (highlighted && root.keyboardNav)
          root.ensureVisible(_row)
      }

      Rectangle {
        radius: 4
        anchors.fill: parent
        anchors.rightMargin: 2
        opacity: _row.itemEnabled ? 1.0 : 0.4
        color: _row.highlighted ? Cpp_ThemeManager.colors["highlight"]
                                : (_row.hovered ? Cpp_ThemeManager.colors["groupbox_background"]
                                                : "transparent")

        RowLayout {
          spacing: 8
          anchors.fill: parent
          anchors.leftMargin: 8
          anchors.rightMargin: 8

          IconImage {
            width: 18
            height: 18
            opacity: 0.9
            color: "transparent"
            sourceSize: Qt.size(18, 18)
            source: _row.modelData.iconId && _row.modelData.iconId.length
                    ? Cpp_Misc_IconRegistry.iconById(_row.modelData.iconId, 18)
                    : _row.modelData.icon
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            elide: Text.ElideRight
            Layout.fillWidth: true
            text: _row.modelData.text
            font: Cpp_Misc_CommonFonts.uiFont
            verticalAlignment: Text.AlignVCenter
            color: _row.highlighted ? Cpp_ThemeManager.colors["highlighted_text"]
                                    : Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.7
            elide: Text.ElideRight
            Layout.maximumWidth: Math.min(240, _row.width * 0.5)
            verticalAlignment: Text.AlignVCenter
            text: _row.modelData.subtitle !== undefined ? _row.modelData.subtitle : ""
            visible: _row.modelData.subtitle !== undefined && _row.modelData.subtitle.length > 0
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
            color: _row.highlighted ? Cpp_ThemeManager.colors["highlighted_text"]
                                    : Cpp_ThemeManager.colors["text"]
          }

          Label {
            opacity: 0.55
            verticalAlignment: Text.AlignVCenter
            text: _row.modelData.shortcut !== undefined ? _row.modelData.shortcut : ""
            visible: _row.modelData.shortcut !== undefined && _row.modelData.shortcut.length > 0
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
            color: Cpp_ThemeManager.colors["text"]
          }
        }

        MouseArea {
          id: _rowMouse

          hoverEnabled: true
          anchors.fill: parent
          enabled: _row.itemEnabled
          cursorShape: _row.itemEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
          onEntered: {
            root.keyboardNav = false
            root.currentIndex = _row.modelData._idx
          }
          onClicked: {
            root.currentIndex = _row.modelData._idx
            root.activate(_row.modelData)
          }
        }
      }
    }
  }

  //
  // Palette dialog
  //
  Rectangle {
    id: panel

    clip: true
    radius: 12
    border.width: 1
    anchors.centerIn: parent
    scale: root.opened ? 1 : 0.96
    width: Math.min(680, root.width - 32)
    height: Math.min(480, root.height - 32)
    color: Cpp_ThemeManager.colors["pane_background"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]

    //
    // Scale-in accompanies the fade for a subtle pop on open.
    //
    Behavior on scale {
      NumberAnimation {
        duration: 150
        easing.type: Easing.OutCubic
      }
    }

    MouseArea {
      enabled: root.opened
      anchors.fill: parent
      acceptedButtons: Qt.LeftButton | Qt.RightButton
      onWheel: (wheel) => { wheel.accepted = true }
    }

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Header
      //
      Rectangle {
        z: 999
        border.width: 1
        Layout.fillWidth: true
        Layout.preferredHeight: 56
        topLeftRadius: panel.radius
        topRightRadius: panel.radius
        border.color: Cpp_ThemeManager.colors["pane_caption_border"]

        gradient: Gradient {
          GradientStop {
            position: 0
            color: Cpp_ThemeManager.colors["toolbar_top"]
          }

          GradientStop {
            position: 1
            color: Cpp_ThemeManager.colors["toolbar_bottom"]
          }
        }

        IconImage {
          id: _headerIcon

          anchors.leftMargin: 12
          anchors.left: parent.left
          source: root.titleIcon
          sourceSize: Qt.size(18, 18)
          visible: !root.compact
          anchors.verticalCenter: parent.verticalCenter
          color: Cpp_ThemeManager.colors["toolbar_text"]
        }

        Label {
          id: _headerTitle

          anchors.leftMargin: 8
          text: root.title
          visible: !root.compact
          anchors.left: _headerIcon.right
          verticalAlignment: Text.AlignVCenter
          font: Cpp_Misc_CommonFonts.boldUiFont
          anchors.verticalCenter: parent.verticalCenter
          color: Cpp_ThemeManager.colors["toolbar_text"]
        }

        Widgets.SearchField {
          id: _search

          anchors.centerIn: parent
          onAccepted: root.activateCurrent()
          onTextChanged: root.recompute()
          placeholderText: qsTr("Search…")
          Keys.onLeftPressed: root.move(-1)
          Keys.onRightPressed: root.move(1)
          Keys.onEscapePressed: root.close()
          width: Math.max(120, Math.min(300, parent.width - 360))
          Keys.onUpPressed: root.move(root.searching ? -1 : -root.columns)
          Keys.onDownPressed: root.move(root.searching ? 1 : root.columns)
        }

        Widgets.IconButton {
          flat: true
          iconSize: 18
          implicitWidth: 28
          background: Item {}
          onClicked: root.close()
          anchors.rightMargin: 12
          ToolTip.text: qsTr("Close")
          anchors.right: parent.right
          icon.source: "qrc:/icons/buttons/close.svg"
          anchors.verticalCenter: parent.verticalCenter
          icon.color: (hovered || down) ? Cpp_ThemeManager.colors["highlight"]
                                        : Cpp_ThemeManager.colors["toolbar_text"]

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }
        }
      }

      //
      // Breadcrumb bar (kept above the unclipped scroll content).
      //
      Rectangle {
        z: 2
        border.width: 1
        Layout.topMargin: -1
        Layout.fillWidth: true
        Layout.preferredHeight: 32
        border.color: Cpp_ThemeManager.colors["pane_caption_border"]

        gradient: Gradient {
          GradientStop {
            position: 0
            color: Cpp_ThemeManager.colors["pane_caption_bg_top"]
          }

          GradientStop {
            position: 1
            color: Cpp_ThemeManager.colors["pane_caption_bg_bottom"]
          }
        }

        RowLayout {
          spacing: 4
          anchors.fill: parent
          anchors.leftMargin: 8
          anchors.rightMargin: 12

          Widgets.IconButton {
            flat: true
            iconSize: 16
            implicitWidth: 24
            background: Item {}
            onClicked: root.goUp()
            ToolTip.text: qsTr("Back")
            Layout.alignment: Qt.AlignVCenter
            enabled: root.levelStack.length > 1
            icon.source: "qrc:/icons/buttons/backward.svg"
            icon.color: Cpp_ThemeManager.colors["pane_caption_foreground"]

            HoverHandler {
              cursorShape: Qt.PointingHandCursor
              enabled: root.levelStack.length > 1
            }
          }

          Repeater {
            model: root.levelStack

            delegate: RowLayout {
              id: _crumb

              spacing: 4
              visible: root.levelStack.length > 1
              required property int index
              required property var modelData

              readonly property bool navigable: !_crumb.isLast
              readonly property bool isLast: _crumb.index === root.levelStack.length - 1

              Label {
                elide: Text.ElideRight
                Layout.maximumWidth: 160
                text: _crumb.modelData.text
                verticalAlignment: Text.AlignVCenter
                color: Cpp_ThemeManager.colors["pane_caption_foreground"]
                font: _crumb.isLast ? Cpp_Misc_CommonFonts.boldUiFont
                                    : Cpp_Misc_CommonFonts.uiFont

                MouseArea {
                  anchors.fill: parent
                  enabled: _crumb.navigable
                  cursorShape: Qt.PointingHandCursor
                  onClicked: root.goToLevel(_crumb.index)
                }
              }

              Label {
                text: "/"
                opacity: 0.4
                visible: !_crumb.isLast
                font: Cpp_Misc_CommonFonts.uiFont
                color: Cpp_ThemeManager.colors["pane_caption_foreground"]
              }
            }
          }

          Item { Layout.fillWidth: true }

          Label {
            opacity: 0.5
            elide: Text.ElideRight
            visible: !root.compact
            verticalAlignment: Text.AlignVCenter
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            text: qsTr("Type to search, Enter to open, Esc to close")
            color: Cpp_ThemeManager.colors["pane_caption_foreground"]
          }
        }
      }

      Flickable {
        id: _scroll

        Layout.margins: 16
        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: _sectionColumn.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
          policy: _scroll.contentHeight > _scroll.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        }

        Column {
          id: _sectionColumn

          spacing: 12
          width: _scroll.width
          transform: Translate {
            id: _slide
          }

          NumberAnimation {
            id: _slideAnim

            to: 0
            property: "x"
            duration: 200
            target: _slide
            easing.type: Easing.OutCubic
          }

          Repeater {
            model: root.sections

            delegate: Column {
              id: _section

              spacing: 6
              width: _sectionColumn.width

              required property var modelData

              RowLayout {
                spacing: 8
                width: parent.width
                visible: _section.modelData.title.length > 0

                Label {
                  text: _section.modelData.title
                  color: Cpp_ThemeManager.colors["text"]
                  font: Cpp_Misc_CommonFonts.boldUiFont
                  Component.onCompleted: font.capitalization = Font.AllUppercase
                }

                Rectangle {
                  height: 1
                  Layout.fillWidth: true
                  Layout.alignment: Qt.AlignVCenter
                  color: Cpp_ThemeManager.colors["groupbox_border"]
                }
              }

              Flow {
                width: parent.width
                visible: !root.searching

                Repeater {
                  delegate: _cellComponent
                  model: root.searching ? [] : _section.modelData.items
                }
              }

              Column {
                spacing: 2
                width: parent.width
                visible: root.searching

                Repeater {
                  delegate: _rowComponent
                  model: root.searching ? _section.modelData.items : []
                }
              }
            }
          }
        }
      }

      Label {
        opacity: 0.6
        Layout.fillWidth: true
        Layout.bottomMargin: 16
        font: Cpp_Misc_CommonFonts.uiFont
        text: qsTr("No results found")
        color: Cpp_ThemeManager.colors["text"]
        horizontalAlignment: Text.AlignHCenter
        visible: root.displayNodes.length === 0
      }
    }

    //
    // Thin bottom rule kept above the unclipped scroll content, inset by the corner radius.
    //
    Rectangle {
      z: 2
      height: 1
      width: panel.width - 2 * panel.radius
      color: Cpp_ThemeManager.colors["groupbox_border"]

      anchors {
        bottom: parent.bottom
        horizontalCenter: parent.horizontalCenter
      }
    }
  }
}
