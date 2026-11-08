/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  objectName: "shortcutGenerator"

  title: qsTr("New Deployment")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  property int outputMode: 0
  property string themeName: ""
  property string shortcutTitle: ""
  property string iconPathValue: ""
  property string lastErrorText: ""
  property bool userPickedIcon: false
  property string taskbarMode: "shown"
  property string savedPackagePath: ""

  readonly property bool packageMode: outputMode === 0
  readonly property var packager: Cpp_ShortcutGenerator.packager
  readonly property string projectTitle: Cpp_JSON_ProjectModel.title
  readonly property string projectPath: Cpp_AppState.projectFilePath
  readonly property bool alsoShortcut: packageMode && desktopShortcut.checked

  function selectedTaskbarButtons() {
    var ids = []
    if (pinConsole.checked)          ids.push("console")
    if (pinNotifications.checked)    ids.push("notifications")
    if (pinClock.checked)            ids.push("clock")
    if (pinStopwatch.checked)        ids.push("stopwatch")
    if (pinPause.checked)            ids.push("pause")
    if (pinFileTransmission.checked) ids.push("file_transmission")
    return ids
  }

  function formatSize(bytes) {
    if (bytes < 1024)
      return qsTr("%1 B").arg(bytes)

    if (bytes < 1024 * 1024)
      return qsTr("%1 KB").arg((bytes / 1024).toFixed(1))

    return qsTr("%1 MB").arg((bytes / (1024 * 1024)).toFixed(1))
  }

  function folderUrl(path) {
    if (path.length === 0)
      return StandardPaths.writableLocation(StandardPaths.DesktopLocation)

    return path.startsWith("/") ? "file://" + path : "file:///" + path
  }

  function packageOptions() {
    return {
      "title": root.shortcutTitle,
      "iconPath": root.iconPathValue,
      "fullscreen": fullscreen.checked,
      "actionsPanel": actionsPanel.checked,
      "fileTransmission": fileTransmission.checked,
      "csvExport": csvExport.checked,
      "mdfExport": mdfExport.checked,
      "sessionExport": sessionExport.checked,
      "consoleExport": consoleExport.checked,
      "taskbarMode": root.taskbarMode,
      "taskbarButtons": root.selectedTaskbarButtons(),
      "taskbarSearch": searchBar.checked,
      "theme": root.themeName
    }
  }

  onPackageModeChanged: {
    savedPackagePath = ""
    if (packageMode)
      packager.refresh()
    else if (_tab.currentIndex === 3)
      _tab.currentIndex = 0
  }

  onVisibleChanged: {
    if (visible) {
      outputMode = 0
      savedPackagePath = ""
      desktopShortcut.checked = false
      if (outputCombo)
        outputCombo.currentIndex = 0

      packager.refresh()
      shortcutTitle = projectTitle
      iconPathValue = Cpp_ShortcutGenerator.defaultIconPath
      userPickedIcon = false
      lastErrorText = ""
      taskbarMode = "shown"
      themeName = ""
      if (taskbarModeCombo)
        taskbarModeCombo.currentIndex = 0

      if (searchBar)
        searchBar.checked = true

      if (themeCombo)
        themeCombo.currentIndex = 0
    }
  }

  //
  // Keep the title in sync whenever a new project is loaded
  //
  Connections {
    target: Cpp_JSON_ProjectModel
    function onTitleChanged() {
      root.shortcutTitle = root.projectTitle
    }
  }

  Connections {
    target: Cpp_ShortcutGenerator

    function onShortcutGenerated(path) {
      lastErrorText = ""
      if (!root.packageMode)
        root.close()
    }

    function onShortcutFailed(message) {
      lastErrorText = message
    }
  }

  Connections {
    target: root.packager

    function onExportFinished(path) {
      lastErrorText = ""
      root.savedPackagePath = path
      if (!root.alsoShortcut)
        return

      packageShortcutPicker.packagePath = path
      const sanitized = root.shortcutTitle.replace(/[\\/:*?"<>|]/g, "_").trim()
      const desktop = StandardPaths.writableLocation(StandardPaths.DesktopLocation)
      const suffix = "." + Cpp_ShortcutGenerator.shortcutDefaultSuffix
      packageShortcutPicker.selectedFile = desktop + "/"
          + (sanitized.length > 0 ? sanitized : "Untitled Deployment") + suffix
      packageShortcutPicker.open()
    }

    function onExportFailed(message) {
      lastErrorText = message
    }
  }

  FileDialog {
    id: packagePicker

    title: qsTr("Save Deployment Package")
    fileMode: FileDialog.SaveFile
    defaultSuffix: "ssdeploy"
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    nameFilters: [qsTr("Serial Studio Deployment Package (*.ssdeploy)")]
    onAccepted: {
      const file = selectedFile
      const options = root.packageOptions()
      Qt.callLater(() => root.packager.exportPackage(file, options))
    }
  }

  FileDialog {
    id: packageShortcutPicker

    property string packagePath: ""

    title: qsTr("Save Deployment")
    fileMode: FileDialog.SaveFile
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    defaultSuffix: Cpp_ShortcutGenerator.shortcutDefaultSuffix
    nameFilters: [Cpp_ShortcutGenerator.shortcutFileFilter]
    onAccepted: {
      const file = selectedFile
      const title = root.shortcutTitle
      const iconPath = root.iconPathValue
      const pkg = packagePath
      Qt.callLater(() => Cpp_ShortcutGenerator.generatePackageShortcut(
                     file, title, iconPath, pkg))
    }
  }

  FileDialog {
    id: extraFilesPicker

    title: qsTr("Add Files")
    fileMode: FileDialog.OpenFiles
    currentFolder: root.folderUrl(root.packager.projectFolder)
    nameFilters: [qsTr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg *.webp)"),
                  qsTr("All files (*)")]
    onAccepted: {
      const files = selectedFiles.map(f => f.toString())
      Qt.callLater(() => root.packager.addExtraFiles(files))
    }
  }

  FileDialog {
    id: iconPicker

    title: qsTr("Choose an Icon")
    fileMode: FileDialog.OpenFile
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    nameFilters: [Cpp_ShortcutGenerator.iconFileFilter]
    onAccepted: {
      const file = selectedFile
      Qt.callLater(() => {
        root.iconPathValue = file
        root.userPickedIcon = true
      })
    }
  }

  FileDialog {
    id: outputPicker

    title: qsTr("Save Deployment")
    fileMode: FileDialog.SaveFile
    currentFolder: StandardPaths.writableLocation(StandardPaths.DesktopLocation)
    defaultSuffix: Cpp_ShortcutGenerator.shortcutDefaultSuffix
    nameFilters: [Cpp_ShortcutGenerator.shortcutFileFilter]
    onAccepted: {
      const file = selectedFile
      const title = root.shortcutTitle
      const iconPath = root.iconPathValue
      const projectPath = root.projectPath
      const isFullscreen = fullscreen.checked
      const hasActions = actionsPanel.checked
      const hasFileTransmission = fileTransmission.checked
      const hasCsv = csvExport.checked
      const hasMdf = mdfExport.checked
      const hasSession = sessionExport.checked
      const hasConsole = consoleExport.checked
      const mode = root.taskbarMode
      const buttons = root.selectedTaskbarButtons()
      const hasSearch = searchBar.checked
      const theme = root.themeName
      Qt.callLater(() => Cpp_ShortcutGenerator.generate(
                     file, title, iconPath, projectPath, isFullscreen,
                     hasActions, hasFileTransmission, hasCsv, hasMdf,
                     hasSession, hasConsole, mode, buttons, hasSearch, theme))
    }
  }

  //
  // Framed list and its alternating rows, shared by the Contents tab sections
  //
  component ListFrame: Rectangle {
    default property alias rows: _rows.data

    radius: 2
    clip: true
    border.width: 1
    Layout.fillWidth: true
    implicitHeight: _rows.implicitHeight + 2
    color: Cpp_ThemeManager.colors["widget_base"]
    border.color: Cpp_ThemeManager.colors["widget_border"]

    ColumnLayout {
      id: _rows

      spacing: 0
      anchors.margins: 1
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
    }
  }

  component ListRow: Rectangle {
    required property int index
    default property alias cells: _cells.data

    Layout.fillWidth: true
    implicitHeight: Math.max(24, _cells.implicitHeight + 4)
    color: index % 2 === 1 ? Cpp_ThemeManager.colors["alternate_base"] : "transparent"

    RowLayout {
      id: _cells

      spacing: 8
      anchors.fill: parent
      anchors.leftMargin: 8
      anchors.rightMargin: 4
    }
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("General")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Taskbar")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Logging")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Contents")
        height: _tab.height + 3
        visible: root.packageMode
        width: visible ? implicitWidth + 2 * 8 : 0
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 520
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(generalTab.implicitHeight,
                               taskbarTab.implicitHeight,
                               loggingTab.implicitHeight,
                               contentsTab.implicitHeight)

      //
      // General tab
      //
      Item {
        id: generalTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: generalLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: generalLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.margins: 8
          anchors.fill: parent

          //
          // Section: Output
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Output")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Format")
            color: Cpp_ThemeManager.colors["text"]
            Layout.minimumWidth: desktopShortcutLabel.implicitWidth
          } Widgets.Combo {
            id: outputCombo

            currentIndex: 0
            Layout.fillWidth: true
            model: [
              { value: 0, label: qsTr("Deployment package (.ssdeploy)") },
              { value: 1, label: qsTr("Shortcut to the project file") }
            ]
            textRole: "label"
            onActivated: root.outputMode = model[currentIndex].value
          }

          Label {
            text: qsTr("Project")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 2
            Layout.fillWidth: true

            Widgets.LineField {
              readOnly: true
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              placeholderText: qsTr("Choose a project file to begin")
              text: root.packageMode ? Cpp_JSON_ProjectModel.jsonFileName
                                     : Cpp_AppState.projectFileName
            }

            Widgets.IconButton {
              Layout.fillWidth: false
              Layout.maximumWidth: 24
              Layout.maximumHeight: 24
              visible: !root.packageMode
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectModel.openJsonFile()
              icon.source: "qrc:/icons/buttons/open.svg"
            }
          }

          Label {
            id: desktopShortcutLabel

            visible: root.packageMode
            text: qsTr("Desktop Shortcut")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: desktopShortcut

            visible: root.packageMode
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          //
          // Section: Identity
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Identity")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Name")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.LineField {
            id: titleField

            Layout.fillWidth: true
            text: root.shortcutTitle
            placeholderText: qsTr("Deployment Name")
            onTextEdited: root.shortcutTitle = text
          }

          Label {
            text: qsTr("Icon")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Item {
              Layout.alignment: Qt.AlignVCenter
              implicitWidth: changeIcon.implicitHeight
              implicitHeight: changeIcon.implicitHeight

              Image {
                smooth: true
                mipmap: true
                cache: false
                asynchronous: true
                anchors.fill: parent
                sourceSize: Qt.size(64, 64)
                fillMode: Image.PreserveAspectFit
                source: root.userPickedIcon
                        ? root.iconPathValue
                        : "qrc:/images/shortcut.svg"
              }

              HoverHandler {
                id: iconHover

                cursorShape: Qt.PointingHandCursor
              }

              TapHandler {
                onTapped: iconPicker.open()
              }

              ToolTip.delay: 600
              ToolTip.visible: iconHover.hovered
              ToolTip.text: qsTr("Click to choose an icon")
            }

            Widgets.PushButton {
              id: changeIcon

              text: qsTr("Change Icon…")
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: iconPicker.open()
              Layout.alignment: Qt.AlignVCenter
            }

            Item {
              Layout.fillWidth: true
            }
          }

          //
          // Section: Behavior
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Behavior")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Theme")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            id: themeCombo

            currentIndex: 0
            Layout.fillWidth: true
            model: {
              const items = [{ value: "", label: qsTr("Same as Serial Studio") }]
              const themes = Cpp_ThemeManager.availableThemes
              for (let i = 0; i < themes.length; ++i)
                items.push({ value: themes[i], label: themes[i] })

              return items
            }
            textRole: "label"
            onActivated: root.themeName = model[currentIndex].value
          }

          Label {
            text: qsTr("Fullscreen")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: fullscreen

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Actions Panel")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: actionsPanel

            checked: true
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("File Transmission")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: fileTransmission

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            Layout.columnSpan: 2
            Layout.topMargin: 4
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: root.packageMode
                  ? qsTr("Opens straight to the live dashboard, with no toolbar, no setup "
                         + "pane, and no way to view or export the project. Serial Studio "
                         + "quits when the device disconnects.")
                  : qsTr("Opens the project straight to the live dashboard, with no toolbar "
                         + "or setup pane. Serial Studio quits when the device disconnects.")
          }

          Item { Layout.fillHeight: true }
        }
      }

      //
      // Taskbar tab
      //
      Item {
        id: taskbarTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: taskbarLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: taskbarLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.margins: 8
          anchors.fill: parent

          //
          // Section: Visibility
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Visibility")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Mode")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            id: taskbarModeCombo

            Layout.fillWidth: true
            model: [
              { value: "shown",    label: qsTr("Always shown") },
              { value: "autohide", label: qsTr("Auto-hide") },
              { value: "hidden",   label: qsTr("Hidden") }
            ]
            textRole: "label"
            currentIndex: 0
            onActivated: root.taskbarMode = model[currentIndex].value
          }

          Label {
            text: qsTr("Search Bar")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: searchBar

            checked: true
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            Layout.columnSpan: 2
            Layout.topMargin: 4
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: root.taskbarMode === "hidden"
                  ? qsTr("Hiding the taskbar removes window minimize/maximize/close "
                       + "buttons and forces auto-layout, so the dashboard always "
                       + "fills the available area.")
                  : root.taskbarMode === "autohide"
                    ? qsTr("The taskbar slides in when the user moves the cursor "
                         + "near the bottom edge.")
                    : qsTr("The taskbar is permanently visible at the bottom of "
                         + "the dashboard.")
          }

          //
          // Section: Buttons
          //
          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Pinned Buttons")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Console")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinConsole

            checked: false
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Notifications")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinNotifications

            checked: true
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Clock")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinClock

            checked: false
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Stopwatch")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinStopwatch

            checked: false
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Pause")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden"
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinPause

            checked: true
            enabled: root.taskbarMode !== "hidden"
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("File Transmission")
            color: Cpp_ThemeManager.colors["text"]
            enabled: root.taskbarMode !== "hidden" && fileTransmission.checked
            opacity: enabled ? 1 : 0.5
          } Widgets.Toggle {
            id: pinFileTransmission

            checked: false
            enabled: root.taskbarMode !== "hidden" && fileTransmission.checked
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Item { Layout.fillHeight: true }
        }
      }

      //
      // Logging tab
      //
      Item {
        id: loggingTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: loggingLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: loggingLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.margins: 8
          anchors.fill: parent

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Recorders")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("CSV File")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: csvExport

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("MDF4 File")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: mdfExport

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Historian")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: sessionExport

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            text: qsTr("Console Log")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Toggle {
            id: consoleExport

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
          }

          Label {
            Layout.columnSpan: 2
            Layout.topMargin: 8
            Layout.fillWidth: true
            wrapMode: Label.Wrap
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("Recordings are saved in the Serial Studio workspace folder")
          }

          Item { Layout.fillHeight: true }
        }
      }

      //
      // Contents tab
      //
      Item {
        id: contentsTab

        implicitHeight: 360
        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ScrollView {
          clip: true
          anchors.margins: 8
          anchors.fill: parent
          contentWidth: availableWidth

          ColumnLayout {
            spacing: 4
            width: parent.width

            //
            // Section: Carried files
            //
            RowLayout {
              spacing: 8
              Layout.topMargin: 2
              Layout.fillWidth: true

              Label {
                Layout.fillWidth: true
                text: qsTr("Carried Files")
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
                color: Cpp_ThemeManager.colors["pane_section_label"]
                Component.onCompleted: font.capitalization = Font.AllUppercase
              }

              Label {
                font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
                color: Cpp_ThemeManager.colors["placeholder_text"]
                text: qsTr("%1 in total").arg(root.formatSize(root.packager.estimatedSize))
              }
            } Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              color: Cpp_ThemeManager.colors["groupbox_border"]
            } Item {
              implicitHeight: 2
            }

            ListFrame {
              Repeater {
                model: root.packager.carriedFiles

                delegate: ListRow {
                  required property var modelData

                  Label {
                    text: modelData.path
                    Layout.fillWidth: true
                    elide: Label.ElideMiddle
                    color: Cpp_ThemeManager.colors["text"]
                  }

                  Label {
                    text: root.formatSize(modelData.size)
                    color: Cpp_ThemeManager.colors["placeholder_text"]
                  }

                  Widgets.IconButton {
                    flat: true
                    iconSize: 12
                    implicitWidth: 20
                    background: Item {}
                    ToolTip.text: qsTr("Remove")
                    enabled: modelData.extra === true
                    Layout.alignment: Qt.AlignVCenter
                    opacity: modelData.extra === true ? 1 : 0
                    icon.source: "qrc:/icons/buttons/close.svg"
                    onClicked: root.packager.removeExtraFile(modelData.path)
                  }
                }
              }
            }

            RowLayout {
              spacing: 8
              Layout.topMargin: 2
              Layout.fillWidth: true

              Label {
                Layout.fillWidth: true
                wrapMode: Label.Wrap
                font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
                color: Cpp_ThemeManager.colors["placeholder_text"]
                text: qsTr("Add images a Canvas script loads at run time.")
              }

              Widgets.PushButton {
                text: qsTr("Add Files…")
                font: Cpp_Misc_CommonFonts.uiFont
                onClicked: extraFilesPicker.open()
                Layout.alignment: Qt.AlignVCenter
              }
            }

            //
            // Section: Missing files
            //
            Label {
              Layout.topMargin: 8
              text: qsTr("Missing Files")
              color: Cpp_ThemeManager.colors["alarm"]
              visible: root.packager.missingFiles.length > 0
              font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
              Component.onCompleted: font.capitalization = Font.AllUppercase
            } Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              color: Cpp_ThemeManager.colors["alarm"]
              visible: root.packager.missingFiles.length > 0
            }

            ListFrame {
              visible: root.packager.missingFiles.length > 0

              Repeater {
                model: root.packager.missingFiles

                delegate: ListRow {
                  required property var modelData

                  Label {
                    text: modelData
                    Layout.fillWidth: true
                    elide: Label.ElideMiddle
                    color: Cpp_ThemeManager.colors["alarm"]
                  }
                }
              }
            }

            Label {
              Layout.fillWidth: true
              wrapMode: Label.Wrap
              color: Cpp_ThemeManager.colors["alarm"]
              visible: root.packager.missingFiles.length > 0
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              text: qsTr("Export is blocked until these files exist.")
            }

            //
            // Section: External dependencies
            //
            Label {
              Layout.topMargin: 8
              text: qsTr("External Dependencies")
              visible: root.packager.externals.length > 0
              font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
              color: Cpp_ThemeManager.colors["pane_section_label"]
              Component.onCompleted: font.capitalization = Font.AllUppercase
            } Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              visible: root.packager.externals.length > 0
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            ListFrame {
              visible: root.packager.externals.length > 0

              Repeater {
                model: root.packager.externals

                delegate: ListRow {
                  required property var modelData

                  Label {
                    text: modelData
                    Layout.fillWidth: true
                    elide: Label.ElideMiddle
                    color: Cpp_ThemeManager.colors["text"]
                  }
                }
              }
            }

            Label {
              Layout.fillWidth: true
              wrapMode: Label.Wrap
              visible: root.packager.externals.length > 0
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: qsTr("The target machine must provide these.")
            }

            //
            // Section: Credentials and private keys
            //
            Label {
              Layout.topMargin: 8
              visible: root.packager.secrets.length > 0
              text: qsTr("Credentials and Private Keys")
              font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
              color: Cpp_ThemeManager.colors["pane_section_label"]
              Component.onCompleted: font.capitalization = Font.AllUppercase
            } Rectangle {
              implicitHeight: 1
              Layout.fillWidth: true
              visible: root.packager.secrets.length > 0
              color: Cpp_ThemeManager.colors["groupbox_border"]
            }

            Repeater {
              model: root.packager.secrets

              delegate: CheckBox {
                required property int index
                required property var modelData

                text: modelData.label
                Layout.fillWidth: true
                checked: modelData.included === true
                onToggled: root.packager.setSecretIncluded(index, checked)
              }
            }

            Label {
              Layout.fillWidth: true
              wrapMode: Label.Wrap
              visible: root.packager.anySecretIncluded
              color: Cpp_ThemeManager.colors["alarm"]
              font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
              text: qsTr("Packages are encoded, not encrypted. Anyone with the file "
                         + "can recover the items you include.")
            }
          }
        }
      }
    }

    //
    // Inline error banner
    //
    Label {
      Layout.fillWidth: true
      visible: root.lastErrorText.length > 0
      color: Cpp_ThemeManager.colors["alarm"]
      wrapMode: Label.Wrap
      text: root.lastErrorText
    }

    //
    // Footer buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Label {
        id: savedLabel

        Layout.fillWidth: true
        elide: Label.ElideMiddle
        Layout.alignment: Qt.AlignVCenter
        visible: root.savedPackagePath.length > 0
        color: Cpp_ThemeManager.colors["placeholder_text"]
        text: qsTr("Saved %1").arg(root.savedPackagePath.split(/[\\/]/).pop())
      }

      Item {
        Layout.fillWidth: true
        visible: !savedLabel.visible
      }

      Widgets.IconButton {
        horizontalPadding: 8
        text: qsTr("Open Package")
        visible: savedLabel.visible
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/play.svg"
        onClicked: Cpp_JSON_ProjectModel.openDeployment(root.savedPackagePath)
      }

      Widgets.IconButton {
        horizontalPadding: 8
        onClicked: root.close()
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/close.svg"
        text: savedLabel.visible ? qsTr("Close") : qsTr("Cancel")
      }

      Widgets.IconButton {
        highlighted: true
        text: qsTr("Save")
        horizontalPadding: 8
        opacity: app.proVersion ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/apply.svg"
        enabled: root.shortcutTitle.trim().length > 0
                 && (root.packageMode
                     ? root.projectTitle.length > 0
                       && root.packager.missingFiles.length === 0
                     : root.projectPath.length > 0)
        onClicked: {
          const sanitized = root.shortcutTitle.replace(/[\\/:*?"<>|]/g, "_").trim()
          const desktop = StandardPaths.writableLocation(StandardPaths.DesktopLocation)
          const suffix = "." + (root.packageMode ? "ssdeploy"
                                                 : Cpp_ShortcutGenerator.shortcutDefaultSuffix)
          const fileName = (sanitized.length > 0 ? sanitized : "Untitled Deployment") + suffix
          const picker = root.packageMode ? packagePicker : outputPicker
          picker.selectedFile = desktop + "/" + fileName
          picker.open()
        }
      }
    }
  }
}
