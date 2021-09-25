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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

import "Windows"
import "JsonEditor"

ApplicationWindow {
    id: app

    //
    // Global properties
    //
    readonly property int spacing: 8
    property bool firstValidFrame: false
    readonly property string monoFont: qmlMain.monoFont

    //
    // We use this variable to ask the user if he/she wants to enable/disable
    // automatic update checking on the second run
    //
    property int appLaunchCount: 0
    property bool automaticUpdates: false

    //
    // Hacks to fix window maximized behavior
    //
    property bool firstChange: true
    property bool windowMaximized: false
    onVisibilityChanged: {
        if (visibility == Window.Maximized) {
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
                app.x = 100
                app.y = 100
                app.width = app.minimumWidth
                app.height = app.minimumHeight
            }

            fullScreen = false
            windowMaximized = false
        }

        // Hide splash screen
        Cpp_ModuleManager.hideSplashscreen()
    }

    //
    // Application UI status variables (used for the menubar)
    //
    readonly property bool setupVisible: setup.visible
    property alias vt100emulation: terminal.vt100emulation
    readonly property bool consoleVisible: terminal.visible
    readonly property bool dashboardVisible: dashboard.visible

    //
    // Menubar status
    //
    property bool menubarEnabled: true

    //
    // Fullscreen status
    //
    property bool fullScreen: false

    //
    // Check for updates (non-silent mode)
    //
    function checkForUpdates() {
        Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
        Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
    }

    //
    // Display about dialog
    //
    function showAbout() {
        about.show()
    }

    //
    // Display the console
    //
    function showConsole() {
        toolbar.consoleClicked()
    }

    //
    // Display the dashboard
    //
    function showDashboard() {
        toolbar.dashboardClicked()
    }

    //
    // Toggle preferences pane
    //
    function togglePreferences() {
        toolbar.setupClicked()
    }

    //
    // Clears console output
    //
    function clearConsole() {
        terminal.clearConsole()
    }

    //
    // Copy console selection
    //
    function copyConsole() {
        terminal.copy()
    }

    //
    // Hide/show menubar
    //
    function toggleMenubar() {
        app.menubarEnabled = !app.menubarEnabled
    }

    //
    // Select all console text
    //
    function selectAllConsole() {
        terminal.selectAll()
    }

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        app.fullScreen = !app.fullScreen
        if (app.fullScreen)
            app.showFullScreen()
        else
            app.showNormal()
    }

    //
    // Show donations dialog
    //
    function showDonationsDialog() {
        donations.show()
    }

    //
    // Window geometry (different minimum size for non-macOS
    // operating systems because of the global menubar in macOS)
    //
    visible: false
    minimumWidth: 1100
    title: Cpp_AppName
    minimumHeight: Qt.platform.os == "osx" ? 720 : 740

    //
    // Set user interface font
    //
    font.family: qmlMain.uiFont
    
    //
    // Define default window size to avoid issues with
    // special window managers (such as Haiku)
    //
    width: minimumWidth
    height: minimumHeight

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
        // Hide dialogs, show devices pane
        about.hide()
        setup.show()
        csvPlayer.hide()

        // Hide everything except the terminal
        dashboard.opacity = 0
        terminal.opacity = 1

        // Load welcome guide
        terminal.showWelcomeGuide()

        // Load JSON map file (if any)
        Cpp_JSON_Generator.readSettings()

        // Display the window & check for updates in 500 ms (we do this so that
        // we wait for the window to read settings before showing it)
        timer.start()
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
        onTriggered: {
            // Startup verifications to ensure that bad settings
            // do not make our app reside outside screen
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
            if (app.fullScreen)
                app.showFullScreen()
            else if (app.windowMaximized)
                app.showMaximized()
            else
                app.showNormal()

            // Increment app launch count
            ++appLaunchCount

            // Show donations dialog every 15 launches
            if (appLaunchCount % 15 == 0 && !donations.doNotShowAgain)
                donations.showAutomatically()

            // Ask user if he/she wants to enable automatic updates
            if (appLaunchCount == 2 && Cpp_UpdaterEnabled) {
                if (Cpp_Misc_Utilities.askAutomaticUpdates()) {
                    app.automaticUpdates = true
                    Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
                }

                else
                    app.automaticUpdates = false
            }

            // Check for updates (if we are allowed)
            if (automaticUpdates && Cpp_UpdaterEnabled)
                Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
        }
    }

    //
    // Hide console & device manager when we receive first valid frame
    //
    Connections {
        target: Cpp_UI_Dashboard
        enabled: !app.firstValidFrame

        function onUpdated()  {
            if ((Cpp_IO_Manager.connected || Cpp_CSV_Player.isOpen) &&
                    Cpp_UI_Dashboard.frameValid()) {
                setup.hide()
                app.showDashboard()
                app.firstValidFrame = true
            }

            else {
                setup.show()
                app.showConsole()
                app.firstValidFrame = false
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
            app.showConsole()
            app.firstValidFrame = false
        }
    }

    //
    // Save window size & position
    //
    Settings {
        property alias appX: app.x
        property alias appY: app.y
        property alias appW: app.width
        property alias appH: app.height
        property alias appStatus: app.appLaunchCount
        property alias windowFullScreen: app.fullScreen
        property alias autoUpdater: app.automaticUpdates
        property alias appMaximized: app.windowMaximized
        property alias menubarVisible: app.menubarEnabled
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
            setupChecked: app.setupVisible
            consoleChecked: app.consoleVisible
            dashboardChecked: app.dashboardVisible
            onJsonEditorClicked: jsonEditor.show()
            onSetupClicked: setup.visible ? setup.hide() : setup.show()

            onDashboardClicked: {
                if (Cpp_UI_Dashboard.available) {
                    consoleChecked = 0
                    dashboardChecked = 1
                    swipe.currentIndex = 1
                }

                else
                    app.showConsole()
            }

            onConsoleClicked: {
                consoleChecked = 1
                dashboardChecked = 0
                swipe.currentIndex = 0
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

            SwipeView {
                id: swipe
                interactive: false
                Layout.fillWidth: true
                Layout.fillHeight: true

                onCurrentIndexChanged: {
                    if (currentIndex == 0) {
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
                    id: dashboard
                    enabled: opacity > 0
                    visible: opacity > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
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

    //
    // About window
    //
    About {
        id: about
    }

    //
    // CSV player window
    //
    CsvPlayer {
        id: csvPlayer
    }

    //
    // JSON Editor dialog
    //
    JsonEditor {
        id: jsonEditor
    }

    //
    // Donations dialog
    //
    Donate {
        id: donations
    }

    //
    // JSON project drop area
    //
    JSONDropArea {
        anchors.fill: parent
        enabled: terminal.visible
    }
}
