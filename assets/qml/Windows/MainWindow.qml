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

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.settings

import "../Panes"
import "../Windows"
import "../Widgets"
import "../JsonEditor"

ApplicationWindow {
    id: root

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
    property bool firstChange: true
    property bool fullScreen: false
    property bool menubarEnabled: true
    property bool windowMaximized: false
    property bool firstValidFrame: false
    property bool automaticUpdates: false
    property alias vt100emulation: terminal.vt100emulation

    //
    // Toolbar functions aliases
    //
    function showSetup()     { toolbar.setupClicked()     }
    function showConsole()   { toolbar.consoleClicked()   }
    function showDashboard() { toolbar.dashboardClicked() }

    //
    // Console-related functions
    //
    function consoleCopy()      { terminal.copy()      }
    function consoleClear()     { terminal.clear()     }
    function consoleSelectAll() { terminal.selectAll() }

    //
    // Hide/show menubar
    //
    function toggleMenubar() {
        root.menubarEnabled = !root.menubarEnabled
    }

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        root.fullScreen = !root.fullScreen
        if (root.fullScreen)
            root.showFullScreen()
        else
            root.showNormal()
    }

    //
    // Displays the main window & checks for updates
    //
    function showMainWindow() {
        // Startup verifications to ensure that app is displayed inside the screen
        if (x < 0 || x >= Screen.desktopAvailableWidth)
            x = 100
        if (y < 0 || y >= Screen.desktopAvailableHeight)
            y = 100

        // Startup verifications to ensure that app fits in current screen
        if (width > Screen.desktopAvailableWidth) {
            x = 100
            width = Screen.desktopAvailableWidth - x
        }

        // Startup verifications to ensure that app fits in current screen
        if (height > Screen.desktopAvailableHeight) {
            y = 100
            height = Screen.desktopAvailableHeight - y
        }

        // Show app window
        if (root.fullScreen)
            root.showFullScreen()
        else if (root.windowMaximized)
            root.showMaximized()
        else
            root.showNormal()

        // Increment app launch count
        ++appLaunchCount

        // Show donations dialog every 15 launches
        if (appLaunchCount % 15 == 0 && !donations.doNotShowAgain)
            donations.showAutomatically()

        // Ask user if he/she wants to enable automatic updates
        if (appLaunchCount == 2 && Cpp_UpdaterEnabled) {
            if (Cpp_Misc_Utilities.askAutomaticUpdates()) {
                root.automaticUpdates = true
                Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
            }

            else
                root.automaticUpdates = false
        }

        // Check for updates (if we are allowed)
        if (automaticUpdates && Cpp_UpdaterEnabled)
            Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
    }

    //
    // Window geometry (different minimum size for non-macOS
    // operating systems because of the global menubar in macOS)
    //
    visible: false
    minimumWidth: 1100
    title: Cpp_AppName
    width: minimumWidth
    height: minimumHeight
    minimumHeight: Qt.platform.os === "osx" ? 720 : 740

    //
    // Theme options
    //
    palette.text: Cpp_ThemeManager.text
    palette.buttonText: Cpp_ThemeManager.text
    palette.windowText: Cpp_ThemeManager.text
    palette.window: Cpp_ThemeManager.windowBackground
    background: Rectangle {
        color: Cpp_ThemeManager.windowBackground
    }

    //
    // Startup code
    //
    Component.onCompleted: {
        // Load welcome guide
        terminal.showWelcomeGuide()

        // Display the window & check for updates in 500 ms (we do this so that
        // we wait for the window to read settings before showing it)
        timer.start()
    }

    //
    // Hacks to fix window maximized behavior
    //
    Connections {
        target: root

        function  onVisibilityChanged(visibility) {
            if (visibility === Window.Maximized) {
                if (!windowMaximized)
                    firstChange = false

                windowMaximized = true
                fullScreen = false
            }

            else if (visibility === Window.FullScreen) {
                if (!fullScreen)
                    firstChange = false

                windowMaximized = false
                fullScreen = true
            }

            else if (visibility !== Window.Hidden) {
                if (windowMaximized || fullScreen && firstChange) {
                    root.x = 100
                    root.y = 100
                    root.width = root.minimumWidth
                    root.height = root.minimumHeight
                }

                fullScreen = false
                windowMaximized = false
            }

            // Hide splash screen
            Cpp_ModuleManager.hideSplashscreen()
        }
    }

    //
    // Hide console & device manager when we receive first valid frame
    //
    Connections {
        target: Cpp_UI_Dashboard
        enabled: !root.firstValidFrame

        function onUpdated()  {
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
        property alias appX: root.x
        property alias appY: root.y
        property alias appW: root.width
        property alias appH: root.height
        property alias appStatus: root.appLaunchCount
        property alias windowFullScreen: root.fullScreen
        property alias autoUpdater: root.automaticUpdates
        property alias appMaximized: root.windowMaximized
        property alias menubarVisible: root.menubarEnabled
    }

    //
    // Application menubar loader (we need to use a different version in macOS)
    //
    Loader {
        asynchronous: false
        source: {
            if (Qt.platform.os === "osx")
                return "qrc:/qml/PlatformDependent/MenubarMacOS.qml"

            return "qrc:/qml/PlatformDependent/Menubar.qml"
        }
    }

    //
    // Startup timer
    //
    Timer {
        id: timer
        interval: 500
        onTriggered: root.showMainWindow()
    }

    //
    // Main layout
    //
    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        //
        // Application toolbar
        //
        Toolbar {
            z: 1
            id: toolbar
            Layout.fillWidth: true
            Layout.minimumHeight: 48
            Layout.maximumHeight: 48
            setupChecked: root.setupVisible
            consoleChecked: root.consoleVisible
            dashboardChecked: root.dashboardVisible
            onJsonEditorClicked: jsonEditor.show()
            onSetupClicked: setup.visible ? setup.hide() : setup.show()

            onDashboardClicked: {
                if (Cpp_UI_Dashboard.available) {
                    consoleChecked = 0
                    dashboardChecked = 1
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
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            StackView {
                id: stack
                clip: true
                initialItem: terminal
                Layout.fillWidth: true
                Layout.fillHeight: true

                onCurrentItemChanged: {
                    if (currentItem === terminal) {
                        terminal.opacity = 1
                        dashboard.opacity = 0
                    }

                    else {
                        terminal.opacity = 0
                        dashboard.opacity = 1
                    }
                }

                Console {
                    id: terminal
                    enabled: opacity > 0
                    visible: opacity > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Dashboard {
                    opacity: 0
                    id: dashboard
                    enabled: opacity > 0
                    visible: opacity > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            Setup {
                id: setup
                opacity: 1
                setupMargin: 0
                Layout.fillHeight: true
                Layout.rightMargin: setupMargin
                Layout.minimumWidth: displayedWidth
                Layout.maximumWidth: displayedWidth
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
}
