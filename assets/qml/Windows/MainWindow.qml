/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick 2.3
import QtQuick.Window 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import Qt.labs.settings 1.0

import "../Panes"
import "../Windows"
import "../Widgets"
import "../JsonEditor"
import "../FramelessWindow" as FramelessWindow
import "../PlatformDependent" as PlatformDependent

FramelessWindow.CustomWindow {
    id: root

    //
    // Quit application when this window is closed
    //
    onClosed: Qt.quit()

    //
    // Customize window border
    //
    showIcon: true
    borderWidth: 1
    borderColor: Qt.darker(Cpp_ThemeManager.toolbarGradient2, 1.5)

    //
    // Global properties
    //
    readonly property bool setupVisible: setup.visible
    readonly property bool consoleVisible: terminal.visible
    readonly property bool dashboardVisible: dashboard.visible

    //
    // Custom properties
    //
    property int appLaunchCount: 0
    property bool firstValidFrame: false
    property bool automaticUpdates: false
    property alias vt100emulation: terminal.vt100emulation

    //
    // Toolbar functions aliases
    //
    function showSetup()     { toolbar.setupClicked()     }
    function showConsole()   { toolbar.consoleClicked()   }
    function showDashboard() { dbTimer.start() }

    //
    // Wait a little before showing the dashboard to avoid UI glitches and/or overloading
    // the rendering engine
    //
    Timer {
        id: dbTimer
        interval: 500
        onTriggered: toolbar.dashboardClicked()
    }

    //
    // Console-related functions
    //
    function consoleCopy()      { terminal.copy()      }
    function consoleClear()     { terminal.clear()     }
    function consoleSelectAll() { terminal.selectAll() }

    //
    // Window geometry
    //
    visible: false
    title: Cpp_AppName
    width: minimumWidth
    height: minimumHeight
    minimumWidth: 1120 + 2 * root.shadowMargin
    backgroundColor: Cpp_ThemeManager.windowBackground
    minimumHeight: 650 + 2 * root.shadowMargin + root.titlebar.height

    //
    // Startup code
    //
    Component.onCompleted: {
        // Load welcome text
        terminal.showWelcomeGuide()

        // Increment app launch count & hide splash screen
        ++appLaunchCount

        // Show app window
        if (root.isFullscreen)
            root.showFullScreen()
        else if (root.isMaximized)
            root.showMaximized()
        else {
            // Fix maximize not working on first try on macOS & Windows
            root.opacity = 0
            var x = root.x
            var y = root.y
            var w = root.width
            var h = root.height
            root.showMaximized()
            root.showNormal()
            root.setGeometry(x, y, w,h)
            root.opacity = 1
        }

        // Force active focus
        root.requestActivate()
        root.requestUpdate()

        // Show donations dialog every 15 launches
        if (root.appLaunchCount % 15 == 0 && !app.donateDialog.doNotShowAgain)
            app.donateDialog.showAutomatically()

        // Ask user if he/she wants to enable automatic updates
        if (root.appLaunchCount == 2 && Cpp_UpdaterEnabled) {
            if (Cpp_Misc_Utilities.askAutomaticUpdates()) {
                root.automaticUpdates = true
                Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
            }

            else
                root.automaticUpdates = false
        }

        // Check for updates (if we are allowed)
        if (root.automaticUpdates && Cpp_UpdaterEnabled)
            Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
    }

    //
    // Hide console & device manager when we receive first valid frame
    //
    Connections {
        target: Cpp_UI_Dashboard

        function onUpdated()  {
            if (root.firstValidFrame)
                return

            if ((Cpp_IO_Manager.connected || Cpp_CSV_Player.isOpen) && Cpp_UI_Dashboard.frameValid()) {
                setup.hide()
                root.showDashboard()
                root.firstValidFrame = true
            }

            else {
                setup.show()
                root.showConsole()
                root.firstValidFrame = false
            }
        }
    }

    //
    // Show console tab on serial disconnect
    //
    Connections {
        target: Cpp_UI_Dashboard
        function onDataReset() {
            setup.show()
            root.showConsole()
            root.firstValidFrame = false
        }
    }

    //
    // Save window size & position
    //
    Settings {
        property alias wx: root.x
        property alias wy: root.y
        property alias ww: root.width
        property alias wh: root.height
        property alias wm: root.isMaximized
        property alias wf: root.isFullscreen
        property alias appStatus: root.appLaunchCount
        property alias autoUpdater: root.automaticUpdates
    }

    //
    // macOS menubar loader
    //
    Loader {
        active: Cpp_IsMac
        asynchronous: false
        sourceComponent: PlatformDependent.MenubarMacOS {}
    }

    //
    // Rectangle for the menubar (only used if custom window flags are disabled)
    //
    Rectangle {
        color: root.titlebarColor
        anchors.fill: menubarLayout
        visible: !Cpp_ThemeManager.customWindowDecorations
    }

    //
    // Menubar, shown by default on Windows & Linux and when the app is fullscreen
    //
    RowLayout {
        id: menubarLayout
        z: titlebar.z + 1
        spacing: app.spacing
        height: !showMenubar ? titlebar.height : 38

        readonly property bool showMenubar: !root.showMacControls || isFullscreen

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: root.leftTitlebarMargin + root.shadowMargin
            rightMargin: root.rightTitlebarMargin + root.shadowMargin
            topMargin: root.shadowMargin + (!root.showMacControls ? 1 : 0)
        }

        //
        // Menubar
        //
        Loader {
            opacity: 0.8
            asynchronous: false
            Layout.alignment: Qt.AlignVCenter
            sourceComponent: PlatformDependent.Menubar {
                enabled: !root.showMacControls || isFullscreen
                visible: !root.showMacControls || isFullscreen
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }
    }

    //
    // Main layout
    //
    Page {
        clip: true
        anchors.fill: parent
        anchors.margins: root.shadowMargin
        palette.text: Cpp_ThemeManager.text
        palette.buttonText: Cpp_ThemeManager.text
        palette.windowText: Cpp_ThemeManager.text
        anchors.topMargin: menubarLayout.height + root.shadowMargin

        background: Rectangle {
            radius: root.radius
            color: Cpp_ThemeManager.windowBackground
        }

        ColumnLayout {
            spacing: 0
            anchors.fill: parent

            //
            // Application toolbar
            //
            Toolbar {
                id: toolbar
                window: root
                z: titlebar.z
                Layout.fillWidth: true
                Layout.minimumHeight: 48
                Layout.maximumHeight: 48
                setupChecked: root.setupVisible
                consoleChecked: root.consoleVisible
                dashboardChecked: root.dashboardVisible
                onJsonEditorClicked: app.jsonEditorWindow.show()
                onSetupClicked: setup.visible ? setup.hide() : setup.show()

                onDashboardClicked: {
                    if (Cpp_UI_Dashboard.available) {
                        consoleChecked = 0
                        dashboardChecked = 1
                        if (stack.currentItem != dashboard)
                            stack.push(dashboard)
                    }

                    else
                        root.showConsole()
                }

                onConsoleClicked: {
                    consoleChecked = 1
                    dashboardChecked = 0
                    stack.pop()
                }
            }

            //
            // Console, dashboard & setup panel
            //
            RowLayout {
                spacing: 0
                Layout.fillWidth: true
                Layout.fillHeight: true

                StackView {
                    id: stack
                    clip: true
                    initialItem: terminal
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    data: [
                        Console {
                            id: terminal
                            visible: false
                            width: parent.width
                            height: parent.height
                        },

                        Dashboard {
                            id: dashboard
                            visible: false
                            width: parent.width
                            height: parent.height
                        }
                    ]
                }

                Setup {
                    id: setup
                    Layout.fillHeight: true
                    Layout.rightMargin: setupMargin
                    Layout.minimumWidth: displayedWidth
                    Layout.maximumWidth: displayedWidth
                }
            }
        }
    }

    //
    // JSON project drop area
    //
    JSONDropArea {
        anchors.fill: parent
        enabled: !Cpp_IO_Manager.connected
    }

    //
    // Resize handler
    //
    FramelessWindow.ResizeHandles {
        window: root
        anchors.fill: parent
        handleSize: root.handleSize
    }
}
